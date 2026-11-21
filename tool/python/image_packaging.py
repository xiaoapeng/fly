"""Create and publish an image archive shared by the probe-specific tools."""

from datetime import datetime
from itertools import count
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
from typing import NamedTuple

from image_directory_link import publish_directory
from image_packaging_state import package_is_current, package_key, package_manifest, remember_package
from mk_mcu_firmware_factory_data import append_factory_data_to_bin


FIRMWARE_PART_FORMAT = "NAME:VERSION:ADDRESS:CAPACITY:BIN"
_UNSIGNED_NUMBER = re.compile(r"[0-9]+|0[xX][0-9a-fA-F]+")


class FirmwarePart(NamedTuple):
    bin_name: str
    bin_version_str: str
    bin_version_uint32: int
    start_addr: str
    capacity: int
    file_path: str


def _parse_firmware_part(specification):
    # Leave the BIN path intact, including any colon in a Windows drive letter.
    fields = specification.split(":", 4)
    if len(fields) != 5:
        raise ValueError(f"Invalid firmware part {specification!r}. Expected {FIRMWARE_PART_FORMAT}.")
    name, version, address, capacity_text, path = fields
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_.-]*", name):
        raise ValueError(f"Invalid firmware partition name {name!r}.")
    version_match = re.fullmatch(r"[vr]?([0-9]+)\.([0-9]+)(?:\.([0-9]+))?", version.strip().lower())
    if not version_match:
        raise ValueError(f"Partition {name}: invalid version {version!r}.")
    major, minor, patch = map(int, version_match.groups(default="0"))
    if max(major, minor, patch) > 255:
        raise ValueError(f"Partition {name}: version components must be between 0 and 255.")
    encoded = (patch << 16) | (minor << 8) | major
    if not _UNSIGNED_NUMBER.fullmatch(address):
        raise ValueError(f"Partition {name}: invalid flash address {address!r}.")
    if not _UNSIGNED_NUMBER.fullmatch(capacity_text):
        raise ValueError(f"Partition {name}: invalid capacity {capacity_text!r}. Expected decimal or hexadecimal bytes.")
    base = 16 if capacity_text.lower().startswith("0x") else 10
    capacity = int(capacity_text, base)
    if capacity <= 0:
        raise ValueError(f"Partition {name}: capacity must be positive.")
    binary = Path(path)
    if not binary.is_file() or binary.stat().st_size == 0:
        raise ValueError(f"Firmware binary is missing or empty: {path}")
    return FirmwarePart(name, f"v{major}.{minor}.{patch}", encoded, address, capacity, path)


def parse_firmware_parts(specifications):
    names = set()
    parts = []
    for specification in specifications:
        part = _parse_firmware_part(specification)
        name = part.bin_name.casefold()
        if name in names:
            raise ValueError(f"Firmware partition {part.bin_name!r} was specified more than once.")
        names.add(name)
        parts.append(part)
    return parts


def _git_state():
    revision = subprocess.run(
        ["git", "rev-parse", "--verify", "HEAD"], capture_output=True, text=True,
    )
    status = subprocess.run(["git", "status", "--porcelain"], capture_output=True, text=True)
    if status.returncode:
        print(f"Error occurred: {status.stderr.strip()}")
    return (revision.stdout.strip() if revision.returncode == 0 else None,
            bool(status.returncode or status.stdout.strip()))


def _place_package(output_dir, stem, git_field, staged, archive):
    for sequence in count(1):
        suffix = f"_{sequence}" if sequence > 1 else ""
        package = output_dir / f"{stem}{suffix}_{git_field}"
        zip_path = package.with_name(package.name + ".zip")
        # Reserve this filename before inspecting or moving either output.
        claim = output_dir / f".{package.name}.publishing"
        try:
            claim.mkdir()
        except FileExistsError:
            continue
        try:
            if package.exists() or package.is_symlink() or zip_path.exists() or zip_path.is_symlink():
                continue
            staged.rename(package)
            try:
                archive.rename(zip_path)
            except BaseException:
                shutil.rmtree(package)
                raise
            return package
        finally:
            claim.rmdir()


def create_image_package(options, backend, cache_file, inputs, write_probe_files):
    output_dir = Path(options.output_dir)
    current = output_dir / options.symlink_name
    revision, dirty = _git_state()
    key = None
    if cache_file:
        binaries = [Path(part.file_path) for part in options.firmware_parts]
        key = package_key(options, [revision, dirty], [Path(__file__), *binaries, *inputs])
        if package_is_current(cache_file, key, current):
            print(f"{backend.upper()} package is up to date")
            return

    git_field = revision[:10] if revision else "uncommitted"
    if dirty:
        git_field = f"dirty_{git_field}"
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    parts = "".join(f"_{part.bin_name}{part.bin_version_str}" for part in options.firmware_parts)
    stem = f"{options.firmware_name}_{backend}{parts}_{timestamp}"

    output_dir.mkdir(parents=True, exist_ok=True)
    # Keep the previous package and alias readable until all new files are ready.
    with tempfile.TemporaryDirectory(prefix=".fly-package-", dir=output_dir) as scratch:
        staged = Path(scratch) / "package"
        staged.mkdir()
        for part in options.firmware_parts:
            binary = staged / f"{part.bin_name}.bin"
            shutil.copyfile(part.file_path, binary)
            append_factory_data_to_bin(binary, part.bin_version_uint32, part.bin_name,
                                       options.firmware_name, f"git@{git_field} time@{timestamp}")
            packed_size = binary.stat().st_size
            if packed_size > part.capacity:
                raise ValueError(
                    f"Partition {part.bin_name}: packed firmware is {packed_size} bytes, "
                    f"exceeds capacity {part.capacity} bytes by {packed_size - part.capacity} bytes."
                )
        write_probe_files(options, staged)
        archive = Path(shutil.make_archive(str(staged), "zip", staged))
        manifest = package_manifest(staged, archive) if cache_file else None

        package = _place_package(output_dir, stem, git_field, staged, archive)
        print("make_firmware_name:", package.name)
        publish_directory(current, package)
        remember_package(cache_file, key, package, manifest)
