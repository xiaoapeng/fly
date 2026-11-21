#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import shutil
import os
import subprocess
import argparse
from collections import namedtuple
from pathlib import Path

from image_packaging import FIRMWARE_PART_FORMAT, create_image_package, parse_firmware_parts


def find_rtt_ctrl_block_addr(nm_path, elf_path):
    """Read the _SEGGER_RTT symbol address from an ELF via nm.

    nm_path comes from CMake (CMAKE_NM), so no toolchain names are hardcoded
    here. Returns a hex string like '0x2000249c', or None when the symbol is
    not found or nm is unavailable. Used to pin the RTT control block address
    for the log viewer, so it does not depend on J-Link's RAM auto-scan.
    """
    try:
        result = subprocess.run(
            [nm_path, elf_path],
            capture_output=True, text=True, timeout=10
        )
        if result.returncode != 0:
            return None
        for line in result.stdout.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[2] == '_SEGGER_RTT':
                return '0x' + parts[0].lower()
    except (OSError, subprocess.SubprocessError):
        pass
    return None

def parse_args():
    ArgsInfo = namedtuple('ArgsInfo', ['firmware_name', 'chip_name', 'jlink_path', 'output_dir', 'firmware_parts', 'speed', "symlink_name", "interface_name", 'jtagconf', 'nm'])
    
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
    parser.add_argument('--nm', required=True, help='Path to nm (from CMAKE_NM), used to read the RTT control block symbol address from the ELF.')

    parser.add_argument("--firmware-parts", nargs="+", required=True, metavar=FIRMWARE_PART_FORMAT,
                        help="Firmware records with partition capacity in decimal or hexadecimal bytes.")
    parser.add_argument("--cache-file", help="Reuse the current package when the inputs match.")
    args = parser.parse_args()
    if not os.path.exists(args.jlink_path):
        parser.error(f"J-Link tools path does not exist: {args.jlink_path}")

    try:
        parts = parse_firmware_parts(args.firmware_parts)
    except ValueError as error:
        parser.error(str(error))
    args_info = ArgsInfo(args.firmware_name, args.chip_name, args.jlink_path, args.output_dir,
                         parts, args.speed, args.symlink_name, args.interface_name, args.jtagconf, args.nm)
    return args_info, args.cache_file


def write_probe_files(args_info, package):
    # 复制jlink工具
    shutil.copytree(args_info.jlink_path, package / 'tool/jlink')
    # 创建download.jlink文件
    with open(package / 'download.jlink', 'w') as f:
        f.write("r\n")  # 擦除芯片
        f.write("h\n")  
        for firmware_part in args_info.firmware_parts:
            f.write("loadfile {} {}\n".format(firmware_part.bin_name + '.bin', firmware_part.start_addr))  # 加载固件文件
        f.write("r\n")  # 读取内存数据
        f.write("g\n")  # 开始执行
        f.write("qc\n")  # 退出 J-Link 连接
    jlink_flash_args = (
        f'-ExitOnError 1 -autoconnect 1 -device {args_info.chip_name} -if {args_info.interface_name} '
        f'-JTAGConf {args_info.jtagconf} -speed {args_info.speed} -commandfile download.jlink'
    )

    # 生成windows烧写脚本
    with open(package / 'burn.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('cd /d "%script_dir%" || exit /b 1\n\n')

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
        f.write(f'"%JLINK_EXE%" {jlink_flash_args}\n')
        f.write('if not "%errorlevel%"=="0" exit /b %errorlevel%\n\n')

        f.write('if not defined nowait_option (\n')
        f.write('    echo "Execution completed, will automatically exit in 10 seconds."\n')
        f.write('    timeout /t 10 /nobreak > nul\n')
        f.write(')\n')
        f.write('exit 0\n')
    # 生成windows日志查看脚本
    with open(package / 'log.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('cd /d "%script_dir%" || exit /b 1\n\n')

        f.write('set "RTT_EXE=%script_dir%\\tool\\jlink\\win-x64\\rtt-shell.exe"\n')
        # 只有在纯 32 位系统下，PROCESSOR_ARCHITECTURE 才是 x86 且没有 64 位兼容层变量
        f.write('if "%PROCESSOR_ARCHITECTURE%" == "x86" if "%PROCESSOR_ARCHITEW6432%" == "" (\n')
        f.write('    set "RTT_EXE=%script_dir%\\tool\\jlink\\win-x86\\rtt-shell.exe"\n')
        f.write(')\n\n')
        # 使用引号包裹路径以防含有空格，并执行
        f.write(f'"%RTT_EXE%" --device {args_info.chip_name} --if {args_info.interface_name} --speed {args_info.speed} --out_log log.txt\n')
        f.write('if not "%errorlevel%"=="0" exit /b %errorlevel%\n')
        f.write('pause\n')
        f.write('exit /b 0\n')

    # 生成wsl/linux/macos的烧写脚本
    with open(package / 'burn.sh', 'w') as f:
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
        f.write('    CUR_SH_DIR=$(dirname "$(readlink -f "$0")")\n')
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
        f.write('        JLINK_EXE="$JLINK_LOCAL"\n')
        f.write('    else\n')
        f.write('        echo "[WSL] 未检测到J-Link设备，尝试使用tool/jlink/win-x64/JLink.exe下载..."\n')
        f.write('        JLINK_EXE="$CUR_SH_DIR/tool/jlink/win-x64/JLink.exe"\n')
        f.write('    fi\n')
        f.write('\n')
        f.write('# ====================== 4. 普通Linux逻辑：直接执行下载 ======================\n')
        f.write('elif [ "$OS_TYPE" = "Linux" ]; then\n')
        f.write('    echo "[Linux] 直接执行J-Link下载..."\n')
        f.write('    JLINK_EXE="$JLINK_LOCAL"\n')
        f.write('\n')
        f.write('# ====================== 5. macOS逻辑：直接执行下载 ======================\n')
        f.write('elif [ "$OS_TYPE" = "macOS" ]; then\n')
        f.write('    echo "[macOS] 直接执行J-Link下载..."\n')
        f.write('    # 适配macOS ARM64（若JLinkExe路径非默认，可手动指定，如/Applications/SEGGER/JLink/JLinkExe）\n')
        f.write('    JLINK_EXE="$JLINK_LOCAL"\n')
        f.write('fi\n')
        f.write('\n')
        f.write('# ====================== 6. 执行下载命令 ======================\n')
        f.write('cd "$CUR_SH_DIR"\n')
        f.write('echo "使用 $JLINK_EXE 下载固件"\n')
        f.write(f'"$JLINK_EXE" -NoGui 1 {jlink_flash_args}\n')
        f.write('\n')
        f.write('exit 0\n')
        
    os.chmod(package / 'burn.sh', 0o755)

    # 生成wsl/linux/macos的日志查看脚本
    with open(package / 'log.sh', 'w') as f:
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
        f.write('else CUR_SH_DIR=$(dirname "$(readlink -f "$0")"); fi\n')
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

        # Pin the RTT control block address read from the ELF (_SEGGER_RTT).
        # J-Link's RAM auto-scan can miss control blocks that sit outside the
        # region it probes (e.g. low in SRAM next to the kernel data), which
        # shows up as "RTT_CMD_GET_NUM_BUF failed, ret = -2". Passing the
        # exact address removes the dependency on the scan. When nm could not
        # resolve the symbol, fall back to plain auto-scan.
        #
        # The firmware parts are .bin files; the matching ELF sits next to
        # them with the .bin suffix stripped (the FLY build always emits
        # <target>.bin next to <target>).
        rtt_addr = None
        for firmware_part in args_info.firmware_parts:
            bin_path = firmware_part.file_path
            elf_path = bin_path[:-4] if bin_path.endswith('.bin') else bin_path
            if os.path.isfile(elf_path):
                rtt_addr = find_rtt_ctrl_block_addr(args_info.nm, elf_path)
                if rtt_addr:
                    break
        if rtt_addr:
            print('RTT control block address: ' + rtt_addr)
            f.write(f'exec "$RTT_EXE" --device {args_info.chip_name} --if {args_info.interface_name} --speed {args_info.speed} --addr {rtt_addr} --out_log log.txt\n')
        else:
            f.write(f'exec "$RTT_EXE" --device {args_info.chip_name} --if {args_info.interface_name} --speed {args_info.speed} --out_log log.txt\n')

    os.chmod(package / 'log.sh', 0o755)


def main():
    options, cache_file = parse_args()
    inputs = [Path(__file__)]
    if cache_file:
        inputs.extend(Path(part.file_path).with_suffix('') for part in options.firmware_parts)
        inputs.extend(sorted(Path(options.jlink_path).rglob('*')))
    try:
        create_image_package(options, 'jlink', cache_file, inputs, write_probe_files)
    except ValueError as error:
        raise SystemExit(f"Error: {error}") from None


if __name__ == '__main__':
    main()
