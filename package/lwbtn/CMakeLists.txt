cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_LWBTN)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

if(NOT CONFIG_PACKAGE_LWBTN_GIT_COMMIT)
    message(FATAL_ERROR "PACKAGE_LWBTN_GIT_COMMIT unresolved — add a default to package/lwbtn/Kconfig")
endif()

message("Fetching lwbtn @ ${CONFIG_PACKAGE_LWBTN_GIT_COMMIT}...")

fetch_git_project(
    lwbtn
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/MaJerle/lwbtn.git
    GIT_TAG        "${CONFIG_PACKAGE_LWBTN_GIT_COMMIT}"
)

# FLY compiles the upstream single source file directly instead of adding
# upstream's CMake target, so Kconfig stays the only configuration entry.
set(LWBTN_SRC_DIR "${FLY_SOURCE_DL_BASE_DIR}/lwbtn-src")

add_library(lwbtn "${LWBTN_SRC_DIR}/lwbtn/src/lwbtn/lwbtn.c")

target_include_directories(lwbtn PUBLIC
    "${LWBTN_SRC_DIR}/lwbtn/src/include"
    # Provides the default lwbtn_opts.h; lwbtn_opt.h falls back to its
    # #ifndef defaults when it is not found, so this must stay in the list.
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Kconfig bools land in auto-generate/.config.cmake as `y`, ints as bare
# numbers: bools need the if()/1-0 conversion, ints pass straight through.
if(CONFIG_PACKAGE_LWBTN_USE_KEEPALIVE)
    set(LWBTN_CFG_USE_KEEPALIVE 1)
else()
    set(LWBTN_CFG_USE_KEEPALIVE 0)
endif()

if(CONFIG_PACKAGE_LWBTN_USE_CLICK)
    set(LWBTN_CFG_USE_CLICK 1)
else()
    set(LWBTN_CFG_USE_CLICK 0)
endif()

if(CONFIG_PACKAGE_LWBTN_CLICK_SEND_IMMEDIATELY)
    set(LWBTN_CFG_CLICK_SEND_IMMEDIATELY 1)
else()
    set(LWBTN_CFG_CLICK_SEND_IMMEDIATELY 0)
endif()

# Upstream lwbtn.c references btn->click / LWBTN_EVT_ONCLICK inside
# `#if !LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY` (lwbtn.c:200-210)
# without a USE_CLICK guard, while struct lwbtn_btn has no click member when
# click support is off. Forcing 1 there keeps that block compiled out, so
# USE_CLICK=n builds. (No click code exists in that configuration anyway.)
if(NOT CONFIG_PACKAGE_LWBTN_USE_CLICK)
    set(LWBTN_CFG_CLICK_SEND_IMMEDIATELY 1)
endif()

if(CONFIG_PACKAGE_LWBTN_CLICK_KEEP_AFTER_SHORT_PRESS)
    set(LWBTN_CFG_CLICK_KEEP_AFTER_SHORT_PRESS 1)
else()
    set(LWBTN_CFG_CLICK_KEEP_AFTER_SHORT_PRESS 0)
endif()

# PUBLIC: struct lwbtn_btn layout depends on USE_CLICK / USE_KEEPALIVE, so
# every consumer translation unit must see the exact same values.
target_compile_definitions(lwbtn PUBLIC
    LWBTN_CFG_USE_KEEPALIVE=${LWBTN_CFG_USE_KEEPALIVE}
    LWBTN_CFG_USE_CLICK=${LWBTN_CFG_USE_CLICK}
    LWBTN_CFG_TIME_DEBOUNCE_PRESS=${CONFIG_PACKAGE_LWBTN_DEBOUNCE_PRESS_MS}
    LWBTN_CFG_TIME_DEBOUNCE_RELEASE=${CONFIG_PACKAGE_LWBTN_DEBOUNCE_RELEASE_MS}
    LWBTN_CFG_TIME_KEEPALIVE_PERIOD=${CONFIG_PACKAGE_LWBTN_KEEPALIVE_PERIOD_MS}
    LWBTN_CFG_TIME_CLICK_MIN=${CONFIG_PACKAGE_LWBTN_CLICK_MIN_MS}
    LWBTN_CFG_TIME_CLICK_MAX=${CONFIG_PACKAGE_LWBTN_CLICK_MAX_MS}
    LWBTN_CFG_TIME_CLICK_MULTI_MAX=${CONFIG_PACKAGE_LWBTN_CLICK_MULTI_MAX_MS}
    LWBTN_CFG_CLICK_MAX_CONSECUTIVE=${CONFIG_PACKAGE_LWBTN_CLICK_MAX_CONSECUTIVE}
    LWBTN_CFG_CLICK_MAX_CONSECUTIVE_SEND_IMMEDIATELY=${LWBTN_CFG_CLICK_SEND_IMMEDIATELY}
    LWBTN_CFG_CLICK_CONSECUTIVE_KEEP_AFTER_SHORT_PRESS=${LWBTN_CFG_CLICK_KEEP_AFTER_SHORT_PRESS}
)

# package_lib_compile_options() appends the project's strict flags AFTER this
# package is configured (later flags win), so the exemption cannot live in
# target_compile_options(). Source-level options are applied after target-level
# ones, which keeps the exemption with the package (same pattern as nanosig).
set_source_files_properties("${LWBTN_SRC_DIR}/lwbtn/src/lwbtn/lwbtn.c" PROPERTIES
    COMPILE_OPTIONS "-Wno-conversion;-Wno-sign-conversion"
)

package_lib_add(lwbtn)

endif()
