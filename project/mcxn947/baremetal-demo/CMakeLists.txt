cmake_minimum_required(VERSION 3.10)
project(mcxn947-baremetal-demo LANGUAGES C CXX ASM)

if(NOT CONFIG_PACKAGE_SCHEDULER_NONE OR
   NOT CONFIG_PACKAGE_NANOSIG OR NOT CONFIG_PACKAGE_NSBTN)
    message(FATAL_ERROR "baremetal-demo requires scheduler=None, nanosig and nsbtn; load its defconfig")
endif()

set(TARGET_FLAGS
    "-Wall" "-Wextra" "-Wconversion" "-Wsign-conversion" "-Wno-psabi")
package_lib_compile_options("${TARGET_FLAGS}")
target_compile_options(nanosig PRIVATE -Wno-conversion -Wno-sign-conversion)

add_executable(mcxn947_baremetal_demo_app)
target_link_libraries(mcxn947_baremetal_demo_app
    arch-mcxn947-core0 c rdimon m gcc)
target_link_package_lib(mcxn947_baremetal_demo_app)
target_compile_options(mcxn947_baremetal_demo_app PRIVATE "${TARGET_FLAGS}")
target_include_directories(mcxn947_baremetal_demo_app PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/app/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/board/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/board")
target_sources(mcxn947_baremetal_demo_app PRIVATE
    board/clock_config.c board/pin_mux.c board/init.c board/debug.c board/led.c
    app/main.c app/runtime.c app/key.c app/test.c)
target_link_options(mcxn947_baremetal_demo_app PRIVATE
    "-T${CMAKE_CURRENT_SOURCE_DIR}/app/MCXN947_cm33_core0_flash.ld"
    "-Wl,--defsym=__stack_size__=0x1000")

add_target_make_map(mcxn947_baremetal_demo_app)
add_target_make_bin(mcxn947_baremetal_demo_app)
add_target_print_memory_usage(mcxn947_baremetal_demo_app)
add_jlink_image(default
    IMAGE_NAME "MCXN947_BAREMETAL_DEMO_APP"
    CHIP_NAME "MCXN947_M33_0"
    FIRMWARE_LIST "APP:V0.0.1:0x00000000:mcxn947_baremetal_demo_app.bin"
    DEPENDS mcxn947_baremetal_demo_app)
add_vscode_cortex_debug_gdb(default
    CHIP_NAME "MCXN947_M33_0"
    ELF_NAME_LIST "mcxn947_baremetal_demo_app"
    SERVER_TYPE "jlink"
    RTT_DEBUG ON
    DEPENDS mcxn947_baremetal_demo_app)
