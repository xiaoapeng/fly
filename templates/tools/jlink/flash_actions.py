"""J-Link consumes an existing common payload without altering it."""

import hashlib
import os
from pathlib import Path
import sys
import tempfile
from zipfile import ZIP_DEFLATED, ZipFile, ZipInfo

sys.path.insert(0, str(Path(os.environ["FLY_SDK_DIR"]) / "tool" / "python"))

from action_protocol import read_json
from firmware_payload import manifest_file
from jlink_support import check_tool, device_settings, materialized, offline_assets, tool, tool_path
from provider_runtime import ProviderError, provider_main, require_tool, run_child, tool_failed, validate_artifact


def _command_body(partitions, filenames):
    lines = ["r", "h"]
    for partition, filename in zip(partitions, filenames):
        if any(char in filename for char in '\r\n"'):
            raise ProviderError("INVALID_REQUEST", "J-Link command paths cannot contain quotes or newlines", exit_code=2)
        lines.append(f'loadfile "{filename}" {partition["address"]}')
    return "\n".join([*lines, "r", "g", "qc", ""])


def _commander_args(settings):
    device, interface, speed, scan_chain = settings
    return [
        "-ExitOnError", "1", "-NoGui", "1", "-AutoConnect", "1",
        "-Device", device, "-If", interface, "-Speed", speed, "-JTAGConf", scan_chain,
    ]


def flash_run(request, path, manifest, settings):
    commander = require_tool(tool(request, "jlink"), "J-Link Commander")
    partitions = manifest["partitions"]
    filenames = [tool_path(commander, manifest_file(path, part["bin"])) for part in partitions]
    identity = [
        {"name": part["name"], "address": part["address"], "sha256": part["bin"]["sha256"]}
        for part in partitions
    ]
    print(f"J-Link firmware {manifest['artifact_id']}", flush=True)
    for part in identity:
        print(f"  {part['name']} {part['address']} sha256={part['sha256']}", flush=True)
    with tempfile.TemporaryDirectory(prefix="fly-jlink-") as work:
        script = Path(work) / "download.jlink"
        script.write_text(_command_body(partitions, filenames), encoding="utf-8")
        status = run_child([commander, *_commander_args(settings), "-CommandFile", tool_path(commander, script)])
    if status:
        raise tool_failed("J-Link Commander", status)
    return {
        "artifact_id": manifest["artifact_id"], "partitions": identity,
        "record_path": None, "virtual_ms": None,
    }


def _launchers(settings):
    device, interface, speed, _ = settings
    flash_args = " ".join(_commander_args(settings))
    rtt_args = f"--device {device} --if {interface} --speed {speed}"
    cd_bin = '#!/bin/sh\nset -eu\ncd "$(dirname "$0")"\n'
    platform_case = (
        'case "$(uname -s):$(uname -m)" in\n'
        '  Darwin:*) reader=tool/jlink/macos/rtt-shell ;;\n'
        '  Linux:x86_64) reader=tool/jlink/linux-x64/rtt-shell ;;\n'
        '  Linux:i?86) reader=tool/jlink/linux-x86/rtt-shell ;;\n'
        '  *) echo "RTT reader unavailable on this host" >&2; exit 1 ;;\n'
        'esac\n'
    )
    win_init = (
        '@echo off\r\ncd /d "%~dp0" || exit /b 1\r\n'
        'set "platform=win-x64"\r\n'
        'if "%PROCESSOR_ARCHITECTURE%"=="x86" if "%PROCESSOR_ARCHITEW6432%"=="" set "platform=win-x86"\r\n'
    )
    return {
        "burn.sh": f'{cd_bin}exec "${{JLINK_EXE:-JLinkExe}}" {flash_args} -CommandFile download.jlink\n',
        "log.sh": f'{cd_bin}{platform_case}exec "$reader" {rtt_args}\n',
        "burn.bat": f'{win_init}"tool\\jlink\\%platform%\\JLink.exe" {flash_args} -CommandFile download.jlink\r\nexit /b %errorlevel%\r\n',
        "log.bat": f'{win_init}"tool\\jlink\\%platform%\\rtt-shell.exe" {rtt_args}\r\nexit /b %errorlevel%\r\n',
    }


def _files(partitions):
    for part in partitions:
        yield part["bin"]
        symbols = part["symbols"]
        if symbols:
            yield symbols["elf"]
            if symbols["debug_file"]:
                yield symbols["debug_file"]


def _digest(path):
    checksum = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            checksum.update(block)
    return checksum.hexdigest()


def export(request, path, manifest, settings):
    assets = offline_assets(request)
    missing = [str(src) for src, _ in assets if not materialized(src)]
    if missing:
        raise ProviderError("TOOL_MISSING", "Run git lfs pull in the FLY SDK for: " + ", ".join(missing))
    archive_dir = Path(request["project_root"]) / "image" / "exports" / "jlink" / request["target"] / request["build_type"]
    archive_dir.mkdir(parents=True, exist_ok=True)
    artifact_id = manifest["artifact_id"]
    archive = archive_dir / f"{artifact_id}.zip"
    partitions = manifest["partitions"]
    with tempfile.TemporaryDirectory(prefix=".export-", dir=archive_dir) as staging:
        staged = Path(staging) / archive.name
        with ZipFile(staged, "w", ZIP_DEFLATED) as output:
            output.write(path, "manifest.json")
            seen = {"manifest.json"}
            for record in _files(partitions):
                if record["path"] not in seen:
                    output.write(manifest_file(path, record), record["path"])
                    seen.add(record["path"])
            output.writestr("download.jlink", _command_body(partitions, [part["bin"]["path"] for part in partitions]))
            for name, body in _launchers(settings).items():
                entry = ZipInfo(name)
                entry.external_attr = (0o100755 if name.endswith(".sh") else 0o100644) << 16
                output.writestr(entry, body)
            for src, relative in assets:
                output.write(src, relative)
        sha256 = _digest(staged)
        os.replace(staged, archive)
    print(f"J-Link export {artifact_id}: {archive}", flush=True)
    return {"artifact_id": artifact_id, "archive_path": str(archive), "sha256": sha256}


def action(request, _session):
    if request["action"] == "flash-run" and "duration_ms" in request["options"]:
        raise ProviderError("INVALID_REQUEST", "--duration-ms is only available for simulation", exit_code=2)
    settings = device_settings(request)
    if request["operation"] == "check":
        if request["action"] == "export":
            missing = [
                {"id": relative, "message": f"{path} is missing or an LFS pointer; run git lfs pull in the SDK"}
                for path, relative in offline_assets(request) if not materialized(path)
            ]
            return {"available": not missing, "missing": missing}
        return check_tool(request, "jlink", "Install SEGGER J-Link Commander or set the jlink tool path")
    board = read_json(Path(request["board"]["manifest_path"]))["board"]
    flash = [region for region in board["memories"] if region["kind"] == "flash"]
    path, manifest = validate_artifact(request, memories=flash)
    handler = {"flash-run": flash_run, "export": export}[request["action"]]
    return handler(request, path, manifest, settings)


if __name__ == "__main__":
    raise SystemExit(provider_main(action))
