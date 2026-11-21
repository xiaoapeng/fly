# @PROJECT_NAME@

**@BOARD_LABEL@ / @BOARD_CORE@** · `@TEMPLATE_ID@`

Build with **@TOOLCHAIN@**. The selected debug interface is **@PROBE@**.

After cloning this project, use Git and Python 3 to install the exact FLY
commit recorded in `fly.lock`.

```sh
./build.sh setup
```

On Windows run `build.bat setup`. The installed SDK lives in the ignored
`fly/` directory. Install and enable Git LFS before setup if you need the
bundled J-Link and RTT tools.

The build scripts and CMake use the same application-side SDK resolver. It
looks for `FLY_ROOT`, the application's `fly/` directory or link, and
`.fly-root`, in that order. An explicit `FLY_SDK_DIR` takes priority when
configuring CMake directly. The chosen path is recorded separately. The
resolver checks the SDK's `HEAD` against `fly.lock` before loading the SDK.
A mismatch stops the operation. A change to the remote branch tip leaves
the locked commit in effect.

- For an SDK installed by setup, rerun `./build.sh setup` or `build.bat setup`
  after updating the commit in `fly.lock`, then rebuild. If the Git URL
  changes, move the existing `fly/` checkout aside before running setup.
- For local development, select a checkout with `FLY_ROOT`, a `fly/` link
  or directory in the project root, or the `.fly-root` path file. Direct
  CMake configuration also accepts `-DFLY_SDK_DIR=/path/to/fly`. If the
  selected SDK differs from the lock, select a checkout with the matching
  `HEAD` or clear the local path overrides and move any manually supplied
  `fly/` aside before running setup. Update or clear any cached `FLY_SDK_DIR`
  used for direct CMake configuration. The version check is read-only.

When upgrading, clear any `FLY_SDK_DIR` entry that an older tool generated
in the CMake cache or VS Code settings. Keep explicit entries you use to
select an SDK for CMake.

Manually selected SDK checkouts can contain local edits when their `HEAD`
matches the pin. A project without `fly.lock`, or with `commit` set to
`null`, can use a local SDK. Managed installation requires a Git URL and
the full commit hash in `fly.lock`.

Before sharing the application, commit `fly.lock` with it and publish the
matching SDK commit at the URL in the lock file. Verify setup and the build
in a fresh application clone with local SDK overrides cleared.

Run these commands from the project directory.

```sh
./build.sh build Debug
```

The root `CMakeLists.txt` declares the firmware, packaging, and debug targets
and requires CMake 3.21 or later. `app/CMakeLists.txt` registers application
sources, include directories, and optional application configuration. With
the SDK and toolchain installed, you can configure and build directly from the
application directory.

```sh
cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build/cmake
```

When configuring directly with MinGW-w64 and Ninja on Windows, add `-G Ninja`
on the first configure.

Run these commands on boards with a firmware packaging target.

```sh
./build.sh make_img
./build.sh flash
./build.sh rttlog
```

On Windows use `build.bat` with the same subcommands and keep the project on a
local filesystem that supports directory junctions, such as NTFS. @STARTUP_LOG_NOTE@Embedded builds produce the ELF at
`build/cmake/fly_app` and the BIN at `build/cmake/fly_app.bin`. Packaged firmware
and ZIP archives are under `image/` in the application root, with the current
package at `image/CURRENT/`. Deleting `build/` or running
`./build.sh distclean` leaves the packages under `image/` available.

Packaging reuses a previous package when its inputs match and all packaged
files have their recorded sizes. Missing or truncated firmware, flash
scripts, or bundled files trigger a new package.

If an earlier SDK saved packages under `build/image/`, copy the ZIP archives
you want to keep to `image/` before cleaning. Packaging again with the updated
SDK refreshes `image/CURRENT/`.

Set the firmware name and version in the root `CMakeLists.txt`. It includes
`tool/cmake/app-image.cmake` and declares the images with
`app_add_jlink_image()`. The application module passes the board's
`FLY_JLINK_DEVICE` to the SDK's `add_jlink_image()` function. Each
`FIRMWARE_LIST` entry uses `NAME:VERSION:ADDRESS:CAPACITY:BIN`. The default
`APP` entry uses `FLY_FLASH_SIZE` from the board configuration. A capacity
is a byte count in decimal or hexadecimal with a `0x` prefix. Packaging
checks the final BIN, including alignment padding and the metadata trailer,
and fails if it exceeds its partition capacity. This check runs regardless
of `CONFIG_FACTORY_DATA_SUPPORTED`.

To package a bootloader alongside the application, set its version, flash
address, partition capacity, and BIN path, then add this entry to
`FIRMWARE_LIST`.

```cmake
"BOOT:${BOOT_VERSION}:${BOOT_FLASH_ADDRESS}:${BOOT_FLASH_SIZE}:${BOOT_BIN}"
```

Resolve relative BIN paths from the CMake build directory (`build/cmake/`
with the build script). Add the bootloader target to `DEPENDS` if this build
generates its BIN. Match each partition's address and capacity to the flash
layout and keep the partitions from overlapping.

Embedded templates enable `CONFIG_FACTORY_DATA_SUPPORTED` by default. After
flashing the packaged BIN, the application validates the trailer in Flash and
prints the firmware name and version, partition and address, aligned image
length, trailer length, CRC32 and packaging details once on RTT channel 0.
If no valid trailer is found, it prints `Firmware info unavailable` and
continues starting. To disable the check and log, turn off the option in
`menuconfig` and use `saveconfig` to update `defconfig`.

The root `CMakeLists.txt` includes `tool/cmake/app-vscode.cmake` and calls
`app_add_vscode_debug(fly_app)`. Edit `tool/cmake/app-vscode.cmake` to change
application debug options such as RTT. The application module checks the
board's `FLY_JLINK_DEVICE`, GDB, and objdump, then passes the options to the
SDK's `add_vscode_cortex_debug_gdb()` function.

Host projects and boards without J-Link configured skip J-Link packaging
and debugging. The application module also skips debug configuration if
GDB or objdump is missing. Embedded builds use objdump for disassembly
listings. Open the project root in VS Code to use the generated
`.vscode/launch.json` entries.

When `default_flash` exists, the application module enables the flash task
for **default-fly_app (flash BIN and attach)**. After the first build, select
it to debug the packaged firmware. VS Code runs **FLY: Flash** to program the
images in the package, then attaches using symbols from the application ELF.
The target resets and runs before the debugger attaches.

Open **Tasks: Run Task** and use **FLY: Setup** if the SDK needs installation.
Run **FLY: Configure** to prepare local settings for clangd and CMake Tools
before compiling, or use `./build.sh vscode Debug` (`build.bat vscode Debug`
on Windows). **FLY: Build** also generates these settings. In CMake Tools,
select the **FLY** kit and run **CMake: Configure** from the application
workspace. On boards with packaging targets, select `default_make_img` or
`default_flash` through **CMake: Build Target**.
The **FLY: Build**, **FLY: Make image**, and **FLY: Flash** tasks call the
application build script.

## Application layout

Embedded templates start in `app/main.c`. Native host profiles select their
entry file in `board/board.cmake`.

| Source | Public header | Purpose |
| --- | --- | --- |
| `app/runtime.c` | `app/include/runtime.h` | Runtime for the two nanosig templates |
| `app/biz/app_demo.c` | `app/include/biz/app_demo.h` | Timer demo for the two nanosig templates |
| `app/mid/firmware_info.c` | `app/include/mid/firmware_info.h` | Firmware trailer logging shared by the templates |

Place single-source business modules directly in `app/biz/` and middleware
modules in `app/mid/`. Give larger modules a subdirectory and keep their
private headers next to their sources. `app/CMakeLists.txt` registers
`app/include/` as the application's public header search path. Include
headers as `"biz/app_demo.h"`, `"mid/firmware_info.h"`, or `"runtime.h"`.
The bare-metal template uses `.gitkeep` to keep `app/biz/` and
`app/include/biz/` ready for business modules.

Board headers, when provided, live in `board/include/`. The board manifest
`board.json`, build configuration `board.cmake`, and any board-specific
linker scripts live in `board/`.

In the nanosig templates, the bare-metal entry point or the FreeRTOS application
task calls `app_runtime_init()` and passes the loop from `app_runtime_get_loop()`
to `app_demo_init()`. After `app_runtime_run()` returns, the entry point calls
`app_demo_deinit()` before `app_runtime_deinit()`. Each module manages its own
resources. The entry point releases modules in reverse initialization order
before shutting down the runtime. If a module needs separate start and stop
operations, define them in the module and call them around the run step.

Use `menuconfig` to experiment with Kconfig settings and `saveconfig` to
persist the selected settings into `defconfig`.
