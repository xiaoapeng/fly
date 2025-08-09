cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EVENTHUB_OS)

project(packge-eventhub-os)

include(FetchContent)

message("Fetching eventhub_os...")

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    packge-eventhub-os
    GIT_REPOSITORY https://github.com/xiaoapeng/eventhub_os.git
    GIT_TAG        main
    SOURCE_DIR     "${FLY_SOURCE_DL_BASE_DIR}/eventhub_os-src"
    BINARY_DIR     "${CMAKE_BINARY_DIR}/package_build/${PROJECT_NAME}"
)



FetchContent_MakeAvailable(packge-eventhub-os)

endif()
