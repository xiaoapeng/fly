
include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/git-status.cmake")

function(add_jlink_image CMAKE_TARGET)
    cmake_parse_arguments(
        CUSTOM_FUNC ""
        "CHIP_NAME;DEPENDS;IMAGE_NAME;SPEED"
        "FIRMWARE_LIST"
        ""
        ${ARGN}
    )

    # 设置 SPEED 的默认值
    if(NOT CUSTOM_FUNC_SPEED)
        set(CUSTOM_FUNC_SPEED "4000")
    endif()

    add_custom_command(
        OUTPUT ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.make_jlink_img.timestamp
        COMMAND python3 ${FLY_TOP_DIR}/tool/python/mk_jlink_img.py 
            --firmware-name ${CUSTOM_FUNC_IMAGE_NAME}_${CMAKE_BUILD_TYPE} --chip-name ${CUSTOM_FUNC_CHIP_NAME} --win-jlink-path 
            ${FLY_TOP_DIR}/tool/win/jlink/  --output-dir ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/ --speed ${CUSTOM_FUNC_SPEED}
            ${CUSTOM_FUNC_FIRMWARE_LIST}
        COMMAND ${CMAKE_COMMAND} -E touch ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.make_jlink_img.timestamp
        DEPENDS ${CUSTOM_FUNC_DEPENDS} ${CMAKE_BINARY_DIR}/.gitstatus.timestamp
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Make a jlink burn package"
    )

    # 添加自定义目标来生成 img 文件
    add_custom_target(${CMAKE_TARGET}_make_jlink_img
        DEPENDS ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/.make_jlink_img.timestamp
    )

    if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND cd ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/CURRENT/ && ./burn.bat
            DEPENDS ${CMAKE_TARGET}_make_jlink_img
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    else()
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND cd ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/CURRENT/ && ./burn.sh
            DEPENDS ${CMAKE_TARGET}_make_jlink_img
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    endif()

    add_custom_target(${CMAKE_TARGET}_clean_img
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${FLY_TOP_DIR}/image/${CUSTOM_FUNC_IMAGE_NAME}/
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:clean img"
    )

endfunction()