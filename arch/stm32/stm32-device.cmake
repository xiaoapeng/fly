include_guard(GLOBAL)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# The STM32 families use the Core headers from the CMSIS 5.4.0 release.
function(_fly_fetch_stm32_cmsis output_variable)
    fly_fetch_git_project(arm-cmsis5
        GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
        GIT_TAG 0b521765067ac87b142cd96b5f578ffb399090cc
        SPARSE_PATHS CMSIS/Core/Include
        SOURCE_DIR_VAR cmsis_source
    )
    set(${output_variable} "${cmsis_source}/CMSIS/Core/Include" PARENT_SCOPE)
endfunction()

function(_fly_fetch_stm32_sources family device_commit hal_commit)
    _fly_fetch_stm32_cmsis(cmsis_include)
    fly_fetch_git_project(cmsis-device-${family}
        GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-${family}.git
        GIT_TAG "${device_commit}"
        SOURCE_DIR_VAR device_source
    )
    fly_fetch_git_project(stm32${family}xx-hal-driver
        GIT_REPOSITORY https://github.com/STMicroelectronics/stm32${family}xx-hal-driver.git
        GIT_TAG "${hal_commit}"
        SOURCE_DIR_VAR hal_source
    )
    set(STM32_CMSIS_INCLUDE "${cmsis_include}" PARENT_SCOPE)
    set(STM32_DEVICE_DIR "${device_source}" PARENT_SCOPE)
    set(STM32_HAL_DIR "${hal_source}" PARENT_SCOPE)
endfunction()

# Each family supplies its vendor paths and shared source lists.
function(_fly_add_stm32_device target device_macro startup)
    get_filename_component(startup_source "${startup}" ABSOLUTE BASE_DIR "${STM32_DEVICE_TEMPLATES}/gcc")
    add_library(${target} OBJECT EXCLUDE_FROM_ALL)
    target_compile_definitions(${target} PUBLIC USE_FULL_LL_DRIVER USE_HAL_DRIVER ${device_macro} ${ARGN})
    target_include_directories(${target} PUBLIC ${STM32_INCLUDE_DIRS})
    target_sources(${target} PRIVATE
        "${startup_source}"
        "${STM32_DEVICE_TEMPLATES}/${STM32_SYSTEM_SOURCE}"
        ${STM32_LL_SOURCES}
        ${STM32_HAL_SOURCES}
    )
    set_property(TARGET ${target} PROPERTY FLY_STM32_DEVICE_HEADER "${STM32_DEVICE_HEADER}")
endfunction()
