include_guard(GLOBAL)

find_program(GIT_EXECUTABLE git)
if(NOT GIT_EXECUTABLE)
    message(FATAL_ERROR "Git not found")
endif()

message(STATUS "[fetch_git_project] Found Git: ${GIT_EXECUTABLE}")


# Generic Git-package patch mechanism (consensus plan Option A):
#   patches live in package/<name>/patches/*.patch and are applied to
#   dl/<name>-src in filename order after fetch, before add_subdirectory.
# Applicability is self-describing via marker file .fly-patches-state
# (HEAD sha + per-patch sha256) stored next to the source tree so it
# survives distclean. Any mismatch: forward-check first, then reverse-check
# (already applied => skip), else FATAL_ERROR (advise re-clone).
function(apply_git_project_patches GIT_PROJECT_NAME PATCH_DIR)
    set(SOURCE_DIR "${FLY_SOURCE_DL_BASE_DIR}/${GIT_PROJECT_NAME}-src")
    set(STATE_FILE "${SOURCE_DIR}/.fly-patches-state")

    if(NOT EXISTS "${SOURCE_DIR}/.git")
        message(FATAL_ERROR "[apply_git_project_patches] ${SOURCE_DIR} has no .git; fetch first")
    endif()
    if(NOT EXISTS "${PATCH_DIR}")
        # No patches dir = nothing to apply; treat as clean.
        return()
    endif()

    file(GLOB PATCH_FILES "${PATCH_DIR}/*.patch")
    if(NOT PATCH_FILES)
        return()
    endif()
    list(SORT PATCH_FILES)

    execute_process(
        COMMAND ${GIT_EXECUTABLE} -C "${SOURCE_DIR}" rev-parse HEAD
        OUTPUT_VARIABLE HEAD_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE HEAD_RESULT
        ERROR_QUIET
    )
    if(NOT HEAD_RESULT EQUAL 0)
        message(FATAL_ERROR "[apply_git_project_patches] cannot resolve HEAD in ${SOURCE_DIR}")
    endif()

    # Recompute expected state: HEAD sha + sha256 of each patch in order.
    set(EXPECTED_STATE "HEAD=${HEAD_SHA}\n")
    set(ALL_SHA_MATCH TRUE)
    foreach(PATCH_FILE ${PATCH_FILES})
        file(SHA256 "${PATCH_FILE}" PATCH_SHA)
        string(APPEND EXPECTED_STATE "patch=${PATCH_SHA} ${PATCH_FILE}\n")
    endforeach()

    set(NEED_APPLY FALSE)
    if(EXISTS "${STATE_FILE}")
        file(READ "${STATE_FILE}" RECORDED_STATE)
        if(RECORDED_STATE STREQUAL EXPECTED_STATE)
            message(STATUS "[apply_git_project_patches] ${GIT_PROJECT_NAME}: patches already applied at ${HEAD_SHA}")
            return()
        endif()
        # Marker differs: HEAD changed or patch content changed. Verify by
        # probing the tree itself rather than trusting the stale marker.
        set(NEED_APPLY TRUE)
    else()
        set(NEED_APPLY TRUE)
    endif()

    if(NEED_APPLY)
        foreach(PATCH_FILE ${PATCH_FILES})
            get_filename_component(PATCH_NAME "${PATCH_FILE}" NAME)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} -C "${SOURCE_DIR}" apply --check "${PATCH_FILE}"
                RESULT_VARIABLE FWD_CHECK
                ERROR_QUIET OUTPUT_QUIET
            )
            if(FWD_CHECK EQUAL 0)
                execute_process(
                    COMMAND ${GIT_EXECUTABLE} -C "${SOURCE_DIR}" apply "${PATCH_FILE}"
                    RESULT_VARIABLE APPLY_RESULT
                    ERROR_VARIABLE APPLY_ERR
                )
                if(NOT APPLY_RESULT EQUAL 0)
                    message(FATAL_ERROR "[apply_git_project_patches] ${GIT_PROJECT_NAME}: failed to apply ${PATCH_NAME}: ${APPLY_ERR}")
                endif()
                message(STATUS "[apply_git_project_patches] ${GIT_PROJECT_NAME}: applied ${PATCH_NAME}")
            else()
                # Forward fails: is it already applied?
                execute_process(
                    COMMAND ${GIT_EXECUTABLE} -C "${SOURCE_DIR}" apply --check --reverse "${PATCH_FILE}"
                    RESULT_VARIABLE REV_CHECK
                    ERROR_QUIET OUTPUT_QUIET
                )
                if(REV_CHECK EQUAL 0)
                    message(STATUS "[apply_git_project_patches] ${GIT_PROJECT_NAME}: ${PATCH_NAME} already applied; skipping")
                else()
                    message(FATAL_ERROR
                        "[apply_git_project_patches] ${GIT_PROJECT_NAME}: patch ${PATCH_NAME} no longer matches HEAD ${HEAD_SHA}. "
                        "The pinned source tree changed. Re-clone: rm -rf ${SOURCE_DIR}")
                endif()
            endif()
        endforeach()

        file(WRITE "${STATE_FILE}" "${EXPECTED_STATE}")
    endif()
endfunction()

function(fetch_git_project GIT_PROJECT_NAME)
    cmake_parse_arguments(
        CUSTOM_FUNC
        ""
        "GIT_REPOSITORY;GIT_TAG;DEEP_CLONE;AUTO_UPDATE;PATCH_DIR"
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

    set(PATCH_STATE_FILE "${SOURCE_DL_DIR}/.fly-patches-state")

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

    # Configure-time patch application (runs on every fresh configure so a
    # re-synced dl/ tree is re-patched before any source file is compiled).
    if(CUSTOM_FUNC_PATCH_DIR)
        apply_git_project_patches(${GIT_PROJECT_NAME} "${CUSTOM_FUNC_PATCH_DIR}")
    endif()

    if(NOT CUSTOM_FUNC_AUTO_UPDATE)
        list(APPEND PYTHON_COMMAND_ARGS --auto-update)
    endif()

    # Update target chains fetch -> patch (re)application so a build-time
    # re-sync cannot compile unpatched sources.
    if(CUSTOM_FUNC_PATCH_DIR)
        add_custom_target(
            ${UPDATE_TARGET}
            COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/fetch_git_project.py ${PYTHON_COMMAND_ARGS}
            COMMAND ${CMAKE_COMMAND} -DFLY_PATCH_PROJECT_NAME=${GIT_PROJECT_NAME} -DFLY_PATCH_DIR=${CUSTOM_FUNC_PATCH_DIR} -DFLY_SOURCE_DL_BASE_DIR=${FLY_SOURCE_DL_BASE_DIR} -P ${FLY_TOP_DIR}/cmake/apply_git_project_patches.cmake
            VERBATIM
        )
    else()
        add_custom_target(
            ${UPDATE_TARGET}
            COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/fetch_git_project.py ${PYTHON_COMMAND_ARGS}
            VERBATIM
        )
    endif()

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