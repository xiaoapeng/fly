cmake_minimum_required(VERSION 3.10)
SET(DEMAND_CMAKE_SYSTEM_PROCESSOR cortex-m3)

set(DEMAND_COMPILER_OPTIONS
    "-mcpu=cortex-m3"
    "-ffunction-sections"
    "-fdata-sections"
    "-mfloat-abi=softfp"
)

set(DEMAND_LINK_OPTIONS
    "-mcpu=cortex-m3"
    "-mfloat-abi=softfp"
    "-ffunction-sections" 
    "-fdata-sections"
    "-specs=nano.specs" 
    "-specs=nosys.specs" 
    "-Wl,--gc-sections"
)

set(DEMAND_COMPILE_DEFINITIONS "")
set(DEMAND_COMPILE_TOOL_CHAIN "gcc-arm-none-eabi")
