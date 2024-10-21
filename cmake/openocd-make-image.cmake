
include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/git-status.cmake")

function(add_openocd_image CMAKE_TARGET)
    cmake_parse_arguments(
        CUSTOM_FUNC ""
        "DEPENDS;IMAGE_NAME"
        "FIRMWARE_LIST;CONFIGFILE_LIST;RTT_ADDR_AND_SIZE"
        ""
        ${ARGN}
    )

    if(NOT CUSTOM_FUNC_CONFIGFILE_LIST )
        set(CUSTOM_FUNC_CONFIGFILE_LIST " ")
    endif()

    # 获取列表长度
    if( CUSTOM_FUNC_RTT_ADDR_AND_SIZE )
        list(APPEND FUNC_RTT_ADDR_AND_SIZE_OPTION "--rtt-setting")
        list(APPEND FUNC_RTT_ADDR_AND_SIZE_OPTION ${CUSTOM_FUNC_RTT_ADDR_AND_SIZE})

        list(LENGTH CUSTOM_FUNC_RTT_ADDR_AND_SIZE CUSTOM_FUNC_RTT_ADDR_AND_SIZE_LENGTH)

        if ( ${CUSTOM_FUNC_RTT_ADDR_AND_SIZE_LENGTH} LESS 2 )
            message(FATAL_ERROR "RTT_ADDR_AND_SIZE must be a list of two elements")
        endif()
    endif()

    add_custom_command(
        OUTPUT ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.${CMAKE_TARGET}_make_img.timestamp
        COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/mk_openocd_img.py 
            --firmware-name ${CUSTOM_FUNC_IMAGE_NAME}_${CMAKE_BUILD_TYPE} ${FUNC_RTT_ADDR_AND_SIZE_OPTION}
            --output-dir ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/ --config-file-list ${CUSTOM_FUNC_CONFIGFILE_LIST} 
            --symlink-name ${CMAKE_TARGET}_openocd_CURRENT --firmware-parts ${CUSTOM_FUNC_FIRMWARE_LIST} 
        COMMAND ${CMAKE_COMMAND} -E touch ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.${CMAKE_TARGET}_make_img.timestamp
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/${CMAKE_TARGET}_openocd_CURRENT ${FLY_TOP_DIR}/image/CURRENT
        DEPENDS ${CUSTOM_FUNC_DEPENDS} ${CMAKE_BINARY_DIR}/.gitstatus.timestamp 
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Make a openocd burn package"
    )

    # 添加自定义目标来生成 img 文件
    add_custom_target(${CMAKE_TARGET}_make_img
        DEPENDS check_git_status ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.${CMAKE_TARGET}_make_img.timestamp
    )

    if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/${CMAKE_TARGET}_openocd_CURRENT/burn.bat --nowait
            DEPENDS ${CMAKE_TARGET}_make_img
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    else()
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/${CMAKE_TARGET}_openocd_CURRENT/burn.sh
            DEPENDS ${CMAKE_TARGET}_make_img
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    endif()

    add_custom_target(${CMAKE_TARGET}_clean_img
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:clean img"
    )

endfunction()