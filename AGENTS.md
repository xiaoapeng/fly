# Repository Guidelines

## Project at a Glance

FLY is a one-stop embedded build/flash/debug/firmware-packaging platform written in C (firmware) + Python 3 (tooling), driven by **CMake + Kconfig** (via `kconfiglib`), in the style of Linux kernel configuration.

- **Build chain:** Application `build.sh` (Linux/macOS) or `build.bat` (Windows) → SDK `tool/python/build.py` → CMake. Create applications from the SDK via `./fly.sh new` or `fly.bat new`.
- **Config flow:** Application `Kconfig` → `build/config/.config` → `build/config/.config.cmake` and `build/config/autoconf.h`.
- **Application `CMakeLists.txt`** is the top-level entry. `cmake/fly-configure.cmake` prepares Kconfig and the toolchain before `project()`, then `cmake/fly-sdk.cmake` adds the SDK architecture and packages.
- **CI:** GitHub Actions generates projects for the seven Cortex-M targets and three host platforms. Nightly runs the generated ARM matrix in Release. The required check names are documented in [`docs/CI.md`](docs/CI.md).
- **No formal unit-test framework.** Validation = CI + build + Segger RTT log on hardware.
- `CLAUDE.md` at the repo root is gitignored — it's the user's local OMC orchestration config and is not part of this repo.

## Directory Layout

- `arch/<vendor>/<family>/` — vendor SDK, HAL, startup, linker scripts. Each arch has a `gcc-config.cmake` (CPU flags, toolchain file selection) and a `Kconfig`. Subdirs: `arm64`, `ch58x`, `gd32`, `mcxn947`, `stm32`, `x64` (Linux + Windows host).
- `templates/` — project scaffold, application templates and board support. Generated applications live in independent repositories under the sibling `fly-project/` by default.
- `package/<name>/` — reusable components. Git-fetched packages such as `eventhub-os` and `ehip` are cloned into the application's `build/deps/<name>-src/` via `fetch_git_project()`.
- `cmake/` — toolchain files, package helpers (`package_lib_add` / `target_link_package_lib`), image packaging (`jlink-make-image.cmake`, `openocd-make-image.cmake`), VS Code debug JSON generation.
- `tool/python/` — `build.py` orchestration, `mk_jlink_img.py` / `mk_openocd_img.py` packaging, `fetch_git_project.py` mirror-aware cloning, `mk_mcu_firmware_factory_data.py` factory-data image suffix.
- `tool/test/` — `check_standalone_projects.py` exercises generated configurations, ARM and host builds, packaging and SDK setup.
- `.github/workflows/` — CI: `ci.yml` (PR gate) and `nightly.yml` (Release sweep + packaging smoke). `.github/pull_request_template.md` — PR description template.
- `resource/git.mirror-source.json` — seed for the user-local `.git.mirror-source.json` (gitignored); used to speed up package fetches for users in China.
- `tool/jlink/**` is Git-LFS tracked (`.gitattributes`).

## Build Script Subcommands

Run these commands from an application's root with `./build.sh` or `build.bat`. With no command, the script builds using the last-used build type (default `Release`).

| Command | Purpose | Example |
|---|---|---|
| `setup` | Install the SDK revision recorded in `fly.lock` | `./build.sh setup` |
| `menuconfig` | Open Kconfig TUI for application and package configuration | `./build.sh menuconfig` |
| `loadconfig <file>` | Load a configuration and regenerate `build/config/` | `./build.sh loadconfig defconfig` |
| `saveconfig [name]` | Save the selected configuration as a minimal defconfig in the application | `./build.sh saveconfig` |
| `build [type] [-j N]` | Configure + compile. `-j` also works at the top level: `./build.sh -j 44`. | `./build.sh build Debug -j 8` |
| `vscode [type]` | Configure CMake and write local VS Code and clangd settings | `./build.sh vscode Debug` |
| `clean` | `cmake --build build/cmake --target clean` (keeps the generated configuration) | `./build.sh clean` |
| `distclean` | Remove the application's `build/` and keep packaged firmware under `image/` | `./build.sh distclean` |
| `make_img [target]` | Generate firmware image package; requires `build/` to exist | `./build.sh make_img` |
| `flash [target]` | Flash via J-Link/OpenOCD backend | `./build.sh flash` |
| `rttlog` | Stream Segger RTT logs using the script in `image/CURRENT/` after packaging | `./build.sh rttlog` |
| `package_update [name] [-l]` | Run a package update target in `build/cmake/`. `-l` lists packages | `./build.sh package_update -l` |
| `package_mirror -l/-d/-e/-u` | Manage mirror sources (list / disable / enable / refresh benchmark) | `./build.sh package_mirror -l` |
| `add_path_env <path>` | Prepend a toolchain/debugger `bin/` dir to FLY's PATH (stored in `.PATH.env.json`, gitignored) | `./build.sh add_path_env /opt/gcc-arm/bin` |

## Image Targets

The application's root `CMakeLists.txt` declares firmware, debug, and image targets. `app/CMakeLists.txt` registers application sources and include paths. Image targets declared via `add_jlink_image(<name> ...)` or `add_openocd_image(<name> ...)` provide the CMake targets `<name>_flash` and `<name>_make_img`. The default name is `default`. Pass a different name as the argument to `flash` or `make_img`.

## Package System

- Each `package/<name>/CMakeLists.txt` registers its library target with `package_lib_add(<target>)` (defined in `cmake/package.cmake`).
- Registration is gated by `if(CONFIG_PACKAGE_<NAME>)` so disabling a package in `menuconfig` removes it from the build.
- A project's executable calls `target_link_package_lib(<exec>)` to link every enabled package, then may add explicit `target_compile_definitions` / `target_include_directories` per package.

## Build Environment & Toolchain

- `build.sh` picks Python in this order: `$FLY_PYTHON` → `.venv/bin/python` → `python3` → `python`. Same for `build.bat` (`%FLY_PYTHON%`, `.venv\Scripts\python.exe`, `python3`, `python`).
- If `kconfiglib` or `requests` is missing, the Python build entry installs both in the application's `build/.venv`. Set `FLY_AUTO_SETUP_PY=0` to disable installation.
- On Windows, `menuconfig` additionally needs `windows-curses` installed in the venv.
- CMake build tool is auto-detected: `ninja` preferred, then `make` (Windows path). If neither is found, the script errors out.
- Toolchain and debugger binaries are found on `PATH`. Use the application's `./build.sh add_path_env <dir-to-bin>` to prepend a directory on later builds. The setting is recorded in the SDK's ignored `.PATH.env.json`.
- Compiled outputs and generated configuration are under the application's `build/`. Packaged firmware and archives are under the application's `image/`. VS Code reads the application's generated `.vscode/launch.json`.

## Conventions

- C: `snake_case` functions/variables, `UPPERCASE` macros. Generated applications use the `fly_app` firmware target.
- Firmware target link order in this repo: `target_link_libraries(<exec> arch-mcxn947-core0 c rdimon m gcc)` then `target_link_package_lib(<exec>)` — keep that ordering when adding new projects.
- The scaffold applies its warning options to the executable and passes them through `package_lib_compile_options()`. A package that needs an exception declares its own `FLY_EXCLUDED_PACKAGE_COMPILE_OPTIONS` target property.
- Template names, overlays, and board requirements belong in template and board manifests. Shared tooling reads those declarations and applies the same rules to every template.

## PR & CI

原则借用 nanosig 上游的门禁模型：**PR 是唯一交付路径，必需检查全绿才允许合并，合并保持线性历史**。

1. 改动走特性分支 + PR，**不直接 push `master`**（push 到 master 只触发事后兜底的 CI，不是交付路径）
2. 必需检查（`kconfig`、`arm-build`、`host-linux`、`host-macos`、`host-windows`）必须全绿；分支保护设了 `strict` 时，分支落后要先与 `master` 同步并重跑。这五项曾因上游 `eventhub`（Apple Clang C11 atomic）与 `ehshell`（MinGW 缺 `bzero`/`isprint`）而暂缓，现已在 `96916a7`/`a1c1bcf` 修复合入，五项全部必需
3. 合并统一用 **Rebase and merge**（`gh pr merge --rebase`），不用 squash / merge commit；合并后删除特性分支，并自动切回本地 `master`，用 `git pull --ff-only` 同步远端；若切换会覆盖未提交改动或本地 `master` 已分叉，保留现场并报告，不自动丢弃改动或重置分支
4. 本地先跑 `python3 tool/test/check_standalone_projects.py --matrix config`。涉及 ARM 或宿主工程时，用同一脚本运行 `--matrix arm` 或 `--matrix host`。
5. 新增模板或板卡时，在板卡清单中声明已验证的模板组合，供生成和 CI 使用。
6. 分支保护与合并方式的配置命令见 [`docs/CI.md`](docs/CI.md)，改了 job 名必须同步改 `contexts`，否则检查永远不会被要求

## Commit & PR

- Commit style is emoji + Conventional Commits: `✨ feat(scope): ...`, `🐞 fix(scope): ...`, `🧪 test: ...`, `📃 docs: ...`, `🔧 chore/fix: ...`, `🧹 chore: ...`, `🎈 perf: ...`, `🔨 refactor: ...`.
- One logical change per commit. Pair code with the Kconfig / defconfig it requires.
- PR description should mention target board/project, `defconfig` used, build/test commands run, and runtime evidence (RTT log screenshot for shell/network features). Template: `.github/pull_request_template.md`.
- Regression builds use freshly generated applications outside the SDK checkout. See [CI.md](docs/CI.md).

## Do Not Commit

Tracked-by-`.gitignore` (touching them is safe, but the repo must not record them):

- `.config`, `.config.old`, `auto-generate/`
- `.PATH.env.json`, `.git.mirror-source.json`, `.cache/`
- `build/`, `image/`, `dl/`
- `tool/python/__pycache__/`, `.venv/`
- `.vscode/`, `.clangd`, `.codex`, `.claude/`, `.omc/`, `CLAUDE.md`

## Docs

Design notes for porting work, migration plans, and reference material live under `docs/` (see [`docs/README.md`](docs/README.md)). `README.md` is the user tutorial; `AGENTS.md` is repo conventions; `docs/` is for contributors and reviewers.
