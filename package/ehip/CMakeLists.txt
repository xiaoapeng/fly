cmake_minimum_required(VERSION 3.5)


if(CONFIG_PACKAGE_EHIP)

add_library(ehip OBJECT)
target_sources(ehip PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_core.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_buffer.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_netdev.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_netdev_tool.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_netdev_trait.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_protocol_handle.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip-netdev-class/ethernet_dev.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip-mac/ethernet.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip-mac/ethernet_v2.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip-ipv4/arp.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip-ipv4/ip.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip-ipv4/route.c"
)

target_include_directories(ehip PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/include/")
target_link_libraries(ehip PRIVATE eventhub)

endif(CONFIG_PACKAGE_EHIP)