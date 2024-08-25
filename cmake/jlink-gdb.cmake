include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/git-status.cmake")

function(add_vscode_jlink_gdb CMAKE_TARGET)
    cmake_parse_arguments(
        CUSTOM_FUNC ""
        "CHIP_NAME;DEPENDS;ELF_NAME;SPEED"
        ""
        ""
        ${ARGN}
    )

    # 设置 SPEED 的默认值
    if(NOT CUSTOM_FUNC_SPEED)
        set(CUSTOM_FUNC_SPEED "4000")
    endif()
    
    if(NOT CMAKE_GDB)
        message(FATAL_ERROR "Please install gdb-multiarch")
    endif()
    
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/.${CMAKE_TARGET}_make_vscode_jlink_gnu_gdb.timestamp
        COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/mk_vscode_jlink_gnu_gdb.py 
            --config-name ${CMAKE_TARGET}
            --top-path ${FLY_TOP_DIR} --chip-name ${CUSTOM_FUNC_CHIP_NAME} --gdb-path ${CMAKE_GDB}
            --elf-path ${CMAKE_CURRENT_BINARY_DIR}/${CUSTOM_FUNC_ELF_NAME} --speed ${CUSTOM_FUNC_SPEED}
        COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/.${CMAKE_TARGET}_make_vscode_jlink_gnu_gdb.timestamp
        DEPENDS ${CUSTOM_FUNC_DEPENDS} ${CMAKE_BINARY_DIR}/.gitstatus.timestamp
        COMMENT "${CUSTOM_FUNC_ELF_NAME}:Make vscode jlink gnu gdb"
    )

    # 添加自定义目标来生成 img 文件
    add_custom_target(${CMAKE_TARGET}_make_vscode_jlink_gnu_gdb ALL
        DEPENDS ${CMAKE_BINARY_DIR}/.${CMAKE_TARGET}_make_vscode_jlink_gnu_gdb.timestamp
    )

endfunction()
