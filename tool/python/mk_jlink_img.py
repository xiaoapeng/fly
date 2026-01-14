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

# ./mk_jlink_img.py --firmware-name TEST --chip-name GD32F103C8 --jlink-path ../win/jlink/  --output-dir ../../image/  --speed 4000 BOOT:V0.0.1:0x8000000:../../build/gd32_t527_mcu_demo_app.bin APP:V0.0.1:0x8000000:../../build/gd32_t527_mcu_demo_app.bin
def parse_args():
    ArgsInfo = namedtuple('ArgsInfo', ['firmware_name', 'chip_name', 'jlink_path', 'output_dir', 'firmware_parts', 'speed', "symlink_name", "interface_name", 'jtagconf'])
    FirmwareInfo = namedtuple('FirmwareInfo', ['bin_name', 'bin_version_str', 'bin_version_uint32', 'start_addr', 'file_path'])
    
    parser = argparse.ArgumentParser(description='Generate J-Link compatible firmware image.')

    # Required arguments
    parser.add_argument('--firmware-name', required=True, help='Name of the firmware.')
    parser.add_argument('--jlink-path', required=True, help='Path to the J-Link tool.')
    parser.add_argument('--output-dir', required=True, help='Directory to output the generated image.')
    parser.add_argument('--chip-name', required=True, help='Chip models supported by J-Link.')
    parser.add_argument('--symlink-name', default='CURRENT' ,required=False, help='The name of the soft connection pointing to the firmware.')
    parser.add_argument('--speed', required=True, default=4000, help='J-Link speed.')
    parser.add_argument('--interface-name', default='swd', required=False, help='J-Link interface name.')
    parser.add_argument('--jtagconf', default='-1,-1', required=True, help='Configures the JTAG scan configuration of the target device.\nIRPre==-1 and DRPre==-1 can be passed to use auto-detection (=> first known device will be used).')

    # Positional arguments for firmware parts
    parser.add_argument('--firmware-parts', nargs='+', type=str, required=True, help='Memory address and firmware file pairs in the format bin_name:bin_version:address:file_path.')

    args = parser.parse_args()
    if not os.path.exists(args.jlink_path):
        print('JLink executable does not exist: {}'.format(args.jlink_path))
        sys.exit(1)
        
    args_info = ArgsInfo(args.firmware_name, args.chip_name, args.jlink_path, args.output_dir, [], args.speed, args.symlink_name, args.interface_name, args.jtagconf)
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
    shutil.copytree(args_info.jlink_path, make_firmware_path + '/tool/jlink')
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
        f.write('cd /d "%script_dir%"\n\n')

        # --- 平台检测逻辑 ---
        f.write(':: Detect Architecture\n')
        f.write('set "JLINK_EXE=%script_dir%\\tool\\jlink\\win-x64\\JLink.exe"\n')
        # 如果是 x86 系统，PROCESSOR_ARCHITECTURE 会是 x86
        # 且 PROCESSOR_ARCHITEW6432 不存在
        f.write('if "%PROCESSOR_ARCHITECTURE%" == "x86" if "%PROCESSOR_ARCHITEW6432%" == "" (\n')
        f.write('    set "JLINK_EXE=%script_dir%\\tool\\jlink\\win-x86\\JLink.exe"\n')
        f.write(')\n\n')

        f.write('set "nowait_option="\n')
        f.write('if "%1" == "--nowait" (\n')
        f.write('    set "nowait_option=--nowait"\n')
        f.write(')\n\n')

        # --- 调用 JLink ---
        f.write('echo Using: %JLINK_EXE%\n')
        f.write(f'"%JLINK_EXE%" -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink\n\n')

        f.write('if not defined nowait_option (\n')
        f.write('    echo "Execution completed, will automatically exit in 10 seconds."\n')
        f.write('    timeout /t 10 /nobreak > nul\n')
        f.write(')\n')
        f.write('exit 0\n')
    # 生成windows日志查看脚本
    with open(make_firmware_path + '/' + 'log.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('cd /d "%script_dir%"\n\n')

        f.write('set "RTT_EXE=%script_dir%\\tool\\jlink\\win-x64\\rtt-shell.exe"\n')
        # 只有在纯 32 位系统下，PROCESSOR_ARCHITECTURE 才是 x86 且没有 64 位兼容层变量
        f.write('if "%PROCESSOR_ARCHITECTURE%" == "x86" if "%PROCESSOR_ARCHITEW6432%" == "" (\n')
        f.write('    set "RTT_EXE=%script_dir%\\tool\\jlink\\win-x86\\rtt-shell.exe"\n')
        f.write(')\n\n')
        # 使用引号包裹路径以防含有空格，并执行
        f.write(f'"%RTT_EXE%" --device {args_info.chip_name} --if {args_info.interface_name} --speed {args_info.speed} --out_log log.txt\n\n')
        
        f.write('pause\n')

    # 生成wsl/linux/macos的烧写脚本
    with open(make_firmware_path + '/' + 'burn.sh', 'w') as f:
        f.write('#!/bin/bash\n')
        f.write('set -e  # 错误立即退出，提升脚本健壮性\n')
        f.write('\n')
        f.write('# ====================== 1. 基于uname精准识别系统类型 ======================\n')
        f.write('# 初始化系统类型\n')
        f.write('OS_TYPE=""\n')
        f.write('KERNEL=$(uname -r)       # 内核版本（用于识别WSL）\n')
        f.write('SYSTEM=$(uname -s)       # 系统内核名称（Darwin/Linux）\n')
        f.write('ARCH=$(uname -m)         # 系统架构（arm64/x86_64）\n')
        f.write('\n')
        f.write('# 识别WSL（内核版本包含microsoft/WSL）\n')
        f.write('if echo "$KERNEL" | grep -qi "WSL"; then\n')
        f.write('    OS_TYPE="WSL"\n')
        f.write('# 识别macOS（SYSTEM为Darwin）\n')
        f.write('elif [ "$SYSTEM" = "Darwin" ]; then\n')
        f.write('    OS_TYPE="macOS"\n')
        f.write('# 剩余Linux为普通Linux（如Ubuntu）\n')
        f.write('elif [ "$SYSTEM" = "Linux" ]; then\n')
        f.write('    OS_TYPE="Linux"\n')
        f.write('else\n')
        f.write('    echo "错误：不支持的系统！仅支持WSL、macOS、普通Linux"\n')
        f.write('    exit 1\n')
        f.write('fi\n')
        f.write('\n')
        f.write('# ====================== 2. 路径与工具适配 ======================\n')
        f.write('# 获取脚本绝对路径（兼容macOS readlink -f不兼容问题）\n')
        f.write('if [ "$OS_TYPE" = "macOS" ]; then\n')
        f.write('    CUR_SH_DIR=$(cd "$(dirname "$0")" && pwd -P)\n')
        f.write('else\n')
        f.write('    CUR_SH_DIR=$(dirname $(readlink -f "$0"))\n')
        f.write('fi\n')
        f.write('\n')
        f.write('# 定义不同系统的JLink执行文件\n')
        f.write('case $OS_TYPE in\n')
        f.write('    WSL|Linux)\n')
        f.write('        # WSL/普通Linux共用JLinkExe\n')
        f.write('        JLINK_LOCAL="JLinkExe"\n')
        f.write('        ;;    \n')
        f.write('    macOS)\n')
        f.write('        # macOS用系统自带命令检测USB设备，JLinkExe适配ARM64架构\n')
        f.write('        JLINK_LOCAL="JLinkExe"\n')
        f.write('        ;;  \n')
        f.write('esac\n')
        f.write('\n')
        f.write('# ====================== 3. WSL专属逻辑：检测JLink并选择执行方式 ======================\n')
        f.write('JLINK_CMD=""\n')
        f.write('if [ "$OS_TYPE" = "WSL" ]; then\n')
        f.write('    # 检测WSL内是否有JLink设备\n')
        f.write('    JLINK_DEVICE_CHECK="lsusb | grep -i \'J-Link\'"\n')
        f.write('    if eval "$JLINK_DEVICE_CHECK" >/dev/null 2>&1; then\n')
        f.write('        echo "[WSL] 检测到J-Link设备，使用本地JLinkExe下载..."\n')
        f.write(f'        JLINK_CMD="$JLINK_LOCAL -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink"\n')
        f.write('    else\n')
        f.write('        echo "[WSL] 未检测到J-Link设备，尝试使用tool/JLink.exe下载..."\n')
        f.write(f'        JLINK_CMD="$CUR_SH_DIR/tool/win-x64/JLink.exe -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink"\n')
        f.write('    fi\n')
        f.write('\n')
        f.write('# ====================== 4. 普通Linux逻辑：直接执行下载 ======================\n')
        f.write('elif [ "$OS_TYPE" = "Linux" ]; then\n')
        f.write('    echo "[Linux] 直接执行J-Link下载..."\n')
        f.write(f'    JLINK_CMD="$JLINK_LOCAL -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink"\n')
        f.write('\n')
        f.write('# ====================== 5. macOS逻辑：直接执行下载 ======================\n')
        f.write('elif [ "$OS_TYPE" = "macOS" ]; then\n')
        f.write('    echo "[macOS] 直接执行J-Link下载..."\n')
        f.write('    # 适配macOS ARM64（若JLinkExe路径非默认，可手动指定，如/Applications/SEGGER/JLink/JLinkExe）\n')
        f.write(f'    JLINK_CMD="$JLINK_LOCAL -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} -JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink"\n')
        f.write('fi\n')
        f.write('\n')
        f.write('# ====================== 6. 执行下载命令 ======================\n')
        f.write('cd "$CUR_SH_DIR"\n')
        f.write('echo "执行命令：$JLINK_CMD"\n')
        f.write('eval "$JLINK_CMD"\n')
        f.write('\n')
        f.write('exit 0\n')
        
    os.chmod(make_firmware_path + '/' + 'burn.sh', 0o755)

    # 生成wsl/linux/macos的日志查看脚本
    with open(make_firmware_path + '/' + 'log.sh', 'w') as f:
        f.write('#!/bin/bash\n\n')
        
        f.write('# --- 环境识别 ---\n')
        f.write('KERNEL=$(uname -r)\n')
        f.write('SYSTEM=$(uname -s)\n')
        f.write('ARCH=$(uname -m)\n\n')
        
        f.write('if echo "$KERNEL" | grep -qi "WSL"; then OS_TYPE="WSL";\n')
        f.write('elif [ "$SYSTEM" = "Darwin" ]; then OS_TYPE="macOS";\n')
        f.write('else OS_TYPE="Linux"; fi\n\n')

        f.write('# --- 路径获取 ---\n')
        f.write('if [ "$OS_TYPE" = "macOS" ]; then CUR_SH_DIR=$(cd "$(dirname "$0")" && pwd -P);\n')
        f.write('else CUR_SH_DIR=$(dirname $(readlink -f "$0")); fi\n')
        f.write('cd "$CUR_SH_DIR"\n\n')

        f.write('# --- 架构与平台校验 ---\n')
        f.write('case $OS_TYPE in\n')
        f.write('    "WSL")\n')
        f.write('        if lsusb | grep -qi "J-Link"; then\n')
        f.write('            # WSL 内部运行，必须是 x86_64\n')
        f.write('            if [ "$ARCH" != "x86_64" ]; then echo "错误：WSL环境仅支持 x86_64 架构"; exit 1; fi\n')
        f.write('            RTT_EXE="$CUR_SH_DIR/tool/jlink/linux-x64/rtt-shell"\n')
        f.write('        else\n')
        f.write('            RTT_EXE="$CUR_SH_DIR/tool/jlink/win-x64/rtt-shell.exe"\n')
        f.write('        fi ;;\n')
        f.write('    "macOS")\n')
        f.write('        # macOS 无论是 Intel(x86_64) 还是 Apple Silicon(arm64) 统统指向 macos 目录\n')
        f.write('        RTT_EXE="$CUR_SH_DIR/tool/jlink/macos/rtt-shell" ;;\n')
        f.write('    "Linux")\n')
        f.write('        if [ "$ARCH" = "x86_64" ]; then\n')
        f.write('            RTT_EXE="$CUR_SH_DIR/tool/jlink/linux-x64/rtt-shell"\n')
        f.write('        elif [[ "$ARCH" == i*86 ]]; then\n')
        f.write('            RTT_EXE="$CUR_SH_DIR/tool/jlink/linux-x86/rtt-shell"\n')
        f.write('        else\n')
        f.write('            echo "错误：Linux环境下不支持的架构: $ARCH (仅支持 x86_64/i386)"\n')
        f.write('            exit 1\n')
        f.write('        fi ;;\n')
        f.write('esac\n\n')

        f.write('if [ ! -f "$RTT_EXE" ]; then echo "错误：找不到执行文件 $RTT_EXE"; exit 1; fi\n')
        f.write('chmod +x "$RTT_EXE" 2>/dev/null\n\n')

        f.write('echo "Starting Log Viewer [$OS_TYPE / $ARCH]: $RTT_EXE"\n')

        f.write(f'"$RTT_EXE" --device {args_info.chip_name} --if {args_info.interface_name} --speed {args_info.speed} --out_log log.txt\n\n')
        
        f.write('echo "Debug finished.."\n')
        f.write('exit 0\n')

    os.chmod(make_firmware_path + '/' + 'log.sh', 0o755)


    if os.path.exists(firmware_link_path):
        os.remove(firmware_link_path)
    
    # 制作同名压缩包，避免手动压缩

    shutil.make_archive(make_firmware_path, 'zip', make_firmware_path)
    try:
        os.symlink(make_firmware_name, firmware_link_path)
    except (OSError, PermissionError) as e:
        if sys.platform.startswith("win"):
            print(
                "Failed to create symlink. On Windows, please enable Developer Mode "
                "to allow symlink creation.\nError details:", e
            )
        else:
            print("Failed to create symlink. Error details:", e)
    




