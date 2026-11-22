include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# GigaDevice GD32E23x Firmware Library V2.5.0.
fly_fetch_git_project(vendor-gd32e23x
    GIT_REPOSITORY "https://github.com/GigaDevice-GD32-MCU/GD32E23x_Firmware_Library.git"
    GIT_TAG "d9fd2491593dee4385bf3e844964c10d269b2af4"
    SOURCE_DIR_VAR gd32e23x_vendor_source
    PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
    SPARSE_PATHS "Firmware" "Template"
)
set(gd32e23x_vendor_dir "${gd32e23x_vendor_source}/Firmware")
# Use the vendor configuration header without exposing example headers such as main.h.
set(gd32e23x_libopt_dir "${CMAKE_CURRENT_BINARY_DIR}/include")
configure_file(
    "${gd32e23x_vendor_source}/Template/gd32e23x_libopt.h"
    "${gd32e23x_libopt_dir}/gd32e23x_libopt.h"
    COPYONLY
)

add_library(arch-gd32e23x OBJECT EXCLUDE_FROM_ALL)

target_include_directories(arch-gd32e23x PUBLIC
    "${gd32e23x_vendor_dir}/CMSIS/GD/GD32E23x/Include"
    "${gd32e23x_vendor_dir}/CMSIS"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Include"
    "${gd32e23x_libopt_dir}"
)
target_sources(arch-gd32e23x PRIVATE
    "${gd32e23x_vendor_dir}/CMSIS/GD/GD32E23x/Source/system_gd32e23x.c"
    "${gd32e23x_vendor_dir}/CMSIS/GD/GD32E23x/Source/GCC/startup_gd32e23x.S"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_fwdgt.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_pmu.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_misc.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_rcu.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_wwdgt.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_gpio.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_rtc.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_i2c.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_timer.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_spi.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_cmp.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_usart.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_adc.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_dma.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_fmc.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_syscfg.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_exti.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_dbg.c"
    "${gd32e23x_vendor_dir}/GD32E23x_standard_peripheral/Source/gd32e23x_crc.c"
)
