#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import shutil
import os
import subprocess
import argparse
from datetime import datetime
from collections import namedtuple
from pathlib import Path
from image_directory_link import publish_directory
from image_packaging_state import package_is_current, package_key, remember_package
from mk_mcu_firmware_factory_data import append_factory_data_to_bin
import sys


def git_get_hash():
    try:
        # 使用 subprocess 运行 git rev-parse HEAD 命令
        result = subprocess.run(['git', 'rev-parse', '--verify', 'HEAD'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
        # Read the full commit for invalidation and shorten it only for display.
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return None

def git_is_workspace_clean():
    try:
        # 使用 subprocess 运行 git status --porcelain 命令
        result = subprocess.run(['git', 'status', '--porcelain'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
        # 如果 stdout 为空，则工作区是干净的
        return result.stdout.strip() == ''
    except subprocess.CalledProcessError as e:
        # 处理错误并输出错误信息
        print(f"Error occurred: {e.stderr.strip()}")
        return False

def _parse_version(version_string):
    # 将字符串转换为小写并移除首尾空格
    version_string = version_string.strip().lower()

    # 假设版本号格式为 v0.0.1，v0.0.1，r0.0.1 或 r0.0.1
    if version_string.startswith('v'):
        version_string = version_string[1:]
    elif version_string.startswith('r'):
        version_string = version_string[1:]

    # 分割版本号的三个部分
    version = version_string.split('.')
    major = 0
    minor = 0
    patch = 0

    # 解析版本号的三个部分，并确保转换为整数
    major = int(version[0])
    minor = int(version[1])
    if len(version) == 3:
        patch = int(version[2])
    

    # 使用小端序将版本号转换为uint32_t类型的变量
    version_number = patch << 16 | minor << 8 | major

    return version_number,"v%d.%d.%d" % (major, minor, patch)

# ./mk_openocd_img.py --firmware-name TEST --chip-name GD32F103C8 --win-openocd-path ../win/openocd/  --output-dir ../../image/  --speed 4000 BOOT:V0.0.1:0x8000000:../../build/gd32_t527_mcu_demo_app.bin APP:V0.0.1:0x8000000:../../build/gd32_t527_mcu_demo_app.bin
def parse_args():
    ArgsInfo = namedtuple('ArgsInfo', ['firmware_name', 'output_dir', 'firmware_parts', 'config_file_list', "symlink_name", "rtt_setting"])
    FirmwareInfo = namedtuple('FirmwareInfo', ['bin_name', 'bin_version_str', 'bin_version_uint32', 'start_addr', 'file_path'])
    
    parser = argparse.ArgumentParser(description='Generate J-Link compatible firmware image.')

    # Required arguments
    parser.add_argument('--firmware-name', required=True, help='Name of the firmware.')
    parser.add_argument('--output-dir', required=True, help='Directory to output the generated image.')
    parser.add_argument('--symlink-name', default='CURRENT' ,required=False, help='Chip models supported by J-Link.')
    parser.add_argument('--config-file-list', nargs='+', type=str, required=True, help='OpenOCD/PE GDB Server configuration file(s) to use when debugging (OpenOCD -f option)')
    parser.add_argument('--rtt-setting', nargs=2, type=lambda x: str(x), default=["0x20000000", "0x8000"], help='RTT setting values in hexadecimal')

    # Positional arguments for firmware parts
    parser.add_argument('--firmware-parts', nargs='+', type=str, required=True, help='Memory address and firmware file pairs in the format bin_name:bin_version:address:file_path.')

    parser.add_argument("--cache-file", help="Reuse the current package when the inputs match.")
    args = parser.parse_args()

    args_info = ArgsInfo(args.firmware_name, args.output_dir, [], args.config_file_list, args.symlink_name, args.rtt_setting)
    for firmware_part in args.firmware_parts:
        try:
            bin_name, bin_version, start_addr,file_path = firmware_part.split(':', 3)
        except ValueError:
            print('Invalid firmware part: %s' % firmware_part)
            continue
        if not os.path.isfile(file_path):
            print('Invalid firmware part: %s' % firmware_part)
            continue
            
        bin_version_uint32,bin_version_str = _parse_version(bin_version)
        args_info.firmware_parts.append(FirmwareInfo(bin_name, bin_version_str, bin_version_uint32, start_addr, file_path))
    return args_info, args.cache_file


if __name__ == '__main__':
    args_info, cache_file = parse_args()
    revision = git_get_hash()
    dirty = not git_is_workspace_clean()
    git_field = revision[:10] if revision else 'uncommitted'
    if dirty:
        git_field = 'dirty_' + git_field
    firmware_link_path = Path(args_info.output_dir) / args_info.symlink_name
    key = None
    if cache_file:
        sources = [Path(__file__)]
        sources.extend(Path(part.file_path) for part in args_info.firmware_parts)
        sources.extend(Path(path) for path in args_info.config_file_list)
        key = package_key(args_info, [revision, dirty], sources)
        if package_is_current(cache_file, key, firmware_link_path):
            print('OPENOCD package is up to date')
            sys.exit(0)
    time_field = datetime.now().strftime("%Y%m%d_%H%M%S")
    make_firmware_name = args_info.firmware_name + '_openocd'
    for firmware_part in args_info.firmware_parts:
        make_firmware_name = make_firmware_name + '_' + firmware_part.bin_name + firmware_part.bin_version_str
    make_firmware_name = make_firmware_name + '_' + time_field + '_' + git_field
    print('make_firmware_name:', make_firmware_name)
    make_firmware_path = args_info.output_dir + '/' + make_firmware_name
    config_option = ''
    # 创建固件文件夹（如果已存在则清空后重建）
    if os.path.exists(make_firmware_path):
        shutil.rmtree(make_firmware_path)
    os.makedirs(make_firmware_path)
    os.makedirs(f'{make_firmware_path}/openocd_config')


    # 拷贝自定义的配置文件
    for config_file_path in args_info.config_file_list:
        if os.path.isfile(config_file_path):
            openocd_config_path = f'openocd_config/{os.path.basename(config_file_path)}'
            shutil.copyfile(config_file_path, f'{make_firmware_path}/{openocd_config_path}')
            config_option = f'{config_option} -f "{openocd_config_path}"'
        else:
            config_option = f'{config_option} -f "{config_file_path}"'
    
    config_option = f'{config_option} -c "init"'

    burn_script_option = f'{config_option} -c "halt"'
    # 拷贝bin文件
    for firmware_part in args_info.firmware_parts:
        burn_script_option = f'{burn_script_option} -c "flash write_image erase {firmware_part.bin_name}.bin {firmware_part.start_addr}"'
        burn_script_option = f'{burn_script_option} -c "verify_image {firmware_part.bin_name}.bin {firmware_part.start_addr}"'
        dst_bin_path = make_firmware_path + '/' + firmware_part.bin_name + '.bin'
        shutil.copyfile(firmware_part.file_path, dst_bin_path)
        # bin文件添加尾部信息
        append_factory_data_to_bin(dst_bin_path, firmware_part.bin_version_uint32, 
            firmware_part.bin_name, args_info.firmware_name, 'git@' + git_field + ' ' + 'time@' + time_field)

    burn_script_option = f'{burn_script_option} -c reset -c resume -c exit'
    
    log_script_option = f'{config_option} -c "rtt server start 19021 0" -c "rtt setup {args_info.rtt_setting[0]} {args_info.rtt_setting[1]} \\"SEGGER RTT\\""  -c "rtt start" '

    

    # # 生成windows烧写脚本
    with open(make_firmware_path + '/' + 'burn.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('\n')
        f.write('set "nowait_option="\n')
        f.write('if "%1" == "--nowait" (\n')
        f.write('    set "nowait_option=--nowait"\n')
        f.write(')\n')
        f.write('\n')
        f.write('cd /d "%script_dir%" || exit /b 1\n')
        f.write(f'openocd {burn_script_option}\n')
        f.write('if errorlevel 1 exit /b %errorlevel%\n')
        f.write('\n')
        f.write('if not defined nowait_option (\n')
        f.write('    echo "Execution completed, will automatically exit in 10 seconds."\n')
        f.write('    timeout /t 10 /nobreak > nul\n')
        f.write(')\n')
        f.write('\n')
        f.write('exit 0\n')
    # # 生成windows日志查看脚本
    '''
    @echo off

    set CUR_SH_DIR=%~dp0
    cd /d %CUR_SH_DIR%

    powershell -ExecutionPolicy Bypass -File "log.ps1"
    '''
    with open(make_firmware_path + '/' + 'log.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('cd /d "%script_dir%" || exit /b 1\n\n')
        f.write('powershell -ExecutionPolicy Bypass -File "log.ps1"\n')

    with open(make_firmware_path + '/' + 'log.ps1', 'w') as f:
        f.write(f'''
Write-Output "Use openocd to obtain RTT logs..."
$openocdArgs = @"
{log_script_option}
"@
# Start OpenOCD and store its PID
Start-Process -FilePath "openocd" -ArgumentList $openocdArgs -NoNewWindow -PassThru

Start-Sleep -Seconds 1

$tcpClient = New-Object System.Net.Sockets.TcpClient
$tcpClient.Connect("localhost", 19021)
$stream = $tcpClient.GetStream()
$reader = New-Object System.IO.StreamReader($stream)
$writer = New-Object System.IO.StreamWriter("log.txt", $true)

Write-Output "Waiting for RTT log..."
while ($tcpClient.Connected) {{
    $line = $reader.ReadLine()
    
    if ($line -ne $null) {{
        $date_str = (Get-Date).ToString("[yyyy-MM-dd HH:mm:ss.fff]")
        $logLine = "$date_str $line"
        Write-Output $logLine
        $writer.WriteLine($logLine)
        $writer.Flush()
    }}
}}

# Close resources
$reader.Close()
$writer.Close()
$tcpClient.Close()

# 
''')
    os.chmod(make_firmware_path + '/' + 'log.ps1', 0o755)

    # 生成wsl烧写脚本
    with open(make_firmware_path + '/' + 'burn.sh', 'w') as f:
        f.write('#!/bin/sh\n')
        f.write('CDPATH= cd -P -- "$(dirname -- "$0")" || exit 1\n')
        f.write('echo "Download firmware using openocd.."\n')
        f.write(f'exec openocd {burn_script_option}\n')
    os.chmod(make_firmware_path + '/' + 'burn.sh', 0o755)

    # 生成wsl日志查看脚本
    with open(make_firmware_path + '/' + 'log.sh', 'w') as f:
        f.write('#!/bin/sh\n')
        f.write('CDPATH= cd -P -- "$(dirname -- "$0")" || exit 1\n')
        f.write('echo "Use openocd to obtain RTT logs.."\n')

        f.write(f'openocd {log_script_option} &\n')

        f.write('OPENOCD_PID=$!\n')
        f.write('cleanup() {\n')
        f.write('    kill $OPENOCD_PID\n')
        f.write('}\n')
        f.write('trap cleanup  EXIT\n')
        f.write('sleep 1\n')
        f.write('telnet localhost 19021 | awk \'{ cmd="date +\\"[%Y-%m-%d %H:%M:%S.%3N]\\""; cmd | getline date_str; close(cmd); print date_str, $0; fflush() }\' | tee -a log.txt\n')
        f.write('trap - EXIT\n')
        f.write('exit 0\n')
    os.chmod(make_firmware_path + '/' + 'log.sh', 0o755)


    # 制作同名压缩包，避免手动压缩

    shutil.make_archive(make_firmware_path, 'zip', make_firmware_path)
    publish_directory(firmware_link_path, make_firmware_path)
    remember_package(cache_file, key, make_firmware_path)
    




