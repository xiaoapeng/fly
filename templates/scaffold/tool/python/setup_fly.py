"""Install this project's FLY SDK from fly.lock using only Git and Python."""

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


MANAGED_MARKER = ".git/fly-managed"
SDK_ENTRY_FILES = ("tool/python/build.py", "cmake/fly-configure.cmake")
SETUP_COMMAND = "./build.sh setup or build.bat setup on Windows"
RUN_SETUP = f"Run {SETUP_COMMAND}."
LOCAL_SDK_RECOVERY = (
    "Select the locked SDK commit. To install from fly.lock, clear the applicable "
    "FLY_ROOT, .fly-root, or CMake FLY_SDK_DIR override, or move the unmanaged fly path aside, "
    f"then run {SETUP_COMMAND}."
)


def git(directory, *args, capture=False):
    result = subprocess.run(
        ["git", "-C", str(directory), *args],
        check=True,
        text=True,
        stdout=subprocess.PIPE if capture else None,
    )
    return result.stdout.strip() if capture else None


def read_lock(project):
    try:
        with (project / "fly.lock").open(encoding="utf-8") as lock:
            pin = json.load(lock)
    except json.JSONDecodeError as error:
        raise ValueError("fly.lock is not valid JSON. Ask the project maintainer for the pin.") from error
    if not isinstance(pin, dict):
        raise ValueError("fly.lock must be a JSON object with url and commit fields.")
    return pin


def is_commit(value):
    return isinstance(value, str) and re.fullmatch(r"[0-9a-fA-F]{40}(?:[0-9a-fA-F]{24})?", value) is not None


def read_pin(project):
    pin = read_lock(project)
    url = pin.get("url")
    commit = pin.get("commit")
    if not isinstance(url, str) or not url.strip() or not is_commit(commit):
        raise ValueError("fly.lock needs a repository URL and a full FLY commit. Ask the project maintainer for the pin.")
    return url, commit.lower()


def is_sdk(directory):
    return all((directory / entry).is_file() for entry in SDK_ENTRY_FILES)


def is_managed(directory):
    # A junction can expose a different project's managed SDK on Windows.
    return (
        not directory.is_symlink()
        and (directory / MANAGED_MARKER).is_file()
        and directory.resolve() == directory.parent.resolve() / directory.name
    )


def head(directory):
    return git(directory, "rev-parse", "--verify", "HEAD", capture=True)


def configured_origin(directory):
    return git(directory, "config", "--local", "--get", "remote.origin.url", capture=True)


def has_changes(directory):
    return bool(git(directory, "status", "--porcelain", capture=True))


def check_local_pin(project, sdk):
    try:
        pin = read_lock(project)
    except FileNotFoundError:
        return
    # Generation from an SDK archive records a null commit for local use.
    if "commit" in pin and pin["commit"] is None:
        return
    expected = pin.get("commit")
    if not is_commit(expected):
        raise ValueError("fly.lock needs a full FLY commit. Ask the project maintainer for the pin.")

    result = subprocess.run(
        ["git", "-C", str(sdk), "rev-parse", "--show-toplevel", "HEAD"],
        capture_output=True,
        text=True,
        encoding=sys.getfilesystemencoding(),
        errors="surrogateescape",
    )
    # Git can walk up to the application's repository when the SDK has no Git metadata.
    lines = result.stdout.rstrip("\n").rsplit("\n", 1)
    if (result.returncode != 0 or len(lines) != 2 or not is_commit(lines[1])
            or Path(lines[0]).resolve() != sdk.resolve()):
        raise ValueError(f"Cannot verify the commit of the selected FLY SDK at {sdk}. {LOCAL_SDK_RECOVERY}")
    actual = lines[1]
    expected = expected.lower()
    if actual != expected:
        raise ValueError(
            f"The selected FLY SDK at {sdk} differs from fly.lock "
            f"(expected {expected}, found {actual}). {LOCAL_SDK_RECOVERY}"
        )


def check_managed_sdk(project, sdk):
    url, expected = read_pin(project)
    actual = head(sdk)
    if actual != expected:
        raise ValueError(
            f"The installed FLY SDK differs from fly.lock (expected {expected}, found {actual}). "
            f"Clear any FLY_ROOT override, then run {SETUP_COMMAND}."
        )
    if configured_origin(sdk) != url:
        raise ValueError(
            f"The installed FLY SDK repository differs from fly.lock. Move the existing {sdk} aside. {RUN_SETUP}"
        )
    if has_changes(sdk):
        raise ValueError(
            "The installed FLY SDK has local changes. Use FLY_ROOT to build with a separate local checkout "
            "at the locked commit."
        )


def check_selected_sdk(project, sdk):
    if not is_sdk(sdk):
        raise ValueError(f"A compatible FLY SDK was not found at {sdk}. Check the selected path. {RUN_SETUP}")
    installed = project / "fly"
    if is_managed(installed) and installed.resolve() == sdk.resolve():
        check_managed_sdk(project, installed)
    else:
        check_local_pin(project, sdk)


def checkout(directory, commit):
    git(directory, "fetch", "--depth=1", "--no-tags", "origin", commit)
    if git(directory, "rev-parse", "--verify", "FETCH_HEAD", capture=True) != commit:
        raise ValueError("Git fetched a different commit than the one in fly.lock.")
    try:
        for entry in SDK_ENTRY_FILES:
            git(directory, "cat-file", "-e", f"{commit}:{entry}")
    except subprocess.CalledProcessError as error:
        raise ValueError("The locked FLY commit does not support application-root CMake. Update fly.lock.") from error
    git(directory, "checkout", "--detach", commit)


def install(project, sdk, url, commit):
    with tempfile.TemporaryDirectory(prefix=".fly-setup-", dir=project) as temp:
        staging = Path(temp)
        object_format = ["--object-format=sha256"] if len(commit) == 64 else []
        git(staging, "init", "--quiet", *object_format)
        git(staging, "remote", "add", "--", "origin", url)
        checkout(staging, commit)
        if not is_sdk(staging):
            raise ValueError("The locked commit does not contain a FLY SDK.")
        (staging / MANAGED_MARKER).touch()
        if os.path.lexists(sdk):
            raise FileExistsError(f"{sdk} appeared during setup. Run setup again to inspect it.")
        staging.rename(sdk)


def setup(project, sdk):
    override = os.environ.get("FLY_ROOT")
    stored_path = project / ".fly-root"
    if override or (not os.path.lexists(sdk) and stored_path.is_file()):
        path = Path(override or stored_path.read_text(encoding="utf-8").rstrip("\r\n")).expanduser()
        check_selected_sdk(project, path)
        print(f"Using local FLY SDK at {path.resolve()}")
        return
    if os.path.lexists(sdk) and not is_managed(sdk):
        if not is_sdk(sdk):
            raise ValueError(f"{sdk} already exists. Point FLY_ROOT at a compatible FLY SDK.")
        check_local_pin(project, sdk)
        print(f"Using local FLY SDK at {sdk.resolve()}")
        return

    url, commit = read_pin(project)
    if is_managed(sdk):
        if configured_origin(sdk) != url:
            raise ValueError(f"The URL in fly.lock changed. Move your existing {sdk} checkout before running setup.")
        if has_changes(sdk):
            raise ValueError(f"{sdk} contains local changes. Review them before running setup.")
        if head(sdk) != commit:
            branch = git(sdk, "rev-parse", "--abbrev-ref", "HEAD", capture=True)
            if branch != "HEAD":
                raise ValueError(f"{sdk} is on branch {branch}. Setup updates detached checkouts only.")
            checkout(sdk, commit)
    else:
        install(project, sdk, url, commit)
    if not is_sdk(sdk):
        raise ValueError("The locked FLY commit does not support application-root CMake. Update fly.lock.")
    print(f"FLY SDK ready at {commit} in {sdk}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--sdk-dir", type=Path, help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.sdk_dir is not None and not args.check:
        parser.error("--sdk-dir requires --check")
    project = Path(__file__).resolve().parents[2]
    sdk = project / "fly"
    try:
        if args.check:
            if args.sdk_dir is not None:
                check_selected_sdk(project, args.sdk_dir.expanduser())
            elif is_managed(sdk):
                check_managed_sdk(project, sdk)
        else:
            setup(project, sdk)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"FLY SDK: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
