# FLY
## 项目介绍

本项目为单片机提供一站式开发平台，包括：编译、烧写、调试、发布打包等功能，可以在多个平台下进行编译，如：linux、windows(macos待验证)。

本项目的前身本来是为[eventhub-os](https://github.com/xiaoapeng/eventhub_os)做的一些验证demo，因为不想重复的维护多个仓库，且讨厌keil的律师函，想从重复的工作解脱出来(复制、粘贴、编写launch.json)，慢慢的发展成了现在这个样子。

本项目由cmake和一系列python脚本组成，使用Kconfig系统管理配置，可以通过./build.sh menuconfig(./build.bat menuconfig)来使能和失能包，且本项目支持在多个平台下进行交叉编译，如：linux、windows(macos待验证)。

站在巨人的肩膀上，配合vscode使用，可以擦出哪些火花呢？本项目可以搭配vscode插件[Cortex-Debug](https://github.com/Marus/cortex-debug.git)、[CMake Tools](https://github.com/microsoft/vscode-cmake-tools)、[clangd](https://github.com/clangd/vscode-clangd),实现一键编译、一键烧写、一键调试、一键发布打包、完美跳转等功能。

## 使用教程
### 1. clone项目到本地
```
git clone https://github.com/xiaoapeng/fly.git
cd fly
```

### 2. 安装必要python3环境和部分库
#### windows 安装python
- [下载](https://www.python.org/downloads/)python,安装时(或安装后)请将python加入全局环境变量，并重启设备。
- 安装kconfiglib
```
pip install kconfiglib
```

#### ubuntu 安装python
- 运行 sudo apt install python3
```
sudo apt install python3
```
- 安装kconfiglib
```
pip3 install kconfiglib
```

### 3.安装cmake
#### windows 安装cmake
- [下载](https://cmake.org/download/)cmake,安装时请将cmake加入全局环境变量，并重启设备。

#### linux 安装cmake
- 使用命令 sudo apt install cmake
```
sudo apt install cmake
```

### 4. 根据项目需要下载交叉编译器

|平台|交叉编译器前缀|下载链接|
| --- | --- | --- |
|stm32fx<br>gd32fx<br>mcxn947|arm-none-eabi|[下载](https://developer.arm.com/downloads/-/gnu-rm)|
|ch58x|riscv-none-elf|[下载](http://www.mounriver.com/download/)|
|gd32vfx|riscv-none-embed|[下载](https://github.com/xpack-dev-tools/riscv-none-embed-gcc-xpack/releases/tag/v10.2.0-1.2)|

### 5. 根据项目需要下载调试器
#### Jlink 安装
- [下载](https://www.segger.com/downloads/jlink/)
后进行安装，不同平台直接安装方式略有差别，请自行百度(谷歌)。

windows 安装成功后

![alt text](resource/image0.png)

linux安装成功后运行 JLinkExe 
```
user@:~/project/fly$ JLinkExe 
SEGGER J-Link Commander V7.98h (Compiled Sep 11 2024 14:26:16)
DLL version V7.98h, compiled Sep 11 2024 14:25:50

Connecting to J-Link via USB...FAILED: Cannot connect to J-Link.
J-Link>
```

#### openocd 安装
ubuntu  安装方式
使用如下命令
```
sudo apt install  openocd
```
安装成功后运行如下命令
```
simon@:~/project/fly$ openocd -v 
Open On-Chip Debugger 0.11.0
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
```

windows 安装方式

- [下载](https://github.com/xpack-dev-tools/openocd-xpack/releases)后解压到你自定义的安装目录。
- 添加进环境变量并重启设备

- 安装成功后打开cmd运行命令
```
PS C:\Users\user> openocd.exe -v
xPack Open On-Chip Debugger 0.12.0+dev-01685-gb9224c0c0-dirty (2024-08-02-19:51)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
PS C:\Users\user>
```
### 6.选择芯片，开始构建
构建过程中windows使用./build.bat，linux使用./build.sh
####  构建前准备，只需要执行一次（./build.sh distclean后需要重新执行）
- 添加编译器路径到FLY,添加的路径请根据你安装的路径进行添加，若路径有空格，请用引号包裹路径。(注意，路径应该要加上bin目录)
```
./build.sh add_path_env /home/xxx/xxxx/toolchain/riscv-none-embed-gcc-10.1.0-1.1/bin/
```
- 若没有添加仿真器程序到全局路径，请将他们也使用add_path_env添加到FLY环境中。
```
./build.sh add_path_env   /xxxx/xxx/openocd/
```

#### 选择项目
- 打开menuconfig
```
./build.sh menuconfig
```
- 分别选择Arch
```
Select CPU Architecture (gd32vf103x)  --->
```
- 选择项目
```
- Select a gd32vf103x platform project (GD32VF103C demo project)  --->
```
- ESC 选择Y保存退出

#### 编译项目
- 输入./build.sh进行编译
```
# 一般自动编译为上一次选择的编译类型或者Release
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
```
./build.sh make_img
```
- 查看打包
![alt text](resource/image1.png)

#### 烧写
- 输入./build.sh flash
```
./build.sh flash
```
