# 独立工程模板

`./fly.sh new` 从三个目录组装一份独立工程。Windows 下使用 `fly.bat new`。

| 目录 | 内容 | 生成位置 |
| --- | --- | --- |
| `scaffold/` | 共用的构建脚本、CMake、Kconfig 和 README 骨架 | 工程根目录 |
| `apps/<模板>/` | 应用入口、调度器和包的默认配置 | `app/` 和 `defconfig` |
| `boards/<板卡>/` | 板卡信息、初始化、链接脚本和烧录配置 | `board/` |

新增运行方式时，在 `apps/` 添加应用入口和默认配置，并在生成器里注册模板名。新增板卡时，在 `boards/` 提供 `board.json`、`board.cmake` 和应用使用的板级接口，并在 `board.json` 中声明已经验证的模板组合。

生成后的应用以根目录 `CMakeLists.txt` 为入口，调用 SDK 的 CMake 模块准备 Kconfig、工具链和依赖包。各模板的编译目标定义在 `app/CMakeLists.txt`。板卡可以提供 `board/defconfig` 作为默认硬件配置，例如启用 RTT。

## 板卡与宿主平台

| 目标 | `--board` | 模板 |
| --- | --- | --- |
| FRDM-MCXN947 Core 0 | `frdm-mcxn947-core0` | `baremetal`、`baremetal-nanosig`、`freertos-nanosig` |
| STM32 | `stm32f030x8`、`stm32f103x6`、`stm32f401cb`、`stm32h750vb` | `baremetal` |
| GD32 Cortex-M | `gd32f103rc`、`gd32e230c8` | `baremetal` |
| 宿主平台 | `host-linux-x64`、`host-macos-arm64`、`host-windows-x64` | `baremetal` |

宿主配置使用原生工具链构建最小程序。CI 的覆盖范围和本地执行方法见 [CI 流程](../docs/CI.md)。
