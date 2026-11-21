# Attach the SDK to the application after project() enables its languages.
if(FLY_TOOLCHAIN_SYSTEM_NAME)
    set(CMAKE_SYSTEM_NAME "${FLY_TOOLCHAIN_SYSTEM_NAME}")
endif()
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set(fly_project_rule_hashes "")
foreach(rule IN ITEMS CMakeLists.txt app/CMakeLists.txt board/board.cmake)
    if(EXISTS "${FLY_WORKSPACE_DIR}/${rule}")
        file(SHA256 "${FLY_WORKSPACE_DIR}/${rule}" rule_hash)
        string(APPEND fly_project_rule_hashes "${rule_hash}")
    endif()
endforeach()
set(FLY_PROJECT_RULES_HASH "${fly_project_rule_hashes}" CACHE INTERNAL "Application CMake rules")

include("${FLY_TOP_DIR}/cmake/env.cmake")
include("${FLY_TOP_DIR}/cmake/git-status.cmake")
include("${FLY_TOP_DIR}/cmake/jlink-make-image.cmake")
include("${FLY_TOP_DIR}/cmake/openocd-make-image.cmake")
include("${FLY_TOP_DIR}/cmake/vscode-cortex-debug-gdb.cmake")

arch_build_config()
include_directories("${FLY_CONFIG_DIR}")

add_subdirectory("${FLY_TOP_DIR}/arch" "${CMAKE_CURRENT_BINARY_DIR}/arch")
add_subdirectory("${FLY_TOP_DIR}/package" "${CMAKE_CURRENT_BINARY_DIR}/package")

add_custom_target(package_update_all DEPENDS ${FLY_ALL_PACKAGE_UPDATE_TARGETS_LIST})

set(MERGED_INFO_FILE "${CMAKE_BINARY_DIR}/package_build/all_package_info.txt")
file(WRITE "${MERGED_INFO_FILE}" "")
foreach(file IN LISTS FLY_ALL_PACKAGE_INFO_FILE_LIST)
    file(READ "${file}" CONTENT)
    file(APPEND "${MERGED_INFO_FILE}" "${CONTENT}\n")
endforeach()

add_custom_target(package_update_show_list
    COMMAND ${CMAKE_COMMAND} -E cat "${MERGED_INFO_FILE}"
    VERBATIM
)
