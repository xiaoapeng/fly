cmake_minimum_required(VERSION 3.5)

project(mcxn947-eth-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)

# app程序生成
target_include_directories( eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/app/inc")
target_compile_options(eventhub PRIVATE "${TARGET_FLAGS}")
target_compile_options(ehip PRIVATE "${TARGET_FLAGS}")


target_compile_definitions(eventhub PUBLIC
    "-DEH_CONFIG_DEFAULT_DEBUG_LEVEL=EH_DBG_DEBUG"
    #"-DEH_CONFIG_DEFAULT_DEBUG_LEVEL=EH_DBG_INFO"
)

target_compile_definitions(ehip PUBLIC
    "-DEHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_NUM=28" 
    "-DEHIP_ARP_DEBUG=0"
    "-DEHIP_IP_DEBUG=1"
    "-DEHIP_ETHERNET_V2_DEBUG=0"
    "-DEHIP_IP_MAX_FRAGMENT_NUM=16"
)

target_compile_options(segger-rtt PRIVATE 
    "-DBUFFER_SIZE_UP=20480"
)

add_executable(mcxn947_eth_demo_app )
target_link_libraries(mcxn947_eth_demo_app  arch-mcxn947-core0 eventhub ehip segger-rtt c rdimon m gcc  )
target_include_directories( mcxn947_eth_demo_app PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/board/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/general/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/board"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtt/inc"
)

set(BOARD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board/clock_config.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/debug.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/clock_cnt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/button.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/led.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/eth-lan8741a-en.c"
)


target_sources(mcxn947_eth_demo_app PRIVATE 
    "${BOARD_SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/test.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/task_main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/ip_message_test.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/app/run.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/app/mpu-burn-mcu-task.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/app/mpu_general_server.c"
)

target_compile_options(mcxn947_eth_demo_app PRIVATE "${TARGET_FLAGS}")
target_link_options(mcxn947_eth_demo_app  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/MCXN947_cm33_core0_flash.ld"
)
# 自动生成map
add_target_make_map(mcxn947_eth_demo_app)
# 自动生成bin
add_target_make_bin(mcxn947_eth_demo_app)
# 自动打印内存/存储占用
add_target_print_memory_usage(mcxn947_eth_demo_app)

add_jlink_image( default
    IMAGE_NAME "MCXN947_ETH_DEMO_APP"
    CHIP_NAME "MCXN947_M33_0"
    FIRMWARE_LIST "APP:V0.0.1:0x00000000:mcxn947_eth_demo_app.bin"
    DEPENDS mcxn947_eth_demo_app
)

add_vscode_cortex_debug_gdb( default
    CHIP_NAME "MCXN947_M33_0"
    ELF_NAME_LIST "mcxn947_eth_demo_app"
    SERVER_TYPE "jlink"
    RTT_DEBUG  ON
    DEPENDS mcxn947_eth_demo_app
)