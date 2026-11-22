"""Parse and validate firmware partition declarations."""

from pathlib import Path
import re
from typing import NamedTuple


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
