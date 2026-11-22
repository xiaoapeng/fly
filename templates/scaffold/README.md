# @PROJECT_NAME@

**@BOARD_LABEL@ / @BOARD_CORE@** · `@TEMPLATE_ID@`

Build with **@TOOLCHAIN@**. The selected tool packages are **@PROBE@**.

After cloning this project, use Git and Python 3 to install the exact FLY
commit recorded in `fly.lock`.

```sh
./build.sh setup
```

On Windows run `build.bat setup`. The installed SDK lives in the ignored
`fly/` directory. Use the capability report to check tool requirements for
the selected packages.

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

## Packaging, running and debugging

```sh
./build.sh make_img
./build.sh flash-run --list
./build.sh log --list
./build.sh debug --list
```

Embedded builds produce `build/cmake/fly_app` and `fly_app.bin`. Each common
package has a manifest, final BIN, ELF and debug file under
`image/<target>/<build-type>/<artifact-id>/`. The target and build type have
their own `CURRENT.json`. Packages remain available after `distclean`.
Use `./build.sh make_img --list` to find installed export actions and
`./build.sh make_img --via PROVIDER` to run one.

Use `flash-run --via PROVIDER` to load and start the packaged firmware on
hardware or in simulation. If more than one provider is enabled for an
action, set a default in `fly.project.json` or select it explicitly. For an
archived package, pass `--artifact /path/to/manifest.json` to `flash-run`,
`log` or `debug` where the selected provider supports that input. Use
`./build.sh capabilities` for the installed packages, actions, and tool status.

Renode streams RTT during `flash-run` and saves it under `image/logs/renode/`.
Press `Ctrl+C` to stop the simulation or pass `--duration-ms` for a timed run.
In another terminal, `./build.sh log --via renode` follows the active run or
replays the saved log after it ends.

The root CMake file declares partitions with `fly_add_firmware_image()`.
Each `FIRMWARE_LIST` entry uses `NAME:VERSION:ADDRESS:CAPACITY:BIN`. To add a
bootloader, append a BOOT entry and add its build target to `DEPENDS` where
applicable. Use `SYMBOLS BOOT <target>` to include that target's ELF and debug
file alongside its BIN. Relative BIN paths use the CMake build directory.
The packager checks final size with padding and the CRC trailer, plus
partition ranges and the ELF load address.

@STARTUP_LOG_NOTE@Embedded templates enable `CONFIG_FACTORY_DATA_SUPPORTED`.
With a logging sink enabled, startup logs the firmware identity, version,
address and CRC read from Flash. It reports `Firmware info unavailable`
when it cannot find a valid trailer.

Open the application root in VS Code. **FLY: Configure** generates local
settings and the debug profiles available on this machine. Use
`./build.sh vscode --artifact /path/to/manifest.json` to bind archived
symbols and select the generated profile. **Flash and attach** performs an
explicit flash of that archive before connecting. Use ordinary attach for
the current build's ELF.
For a plain CMake build, select `default_make_img` to publish the same
common package. On Windows use `build.bat` with the same subcommands.

## Application layout

Embedded templates start in `app/main.c`. Native host profiles select their
entry file in `board/board.cmake`.

| Source | Public header | Purpose |
| --- | --- | --- |
| `app/mid/firmware_info.c` | `app/include/mid/firmware_info.h` | Firmware trailer information |
| `app/mid/fly_log.c` | `app/include/mid/fly_log.h` | Common logging interface and fallback sink |

Place single-source business modules directly in `app/biz/` and middleware
modules in `app/mid/`. Give larger modules a subdirectory and keep their
private headers next to their sources. `app/CMakeLists.txt` registers
`app/include/` as the application's public header search path. Include
headers from their subdirectory, for example `"mid/firmware_info.h"`.

Board headers live in `board/include/`, beside the board CMake file and
linker script. Installed package manifests live under `tool/packs/` and
the chosen package IDs are recorded in `fly.project.json`.

The selected board determines the architecture and toolchain.

Use `menuconfig` to experiment with Kconfig settings and `saveconfig` to
persist the selected settings into `defconfig`.
