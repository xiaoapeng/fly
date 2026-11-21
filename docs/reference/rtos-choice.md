# RTOS 选择快速参考

## 1. Scheduler 选择器

```
choice PACKAGE_SCHEDULER
    default PACKAGE_SCHEDULER_FREERTOS if PACKAGE_SCHEDULER_REQUIRE_FREERTOS
    default PACKAGE_SCHEDULER_EVENTHUB_OS if PACKAGE_SCHEDULER_REQUIRE_EVENTHUB_OS
    default PACKAGE_SCHEDULER_NONE
    PACKAGE_SCHEDULER_NONE          → 无 scheduler（bare-metal main loop）
    PACKAGE_SCHEDULER_EVENTHUB_OS   → eventhub-os（cooperative signal/slot framework）
    PACKAGE_SCHEDULER_FREERTOS      → depends on ARCH_CPU_FAMILY_CORTEX_M
                                       FreeRTOS（preemptive kernel）
```

- `PACKAGE_SCHEDULER` 负责互斥选择。没有显式选择或请求时，默认使用 `NONE`。
- 自定义应用在 Kconfig 中可 `select PACKAGE_SCHEDULER_REQUIRE_<leg>`，由条件 `default` 选择对应支路。显式写入 `defconfig` 的选择优先于请求位。 [Kconfig 规则](kconfig-rules.md)
- 手工 dual-pin 时（`CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y` + `CONFIG_PACKAGE_SCHEDULER_FREERTOS=y`），kconfiglib 归一为 `scheduler/Kconfig` 中声明顺序靠后的 leg（FreeRTOS）。

## 2. 生成模板的配置

工程生成器把对应模板的 `defconfig` 与板卡的架构标识写入应用。应用根目录的 `Kconfig` 引入 SDK 的架构和组件配置。构建从应用的 `CMakeLists.txt` 进入，生成的 Kconfig 文件位于 `build/config/`。

| 模板 | 调度器 | 组件 |
|---|---|---|
| `baremetal` | `CONFIG_PACKAGE_SCHEDULER_NONE=y` | 依板卡配置 |
| `baremetal-nanosig` | `CONFIG_PACKAGE_SCHEDULER_NONE=y` | `CONFIG_PACKAGE_NANOSIG=y` |
| `freertos-nanosig` | `CONFIG_PACKAGE_SCHEDULER_FREERTOS=y` | `CONFIG_PACKAGE_NANOSIG=y` |

从 SDK 根目录执行 `python3 tool/test/check_standalone_projects.py --matrix config`，可验证 CI 覆盖的板卡模板组合均选中对应架构和调度器，并且没有 Kconfig 警告。 [模板说明](../../templates/README.md) · [CI 流程](../CI.md)

## 3. 子包 Kconfig 与 CMake

调度器 Kconfig 按以下条件加载子包，子包内部也用所选调度器限制选项可见性。

- `package/scheduler/eventhub-os/Kconfig` → `if PACKAGE_SCHEDULER_EVENTHUB_OS`
- `package/scheduler/freertos/Kconfig` → `if PACKAGE_SCHEDULER_FREERTOS`

组件的 `CMakeLists.txt` 按 `CONFIG_PACKAGE_SCHEDULER_*` 决定拉取源码和注册目标。裸机应用使用 `NONE` 时，这两种调度器组件均保持禁用。

## 4. FreeRTOS 链接与中断

`freertos-config` 是 `INTERFACE` 目标，提供配置头文件、内核头文件和编译定义。`freertos-heap`、`freertos-kernel`、`freertos-port` 和 `freertos-assert` 是通过 `package_lib_add()` 注册的 `OBJECT` 目标。应用调用 `target_link_package_lib()` 加入这些目标，并沿 `PUBLIC` 依赖获得配置。 [FreeRTOS 构建定义](../../package/scheduler/freertos/CMakeLists.txt)

FRDM-MCXN947 的裸机 nanosig 模板把板卡的 SysTick 时基加入构建。FreeRTOS 模板由 FreeRTOS port 接管 SysTick。 [裸机时基配置](../../templates/apps/baremetal-nanosig/app/configure.cmake)

## 5. 历史迁移名册

> 本节记录 SDK 仍包含 `project/` 目录时的迁移。当前模板和测试入口见上文。

### 当时的两步迁移

注：`gd32f103-demo` 与 `gd32e230-demo` 未启用任何 OS 包，依赖 choice 默认值 `NONE`，defconfig 无需 pin。

| 项目 Kconfig | defconfig 改动 |
|---|---|
| `project/gd32f10x/Kconfig:23` | `project/gd32f10x/gd32f105-demo/defconfig` 加 `CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y` |
| `project/stm32f0xx/Kconfig:17` | `project/stm32f0xx/stm32f030x8-demo/defconfig` 加同上 |
| `project/stm32f1xx/Kconfig:17` | `project/stm32f1xx/stm32f103c6-demo/defconfig` 加同上 |
| `project/stm32f4xx/Kconfig:17` | `project/stm32f4xx/stm32f401cb-demo/defconfig` 加同上 |
| `project/stm32h7xx/Kconfig:17` | `project/stm32h7xx/stm32h750vb-demo/defconfig` 加同上 |
| `project/gd32vf103x/Kconfig:17` | `project/gd32vf103x/gd32vf103c-demo/defconfig` 加同上 |
| `project/mcxn947/Kconfig:17,24` | `project/mcxn947/mcxn947-demo/defconfig`、`mcxn947-eth-demo/defconfig` 加同上 |
| `project/macos/Kconfig:17` | `project/macos/ehip-test/defconfig` 加同上 |
| `project/linux/Kconfig:17` | `project/linux/ehip-test/defconfig` 加同上 |
| `project/windows/Kconfig:15,24` | `project/windows/ehip-test/defconfig`、`windows/co-ctx-test/defconfig` 加同上 |
| `project/ch58x/Kconfig:17`（注释） | `project/ch58x/ch582-demo/defconfig` 加 `CONFIG_PACKAGE_SCHEDULER_NONE=y` |

Kconfig 改动：`select PACKAGE_EVENTHUB_OS` → `select PACKAGE_SCHEDULER_EVENTHUB_OS`（当时是 no-op），
现已改为 `select PACKAGE_SCHEDULER_REQUIRE_EVENTHUB_OS`（生效）。

### 依赖型外部包

`ehip` / `ehshell` / `argparse` / `ehip-tools` 四个 Kconfig 把 `depends on PACKAGE_EVENTHUB_OS` 改为 `depends on PACKAGE_SCHEDULER_EVENTHUB_OS`。等价变换，但语义更清晰。

### 当时新增的 FreeRTOS 示例

- `project/mcxn947/freertos-demo/`：`select PACKAGE_SCHEDULER_REQUIRE_FREERTOS`（项目 Kconfig select 的是请求位，不是 choice 成员）。

### 当时的中断及组件边界

| 项 | 内容 |
|---|---|
| `mcxn947-demo/board/clock_cnt.c:21` | 含 `SysTick_Handler` 强符号，freertos-demo 不可复用 |
| `target_include_directories(eventhub PUBLIC "app/inc")` | FreeRTOS demo 不包含 eventhub target，因此不应添加此配置 |
| 启动文件 `startup_MCXN947_cm33_core0.S` | 含 weak spin-loop；FreeRTOS port.c 强符号自动覆盖，**不动启动文件** |

## 6. 相关文档

- [Kconfig 硬性规则](kconfig-rules.md)
- [FreeRTOS 首次移植设计](../design/freertos-port.md)
- [FreeRTOS queue-set 后端](../design/queueset-backend.md)
- [OS 包并入 scheduler/ 的迁移方案](../plans/scheduler-os-merge.md)
