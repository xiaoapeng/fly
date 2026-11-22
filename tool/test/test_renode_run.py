"""Regressions for streaming and recording Renode output across processes."""

import hashlib
import os
import signal
import subprocess
import sys
import tempfile
import time
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'python'))

from action_protocol import read_json, write_json


SDK = Path(__file__).resolve().parents[2]
ARTIFACT_ID = 'a' * 64

FAKE_RENODE = '''
import base64, json, os, re, sys, time
from pathlib import Path

root, script = map(Path, sys.argv[1:])
commands = script.read_text().splitlines()
_, duration, completion = next(line.split(maxsplit=2) for line in commands if line.startswith('fly_run '))
duration = int(duration)
verify = next(line for line in commands if line.startswith('fly_verify_binary '))
address, digest = re.fullmatch(r'fly_verify_binary (0x[0-9a-f]+) [0-9]+ "([0-9a-f]+)"', verify).groups()
if (root / 'wrong-digest').exists():
    digest = '0' * 64
print(f'FLY_BIN_VERIFIED {int(address, 16):x} {digest}', flush=True)
print('FLY_RENODE_RUN_STARTED', flush=True)
(root / 'renode-pid').write_text(str(os.getpid()))

def emit(text):
    print('FLY_RTT64 ' + base64.b64encode(text).decode('ascii'), flush=True)

emit(b'first RTT line\\n')
while not (root / 'advance').exists():
    time.sleep(0.01)
emit(b'last RTT line\\n')
while not (root / 'finish').exists():
    time.sleep(0.01)
if duration:
    if not (root / 'missing-completion').exists():
        Path(json.loads(completion)).write_text(str(duration), encoding='utf-8')
    suffix = '[19:53:34.4860] [INFO] fly: Machine paused.' if (root / 'joined').exists() else ''
    print('FLY_RENODE_RUN_COMPLETE' + suffix, flush=True)
'''

HARNESS = '''
import json, sys
from pathlib import Path
from unittest.mock import patch

sdk, root = Path(sys.argv[1]), Path(sys.argv[2])
sys.path.insert(0, str(sdk / 'templates/tools/renode'))
from actions import _log_dir, _run
from provider_runtime import ProviderError

artifact = root / 'manifest.json'
manifest = json.loads(artifact.read_text())
request = {'project_root': str(root), 'provider': 'renode', 'target': 'default',
           'build_type': 'Release', 'options': {}}
if sys.argv[3] == 'log':
    from run_log import show_log
    show_log(_log_dir(request, manifest['artifact_id']), manifest['artifact_id'], 'default', 'Release')
else:
    if sys.argv[3] != 'unbounded':
        request['options']['duration_ms'] = int(sys.argv[3])
    with patch('actions.find_renode', return_value=sys.executable), patch('actions._command',
            side_effect=lambda program, script: [program, str(root / 'fake_renode.py'), str(root), str(script)]):
        try:
            print(json.dumps(_run(request, artifact, manifest, root / 'model.repl')), flush=True)
        except ProviderError as error:
            print(error.code, str(error), file=sys.stderr)
            sys.exit(error.exit_code)
        except KeyboardInterrupt:
            sys.exit(130)
'''


class RenodeRunTests(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory(prefix='fly renode run ')
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name).resolve()
        binary = b'firmware'
        (self.root / 'APP.bin').write_bytes(binary)
        (self.root / 'fake_renode.py').write_text(FAKE_RENODE, encoding='utf-8')
        write_json(self.root / 'manifest.json', {
            'artifact_id': ARTIFACT_ID,
            'partitions': [{'name': 'APP', 'address': '0x0', 'symbols': None,
                            'bin': {'path': 'APP.bin', 'size': len(binary),
                                    'sha256': hashlib.sha256(binary).hexdigest()}}],
        })

    def start(self, mode, label):
        output = self.root / f'{label}.txt'
        with output.open('wb') as stream:
            process = subprocess.Popen([sys.executable, '-B', '-c', HARNESS, str(SDK), str(self.root), mode],
                                       env=dict(os.environ, FLY_SDK_DIR=str(SDK)),
                                       stdout=stream, stderr=subprocess.STDOUT)

        def cleanup():
            for gate in ('advance', 'finish'):
                (self.root / gate).touch()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=5)

        self.addCleanup(cleanup)
        return process, output

    def wait_for_rtt(self, process, output, line):
        deadline = time.monotonic() + 10
        while line not in output.read_bytes() and time.monotonic() < deadline and process.poll() is None:
            time.sleep(0.02)
        self.assertIn(line, output.read_bytes())

    def record(self):
        records = list((self.root / 'image' / 'logs').rglob('record.json'))
        self.assertEqual(len(records), 1)
        return records[0], read_json(records[0])

    def test_streams_and_follows_until_timed_run_finishes(self):
        runner, run_output = self.start('175', 'run')
        self.wait_for_rtt(runner, run_output, b'first RTT line')
        self.assertIsNone(runner.poll())
        record_path, record = self.record()
        self.assertEqual(record['status'], 'running')
        self.assertIn('fly_run 175', (record_path.parent / 'run.resc').read_text())

        follower, follow_output = self.start('log', 'follow')
        self.wait_for_rtt(follower, follow_output, b'first RTT line')
        self.assertIsNone(follower.poll())
        (self.root / 'advance').touch()
        self.wait_for_rtt(follower, follow_output, b'last RTT line')
        self.assertIsNone(runner.poll())
        (self.root / 'finish').touch()
        self.assertEqual(runner.wait(timeout=10), 0, run_output.read_text())
        self.assertEqual(follower.wait(timeout=10), 0, follow_output.read_text())
        record = read_json(record_path)
        self.assertEqual(record['status'], 'ok')
        self.assertEqual(record['virtual_ms'], 175)
        saved = (record_path.parent / 'rtt.log').read_bytes()
        self.assertEqual(saved, b'first RTT line\nlast RTT line\n')
        self.assertEqual(follow_output.read_bytes(), saved)
        replay, replay_output = self.start('log', 'replay')
        self.assertEqual(replay.wait(timeout=10), 0, replay_output.read_text())
        self.assertEqual(replay_output.read_bytes(), saved)

    @unittest.skipIf(os.name == 'nt', 'The process-targeted SIGINT test requires POSIX.')
    def test_unbounded_run_stops_and_saves_log_on_interrupt(self):
        runner, output = self.start('unbounded', 'run')
        self.wait_for_rtt(runner, output, b'first RTT line')
        record_path, record = self.record()
        self.assertEqual(record['status'], 'running')
        self.assertIn('fly_run 0', (record_path.parent / 'run.resc').read_text())
        renode_pid = int((self.root / 'renode-pid').read_text())
        runner.send_signal(signal.SIGINT)
        self.assertEqual(runner.wait(timeout=10), 130, output.read_text())
        self.assertEqual(read_json(record_path)['status'], 'cancelled')
        self.assertEqual((record_path.parent / 'rtt.log').read_bytes(), b'first RTT line\n')
        with self.assertRaises(ProcessLookupError):
            os.kill(renode_pid, 0)

    def test_rejects_a_completed_run_with_wrong_flash_digest(self):
        for flag in ('wrong-digest', 'advance', 'finish'):
            (self.root / flag).touch()
        runner, output = self.start('175', 'run')
        self.assertEqual(runner.wait(timeout=10), 1, output.read_text())
        self.assertIn('ARTIFACT_MISMATCH', output.read_text())
        _, record = self.record()
        self.assertEqual(record['status'], 'error')

    def test_completion_with_interleaved_monitor_log(self):
        for flag in ('joined', 'advance', 'finish'):
            (self.root / flag).touch()
        runner, output = self.start('175', 'run')
        self.assertEqual(runner.wait(timeout=10), 0, output.read_text())
        _, record = self.record()
        self.assertEqual(record['status'], 'ok')

    def test_clean_exit_without_completion_is_rejected(self):
        for flag in ('missing-completion', 'advance', 'finish'):
            (self.root / flag).touch()
        runner, output = self.start('175', 'run')
        self.assertEqual(runner.wait(timeout=10), 1, output.read_text())
        self.assertIn('EXECUTION_ERROR', output.read_text())
