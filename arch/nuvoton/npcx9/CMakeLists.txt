include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# CMSIS 5.9.0 provides the generic Cortex-M4 GCC reset and exception vectors.
fly_fetch_git_project(arm-cmsis5-cortex-m4-startup
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
    GIT_TAG 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c
    SPARSE_PATHS Device/ARM/ARMCM4/Source/GCC
    SOURCE_DIR_VAR cmsis_dir
)

add_library(arch-npcx9m6f OBJECT EXCLUDE_FROM_ALL)
target_sources(arch-npcx9m6f PRIVATE
    "${cmsis_dir}/Device/ARM/ARMCM4/Source/GCC/startup_ARMCM4.S"
    "${CMAKE_CURRENT_LIST_DIR}/system.c"
)
