"""Track packaging inputs and the artifacts needed to reuse a published image."""

from hashlib import sha256
import json
from pathlib import Path
from stat import S_ISREG
from tempfile import TemporaryDirectory


def package_key(options, revision, sources):
    digest = sha256(json.dumps([options, revision]).encode("utf-8"))
    sources = [Path(__file__), Path(__file__).with_name("mk_mcu_firmware_factory_data.py"), *sources]
    for path in sources:
        if path.is_file():
            digest.update(str(path).encode("utf-8"))
            digest.update(sha256(path.read_bytes()).digest())
    return digest.hexdigest()


def package_manifest(directory, archive):
    return {
        "files": {
            artifact.relative_to(directory).as_posix(): artifact.stat().st_size
            for artifact in directory.rglob("*") if artifact.is_file()
        },
        "archive_size": archive.stat().st_size,
    }


def _matches_size(path, size):
    if type(size) is not int or size < 0:
        return False
    entry = path.stat()
    return S_ISREG(entry.st_mode) and entry.st_size == size


def package_is_current(state_file, key, current):
    if not state_file:
        return False
    try:
        state = json.loads(Path(state_file).read_text(encoding="utf-8"))
        package = Path(current).resolve(strict=True)
        if not isinstance(state, dict) or state.get("key") != key or state.get("package") != package.name:
            return False
        files = state.get("files")
        if not package.is_dir() or not isinstance(files, dict) or not files:
            return False
        for name, size in files.items():
            relative = Path(name)
            if (not relative.parts or relative.anchor or ".." in relative.parts
                    or not _matches_size(package / relative, size)):
                return False
        return _matches_size(package.with_name(package.name + ".zip"), state.get("archive_size"))
    except (OSError, RuntimeError, ValueError):
        return False


def remember_package(state_file, key, package, manifest):
    if not state_file:
        return
    destination = Path(state_file)
    state = {"key": key, "package": package.name, **manifest}
    with TemporaryDirectory(dir=destination.parent, prefix=".fly-package-state-") as scratch:
        staged = Path(scratch) / "state"
        staged.write_text(json.dumps(state, sort_keys=True) + "\n", encoding="utf-8")
        staged.replace(destination)
