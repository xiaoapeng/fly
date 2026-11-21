#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import shutil
import os
import argparse
from collections import namedtuple
from pathlib import Path

from image_packaging import FIRMWARE_PART_FORMAT, create_image_package, parse_firmware_parts


def parse_args():
    ArgsInfo = namedtuple('ArgsInfo', ['firmware_name', 'output_dir', 'firmware_parts', 'config_file_list', "symlink_name", "rtt_setting"])
    
    parser = argparse.ArgumentParser(description='Generate an OpenOCD firmware package.')

    # Required arguments
    parser.add_argument('--firmware-name', required=True, help='Name of the firmware.')
    parser.add_argument('--output-dir', required=True, help='Directory to output the generated image.')
    parser.add_argument('--symlink-name', default='CURRENT' ,required=False, help='Name of the current package link.')
    parser.add_argument('--config-file-list', nargs='+', type=str, required=True, help='OpenOCD/PE GDB Server configuration file(s) to use when debugging (OpenOCD -f option)')
    parser.add_argument('--rtt-setting', nargs=2, type=lambda x: str(x), default=["0x20000000", "0x8000"], help='RTT setting values in hexadecimal')

    parser.add_argument("--firmware-parts", nargs="+", required=True, metavar=FIRMWARE_PART_FORMAT,
                        help="Firmware records with partition capacity in decimal or hexadecimal bytes.")
    parser.add_argument("--cache-file", help="Reuse the current package when the inputs match.")
    args = parser.parse_args()

    try:
        parts = parse_firmware_parts(args.firmware_parts)
    except ValueError as error:
        parser.error(str(error))
    args_info = ArgsInfo(args.firmware_name, args.output_dir, parts, args.config_file_list,
                         args.symlink_name, args.rtt_setting)
    return args_info, args.cache_file


def write_probe_files(args_info, package):
    config_option = ''
    (package / 'openocd_config').mkdir()

    # 拷贝自定义的配置文件
    for config_file_path in args_info.config_file_list:
        if os.path.isfile(config_file_path):
            openocd_config_path = f'openocd_config/{os.path.basename(config_file_path)}'
            shutil.copyfile(config_file_path, package / openocd_config_path)
            config_option = f'{config_option} -f "{openocd_config_path}"'
        else:
            config_option = f'{config_option} -f "{config_file_path}"'
    
    config_option = f'{config_option} -c "init"'

    burn_script_option = f'{config_option} -c "halt"'
    for firmware_part in args_info.firmware_parts:
        burn_script_option = f'{burn_script_option} -c "flash write_image erase {firmware_part.bin_name}.bin {firmware_part.start_addr}"'
        burn_script_option = f'{burn_script_option} -c "verify_image {firmware_part.bin_name}.bin {firmware_part.start_addr}"'
    burn_script_option = f'{burn_script_option} -c reset -c resume -c exit'
    
    log_script_option = f'{config_option} -c "rtt server start 19021 0" -c "rtt setup {args_info.rtt_setting[0]} {args_info.rtt_setting[1]} \\"SEGGER RTT\\""  -c "rtt start" '

    

    # # 生成windows烧写脚本
    with open(package / 'burn.bat', 'w') as f:
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
    with open(package / 'log.bat', 'w') as f:
        f.write('@echo off\n')
        f.write('set "script_dir=%~dp0"\n')
        f.write('cd /d "%script_dir%" || exit /b 1\n\n')
        f.write('powershell -ExecutionPolicy Bypass -File "log.ps1"\n')
        f.write('exit /b %errorlevel%\n')

    with open(package / 'log.ps1', 'w') as f:
        f.write(f'''
$ErrorActionPreference = "Stop"
Write-Output "Use openocd to obtain RTT logs..."
$openocdArgs = @"
{log_script_option}
"@
$probe = $null
$tcpClient = $null
$reader = $null
$writer = $null
$status = 0
try {{
    $probe = Start-Process -FilePath "openocd" -ArgumentList $openocdArgs -NoNewWindow -PassThru
    Start-Sleep -Seconds 1
    if ($probe.HasExited) {{
        throw "OpenOCD exited before the RTT connection."
    }}

    $tcpClient = New-Object System.Net.Sockets.TcpClient
    $tcpClient.Connect("localhost", 19021)
    $reader = New-Object System.IO.StreamReader($tcpClient.GetStream())
    $writer = New-Object System.IO.StreamWriter("log.txt", $true)

    Write-Output "Waiting for RTT log..."
    while ($null -ne ($line = $reader.ReadLine())) {{
        $time = (Get-Date).ToString("[yyyy-MM-dd HH:mm:ss.fff]")
        $logLine = "$time $line"
        Write-Output $logLine
        $writer.WriteLine($logLine)
        $writer.Flush()
    }}
    if ($probe.HasExited) {{
        $status = $probe.ExitCode
    }}
}} catch {{
    $status = 1
    if ($null -ne $probe -and $probe.HasExited -and $probe.ExitCode -ne 0) {{
        $status = $probe.ExitCode
    }}
    [Console]::Error.WriteLine($_.Exception.Message)
}} finally {{
    try {{
        if ($null -ne $writer) {{ $writer.Dispose() }}
        if ($null -ne $reader) {{ $reader.Dispose() }}
        if ($null -ne $tcpClient) {{ $tcpClient.Dispose() }}
    }} finally {{
        if ($null -ne $probe) {{
            if (-not $probe.HasExited) {{
                Stop-Process -InputObject $probe -ErrorAction SilentlyContinue
                if (-not $probe.WaitForExit(3000)) {{
                    [Console]::Error.WriteLine("Could not stop the OpenOCD process.")
                    if ($status -eq 0) {{ $status = 1 }}
                }}
            }}
            $probe.Dispose()
        }}
    }}
}}
exit $status
''')
    os.chmod(package / 'log.ps1', 0o755)

    # 生成wsl烧写脚本
    with open(package / 'burn.sh', 'w') as f:
        f.write('#!/bin/sh\n')
        f.write('CDPATH= cd -P -- "$(dirname -- "$0")" || exit 1\n')
        f.write('echo "Download firmware using openocd.."\n')
        f.write(f'exec openocd {burn_script_option}\n')
    os.chmod(package / 'burn.sh', 0o755)

    (package / 'log.sh').write_text(fr'''#!/bin/bash
set -o pipefail
CDPATH= cd -P -- "$(dirname -- "$0")" || exit 1
echo "Use openocd to obtain RTT logs.."

openocd {log_script_option} &
probe_pid=$!

cleanup() {{
    if [ -n "$probe_pid" ]; then
        kill "$probe_pid" 2>/dev/null || true
        wait "$probe_pid" 2>/dev/null || true
    fi
}}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

probe_has_exited() {{
    if kill -0 "$probe_pid" 2>/dev/null; then
        return 1
    fi
    wait "$probe_pid"
    probe_status=$?
    probe_pid=
    return 0
}}

sleep 1
if probe_has_exited; then
    echo "OpenOCD exited before the RTT connection." >&2
    if [ "$probe_status" -eq 0 ]; then exit 1; fi
    exit "$probe_status"
fi

telnet localhost 19021 | awk '{{ cmd="date +\"[%Y-%m-%d %H:%M:%S.%3N]\""; cmd | getline date_str; close(cmd); print date_str, $0; fflush() }}' | tee -a log.txt
status=$?
if probe_has_exited && [ "$probe_status" -ne 0 ]; then
    exit "$probe_status"
fi
exit "$status"
''', encoding='utf-8')
    os.chmod(package / 'log.sh', 0o755)


def main():
    options, cache_file = parse_args()
    inputs = [Path(__file__), *(Path(path) for path in options.config_file_list)]
    try:
        create_image_package(options, 'openocd', cache_file, inputs, write_probe_files)
    except ValueError as error:
        raise SystemExit(f"Error: {error}") from None


if __name__ == '__main__':
    main()
