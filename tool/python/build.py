import argparse
import os
import sys
import kconfiglib
import menuconfig

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

def run_menuconfig(source_dir):
    """启动 menuconfig 界面"""
    kconf = kconfiglib.Kconfig(f"{source_dir}/Kconfig")
    menuconfig.menuconfig(kconf)
    exit(0)

def run_savedefconfig(source_dir, output_file='defconfig'):
    """保存 defconfig 文件到指定路径"""
    kconf = kconfiglib.Kconfig(f"{source_dir}/Kconfig")
    kconf.load_config(f"{source_dir}/.config")
    config = parse_config_file(f"{source_dir}/.config")

    # 查找 CONFIG_DEFCONFIG_PATH 的值
    try:
        if 'CONFIG_COMMON_PROJECT_SET' in config and config['CONFIG_COMMON_PROJECT_SET'] == 'y':
            defconfig_path = f"{source_dir}/project/common/" + config['CONFIG_PROJECT'].strip('"')
        else:
            defconfig_path = f"{source_dir}/project/" + config['CONFIG_ARCH'].strip('"') +'/' + config['CONFIG_PROJECT'].strip('"')
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
    

    