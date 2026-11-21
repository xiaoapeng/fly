# 贡献者文档

从 [用户教程](../README.md) 开始生成和构建应用。这里记录板卡移植、SDK 设计和回归检查，供维护者和移植贡献者参考。

## 结构

| 目录 | 职责 |
|---|---|
| `design/` | 移植/适配的设计文档（Kconfig 选路、port glue、handler 安装） |
| `plans/` | 已归档的迁移与变更方案；执行前请先确认方案仍适用 |
| `reference/` | 当前模板的配置规则、RTOS 选择和组件加载方式 |

## 索引

- [工程模板和板卡](../templates/README.md) — 应用目录结构、CMake 入口和板卡配置
- [CI 流程](CI.md) — 从模板生成 ARM 和宿主平台应用，覆盖 PR 必需检查、夜间 Release 以及本地复现命令
- [回归脚本](../tool/test/README.md) — 应用配置、构建、SDK 安装及固件打包的校验入口
- [Kconfig 硬性规则](reference/kconfig-rules.md) — choice、请求位、默认选项和应用到 SDK 组件的依赖方向
- [RTOS 选择语义](reference/rtos-choice.md) — 生成模板如何选择调度器，以及 `package/scheduler/` 下的子包如何加载
- [FreeRTOS queue-set 后端](design/queueset-backend.md) — watcher 注册和注销、退出前排空队列、容量校验及 ISR 通知

## 历史记录

- [FreeRTOS 首次移植设计](design/freertos-port.md) — 初期 MCXN947 适配及验证记录
- [OS 包迁移到 scheduler/ 的方案](plans/scheduler-os-merge.md) — 已实施的包目录迁移和 scheduler choice 改动
