cmake_minimum_required(VERSION 3.10)


if(CONFIG_FACTORY_DATA_SUPPORTED)

add_library(factory-data OBJECT)
target_sources(factory-data PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/factory-data.c
)

target_include_directories(factory-data PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc/")

# Some boards map application Flash at address zero.
target_compile_options(factory-data PRIVATE -fno-delete-null-pointer-checks)

fly_register_package_library(factory-data)
endif(CONFIG_FACTORY_DATA_SUPPORTED)
