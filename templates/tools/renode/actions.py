"""Renode execution, live RTT, and archive debugging."""

import base64
from datetime import datetime, timezone
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from time import monotonic, sleep
from uuid import uuid4

sys.path.insert(0, str(Path(os.environ["FLY_SDK_DIR"]) / "tool" / "python"))

from action_protocol import read_json, write_json
from firmware_payload import manifest_file
from gdb_provider import serve, stop
from project_actions import file_lock
from provider_runtime import ProviderError, executable, provider_main, require_tool, stop_child, tool_failed, validate_artifact
from run_log import RUN_LOCK, show_log


ROOT = Path(__file__).resolve().parent
RUN_STARTED = b"FLY_RENODE_RUN_STARTED"
RUN_COMPLETE = b"FLY_RENODE_RUN_COMPLETE"
RTT_LINE = re.compile(rb"FLY_RTT64 ([A-Za-z0-9+/=]+)")
EVIDENCE_LINE = re.compile(rb"FLY_BIN_VERIFIED ([a-f0-9]+) ([a-f0-9]{64})")
TOOL_ID = "renode"


def find_renode(request):
    if TOOL_ID in request["tool_overrides"]:
        return executable(request["tool_overrides"][TOOL_ID])
    for name in ("renode", "Renode.exe"):
        command = executable(shutil.which(name))
        if command:
            return command
    if sys.platform == "darwin":
        for base in (Path.home() / "Applications", Path("/Applications")):
            command = executable(base / "Renode.app" / "Contents" / "MacOS" / "renode")
            if command:
                return command
    return None


def _path_arg(path):
    value = Path(path).as_posix()
    if any(char in value for char in '\r\n"\\;'):
        raise ProviderError("INVALID_REQUEST", f"Renode command path contains unsupported characters: {path}", exit_code=2)
    return "@" + value.replace(" ", "\\ ")


def _data(request):
    project = read_json(Path(request["project_root"]) / "fly.project.json")
    manifest = read_json(ROOT / "pack.json")
    app_id = next(uid for uid in project["packs"] if uid.startswith("app/"))
    for model in manifest["models"]:
        if model["board"] == request["board"]["id"] and app_id in model["apps"]:
            return manifest, Path(request["project_root"]) / model["path"]
    raise ProviderError("INCOMPATIBLE_TARGET", f"No Renode model for {request['board']['id']} / {app_id}")


def _base_script(artifact, manifest, model, *, vector_partition=None):
    script = [
        'mach create "fly"',
        f"machine LoadPlatformDescription {_path_arg(model)}",
        f"include {_path_arg(ROOT / 'rtt_monitor.py')}",
    ]
    parts = sorted(manifest["partitions"], key=lambda part: int(part["address"], 16))
    for part in parts:
        source = manifest_file(artifact, part["bin"])
        script.append(f'sysbus LoadBinary {_path_arg(source)} {part["address"]}')
        if part["symbols"]:
            elf = manifest_file(artifact, part["symbols"]["elf"])
            script.append(f"sysbus LoadSymbolsFrom {_path_arg(elf)}")
    for part in parts:
        script.append(f'fly_verify_binary {part["address"]} {part["bin"]["size"]} "{part["bin"]["sha256"]}"')
    vector = next((part for part in parts if part["name"] == vector_partition), parts[0])
    script.append(f'cpu VectorTableOffset {vector["address"]}')
    return script


def _command(program, run_script):
    config = run_script.parent / "renode.config"
    return [
        program, "--console", "--disable-gui", "--plain",
        "--config", config, "-e", f"include {_path_arg(run_script)}",
    ]


def _log_dir(request, artifact_id):
    return (Path(request["project_root"]) / "image" / "logs" / "renode"
            / request["target"] / request["build_type"] / artifact_id)


def _stream_run(program, script_path, rtt, identity, duration):
    record_dir = script_path.parent
    record_path = record_dir / "record.json"
    console_path = record_dir / "console.log"
    record_ref = {"record_path": str(record_path)}
    expected = {(int(part["address"], 16), part["sha256"].encode("ascii")) for part in identity}
    verified = set()
    started = completed = False
    deadline = monotonic() + 30 + 3 * (duration or 0) / 1000
    pending = b""

    def consume(chunk):
        nonlocal pending, started, completed
        lines = (pending + chunk).split(b"\n")
        pending = lines.pop()
        for raw in lines:
            line = raw.strip()
            evidence = EVIDENCE_LINE.fullmatch(line)
            if evidence:
                verified.add((int(evidence[1], 16), evidence[2]))
            if (b"Memory digest mismatch" in line or line == RUN_STARTED and not expected <= verified):
                raise ProviderError("ARTIFACT_MISMATCH", f"Renode did not load the verified firmware. See {console_path}",
                                    data=record_ref)
            if b"There was an error executing command" in line:
                raise ProviderError("EXECUTION_ERROR", f"Renode monitor command failed. See {console_path}",
                                    data=record_ref)
            if line == RUN_STARTED:
                started = True
            elif line == RUN_COMPLETE:
                completed = True
            else:
                entry = RTT_LINE.fullmatch(line)
                if entry and started:
                    payload = base64.b64decode(entry[1], validate=True)
                    rtt.write(payload)
                    rtt.flush()
                    sys.stdout.buffer.write(payload)
                    sys.stdout.buffer.flush()

    with console_path.open("wb") as console, console_path.open("rb") as output:
        with subprocess.Popen(
            _command(program, script_path), cwd=record_dir, stdin=subprocess.DEVNULL,
            stdout=console, stderr=subprocess.STDOUT, start_new_session=os.name != "nt",
        ) as process:
            try:
                while True:
                    exit_value = process.poll()
                    chunk = output.read(65536)
                    if chunk:
                        consume(chunk)
                    elif exit_value is not None:
                        consume(b"\n")
                        break
                    else:
                        sleep(0.025)
                    if (duration is not None or not started) and monotonic() >= deadline:
                        raise ProviderError("TIMEOUT", f"Renode exceeded the wall-clock time limit. See {console_path}",
                                            data=record_ref)
            except KeyboardInterrupt:
                stop_child(process)
                while chunk := output.read(65536):
                    consume(chunk)
                consume(b"\n")
                raise
            finally:
                stop_child(process)
    if exit_value:
        raise tool_failed("Renode", exit_value, data=record_ref)
    if not started or not completed or duration is None:
        raise ProviderError("EXECUTION_ERROR", f"Renode exited before completing the requested run. See {console_path}",
                            data=record_ref)


def _run(request, artifact, manifest, model):
    program = require_tool(find_renode(request), "Renode")
    duration = request["options"].get("duration_ms")
    if duration is not None and (type(duration) is not int or not 1 <= duration <= 60000):
        raise ProviderError("INVALID_REQUEST", "Renode duration must be 1 to 60000 ms", exit_code=2)
    run_id = str(uuid4())
    record_dir = _log_dir(request, manifest["artifact_id"]) / run_id
    record_dir.mkdir(parents=True)
    record_path = record_dir / "record.json"
    script_path = record_dir / "run.resc"
    rtt_path = record_dir / "rtt.log"
    identity = [
        {"name": part["name"], "address": part["address"], "sha256": part["bin"]["sha256"]}
        for part in manifest["partitions"]
    ]
    script = [*_base_script(artifact, manifest, model), f"fly_run {duration or 0}", "quit"]
    script_path.write_text("\n".join(script) + "\n", encoding="utf-8")
    print(f"Renode firmware {manifest['artifact_id']}", flush=True)
    for part in identity:
        print(f"  {part['name']} {part['address']} sha256={part['sha256']}", flush=True)
    record = {
        "schema_version": 1, "run_id": run_id, "status": "running", "message": "",
        "started_at": datetime.now(timezone.utc).isoformat(), "completed_at": None,
        "provider": request["provider"], "artifact_id": manifest["artifact_id"],
        "target": request["target"], "build_type": request["build_type"],
        "manifest_path": str(artifact), "model": str(model), "virtual_ms": None,
        "requested_virtual_ms": duration, "exit_code": None,
        "partitions": identity, "rtt_log": rtt_path.name, "console_log": "console.log",
    }
    with file_lock(record_dir / RUN_LOCK), rtt_path.open("wb") as rtt:
        write_json(record_path, record)
        try:
            _stream_run(program, script_path, rtt, identity, duration)
            record.update(status="ok", virtual_ms=duration, exit_code=0)
        except KeyboardInterrupt:
            record.update(status="cancelled", message="Renode run was cancelled")
            raise ProviderError("CANCELLED", record["message"], exit_code=130,
                                data={"record_path": str(record_path)}) from None
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            data = error.data if isinstance(error, ProviderError) else {}
            record.update(status="error", message=str(error), exit_code=data.get("tool_exit_code"))
            raise
        finally:
            record["completed_at"] = datetime.now(timezone.utc).isoformat()
            write_json(record_path, record)
    return {
        "artifact_id": manifest["artifact_id"], "partitions": identity,
        "record_path": str(record_path), "virtual_ms": duration,
    }


def _debug(request, session, manifest, model, pack):
    program = require_tool(find_renode(request), "Renode")
    declaration = next(action for action in pack["actions"] if action["name"] == "debug")
    profile = next(item for item in declaration["debug_profiles"] if item["id"] == request["options"]["profile"])
    artifact, payload = manifest

    def launch(endpoint, log):
        script = _base_script(artifact, payload, model, vector_partition=request["options"]["partition"])
        port = endpoint["port"]
        script.append(f"machine StartGdbServer {port} false")
        script.append(f"fly_watch_gdb {port}")
        script_path = session.parent / "debug.resc"
        script_path.write_text("\n".join(script) + "\n", encoding="utf-8")
        return subprocess.Popen(
            _command(program, script_path), cwd=session.parent, stdin=subprocess.PIPE,
            stdout=log, stderr=subprocess.STDOUT,
        )

    def ready(log, endpoint):
        if "Memory digest mismatch" in log:
            raise ProviderError("ARTIFACT_MISMATCH", f"Renode loaded different bytes. See {session}.log")
        if "There was an error executing command" in log:
            raise ProviderError("SESSION_NOT_READY", f"Renode could not start GDB. See {session}.log")
        return (f"GDB server with all CPUs started on port :{endpoint['port']}" in log
                and "FLY_GDB_WATCH_READY" in log)

    return serve(
        request, session, profile, start_server=launch, artifact=manifest,
        ready=ready, connected=lambda log: "FLY_GDB_CONNECTED" in log,
        disconnected=lambda log: "FLY_GDB_DISCONNECTED" in log,
    )


def action(request, session):
    operation = request["operation"]
    if operation == "stop":
        return stop(request)
    pack, model = _data(request)
    if operation == "check":
        missing = []
        if request["action"] != "log" and not find_renode(request):
            missing.append({"id": TOOL_ID, "message": "Install Renode or set the renode tool path"})
        if not model.is_file() and request["action"] != "log":
            missing.append({"id": "model", "message": f"Model not found: {model}"})
        return {"available": not missing, "missing": missing}
    artifact, manifest = validate_artifact(request)
    if request["action"] == "flash-run":
        return _run(request, artifact, manifest, model)
    if request["action"] == "log":
        artifact_id = manifest["artifact_id"]
        return show_log(_log_dir(request, artifact_id), artifact_id, request["target"], request["build_type"])
    if request["action"] == "debug":
        return _debug(request, session, (artifact, manifest), model, pack)
    raise ProviderError("INVALID_REQUEST", "Unknown Renode action", exit_code=2)


if __name__ == "__main__":
    raise SystemExit(provider_main(action))
