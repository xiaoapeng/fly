include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# Silicon Labs Simplicity SDK 2025.6, with the startup and common headers
# pulled from their original Git tree.
fly_fetch_git_project(silabs-simplicity-mg26
    GIT_REPOSITORY https://github.com/SiliconLabs/simplicity_sdk.git
    GIT_TAG b41bec3ff2485199c1a5a9995b3e649e118c1b8d
    SPARSE_PATHS platform/Device/SiliconLabs/EFR32MG26 platform/common/inc
    SOURCE_DIR_VAR sisdk
)

fly_fetch_git_project(arm-cmsis5
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
    GIT_TAG 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c
    SOURCE_DIR_VAR cmsis
)

set(device_dir "${sisdk}/platform/Device/SiliconLabs/EFR32MG26")
foreach(part IN ITEMS
    efr32mg26b410f3200im48
    efr32mg26b410f3200im68
    efr32mg26b420f3200im48
    efr32mg26b420f3200im68
    efr32mg26b510f3200il136
)
    string(TOUPPER "${part}" device_macro)
    set(target "arch-${part}")
    add_library("${target}" OBJECT EXCLUDE_FROM_ALL)
    target_compile_definitions("${target}" PUBLIC "${device_macro}" PRIVATE SL_LEGACY_LINKER)
    target_include_directories("${target}" SYSTEM PUBLIC
        "${device_dir}/Include"
        "${cmsis}/CMSIS/Core/Include"
        "${sisdk}/platform/common/inc"
    )
    target_sources("${target}" PRIVATE
        "${device_dir}/Source/startup_efr32mg26.c"
        "${device_dir}/Source/system_efr32mg26.c"
    )
endforeach()
