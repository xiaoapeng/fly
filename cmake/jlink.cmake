
include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/git-status.cmake")

function(add_jlink_image IMAGE_NAME)
    cmake_parse_arguments(
        CUSTOM_FUNC ""
        "CHIP_NAME;DEPENDS"
        "FIRMWARE_LIST"
        ""
        ${ARGN}
    )

    add_custom_command(
        OUTPUT ${FLY_TOP_DIR}/image/${IMAGE_NAME}/make_jlink_img.timestamp
        COMMAND python3 ${FLY_TOP_DIR}/tool/python/mk_jlink_img.py 
            --firmware-name ${IMAGE_NAME}_${CMAKE_BUILD_TYPE} --chip-name ${CUSTOM_FUNC_CHIP_NAME} --win-jlink-path 
            ${FLY_TOP_DIR}/tool/win/jlink/  --output-dir ${FLY_TOP_DIR}/image/${IMAGE_NAME}/
            ${CUSTOM_FUNC_FIRMWARE_LIST}
        COMMAND ${CMAKE_COMMAND} -E touch ${FLY_TOP_DIR}/image/.${IMAGE_NAME}_make_jlink_img.timestamp
        DEPENDS ${CUSTOM_FUNC_DEPENDS} ${CMAKE_BINARY_DIR}/.gitstatus.timestamp
        COMMENT "${IMAGE_NAME}:Make a jlink burn package"
    )

    # 添加自定义目标来生成 img 文件
    add_custom_target(${IMAGE_NAME}_make_jlink_img
        DEPENDS ${FLY_TOP_DIR}/image/${IMAGE_NAME}/make_jlink_img.timestamp
    )

    # 添加自定义目标 自动烧写
    add_custom_target(${IMAGE_NAME}_wsl_flash
        COMMAND cd ${FLY_TOP_DIR}/image/${IMAGE_NAME}/CURRENT/ && ./wsl_burn.sh
        DEPENDS ${IMAGE_NAME}_make_jlink_img
        COMMENT "${IMAGE_NAME}:Flashing the device with JLink"
    )

    add_custom_target(${IMAGE_NAME}_clean_img
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${FLY_TOP_DIR}/image/${IMAGE_NAME}/
        COMMENT "${IMAGE_NAME}:clean img"
    )

endfunction()