cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EHSHELL)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

message("Fetching ehshell...")

fetch_git_project(
    ehshell
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/ehshell.git
    GIT_TAG        master
)

fetch_git_project_add(ehshell)
package_lib_add(ehshell)

if(NOT CONFIG_PACKAGE_EHSHELL_BUILTIN_NONE)
    package_lib_add(ehshell_builtin)
endif()

endif()
