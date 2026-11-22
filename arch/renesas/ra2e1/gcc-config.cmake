set(DEMAND_COMPILER_OPTIONS -ffunction-sections -fdata-sections -fno-common)
set(DEMAND_LINK_OPTIONS --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections)
