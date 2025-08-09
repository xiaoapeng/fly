cmake_minimum_required(VERSION 3.10)
SET(DEMAND_CMAKE_SYSTEM_PROCESSOR cortex-m7)

set(DEMAND_COMPILER_OPTIONS
    "-mcpu=cortex-m4"
    "-mfpu=fpv4-sp-d16"
    "-mfloat-abi=hard"
    "-ffunction-sections"
    "-fdata-sections"
)

set(DEMAND_LINK_OPTIONS
    "-mcpu=cortex-m4"
    "-mfpu=fpv4-sp-d16"
    "-mfloat-abi=hard"
    "-ffunction-sections" 
    "-fdata-sections"
    "-specs=nano.specs" 
    "-specs=nosys.specs" 
    "-Wl,--gc-sections"
)
set(DEMAND_COMPILE_DEFINITIONS "")
set(DEMAND_COMPILE_TOOL_CHAIN "gcc-arm-none-eabi")
