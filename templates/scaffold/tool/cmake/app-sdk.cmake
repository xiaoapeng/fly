if(NOT FLY_SDK_DIR)
    if(DEFINED ENV{FLY_ROOT} AND NOT "$ENV{FLY_ROOT}" STREQUAL "")
        set(FLY_SDK_DIR "$ENV{FLY_ROOT}")
    elseif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fly")
        set(FLY_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fly")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.fly-root")
        file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/.fly-root" FLY_SDK_DIR LIMIT_COUNT 1)
    endif()
endif()
if(NOT FLY_SDK_DIR)
    message(FATAL_ERROR "FLY SDK is missing. Run build.sh setup or build.bat setup.")
endif()
file(REAL_PATH "${FLY_SDK_DIR}" FLY_SDK_DIR BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set(app_sdk_setup "${CMAKE_CURRENT_LIST_DIR}/../python/setup_fly.py")
function(app_check_sdk)
    if(NOT Python3_EXECUTABLE AND IS_DIRECTORY "${FLY_SDK_DIR}/.venv")
        set(Python3_ROOT_DIR "${FLY_SDK_DIR}/.venv")
    endif()
    find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -B "${app_sdk_setup}" --check --sdk-dir "${FLY_SDK_DIR}"
        COMMAND_ERROR_IS_FATAL ANY
    )
endfunction()
app_check_sdk()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fly.lock")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/fly.lock")
endif()

function(app_add_sdk_check firmware_target)
    add_custom_target(fly_check_sdk
        COMMAND "${Python3_EXECUTABLE}" -B "${app_sdk_setup}" --check --sdk-dir "${FLY_SDK_DIR}"
        VERBATIM
    )
    add_dependencies("${firmware_target}" fly_check_sdk)
endfunction()
