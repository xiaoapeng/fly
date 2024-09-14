CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
SET(DEMAND_CMAKE_SYSTEM_PROCESSOR riscv32)

set(DEMAND_COMPILER_OPTIONS
    "-march=rv32imac"
    "-mabi=ilp32"
    "-mcmodel=medlow"
    "-ffunction-sections"
    "-fdata-sections"
    "-nostartfiles"
    "-msmall-data-limit=8"
)

set(DEMAND_LINK_OPTIONS
    "-march=rv32imac"
    "-mabi=ilp32"
    "-mcmodel=medlow"
    "-ffunction-sections"
    "-fdata-sections"
    "-nostartfiles"
    "-msmall-data-limit=8"
    "-Wl,--gc-sections"
)

set(DEMAND_COMPILE_DEFINITIONS "")
set(DEMAND_COMPILE_TOOL_CHAIN "gcc-riscv-none-embed")