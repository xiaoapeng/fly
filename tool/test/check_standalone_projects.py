#!/usr/bin/env python3
"""Validate FLY projects generated in temporary application workspaces."""

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import re
import shutil
import signal
import struct
import subprocess
import sys
import tempfile
from zipfile import ZipFile
from zlib import crc32


FRDM_BOARD = "frdm-mcxn947-core0"
FRDM_NANOSIG_SMOKES = ("baremetal-nanosig", "freertos-nanosig")
FRDM_SMOKES = ("baremetal", *FRDM_NANOSIG_SMOKES)
FACTORY_DATA_ENABLED = "CONFIG_FACTORY_DATA_SUPPORTED=y"
FACTORY_DATA_DISABLED = "# CONFIG_FACTORY_DATA_SUPPORTED is not set"
OPENOCD_TEST_RTT = ("0x10002000", "0x1800")


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
    manifest = sdk / "templates/apps" / template / "template.json"
    return json.loads(manifest.read_text(encoding="utf-8"))


def generate(sdk, project, name, board, template):
    run(entrypoint(sdk, "fly", "new", name, "--board", board,
                   "--template", template, "--output", str(project)), sdk)
    cmake = (project / "CMakeLists.txt").read_text(encoding="utf-8")
    tool_settings = ("FLY_JLINK_DEVICE", "CMAKE_GDB", "CMAKE_OBJDUMP")
    if (any(setting in cmake for setting in tool_settings)
            or re.search(r"\b(?:add_jlink_image|add_vscode_cortex_debug_gdb)\s*\(", cmake)):
        raise RuntimeError(f"{project.name} must configure image and debug targets through its wrappers")
    for filename, call, settings in (
        ("app-image.cmake", "app_add_jlink_image(", ("FLY_JLINK_DEVICE",)),
        ("app-vscode.cmake", "app_add_vscode_debug(fly_app)", tool_settings),
    ):
        relative_path = f"tool/cmake/{filename}"
        wrapper = project / relative_path
        if not wrapper.is_file() or relative_path not in cmake or call not in cmake:
            raise RuntimeError(f"{project.name} is missing its {relative_path} wrapper or call")
        wrapper_cmake = wrapper.read_text(encoding="utf-8")
        if any(setting not in wrapper_cmake for setting in settings):
            raise RuntimeError(f"{project.name} must check availability in {relative_path}")
    for relative_path in template_manifest(sdk, template).get("required_files", ()):
        if not (project / relative_path).is_file():
            raise RuntimeError(f"{project.name} is missing {relative_path}")

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
    board_source = sdk / "templates/boards" / board
    for source_header in (board_source / "include").rglob("*.h"):
        relative_path = source_header.relative_to(board_source)
        if not (board_dir / relative_path).is_file():
            raise RuntimeError(f"{project.name} is missing board/{relative_path.as_posix()}")


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
    for relative_path in ("runtime.c", "include/runtime.h", "biz/app_demo.c", "include/biz/app_demo.h"):
        source = app / relative_path
        if needs_runtime and not source.is_file():
            raise RuntimeError(f"{project.name} is missing app/{relative_path}")
        if not needs_runtime and source.exists():
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
         "--firmware-parts", "APP:0.1.0:0x00000000:0x1000:firmware.bin"], project)
    if any((images / "CURRENT").glob("log.*")):
        raise RuntimeError("OpenOCD generated RTT scripts without a board-specific RTT range")
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
                   "--firmware-parts", f"APP:0.1.0:0x00000000:0x1000:{binary}"]
        result = subprocess.run(command, cwd=workspace, capture_output=True, text=True, timeout=30)
        if result.returncode == 0 or "Firmware binary is missing or empty" not in result.stderr or images.exists():
            raise RuntimeError(f"J-Link packager accepted {binary.name} or published a package:\n"
                               f"{result.stdout}{result.stderr}")
    print("J-Link refuses missing or empty firmware: PASS", flush=True)


def check_packaged_partition(package, archive, name, raw, version, revision, *, firmware_name="packager-check"):
    binary_name = f"{name}.bin"
    binary = (package / binary_name).read_bytes()
    tail = struct.Struct("<IIIIII12s32s64s")
    offset = (len(raw) + 7) & ~7
    magic_a, magic_b, firmware_size, tail_size, expected_crc, packed_version, part, firmware, description = (
        tail.unpack_from(binary, offset)
    )
    if ((magic_a, magic_b, firmware_size, tail_size, packed_version) !=
            (0xAA5555AA, 0x88777788, offset, tail.size, version)
            or part.rstrip(b"\0") != name.encode("ascii")
            or firmware.rstrip(b"\0") != firmware_name.encode("ascii")
            or not description.startswith(f"git@{revision[:10]} time@".encode("ascii"))):
        raise RuntimeError(f"Unexpected {name} metadata in {package}")
    padded_input = raw + b"\xff" * (offset - len(raw))
    checksum_input = binary[:offset + 16] + b"\xff" * 4 + binary[offset + 20:offset + tail_size]
    if (binary[:offset] != padded_input or crc32(checksum_input) != expected_crc
            or len(binary) % 16 or binary[offset + tail_size:] != b"\xff" * (len(binary) - offset - tail_size)):
        raise RuntimeError(f"Unexpected {name} data, padding or CRC in {package}")
    if archive.read(binary_name) != binary:
        raise RuntimeError(f"Archive and published package disagree on {binary_name}")


def check_partition_overflow(command, project, current, cache, partition, *, packed_size, capacity):
    previous = current.resolve(strict=True)
    previous_key = cache.read_bytes()
    published_files = set(cache.parent.iterdir())
    result = subprocess.run(command, cwd=project, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, timeout=90)
    diagnostic = (partition, "exceeds capacity", f"{packed_size} bytes", f"{capacity} bytes")
    if result.returncode == 0 or not any(all(marker in line for marker in diagnostic)
                                         for line in result.stdout.splitlines()):
        raise RuntimeError(f"Packing {partition} did not report its final BIN exceeding capacity:\n"
                           f"{result.stdout[-6000:]}")
    if (current.resolve(strict=True) != previous or cache.read_bytes() != previous_key
            or set(cache.parent.iterdir()) != published_files):
        raise RuntimeError("Partition overflow changed the published package, CURRENT or cache")


def check_image_packagers(sdk, workspace):
    project = workspace / "Dual partition app"
    project.mkdir()
    run(["git", "init", "-q"], project)
    revision = commit(project, "Known packaging revision", allow_empty=True)
    sources = workspace / "Partition inputs"
    sources.mkdir()
    # The extra colon exercises the same fixed-field split as an absolute Windows drive path.
    boot = sources / ("boot raw.bin" if os.name == "nt" else "boot drive:raw.bin")
    app = sources / "app raw.bin"
    boot_data, app_data = b"BOOT image!", b"original APP firmware"
    boot.write_bytes(boot_data)
    app.write_bytes(app_data)
    board_config = sources / "custom board.cfg"
    board_config.write_text("# test board config\n", encoding="utf-8")
    jlink_tools = workspace / "Minimal J-Link tools"
    jlink_tools.mkdir()
    backends = {
        "jlink": ["--jlink-path", str(jlink_tools), "--chip-name", "MCXN947_M33_0",
                  "--speed", "4000", "--jtagconf=-1,-1", "--nm", sys.executable],
        "openocd": ["--config-file-list", str(board_config), "--rtt-setting", *OPENOCD_TEST_RTT],
    }
    packages = {}
    for backend, options in backends.items():
        images = workspace / f"{backend} image output"
        current = images / f"{backend}_CURRENT"
        cache = images / "cache"
        base_command = [sys.executable, "-B", str(sdk / f"tool/python/mk_{backend}_img.py"),
                        "--firmware-name", "packager-check", "--output-dir", str(images),
                        "--symlink-name", current.name, "--cache-file", str(cache),
                        *options, "--firmware-parts"]
        app.write_bytes(app_data)
        old_spec = f"APP:r2.7:0x08004000:{os.path.relpath(app, project)}"
        result = subprocess.run([*base_command, old_spec], cwd=project, capture_output=True,
                                text=True, timeout=30)
        if (result.returncode == 0 or images.exists()
                or "Expected NAME:VERSION:ADDRESS:CAPACITY:BIN" not in result.stderr):
            raise RuntimeError(f"{backend} did not reject a four-field firmware record:\n"
                               f"{result.stdout}{result.stderr}")

        app_spec = f"APP:r2.7:0x08004000:160:{app}"
        command = [*base_command, f"BOOT:v1.2.3:0x08000000:0xA0:{boot}", app_spec]
        run(command, project)
        package = current.resolve(strict=True)
        prefix = f"packager-check_{backend}_BOOTv1.2.3_APPv2.7.0_"
        if not package.name.startswith(prefix) or not package.name.endswith(revision[:10]):
            raise RuntimeError(f"Unexpected name for {backend} package: {package.name}")
        burn_command = "download.jlink" if backend == "jlink" else "burn.sh"
        burn = (package / burn_command).read_text(encoding="utf-8")
        for partition, address in (("BOOT", "0x08000000"), ("APP", "0x08004000")):
            if f"{partition}.bin {address}" not in burn:
                raise RuntimeError(f"{backend} burn script has incorrect {partition} address")
            if (package / f"{partition}.bin").stat().st_size != 160:
                raise RuntimeError(f"{backend} did not package {partition} at its exact capacity")
        with ZipFile(package.with_name(package.name + ".zip")) as archive:
            check_packaged_partition(package, archive, "BOOT", boot_data, 0x030201, revision)
            check_packaged_partition(package, archive, "APP", app_data, 0x000702, revision)
        if boot.read_bytes() != boot_data or app.read_bytes() != app_data:
            raise RuntimeError(f"{backend} packaging changed an input binary")

        # The raw BOOT fits at 159 bytes, but its metadata and final padding need 160.
        check_partition_overflow([*base_command, f"BOOT:v1.2.3:0x08000000:159:{boot}", app_spec],
                                 project, current, cache, "BOOT", packed_size=160, capacity=159)
        archives = set(images.glob("*.zip"))
        output = run(command, project)
        if ("package is up to date" not in output or current.resolve(strict=True) != package
                or set(images.glob("*.zip")) != archives):
            raise RuntimeError(f"{backend} packaging did not reuse the unchanged image")

        before = app.stat()
        changed_app = b"modified APP firmware"
        app.write_bytes(changed_app)
        os.utime(app, ns=(before.st_atime_ns, before.st_mtime_ns))
        output = run(command, project)
        if "package is up to date" in output:
            raise RuntimeError(f"{backend} reused a changed image with the same input timestamp")
        updated = current.resolve(strict=True)
        if updated == package:
            raise RuntimeError(f"{backend} replaced a previously published package")
        with ZipFile(package.with_name(package.name + ".zip")) as archive:
            check_packaged_partition(package, archive, "APP", app_data, 0x000702, revision)
        with ZipFile(updated.with_name(updated.name + ".zip")) as archive:
            check_packaged_partition(updated, archive, "APP", changed_app, 0x000702, revision)

        scripts = {name: (updated / name).read_bytes() for name in {burn_command, "burn.sh", "burn.bat"}}
        for name in ("APP.bin", "BOOT.bin", burn_command, "burn.bat"):
            damaged = updated / name
            if name == "burn.bat":
                damaged.write_bytes(scripts[name][:8])
            else:
                damaged.unlink()
            output = run(command, project)
            repaired = current.resolve(strict=True)
            if repaired == updated or "package is up to date" in output:
                raise RuntimeError(f"{backend} reused a package with damaged {name}:\n{output}")
            with ZipFile(repaired.with_name(repaired.name + ".zip")) as archive:
                check_packaged_partition(repaired, archive, "APP", changed_app, 0x000702, revision)
                check_packaged_partition(repaired, archive, "BOOT", boot_data, 0x030201, revision)
                for script, contents in scripts.items():
                    if (repaired / script).read_bytes() != contents or archive.read(script) != contents:
                        raise RuntimeError(f"{backend} did not restore {script} in the package and archive")
            updated = repaired
        packages[backend] = updated
        print(f"{backend}: dual partition format, capacity, archive, alias and cache repair PASS", flush=True)
    return packages


def write_test_tool(path, script):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("#!/bin/sh\n" + script, encoding="utf-8")
    path.chmod(0o755)


def check_image_rtt_failures(sdk, workspace, packages):
    if sys.platform not in ("darwin", "linux"):
        return
    project = workspace / "Packaged RTT app"
    generate(sdk, project, "rtt-check", FRDM_BOARD, "baremetal")
    current = project / "image/CURRENT"
    current.parent.mkdir()
    link = [sys.executable, "-B", str(sdk / "tool/python/image_directory_link.py"), str(current)]
    command = entrypoint(project, "build", "rttlog")

    jlink = packages["jlink"]
    for executable in ("macos/rtt-shell", "linux-x64/rtt-shell", "linux-x86/rtt-shell", "win-x64/rtt-shell.exe"):
        write_test_tool(jlink / "tool/jlink" / executable, "printf '%s\\n' 'RTT test viewer'\nexit 23\n")
    run([*link, str(jlink)], project)
    result = subprocess.run(command, cwd=project, capture_output=True, text=True, timeout=15)
    if result.returncode != 23 or "RTT test viewer" not in result.stdout:
        raise RuntimeError(f"rttlog did not propagate the J-Link viewer exit 23 (got {result.returncode}):\n"
                           f"{result.stdout}{result.stderr}")

    openocd = packages["openocd"]
    if f"rtt setup {' '.join(OPENOCD_TEST_RTT)}" not in (openocd / "log.sh").read_text(encoding="utf-8"):
        raise RuntimeError("OpenOCD did not use the explicit RTT range")
    tools = workspace / "RTT test tools"
    probe = tools / "openocd"
    telnet = tools / "telnet"
    write_test_tool(probe, "exit 37\n")
    write_test_tool(telnet, "exit 42\n")
    env = dict(os.environ, PATH=str(tools) + os.pathsep + os.environ["PATH"])
    run([*link, str(openocd)], project)
    result = subprocess.run(command, cwd=project, env=env, capture_output=True, text=True, timeout=15)
    if result.returncode != 37:
        raise RuntimeError(f"rttlog did not propagate the OpenOCD startup exit 37 (got {result.returncode}):\n"
                           f"{result.stdout}{result.stderr}")

    write_test_tool(probe, "printf '%s\\n' \"$$\" > test-probe.pid\nexec sleep 30 >/dev/null 2>&1\n")
    pid_file = openocd / "test-probe.pid"
    probe_leaked = False
    try:
        result = subprocess.run(command, cwd=project, env=env, capture_output=True, text=True, timeout=15)
    finally:
        if pid_file.is_file():
            pid = int(pid_file.read_text(encoding="ascii"))
            try:
                os.kill(pid, 0)
                probe_leaked = True
                os.kill(pid, signal.SIGTERM)
            except ProcessLookupError:
                pass
    if result.returncode != 42 or not pid_file.is_file() or probe_leaked:
        raise RuntimeError(f"rttlog did not propagate exit 42 and clean up the owned OpenOCD process "
                           f"(exit {result.returncode}, leaked {probe_leaked}):\n{result.stdout}{result.stderr}")
    if (project / "build").exists():
        raise RuntimeError("rttlog created build output while opening an existing package")
    print("Packaged J-Link and OpenOCD RTT failures propagate through rttlog: PASS", flush=True)


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
    image_dir = project / "image"
    package = (image_dir / "CURRENT").resolve(strict=True)
    if image_dir.resolve() not in package.parents or (project / "build/image").exists():
        raise RuntimeError(f"{project.name} did not keep its firmware package under image/")
    if not package.name.startswith(f"{name}_{build_type}_jlink_APPv{version}_"):
        raise RuntimeError(f"Image did not use application name and version: {package.name}")
    if not package.name.endswith(revision[:10]):
        raise RuntimeError(f"Image did not use application HEAD {revision}: {package.name}")
    firmware = package / "APP.bin"
    if not firmware.is_file() or not firmware.stat().st_size:
        raise RuntimeError(f"Firmware binary is missing or empty for {package.name}")
    if not package.with_name(package.name + ".zip").is_file():
        raise RuntimeError(f"Image archive is missing for {package.name}")


def check_bundle_image(project, name, build_type):
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
app_add_jlink_image(bundle
    IMAGE_NAME "${FLY_FIRMWARE_NAME}"
    FIRMWARE_LIST
        "APP:${FLY_FIRMWARE_VERSION}:${FLY_FLASH_ADDRESS}:${FLY_FLASH_SIZE}:fly_app.bin"
        "BOOT:1.2.3:0x000C0000:${TEST_BOOT_CAPACITY}:${boot_bin}"
    DEPENDS fly_app test_bootloader
)
''', encoding="utf-8")
    revision = commit(project, "Add a generated bootloader to a root-level image target")
    command = entrypoint(project, "build", "make_img", "bundle")
    run(command, project)

    current = project / "image/CURRENT"
    package = current.resolve(strict=True)
    firmware_name = f"{name}_{build_type}"
    downloads = (package / "download.jlink").read_text(encoding="utf-8").splitlines()
    if [line for line in downloads if line.startswith("loadfile ")] != [
        "loadfile APP.bin 0x00000000", "loadfile BOOT.bin 0x000C0000",
    ]:
        raise RuntimeError("The application image did not use distinct APP and BOOT addresses")
    app = (project / "build/cmake/fly_app.bin").read_bytes()
    with ZipFile(package.with_name(package.name + ".zip")) as archive:
        check_packaged_partition(package, archive, "APP", app, 0x070200, revision, firmware_name=firmware_name)
        check_packaged_partition(package, archive, "BOOT", boot, 0x030201, revision, firmware_name=firmware_name)
    if (package / "BOOT.bin").stat().st_size != 176:
        raise RuntimeError("The application image did not fill BOOT's exact capacity")
    run(["cmake", "-S", str(project), "-B", str(project / "build/cmake"), "-DTEST_BOOT_CAPACITY=175"], project)
    check_partition_overflow(command, project, current, package.parent / ".bundle_package_key", "BOOT",
                             packed_size=176, capacity=175)
    print("Application-root APP and BOOT package and capacity checks: PASS", flush=True)


def check_image_after_distclean(project):
    current = project / "image/CURRENT"
    package = current.resolve(strict=True)
    firmware = (current / "APP.bin").read_bytes()

    run(entrypoint(project, "build", "distclean"), project)
    if (project / "build").exists():
        raise RuntimeError("distclean did not remove the application build directory")
    if current.resolve(strict=True) != package or (current / "APP.bin").read_bytes() != firmware:
        raise RuntimeError("distclean changed the current firmware package")
    with ZipFile(package.with_name(package.name + ".zip")) as archive:
        if archive.read("APP.bin") != firmware:
            raise RuntimeError("The archive did not retain the current firmware binary")

    # Exercise the package path without starting a probe.
    marker = "RTT package script ran"
    if os.name == "nt":
        (current / "log.bat").write_bytes(f"@echo off\r\necho {marker}\r\n".encode("ascii"))
    else:
        (current / "log.sh").write_text(f"printf '%s\\n' '{marker}'\n", encoding="utf-8")
    output = run(entrypoint(project, "build", "rttlog"), project)
    if marker not in output or (project / "build").exists():
        raise RuntimeError("rttlog did not use the preserved package independently of build/")
    print("Published firmware and RTT entry after distclean: PASS", flush=True)


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


def check_build_outputs(sdk, project, *, host=False):
    cmake_dir = project / "build/cmake"
    cache = (cmake_dir / "CMakeCache.txt").read_text(encoding="utf-8")
    cache_paths = {}
    for line in cache.splitlines():
        if line.startswith(("CMAKE_HOME_DIRECTORY:", "FLY_RESOLVED_SDK_DIR:")):
            key, _, value = line.partition("=")
            cache_paths[key.split(":", 1)[0]] = Path(value).resolve()
    if cache_paths.get("CMAKE_HOME_DIRECTORY") != project.resolve():
        raise RuntimeError(f"{project.name} did not configure from its own CMakeLists.txt")
    if cache_paths.get("FLY_RESOLVED_SDK_DIR") != sdk:
        raise RuntimeError(f"{project.name} did not use the current SDK checkout")

    executable_name = "fly_app.exe" if host and os.name == "nt" else "fly_app"
    outputs = [
        project / "build/config/.config",
        project / "build/config/.config.cmake",
        project / "build/config/autoconf.h",
        cmake_dir / "compile_commands.json",
        cmake_dir / executable_name,
    ]
    if not host:
        outputs.append(cmake_dir / "fly_app.bin")
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


def check_optional_app_targets(sdk, workspace):
    project = workspace / "Optional application targets"
    project.mkdir()
    (project / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.21)
project(test_app_targets NONE)
include("${FLY_TEST_SDK_DIR}/cmake/jlink-make-image.cmake")
include("${FLY_TEST_SDK_DIR}/cmake/vscode-cortex-debug-gdb.cmake")
include("${FLY_TEST_SDK_DIR}/templates/scaffold/tool/cmake/app-image.cmake")
include("${FLY_TEST_SDK_DIR}/templates/scaffold/tool/cmake/app-vscode.cmake")

set(FLY_FLASH_ADDRESS "0x00000000")
set(FLY_FLASH_SIZE "0x40000")
app_add_jlink_image(no_device IMAGE_NAME test FIRMWARE_LIST "APP:0.1.0:0x00000000:0x40000:fly_app.bin")
if(TARGET no_device_make_img OR TARGET no_device_flash)
    message(FATAL_ERROR "Unexpected J-Link image targets without a board device")
endif()

function(check_unavailable name device gdb objdump)
    set(FLY_JLINK_DEVICE "${device}")
    set(CMAKE_GDB "${gdb}")
    set(CMAKE_OBJDUMP "${objdump}")
    app_add_vscode_debug(fly_app)
    if(TARGET default_make_vscode_jlink_gnu_gdb)
        message(FATAL_ERROR "Unexpected debug target for ${name}")
    endif()
endfunction()

check_unavailable(no_device "" "${CMAKE_COMMAND}" "${CMAKE_COMMAND}")
check_unavailable(no_gdb MCXN947_M33_0 "CMAKE_GDB-NOTFOUND" "${CMAKE_COMMAND}")
check_unavailable(no_objdump MCXN947_M33_0 "${CMAKE_COMMAND}" "CMAKE_OBJDUMP-NOTFOUND")
''', encoding="utf-8")
    run(["cmake", "-S", str(project), "-B", str(project / "build"), f"-DFLY_TEST_SDK_DIR:PATH={sdk}"], project)
    print("Application wrappers skip image and debug targets when board or tools are unavailable: PASS", flush=True)


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

    elf = (project / "build/cmake/fly_app").resolve()
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


def check_frdm_project(sdk, workspace, template, build_type):
    project = workspace / template
    name = f"test-{template}"
    generate(sdk, project, name, FRDM_BOARD, template)
    check_frdm_layout(project, template)
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
    cmake_cache = check_build_outputs(sdk, project)
    check_firmware_info_messages(project, enabled=True)
    if template in FRDM_NANOSIG_SMOKES:
        binary = (project / "build/cmake/fly_app.bin").read_bytes()
        for message in (b"nanosig ready", b"nanosig stopped"):
            if message not in binary:
                raise RuntimeError(f"{project.name} did not link the nanosig demo message {message!r}")
    make_img = entrypoint(project, "build", "make_img")
    run(make_img, project)

    for directory in ("build/", "image/"):
        run(["git", "check-ignore", "--", directory], project)
    check_vscode(project)
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

        config = (project / "build/config/.config").read_text(encoding="utf-8")
        opt_out = project / "build/no-factory-data.config"
        opt_out.write_text(config.replace(FACTORY_DATA_ENABLED, FACTORY_DATA_DISABLED), encoding="utf-8")
        run(entrypoint(project, "build", "loadconfig", str(opt_out)), project)
        run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
        check_firmware_info_messages(project, enabled=False)
        print("baremetal: factory-data opt-out PASS", flush=True)
        check_bundle_image(project, name, build_type)
        check_image_after_distclean(project)
    print(f"{template}: build and J-Link package PASS", flush=True)


def check_generated_config(sdk, workspace, board, template):
    name = f"{board['id']}-{template}"
    project = workspace / name
    generate(sdk, project, name, board["id"], template)
    output = run(entrypoint(project, "build", "loadconfig", "defconfig"), project)
    if any("warning:" in line.lower() for line in output.splitlines()):
        raise RuntimeError(f"{name} generated Kconfig warnings:\n{output}")

    config = (project / "build/config/.config").read_text(encoding="utf-8").splitlines()
    expected = [f'{board["arch_config"]}=y', *template_manifest(sdk, template).get("expected_config", ())]
    board_config = sdk / "templates/boards" / board["id"] / "defconfig"
    if board_config.is_file():
        expected.extend(line for line in board_config.read_text(encoding="utf-8").splitlines()
                        if re.fullmatch(r"CONFIG_[A-Z0-9_]+=.*|# CONFIG_[A-Z0-9_]+ is not set", line))
    for assignment in expected:
        if assignment not in config:
            raise RuntimeError(f"{name} is missing Kconfig line {assignment!r}")
    print(f"{name}: Kconfig PASS", flush=True)


def check_registered_template(sdk, workspace):
    fixture = workspace / "SDK with an added template"
    fixture.mkdir()
    for launcher in ("fly.sh", "fly.bat"):
        shutil.copy2(sdk / launcher, fixture / launcher)
    for directory in ("templates", "tool/python"):
        shutil.copytree(sdk / directory, fixture / directory, ignore=shutil.ignore_patterns("__pycache__"))
    template = {
        "label": "Data-only fixture template",
        "expected_log": "fixture says hello",
        "overlays": ["common/fixture-layer"],
        "board_bindings": {"FIXTURE_NUMBER": {"field": "fixture_number", "type": "positive_integer"}},
        "expected_config": ["CONFIG_FIXTURE_NUMBER=6100"],
        "required_files": ["app/main.c", "app/biz/fixture.c"],
    }
    board = {
        "id": "fixture-board", "label": "Fixture board", "chip": "Fixture chip", "core": "Fixture core",
        "arch_config": "CONFIG_ARCH_FIXTURE", "toolchain": "Fixture toolchain", "probe": "No probe",
        "fixture_number": 6100, "templates": ["fixture-template"],
    }
    files = {
        "templates/apps/fixture-template/template.json": json.dumps(template),
        "templates/apps/fixture-template/defconfig": "CONFIG_FIXTURE_NUMBER=@FIXTURE_NUMBER@\n",
        "templates/apps/fixture-template/app/main.c": "/* selected template */\n",
        "templates/apps/common/fixture-layer/app/main.c": "/* shared overlay */\n",
        "templates/apps/common/fixture-layer/app/biz/fixture.c": "/* overlay marker */\n",
        "templates/boards/fixture-board/board.json": json.dumps(board),
        "templates/boards/fixture-board/board.cmake": "# fixture board\n",
    }
    for name, content in files.items():
        destination = fixture / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(content, encoding="utf-8")

    project = workspace / "fixture-app"
    generate(fixture, project, "fixture-app", board["id"], "fixture-template")
    if (project / "app/main.c").read_text(encoding="utf-8") != files["templates/apps/fixture-template/app/main.c"]:
        raise RuntimeError("The selected template did not override the shared overlay")
    if (project / "app/biz/fixture.c").read_text(encoding="utf-8") != "/* overlay marker */\n":
        raise RuntimeError("The manifest's overlay content did not reach the generated application")
    config = (project / "defconfig").read_text(encoding="utf-8").splitlines()
    if config != [f'{board["arch_config"]}=y', *template["expected_config"]]:
        raise RuntimeError("The generator did not render the manifest's board binding")
    if template["expected_log"] not in (project / "README.md").read_text(encoding="utf-8"):
        raise RuntimeError("The generated README did not use the manifest's expected log")
    print("Data-only template registration, overlay and board binding: PASS", flush=True)


def check_board_build(sdk, workspace, board, template, build_type):
    name = f"{board['id']}-{template}"
    project = workspace / name
    generate(sdk, project, name, board["id"], template)
    run(entrypoint(project, "build", "build", build_type, "-j", "2"), project)
    check_build_outputs(sdk, project, host=bool(board.get("host_platform")))
    print(f"{name}: {build_type} compile and link PASS", flush=True)


def check_frdm_templates(sdk, workspace, build_type):
    check_image_links(sdk, workspace)
    check_sdk_setup(sdk, workspace)
    check_native_sdk_pin(sdk, workspace)
    check_optional_app_targets(sdk, workspace)
    check_openocd_script(sdk, workspace)
    check_jlink_requires_binary(sdk, workspace)
    packages = check_image_packagers(sdk, workspace)
    check_image_rtt_failures(sdk, workspace, packages)
    with ThreadPoolExecutor(max_workers=len(FRDM_SMOKES)) as pool:
        futures = [pool.submit(check_frdm_project, sdk, workspace, template, build_type) for template in FRDM_SMOKES]
        for future in futures:
            future.result()
    return {(FRDM_BOARD, template) for template in FRDM_SMOKES}


def check_matrix(sdk, workspace, matrix, build_type):
    boards = [json.loads(path.read_text(encoding="utf-8"))
              for path in sorted((sdk / "templates/boards").glob("*/board.json"))]
    if not boards:
        raise RuntimeError("No board manifests found")
    if matrix == "config":
        for board in boards:
            for template in board["templates"]:
                check_generated_config(sdk, workspace, board, template)
        check_registered_template(sdk, workspace)
        return

    platform = sys.platform if matrix == "host" else None
    selected = [board for board in boards if board.get("host_platform") == platform]
    if not selected:
        raise RuntimeError(f"No boards registered for the {matrix} matrix on {sys.platform}")
    smoke_coverage = check_frdm_templates(sdk, workspace, build_type) if matrix == "arm" else set()
    for board in selected:
        for template in board["templates"]:
            if (board["id"], template) not in smoke_coverage:
                check_board_build(sdk, workspace, board, template, build_type)


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
