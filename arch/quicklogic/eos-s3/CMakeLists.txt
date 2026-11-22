include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# The QORC reset code and EOS S3 vectors retain their original ARM notices.
fly_fetch_git_project(quicklogic-qorc-sdk
    GIT_REPOSITORY https://github.com/QuickLogic-Corp/qorc-sdk.git
    GIT_TAG d61d064146c0ee927aa12b088b3bbbce60615f4d
    SPARSE_PATHS HAL/startup
    SOURCE_DIR_VAR qorc_dir
)

add_library(arch-quicklogic-eos-s3 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-quicklogic-eos-s3 PRIVATE __STARTUP_CLEAR_BSS)
target_sources(arch-quicklogic-eos-s3 PRIVATE
    "${qorc_dir}/HAL/startup/startup_EOSS3B_GCC.c"
    "${qorc_dir}/HAL/startup/vectors_CM4F_gcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/system.c"
)
