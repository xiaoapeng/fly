# nanosig 集成

固定上游提交 `3110cf3e3d529097a531ee259eb84d7fa4ed98f8`，包含 nanosig PR #65
的 MPSC 抢占修复，不再应用临时补丁。FLY 仍根据调度模型选择 FreeRTOS 或裸机后端。

RTOS 在短预留阶段使用 port 抢占保护，payload 复制在保护区外；不新增 Kconfig
选项，也不扩展 ISR 支持。FreeRTOS demo 在 MCXN947 上完成旧版活锁对照，修复版
128 轮、2571 项检查通过，验证后恢复正常 demo。

上游测试与实机证据：
https://github.com/xiaoapeng/nanosig/pull/65
