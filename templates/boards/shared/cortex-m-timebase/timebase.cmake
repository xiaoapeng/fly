include_guard(GLOBAL)

function(fly_add_board_timebase target)
    if(NOT CONFIG_PACKAGE_SCHEDULER_NONE)
        message(FATAL_ERROR "The board SysTick timebase requires the bare-metal scheduler.")
    endif()
    if(NOT FLY_BOARD_CLOCK_HZ MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "The board SysTick timebase requires a positive core clock frequency.")
    endif()

    target_sources(${target} PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/board_time.c")
    target_compile_definitions(${target} PRIVATE FLY_BOARD_CLOCK_HZ=${FLY_BOARD_CLOCK_HZ})
endfunction()
