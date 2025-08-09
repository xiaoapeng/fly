import shutil
import argparse
import os
import sys
import kconfiglib
import menuconfig
import platform
import json
import check_git_status

K_CONFIG_FILR_PATH = 'Kconfig'
DOT_CONFIG_FILE_PATH = '.config'
CMAKE_BUILD_DIR_PATH = 'build'
AUTO_GENERATE_DIR_PATH = 'auto-generate'
AUTO_GENERATE_CMAKE_CONFIG_FILE_PATH = f'{AUTO_GENERATE_DIR_PATH}/.config.cmake'
AUTO_GENERATE_C_HEADER_FILE_PATH = f'{AUTO_GENERATE_DIR_PATH}/autoconf.h'
CURRENT_IMAGE_DIR_PATH = 'image/CURRENT'
ENV_PATH_CONFIG_JSON_FILE_PATH = '.PATH.evn.json'

def check_tool_installed(tool_name):
    # Check if the tool is in PATH
    tool_path = shutil.which(tool_name)
    if tool_path:
        return True
    else:
        return False

def get_file_modification_time(file_path):
    # 获取文件的最后修改时间戳
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
    
    print(f"Conversion complete. CMake file written to {cmake_file}")

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
    """加载配置文件"""
    kconf = kconfiglib.Kconfig(f"{source_dir}/{K_CONFIG_FILR_PATH}")
    kconf.load_config(input_file)
    kconf.write_config(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")
    if not os.path.exists(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}"):
        os.mkdir(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}")
    kconf.write_autoconf(f"{source_dir}/{AUTO_GENERATE_C_HEADER_FILE_PATH}")
    try_update_cmake_config(f"{source_dir}")


def run_menuconfig(source_dir):
    """启动 menuconfig 界面"""
    kconf = kconfiglib.Kconfig(f"{source_dir}/{K_CONFIG_FILR_PATH}")
    menuconfig.menuconfig(kconf)
    
    if not os.path.exists(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}"):
        os.mkdir(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}")
    kconf.write_autoconf(f"{source_dir}/{AUTO_GENERATE_C_HEADER_FILE_PATH}")
    try_update_cmake_config(f"{source_dir}")


def run_saveconfig(source_dir, output_file='defconfig'):
    """保存 defconfig 文件到指定路径"""
    kconf = kconfiglib.Kconfig(f"{source_dir}/{K_CONFIG_FILR_PATH}")
    kconf.load_config(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")
    config = parse_config_file(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")

    # 查找 CONFIG_DEFCONFIG_PATH 的值
    try:
        
        defconfig_path = f"{source_dir}/project/" + config['CONFIG_PROJECT'].strip('"')
    except Exception as e:
        print(f"Error while accessing : {e}. Using default path: {defconfig_path}")
        exit(1)
    
    if not os.path.exists(defconfig_path):
        print(f"Defconfig file does not exist at {defconfig_path}")
        exit(1)
    defconfig_file = f"{defconfig_path}/{output_file}"
    # 将当前配置保存到 defconfig 文件
    kconf.write_min_config(defconfig_file)
    print(f"Defconfig saved to {defconfig_file}")

def run_clean(source_dir):
    """清理编译目录"""
    if not check_tool_installed('cmake'):
        print("cmake not installed!!")
        exit(1)
    
    if os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        os.system(f"cmake --build {source_dir}/build --target clean")

def run_distclean(source_dir):
    """ 删除build 和 dl """
    if os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        shutil.rmtree(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}")
    if os.path.exists(f"{source_dir}/{DOT_CONFIG_FILE_PATH}"):
        os.remove(f"{source_dir}/{DOT_CONFIG_FILE_PATH}")
    if os.path.exists(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}"):
        shutil.rmtree(f"{source_dir}/{AUTO_GENERATE_DIR_PATH}")

def run_build(source_dir, build_type=None, jobs=None):
    """编译"""
    if not check_tool_installed('cmake'):
        print("cmake not installed!!")
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
        print(f'Build failed:{cmake_ret}')
    
def run_target(source_dir, target):
    if not check_tool_installed('cmake'):
        print("cmake not installed!!")
        exit(1)
    
    if not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        print("build not exist!!")
        exit(1)
    os.system(f"cmake --build {source_dir}/{CMAKE_BUILD_DIR_PATH} --target {target}")

def run_flash(source_dir, flash_target='default'):
    run_target(source_dir, f'{flash_target}_flash')

def run_make_img(source_dir, flash_target='default'):
    if not check_tool_installed('cmake'):
        print("cmake not installed!!")
        exit(1)
    
    if not os.path.exists(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}"):
        print("build not exist!!")
        exit(1)
    
    # 生成时间戳文件,
    check_git_status.generate_timestamp(f"{source_dir}/{CMAKE_BUILD_DIR_PATH}")
    os.system(f"cmake --build {source_dir}/{CMAKE_BUILD_DIR_PATH} --target {flash_target}_make_img")

def run_rttlog(source_dir):
    if not os.path.exists(f"{source_dir}/{CURRENT_IMAGE_DIR_PATH}/"):
        print("image not exist!!")
        exit(1)
    # 判断操作系统
    if platform.system() == 'Windows':
        os.system(f"{source_dir}/{CURRENT_IMAGE_DIR_PATH}/log.bat")
    else :
        os.system(f"{source_dir}/{CURRENT_IMAGE_DIR_PATH}/log.sh")

def run_append_path_env(source_dir, env_path):
    env_PATH_data = []
    env_path = os.path.abspath(env_path)
    env_path_config_json_path = f"{source_dir}/{ENV_PATH_CONFIG_JSON_FILE_PATH}"
    if not os.path.exists(env_path):
        print(f"{env_path} not exist!!")
        exit(1)
    if os.path.exists(env_path_config_json_path):
        with open(env_path_config_json_path, 'r') as f:
            env_PATH_data = json.load(f)
    env_PATH_data.append(env_path)
    with open(env_path_config_json_path, 'w') as f:
        json.dump(env_PATH_data, f, indent=4)

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
    parser = argparse.ArgumentParser(description='Compile script.')
    #parser.add_argument('source_dir', type=str, help='Directory containing the Kconfig file.')
    
    subparsers = parser.add_subparsers(dest='command', help='Command to execute.', required=False)

    # Subparser for menuconfig
    parser_menuconfig = subparsers.add_parser('menuconfig', help='Start menuconfig interface.')

    # Subparser for saveconfig
    parser_saveconfig = subparsers.add_parser('saveconfig', help='Run saveconfig')
    parser_saveconfig.add_argument('savefile_name', type=str,nargs='?', default="defconfig", help='Output file for saveconfig command.')

    # Subparser for loadconfig
    parser_loadconfig = subparsers.add_parser('loadconfig', help='Run loadconfig')
    parser_loadconfig.add_argument('loadfile_name', type=str, help='Input file for loadconfig command.')

    # clean
    parser_clean = subparsers.add_parser('clean', help='Clean the build directory.')

    # distclean 
    parser_distclean = subparsers.add_parser('distclean', help='Delete the build directory.')

    # flash
    parser_flash = subparsers.add_parser('flash', help='Flash the project.')
    parser_flash.add_argument('flash_target', type=str, nargs='?', default='default', help='Flash type (e.g., default,...).')

    # make_img
    parser_make_img = subparsers.add_parser('make_img', help='Make image.')
    parser_make_img.add_argument('flash_target', type=str, nargs='?', default='default', help='Flash type (e.g., default,...).')

    # Subparser for build
    parser_build = subparsers.add_parser('build', help='Build the project.')
    parser_build.add_argument('build_type', type=str, nargs='?', default=None, help='Build type (e.g., Debug, Release, MinSizeRel).')
    parser_build.add_argument('-j', '--jobs', type=int, default=os.cpu_count(), help='Number of jobs to run simultaneously.')
    parser.add_argument('-j', '--jobs', type=int, default=os.cpu_count(), help='Number of jobs to run simultaneously.')
    
    # Subparser for rttlog
    parser_log = subparsers.add_parser('rttlog', help='Capture log.')

    # Add the PATH environment variable
    parser_add_path = subparsers.add_parser('add_path_env', help='Add the PATH environment variable.')
    parser_add_path.add_argument('env_path', type=str, help='Environment variable path')

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
    else:
        run_build(source_dir, jobs = args.jobs)