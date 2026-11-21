"""Publish a firmware directory at a stable path."""

import argparse
import os
from pathlib import Path
import stat
import subprocess
import tempfile


def directory_link_kind(link):
    try:
        metadata = link.lstat()
    except FileNotFoundError:
        return None
    if stat.S_ISLNK(metadata.st_mode):
        return "symlink"
    # Python reports Windows junctions as mount-point reparse points.
    if os.name == "nt" and metadata.st_reparse_tag == stat.IO_REPARSE_TAG_MOUNT_POINT:
        return "junction"
    raise FileExistsError(f"Refusing to replace a regular file or directory at {link}")


def create_junction(link, target):
    env = dict(os.environ, FLY_IMAGE_LINK=str(link), FLY_IMAGE_TARGET=str(target))
    try:
        subprocess.run(
            ["powershell.exe", "-NoProfile", "-NonInteractive", "-Command",
             "$ErrorActionPreference = 'Stop'; "
             "New-Item -ItemType Junction -Path $env:FLY_IMAGE_LINK "
             "-Value $env:FLY_IMAGE_TARGET | Out-Null"],
            env=env,
            check=True,
        )
    except (OSError, subprocess.CalledProcessError) as error:
        raise OSError(f"Could not create the firmware junction at {link}. "
                      "Use a local Windows filesystem that supports directory junctions.") from error


def publish_directory(link, target):
    """Set link to target, preserving normal files and directories at link."""
    # Resolve the parent without following the link we are replacing.
    link = Path(link)
    link = link.parent.resolve(strict=True) / link.name
    target = Path(target).resolve(strict=True)
    if not target.is_dir():
        raise NotADirectoryError(target)

    existing = directory_link_kind(link)
    if existing and link.is_dir() and link.samefile(target):
        return

    # Prepare the replacement first so a creation failure leaves the old link in place.
    with tempfile.TemporaryDirectory(prefix=".fly-current-", dir=link.parent) as temp:
        pending = Path(temp) / "link"
        if os.name == "nt":
            create_junction(pending, target)
            if existing == "junction":
                link.rmdir()
            elif existing == "symlink":
                link.unlink()
        else:
            pending.symlink_to(os.path.relpath(target, link.parent), target_is_directory=True)
        pending.replace(link)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("link")
    parser.add_argument("target")
    args = parser.parse_args()
    publish_directory(args.link, args.target)
