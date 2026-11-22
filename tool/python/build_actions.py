"""CLI resolution of build and immutable archive inputs."""

import argparse
import json
from contextlib import nullcontext
from pathlib import Path

from action_protocol import ActionError, ID_PATTERN, KEY_PATTERN, read_json, string
from project_actions import (Dispatcher, binding_key, build_tree_lock, choose_profile, file_lock, parse_endpoint,
                             project_fingerprint, verify_build_symbols)


def _tool(value):
    name, separator, path = value.partition('=')
    if not separator or not path:
        raise argparse.ArgumentTypeError('Use --tool ID=/path/to/tool')
    try:
        string(name, 'tool ID', pattern=KEY_PATTERN)
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error)) from error
    return name, str(Path(path).resolve())


def add_commands(subparsers):
    capabilities = subparsers.add_parser('capabilities', help='List selected packs, capabilities, and available actions.')
    capabilities.add_argument('--json', action='store_true', help='Print the capability catalogue as JSON')
    capabilities.add_argument('--target', help='Named image target (default: default)')
    capabilities.add_argument('--build-type', help='Select the build configuration')
    capabilities.add_argument('--tool', action='append', type=_tool, default=[], metavar='ID=PATH',
                              help='Override a tool executable or resource directory')
    capabilities.add_argument('--endpoint', type=parse_endpoint, metavar='HOST:PORT',
                              help='Check an external debug profile with this endpoint')
    for name, help_text in (
        ('make_img', 'Build a common firmware archive, or export it with --via.'),
        ('flash-run', 'Load and start a published firmware archive.'),
        ('log', 'Read logs from a provider.'),
        ('debug', 'Debug a build or a firmware archive.'),
        ('rttlog', 'Deprecated alias for log.'),
    ):
        command = subparsers.add_parser(name, help=help_text, description=help_text)
        command.add_argument('target', nargs='?', help='Named image target (default: default)')
        command.add_argument('--via', metavar='PROVIDER', help='Choose an enabled provider')
        command.add_argument('--list', action='store_true', help='List providers and tool status')
        command.add_argument('--artifact', type=Path, help='Use this immutable firmware manifest.json')
        command.add_argument('--build-type', help='Select the build configuration')
        command.add_argument('--tool', action='append', type=_tool, default=[], metavar='ID=PATH',
                             help='Override a tool executable or resource directory')
        if name == 'flash-run':
            command.add_argument('--duration-ms', type=int, help='Virtual runtime in milliseconds for simulation')
        if name == 'debug':
            command.add_argument('--profile', help='Select the debug profile')
            command.add_argument('--mode', choices=('start', 'attach'))
            command.add_argument('--server', choices=('owned', 'external'))
            command.add_argument('--endpoint', type=parse_endpoint, metavar='HOST:PORT')
            command.add_argument('--partition', help='Partition to debug in a multi-partition archive')
            command.add_argument('--ide-binding', help=argparse.SUPPRESS)
            command.add_argument('--ide-stop', help=argparse.SUPPRESS)


def _load_current(paths, target, build_type):
    from firmware_payload import find_artifact

    try:
        return find_artifact(paths.project, target, build_type)
    except (OSError, ValueError) as error:
        raise ActionError(getattr(error, 'code', 'ARTIFACT_INVALID'),
                          f'{error} — run ./build.sh make_img {target} --build-type {build_type}') from error


def _build_elf(paths, build_type, dispatcher):
    try:
        index = read_json(paths.generated / 'actions.json')
        if (index['fingerprint'] != project_fingerprint(paths.project)
                or index['build_type'] != build_type or index['build_dir'] != str(paths.cmake)
                or index['actions'] != list(dispatcher.composition.actions.values())
                or index['defaults'] != dispatcher.composition.defaults):
            raise ValueError('Stale or modified CMake action index')
        # Build paths come from the build target, not an arbitrary path added to the index.
        elf = Path(index['elf_path'])
        target_name = dispatcher.composition.app.data['app']['firmware_target']
        if elf.parent != paths.cmake or elf.name not in (target_name, f'{target_name}.elf', f'{target_name}.exe'):
            raise ValueError('Unexpected configured ELF path')
    except (OSError, KeyError, TypeError, ValueError) as error:
        raise ActionError('SYMBOLS_UNAVAILABLE', f'{error} — run ./build.sh build {build_type} first.') from error
    if not elf.is_file():
        raise ActionError('SYMBOLS_UNAVAILABLE', f'{elf} does not exist. Build {build_type} first.')
    debug_file = elf.parent / f'{target_name}.debug'
    debug_file = debug_file if debug_file.is_file() else None
    verify_build_symbols(paths.project, build_type, elf, debug_file)
    return elf, debug_file


def execute(paths, args, *, cache, configure, build_target):
    command = 'log' if args.command == 'rttlog' else args.command
    target = string(args.target or 'default', 'target', pattern=KEY_PATTERN)
    build_type = string(args.build_type or cache(paths.cmake, 'CMAKE_BUILD_TYPE') or 'Release',
                        'build type', pattern=KEY_PATTERN)
    dispatcher = Dispatcher(paths.project, paths.sdk, build_dir=paths.cmake, build_type=build_type, tools=args.tool)
    name = 'export' if command == 'make_img' else command
    if command == 'capabilities':
        address = args.endpoint
        catalogue = dispatcher.capabilities(target, gdb_hint=cache(paths.cmake, 'CMAKE_GDB'), address=address)
        if args.json:
            print(json.dumps(catalogue, indent=2))
        else:
            print('Packages')
            for pack in catalogue['packages']:
                suffix = '' if pack['selected'] else ' (dependency)'
                print(f'  {pack["id"]}{suffix}  {pack["label"]}')
            print('Capabilities')
            for capability in catalogue['capabilities']:
                print(f'  {capability}')
            print('Actions')
            if not catalogue['actions']:
                print('  No provider actions selected.')
            dispatcher.print_actions(catalogue['actions'], target, address)
        return
    if args.list:
        dispatcher.list(name, target, gdb_hint=cache(paths.cmake, 'CMAKE_GDB'),
                        address=getattr(args, 'endpoint', None))
        return
    if command == 'debug' and args.ide_stop:
        binding = string(args.ide_stop, 'binding ID', pattern=ID_PATTERN)
        dispatcher.stop_ide(paths.images / '.sessions' / binding)
        return
    if command == 'debug' and args.ide_binding:
        _execute_binding(paths, dispatcher, args.ide_binding)
        return

    action = dispatcher.select(name, args.via) if command != 'make_img' or args.via else None
    profile = None
    artifact_path = args.artifact
    if command == 'make_img' and artifact_path is None:
        if action:
            dispatcher.require_tools(action, target, 'artifact')
        with build_tree_lock(paths.project):
            configure(paths, build_type)
            build_target(paths, f'{target}_make_img')
            artifact_path = _load_current(paths, target, build_type)

    if command == 'debug':
        desired = 'artifact' if artifact_path else None
        if not desired and not args.profile and any(
                item['input'] == 'build' and 'pre_action' not in item for item in action['debug_profiles']):
            desired = 'build'
        profile = choose_profile(action, input_kind=desired, profile_id=args.profile, mode=args.mode,
                                 server=args.server, address=args.endpoint)
        kind = profile['input']
    else:
        kind = 'none' if action and action['inputs'] == ['none'] else 'artifact'
    if artifact_path and kind != 'artifact':
        raise ActionError('INVALID_REQUEST', 'Selected action does not accept --artifact', exit_code=2)
    if action and kind not in action['inputs']:
        raise ActionError('INVALID_REQUEST', f'{name} does not accept {kind}', exit_code=2)

    input_data = {'kind': kind}
    artifact = None
    if kind == 'artifact':
        artifact_path = Path(artifact_path or _load_current(paths, target, build_type)).resolve()
        artifact = dispatcher.artifact(artifact_path, target=args.target, build_type=args.build_type)
        target, build_type = artifact['target'], artifact['build_type']
        input_data.update(manifest_path=str(artifact_path), artifact_id=artifact['artifact_id'])
    if not action:
        print(json.dumps({**input_data, 'target': target, 'build_type': build_type}))
        return

    dispatcher.require_tools(action, target, kind, profile)
    options = {}
    if command == 'flash-run' and args.duration_ms is not None:
        options['duration_ms'] = args.duration_ms
    if command == 'debug':
        lock = build_tree_lock(paths.project) if kind == 'build' else nullcontext()
        with lock:
            if kind == 'artifact':
                partition, elf, debug_file = dispatcher.debug_paths(artifact_path, artifact, args.partition)
            else:
                if args.partition:
                    raise ActionError('INVALID_REQUEST', '--partition requires an archive', exit_code=2)
                partition = None
                elf, debug_file = _build_elf(paths, build_type, dispatcher)
                input_data.update(elf_path=str(elf), debug_file_path=str(debug_file) if debug_file else None)
            options = {'profile': profile['id'], 'mode': profile['mode'], 'server': profile['server'],
                       'client': 'cli', 'gdb_path': dispatcher.gdb(cache(paths.cmake, 'CMAKE_GDB')),
                       'partition': partition, 'endpoint': args.endpoint}
            request = dispatcher.request(action, 'execute', target=target, input_data=input_data,
                                         options=options, build_type=build_type)
            data = dispatcher.execute_debug(action, request, profile, elf, debug_file, artifact=artifact)
    else:
        request = dispatcher.request(action, 'execute', target=target, input_data=input_data,
                                     options=options, build_type=build_type)
        data = (dispatcher.flash_run(action, request, artifact) if name == 'flash-run'
                else dispatcher.invoke(action, request))
    if name != 'log':
        print(json.dumps(data))


def _execute_binding(paths, dispatcher, binding_id):
    binding_id = string(binding_id, 'binding ID', pattern=ID_PATTERN)
    directory = paths.images / '.sessions' / binding_id
    with file_lock(directory / 'owned.lock', blocking=False):
        print('FLY_GDB_STARTING', flush=True)
        binding = read_json(paths.project / '.vscode/fly-debug' / f'{binding_id}.json')
        if (binding_key(binding) != binding_id
                or binding['fingerprint'] != project_fingerprint(paths.project, generated=False)):
            raise ActionError('SESSION_MISMATCH', 'Debug configuration changed. Run ./build.sh vscode again.')
        action = dispatcher.select('debug', binding['provider'])
        options = binding['options']
        input_data = binding['input']
        profile = choose_profile(action, input_kind=input_data['kind'], profile_id=options['profile'],
                                 mode=options['mode'], server=options['server'], address=options['endpoint'])
        dispatcher.require_tools(action, binding['target'], profile['input'], profile)
        artifact = None
        if profile['input'] == 'artifact':
            artifact_path = Path(input_data['manifest_path'])
            artifact = dispatcher.artifact(artifact_path, target=binding['target'], build_type=binding['build_type'])
            if artifact['artifact_id'] != input_data['artifact_id']:
                raise ActionError('ARTIFACT_MISMATCH', 'Bound debugging artifact has changed.')
            _, elf, debug_file = dispatcher.debug_paths(artifact_path, artifact, options['partition'])
        else:
            elf = Path(input_data['elf_path'])
            debug_file = Path(input_data['debug_file_path']) if input_data['debug_file_path'] else None
        resolved = [str(elf), str(debug_file) if debug_file else None]
        if resolved != binding['symbols'] or dispatcher.gdb(options['gdb_path']) != options['gdb_path']:
            raise ActionError('SESSION_MISMATCH', 'Debugger or symbol paths changed. Reconfigure VS Code.')
        request = dispatcher.request(action, 'execute', target=binding['target'], input_data=input_data,
                                     options=options, build_type=binding['build_type'])
        port = options['endpoint']['port']
        lock = build_tree_lock(paths.project) if profile['input'] == 'build' else nullcontext()
        with lock, file_lock(paths.images / '.sessions' / f'port-{port}.lock', blocking=False):
            if profile['input'] == 'build':
                verify_build_symbols(paths.project, binding['build_type'], elf, debug_file)
            dispatcher.execute_debug(action, request, profile, elf, debug_file, artifact=artifact,
                                     session_dir=directory)
