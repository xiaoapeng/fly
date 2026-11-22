"""Provider-side lifetime of a GDB server and its ready record."""

from pathlib import Path
import socket
import time
from uuid import uuid4

from action_protocol import read_json, write_json
from firmware_payload import manifest_file
from provider_runtime import ProviderError, stop_child, tool_failed, validate_artifact


def _related(path, suffix):
    return path.with_name(path.name + suffix)


def _matches(path, identity):
    return path.is_file() and read_json(path) == identity


def _endpoint(request):
    options = request["options"]
    endpoint = options["endpoint"]
    if options["server"] == "external":
        return endpoint
    host = endpoint["host"] if endpoint else "127.0.0.1"
    if host not in ("127.0.0.1", "localhost"):
        raise ProviderError("INVALID_REQUEST", "An owned GDB server needs a loopback endpoint", exit_code=2)
    with socket.socket() as listener:
        try:
            listener.bind(("127.0.0.1", endpoint["port"] if endpoint else 0))
        except OSError as error:
            raise ProviderError("ENDPOINT_IN_USE", f"GDB endpoint {endpoint} is unavailable: {error}") from error
        return endpoint or {"host": host, "port": listener.getsockname()[1]}


def _symbols(request, artifact):
    source = request["input"]
    if source["kind"] == "build":
        return source["elf_path"], source["debug_file_path"]
    path, manifest = artifact or validate_artifact(request)
    partition_name = request["options"]["partition"]
    partition = next((part for part in manifest["partitions"] if part["name"] == partition_name), None)
    if not partition or not partition["symbols"]:
        raise ProviderError("SYMBOLS_UNAVAILABLE", f"No ELF for partition {partition_name}")
    symbols = partition["symbols"]
    debug = symbols["debug_file"]
    return str(manifest_file(path, symbols["elf"])), str(manifest_file(path, debug)) if debug else None


def serve(request, path, profile, *, start_server, ready, connected,
          disconnected=None, artifact=None, start_timeout=30, connect_timeout=45, association_timeout=60):
    """Keep the ready descriptor bound to the specific child owned here."""
    if path is None:
        raise ProviderError("INVALID_REQUEST", "Debug start requires a session path", exit_code=2)
    options = request["options"]
    endpoint = _endpoint(request)
    elf, symbols = _symbols(request, artifact)
    identity = {"session_id": str(uuid4()), "request_id": request["request_id"]}
    owned = options["server"] == "owned"
    stop = _related(path, ".stop")
    closed = _related(path, ".closed")
    log_path = _related(path, ".log")
    project = {"project_root": request["project_root"], "sdk_root": request["sdk_root"]}
    process = None
    try:
        if owned:
            with log_path.open("w", encoding="utf-8") as log:
                process = start_server(endpoint, log)
            deadline = time.monotonic() + start_timeout
            while not ready(log_path.read_text(encoding="utf-8", errors="replace"), endpoint):
                exit_value = process.poll()
                if exit_value is not None:
                    if exit_value:
                        raise tool_failed(request["provider"], exit_value)
                    raise ProviderError("SESSION_NOT_READY", f"GDB server closed before ready. See {log_path}")
                if time.monotonic() >= deadline:
                    raise ProviderError("TIMEOUT", f"GDB server did not start. See {log_path}")
                time.sleep(0.05)
            if process.poll() is not None:
                raise ProviderError("SESSION_NOT_READY", f"GDB server closed before ready. See {log_path}")
        descriptor = {
            "schema_version": 1, "state": "ready", **identity,
            "provider": request["provider"], "board_id": request["board"]["id"],
            "core_id": request["board"]["core_id"], "target": request["target"],
            "build_type": request["build_type"], "profile": options["profile"],
            "mode": options["mode"], "server": options["server"],
            "after_connect": profile["after_connect"], "endpoint": endpoint,
            "gdb_path": options["gdb_path"], "elf_path": elf, "debug_file_path": symbols,
            "artifact_id": request["input"].get("artifact_id"), "partition": options["partition"],
        }
        write_json(_related(path, ".owner"), {**identity, **project})
        write_json(path, descriptor)
        if not owned:
            # The association owns no process. Bound its lifetime in case the
            # IDE fails before it can send a stop request.
            deadline = time.monotonic() + association_timeout
            while time.monotonic() < deadline and not _matches(stop, identity):
                time.sleep(0.1)
            return {"session_id": identity["session_id"]}
        deadline = time.monotonic() + connect_timeout
        client_seen = False
        while not _matches(stop, identity):
            log = log_path.read_text(encoding="utf-8", errors="replace")
            client_seen = client_seen or connected(log)
            exit_value = process.poll()
            if exit_value is not None:
                if exit_value:
                    raise tool_failed(request["provider"], exit_value)
                if client_seen:
                    break
                raise ProviderError("SESSION_NOT_READY", f"GDB server closed without a client. See {log_path}")
            if client_seen and disconnected and disconnected(log):
                break
            if not client_seen and time.monotonic() >= deadline:
                raise ProviderError("TIMEOUT", f"No GDB client connected. See {log_path}")
            time.sleep(0.1)
        return {"session_id": identity["session_id"]}
    finally:
        if process:
            stop_child(process)
            if process.stdin:
                process.stdin.close()
        if path.is_file():
            write_json(closed, identity)


def stop(request):
    source = request["input"]
    path = Path(source["session_path"])
    if not path.is_file():
        raise ProviderError("SESSION_NOT_READY", f"No ready descriptor at {path}")
    session = read_json(path)
    identity = {"session_id": source["session_id"], "request_id": session.get("request_id")}
    owner = {**identity, "project_root": request["project_root"], "sdk_root": request["sdk_root"]}
    if (session.get("session_id") != source["session_id"]
            or session.get("provider") != request["provider"]
            or session.get("board_id") != request["board"]["id"]
            or session.get("core_id") != request["board"]["core_id"]
            or session.get("target") != request["target"]
            or session.get("build_type") != request["build_type"]
            or not _matches(_related(path, ".owner"), owner)):
        raise ProviderError("SESSION_MISMATCH", f"Session {source['session_id']} does not belong to this request")
    closed = _related(path, ".closed")
    if _matches(closed, identity):
        return {"session_id": source["session_id"], "stopped": False}
    write_json(_related(path, ".stop"), identity)
    deadline = time.monotonic() + 8
    while not _matches(closed, identity):
        if time.monotonic() >= deadline:
            raise ProviderError("TIMEOUT", f"Session {source['session_id']} did not stop")
        time.sleep(0.05)
    return {"session_id": source["session_id"], "stopped": session["server"] == "owned"}
