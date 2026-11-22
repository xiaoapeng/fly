include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../stm32-device.cmake")

_fly_fetch_stm32_cmsis(stm32_cmsis_include_dir)

# ST's CMSIS Device F7 v1.2.10 supplies headers and unmodified GCC startup.
fly_fetch_git_project(stm32-cmsis-device-f7
    GIT_REPOSITORY "https://github.com/STMicroelectronics/cmsis-device-f7.git"
    GIT_TAG "2352e888e821aa0f4fe549bd5ea81d29c67a3222"
    SOURCE_DIR_VAR stm32f7_device_dir
)

function(_fly_add_stm32f7_device target device_macro startup)
    add_library(${target} OBJECT EXCLUDE_FROM_ALL)
    target_compile_definitions(${target} PUBLIC "${device_macro}")
    target_include_directories(${target} PUBLIC
        "${stm32f7_device_dir}/Include"
        "${stm32_cmsis_include_dir}"
    )
    target_sources(${target} PRIVATE
        "${stm32f7_device_dir}/Source/Templates/gcc/${startup}"
        "${stm32f7_device_dir}/Source/Templates/system_stm32f7xx.c"
    )
    set_property(TARGET ${target} PROPERTY FLY_STM32_DEVICE_HEADER "stm32f7xx.h")
endfunction()

if(CONFIG_ARCH_PROFILE STREQUAL "cm7-sp")
    _fly_add_stm32f7_device(arch-stm32f746xx STM32F746xx startup_stm32f746xx.s)
elseif(CONFIG_ARCH_PROFILE STREQUAL "cm7-dp")
    _fly_add_stm32f7_device(arch-stm32f767xx STM32F767xx startup_stm32f767xx.s)
endif()
