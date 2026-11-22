"""J-Link tool discovery and board device settings."""

from pathlib import Path
import platform
import shutil
import subprocess

from action_protocol import read_json
from provider_runtime import ProviderError, executable


OFFLINE_ASSETS = (
    "linux-x64/rtt-shell", "linux-x86/rtt-shell", "macos/rtt-shell",
    "win-x64/JLink.exe", "win-x64/JLink_x64.dll", "win-x64/rtt-shell.exe",
    "win-x86/JLink.exe", "win-x86/JLinkARM.dll", "win-x86/rtt-shell.exe",
)


def device_settings(request):
    manifest = read_json(Path(__file__).with_name("pack.json"))
    board = manifest["compatibility"]["board_data"].get(request["board"]["id"])
    if not board:
        raise ProviderError("INCOMPATIBLE_TARGET", f"No J-Link device for {request['board']['id']}")
    config = manifest["settings"]
    return board["device"], config["interface"], str(config["speed"]), config["scan_chain"]


def asset_directory(request):
    return Path(request["sdk_root"]) / "templates" / "tools" / "jlink" / "assets"


def materialized(path):
    if not path.is_file():
        return False
    with path.open("rb") as stream:
        header = stream.read(80)
    return bool(header) and not header.startswith(b"version https://git-lfs.github.com/spec/v1")


def offline_assets(request):
    root = asset_directory(request)
    return [(root / name, f"tool/jlink/{name}") for name in OFFLINE_ASSETS]


def _windows_variant():
    return "win-x86" if platform.machine().lower() in {"x86", "i386", "i686"} else "win-x64"


def _asset_executable(request, name):
    path = asset_directory(request) / name
    return executable(path) if materialized(path) else None


def _console(names):
    for name in names:
        path = executable(shutil.which(name))
        if not path and platform.system() == "Darwin":
            path = executable(Path("/Applications/SEGGER/JLink") / name)
        if path:
            return path
    return None


def tool(request, tool_id):
    overrides = request["tool_overrides"]
    if tool_id in overrides:
        return executable(overrides[tool_id])
    system = platform.system()
    if tool_id == "jlink":
        if system == "Windows":
            return _console(("JLink.exe",)) or _asset_executable(request, f"{_windows_variant()}/JLink.exe")
        return _console(("JLinkExe",))
    if tool_id == "jlink-gdb-server":
        names = (("JLinkGDBServerCL.exe",) if system == "Windows" else
                 ("JLinkGDBServerCLExe", "JLinkGDBServerCL", "JLinkGDBServer"))
        return _console(names)
    path = _console(("rtt-shell.exe",) if system == "Windows" else ("rtt-shell",))
    if path:
        return path
    if system == "Windows":
        return _asset_executable(request, f"{_windows_variant()}/rtt-shell.exe")
    if system == "Darwin":
        return _asset_executable(request, "macos/rtt-shell")
    if system == "Linux":
        machine = platform.machine().lower()
        if machine in {"x86_64", "amd64"}:
            return _asset_executable(request, "linux-x64/rtt-shell")
        if machine in {"i386", "i686", "x86"}:
            return _asset_executable(request, "linux-x86/rtt-shell")
    return None


def check_tool(request, tool_id, hint):
    missing = [] if tool(request, tool_id) else [{"id": tool_id, "message": hint}]
    return {"available": not missing, "missing": missing}


def tool_path(program, path):
    if program.suffix.lower() == ".exe" and platform.system() == "Linux":
        if "microsoft" in platform.release().lower():
            return subprocess.check_output(["wslpath", "-w", str(path)], text=True).strip()
    return str(path)
