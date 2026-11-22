"""Bind provider debug profiles to VS Code tasks and immutable firmware paths."""

import argparse
import re
import sys
from hashlib import sha256
from pathlib import Path

from action_protocol import ActionError, endpoint, read_json, write_json
from project_actions import Dispatcher, _endpoint_text, binding_key, project_fingerprint
from vscode_project import WORKSPACE_FOLDER, workspace_task, write_owned_entries, write_vscode_scaffold


def bind_artifact(project, sdk, manifest_path):
    manifest_path = Path(manifest_path).resolve()
    dispatcher = Dispatcher(project, sdk)
    artifact = dispatcher.artifact(manifest_path)
    write_json(project / '.vscode/fly-artifact.json', {
        'manifest_path': str(manifest_path), 'artifact_id': artifact['artifact_id'],
    })


def _task(label, argument, identity, *, background=False):
    task = workspace_task(label, ('debug', argument, identity))
    if background:
        task.update(isBackground=True, problemMatcher={
            'owner': 'fly-gdb',
            'pattern': {
                'regexp': r'^(.*?):(\d+):(\d+): (error|warning): (.*)$',
                'file': 1, 'line': 2, 'column': 3, 'severity': 4, 'message': 5,
            },
            'background': {
                'activeOnStart': True,
                'beginsPattern': '^FLY_GDB_STARTING$',
                'endsPattern': '^FLY_GDB_READY$',
            },
        })
    return task


def _render_fragment(value, variables):
    if isinstance(value, dict):
        return {key: _render_fragment(item, variables) for key, item in value.items()}
    if isinstance(value, list):
        return [_render_fragment(item, variables) for item in value]
    if isinstance(value, str) and re.fullmatch(r'@FLY_[A-Z_]+@', value):
        if value not in variables:
            raise ValueError(f'Unknown VS Code debug placeholder {value}')
        return variables[value]
    return value


def _configuration(dispatcher, profile, provider, binding, identifier, label, address):
    project = dispatcher.project
    fragment_path = (project / profile['vscode_fragment']).resolve()
    fragment_path.relative_to(project)
    fragment = read_json(fragment_path)
    if not isinstance(fragment, dict):
        raise ValueError(f'Invalid VS Code debug fragment {fragment_path}')
    options = binding['options']
    elf, _ = binding['symbols']
    name = f'FLY: {provider} {label}'
    start = f'{name} — start {identifier}'
    stop = f'{name} — stop {identifier}'
    fragment = _render_fragment(fragment, {
        '@FLY_EXECUTABLE@': elf,
        '@FLY_GDB@': options['gdb_path'],
        '@FLY_ENDPOINT@': _endpoint_text(address),
        '@FLY_AFTER_CONNECT@': profile['after_connect'],
        '@FLY_USES_ARCHIVE@': binding['input']['kind'] == 'artifact',
    })
    launch = {
        **fragment, 'name': name, 'cwd': WORKSPACE_FOLDER,
        'preLaunchTask': start, 'postDebugTask': stop,
    }
    write_json(project / '.vscode/fly-debug' / f'{identifier}.json', binding)
    return launch, [_task(start, '--ide-binding', identifier, background=True),
                    _task(stop, '--ide-stop', identifier)]


def write_debug_configs(project, sdk, build_dir, gdb_hint, *, build_type=None, elf=None):
    vscode = project / '.vscode'
    if build_type is None or elf is None:
        index = read_json(project / 'build/config/actions.json')
        build_type, elf = index['build_type'], Path(index['elf_path'])
    dispatcher = Dispatcher(project, sdk, build_dir=build_dir, build_type=build_type)
    debug_actions = [action for key, action in dispatcher.composition.actions.items() if key[0] == 'debug']
    artifact_path = artifact = None
    try:
        selection = read_json(vscode / 'fly-artifact.json')
        artifact_path = Path(selection['manifest_path']).resolve()
        artifact = dispatcher.artifact(artifact_path)
        if artifact['artifact_id'] != selection['artifact_id']:
            raise ValueError('Archive identity changed')
    except FileNotFoundError:
        pass
    except (OSError, ValueError, KeyError) as error:
        artifact_path = artifact = None
        print(f'FLY: Ignoring invalid bound archive: {error}. Run ./build.sh vscode --artifact again.', file=sys.stderr)

    try:
        external_address = endpoint(read_json(vscode / 'fly-debug-endpoint.json'))
    except FileNotFoundError:
        external_address = None
    except (OSError, ValueError) as error:
        external_address = None
        print(f'FLY: Invalid external debug endpoint: {error}', file=sys.stderr)
    configs = []
    tasks = []
    used_ports = set()
    fingerprint = project_fingerprint(project, generated=False)
    target_name = dispatcher.composition.app.data['app']['firmware_target']
    # Project scaffold always creates a companion .debug on embedded targets.
    build_debug_file = elf.with_name(f'{target_name}.debug')
    for action in debug_actions:
        provider = action['provider']
        for profile in action['debug_profiles']:
            if not profile.get('vscode_fragment'):
                continue
            if profile['server'] == 'external' and external_address is None:
                continue
            if profile['input'] == 'artifact' and artifact is None:
                continue
            target, active_type, partition = 'default', build_type, None
            try:
                gdb = dispatcher.gdb(gdb_hint)
                if profile['input'] == 'artifact':
                    target, active_type = artifact['target'], artifact['build_type']
                    partition, profile_elf, profile_debug_file = dispatcher.debug_paths(artifact_path, artifact)
                    input_data = {'kind': 'artifact', 'manifest_path': str(artifact_path),
                                  'artifact_id': artifact['artifact_id']}
                else:
                    profile_elf, profile_debug_file = elf, build_debug_file
                    input_data = {'kind': 'build', 'elf_path': str(elf), 'debug_file_path': str(build_debug_file)}
                dispatcher.require_tools(action, target, profile['input'], profile)
            except (ActionError, OSError) as error:
                print(f'FLY: Skipped {provider}/{profile["id"]}: {error}', file=sys.stderr)
                continue
            digest = sha256(f'{project}|{provider}|{profile["id"]}'.encode()).hexdigest()
            port = 20000 + int(digest[:8], 16) % 25000
            while port in used_ports:
                port = 20000 + (port - 19999) % 25000
            used_ports.add(port)
            address = external_address if profile['server'] == 'external' else {'host': '127.0.0.1', 'port': port}
            options = {'profile': profile['id'], 'mode': profile['mode'], 'server': profile['server'],
                       'client': 'ide', 'gdb_path': gdb, 'partition': partition, 'endpoint': address}
            binding = {
                'fingerprint': fingerprint, 'target': target, 'build_type': active_type, 'provider': provider,
                'input': input_data, 'options': options,
                'symbols': [str(profile_elf), str(profile_debug_file) if profile_debug_file else None],
            }
            label = profile['id']
            if partition:
                label += f' [{artifact["artifact_id"][:10]}]'
            try:
                launch, profile_tasks = _configuration(dispatcher, profile, provider, binding, binding_key(binding),
                                                       label, address)
            except (OSError, ValueError) as error:
                print(f'FLY: Skipped {provider}/{profile["id"]}: {error}', file=sys.stderr)
                continue
            configs.append(launch)
            tasks.extend(profile_tasks)
    if any(item['input'] == 'artifact' for action in debug_actions for item in action['debug_profiles']) and not artifact:
        print('FLY: Run ./build.sh vscode --artifact <manifest.json> to add archive debug configurations.')
    write_vscode_scaffold(project)
    owners = vscode / 'fly-owned.json'
    if configs or (vscode / 'launch.json').is_file():
        write_owned_entries(vscode / 'launch.json', '0.2.0', 'configurations', 'name',
                            configs, owners, 'debug-launch')
    write_owned_entries(vscode / 'tasks.json', '2.0.0', 'tasks', 'label', tasks, owners, 'debug-tasks')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', required=True, type=Path)
    parser.add_argument('--build-dir', required=True, type=Path)
    parser.add_argument('--build-type', required=True)
    parser.add_argument('--gdb')
    parser.add_argument('--elf', required=True, type=Path)
    args = parser.parse_args()
    try:
        write_debug_configs(args.project.resolve(), Path(__file__).resolve().parents[2], args.build_dir,
                            args.gdb, build_type=args.build_type, elf=args.elf)
    except (OSError, ValueError) as error:
        print(f'FLY VS Code: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
