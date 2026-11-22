include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/tool_chain/${CONFIG_ARCH_TOOLCHAIN}.cmake")
set(CMAKE_SYSTEM_PROCESSOR "${CONFIG_ARCH_CORE}")

function(_fly_configure_architecture)
    # project() may reload the previously configured CMake system information.
    set(CMAKE_SYSTEM_PROCESSOR "${CONFIG_ARCH_CORE}" PARENT_SCOPE)
    set(architecture_flags)
    if(CONFIG_ARCH_ISA STREQUAL "arm")
        list(APPEND architecture_flags "-mcpu=${CONFIG_ARCH_CORE}" -mthumb "-mfloat-abi=${CONFIG_ARCH_FLOAT_ABI}")
        if(CONFIG_ARCH_FPU)
            list(APPEND architecture_flags "-mfpu=${CONFIG_ARCH_FPU}")
        endif()
    elseif(CONFIG_ARCH_ISA STREQUAL "riscv")
        list(APPEND architecture_flags "-march=${CONFIG_ARCH_MARCH}" "-mabi=${CONFIG_ARCH_ABI}")
    endif()
    add_compile_options(${architecture_flags} ${DEMAND_COMPILER_OPTIONS})
    add_link_options(${architecture_flags} ${DEMAND_LINK_OPTIONS})
    add_definitions(${DEMAND_COMPILE_DEFINITIONS})
endfunction()

