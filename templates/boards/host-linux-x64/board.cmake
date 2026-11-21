set(FLY_BOARD_HOST TRUE)
set(FLY_BOARD_APP_SOURCE "${CMAKE_CURRENT_LIST_DIR}/app_main.c")

function(fly_configure_board target)
    if(NOT CONFIG_ARCH_X64_LINUX OR NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        message(FATAL_ERROR "This application requires a Linux x86-64 host and its original defconfig.")
    endif()
endfunction()
