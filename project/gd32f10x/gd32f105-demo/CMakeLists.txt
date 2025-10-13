cmake_minimum_required(VERSION 3.10)


project(gd32f105-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
    "$<$<CONFIG:Debug>:-g;-O0>"
    "$<$<CONFIG:Release>:-g;-O3>"
    "$<$<CONFIG:MinSizeRel>:-g;-Os>"
)

target_compile_definitions(arch-gd32f10x PUBLIC -DGD32F10X_CL -DHXTAL_VALUE=8000000U )

target_include_directories( eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/inc")
target_compile_options(eventhub PRIVATE "${TARGET_FLAGS}")

target_compile_options(segger-rtt PRIVATE 
    "-DBUFFER_SIZE_UP=4096"
)

# app程序生成
add_executable(gd32f105-demo )
target_link_libraries(gd32f105-demo eventhub segger-rtt arch-gd32f10x c m gcc  )

target_sources(gd32f105-demo PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/clock_cnt.c"
)

target_compile_options(gd32f105-demo PRIVATE "${TARGET_FLAGS}")
target_link_options(gd32f105-demo  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/src/gd32f10x_flash.ld"
)

# 自动生成map
add_target_make_map(gd32f105-demo)
# 自动生成bin
add_target_make_bin(gd32f105-demo)
# 自动打印内存/存储占用
add_target_print_memory_usage(gd32f105-demo)

add_jlink_image( default
    IMAGE_NAME "GD32F105_DEMO"
    CHIP_NAME "GD32F105VC"
    FIRMWARE_LIST "BOOT:V0.0.1:0x08000000:gd32f105-demo.bin"
    DEPENDS gd32f105-demo
)

add_vscode_cortex_debug_gdb( default
    CHIP_NAME "GD32F105VC"
    ELF_NAME_LIST "gd32f105-demo"
    SERVER_TYPE "jlink"
    RTT_DEBUG  OFF
    DEPENDS gd32f105-demo
)