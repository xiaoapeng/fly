
include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/git-status.cmake")

function(add_jlink_image CMAKE_TARGET)
    cmake_parse_arguments(
        CUSTOM_FUNC ""
        "CHIP_NAME;IMAGE_NAME;SPEED;INTERFACE;JTAGCONF"
        "FIRMWARE_LIST;DEPENDS"
        ""
        ${ARGN}
    )

    # 设置 SPEED 的默认值
    if(NOT CUSTOM_FUNC_SPEED)
        set(CUSTOM_FUNC_SPEED "4000")
    endif()
    
    if(NOT CUSTOM_FUNC_INTERFACE)
        # swd jtag cjtag
        set(CUSTOM_FUNC_INTERFACE "swd")
    endif()

    if(NOT CUSTOM_FUNC_JTAGCONF)
        set(CUSTOM_FUNC_JTAGCONF "-1,-1")
    endif()
    
    add_custom_command(
        OUTPUT ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.${CMAKE_TARGET}_make_img.timestamp
        COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/mk_jlink_img.py 
            --firmware-name ${CUSTOM_FUNC_IMAGE_NAME}_${CMAKE_BUILD_TYPE} --chip-name ${CUSTOM_FUNC_CHIP_NAME} --interface-name ${CUSTOM_FUNC_INTERFACE}
            --win-jlink-path  ${FLY_TOP_DIR}/tool/win/jlink/  --output-dir ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/ --jtagconf=${CUSTOM_FUNC_JTAGCONF}
            --speed ${CUSTOM_FUNC_SPEED} --symlink-name ${CMAKE_TARGET}_jlink_CURRENT --firmware-parts ${CUSTOM_FUNC_FIRMWARE_LIST} 
        COMMAND ${CMAKE_COMMAND} -E touch ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.${CMAKE_TARGET}_make_img.timestamp
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/${CMAKE_TARGET}_jlink_CURRENT ${FLY_TOP_DIR}/image/CURRENT
        DEPENDS ${CUSTOM_FUNC_DEPENDS} ${CMAKE_BINARY_DIR}/.gitstatus.timestamp 
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Make a jlink burn package"
    )

    # 添加自定义目标来生成 img 文件
    add_custom_target(${CMAKE_TARGET}_make_img
        DEPENDS check_git_status ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.${CMAKE_TARGET}_make_img.timestamp
    )

    if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/${CMAKE_TARGET}_jlink_CURRENT/burn.bat --nowait
            DEPENDS ${CMAKE_TARGET}_make_img
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    else()
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/${CMAKE_TARGET}_jlink_CURRENT/burn.sh
            DEPENDS ${CMAKE_TARGET}_make_img
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    endif()

    add_custom_target(${CMAKE_TARGET}_clean_img
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:clean img"
    )

endfunction()