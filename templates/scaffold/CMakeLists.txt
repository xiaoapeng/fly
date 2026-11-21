cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-sdk.cmake")
include("${FLY_SDK_DIR}/cmake/fly-configure.cmake")
project("@PROJECT_NAME@" LANGUAGES C CXX ASM)
include("${FLY_SDK_DIR}/cmake/fly-sdk.cmake")

set(FLY_FIRMWARE_NAME "@PROJECT_NAME@")
set(FLY_FIRMWARE_VERSION "0.1.0")

if(NOT FLY_FIRMWARE_NAME MATCHES "^[A-Za-z0-9][A-Za-z0-9_-]*$")
    message(FATAL_ERROR "Use letters, digits, hyphens or underscores for FLY_FIRMWARE_NAME.")
endif()
if(NOT FLY_FIRMWARE_VERSION MATCHES "^([0-9]+)[.]([0-9]+)[.]([0-9]+)$"
        OR CMAKE_MATCH_1 GREATER 255 OR CMAKE_MATCH_2 GREATER 255 OR CMAKE_MATCH_3 GREATER 255)
    message(FATAL_ERROR "FLY_FIRMWARE_VERSION must be major.minor.patch, with each number between 0 and 255.")
endif()

include("${FLY_WORKSPACE_DIR}/board/board.cmake")
add_executable(fly_app)
app_add_sdk_check(fly_app)
fly_configure_board(fly_app)
add_subdirectory(app)
target_link_package_lib(fly_app)

if(CONFIG_FACTORY_DATA_SUPPORTED AND NOT FLY_BOARD_HOST)
    if(NOT DEFINED FLY_FLASH_ADDRESS OR NOT FLY_FLASH_SIZE)
        message(FATAL_ERROR "Factory data requires the board's FLY_FLASH_ADDRESS and FLY_FLASH_SIZE.")
    endif()
    target_compile_definitions(fly_app PRIVATE
        FLY_FLASH_ADDRESS=${FLY_FLASH_ADDRESS}
        FLY_FLASH_SIZE=${FLY_FLASH_SIZE}
    )
endif()

set(app_warnings -Wall -Wextra -Wconversion -Wsign-conversion -Wno-psabi)
target_compile_options(fly_app PRIVATE ${app_warnings})
package_lib_compile_options("${app_warnings}")

add_target_make_map(fly_app)
if(NOT FLY_BOARD_HOST)
    add_target_make_bin(fly_app)
    add_target_print_memory_usage(fly_app)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-image.cmake")
app_add_jlink_image(default
    IMAGE_NAME "${FLY_FIRMWARE_NAME}"
    FIRMWARE_LIST
        "APP:${FLY_FIRMWARE_VERSION}:${FLY_FLASH_ADDRESS}:${FLY_FLASH_SIZE}:fly_app.bin"
        # "BOOT:${BOOT_VERSION}:${BOOT_FLASH_ADDRESS}:${BOOT_FLASH_SIZE}:${BOOT_BIN}"
    DEPENDS fly_app
)

include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-vscode.cmake")
app_add_vscode_debug(fly_app)
