cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_SCHEDULER_FREERTOS)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

if(NOT CONFIG_PACKAGE_FREERTOS_GIT_TAG)
    message(FATAL_ERROR "PACKAGE_FREERTOS_GIT_TAG unresolved — add a default to package/scheduler/freertos/Kconfig")
endif()

message("Fetching FreeRTOS-Kernel @ ${CONFIG_PACKAGE_FREERTOS_GIT_TAG}...")

fly_fetch_git_project(
    freertos-kernel
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG        "${CONFIG_PACKAGE_FREERTOS_GIT_TAG}"
    SOURCE_DIR_VAR FREERTOS_SRC_DIR
)

# Select the FreeRTOS port from generic architecture facts. The package owns
# this mapping; architecture Kconfig only supplies hardware/startup facts.
set(FREERTOS_PORT_SOURCES port.c)
set(FREERTOS_PORT_GLOBAL_DEFS)
set(FREERTOS_PORT_VECTOR_DEFS)
if(CONFIG_ARCH_CORE STREQUAL "cortex-m33")
    if(CONFIG_ARCH_TRUSTZONE_MODE STREQUAL "non-secure")
        set(FREERTOS_PORT_DIR "portable/GCC/ARM_CM33_NTZ/non_secure")
    else()
        message(FATAL_ERROR "FreeRTOS: unsupported Cortex-M33 security mode: ${CONFIG_ARCH_TRUSTZONE_MODE}")
    endif()
    set(FREERTOS_PORT_GLOBAL_DEFS _ARMV8M_LM_PRESENT _ARMV8M_MAINLINE_PRESENT)
    list(APPEND FREERTOS_PORT_SOURCES portasm.c)
elseif(CONFIG_ARCH_CORE STREQUAL "cortex-m0")
    set(FREERTOS_PORT_DIR "portable/GCC/ARM_CM0")
    list(APPEND FREERTOS_PORT_SOURCES portasm.c)
elseif(CONFIG_ARCH_CORE STREQUAL "cortex-m3" OR CONFIG_ARCH_CORE STREQUAL "cortex-m4")
    if(CONFIG_ARCH_CORE STREQUAL "cortex-m3")
        set(FREERTOS_PORT_DIR "portable/GCC/ARM_CM3")
    elseif(CONFIG_ARCH_HAS_HARD_FPU AND CONFIG_ARCH_FLOAT_ABI STREQUAL "hard")
        set(FREERTOS_PORT_DIR "portable/GCC/ARM_CM4F")
    else()
        message(FATAL_ERROR "FreeRTOS: Cortex-M4 requires the hard-FPU ARM_CM4F port in v1")
    endif()
    set(FREERTOS_PORT_VECTOR_DEFS
        vPortSVCHandler=SVC_Handler
        xPortPendSVHandler=PendSV_Handler
        xPortSysTickHandler=SysTick_Handler
    )
else()
    message(FATAL_ERROR "FreeRTOS: unsupported generic architecture core: ${CONFIG_ARCH_CORE}")
endif()

set(FREERTOS_HEAP_FILE "heap_4.c")
if(CONFIG_PACKAGE_FREERTOS_HEAP_HEAP1)
    set(FREERTOS_HEAP_FILE "heap_1.c")
elseif(CONFIG_PACKAGE_FREERTOS_HEAP_HEAP2)
    set(FREERTOS_HEAP_FILE "heap_2.c")
elseif(CONFIG_PACKAGE_FREERTOS_HEAP_HEAP3)
    set(FREERTOS_HEAP_FILE "heap_3.c")
elseif(CONFIG_PACKAGE_FREERTOS_HEAP_HEAP5)
    set(FREERTOS_HEAP_FILE "heap_5.c")
endif()

# 1) config (INTERFACE). Include ORDER matters for the OBJECT libs below:
#    package inc/ (FreeRTOSConfig.h) BEFORE kernel include/ (portable headers)
#    so the packaged template wins; no other directory may shadow it for
#    these libs. This target is not passed to fly_register_package_library() because
#    fly_add_package_compile_options() applies
#    `target_compile_options(<lib> PRIVATE ...)` to every registered lib
#    (cmake/package.cmake), which is a hard configure error for INTERFACE
#    targets. The executable reaches it through the PUBLIC chain
#    (freertos-port -> freertos-kernel -> freertos-heap -> freertos-config)
#    instead; it carries only INTERFACE include directories and compile
#    definitions, so skipping the strict global flag pass is acceptable.
add_library(freertos-config INTERFACE)
target_include_directories(freertos-config INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/inc"                       # FreeRTOSConfig.h template (authoritative)
    "${FREERTOS_SRC_DIR}/include"                           # tasks.h, queue.h,...
    "${FREERTOS_SRC_DIR}/${FREERTOS_PORT_DIR}"              # portmacro.h
)
target_compile_definitions(freertos-config INTERFACE ${FREERTOS_PORT_GLOBAL_DEFS})

# 2) heap (OBJECT)
add_library(freertos-heap OBJECT "${FREERTOS_SRC_DIR}/portable/MemMang/${FREERTOS_HEAP_FILE}")

# 3) kernel (OBJECT). timers.c is compiled only when USE_TIMERS=y; the
# source set is fixed at configure time. FreeRTOS-Kernel V11.1.0 still
# ships message_buffer APIs inside stream_buffer.c; later tags split
# message_buffer.c out, so update the source list if the pin changes.
set(FREERTOS_KERNEL_SOURCES
    "${FREERTOS_SRC_DIR}/tasks.c"
    "${FREERTOS_SRC_DIR}/queue.c"
    "${FREERTOS_SRC_DIR}/list.c"
    "${FREERTOS_SRC_DIR}/event_groups.c"
    "${FREERTOS_SRC_DIR}/stream_buffer.c"
)
if(CONFIG_PACKAGE_FREERTOS_USE_TIMERS)
    list(APPEND FREERTOS_KERNEL_SOURCES "${FREERTOS_SRC_DIR}/timers.c")
endif()
add_library(freertos-kernel OBJECT ${FREERTOS_KERNEL_SOURCES})

# Default configASSERT handler (weak). Projects override it with their own
# strong vAssertCalled() if they want a different policy (e.g. printing the
# assert location over a debug backend they own). The default keeps the
# package free of any debug-output dependency.
add_library(freertos-assert OBJECT
    "${CMAKE_CURRENT_SOURCE_DIR}/freertos_assert.c"
)

# 4) port (OBJECT)
list(TRANSFORM FREERTOS_PORT_SOURCES PREPEND "${FREERTOS_SRC_DIR}/${FREERTOS_PORT_DIR}/")
add_library(freertos-port OBJECT ${FREERTOS_PORT_SOURCES})
target_compile_definitions(freertos-port PRIVATE ${FREERTOS_PORT_VECTOR_DEFS})

target_link_libraries(freertos-heap   PUBLIC freertos-config)
target_link_libraries(freertos-kernel PUBLIC freertos-heap freertos-config)
target_link_libraries(freertos-port   PUBLIC freertos-kernel freertos-heap freertos-config)
target_link_libraries(freertos-assert PUBLIC freertos-kernel freertos-heap freertos-config)

set_property(TARGET freertos-heap freertos-kernel freertos-port
    PROPERTY FLY_EXCLUDED_PACKAGE_COMPILE_OPTIONS -Wconversion -Wsign-conversion)

# Register only the OBJECT libs. freertos-config is an INTERFACE target
# and must not enter FLY_ALL_PACKAGE_LIB_LIST because
# fly_add_package_compile_options() would apply PRIVATE compile options,
# which is a hard configure error for INTERFACE targets.
fly_register_package_library(freertos-heap)
fly_register_package_library(freertos-kernel)
fly_register_package_library(freertos-port)
fly_register_package_library(freertos-assert)

endif()
