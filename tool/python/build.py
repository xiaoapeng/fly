import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
from dataclasses import dataclass
from hashlib import sha256
from pathlib import Path

from action_protocol import ActionError, write_json
from arch_registry import for_board, validate_choice
from build_environment import configure_cli_output, prepare_python
from build_actions import add_commands, execute as execute_action
from project_actions import build_tree_lock, parse_endpoint, project_fingerprint
from template_packs import load_project

ENV_PATH_CONFIG_JSON_FILE_PATH = '.PATH.env.json'
GIT_MIRROR_SOURCE_JSON_FILE_PATH = '.git.mirror-source.json'
GIT_MIRROR_SOURCE_JSON_FILE_PATH_INIT = 'resource/git.mirror-source.json'


@dataclass(frozen=True)
class BuildPaths:
    sdk: Path
    project: Path

    @property
    def output(self):
        return self.project / 'build'

    @property
    def cmake(self):
        return self.output / 'cmake'

    @property
    def generated(self):
        return self.output / 'config'

    @property
    def config(self):
        return self.generated / '.config'

    @property
    def images(self):
        return self.project / 'image'

    @property
    def mirror(self):
        return self.output / GIT_MIRROR_SOURCE_JSON_FILE_PATH

    def kconfig(self):
        import kconfiglib

        board = load_project(self.project).board
        architecture = for_board(self.sdk / 'arch', board.id, board.data['board'])
        os.environ.update(architecture.kconfig_environment())
        return kconfiglib.Kconfig(str(self.project / 'Kconfig')), architecture


def get_cmake_cache_value(build_dir, key):
    cache = build_dir / 'CMakeCache.txt'
    if cache.is_file():
        with cache.open(encoding='utf-8') as f:
            for line in f:
                if line.startswith(f'{key}:'):
                    _, value = line.split('=', 1)
                    return value.strip()
    return None


def write_generated_config(paths, kconf, architecture):
    paths.generated.mkdir(parents=True, exist_ok=True)
    kconf.write_autoconf(str(paths.generated / 'autoconf.h'))
    config = paths.generated / '.config.cmake'
    lines = ['# CMake config generated from Kconfig',
             f'set(FLY_ARCH_RELATIVE_DIR "{architecture.relative_path}")']
    for line in paths.config.read_text(encoding='utf-8').splitlines():
        line = line.strip()
        if line and not line.startswith('#'):
            key, value = line.split('=', 1)
            lines.append(f'set({key.strip()} {value.strip()})')
    content = '\n'.join(lines) + '\n'
    if not config.is_file() or config.read_text(encoding='utf-8') != content:
        config.write_text(content, encoding='utf-8')


def _validate_project_config(kconf, architecture):
    validate_choice(kconf, architecture)
    for symbol in kconf.unique_defined_syms:
        if symbol.choice is not None and symbol.choice.visibility and symbol.user_value == 2 and not symbol.visibility:
            raise ValueError(f'CONFIG_{symbol.name}=y is unavailable for the selected configuration '
                             f'({architecture.id}/{architecture.profile}).')


def run_loadconfig(paths, input_file):
    kconf, architecture = paths.kconfig()
    kconf.load_config(str(input_file))
    _validate_project_config(kconf, architecture)
    paths.config.parent.mkdir(parents=True, exist_ok=True)
    kconf.write_config(str(paths.config))
    write_generated_config(paths, kconf, architecture)


def refresh_config(paths):
    selected = paths.config if paths.config.is_file() else paths.project / 'defconfig'
    run_loadconfig(paths, selected)


def check_managed_sdk(paths):
    installed = paths.project / 'fly'
    if (not installed.is_symlink() and (installed / '.git/fly-managed').is_file()
            and installed.resolve() == paths.sdk):
        subprocess.run([
            sys.executable, '-B', str(paths.project / 'tool/python/setup_fly.py'), '--check',
        ], cwd=paths.project, check=True)


def run_menuconfig(paths):
    # Windows only needs the optional windows-curses wheel for menuconfig.
    try:
        import menuconfig
    except ImportError as error:
        raise ValueError(
            f"menuconfig UI unavailable ({error}). On Windows run: pip install windows-curses"
        ) from error

    refresh_config(paths)
    os.environ['KCONFIG_CONFIG'] = str(paths.config)
    kconf, architecture = paths.kconfig()
    menuconfig.menuconfig(kconf)
    # Use the saved .config, including when the user exits without saving.
    if paths.config.is_file():
        kconf.load_config(str(paths.config))
        _validate_project_config(kconf, architecture)
        write_generated_config(paths, kconf, architecture)


def run_saveconfig(paths, output_file):
    if Path(output_file).is_absolute():
        raise ValueError('Use a path relative to the project for saveconfig.')
    if not paths.config.is_file():
        refresh_config(paths)
    kconf, architecture = paths.kconfig()
    kconf.load_config(str(paths.config))
    _validate_project_config(kconf, architecture)
    config_file = paths.project / output_file
    kconf.write_min_config(str(config_file))
    print(f'Defconfig saved to: {config_file}')


def run_distclean(paths):
    if paths.output.exists():
        shutil.rmtree(paths.output)


def require_cmake():
    if not shutil.which('cmake'):
        raise ValueError('cmake not installed. Please install cmake first.')


def run_cmake(paths, command):
    # Reconfigure and build with the SDK that launched this command, without
    # persisting an explicit CMake override.
    sdk = paths.sdk.as_posix()
    env = dict(os.environ, FLY_APP_SDK_DIR=sdk, FLY_ROOT=sdk)
    subprocess.run(command, cwd=paths.project, env=env, check=True)


def configured_sdk_path(build_dir):
    return (get_cmake_cache_value(build_dir, 'FLY_RESOLVED_SDK_DIR')
            or get_cmake_cache_value(build_dir, 'FLY_SDK_DIR'))


def project_rules_changed(paths):
    configured = get_cmake_cache_value(paths.cmake, 'FLY_PROJECT_RULES_HASH')
    files = ('CMakeLists.txt', 'app/CMakeLists.txt', 'board/board.cmake')
    rule_hashes = ''.join(sha256(path.read_bytes()).hexdigest()
                          for path in (paths.project / name for name in files) if path.is_file())
    return configured != rule_hashes


def action_index_changed(paths):
    index = paths.generated / 'actions.json'
    try:
        data = json.loads(index.read_text(encoding='utf-8'))
        return data['fingerprint'] != project_fingerprint(paths.project)
    except (OSError, KeyError, TypeError, ValueError):
        return True


def configure_project(paths, build_type=None, *, force=False):
    require_cmake()
    refresh_config(paths)

    previous_source = get_cmake_cache_value(paths.cmake, 'CMAKE_HOME_DIRECTORY')
    previous_type = get_cmake_cache_value(paths.cmake, 'CMAKE_BUILD_TYPE')
    previous_sdk = configured_sdk_path(paths.cmake)
    retained_override = None
    if previous_source and (
        Path(previous_source).resolve() != paths.project
        or not previous_sdk or Path(previous_sdk).resolve() != paths.sdk
    ):
        if (Path(previous_source).resolve() == paths.project
                and get_cmake_cache_value(paths.cmake, 'FLY_RESOLVED_SDK_DIR')):
            retained_override = get_cmake_cache_value(paths.cmake, 'FLY_SDK_DIR')
        # The CMake cache stores absolute source directories.
        (paths.cmake / 'CMakeCache.txt').unlink()
        cmake_files = paths.cmake / 'CMakeFiles'
        if cmake_files.is_dir():
            shutil.rmtree(cmake_files)

    build_tool = get_cmake_cache_value(paths.cmake, 'CMAKE_MAKE_PROGRAM')
    build_program = Path(build_tool).stem if build_tool else ''
    build_type = build_type or previous_type or 'Release'
    configure = (
        force or not build_tool or previous_type != build_type
        or (build_program == 'ninja' and not (paths.cmake / 'build.ninja').is_file())
        or (build_program in ('make', 'gmake') and not (paths.cmake / 'Makefile').is_file())
        or not (paths.cmake / 'compile_commands.json').is_file()
        or get_cmake_cache_value(paths.cmake, 'Python3_EXECUTABLE') != Path(sys.executable).as_posix()
        or project_rules_changed(paths)
        or action_index_changed(paths)
    )

    if configure:
        command = [
            'cmake', '-S', str(paths.project), '-B', str(paths.cmake),
            f'-DCMAKE_BUILD_TYPE:STRING={build_type}',
            '-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE',
            f'-DPython3_EXECUTABLE:FILEPATH={sys.executable}',
        ]
        if retained_override:
            command.append(f'-DFLY_SDK_DIR:PATH={retained_override}')
        if platform.system() == 'Windows' and not build_tool:
            if shutil.which('ninja'):
                command.extend(['-G', 'Ninja'])
            elif shutil.which('make'):
                command.extend(['-G', 'Unix Makefiles'])
            else:
                raise ValueError('No build tool found. Please install ninja or make.')
        run_cmake(paths, command)


def update_vscode(paths, tool_paths):
    from vscode_project import write_vscode_project

    keys = (
        'CMAKE_GENERATOR', 'CMAKE_C_COMPILER', 'CMAKE_CXX_COMPILER',
        'CMAKE_ASM_COMPILER', 'CMAKE_MAKE_PROGRAM', 'CMAKE_GDB',
        'Python3_EXECUTABLE', 'CMAKE_COMMAND',
    )
    cache = {key: get_cmake_cache_value(paths.cmake, key) for key in keys}
    write_vscode_project(paths.project, paths.cmake, cache, tool_paths)


def run_build(paths, build_type=None, jobs=None, tool_paths=()):
    configure_project(paths, build_type)
    if (paths.project / 'build.sh').is_file() or (paths.project / 'build.bat').is_file():
        try:
            update_vscode(paths, tool_paths)
            if not (paths.project / '.vscode/launch.json').is_file():
                from project_ide import write_debug_configs

                write_debug_configs(paths.project, paths.sdk, paths.cmake, get_cmake_cache_value(paths.cmake, 'CMAKE_GDB'))
        except (OSError, ValueError) as error:
            print(f'Could not update VS Code configuration: {error}', file=sys.stderr)
    jobs = jobs if jobs is not None else (os.cpu_count() or 1)
    run_cmake(paths, [
        'cmake', '--build', str(paths.cmake), '--target', 'all', f'-j{jobs}', '--',
    ])


def run_target(paths, target):
    require_cmake()
    if not (paths.cmake / 'CMakeCache.txt').is_file():
        raise ValueError('Build directory is not configured. Run build first.')
    configured_sdk = configured_sdk_path(paths.cmake)
    configured_project = get_cmake_cache_value(paths.cmake, 'CMAKE_HOME_DIRECTORY')
    if not configured_sdk or Path(configured_sdk).resolve() != paths.sdk:
        raise ValueError('The FLY SDK location changed. Run build first.')
    if not configured_project or Path(configured_project).resolve() != paths.project:
        raise ValueError('The project location changed. Run build first.')
    # Older Make versions can miss same-second edits to app packaging settings.
    if target.endswith('_make_img') and (project_rules_changed(paths) or action_index_changed(paths)):
        run_cmake(paths, ['cmake', '-S', str(paths.project), '-B', str(paths.cmake)])
    run_cmake(paths, ['cmake', '--build', str(paths.cmake), '--target', target])


def run_append_path_env(source_dir, env_path):
    env_PATH_data = []
    env_path = os.path.abspath(env_path)
    env_path_config_json_path = f"{source_dir}/{ENV_PATH_CONFIG_JSON_FILE_PATH}"
    if not os.path.exists(env_path):
        print(f"Path does not exist: {env_path}")
        sys.exit(1)
    if os.path.exists(env_path_config_json_path):
        with open(env_path_config_json_path, 'r') as f:
            env_PATH_data = json.load(f)
    env_PATH_data.append(env_path)
    with open(env_path_config_json_path, 'w') as f:
        json.dump(env_PATH_data, f, indent=4)

def run_package_mirror(paths, list=False, enable=None, disable=None, update=False):
    if not paths.mirror.is_file():
        paths.mirror.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(paths.sdk / GIT_MIRROR_SOURCE_JSON_FILE_PATH_INIT, paths.mirror)
        print(f"Mirror source config initialized: {paths.mirror}")

    with paths.mirror.open(encoding='utf-8') as f:
        mirror_sources = json.load(f)

    for item in mirror_sources:
        show = list
        if item['name'] == enable:
            item['enable'] = True
            show = True
        if item['name'] == disable:
            item['enable'] = False
            show = True
        if show:
            state = 'enable' if item['enable'] else 'disable'
            print(f"{item['name']:<40} [{state:<7}]")

    with paths.mirror.open('w', encoding='utf-8') as f:
        json.dump(mirror_sources, f, indent=4)

    if update:
        import fetch_git_project

        manager = fetch_git_project.MirrorSourceManager(paths.mirror)
        manager.find_best_mirror_source(force_update=True)


def load_env(source_dir):
    path_config = source_dir / ENV_PATH_CONFIG_JSON_FILE_PATH
    if not path_config.is_file():
        return []
    tool_paths = json.loads(path_config.read_text(encoding='utf-8'))
    os.environ['PATH'] = os.pathsep.join([*tool_paths, os.environ.get('PATH', '')])
    return tool_paths


def main():
    configure_cli_output()
    parser = argparse.ArgumentParser(description='Build a FLY application.')
    parser.add_argument('--project-dir', type=Path, required=True, help='Path to the application repository.')
    
    subparsers = parser.add_subparsers(dest='command', help='Command to execute.', required=False)
    subparsers.add_parser('prepare', help='Generate Kconfig output and report the build interpreter.')

    # Subparser for menuconfig
    parser_menuconfig = subparsers.add_parser('menuconfig', help='Open menuconfig UI.')

    # Subparser for saveconfig
    parser_saveconfig = subparsers.add_parser('saveconfig', help='Save current config as a minimal file.')
    parser_saveconfig.add_argument('savefile_name', type=str,nargs='?', default="defconfig", help='Output filename (default: defconfig).')

    # Subparser for loadconfig
    parser_loadconfig = subparsers.add_parser('loadconfig', help='Load config file.')
    parser_loadconfig.add_argument('loadfile_name', type=str, help='Input config filepath.')

    # clean
    parser_clean = subparsers.add_parser('clean', help='Clean build outputs (keep config).')

    # distclean 
    parser_distclean = subparsers.add_parser('distclean', help='Delete build and generated configs, keeping top-level image packages.')

    add_commands(subparsers)

    # Subparser for build
    parser_build = subparsers.add_parser('build', help='Build project.')
    parser_build.add_argument('build_type', type=str, nargs='?', default=None, help='Build type (e.g. Debug/Release/MinSizeRel).')
    parser_build.add_argument('-j', '--jobs', type=int, default=argparse.SUPPRESS, help='Number of parallel jobs.')
    parser.add_argument('-j', '--jobs', type=int, default=os.cpu_count(), help='Number of parallel jobs.')

    parser_vscode = subparsers.add_parser('vscode', help='Configure the project and generate VS Code settings.')
    parser_vscode.add_argument('build_type', nargs='?', help='CMake build type, for example Debug or Release.')
    parser_vscode.add_argument('--artifact', type=Path, help='Bind archive debug to this manifest.json.')
    parser_vscode.add_argument('--endpoint', type=parse_endpoint, help='Enable external GDB profiles at HOST:PORT.')

    # Subparser for package_update <pack_name> -l --list 
    parser_package_update = subparsers.add_parser('package_update', help='Update package dependencies.')
    parser_package_update.add_argument('pack_name', type=str, nargs='?', default='all', help='Package name (default: all).')
    parser_package_update.add_argument('-l', '--list', action='store_true', help='List updatable dependency packages.')

    # Subparser for package_mirror [-l,--list] [-e,--enable <name>] [-d,--disable <name>] [-u,--update]
    parser_package_mirror = subparsers.add_parser('package_mirror', help='Manage package mirror source settings.')
    parser_package_mirror.add_argument('-l', '--list', action='store_true', help='List mirror source status.')
    parser_package_mirror.add_argument('-e', '--enable', type=str, nargs='?', default=None, help='Enable specified mirror source.')
    parser_package_mirror.add_argument('-d', '--disable', type=str, nargs='?', default=None, help='Disable specified mirror source.')
    parser_package_mirror.add_argument('-u', '--update', action='store_true', help='Refresh mirror benchmark and best source.')

    # Add the PATH environment variable
    parser_add_path = subparsers.add_parser('add_path_env', help='Append path to FLY PATH config.')
    parser_add_path.add_argument('env_path', type=str, help='Path to append.')

    args = parser.parse_args()
    sdk = Path(__file__).resolve().parents[2]
    paths = BuildPaths(sdk, args.project_dir.resolve())
    if paths.project == sdk:
        parser.error('Specify an application directory. Use fly.sh or fly.bat new to create one.')
    if not (paths.project / 'Kconfig').is_file() or not (paths.project / 'CMakeLists.txt').is_file():
        parser.error(f'A FLY application requires Kconfig and CMakeLists.txt in {paths.project}')
    if not (paths.project / 'fly.project.json').is_file():
        parser.error('No fly.project.json. Generate a project with fly.sh new or fly.bat new.')

    os.environ['FLY_SDK_DIR'] = sdk.as_posix()
    os.environ['PYTHONDONTWRITEBYTECODE'] = '1'

    try:
        os.chdir(paths.project)
        tool_paths = load_env(paths.sdk)
        if args.command == 'distclean':
            with build_tree_lock(paths.project):
                run_distclean(paths)
            return 0
        if args.command == 'add_path_env':
            run_append_path_env(sdk, args.env_path)
            return 0
        if args.command == 'rttlog':
            print('rttlog is deprecated. Use ./build.sh log instead.', file=sys.stderr)
        if (args.command in ('capabilities', 'flash-run', 'log', 'debug', 'rttlog')
                or args.command == 'make_img' and (args.list or args.artifact is not None)):
            run_project_action(paths, args)
            return 0

        if args.command == 'prepare':
            check_managed_sdk(paths)
        interpreter = prepare_python(paths.output, output=sys.stderr if args.command == 'prepare' else None)
        if interpreter is not None:
            return subprocess.call([str(interpreter), '-B', str(Path(__file__).resolve()), *sys.argv[1:]])

        import kconfiglib

        try:
            run_command(paths, args, tool_paths)
        except kconfiglib.KconfigError as error:
            raise ValueError(str(error)) from error
    except ActionError as error:
        print(f'Error: {error}', file=sys.stderr)
        return error.exit_code
    except KeyboardInterrupt:
        print('Cancelled.', file=sys.stderr)
        return 130
    except (OSError, ValueError) as error:
        print(f'Error: {error}', file=sys.stderr)
        return 1
    except subprocess.CalledProcessError as error:
        print(f'Build command failed with exit code {error.returncode}', file=sys.stderr)
        return error.returncode
    return 0


def run_project_action(paths, args):
    execute_action(paths, args, cache=get_cmake_cache_value,
                   configure=configure_project, build_target=run_target)


def run_command(paths, args, tool_paths):
    if args.command == 'prepare':
        refresh_config(paths)
        print(Path(sys.executable).as_posix())
        return
    if args.command == 'make_img':
        run_project_action(paths, args)
        return
    with build_tree_lock(paths.project):
        run_build_command(paths, args, tool_paths)


def run_build_command(paths, args, tool_paths):
    if args.command == 'menuconfig':
        run_menuconfig(paths)
    elif args.command == 'saveconfig':
        run_saveconfig(paths, args.savefile_name)
    elif args.command == 'loadconfig':
        run_loadconfig(paths, args.loadfile_name)
    elif args.command == 'clean':
        require_cmake()
        if (paths.cmake / 'CMakeCache.txt').is_file():
            run_target(paths, 'clean')
    elif args.command == 'package_update':
        target = 'show_list' if args.list else args.pack_name
        run_target(paths, f'package_update_{target}')
    elif args.command == 'package_mirror':
        run_package_mirror(paths, args.list, args.enable, args.disable, args.update)
    elif args.command == 'vscode':
        from project_ide import bind_artifact, write_debug_configs

        if args.artifact is not None:
            bind_artifact(paths.project, paths.sdk, args.artifact)
        if args.endpoint is not None:
            write_json(paths.project / '.vscode/fly-debug-endpoint.json', args.endpoint)
        configure_project(paths, args.build_type, force=True)
        update_vscode(paths, tool_paths)
        write_debug_configs(paths.project, paths.sdk, paths.cmake,
                            get_cmake_cache_value(paths.cmake, 'CMAKE_GDB'))
    else:
        run_build(paths, getattr(args, 'build_type', None), args.jobs, tool_paths)


if __name__ == '__main__':
    sys.exit(main())
