include_guard(GLOBAL)

function(fly_configure_board target)
    if(NOT TARGET "${FLY_STM32_ARCH_TARGET}")
        message(FATAL_ERROR "The board requires ${FLY_STM32_ARCH_TARGET}. Load its defconfig.")
    endif()

    string(TOLOWER "${FLY_STM32_FAMILY}" family_lower)
    set(sdk_family "${FLY_TOP_DIR}/arch/stm32/${family_lower}")
    set(support_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
    set(system_source "${sdk_family}/Device/ST/${FLY_STM32_FAMILY}/Source/Templates/system_${family_lower}.c")

    # Older generated projects keep the HAL configuration header in board/.
    set(board_include_dirs "${FLY_BOARD_DIR}/include" "${FLY_BOARD_DIR}")
    target_include_directories(${FLY_STM32_ARCH_TARGET} PRIVATE ${board_include_dirs})
    target_include_directories(${target} PRIVATE ${board_include_dirs} "${support_dir}")
    target_compile_definitions(${target} PRIVATE FLY_STM32_DEVICE_HEADER="${family_lower}.h")
    target_sources(${target} PRIVATE "${support_dir}/board.c" "${system_source}")
    target_link_libraries(${target} ${FLY_STM32_ARCH_TARGET} c m gcc)

    set(linker_script "${CMAKE_CURRENT_BINARY_DIR}/${target}.ld")
    configure_file("${support_dir}/flash.ld.in" "${linker_script}" @ONLY)
    target_link_options(${target} PRIVATE "-T${linker_script}")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${linker_script}")
endfunction()
