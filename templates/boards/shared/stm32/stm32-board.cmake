include_guard(GLOBAL)

function(fly_configure_board target)
    if(NOT TARGET "${FLY_STM32_ARCH_TARGET}")
        message(FATAL_ERROR "The board requires ${FLY_STM32_ARCH_TARGET}. Load its defconfig.")
    endif()

    string(TOLOWER "${FLY_STM32_FAMILY}" family_lower)
    set(sdk_family "${FLY_TOP_DIR}/arch/stm32/${family_lower}")
    set(system_source "${sdk_family}/Device/ST/${FLY_STM32_FAMILY}/Source/Templates/system_${family_lower}.c")

    target_include_directories(${FLY_STM32_ARCH_TARGET} PRIVATE "${FLY_BOARD_DIR}/include")
    target_include_directories(${target} PRIVATE "${FLY_BOARD_DIR}/include")
    target_compile_definitions(${target} PRIVATE FLY_STM32_DEVICE_HEADER="${family_lower}.h")
    target_sources(${target} PRIVATE
        "${FLY_BOARD_DIR}/board.c"
        "${FLY_BOARD_DIR}/log.c"
        "${system_source}"
    )
    target_link_libraries(${target} ${FLY_STM32_ARCH_TARGET} c m gcc)

    set(linker_script "${CMAKE_CURRENT_BINARY_DIR}/${target}.ld")
    configure_file("${FLY_BOARD_DIR}/flash.ld.in" "${linker_script}" @ONLY)
    target_link_options(${target} PRIVATE "-T${linker_script}")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${linker_script}")
endfunction()
