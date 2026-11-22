cmake_minimum_required(VERSION 3.10)

set(DEMAND_COMPILER_OPTIONS
    "-mcmodel=medlow"
    "-ffunction-sections"
    "-fdata-sections"
    "-nostartfiles"
    "-msmall-data-limit=8"
)

set(DEMAND_LINK_OPTIONS
    "-mcmodel=medlow"
    "-ffunction-sections"
    "-fdata-sections"
    "-nostartfiles"
    "-msmall-data-limit=8"
    "-Wl,--gc-sections"
)
