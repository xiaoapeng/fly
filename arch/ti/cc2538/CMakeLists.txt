include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

fly_fetch_git_project(vendor-cc2538
    GIT_REPOSITORY "https://github.com/contiki-ng/contiki-ng.git"
    GIT_TAG "dba7cb6d98e1f435dc84b45ee08b0061d7c5628e"
    SOURCE_DIR_VAR vendor_cc2538_source
    SPARSE_PATHS "arch/cpu/cc2538"
    PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
)

add_library(arch-cc2538-sf53 OBJECT EXCLUDE_FROM_ALL "${vendor_cc2538_source}/arch/cpu/cc2538/startup-gcc.c")
target_compile_definitions(arch-cc2538-sf53 PUBLIC CC2538SF53)
