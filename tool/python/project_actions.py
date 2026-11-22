"""Discover actions and dispatch against the application's selected package manifests."""

import argparse
import json
import os
import re
import shutil
import signal
import socket
import subprocess
import sys
from collections import Counter
from contextlib import contextmanager
from hashlib import sha256
from pathlib import Path
from tempfile import TemporaryDirectory
from time import monotonic, sleep
from uuid import uuid4

from action_protocol import (ActionError, endpoint, read_json, validate_request,
                             validate_result, validate_session, write_json)
from file_lock import LockBusy, file_lock as lock_file
from template_packs import load_project


def project_fingerprint(project, *, generated=True):
    digest = sha256()
    paths = [project / 'fly.project.json', project / 'CMakeLists.txt', project / 'app/CMakeLists.txt',
             project / 'board/board.cmake', project / 'Kconfig', project / 'defconfig']
    if generated:
        paths.append(project / 'build/config/.config.cmake')
    paths.extend(sorted((project / 'tool/packs').rglob('*.json')))
    paths.extend(sorted((project / 'tool/packs').rglob('*.cmake')))
    for path in paths:
        if path.is_file():
            digest.update(path.relative_to(project).as_posix().encode())
            digest.update(sha256(path.read_bytes()).digest())
    return digest.hexdigest()


def binding_key(binding):
    identity = json.dumps(binding, sort_keys=True, separators=(',', ':'), ensure_ascii=False).encode()
    profile = binding['options']['profile']
    return f'{binding["provider"]}-{profile}-{sha256(identity).hexdigest()[:20]}'


def _cmake_value(value):
    equals = ''
    while f']{equals}]' in str(value):
        equals += '='
    return f'[{equals}[{value}]{equals}]'


def write_cmake(project, output):
    composition = load_project(project)
    board, app = composition.board, composition.app
    board_data = board.data['board']
    values = {
        'FLY_PACK_TARGET': app.data['app']['firmware_target'],
        'FLY_BOARD_ID': board.id, 'FLY_BOARD_CHIP': board_data['chip'] or '',
        'FLY_BOARD_CORE_ID': board_data['core']['id'], 'FLY_BOARD_MANIFEST': str(board.path),
        'FLY_BOARD_CLOCK_HZ': board_data['clock_hz'] or '',
        'FLY_PACK_BOARD_CMAKE': board.data.get('cmake', ''),
        'FLY_PACK_APP_CMAKE': app.data.get('cmake', ''),
    }
    lines = [f'set({key} {_cmake_value(value)})' for key, value in values.items()]
    for name, kind in [('FLY_PACK_SUPPORT_CMAKE', 'support'), ('FLY_PACK_TOOLS_CMAKE', 'tool')]:
        entries = [pack.data['cmake'] for pack in composition.packs
                   if pack.kind == kind and pack.data.get('cmake')]
        lines.append(f'set({name} {" ".join(map(_cmake_value, entries))})')
    action_targets = [f'{action}:{provider}' for action, provider in composition.actions if action != 'export']
    lines.append(f'set(FLY_PACK_ACTION_TARGETS {" ".join(map(_cmake_value, action_targets))})')
    for pack in composition.packs:
        sink = pack.data.get('log_sink')
        if sink:
            lines += [f'if({sink["enabled_by"]})',
                      f'list(APPEND FLY_PACK_LOG_SINKS {_cmake_value(sink["target"])})',
                      'endif()']
        lines.append(f'set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS '
                     f'{_cmake_value(pack.path)})')
    content = '\n'.join(lines) + '\n'
    output.parent.mkdir(parents=True, exist_ok=True)
    if not output.exists() or output.read_text(encoding='utf-8') != content:
        output.write_text(content, encoding='utf-8')


def write_index(project, build_dir, build_type, elf):
    composition = load_project(project)
    output = project / 'build/config/actions.json'
    write_json(output, {
        'schema_version': 1, 'build_type': build_type,
        'build_dir': str(build_dir), 'elf_path': str(elf),
        'fingerprint': project_fingerprint(project),
        'actions': list(composition.actions.values()),
        'defaults': composition.defaults,
    })


def _symbol_files(elf, debug_file):
    files = [elf] + ([debug_file] if debug_file is not None else [])
    return {path.name: sha256(path.read_bytes()).hexdigest() for path in files}


def write_build_symbols(project, build_type, elf, debug_file):
    write_json(elf.parent / 'fly-build-symbols.json', {
        'build_type': build_type, 'files': _symbol_files(elf, debug_file),
        'fingerprint': project_fingerprint(project),
    })


def verify_build_symbols(project, build_type, elf, debug_file):
    try:
        data = read_json(elf.parent / 'fly-build-symbols.json')
        if data != {'build_type': build_type, 'files': _symbol_files(elf, debug_file),
                    'fingerprint': project_fingerprint(project)}:
            raise ValueError('The available ELF is from a different configuration')
    except (OSError, ValueError) as error:
        raise ActionError('SYMBOLS_UNAVAILABLE', f'{error} — run ./build.sh build {build_type} first.') from error


@contextmanager
def file_lock(path, *, blocking=True):
    try:
        with lock_file(path, blocking=blocking):
            yield
    except LockBusy:
        raise ActionError('SESSION_BUSY', 'Debug configuration is already active') from None


def build_tree_lock(project):
    return file_lock(project / '.fly-build.lock')


def _endpoint_text(value):
    host = value['host']
    return f'[{host}]:{value["port"]}' if ':' in host else f'{host}:{value["port"]}'


def parse_endpoint(value):
    host, separator, port = value.rpartition(':')
    if not separator or not port.isdecimal():
        raise argparse.ArgumentTypeError('Use HOST:PORT for the GDB endpoint')
    try:
        return endpoint({'host': host[1:-1] if host.startswith('[') and host.endswith(']') else host,
                         'port': int(port)})
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error)) from error


def _relative_file(project, value):
    path = (project / value).resolve()
    try:
        path.relative_to(project)
    except ValueError as error:
        raise ActionError('INVALID_REQUEST', f'Path escapes the application: {value}', exit_code=2) from error
    if not path.is_file():
        raise ActionError('INVALID_REQUEST', f'Package file missing: {path}', exit_code=2)
    return path


def choose_profile(action, *, input_kind=None, profile_id=None, mode=None, server=None, address=None):
    candidates = [
        item for item in action['debug_profiles']
        if (input_kind is None or item['input'] == input_kind)
        and (mode is None or item['mode'] == mode)
        and (server is None or item['server'] == server)
        and (item['id'] == profile_id if profile_id else 'pre_action' not in item)
        and (item['server'] != 'external' or address is not None)
    ]
    if len(candidates) != 1:
        names = ', '.join(item['id'] for item in action['debug_profiles'])
        raise ActionError('INVALID_REQUEST', f'Choose a compatible --profile from: {names}', exit_code=2)
    return candidates[0]


def action_command(name):
    return 'make_img' if name == 'export' else name


class Dispatcher:
    def __init__(self, project, sdk, *, build_dir=None, build_type=None, tools=()):
        self.project = Path(project).resolve()
        self.sdk = Path(sdk).resolve()
        self.composition = load_project(self.project)
        selected = self.composition.board
        self.board = {'id': selected.id, 'chip': selected.data['board']['chip'],
                      'core_id': selected.data['board']['core']['id'],
                      'manifest_path': str(selected.path)}
        self.build_dir = Path(build_dir).resolve() if build_dir is not None else None
        self.build_type = build_type
        self.tools = dict(tools)

    def select(self, name, provider=None):
        actions = self.composition.actions
        candidates = [action for key, action in actions.items() if key[0] == name]
        selected = provider or self.composition.defaults.get(name)
        if not selected and len(candidates) == 1:
            return candidates[0]
        if selected and (name, selected) in actions:
            return actions[name, selected]
        command = action_command(name)
        examples = ', '.join(f'./build.sh {command} --via {item["provider"]}' for item in candidates)
        if candidates and not selected:
            raise ActionError('AMBIGUOUS_PROVIDER', f'Choose a provider: {examples}', exit_code=2)
        raise ActionError('PROVIDER_NOT_FOUND', f'No {command} provider '
                          f'{selected or "enabled"}. Available: {examples or "none"}', exit_code=2)

    def request(self, action, operation, *, target='default', input_data=None, options=None,
                build_type=None):
        return validate_request({
            'schema_version': 1, 'request_id': str(uuid4()), 'operation': operation,
            'action': action['name'], 'provider': action['provider'], 'project_root': str(self.project),
            'sdk_root': str(self.sdk), 'board': self.board, 'target': target,
            'build_type': build_type or self.build_type,
            'build_dir': str(self.build_dir) if self.build_dir and self.build_dir.is_dir() else None,
            'input': input_data or {'kind': 'none'}, 'tool_overrides': self.tools,
            'options': options or {},
        })

    def _command(self, script, request, directory, session=False):
        request_path, result_path = directory / 'request.json', directory / 'result.json'
        write_json(request_path, request)
        command = [sys.executable, '-B', str(_relative_file(self.project, script)),
                   '--request', str(request_path), '--result', str(result_path)]
        if session:
            command.extend(('--session', str(directory / 'session.json')))
        return command, result_path

    def _env(self):
        return dict(os.environ, FLY_SDK_DIR=str(self.sdk))

    def _result(self, request, path, returncode):
        try:
            value = validate_result(read_json(path), request, returncode)
        except (OSError, ValueError) as error:
            raise ActionError('PROTOCOL_ERROR', f'{request["provider"]} returned {returncode}: {error}') from error
        if value['status'] != 'ok':
            code = (128 + abs(returncode)) if returncode < 0 else returncode
            raise ActionError(value['code'], value['message'], exit_code=code, data=value['data'])
        return value['data']

    def invoke(self, action, request, *, capture=False):
        key = 'tool_check' if request['operation'] == 'check' else 'handler'
        with TemporaryDirectory(prefix='fly-action-') as scratch:
            command, result_path = self._command(action[key], request, Path(scratch))
            process = subprocess.run(command, cwd=self.project, env=self._env(), check=False,
                                     capture_output=capture, text=capture)
            try:
                return self._result(request, result_path, process.returncode)
            except ActionError as error:
                if capture and (process.stderr or process.stdout):
                    print((process.stderr or process.stdout).strip(), file=sys.stderr)
                raise error

    def flash_run(self, action, request, artifact):
        expected = [{'name': part['name'], 'address': part['address'], 'sha256': part['bin']['sha256']}
                    for part in artifact['partitions']]
        data = self.invoke(action, request)
        if sorted(data['partitions'], key=lambda part: part['name']) != sorted(expected, key=lambda part: part['name']):
            raise ActionError('ARTIFACT_MISMATCH', 'Provider reported different partition bytes or addresses')
        return data

    def check(self, action, *, target='default', input_kind=None, profile=None):
        if input_kind is not None and input_kind not in action['inputs']:
            raise ActionError('INVALID_REQUEST', f'Unsupported check input: {input_kind}', exit_code=2)
        options = {'input_kind': input_kind} if input_kind is not None else {}
        if profile is not None:
            options['profile'] = profile['id']
        missing = []
        if 'tool_check' in action:
            request = self.request(action, 'check', target=target, options=options)
            missing.extend(self.invoke(action, request, capture=True)['missing'])
        if profile and 'pre_action' in profile:
            prerequisite = profile['pre_action']
            try:
                prerequisite_action = self.select(prerequisite['action'], prerequisite['provider'])
                missing.extend(self.check(prerequisite_action, target=target, input_kind='artifact')['missing'])
            except ActionError as error:
                missing.append({'id': prerequisite['provider'], 'message': str(error)})
        return {'available': not missing, 'missing': missing}

    def require_tools(self, action, target, input_kind, profile=None):
        status = self.check(action, target=target, input_kind=input_kind, profile=profile)
        if not status['available']:
            raise ActionError('TOOL_MISSING', '; '.join(item['message'] for item in status['missing']))

    def _readiness(self, action, target, profile, gdb_hint, address):
        input_kind = profile['input'] if profile else None
        try:
            missing = list(self.check(action, target=target, input_kind=input_kind, profile=profile)['missing'])
        except (OSError, ValueError) as error:
            missing = [{'id': 'tool-check', 'message': str(error)}]
        if profile:
            try:
                self.gdb(gdb_hint)
            except ActionError as error:
                missing.append({'id': 'gdb', 'message': str(error)})
            if profile['server'] == 'external' and address is None:
                missing.append({'id': 'endpoint', 'message': 'Specify --endpoint HOST:PORT for this profile.'})
        return {'available': not missing, 'missing': missing}

    def _action_catalog(self, name, target, gdb_hint, address):
        actions = self.composition.actions
        counts = Counter(action_name for action_name, _ in actions)
        for (action_name, provider), action in actions.items():
            if name is not None and action_name != name:
                continue
            profiles = [
                {**profile, **self._readiness(action, target, profile, gdb_hint, address)}
                for profile in action.get('debug_profiles', [])
            ]
            if profiles:
                available = any(profile['available'] for profile in profiles)
                missing = {(item['id'], item['message']): item for profile in profiles for item in profile['missing']}
                status = {'available': available, 'missing': [] if available else list(missing.values())}
            else:
                status = self._readiness(action, target, None, gdb_hint, address)
            yield {
                'name': action_name, 'provider': provider, 'inputs': action['inputs'], 'help': action['help'],
                'default': counts[action_name] == 1 or self.composition.defaults.get(action_name) == provider,
                'profiles': profiles, **status,
            }

    def capabilities(self, target, *, gdb_hint=None, address=None):
        selected = set(read_json(self.project / 'fly.project.json')['packs'])
        packages = [{
            'id': pack.id, 'kind': pack.kind, 'label': pack.label, 'selected': pack.id in selected,
            **{key: pack.data.get(key, []) for key in ('provides', 'requires', 'dependencies')},
        } for pack in self.composition.packs]
        return {
            'packages': packages,
            'capabilities': sorted({item for pack in packages for item in pack['provides']}),
            'actions': list(self._action_catalog(None, target, gdb_hint, address)),
        }

    def list(self, name, target, *, gdb_hint=None, address=None):
        entries = list(self._action_catalog(name, target, gdb_hint, address))
        if not entries:
            print(f'No provider enabled for {name}. Choose an appropriate template package when generating.')
        self.print_actions(entries, target, address, include_name=False)

    @staticmethod
    def print_actions(entries, target, address, *, include_name=True):
        for action in entries:
            name, provider = action['name'], action['provider']
            label = f'{name}/{provider}' if include_name else provider
            default = ' (default)' if action['default'] else ''
            print(f'{label}{default}  {action["help"]}')
            for profile in action['profiles'] or [None]:
                args = f' --profile {profile["id"]}' if profile else ''
                suffix = f' [{profile["input"]}/{profile["server"]}]' if profile else ''
                if profile and profile['server'] == 'external':
                    args += f' --endpoint {_endpoint_text(address) if address else "HOST:PORT"}'
                status = profile or action
                missing = '; '.join(item['message'] for item in status['missing'])
                print(f'  ./build.sh {action_command(name)} {target} --via {provider}{args}{suffix}'
                      f' — {missing or "tools available"}')

    def artifact(self, path, *, target=None, build_type=None):
        from firmware_payload import load_artifact

        path = Path(path).resolve()
        try:
            return load_artifact(path, board_id=self.board['id'], chip=self.board['chip'],
                                 core_id=self.board['core_id'], target=target, build_type=build_type,
                                 memories=self.composition.board.data['board']['memories'])
        except ValueError as error:
            raise ActionError(getattr(error, 'code', 'ARTIFACT_INVALID'), str(error)) from error

    def gdb(self, cached=None):
        toolchain = self.composition.board.data['board']['toolchain']
        names = [self.tools.get('gdb'), cached]
        if toolchain.endswith('-gcc'):
            names.append(toolchain[:-3] + 'gdb')
        names.extend(['gdb-multiarch', 'gdb'])
        for name in names:
            executable = shutil.which(str(name)) if name else None
            if executable:
                return str(Path(executable).absolute())
        raise ActionError('TOOL_MISSING', f'Cannot find the GDB client for {toolchain}.')

    def debug_paths(self, artifact_path, artifact, partition=None):
        from firmware_payload import manifest_file

        parts = {item['name']: item for item in artifact['partitions']}
        if partition is None:
            partition = 'APP' if 'APP' in parts else next(iter(parts)) if len(parts) == 1 else None
        if partition not in parts:
            raise ActionError('INVALID_REQUEST', f'Choose a partition from: {", ".join(parts)}', exit_code=2)
        symbols = parts[partition]['symbols']
        if not symbols:
            raise ActionError('SYMBOLS_UNAVAILABLE', f'No symbols for {partition} in {artifact_path}')
        return partition, manifest_file(artifact_path, symbols['elf']), (
            manifest_file(artifact_path, symbols['debug_file']) if symbols['debug_file'] else None)

    def execute_debug(self, action, request, profile, elf, debug_file, *, artifact=None, session_dir=None):
        options = request['options']
        pre_action = profile.get('pre_action')
        address = options['endpoint']
        if options['server'] == 'owned' and address is not None:
            if address['host'] not in ('127.0.0.1', 'localhost'):
                raise ActionError('INVALID_REQUEST', 'An owned GDB server needs a loopback endpoint', exit_code=2)
            with socket.socket() as listener:
                try:
                    listener.bind(('127.0.0.1', address['port']))
                except OSError as error:
                    raise ActionError('ENDPOINT_IN_USE', f'{_endpoint_text(address)}: {error}') from error
        if pre_action:
            prerequisite = self.select(pre_action['action'], pre_action['provider'])
            prerequisite_request = self.request(prerequisite, 'execute', target=request['target'],
                                                input_data=request['input'], build_type=request['build_type'])
            try:
                self.flash_run(prerequisite, prerequisite_request, artifact)
            except ActionError as error:
                error.data['prerequisite'] = {
                    'action': prerequisite['name'], 'provider': prerequisite['provider'],
                    'artifact_id': request['input']['artifact_id'], 'request_id': prerequisite_request['request_id'],
                }
                raise

        with TemporaryDirectory(prefix='fly-debug-') as scratch:
            directory = Path(scratch)
            command, result_path = self._command(action['handler'], request, directory, session=True)
            session_path = directory / 'session.json'
            spawn_options = ({'creationflags': subprocess.CREATE_NEW_PROCESS_GROUP}
                             if os.name == 'nt' else {'start_new_session': True})
            # Keep provider lifecycle separate from the terminal's GDB client.
            with subprocess.Popen(command, cwd=self.project, env=self._env(), **spawn_options) as process:
                session = None
                try:
                    end = monotonic() + 45
                    while not session_path.is_file():
                        returncode = process.poll()
                        if returncode is not None:
                            self._result(request, result_path, returncode)
                            raise ActionError('SESSION_NOT_READY', 'Provider exited before publishing readiness')
                        if monotonic() > end:
                            raise ActionError('TIMEOUT', 'Provider did not publish readiness')
                        sleep(0.05)
                    try:
                        session = validate_session(read_json(session_path), request, profile, elf, debug_file)
                    except (OSError, ValueError) as error:
                        raise ActionError('SESSION_MISMATCH', str(error)) from error
                    if process.poll() is not None:
                        self._result(request, result_path, process.returncode)
                        raise ActionError('SESSION_NOT_READY', 'Provider exited after publishing readiness')
                    address = _endpoint_text(session['endpoint'])
                    print(f'GDB ready at {address} with {elf}', flush=True)
                    if options['client'] == 'ide':
                        write_json(session_dir / 'active.json', {'request': request, 'session': str(session_path)})
                        print('FLY_GDB_READY', flush=True)
                        return self._result(request, result_path, process.wait())
                    self._client(session, elf)
                finally:
                    if process.poll() is None:
                        self._close_debug(action, request, session, session_path, process)
                    if session_dir:
                        (session_dir / 'active.json').unlink(missing_ok=True)
                return self._result(request, result_path, process.returncode)

    def _client(self, session, elf):
        args = [session['gdb_path'], '-q', '-nx', str(elf)]
        connect = f'target remote {_endpoint_text(session["endpoint"])}'
        commands = session['after_connect']
        if session['mode'] == 'start':
            commands = ['break main', *commands, 'continue']
        # GDB abandons the rest of a command file on error, but still opens a prompt.
        # A second command file turns initialization failure into a nonzero process exit.
        with TemporaryDirectory(prefix='fly-gdb-') as scratch:
            script = Path(scratch) / 'startup.gdb'
            script.write_text('\n'.join(['set $fly_ready = 0', connect, *commands, 'set $fly_ready = 1']) + '\n',
                              encoding='utf-8')
            check = script.with_name('check.gdb')
            check.write_text('if $fly_ready == 0\n  quit 1\nend\n', encoding='utf-8')
            code = subprocess.call([*args, '-x', str(script), '-x', str(check)], cwd=self.project, env=self._env())
        if code:
            raise ActionError('TOOL_FAILED', f'GDB exited with {code}', exit_code=code)

    def _close_debug(self, action, request, session, session_path, process):
        try:
            if session is not None:
                self._request_stop(action, request, session, session_path)
            else:
                process.send_signal(signal.CTRL_BREAK_EVENT if os.name == 'nt' else signal.SIGINT)
            process.wait(timeout=8)
        except (OSError, ActionError, subprocess.TimeoutExpired):
            process.terminate()
            try:
                process.wait(timeout=4)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()

    def _request_stop(self, action, request, session, session_path):
        stop = self.request(action, 'stop', target=request['target'], build_type=request['build_type'],
                            input_data={'kind': 'session', 'session_path': str(session_path),
                                        'session_id': session['session_id']})
        return self.invoke(action, stop)

    def stop_ide(self, directory):
        try:
            active = read_json(directory / 'active.json')
        except FileNotFoundError:
            return
        request = validate_request(active['request'])
        if request['project_root'] != str(self.project) or request['board'] != self.board:
            raise ActionError('SESSION_MISMATCH', 'Session belongs to another application')
        session_path = Path(active['session'])
        session = read_json(session_path)
        action = self.select('debug', request['provider'])
        return self._request_stop(action, request, session, session_path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('cmake', 'index', 'symbols'))
    parser.add_argument('--project', required=True, type=Path)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--build-dir', type=Path)
    parser.add_argument('--build-type')
    parser.add_argument('--elf', type=Path)
    parser.add_argument('--debug-file', type=Path)
    args = parser.parse_args()
    try:
        if args.command == 'cmake':
            write_cmake(args.project.resolve(), args.output)
        elif args.command == 'index':
            write_index(args.project.resolve(), args.build_dir, args.build_type, args.elf)
        else:
            write_build_symbols(args.project.resolve(), args.build_type, args.elf, args.debug_file)
    except (OSError, ValueError) as error:
        print(f'FLY template packages: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
