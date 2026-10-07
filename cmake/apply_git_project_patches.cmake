# Build-time patch re-application for package_update_<name> targets.
# Invoked via: cmake -DFLY_PATCH_PROJECT_NAME=<name> -DFLY_PATCH_DIR=<dir> \
#                   -DFLY_SOURCE_DL_BASE_DIR=<base> -P apply_git_project_patches.cmake
#
# Delegates to the canonical function in fetch_git_project.cmake.

include("${CMAKE_CURRENT_LIST_DIR}/fetch_git_project.cmake")

if(NOT DEFINED FLY_PATCH_PROJECT_NAME OR NOT DEFINED FLY_PATCH_DIR OR NOT DEFINED FLY_SOURCE_DL_BASE_DIR)
    message(FATAL_ERROR "[apply_git_project_patches] FLY_PATCH_PROJECT_NAME, FLY_PATCH_DIR and FLY_SOURCE_DL_BASE_DIR are required")
endif()

apply_git_project_patches(${FLY_PATCH_PROJECT_NAME} "${FLY_PATCH_DIR}")
