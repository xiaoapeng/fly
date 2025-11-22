cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EVENTHUB_OS)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

message("Fetching eventhub_os...")

fetch_git_project(
    eventhub
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/eventhub_os.git
    GIT_TAG        main
)

fetch_git_project_add(eventhub)
package_lib_add(eventhub)

endif()
