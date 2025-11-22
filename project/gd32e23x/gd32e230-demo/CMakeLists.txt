cmake_minimum_required(VERSION 3.10)


project(gd32e230-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)

package_lib_compile_options("${TARGET_FLAGS}")

# app程序生成
add_executable(gd32e230-demo )
target_link_libraries(gd32e230-demo arch-gd32e23x c m gcc )
target_link_package_lib(gd32e230-demo)
target_sources(gd32e230-demo PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.c"
)
target_compile_options(gd32e230-demo PRIVATE "${TARGET_FLAGS}")
target_link_options(gd32e230-demo  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/gd32e23xc8t6_flash.ld"
)

# 自动生成map
add_target_make_map(gd32e230-demo)
# 自动生成bin
add_target_make_bin(gd32e230-demo)
# 自动打印内存/存储占用
add_target_print_memory_usage(gd32e230-demo)

add_jlink_image( default
    IMAGE_NAME "GD32E230_DEMO"
    CHIP_NAME "GD32E230C8"
    FIRMWARE_LIST "APP:V0.0.1:0x08000000:gd32e230-demo.bin"
    DEPENDS gd32e230-demo
)

add_vscode_cortex_debug_gdb( default
    CHIP_NAME "GD32E230C8"
    ELF_NAME_LIST "gd32e230-demo"
    SERVER_TYPE "jlink"
    RTT_DEBUG  ON
    DEPENDS gd32e230-demo
)