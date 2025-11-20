cmake_minimum_required(VERSION 3.10)

project(mcxn947-eth-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS 
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
    "$<$<CONFIG:Debug>:-g;-O0;-fprofile-arcs;-ftest-coverage;-fsanitize=address,leak>"
    "$<$<CONFIG:Release>:-g;-O3>"
    "$<$<CONFIG:MinSizeRel>:-g;-Os>"
    "$<$<CONFIG:RelWithDebInfo>:-g;-Os>"
)

# app程序生成
target_compile_options(eventhub PRIVATE "${TARGET_FLAGS}")
target_compile_options(ehip PRIVATE "${TARGET_FLAGS}")
target_compile_options(ehshell PRIVATE "${TARGET_FLAGS}")
target_compile_options(argparse PRIVATE "${TARGET_FLAGS}")


target_compile_definitions(eventhub PUBLIC
    "-DEH_CONFIG_DEFAULT_DEBUG_LEVEL=EH_DBG_DEBUG"
    "-DEH_DBG_MODEULE_LEVEL_EPOL_HUB=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_MEM_POOL=EH_DBG_WARNING"
    "-DEH_CONFIG_EVENT_CB_DISPATCH_CNT_PER_YIELD=65535"
    "-DEH_CONFIG_EVENT_CB_DISPATCH_CNT_PER_CHECKTIMER=1"
    "-DEH_CONFIG_DEBUG_ENTER_SIGN=\"\\r\\n\""
)

target_compile_definitions(ehip PUBLIC
    "-DEHIP_NETDEV_TYPE_GENERAL_POOL_BUFFER_NUM=28"
    "-DEHIP_NETDEV_POLL_BUFFER_MAX_NUM=32"
    "-DEHIP_CORE_MBOX_NETDEV_RX_MSG_BUFFER_NUM=16"
    "-DEHIP_CORE_MBOX_NETDEV_TX_MSG_BUFFER_NUM=16"
    "-DEHIP_IP_MAX_FRAGMENT_NUM=16"
    "-DEHIP_MAX_PACKET_PROCESS_NUM=16"
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
    "-DEH_DBG_MODEULE_LEVEL_TCP_RETRY=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TCP_RTO_TIMEOUT=EH_DBG_WARNING"
)

add_executable(ehip_test )
target_compile_options(ehip_test PRIVATE "${TARGET_FLAGS}")
target_link_libraries(ehip_test eventhub ehshell ehip argparse c m gcc  )
target_link_options(ehip_test PRIVATE 
    "$<$<CONFIG:Debug>:-fprofile-arcs;-ftest-coverage;-fsanitize=address,leak>"
)
target_include_directories( ehip_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/inc"
)

set(BOARD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board/linux_tap.c"
)


target_sources(ehip_test PRIVATE 
    "${BOARD_SOURCES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/run.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/static_ip_setting.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tty_io.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/test_cmd.c"
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/test_ehshell_escape_test_parse.c"
)

target_compile_options(ehip_test PRIVATE "${TARGET_FLAGS}")

target_compile_definitions(ehip_test PRIVATE
    "-DEH_DBG_MODEULE_LEVEL_ROUTE_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_UDP_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_PING_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TRACEROUTE_TEST=EH_DBG_WARNING"
    "-DEH_DBG_MODEULE_LEVEL_TCP_TEST_ECHO=EH_DBG_WARNING"
    
)
# 自动生成map
add_target_make_map(ehip_test)
