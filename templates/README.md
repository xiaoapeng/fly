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

板卡的 `board.arch` 引用 SDK 中的 `arch.json` ID。架构清单声明 ISA、工具链和 CPU 构建配置。单配置在清单顶层填写 `cpu` 及适用的 `fpu`、`float_abi`、`march`、`abi`。例如 STM32H7 用 `profiles` 分开声明 `cm7` 和 `cm4`，板卡再用 `board.arch_profile` 选择其中一项。Kconfig 和 CMake 共用这个选择，`.config` 保存架构 ID 和配置名，CMake 使用注册表解析出的目录。

各架构的 `gcc-config.cmake` 保留系列专用的额外选项。芯片目标在自身 `CMakeLists.txt` 中绑定设备宏、启动文件、芯片级默认 `SystemInit` 和驱动。板卡选择芯片目标，并提供自己的时钟配置、引脚初始化、HAL 配置和链接布局。可选的架构 `Kconfig` 补充 NVIC 等系列能力，组件包根据架构能力选择自身的实现。

每个板卡包绑定实际焊装的芯片型号和对应的构建目标。运行 `fly.sh new` 时，用户选择板卡，生成的工程沿用该绑定。GD32F10x 系列提供 `arch-gd32f10x-md`、`arch-gd32f10x-hd`、`arch-gd32f10x-xd` 和 `arch-gd32f10x-cl`。例如 GD32F103RC 板卡包绑定 `GD32F103RCT6` 和 `arch-gd32f10x-hd`，该目标同时提供 `GD32F10X_HD` 和对应启动文件。定义自有 CL 板卡包时，可在对应目标上用 `HXTAL_VALUE` 声明板载晶振，厂商默认的 108 MHz 初始化支持 8 MHz 和 25 MHz 晶振。

板级接口和头文件位于 `board/`、`board/include/`。根 CMake 创建固件目标，按板卡、依赖、APP 和工具包的顺序装配。APP 配置可用 `board_bindings` 将 `@TOKEN@` 绑定到板卡数据，请声明所需类型。日志通过 `fly_log_write` 输出。依赖 `support/rtt` 的工具包会启用 RTT 日志，用户可以在应用 Kconfig 中关掉日志实现。

`fly_add_firmware_image()` 是应用根 `CMakeLists.txt` 声明通用固件包的入口。每条 `FIRMWARE_LIST` 使用 `NAME:VERSION:ADDRESS:CAPACITY:BIN`。加入 Bootloader 时追加 BOOT 条目，需要编译的目标放入 `DEPENDS`，再用 `SYMBOLS BOOT <目标>` 打包同次构建的 ELF 与调试文件。公共包位于 `image/<target>/<build-type>/<artifact-id>/`，内含 CRC 尾部的最终 BIN 和 `manifest.json`。各工具包在自己的清单中声明 `export`，并复用其中的文件。

烧写与仿真共用 `flash-run` 动作。用 `./build.sh flash-run --list` 查看提供者，指定 `--via jlink` 或 `--via renode` 后加载并启动固件。两边都可传入同一 `--artifact`，按清单校验最终固件。

### Renode 验证

生成菜单按板卡和 APP 显示可用的 Renode 模型。每个模型声明验证范围及限制。[工具包模型清单](tools/renode/pack.json)记录已可选的组合，[板级平台接入清单](../docs/renode-board-inventory.md)记录上游 Renode 1.17.0 候选。

`startup-smoke` 覆盖最终 BIN 加载、固件启动、尾部 CRC 和 RTT。FRDM-MCXN947 Core 0 的两个 nanosig 模板还验证心跳。运行整个仿真回归可使用 `python3 tool/test/check_standalone_projects.py --matrix renode --build-type Release`。

Renode 默认持续运行并实时输出 RTT，按 `Ctrl+C` 停止。`--duration-ms` 可指定结束时的虚拟时间。运行中的 `./build.sh log --via renode` 会跟随同一份 RTT，结束后可用同一命令回看 `image/logs/renode/` 中的记录。

配置应用时按已选工具包声明的调试配置生成 VS Code 配置。运行 `./build.sh vscode --artifact /path/to/manifest.json` 可绑定历史归档。

## 板卡与宿主平台

| 目标 | `--board` | 模板 |
| --- | --- | --- |
| FRDM-MCXN947 Core 0 | `frdm-mcxn947-core0` | `baremetal`、`baremetal-nanosig`、`freertos-nanosig` |
| STM32 初始目标 | `stm32f030x8`、`stm32f103x6`、`stm32f401cb`、`stm32h750vb` | `baremetal` |
| STM32 开发板 | `stm32f072b-disco`、`stm32f407g-disc1`、`stm32f746g-disco`、`nucleo-f767zi`、`nucleo-h753zi`、`stm32h747i-disco-cm7` | `baremetal` |
| GD32 Cortex-M | `gd32f103rc`、`gd32e230c8` | `baremetal` |
| Nordic nRF52840 | `nrf52840dk`、`arduino-nano-33-ble` | `baremetal` |
| TI CC2538 | `zolertia-firefly` | `baremetal` |
| ADI MAX32652 | `max32652-evkit` | `baremetal` |
| QuickLogic EOS S3 | `qomu`、`quickfeather` | `baremetal` |
| Renesas RA | `renesas-ek-ra2e1`、`arduino-uno-r4-minima` | `baremetal` |
| Silicon Labs EFR32MG26 | `silabs-brd4116a`、`silabs-brd4117a`、`silabs-brd4118a`、`silabs-brd4120a`、`silabs-brd4121a` | `baremetal` |
| Nuvoton NPCX9 | `npcx9m6fb-evb` | `baremetal` |
| 宿主平台 | `host-linux-x64`、`host-macos-arm64`、`host-windows-x64` | `baremetal` |

宿主配置使用原生工具链构建最小程序。CI 的覆盖范围和本地执行方法见 [CI 流程](../docs/CI.md)。

三个嵌入式模板默认启用 `CONFIG_FACTORY_DATA_SUPPORTED`。共用的 `app/mid/firmware_info.c` 按板卡提供的 `FLY_FLASH_ADDRESS` 和 `FLY_FLASH_SIZE` 查找打包时追加的尾部。这两个值对应链接脚本中用于存放 APP 镜像的区域。固件打包默认以板卡声明的 Flash 为边界。如果板卡需要从 SRAM 直接加载 APP，将对应的 RAM 区域声明为 `loadable: true`。J-Link 烧录和导出要求目标分区位于 Flash。宿主配置默认关闭尾部读取。

## Git 源码与补丁

架构和组件通过 `fly_fetch_git_project()` 声明 Git 仓库及版本。`SOURCE_DIR_VAR` 收到实际的 SDK 缓存目录，`SPARSE_PATHS` 可限定要检出的目录。编译和生成文件仍写到各应用自己的构建目录。固定提交的缓存命中可离线构建。

`arch/` 保留 FLY 的构建配置和启动适配。GD32 的固件库及 CH58x 的驱动、启动代码与 ISP 库均从厂商 Git 仓库按固定提交获取。MCXN947 使用 MCUXpresso SDK 25.06.00 清单对应的器件、核心驱动、CMSIS 和 FreeRTOS 驱动适配提交。FLY 对厂商源码的定制通过对应提交的补丁应用。 [GD32F10x](https://github.com/GigaDevice-GD32-MCU/GD32F10x_Firmware_Library) · [GD32E23x](https://github.com/GigaDevice-GD32-MCU/GD32E23x_Firmware_Library) · [GD32VF103](https://github.com/GigaDevice-Semiconductor/GD32VF103_Firmware_Library) · [WCH CH583](https://github.com/openwch/ch583) · [NXP SDK 25.06.00](https://github.com/nxp-mcuxpresso/mcuxsdk-manifests/tree/v25.06.00)

MCXN947 应用直接引用 CMSIS Armv8-M MPU 头文件时，使用 `m-profile/armv8m_mpu.h`。

SDK 的 `PATCH_DIR` 和应用的 `patches/<依赖名>/` 各自先按文件名应用根目录的补丁，再应用 `<完整提交>/` 下的补丁。SDK 补丁先于应用补丁。缓存目录以依赖名、提交和补丁内容区分，修改文件会选中新的目录。应用只需通过各组件现有的版本设置选择 commit。

`dl/` 中的源码供多个构建复用。需要定制时把差异维护为补丁，避免其他应用同时读到手工更改。可用 `package_update -l` 核对实际使用的提交、补丁标识和路径。

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
