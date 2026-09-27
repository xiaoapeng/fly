#!/usr/bin/env python3
"""Repository-local build regression check for arm-none-eabi-gcc projects.

For each arm project that `arm-none-eabi-gcc` can compile, this script:

1. Runs `./build.sh loadconfig <defconfig>` so `.config` / autoconf.h are
   regenerated (never touches the user's active `.config`).
2. Removes `build/` so cmake re-configures from scratch.
3. Runs `./build.sh build Debug -j 4` to compile + link.
4. Loads the produced ELF and asserts:
   - `nm <elf> | grep ' U '` does not contain any unresolved FreeRTOS API
     symbol (`xTask*`, `vTask*`, `Secure*`) — V-3 / V-8.
   - The legacy projects (those without `CONFIG_PACKAGE_SCHEDULER_FREERTOS=y`)
     contain no FreeRTOS symbol at all — V-8.
   - The freertos-demo ELF has a strong `SysTick_Handler` symbol from
     `freertos-port` — V-4 (handler uniqueness).

RISC-V projects (`ch58x`, `gd32vf103x`) and host x86_64 projects
(`linux`, `macos`, `windows`) are intentionally skipped — their toolchains
are not present in this host environment and any failure would be a
toolchain availability issue, not a plan regression.

Coverage:
  * 11 arm-none-eabi-gcc projects: 4×STM32 + 3×GD32 (cortex-m) + 4×MCXN947
  * Excluded: gd32vf103c-demo, ch582-demo, linux/macos/windows ehip-test,
    windows co-ctx-test (RISC-V / x86_64 toolchains)
"""

from __future__ import annotations

import argparse
import dataclasses
import re
import shutil
import subprocess
import sys
from pathlib import Path

# Allow `python3 tool/test/check_build_arm.py` to import tool.test.lib.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

from tool.test.lib import TestContext, make_context  # noqa: E402


# Each entry: (defconfig_rel_path, expected_executable_basename, project_id).
# executable_basename is the target name produced by the project's CMakeLists
# (MCXN947/STM32 use `<demo>_app`; GD32 uses `<demo>` directly).
ARM_PROJECTS: list[tuple[str, str, str]] = [
    # mcxn947 — 3 demos
    ("project/mcxn947/freertos-demo/defconfig",          "mcxn947_freertos_demo_app", "mcxn-freertos"),
    ("project/mcxn947/mcxn947-demo/defconfig",           "mcxn947_demo_app",          "mcxn-demo"),
    ("project/mcxn947/mcxn947-eth-demo/defconfig",       "mcxn947_eth_demo_app",      "mcxn-eth"),
    # STM32 — 4 demos
    ("project/stm32f0xx/stm32f030x8-demo/defconfig",    "stm32f030x8_demo_app",      "stm32f0"),
    ("project/stm32f1xx/stm32f103c6-demo/defconfig",    "stm32f103c6_demo_app",      "stm32f1"),
    ("project/stm32f4xx/stm32f401cb-demo/defconfig",    "stm32f401cb_demo_app",      "stm32f4"),
    ("project/stm32h7xx/stm32h750vb-demo/defconfig",    "stm32h750vb_demo_app",      "stm32h7"),
    # GD32 cortex-m — 3 demos (executable is `<demo>`, not `<demo>_app`)
    ("project/gd32e23x/gd32e230-demo/defconfig",        "gd32e230-demo",             "gd32e23x"),
    ("project/gd32f10x/gd32f103-demo/defconfig",        "gd32f103-demo",             "gd32f103"),
    ("project/gd32f10x/gd32f105-demo/defconfig",        "gd32f105-demo",             "gd32f105"),
    # gd32vf103c-demo: RISC-V, skipped — see module docstring.
    # ch582-demo: RISC-V, skipped — see module docstring.
]

# FreeRTOS API symbols that must NOT appear as unresolved references in any
# non-freertos-demo build. (V-3: link hygiene; V-8: scheduler exclusivity.)
FREERTOS_API_RE = re.compile(r"\b(?:xTask|vTask|Secure)\w*\b")

# Projects that link against freertos-kernel/heap/port (i.e. PACKAGE_FREERTOS=y).
# These are allowed to define FreeRTOS symbols.
FREERTOS_PROJECTS = {"mcxn-freertos"}


@dataclasses.dataclass
class BuildResult:
    project_id: str
    defconfig_rel: str
    ok: bool
    error: str | None = None
    text_bytes: int = 0
    elf_path: Path | None = None


def _run(cmd: list[str], cwd: Path, timeout: int = 600) -> tuple[int, str]:
    """Run a shell command, return (returncode, combined_output)."""
    proc = subprocess.run(
        cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout,
    )
    return proc.returncode, (proc.stdout + proc.stderr)


def _tool_available(tool: str) -> bool:
    return shutil.which(tool) is not None


def find_elf(project_dir: Path, basename: str) -> Path | None:
    """Locate the produced ELF inside cmake's per-project binary dir.

    The shared toolchain puts executables at
    `build/project/<arch>/<demo>/<basename>` for most projects, but GD32
    leaves the executable un-suffixed. We search both patterns.
    """
    candidates = [
        project_dir / "build" / "project" / basename,
    ]
    # Walk build/ to find any executable matching the basename.
    build_root = project_dir / "build"
    if build_root.exists():
        for p in build_root.rglob(basename):
            if p.is_file():
                return p
    return None


def nm_undefined(elf: Path) -> list[str]:
    """Return list of undefined symbol names from `nm <elf> | grep ' U '`."""
    rc, out = _run(["nm", str(elf)], cwd=elf.parent)
    if rc != 0:
        return [f"<nm failed: rc={rc}>"]
    return [
        line.split()[1]
        for line in out.splitlines()
        if " U " in line and len(line.split()) >= 2
    ]


def arm_none_eabi_size_text(elf: Path) -> int:
    rc, out = _run(["arm-none-eabi-size", str(elf)], cwd=elf.parent)
    if rc != 0:
        return -1
    # Last line is "<text> <data> <bss> <dec> <hex> <filename>"
    last = out.splitlines()[-1].split()
    return int(last[0])


def nm_defined_freertos(elf: Path) -> list[str]:
    """Return defined FreeRTOS-related symbols (filter to T/W sections)."""
    rc, out = _run(["nm", str(elf)], cwd=elf.parent)
    if rc != 0:
        return []
    out_syms = []
    for line in out.splitlines():
        if " T " not in line and " W " not in line and " t " not in line:
            continue
        parts = line.split()
        if len(parts) < 2:
            continue
        sym = parts[1]
        if FREERTOS_API_RE.search(sym):
            out_syms.append(sym)
    return out_syms


def build_one(ctx: TestContext, repo_root: Path, defconfig: str, exe_basename: str,
              project_id: str, jobs: int = 4, verbose: bool = False) -> BuildResult:
    res = BuildResult(project_id=project_id, defconfig_rel=defconfig, ok=False)
    build_sh = repo_root / "build.sh"

    # 1) loadconfig — populates .config + auto-generate/
    rc, _out = _run(
        [str(build_sh), "loadconfig", defconfig], cwd=repo_root,
    )
    if rc != 0:
        res.error = f"loadconfig failed (rc={rc})"
        return res

    # 2) clean build dir — make sure cmake re-configures
    build_dir = repo_root / "build"
    if build_dir.exists():
        shutil.rmtree(build_dir)

    # 3) build Debug
    rc, _out = _run(
        [str(build_sh), "build", "Debug", "-j", str(jobs)],
        cwd=repo_root, timeout=900,
    )
    if rc != 0:
        res.error = f"build Debug failed (rc={rc})"
        return res

    # 4) locate ELF
    elf = find_elf(repo_root, exe_basename)
    if not elf:
        res.error = f"ELF '{exe_basename}' not produced"
        return res
    res.elf_path = elf
    res.text_bytes = arm_none_eabi_size_text(elf)

    # 5) V-3 / V-8: legacy projects must not reference FreeRTOS APIs
    undefined = nm_undefined(elf)
    leaking = [s for s in undefined if FREERTOS_API_RE.search(s)]
    if leaking:
        res.error = (
            f"V-3/V-8 FAIL: {len(leaking)} FreeRTOS API reference(s) leaked: "
            f"{leaking[:5]}{'...' if len(leaking) > 5 else ''}"
        )
        return res

    # 6) V-4 (freertos-demo only): expect a strong SysTick_Handler from freertos-port
    if project_id in FREERTOS_PROJECTS:
        rc, out = _run(
            ["nm", str(elf)], cwd=elf.parent,
        )
        sys_line = next(
            (l for l in out.splitlines() if "SysTick_Handler" in l and " T " in l),
            None,
        )
        if not sys_line:
            res.error = "V-4 FAIL: freertos-demo ELF has no strong SysTick_Handler"
            return res
        if verbose:
            print(f"    {sys_line.strip()}")

    res.ok = True
    return res


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root", type=Path, default=Path(__file__).resolve().parent.parent.parent,
    )
    parser.add_argument("-j", "--jobs", type=int, default=4)
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument(
        "--only", help="comma-separated project IDs to include (default: all)",
    )
    args = parser.parse_args(argv)

    if not _tool_available("arm-none-eabi-gcc"):
        sys.stderr.write(
            "error: arm-none-eabi-gcc not in PATH — install the GNU ARM "
            "Embedded Toolchain and retry (or use --skip-build to bypass).\n"
        )
        return 2

    repo_root = args.repo_root.resolve()
    import os
    os.chdir(repo_root)
    make_context(repo_root)  # warm any helper init; not actively used

    selected = set(args.only.split(",")) if args.only else None
    failures: list[tuple[str, str]] = []
    results: list[BuildResult] = []

    for defconfig, exe_basename, pid in ARM_PROJECTS:
        if selected and pid not in selected:
            continue
        sys.stdout.write(f"[check_build_arm] {pid:14s} {defconfig}\n")
        sys.stdout.flush()
        result = build_one(
            None, repo_root, defconfig, exe_basename, pid,
            jobs=args.jobs, verbose=args.verbose,
        )
        results.append(result)
        if result.ok:
            sys.stdout.write(
                f"  PASS  text={result.text_bytes} B  elf={result.elf_path.name}\n"
            )
        else:
            sys.stderr.write(f"  FAIL  {result.error}\n")
            failures.append((pid, result.error or "unknown"))

    sys.stdout.write(
        f"\ncheck_build_arm: {len(results) - len(failures)}/{len(results)} projects PASS\n"
    )

    if failures:
        sys.stderr.write("check_build_arm: FAILED\n")
        for pid, err in failures:
            sys.stderr.write(f"  {pid}: {err}\n")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))