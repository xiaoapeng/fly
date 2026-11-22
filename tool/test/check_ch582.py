#!/usr/bin/env python3
"""Build, link and package a CH582 fixture using the installed WCH toolchain."""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from zipfile import ZipFile

SDK = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(SDK / "tool/python"))

from arch_registry import for_board
from firmware_payload import find_artifact, load_artifact


def check_package(project, build):
    manifest_path = find_artifact(project, "ch582", "Release")
    manifest = load_artifact(
        manifest_path, board_id="board/ch582-smoke", chip="ch582", core_id="core0",
        target="ch582", build_type="Release",
    )
    app, = manifest["partitions"]
    if (manifest["firmware_name"] != "ch582-smoke" or app["name"] != "APP"
            or app["version"] != "v1.0.0" or int(app["address"], 16) != 0
            or app["capacity"] != 448 * 1024 or app["origin"] != "built"):
        raise RuntimeError("CH582 package has incorrect firmware identity or flash layout")

    package = manifest_path.parent
    binary = (package / app["bin"]["path"]).read_bytes()
    raw = (build / "ch582-smoke.bin").read_bytes()
    elf = (build / "ch582-smoke").read_bytes()
    # ELFCLASS32, ELFDATA2LSB, and EM_RISCV.
    if elf[:6] != b"\x7fELF\x01\x01" or elf[18:20] != b"\xf3\x00":
        raise RuntimeError("CH582 build did not produce a little-endian RV32 ELF")
    listing = (build / "ch582-smoke.list").read_bytes()
    if elf[24:28] != bytes(4) or b"<FLASH_EEPROM_CMD>:" not in listing:
        raise RuntimeError("CH582 executable is missing the vendor startup or ISP routine")
    if (app["raw"]["size"] != len(raw) or binary[:len(raw)] != raw
            or (package / app["symbols"]["elf"]["path"]).read_bytes() != elf):
        raise RuntimeError("CH582 package differs from the built BIN or ELF")

    filenames = [
        manifest_path.name,
        app["bin"]["path"],
        app["symbols"]["elf"]["path"],
        app["symbols"]["debug_file"]["path"],
    ]
    with ZipFile(package.with_suffix(".zip")) as archive:
        for filename in filenames:
            if archive.read(filename) != (package / filename).read_bytes():
                raise RuntimeError(f"CH582 ZIP differs from the published {filename}")
    print(f"CH582 firmware package and CRC {app['trailer']['crc32']}: PASS", flush=True)


def main():
    arch = for_board(SDK / "arch", "board/ch582-smoke", {"arch": "ch58x", "target_kind": "mcu"})
    with tempfile.TemporaryDirectory(prefix="fly CH582 ") as workdir:
        project = Path(workdir) / "app"
        build = project / "build"
        shutil.copytree(SDK / "tool/test/fixtures/ch582", project)
        subprocess.run([
            "cmake", "-S", str(project), "-B", str(build), "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=Release", f"-DPython3_EXECUTABLE={sys.executable}",
            f"-DFLY_TOP_DIR={SDK}", f"-DFLY_ARCH_DIR={arch.directory}",
            f"-DCONFIG_ARCH_TOOLCHAIN={arch.toolchain}", f"-DCONFIG_ARCH_CORE={arch.cpu}",
            f"-DCONFIG_ARCH_ISA={arch.isa}", f"-DCONFIG_ARCH_MARCH={arch.march}",
            f"-DCONFIG_ARCH_ABI={arch.abi}",
        ], cwd=project, check=True)
        subprocess.run(["cmake", "--build", str(build), "--target", "ch582-smoke"],
                       cwd=project, check=True)
        print("CH582 Release compile and link: PASS", flush=True)
        subprocess.run(["cmake", "--build", str(build), "--target", "ch582_make_img"],
                       cwd=project, check=True)
        check_package(project, build)


if __name__ == "__main__":
    main()
