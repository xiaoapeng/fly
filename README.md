# FLY

## 项目介绍

本项目为单片机提供一站式开发平台，包括：编译、烧写、调试、发布打包等功能。

本项目的前身本来是为[eventhub-os](https://github.com/xiaoapeng/eventhub_os)做的一些验证demo，因为不想重复的维护多个仓库，想从重复的工作解脱出来(复制、粘贴、编写launch.json)，慢慢的发展成了现在这个样子。

本项目由cmake和一系列python脚本组成，使用Kconfig系统管理配置，可以通过./build.sh menuconfig(./build.bat menuconfig)来使能和失能包，本项目支持在多个平台下进行交叉编译，如：linux、windows、mac os。

站在巨人的肩膀上，配合vscode使用，可以擦出哪些火花呢？本项目可以搭配vscode插件[Cortex-Debug](https://github.com/Marus/cortex-debug.git)、[CMake Tools](https://github.com/microsoft/vscode-cmake-tools)、[clangd](https://github.com/clangd/vscode-clangd),实现一键编译、一键烧写、一键调试、一键发布打包、完美跳转等功能。

## 使用教程

### 快速开始

在 SDK 目录创建应用，然后进入生成的工程目录编译。首次构建自动加载应用的 `defconfig`。

```sh
./fly.sh new sensor-node --board frdm-mcxn947-core0 --template baremetal-nanosig
cd ../fly-project/sensor-node
./build.sh build Debug
./build.sh make_img
```

Windows 在 SDK 中使用 `fly.bat new`，在应用中使用 `build.bat`。应用里的 `build.sh` / `build.bat` 调用 SDK 的 Python 构建工具。首次使用请安装 Git、Python 3、CMake 3.21 或更新版本以及目标工具链。

克隆已有应用后，在应用目录运行 `./build.sh setup` 安装 `fly.lock` 指定的 SDK。Windows 使用 `build.bat setup`。如果构建提示 SDK 版本与锁文件不一致，重跑 setup。手动指定过 SDK 时，先解除该路径设置，或切到锁定的提交。 [SDK 安装与版本检查](#8-从模板创建独立工程)

### 1. clone项目到本地

```bash
git clone https://github.com/xiaoapeng/fly.git
cd fly
```

### 2. 安装 Python 3

请将 Python 3 加入 PATH，或通过 `FLY_PYTHON` 指定解释器。创建模板只需要 Python 标准库。应用首次构建时会安装缺失的 `kconfiglib`、`requests`。可以设置 `FLY_AUTO_SETUP_PY=0` 关闭自动安装。Windows 的 `menuconfig` 还需要在同一环境安装 `windows-curses`。

### 3.安装cmake

#### windows 安装cmake

- [下载](https://cmake.org/download/)cmake,安装时请将cmake加入全局环境变量，并重启设备。

#### linux 安装cmake

- 使用命令 sudo apt install cmake

    ```bash
    sudo apt install cmake
    ```
#### mac os 安装cmake

- 使用命令 brew install cmake

    ```bash
    brew install cmake
    ```

### 4. 根据项目需要下载交叉编译器

下载后请自行解压到你自定义的安装目录。

|平台|交叉编译器前缀|下载链接|
| --- | --- | --- |
|stm32fx<br>gd32fx<br>mcxn947<br>gd32e23x|arm-none-eabi|[下载](https://developer.arm.com/downloads/-/gnu-rm)|
|ch58x|riscv-none-elf|[下载](http://www.mounriver.com/download/)|
|gd32vfx|riscv-none-embed|[下载](https://github.com/xpack-dev-tools/riscv-none-embed-gcc-xpack/releases/tag/v10.2.0-1.2)|

### 5. 根据项目需要下载调试器-Jlink

- [下载](https://www.segger.com/downloads/jlink/)
后进行安装，不同平台直接安装方式略有差别，请自行百度(谷歌)。

    windows 安装成功后

    ![alt text](resource/image0.png)

    linux安装成功后运行 JLinkExe

    ```bash
    user@:~/project/fly$ JLinkExe 
    SEGGER J-Link Commander V7.98h (Compiled Sep 11 2024 14:26:16)
    DLL version V7.98h, compiled Sep 11 2024 14:25:50

    Connecting to J-Link via USB...FAILED: Cannot connect to J-Link.
    J-Link>
    ```

### 6. 根据项目需要下载调试器-openocd

#### ubuntu  安装方式

- 使用如下命令

    ```bash
    sudo apt install  openocd
    ```

- 安装成功后运行如下命令

    ```bash
    simon@:~/project/fly$ openocd -v 
    Open On-Chip Debugger 0.11.0
    Licensed under GNU GPL v2
    For bug reports, read
            http://openocd.org/doc/doxygen/bugs.html
    ```

#### windows 安装方式

- [下载](https://github.com/xpack-dev-tools/openocd-xpack/releases)后解压到你自定义的安装目录。
- 添加进环境变量并重启设备

- 安装成功后打开cmd运行命令

    ```bash
    PS C:\Users\user> openocd.exe -v
    xPack Open On-Chip Debugger 0.12.0+dev-01685-gb9224c0c0-dirty (2024-08-02-19:51)
    Licensed under GNU GPL v2
    For bug reports, read
            http://openocd.org/doc/doxygen/bugs.html
    PS C:\Users\user>
    ```

### 7. 在应用目录构建、打包和烧写

以下命令都从应用根目录执行。Windows 使用相同子命令的 `build.bat`。

#### 命令速查（全量）

| 命令 | 作用 | 示例 |
| --- | --- | --- |
| `setup` | 从锁文件安装 SDK | `./build.sh setup` |
| `menuconfig` | 打开图形配置菜单 | `./build.sh menuconfig` |
| `loadconfig <file>` | 加载配置文件 | `./build.sh loadconfig defconfig` |
| `saveconfig [name]` | 保存当前配置 | `./build.sh saveconfig` |
| `build [type] [-j N]` | 编译项目 | `./build.sh build Debug -j 8` |
| `vscode [type]` | 配置 CMake 并刷新 VS Code 文件 | `./build.sh vscode Debug` |
| `clean` | 清理构建产物（保留配置） | `./build.sh clean` |
| `distclean` | 删除 `build/` 和自动生成的配置，保留 `image/` | `./build.sh distclean` |
| `make_img [target]` | 打包固件到应用根目录的 `image/` | `./build.sh make_img` |
| `flash [target]` | 烧写固件 | `./build.sh flash` |
| `rttlog` | 打开 RTT 日志 | `./build.sh rttlog` |
| `package_update [name] [-l]` | 更新包或列出可更新包 | `./build.sh package_update -l` |
| `package_mirror [-l/-e/-d/-u]` | 管理包镜像源 | `./build.sh package_mirror -l` |
| `add_path_env <path>` | 追加工具路径到 FLY 环境 | `./build.sh add_path_env /path/to/tool/bin` |

> Windows 下将 `./build.sh` 替换为 `build.bat` 即可，例如：`build.bat build Release -j 8`。

#### 构建前准备，只需要执行一次（执行后环境变量会存储到 .PATH.env.json中）

- 添加编译器路径到FLY,添加的路径请根据你安装的路径进行添加，若路径有空格，请用引号包裹路径。(注意，路径应该要加上bin目录)

    ```bash
    ./build.sh add_path_env /home/xxx/xxxx/toolchain/riscv-none-embed-gcc-10.1.0-1.1/bin/
    ```

- 若没有添加仿真器程序到全局路径，请将他们也使用add_path_env添加到FLY环境中。

    ```bash
    ./build.sh add_path_env   /xxxx/xxx/openocd/
    ```

#### 配置应用

生成工程时确定板卡和模板。进入应用目录后，用 `./build.sh menuconfig` 调整配置，再用 `./build.sh saveconfig` 写回应用的 `defconfig`。

#### 编译项目

- 输入./build.sh进行编译

    ```bash
    # 一般直接运行/build.sh 会使用上一次选择的编译类型, 默认使用Release
    ./build.sh
    # 编译为Release
    ./build.sh build Release
    # 编译为Debug
    ./build.sh build Debug
    # 编译为MinSizeRel
    ./build.sh build  MinSizeRel
    # -j可以指定作业数量
    ./build.sh build  MinSizeRel -j 44
    ./build.sh -j 44
    ```

#### 打包

- 输入./build.sh make_img

    ```bash
    ./build.sh make_img
    ```

打包后的 ZIP 存在应用根目录的 `image/` 下，当前固件包通过 `image/CURRENT/` 访问。删除 `build/` 或执行 `./build.sh distclean` 后，这些固件包仍可访问。

输入未变且上一次打包生成的文件都在、大小一致时，打包命令会复用该包。缺少固件、烧录脚本或随包文件，或文件被截短时，会重新生成完整包。

如果旧版 SDK 的固件包还在 `build/image/`，请在清理前将要保留的 ZIP 复制到应用根目录的 `image/`。已有应用可在自己的 `.gitignore` 中加入 `/image/`。用新版 SDK 重新打包会更新 `image/CURRENT/`。

#### 烧写

- 输入./build.sh flash

    ```bash
    ./build.sh flash
    ```

#### 配置保存

如果进行复制的menuconfig配置后，如何保存配置呢

- 输入./build.sh saveconfig, 可自动保存配置

    ```bash
    # 自动保存到项目文件夹下的defconfig文件里面
    ./build.sh saveconfig
    # 自定义文件名称
    ./build.sh saveconfig xxconfig
    ```

#### 配置文件加载

在应用目录运行 `./build.sh loadconfig defconfig`。也可以传入其他配置文件的绝对路径或相对路径。

### 8. 从模板创建独立工程

从 FLY 根目录运行 `./fly.sh new`，按提示选择板卡和模板。也可以直接传参，Windows 下使用 `fly.bat new`。

```sh
./fly.sh new sensor-node --board frdm-mcxn947-core0 --template baremetal-nanosig
cd ../fly-project/sensor-node
./build.sh build Debug
./build.sh make_img
```

| 模板 | FRDM-MCXN947 Core 0 的启动验收信号 |
|---|---|
| `baremetal` | 板级初始化完成后 RTT 输出 `boot ok` |
| `baremetal-nanosig` | 裸机定时器回调输出 `nanosig ready`，回收后输出 `nanosig stopped` |
| `freertos-nanosig` | 应用任务输出 `nanosig ready`，回收后输出 `nanosig stopped` |

工程默认生成到 FLY 同级的 `fly-project/`，自带独立 Git 仓库。用 `--output <目录>` 可指定工程位置。第一次构建加载工程自己的 `defconfig`，后续可用 `menuconfig` 和 `saveconfig` 更新配置。

分享工程时，把 `fly.lock` 随应用一起提交。它记录 FLY 的 Git 地址和完整提交哈希。将该提交发布到锁定地址，并在没有本地 SDK 覆盖的干净克隆中验证安装、构建以及该应用提供的打包目标。

```sh
./build.sh setup
./build.sh build Debug
```

Windows 使用 `build.bat setup` 和 `build.bat build Debug`。首次安装需要 Git 和 Python 3。SDK 安装在被 Git 忽略的 `fly/` 下。远端分支继续更新时，应用仍使用锁定的提交。

`build.sh`、`build.bat` 和 CMake 共用应用自己的 SDK 解析器，按 `FLY_ROOT`、应用根目录的 `fly/`、`.fly-root` 的顺序选择 SDK。直接配置 CMake 时，显式设置的 `FLY_SDK_DIR` 优先，实际选中的路径单独记录。有提交锁定时，版本不匹配会停止当前操作并提示如何处理。

- `fly/` 由 `setup` 管理时，更新锁定提交后重新执行 `./build.sh setup` 或 `build.bat setup`，然后重新构建。如果更改了锁定的 Git 地址，先移开原有的 `fly/`，再执行 `setup`。
- 本地开发可以通过 `FLY_ROOT`、应用根目录下的 `fly/` 链接或 SDK 目录、`.fly-root` 路径文件来选择 SDK。直接运行 CMake 时也可传入 `-DFLY_SDK_DIR=/path/to/fly`。版本不匹配时，选择 `HEAD` 与锁文件一致的工作树，或者清除本地路径设置、移开手动放置的 `fly/` 后再运行 `setup`。用过 `-DFLY_SDK_DIR` 时，也要更新或清除 CMake 缓存里的该项。版本检查只读取当前选择。

手动选择的 SDK 可以包含未提交改动，`fly.lock` 只锁定它的 `HEAD`。没有 `fly.lock` 或 `commit` 为 `null` 的工程可使用本地 SDK 开发。通过 `setup` 安装 SDK 需要在锁文件中提供 Git 地址和完整提交哈希。

从旧版迁移时，如果构建缓存或 VS Code 配置里的 `FLY_SDK_DIR` 是旧工具自动写入的，清除该项后再配置。应用可继续通过 `-DFLY_SDK_DIR=/path/to/fly` 显式固定 CMake 使用的路径。

应用使用 `#include "board.h"` 调用板级接口。随板卡生成的头文件位于 `board/include/`，CMake 从 `board/board.cmake` 加载板级构建设置。配置和依赖分别位于应用的 `build/config/` 和 `build/deps/`，打包产物放在应用根目录的 `image/`。维护指南见[模板目录说明](templates/README.md)。

应用根目录的 `CMakeLists.txt` 是构建入口，负责设置固件名称和版本，并定义固件、调试和打包目标。`app/CMakeLists.txt` 向固件目标登记应用源码、头文件目录和可选的应用配置。SDK 在 `project()` 启用编译语言前选择工具链。使用应用构建脚本时，嵌入式 ELF 和 BIN 分别生成到 `build/cmake/fly_app` 和 `build/cmake/fly_app.bin`。

嵌入式应用从 `app/main.c` 启动，业务和中间件源码分别放在 `app/biz/` 和 `app/mid/`，对应的公开头文件放在 `app/include/biz/` 和 `app/include/mid/`。

安装 SDK 和目标工具链后可以在应用目录直接配置和构建。首次配置会自动生成 Kconfig 输出。

```sh
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build/cmake
```

Windows 下直接使用 MinGW-w64 和 Ninja 时，首次配置加上 `-G Ninja`。如果使用 `build.bat`，脚本会选择已安装的生成器。

CI 每次从模板生成工程，覆盖七个 Cortex-M 架构和 Linux、macOS、Windows 三种宿主平台，同时验证 FRDM-MCXN947 的三种模板打包。两个 RISC-V 目标暂不纳入 CI。见 [CI 流程](docs/CI.md)。

### 9. 在独立应用中使用 SDK 源码包

SDK 从 `package/` 加载包的 CMake 和 Kconfig 设置。下面用一个求和函数演示如何把包注册到 SDK，再从生成的 FRDM-MCXN947 裸机应用调用它。

在 SDK 内创建以下文件。

~~~text
package/test-add/
├── Kconfig
├── CMakeLists.txt
├── add.c
└── inc/
    └── add.h
~~~

在 `Kconfig` 中定义开关。

~~~kconfig
config PACKAGE_TEST_ADD
    bool "Test add"
~~~

在包的 `CMakeLists.txt` 中注册对象库。

~~~cmake
if(CONFIG_PACKAGE_TEST_ADD)
    add_library(test-add OBJECT add.c)
    target_include_directories(test-add PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inc")
    package_lib_add(test-add)
endif()
~~~

在 `inc/add.h` 声明 `int add_int(int a, int b);`，然后在 `add.c` 引入该头文件并实现函数。

~~~c
#include "add.h"

int add_int(int a, int b)
{
    return a + b;
}
~~~

应用根目录的 `CMakeLists.txt` 通过 `target_link_package_lib(fly_app)` 链接启用的包。在 `app/main.c` 中引入 `add.h`，并在板卡初始化后调用求和函数。

~~~c
board_log(add_int(3, 4) == 7 ? "add ok\n" : "add failed\n");
~~~

在应用的 `defconfig` 中加上 `CONFIG_PACKAGE_TEST_ADD=y`，然后从应用根目录重新加载配置并构建。

~~~sh
./build.sh loadconfig defconfig
./build.sh build Debug
~~~

将应用分发给其他人时，把 `fly.lock` 指向包含该包的 SDK 提交。

### 10. 配合VS Code插件使用-插件安装

- 安装插件调试插件[Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug)、插件栏搜索安装或者使用命令安装 

    ```bash
    code --install-extension marus25.cortex-debug
    ```

- 安装[CMake Tools](https://github.com/microsoft/vscode-cmake-tools)、插件栏搜索安装或者使用命令安装

    ```bash
    code --install-extension ms-vscode.cmake-tools
    ```

- 安装[clangd](https://github.com/clangd/vscode-clangd)、插件栏搜索安装或者使用命令安装

    ```bash
    code --install-extension llvm-vs-code-extensions.vscode-clangd
    ```

### 11. 在 VS Code 构建、烧录和打包

打开应用根目录。在命令面板运行 `Tasks: Run Task`，克隆后的应用可先用 **FLY: Setup** 安装锁定的 SDK。**FLY: Configure** 准备本机工具链和 VS Code 设置。也可以在任务菜单中选择 **FLY: Build**、**FLY: Make image** 或 **FLY: Flash**。

首次构建也会更新 `.vscode/settings.json` 和本机的 `.vscode/cmake-kits.json`。还可以执行 `./build.sh vscode Debug`（Windows 使用 `build.bat vscode Debug`）生成这些文件和编译数据库。CMake Tools 和命令行都从应用根目录配置，并使用应用的 `build/cmake/`。SDK 位置和交叉工具链路径由本机配置提供。

在命令面板执行 `CMake: Select a Kit`，选 **FLY**，然后执行 `CMake: Configure`。用 **CMake: Build** 编译，或用 **CMake: Build Target** 选择 `default_make_img` 打包和 `default_flash` 烧录。

如果已有带注释的 VS Code 配置，FLY 会保留原文件并生成 `.vscode/fly-*.json` 合并片段。

在应用根目录的 `CMakeLists.txt` 中载入打包模块并声明镜像。应用模块读取板卡配置中的 `FLY_JLINK_DEVICE`，并把器件名传给 SDK 的 `add_jlink_image()`。宿主工程和未配置 J-Link 的板卡会跳过 J-Link 打包目标。应用镜像使用板卡给出的 Flash 地址和分区容量。

~~~cmake
include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-image.cmake")
app_add_jlink_image(default
    IMAGE_NAME "${FLY_FIRMWARE_NAME}"
    FIRMWARE_LIST
        "APP:${FLY_FIRMWARE_VERSION}:${FLY_FLASH_ADDRESS}:${FLY_FLASH_SIZE}:fly_app.bin"
        # "BOOT:${BOOT_VERSION}:${BOOT_FLASH_ADDRESS}:${BOOT_FLASH_SIZE}:${BOOT_BIN}"
    DEPENDS fly_app
)
~~~

`FIRMWARE_LIST` 的每一项采用 `NAME:VERSION:ADDRESS:CAPACITY:BIN`，容量是十进制或带 `0x` 前缀的十六进制字节数。打包工具校验加上对齐填充和元数据尾部后的最终 BIN。超过对应分区容量时，打包失败。应用侧的 `CONFIG_FACTORY_DATA_SUPPORTED` 控制板上读取和日志，打包始终检查容量。已有工程需要在固件条目的 BIN 路径前填写分区容量。

要在同一个包中加入 bootloader，填写示例中的 `BOOT` 条目和对应的版本、烧录地址、分区容量和 BIN 路径。相对 BIN 路径以 `build/cmake/` 为基准，已有 BIN 可以用绝对路径。若 BIN 由当前 CMake 工程生成，将对应的构建目标加入 `DEPENDS`。各镜像的烧录地址和分区容量需与 Flash 布局一致，并确保分区互不重叠。若为 bootloader 调整应用的起始地址，同时更新应用链接脚本和板卡配置中的 `FLY_FLASH_ADDRESS`、`FLY_FLASH_SIZE`。

OpenOCD 使用 `add_openocd_image()`，在应用根目录的 `CMakeLists.txt` 中指定配置文件及同样格式的 `FIRMWARE_LIST`。OpenOCD 的 RTT 日志还需要传入板卡的 `RTT_ADDR_AND_SIZE`，即搜索的起始地址和字节数。每个打包目标都会生成相应的烧录目标。J-Link 的当前包位于 `image/CURRENT/`，其中包含清单中的 BIN、`download.jlink` 和烧录脚本。启用 RTT 的工程也可在应用目录用 `./build.sh rttlog` 查看日志。

### 12. 在 VS Code 调试

安装 Cortex-Debug、板卡对应的调试工具和交叉 GDB。应用根目录的 `CMakeLists.txt` 载入应用自己的调试模块，并传入固件目标。

~~~cmake
include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-vscode.cmake")
app_add_vscode_debug(fly_app)
~~~

在应用的 `tool/cmake/app-vscode.cmake` 中可以调整 RTT 等调试参数。`app_add_vscode_debug()` 检查板卡的 J-Link 器件及 GDB 和 objdump，再调用 SDK 的 `add_vscode_cortex_debug_gdb()` 生成 `.vscode/launch.json`。宿主工程、未配置 J-Link 的板卡以及缺少 GDB 或 objdump 的环境会跳过调试配置生成。嵌入式构建也会用 objdump 生成反汇编列表。打开应用根目录，在「运行和调试」里选择生成的配置。

应用模块检测到 `default_flash` 时，也会为 SDK 配置 **FLY: Flash** 前置任务，生成 **default-fly_app (flash BIN and attach)**。要调试带 CRC 尾部的发布固件，首次构建后选中该配置并按 F5。**FLY: Flash** 会烧录包中的各个镜像，再从同一次构建的应用 ELF 读取符号并附加。烧录结束时目标会复位并运行，因此附加时程序可能已执行过启动代码。

三个嵌入式模板默认启用 `CONFIG_FACTORY_DATA_SUPPORTED`。烧录打包后的 BIN 后，启动时会校验 Flash 中的尾部，并在 RTT 通道 0 打印固件名、版本、分区和烧录地址、对齐后的镜像长度、尾部长度、CRC32，以及打包时记录的 Git 版本和时间。找不到有效尾部时（例如普通 ELF 调试只下载了程序），会打印一次 `Firmware info unavailable`，随后继续启动。在 `menuconfig` 中关闭该选项即可跳过这项检查和日志。

FRDM-MCXN947 裸机模板随后输出 `boot ok`。两个 nanosig 模板在定时器回调后退出循环，并在完成资源回收后输出 `nanosig stopped`。可在 [nanosig 应用运行时](templates/README.md#nanosig-应用运行时) 查看接口和示例。

### 13. clangd 代码补全

安装 clangd 扩展后打开应用根目录，运行一次 `./build.sh vscode Debug` 或构建应用。工作区设置会让 clangd 读取 `build/cmake/compile_commands.json`，并通过 `--query-driver` 向当前交叉编译器获取系统头文件路径。切换工具链或 SDK 后重新运行该命令。

如果 clangd 在首次生成设置前已启动，可以执行 `clangd: Restart language server`。
