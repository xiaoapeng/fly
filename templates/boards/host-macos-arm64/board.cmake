set(FLY_BOARD_HOST TRUE)
set(FLY_BOARD_APP_SOURCE "${CMAKE_CURRENT_LIST_DIR}/app_main.c")

function(fly_configure_board target)
    if(NOT CONFIG_ARCH_ARM64_MACOS OR NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        message(FATAL_ERROR "This application requires a macOS Arm64 host and its original defconfig.")
    endif()
endfunction()
