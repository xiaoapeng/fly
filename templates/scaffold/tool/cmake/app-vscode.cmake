function(app_add_vscode_debug firmware_target)
    if(NOT FLY_JLINK_DEVICE OR NOT CMAKE_GDB OR NOT CMAKE_OBJDUMP)
        return()
    endif()

    set(flash_debug_args "")
    if(TARGET default_flash)
        set(flash_debug_args FLASH_TASK "FLY: Flash")
    endif()

    add_vscode_cortex_debug_gdb(default
        CHIP_NAME "${FLY_JLINK_DEVICE}"
        ELF_NAME_LIST "${firmware_target}"
        SERVER_TYPE jlink
        RTT_DEBUG "${CONFIG_PACKAGE_SEGGER_RTT}"
        DEPENDS "${firmware_target}"
        ${flash_debug_args}
    )
endfunction()
