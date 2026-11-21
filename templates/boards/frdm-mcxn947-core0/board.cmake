set(FLY_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(FLY_JLINK_DEVICE "MCXN947_M33_0")
set(FLY_FLASH_ADDRESS "0x00000000")
# Internal flash partition ends where the Core 1 image begins.
set(FLY_FLASH_SIZE "0x000C0000")

function(fly_configure_board target)
    if(NOT CONFIG_ARCH_MCXN947)
        message(FATAL_ERROR "This project requires the MCXN947 architecture. Load its defconfig.")
    endif()

    target_sources(${target} PRIVATE
        "${FLY_BOARD_DIR}/board.c"
        "${FLY_BOARD_DIR}/clock_config.c"
    )
    target_include_directories(${target} PRIVATE "${FLY_BOARD_DIR}/include")
    target_link_libraries(${target} arch-mcxn947-core0 c rdimon m gcc)
    set(linker_script "${FLY_BOARD_DIR}/MCXN947_cm33_core0_flash.ld")
    target_link_options(${target} PRIVATE
        "-T${linker_script}"
        "-Wl,--defsym=__stack_size__=0x1000"
    )
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${linker_script}")
endfunction()

function(fly_add_board_timebase target)
    if(NOT CONFIG_PACKAGE_SCHEDULER_NONE)
        message(FATAL_ERROR "The board SysTick timebase requires the bare-metal scheduler.")
    endif()
    target_sources(${target} PRIVATE "${FLY_BOARD_DIR}/board_time.c")
endfunction()
