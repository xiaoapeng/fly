cmake_minimum_required(VERSION 3.10)

project(stm32h750vb-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)


target_include_directories(arch-stm32h750xx PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config-inc")

target_compile_definitions(segger-rtt PUBLIC 
    "-DBUFFER_SIZE_UP=512"
    #"-DSEGGER_RTT_MODE_DEFAULT=SEGGER_RTT_MODE_NO_BLOCK_TRIM"
)

# app程序生成
target_include_directories( eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/config-inc")
target_compile_options(eventhub PRIVATE "${TARGET_FLAGS}")


add_executable(stm32h750vb_demo_app )
target_link_libraries(stm32h750vb_demo_app factory-data arch-stm32h750xx eventhub segger-rtt c m gcc  )
target_include_directories( stm32h750vb_demo_app PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/config-inc"
)

set(BOARD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board/system_stm32h7xx.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/clock_cnt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/led.c"
)

target_sources(stm32h750vb_demo_app PRIVATE 
    "${BOARD_SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/init.c"
)
target_compile_options(stm32h750vb_demo_app PRIVATE "${TARGET_FLAGS}")
target_link_options(stm32h750vb_demo_app  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/STM32H750VBTx_FLASH.ld"
)
# 自动生成map
add_target_make_map(stm32h750vb_demo_app)
# 自动生成bin
add_target_make_bin(stm32h750vb_demo_app)
# 自动打印内存/存储占用
add_target_print_memory_usage(stm32h750vb_demo_app)

add_jlink_image( default
    IMAGE_NAME "STM32H750VB_DEMO_APP"
    CHIP_NAME "STM32H750VB"
    SPEED "10000"
    FIRMWARE_LIST "APP:V0.0.1:0x8000000:stm32h750vb_demo_app.bin"
    DEPENDS stm32h750vb_demo_app
)

add_vscode_cortex_debug_gdb( default
    CHIP_NAME "STM32H750VB"
    ELF_NAME_LIST "stm32h750vb_demo_app"
    SERVER_TYPE "jlink"
    RTT_DEBUG  ON
    DEPENDS stm32h750vb_demo_app
)