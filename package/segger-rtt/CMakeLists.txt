cmake_minimum_required(VERSION 3.10)


if(CONFIG_PACKAGE_SEGGER_RTT)

add_library(segger-rtt OBJECT)
target_sources(segger-rtt PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/SEGGER_RTT.c
)

target_include_directories(segger-rtt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc/")

endif(CONFIG_PACKAGE_SEGGER_RTT)