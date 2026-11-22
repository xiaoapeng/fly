cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EHIP)


include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

message("Fetching ehip...")

fly_fetch_git_project(
    ehip
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/ehip.git
    GIT_TAG        master
)

fly_add_git_project_subdirectory(ehip)
fly_register_package_library(ehip)

endif()
