import argparse
import json
import os
from pathlib import Path
import sys


def launch_json_read_or_init(launch_path):
    try:
        launch_json = json.loads(launch_path.read_text(encoding='utf-8'))
    except FileNotFoundError:
        return {"version": "0.2.0", "configurations": []}

    if not isinstance(launch_json, dict) or not isinstance(launch_json.get('configurations'), list):
        raise ValueError('Expected a JSON object with a configurations array')
    return launch_json


def update_configuration(configurations, new_config):
    for index, configuration in enumerate(configurations):
        if isinstance(configuration, dict) and configuration.get('name') == new_config['name']:
            configurations[index] = new_config
            return
    configurations.append(new_config)


def launch_json_append(launch_json, config_name, chip_name, gdb_path, objdump_path,
    elf_path_list, server_type, openocd_config_file_list, rtt_enabled, interface, ext_config_json):
    

    for elf_path in elf_path_list:
        file_name = os.path.basename(elf_path)
        attach_all_config_name = f'{config_name}-{file_name} (attach)'
        all_config_name = f'{config_name}-{file_name}'

        attach_all_config = {
            "name": attach_all_config_name,
            "cwd": "${workspaceFolder}",
            "type": "cortex-debug",
            "executable": elf_path,
            "request": "attach",
            "servertype": server_type,
            "device": chip_name,
            "objdumpPath": objdump_path,
            "gdbPath": gdb_path,
            "rttConfig":{
                "enabled": rtt_enabled,
                "address": "auto",
                "decoders": [
                    {
                        "label": "RTT OUT",
                        "port": 0,
                        "type": "console"
                    }
                ]
            },
            "configFiles": openocd_config_file_list
        }
        
        launch_all_config = {
            "name": all_config_name,
            "cwd": "${workspaceFolder}",
            "type": "cortex-debug",
            "executable": elf_path,
            "request": "launch",
            "breakAfterReset":True,
            "servertype": server_type,
            "device": chip_name,
            "objdumpPath": objdump_path,
            "gdbPath": gdb_path,
            "rttConfig":{
                "enabled": rtt_enabled,
                "address": "auto",
                "decoders": [
                    {
                        "label": "RTT OUT",
                        "port": 0,
                        "type": "console"
                    }
                ]
            },
            "configFiles": openocd_config_file_list
        }
        
        if server_type == 'jlink':
            attach_all_config['interface'] = interface
            launch_all_config['interface'] = interface
        
        if ext_config_json and os.path.exists(ext_config_json):
            with open(ext_config_json, 'r') as file:
                ext_config_json_obj = json.load(file)
                ext_launch_config = ext_config_json_obj.get('ext-launch-config', None)
                ext_attach_config = ext_config_json_obj.get('ext-attach-config', None)
                # 合并扩展的配置项
                if ext_launch_config:
                    launch_all_config.update(ext_launch_config)
                if ext_attach_config:
                    attach_all_config.update(ext_attach_config)


        update_configuration(launch_json['configurations'], attach_all_config)
        update_configuration(launch_json['configurations'], launch_all_config)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--server-type', type=str, required=True, help='GDB Server type - supported types are jlink, openocd, pyocd, pe, stlink, stutil, qemu, bmp and external. For \"external\", [please read our Wiki](https://github.com/Marus/cortex-debug/wiki/External-gdb-server-configuration). The executable in your PATH is used by default, to override this use serverpath.')
    parser.add_argument('--interface',default='swd', type=str, required=False, help='interface type')
    parser.add_argument('--config-name', type=str, required=True, help='config name')
    parser.add_argument('--top-path', type=str, required=True, help='top path')
    parser.add_argument('--chip-name', type=str, required=True, help='chip name')
    parser.add_argument('--gdb-path', type=str, required=True, help='gdb path')
    parser.add_argument('--objdump-path', type=str, required=True, help='objdump path')
    parser.add_argument('--elf-path-list', nargs='+', type=str, required=True, help='elf path')
    #是否启用rtt
    parser.add_argument('--rtt-enabled', action='store_true', default=False, help='Enable RTT logging')
    parser.add_argument('--openocd-config-file-list', nargs='+', type=str, required=False, help='OpenOCD/PE GDB Server configuration file(s) to use when debugging (OpenOCD -f option)')
    parser.add_argument('--ext-config-json', type=str,  help='Path to external configuration JSON file')
    
    args = parser.parse_args()

    launch_path = Path(args.top_path) / '.vscode' / 'launch.json'
    try:
        launch_json = launch_json_read_or_init(launch_path)
    except ValueError as error:
        print(f'Keeping existing VS Code configuration at {launch_path}: {error}', file=sys.stderr)
        sys.exit(0)

    previous_content = json.dumps(launch_json, indent=4)
    launch_json_append(launch_json, args.config_name, args.chip_name, args.gdb_path, args.objdump_path,
                       args.elf_path_list, args.server_type, args.openocd_config_file_list,
                       args.rtt_enabled, args.interface, args.ext_config_json)

    content = json.dumps(launch_json, indent=4)
    if content != previous_content or not launch_path.is_file():
        launch_path.parent.mkdir(parents=True, exist_ok=True)
        launch_path.write_text(content + '\n', encoding='utf-8')




