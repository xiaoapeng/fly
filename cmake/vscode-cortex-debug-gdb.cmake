include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/git-status.cmake")

function(add_vscode_cortex_debug_gdb CMAKE_TARGET)
    cmake_parse_arguments(
        CUSTOM_FUNC ""
        "CHIP_NAME;DEPENDS;SPEED;SERVER_TYPE;RTT_DEBUG;INTERFACE;OTHER_EXT_CONFIG"
        "OPENOCD_CONFIGFILE_LIST;ELF_NAME_LIST"
        ""
        ${ARGN}
    )

    # 设置 SPEED 的默认值
    if(${CUSTOM_FUNC_RTT_DEBUG})
        set(RTT_EN_FLAGS "--rtt-enabled")
    endif()
    
    # 追加的config配置项，最后合并到launch.json中去
    # 基本格式
    # {
    #     "ext-attach-config":{
    #        /* 写这里追加到attach的配置项 */
    #     },
    #     "ext-launch-config":{
    #        /* 写这里追加到launch的配置项 */
    #     }
    # }
    if( CUSTOM_FUNC_OTHER_EXT_CONFIG )
        set(CUSTOM_FUNC_OTHER_EXT_CONFIG_OPTION "--ext-config-json")
    endif()

    if(NOT CUSTOM_FUNC_SPEED)
        set(CUSTOM_FUNC_SPEED "4000")
    endif()

    if(NOT CUSTOM_FUNC_SERVER_TYPE)
        set(CUSTOM_FUNC_SERVER_TYPE "jlink")
    endif()
    
    if(NOT CUSTOM_FUNC_INTERFACE)
        # swd jtag cjtag
        set(CUSTOM_FUNC_INTERFACE "swd")
    endif()

    if(NOT CUSTOM_FUNC_OPENOCD_CONFIGFILE_LIST )
        set(CUSTOM_FUNC_OPENOCD_CONFIGFILE_LIST " ")
    endif()

    if(NOT CUSTOM_FUNC_ELF_NAME_LIST)
        message(FATAL_ERROR "No elf file name specified")
    endif()
    
    foreach(ELF_NAME ${CUSTOM_FUNC_ELF_NAME_LIST})
        list(APPEND ELF_PATH_LIST "${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME}")
        
    endforeach()
    

    if(NOT CMAKE_GDB)
        message(FATAL_ERROR "Please install ${CROSS_COMPILE_PREFIX}gdb")
    endif()
    
    if(NOT CMAKE_OBJDUMP)
        message(FATAL_ERROR "Please install ${CROSS_COMPILE_PREFIX}objdump")
    endif()

    
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/.${CMAKE_TARGET}_make_vscode_jlink_gnu_gdb.timestamp
        COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/mk_vscode_cortex_debug_gdb.py
            --config-name ${CMAKE_TARGET} --server-type ${CUSTOM_FUNC_SERVER_TYPE}  --top-path ${FLY_TOP_DIR} 
            --interface ${CUSTOM_FUNC_INTERFACE} --chip-name ${CUSTOM_FUNC_CHIP_NAME} --gdb-path "${CMAKE_GDB}" 
            --objdump-path "${CMAKE_OBJDUMP}" ${RTT_EN_FLAGS}  ${CUSTOM_FUNC_OTHER_EXT_CONFIG_OPTION} ${CUSTOM_FUNC_OTHER_EXT_CONFIG}
            --elf-path ${ELF_PATH_LIST} --openocd-config-file-list ${CUSTOM_FUNC_OPENOCD_CONFIGFILE_LIST} 
        DEPENDS ${CUSTOM_FUNC_DEPENDS}
        COMMENT "Make vscode jlink launch.json"
    )

    # 添加自定义目标来生成 img 文件
    add_custom_target(${CMAKE_TARGET}_make_vscode_jlink_gnu_gdb ALL
        DEPENDS ${CMAKE_BINARY_DIR}/.${CMAKE_TARGET}_make_vscode_jlink_gnu_gdb.timestamp
    )

endfunction()
