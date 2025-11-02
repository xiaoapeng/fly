include_guard(GLOBAL)

find_program(GIT_EXECUTABLE git)
if(NOT GIT_EXECUTABLE)
    message(FATAL_ERROR "Git not found")
endif()

message(STATUS "[fetch_git_project] Found Git: ${GIT_EXECUTABLE}")


function(fetch_git_project GIT_PROJECT_NAME)
    cmake_parse_arguments(
        CUSTOM_FUNC ""
        "GIT_REPOSITORY;GIT_TAG;DEEP_CLONE;AUTO_UPDATE"
        ""
        ${ARGN}
    )
    # 检查必填参数
    if(NOT CUSTOM_FUNC_GIT_REPOSITORY)
        message(FATAL_ERROR "GIT_REPOSITORY is required")
    endif()
    if(NOT CUSTOM_FUNC_GIT_TAG)
        message(FATAL_ERROR "GIT_TAG is required")
    endif()
    if(NOT CUSTOM_FUNC_DEEP_CLONE)
        set(CUSTOM_FUNC_DEEP_CLONE OFF)
    endif()
    if(NOT CUSTOM_FUNC_AUTO_UPDATE)
        set(CUSTOM_FUNC_AUTO_UPDATE OFF)
    endif()
    
    # 源码下载到 $FLY_SOURCE_DL_BASE_DIR/${GIT_PROJECT_NAME}-src
    set(SOURCE_DL_DIR "${FLY_SOURCE_DL_BASE_DIR}/${GIT_PROJECT_NAME}-src")
    # Binary 目录
    set(BINARY_DL_DIR "${CMAKE_BINARY_DIR}/package_build/${GIT_PROJECT_NAME}")
    set(PACKAGE_INFO_FILE "${BINARY_DL_DIR}/package_info.txt")

    set(UPDATE_TARGET "package_update_${GIT_PROJECT_NAME}")
    set(UPDATE_TARGET_INFO_SHOW "package_update_${GIT_PROJECT_NAME}_info_show")

    # 构建命令参数列表
    set(PYTHON_COMMAND_ARGS 
        --project-name ${GIT_PROJECT_NAME}
        --repository ${CUSTOM_FUNC_GIT_REPOSITORY}
        --tag ${CUSTOM_FUNC_GIT_TAG}
        --source-dl-dir ${SOURCE_DL_DIR}
        --package-info-file ${PACKAGE_INFO_FILE}
        --mirror-source-json-file ${FLY_MIRROR_SOURCE_JSON_FILE}
    )
    # 条件添加参数
    if(CUSTOM_FUNC_DEEP_CLONE)
        list(APPEND PYTHON_COMMAND_ARGS --deep-clone)
    endif()
    
    if(CUSTOM_FUNC_AUTO_UPDATE)
        list(APPEND PYTHON_COMMAND_ARGS --auto-update)
    endif()

    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/fetch_git_project.py ${PYTHON_COMMAND_ARGS}
        RESULT_VARIABLE FETCH_RESULT
    )
    
    if(NOT FETCH_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to fetch git project. Result code: ${FETCH_RESULT}")
    endif()

    if(NOT CUSTOM_FUNC_AUTO_UPDATE)
        list(APPEND PYTHON_COMMAND_ARGS --auto-update)
    endif()

    add_custom_target(
        ${UPDATE_TARGET}
        COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/fetch_git_project.py ${PYTHON_COMMAND_ARGS} 
        VERBATIM
    )

    add_custom_target(
        ${UPDATE_TARGET_INFO_SHOW}
        COMMAND ${CMAKE_COMMAND} -E cat "${PACKAGE_INFO_FILE}"
        VERBATIM
    )

    # 全局变量保存所有更新目标信息
    list(APPEND FLY_ALL_PACKAGE_INFO_FILE_LIST "${PACKAGE_INFO_FILE}")
    set(FLY_ALL_PACKAGE_INFO_FILE_LIST "${FLY_ALL_PACKAGE_INFO_FILE_LIST}" CACHE INTERNAL "All package info")

    # 全局变量保存所有更新目标
    list(APPEND FLY_ALL_PACKAGE_UPDATE_TARGETS_LIST ${UPDATE_TARGET})
    set(FLY_ALL_PACKAGE_UPDATE_TARGETS_LIST "${FLY_ALL_PACKAGE_UPDATE_TARGETS_LIST}" CACHE INTERNAL "All package update targets")

endfunction()

function(fetch_git_project_add GIT_PROJECT_NAME)
    set(SOURCE_DL_DIR "${FLY_SOURCE_DL_BASE_DIR}/${GIT_PROJECT_NAME}-src")
    set(BINARY_DL_DIR "${CMAKE_BINARY_DIR}/package_build/${GIT_PROJECT_NAME}")
    # 添加项目到本工程
    if(EXISTS "${SOURCE_DL_DIR}/CMakeLists.txt")
        message(STATUS "[fetch_git_project_add] Adding ${GIT_PROJECT_NAME} to this project...")
        add_subdirectory(${SOURCE_DL_DIR} ${BINARY_DL_DIR})
    else()
        message(WARNING "[fetch_git_project_add] ${GIT_PROJECT_NAME} does not have a CMakeLists.txt")
    endif()
endfunction()