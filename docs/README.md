# Docs

> README 是用户教程；docs 是设计、方案与参考。读者：维护者、移植贡献者。

## 结构

| 目录 | 职责 |
|---|---|
| `design/` | 移植/适配的设计文档（Kconfig 选路、port glue、handler 安装） |
| `plans/` | 已归档的迁移与变更方案；执行前请先确认方案仍适用 |
| `reference/` | 快速参考卡（RTOS choice、defconfig 迁移表等） |
| `notes/` | 本地工具/环境笔记 |

## 索引

- [OS 包迁移到 scheduler/ 的方案](plans/scheduler-os-merge.md) — 已实施；将 eventhub-os 与 freertos 包并入 `package/scheduler/`，scheduler choice 成为唯一 OS 入口，`PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS` 内部 bool 已删除
- [RTOS 选择语义](reference/rtos-choice.md) — FreeRTOS 与 eventhub-os 的互斥约定，以及 scheduler/ 下的子包如何被引用
- [FreeRTOS 移植设计](design/freertos-port.md) — mcxn947 适配、Kconfig、port glue；当前 OS 包布局为 `package/scheduler/eventhub-os/` 与 `package/scheduler/freertos/`