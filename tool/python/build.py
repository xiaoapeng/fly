import shutil
import argparse
import os
import sys
import kconfiglib
import menuconfig

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

def get_build_type(build_dir):
    with open(f"{build_dir}/CMakeCache.txt", "r") as cache_file:
        for line in cache_file:
            if line.startswith("CMAKE_BUILD_TYPE:"):
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
    if get_file_modification_time(f"{source_dir}/.config") > get_file_modification_time(f"{source_dir}/auto-generate/.config.cmake"):
        convert_config_to_cmake(f"{source_dir}/.config", f"{source_dir}/auto-generate/.config.cmake")

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
    kconf = kconfiglib.Kconfig(f"{source_dir}/Kconfig")
    kconf.load_config(input_file)
    kconf.write_config(f"{source_dir}/.config")
    if not os.path.exists(f"{source_dir}/auto-generate"):
        os.mkdir(f"{source_dir}/auto-generate")
    kconf.write_autoconf(f"{source_dir}/auto-generate/autoconf.h")
    try_update_cmake_config(f"{source_dir}")


def run_menuconfig(source_dir):
    """启动 menuconfig 界面"""
    kconf = kconfiglib.Kconfig(f"{source_dir}/Kconfig")
    menuconfig.menuconfig(kconf)
    
    if not os.path.exists(f"{source_dir}/auto-generate"):
        os.mkdir(f"{source_dir}/auto-generate")
    kconf.write_autoconf(f"{source_dir}/auto-generate/autoconf.h")
    try_update_cmake_config(f"{source_dir}")


def run_savedefconfig(source_dir, output_file='defconfig'):
    """保存 defconfig 文件到指定路径"""
    kconf = kconfiglib.Kconfig(f"{source_dir}/Kconfig")
    kconf.load_config(f"{source_dir}/.config")
    config = parse_config_file(f"{source_dir}/.config")

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
    
    if os.path.exists(f"{source_dir}/build"):
        os.system(f"cmake --build {source_dir}/build --target clean")

def run_distclean(source_dir):
    """ 删除build 和 dl """
    if os.path.exists(f"{source_dir}/build"):
        shutil.rmtree(f"{source_dir}/build")
    if os.path.exists(f"{source_dir}/dl"):
        shutil.rmtree(f"{source_dir}/dl")
    

def run_build(source_dir, build_type='Release'):
    """编译"""
    if not check_tool_installed('cmake'):
        print("cmake not installed!!")
        exit(1)
    
    rebuild = False

    if not os.path.exists(f"{source_dir}/build"):
        os.mkdir(f"{source_dir}/build")
        rebuild = True

    if rebuild or get_build_type(f"{source_dir}/build") != build_type:
        """ 如果发现ninja优先使用 """
        if check_tool_installed('ninja'):
            os.system(f"cmake -DCMAKE_BUILD_TYPE:STRING={build_type} -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S{source_dir} -B{source_dir}/build -GNinja")
        else:
            os.system(f"cmake -DCMAKE_BUILD_TYPE:STRING={build_type} -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S{source_dir} -B{source_dir}/build")
    
    os.system(f"cmake --build {source_dir}/build --target all --")
    


def run_flash(source_dir, flash_target='default'):
    if not check_tool_installed('cmake'):
        print("cmake not installed!!")
        exit(1)
    
    if not os.path.exists(f"{source_dir}/build"):
        print("build not exist!!")
        exit(1)
    os.system(f"cmake --build {source_dir}/build --target {flash_target}_flash")



def run_make_jlink_img(source_dir, flash_target='default'):
    if not check_tool_installed('cmake'):
        print("cmake not installed!!")
        exit(1)
    
    if not os.path.exists(f"{source_dir}/build"):
        print("build not exist!!")
        exit(1)
    os.system(f"cmake --build {source_dir}/build --target {flash_target}_make_jlink_img")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Compile script.')
    #parser.add_argument('source_dir', type=str, help='Directory containing the Kconfig file.')
    

    subparsers = parser.add_subparsers(dest='command', help='Command to execute.', required=False)

    # Subparser for menuconfig
    parser_menuconfig = subparsers.add_parser('menuconfig', help='Start menuconfig interface.')

    # Subparser for savedefconfig
    parser_savedefconfig = subparsers.add_parser('savedefconfig', help='Run savedefconfig')
    parser_savedefconfig.add_argument('savefile_name', type=str,nargs='?', default="defconfig", help='Output file for savedefconfig command.')

    # Subparser for loadconfig
    parser_loadconfig = subparsers.add_parser('loadconfig', help='Run loadconfig')
    parser_loadconfig.add_argument('loadfile_name', type=str, help='Input file for loadconfig command.')

    # clean
    parser_clean = subparsers.add_parser('clean', help='Clean the build directory.')

    # distclean 
    parser_distclean = subparsers.add_parser('distclean', help='Clean the build directory.')

    # flash
    parser_flash = subparsers.add_parser('flash', help='Flash the project.')
    parser_flash.add_argument('flash_target', type=str, nargs='?', default='default', help='Flash type (e.g., default,...).')

    # make_jlink_img
    parser_make_jlink_img = subparsers.add_parser('make_jlink_img', help='Make jlink img.')
    parser_make_jlink_img.add_argument('flash_target', type=str, nargs='?', default='default', help='Flash type (e.g., default,...).')

    # Subparser for build
    parser_build = subparsers.add_parser('build', help='Build the project.')
    parser_build.add_argument('build_type', type=str, nargs='?', default='Release', help='Build type (e.g., Debug, Release, MinSizeRel, RelWithDebInfo).')


    all_args = sys.argv[1:]
    source_dir = all_args[0]

    args = parser.parse_args(all_args[1:])
    
    os.chdir(source_dir)
    if args.command == 'menuconfig':
        run_menuconfig(source_dir)
    elif args.command == 'savedefconfig':
        run_savedefconfig(source_dir, args.savefile_name)
    elif args.command == 'loadconfig':
        run_loadconfig(source_dir, args.loadfile_name)
    elif args.command == 'clean':
        run_clean(source_dir)
    elif args.command == 'distclean':
        run_distclean(source_dir)
    elif args.command == 'build':
        run_build(source_dir, args.build_type)
    elif args.command == 'flash':
        run_flash(source_dir, args.flash_target)
    elif args.command == 'make_jlink_img':
        run_make_jlink_img(source_dir, args.flash_target)
    elif args.command == None:
        run_build(source_dir)