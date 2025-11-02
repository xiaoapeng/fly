cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(FLY_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include("${FLY_TOP_DIR}/cmake/env.cmake")
# 加载配置 .config
include("${FLY_TOP_DIR}/auto-generate/.config.cmake")

include("${FLY_TOP_DIR}/arch/${CONFIG_ARCH}/gcc-config.cmake")
include("${FLY_TOP_DIR}/cmake/python3.cmake")
include("${FLY_TOP_DIR}/cmake/gcc-tool-chain-general.cmake")
include("${FLY_TOP_DIR}/cmake/arch-config.cmake")
include("${FLY_TOP_DIR}/cmake/git-status.cmake")
include("${FLY_TOP_DIR}/cmake/jlink-make-image.cmake")
include("${FLY_TOP_DIR}/cmake/openocd-make-image.cmake")
include("${FLY_TOP_DIR}/cmake/vscode-cortex-debug-gdb.cmake")

project(fly LANGUAGES C CXX ASM)

arch_build_config()

include_directories("${FLY_TOP_DIR}/auto-generate/")

add_subdirectory("${FLY_TOP_DIR}/arch/" "${CMAKE_CURRENT_BINARY_DIR}/arch/")
add_subdirectory("${FLY_TOP_DIR}/package/" "${CMAKE_CURRENT_BINARY_DIR}/package/")
add_subdirectory("${FLY_TOP_DIR}/project/${CONFIG_PROJECT}")

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


