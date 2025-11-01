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
    # info_show_tmp 文件
    set(INFO_SHOW_TMP_FILE "${CMAKE_BINARY_DIR}/package_build/${GIT_PROJECT_NAME}/info_show_tmp.txt")
    set(UPDATE_TARGET "package_update_${GIT_PROJECT_NAME}")
    set(UPDATE_TARGET_INFO_SHOW "package_update_${GIT_PROJECT_NAME}_info_show")

    # 检查源码和源码.git目录是否存在
    if(NOT EXISTS "${SOURCE_DL_DIR}" OR NOT EXISTS "${SOURCE_DL_DIR}/.git")
        message(STATUS "[fetch_git_project] Cloning ${CUSTOM_FUNC_GIT_REPOSITORY} to ${SOURCE_DL_DIR}")
        if(CUSTOM_FUNC_DEEP_CLONE)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} clone --branch ${CUSTOM_FUNC_GIT_TAG} ${CUSTOM_FUNC_GIT_REPOSITORY} ${SOURCE_DL_DIR}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE GIT_CLONE_RESULT
            )
        else()
            execute_process(
                COMMAND ${GIT_EXECUTABLE} clone --depth 1 --branch ${CUSTOM_FUNC_GIT_TAG} ${CUSTOM_FUNC_GIT_REPOSITORY} ${SOURCE_DL_DIR}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RESULT_VARIABLE GIT_CLONE_RESULT
            )
        endif()
        if(NOT GIT_CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone ${CUSTOM_FUNC_GIT_REPOSITORY} to ${SOURCE_DL_DIR}")
        endif()
    else()
        message(STATUS "[fetch_git_project] ${GIT_PROJECT_NAME} already cloned to ${SOURCE_DL_DIR}")
        if(CUSTOM_FUNC_AUTO_UPDATE)
            message(STATUS "[fetch_git_project] Updating ${GIT_PROJECT_NAME}...")
            # Step 1: 获取当前 HEAD 信息
            execute_process(
                COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} rev-parse --abbrev-ref HEAD
                OUTPUT_VARIABLE CURRENT_BRANCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )

            # Step 2: 拉取最新内容
            execute_process(
                COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} fetch --all --tags
                RESULT_VARIABLE FETCH_RESULT
                OUTPUT_QUIET ERROR_QUIET
            )

            if(NOT FETCH_RESULT EQUAL 0)
                message(WARNING "[fetch_git_project] Failed to fetch updates for ${GIT_PROJECT_NAME}")
            else()
                # Step 3: 检查是否为 tag 或 branch
                execute_process(
                    COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} tag --list ${CUSTOM_FUNC_GIT_TAG}
                    OUTPUT_VARIABLE IS_TAG
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                )

                if(IS_TAG)
                    # tag 通常是静态版本，不自动更新
                    message(STATUS "[fetch_git_project] ${CUSTOM_FUNC_GIT_TAG} is a tag — no auto-update applied")
                else()
                    # 分支 → 自动拉取最新
                    message(STATUS "[fetch_git_project] Switching to ${CUSTOM_FUNC_GIT_TAG} and pulling latest")
                    execute_process(
                        COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} checkout ${CUSTOM_FUNC_GIT_TAG}
                        COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} pull
                        RESULT_VARIABLE PULL_RESULT
                        OUTPUT_QUIET ERROR_QUIET
                    )
                    if(NOT PULL_RESULT EQUAL 0)
                        message(WARNING "[fetch_git_project] Failed to update branch ${CUSTOM_FUNC_GIT_TAG}")
                    endif()
                endif()
            endif()
        endif()
    endif()

    add_custom_target(
        ${UPDATE_TARGET}
        COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} fetch --all --tags
        COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} checkout ${CUSTOM_FUNC_GIT_TAG}
        COMMAND ${GIT_EXECUTABLE} -C ${SOURCE_DL_DIR} pull
        COMMENT "Updating ${GIT_PROJECT_NAME} repository"
        VERBATIM
    )

    add_custom_target(
        ${UPDATE_TARGET_INFO_SHOW}
        COMMAND ${CMAKE_COMMAND} -E echo "${GIT_PROJECT_NAME}:" > ${INFO_SHOW_TMP_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "      tag: ${CUSTOM_FUNC_GIT_TAG}" > ${INFO_SHOW_TMP_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "      remote: ${CUSTOM_FUNC_GIT_REPOSITORY}" > ${INFO_SHOW_TMP_FILE}
        COMMAND ${CMAKE_COMMAND} -E cat ${INFO_SHOW_TMP_FILE}
        VERBATIM
    )
    
    # 全局变量保存所有更新目标
    set(_targets "${FLY_ALL_PACKAGE_UPDATE_TARGETS}")
    list(APPEND _targets ${UPDATE_TARGET})
    set(FLY_ALL_PACKAGE_UPDATE_TARGETS "${_targets}" CACHE INTERNAL "All package update targets")

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


