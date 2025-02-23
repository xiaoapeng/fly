CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
SET(DEMAND_CMAKE_SYSTEM_PROCESSOR cortex-m23)
set(DEMAND_COMPILER_OPTIONS
    "-mcpu=cortex-m23"
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
    "-mcpu=cortex-m23" 
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
