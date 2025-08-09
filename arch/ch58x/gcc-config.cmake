cmake_minimum_required(VERSION 3.10)
SET(DEMAND_CMAKE_SYSTEM_PROCESSOR riscv)

set(DEMAND_COMPILER_OPTIONS
    "-march=rv32imac"
    "-mabi=ilp32"
    "-mcmodel=medany"
    "-msmall-data-limit=8"
    "-fmessage-length=0"
    "-fsigned-char"
    "-fno-common"
    "-ffunction-sections"
    "-fdata-sections"
)

set(DEMAND_LINK_OPTIONS
    "-march=rv32imac"
    "-mabi=ilp32"
    "--specs=nano.specs"
    "--specs=nosys.specs"
    "-nostartfiles"
    "-Xlinker"
    "--gc-sections"
)

set(DEMAND_COMPILE_DEFINITIONS "")
set(DEMAND_COMPILE_TOOL_CHAIN "gcc-mrs-riscv-none-elf")
