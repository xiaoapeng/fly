import shutil
import argparse
import os
import sys
import kconfiglib
import menuconfig
import platform
import json
import fetch_git_project
import check_git_status

K_CONFIG_FILR_PATH = 'Kconfig'
DOT_CONFIG_FILE_PATH = '.config'
CMAKE_BUILD_DIR_PATH = 'build'
AUTO_GENERATE_DIR_PATH = 'auto-generate'
AUTO_GENERATE_CMAKE_CONFIG_FILE_PATH = f'{AUTO_GENERATE_DIR_PATH}/.config.cmake'
AUTO_GENERATE_C_HEADER_FILE_PATH = f'{AUTO_GENERATE_DIR_PATH}/autoconf.h'
CURRENT_IMAGE_DIR_PATH = 'image/CURRENT'
ENV_PATH_CONFIG_JSON_FILE_PATH = '.PATH.evn.json'
GIT_MIRROR_SOURCE_JSON_FILE_PATH = '.git.mirror-source.json'
GIT_MIRROR_SOURCE_JSON_FILE_PATH_INIT = 'resource/git.mirror-source.json'

def check_tool_installed(tool_name):
    # Check if the tool is in PATH
    tool_path = shutil.which(tool_name)
    if tool_path:
        return True
    else:
        return False

def get_file_modification_time(file_path):
    # Return the last modification timestamp of a file.
    if not os.path.exists(file_path):
        return 0
    return os.path.getmtime(file_path)

def get_cmake_cache_value(build_dir, key):
    if os.path.exists(f"{build_dir}/CMakeCache.txt"):
        with open(f"{build_dir}/CMakeCache.txt", "r") as cache_file:
            for line in cache_file:
                if line.startswith(f'{key}:'):
                    _, value = line.split("=", 1)
                    return value.strip()
    return None

def convert_config_to_cmake(config_file, cmake_file):
    with open(config_file, 'r') as f:
        lines = f.readlines()
    
    cmake_lines = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        
        key, value = line.split('=', 1)
        key = key.strip()
        value = value.strip()
        
        # Handle the case where 'm' or 'y' are not treated as booleans but as strings
        cmake_line = f"set({key} {value})"
        
        cmake_lines.append(cmake_line)
    
    with open(cmake_file, 'w') as f:
        f.write("# CMakeLists.txt generated from .config file\n")
        f.write("\n".join(cmake_lines) + "\n")
    
    print(f"Conversion complete. CMake file written to: {cmake_file}")

def try_update_cmake_config(source_dir):
    if get_file_modification_time(f"{source_dir}/{DOT_CONFIG_FILE_PATH}") > get_file_modification_time(f"{source_dir}/{AUTO_GENERATE_CMAKE_CONFIG_FILE_PATH}"):
        convert_config_to_cmake(f"{source_dir}/{DOT_CONFIG_FILE_PATH}", f"{source_dir}/{AUTO_GENERATE_CMAKE_CONFIG_FILE_PATH}")

def parse_config_file(file_path):
    config_dict = {}
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            # Skip comments and empty lines
            if not line or line.startswith('#'):
                continue
            # Handle the CONFIG_OPTION=value format
            if '=' in line:
                key, value = line.split('=', 1)
                config_dict[key.strip()] = value.strip()
            else:
                # Handle the CONFIG_OPTION is not set format
                key = line
                config_dict[key] = None
    return config_dict

def run_loadconfig(source_dir, input_file):
    """Load a config file."""
    kconf = kconfiglib.Kconfig(f"{source_dir}/{K_CONFIG_FILR_PATH}")
    kconf.load_config(input_file)
    kconf.write_config(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")
    if not os.path.exists(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}"):
        os.mkdir(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}")
    kconf.write_autoconf(f"{source_dir}/{AUTO_GENERATE_C_HEADER_FILE_PATH}")
    try_update_cmake_config(f"{source_dir}")


def run_menuconfig(source_dir):
    """Launch the menuconfig UI."""
    kconf = kconfiglib.Kconfig(f"{source_dir}/{K_CONFIG_FILR_PATH}")
    menuconfig.menuconfig(kconf)
    
    if not os.path.exists(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}"):
        os.mkdir(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}")
    kconf.write_autoconf(f"{source_dir}/{AUTO_GENERATE_C_HEADER_FILE_PATH}")
    try_update_cmake_config(f"{source_dir}")


def run_saveconfig(source_dir, output_file='defconfig'):
    """Save minimal config to the target defconfig path."""
    kconf = kconfiglib.Kconfig(f"{source_dir}/{K_CONFIG_FILR_PATH}")
    kconf.load_config(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")
    config = parse_config_file(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")

    # Resolve CONFIG_PROJECT path from .config.
    try:
        
        defconfig_path = f"{source_dir}/project/" + config['CONFIG_PROJECT'].strip('"')
    except Exception as e:
        print(f"Failed to read CONFIG_PROJECT from .config: {e}")
        exit(1)
    
    if not os.path.exists(defconfig_path):
        print(f"Project config directory does not exist: {defconfig_path}")
        exit(1)
    defconfig_file = f"{defconfig_path}/{output_file}"
    # Save current config as minimal defconfig.
    kconf.write_min_config(defconfig_file)
    print(f"Defconfig saved to: {defconfig_file}")

def run_clean(source_dir):
    """Clean build outputs."""
    if not check_tool_installed('cmake'):
        print("cmake not installed. Please install cmake first.")
        exit(1)
    
    if os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        os.system(f"cmake --build {source_dir}/build --target clean")

def run_distclean(source_dir):
    """Remove build directory and generated config outputs."""
    if os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        shutil.rmtree(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}")
    if os.path.exists(f"{source_dir}/{DOT_CONFIG_FILE_PATH}"):
        os.remove(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")
    if os.path.exists(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}"):
        shutil.rmtree(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}")

def run_build(source_dir, build_type=None, jobs=None):
    """Build project."""
    if not check_tool_installed('cmake'):
        print("cmake not installed. Please install cmake first.")
        exit(1)
    
    if jobs == None:
        jobs = os.cpu_count()
    
    rebuild = False

    build_program = get_cmake_cache_value(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}", "CMAKE_MAKE_PROGRAM")
    build_program = os.path.basename(build_program) if build_program else  build_program
    if not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        rebuild = True
    elif build_program == None:
        rebuild = True
    elif 'ninja' == build_program and not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}/build.ninja"):
        rebuild = True
    elif ('make' == build_program or 'gmake' == build_program) and not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}/Makefile"):
        rebuild = True

    generator_args = ''

    if platform.system() == 'Windows' and build_program == None :
        if shutil.which("ninja") is not None:
            generator_args = '-G Ninja'
        elif shutil.which("make") is not None:
            generator_args = '-G "Unix Makefiles"'
        else:
            print("No build tool found. Please install ninja or make.")
            sys.exit(1)
        
    if build_type == None:
        build_type = get_cmake_cache_value(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}", "CMAKE_BUILD_TYPE")
        if build_type == None:
            build_type = "Release"

    if rebuild or get_cmake_cache_value(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}", "CMAKE_BUILD_TYPE") != build_type:
        os.system(
            f'cmake '
            f'-DCMAKE_BUILD_TYPE:STRING={build_type} '
            f'-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE '
            f'--no-warn-unused-cli '
            f'-S{source_dir} '
            f'-B{source_dir}/build '
            f'{generator_args}'
        )

    cmake_ret = os.system(f"cmake --build {source_dir}/{CMAKE_BUILD_DIR_PATH} --target all -j{jobs} --")
    if cmake_ret != 0:
        print(f'Build failed, exit code: {cmake_ret}')
    
def run_target(source_dir, target):
    if not check_tool_installed('cmake'):
        print("cmake not installed. Please install cmake first.")
        exit(1)
    
    if not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        print("Build directory does not exist. Run build first.")
        exit(1)
    os.system(f"cmake --build {source_dir}/{CMAKE_BUILD_DIR_PATH} --target {target}")

def run_flash(source_dir, flash_target='default'):
    run_target(source_dir, f'{flash_target}_flash')

def run_make_img(source_dir, flash_target='default'):
    if not check_tool_installed('cmake'):
        print("cmake not installed. Please install cmake first.")
        exit(1)
    
    if not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        print("Build directory does not exist. Run build first.")
        exit(1)
    
    # Generate build timestamp file.
    check_git_status.generate_timestamp(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}")
    os.system(f"cmake --build {source_dir}/{CMAKE_BUILD_DIR_PATH} --target {flash_target}_make_img")

def run_rttlog(source_dir):
    if not os.path.exists(f"{source_dir}/{CURRENT_IMAGE_DIR_PATH}/"):
        print("Image directory not found. Run make_img first.")
        exit(1)
    # Dispatch to platform-specific log script.
    if platform.system() == 'Windows':
        os.system(f"{source_dir}/{CURRENT_IMAGE_DIR_PATH}/log.bat")
    else :
        os.system(f"{source_dir}/{CURRENT_IMAGE_DIR_PATH}/log.sh")

def run_append_path_env(source_dir, env_path):
    env_PATH_data = []
    env_path = os.path.abspath(env_path)
    env_path_config_json_path = f"{source_dir}/{ENV_PATH_CONFIG_JSON_FILE_PATH}"
    if not os.path.exists(env_path):
        print(f"Path does not exist: {env_path}")
        exit(1)
    if os.path.exists(env_path_config_json_path):
        with open(env_path_config_json_path, 'r') as f:
            env_PATH_data = json.load(f)
    env_PATH_data.append(env_path)
    with open(env_path_config_json_path, 'w') as f:
        json.dump(env_PATH_data, f, indent=4)

def run_package_update(source_dir, pack_name, list=False):
    if not check_tool_installed('cmake'):
        print("cmake not installed. Please install cmake first.")
        exit(1)
    
    if not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        print("Build directory does not exist. Run build first.")
        exit(1)
    
    if list:
        os.system(f"cmake --build {source_dir}/{CMAKE_BUILD_DIR_PATH} --target package_update_show_list")
    else:
        os.system(f"cmake --build {source_dir}/{CMAKE_BUILD_DIR_PATH} --target package_update_{pack_name}")


def run_package_mirror(source_dir, list=False, enable=None, disable=None, update=False):
    # Initialize mirror source file if missing.
    if not os.path.exists(f"{source_dir}/{GIT_MIRROR_SOURCE_JSON_FILE_PATH}"):
        # Copy default mirror source list.
        shutil.copy(f"{source_dir}/{GIT_MIRROR_SOURCE_JSON_FILE_PATH_INIT}", f"{source_dir}/{GIT_MIRROR_SOURCE_JSON_FILE_PATH}")
        print(f"Mirror source config initialized: {source_dir}/{GIT_MIRROR_SOURCE_JSON_FILE_PATH}")
    
    # Load mirror source config.
    git_mirror_source_data = []
    with open(f"{source_dir}/{GIT_MIRROR_SOURCE_JSON_FILE_PATH}", 'r') as f:
        git_mirror_source_data = json.load(f)
    
    for item in git_mirror_source_data:
        is_list = list
        if item['name'] == enable:
            item['enable'] = True
            is_list = True
        
        if item['name'] == disable:
            item['enable'] = False
            is_list = True

        if is_list:
            print(f"{item['name']:<40} [{'ENABLE' if item['enable'] else 'DISABLE':<7}]")
    # Save mirror source config.
    with open(f"{source_dir}/{GIT_MIRROR_SOURCE_JSON_FILE_PATH}", 'w') as f:
        json.dump(git_mirror_source_data, f, indent=4)
    
    if update:
        mirror_source_manager = fetch_git_project.MirrorSourceManager(f"{source_dir}/{GIT_MIRROR_SOURCE_JSON_FILE_PATH}")
        mirror_source_manager.find_best_mirror_source(force_update=True)

def load_env(source_dir):
    if not os.path.exists(f"{source_dir}/{ENV_PATH_CONFIG_JSON_FILE_PATH}"):
        return None
    with open(f"{source_dir}/{ENV_PATH_CONFIG_JSON_FILE_PATH}", 'r') as f:
        env_PATH_data = json.load(f)
    path = os.environ.get('PATH', '')
    for path_item in reversed(env_PATH_data):
        path =  path_item + os.pathsep + path
    os.environ['PATH'] = path


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='FLY build script.')
    #parser.add_argument('source_dir', type=str, help='Directory containing Kconfig.')
    
    subparsers = parser.add_subparsers(dest='command', help='Command to execute.', required=False)

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
    parser_distclean = subparsers.add_parser('distclean', help='Delete build directory and generated configs.')

    # flash
    parser_flash = subparsers.add_parser('flash', help='Flash firmware to target.')
    parser_flash.add_argument('flash_target', type=str, nargs='?', default='default', help='Flash target (default: default).')

    # make_img
    parser_make_img = subparsers.add_parser('make_img', help='Generate image files.')
    parser_make_img.add_argument('flash_target', type=str, nargs='?', default='default', help='Image target (default: default).')

    # Subparser for build
    parser_build = subparsers.add_parser('build', help='Build project.')
    parser_build.add_argument('build_type', type=str, nargs='?', default=None, help='Build type (e.g. Debug/Release/MinSizeRel).')
    parser_build.add_argument('-j', '--jobs', type=int, default=os.cpu_count(), help='Number of parallel jobs.')
    parser.add_argument('-j', '--jobs', type=int, default=os.cpu_count(), help='Number of parallel jobs.')
    
    # Subparser for rttlog
    parser_log = subparsers.add_parser('rttlog', help='Capture RTT logs.')

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

    all_args = sys.argv[1:]
    source_dir = all_args[0]

    args = parser.parse_args(all_args[1:])
    
    os.chdir(source_dir)

    load_env(source_dir)

    if args.command == 'menuconfig':
        run_menuconfig(source_dir)
    elif args.command == 'saveconfig':
        run_saveconfig(source_dir, args.savefile_name)
    elif args.command == 'loadconfig':
        run_loadconfig(source_dir, args.loadfile_name)
    elif args.command == 'clean':
        run_clean(source_dir)
    elif args.command == 'distclean':
        run_distclean(source_dir)
    elif args.command == 'build':
        run_build(source_dir, args.build_type, args.jobs)
    elif args.command == 'flash':
        run_flash(source_dir, args.flash_target)
    elif args.command == 'make_img':
        run_make_img(source_dir, args.flash_target)    
    elif args.command == 'rttlog':
        run_rttlog(source_dir)
    elif args.command == 'add_path_env':
        run_append_path_env(source_dir, args.env_path)
    elif args.command == 'package_update':
        run_package_update(source_dir, args.pack_name, args.list)
    elif args.command == 'package_mirror':
        run_package_mirror(source_dir, args.list, args.enable, args.disable, args.update)
    else:
        run_build(source_dir, jobs = args.jobs)
