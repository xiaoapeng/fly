target_sources(${FLY_PACK_TARGET} PRIVATE runtime.c biz/heartbeat.c)
fly_add_board_timebase(${FLY_PACK_TARGET})
