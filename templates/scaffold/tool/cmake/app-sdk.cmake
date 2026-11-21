set(app_sdk_setup "${CMAKE_CURRENT_LIST_DIR}/../python/setup_fly.py")
set(app_sdk_input "${FLY_SDK_DIR}")
function(app_resolve_sdk)
    find_package(Python3 3.8 COMPONENTS Interpreter REQUIRED)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -B "${app_sdk_setup}" --resolve "--sdk-dir=${app_sdk_input}"
        OUTPUT_VARIABLE resolved_sdk
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )
    set(FLY_SDK_DIR "${resolved_sdk}" PARENT_SCOPE)
endfunction()
app_resolve_sdk()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/fly.lock")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/fly.lock")
endif()

function(app_add_sdk_check firmware_target)
    add_custom_target(fly_check_sdk
        COMMAND "${Python3_EXECUTABLE}" -B "${app_sdk_setup}" --resolve
            "--sdk-dir=${app_sdk_input}" "--configured-sdk-dir=${FLY_SDK_DIR}"
        VERBATIM
    )
    add_dependencies("${firmware_target}" fly_check_sdk)
endfunction()
