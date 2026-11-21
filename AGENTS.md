# 仓库协作规范

## 项目概览

FLY 是一套嵌入式固件构建、打包、烧录和调试工具。固件使用 C，工具使用 Python 3。构建由 **CMake + Kconfig** 驱动，通过 `kconfiglib` 处理类似 Linux 内核的配置。

- 在 SDK 目录运行 `./fly.sh new` 或 `fly.bat new` 创建应用。应用的 `build.sh`（Linux/macOS）或 `build.bat`（Windows）调用 SDK 的 `tool/python/build.py`，再由它调用 CMake。
- 应用的 `Kconfig` 生成 `build/config/.config`，再由该配置生成 `build/config/.config.cmake` 和 `build/config/autoconf.h`。
- 应用根目录的 `CMakeLists.txt` 是构建入口。`cmake/fly-configure.cmake` 在 `project()` 前准备 Kconfig 和工具链，随后 `cmake/fly-sdk.cmake` 加载 SDK 的架构和组件包。
- GitHub Actions 为七个 Cortex-M 目标和三个宿主平台生成工程。夜间任务以 Release 模式运行 ARM 构建矩阵。必需检查见 [持续集成说明](docs/CI.md)。
- 仓库根目录的 `CLAUDE.md` 是本地 OMC 编排配置，由 Git 忽略。

## 目录结构

- `arch/<vendor>/<family>/` 提供芯片支持，包括厂商 SDK、HAL、启动代码和芯片级的默认链接脚本。各架构通过 `gcc-config.cmake` 配置 CPU 选项和工具链，并提供 `Kconfig`。覆盖 `arm64`、`ch58x`、`gd32`、`mcxn947`、`stm32` 和 `x64`（Linux 和 Windows 宿主）。
- `templates/` 包含工程骨架、应用模板和板卡支持。生成的应用是独立仓库，默认位于 SDK 同级的 `fly-project/` 下。
- `package/<name>/` 存放可复用组件。`eventhub-os`、`ehip` 等 Git 依赖通过 `fetch_git_project()` 克隆到应用的 `build/deps/<name>-src/`。
- `cmake/` 存放工具链文件、组件包辅助函数（`package_lib_add`、`target_link_package_lib`）、打包模块（`jlink-make-image.cmake`、`openocd-make-image.cmake`）及 VS Code 调试配置的生成逻辑。
- `tool/python/` 中的 `build.py` 调度构建，`mk_jlink_img.py` 和 `mk_openocd_img.py` 打包固件，`fetch_git_project.py` 按镜像设置拉取 Git 仓库，`mk_mcu_firmware_factory_data.py` 生成固件尾部的出厂数据。
- `tool/test/` 下的 `check_standalone_projects.py` 验证工程配置、ARM 和宿主构建、固件打包及 SDK 安装。
- `.github/workflows/` 下的 `ci.yml` 负责 PR 检查，`nightly.yml` 负责夜间 Release 构建和打包冒烟测试。PR 描述模板位于 `.github/pull_request_template.md`。
- `resource/git.mirror-source.json` 用于初始化本地 `.git.mirror-source.json`。本地文件由 Git 忽略，可配置镜像以加快在中国大陆拉取依赖。
- `tool/jlink/**` 根据 `.gitattributes` 由 Git LFS 管理。

## 职责归属与依赖方向

- SDK 提供芯片支持、可复用组件以及构建、调试和打包工具。板级初始化、探针和 RTT 设置、日志适配和默认 Flash/RAM 布局归板卡模板，生成时复制到应用的 `board/`。复用同一份实现的板卡在各自清单中声明共享目录。
- 通用 Python 和 CMake 代码通过清单字段、目标属性和显式参数读取差异。模板显示名、组件选择、必需的板卡数据和启动文案由清单或模板配置声明。板卡专用地址和组件专用的编译选项例外由对应板卡或组件维护。使用现有能力的新板卡或模板只需提供自身的数据和源码。
- 应用根目录声明固件目标和打包分区清单。`app/CMakeLists.txt` 登记应用源码和头文件搜索路径。板级 CMake 选择芯片目标及其链接依赖。应用的 `tool/cmake/` 封装根据板卡数据和工具是否可用，决定是否添加打包、烧录和调试目标。 [模板说明](templates/README.md)
- 命令行和直接运行 CMake 时共用 SDK 选择和版本校验逻辑。用户显式指定的 SDK 路径与解析后的实际路径分开处理。分享应用时，把 SDK 锁定到配置的远端上可获取的提交，并在未设置本地 SDK 覆盖的干净克隆中验证安装、构建及适用的打包流程。

## 构建脚本子命令

在应用根目录通过 `./build.sh` 或 `build.bat` 执行以下命令。省略子命令时，按上次使用的构建类型编译，首次默认使用 `Release`。

| 命令 | 用途 | 示例 |
|---|---|---|
| `setup` | 安装 `fly.lock` 锁定的 SDK 版本 | `./build.sh setup` |
| `menuconfig` | 打开应用和组件包的 Kconfig 终端配置菜单 | `./build.sh menuconfig` |
| `loadconfig <file>` | 加载配置并重新生成 `build/config/` | `./build.sh loadconfig defconfig` |
| `saveconfig [name]` | 将当前配置保存为应用的最小 defconfig | `./build.sh saveconfig` |
| `build [type] [-j N]` | 配置并编译。`-j` 也可直接传给构建脚本，例如 `./build.sh -j 44` | `./build.sh build Debug -j 8` |
| `vscode [type]` | 配置 CMake 并生成本地的 VS Code 和 clangd 设置 | `./build.sh vscode Debug` |
| `clean` | 执行 `cmake --build build/cmake --target clean`，保留已生成的配置 | `./build.sh clean` |
| `distclean` | 删除应用的 `build/`，保留 `image/` 下的固件包 | `./build.sh distclean` |
| `make_img [target]` | 生成固件包，需要先有 `build/` | `./build.sh make_img` |
| `flash [target]` | 通过 J-Link 或 OpenOCD 烧录固件 | `./build.sh flash` |
| `rttlog` | 打包后，通过 `image/CURRENT/` 下的脚本实时读取 SEGGER RTT 日志 | `./build.sh rttlog` |
| `package_update [name] [-l]` | 执行 `build/cmake/` 中的组件更新目标。`-l` 列出组件包 | `./build.sh package_update -l` |
| `package_mirror -l/-d/-e/-u` | 列出、禁用、启用镜像或重新测速 | `./build.sh package_mirror -l` |
| `add_path_env <path>` | 将工具链或调试器的 `bin/` 目录加到 FLY 的 PATH 最前面，记录到由 Git 忽略的 `.PATH.env.json` | `./build.sh add_path_env /opt/gcc-arm/bin` |

## 固件打包与烧录目标

应用根目录的 `CMakeLists.txt` 声明固件、调试和打包目标。`app/CMakeLists.txt` 登记应用源码和头文件搜索路径。通过 `add_jlink_image(<name> ...)` 或 `add_openocd_image(<name> ...)` 声明的打包目标会生成 CMake 目标 `<name>_flash` 和 `<name>_make_img`。默认名称为 `default`。使用其他名称时，将它传给 `flash` 或 `make_img`。

## 组件包

- 各 `package/<name>/CMakeLists.txt` 通过 `package_lib_add(<target>)` 注册自己的库目标。该函数定义在 `cmake/package.cmake`。
- 用 `if(CONFIG_PACKAGE_<NAME>)` 控制是否注册组件。在 `menuconfig` 中关闭组件后，它就不会参与构建。
- 工程的可执行目标调用 `target_link_package_lib(<exec>)`，链接所有已启用的组件。需要时可为各组件单独设置 `target_compile_definitions` 和 `target_include_directories`。

## 构建环境与工具链

- `build.sh` 按 `$FLY_PYTHON`、`.venv/bin/python`、`python3`、`python` 的顺序选择 Python。`build.bat` 对应的顺序为 `%FLY_PYTHON%`、`.venv\Scripts\python.exe`、`python3`、`python`。
- 缺少 `kconfiglib` 或 `requests` 时，Python 构建入口会在应用的 `build/.venv` 中安装这两个依赖。设置 `FLY_AUTO_SETUP_PY=0` 可以关闭自动安装。
- Windows 上使用 `menuconfig` 还需在虚拟环境中安装 `windows-curses`。
- 构建脚本自动查找 CMake 的构建工具，优先使用 `ninja`，其次使用 `make`（Windows 下也从 PATH 查找）。两者都找不到时会报错。
- 构建脚本从 `PATH` 查找工具链及调试器。可在应用目录运行 `./build.sh add_path_env <dir-to-bin>`，把工具所在目录加到后续构建的搜索路径最前面。设置记录在 SDK 中由 Git 忽略的 `.PATH.env.json`。
- 编译产物和生成的配置位于应用的 `build/`，固件包和压缩包位于 `image/`。VS Code 读取应用生成的 `.vscode/launch.json`。

## 编码约定

- C 的函数名和变量名使用 `snake_case`，宏使用 `UPPERCASE`。生成应用的固件目标名为 `fly_app`。
- 先链接板卡选定的架构目标及其依赖库，再调用 `target_link_package_lib(<exec>)`。芯片专用库及其链接顺序由板级配置管理。
- 工程骨架为可执行目标设置告警选项，并用 `package_lib_compile_options()` 将选项传给组件包。需要例外的组件自行声明 `FLY_EXCLUDED_PACKAGE_COMPILE_OPTIONS` 目标属性。

## 审阅与验证

- 遇到描述不明确的问题，先确认具体文件、符号和调用链。审查依赖时，先确定行为应由哪一层负责，并查清改动会影响哪些调用方，再调整实现。
- 可选功能的开关同时管住头文件引用、调用和链接依赖。验证生成工程的默认配置。支持增量切换时，在同一个构建目录关闭功能并重新构建。
- 分别报告工程生成、编译链接、固件包完整性和实板烧录或运行的验证结果。打包或布局有变动时，核对实际固件地址和打包后的字节，包括尾部 CRC 和填充。报告运行结果时，说明实板上的观察结果或所用的测试环境。

## 拉取请求与持续集成

沿用 nanosig 上游的分支保护原则。**所有交付都走拉取请求（PR）。必需检查全部通过后才能合并，主分支保持线性历史。**

1. 在特性分支提交改动并创建 PR。禁止直接向 `master` 推送交付改动，主分支上的推送检查只作事后兜底。
2. 必需检查（`kconfig`、`arm-build`、`host-linux`、`host-macos`、`host-windows`）全部通过后才合并。分支保护启用 `strict` 且特性分支落后时，先同步 `master` 再重跑。这五项曾因上游 `eventhub` 的 Apple Clang C11 原子操作问题及 `ehshell` 在 MinGW 缺少 `bzero`、`isprint` 而暂缓，现已通过 `96916a7`、`a1c1bcf` 修复并全部设为必需。
3. 统一用变基方式合并（`gh pr merge --rebase`）。合并后删除特性分支，切回本地 `master` 并用 `git pull --ff-only` 同步远端。如果切换会覆盖未提交的改动，或本地 `master` 已分叉，保留现场并报告。禁止自动丢弃改动或重置分支。
4. 本地先跑 `python3 tool/test/check_standalone_projects.py --matrix config`。涉及 ARM 或宿主工程时，用同一脚本运行 `--matrix arm` 或 `--matrix host`。
5. 新增模板或板卡时，在板卡清单中声明已验证的模板组合，供生成器和持续集成使用。
6. 更改检查任务名称时，同时更新分支保护中的 `contexts`。 [分支保护和合并配置](docs/CI.md)

## 提交与 PR 说明

- 提交说明采用表情符号和约定式提交格式，例如 `✨ feat(scope): ...`、`🐞 fix(scope): ...`、`🧪 test: ...`、`📃 docs: ...`、`🔧 chore/fix: ...`、`🧹 chore: ...`、`🎈 perf: ...` 和 `🔨 refactor: ...`。
- 一个提交只包含一项独立改动，并带上该改动所需的 Kconfig 和 defconfig。
- 用户要求先审阅时，将改动留在工作区，并给出受影响的文件和验证结果。获准执行的每一步都要验证到位，合并时包括必需的持续集成检查及最终分支状态。
- PR 描述应列出目标板卡或工程、所用的 `defconfig`、实际运行的构建和测试命令，以及运行证据。涉及命令行交互或网络功能时，附上 RTT 日志截图。 [PR 描述模板](.github/pull_request_template.md)
- 回归构建使用在 SDK 仓库之外新生成的应用。 [持续集成说明](docs/CI.md)

## 禁止提交的文件

以下文件和目录由 `.gitignore` 忽略，可在本地使用。

- `.config`、`.config.old`、`auto-generate/`
- `.PATH.env.json`、`.git.mirror-source.json`、`.cache/`
- `build/`、`image/`、`dl/`
- `tool/python/__pycache__/`、`.venv/`
- `.vscode/`、`.clangd`、`.codex`、`.claude/`、`.omc/`、`CLAUDE.md`

## 文档

`README.md` 是用户教程，`AGENTS.md` 记录仓库协作规范。`docs/` 面向贡献者和审阅者，包含移植设计、迁移方案和参考资料。 [贡献者文档索引](docs/README.md)
