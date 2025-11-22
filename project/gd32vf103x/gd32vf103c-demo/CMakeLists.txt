cmake_minimum_required(VERSION 3.10)


project(gd32vf103c-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)


target_include_directories(arch-gd32vf103c PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/board/inc/")
target_include_directories(eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/config-inc")

package_lib_compile_options("${TARGET_FLAGS}")

# app程序生成
add_executable(gd32vf103c-demo)
target_include_directories(gd32vf103c-demo PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/board/inc/")

target_link_libraries(gd32vf103c-demo  arch-gd32vf103c c )
target_link_package_lib(gd32vf103c-demo)

target_sources(gd32vf103c-demo PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/system_gd32vf103.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/led.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/button.c"
)

target_compile_options(gd32vf103c-demo PRIVATE "${TARGET_FLAGS}")
target_link_options(gd32vf103c-demo  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/GD32VF103xB.lds"
)

# 自动生成map
add_target_make_map(gd32vf103c-demo)
# 自动生成bin
add_target_make_bin(gd32vf103c-demo)
# 自动打印内存/存储占用
add_target_print_memory_usage(gd32vf103c-demo)

add_jlink_image( default
    IMAGE_NAME "GD32VF103C_DEMO"
    CHIP_NAME "GD32VF103CBT6"
    INTERFACE "jtag"
    SPEED "10000"
    FIRMWARE_LIST "APP:V0.0.1:0x08000000:gd32vf103c-demo.bin"
    DEPENDS gd32vf103c-demo
)

add_vscode_cortex_debug_gdb( jlink
    CHIP_NAME "GD32VF103CBT6"
    ELF_NAME_LIST "gd32vf103c-demo"
    SERVER_TYPE "jlink"
    INTERFACE "jtag"
    OTHER_EXT_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/debugconfig/ext-cortex-debug-config.json"
    RTT_DEBUG  ON
    DEPENDS gd32vf103c-demo
)
