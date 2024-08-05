CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
SET(DEMAND_CMAKE_SYSTEM_PROCESSOR cortex-m33)

set(DEMAND_COMPILER_OPTIONS
    "-mcpu=cortex-m33" 
    "-mthumb" 
    "-mfpu=fpv5-sp-d16" 
    "-mfloat-abi=hard" 
    "-ffunction-sections"
    "-fdata-sections"
)

set(DEMAND_LINK_OPTIONS
    "-mcpu=cortex-m33" 
    "-mthumb" 
    "-mfpu=fpv5-sp-d16" 
    "-mfloat-abi=hard" 
    "-ffunction-sections" 
    "-fdata-sections" 
    "-specs=nano.specs" 
    "-specs=nosys.specs" 
    "-Wl,--gc-sections"
)

set(DEMAND_COMPILE_DEFINITIONS "-D__FPU_USED__=1")
set(DEMAND_COMPILE_TOOL_CHAIN "gcc-arm-none-eabi")
