cmake_minimum_required(VERSION 3.10)

set(DEMAND_COMPILER_OPTIONS
    "-mcmodel=medany"
    "-msmall-data-limit=8"
    "-fmessage-length=0"
    "-fsigned-char"
    "-fno-common"
    "-ffunction-sections"
    "-fdata-sections"
)

set(DEMAND_LINK_OPTIONS
    "--specs=nano.specs"
    "--specs=nosys.specs"
    "-nostartfiles"
    "-Xlinker"
    "--gc-sections"
)
