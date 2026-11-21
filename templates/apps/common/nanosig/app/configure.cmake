target_sources(fly_app PRIVATE runtime.c biz/heartbeat.c)

if(CONFIG_PACKAGE_SCHEDULER_NONE)
    fly_add_board_timebase(fly_app)
endif()
