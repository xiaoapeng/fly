cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EHSHELL)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

message("Fetching ehip-tools...")

fetch_git_project(
    ehip-tools
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/ehip-tools.git
    GIT_TAG        main
)

fetch_git_project_add(ehip-tools)
package_lib_add(ehip-tools)

endif()
