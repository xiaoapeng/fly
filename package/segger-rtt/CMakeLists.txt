if(CONFIG_PACKAGE_SEGGER_RTT)
    include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

    # Preserve RTT 7.88m using the published FLY snapshot. SEGGER's public
    # RTT Git history does not contain this release.
    fly_fetch_git_project(segger-rtt
        GIT_REPOSITORY "https://github.com/xiaoapeng/fly.git"
        GIT_TAG "e8fa28b5e84a2ed38fec7d8bac0d6902952c0578"
        SOURCE_DIR_VAR segger_rtt_checkout
        SPARSE_PATHS "package/segger-rtt/inc"
        PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
    )
    set(segger_rtt_vendor_dir "${segger_rtt_checkout}/package/segger-rtt")

    add_library(segger-rtt OBJECT "${segger_rtt_vendor_dir}/SEGGER_RTT.c")
    # The snapshot patch moves its configuration header aside so the quoted
    # include in SEGGER_RTT.h resolves FLY's local configuration.
    target_include_directories(segger-rtt PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/inc"
        "${segger_rtt_vendor_dir}/inc"
    )
    fly_register_package_library(segger-rtt)
endif()
