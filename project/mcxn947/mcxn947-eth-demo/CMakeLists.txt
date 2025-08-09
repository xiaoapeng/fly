cmake_minimum_required(VERSION 3.10)

project(mcxn947-eth-demo LANGUAGES C CXX ASM)

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

# app程序生成
target_include_directories( eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/app/inc")
target_compile_options(eventhub PRIVATE "${TARGET_FLAGS}")
target_compile_options(ehip PRIVATE "${TARGET_FLAGS}")


target_compile_definitions(eventhub PUBLIC
    "-DEH_CONFIG_DEFAULT_DEBUG_LEVEL=EH_DBG_DEBUG"
    "-DEH_DBG_MODEULE_LEVEL_MEM_POOL=EH_DBG_WARNING"
)

target_compile_definitions(ehip PUBLIC
    "-DEHIP_NETDEV_TYPE_GENERAL_POOL_BUFFER_NUM=28"
    "-DEHIP_NETDEV_POLL_BUFFER_MAX_NUM=32"
    "-DEHIP_CORE_MBOX_NETDEV_RX_MSG_BUFFER_NUM=16"
    "-DEHIP_CORE_MBOX_NETDEV_TX_MSG_BUFFER_NUM=16"
    "-DEHIP_IP_MAX_FRAGMENT_NUM=16"
    "-DEH_DBG_MODEULE_LEVEL_UDP_INPUT=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_LAN8741_XMIT=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_ETHERNET_V2=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_IP_REASSE=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_IP_INPUT=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_IP_TX=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_ARP=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_PING_REQUEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_RX_FRAGMENT=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_ICMP_ERROR_INPUT=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TCP=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TCP_INPUT=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TCP_CWND=EH_DBG_WARNING"
)

target_compile_options(segger-rtt PRIVATE 
    "-DBUFFER_SIZE_UP=16384"
)

add_executable(mcxn947_eth_demo_app )
target_link_libraries(mcxn947_eth_demo_app arch-mcxn947-core0 eventhub ehip segger-rtt c rdimon m gcc  )
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
    "${CMAKE_CURRENT_SOURCE_DIR}/app/static_ip_setting.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/test.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/task_main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/ip_message_test.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/udp_test.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/ping_test.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/tcp_test_send.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/tcp_test_recv.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/tcp_test_echo.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/traceroute_test.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/app/run.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/app/mpu-burn-mcu-task.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/app/mpu_general_server.c"
)

target_compile_options(mcxn947_eth_demo_app PRIVATE "${TARGET_FLAGS}")
target_link_options(mcxn947_eth_demo_app  PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/MCXN947_cm33_core0_flash.ld"
)

target_compile_definitions(mcxn947_eth_demo_app PRIVATE
    "-DEH_DBG_MODEULE_LEVEL_ROUTE_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_UDP_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_PING_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TRACEROUTE_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TCP_TEST_ECHO=EH_DBG_WARNING"
    
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