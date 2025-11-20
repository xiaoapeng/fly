cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_ARGUMENT_PARSER)

project(packge-argparse)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

message("Fetching ehshell...")

fetch_git_project(
    argparse
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/argparse.git
    GIT_TAG        fly-package
)

fetch_git_project_add(argparse)

endif()
