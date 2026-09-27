# RTOS 选择快速参考

## 1. Scheduler 选择器

```
choice PACKAGE_SCHEDULER
    default PACKAGE_SCHEDULER_NONE
    PACKAGE_SCHEDULER_NONE          → 无 scheduler（bare-metal main loop）
    PACKAGE_SCHEDULER_EVENTHUB_OS   → eventhub-os（cooperative signal/slot framework）
    PACKAGE_SCHEDULER_FREERTOS      → depends on ARCH_CPU_FAMILY_CORTEX_M
                                       FreeRTOS（preemptive kernel）
```

- choice 是**唯一**互斥 authority；OS 子包按 `if PACKAGE_SCHEDULER_*` rsource 进入。
- 默认 `NONE`：未 pin 的项目不会自动启用 scheduler。
- kconfiglib 对 choice member 的 `select` 是 no-op → 项目 Kconfig 中 `select PACKAGE_SCHEDULER_*` 仅表达迁移意图，真正驱动必须靠 defconfig pin。
- 手工 dual-pin 时（`CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y` + `CONFIG_PACKAGE_SCHEDULER_FREERTOS=y`），kconfiglib 归一为 `scheduler/Kconfig` 中声明顺序靠后的 leg（FreeRTOS）。

## 2. 子包 Kconfig 与 CMake

子包 Kconfig 用 `if PACKAGE_SCHEDULER_*` 双重 gate：

- `package/scheduler/eventhub-os/Kconfig` → `if PACKAGE_SCHEDULER_EVENTHUB_OS`
- `package/scheduler/freertos/Kconfig` → `if PACKAGE_SCHEDULER_FREERTOS`

子包 CMakeLists 用 `if(CONFIG_PACKAGE_SCHEDULER_*)` 自 gate，无 OS 选中时不 fetch、不注册 target。

## 3. 项目迁移名册

### Legacy 两步迁移（12 active 项目）

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

Kconfig 改动统一把 `select PACKAGE_EVENTHUB_OS` 改成 `select PACKAGE_SCHEDULER_EVENTHUB_OS`（仍是 no-op，仅表达迁移意图）。

### 依赖型外部包

`ehip` / `ehshell` / `argparse` / `ehip-tools` 四个 Kconfig 把 `depends on PACKAGE_EVENTHUB_OS` 改为 `depends on PACKAGE_SCHEDULER_EVENTHUB_OS`。等价变换，但语义更清晰。

### 新项目

- `project/mcxn947/freertos-demo/`：`defconfig` 含 `CONFIG_PACKAGE_SCHEDULER_FREERTOS=y`（项目 Kconfig **不** select scheduler leg）。

## 4. 自动验证

`tool/test/check_freertos_config.py` 在每次 defconfig 迁移后 sweep 14 个 defconfig（12 legacy + freertos-demo + ch582-demo），外加 1 个 synthetic dual-leg 安全网用例，断言：

- legacy 项目 → `SCHEDULER_EVENTHUB_OS=y` 且其他 leg `n`
- freertos-demo → `SCHEDULER_FREERTOS=y` 且其他 leg `n`，架构事实齐备
- ch582-demo → `SCHEDULER_NONE=y` 且其他 leg `n`
- malformed dual-leg → scheduler 归一为 `SCHEDULER_FREERTOS=y`，eventhub 子菜单不可见

非零退出并指出失败项与项目路径。

## 5. FreeRTOS 子包内 4-lib 链接链

```
freertos-config (INTERFACE, 不注册)
   ↑ PUBLIC
freertos-heap (OBJECT)
   ↑ PUBLIC
freertos-kernel (OBJECT)
   ↑ PUBLIC
freertos-port (OBJECT)
```

Executable 通过 `target_link_package_lib` 经 PUBLIC 链拉到 `freertos-config` 的 include dirs；只注册 3 个 OBJECT（`freertos-config` 不能 `package_lib_add`，INTERFACE 上 `target_compile_options PRIVATE` 是 CMake 硬错误）。四个 target 现在注册于 `package/scheduler/freertos/CMakeLists.txt`。

## 6. Kconfig 互斥规则

- 正常路径：freertos-demo config `SCHEDULER_FREERTOS=y`；eventhub-pinned 反之。
- 手工 dual-pin 路径：scheduler 归一为 `scheduler/Kconfig` 中声明顺序靠后的 leg（FreeRTOS）。
- CMake 不写跨 OS guard；任何残留 `FATAL_ERROR` 都是回归。

## 7. 与现有项目的边界

| 项 | 内容 |
|---|---|
| `mcxn947-demo/board/clock_cnt.c:21` | 含 `SysTick_Handler` 强符号，freertos-demo 不可复用 |
| `target_include_directories(eventhub PUBLIC "app/inc")` | FreeRTOS demo 不包含 eventhub target，因此不应添加此配置 |
| 启动文件 `startup_MCXN947_cm33_core0.S` | 含 weak spin-loop；FreeRTOS port.c 强符号自动覆盖，**不动启动文件** |

## 8. 相关文档

- FreeRTOS 移植设计：`docs/design/freertos-port.md`
- OS 包并入 scheduler/ 的迁移方案：`docs/plans/scheduler-os-merge.md`