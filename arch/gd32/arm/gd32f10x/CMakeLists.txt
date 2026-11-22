include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# GigaDevice GD32F10x Firmware Library V2.7.0.
fly_fetch_git_project(vendor-gd32f10x
    GIT_REPOSITORY "https://github.com/GigaDevice-GD32-MCU/GD32F10x_Firmware_Library.git"
    GIT_TAG "23a80f96368336b4d47014da33cd9b7ba550165e"
    SOURCE_DIR_VAR gd32f10x_vendor_source
    PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
    SPARSE_PATHS
        "Firmware/CMSIS"
        "Firmware/GD32F10x_standard_peripheral"
        "Template"
)
set(gd32f10x_vendor_dir "${gd32f10x_vendor_source}/Firmware")

set(gd32_cmsis_dir "${gd32f10x_vendor_dir}/CMSIS")
set(gd32_device_dir "${gd32_cmsis_dir}/GD/GD32F10x")
set(gd32_driver_dir "${gd32f10x_vendor_dir}/GD32F10x_standard_peripheral")
set(gd32_libopt_dir "${CMAKE_CURRENT_BINARY_DIR}/include")
configure_file("${gd32f10x_vendor_source}/Template/gd32f10x_libopt.h"
    "${gd32_libopt_dir}/gd32f10x_libopt.h" COPYONLY)

set(gd32_driver_sources
    i2c dac pmu gpio sdio fwdgt exti rtc timer dbg enet rcu
    dma spi crc exmc adc can wwdgt fmc usart misc bkp
)
list(TRANSFORM gd32_driver_sources PREPEND "${gd32_driver_dir}/Source/gd32f10x_")
list(TRANSFORM gd32_driver_sources APPEND ".c")

# Each board selects a target so the device macro and startup vector table
# always match. Drivers share their sources and compile for the selected variant.
foreach(variant IN ITEMS md hd xd cl)
    set(target "arch-gd32f10x-${variant}")
    string(TOUPPER "${variant}" device_variant)
    add_library(${target} OBJECT EXCLUDE_FROM_ALL)
    target_compile_definitions(${target} PUBLIC USE_STDPERIPH_DRIVER "GD32F10X_${device_variant}")
    target_include_directories(${target} PUBLIC
        "${gd32_device_dir}/Include"
        "${gd32_cmsis_dir}"
        "${gd32_driver_dir}/Include"
        "${gd32_libopt_dir}"
    )
    target_sources(${target} PRIVATE
        "${gd32_device_dir}/Source/system_gd32f10x.c"
        # The upstream library ships ARMCC and IAR startup files.
        "${CMAKE_CURRENT_SOURCE_DIR}/gcc/startup_gd32f10x_${variant}.S"
        ${gd32_driver_sources}
    )
endforeach()
