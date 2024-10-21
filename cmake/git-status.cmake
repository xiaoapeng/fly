include_guard(GLOBAL)

# 添加自定义目标来生成 git 状态时间戳文件
add_custom_target(check_git_status
    COMMAND ${Python3_EXECUTABLE} ${FLY_TOP_DIR}/tool/python/check_git_status.py --output-dir ${CMAKE_BINARY_DIR}/
)

