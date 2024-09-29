# FLY

## 项目介绍

本项目为单片机提供一站式开发平台，包括：编译、烧写、调试、发布打包等功能，可以在多个平台下进行编译，如：linux、windows(macos待验证)。

本项目的前身本来是为[eventhub-os](https://github.com/xiaoapeng/eventhub_os)做的一些验证demo，因为不想重复的维护多个仓库，想从重复的工作解脱出来(复制、粘贴、编写launch.json)，慢慢的发展成了现在这个样子。

本项目由cmake和一系列python脚本组成，使用Kconfig系统管理配置，可以通过./build.sh menuconfig(./build.bat menuconfig)来使能和失能包，且本项目支持在多个平台下进行交叉编译，如：linux、windows(macos待验证)。

站在巨人的肩膀上，配合vscode使用，可以擦出哪些火花呢？本项目可以搭配vscode插件[Cortex-Debug](https://github.com/Marus/cortex-debug.git)、[CMake Tools](https://github.com/microsoft/vscode-cmake-tools)、[clangd](https://github.com/clangd/vscode-clangd),实现一键编译、一键烧写、一键调试、一键发布打包、完美跳转等功能。

## 使用教程

### 1. clone项目到本地

```bash
git clone https://github.com/xiaoapeng/fly.git
cd fly
```

### 2. 安装必要python3环境和部分库

#### windows 安装python

- [下载](https://www.python.org/downloads/)python,安装时(或安装后)请将python加入全局环境变量，并重启设备。
- 安装kconfiglib

    ```bash
    pip install kconfiglib
    ```

#### ubuntu 安装python

- 运行 sudo apt install python3

    ```bash
    sudo apt install python3
    ```

- 安装kconfiglib

    ```bash
    pip3 install kconfiglib
    ```

### 3.安装cmake

#### windows 安装cmake

- [下载](https://cmake.org/download/)cmake,安装时请将cmake加入全局环境变量，并重启设备。

#### linux 安装cmake

- 使用命令 sudo apt install cmake

    ```bash
    sudo apt install cmake
    ```

### 4. 根据项目需要下载交叉编译器

下载后请自行解压到你自定义的安装目录。

|平台|交叉编译器前缀|下载链接|
| --- | --- | --- |
|stm32fx<br>gd32fx<br>mcxn947|arm-none-eabi|[下载](https://developer.arm.com/downloads/-/gnu-rm)|
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

### 7. 选择芯片，开始构建、打包、烧写

<p><span style="color: yellow;">构建时 windows使用build.bat脚本，linux使用build.sh脚本</span></p>

#### 构建前准备，只需要执行一次（执行后环境变量会存储到 .PATH.evn.json中）

- 添加编译器路径到FLY,添加的路径请根据你安装的路径进行添加，若路径有空格，请用引号包裹路径。(注意，路径应该要加上bin目录)

    ```bash
    ./build.sh add_path_env /home/xxx/xxxx/toolchain/riscv-none-embed-gcc-10.1.0-1.1/bin/
    ```

- 若没有添加仿真器程序到全局路径，请将他们也使用add_path_env添加到FLY环境中。

    ```bash
    ./build.sh add_path_env   /xxxx/xxx/openocd/
    ```

#### 选择项目

- 打开menuconfig

    ```bash
    ./build.sh menuconfig
    ```

- 分别选择Arch

    ```bash
    Select CPU Architecture (gd32vf103x)  --->
    ```

- 选择项目

    ```bash
    - Select a gd32vf103x platform project (GD32VF103C demo project)  --->
    ```

- 按ESC后选择Y保存退出

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

- 查看打包
![alt text](resource/image1.png)

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

- 输入./build.sh loadconfig ./project/xxxx/xxxx/defconfig

    ```bash
    # 以./project/gd32f10x/gd32f103-demo/defconfig 举例， 必须输入绝对或者相对目录
    ./build.sh loadconfig ./project/gd32f10x/gd32f103-demo/defconfig
    ```


### 8. 配合VS Code插件使用-插件安装

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

### 9. 配合VS Code插件使用-编译、烧写、打包

- 如果你已经使用./build.sh 完成了项目的初次构建，那么应该可以使用CAMKE Tools插件直接进行编译。
- 可以通过cmake插件旁边的项目大纲来一键烧写或一键打包固件
![alt text](resource/image2.png)
- 请不要使用cmake tools插件进行配置，可能会找不到编译器(除非编译器在全局环境变量)，如果发生配置失败的情况，请删除./build目录，再使用./build.sh进行重新构建。
![alt text](resource/image3.png)
- 打包固件使用命名规则 <IMAGE_NAME>_<CMAKE_BUILD_TYPE>_<BURN_MODE>_<PARTITION_1_NAME>v<PARTITION_1_VERSION>..._[<PARTITION_x_NAME>v<PARTITION_x_VERSION>]_<MAKE_IMG_TIME>_[dirty]_<GIT_COMMIT_HASH>, 该命名在CMAKE文件中指定比如：

    ```cmake
    # JLINK固件
    STM32F030X8_DEMO_APP__jlink_APPv0.0.1_20240928_150140_dirty_9bf0eb71fd
    在cmake中调用这个函数生成的
    add_jlink_image( default
        IMAGE_NAME "STM32F030X8_DEMO_APP"
        CHIP_NAME "STM32F030C8"
        FIRMWARE_LIST "APP:V0.0.1:0x8000000:stm32f030x8_demo_app.bin"
        DEPENDS stm32f030x8_demo_app openocd_image_make_img
    )

    
    # OPENOCD固件
    STM32F030X8_DEMO_APP__openocd_APPv0.0.1_20240928_150140_dirty_9bf0eb71fd
    add_vscode_cortex_debug_gdb( jlink
        CHIP_NAME "STM32F030C8"
        ELF_NAME_LIST "stm32f030x8_demo_app"
        SERVER_TYPE "jlink"
        RTT_DEBUG  ON
        DEPENDS stm32f030x8_demo_app
    )

    ```

- 打包img包各文件解释(jlink包为例)

    ```txt
    ├── APP.bin                                             # 分区固件
    ├── burn.bat                                            # 在windows下的烧写脚本
    ├── burn.sh                                             # 在linux下的烧写脚本
    ├── download.jlink
    ├── log.bat                                             # 在windows下日志抓取脚本
    ├── log.ps1
    ├── log.sh                                              # 在linux下日志烧写脚本
    └── tool
        ├── JLinkARM.dll
        ├── JLink.exe
        ├── 
    ```

### 10. 配合VS Code插件使用-调试

- 如果你已经使用./build.sh 完成了项目的初次构建，那么应该可以使用Cortex-Debug插件直接进行调试。
![alt text](resource/image4.png)
- 调试文件launch.json是由cmake函数调用python脚本生成的,如果想要编译时自动生成launch.json，需要在项目的CMakeLists.txt调用add_vscode_cortex_debug_gdb

    这里以project/gd32vf103x/gd32vf103c-demo/CMakeLists.txt为例：

    ```cmake
    ...
    
    add_vscode_cortex_debug_gdb( jlink
        CHIP_NAME "GD32VF103CBT6"
        ELF_NAME_LIST "gd32vf103c-demo"
        SERVER_TYPE "jlink"
        INTERFACE "jtag"
        OTHER_EXT_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/debugconfig/ext-cortex-debug-config.json"
        RTT_DEBUG  ON
        DEPENDS gd32vf103c-demo
    )
    ```

- 调试已经支持了RTT日志的查看，你要使能RTT相关包，然后配置eventhub-os包提供的格式化输出，就能看到相关日志的输出。

    ```c
    #include "SEGGER_RTT.h"
    #include "eh.h"
    #include "eh_debug.h"

    void stdout_write(void *stream, const uint8_t *buf, size_t size){
        (void)stream;
        SEGGER_RTT_Write(0, buf, (unsigned)size);
    }
    int main(void){
        eh_infoln("hello fly!");
        return 0;
    }
    ```

### 11. 配合VS Code插件使用-LSP语法提示、跳转、代码补全

- 想用 c/c++ 插件请自行配置，略过此章。
- 如果你已经使用./build.sh 完成了项目的初次构建，应该可以使用clangd进行代码补全。
- 先在.vscode下建立.vscode/settings.json,添加如下内容。

    ```json
    "C_Cpp.intelliSenseEngine": "disabled",
    "clangd.onConfigChanged": "restart",
    "clangd.checkUpdates": false,
    "clangd.arguments": [
        // 在后台自动分析文件（基于complie_commands）
        "--background-index",
        // 同时开启的任务数量
        "-j=6",
        // 标记compelie_commands.json文件的目录位置
        "--compile-commands-dir=${workspaceFolder}/build",
        // 使用的编译器，通过编译器来寻找系统头文件位置,这里必须使用绝对路径，不能使用链接路径
        //"--query-driver=",
        // 补充头文件的形式
        /* iwyu:自动补充  never：不自动补充*/
        "--header-insertion=iwyu",
        //"--header-insertion=never",
        // pch优化的位置
        "--pch-storage=disk",
        // 全局补全（会自动补充头文件）
        "--all-scopes-completion",
        // clang-tidy功能
        "--clang-tidy",
        "--clang-tidy-checks=performance-*,bugprone-*",
        // 建议风格：打包(重载函数只会给出一个建议）
        // 相反可以设置为bundled 
        "--completion-style=detailed",
        // 跨文件重命名变量
        "--cross-file-rename",
        // 使能.cland文件
        // .clangd示例:
        //  CompileFlags:                     # Tweak the parse settings
        //      Remove: [-march=rv32imac, -mabi=ilp32] 
        //
        //"--enable-config",
    ],
    "editor.inlayHints.enabled": "off",
    ```

- 然后使用vs code顶部命令（或者直接重启VS Code） >clangd:Restart language server 重启clangd服务，即可支持代码补全。
- 对于一些新架构，比如riscv，clangd会出现报错，在fly目录下建立.clangd文件(指定命令来移除clangd不能识别的选项)可以解决问题，比如下面的。

    ```.clangd
    # 这里就是移除了 -march=rv32imac, -mabi=ilp32，防止clangd报错
    CompileFlags:                     # Tweak the parse settings
    Remove: [-march=rv32imac, -mabi=ilp32]
    ```
