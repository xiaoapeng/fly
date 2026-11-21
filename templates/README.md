# 独立工程模板

`./fly.sh new` 从三个目录组装一份独立工程。Windows 下使用 `fly.bat new`。

| 目录 | 内容 | 生成位置 |
| --- | --- | --- |
| `scaffold/` | 共用的构建脚本、CMake、Kconfig 和 README 骨架 | 工程根目录 |
| `apps/<模板>/` | 应用入口、默认配置和 `template.json` 清单 | `app/` 和 `defconfig` |
| `apps/common/nanosig/` | 两个 nanosig 模板共用的运行时和定时器示例 | `app/` |
| `boards/<板卡>/` | 板卡信息、初始化、链接脚本和烧录配置 | `board/` |
| `boards/common/stm32/` | STM32 板卡共用的初始化、日志适配和默认链接脚本 | `board/` |

新增模板时，在 `apps/<模板>/` 放置默认配置、应用源码和 `template.json`。生成器发现清单，使用目录名作为模板 ID。

- `label` 指定显示名称。可选的 `expected_log` 用于生成启动提示。可选的 `overlays` 按顺序列出需要复制的公共目录，路径以 `apps/` 为起点。
- `board_bindings` 可将默认配置中的 `@TOKEN@` 绑定到板卡清单字段。每个绑定声明 `field` 和 `type`，类型支持 `positive_integer`、`integer` 和 `string`。生成器复制前校验字段和值。
- `expected_config` 和 `required_files` 供生成回归检查使用，分别列出配置行与生成工程中的相对文件路径。

新增板卡时，在 `boards/` 提供 `board.json`、`board.cmake` 和应用使用的板级接口。用 `board.json` 的 `templates` 声明已验证的模板组合。宿主板卡还通过 `host_platform` 指定对应的 CI 平台。

板卡清单的可选 `overlays` 字段按顺序列出相对 `boards/` 的共享目录。生成器先把这些目录复制到应用的 `board/`，再复制具体板卡的目录。同名文件以最后复制的为准。STM32 板卡通过 `"overlays": ["common/stm32"]` 复用同一份板级源码和脚本。

生成后的应用以根目录 `CMakeLists.txt` 为入口，调用 SDK 的 CMake 模块准备 Kconfig、工具链和依赖包。根目录的 CMake 文件定义固件、调试和打包目标。`app/CMakeLists.txt` 登记应用源码、公开头文件的搜索路径 `app/include/` 和可选的应用配置。

板级头文件放在 `board/include/`。板卡清单 `board.json`、构建配置 `board.cmake` 和板卡自带的链接脚本放在 `board/`。板卡可以提供 `board/defconfig` 作为默认硬件配置，例如启用 RTT。

STM32 的 `board/board.cmake` 声明芯片目标和 Flash/RAM 布局，`board/flash.ld.in` 使用这些参数生成链接脚本。日志的实现位于应用的 `board/log.c`，默认用 RTT，关闭 `CONFIG_PACKAGE_SEGGER_RTT` 时日志调用为空。需要串口日志时，可在应用仓实现自己的 `board_log()`。CMSIS、HAL 和芯片启动代码由 SDK 提供。

已有 STM32 工程升级 SDK 时，先用同款板卡的新模板更新 `board/`，合回应用自定义的内存布局和板级配置，再更新 `fly.lock`。

使用应用构建脚本时，嵌入式 ELF 和 BIN 分别位于 `build/cmake/fly_app` 和 `build/cmake/fly_app.bin`。打包产物位于应用根目录的 `image/`，当前固件包通过 `image/CURRENT/` 访问。清理 `build/` 后仍可访问已打包的固件。

根目录通过 `tool/cmake/app-image.cmake` 中的 `app_add_jlink_image()` 声明打包目标。应用模块将板卡的 `FLY_JLINK_DEVICE` 传给 SDK 的 `add_jlink_image()`。`FIRMWARE_LIST` 的每一项采用 `NAME:VERSION:ADDRESS:CAPACITY:BIN`。`APP` 条目的容量取自板卡的 `FLY_FLASH_SIZE`。加入 bootloader 时，追加 `"BOOT:${BOOT_VERSION}:${BOOT_FLASH_ADDRESS}:${BOOT_FLASH_SIZE}:${BOOT_BIN}"`，若工程需要生成该 BIN，再将相应目标加入 `DEPENDS`。分区的烧录地址和容量应与 Flash 布局一致，且分区互不重叠。

分区容量使用十进制或 `0x` 十六进制字节数。打包工具校验包含对齐填充和元数据尾部的最终 BIN，超过容量就停止打包。`CONFIG_FACTORY_DATA_SUPPORTED` 控制板上读取和日志，打包始终检查容量。

OpenOCD 的 RTT 搜索范围由板卡或应用通过 `RTT_ADDR_AND_SIZE` 传给打包目标。省略该范围时，包包含烧录脚本。

应用根目录调用 `tool/cmake/app-vscode.cmake` 中的 `app_add_vscode_debug(fly_app)`。应用模块检查板卡的 `FLY_JLINK_DEVICE` 以及 GDB 和 objdump，再将 RTT 等调试参数交给 SDK。宿主工程和未配置 J-Link 的板卡会跳过 J-Link 打包及调试目标。缺少 GDB 或 objdump 时，应用模块跳过调试配置生成。存在 `default_flash` 时，应用模块会启用先烧录 BIN 再附加的配置。

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
| `app/biz/app_demo.c` | `app/include/biz/app_demo.h` | nanosig 定时器业务示例 |
| `app/mid/firmware_info.c` | `app/include/mid/firmware_info.h` | 读取并记录固件尾部信息，各模板共用 |

单文件业务和中间件直接放到 `app/biz/` 或 `app/mid/`。模块需要多个源文件时，再建模块子目录，私有头文件跟实现放在一起。应用的公开头文件统一从 `app/include/` 搜索，例如 `#include "biz/app_demo.h"` 和 `#include "mid/firmware_info.h"`。裸机模板用 `.gitkeep` 保留 `app/biz/` 和 `app/include/biz/`，便于直接添加业务。

## nanosig 应用运行时

`app/main.c` 中的裸机入口或 FreeRTOS 应用任务直接调用 `app_runtime_init()`，然后把 `app_runtime_get_loop()` 返回的原生 `ns_loop_t *` 传给 `app_demo_init()`。初始化成功后，`app_runtime_run()` 在裸机上轮询事件循环，在 FreeRTOS 应用任务中运行阻塞的事件循环。

循环退出后，入口依次调用 `app_demo_deinit()` 和 `app_runtime_deinit()`。业务模块负责创建和释放自己的定时器、信号连接等资源。增加业务时依次初始化各模块，退出时按相反顺序释放，最后销毁运行时。业务需要独立启停时，由业务模块提供 `start` 和 `stop`，由入口在运行前后调用。示例 `app/biz/app_demo.c` 在定时器回调中打印 `nanosig ready` 并请求退出，运行和清理均成功后打印 `nanosig stopped`。

模板将 nanosig 的原生调试输出接到 RTT 通道 0，示例用 `ns_sysln()` 打印 `nanosig ready`。默认日志等级是 System，可在 `menuconfig` 的 nanosig 菜单调整。回收完成的提示由板级日志输出。
