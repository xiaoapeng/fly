cmake_minimum_required(VERSION 3.5)


if(CONFIG_PACKAGE_EHIP)

add_library(ehip OBJECT)
target_sources(ehip PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_core.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_buffer.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_netdev.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip_netdev_trait.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/ehip-netdev-class/ethernet_dev.c"
)

target_include_directories(ehip PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/include/")
target_link_libraries(ehip PRIVATE eventhub)

endif(CONFIG_PACKAGE_EHIP)