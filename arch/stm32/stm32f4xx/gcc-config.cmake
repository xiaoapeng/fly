cmake_minimum_required(VERSION 3.10)

set(DEMAND_COMPILER_OPTIONS
    "-ffunction-sections"
    "-fdata-sections"
)

set(DEMAND_LINK_OPTIONS
    "-ffunction-sections" 
    "-fdata-sections"
    "-specs=nano.specs" 
    "-specs=nosys.specs" 
    "-Wl,--gc-sections"
)
