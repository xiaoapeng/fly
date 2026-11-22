include_guard(GLOBAL)

find_program(GIT_EXECUTABLE git REQUIRED)

function(_fly_watch_git_patches directory)
    file(GLOB patches LIST_DIRECTORIES false CONFIGURE_DEPENDS
        "${directory}/*.patch" "${directory}/*/*.patch")
    if(patches)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${patches})
    endif()
endfunction()

function(fly_fetch_git_project name)
    cmake_parse_arguments(PARSE_ARGV 1 fetch ""
        "GIT_REPOSITORY;GIT_TAG;DEEP_CLONE;AUTO_UPDATE;PATCH_DIR;SOURCE_DIR_VAR"
        "SPARSE_PATHS")
    if(NOT fetch_GIT_REPOSITORY OR NOT fetch_GIT_TAG)
        message(FATAL_ERROR "${name}: GIT_REPOSITORY and GIT_TAG are required")
    endif()

    set(package_dir "${CMAKE_BINARY_DIR}/package_build/${name}")
    set(package_info "${package_dir}/package_info.txt")
    set(result_file "${package_dir}/git-source.json")
    set(project_patch_dir "${FLY_WORKSPACE_DIR}/patches/${name}")
    set(command "${Python3_EXECUTABLE}" -B "${FLY_TOP_DIR}/tool/python/fetch_git_project.py"
        --project-name "${name}"
        --repository "${fetch_GIT_REPOSITORY}"
        --tag "${fetch_GIT_TAG}"
        --cache-dir "${FLY_TOP_DIR}/dl"
        --result-file "${result_file}"
        --package-info-file "${package_info}"
        --mirror-source-json-file "${FLY_MIRROR_SOURCE_JSON_FILE}"
        --project-patch-dir "${project_patch_dir}"
    )
    _fly_watch_git_patches("${project_patch_dir}")
    if(fetch_PATCH_DIR)
        get_filename_component(patch_dir "${fetch_PATCH_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND command --patch-dir "${patch_dir}")
        _fly_watch_git_patches("${patch_dir}")
    endif()
    foreach(path IN LISTS fetch_SPARSE_PATHS)
        list(APPEND command --sparse-path "${path}")
    endforeach()
    if(fetch_DEEP_CLONE)
        list(APPEND command --deep-clone)
    endif()

    set(configure_args)
    if(fetch_AUTO_UPDATE)
        set(configure_args --auto-update)
    endif()
    string(SHA256 request "${command};${configure_args}")
    get_property(previous GLOBAL PROPERTY "FLY_GIT_REQUEST_${name}")
    if(previous AND NOT previous STREQUAL request)
        message(FATAL_ERROR "${name}: conflicting Git dependency declarations")
    endif()

    if(NOT previous)
        execute_process(COMMAND ${command} ${configure_args} COMMAND_ERROR_IS_FATAL ANY)
        file(READ "${result_file}" result)
        string(JSON source_dir GET "${result}" source_dir)
        if(NOT IS_ABSOLUTE "${source_dir}" OR NOT IS_DIRECTORY "${source_dir}")
            message(FATAL_ERROR "${name}: invalid source directory in ${result_file}: ${source_dir}")
        endif()
        set_property(GLOBAL PROPERTY "FLY_GIT_REQUEST_${name}" "${request}")
        set_property(GLOBAL PROPERTY "FLY_GIT_SOURCE_${name}" "${source_dir}")

        set(update_target "package_update_${name}")
        add_custom_target("${update_target}" COMMAND ${command} --auto-update VERBATIM)
        add_custom_target("${update_target}_info_show"
            COMMAND "${CMAKE_COMMAND}" -E cat "${package_info}"
            VERBATIM
        )

        list(APPEND FLY_ALL_PACKAGE_INFO_FILE_LIST "${package_info}")
        set(FLY_ALL_PACKAGE_INFO_FILE_LIST "${FLY_ALL_PACKAGE_INFO_FILE_LIST}" CACHE INTERNAL "All package info")
        list(APPEND FLY_ALL_PACKAGE_UPDATE_TARGETS_LIST "${update_target}")
        set(FLY_ALL_PACKAGE_UPDATE_TARGETS_LIST "${FLY_ALL_PACKAGE_UPDATE_TARGETS_LIST}" CACHE INTERNAL "All package update targets")
    else()
        get_property(source_dir GLOBAL PROPERTY "FLY_GIT_SOURCE_${name}")
    endif()

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${result_file}"
        "${FLY_TOP_DIR}/tool/python/fetch_git_project.py"
        "${FLY_TOP_DIR}/tool/python/git_source_cache.py")
    if(fetch_SOURCE_DIR_VAR)
        set("${fetch_SOURCE_DIR_VAR}" "${source_dir}" PARENT_SCOPE)
    endif()
endfunction()

function(fly_add_git_project_subdirectory name)
    get_property(source_dir GLOBAL PROPERTY "FLY_GIT_SOURCE_${name}")
    if(NOT source_dir)
        message(FATAL_ERROR "${name}: call fly_fetch_git_project before fly_add_git_project_subdirectory")
    endif()
    if(EXISTS "${source_dir}/CMakeLists.txt")
        add_subdirectory("${source_dir}" "${CMAKE_BINARY_DIR}/package_build/${name}")
    else()
        message(WARNING "[fly_add_git_project_subdirectory] ${name} does not have a CMakeLists.txt")
    endif()
endfunction()
