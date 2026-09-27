# FreeRTOS v1 设计（mcxn947）

## 1. 目标

把 FreeRTOS-Kernel 以 `package/scheduler/freertos/` 形式接入 FLY，v1 仅支持 mcxn947；通过集中式 scheduler choice 与 eventhub-os 互斥；新增自包含 demo 跑 2 个任务并通过硬件 RTT + LED 验证；既有项目零回归、可回滚。

## 2. 模块边界

| 模块 | 角色 | 关键约束 |
|---|---|---|
| `package/scheduler/Kconfig` | 全局 scheduler 注册中心，`choice PACKAGE_SCHEDULER` = none / eventhub-os / FreeRTOS，默认 `PACKAGE_SCHEDULER_NONE` | choice 是唯一互斥 authority；OS 子包按 `if PACKAGE_SCHEDULER_*` rsource 引入 |
| `package/scheduler/eventhub-os/` | eventhub-os 子包，CMake gate `if(CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS)` | 不被其他 OS 包直接引用；只对 `PACKAGE_SCHEDULER_EVENTHUB_OS` 选择响应 |
| `package/scheduler/freertos/` | FreeRTOS kernel 包，CMake gate `if(CONFIG_PACKAGE_SCHEDULER_FREERTOS)` | 不引用具体架构名、不引用 eventhub-os；通过 4 个 target 串接 |
| `arch/mcxn947/Kconfig` | 仅声明通用硬件事实（CPU family/core/FPU/ABI/TrustZone/NVIC） | 不出现 FreeRTOS 专用符号 |
| `arch/Kconfig` | 加 `rsource "*/Kconfig"`（单层 glob，匹配 `arch/<vendor>/Kconfig`） | 必须在 scheduler registry 创建**之前**就位，否则 `ARCH_*` 不可见；多 family 厂商（如 stm32）由 `arch/<vendor>/Kconfig` 内自己的 `rsource "*/Kconfig"` 承接 |
| `project/mcxn947/freertos-demo/` | 自包含 demo，`board/` 自建、2 任务（tick RTT + LED） | 不复用 mcxn947-demo/board/；不维护项目本地 `FreeRTOSConfig.h` |
| `auto-generate/autoconf.h` | kconfiglib 渲染的 `#define CONFIG_<sym> <val>`，全局 include path | 模板通过 `#ifndef/#ifdef/#else` 三层桥接 user-tunable 宏 |

## 3. 4-lib 拆分

```
INTERFACE  freertos-config   ←  公共 include + 全局 macro；**不**进入 FLY_ALL_PACKAGE_LIB_LIST
OBJECT     freertos-heap      ←  portable/MemMang/<heap>.c
OBJECT     freertos-kernel    ←  tasks/queue/list/event_groups/stream_buffer.c（USE_TIMERS=y 加 timers.c）
OBJECT     freertos-port      ←  portable/GCC/ARM_CM33_NTZ/non_secure/{port.c,portasm.c}
```

注册顺序 `heap→kernel→port`；`freertos-config` 经 PUBLIC 链被 executable 拉到（不是 PRIVATE chain）。`cmake/package.cmake` 的 `package_lib_compile_options` 用 `target_compile_options(${lib} PRIVATE ...)`，INTERFACE 上写 PRIVATE 会硬错误 → `freertos-config` 不可 `package_lib_add`。

四个 target 现在注册于 `package/scheduler/freertos/CMakeLists.txt`；其它 OS 子包（eventhub-os 等）各自包内独立 CMake，不与此共享。

## 4. 通用事实 → port 映射

包内 `if(CONFIG_ARCH_CORE STREQUAL "cortex-m33")` + TrustZone mode 分支选择 `portable/GCC/ARM_CM33_NTZ/non_secure`（强制 `configENABLE_TRUSTZONE=0`，避免 NTZ 变体链接时报 `SecureContext_*` 未定义）。架构侧只声明 `ARCH_CORE="cortex-m33"`、`ARCH_TRUSTZONE_MODE="non-secure"`、`ARCH_TRUSTZONE_NON_SECURE=y`、`ARCH_HAS_HARD_FPU=y`、`ARCH_FLOAT_ABI="hard"`、`ARCH_NVIC_PRIO_BITS=3`。用户不选 port。

## 5. 配置注入

不维护独立 renderer 脚本。`tool/python/build.py` 在 `menuconfig`/`saveconfig`/`loadconfig` 时通过 `kconfiglib.write_autoconf()` 写 `auto-generate/autoconf.h`；plain `build` 时 `package/scheduler/freertos/CMakeLists.txt` 调 3 行 inline kconfiglib 再 refresh 一次。

`inc/FreeRTOSConfig.h` 直接 `#include <autoconf.h>`，用以下三层兜底：

```c
#ifndef configTICK_RATE_HZ
#  ifdef CONFIG_PACKAGE_FREERTOS_TICK_RATE_HZ
#    define configTICK_RATE_HZ  CONFIG_PACKAGE_FREERTOS_TICK_RATE_HZ
#  else
#    define configTICK_RATE_HZ  1000
#  endif
#endif
```

① 已定义跳过；② Kconfig 有就走 Kconfig 值；③ Kconfig 没暴露走 curated 默认。项目源码不再 `#include "freertos_inc.h"`，不维护 per-project `FreeRTOSConfig.h`。

## 6. Scheduler 选择器

```
choice PACKAGE_SCHEDULER
    default PACKAGE_SCHEDULER_NONE
    PACKAGE_SCHEDULER_NONE          → 无 scheduler
    PACKAGE_SCHEDULER_EVENTHUB_OS   → 用户主动选 eventhub-os；其他包可 depends on PACKAGE_SCHEDULER_EVENTHUB_OS
    PACKAGE_SCHEDULER_FREERTOS      → depends on ARCH_CPU_FAMILY_CORTEX_M
```

`PACKAGE_SCHEDULER_EVENTHUB_OS` / `PACKAGE_SCHEDULER_FREERTOS` 是 choice member；`package/scheduler/Kconfig` 不再写 `select` 行（kconfiglib 中 choice member 的 select 本来就是 no-op）。`PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS` 内部 bool 已删除；其他包通过 `depends on PACKAGE_SCHEDULER_*` 接入。

12 个 legacy 项目的迁移采用**两步法**：Kconfig 中保留 `select PACKAGE_SCHEDULER_EVENTHUB_OS` 表达意图（no-op），defconfig 中硬写 `CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y` 真正驱动 choice。

`package/scheduler/Kconfig` 在 choice 之后追加条件 `rsource`，整体包在 `menu "Scheduler / OS model"` 内：

```
if PACKAGE_SCHEDULER_EVENTHUB_OS
    rsource "eventhub-os/Kconfig"
endif
if PACKAGE_SCHEDULER_FREERTOS
    rsource "freertos/Kconfig"
endif
```

`rsource` 把路径解析为相对当前文件（`scheduler/Kconfig`），确保子包 Kconfig 在 scheduler 目录内被引入；kconfiglib 的普通 `source` 相对顶层 Kconfig 解析，这里必须用 `rsource`。子包 Kconfig 自身用 `if PACKAGE_SCHEDULER_*` 包裹内容（双重 gate，子包内菜单项仅在对应 scheduler 选中时可见）。

## 7. Legacy 迁移名册

12 个 active 项目 Kconfig 把 `select PACKAGE_EVENTHUB_OS` 改成 `select PACKAGE_SCHEDULER_EVENTHUB_OS`（意图声明，no-op）；对应的 defconfig 加 `CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y`。`project/ch58x/ch582-demo/defconfig` 加 `CONFIG_PACKAGE_SCHEDULER_NONE=y`。`ehip` / `ehshell` / `argparse` / `ehip-tools` 4 个依赖型 Kconfig 把 `depends on PACKAGE_EVENTHUB_OS` 改为 `depends on PACKAGE_SCHEDULER_EVENTHUB_OS`。

## 8. 验证

- 确认 `menuconfig` 能解析配置，FreeRTOS 内核已下载，CMake 配置和 Debug 构建成功。
- 检查 ELF，确认没有未解析的 `xTask*`、`vTask*` 或 `Secure*` 符号；`SysTick_Handler`、`PendSV_Handler`、`SVC_Handler` 各有且仅有一个强符号定义，且来自 FreeRTOS port。
- 确认调度器互斥；手工同时启用两个 scheduler leg 的 `.config` 经 kconfiglib 处理后，归一为声明顺序靠后的 leg（FreeRTOS）。
- 将 `PACKAGE_FREERTOS_TICK_RATE_HZ` 改为 500 后重建，确认配置头文件同步，架构配置正确桥接，demo 不包含项目本地 `FreeRTOSConfig.h`。
- 确认 Release map 文件中的 FreeRTOS `.text` 不超过 16 KiB，`tool/test/check_freertos_config.py` 全部通过（14 个 defconfig + 1 个 synthetic 用例），且 `mcxn947-demo` 不含 FreeRTOS 符号。
- 硬件 RTT 每 500 ms 输出一次 `tick @<n>`，LED 以 1 Hz 闪烁。
- 清理生成文件并移除 FreeRTOS 新增配置后，确认 `mcxn947-demo` 仍可成功构建。

## 9. 风险

- 默认 `NONE` 与既有 eventhub 行为不兼容 → 必须一次性原子迁移所有 legacy 项目 + defconfig；中间状态不允许 build/flash。
- Upstream `-Wconversion/-Wsign-conversion` → 三个 OBJECT lib 在 `package_lib_compile_options` 之后追加 `-Wno-*`，later flags win。
- 手工 dual-pin scheduler leg → Kconfig 归一为声明顺序靠后的 leg；不依赖跨 OS guard。
- 网络/工具链前置 → 沙箱内 `github.com` 不可解析，需切换到有网环境或配置 `FLY_MIRROR_SOURCE_JSON_FILE`。

## 10. 配套实施

实施计划见 `docs/plans/scheduler-os-merge.md`；RTOS 选择快速参考见 `docs/reference/rtos-choice.md`。