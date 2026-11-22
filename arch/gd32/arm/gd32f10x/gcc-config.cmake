cmake_minimum_required(VERSION 3.10)
set(DEMAND_COMPILER_OPTIONS
    "-fmessage-length=0"
    "-funsigned-bitfields"
    "-ffunction-sections"
    "-fdata-sections"
    "-Wl,--gc-sections"
    "-fno-common"
    "-flto"
)

set(DEMAND_LINK_OPTIONS
    "-specs=nano.specs" 
    "-specs=nosys.specs" 
    "-Xlinker"
    "--gc-sections"
    "-ffunction-sections"
    "-fdata-sections"
    "-flto"
)
