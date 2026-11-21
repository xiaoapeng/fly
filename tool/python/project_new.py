"""Generate an independent FLY project from a supported board and template."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from urllib.parse import urlsplit

from vscode_project import write_vscode_scaffold


TEMPLATE_LABELS = {
    "baremetal": "Bare metal",
    "baremetal-nanosig": "Bare metal + nanosig",
    "freertos-nanosig": "FreeRTOS + nanosig",
}

PROJECT_NAME = re.compile(r"[A-Za-z0-9][A-Za-z0-9_-]*")
RESERVED_NAME = re.compile(r"(?:CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])", re.IGNORECASE)
GIT_COMMIT = re.compile(r"[0-9a-f]{40}(?:[0-9a-f]{24})?")
SSH_ORIGIN = re.compile(r"[\w.-]+@([\w.-]+):([^\s]+)")


def add_parser(subparsers):
    parser = subparsers.add_parser("new", help="Generate an independent FLY project.")
    parser.add_argument("name", nargs="?", help="Project name.")
    parser.add_argument("--board", help="Board ID (for example frdm-mcxn947-core0).")
    parser.add_argument("--template", choices=TEMPLATE_LABELS, help="Application template.")
    parser.add_argument(
        "--output", type=Path,
        help="Destination directory (defaults to ../fly-project/<name> next to FLY).",
    )


def load_boards(templates_dir):
    boards = {}
    for manifest in sorted((templates_dir / "boards").glob("*/board.json")):
        board = json.loads(manifest.read_text(encoding="utf-8"))
        if not isinstance(board, dict):
            raise ValueError(f"Board manifest must be an object: {manifest}")
        for key in ("id", "label", "chip", "core", "arch_config", "toolchain", "probe"):
            if not isinstance(board.get(key), str) or not board[key]:
                raise ValueError(f"Board manifest needs {key}: {manifest}")
        if board["id"] != manifest.parent.name:
            raise ValueError(f"Board ID must match its directory: {manifest}")
        if not re.fullmatch(r"CONFIG_ARCH_[A-Z0-9_]+", board["arch_config"]):
            raise ValueError(f"Invalid architecture symbol: {manifest}")
        supported = board.get("templates")
        if not isinstance(supported, list) or not supported or not all(
            isinstance(name, str) and name in TEMPLATE_LABELS
            and (templates_dir / "apps" / name / "defconfig").is_file()
            for name in supported
        ):
            raise ValueError(f"Board manifest lists missing or unknown templates: {manifest}")
        clock_hz = board.get("clock_hz")
        if (clock_hz is not None or "freertos-nanosig" in supported) and (
            type(clock_hz) is not int or clock_hz <= 0
        ):
            raise ValueError(f"Board clock_hz must be a positive integer: {manifest}")
        boards[board["id"]] = board
    if not boards:
        raise ValueError(f"No boards found in {templates_dir / 'boards'}")
    return boards


def choose(title, options):
    if len(options) == 1:
        return next(iter(options))
    numbered = {str(index): key for index, key in enumerate(options, start=1)}
    print(title)
    for index, key in numbered.items():
        print(f"  {index}. {options[key]} ({key})")
    while True:
        selected = input("Enter a number or ID: ").strip()
        if selected in options:
            return selected
        if selected in numbered:
            return numbered[selected]
        print("Select one of the listed options.")


def select_project(boards, args):
    if not sys.stdin.isatty() and not all((args.name, args.board, args.template)):
        raise ValueError("Non-interactive generation needs <name>, --board and --template.")
    name = args.name if args.name is not None else input("Project name: ").strip()
    if not PROJECT_NAME.fullmatch(name) or RESERVED_NAME.fullmatch(name):
        raise ValueError("Use a portable project name with letters, digits, '-' or '_', starting with a letter or digit.")

    candidates = {
        board_id: board for board_id, board in boards.items()
        if args.template is None or args.template in board["templates"]
    }
    if args.board is not None:
        if args.board not in boards:
            raise ValueError(f"Unknown board {args.board!r}. Available boards: {', '.join(boards)}")
        if args.board not in candidates:
            supported = ", ".join(boards[args.board]["templates"])
            raise ValueError(f"{args.board} supports these templates: {supported}")
        board = boards[args.board]
    else:
        if not candidates:
            raise ValueError(f"No boards support {args.template}.")
        chips = dict.fromkeys(board["chip"] for board in candidates.values())
        chip = choose("Select a chip", {chip: chip for chip in chips})
        board_id = choose(
            "Select a board",
            {
                board_id: f'{board["label"]} / {board["core"]}'
                for board_id, board in candidates.items() if board["chip"] == chip
            },
        )
        board = candidates[board_id]

    template = args.template or choose(
        "Select an application template",
        {key: TEMPLATE_LABELS[key] for key in board["templates"]},
    )
    return name, board, template


def shareable_origin(origin):
    if not origin or any(character.isspace() for character in origin):
        return None
    ssh_origin = SSH_ORIGIN.fullmatch(origin)
    if ssh_origin:
        host, path = ssh_origin.groups()
        if host.lower() == "github.com":
            repository = path.lstrip("/")
            if re.fullmatch(r"[\w.-]+/[\w.-]+", repository):
                return f"https://github.com/{repository}"
            return None
        return origin
    try:
        url = urlsplit(origin)
        if not url.hostname or not url.path or url.query or url.fragment or url.password:
            return None
        if url.scheme in ("http", "https") and url.username is None:
            return origin
        if url.scheme in ("ssh", "git"):
            return origin
    except ValueError:
        pass
    return None


def git_output(sdk_dir, *args):
    result = subprocess.run(
        ["git", "-C", str(sdk_dir), *args], capture_output=True, text=True,
    )
    return result.stdout.strip() if result.returncode == 0 else ""


def sdk_lock(sdk_dir):
    # Read the configured URL before Git applies local URL rewrites.
    origin = shareable_origin(git_output(sdk_dir, "config", "--local", "--get", "remote.origin.url"))
    commit = git_output(sdk_dir, "rev-parse", "--verify", "HEAD")
    entry = git_output(sdk_dir, "cat-file", "-t", "HEAD:cmake/fly-configure.cmake")
    if not GIT_COMMIT.fullmatch(commit) or entry != "blob":
        commit = None
    return {"url": origin, "commit": commit}


def generate(sdk_dir, destination, name, board, template):
    templates_dir = sdk_dir / "templates"
    lock = sdk_lock(sdk_dir)
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Reserve this name before copying. Concurrent generators or an existing
    # directory, file, or symlink cannot be overwritten.
    destination.mkdir()
    try:
        shutil.copytree(templates_dir / "scaffold", destination, dirs_exist_ok=True)
        shutil.copytree(templates_dir / "boards" / board["id"], destination / "board")
        shutil.copytree(templates_dir / "apps" / template, destination, dirs_exist_ok=True)
        write_vscode_scaffold(destination)
        defconfig = destination / "defconfig"
        selected = defconfig.read_text(encoding="utf-8")
        if "@BOARD_CLOCK_HZ@" in selected:
            selected = selected.replace("@BOARD_CLOCK_HZ@", str(board["clock_hz"]))
        board_config = destination / "board/defconfig"
        board_defaults = board_config.read_text(encoding="utf-8").rstrip() + "\n" if board_config.is_file() else ""
        defconfig.write_text(f'{board["arch_config"]}=y\n{board_defaults}{selected}', encoding="utf-8")

        cmake_file = destination / "CMakeLists.txt"
        cmake_file.write_text(
            cmake_file.read_text(encoding="utf-8").replace("@PROJECT_NAME@", name),
            encoding="utf-8",
        )

        readme = destination / "README.md"
        content = readme.read_text(encoding="utf-8")
        fields = {
            "@PROJECT_NAME@": name,
            "@BOARD_LABEL@": board["label"],
            "@BOARD_CORE@": board["core"],
            "@TOOLCHAIN@": board["toolchain"],
            "@PROBE@": board["probe"],
            "@TEMPLATE_ID@": template,
            "@EXPECTED_LOG@": "boot ok" if template == "baremetal" else "nanosig ready",
        }
        for token, value in fields.items():
            content = content.replace(token, value)
        readme.write_text(content, encoding="utf-8")

        (destination / "fly.lock").write_text(json.dumps(lock, indent=2) + "\n", encoding="utf-8")
        if os.name == "nt":
            (destination / ".fly-root").write_text(f"{sdk_dir}\n", encoding="utf-8")
        else:
            (destination / "build.sh").chmod(0o755)
            (destination / "fly").symlink_to(os.path.relpath(sdk_dir, destination), target_is_directory=True)
        subprocess.run(["git", "init", "--quiet"], cwd=destination, check=True)
        # The Git index keeps the launcher executable on filesystems without mode bits.
        subprocess.run(["git", "add", "--chmod=+x", "--", "build.sh"], cwd=destination, check=True)
    except BaseException:
        shutil.rmtree(destination)
        raise
    return lock


def run(sdk_dir, args):
    sdk_dir = Path(sdk_dir).resolve()
    try:
        name, board, template = select_project(load_boards(sdk_dir / "templates"), args)
        output = args.output.expanduser() if args.output is not None else sdk_dir.parent / "fly-project" / name
        destination = output.parent.resolve() / output.name
        lock = generate(sdk_dir, destination, name, board, template)
    except FileExistsError as exc:
        print(f"Cannot create project because a path already exists: {exc.filename}", file=sys.stderr)
        return 1
    except (ValueError, OSError, EOFError, subprocess.CalledProcessError) as exc:
        print(f"Cannot create project: {exc}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nProject generation cancelled.", file=sys.stderr)
        return 130

    print(f"Created {destination}")
    print(f'{board["label"]} / {board["core"]} | {TEMPLATE_LABELS[template]}')
    if lock["commit"] is None:
        print(
            "FLY HEAD does not support application-root CMake. "
            "Set fly.lock to a published FLY commit before sharing this project.",
            file=sys.stderr,
        )
    elif git_output(sdk_dir, "status", "--porcelain"):
        print("fly.lock records FLY HEAD. Local SDK changes apply only to this checkout.", file=sys.stderr)
    if lock["url"] is None:
        print("Set a shareable FLY repository URL in fly.lock before sharing this project.", file=sys.stderr)
    print("To build, run ./build.sh (or build.bat on Windows) from the new project.")
    return 0


def main():
    parser = argparse.ArgumentParser(prog="fly", description=__doc__)
    add_parser(parser.add_subparsers(dest="command", required=True))
    return run(Path(__file__).resolve().parents[2], parser.parse_args())


if __name__ == "__main__":
    sys.exit(main())
