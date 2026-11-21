if(NOT FLY_FIRMWARE_NAME MATCHES "^[A-Za-z0-9][A-Za-z0-9_-]*$")
    message(FATAL_ERROR "Use letters, digits, hyphens or underscores for FLY_FIRMWARE_NAME.")
endif()
if(NOT FLY_FIRMWARE_VERSION MATCHES "^([0-9]+)[.]([0-9]+)[.]([0-9]+)$"
        OR CMAKE_MATCH_1 GREATER 255 OR CMAKE_MATCH_2 GREATER 255 OR CMAKE_MATCH_3 GREATER 255)
    message(FATAL_ERROR "FLY_FIRMWARE_VERSION must be major.minor.patch, with each number between 0 and 255.")
endif()

include("${FLY_WORKSPACE_DIR}/board/board.cmake")

if(NOT FLY_BOARD_APP_SOURCE)
    set(FLY_BOARD_APP_SOURCE main.c)
endif()
add_executable(fly_app "${FLY_BOARD_APP_SOURCE}")
fly_configure_board(fly_app)
include("${CMAKE_CURRENT_SOURCE_DIR}/configure.cmake" OPTIONAL)
target_link_package_lib(fly_app)

set(app_warnings -Wall -Wextra -Wconversion -Wsign-conversion -Wno-psabi)
target_compile_options(fly_app PRIVATE ${app_warnings})
package_lib_compile_options("${app_warnings}")
foreach(upstream_target nanosig freertos-kernel freertos-heap freertos-port)
    if(TARGET ${upstream_target})
        target_compile_options(${upstream_target} PRIVATE -Wno-conversion -Wno-sign-conversion)
    endif()
endforeach()

add_target_make_map(fly_app)
if(NOT FLY_BOARD_HOST)
    add_target_make_bin(fly_app)
    add_target_print_memory_usage(fly_app)
endif()

if(FLY_JLINK_DEVICE AND DEFINED FLY_FLASH_ADDRESS)
    add_jlink_image(default
        IMAGE_NAME "${FLY_FIRMWARE_NAME}"
        CHIP_NAME "${FLY_JLINK_DEVICE}"
        FIRMWARE_LIST "APP:${FLY_FIRMWARE_VERSION}:${FLY_FLASH_ADDRESS}:fly_app.bin"
        DEPENDS fly_app
    )
endif()
if(FLY_JLINK_DEVICE AND CMAKE_GDB AND CMAKE_OBJDUMP)
    set(flash_debug_args "")
    if(TARGET default_flash)
        set(flash_debug_args FLASH_TASK "FLY: Flash")
    endif()
    add_vscode_cortex_debug_gdb(default
        CHIP_NAME "${FLY_JLINK_DEVICE}"
        ELF_NAME_LIST fly_app
        SERVER_TYPE jlink
        RTT_DEBUG "${CONFIG_PACKAGE_SEGGER_RTT}"
        DEPENDS fly_app
        ${flash_debug_args}
    )
endif()
