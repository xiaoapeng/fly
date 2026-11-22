set(fly_project_actions "${FLY_SDK_DIR}/tool/python/project_actions.py")

function(app_configure_actions firmware_target)
    set(firmware_elf "${CMAKE_CURRENT_BINARY_DIR}/${firmware_target}${CMAKE_EXECUTABLE_SUFFIX}")
    foreach(entry IN LISTS FLY_PACK_ACTION_TARGETS)
        string(REPLACE ":" ";" fields "${entry}")
        list(GET fields 0 action)
        list(GET fields 1 provider)
        add_custom_target("fly_${action}_${provider}"
            COMMAND "${Python3_EXECUTABLE}" -B "${FLY_SDK_DIR}/tool/python/build.py"
                --project-dir "${FLY_WORKSPACE_DIR}" "${action}"
                --via "${provider}" "--build-type=$<CONFIG>"
            DEPENDS fly_check_sdk
            WORKING_DIRECTORY "${FLY_WORKSPACE_DIR}"
            USES_TERMINAL
            VERBATIM
        )
    endforeach()
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -B "${fly_project_actions}" index
            --project "${FLY_WORKSPACE_DIR}" --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
            "--build-type=${CMAKE_BUILD_TYPE}" --elf "${firmware_elf}"
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -B "${FLY_SDK_DIR}/tool/python/project_ide.py"
            --project "${FLY_WORKSPACE_DIR}" --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
            "--build-type=${CMAKE_BUILD_TYPE}" --elf "${firmware_elf}" "--gdb=${CMAKE_GDB}"
        RESULT_VARIABLE ide_status
    )
    if(NOT ide_status EQUAL 0)
        message(WARNING "FLY could not generate debug configurations. See the diagnostic above.")
    endif()
endfunction()
