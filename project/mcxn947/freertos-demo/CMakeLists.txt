cmake_minimum_required(VERSION 3.10)

project(mcxn947-freertos-demo LANGUAGES C CXX ASM)

set(TARGET_FLAGS
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-Wno-psabi"
)

# NOTE (C8): do NOT copy mcxn947-demo's
#   target_include_directories(eventhub PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/app/inc")
# line — the `eventhub` target does not exist in a FreeRTOS config.

# Strict flags on ALL REGISTERED package libs (incl. freertos-kernel/heap/port).
# freertos-config is INTERFACE + UNREGISTERED (NEW-1) so it is exempt from the
# global flag pass — INTERFACE targets cannot take PRIVATE compile options.
package_lib_compile_options("${TARGET_FLAGS}")

# Upstream exemption AFTER the global pass (later flags win): kernel/heap/port
# trigger -Wconversion/-Wsign-conversion on official FreeRTOS sources.
target_compile_options(freertos-kernel PRIVATE -Wno-conversion -Wno-sign-conversion)
target_compile_options(freertos-heap    PRIVATE -Wno-conversion -Wno-sign-conversion)
target_compile_options(freertos-port    PRIVATE -Wno-conversion -Wno-sign-conversion)

add_executable(mcxn947_freertos_demo_app)
target_link_libraries(mcxn947_freertos_demo_app
    arch-mcxn947-core0 c rdimon m gcc)
target_link_package_lib(mcxn947_freertos_demo_app)

target_compile_options(mcxn947_freertos_demo_app PRIVATE "${TARGET_FLAGS}")

# V-6 invariant (C7): no include dir contains a FreeRTOSConfig.h — the packaged
# template at package/scheduler/freertos/inc/ is the only one and reaches the OBJECT libs
# via the freertos-config INTERFACE chain (kernel include → portmacro.h).
target_include_directories(mcxn947_freertos_demo_app PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/board/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/board"
)

set(BOARD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board/clock_config.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/pin_mux.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/debug.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/led.c"
)

target_sources(mcxn947_freertos_demo_app PRIVATE
    ${BOARD_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/app/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/task_main.c"
)

target_link_options(mcxn947_freertos_demo_app PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/MCXN947_cm33_core0_flash.ld"
)

add_target_make_map(mcxn947_freertos_demo_app)
add_target_make_bin(mcxn947_freertos_demo_app)
add_target_print_memory_usage(mcxn947_freertos_demo_app)

add_jlink_image( default
    IMAGE_NAME "MCXN947_FREERTOS_DEMO_APP"
    CHIP_NAME "MCXN947_M33_0"
    FIRMWARE_LIST "APP:V0.0.1:0x00000000:mcxn947_freertos_demo_app.bin"
    DEPENDS mcxn947_freertos_demo_app
)

add_vscode_cortex_debug_gdb( default
    CHIP_NAME "MCXN947_M33_0"
    ELF_NAME_LIST "mcxn947_freertos_demo_app"
    SERVER_TYPE "jlink"
    RTT_DEBUG  ON
    DEPENDS mcxn947_freertos_demo_app
)