"""Contract regressions for template pack actions, independent of device tools."""

import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'python'))

from action_protocol import ActionError, load_request, read_json, result, validate_result, validate_session, write_json
from project_actions import Dispatcher, binding_key, choose_profile, verify_build_symbols, write_build_symbols
from vscode_project import write_owned_entries


class ActionContracts(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory(prefix='fly action contracts ')
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name).resolve()
        self.make_pack('board/dummy', board={
            'target_kind': 'mcu', 'chip': 'fixture', 'core': {'id': 'core0', 'architecture': 'arm-cortex-m3'},
            'arch': 'dummy', 'toolchain': 'arm-none-eabi-gcc',
            'clock_hz': 1000, 'memories': [{'id': 'app', 'kind': 'flash', 'address': 0, 'size': 4096}],
        })
        self.make_pack('app/dummy', app={'config_defaults': [], 'expected_log': [], 'firmware_target': 'fly_app'})
        def handler(name, provider, inputs, profiles=None):
            script = f'tool/packs/tool/{provider}/actions.py'
            action = {'name': name, 'provider': provider, 'handler': script, 'inputs': inputs,
                      'help': f'Fixture {name}', 'tool_check': script}
            if profiles:
                action['debug_profiles'] = profiles
            return action

        base = {'input': 'artifact', 'mode': 'attach', 'after_connect': []}
        self.flash_run = handler('flash-run', 'dummy', ['artifact'])
        self.simulation = handler('flash-run', 'simulator', ['artifact'])
        self.debug = handler('debug', 'dummy', ['artifact'], [
            dict(base, id='owned', server='owned'),
            dict(base, id='outside', server='external'),
            dict(base, id='program', server='owned', pre_action={'action': 'flash-run', 'provider': 'dummy'}),
        ])
        write_json(self.root / 'fly.project.json', {
            'schema_version': 1, 'packs': ['board/dummy', 'app/dummy', 'tool/dummy', 'tool/simulator'],
            'defaults': {},
        })
        script = (
            'import os, sys\n'
            'from pathlib import Path\n'
            'sys.path.insert(0, str(Path(os.environ["FLY_SDK_DIR"]) / "tool/python"))\n'
            'from action_protocol import load_request, result, write_json\n'
            'request = load_request(sys.argv[2])\n'
            'assert request["operation"] == "check" and request["build_dir"] is None\n'
            'missing = [{"id":"local-server","message":"No local server"}] '
            'if request["options"].get("profile") == "owned" else []\n'
            'write_json(sys.argv[4], result(request, data={"available": not missing, "missing": missing}))\n'
        )
        for actions in ([self.flash_run, self.debug], [self.simulation]):
            action = actions[0]
            destination = action['handler']
            self.make_pack(f'tool/{action["provider"]}', actions=actions,
                           files=[{'source': 'actions.py', 'destination': destination}])
            (self.root / destination).write_text(script, encoding='utf-8')
        self.dispatch = Dispatcher(self.root, Path(__file__).resolve().parents[2], build_type='Release')
        self.input = {'kind': 'artifact', 'manifest_path': str(self.root / 'manifest.json'), 'artifact_id': 'a' * 64}

    def make_pack(self, pack_id, **data):
        kind, name = pack_id.split('/')
        write_json(self.root / 'tool/packs' / kind / name / 'pack.json', {
            'schema_version': 1, 'id': pack_id, 'kind': kind,
            'label': name, 'files': [], **data,
        })

    def test_tool_check_distinguishes_owned_and_external_without_build(self):
        debug = self.dispatch.select('debug', 'dummy')
        flash_run = self.dispatch.select('flash-run', 'dummy')
        owned, outside, _ = self.debug['debug_profiles']
        self.assertFalse(self.dispatch.check(debug, input_kind='artifact', profile=owned)['available'])
        self.assertTrue(self.dispatch.check(debug, input_kind='artifact', profile=outside)['available'])
        self.assertTrue(self.dispatch.check(flash_run, input_kind='artifact')['available'])
        self.assertFalse((self.root / 'build').exists())

    def test_flash_profile_requires_explicit_choice(self):
        self.assertEqual(choose_profile(self.debug, input_kind='artifact')['id'], 'owned')
        self.assertEqual(choose_profile(self.debug, profile_id='program')['id'], 'program')
        with self.assertRaises(ActionError):
            choose_profile({'debug_profiles': [self.debug['debug_profiles'][-1]]}, input_kind='artifact')

    def test_failed_pre_action_keeps_exit_code_and_never_launches_gdb(self):
        profile = choose_profile(self.debug, profile_id='program')
        request = self.dispatch.request(self.debug, 'execute', input_data=self.input, options={
            'profile': 'program', 'mode': 'attach', 'server': 'owned', 'client': 'cli',
            'gdb_path': str(self.root / 'gdb'), 'partition': 'APP', 'endpoint': None,
        })
        self.dispatch.flash_run = Mock(side_effect=ActionError('TOOL_FAILED', 'probe failed', exit_code=7))
        with self.assertRaises(ActionError) as error:
            self.dispatch.execute_debug(self.debug, request, profile, self.root / 'app.elf', None, artifact={})
        self.assertEqual(error.exception.exit_code, 7)
        self.dispatch.flash_run.assert_called_once()
        pre_request = self.dispatch.flash_run.call_args[0][1]
        self.assertEqual(pre_request['action'], 'flash-run')
        self.assertEqual(pre_request['provider'], 'dummy')
        self.assertEqual(pre_request['input'], request['input'])
        self.assertNotEqual(pre_request['request_id'], request['request_id'])
        prerequisite = error.exception.data['prerequisite']
        self.assertEqual(prerequisite['action'], 'flash-run')
        self.assertEqual(prerequisite['artifact_id'], self.input['artifact_id'])

    def test_flash_run_lists_both_and_requires_provider_when_ambiguous(self):
        output = StringIO()
        with redirect_stdout(output):
            self.dispatch.list('flash-run', 'default')
        for provider in ('dummy', 'simulator'):
            self.assertIn(f'flash-run default --via {provider}', output.getvalue())
        self.assertNotIn('(default)', output.getvalue())

        with self.assertRaises(ActionError) as error:
            self.dispatch.select('flash-run')
        self.assertEqual(error.exception.code, 'AMBIGUOUS_PROVIDER')
        self.assertEqual(error.exception.exit_code, 2)
        self.assertEqual(self.dispatch.select('flash-run', 'simulator'), self.simulation)

        project_path = self.root / 'fly.project.json'
        write_json(project_path, dict(read_json(project_path), defaults={'flash-run': 'dummy'}))
        dispatch = Dispatcher(self.root, self.dispatch.sdk, build_type='Release')
        self.assertEqual(dispatch.select('flash-run'), self.flash_run)
        self.assertEqual(dispatch.select('flash-run', 'simulator'), self.simulation)

    def test_flash_run_validates_each_providers_artifact_and_runtime(self):
        partitions = [{'name': 'APP', 'address': '0x0', 'sha256': 'b' * 64}]
        artifact = {'partitions': [{'name': 'APP', 'address': '0x0', 'bin': {'sha256': 'b' * 64}}]}
        for action, record_path, virtual_ms in (
            (self.flash_run, None, None),
            (self.simulation, str(self.root / 'simulation.json'), 100),
        ):
            with self.subTest(provider=action['provider']):
                request = self.dispatch.request(action, 'execute', input_data=self.input)
                data = {'artifact_id': self.input['artifact_id'], 'partitions': partitions,
                        'record_path': record_path, 'virtual_ms': virtual_ms}
                response = result(request, data=data)
                self.assertEqual(validate_result(response, request, 0), response)
                with patch.object(self.dispatch, 'invoke', return_value=data):
                    self.assertEqual(self.dispatch.flash_run(action, request, artifact), data)
                for changed in ({'address': '0x4'}, {'sha256': 'c' * 64}):
                    mismatched = dict(data, partitions=[dict(partitions[0], **changed)])
                    with patch.object(self.dispatch, 'invoke', return_value=mismatched):
                        with self.assertRaises(ActionError) as error:
                            self.dispatch.flash_run(action, request, artifact)
                    self.assertEqual(error.exception.code, 'ARTIFACT_MISMATCH')

                for invalid in ({'artifact_id': 'd' * 64}, {'partitions': []},
                                {'record_path': None, 'virtual_ms': 100},
                                {'record_path': str(self.root / 'missing-time.json'), 'virtual_ms': None}):
                    with self.assertRaises(ValueError):
                        validate_result(result(request, data=dict(data, **invalid)), request, 0)

    def test_rejects_unknown_request_field_and_result_mismatch(self):
        request = self.dispatch.request(self.flash_run, 'execute', input_data=self.input)
        path = self.root / 'request.json'
        write_json(path, dict(request, surprise=True))
        with self.assertRaises(ValueError):
            load_request(path)
        failure = result(request, data={'tool_exit_code': 7}, code='TOOL_FAILED', message='probe failed')
        self.assertEqual(validate_result(failure, request, 7), failure)
        with self.assertRaises(ValueError):
            validate_result(failure, request, 0)
        with self.assertRaises(ValueError):
            validate_result(failure, request, 8)

    def test_rejects_wrong_ready_session(self):
        profile = choose_profile(self.debug, input_kind='artifact')
        options = {'profile': 'owned', 'mode': 'attach', 'server': 'owned', 'client': 'ide',
                   'gdb_path': str(self.root / 'gdb'), 'partition': 'APP',
                   'endpoint': {'host': '127.0.0.1', 'port': 3333}}
        request = self.dispatch.request(self.debug, 'execute', input_data=self.input, options=options)
        elf = self.root / 'app.elf'
        session = {
            'schema_version': 1, 'state': 'ready', 'session_id': '5620834b-51d6-4afe-8de7-ded0df1cc143',
            'request_id': request['request_id'], 'provider': request['provider'],
            'board_id': request['board']['id'], 'core_id': request['board']['core_id'],
            'target': request['target'], 'build_type': request['build_type'],
            'profile': options['profile'], 'mode': options['mode'], 'server': options['server'],
            'endpoint': options['endpoint'], 'artifact_id': self.input['artifact_id'],
            'gdb_path': options['gdb_path'], 'elf_path': str(elf), 'debug_file_path': None,
            'partition': options['partition'], 'after_connect': [],
        }
        self.assertEqual(validate_session(session, request, profile, elf, None), session)
        with self.assertRaises(ValueError):
            validate_session(dict(session, after_connect=['continue']), request, profile, elf, None)
        with self.assertRaises(ValueError):
            validate_session(dict(session, endpoint={'host': '127.0.0.1', 'port': 3334}),
                             request, profile, elf, None)

    def test_vscode_keeps_custom_item_with_the_same_name(self):
        path, owners = self.root / 'launch.json', self.root / 'owned.json'
        user = {'name': 'my profile', 'request': 'launch'}
        write_json(path, {'configurations': [user]})
        options = ('0.2.0', 'configurations', 'name')
        write_owned_entries(path, *options, [
            {'name': 'my profile', 'request': 'attach'}, {'name': 'generated', 'request': 'attach'},
        ], owners, 'debug-launch')
        write_owned_entries(path, *options, [], owners, 'debug-launch')
        self.assertEqual(read_json(path)['configurations'], [user])

    def test_archive_rebinding_and_endpoint_change_get_distinct_task_keys(self):
        initial = {'provider': 'dummy', 'options': {'profile': 'owned', 'endpoint': 25000},
                   'input': self.input}
        modified = dict(initial, input=dict(self.input, artifact_id='b' * 64))
        other_endpoint = dict(initial, options=dict(initial['options'], endpoint=25001))
        self.assertEqual(len({binding_key(item) for item in (initial, modified, other_endpoint)}), 3)

    def test_configure_cannot_relabel_a_previous_builds_symbols(self):
        elf, debug = self.root / 'app.elf', self.root / 'app.debug'
        elf.write_bytes(b'ELF')
        debug.write_bytes(b'DWARF')
        write_build_symbols(self.root, 'Release', elf, debug)
        verify_build_symbols(self.root, 'Release', elf, debug)
        with self.assertRaisesRegex(ActionError, 'different configuration'):
            verify_build_symbols(self.root, 'Debug', elf, debug)
        elf.write_bytes(b'a different ELF')
        with self.assertRaises(ActionError):
            verify_build_symbols(self.root, 'Release', elf, debug)


if __name__ == '__main__':
    unittest.main()
