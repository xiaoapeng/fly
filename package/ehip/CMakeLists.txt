cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EHIP)

project(packge-ehip)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

message("Fetching ehip...")

fetch_git_project(
    ehip
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/ehip.git
    GIT_TAG        master
)

fetch_git_project_add(ehip)

endif()
