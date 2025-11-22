cmake_minimum_required(VERSION 3.10)


if(CONFIG_FACTORY_DATA_SUPPORTED)

add_library(factory-data OBJECT)
target_sources(factory-data PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/factory-data.c
)

target_include_directories(factory-data PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc/")

package_lib_add(factory-data)
endif(CONFIG_FACTORY_DATA_SUPPORTED)