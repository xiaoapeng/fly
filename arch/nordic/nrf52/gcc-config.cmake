set(DEMAND_COMPILER_OPTIONS
    -ffunction-sections
    -fdata-sections
)

set(DEMAND_LINK_OPTIONS
    --specs=nano.specs
    --specs=nosys.specs
    -Wl,--gc-sections
)
