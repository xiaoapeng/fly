include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

fly_fetch_git_project(vendor-max32650
    GIT_REPOSITORY "https://github.com/analogdevicesinc/msdk.git"
    GIT_TAG "92a7c488e1f81c3a2a279223c072598e7d7a53c6"
    SOURCE_DIR_VAR vendor_max32650_source
    SPARSE_PATHS "Libraries/CMSIS/Device/Maxim/MAX32650/Source/GCC"
    PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
)

add_library(arch-max32652 OBJECT EXCLUDE_FROM_ALL
    "${vendor_max32650_source}/Libraries/CMSIS/Device/Maxim/MAX32650/Source/GCC/startup_max32650.S"
    system_max32650.c
)
