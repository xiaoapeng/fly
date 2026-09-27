#!/usr/bin/env python3
"""Repository-local Kconfig regression check for the FreeRTOS v1 migration.

Loads each covered defconfig in a fresh kconfiglib.Kconfig instance via
tool.test.lib helpers (which load each defconfig into a private tmpdir
copy so the user's active `.config` is never touched), then exits
nonzero with the failing project paths and mismatched symbols on any
mismatch.

Coverage:
  * 12 active legacy defconfigs   (must pin SCHEDULER_EVENTHUB_OS)
  * freertos-demo defconfig       (must pin SCHEDULER_FREERTOS)
  * ch582-demo defconfig          (must pin SCHEDULER_NONE)
  * synthetic dual-leg .config    (must normalize to one scheduler leg)
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

# Allow `python3 tool/test/check_freertos_config.py` to import tool.test.lib.
# The script lives at <repo>/tool/test/<name>.py; add <repo> to sys.path.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

from tool.test.lib import (  # noqa: E402
    TestContext,
    expect,
    load_defconfig,
    load_inline_config,
    make_context,
)


LEGACY_DEFCONFIGS = [
    "project/gd32f10x/gd32f105-demo/defconfig",
    "project/gd32vf103x/gd32vf103c-demo/defconfig",
    "project/linux/ehip-test/defconfig",
    "project/macos/ehip-test/defconfig",
    "project/mcxn947/mcxn947-demo/defconfig",
    "project/mcxn947/mcxn947-eth-demo/defconfig",
    "project/stm32f0xx/stm32f030x8-demo/defconfig",
    "project/stm32f1xx/stm32f103c6-demo/defconfig",
    "project/stm32f4xx/stm32f401cb-demo/defconfig",
    "project/stm32h7xx/stm32h750vb-demo/defconfig",
    "project/windows/co-ctx-test/defconfig",
    "project/windows/ehip-test/defconfig",
]

FREERTOS_DEFCONFIG = "project/mcxn947/freertos-demo/defconfig"
NONE_DEFCONFIG    = "project/ch58x/ch582-demo/defconfig"


def check_legacy_eventhub(ctx: TestContext, rel: str) -> list[str]:
    k, _ = load_defconfig(ctx, rel)
    return [
        m for m in (
            expect(k, "PACKAGE_SCHEDULER_EVENTHUB_OS", "y"),
            expect(k, "PACKAGE_SCHEDULER_FREERTOS",     "n"),
            expect(k, "PACKAGE_SCHEDULER_NONE",         "n"),
        ) if m
    ]


def check_freertos(ctx: TestContext) -> list[str]:
    k, _ = load_defconfig(ctx, FREERTOS_DEFCONFIG)
    arch_facts = [
        ("ARCH_CPU_FAMILY_CORTEX_M", "y"),
        ("ARCH_CORE",                 "cortex-m33"),
        ("ARCH_HAS_HARD_FPU",         "y"),
        ("ARCH_FLOAT_ABI",            "hard"),
        ("ARCH_HAS_TRUSTZONE",        "y"),
        ("ARCH_TRUSTZONE_NON_SECURE", "y"),
        ("ARCH_NVIC_PRIO_BITS",       "3"),
    ]
    sym_checks = [
        ("PACKAGE_SCHEDULER_FREERTOS",     "y"),
        ("PACKAGE_SCHEDULER_EVENTHUB_OS",  "n"),
        ("PACKAGE_SCHEDULER_NONE",         "n"),
        *arch_facts,
    ]
    return [m for (s, w) in sym_checks for m in (expect(k, s, w),) if m]


def check_none(ctx: TestContext) -> list[str]:
    k, _ = load_defconfig(ctx, NONE_DEFCONFIG)
    return [
        m for m in (
            expect(k, "PACKAGE_SCHEDULER_NONE",         "y"),
            expect(k, "PACKAGE_SCHEDULER_EVENTHUB_OS",  "n"),
            expect(k, "PACKAGE_SCHEDULER_FREERTOS",     "n"),
        ) if m
    ]


def check_malformed_safety(ctx: TestContext) -> list[str]:
    """Hand-edited dual-leg .config: both scheduler legs pinned to y.

    kconfiglib normalizes the choice to the leg declared later in
    scheduler/Kconfig (FreeRTOS); the eventhub leg must drop back to n,
    and the eventhub sub-config must not be visible.
    """
    k = load_inline_config(
        ctx,
        "malformed_both_on",
        (
            "CONFIG_ARCH_MCXN947=y\n"
            "CONFIG_PACKAGE_SCHEDULER_EVENTHUB_OS=y\n"
            "CONFIG_PACKAGE_SCHEDULER_FREERTOS=y\n"
        ),
    )
    sched_active = [
        s.name for s in k.syms.values()
        if s.choice and s.choice.name == "PACKAGE_SCHEDULER" and s.tri_value == 2
    ]
    eht = k.syms["PACKAGE_EVENTHUB_GLOBAL_TIMER_SIGNAL"]
    return [
        m for m in (
            expect(k, "PACKAGE_SCHEDULER_FREERTOS",     "y"),
            expect(k, "PACKAGE_SCHEDULER_EVENTHUB_OS",  "n"),
            f"PACKAGE_SCHEDULER active leg: {sched_active}" if sched_active != ["PACKAGE_SCHEDULER_FREERTOS"] else None,
            f"PACKAGE_EVENTHUB_GLOBAL_TIMER_SIGNAL visibility: got {eht.visibility}, expected 0 (hidden)"
                if eht.visibility != 0 else None,
        ) if m
    ]


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path(__file__).resolve().parent.parent.parent,
        help="repository root (default: parent of tool/test)",
    )
    args = parser.parse_args(argv)

    repo_root = args.repo_root.resolve()
    # kconfiglib resolves `source "arch/Kconfig"` relative to cwd; keep the
    # test deterministic by running from the repo root.
    import os
    os.chdir(repo_root)

    ctx = make_context(repo_root)
    failures: list[tuple[str, list[str]]] = []

    for rel in LEGACY_DEFCONFIGS:
        try:
            f = check_legacy_eventhub(ctx, rel)
        except Exception as e:
            failures.append((f"legacy:{rel}", [f"exception: {e!r}"]))
            continue
        if f:
            failures.append((f"legacy:{rel}", f))

    for name, runner in (
        ("freertos-demo",         check_freertos),
        ("ch582-demo-none",       check_none),
        ("malformed-safety-net",  check_malformed_safety),
    ):
        try:
            f = runner(ctx)
        except Exception as e:
            failures.append((name, [f"exception: {e!r}"]))
            continue
        if f:
            failures.append((name, f))

    if failures:
        sys.stderr.write("check_freertos_config: FAILED\n")
        for name, errs in failures:
            sys.stderr.write(f"  {name}:\n")
            for e in errs:
                sys.stderr.write(f"    - {e}\n")
        return 1

    total = len(LEGACY_DEFCONFIGS) + 3
    sys.stdout.write(f"check_freertos_config: PASS ({total} checks)\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))