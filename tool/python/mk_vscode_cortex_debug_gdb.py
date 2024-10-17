import argparse
import os
import platform
import json

def launch_json_read_or_init(top_path, config_name):
    launch_json_path = f'{top_path}/.vscode/launch.json'
    launch_json = {
        "version": "0.2.0",
        "configurations": [
        ]
    }
    if not os.path.exists(launch_json_path):
        return launch_json

    try:
        with open(launch_json_path, 'r') as file:
            launch_json = json.load(file)
    except:
        return launch_json

    return launch_json

def launch_json_append(launch_json, top_path, config_name, chip_name, gdb_path, objdump_path, 
    elf_path_list, server_type, openocd_config_file_list, rtt_enabled, interface, ext_config_json):
    

    for elf_path in elf_path_list:
        file_name = os.path.basename(elf_path)
        attach_all_config_name = f'{config_name}-{file_name} (attach)'
        all_config_name = f'{config_name}-{file_name}'

        # 如果 launch_json 中已经有 attach_config_name了就将那一项删除掉
        for i in range(len(launch_json['configurations']) - 1, -1, -1):
            if launch_json['configurations'][i]['name'] == attach_all_config_name or launch_json['configurations'][i]['name'] == all_config_name:
                del launch_json['configurations'][i]
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


        launch_json['configurations'].append(attach_all_config)
        launch_json['configurations'].append(launch_all_config)
        

def launch_json_save(launch_json, top_path):
    vscode_dir = f'{top_path}/.vscode'
    if not os.path.exists(vscode_dir):
        os.mkdir(vscode_dir)

    launch_json_path = f'{top_path}/.vscode/launch.json'
    with open(launch_json_path, 'w') as file:
        json.dump(launch_json, file, indent=4)



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

    launch_json = launch_json_read_or_init(args.top_path, args.config_name)
    launch_json_append(launch_json, args.top_path, args.config_name, args.chip_name, 
                        args.gdb_path, args.objdump_path, args.elf_path_list, args.server_type, 
                        args.openocd_config_file_list, args.rtt_enabled, args.interface, args.ext_config_json)
    
    launch_json_save(launch_json, args.top_path)




