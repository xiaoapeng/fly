include_guard(GLOBAL)

# FIRMWARE_LIST uses NAME:VERSION:ADDRESS:CAPACITY:BIN. Each SYMBOLS pair names
# a partition and the executable target passed to fly_add_target_bin().
function(fly_add_firmware_image target)
    cmake_parse_arguments(PARSE_ARGV 1 image "" "IMAGE_NAME;BOARD_ID;CHIP;CORE_ID;BOARD_MANIFEST"
        "FIRMWARE_LIST;SYMBOLS;EXTERNAL_SYMBOLS;DEPENDS")

    if(image_UNPARSED_ARGUMENTS OR image_KEYWORDS_MISSING_VALUES OR NOT image_IMAGE_NAME
        OR NOT image_BOARD_ID OR NOT image_CHIP OR NOT image_CORE_ID OR NOT image_FIRMWARE_LIST)
        message(FATAL_ERROR "${target}: incomplete firmware image declaration")
    endif()

    set(args
        --project-root "${CMAKE_SOURCE_DIR}"
        --target "${target}" --build-type "$<CONFIG>"
        --board-id "${image_BOARD_ID}" --chip "${image_CHIP}" --core-id "${image_CORE_ID}"
        --firmware-name "${image_IMAGE_NAME}"
        --firmware-parts ${image_FIRMWARE_LIST})
    set(dependencies ${image_DEPENDS})
    if(image_BOARD_MANIFEST)
        list(APPEND args --board-manifest "${image_BOARD_MANIFEST}")
    endif()
    if(image_SYMBOLS OR image_EXTERNAL_SYMBOLS)
        if(NOT CMAKE_OBJCOPY)
            message(FATAL_ERROR "${target}: firmware symbol packaging requires a toolchain objcopy")
        endif()
        list(APPEND args --objcopy "${CMAKE_OBJCOPY}")
    endif()
    while(image_SYMBOLS)
        list(LENGTH image_SYMBOLS remaining)
        if(remaining LESS 2)
            message(FATAL_ERROR "${target}: SYMBOLS expects partition and executable target pairs")
        endif()
        list(POP_FRONT image_SYMBOLS partition executable)
        if(NOT TARGET "${executable}")
            message(FATAL_ERROR "${target}: symbol target ${executable} does not exist")
        endif()
        get_target_property(build_dir "${executable}" BINARY_DIR)
        list(APPEND args --symbols "${partition}" "$<TARGET_FILE:${executable}>" "${build_dir}/${executable}.debug")
        list(APPEND dependencies "${executable}")
    endwhile()
    while(image_EXTERNAL_SYMBOLS)
        list(LENGTH image_EXTERNAL_SYMBOLS remaining)
        if(remaining LESS 3)
            message(FATAL_ERROR "${target}: EXTERNAL_SYMBOLS expects partition, ELF and debug file triples")
        endif()
        list(POP_FRONT image_EXTERNAL_SYMBOLS partition elf debug_file)
        list(APPEND args --external-symbols "${partition}" "${elf}" "${debug_file}")
    endwhile()

    add_custom_target(${target}_make_img
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../tool/python/firmware_payload.py" ${args}
        DEPENDS ${dependencies}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Packaging ${target} firmware"
        VERBATIM)
endfunction()
