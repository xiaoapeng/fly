include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# nrfx v3.12.0 has BSD-3-Clause device headers and Apache-2.0 startup parts.
fly_fetch_git_project(nordic-nrfx
    GIT_REPOSITORY https://github.com/nordicsemi/nrfx.git
    GIT_TAG v3.12.0
    SOURCE_DIR_VAR nrfx
)
# CMSIS 5.9.0 provides the Apache-2.0 Arm Cortex-M4 core headers.
fly_fetch_git_project(arm-cmsis5
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
    GIT_TAG 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c
    SOURCE_DIR_VAR cmsis
)

# Verify the immutable revisions independently of a moving release tag.
function(_fly_check_nrf52_source_revision directory expected)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${directory}" rev-parse HEAD
        OUTPUT_VARIABLE revision
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    if(NOT revision STREQUAL expected)
        message(FATAL_ERROR "Unexpected revision in ${directory}: ${revision}, expected ${expected}")
    endif()
endfunction()

_fly_check_nrf52_source_revision("${nrfx}" db85149b1871aa09e8ff8ed240a177e8406ead58)
_fly_check_nrf52_source_revision("${cmsis}" 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c)

add_library(arch-nrf52840 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-nrf52840 PUBLIC NRF52840_XXAA)
target_include_directories(arch-nrf52840 SYSTEM PUBLIC
    "${nrfx}/mdk"
    "${cmsis}/CMSIS/Core/Include"
)
target_sources(arch-nrf52840 PRIVATE
    "${nrfx}/mdk/gcc_startup_nrf52840.S"
    "${nrfx}/mdk/system_nrf52840.c"
)
target_link_directories(arch-nrf52840 INTERFACE "${nrfx}/mdk")
set_property(TARGET arch-nrf52840 PROPERTY INTERFACE_LINK_DEPENDS "${nrfx}/mdk/nrf_common.ld")
