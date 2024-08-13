cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(FLY_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 加载配置 .config
include("${FLY_TOP_DIR}/auto-generate/.config.cmake")

include("${FLY_TOP_DIR}/arch/${CONFIG_ARCH}/gcc-config.cmake")
include("${FLY_TOP_DIR}/cmake/arch-config.cmake")
include("${FLY_TOP_DIR}/cmake/gcc-make-output.cmake")
include("${FLY_TOP_DIR}/cmake/git-status.cmake")
include("${FLY_TOP_DIR}/cmake/jlink.cmake")

project(fly LANGUAGES C CXX ASM)

arch_build_config()

add_compile_options(
    "$<$<CONFIG:Debug>:-g;-O0>"
    "$<$<CONFIG:Release>:-g;-O3>"
    "$<$<CONFIG:MinSizeRel>:-g;-Os>"
)

include_directories("${FLY_TOP_DIR}/auto-generate/")

add_subdirectory("${FLY_TOP_DIR}/arch/" "${CMAKE_CURRENT_BINARY_DIR}/arch/")
add_subdirectory("${FLY_TOP_DIR}/package/" "${CMAKE_CURRENT_BINARY_DIR}/package/")
add_subdirectory("${FLY_TOP_DIR}/project/${CONFIG_PROJECT}")


