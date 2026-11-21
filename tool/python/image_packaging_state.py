"""Track packaging inputs independently of a build tool's timestamp precision."""

from hashlib import sha256
import json
from pathlib import Path


def package_key(options, revision, sources):
    digest = sha256(json.dumps([options, revision]).encode("utf-8"))
    sources = [Path(__file__), Path(__file__).with_name("mk_mcu_firmware_factory_data.py"), *sources]
    for path in sources:
        if path.is_file():
            digest.update(str(path).encode("utf-8"))
            digest.update(sha256(path.read_bytes()).digest())
    return digest.hexdigest()


def package_is_current(state_file, key, current):
    if not state_file or not Path(state_file).is_file():
        return False
    current = Path(current)
    if not current.is_dir():
        return False
    package = current.resolve()
    return (
        Path(state_file).read_text(encoding="utf-8").splitlines() == [key, package.name]
        and package.with_name(package.name + ".zip").is_file()
    )


def remember_package(state_file, key, package):
    if state_file:
        Path(state_file).write_text(f"{key}\n{Path(package).name}\n", encoding="utf-8")
