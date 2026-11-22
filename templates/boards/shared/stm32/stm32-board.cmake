include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/timebase.cmake")

function(fly_configure_board target)
    if(NOT TARGET "${FLY_STM32_ARCH_TARGET}")
        message(FATAL_ERROR "The board requires ${FLY_STM32_ARCH_TARGET}. Load its defconfig.")
    endif()

    get_property(device_header TARGET ${FLY_STM32_ARCH_TARGET} PROPERTY FLY_STM32_DEVICE_HEADER)
    if(NOT device_header)
        message(FATAL_ERROR "The chip target ${FLY_STM32_ARCH_TARGET} does not expose an STM32 device header.")
    endif()

    target_include_directories(${FLY_STM32_ARCH_TARGET} PRIVATE "${FLY_BOARD_DIR}/include")
    target_include_directories(${target} PRIVATE "${FLY_BOARD_DIR}/include")
    target_compile_definitions(${target} PRIVATE FLY_STM32_DEVICE_HEADER="${device_header}")
    target_sources(${target} PRIVATE
        "${FLY_BOARD_DIR}/board.c"
        "${FLY_BOARD_DIR}/log.c"
    )
    target_link_libraries(${target} ${FLY_STM32_ARCH_TARGET} c m gcc)

    set(linker_script "${CMAKE_CURRENT_BINARY_DIR}/${target}.ld")
    configure_file("${FLY_BOARD_DIR}/flash.ld.in" "${linker_script}" @ONLY)
    target_link_options(${target} PRIVATE "-T${linker_script}")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${linker_script}")
endfunction()
