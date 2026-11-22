# 独立工程模板

运行 `./fly.sh new` 打开生成菜单。Windows 使用 `fly.bat new`。在 **Project path** 填写新工程目录，最后一级目录名用于生成工程名和固件名。命令行传入工程名、`--board` 和 `--template` 生成基础工程，再用一个或多个 `--pack` 选工具包。运行 `./fly.sh packs` 查看清单，可用 `--board` 和 `--template` 筛选。

| 目录 | 内容 |
| --- | --- |
| `scaffold/` | 共用构建脚本、CMake、Kconfig 和应用骨架 |
| `boards/<板卡>/` | 芯片、CPU 核、内存范围、板级初始化和链接布局 |
| `apps/<模板>/` | 应用入口、组件配置及可用的板卡能力要求 |
| `boards/shared/<包>/` | 多块板卡共用的板级源码和链接脚本 |
| `apps/shared/<包>/` | 多个 APP 模板共享的实现 |
| `tools/<工具>/` | 工具声明的动作、依赖探测、运行脚本、模型和 VS Code 片段 |
| `logging/<实现>/` | 工具包按需引入的固件侧日志实现 |

生成器递归查找 `pack.json`，由清单中的 `id` 和 `kind` 确定包的身份和类别，同一个 ID 只能声明一次。每个清单还需登记 `schema_version`、`label` 和 `files`，并按需声明能力、依赖、兼容组合和动作。`support` 类别的包由依赖引入，目录按职责组织。文件写入应用前校验路径及所有权，同一目标路径只能由一个包提供。

应用仓的 `fly.project.json` 记录选包及动作默认值，包清单按 ID 安装到 `tool/packs/`。J-Link 和 Renode 的包 ID 分别是 `tool/jlink` 和 `tool/renode`，在生成菜单 **Tools** 各选一次。新增探针或仿真器时，把工具探测、器件映射、启动命令和模型放进它自己的工具包，按需声明 `flash-run`、`log`、`debug` 或 `export`。公共脚本按 `(action, provider)` 分派并逐项检查本机的工具状态，应用中用 `./build.sh capabilities` 查看结果。

J-Link 工具包的 `assets/` 存放 SDK 随附的 Git LFS 二进制。生成应用时只复制清单 `files` 中列出的文件，J-Link 在运行和按需导出离线 ZIP 时从 SDK 读取这些资源。

板卡与 APP 通过 `provides` 和 `requires` 声明能力，工具包可声明器件表和仿真模型的适用范围。多个工具提供同一动作时，**Default providers by action** 才为该动作显示默认项，初始保留 **Automatic**。

板卡的 `board.arch` 引用 SDK 中的 `arch.json` ID。各架构目录提供自己的 `gcc-config.cmake`、`CMakeLists.txt` 和可选的能力 `Kconfig`。Kconfig 和 CMake 使用同一个板卡选定的架构。

板级接口和头文件位于 `board/`、`board/include/`。根 CMake 创建固件目标，按板卡、依赖、APP 和工具包的顺序装配。APP 配置可用 `board_bindings` 将 `@TOKEN@` 绑定到板卡数据，请声明所需类型。日志通过 `fly_log_write` 输出。依赖 `support/rtt` 的工具包会启用 RTT 日志，用户可以在应用 Kconfig 中关掉日志实现。

`fly_add_firmware_image()` 是应用根 `CMakeLists.txt` 声明通用固件包的入口。每条 `FIRMWARE_LIST` 使用 `NAME:VERSION:ADDRESS:CAPACITY:BIN`。加入 Bootloader 时追加 BOOT 条目，需要编译的目标放入 `DEPENDS`，再用 `SYMBOLS BOOT <目标>` 打包同次构建的 ELF 与调试文件。公共包位于 `image/<target>/<build-type>/<artifact-id>/`，内含 CRC 尾部的最终 BIN 和 `manifest.json`。各工具包在自己的清单中声明 `export`，并复用其中的文件。

烧写与仿真共用 `flash-run` 动作。用 `./build.sh flash-run --list` 查看提供者，指定 `--via jlink` 或 `--via renode` 后加载并启动固件。两边都可传入同一 `--artifact`，按清单校验最终固件。Renode 当前覆盖 FRDM-MCXN947 Core 0 三个 APP 的启动、CRC 和适用的 nanosig 心跳。

Renode 默认持续运行并实时输出 RTT，按 `Ctrl+C` 停止。`--duration-ms` 可指定结束时的虚拟时间。运行中的 `./build.sh log --via renode` 会跟随同一份 RTT，结束后可用同一命令回看 `image/logs/renode/` 中的记录。

配置应用时按已选工具包声明的调试配置生成 VS Code 配置。运行 `./build.sh vscode --artifact /path/to/manifest.json` 可绑定历史归档。

## 板卡与宿主平台

| 目标 | `--board` | 模板 |
| --- | --- | --- |
| FRDM-MCXN947 Core 0 | `frdm-mcxn947-core0` | `baremetal`、`baremetal-nanosig`、`freertos-nanosig` |
| STM32 | `stm32f030x8`、`stm32f103x6`、`stm32f401cb`、`stm32h750vb` | `baremetal` |
| GD32 Cortex-M | `gd32f103rc`、`gd32e230c8` | `baremetal` |
| 宿主平台 | `host-linux-x64`、`host-macos-arm64`、`host-windows-x64` | `baremetal` |

宿主配置使用原生工具链构建最小程序。CI 的覆盖范围和本地执行方法见 [CI 流程](../docs/CI.md)。

三个嵌入式模板默认启用 `CONFIG_FACTORY_DATA_SUPPORTED`。共用的 `app/mid/firmware_info.c` 按板卡提供的 `FLY_FLASH_ADDRESS` 和 `FLY_FLASH_SIZE` 查找打包时追加的尾部。新增板卡时，这两个值应覆盖链接脚本中当前应用可用的 Flash 区域。宿主配置默认关闭该选项。

## 应用目录

| 源文件 | 公开头文件 | 职责 |
| --- | --- | --- |
| `app/main.c` | | 嵌入式应用入口，按顺序调用各模块的生命周期函数 |
| `app/runtime.c` | `app/include/runtime.h` | nanosig 运行时，仅在 nanosig 模板生成 |
| `app/biz/heartbeat.c` | `app/include/biz/heartbeat.h` | nanosig 心跳业务 |
| `app/mid/firmware_info.c` | `app/include/mid/firmware_info.h` | 读取并记录固件尾部信息，各模板共用 |

单文件业务和中间件直接放到 `app/biz/` 或 `app/mid/`。模块需要多个源文件时，再建模块子目录，私有头文件跟实现放在一起。应用的公开头文件统一从 `app/include/` 搜索，例如 `#include "biz/heartbeat.h"` 和 `#include "mid/firmware_info.h"`。裸机模板用 `.gitkeep` 保留 `app/biz/` 和 `app/include/biz/`，便于直接添加业务。

## nanosig 应用运行时

`app/main.c` 中的裸机入口或 FreeRTOS 应用任务直接调用 `app_runtime_init()`，然后把 `app_runtime_get_loop()` 返回的原生 `ns_loop_t *` 传给 `heartbeat_init()`。初始化成功后，`app_runtime_run()` 在裸机上轮询事件循环，在 FreeRTOS 应用任务中运行阻塞的事件循环。

事件循环运行期间，`app/biz/heartbeat.c` 用 nanosig 周期定时器每秒调用一次 `ns_sysln("heartbeat")`。模板将 nanosig 的原生日志接到所选日志实现，默认日志等级为 System，可在 `menuconfig` 的 nanosig 菜单调整。

循环退出时，入口先调用 `heartbeat_deinit()` 释放心跳定时器，再调用 `app_runtime_deinit()`。新增业务时，由各模块管理自己的定时器、信号连接等资源，入口按初始化的相反顺序释放模块。业务需要独立启停时，由模块提供 `start` 和 `stop`，入口在运行前后调用。
