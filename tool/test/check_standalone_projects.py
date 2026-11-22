#!/usr/bin/env python3
"""Validate FLY projects generated in temporary application workspaces."""

import argparse
from concurrent.futures import ThreadPoolExecutor
from contextlib import redirect_stderr
from io import StringIO
import json
import os
from pathlib import Path
import platform
import shutil
import struct
import subprocess
import sys
import tempfile
from types import SimpleNamespace
from unittest.mock import patch
from zipfile import ZipFile
from zlib import crc32

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "python"))

from arch_registry import discover as discover_architectures, resolve_board
from project_new import select_interactively, write_menu
from template_packs import discover as discover_packs, from_registry


FRDM_BOARD = "frdm-mcxn947-core0"
FRDM_NANOSIG_SMOKES = ("baremetal-nanosig", "freertos-nanosig")
FRDM_SMOKES = ("baremetal", *FRDM_NANOSIG_SMOKES)
FACTORY_DATA_ENABLED = "CONFIG_FACTORY_DATA_SUPPORTED=y"
FACTORY_DATA_DISABLED = "# CONFIG_FACTORY_DATA_SUPPORTED is not set"


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


def template_manifest(sdk, template):
    manifest = sdk / "templates/apps" / template / "pack.json"
    return json.loads(manifest.read_text(encoding="utf-8"))


def generate(sdk, project, name, board, template, *pack_options):
    run(entrypoint(sdk, "fly", "new", name, "--board", board, "--template", template,
                   "--output", str(project), *pack_options), sdk)
    cmake = (project / "CMakeLists.txt").read_text(encoding="utf-8")
    manifest = json.loads((project / "fly.project.json").read_text(encoding="utf-8"))
    if manifest["schema_version"] != 1 or manifest["packs"][:2] != [f"board/{board}", f"app/{template}"]:
        raise RuntimeError(f"{project.name} selected the wrong board or APP pack")
    if any(tool in cmake for tool in ("FLY_JLINK_DEVICE", "add_jlink_image", "add_openocd_image", "RENODE")):
        raise RuntimeError(f"{project.name} coupled the application skeleton to a specific tool")
    if "fly_add_firmware_image(" not in cmake or "fly_add_firmware_payload(" in cmake:
        raise RuntimeError(f"{project.name} must declare its firmware with fly_add_firmware_image()")
    for item in template_manifest(sdk, template)["files"]:
        if not (project / item["destination"]).is_file():
            raise RuntimeError(f"{project.name} is missing {item['destination']}")

    misplaced_headers = sorted(header.name for header in (project / "app").glob("*.h"))
    if misplaced_headers:
        raise RuntimeError(f"{project.name} has headers directly under app/: {misplaced_headers}")
    board_dir = project / "board"
    board_include = board_dir / "include"
    misplaced_headers = sorted(
        header.relative_to(board_dir).as_posix()
        for header in board_dir.rglob("*.h")
        if header.is_file() and board_include not in header.parents
    )
    if misplaced_headers:
        raise RuntimeError(f"{project.name} has headers outside board/include/: {misplaced_headers}")


def check_frdm_layout(project, template):
    app = project / "app"
    include = app / "include"
    for relative_path in ("mid/firmware_info.c", "include/mid/firmware_info.h"):
        if not (app / relative_path).is_file():
            raise RuntimeError(f"{project.name} is missing app/{relative_path}")
    for relative_path in (
        "app.c", "app_runtime.c", "app_demo.c", "firmware_info.c",
        "include/app.h", "include/app_runtime.h", "include/app_demo.h", "include/firmware_info.h",
    ):
        if (app / relative_path).exists():
            raise RuntimeError(f"{project.name} unexpectedly contains app/{relative_path}")

    needs_runtime = template in FRDM_NANOSIG_SMOKES
    for business_dir in (app / "biz", include / "biz"):
        relative_path = business_dir.relative_to(project).as_posix()
        if not business_dir.is_dir():
            raise RuntimeError(f"{project.name} is missing {relative_path}")
        if not needs_runtime and set(business_dir.iterdir()) != {business_dir / ".gitkeep"}:
            raise RuntimeError(f"{project.name} should only contain .gitkeep in {relative_path}")
    if not needs_runtime:
        for relative_path in ("runtime.c", "include/runtime.h"):
            if (app / relative_path).exists():
                raise RuntimeError(f"{project.name} unexpectedly contains app/{relative_path}")


def check_sdk_pin_mismatch(command, project, env, *, expected, actual, source=None):
    result = subprocess.run(command, cwd=project, env=env, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    details = ["differs from fly.lock", expected, actual, "setup"]
    if source:
        details.append(source)
    if result.returncode == 0 or any(detail not in result.stdout for detail in details):
        raise RuntimeError(f"SDK pin mismatch did not report the revisions and recovery:\n{result.stdout}")


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
    check_sdk_pin_mismatch(build, clone, dict(env, FLY_ROOT=str(origin)),
                           expected=pinned, actual=latest, source="FLY_ROOT")
    stored_path = clone / ".fly-root"
    stored_path.write_text(f"{origin}\n", encoding="utf-8")
    check_sdk_pin_mismatch(build, clone, env, expected=pinned, actual=latest, source=".fly-root")
    stored_path.unlink()
    if os.name != "nt":
        installed.symlink_to(origin, target_is_directory=True)
        check_sdk_pin_mismatch(build, clone, env, expected=pinned, actual=latest, source="fly")
        installed.unlink()
    run(["git", "clone", "-q", "--", str(origin), str(installed)], clone)
    check_sdk_pin_mismatch(build, clone, env, expected=pinned, actual=latest, source="fly")
    unmanaged = workspace / "unmanaged SDK"
    installed.rename(unmanaged)

    # The remote branch already points at latest, but setup and build follow fly.lock.
    run(setup, clone, env)
    if run(["git", "rev-parse", "HEAD"], installed).strip() != pinned:
        raise RuntimeError("Setup must install the locked SDK revision")
    run(setup, clone, env)
    output = run(build, clone, env)
    if "sdk-one --project-dir" not in output:
        raise RuntimeError(f"Build did not use the installed SDK:\n{output}")

    (clone / "fly.lock").write_text(json.dumps({"url": origin.as_uri(), "commit": latest}),
                                  encoding="utf-8")
    check_sdk_pin_mismatch(build, clone, env, expected=latest, actual=pinned)
    if run(["git", "rev-parse", "HEAD"], installed).strip() != pinned:
        raise RuntimeError("Build changed the installed SDK")
    run(setup, clone, env)
    output = run(build, clone, env)
    if "sdk-two --project-dir" not in output:
        raise RuntimeError(f"Build did not use the updated SDK revision:\n{output}")

    # A manual checkout can carry developer edits when its HEAD matches the lock.
    (unmanaged / "tool/python/build.py").write_text('import sys\nprint("sdk-local", *sys.argv[1:])\n',
                                                     encoding="utf-8")
    output = run(build, clone, dict(env, FLY_ROOT=str(unmanaged)))
    if "sdk-local --project-dir" not in output:
        raise RuntimeError(f"Build did not use the matching manual SDK checkout:\n{output}")
    print("SDK setup, local selection and pinned launcher validation: PASS", flush=True)


def check_native_sdk_pin(sdk, workspace):
    origin = workspace / "native SDK origin"
    origin.mkdir()
    run(["git", "init", "-q"], origin)
    for file in ("cmake/fly-configure.cmake", "tool/python/build.py"):
        destination = origin / file
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.touch()
    (origin / "cmake/fly-configure.cmake").write_text(
        'get_filename_component(sdk "${CMAKE_CURRENT_LIST_DIR}/.." REALPATH)\n'
        'set(FLY_RESOLVED_SDK_DIR "${sdk}" CACHE INTERNAL "Resolved fixture SDK")\n'
        'file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/sdk-loaded" "SDK configured\\n")\n',
        encoding="utf-8",
    )
    installed_pin = commit(origin, "SDK for native configure")
    new_pin = commit(origin, "New SDK revision", allow_empty=True)

    app = workspace / "native app"
    generate(sdk, app, "native-pin-check", FRDM_BOARD, "baremetal")
    (app / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_LIST_DIR}/tool/cmake/app-sdk.cmake")
include("${FLY_SDK_DIR}/cmake/fly-configure.cmake")
project(test_sdk_pin NONE)
add_custom_target(app_target)
app_add_sdk_check(app_target)
''', encoding="utf-8")
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
    command = ["cmake", "-S", str(app), f"-DPython3_EXECUTABLE:FILEPATH={sys.executable}"]

    env_a = dict(env, FLY_ROOT=str(local_sdk))
    env_b = dict(env, FLY_ROOT=str(origin))
    reused = app / "build/cmake"
    reconfigure = [*command, "-B", str(reused)]
    run(reconfigure, app, env_a)
    cache = reused / "CMakeCache.txt"
    cache_a = cache.read_text(encoding="utf-8").splitlines()
    resolved = next((line for line in cache_a if line.startswith("FLY_RESOLVED_SDK_DIR:")), "")
    configured = reused / "sdk-loaded"
    marker_a = configured.read_bytes()
    if (not resolved or Path(resolved.partition("=")[2]).resolve() != local_sdk.resolve()
            or any(line.startswith("FLY_SDK_DIR:") for line in cache_a)):
        raise RuntimeError("Native CMake did not distinguish a resolved SDK from an explicit override")
    build = ["cmake", "--build", str(reused), "--target", "app_target"]
    run(build, app, env_a)
    check_sdk_pin_mismatch(build, app, env_b, expected=installed_pin, actual=new_pin)
    check_sdk_pin_mismatch(reconfigure, app, env_b, expected=installed_pin, actual=new_pin)
    cache_after = cache.read_text(encoding="utf-8").splitlines()
    if (resolved not in cache_after or configured.read_bytes() != marker_a
            or any(line.startswith("FLY_SDK_DIR:") for line in cache_after)):
        raise RuntimeError("A rejected FLY_ROOT change altered the existing native configure state")

    check_sdk_pin_mismatch([*command, "-B", str(app / "build/explicit-cmake"), f"-DFLY_SDK_DIR:PATH={origin}"],
                           app, env_a, expected=installed_pin, actual=new_pin)
    lock.write_text(json.dumps({"url": origin.as_uri(), "commit": new_pin}), encoding="utf-8")
    check_sdk_pin_mismatch([*command, "-B", str(app / "build/managed-cmake")], app, env,
                           expected=new_pin, actual=installed_pin)
    if any((app / "build" / directory / "sdk-loaded").exists()
           for directory in ("explicit-cmake", "managed-cmake")):
        raise RuntimeError("Direct CMake loaded the SDK before validating its pin")
    print("Native CMake validates explicit, managed and reselected SDK pins: PASS", flush=True)


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


def current_package(project, build_type, target="default"):
    output = project / "image" / target / build_type
    pointer = json.loads((output / "CURRENT.json").read_text(encoding="utf-8"))
    package = output / pointer["artifact_id"]
    manifest = json.loads((package / "manifest.json").read_text(encoding="utf-8"))
    if manifest["artifact_id"] != pointer["artifact_id"]:
        raise RuntimeError("The current pointer does not match its firmware manifest")
    return package, manifest


def check_partition(package, archive, part, raw):
    binary = (package / part["bin"]["path"]).read_bytes()
    if not binary or archive.read(part["bin"]["path"]) != binary:
        raise RuntimeError(f"The archive does not contain the published {part['name']} bytes")
    trailer = part["trailer"]
    header = struct.Struct("<IIIIII12s32s64s")
    offset = trailer["offset"]
    checksum_end = offset + trailer["size"]
    crc_offset = offset + 16
    checksum_input = binary[:crc_offset] + b"\xff" * 4 + binary[crc_offset + 4:checksum_end]
    stored_crc = header.unpack_from(binary, offset)[4]
    if (binary[:len(raw)] != raw or stored_crc != int(trailer["crc32"], 16)
            or crc32(checksum_input) != stored_crc or len(binary) > part["capacity"]):
        raise RuntimeError(f"{part['name']} no longer matches the raw output, CRC, or capacity")


def check_image(project, name, version, revision, build_type):
    package, manifest = current_package(project, build_type)
    if (project / "build/image").exists() or manifest["firmware_name"] != name:
        raise RuntimeError("Packaging did not use the application's image directory and firmware name")
    if (project / "image/exports").exists():
        raise RuntimeError("Default packaging unexpectedly created a tool-specific export")
    if manifest["source"]["git_commit"] != revision:
        raise RuntimeError(f"Image did not use application HEAD {revision}")
    app, = manifest["partitions"]
    if app["name"] != "APP" or app["version"] != f"v{version}" or not app["symbols"]:
        raise RuntimeError("Application partition version or ELF is missing")
    with ZipFile(package.with_suffix(".zip")) as archive:
        check_partition(package, archive, app, (project / "build/cmake/fly_app.bin").read_bytes())
        for item in app["symbols"].values():
            if item and archive.read(item["path"]) != (package / item["path"]).read_bytes():
                raise RuntimeError("The archive does not contain the published symbols")


def check_bundle_image(project, build_type):
    boot = b"BOOT image built by CMake"
    (project / "boot fixture.bin").write_bytes(boot)
    cmake = project / "CMakeLists.txt"
    cmake.write_text(cmake.read_text(encoding="utf-8") + '''
set(boot_bin "${CMAKE_CURRENT_BINARY_DIR}/generated boot.bin")
set(TEST_BOOT_CAPACITY 176 CACHE STRING "Test bootloader partition capacity")
add_custom_command(OUTPUT "${boot_bin}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/boot fixture.bin" "${boot_bin}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/boot fixture.bin"
    VERBATIM
)
add_custom_target(test_bootloader DEPENDS "${boot_bin}")
fly_add_firmware_image(bundle
    IMAGE_NAME "${FLY_FIRMWARE_NAME}"
    BOARD_ID "${FLY_BOARD_ID}" CHIP "${FLY_BOARD_CHIP}" CORE_ID "${FLY_BOARD_CORE_ID}"
    FIRMWARE_LIST
        "APP:${FLY_FIRMWARE_VERSION}:${FLY_FLASH_ADDRESS}:${FLY_FLASH_SIZE}:fly_app.bin"
        "BOOT:1.2.3:0x00100000:${TEST_BOOT_CAPACITY}:${boot_bin}"
    SYMBOLS APP fly_app
    DEPENDS fly_app test_bootloader
)
''', encoding="utf-8")
    command = entrypoint(project, "build", "make_img", "bundle")
    run(command, project)
    package, manifest = current_package(project, build_type, "bundle")
    app, boot_part = manifest["partitions"]
    if (app["name"], app["address"], boot_part["name"], boot_part["address"], boot_part["symbols"]) != (
            "APP", "0x0", "BOOT", "0x100000", None):
        raise RuntimeError("The two partitions did not retain their starting addresses and symbol ownership")
    with ZipFile(package.with_suffix(".zip")) as archive:
        check_partition(package, archive, app, (project / "build/cmake/fly_app.bin").read_bytes())
        check_partition(package, archive, boot_part, boot)
    if boot_part["bin"]["size"] != 176:
        raise RuntimeError("BOOT did not fill its exact capacity")
    run(["cmake", "-S", str(project), "-B", str(project / "build/cmake"), "-DTEST_BOOT_CAPACITY=175"], project)
    failed = subprocess.run(command, cwd=project, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    if failed.returncode == 0 or "exceeds capacity 175 bytes" not in failed.stdout:
        raise RuntimeError(f"The BOOT capacity failure was not reported correctly:\n{failed.stdout}")
    if current_package(project, build_type, "bundle") != (package, manifest):
        raise RuntimeError("Capacity failure changed the previous complete package")
    print("Application-root APP and BOOT package and capacity checks: PASS", flush=True)


def check_image_after_distclean(project, build_type):
    package, manifest = current_package(project, build_type)
    firmware = (package / "APP.bin").read_bytes()
    run(entrypoint(project, "build", "distclean"), project)
    if (project / "build").exists():
        raise RuntimeError("distclean did not remove the application build directory")
    if current_package(project, build_type) != (package, manifest) or (package / "APP.bin").read_bytes() != firmware:
        raise RuntimeError("distclean changed the current firmware package")
    with ZipFile(package.with_suffix(".zip")) as archive:
        if archive.read("APP.bin") != firmware:
            raise RuntimeError("The archive did not retain the current firmware binary")
    print("Published firmware and symbols after distclean: PASS", flush=True)


def check_vscode(project):
    vscode = project / ".vscode"
    settings = json.loads((vscode / "settings.json").read_text(encoding="utf-8"))
    if settings["cmake.sourceDirectory"] != "${workspaceFolder}":
        raise RuntimeError("CMake Tools must configure the application entry point")
    if (settings["cmake.buildDirectory"] != "${workspaceFolder}/build/cmake"
            or "FLY_SDK_DIR" in settings["cmake.configureSettings"]):
        raise RuntimeError("CMake Tools must use the application build directory without forcing an SDK override")

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


def check_build_outputs(sdk, project, *, host=False, target="fly_app"):
    cmake_dir = project / "build/cmake"
    cache = (cmake_dir / "CMakeCache.txt").read_text(encoding="utf-8")
    cache_paths = {}
    for line in cache.splitlines():
        if line.startswith(("CMAKE_HOME_DIRECTORY:", "FLY_RESOLVED_SDK_DIR:")):
            key, _, value = line.partition("=")
            cache_paths[key.split(":", 1)[0]] = Path(value).resolve()
    if cache_paths.get("CMAKE_HOME_DIRECTORY") != project.resolve():
        raise RuntimeError(f"{project.name} did not configure from its own CMakeLists.txt")
    if cache_paths.get("FLY_RESOLVED_SDK_DIR") != sdk.resolve():
        raise RuntimeError(f"{project.name} did not use the current SDK checkout")

    executable_name = f"{target}.exe" if host and os.name == "nt" else target
    outputs = [
        project / "build/config/.config",
        project / "build/config/.config.cmake",
        project / "build/config/autoconf.h",
        cmake_dir / "compile_commands.json",
        cmake_dir / executable_name,
    ]
    if not host:
        outputs.append(cmake_dir / f"{target}.bin")
    for path in outputs:
        if not path.is_file() or not path.stat().st_size:
            raise RuntimeError(f"{project.name} did not produce {path.relative_to(project)}")
    return cache


def check_firmware_info_messages(project, *, enabled):
    binary = (project / "build/cmake/fly_app.bin").read_bytes()
    for marker in (b"CRC32 0x", b"Firmware info unavailable"):
        if (marker in binary) != enabled:
            expectation = "include" if enabled else "omit"
            raise RuntimeError(f"{project.name} must {expectation} the firmware log message {marker!r}")


def check_frdm_project(sdk, workspace, template, build_type):
    project = workspace / template
    name = f"test-{template}"
    tools = ["tool/jlink"]
    if template == "baremetal":
        tools.append("tool/renode")
    options = [argument for tool in tools for argument in ("--pack", tool)]
    generate(sdk, project, name, FRDM_BOARD, template, *options)
    selected = json.loads((project / "fly.project.json").read_text(encoding="utf-8"))["packs"]
    if selected[2:] != tools or not all((project / "tool/packs" / tool / "pack.json").is_file() for tool in tools):
        raise RuntimeError("New applications must install one manifest per selected tool")
    asset_names = {path.name for path in (sdk / "templates/tools/jlink/assets").rglob("*") if path.is_file()}
    if not asset_names:
        raise RuntimeError("The SDK J-Link tool pack is missing its bundled assets")
    copied_assets = sorted(path.relative_to(project).as_posix()
                           for path in project.rglob("*") if path.is_file() and path.name in asset_names)
    if copied_assets:
        raise RuntimeError(f"{project.name} copied SDK J-Link binaries: {copied_assets}")
    check_frdm_layout(project, template)
    git_root = run(["git", "rev-parse", "--show-toplevel"], project).strip()
    if Path(git_root).resolve() != project.resolve():
        raise RuntimeError(f"{project} was not initialized as its own Git repository")

    tasks = json.loads((project / ".vscode/tasks.json").read_text(encoding="utf-8"))["tasks"]
    task_commands = {task["label"]: task["args"] for task in tasks}
    base_tasks = {
        "FLY: Setup": ["setup"], "FLY: Configure": ["vscode"], "FLY: Build": ["build"],
        "FLY: Make image": ["make_img"],
    }
    providers = [tool.split("/")[1] for tool in tools]
    if (any(task_commands.get(name) != args for name, args in base_tasks.items())
            or any(["flash-run", "--via", provider] not in task_commands.values() for provider in providers)):
        raise RuntimeError("New applications must include portable tasks and their discovered flash-run tasks")
    listing = run(entrypoint(project, "build", "flash-run", "--list"), project)
    if any(f"flash-run default --via {provider}" not in listing for provider in providers):
        raise RuntimeError(f"flash-run did not list the selected tool providers:\n{listing}")
    if ("(default)" in listing) != (len(providers) == 1):
        raise RuntimeError(f"flash-run did not select a default only for a single tool provider:\n{listing}")
    if template == "baremetal":
        for removed in ("flash", "run"):
            rejected = subprocess.run(entrypoint(project, "build", removed, "--list"), cwd=project,
                                      stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=30)
            if rejected.returncode != 2 or "invalid choice" not in rejected.stdout:
                raise RuntimeError(f"The retired {removed} CLI command was accepted:\n{rejected.stdout}")
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
        if (project / "build/cmake/fly_app").exists():
            raise RuntimeError("The vscode command must prepare the workspace without compiling")
        check_vscode(project)
        settings_path = project / ".vscode/settings.json"
        previous = settings_path.read_text(encoding="utf-8")
        settings = json.loads(previous)
        settings["cmake.configureSettings"]["FLY_SDK_DIR"] = sdk.as_posix()
        settings_path.write_text(json.dumps(settings), encoding="utf-8")
        run(entrypoint(project, "build", "vscode", build_type), project)
        preserved = json.loads(settings_path.read_text(encoding="utf-8"))["cmake.configureSettings"]
        if preserved.get("FLY_SDK_DIR") != sdk.as_posix():
            raise RuntimeError("Refreshing VS Code discarded an explicit user SDK override")
        settings_path.write_text(previous, encoding="utf-8")
    run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
    check_build_outputs(sdk, project)
    check_firmware_info_messages(project, enabled=True)
    for expected_log in template_manifest(sdk, template)["app"]["expected_log"]:
        binary = (project / "build/cmake/fly_app.bin").read_bytes()
        message = expected_log.encode("utf-8")
        if all(message + ending not in binary for ending in (b"\0", b"\n\0")):
            raise RuntimeError(f"{project.name} did not link the expected log {expected_log!r}")
    make_img = entrypoint(project, "build", "make_img")
    run(make_img, project)

    for directory in ("build/", "image/"):
        run(["git", "check-ignore", "--", directory], project)
    check_vscode(project)
    check_image(project, name, "0.1.0", revision, build_type)
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

        config = (project / "build/config/.config").read_text(encoding="utf-8")
        opt_out = project / "build/no-factory-data.config"
        opt_out.write_text(config.replace(FACTORY_DATA_ENABLED, FACTORY_DATA_DISABLED), encoding="utf-8")
        run(entrypoint(project, "build", "loadconfig", str(opt_out)), project)
        run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
        check_firmware_info_messages(project, enabled=False)
        print("baremetal: factory-data opt-out PASS", flush=True)
        check_bundle_image(project, build_type)
        check_image_after_distclean(project, build_type)
    print(f"{template}: build and common firmware package PASS", flush=True)


def check_generated_config(sdk, workspace, board, template):
    name = f"{board['id']}-{template}"
    project = workspace / name
    generate(sdk, project, name, board["id"], template)
    selected = json.loads((project / "fly.project.json").read_text(encoding="utf-8"))
    if selected["packs"] != [f"board/{board['id']}", f"app/{template}"]:
        raise RuntimeError(f"{name} got tool packs without selecting them")
    tasks = json.loads((project / ".vscode/tasks.json").read_text(encoding="utf-8"))["tasks"]
    if any(task["args"][:1] == ["flash-run"] for task in tasks):
        raise RuntimeError(f"{name} advertised a flash-run task without a provider")
    output = run(entrypoint(project, "build", "loadconfig", "defconfig"), project)
    if any("warning:" in line.lower() for line in output.splitlines()):
        raise RuntimeError(f"{name} generated Kconfig warnings:\n{output}")

    config = (project / "build/config/.config").read_text(encoding="utf-8").splitlines()
    architecture = board["architecture"]
    expected = (project / "defconfig").read_text(encoding="utf-8").splitlines() + [
        f"{architecture.config}=y", f'CONFIG_ARCH="{architecture.id}"',
        f'CONFIG_ARCH_PROFILE="{architecture.profile}"', f'CONFIG_ARCH_CORE="{architecture.cpu}"',
        f'CONFIG_ARCH_ISA="{architecture.isa}"', f'CONFIG_ARCH_TOOLCHAIN="{architecture.toolchain}"',
    ]
    for assignment in expected:
        if assignment not in config:
            raise RuntimeError(f"{name} is missing Kconfig line {assignment!r}")
    print(f"{name}: Kconfig PASS", flush=True)


def check_registered_template(sdk, workspace, *, build_type=None):
    fixture = workspace / "SDK with an added template"
    fixture.mkdir()
    for launcher in ("fly.sh", "fly.bat"):
        shutil.copy2(sdk / launcher, fixture / launcher)
    for directory in ("templates", "tool/python", "cmake"):
        shutil.copytree(sdk / directory, fixture / directory, ignore=shutil.ignore_patterns("__pycache__"))
    arch_dir = fixture / "arch/experimental/fixture"
    arch_dir.mkdir(parents=True)
    for name in ("CMakeLists.txt", "Kconfig"):
        shutil.copy2(sdk / "arch" / name, fixture / "arch" / name)
    for native in discover_architectures(sdk / "arch").values():
        source = sdk / "arch" / native.relative_path
        target = fixture / "arch" / native.relative_path
        target.mkdir(parents=True)
        for name in ("arch.json", "Kconfig", "CMakeLists.txt", "gcc-config.cmake"):
            if (source / name).is_file():
                shutil.copy2(source / name, target / name)
    # This architecture is present solely through its own manifest.
    declaration = {"schema_version": 1, "id": "fixture-arch", "config": "CONFIG_ARCH_FIXTURE",
                   "isa": "riscv", "toolchain": "gcc-riscv-none-embed", "cpu": "fixture-core",
                   "march": "rv32imac", "abi": "ilp32"}
    toolchain_config = "# Fixture architecture has no additional flags\n"
    if build_type:
        native = next(arch for arch in discover_architectures(sdk / "arch").values()
                      if arch.host_system == platform.system())
        declaration = json.loads((native.directory / "arch.json").read_text(encoding="utf-8"))
        declaration.update(id="fixture-arch", config="CONFIG_ARCH_FIXTURE")
    (arch_dir / "arch.json").write_text(json.dumps(declaration), encoding="utf-8")
    (arch_dir / "gcc-config.cmake").write_text(toolchain_config, encoding="utf-8")
    (arch_dir / "CMakeLists.txt").write_text("add_library(fixture_arch INTERFACE)\n", encoding="utf-8")
    (arch_dir / "Kconfig").write_text("config ARCH_FIXTURE_CAPABILITY\n    bool\n    default y\n", encoding="utf-8")
    (fixture / "package").mkdir()
    (fixture / "package/Kconfig").write_text("config FIXTURE_NUMBER\n    int \"Fixture number\"\n",
                                            encoding="utf-8")
    (fixture / "package/CMakeLists.txt").write_text('include("${FLY_TOP_DIR}/cmake/package.cmake")\n',
                                                   encoding="utf-8")
    def pack(kind, name, files, *, source_dir=None, **metadata):
        if source_dir is None:
            root = {"app": "apps", "board": "boards", "tool": "tools"}[kind]
            source_dir = f"{root}/{name}"
        source = fixture / "templates" / source_dir
        source.mkdir(parents=True)
        for filename, (destination, content) in files.items():
            path = source / filename
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
        manifest = {
            "schema_version": 1, "id": f"{kind}/{name}", "kind": kind, "label": name,
            "files": [{"source": filename, "destination": destination}
                      for filename, (destination, _) in files.items()], **metadata,
        }
        path = source / "pack.json"
        path.write_text(json.dumps(manifest), encoding="utf-8")
        return path, manifest

    support_path, support = pack("support", "fixture-board-shared", {
        "init.c": ("board/init.c", "/* shared init */\n"),
        "include/fixture.h": ("board/include/fixture.h", "/* shared header */\n"),
    }, source_dir="boards/shared/custom/family")
    board_path, board = pack("board", "fixture-board", {
        "board.cmake": ("board/board.cmake",
                        ("set(FLY_BOARD_HOST TRUE)\n" if build_type else "") +
                        "function(fly_configure_board target)\n"
                        "    if(NOT CONFIG_ARCH_FIXTURE_CAPABILITY OR NOT TARGET fixture_arch)\n"
                        '        message(FATAL_ERROR "The discovered architecture was not loaded")\n'
                        "    endif()\n"
                        "    target_link_libraries(${target} fixture_arch)\n"
                        "endfunction()\n"),
    }, cmake="board/board.cmake", dependencies=["support/fixture-board-shared"], board={
        "target_kind": "host" if build_type else "mcu", "chip": None if build_type else "Fixture",
        "core": {"id": "native" if build_type else "core0", "architecture": "fixture-core"},
        "arch": "fixture-arch", "toolchain": "fixture-gcc", "clock_hz": 6100,
        "memories": [] if build_type else [{"id": "flash", "kind": "flash", "address": 0, "size": 256}],
    })
    fixture_arch = resolve_board(discover_architectures(fixture / "arch"), board["id"], board["board"])
    if in_build_matrix(fixture_arch, "arm") or in_build_matrix(fixture_arch, "host") != bool(build_type):
        raise RuntimeError("Build matrix picked the wrong fixture architecture")
    pack("app", "fixture-template", {
        "main.c": ("app/main.c", "int main(void) { return 0; }\n"),
        "biz/fixture.c": ("app/biz/fixture.c", "/* fixture business */\n"),
    }, app={
        "firmware_target": "fixture_firmware", "expected_log": ["fixture says hello"],
        "config_defaults": ["CONFIG_FIXTURE_NUMBER=@FIXTURE_NUMBER@"],
        "board_bindings": {"FIXTURE_NUMBER": {"field": "clock_hz", "type": "positive_integer"}},
    })
    flash_id, companion_id = "tool/fixture-flash", "tool/fixture-with-companion"
    flash_handler = f"tool/packs/{flash_id}/fixture.py"
    pack("tool", "fixture-flash", {"fixture.py": (flash_handler, "# fixture\n")},
         provides=["test/fixture-transport"],
         actions=[{"name": "flash-run", "provider": "fixture-flash", "help": "Fixture flash-run",
                   "handler": flash_handler, "inputs": ["artifact"]}])
    handler = f"tool/packs/{companion_id}/fixture.py"
    pack("tool", "fixture-with-companion", {"fixture.py": (handler, "# fixture\n")},
         source_dir="experiment/nested/companion",
         requires=["test/fixture-transport"], actions=[
             {"name": "flash-run", "provider": "fixture-with-companion", "help": "Fixture alternative flash-run",
              "handler": handler, "inputs": ["artifact"]},
             {"name": "log", "provider": "fixture-with-companion", "help": "Fixture log",
              "handler": handler, "inputs": ["none"]},
         ])
    pack("tool", "fixture-impossible", {}, requires=["test/no-provider"])
    pack("board", "fixture-unavailable", {}, board=board["board"], requires=["test/no-provider"])
    catalogue = run(entrypoint(fixture, "fly", "packs", "--board", "fixture-board",
                               "--template", "fixture-template"), fixture)
    if any(name not in catalogue for name in (flash_id, "test/fixture-transport", companion_id)):
        raise RuntimeError(f"The catalogue did not discover the compatible packs and capabilities:\n{catalogue}")
    if "tool/fixture-impossible" in catalogue:
        raise RuntimeError("The catalogue advertised an impossible tool")
    from kconfiglib import Kconfig

    menu = workspace / "discovery-menu.Kconfig"
    registry = discover_packs(fixture / "templates")
    scaffold = fixture / "templates/scaffold"
    symbols, defaults = write_menu(menu, registry, scaffold)
    fixture_provider = next(symbol for symbol, action_provider in defaults.items()
                            if action_provider == ("flash-run", "fixture-flash"))
    default_path = workspace.resolve() / "suggested $(projects)" / "fixture-app"
    with patch.dict(os.environ, {"FLY_NEW_INITIAL_PATH": str(default_path)}):
        kconf = Kconfig(str(menu))
    kconf.syms[symbols["board/fixture-board"]].set_value(2)
    kconf.syms[symbols["app/fixture-template"]].set_value(2)
    for pack_id, visible in ((flash_id, True), (companion_id, True),
                             ("tool/fixture-impossible", False), ("board/fixture-unavailable", False)):
        if bool(kconf.syms[symbols[pack_id]].visibility) != visible:
            raise RuntimeError(f"The generation menu advertised incorrect availability for {pack_id}")
    default_provider = kconf.syms[fixture_provider]
    flash_choice = default_provider.choice
    if flash_choice.visibility:
        raise RuntimeError("The menu showed a default choice with zero selected providers")
    flash_pack = kconf.syms[symbols[flash_id]]
    companion_pack = kconf.syms[symbols[companion_id]]
    if flash_pack.nodes[0].parent is not companion_pack.nodes[0].parent:
        raise RuntimeError("The generation menu put the tools in separate groups")
    flash_pack.set_value(2)
    if flash_choice.visibility:
        raise RuntimeError("The menu showed a default choice with one selected provider")
    companion_pack.set_value(2)
    if not flash_choice.visibility or flash_choice.selection.nodes[0].prompt[0] != "Automatic":
        raise RuntimeError("The menu did not show an Automatic default with two selected providers")
    for symbol, (action, _) in defaults.items():
        if action == "log" and kconf.syms[symbol].choice.visibility:
            raise RuntimeError("The menu showed a default choice for an unrelated single-provider action")
    saved_versions = []
    custom_path = Path("custom projects") / "fixture-revised"
    expected_path = (Path.cwd() / custom_path).parent.resolve() / custom_path.name

    def save_menu(config):
        saved = Path(os.environ["KCONFIG_CONFIG"])
        if saved_versions:
            config.load_config(str(saved))
            config.syms[symbols[flash_id]].set_value(2)
            config.syms[fixture_provider].set_value(2)
        else:
            project_path = config.syms["NEW_PATH"]
            if project_path.nodes[0].prompt[0] != "Project path" or project_path.str_value != str(default_path):
                raise RuntimeError("The menu did not show the supplied project path as its default")
            project_path.set_value(str(custom_path))
            config.syms[symbols["board/fixture-board"]].set_value(2)
            config.syms[symbols["app/fixture-template"]].set_value(2)
            config.syms[symbols[companion_id]].set_value(2)
        config.write_config(str(saved))
        saved_versions.append(saved)

    diagnostic = StringIO()
    mock_menu = SimpleNamespace(menuconfig=save_menu)
    with patch.dict(sys.modules, {"menuconfig": mock_menu}), patch("builtins.input", return_value=""):
        with redirect_stderr(diagnostic):
            selection = select_interactively(registry, scaffold, default_path=default_path)
    expected_packs = {"board/fixture-board", "app/fixture-template", flash_id, companion_id}
    if (len(saved_versions) != 2 or saved_versions[0] != saved_versions[1]
            or "test/fixture-transport" not in diagnostic.getvalue()
            or selection is None or selection[0] != expected_path or set(selection[1]) != expected_packs
            or selection[2] != {"flash-run": "fixture-flash"}):
        raise RuntimeError("The menu did not preserve an incomplete selection while letting the user fix it")

    def save_with_one_provider(config):
        for pack_id in expected_packs:
            config.syms[symbols[pack_id]].set_value(2)
        config.syms[fixture_provider].set_value(2)
        config.syms[symbols[companion_id]].set_value(0)
        config.write_config(os.environ["KCONFIG_CONFIG"])

    with patch.dict(sys.modules, {"menuconfig": SimpleNamespace(menuconfig=save_with_one_provider)}):
        selection = select_interactively(registry, scaffold, default_path=default_path)
    if selection is None or selection[2]:
        raise RuntimeError("The menu persisted an explicit default after its choice was hidden")
    if selection[0] != default_path:
        raise RuntimeError("The menu did not preserve the suggested destination when accepting it unchanged")
    project = workspace / "fixture-app"
    generate(fixture, project, "fixture-app", "fixture-board", "fixture-template", "--pack", flash_id,
             "--default", "flash-run=fixture-flash")
    if (project / "board/init.c").read_text(encoding="utf-8") != "/* shared init */\n":
        raise RuntimeError("The board file dependency was not copied")
    config = (project / "defconfig").read_text(encoding="utf-8").splitlines()
    if config != ["CONFIG_FIXTURE_NUMBER=6100"]:
        raise RuntimeError("The generator did not render the pack's board binding")
    selected = json.loads((project / "fly.project.json").read_text(encoding="utf-8"))
    if selected["defaults"] != {"flash-run": "fixture-flash"} or flash_id not in selected["packs"]:
        raise RuntimeError("The arbitrary tool pack and default were not recorded")
    catalogue = json.loads(run(entrypoint(project, "build", "capabilities", "--json"), project))
    if ("test/fixture-transport" not in catalogue["capabilities"]
            or ("flash-run", "fixture-flash") not in {(item["name"], item["provider"]) for item in catalogue["actions"]}):
        raise RuntimeError("The project capability report did not discover the arbitrary pack")
    tasks = json.loads((project / ".vscode/tasks.json").read_text(encoding="utf-8"))["tasks"]
    if not any(task["args"] == ["flash-run", "--via", "fixture-flash"] for task in tasks):
        raise RuntimeError("VS Code did not discover the installed anonymous provider")
    if "fixture says hello" not in (project / "README.md").read_text(encoding="utf-8"):
        raise RuntimeError("The generated README did not use the manifest's expected log")
    run(entrypoint(project, "build", "loadconfig", "defconfig"), project)
    config_path = project / "build/config/.config"
    config = config_path.read_text(encoding="utf-8")
    if not all(line in config for line in (
            'CONFIG_ARCH="fixture-arch"', "CONFIG_ARCH_FIXTURE=y", "CONFIG_ARCH_FIXTURE_CAPABILITY=y")):
        raise RuntimeError(f"The discovered architecture was not activated:\n{config}")
    if build_type:
        run(entrypoint(project, "build", "build", build_type), project)
    moved_arch = fixture / "arch/relocated/fixture"
    moved_arch.parent.mkdir()
    arch_dir.rename(moved_arch)
    run(entrypoint(project, "build", "loadconfig", str(config_path)), project)
    config_cmake = (project / "build/config/.config.cmake").read_text(encoding="utf-8")
    if config_path.read_text(encoding="utf-8") != config or 'FLY_ARCH_RELATIVE_DIR "relocated/fixture"' not in config_cmake:
        raise RuntimeError("Moving an architecture directory changed its saved configuration or failed to update CMake")
    if build_type:
        run(entrypoint(project, "build", "build", build_type), project)
        check_build_outputs(fixture, project, host=True, target="fixture_firmware")
        print("New architecture and board discovered, configured and built: PASS", flush=True)

    rejected = workspace / "invalid-board-app"
    command = entrypoint(fixture, "fly", "new", rejected.name, "--board", "fixture-board",
                         "--template", "fixture-template", "--output", str(rejected))
    def reject(path, updates, diagnostic):
        path.write_text(json.dumps(updates), encoding="utf-8")
        result = subprocess.run(command, cwd=fixture, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True, timeout=30)
        if result.returncode == 0 or rejected.exists() or diagnostic not in result.stdout:
            raise RuntimeError(f"Invalid pack did not report {diagnostic!r} cleanly:\n{result.stdout}")
    reject(board_path, {**board, "board": {**board["board"], "arch": "unknown-architecture"}}, "unknown-architecture")
    reject(board_path, {**board, "board": {**board["board"], "arch_profile": "missing-profile"}}, "missing-profile")
    board_path.write_text(json.dumps(board), encoding="utf-8")
    wrong_config = project / "wrong.config"
    wrong_config.write_text("CONFIG_ARCH_MCXN947=y\n", encoding="utf-8")
    result = subprocess.run(entrypoint(project, "build", "loadconfig", "wrong.config"), cwd=project,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=30)
    if result.returncode == 0 or "CONFIG_ARCH_MCXN947" not in result.stdout:
        raise RuntimeError(f"Incompatible architecture configuration was accepted:\n{result.stdout}")
    reject(board_path, {**board, "dependencies": ["support/absent"]}, "Unknown pack")
    board_path.write_text(json.dumps(board), encoding="utf-8")
    reject(support_path, {**support, "dependencies": ["board/fixture-board"]}, "Dependency cycle")
    reject(support_path, {**support, "files": [{"source": "../outside", "destination": "board/init.c"}]},
           "Invalid file source")
    reject(support_path, {**support, "files": [{"source": "init.c", "destination": "CMakeLists.txt"}]},
           "Destination would overwrite")
    support_path.write_text(json.dumps(support), encoding="utf-8")
    duplicate_path = fixture / "templates" / "duplicate" / "pack.json"
    duplicate_path.parent.mkdir()
    reject(duplicate_path, support, "Duplicate pack ID")
    reject(duplicate_path, {**support, "id": "support/con"}, "Pack ID must use a portable path")
    print("Arbitrary pack registration, shared files, board binding and invalid combinations: PASS", flush=True)


def check_gd32f10x_variants(project):
    cmake_dir = project / "build/cmake"
    commands = json.loads((cmake_dir / "compile_commands.json").read_text(encoding="utf-8"))
    startup_commands = {Path(entry["file"]).name: entry["command"] for entry in commands}
    variants = ("md", "hd", "xd", "cl")
    for variant in variants:
        startup = f"startup_gd32f10x_{variant}.S"
        if f"-DGD32F10X_{variant.upper()}" not in startup_commands.get(startup, ""):
            raise RuntimeError(f"{startup} does not build with its matching GD32F10x device macro")
    run(["cmake", "--build", str(cmake_dir), "--parallel", "2", "--target",
         *(f"arch-gd32f10x-{variant}" for variant in variants)], project)
    print("GD32F10x MD, HD, XD and CL device targets: PASS", flush=True)


def check_h750_build(project, build_type, cache):
    config_path = project / "build/config/.config"
    config = config_path.read_text(encoding="utf-8")
    required = {'CONFIG_ARCH_CORE="cortex-m7"', 'CONFIG_ARCH_PROFILE="cm7"',
                'CONFIG_ARCH_FPU="fpv5-d16"', 'CONFIG_ARCH_FLOAT_ABI="hard"'}
    if not required.issubset(config.splitlines()) or "CONFIG_PACKAGE_SCHEDULER_FREERTOS=y" in config:
        raise RuntimeError("H750 did not select the Cortex-M7 hard-float profile with a supported scheduler")
    requested = project / "build/unsupported-scheduler.config"
    requested.write_text(config.replace("CONFIG_PACKAGE_SCHEDULER_NONE=y", "CONFIG_PACKAGE_SCHEDULER_FREERTOS=y"),
                         encoding="utf-8")
    rejected = subprocess.run(entrypoint(project, "build", "loadconfig", str(requested)), cwd=project,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, timeout=30)
    if rejected.returncode == 0 or "CONFIG_PACKAGE_SCHEDULER_FREERTOS=y is unavailable" not in rejected.stdout:
        raise RuntimeError(f"Explicit H750 FreeRTOS choice was not rejected:\n{rejected.stdout}")
    if config_path.read_text(encoding="utf-8") != config:
        raise RuntimeError("Rejected scheduler request replaced the current H750 config")
    cmake_dir = project / "build/cmake"
    commands = json.loads((cmake_dir / "compile_commands.json").read_text(encoding="utf-8"))
    flags = ("-mcpu=cortex-m7", "-mfpu=fpv5-d16", "-mfloat-abi=hard")
    if not commands or any(not all(flag in command["command"] for flag in flags)
                           or "-mcpu=cortex-m4" in command["command"] for command in commands):
        raise RuntimeError("H750 compile commands differ from the selected CPU profile")
    readelf = next((line.split("=", 1)[1] for line in cache.splitlines()
                    if line.startswith("CMAKE_READELF:")), "")
    if not Path(readelf).is_file():
        raise RuntimeError("CMake did not resolve readelf for the selected H750 toolchain")
    attributes = run([readelf, "-A", str(cmake_dir / "fly_app")], project)
    if "Tag_FP_arch: FPv5/FP-D16" not in attributes or "Tag_ABI_VFP_args: VFP registers" not in attributes:
        raise RuntimeError(f"H750 ELF does not declare FPv5-D16 with the hard-float calling convention:\n{attributes}")
    run(entrypoint(project, "build", "make_img"), project)
    package, manifest = current_package(project, build_type)
    app, = manifest["partitions"]
    with ZipFile(package.with_suffix(".zip")) as archive:
        check_partition(package, archive, app, (cmake_dir / "fly_app.bin").read_bytes())
    print("STM32H750 M7 compiler flags, ELF attributes and package: PASS", flush=True)


def check_board_build(sdk, workspace, board, template, build_type):
    name = f"{board['id']}-{template}"
    project = workspace / name
    generate(sdk, project, name, board["id"], template)
    run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
    cache = check_build_outputs(sdk, project, host=board["target_kind"] == "host")
    if board["id"] == "gd32f103rc":
        check_gd32f10x_variants(project)
    if board["id"] == "stm32h750vb":
        check_h750_build(project, build_type, cache)
    print(f"{name}: {build_type} compile and link PASS", flush=True)


def check_renode_project(sdk, workspace, board, template, build_type, renode):
    name = board
    project = workspace / f"{board}-{template}"
    generate(sdk, project, name, board, template, "--pack", "tool/renode")
    run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
    run(entrypoint(project, "build", "make_img"), project)
    package, manifest = current_package(project, build_type)
    app, = manifest["partitions"]
    with ZipFile(package.with_suffix(".zip")) as archive:
        check_partition(package, archive, app, (project / "build/cmake/fly_app.bin").read_bytes())

    duration = 3200
    options = ["--tool", f"renode={renode.resolve()}"] if renode else []
    output = run(entrypoint(project, "build", "flash-run", "--via", "renode",
                            "--duration-ms", str(duration), *options), project)
    logs = project / "image/logs/renode/default" / build_type / manifest["artifact_id"]
    record_path, = logs.glob("*/record.json")
    record = json.loads(record_path.read_text(encoding="utf-8"))
    rtt = record_path.with_name(record["rtt_log"]).read_text(encoding="utf-8")
    expected_app = {"name": app["name"], "address": app["address"], "sha256": app["bin"]["sha256"]}
    boot_markers = [f"Firmware {name} v0.1.0", f"CRC32 0x{int(app['trailer']['crc32'], 16):08X} (OK)",
                    *template_manifest(sdk, template)["app"]["expected_log"]]
    if (record["status"] != "ok" or record["virtual_ms"] != duration
            or record["partitions"] != [expected_app]
            or not all(marker in rtt and marker in output for marker in boot_markers)):
        raise RuntimeError(f"{project.name} did not report a verified boot in Renode:\n{output}")
    print(f"{project.name}: Renode boot and {boot_markers[1]} PASS", flush=True)


def check_renode_matrix(sdk, workspace, build_type, renode):
    registry = discover_packs(sdk / "templates")
    models = registry["tool/renode"].data["models"]
    cases = [(model["board"].split("/", 1)[1], app.split("/", 1)[1])
             for model in models for app in model["apps"]]
    if not cases:
        raise RuntimeError("No Renode board and APP combinations declared")
    errors = []
    with ThreadPoolExecutor(max_workers=4) as pool:
        futures = {pool.submit(check_renode_project, sdk, workspace, board, app, build_type, renode): (board, app)
                   for board, app in cases}
        for future, case in futures.items():
            try:
                future.result()
            except (OSError, RuntimeError, ValueError, subprocess.SubprocessError) as error:
                errors.append(f"{' / '.join(case)}: {error}")
    if errors:
        raise RuntimeError("Renode checks failed:\n" + "\n".join(errors))


def check_frdm_templates(sdk, workspace, build_type):
    check_image_links(sdk, workspace)
    check_sdk_setup(sdk, workspace)
    check_native_sdk_pin(sdk, workspace)
    with ThreadPoolExecutor(max_workers=len(FRDM_SMOKES)) as pool:
        futures = [pool.submit(check_frdm_project, sdk, workspace, template, build_type) for template in FRDM_SMOKES]
        for future in futures:
            future.result()
    return {(FRDM_BOARD, template) for template in FRDM_SMOKES}


def in_build_matrix(architecture, matrix):
    if matrix == "arm":
        return architecture.isa == "arm"
    return architecture.host_system == platform.system()


def check_matrix(sdk, workspace, matrix, build_type):
    registry = discover_packs(sdk / "templates")
    architectures = discover_architectures(sdk / "arch")
    boards = []
    for pack in (pack for pack in registry.values() if pack.kind == "board"):
        profiles = []
        for app in (app for app in registry.values() if app.kind == "app"):
            try:
                from_registry(registry, [pack.id, app.id], {})
            except ValueError:
                continue
            profiles.append(app.id.split("/", 1)[1])
        board = pack.data["board"]
        architecture = resolve_board(architectures, pack.id, board)
        boards.append(dict(board, id=pack.id.split("/", 1)[1], templates=profiles, architecture=architecture))
    if not boards:
        raise RuntimeError("No board manifests found")
    if matrix == "config":
        for board in boards:
            for template in board["templates"]:
                check_generated_config(sdk, workspace, board, template)
        check_registered_template(sdk, workspace)
        return

    selected = [board for board in boards if in_build_matrix(board["architecture"], matrix)]
    if not selected:
        raise RuntimeError(f"No boards registered for the {matrix} matrix on {sys.platform}")
    smoke_coverage = check_frdm_templates(sdk, workspace, build_type) if matrix == "arm" else set()
    for board in selected:
        for template in board["templates"]:
            if (board["id"], template) not in smoke_coverage:
                check_board_build(sdk, workspace, board, template, build_type)
    if matrix == "host":
        check_registered_template(sdk, workspace, build_type=build_type)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    partial = parser.add_mutually_exclusive_group()
    partial.add_argument("--image-links-only", action="store_true",
                         help="Check native package directory aliases without a toolchain.")
    partial.add_argument("--sdk-setup-only", action="store_true",
                         help="Check the generator and application launchers without a toolchain.")
    partial.add_argument("--matrix", choices=("config", "arm", "host", "renode"),
                         help="Check configs, build Cortex-M or native host applications, or boot in Renode.")
    parser.add_argument("--build-type", choices=("Debug", "Release"), default="Debug")
    parser.add_argument("--renode", type=Path, help="Path to the Renode executable for the Renode matrix.")
    args = parser.parse_args()
    sdk = Path(__file__).resolve().parents[2]
    sdk_outputs = [sdk / name for name in (".config", "auto-generate", "build", "image", ".vscode")]
    original = {path: path.stat().st_mtime_ns if path.exists() else None for path in sdk_outputs}
    with tempfile.TemporaryDirectory(prefix="fly standalone ") as directory:
        workspace = Path(directory)
        if args.image_links_only:
            check_image_links(sdk, workspace)
        elif args.sdk_setup_only:
            check_sdk_setup(sdk, workspace)
        elif args.matrix == "renode":
            check_renode_matrix(sdk, workspace, args.build_type, args.renode)
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
