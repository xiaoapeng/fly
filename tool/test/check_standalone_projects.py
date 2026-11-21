#!/usr/bin/env python3
"""Validate FLY projects generated in temporary application workspaces."""

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


TEMPLATES = ("baremetal", "baremetal-nanosig", "freertos-nanosig")
FRDM_BOARD = "frdm-mcxn947-core0"
HOST_ARCHES = {
    "linux": "CONFIG_ARCH_X64_LINUX",
    "darwin": "CONFIG_ARCH_ARM64_MACOS",
    "win32": "CONFIG_ARCH_X64_WINDOWS",
}
RISCV_ARCHES = {"CONFIG_ARCH_CH58X", "CONFIG_ARCH_GD32VF103X"}


def entrypoint(directory, name, *args):
    if os.name == "nt":
        return [os.environ.get("COMSPEC", "cmd.exe"), "/d", "/c", "call",
                str(directory / f"{name}.bat"), *args]
    return [str(directory / f"{name}.sh"), *args]


def run(command, cwd, env=None):
    result = subprocess.run(command, cwd=cwd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, timeout=900, env=env)
    if result.returncode:
        raise RuntimeError(f"{command} in {cwd} failed:\n{result.stdout[-6000:]}")
    return result.stdout


def commit(repo, message, *, allow_empty=False):
    run(["git", "add", "-A"], repo)
    command = ["git", "-c", "user.name=FLY test", "-c", "user.email=fly@example.invalid",
               "-c", "commit.gpgsign=false", "commit", "-qm", message]
    if allow_empty:
        command.append("--allow-empty")
    run(command, repo)
    return run(["git", "rev-parse", "HEAD"], repo).strip()


def generate(sdk, project, name, board, template):
    run(entrypoint(sdk, "fly", "new", name, "--board", board,
                   "--template", template, "--output", str(project)), sdk)


def check_sdk_setup(sdk, workspace):
    old_build = subprocess.run(entrypoint(sdk, "fly", "build"), cwd=sdk,
                               capture_output=True, text=True, timeout=30)
    if old_build.returncode == 0:
        raise RuntimeError("The SDK entry point still accepts root builds")

    origin = workspace / "SDK origin"
    origin.mkdir()
    run(["git", "init", "-q"], origin)
    (origin / "tool/python").mkdir(parents=True)
    (origin / ("fly.bat" if os.name == "nt" else "fly.sh")).touch()
    (origin / "cmake").mkdir()
    (origin / "cmake/fly-configure.cmake").touch()
    entry = origin / "tool/python/build.py"
    entry.write_text('import sys\nprint("sdk-one", *sys.argv[1:])\n', encoding="utf-8")
    pinned = commit(origin, "First SDK revision")
    entry.write_text('import sys\nprint("sdk-two", *sys.argv[1:])\n', encoding="utf-8")
    latest = commit(origin, "Second SDK revision")

    app = workspace / "original app"
    generate(sdk, app, "setup-check", FRDM_BOARD, "baremetal")
    if not run(["git", "ls-files", "--stage", "--", "build.sh"], app).startswith("100755 "):
        raise RuntimeError("New project's build.sh is missing the executable bit in Git")
    run(["git", "config", "core.filemode", "false"], app)
    (app / "build.sh").chmod(0o644)
    lock = app / "fly.lock"
    if not lock.is_file():
        raise RuntimeError("New project did not get a fly.lock")
    lock.write_text(json.dumps({"url": origin.as_uri(), "commit": pinned}), encoding="utf-8")
    commit(app, "Publish application scaffold")
    clone = workspace / "downloaded app"
    run(["git", "clone", "-q", "--", str(app), str(clone)], workspace)
    if (clone / "fly").exists() or (clone / "fly").is_symlink():
        raise RuntimeError("The application published its local FLY link")

    env = os.environ.copy()
    env.pop("FLY_ROOT", None)
    missing = subprocess.run(entrypoint(clone, "build", "build"), cwd=clone, env=env, text=True,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    if missing.returncode == 0 or "setup" not in missing.stdout or (clone / "fly").exists():
        raise RuntimeError(f"Build did not direct the user to SDK setup:\n{missing.stdout}")

    installed = clone / "fly"
    setup = entrypoint(clone, "build", "setup")
    build = entrypoint(clone, "build", "build")
    run(setup, clone, env)
    if run(["git", "rev-parse", "HEAD"], installed).strip() != pinned:
        raise RuntimeError("Setup must install the locked SDK revision")
    run(setup, clone, env)
    output = run(build, clone, env)
    if "sdk-one --project-dir" not in output:
        raise RuntimeError(f"Build did not use the installed SDK:\n{output}")

    (clone / "fly.lock").write_text(json.dumps({"url": origin.as_uri(), "commit": latest}),
                                  encoding="utf-8")
    mismatched = subprocess.run(build, cwd=clone, env=env, text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    if mismatched.returncode == 0 or "setup" not in mismatched.stdout:
        raise RuntimeError(f"Build accepted an SDK that differs from fly.lock:\n{mismatched.stdout}")
    if run(["git", "rev-parse", "HEAD"], installed).strip() != pinned:
        raise RuntimeError("Build changed the installed SDK")
    run(setup, clone, env)
    output = run(build, clone, env)
    if "sdk-two --project-dir" not in output:
        raise RuntimeError(f"Build did not use the updated SDK revision:\n{output}")
    print("SDK setup after cloning an application: PASS", flush=True)


def check_native_sdk_pin(sdk, workspace):
    origin = workspace / "native SDK origin"
    origin.mkdir()
    run(["git", "init", "-q"], origin)
    for file in ("cmake/fly-configure.cmake", "cmake/python3.cmake",
                 "tool/python/build.py", "tool/python/build_environment.py"):
        destination = origin / file
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(sdk / file, destination)
    installed_pin = commit(origin, "SDK for native configure")
    new_pin = commit(origin, "New SDK revision", allow_empty=True)

    app = workspace / "native app"
    generate(sdk, app, "native-pin-check", FRDM_BOARD, "baremetal")
    local_sdk = app / "fly"
    if local_sdk.is_symlink():
        local_sdk.unlink()
    (app / ".fly-root").unlink(missing_ok=True)
    lock = app / "fly.lock"
    lock.write_text(json.dumps({"url": origin.as_uri(), "commit": installed_pin}), encoding="utf-8")
    env = os.environ.copy()
    env.pop("FLY_ROOT", None)
    env["FLY_AUTO_SETUP_PY"] = "0"
    run(entrypoint(app, "build", "setup"), app, env)
    lock.write_text(json.dumps({"url": origin.as_uri(), "commit": new_pin}), encoding="utf-8")

    command = ["cmake", "-S", str(app), "-B", str(app / "build/cmake"),
               f"-DPython3_EXECUTABLE:FILEPATH={sys.executable}"]
    result = subprocess.run(command, cwd=app, env=env, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, timeout=30)
    if result.returncode == 0 or "differs from fly.lock" not in result.stdout:
        raise RuntimeError(f"Direct CMake did not report the stale SDK pin:\n{result.stdout}")
    if (app / "build/config/autoconf.h").exists():
        raise RuntimeError("Direct CMake prepared Kconfig before validating the SDK pin")
    print("Native CMake rejects a stale SDK pin: PASS", flush=True)


def check_openocd_script(sdk, workspace):
    project = workspace / "OpenOCD app"
    project.mkdir()
    run(["git", "init", "-q"], project)
    (project / "firmware.bin").write_bytes(b"\x00" * 16)
    config = project / "board.cfg"
    config.touch()
    images = project / "image output"
    run([sys.executable, "-B", str(sdk / "tool/python/mk_openocd_img.py"),
         "--firmware-name", "openocd-check", "--output-dir", str(images),
         "--config-file-list", str(config),
         "--firmware-parts", "APP:0.1.0:0x00000000:firmware.bin"], project)
    tools = project / "test tools"
    tools.mkdir()
    probe = tools / "openocd"
    probe.write_text("#!/bin/sh\ntest -s APP.bin || exit 38\nexit 37\n", encoding="utf-8")
    probe.chmod(0o755)
    env = dict(os.environ, PATH=str(tools) + os.pathsep + os.environ["PATH"])
    result = subprocess.run(["bash", str(images / "CURRENT/burn.sh")], cwd=project,
                            env=env, capture_output=True, text=True, timeout=30)
    if result.returncode != 37:
        raise RuntimeError(f"OpenOCD script failed to run in the package and return its status:\n"
                           f"{result.stdout}{result.stderr}")
    print("OpenOCD package with spaced paths and failed probe: PASS", flush=True)


def check_jlink_requires_binary(sdk, workspace):
    images = workspace / "J-Link images"
    empty = workspace / "empty firmware.bin"
    empty.touch()
    for binary in (workspace / "missing firmware.bin", empty):
        command = [sys.executable, "-B", str(sdk / "tool/python/mk_jlink_img.py"),
                   "--firmware-name", "jlink-check", "--jlink-path", str(sdk / "tool/jlink"),
                   "--output-dir", str(images), "--chip-name", "MCXN947_M33_0",
                   "--speed", "4000", "--jtagconf=-1,-1", "--nm", sys.executable,
                   "--firmware-parts", f"APP:0.1.0:0x00000000:{binary}"]
        result = subprocess.run(command, cwd=workspace, capture_output=True, text=True, timeout=30)
        if result.returncode == 0 or "Firmware binary is missing or empty" not in result.stderr or images.exists():
            raise RuntimeError(f"J-Link packager accepted {binary.name} or published a package:\n"
                               f"{result.stdout}{result.stderr}")
    print("J-Link refuses missing or empty firmware: PASS", flush=True)


def check_image_links(sdk, workspace):
    images = workspace / "image output"
    images.mkdir()
    inner = images / "target CURRENT"
    outer = images / "CURRENT"
    helper = [sys.executable, "-B", str(sdk / "tool/python/image_directory_link.py")]
    for version in ("first", "second"):
        package = images / f"package {version}"
        package.mkdir()
        (package / "APP.bin").write_text(version, encoding="utf-8")
        run(helper + [str(inner), str(package)], workspace)
        run(helper + [str(outer), str(inner)], workspace)
        if (outer / "APP.bin").read_text(encoding="utf-8") != version:
            raise RuntimeError(f"CURRENT did not publish the {version} package")
    print("Firmware directory aliases: PASS", flush=True)


def check_image(project, name, version, revision, build_type):
    package = (project / "build/image/CURRENT").resolve(strict=True)
    if not package.name.startswith(f"{name}_{build_type}_jlink_APPv{version}_"):
        raise RuntimeError(f"Image did not use application name and version: {package.name}")
    if not package.name.endswith(revision[:10]):
        raise RuntimeError(f"Image did not use application HEAD {revision}: {package.name}")
    if not package.with_name(package.name + ".zip").is_file():
        raise RuntimeError(f"Image archive is missing for {package.name}")


def check_vscode(sdk, project):
    vscode = project / ".vscode"
    settings = json.loads((vscode / "settings.json").read_text(encoding="utf-8"))
    if settings["cmake.sourceDirectory"] != "${workspaceFolder}":
        raise RuntimeError("CMake Tools must configure the application entry point")
    if (settings["cmake.buildDirectory"] != "${workspaceFolder}/build/cmake"
            or Path(settings["cmake.configureSettings"]["FLY_SDK_DIR"]).resolve() != sdk):
        raise RuntimeError("CMake Tools must use the application's build directory and the selected SDK")

    database_dir = project / "build/cmake"
    expected_database_arg = f"--compile-commands-dir={database_dir.resolve().as_posix()}"
    gcc = settings["cmake.configureSettings"]["CMAKE_C_COMPILER"]
    clangd_args = settings["clangd.arguments"]
    drivers = next(arg.removeprefix("--query-driver=").split(",") for arg in clangd_args
                   if arg.startswith("--query-driver="))
    if expected_database_arg not in clangd_args or gcc not in drivers:
        raise RuntimeError("clangd must use the application's database and CMake's cross-compiler")
    kits = json.loads((vscode / "cmake-kits.json").read_text(encoding="utf-8"))
    if not any(kit["name"] == "FLY" and kit["compilers"]["C"] == gcc for kit in kits):
        raise RuntimeError("The FLY kit must use the compiler selected by CMake")
    run(["git", "check-ignore", "--", ".vscode/settings.json"], project)
    run(["git", "check-ignore", "--", ".vscode/cmake-kits.json"], project)


def check_build_outputs(sdk, project, *, host=False):
    cmake_dir = project / "build/cmake"
    cache = (cmake_dir / "CMakeCache.txt").read_text(encoding="utf-8")
    cache_paths = {}
    for line in cache.splitlines():
        if line.startswith(("CMAKE_HOME_DIRECTORY:", "FLY_SDK_DIR:")):
            key, _, value = line.partition("=")
            cache_paths[key.split(":", 1)[0]] = Path(value).resolve()
    if cache_paths.get("CMAKE_HOME_DIRECTORY") != project.resolve():
        raise RuntimeError(f"{project.name} did not configure from its own CMakeLists.txt")
    if cache_paths.get("FLY_SDK_DIR") != sdk:
        raise RuntimeError(f"{project.name} did not use the current SDK checkout")

    executable_name = "fly_app.exe" if host and os.name == "nt" else "fly_app"
    outputs = [
        project / "build/config/.config",
        project / "build/config/.config.cmake",
        project / "build/config/autoconf.h",
        cmake_dir / "compile_commands.json",
        cmake_dir / "app" / executable_name,
    ]
    if not host:
        outputs.append(cmake_dir / "app/fly_app.bin")
    for path in outputs:
        if not path.is_file() or not path.stat().st_size:
            raise RuntimeError(f"{project.name} did not produce {path.relative_to(project)}")
    return cache


def check_debug_launch(project):
    launch = json.loads((project / ".vscode/launch.json").read_text(encoding="utf-8"))
    expected_requests = {
        "default-fly_app": "launch",
        "default-fly_app (attach)": "attach",
        "default-fly_app (flash BIN and attach)": "attach",
    }
    entries = [entry for entry in launch["configurations"] if entry["name"] in expected_requests]
    profiles = {entry["name"]: entry for entry in entries}
    if profiles.keys() != expected_requests.keys() or len(entries) != len(profiles):
        raise RuntimeError(f"{project.name} did not generate each FLY debug profile exactly once")

    elf = (project / "build/cmake/app/fly_app").resolve()
    for name, request in expected_requests.items():
        profile = profiles[name]
        if profile["request"] != request or Path(profile["executable"]) != elf:
            raise RuntimeError(f"{project.name} generated an invalid {name} profile for {elf}")

    flash_attach = profiles["default-fly_app (flash BIN and attach)"]
    normal_attach = profiles["default-fly_app (attach)"]
    if flash_attach.get("preLaunchTask") != "FLY: Flash" or flash_attach["device"] != normal_attach["device"]:
        raise RuntimeError(f"{project.name} must flash the packaged BIN before attaching to the same device")
    if "preLaunchTask" in normal_attach or "preLaunchTask" in profiles["default-fly_app"]:
        raise RuntimeError(f"{project.name} must limit packaged BIN flashing to its optional debug profile")
    run(["git", "check-ignore", "--", ".vscode/launch.json"], project)


def check_project(sdk, workspace, template, build_type):
    project = workspace / template
    name = f"test-{template}"
    generate(sdk, project, name, FRDM_BOARD, template)
    git_root = run(["git", "rev-parse", "--show-toplevel"], project).strip()
    if Path(git_root).resolve() != project.resolve():
        raise RuntimeError(f"{project} was not initialized as its own Git repository")

    tasks = json.loads((project / ".vscode/tasks.json").read_text(encoding="utf-8"))["tasks"]
    if {task["label"]: task["args"] for task in tasks} != {
        "FLY: Setup": ["setup"], "FLY: Configure": ["vscode"], "FLY: Build": ["build"],
        "FLY: Make image": ["make_img"], "FLY: Flash": ["flash"],
    }:
        raise RuntimeError("New applications must include portable VS Code tasks")
    settings = json.loads((project / ".vscode/settings.json").read_text(encoding="utf-8"))
    if (settings["cmake.configureOnOpen"] is not False
            or settings["cmake.sourceDirectory"] != "${workspaceFolder}"
            or settings["cmake.buildDirectory"] != "${workspaceFolder}/build/cmake"):
        raise RuntimeError("New workspaces must configure from the application CMake entry point")
    if ((project / "setup_fly.py").exists()
            or not (project / "tool/python/setup_fly.py").is_file()):
        raise RuntimeError("New applications must store the SDK installer under tool/python")
    revision = commit(project, "Generated application")
    run(["git", "ls-files", "--error-unmatch", ".vscode/tasks.json"], project)
    if template == "baremetal":
        run(["cmake", "-S", str(project), "-B", str(project / "build/cmake"),
             f"-DCMAKE_BUILD_TYPE={build_type}", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"], project)
        if not (project / "build/config/autoconf.h").is_file():
            raise RuntimeError("Direct application configuration did not prepare the Kconfig header")
        run(entrypoint(project, "build", "vscode", build_type), project)
        if (project / "build/cmake/app/fly_app").exists():
            raise RuntimeError("The vscode command must prepare the workspace without compiling")
        check_vscode(sdk, project)
    run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
    cmake_cache = check_build_outputs(sdk, project)
    make_img = entrypoint(project, "build", "make_img")
    run(make_img, project)
    if not (project / "build/image/CURRENT/APP.bin").is_file():
        raise RuntimeError(f"{template} did not produce APP.bin in the current firmware package")

    run(["git", "check-ignore", "--", "build/"], project)
    check_vscode(sdk, project)
    check_image(project, name, "0.1.0", revision, build_type)
    if any(line.startswith("CMAKE_GDB:INTERNAL=") and not line.endswith("-NOTFOUND")
           for line in cmake_cache.splitlines()):
        check_debug_launch(project)
    if template == "baremetal":
        revision = commit(project, "Change application revision", allow_empty=True)
        run(make_img, project)
        check_image(project, name, "0.1.0", revision, build_type)
        cmake = project / "CMakeLists.txt"
        cmake.write_text(cmake.read_text(encoding="utf-8").replace(
            'set(FLY_FIRMWARE_VERSION "0.1.0")', 'set(FLY_FIRMWARE_VERSION "0.2.7")',
        ), encoding="utf-8")
        run(make_img, project)
        check_image(project, name, "0.2.7", revision, build_type)
    print(f"{template}: build and J-Link package PASS", flush=True)


def ci_boards(sdk):
    menu = (sdk / "arch/Kconfig").read_text(encoding="utf-8")
    required_arches = set()
    for line in menu.splitlines():
        pieces = line.partition("#")[0].split()
        if len(pieces) == 2 and pieces[0] == "config" and pieces[1].startswith("ARCH_"):
            required_arches.add(f"CONFIG_{pieces[1]}")
    required_arches -= RISCV_ARCHES

    boards = [json.loads(path.read_text(encoding="utf-8"))
              for path in sorted((sdk / "templates/boards").glob("*/board.json"))]
    boards = [board for board in boards if board["arch_config"] not in RISCV_ARCHES]
    baremetal_arches = {board["arch_config"] for board in boards if "baremetal" in board["templates"]}
    if baremetal_arches != required_arches:
        missing = ", ".join(sorted(required_arches - baremetal_arches)) or "none"
        unexpected = ", ".join(sorted(baremetal_arches - required_arches)) or "none"
        raise RuntimeError(f"Generated board coverage differs from arch/Kconfig. "
                           f"Missing: {missing}. Unexpected: {unexpected}.")
    return boards


def check_generated_config(sdk, workspace, board, template):
    name = f"{board['id']}-{template}"
    project = workspace / name
    generate(sdk, project, name, board["id"], template)
    output = run(entrypoint(project, "build", "loadconfig", "defconfig"), project)
    if any("warning:" in line.lower() for line in output.splitlines()):
        raise RuntimeError(f"{name} generated Kconfig warnings:\n{output}")

    config = (project / "build/config/.config").read_text(encoding="utf-8").splitlines()
    architecture = board["arch_config"]
    scheduler = "FREERTOS" if template == "freertos-nanosig" else "NONE"
    for assignment in (f"{architecture}=y", f"CONFIG_PACKAGE_SCHEDULER_{scheduler}=y"):
        if assignment not in config:
            raise RuntimeError(f"{name} did not select {assignment}")
    print(f"{name}: Kconfig PASS", flush=True)


def check_board_build(sdk, workspace, board, build_type, *, host=False):
    name = board["id"]
    project = workspace / name
    generate(sdk, project, name, name, "baremetal")
    run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
    check_build_outputs(sdk, project, host=host)
    print(f"{name}: {build_type} compile and link PASS", flush=True)


def check_frdm_templates(sdk, workspace, build_type):
    check_image_links(sdk, workspace)
    check_sdk_setup(sdk, workspace)
    check_native_sdk_pin(sdk, workspace)
    check_openocd_script(sdk, workspace)
    check_jlink_requires_binary(sdk, workspace)
    with ThreadPoolExecutor(max_workers=len(TEMPLATES)) as pool:
        futures = [pool.submit(check_project, sdk, workspace, template, build_type) for template in TEMPLATES]
        for future in futures:
            future.result()


def check_matrix(sdk, workspace, matrix, build_type):
    boards = ci_boards(sdk)
    if matrix == "config":
        for board in boards:
            for template in board["templates"]:
                check_generated_config(sdk, workspace, board, template)
    elif matrix == "arm":
        for board in boards:
            if board["arch_config"] not in HOST_ARCHES.values() and board["id"] != FRDM_BOARD:
                check_board_build(sdk, workspace, board, build_type)
        check_frdm_templates(sdk, workspace, build_type)
    else:
        host = HOST_ARCHES.get(sys.platform)
        if host is None:
            raise RuntimeError(f"Unsupported CI host: {sys.platform}")
        for board in boards:
            if board["arch_config"] == host:
                check_board_build(sdk, workspace, board, build_type, host=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    partial = parser.add_mutually_exclusive_group()
    partial.add_argument("--image-links-only", action="store_true",
                         help="Check native package directory aliases without a toolchain.")
    partial.add_argument("--sdk-setup-only", action="store_true",
                         help="Check the generator and application launchers without a toolchain.")
    partial.add_argument("--matrix", choices=("config", "arm", "host"),
                         help="Check configs, build all Cortex-M boards, or build the current native host.")
    parser.add_argument("--build-type", choices=("Debug", "Release"), default="Debug")
    args = parser.parse_args()
    sdk = Path(__file__).resolve().parents[2]
    sdk_outputs = [sdk / name for name in (".config", "auto-generate", "build", "dl", "image", ".vscode")]
    original = {path: path.stat().st_mtime_ns if path.exists() else None for path in sdk_outputs}
    with tempfile.TemporaryDirectory(prefix="fly standalone ") as directory:
        workspace = Path(directory)
        if args.image_links_only:
            check_image_links(sdk, workspace)
        elif args.sdk_setup_only:
            check_sdk_setup(sdk, workspace)
        elif args.matrix:
            check_matrix(sdk, workspace, args.matrix, args.build_type)
        else:
            check_frdm_templates(sdk, workspace, args.build_type)
    modified = [path.name for path, before in original.items()
                if (path.stat().st_mtime_ns if path.exists() else None) != before]
    if modified:
        raise RuntimeError(f"Standalone build modified SDK outputs: {', '.join(modified)}")


if __name__ == "__main__":
    main()
