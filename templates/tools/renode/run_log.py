"""Follow or replay the RTT log of a Renode run."""

import codecs
import sys
from time import sleep

from action_protocol import ActionError, read_json
from project_actions import file_lock
from provider_runtime import ProviderError


RUN_LOCK = "run.lock"


def _running(record_path):
    lock = record_path.with_name(RUN_LOCK)
    if not lock.is_file():
        return False
    try:
        with file_lock(lock, blocking=False):
            return False
    except ActionError as error:
        if error.code != "SESSION_BUSY":
            raise
        return True


def _select_record(log_dir, artifact_id, target, build_type):
    records = []
    for path in log_dir.glob("*/record.json"):
        try:
            record = read_json(path)
        except (OSError, ValueError):
            continue
        if (not isinstance(record, dict) or record.get("provider") != "renode"
                or record.get("artifact_id") != artifact_id or record.get("target") != target
                or record.get("build_type") != build_type):
            continue
        running = record.get("status") == "running"
        active = running and _running(path)
        if running and not active:
            # The writer may have finalized while we checked its lock.
            try:
                record = read_json(path)
            except (OSError, ValueError):
                continue
            if not isinstance(record, dict):
                continue
            running = record.get("status") == "running"
        timestamp = record.get("started_at" if running else "completed_at")
        if isinstance(timestamp, str):
            # A stale running record is still readable, behind active and finished runs.
            records.append(((active, not running, timestamp, path), record))
    if not records:
        raise ProviderError("NO_LOG_RECORD",
                            f'No log for {artifact_id}. Run ./build.sh flash-run --via renode first.')
    (active, _, _, path), record = max(records, key=lambda item: item[0])
    return path, record, active


def _display_status(record):
    status = record.get("status")
    if status == "running":
        print("Last run interrupted before finalizing its log.", file=sys.stderr)
    elif status != "ok":
        print(f"Last run {status}: {record.get('message', '')}", file=sys.stderr)


def _stream_log(path, active):
    decoder = codecs.getincrementaldecoder("utf-8")(errors="replace")
    with path.with_name("rtt.log").open("rb") as stream:
        while True:
            data = stream.read(64 * 1024)
            if data:
                print(decoder.decode(data), end="", flush=True)
            elif not active:
                break
            else:
                active = _running(path)
                if active:
                    sleep(0.05)
                # Read again after the writer unlocks to capture its final bytes.
    print(decoder.decode(b"", final=True), end="", flush=True)


def show_log(log_dir, artifact_id, target, build_type):
    path, record, active = _select_record(log_dir, artifact_id, target, build_type)
    if not active:
        _display_status(record)
    _stream_log(path, active)
    if active:
        _display_status(read_json(path))
    return {"source": "recorded", "artifact_id": artifact_id, "record_path": str(path)}
