"""Build and verify probe-independent FLY firmware archives."""

import argparse
from contextlib import contextmanager
from datetime import datetime, timezone
from hashlib import sha256
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory
from zipfile import BadZipFile, ZIP_DEFLATED, ZipFile

from firmware_elf import ElfImage
from firmware_parts import parse_firmware_parts
from mk_mcu_firmware_factory_data import (
    FC_DESCRIPTION_LEN, FC_FIRMWARE_NAME_MAX_SIZE, FC_MAGIC_A, FC_MAGIC_B,
    FC_PART_NAME_MAX_SIZE, append_factory_data_to_bin, calculate_crc32, struct_format,
)


SCHEMA_VERSION = 1
TRAILER_FORMAT = "fly-factory-v1"
OBJCOPY_ARGS = ("--strip-debug", "--strip-unneeded", "-O", "binary")
OBJCOPY_RULE_VERSION = 1

_NAME = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]*")
_BOARD_ID = re.compile(r"board/[a-z0-9][a-z0-9-]*")
_HEX_ADDRESS = re.compile(r"0x(?:0|[1-9a-f][0-9a-f]*)")
_SHA256 = re.compile(r"[0-9a-f]{64}")
_VERSION = re.compile(r"v(0|[1-9][0-9]{0,2})\.(0|[1-9][0-9]{0,2})\.(0|[1-9][0-9]{0,2})")
_TRAILER_SIZE = struct.calcsize(struct_format)


class ArtifactError(ValueError):
    def __init__(self, message, code="ARTIFACT_INVALID", **details):
        super().__init__(message)
        self.code = code
        self.details = details


def _digest(content):
    return sha256(content).hexdigest()


def _canonical_json(value):
    return json.dumps(value, ensure_ascii=False, sort_keys=True, separators=(",", ":"),
                      allow_nan=False).encode("utf-8")


def _artifact_id(manifest):
    data = {key: value for key, value in manifest.items() if key != "artifact_id"}
    data["partitions"] = sorted(data["partitions"], key=lambda part: part["name"])
    return _digest(_canonical_json(data))


def _strict_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ArtifactError(f"Repeated JSON key {key!r}")
        result[key] = value
    return result


def _read_json(path):
    try:
        return json.loads(Path(path).read_text(encoding="utf-8"), object_pairs_hook=_strict_object)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise ArtifactError(f"Cannot read {path}: {error}") from error


def _keys(value, expected, label):
    if not isinstance(value, dict) or set(value) != set(expected.split()):
        raise ArtifactError(f"{label}: expected only the fields {expected}")


def _string(value, label, pattern=None):
    if not isinstance(value, str) or not value or (pattern and not pattern.fullmatch(value)):
        raise ArtifactError(f"{label}: invalid value {value!r}")
    return value


def _uint(value, label, minimum=0):
    if type(value) is not int or value < minimum:
        raise ArtifactError(f"{label}: expected integer >= {minimum}")
    return value


def _record(path, content):
    return {"path": path.as_posix(), "sha256": _digest(content), "size": len(content)}


def manifest_file(manifest_path, record):
    """Resolve a single archive file without allowing paths outside the artifact."""
    relative = _string(record["path"], "file path")
    path = PurePosixPath(relative)
    if (path.is_absolute() or "\\" in relative or ":" in relative or
            any(part in ("", ".", "..") for part in relative.split("/"))):
        raise ArtifactError(f"Unsafe archive path {relative!r}")
    try:
        root = Path(manifest_path).resolve(strict=True).parent
        candidate = root.joinpath(*path.parts).resolve(strict=True)
        if root not in candidate.parents or not candidate.is_file():
            raise ArtifactError(f"Invalid archive file {relative!r}")
    except (OSError, RuntimeError) as error:
        raise ArtifactError(f"Archive file {relative!r} is unavailable: {error}") from error
    return candidate


def _read_file(manifest_path, record, label, claimed):
    _keys(record, "path sha256 size", label)
    _string(record["sha256"], label + " hash", _SHA256)
    _uint(record["size"], label + " size", 1)
    path = manifest_file(manifest_path, record)
    if path in claimed or path == Path(manifest_path).resolve():
        raise ArtifactError(f"{label}: reused archive file {path}")
    claimed.add(path)
    try:
        if path.stat().st_size != record["size"]:
            raise ArtifactError(f"{label}: file size differs from manifest")
        content = path.read_bytes()
    except OSError as error:
        raise ArtifactError(f"{label}: cannot read {path}: {error}") from error
    if len(content) != record["size"] or _digest(content) != record["sha256"]:
        raise ArtifactError(f"{label}: file hash or size differs from manifest")
    return content


def _version_number(version):
    match = _VERSION.fullmatch(_string(version, "version"))
    if not match or any(int(v) > 255 for v in match.groups()):
        raise ArtifactError(f"Invalid firmware version {version!r}")
    major, minor, patch = map(int, match.groups())
    return (patch << 16) | (minor << 8) | major


def _trailer_fields(binary, raw_size):
    offset = (raw_size + 7) & ~7
    end = offset + _TRAILER_SIZE
    if len(binary) != ((end + 15) & ~15) or not (
            binary[raw_size:offset] == b"\xff" * (offset - raw_size)
            and binary[end:] == b"\xff" * (len(binary) - end)):
        raise ArtifactError("Invalid firmware trailer alignment or padding")
    fields = struct.unpack_from(struct_format, binary, offset)
    magic_a, magic_b, pos, length, crc = fields[:5]
    if (magic_a, magic_b, pos, length) != (FC_MAGIC_A, FC_MAGIC_B, offset, _TRAILER_SIZE):
        raise ArtifactError("Invalid firmware trailer header")
    expected = calculate_crc32(binary[:offset + 16] + b"\xff" * 4 + binary[offset + 20:end])
    if crc != expected:
        raise ArtifactError("Invalid firmware trailer CRC")
    return {"format": TRAILER_FORMAT, "offset": offset, "size": length, "crc32": f"0x{crc:08x}"}, fields


def _fixed_field(value, limit, label):
    _string(value, label)
    try:
        length = len(value.encode("utf-8"))
    except UnicodeEncodeError as error:
        raise ArtifactError(f"{label} contains invalid UTF-8 characters") from error
    if "\0" in value or length > limit:
        raise ArtifactError(f"{label} must fit in {limit} UTF-8 bytes")
    return value


def _capacity_failed(name, actual, limit):
    return ArtifactError(f"Partition {name}: packed firmware is {actual} bytes, exceeds capacity {limit} bytes",
                         "CAPACITY_EXCEEDED", partition=name, packed_size=actual, capacity=limit)


def _verify_partition(manifest_path, part, firmware_name, core_id, claimed):
    _keys(part, "name core_id version address capacity raw bin trailer origin symbols objcopy", "partition")
    name = _fixed_field(_string(part["name"], "partition name", _NAME), FC_PART_NAME_MAX_SIZE, "partition name")
    if part["core_id"] != core_id:
        raise ArtifactError(f"Partition {name}: wrong core", "ARTIFACT_MISMATCH", partition=name)
    _keys(part["raw"], "sha256 size", f"{name} raw BIN")
    _string(part["raw"]["sha256"], f"{name} raw BIN hash", _SHA256)
    raw_size = _uint(part["raw"]["size"], f"{name} raw BIN size", 1)
    final = _read_file(manifest_path, part["bin"], f"{name} final BIN", claimed)
    if len(final) > part["capacity"]:
        raise _capacity_failed(name, len(final), part["capacity"])
    raw = final[:raw_size]
    if len(raw) != raw_size or _digest(raw) != part["raw"]["sha256"]:
        raise ArtifactError(f"Partition {name}: raw BIN hash or size differs from manifest")
    try:
        trailer, fields = _trailer_fields(final, raw_size)
    except ArtifactError as error:
        raise ArtifactError(f"Partition {name}: {error}") from error
    if trailer != part["trailer"] or any(type(part["trailer"][key]) is not int for key in ("offset", "size")):
        raise ArtifactError(f"Partition {name}: trailer metadata differs from BIN")
    if (fields[5] != _version_number(part["version"]) or
            fields[6] != name.encode().ljust(FC_PART_NAME_MAX_SIZE, b"\0") or
            fields[7] != firmware_name.encode().ljust(FC_FIRMWARE_NAME_MAX_SIZE, b"\0")):
        raise ArtifactError(f"Partition {name}: trailer identity differs from manifest")

    if part["origin"] not in ("built", "external"):
        raise ArtifactError(f"Partition {name}: unknown origin")
    symbols = part["symbols"]
    if symbols is None:
        if part["origin"] == "built" or part["objcopy"] is not None:
            raise ArtifactError(f"Partition {name}: missing paired ELF")
        return
    _keys(symbols, "elf debug_file", f"{name} symbols")
    elf = _read_file(manifest_path, symbols["elf"], f"{name} ELF", claimed)
    debug = b""
    debug_name = None
    debug_file = symbols["debug_file"]
    if debug_file is not None:
        debug = _read_file(manifest_path, debug_file, f"{name} debug file", claimed)
        path = PurePosixPath(debug_file["path"])
        if path.parent != PurePosixPath(symbols["elf"]["path"]).parent:
            raise ArtifactError(f"Partition {name}: debug file must be next to ELF")
        debug_name = path.name
    _keys(part["objcopy"], "rule_version tool_version args load_address", f"{name} objcopy")
    _string(part["objcopy"]["tool_version"], f"{name} objcopy version")
    _string(part["objcopy"]["load_address"], f"{name} ELF load address", _HEX_ADDRESS)
    if (type(part["objcopy"]["rule_version"]) is not int or
            part["objcopy"]["rule_version"] != OBJCOPY_RULE_VERSION or
            part["objcopy"]["args"] != list(OBJCOPY_ARGS)):
        raise ArtifactError(f"Partition {name}: unsupported ELF export rule")
    try:
        image = ElfImage(elf)
        lma = image.verify_binary(raw, int(part["address"], 16), part["capacity"])
        if part["objcopy"]["load_address"] != hex(lma):
            raise ValueError("ELF load address differs from recorded objcopy load address")
        image.verify_debug_file(debug_name, debug)
    except ValueError as error:
        raise ArtifactError(f"Partition {name}: {error}", "ARTIFACT_MISMATCH", partition=name) from error


def _validate_ranges(partitions, memories):
    names = set()
    regions = []
    flash = None if memories is None else [
        (region["address"], region["address"] + region["size"])
        for region in memories if region["kind"] == "flash"
    ]
    for part in partitions:
        _keys(part, "name core_id version address capacity raw bin trailer origin symbols objcopy", "partition")
        name = _string(part["name"], "partition name", _NAME)
        if name.casefold() in names:
            raise ArtifactError(f"Repeated partition {name!r}")
        names.add(name.casefold())
        address = int(_string(part["address"], f"{name} address", _HEX_ADDRESS), 16)
        capacity = _uint(part["capacity"], f"{name} capacity", 1)
        if flash is not None and not any(start <= address and address + capacity <= end for start, end in flash):
            raise ArtifactError(f"Partition {name}: [{address:#x}, {address + capacity:#x}) exceeds board flash",
                                "ARTIFACT_MISMATCH", partition=name)
        regions.append((address, address + capacity, name))
    regions.sort()
    for previous, current in zip(regions, regions[1:]):
        if current[0] < previous[1]:
            raise ArtifactError(f"Partitions {previous[2]} and {current[2]} overlap")


def load_artifact(manifest_path, *, board_id=None, chip=None, core_id=None, artifact_id=None,
                  target=None, build_type=None, memories=None):
    """Validate a published firmware archive and return its manifest."""
    path = Path(manifest_path)
    manifest = _read_json(path)
    _keys(manifest, "schema_version artifact_id board_id chip core_id target build_type firmware_name "
          "created_at source partitions", "firmware manifest")
    if type(manifest["schema_version"]) is not int or manifest["schema_version"] != SCHEMA_VERSION:
        raise ArtifactError("Unsupported firmware manifest version")
    _string(manifest["artifact_id"], "artifact ID", _SHA256)
    _string(manifest["board_id"], "board ID", _BOARD_ID)
    for field in ("chip", "core_id", "target", "build_type"):
        _string(manifest[field], field, _NAME)
    for field, expected in (("board_id", board_id), ("chip", chip), ("core_id", core_id),
                            ("artifact_id", artifact_id), ("target", target), ("build_type", build_type)):
        if expected is not None and manifest[field] != expected:
            raise ArtifactError(f"Artifact {field} is {manifest[field]!r}, expected {expected!r}", "ARTIFACT_MISMATCH")
    name = _fixed_field(manifest["firmware_name"], FC_FIRMWARE_NAME_MAX_SIZE, "firmware name")
    timestamp = _string(manifest["created_at"], "creation time")
    try:
        if not timestamp.endswith("Z") or "T" not in timestamp:
            raise ValueError
        datetime.fromisoformat(timestamp[:-1] + "+00:00")
    except ValueError as error:
        raise ArtifactError("Expected UTC ISO-8601 creation time") from error
    _keys(manifest["source"], "git_commit dirty", "firmware source")
    if manifest["source"]["git_commit"] is not None:
        _string(manifest["source"]["git_commit"], "git commit", re.compile(r"[0-9a-f]{40}|[0-9a-f]{64}"))
    if type(manifest["source"]["dirty"]) is not bool:
        raise ArtifactError("Source dirty flag must be boolean")
    partitions = manifest["partitions"]
    if not isinstance(partitions, list) or not partitions:
        raise ArtifactError("Expected a nonempty partition array")
    _validate_ranges(partitions, memories)
    claimed = set()
    for part in partitions:
        _verify_partition(path, part, name, manifest["core_id"], claimed)
    if _artifact_id(manifest) != manifest["artifact_id"]:
        raise ArtifactError("Artifact ID differs from the manifest content", "ARTIFACT_MISMATCH")
    return manifest


def find_artifact(project_root, target, build_type):
    """Pin the current artifact for a specific target and build type."""
    _string(target, "target", _NAME)
    _string(build_type, "build type", _NAME)
    output = Path(project_root).resolve() / "image" / target / build_type
    current = output / "CURRENT.json"
    if not current.is_file():
        raise ArtifactError(f"No firmware package at {current}. Run make_img for {target} {build_type}.")
    pointer = _read_json(current)
    _keys(pointer, "artifact_id key zip_sha256", "current firmware pointer")
    artifact_id = _string(pointer["artifact_id"], "artifact ID", _SHA256)
    manifest_path = output / artifact_id / "manifest.json"
    try:
        resolved = manifest_path.resolve(strict=True)
        if resolved != manifest_path or not resolved.is_file():
            raise ArtifactError(f"Invalid current firmware package at {manifest_path}. Run make_img to repair it.")
    except (OSError, RuntimeError) as error:
        raise ArtifactError(f"Cannot find {manifest_path}. Run make_img to repair the package.") from error
    manifest = _read_json(resolved)
    expected = {"artifact_id": artifact_id, "target": target, "build_type": build_type}
    if not isinstance(manifest, dict) or any(manifest.get(field) != value for field, value in expected.items()):
        raise ArtifactError("Current firmware manifest does not match its pointer. Run make_img to repair it.",
                            "ARTIFACT_MISMATCH")
    return resolved


def _git_source(project_root):
    def git(*args):
        try:
            return subprocess.run(["git", "-C", str(project_root), *args], capture_output=True, text=True)
        except FileNotFoundError:
            return None
    root = git("rev-parse", "--show-toplevel")
    if not root or root.returncode or Path(root.stdout.strip()).resolve() != project_root:
        return {"git_commit": None, "dirty": True}
    revision = git("rev-parse", "--verify", "HEAD")
    status = git("status", "--porcelain")
    return {
        "git_commit": revision.stdout.strip() if revision and revision.returncode == 0 else None,
        "dirty": bool(not status or status.returncode or status.stdout.strip()),
    }


@contextmanager
def _publish_lock(path):
    with path.open("a+b") as handle:
        if os.name == "nt":
            import msvcrt
            if path.stat().st_size == 0:
                handle.write(b"\0")
                handle.flush()
            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
            try:
                yield
            finally:
                handle.seek(0)
                msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl
            fcntl.flock(handle, fcntl.LOCK_EX)
            try:
                yield
            finally:
                fcntl.flock(handle, fcntl.LOCK_UN)


def _atomic_json(destination, data):
    with TemporaryDirectory(prefix=".fly-state-", dir=destination.parent) as temp:
        pending = Path(temp) / "state.json"
        pending.write_bytes(_canonical_json(data) + b"\n")
        pending.replace(destination)


def _snapshot(parts, symbols):
    result = {}
    for part in parts:
        name = part.bin_name
        raw = Path(part.file_path).read_bytes()
        if name not in symbols:
            result[name] = (raw, None, None, None)
            continue
        elf_path, debug_path = symbols[name]
        elf = Path(elf_path).read_bytes()
        debug = Path(debug_path).read_bytes() if debug_path else None
        result[name] = (raw, elf, Path(debug_path).name if debug_path else None, debug)
    return result


def _objcopy_version(tool):
    if not tool:
        raise ArtifactError("Specify the toolchain objcopy when archiving ELF symbols", "TOOL_MISSING")
    try:
        result = subprocess.run([tool, "--version"], check=True, capture_output=True, text=True)
        version, *_ = result.stdout.splitlines()
        return version
    except (OSError, subprocess.CalledProcessError, ValueError) as error:
        raise ArtifactError(f"Unable to read objcopy version from {tool}: {error}") from error


def _write_symbols(package, name, snapshot, part, objcopy_tool, objcopy_version):
    raw, content, debug_name, debug = snapshot
    if content is None:
        return
    relative = Path("symbols") / name
    target = package / relative
    target.mkdir(parents=True)
    elf_path = relative / f"{name}.elf"
    (package / elf_path).write_bytes(content)
    debug_file = None
    if debug is not None:
        debug_path = relative / debug_name
        if debug_path == elf_path:
            raise ArtifactError(f"Partition {name}: debug filename collides with archived ELF")
        (package / debug_path).write_bytes(debug)
        debug_file = _record(debug_path, debug)

    try:
        address = int(part["address"], 16)
        elf = ElfImage(content)
        lma = elf.verify_binary(raw, address, part["capacity"])
        elf.verify_debug_file(debug_name, debug)
        with TemporaryDirectory(prefix=".fly-objcopy-", dir=package.parent) as temp:
            export = Path(temp) / "raw.bin"
            command = [objcopy_tool, *OBJCOPY_ARGS, str(package / elf_path), str(export)]
            subprocess.run(command, check=True, capture_output=True, text=True)
            if export.read_bytes() != raw:
                raise ValueError("raw BIN differs from objcopy export")
    except (ValueError, subprocess.CalledProcessError) as error:
        raise ArtifactError(f"Partition {name}: {error}", "ARTIFACT_MISMATCH", partition=name) from error
    part["symbols"] = {"elf": _record(elf_path, content), "debug_file": debug_file}
    part["objcopy"] = {"rule_version": OBJCOPY_RULE_VERSION, "tool_version": objcopy_version,
                       "args": list(OBJCOPY_ARGS), "load_address": hex(lma)}


def _matches_zip(archive, package, digest):
    try:
        if _digest(archive.read_bytes()) != digest:
            return False
        files = {path.relative_to(package).as_posix(): path for path in package.rglob("*") if path.is_file()}
        with ZipFile(archive) as zipped:
            if sorted(zipped.namelist()) != sorted(files):
                return False
            return all(zipped.read(name) == path.read_bytes() for name, path in files.items())
    except (OSError, BadZipFile):
        return False


def _cache_lookup(output, key):
    try:
        state = _read_json(output / "CURRENT.json")
        if state["key"] != key:
            return None
        identity = _string(state["artifact_id"], "artifact ID", _SHA256)
        package = output / identity
        manifest_path = package / "manifest.json"
        load_artifact(manifest_path, artifact_id=identity)
        if _matches_zip(output / f"{identity}.zip", package, state["zip_sha256"]):
            return manifest_path, identity
    except (ArtifactError, KeyError, TypeError):
        pass
    return None


def create_payload(project_root, *, target, build_type, board_id, chip, core_id, firmware_name,
                   firmware_parts, symbols=None, external_symbols=None, objcopy=None, memories=None):
    """Package raw binaries and paired symbols, then publish one artifact atomically."""
    project_root = Path(project_root).resolve(strict=True)
    for value, field in ((target, "target"), (build_type, "build type"), (chip, "chip"), (core_id, "core ID")):
        _string(value, field, _NAME)
    _string(board_id, "board ID", _BOARD_ID)
    _fixed_field(firmware_name, FC_FIRMWARE_NAME_MAX_SIZE, "firmware name")
    parts = parse_firmware_parts(firmware_parts)
    if not parts:
        raise ArtifactError("Expected at least one firmware partition")
    symbols = symbols or {}
    external_symbols = external_symbols or {}
    if symbols.keys() & external_symbols.keys():
        raise ArtifactError("A partition cannot declare both built and external symbols")
    all_symbols = {**symbols, **external_symbols}
    if all_symbols.keys() - {part.bin_name for part in parts}:
        raise ArtifactError("Symbol declaration refers to an unknown partition")
    manifest = {
        "schema_version": SCHEMA_VERSION, "board_id": board_id, "chip": chip, "core_id": core_id,
        "target": target, "build_type": build_type, "firmware_name": firmware_name,
        "source": _git_source(project_root), "partitions": [],
    }
    for part in parts:
        _fixed_field(part.bin_name, FC_PART_NAME_MAX_SIZE, "partition name")
        text = part.start_addr
        address = int(text, 16 if text.lower().startswith("0x") else 10)
        manifest["partitions"].append({
            "name": part.bin_name, "core_id": core_id, "version": part.bin_version_str,
            "address": hex(address), "capacity": part.capacity, "raw": None, "bin": None,
            "trailer": None, "origin": "built" if part.bin_name in symbols else "external",
            "symbols": None, "objcopy": None,
        })
    _validate_ranges(manifest["partitions"], memories)
    snapshot = _snapshot(parts, all_symbols)
    tool_version = _objcopy_version(objcopy) if all_symbols else None

    def snapshot_identity(entry):
        raw, elf, debug_name, debug = entry
        return [_digest(raw), _digest(elf) if elf is not None else None,
                debug_name, _digest(debug) if debug is not None else None]

    sources = ("firmware_payload.py", "firmware_elf.py", "mk_mcu_firmware_factory_data.py")
    implementation = [_digest(Path(__file__).with_name(source).read_bytes()) for source in sources]
    key = _digest(_canonical_json([manifest, {part: snapshot_identity(entry) for part, entry in snapshot.items()},
                                   tool_version, OBJCOPY_ARGS, implementation]))

    output = project_root / "image" / target / build_type
    output.mkdir(parents=True, exist_ok=True)
    with _publish_lock(output / ".publish.lock"):
        cached = _cache_lookup(output, key)
        if cached:
            path, identity = cached
        else:
            manifest["created_at"] = datetime.now(timezone.utc).isoformat(timespec="microseconds").replace("+00:00", "Z")
            path, identity = _package(output, manifest, snapshot, objcopy, tool_version, key)
        _publish_browse_pointer(project_root, target, path, identity)
        return {"artifact_id": identity, "manifest_path": str(path)}


def _publish_browse_pointer(project_root, target, path, identity):
    if target == "default":
        image_root = project_root / "image"
        # This index is only for browsing. Machines select a specific target and configuration.
        try:
            _atomic_json(image_root / "CURRENT.json", {
                "artifact_id": identity, "manifest_path": path.relative_to(image_root).as_posix(),
            })
        except OSError as error:
            print(f"Could not update the convenience image/CURRENT.json pointer: {error}", file=sys.stderr)


def _package(output, manifest, snapshot, objcopy, tool_version, key):
    revision = manifest["source"]["git_commit"]
    short = revision[:10] if revision else "uncommitted"
    git_field = ("dirty_" if manifest["source"]["dirty"] else "") + short
    stamp = manifest["created_at"][:19].replace("-", "").replace(":", "").replace("T", "_")
    description = _fixed_field(f"git@{git_field} time@{stamp}", FC_DESCRIPTION_LEN, "firmware description")
    with TemporaryDirectory(prefix=".fly-payload-", dir=output) as temp:
        package = Path(temp) / "payload"
        package.mkdir()
        for part in manifest["partitions"]:
            name = part["name"]
            raw = snapshot[name][0]
            projected = (((len(raw) + 7) & ~7) + _TRAILER_SIZE + 15) & ~15
            if projected > part["capacity"]:
                raise _capacity_failed(name, projected, part["capacity"])
            binary = package / f"{name}.bin"
            binary.write_bytes(raw)
            append_factory_data_to_bin(binary, _version_number(part["version"]), name,
                                       manifest["firmware_name"], description)
            final = binary.read_bytes()
            part["bin"] = _record(Path(binary.name), final)
            part["raw"] = {"sha256": _digest(raw), "size": len(raw)}
            part["trailer"], _ = _trailer_fields(final, len(raw))
            _write_symbols(package, name, snapshot[name], part, objcopy, tool_version)
        manifest["partitions"].sort(key=lambda part: part["name"])
        identity = manifest["artifact_id"] = _artifact_id(manifest)
        manifest_path = package / "manifest.json"
        manifest_path.write_bytes(_canonical_json(manifest) + b"\n")
        load_artifact(manifest_path, artifact_id=identity)

        archive = Path(temp) / "payload.zip"
        with ZipFile(archive, "w", compression=ZIP_DEFLATED) as zipped:
            for path in sorted(package.rglob("*")):
                if path.is_file():
                    zipped.write(path, path.relative_to(package).as_posix())
        zip_hash = _digest(archive.read_bytes())
        return _publish(output, package, archive, identity, key, zip_hash)


def _publish(output, package, archive, identity, key, zip_hash):
    destination = output / identity
    zip_path = output / f"{identity}.zip"
    if destination.exists() or zip_path.exists():
        raise ArtifactError(f"Artifact {identity} already exists and was not reused")
    destination_published = False
    zip_published = False
    try:
        package.rename(destination)
        destination_published = True
        archive.rename(zip_path)
        zip_published = True
        _atomic_json(output / "CURRENT.json", {"key": key, "artifact_id": identity, "zip_sha256": zip_hash})
    except BaseException:
        if zip_published:
            zip_path.unlink()
        if destination_published:
            shutil.rmtree(destination)
        raise
    return destination / "manifest.json", identity


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ("project-root", "target", "build-type", "board-id", "chip", "core-id", "firmware-name"):
        parser.add_argument("--" + option, required=True)
    parser.add_argument("--firmware-parts", nargs="+", required=True)
    parser.add_argument("--symbols", nargs=3, action="append", default=[], metavar=("NAME", "ELF", "DEBUG"))
    parser.add_argument("--external-symbols", nargs=3, action="append", default=[], metavar=("NAME", "ELF", "DEBUG"))
    parser.add_argument("--board-manifest")
    parser.add_argument("--objcopy")
    options = vars(parser.parse_args())
    try:
        for field in ("symbols", "external_symbols"):
            pairs = options[field]
            symbols = {name: (elf, debug if debug != "-" else None) for name, elf, debug in pairs}
            if len(symbols) != len(pairs):
                raise ArtifactError("Repeated symbols declaration")
            options[field] = symbols
        board_path = options.pop("board_manifest")
        if board_path:
            board = _read_json(board_path)
            try:
                if (board["id"] != options["board_id"] or board["board"]["chip"] != options["chip"]
                        or board["board"]["core"]["id"] != options["core_id"]):
                    raise ArtifactError("Board manifest does not match the firmware board or core")
                options["memories"] = board["board"]["memories"]
            except (KeyError, TypeError) as error:
                raise ArtifactError(f"Invalid board manifest: {error}") from error
        print(json.dumps(create_payload(**options)))
    except (OSError, ValueError) as error:
        parser.exit(1, f"{getattr(error, 'code', 'ARTIFACT_INVALID')}: {error}\n")


if __name__ == "__main__":
    main()
