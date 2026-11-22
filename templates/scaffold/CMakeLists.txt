cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-sdk.cmake")
include("${FLY_SDK_DIR}/cmake/fly-configure.cmake")
project("@PROJECT_NAME@" LANGUAGES C CXX ASM)
include("${FLY_SDK_DIR}/cmake/fly-sdk.cmake")

set(FLY_FIRMWARE_NAME "@PROJECT_NAME@")
set(FLY_FIRMWARE_VERSION "0.1.0")

if(NOT FLY_FIRMWARE_NAME MATCHES "^[A-Za-z0-9][A-Za-z0-9_-]*$")
    message(FATAL_ERROR "Use letters, digits, hyphens or underscores for FLY_FIRMWARE_NAME.")
endif()
if(NOT FLY_FIRMWARE_VERSION MATCHES "^([0-9]+)[.]([0-9]+)[.]([0-9]+)$"
        OR CMAKE_MATCH_1 GREATER 255 OR CMAKE_MATCH_2 GREATER 255 OR CMAKE_MATCH_3 GREATER 255)
    message(FATAL_ERROR "FLY_FIRMWARE_VERSION must be major.minor.patch, with each number between 0 and 255.")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-tools.cmake")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -B "${fly_project_actions}" cmake
        --project "${FLY_WORKSPACE_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}/fly-packs.cmake"
    COMMAND_ERROR_IS_FATAL ANY
)
include("${CMAKE_CURRENT_BINARY_DIR}/fly-packs.cmake")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${FLY_WORKSPACE_DIR}/fly.project.json")

add_executable("${FLY_PACK_TARGET}")
app_add_sdk_check("${FLY_PACK_TARGET}")
include("${FLY_WORKSPACE_DIR}/${FLY_PACK_BOARD_CMAKE}")
fly_configure_board("${FLY_PACK_TARGET}")
foreach(entry IN LISTS FLY_PACK_SUPPORT_CMAKE)
    include("${FLY_WORKSPACE_DIR}/${entry}")
endforeach()
list(LENGTH FLY_PACK_LOG_SINKS sink_count)
if(sink_count GREATER 1)
    message(FATAL_ERROR "Multiple active firmware log sinks: ${FLY_PACK_LOG_SINKS}")
elseif(sink_count EQUAL 1)
    target_link_libraries("${FLY_PACK_TARGET}" ${FLY_PACK_LOG_SINKS})
else()
    target_sources("${FLY_PACK_TARGET}" PRIVATE "${FLY_WORKSPACE_DIR}/app/mid/fly_log.c")
endif()
target_include_directories("${FLY_PACK_TARGET}" PRIVATE "${FLY_WORKSPACE_DIR}/app/include")
add_subdirectory(app)
fly_link_package_libraries("${FLY_PACK_TARGET}")

if(CONFIG_FACTORY_DATA_SUPPORTED AND NOT FLY_BOARD_HOST)
    if(NOT DEFINED FLY_FLASH_ADDRESS OR NOT FLY_FLASH_SIZE)
        message(FATAL_ERROR "Factory data requires the board's FLY_FLASH_ADDRESS and FLY_FLASH_SIZE.")
    endif()
    target_compile_definitions("${FLY_PACK_TARGET}" PRIVATE
        FLY_FLASH_ADDRESS=${FLY_FLASH_ADDRESS}
        FLY_FLASH_SIZE=${FLY_FLASH_SIZE}
    )
endif()

set(app_warnings -Wall -Wextra -Wconversion -Wsign-conversion -Wno-psabi)
if(NOT FLY_BOARD_HOST)
    list(APPEND app_warnings -g)
endif()
target_compile_options("${FLY_PACK_TARGET}" PRIVATE ${app_warnings})
fly_add_package_compile_options("${app_warnings}")

fly_add_target_map("${FLY_PACK_TARGET}")
if(NOT FLY_BOARD_HOST)
    fly_add_target_bin("${FLY_PACK_TARGET}")
    add_custom_command(TARGET "${FLY_PACK_TARGET}" POST_BUILD
        COMMAND "${Python3_EXECUTABLE}" -B "${fly_project_actions}" symbols
            --project "${FLY_WORKSPACE_DIR}" "--build-type=$<CONFIG>"
            --elf "$<TARGET_FILE:${FLY_PACK_TARGET}>" --debug-file "${CMAKE_CURRENT_BINARY_DIR}/${FLY_PACK_TARGET}.debug"
        VERBATIM
    )
    fly_add_target_memory_report("${FLY_PACK_TARGET}")
    fly_add_firmware_image(default
        IMAGE_NAME "${FLY_FIRMWARE_NAME}"
        BOARD_ID "${FLY_BOARD_ID}" CHIP "${FLY_BOARD_CHIP}" CORE_ID "${FLY_BOARD_CORE_ID}"
        BOARD_MANIFEST "${FLY_BOARD_MANIFEST}"
        FIRMWARE_LIST
            "APP:${FLY_FIRMWARE_VERSION}:${FLY_FLASH_ADDRESS}:${FLY_FLASH_SIZE}:${FLY_PACK_TARGET}.bin"
            # "BOOT:${BOOT_VERSION}:${BOOT_FLASH_ADDRESS}:${BOOT_FLASH_SIZE}:${BOOT_BIN}"
        SYMBOLS APP "${FLY_PACK_TARGET}"
        DEPENDS "${FLY_PACK_TARGET}"
    )
endif()

foreach(entry IN LISTS FLY_PACK_TOOLS_CMAKE)
    include("${FLY_WORKSPACE_DIR}/${entry}")
endforeach()
app_configure_actions("${FLY_PACK_TARGET}")
