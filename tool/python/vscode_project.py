import json
import os
import sys
from pathlib import Path

from action_protocol import read_json, write_json
from project_actions import action_command
from template_packs import load_project


COMPILER_CACHE_KEYS = ('CMAKE_C_COMPILER', 'CMAKE_CXX_COMPILER', 'CMAKE_ASM_COMPILER')
TOOL_CACHE_KEYS = (*COMPILER_CACHE_KEYS, 'CMAKE_MAKE_PROGRAM', 'Python3_EXECUTABLE')
CLANGD_MANAGED_OPTIONS = ('--compile-commands-dir', '--query-driver')
WORKSPACE_FOLDER = '${workspaceFolder}'
WORKSPACE_BUILD = f'{WORKSPACE_FOLDER}/build/cmake'


def _update_json(path, generated, merge):
    try:
        previous = json.loads(path.read_text(encoding='utf-8-sig'))
        updated = merge(previous, generated)
    except FileNotFoundError:
        previous = None
        updated = generated
    except ValueError:
        snippet = path.with_name(f'fly-{path.name}')
        snippet.write_text(json.dumps(generated, indent=4, ensure_ascii=False) + '\n', encoding='utf-8')
        print(f'Kept {path}. Merge the FLY entries from {snippet}.', file=sys.stderr)
        return False

    if previous != updated:
        write_json(path, updated)
    return True


def _merge_named_entries(existing, generated, key):
    if not isinstance(existing, list) or any(not isinstance(item, dict) for item in existing):
        raise ValueError('Expected an array of objects')

    pending = {item[key]: item for item in generated}
    merged = []
    for item in existing:
        name = item.get(key)
        if isinstance(name, str) and name in pending:
            merged.append(pending.pop(name))
        else:
            merged.append(item)
    return [*merged, *pending.values()]


def write_owned_entries(path, version, kind, key, generated, owners, group):
    try:
        state = read_json(owners)
    except (OSError, ValueError):
        state = {}
    if not isinstance(state, dict):
        state = {}
    previous_owned = state.get(group, [])
    if not isinstance(previous_owned, list):
        previous_owned = []
    accepted = []

    def merge(previous, _):
        if not isinstance(previous, dict) or not isinstance(previous.get(kind, []), list):
            raise ValueError(f'Expected a {kind} configuration')
        retained = [entry for entry in previous.get(kind, []) if entry not in previous_owned]
        names = {entry[key] for entry in retained
                 if isinstance(entry, dict) and isinstance(entry.get(key), str)}
        for item in generated:
            if item[key] in names:
                print(f'Kept user {kind} entry {item[key]!r} in {path}.', file=sys.stderr)
                continue
            if item[key] == 'FLY: Build' and any(
                    isinstance(entry.get('group'), dict) and entry['group'].get('kind') == 'build'
                    and entry['group'].get('isDefault') for entry in retained if isinstance(entry, dict)):
                item = dict(item, group='build')
            accepted.append(item)
        return {'version': version, **previous, kind: [*retained, *accepted]}

    proposal = {'version': version, kind: generated}
    if not path.exists():
        accepted.extend(generated)
    if _update_json(path, proposal, merge):
        state[group] = accepted
        write_json(owners, state)


def workspace_task(label, arguments):
    return {
        'label': label,
        'type': 'process',
        'command': f'{WORKSPACE_FOLDER}/build.sh',
        'args': list(arguments),
        'windows': {'command': 'cmd.exe', 'args': ['/d', '/c', 'build.bat', *arguments]},
        'options': {'cwd': WORKSPACE_FOLDER},
        'problemMatcher': [],
    }


def _write_tasks(project):
    project = Path(project)
    if not ((project / 'build.sh').is_file() and (project / 'build.bat').is_file()):
        return

    composition = load_project(project)
    tasks = [
        workspace_task('FLY: Setup', ('setup',)),
        workspace_task('FLY: Configure', ('vscode',)),
        dict(workspace_task('FLY: Build', ('build',)), group={'kind': 'build', 'isDefault': True}),
    ]
    if composition.board.data['board']['target_kind'] == 'mcu':
        tasks.append(workspace_task('FLY: Make image', ('make_img',)))
    for name, provider in composition.actions:
        if name == 'debug':
            continue
        tasks.append(workspace_task(f'FLY: {name.capitalize()} ({provider})',
                                    (action_command(name), '--via', provider)))

    vscode = project / '.vscode'
    write_owned_entries(vscode / 'tasks.json', '2.0.0', 'tasks', 'label', tasks,
                        vscode / 'fly-owned.json', 'base-tasks')


def _cmake_settings(build_dir):
    return {
        'cmake.sourceDirectory': WORKSPACE_FOLDER,
        'cmake.buildDirectory': build_dir,
        'cmake.useCMakePresets': 'never',
        'cmake.configureOnOpen': False,
        'cmake.configureSettings': {
            'CMAKE_EXPORT_COMPILE_COMMANDS': True,
        },
    }


def write_vscode_scaffold(project):
    _write_tasks(project)
    settings = _cmake_settings(WORKSPACE_BUILD)
    _update_json(project / '.vscode' / 'settings.json', settings,
                 lambda previous, _: previous)


def _merge_settings(previous, generated):
    if not isinstance(previous, dict):
        raise ValueError('Expected a workspace settings object')

    updated = {**previous, **generated}
    for key in ('cmake.configureSettings', 'cmake.environment'):
        existing = previous.get(key, {})
        if not isinstance(existing, dict):
            raise ValueError(f'Expected an object for {key}')
        updated[key] = {**existing, **generated[key]}
    updated['cmake.configureSettings'].pop('FLY_PROJECT_DIR', None)

    existing_args = previous.get('clangd.arguments', [])
    if not isinstance(existing_args, list) or any(not isinstance(arg, str) for arg in existing_args):
        raise ValueError('Expected an array of strings for clangd.arguments')

    clangd_args = []
    skip_value = False
    for arg in existing_args:
        if skip_value and not arg.startswith('-'):
            skip_value = False
            continue
        option, equals, _ = arg.partition('=')
        skip_value = option in CLANGD_MANAGED_OPTIONS and not equals
        if option not in CLANGD_MANAGED_OPTIONS:
            clangd_args.append(arg)
    updated['clangd.arguments'] = [*clangd_args, *generated['clangd.arguments']]
    return updated


def _cache_path(cache, key):
    value = cache.get(key)
    if value and not value.endswith('-NOTFOUND') and Path(value).is_absolute():
        return Path(value)
    return None


def write_vscode_project(project, cmake_dir, cache, tool_paths):
    project = Path(project).resolve()
    cmake_dir = Path(cmake_dir).resolve()
    vscode = project / '.vscode'

    tool_executables = {}
    for key in TOOL_CACHE_KEYS:
        path = _cache_path(cache, key)
        if path is not None:
            tool_executables[key] = path

    compiler_drivers = dict.fromkeys(tool_executables[key].as_posix()
                                     for key in COMPILER_CACHE_KEYS if key in tool_executables)
    clangd_args = [f'--compile-commands-dir={cmake_dir.as_posix()}']
    if compiler_drivers:
        clangd_args.append('--query-driver=' + ','.join(compiler_drivers))

    tool_dirs = [str(path.parent) for path in tool_executables.values()]
    gdb = _cache_path(cache, 'CMAKE_GDB')
    if gdb is not None:
        tool_dirs.append(str(gdb.parent))
    path_entries = dict.fromkeys([*map(str, tool_paths), *tool_dirs, '${env:PATH}'])

    build_dir = cmake_dir.as_posix()
    if project in cmake_dir.parents:
        build_dir = f'{WORKSPACE_FOLDER}/{cmake_dir.relative_to(project).as_posix()}'
    settings = _cmake_settings(build_dir)
    settings['cmake.configureSettings'].update({key: path.as_posix() for key, path in tool_executables.items()})
    settings['cmake.environment'] = {'PATH': os.pathsep.join(path_entries)}
    settings['clangd.arguments'] = clangd_args
    if cache.get('CMAKE_GENERATOR'):
        settings['cmake.generator'] = cache['CMAKE_GENERATOR']
    cmake = _cache_path(cache, 'CMAKE_COMMAND')
    if cmake is not None:
        settings['cmake.cmakePath'] = cmake.as_posix()
    _update_json(vscode / 'settings.json', settings, _merge_settings)

    compilers = {language: tool_executables[key].as_posix()
                 for language, key in zip(('C', 'CXX'), COMPILER_CACHE_KEYS) if key in tool_executables}
    if compilers:
        kits = [{'name': 'FLY', 'compilers': compilers}]
        _update_json(vscode / 'cmake-kits.json', kits,
                     lambda previous, generated: _merge_named_entries(previous, generated, 'name'))

    _write_tasks(project)
