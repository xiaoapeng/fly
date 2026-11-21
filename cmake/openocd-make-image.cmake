
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

    set(image_dir "${FLY_IMAGE_DIR}/${CUSTOM_FUNC_IMAGE_NAME}")
    set(current_name "${CMAKE_TARGET}_openocd_CURRENT")
    set(image_cache "${image_dir}/.${CMAKE_TARGET}_package_key")
    set(packager "${FLY_TOP_DIR}/tool/python/mk_openocd_img.py")
    set(link_helper "${FLY_TOP_DIR}/tool/python/image_directory_link.py")
    set(image_args
        --firmware-name "${CUSTOM_FUNC_IMAGE_NAME}_${CMAKE_BUILD_TYPE}" ${FUNC_RTT_ADDR_AND_SIZE_OPTION}
        --output-dir "${image_dir}" --config-file-list ${CUSTOM_FUNC_CONFIGFILE_LIST}
        --symlink-name "${current_name}" --firmware-parts ${CUSTOM_FUNC_FIRMWARE_LIST}
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
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with OpenOCD"
        )
    else()
        # 添加自定义目标 自动烧写
        add_custom_target(${CMAKE_TARGET}_flash
            COMMAND "${image_dir}/${current_name}/burn.sh"
            DEPENDS ${CMAKE_TARGET}_make_img
            VERBATIM
            COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:Flashing the device with OpenOCD"
        )
    endif()

    add_custom_target(${CMAKE_TARGET}_clean_img
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${image_dir}"
        VERBATIM
        COMMENT "${CUSTOM_FUNC_IMAGE_NAME}:clean img"
    )

endfunction()
