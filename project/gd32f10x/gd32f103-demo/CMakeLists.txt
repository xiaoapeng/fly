cmake_minimum_required(VERSION 3.10)


project(gd32f103-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)

target_compile_definitions(arch-gd32f10x PRIVATE -DGD32F10X_HD )

# app程序生成


add_executable(gd32f103-demo )
target_link_libraries(gd32f103-demo  arch-gd32f10x c m gcc  )

target_sources(gd32f103-demo PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/boot/main.c"
)
target_compile_options(gd32f103-demo PRIVATE "${TARGET_FLAGS}")
target_link_options(gd32f103-demo  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/boot/gd32f10x_flash.ld"
)

# 自动生成map
add_target_make_map(gd32f103-demo)
# 自动生成bin
add_target_make_bin(gd32f103-demo)
# 自动打印内存/存储占用
add_target_print_memory_usage(gd32f103-demo)

add_jlink_image( default
    IMAGE_NAME "GD32F103_DEMO"
    CHIP_NAME "GD32F103C8"
    FIRMWARE_LIST "BOOT:V0.0.1:0x08000000:gd32f103-demo.bin"
    DEPENDS gd32f103-demo
)