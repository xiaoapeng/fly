cmake_minimum_required(VERSION 3.5)

project(ch582-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)


target_include_directories(arch-ch58x PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/config-inc")

target_compile_definitions(segger-rtt PUBLIC 
    "-DBUFFER_SIZE_UP=512"
    #"-DSEGGER_RTT_MODE_DEFAULT=SEGGER_RTT_MODE_NO_BLOCK_TRIM"
)

# app程序生成
# target_include_directories( eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/config-inc")
# target_compile_options(eventhub PRIVATE "${TARGET_FLAGS}")


add_executable(ch582_demo_app )
target_link_libraries(ch582_demo_app arch-ch58x segger-rtt c m gcc  )
target_include_directories( ch582_demo_app PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/config-inc"
)

set(BOARD_SOURCES ""
)

target_sources(ch582_demo_app PRIVATE 
    "${BOARD_SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.c"
)
target_compile_options(ch582_demo_app PRIVATE "${TARGET_FLAGS}")
target_link_options(ch582_demo_app  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/Link.ld"
)
# 自动生成map
add_target_make_map(ch582_demo_app)
# 自动生成bin
add_target_make_bin(ch582_demo_app)
# 自动打印内存/存储占用
add_target_print_memory_usage(ch582_demo_app)

# add_jlink_image( default
#     IMAGE_NAME "STM32F030X8_DEMO_APP"
#     CHIP_NAME "STM32F030C8"
#     FIRMWARE_LIST "APP:V0.0.1:0x8000000:ch582_demo_app.bin"
#     DEPENDS ch582_demo_app
# )