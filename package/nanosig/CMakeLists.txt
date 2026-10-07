cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_NANOSIG)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

if(NOT CONFIG_PACKAGE_NANOSIG_GIT_COMMIT)
    message(FATAL_ERROR "PACKAGE_NANOSIG_GIT_COMMIT unresolved — add a default to package/nanosig/Kconfig")
endif()

message("Fetching nanosig @ ${CONFIG_PACKAGE_NANOSIG_GIT_COMMIT}...")

fetch_git_project(
    nanosig
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/nanosig.git
    GIT_TAG        "${CONFIG_PACKAGE_NANOSIG_GIT_COMMIT}"
)

# FLY compiles the upstream core directly instead of adding upstream's CMake
# target (same pattern as package/scheduler/freertos), so Kconfig drives the
# cross build and the FreeRTOS SDK headers come from package/scheduler/
# freertos. NANOSIG_PLATFORM_FREERTOS below selects the upstream backend.
set(NANOSIG_SRC_DIR "${FLY_SOURCE_DL_BASE_DIR}/nanosig-src")

# Upstream generates include/nanosig/nanosig_version.h from a template via
# its own CMakeLists (which we deliberately do not add). Reproduce it here:
# the header must live under a nanosig/ subdir (upstream includes it as
# <nanosig/nanosig_version.h>), and the template's @PROJECT_VERSION*@ tokens
# are mirrored from upstream's `project(nanosig VERSION 0.1.0)`.
set(NANOSIG_VERSION_DIR "${CMAKE_CURRENT_BINARY_DIR}/nanosig-version/include")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION 0.1.0)
file(MAKE_DIRECTORY "${NANOSIG_VERSION_DIR}/nanosig")
configure_file(
    "${NANOSIG_SRC_DIR}/include/nanosig/nanosig_version.h.in"
    "${NANOSIG_VERSION_DIR}/nanosig/nanosig_version.h"
    @ONLY
)

add_library(nanosig
    "${NANOSIG_SRC_DIR}/src/nanosig.c"
    "${NANOSIG_SRC_DIR}/src/ns_broker.c"
    "${NANOSIG_SRC_DIR}/src/ns_timer.c"
    "${NANOSIG_SRC_DIR}/src/ns_debug.c"
    "${NANOSIG_SRC_DIR}/src/ns_formatio.c"
    "${NANOSIG_SRC_DIR}/src/ds/ns_mpsc_record_ring.c"
    "${NANOSIG_SRC_DIR}/src/ds/ns_ringbuf.c"
    "${NANOSIG_SRC_DIR}/src/ds/ns_hashtbl.c"
    "${NANOSIG_SRC_DIR}/src/ds/ns_rbtree.c"
    "${NANOSIG_SRC_DIR}/platform/freertos/port.c"
)

target_include_directories(nanosig PUBLIC
    "${NANOSIG_SRC_DIR}/include"
    "${NANOSIG_VERSION_DIR}"
)

# Select the upstream FreeRTOS queue-set platform backend. PUBLIC so the
# consuming project (e.g. the demo app) expands nanosig_port.h on the same
# platform branch and links the matching ABI probe symbol.
target_compile_definitions(nanosig PUBLIC
    NANOSIG_PLATFORM_FREERTOS=1
)

# Global debug level from Kconfig (CONFIG_PACKAGE_NANOSIG_LOG_LEVEL maps to
# NS_DBG_* numeric values). This seeds ns_debug.c's `dbg_level` default; the
# value can still be overridden at runtime via ns_dbg_set_level(). Per-module
# compile-time levels (NS_DBG_MODULE_LEVEL_<MOD>) are orthogonal and can be
# supplied from the consuming project's CMakeLists via
# target_compile_definitions(nanosig PRIVATE NS_DBG_MODULE_LEVEL_<MOD>=...).
target_compile_definitions(nanosig PRIVATE
    NS_CONFIG_DEFAULT_DEBUG_LEVEL=${CONFIG_PACKAGE_NANOSIG_LOG_LEVEL}
)

# The FreeRTOS backend port.c needs the kernel headers provided by the
# freertos-config INTERFACE chain (package/scheduler/freertos).
target_link_libraries(nanosig PUBLIC freertos-config)

package_lib_add(nanosig)

endif()
