
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

    set(image_dir "${FLY_IMAGE_DIR}/${CUSTOM_FUNC_IMAGE_NAME}")
    set(current_name "${CMAKE_TARGET}_jlink_CURRENT")
    set(image_cache "${image_dir}/.${CMAKE_TARGET}_package_key")
    set(packager "${FLY_TOP_DIR}/tool/python/mk_jlink_img.py")
    set(link_helper "${FLY_TOP_DIR}/tool/python/image_directory_link.py")
    set(image_args
        --firmware-name "${CUSTOM_FUNC_IMAGE_NAME}_${CMAKE_BUILD_TYPE}"
        --chip-name "${CUSTOM_FUNC_CHIP_NAME}" --interface-name "${CUSTOM_FUNC_INTERFACE}"
        --jlink-path "${FLY_TOP_DIR}/tool/jlink" --output-dir "${image_dir}"
        "--jtagconf=${CUSTOM_FUNC_JTAGCONF}" --speed "${CUSTOM_FUNC_SPEED}"
        --symlink-name "${current_name}" --firmware-parts ${CUSTOM_FUNC_FIRMWARE_LIST}
        --nm "${CMAKE_NM}"
    )
    # Packaging checks its input contents before reusing an image.
    add_custom_target(${CMAKE_TARGET}_make_img
        COMMAND "${Python3_EXECUTABLE}" "${packager}" ${image_args} --cache-file "${image_cache}"
        COMMAND "${Python3_EXECUTABLE}" "${link_helper}" "${FLY_IMAGE_DIR}/CURRENT" "${image_dir}/${current_name}"
        DEPENDS ${CUSTOM_FUNC_DEPENDS}
        VERBATIM
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Make a burn package"
    )

    if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND "${image_dir}/${current_name}/burn.bat" --nowait
            DEPENDS ${CMAKE_TARGET}_make_img
            VERBATIM
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    else()
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND "${image_dir}/${current_name}/burn.sh"
            DEPENDS ${CMAKE_TARGET}_make_img
            VERBATIM
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with JLink"
        )
    endif()

    add_custom_target(${CMAKE_TARGET}_clean_img
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${image_dir}"
        VERBATIM
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:clean img"
    )

endfunction()
