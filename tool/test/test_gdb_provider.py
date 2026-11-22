"""Regressions for debug associations with an externally owned GDB server."""

from pathlib import Path
import socket
import sys
from tempfile import TemporaryDirectory
from unittest import TestCase
from uuid import uuid4

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "python"))

from action_protocol import read_json
from gdb_provider import serve, stop


class ExternalAssociationTests(TestCase):
    def test_missing_stop_task_does_not_leave_an_open_association(self):
        with TemporaryDirectory() as work, socket.socket() as listener:
            listener.bind(("127.0.0.1", 0))
            listener.listen()
            host, port = listener.getsockname()
            path = Path(work) / "session.json"
            request = {
                "request_id": str(uuid4()),
                "project_root": work, "sdk_root": work,
                "provider": "test", "board": {"id": "board/test", "core_id": "core0"},
                "target": "default", "build_type": "Debug",
                "input": {"kind": "build", "elf_path": str(Path(work) / "app.elf"), "debug_file_path": None},
                "options": {
                    "profile": "external", "mode": "attach", "server": "external", "partition": None,
                    "endpoint": {"host": host, "port": port}, "gdb_path": sys.executable,
                },
            }

            def unexpected_callback(*_args):
                self.fail("An external session must not start or inspect an owned server")

            result = serve(
                request, path, {"after_connect": []}, start_server=unexpected_callback,
                ready=unexpected_callback, connected=unexpected_callback, association_timeout=0,
            )
            self.assertEqual(result["session_id"], read_json(path)["session_id"])
            stop_request = dict(request, input={
                "kind": "session", "session_path": str(path), "session_id": result["session_id"],
            })
            self.assertFalse(stop(stop_request)["stopped"])
            with socket.create_connection((host, port), timeout=1):
                pass
