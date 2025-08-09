cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EHIP)

project(packge-ehip)

include(FetchContent)

message("Fetching ehip...")

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    packge-ehip
    GIT_REPOSITORY https://github.com/xiaoapeng/ehip.git
    GIT_TAG        master
    SOURCE_DIR     "${FLY_SOURCE_DL_BASE_DIR}/ehip-src"
    BINARY_DIR     "${CMAKE_BINARY_DIR}/package_build/${PROJECT_NAME}"
)



FetchContent_MakeAvailable(packge-ehip)

endif()
