# OS 包并入 `package/scheduler/` 迁移方案（v2）

> 状态：**已实施**（含 UX 修订：choice 与子配置包在 `menu "Scheduler / OS model"` 内）。
>
> 修订记录：
> - v2：删去 `PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS` 内部 bool，所有依赖改用 choice 成员 `PACKAGE_SCHEDULER_*`；同步修掉 v1 评审中的 `source` → `rsource`、malformed 测试用例、docs 段落三个问题。
> - 实施追加：`package/scheduler/Kconfig` 外层加 `menu "Scheduler / OS model"`，choice 与子配置在 menuconfig 中收进同一菜单分支；`arch/stm32/Kconfig` vendor hub 承接 stm32 各 family 能力文件，顶层只保留一条 `rsource "*/Kconfig"`。

## 背景与目标

现在 OS 包分散在 `package/eventhub-os/` 与 `package/freertos/`，并在 `package/scheduler/` 之外。menuconfig 中用户会同时看到 `Concurrency / scheduler model`（choice）和两个并列的 `eventhub os kernel support` / `FreeRTOS kernel support` toggle，UX 不一致。

**目标**：把两个 OS 包整建制搬到 `package/scheduler/eventhub-os/` 与 `package/scheduler/freertos/`，让 `package/scheduler/` 成为 OS 单一入口。`PACKAGE_SCHEDULER` choice 是用户唯一的 OS 入口；选完 OS 后才看到对应 OS 的子配置。

进一步：**不再保留 `PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS` 内部 bool**——所有依赖（其他包 CMake、依赖型 Kconfig）都改用 choice 成员 `PACKAGE_SCHEDULER_EVENTHUB_OS` / `PACKAGE_SCHEDULER_FREERTOS`。这样模型最干净，没有"内部 vs 外部"两套标识符。

## 设计决策（已与用户对齐）

| # | 决策点 | 选择 |
|---|---|---|
| 1 | OS 包物理位置 | 整体搬入 `scheduler/eventhub-os/` 与 `scheduler/freertos/`，连同子目录与 CMakeLists |
| 2 | 用户交互 | 单一 `choice PACKAGE_SCHEDULER`，按所选 leg 用 `if PACKAGE_SCHEDULER_* \n rsource "..." \n endif` 条件引入对应子 Kconfig |
| 3 | 标识符模型 | **完全删除 `PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS`**；其他包/CMake 一律依赖 choice 成员 `PACKAGE_SCHEDULER_EVENTHUB_OS` / `PACKAGE_SCHEDULER_FREERTOS` |
| 4 | FreeRTOS 自拉取 | 在 `scheduler/freertos/CMakeLists.txt` 内 `fetch_git_project` |
| 5 | scheduler/CMakeLists | `add_subdirectory(eventhub-os)` + `add_subdirectory(freertos)`，由子包内部 `if(CONFIG_PACKAGE_SCHEDULER_*)` 自行 gate |
| 6 | docs | 同步更新三份文档路径；删除已过时的 `docs/plans/freertos-migration.md` |

## 目标结构

```
package/scheduler/
├── CMakeLists.txt          # add_subdirectory(eventhub-os) + add_subdirectory(freertos)
├── Kconfig                 # menu "Scheduler / OS model" { choice + if … rsource … }
├── eventhub-os/            # ← 从 package/eventhub-os/ 整体搬入
│   ├── Kconfig             # 仅 if PACKAGE_SCHEDULER_EVENTHUB_OS 包住 global timer 信号
│   └── CMakeLists.txt      # fetch_git_project(eventhub) + package_lib_add（gate 改为 SCHEDULER_*）
└── freertos/               # ← 从 package/freertos/ 整体搬入
    ├── Kconfig             # 仅 if PACKAGE_SCHEDULER_FREERTOS 包住 version/heap/tick/…
    ├── CMakeLists.txt      # fetch_git_project + 4-target split（gate 改为 SCHEDULER_*）
    ├── README.md
    └── inc/FreeRTOSConfig.h
```

`package/Kconfig` 的 `rsource "*/Kconfig"` 会匹配 `package/*/Kconfig`，自动找到 `package/scheduler/Kconfig`；子包 Kconfig（`scheduler/eventhub-os/Kconfig`、`scheduler/freertos/Kconfig`）不会被同一 glob 匹配（双段路径不匹配单段 `*/Kconfig`），由 `scheduler/Kconfig` 内的 `rsource` 引入。

## menuconfig 体验（实施后实际效果）

```
Target Packages
├── argument parser / EHIP Tools / ehip / ehshell / Factory Data / SEGGER RTT ...
└── Scheduler / OS model
    └── Concurrency / scheduler model
        ├── [ ] None (bare-metal main loop)
        ├── [ ] eventhub-os (cooperative signal/slot framework)
        │      └── eventhub global timer signal (1s / 500ms / 100ms)
        └── [ ] FreeRTOS (preemptive kernel)
               └── FreeRTOS-Kernel version
                   └── Memory allocator / Tick rate / Max priorities
                       / Total heap / Min stack / SysTick clock
                       / Software timers / Stack overflow check / Tickless idle
```

选 None 时只有 3 个 choice 项可见，其余子配置全部隐藏；`PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS` 已删除，menuconfig 中不再出现两个并列 toggle。

## 改动清单（按依赖顺序）

1. **物理迁移**：
   - `git mv package/eventhub-os/Kconfig package/eventhub-os/CMakeLists.txt` → `package/scheduler/eventhub-os/`
   - `git mv package/freertos/Kconfig package/freertos/CMakeLists.txt package/freertos/README.md package/freertos/inc/` → `package/scheduler/freertos/`

2. **`package/scheduler/CMakeLists.txt`**：
   ```cmake
   cmake_minimum_required(VERSION 3.10)

   # Scheduler mutual exclusion is resolved by the Kconfig choice in this
   # directory. Each OS sub-package self-gates with
   # `if(CONFIG_PACKAGE_SCHEDULER_<leg>)` so add_subdirectory is unconditional.
   add_subdirectory(eventhub-os)
   add_subdirectory(freertos)
   ```

3. **`package/scheduler/Kconfig`**：保留 `choice PACKAGE_SCHEDULER` 现状，**删除 `select PACKAGE_EVENTHUB_OS` 与 `select PACKAGE_FREERTOS` 行**（kconfiglib 中 choice member 的 select 本来就是 no-op，且目标符号已删除），在 `endchoice` 之后追加：
   ```kconfig
   if PACKAGE_SCHEDULER_EVENTHUB_OS
       rsource "eventhub-os/Kconfig"
   endif
   if PACKAGE_SCHEDULER_FREERTOS
       rsource "freertos/Kconfig"
   endif
   ```

4. **`package/scheduler/eventhub-os/Kconfig`**：删除 `menuconfig PACKAGE_EVENTHUB_OS` 与它单独的 `if PACKAGE_EVENTHUB_OS` gate；保留所有 `PACKAGE_EVENTHUB_GLOBAL_TIMER_SIGNAL_*` 子项，**用 `if PACKAGE_SCHEDULER_EVENTHUB_OS` 包住**整个文件。

5. **`package/scheduler/freertos/Kconfig`**：删除 `config PACKAGE_FREERTOS` 与 `if PACKAGE_FREERTOS` gate；保留所有 tunables，**用 `if PACKAGE_SCHEDULER_FREERTOS` 包住**整个文件；同时去掉"`!PACKAGE_EVENTHUB_OS` safety net"行（依赖目标已删除）。

6. **`package/scheduler/eventhub-os/CMakeLists.txt`**：`if(CONFIG_PACKAGE_EVENTHUB_OS)` → `if(CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS)`。

7. **`package/scheduler/freertos/CMakeLists.txt`**：`if(CONFIG_PACKAGE_FREERTOS)` → `if(CONFIG_PACKAGE_SCHEDULER_FREERTOS)`；删掉顶部"`!PACKAGE_EVENTHUB_OS` safety net"注释段。

8. **4 个外部包 Kconfig 改为依赖 choice 成员**：
   - `package/ehip/Kconfig:9` `depends on PACKAGE_EVENTHUB_OS` → `depends on PACKAGE_SCHEDULER_EVENTHUB_OS`
   - `package/ehip-tools/Kconfig:6` 同上
   - `package/ehshell/Kconfig:9` 同上
   - `package/argparse/Kconfig:7` 同上

9. **`project/ch58x/Kconfig:17`**：注释掉的 `#select PACKAGE_EVENTHUB_OS` 改为 `#select PACKAGE_SCHEDULER_EVENTHUB_OS`（保持注释）。

10. **`tool/test/check_freertos_config.py`**：
    - `check_legacy_eventhub`：把 `expect(k, "PACKAGE_EVENTHUB_OS", "y")` 改为 `expect(k, "PACKAGE_SCHEDULER_EVENTHUB_OS", "y")`；把"`PACKAGE_FREERTOS`"断言改为对应 choice member。
    - `check_freertos`：同理。
    - `check_none`：把"`PACKAGE_EVENTHUB_OS`"断言改为 "`PACKAGE_SCHEDULER_NONE`" 已为 y。
    - `check_malformed_safety`：合成用例从 `CONFIG_PACKAGE_EVENTHUB_OS=y\nCONFIG_PACKAGE_FREERTOS=y` 改为 `CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y\nCONFIG_PACKAGE_SCHEDULER_FREERTOS=y`；期望 scheduler 归一为一个腿，剩余 OS 包状态正确。

11. **docs 三份文档**：
    - `docs/design/freertos-port.md`：模块边界小节写"`package/scheduler/eventhub-os/`、`package/scheduler/freertos/`"；依赖段落"`PACKAGE_EVENTHUB_OS`"全部改为"`PACKAGE_SCHEDULER_EVENTHUB_OS`"；`!PACKAGE_EVENTHUB_OS` safety net 段落删除；scheduler choice 不再列 select 行。
    - `docs/reference/rtos-choice.md`：同理；包内 4-lib 链接链小节明确"`freertos-config`/`-heap`/`-kernel`/`-port` 现在注册于 `package/scheduler/freertos/CMakeLists.txt`"。
    - `docs/plans/freertos-migration.md`：删除。
    - `docs/README.md`：索引里删 `plans/freertos-migration.md` 一行；保留两条现有索引。

12. **`arch/Kconfig:66`**：注释里的"`PACKAGE_FREERTOS`"是 generic architecture fact 的描述，无需改。

## 验证步骤

1. **Kconfig sweep**：沙箱内已跑过 16 个 defconfig。期望：
   - ch582-demo / gd32e23x-demo / gd32f103-demo：NONE 选腿，ehip=ehshell=n。
   - 11 个 EH defconfig：EH 选腿，ehip / ehshell 按各自 defconfig 内容出现。
   - freertos-demo：FR 选腿。
   - `PACKAGE_EVENTHUB_GLOBAL_TIMER_SIGNAL` 仅在 EH 选腿时 `visibility=2`；`PACKAGE_FREERTOS_TICK_RATE_HZ` 仅在 FR 选腿时 `visibility=2`。
2. **`tool/test/check_freertos_config.py`**：15 checks 全 PASS（用例按上一步 #10 改）。
3. **Synthetic dual-pin**：合成 `CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y\nCONFIG_PACKAGE_SCHEDULER_FREERTOS=y` 加载后，kconfiglib 归一 choice 为 FR（default 顺序中 FR 在 EH 之后），PACKAGE_EVENTHUB_GLOBAL_TIMER_SIGNAL visibility=0、ehip=0、ehshell=0。
4. **CMake configure**：每个 defconfig 跑 `./build.sh build Debug`（沙箱无 arm 工具链，跳过实际编译但 cmake configure 必须成功）。
5. **`docs/` 静态检查**：所有提到 `package/freertos/` 的路径（grep）应替换为 `package/scheduler/freertos/`；`PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS` 在 docs/ 中不应再出现。

## 风险与缓解

| 风险 | 缓解 |
|---|---|
| `cmake --build build` 旧缓存引用旧路径 | 文档明确要求 `distclean` 再 `./build.sh build` |
| `dl/eventhub-src/`、`dl/freertos-kernel-src/` 路径随 subproject name 落盘，名字不变 | 验证不重新拉取；用户首次构建时无网会失败（与现状一致） |
| choice member 作为 `depends on` 目标的语义在 kconfiglib 中是否完全等价于普通 bool | 已实测：4 个外部包 defconfig sweep 通过，`ehip`/`ehshell` 等子项 visibility 与 choice 选腿同步 |
| 4 个外部包 Kconfig 现在 `depends on PACKAGE_SCHEDULER_EVENTHUB_OS`——legacy 项目没有 defconfig pin，但 Kconfig `select PACKAGE_SCHEDULER_EVENTHUB_OS` 在 choice member 上是 no-op | 现有 defconfig 已全部 pin `CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y`，实测正常 |
| 删除 `PACKAGE_EVENTHUB_OS` / `PACKAGE_FREERTOS` 内部 bool 后，hand-edited `.config` 中残留的 `=y` 行会被 kconfiglib 静默忽略 | 用户不需要再写这两个标识符；写错则被忽略，行为正确 |
| 删除 `docs/plans/freertos-migration.md` 切断历史 | 已用"同步更新所有文档路径"决策；如有读者需要历史，可在 git reflog 中找回 |

## 实施顺序

1. 创建 `package/scheduler/eventhub-os/` 与 `package/scheduler/freertos/`，git mv 现有文件。
2. 更新 `package/scheduler/CMakeLists.txt`：加 `add_subdirectory`。
3. 更新 `package/scheduler/Kconfig`：删 `select` 行，加 `if … rsource` 块。
4. 重写 `package/scheduler/eventhub-os/Kconfig`：去掉 `menuconfig PACKAGE_EVENTHUB_OS`，外层改 `if PACKAGE_SCHEDULER_EVENTHUB_OS`。
5. 重写 `package/scheduler/freertos/Kconfig`：去掉 `config PACKAGE_FREERTOS` 与 `if PACKAGE_FREERTOS`，外层改 `if PACKAGE_SCHEDULER_FREERTOS`，去掉 `!PACKAGE_EVENTHUB_OS` 行。
6. 更新 `package/scheduler/eventhub-os/CMakeLists.txt`：gate 改 `CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS`。
7. 更新 `package/scheduler/freertos/CMakeLists.txt`：gate 改 `CONFIG_PACKAGE_SCHEDULER_FREERTOS`；删 safety net 注释。
8. 改 4 个外部包 Kconfig：`depends on PACKAGE_EVENTHUB_OS` → `depends on PACKAGE_SCHEDULER_EVENTHUB_OS`。
9. 更新 `tool/test/check_freertos_config.py` 4 个用例（legacy/freertos/none/malformed）。
10. 跑 `tool/test/check_freertos_config.py` + 16 defconfig sweep + synthetic dual-pin。
11. 更新 docs/ 三份文档路径与符号引用；删除 `docs/plans/freertos-migration.md`；更新 `docs/README.md` 索引。
12. distclean + 全量 build 验证。

## 回滚

如迁移后构建失败，回滚步骤：

1. `git revert <merge-commit>` 或手动 `git mv` 把目录搬回原位，Kconfig/CMake 用 `git checkout --` 回滚。
2. `distclean`。
3. 跑 `tool/test/check_freertos_config.py` 验证。
4. 检查 docs/ 是否需要同步回滚。