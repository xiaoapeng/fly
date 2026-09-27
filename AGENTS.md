# Repository Guidelines

## Project at a Glance

FLY is a one-stop embedded build/flash/debug/firmware-packaging platform written in C (firmware) + Python 3 (tooling), driven by **CMake + Kconfig** (via `kconfiglib`), in the style of Linux kernel configuration.

- **Build chain:** `./build.sh` (Linux/macOS) or `build.bat` (Windows) → `tool/python/build.py` → CMake → cross-compiled firmware.
- **Config flow:** `Kconfig` menus → `.config` → `auto-generate/.config.cmake` (CMake vars) + `auto-generate/autoconf.h` (C header).
- **Top-level CMakeLists.txt** loads `.config.cmake`, then `arch/${CONFIG_ARCH}/gcc-config.cmake`, then adds `arch/`, `package/`, and `project/${CONFIG_PROJECT}` subdirectories.
- **No CI** (no `.github/`). No formal unit-test framework. Validation = build + Segger RTT log on hardware.
- `CLAUDE.md` at the repo root is gitignored — it's the user's local OMC orchestration config and is not part of this repo.

## Directory Layout

- `arch/<vendor>/<family>/` — vendor SDK, HAL, startup, linker scripts. Each arch has a `gcc-config.cmake` (CPU flags, toolchain file selection) and a `Kconfig`. Subdirs: `arm64`, `ch58x`, `gd32`, `mcxn947`, `stm32`, `x64` (Linux + Windows host).
- `project/<family>/<demo>/` — board target. Always: `CMakeLists.txt`, `defconfig`, `app/` (`main.c`, `.ld`), `board/` (init/clocks/GPIO). Optional: `config-inc/`, `general/`, `inc/`.
- `package/<name>/` — reusable components. Local packages live in `package/`; git-fetched packages (`eventhub-os`, `ehip`) are cloned into `dl/<name>-src/` via `cmake/fetch_git_project.cmake::fetch_git_project()`.
- `cmake/` — toolchain files, package helpers (`package_lib_add` / `target_link_package_lib`), image packaging (`jlink-make-image.cmake`, `openocd-make-image.cmake`), VS Code debug JSON generation.
- `tool/python/` — `build.py` orchestration, `mk_jlink_img.py` / `mk_openocd_img.py` packaging, `fetch_git_project.py` mirror-aware cloning, `mk_mcu_firmware_factory_data.py` factory-data image suffix.
- `resource/git.mirror-source.json` — seed for the user-local `.git.mirror-source.json` (gitignored); used to speed up package fetches for users in China.
- `tool/jlink/**` is Git-LFS tracked (`.gitattributes`).

## Build Script Subcommands

All commands work on `./build.sh` and `build.bat` identically. With no command, the script builds using the last-used build type (default `Release`).

| Command | Purpose | Example |
|---|---|---|
| `menuconfig` | Open Kconfig TUI to change arch/project/packages | `./build.sh menuconfig` |
| `loadconfig <file>` | Load a `defconfig` (regenerates `auto-generate/`) | `./build.sh loadconfig project/stm32f0xx/stm32f030x8-demo/defconfig` |
| `saveconfig [name]` | Save current `.config` as minimal defconfig into the active project's directory (resolves `CONFIG_PROJECT` path) | `./build.sh saveconfig` |
| `build [type] [-j N]` | Configure + compile. `-j` also works at the top level: `./build.sh -j 44`. | `./build.sh build Debug -j 8` |
| `clean` | `cmake --build build --target clean` (keeps `.config`) | `./build.sh clean` |
| `distclean` | Remove `build/`, `.config`, `auto-generate/` | `./build.sh distclean` |
| `make_img [target]` | Generate firmware image package; requires `build/` to exist | `./build.sh make_img` |
| `flash [target]` | Flash via J-Link/OpenOCD backend | `./build.sh flash` |
| `rttlog` | Stream Segger RTT logs by invoking `image/CURRENT/log.sh` (or `log.bat` on Windows); requires `make_img` first | `./build.sh rttlog` |
| `package_update [name] [-l]` | Run `cmake --build build --target package_update_<name>`; `-l` lists packages | `./build.sh package_update -l` |
| `package_mirror -l/-d/-e/-u` | Manage mirror sources (list / disable / enable / refresh benchmark) | `./build.sh package_mirror -l` |
| `add_path_env <path>` | Prepend a toolchain/debugger `bin/` dir to FLY's PATH (stored in `.PATH.evn.json`, gitignored) | `./build.sh add_path_env /opt/gcc-arm/bin` |

## Image Targets

A project's `CMakeLists.txt` declares image targets via `add_jlink_image(<name> ...)` / `add_openocd_image(<name> ...)`. `<name>` resolves to the CMake targets `<name>_flash` and `<name>_make_img`. Default name is `default`; pass a different name as the argument to `flash` / `make_img`.

## Package System

- Each `package/<name>/CMakeLists.txt` registers its library target with `package_lib_add(<target>)` (defined in `cmake/package.cmake`).
- Registration is gated by `if(CONFIG_PACKAGE_<NAME>)` so disabling a package in `menuconfig` removes it from the build.
- A project's executable calls `target_link_package_lib(<exec>)` to link every enabled package, then may add explicit `target_compile_definitions` / `target_include_directories` per package (see `project/mcxn947/mcxn947-eth-demo/CMakeLists.txt` for a worked example).

## Build Environment & Toolchain

- `build.sh` picks Python in this order: `$FLY_PYTHON` → `.venv/bin/python` → `python3` → `python`. Same for `build.bat` (`%FLY_PYTHON%`, `.venv\Scripts\python.exe`, `python3`, `python`).
- If `kconfiglib` (or `requests`) is missing, the script auto-creates `.venv` and installs them. Set `FLY_AUTO_SETUP_PY=0` to disable.
- On Windows, `menuconfig` additionally needs `windows-curses` installed in the venv.
- CMake build tool is auto-detected: `ninja` preferred, then `make` (Windows path). If neither is found, the script errors out.
- Toolchain/debugger binaries are **not** picked up from the system `PATH` automatically — they must be added once via `./build.sh add_path_env <dir-to-bin>` (writes to `.PATH.evn.json`, gitignored). The build script prepends these to `PATH` on every invocation.
- Build artifacts land in `build/` and `image/` (both gitignored). Generated config in `auto-generate/` (gitignored).

## Conventions

- C: `snake_case` functions/variables, `UPPERCASE` macros. CMake target names use `<board>_<role>` form (e.g. `mcxn947_eth_demo_app`).
- Firmware target link order in this repo: `target_link_libraries(<exec> arch-mcxn947-core0 c rdimon m gcc)` then `target_link_package_lib(<exec>)` — keep that ordering when adding new projects.
- Build flags come from a per-project `set(TARGET_FLAGS "-Wall" "-Wextra" "-Wconversion" "-Wsign-conversion" "-Wno-psabi" ...)` and are applied both to packages (`package_lib_compile_options`) and to the executable. Avoid introducing new compiler warnings.

## Commit & PR

- Commit style is emoji + Conventional Commits: `✨ feat(scope): ...`, `🐞 fix(scope): ...`, `🧪 test: ...`, `📃 docs: ...`, `🔧 chore/fix: ...`, `🧹 chore: ...`, `🎈 perf: ...`, `🔨 refactor: ...`.
- One logical change per commit. Pair code with the Kconfig / defconfig it requires.
- PR description should mention target board/project, `defconfig` used, build/test commands run, and runtime evidence (RTT log screenshot for shell/network features).
- Test/demo additions follow the `*-test` or `*-demo` naming already used in `project/`.

## Do Not Commit

Tracked-by-`.gitignore` (touching them is safe, but the repo must not record them):

- `.config`, `.config.old`, `auto-generate/`
- `.PATH.evn.json`, `.git.mirror-source.json`, `.cache/`
- `build/`, `image/`, `dl/`
- `tool/python/__pycache__/`, `.venv/`
- `.vscode/`, `.clangd`, `.codex`, `.claude/`, `.omc/`, `CLAUDE.md`, `.PATH.env.json`

## Docs

Design notes for porting work, migration plans, and reference material live under `docs/` (see [`docs/README.md`](docs/README.md)). `README.md` is the user tutorial; `AGENTS.md` is repo conventions; `docs/` is for contributors and reviewers.
