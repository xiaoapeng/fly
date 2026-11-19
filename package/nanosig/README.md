# nanosig 集成

源码仍固定为 Kconfig 中的上游提交，构建时通过现有 `PATCH_DIR` 机制应用补丁。

`patches/0001-mpsc-preempt-guard.patch` 同步 nanosig 工作区的 MPSC 抢占修复：
port 后端必须提供 `ns_platform_preempt_enter/leave`，FreeRTOS 在短预留阶段使用任务
临界区；payload 复制在保护区外。不增加 Kconfig 选项，不扩展 ISR 支持。

修复尚未发布为上游提交，因此不修改固定 SHA。上游修复发布后，将 pin 更新到包含
修复的提交，再移除此补丁。不得把未提交的本地版本写成一个已发布的 SHA。

本地验证：`tool/test/check_build_arm.py` 检查工程构建；补丁中的
`nanosig_test_mpsc_record_ring_preempt` 运行确定性交错回归，FreeRTOS POSIX harness
检查真实 port 的临界区嵌套恢复。ARM 编译结果不等同于板上运行验证。
