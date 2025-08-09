cmake_minimum_required(VERSION 3.10)
SET(DEMAND_CMAKE_SYSTEM_PROCESSOR cortex-m3)
set(DEMAND_COMPILER_OPTIONS
    "-mcpu=cortex-m3"
    "-mthumb"
    "-fmessage-length=0"
    "-funsigned-bitfields"
    "-ffunction-sections"
    "-fdata-sections"
    "-Wl,--gc-sections"
    "-fno-common"
    "-flto"
)

set(DEMAND_LINK_OPTIONS
    "-mcpu=cortex-m3" 
    "-mthumb"
    "-specs=nano.specs" 
    "-specs=nosys.specs" 
    "-Xlinker"
    "--gc-sections"
    "-ffunction-sections"
    "-fdata-sections"
    "-flto"
)

set(DEMAND_COMPILE_DEFINITIONS  "" )
set(DEMAND_COMPILE_TOOL_CHAIN   "gcc-arm-none-eabi" )
