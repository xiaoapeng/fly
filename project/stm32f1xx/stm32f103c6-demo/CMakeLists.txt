cmake_minimum_required(VERSION 3.10)

project(stm32f103c6-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)


target_include_directories(arch-stm32f103x6 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config-inc")

target_compile_definitions(segger-rtt PUBLIC 
    "-DBUFFER_SIZE_UP=512"
    #"-DSEGGER_RTT_MODE_DEFAULT=SEGGER_RTT_MODE_NO_BLOCK_TRIM"
)

# app程序生成
target_include_directories( eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/config-inc")
target_compile_options(eventhub PRIVATE "${TARGET_FLAGS}")


add_executable(stm32f103c6_demo_app )
target_link_libraries(stm32f103c6_demo_app arch-stm32f103x6 eventhub segger-rtt c m gcc  )
target_include_directories( stm32f103c6_demo_app PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/config-inc"
)

set(BOARD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board/system_stm32f1xx.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/clock_cnt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/led.c"
)

target_sources(stm32f103c6_demo_app PRIVATE 
    "${BOARD_SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/init.c"
)
target_compile_options(stm32f103c6_demo_app PRIVATE "${TARGET_FLAGS}")
target_link_options(stm32f103c6_demo_app  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/STM32F103C6Tx_FLASH.ld"
)
# 自动生成map
add_target_make_map(stm32f103c6_demo_app)
# 自动生成bin
add_target_make_bin(stm32f103c6_demo_app)
# 自动打印内存/存储占用
add_target_print_memory_usage(stm32f103c6_demo_app)

add_jlink_image( default
    IMAGE_NAME "STM32F103C6_DEMO_APP"
    CHIP_NAME "STM32F103C6"
    FIRMWARE_LIST "APP:V0.0.1:0x8000000:stm32f103c6_demo_app.bin"
    DEPENDS stm32f103c6_demo_app
)

add_vscode_cortex_debug_gdb( default
    CHIP_NAME "STM32F103C6"
    ELF_NAME_LIST "stm32f103c6_demo_app"
    SERVER_TYPE "jlink"
    RTT_DEBUG  ON
    DEPENDS stm32f103c6_demo_app
)