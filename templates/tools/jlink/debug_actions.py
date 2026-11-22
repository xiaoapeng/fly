"""Live RTT and GDB actions for the J-Link tool pack."""

import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

sys.path.insert(0, str(Path(os.environ["FLY_SDK_DIR"]) / "tool" / "python"))

from action_protocol import read_json
from firmware_payload import find_artifact, load_artifact, manifest_file
from gdb_provider import serve, stop
from jlink_support import check_tool, device_settings, tool
from project_actions import verify_build_symbols
from provider_runtime import executable, provider_main, require_tool, run_child, tool_failed


def _debug_profile(request):
    path = Path(__file__).with_name("pack.json")
    manifest = read_json(path)
    debug = next(action for action in manifest["actions"] if action["name"] == "debug")
    return next(profile for profile in debug["debug_profiles"] if profile["id"] == request["options"]["profile"])


def _archive_elf(request, memories):
    path = find_artifact(request["project_root"], request["target"], request["build_type"])
    board = request["board"]
    payload = load_artifact(
        path, board_id=board["id"], chip=board["chip"], core_id=board["core_id"],
        target=request["target"], build_type=request["build_type"], memories=memories,
    )
    symbols = next((part["symbols"] for part in payload["partitions"] if part["name"] == "APP"), None)
    return manifest_file(path, symbols["elf"]) if symbols else None


def _build_elf(request):
    if not request["build_dir"] or request["target"] != "default":
        return None
    project = Path(request["project_root"])
    index = read_json(project / "build" / "config" / "actions.json")
    elf = Path(index["elf_path"])
    if elf.parent != Path(request["build_dir"]) or index["build_type"] != request["build_type"]:
        return None
    debug = elf.with_suffix(".debug")
    verify_build_symbols(project, request["build_type"], elf, debug if debug.is_file() else None)
    return elf


def _find_nm(request, toolchain):
    if "nm" in request["tool_overrides"]:
        return executable(request["tool_overrides"]["nm"])
    if request["build_dir"]:
        try:
            for line in (Path(request["build_dir"]) / "CMakeCache.txt").read_text(encoding="utf-8").splitlines():
                if line.startswith("CMAKE_NM:"):
                    path = executable(line.partition("=")[2])
                    if path:
                        return path
        except OSError:
            pass
    prefix, separator, _ = toolchain.rpartition("-")
    return executable(shutil.which(f"{prefix}-nm" if separator else "nm"))


def _rtt_args(request):
    board = read_json(Path(request["board"]["manifest_path"]))["board"]
    regions = [region for region in board["memories"] if region["kind"] == "ram"]
    nm = _find_nm(request, board["toolchain"])
    if nm:
        # A published image best describes the firmware that was last flashed.
        # A checked build ELF is still useful before a package has been created.
        for resolve_elf in (lambda: _archive_elf(request, board["memories"]), lambda: _build_elf(request)):
            try:
                elf = resolve_elf()
                if not elf:
                    continue
                output = subprocess.check_output([nm, "--defined-only", elf], timeout=5, text=True)
                match = re.search(r"(?m)^([\da-fA-F]+)\s+\S\s+_SEGGER_RTT\s*$", output)
                if match:
                    address = int(match[1], 16)
                    if any(region["address"] <= address < region["address"] + region["size"] for region in regions):
                        return ["--addr", f"{address:#x}"]
            except (OSError, ValueError, KeyError, subprocess.SubprocessError):
                continue
    if regions:
        ram = next((region for region in regions if region["id"] == "app-ram"), regions[0])
        return ["--addr", f'{ram["address"]:#x}', "--range", f'{ram["size"]:#x}']
    return []


def action(request, session):
    operation = request["operation"]
    if operation == "stop":
        return stop(request)
    if operation == "check":
        if request["action"] == "log":
            return check_tool(request, "rtt-shell", "Install rtt-shell or materialize the SDK J-Link Git LFS assets")
        if request["options"].get("profile") and _debug_profile(request)["server"] == "owned":
            return check_tool(request, "jlink-gdb-server", "Install SEGGER J-Link GDB Server or set its tool path")
        return {"available": True, "missing": []}

    device, interface, speed, scan_chain = device_settings(request)
    if request["action"] == "log":
        reader = require_tool(tool(request, "rtt-shell"), "J-Link RTT reader")
        code = run_child([reader, "--device", device, "--if", interface.lower(), "--speed", speed, *_rtt_args(request)])
        if code:
            raise tool_failed("J-Link RTT reader", code)
        return {"source": "live", "artifact_id": None, "record_path": None}

    def launch(endpoint, log):
        server = require_tool(tool(request, "jlink-gdb-server"), "J-Link GDB Server")
        return subprocess.Popen(
            [server, "-device", device, "-if", interface, "-speed", speed, "-jtagconf", scan_chain,
             "-port", str(endpoint["port"]), "-localhostonly", "-singlerun", "-noreset", "-halt", "-noir"],
            stdout=log, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
        )

    return serve(
        request, session, _debug_profile(request), start_server=launch,
        ready=lambda log, endpoint: (
            f"Listening on TCP/IP port {endpoint['port']}" in log and "Waiting for GDB connection..." in log
        ),
        connected=lambda log: bool(re.search(
            r"Connected to [0-9a-fA-F:.]+|GDB closed TCP/IP connection",
            log.partition("Waiting for GDB connection...")[2],
        )),
    )


if __name__ == "__main__":
    raise SystemExit(provider_main(action))
