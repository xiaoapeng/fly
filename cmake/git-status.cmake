include_guard(GLOBAL)

# 定义生成 git 状态时间戳文件的自定义命令
add_custom_command(
    OUTPUT .gitstatus.timestamp .gitstatus_make
    COMMAND python3 ${FLY_TOP_DIR}/tool/python/check_git_status.py --output-dir ${CMAKE_BINARY_DIR}/
    COMMENT " "

)

# 添加自定义目标来生成 git 状态时间戳文件
add_custom_target(check_git_status ALL
    DEPENDS .gitstatus_make
)

