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
bundled J-Link and RTT tools. To work with a shared SDK, set `FLY_ROOT` to
its path or create a local `fly` link. The generator creates a local SDK
reference on the machine where the project is first created.

Commit `fly.lock` with the application. Before sharing it, make sure its
SDK commit is available from the Git URL in the lock file. To upgrade the
locked SDK installed by setup, edit its commit in `fly.lock`, run setup
again, and rebuild.

Run these commands from the project directory.

```sh
./build.sh build Debug
```

The application's `CMakeLists.txt` is the top-level entry and requires CMake
3.21 or later. With the SDK and toolchain installed, you can also configure and
build directly from the application directory.

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
local filesystem that supports directory junctions, such as NTFS. The program's
startup output is `@EXPECTED_LOG@`. The binary and packaging output are under
`build/`.

Set the firmware name and version in the application's `CMakeLists.txt`.
For boards with debug targets, building with GDB and objdump installed generates
`.vscode/launch.json` in the project root. Open that directory in VS Code to use
the debug entries.

On those boards, select **default-fly_app (flash BIN and attach)** after the
first build to debug the packaged firmware. VS Code runs **FLY: Flash** to
program the final BIN, then attaches using symbols from the matching ELF. The
target resets and runs before the debugger attaches.

Open **Tasks: Run Task** and use **FLY: Setup** if the SDK needs installation.
Run **FLY: Configure** to prepare local settings for clangd and CMake Tools
before compiling, or use `./build.sh vscode Debug` (`build.bat vscode Debug`
on Windows). **FLY: Build** also generates these settings. In CMake Tools,
select the **FLY** kit and run **CMake: Configure** from the application
workspace. On boards with packaging targets, select `default_make_img` or
`default_flash` through **CMake: Build Target**.
The **FLY: Build**, **FLY: Make image**, and **FLY: Flash** tasks call the
application build script.

Embedded templates start in `app/main.c`. Native host profiles select their
entry file in `board/board.cmake`. The current board setup is described in
`board/board.json`. Use `menuconfig` to experiment with Kconfig settings and
`saveconfig` to persist the selected settings into `defconfig`.
