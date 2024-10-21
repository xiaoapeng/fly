#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import shutil
import os
import subprocess
import argparse
from datetime import datetime
from collections import namedtuple
from mk_mcu_firmware_factory_data import append_factory_data_to_bin
import sys


def git_get_hash():
    try:
        # 使用 subprocess 运行 git rev-parse HEAD 命令
        result = subprocess.run(['git', 'rev-parse', 'HEAD'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
        # 返回结果的标准输出部分，即当前的 git hash
        return result.stdout.strip()[:10]
    except subprocess.CalledProcessError as e:
        # 处理错误并输出错误信息
        print(f"Error occurred: {e.stderr.strip()}")
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

# ./mk_jlink_img.py --firmware-name TEST --chip-name GD32F103C8 --win-jlink-path ../win/jlink/  --output-dir ../../image/  --speed 4000 BOOT:V0.0.1:0x8000000:../../build/gd32_t527_mcu_demo_app.bin APP:V0.0.1:0x8000000:../../build/gd32_t527_mcu_demo_app.bin
def parse_args():
    ArgsInfo = namedtuple('ArgsInfo', ['firmware_name', 'chip_name', 'win_jlink_path', 'output_dir', 'firmware_parts', 'speed', "symlink_name", "interface_name", 'jtagconf'])
    FirmwareInfo = namedtuple('FirmwareInfo', ['bin_name', 'bin_version_str', 'bin_version_uint32', 'start_addr', 'file_path'])
    
    parser = argparse.ArgumentParser(description='Generate J-Link compatible firmware image.')

    # Required arguments
    parser.add_argument('--firmware-name', required=True, help='Name of the firmware.')
    parser.add_argument('--win-jlink-path', required=True, help='Path to the J-Link tool.')
    parser.add_argument('--output-dir', required=True, help='Directory to output the generated image.')
    parser.add_argument('--chip-name', required=True, help='Chip models supported by J-Link.')
    parser.add_argument('--symlink-name', default='CURRENT' ,required=False, help='Chip models supported by J-Link.')
    parser.add_argument('--speed', required=True, default=4000, help='J-Link speed.')
    parser.add_argument('--interface-name', default='swd', required=False, help='J-Link interface name.')
    parser.add_argument('--jtagconf', default='-1,-1', required=True, help='Configures the JTAG scan configuration of the target device.\nIRPre==-1 and DRPre==-1 can be passed to use auto-detection (=> first known device will be used).')

    # Positional arguments for firmware parts
    parser.add_argument('--firmware-parts', nargs='+', type=str, required=True, help='Memory address and firmware file pairs in the format bin_name:bin_version:address:file_path.')

    args = parser.parse_args()
    if not os.path.exists(args.win_jlink_path):
        print('JLink executable does not exist: {}'.format(args.win_jlink_path))
        sys.exit(1)
        
    args_info = ArgsInfo(args.firmware_name, args.chip_name, args.win_jlink_path, args.output_dir, [], args.speed, args.symlink_name, args.interface_name, args.jtagconf)
    for firmware_part in args.firmware_parts:
        try:
            bin_name, bin_version, start_addr,file_path = firmware_part.split(':')
        except ValueError:
            print('Invalid firmware part: %s' % firmware_part)
            continue
        if not os.path.isfile(file_path):
            print('Invalid firmware part: %s' % firmware_part)
            continue
            
        bin_version_uint32,bin_version_str = _parse_version(bin_version)
        args_info.firmware_parts.append(FirmwareInfo(bin_name, bin_version_str, bin_version_uint32, start_addr, file_path))
    return args_info


if __name__ == '__main__':
    args_info = parse_args()
    git_field = ''+git_get_hash()
    if not git_is_workspace_clean():
        git_field = 'dirty_' + git_field
    time_field = datetime.now().strftime("%Y%m%d_%H%M%S")
    make_firmware_name = args_info.firmware_name + '_jlink'
    for firmware_part in args_info.firmware_parts:
        make_firmware_name = make_firmware_name + '_' + firmware_part.bin_name + firmware_part.bin_version_str
    make_firmware_name = make_firmware_name + '_' + time_field + '_' + git_field
    print('make_firmware_name:', make_firmware_name)
    make_firmware_path = args_info.output_dir + '/' + make_firmware_name
    firmware_link_path = args_info.output_dir + '/' + args_info.symlink_name
    # 创建固件文件夹
    os.makedirs(make_firmware_path)
    # 复制jlink工具
    shutil.copytree(args_info.win_jlink_path, make_firmware_path + '/' + 'tool')
    # 创建download.jlink文件
    with open(make_firmware_path + '/' + 'download.jlink', 'w') as f:
        f.write("r\n")  # 擦除芯片
        f.write("h\n")  
        for firmware_part in args_info.firmware_parts:
            f.write("loadfile {} {}\n".format(firmware_part.bin_name + '.bin', firmware_part.start_addr))  # 加载固件文件
        f.write("r\n")  # 读取内存数据
        f.write("g\n")  # 开始执行
        f.write("qc\n")  # 退出 J-Link 连接
    # 拷贝bin文件
    for firmware_part in args_info.firmware_parts:
        dst_bin_path = make_firmware_path + '/' + firmware_part.bin_name + '.bin'
        shutil.copyfile(firmware_part.file_path, dst_bin_path)
        # bin文件添加尾部信息
        append_factory_data_to_bin(dst_bin_path, firmware_part.bin_version_uint32, 
            firmware_part.bin_name, args_info.firmware_name, 'git@' + git_field + ' ' + 'time@' + time_field)

    # 生成windows烧写脚本
    with open(make_firmware_path + '/' + 'burn.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('\n')
        f.write('set "nowait_option="\n')
        f.write('if "%1" == "--nowait" (\n')
        f.write('    set "nowait_option=--nowait"\n')
        f.write(')\n')
        f.write('\n')
        f.write('cd "%script_dir%"\n')
        f.write(f'tool\\JLink.exe -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink\n')
        f.write('\n')
        f.write('if not defined nowait_option (\n')
        f.write('    echo "Execution completed, will automatically exit in 10 seconds."\n')
        f.write('    timeout /t 10 /nobreak > nul\n')
        f.write(')\n')
        f.write('\n')
        f.write('exit 0\n')
    # 生成windows日志查看脚本
    with open(make_firmware_path + '/' + 'log.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('cd "%script_dir%"\n\n')
        f.write('powershell -ExecutionPolicy Bypass -File "log.ps1"')
    with open(make_firmware_path + '/' + 'log.ps1', 'w') as f:
        f.write(f'''
$CUR_SH_DIR = (Get-Location).Path
                
# Set the log file path
$OUTPUT_LOG = "log.txt"

# Remove old log files and create new ones
Write-Host "log path: $CUR_SH_DIR\\$OUTPUT_LOG"

Write-Host "Debug using windows jlink.."
$tmpLogFile = [System.IO.Path]::GetTempFileName()
$outputFile = [System.IO.Path]::GetTempFileName()
$erroroutputFile = [System.IO.Path]::GetTempFileName()
$nulFile = [System.IO.Path]::GetTempFileName()
New-Item -Path $nulFile -ItemType File -Force | Out-Null



# Define the script block for processing the log file
$scriptBlock = {{
    param ($tmpLogFilePath, $outputFilePath)

    # Continuously read the log file
    Get-Content -Path $tmpLogFilePath -Encoding UTF8 -Wait | ForEach-Object {{
        # Process each line, for example, by adding a timestamp
        $date_str = Get-Date -Format "[yyyy-MM-dd HH:mm:ss.fff]"
        Write-Output "$date_str $_"
        Add-Content -Path $outputFilePath -Value "$date_str $_" -Encoding UTF8
    }}
}}

# Save the script block to a temporary PowerShell script file
$scriptFile = [System.IO.Path]::GetTempFileName() + ".ps1"
$scriptBlock | Out-File -FilePath $scriptFile -Encoding UTF8

# Start a new PowerShell process to run the log processing script
$logProcess = Start-Process powershell -ArgumentList "-File `"$scriptFile`" -outputFilePath `"$OUTPUT_LOG`" -tmpLogFilePath `"$tmpLogFile`"" -NoNewWindow -PassThru

# Start the JLinkRTTLogger tool
$jLinkRTTLoggerProcess = Start-Process -FilePath "tool\\JLinkRTTLogger.exe" -ArgumentList "-Device {args_info.chip_name} -If {args_info.interface_name} -Speed {args_info.speed} -RTTChannel 0 $tmpLogFile" `
    -PassThru -NoNewWindow `
    -RedirectStandardOutput $outputFile -RedirectStandardError $erroroutputFile -RedirectStandardInput $nulFile

$jLinkRTTLoggerProcess.WaitForExit()

Write-Output "Debug finished.."
# kill the logProcess
Stop-Process -Id $logProcess.Id -Force

# Clean up temporary files
Remove-Item $tmpLogFile
Remove-Item $nulFile
Remove-Item $outputFile
Remove-Item $erroroutputFile
Remove-Item $scriptFile

exit 0

''')

    # 生成wsl烧写脚本
    with open(make_firmware_path + '/' + 'burn.sh', 'w') as f:
        f.write('#!/bin/bash\n\n')
        f.write('CUR_SH_DIR=$(dirname $(readlink -f "$0"))\n')
        f.write('wsl_is_use_jilink=$(lsusb | grep -i "J-Link")\n\n')
        f.write('cd $CUR_SH_DIR\n')
        f.write('if [ -n "$wsl_is_use_jilink" ]; then\n')
        f.write('   echo "Download using WSL jlink.."\n')
        f.write(f'   JLinkExe -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile  download.jlink\n')
        f.write('else\n')
        f.write('   echo "Download using windows jlink.."\n')
        f.write(f'   tool/JLink.exe -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink\n')
        f.write('fi\n')
        f.write('exit 0\n')
    os.chmod(make_firmware_path + '/' + 'burn.sh', 0o755)

    # 生成wsl日志查看脚本
    with open(make_firmware_path + '/' + 'log.sh', 'w') as f:
        f.write('#!/bin/bash\n\n')
        f.write('CUR_SH_DIR=$(dirname $(readlink -f "$0"))\n')
        f.write('wsl_is_use_jilink=$(lsusb | grep -i "J-Link")\n\n')
        f.write('cd $CUR_SH_DIR\n')
        f.write('TMP_LOG_FILE=$(mktemp)\n')
        f.write('OUTPUT_LOG="log.txt"\n')
        f.write('echo "log path: $CUR_SH_DIR/$OUTPUT_LOG"\n')
        f.write('(tail -F "$TMP_LOG_FILE" | awk \'{cmd="date +\\"[%Y-%m-%d %H:%M:%S.%3N]\\""; cmd | getline date_str; close(cmd); print date_str, $0; fflush();}\' | tee -a "$OUTPUT_LOG") &\n')
        f.write('if [ -n "$wsl_is_use_jilink" ]; then\n')
        f.write('   echo "Debug using WSL jlink.."\n')
        f.write(f'   JLinkRTTLogger -Device {args_info.chip_name} -If {args_info.interface_name} -Speed {args_info.speed}  -RTTChannel 0 "$TMP_LOG_FILE" < /dev/zero >/dev/null 2>&1\n')
        f.write('else\n')
        f.write('   echo "Debug using windows jlink.."\n')
        f.write(f'   tool/JLinkRTTLogger.exe -Device {args_info.chip_name} -If {args_info.interface_name} -Speed {args_info.speed}  -RTTChannel 0 "$TMP_LOG_FILE" < /dev/zero >/dev/null 2>&1\n')
        f.write('fi\n')
        f.write('rm -f "$TMP_LOG_FILE"')
        f.write('echo "Debug finished.."\n')
        f.write('exit 0\n')
    os.chmod(make_firmware_path + '/' + 'log.sh', 0o755)


    if os.path.exists(firmware_link_path):
        os.remove(firmware_link_path)
    
    # 制作同名压缩包，避免手动压缩

    shutil.make_archive(make_firmware_path, 'zip', make_firmware_path)
    os.symlink(make_firmware_name, firmware_link_path)
    




