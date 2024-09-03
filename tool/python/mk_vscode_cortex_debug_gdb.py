import argparse
import os
import platform
import json

def launch_json_read_or_init(top_path, config_name):
    # launch_json_path = f'{top_path}/.vscode/launch.json'
    launch_json = {
        "version": "0.2.0",
        "configurations": [
        ]
    }
    # if not os.path.exists(launch_json_path):
    #     return launch_json

    # try:
    #     with open(launch_json_path, 'r') as file:
    #         launch_json = json.load(file)
    # except:
    #     return launch_json
    
    # for config in launch_json['configurations']:
    #     if config['name'] == config_name:
    #         # 删除这一项
    #         launch_json['configurations'].remove(config)

    return launch_json

def launch_json_append(launch_json, top_path, config_name, chip_name, gdb_path, objdump_path, 
    elf_path_list, server_type, config_file_list, rtt_enabled):

    for elf_path in elf_path_list:
        file_name = os.path.basename(elf_path)
        launch_json['configurations'].append({
            "name": f'{config_name}-{file_name} (attach)',
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
            "configFiles": config_file_list
        })
        launch_json['configurations'].append({
            "name": f'{config_name}-{file_name}',
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
            "configFiles": config_file_list
        })

def launch_json_save(launch_json, top_path):
    launch_json_path = f'{top_path}/.vscode/launch.json'
    with open(launch_json_path, 'w') as file:
        json.dump(launch_json, file, indent=4)



if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--server-type', type=str, required=True, help='GDB Server type - supported types are jlink, openocd, pyocd, pe, stlink, stutil, qemu, bmp and external. For \"external\", [please read our Wiki](https://github.com/Marus/cortex-debug/wiki/External-gdb-server-configuration). The executable in your PATH is used by default, to override this use serverpath.')
    parser.add_argument('--config-name', type=str, required=True, help='config name')
    parser.add_argument('--top-path', type=str, required=True, help='top path')
    parser.add_argument('--chip-name', type=str, required=True, help='chip name')
    parser.add_argument('--gdb-path', type=str, required=True, help='gdb path')
    parser.add_argument('--objdump-path', type=str, required=True, help='objdump path')
    parser.add_argument('--elf-path-list', nargs='+', type=str, required=True, help='elf path')
    #是否启用rtt
    parser.add_argument('--rtt-enabled', action='store_true', default=False, help='Enable RTT logging')
    parser.add_argument('--config-file-list', nargs='+', type=str, required=False, help='OpenOCD/PE GDB Server configuration file(s) to use when debugging (OpenOCD -f option)')
    
    args = parser.parse_args()

    launch_json = launch_json_read_or_init(args.top_path, args.config_name)
    launch_json_append(launch_json, args.top_path, args.config_name, args.chip_name, 
                        args.gdb_path, args.objdump_path, args.elf_path_list, args.server_type, args.config_file_list, args.rtt_enabled)
    launch_json_save(launch_json, args.top_path)




