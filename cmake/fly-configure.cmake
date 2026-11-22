# Prepare the selected SDK and toolchain before the application's project().
get_filename_component(FLY_TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)
set(FLY_RESOLVED_SDK_DIR "${FLY_TOP_DIR}" CACHE INTERNAL "Resolved FLY SDK checkout")
set(FLY_WORKSPACE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(FLY_OUTPUT_DIR "${FLY_WORKSPACE_DIR}/build")
set(FLY_CONFIG_DIR "${FLY_OUTPUT_DIR}/config")
set(FLY_DOWNLOAD_DIR "${FLY_OUTPUT_DIR}/deps")
set(FLY_IMAGE_DIR "${FLY_WORKSPACE_DIR}/image")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(EXISTS "${FLY_TOP_DIR}/.PATH.env.json")
    file(READ "${FLY_TOP_DIR}/.PATH.env.json" fly_tool_paths_json)
    string(JSON fly_tool_paths_length LENGTH "${fly_tool_paths_json}")
    if(fly_tool_paths_length GREATER 0)
        math(EXPR fly_tool_paths_last "${fly_tool_paths_length} - 1")
        set(fly_tool_paths "")
        foreach(index RANGE ${fly_tool_paths_last})
            string(JSON path GET "${fly_tool_paths_json}" ${index})
            list(APPEND fly_tool_paths "${path}")
        endforeach()
        if(CMAKE_HOST_WIN32)
            set(fly_path_separator ";")
        else()
            set(fly_path_separator ":")
        endif()
        list(JOIN fly_tool_paths "${fly_path_separator}" fly_tool_path)
        set(ENV{PATH} "${fly_tool_path}${fly_path_separator}$ENV{PATH}")
    endif()
endif()

include("${FLY_TOP_DIR}/cmake/python3.cmake")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -B "${FLY_TOP_DIR}/tool/python/build.py"
        --project-dir "${FLY_WORKSPACE_DIR}" prepare
    OUTPUT_VARIABLE fly_python
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
if(NOT IS_ABSOLUTE "${fly_python}" OR NOT EXISTS "${fly_python}")
    message(FATAL_ERROR "FLY did not return a usable Python interpreter.")
endif()
set(Python3_EXECUTABLE "${fly_python}" CACHE FILEPATH "Python interpreter for FLY" FORCE)
set(Python3_EXECUTABLE "${fly_python}")

include("${FLY_CONFIG_DIR}/.config.cmake")
set(fly_arch_dir "${FLY_TOP_DIR}/arch/${CONFIG_ARCH}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${FLY_TOP_DIR}/arch/Kconfig" "${fly_arch_dir}/arch.json")
if(EXISTS "${fly_arch_dir}/Kconfig")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${fly_arch_dir}/Kconfig")
endif()
include("${fly_arch_dir}/gcc-config.cmake")
include("${FLY_TOP_DIR}/cmake/tool-chain-general.cmake")
include("${FLY_TOP_DIR}/cmake/arch-config.cmake")
set(FLY_TOOLCHAIN_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
