"""Host-side utilities for action handlers installed by a template pack."""

import argparse
import os
from pathlib import Path
import signal
import subprocess
import sys

from action_protocol import load_request, result, write_json
from firmware_payload import ArtifactError, load_artifact


class ProviderError(ValueError):
    def __init__(self, code, message, *, data=None, exit_code=1):
        super().__init__(message)
        self.code = code
        self.data = data or {}
        self.exit_code = exit_code


def validate_artifact(request):
    source = request["input"]
    path = Path(source["manifest_path"])
    board = request["board"]
    manifest = load_artifact(
        path,
        board_id=board["id"],
        chip=board["chip"],
        core_id=board["core_id"],
        artifact_id=source["artifact_id"],
        target=request["target"],
        build_type=request["build_type"],
    )
    return path, manifest


def tool_failed(name, code, *, data=None):
    exit_code = code if code > 0 else 128 - code
    code_data = {"tool_exit_code": code, **(data or {})}
    return ProviderError(
        "TOOL_FAILED", f"{name} exited with status {code}",
        data=code_data, exit_code=exit_code,
    )


def require_tool(path, name):
    if not path:
        raise ProviderError("TOOL_MISSING", f"{name} is unavailable")
    return path


def executable(path):
    if not path:
        return None
    path = Path(path).expanduser()
    return path.resolve() if path.is_file() and os.access(path, os.X_OK) else None


def stop_child(process):
    if process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=3)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait()


def run_child(command, *, timeout=None, **kwargs):
    with subprocess.Popen(command, **kwargs) as process:
        try:
            return process.wait(timeout=timeout)
        finally:
            stop_child(process)


def provider_main(handler):
    parser = argparse.ArgumentParser()
    parser.add_argument("--request", type=Path, required=True)
    parser.add_argument("--result", type=Path, required=True)
    parser.add_argument("--session", type=Path)
    args = parser.parse_args()
    request = load_request(args.request)

    def cancelled(_signum, _frame):
        raise KeyboardInterrupt()

    stop_signals = [signal.SIGTERM]
    if hasattr(signal, "SIGBREAK"):
        stop_signals.append(signal.SIGBREAK)
    old_handlers = {signum: signal.signal(signum, cancelled) for signum in stop_signals}
    try:
        data = handler(request, args.session)
        response, status = result(request, data=data), 0
    except KeyboardInterrupt:
        response = result(request, code="CANCELLED", message="Action cancelled")
        status = 130
    except ProviderError as error:
        response = result(request, code=error.code, message=str(error), data=error.data)
        status = error.exit_code
    except ArtifactError as error:
        response = result(request, code=error.code, message=str(error), data=error.details)
        status = 1
    except (OSError, subprocess.SubprocessError, ValueError) as error:
        response = result(request, code="EXECUTION_ERROR", message=str(error))
        status = 1
    finally:
        for signum, handler in old_handlers.items():
            signal.signal(signum, handler)
    if status:
        print(response["message"], file=sys.stderr)
    write_json(args.result, response)
    return status
