# Repository Guidelines

## Project Structure & Module Organization
- `arch/`: MCU/CPU architecture support (startup code, linker scripts, CMSIS/HAL glue).
- `package/`: reusable components (for example `eventhub-os`, `ehip`, `ehshell`, `segger-rtt`) gated by Kconfig options.
- `project/`: board/app entry points. Each project has its own `CMakeLists.txt`, `Kconfig` selections, and usually a `defconfig`.
- `cmake/`: shared CMake functions/toolchain integration.
- `tool/python/`: build orchestration scripts (`build.py`, flashing/image helpers, package update tooling).
- Build outputs are generated under `build/` and auto config under `auto-generate/` (do not commit generated artifacts).

## Build, Test, and Development Commands
- `./build.sh menuconfig`: interactive Kconfig selection.
- `./build.sh loadconfig project/stm32f0xx/stm32f030x8-demo/defconfig`: load a known baseline.
- `./build.sh` or `./build.sh build Release -j 8`: configure + compile with CMake.
- `./build.sh build Debug`: debug build variant.
- `./build.sh make_img` / `./build.sh flash`: generate firmware images and flash via configured backend.
- `./build.sh rttlog`: open RTT log capture for runtime validation.
- `./build.sh clean` / `./build.sh distclean`: clean targets or remove build state.

## Coding Style & Naming Conventions
- Language is primarily C/CMake/Python; keep 4-space indentation and braces in K&R style consistent with surrounding files.
- Prefer descriptive snake_case for variables/functions and UPPER_SNAKE_CASE for macros/config symbols (for example `EH_CONFIG_*`, `PACKAGE_*`).
- Keep headers self-contained, with include guards (`_NAME_H_`) and minimal cross-module coupling.
- Reuse existing warning discipline (`-Wall -Wextra`) and avoid introducing new compiler warnings.

## Testing Guidelines
- No single global unit-test runner is enforced; validate per target/project.
- For host checks, use `project/linux/ehip-test/defconfig` or `project/macos/ehip-test/defconfig` and run `./build.sh`.
- For MCU changes, verify: compile passes, image generation works, and device log/behavior is confirmed (`./build.sh rttlog`).
- Name test/demo additions with `*-test` or `*-demo` to match existing project layout.

## Commit & Pull Request Guidelines
- Follow the current commit style seen in history: `emoji type[(scope)]: description` (example: `✨ feat(mcxn947-eth-demo): add DHCP fallback`).
- Keep commits focused: one logical change per commit, with config updates and code together.
- PRs should include target project/board, `defconfig` used, key build/test commands run, and runtime evidence (logs or screenshots for shell/network features).
- Link related issues and call out Kconfig/default behavior changes explicitly.

## Environment & Configuration Tips
- `build.sh` now auto-detects Python in this order: `FLY_PYTHON` -> `.venv/bin/python` -> `python3` -> `python`.
- If `kconfiglib` is missing, `build.sh` will auto-install required deps (`kconfiglib`, `requests`) by default.
- Set `FLY_AUTO_SETUP_PY=0` to disable auto-install and handle dependencies manually.
- On systems where `.venv` creation fails, `build.sh` falls back to user-level pip install (`python -m pip install --user ...`).
- Use `./build.sh add_path_env <toolchain_or_openocd_path>` to persist tool paths in repo-local config.
