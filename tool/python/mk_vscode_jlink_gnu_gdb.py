import argparse
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

def launch_json_append(launch_json, config_name, chip_name, gdb_path, elf_path, speed):
    if platform.system() == 'Linux':
        launch_json['configurations'].append({
            "name": f'{config_name}-{chip_name} (JLink)',
            "type": "gnu-debugger",
            "request": "launch",
            "program": elf_path,
            "client": gdb_path,
            "server": "JLinkGDBServer",
            "serverArgs": [
                "-device", chip_name,
                "-if", "SWD",
                "-speed", f"{speed}",
                "-noLocalhostOnly",
                "-port", "2331", 
                "-SWOPort", "2332",
                "-TelnetPort", "2333",
                "-RTTTelnetPort", "19021"
            ],
            "serverHost": "localhost",
            "serverPort": 2331,
            "customVariables": [
            ],
            "autoRun": False,
            "debugOutput": False,
        })
        launch_json['configurations'].append({
            "name": f'{config_name}-{chip_name} attach (JLink)',
            "type": "gnu-debugger",
            "request": "launch",
            "program": elf_path,
            "client": gdb_path,
            "server": "JLinkGDBServer",
            "serverArgs": [
                "-device", chip_name,
                "-if", "SWD",
                "-speed", f"{speed}",
                "-noLocalhostOnly",
                "-port", "2331", 
                "-SWOPort", "2332",
                "-TelnetPort", "2333",
                "-RTTTelnetPort", "19021"
            ],
            "serverHost": "localhost",
            "serverPort": 2331,
            "customVariables": [
            ],
            "autoRun": False,
            "debugOutput": False,
            "gdbCommands":[
                "-gdb-set target-async on", 
                "-enable-pretty-printing",
                "-target-select remote localhost:2331",
                f"-file-exec-and-symbols \"{elf_path}\"",
                "-interpreter-exec console \"monitor halt\"",
            ]
        })
    else:
        print('not support')
        exit(1)

def launch_json_save(launch_json, top_path):
    launch_json_path = f'{top_path}/.vscode/launch.json'
    with open(launch_json_path, 'w') as file:
        json.dump(launch_json, file, indent=4)



if __name__ == '__main__':
    # ['/home/simon/project/fly/tool/python/mk_vscode_jlink_gnu_gdb.py', '--chip-name', 'STM32H750VB', '--gdb-path', '/usr/bin/gdb-multiarch', '--elf-path', '/home/simon/project/fly/build/project/stm32h7xx/stm32h750vb-demo/stm32h750vb_demo_app', '--speed', '10000']
    parser = argparse.ArgumentParser()
    parser.add_argument('--config-name', type=str, required=True, help='config name')
    parser.add_argument('--top-path', type=str, required=True, help='top path')
    parser.add_argument('--chip-name', type=str, required=True, help='chip name')
    parser.add_argument('--gdb-path', type=str, required=True, help='gdb path')
    parser.add_argument('--elf-path', type=str, required=True, help='elf path')
    parser.add_argument('--speed', type=int, required=True, help='jlink speed')

    args = parser.parse_args()

    launch_json = launch_json_read_or_init(args.top_path, args.config_name)
    launch_json_append(launch_json, args.config_name, args.chip_name, args.gdb_path, args.elf_path, args.speed)
    launch_json_save(launch_json, args.top_path)




