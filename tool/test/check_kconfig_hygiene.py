#!/usr/bin/env python3
"""Repository-local Kconfig hygiene check (CI gate).

Two tree-wide checks, no hardcoded project list:

1. No choice member may be targeted by `select` / `imply`.
   kconfiglib derives a choice member's value from `choice.selection` and
   never reads `rev_dep` (the expression `select` writes to), so such a
   select is silently dropped. Only *named* choices even get the
   "no effect on choice symbols" warning; unnamed ones fail silently.

2. Every `project/*/*/defconfig` must load with zero kconfiglib warnings.

Rule reference: docs/reference/kconfig-rules.md R1 / R2.
"""

from __future__ import annotations

import argparse
import contextlib
import io
import os
import sys
from pathlib import Path

# Allow `python3 tool/test/check_kconfig_hygiene.py` to import tool.test.lib.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent.parent))

import kconfiglib  # noqa: E402

from tool.test.lib import TestContext, load_defconfig, make_context  # noqa: E402


def _load_capturing(ctx: TestContext, defconfig_rel: str):
    """Load a defconfig via tool.test.lib, returning (kconfig, warnings)."""
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(buf):
        k, _ = load_defconfig(ctx, defconfig_rel)
    warnings = [line for line in buf.getvalue().splitlines() if line.strip()]
    return k, warnings


def check_no_choice_selects(k: kconfiglib.Kconfig) -> list[str]:
    """Every choice member must be free of select/imply reverse deps."""
    failures: list[str] = []
    for node in k.node_iter():
        choice = node.item
        if not isinstance(choice, kconfiglib.Choice):
            continue
        for sym in choice.syms:
            for expr, kind in ((sym.rev_dep, "selected"), (sym.weak_rev_dep, "implied")):
                if expr is k.n:
                    continue
                failures.append(
                    f"{sym.name_and_loc}: {kind} by "
                    f"{kconfiglib.expr_str(expr)} — select/imply on a choice "
                    f"member is a no-op (kconfig-rules R1)"
                )
    return failures


def check_defconfig_warnings(ctx: TestContext, defconfig_rel: str) -> list[str]:
    """Each defconfig must load without emitting kconfig warnings."""
    try:
        _, warnings = _load_capturing(ctx, defconfig_rel)
    except Exception as e:  # noqa: BLE001 - report as a check failure
        return [f"{defconfig_rel}: exception: {e!r}"]
    return warnings


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
    # kconfiglib resolves `source "arch/Kconfig"` relative to cwd.
    os.chdir(repo_root)

    ctx = make_context(repo_root)
    defconfigs = sorted(
        str(p.relative_to(repo_root)) for p in repo_root.glob("project/*/*/defconfig")
    )
    if not defconfigs:
        sys.stderr.write("check_kconfig_hygiene: FAILED (no defconfig found)\n")
        return 1

    failures: list[tuple[str, list[str]]] = []

    # Any defconfig gives the same static parse tree; use the first one.
    base, _ = _load_capturing(ctx, defconfigs[0])
    f = check_no_choice_selects(base)
    if f:
        failures.append(("choice-select", f))

    # A single root cause (e.g. one stray select) warns on every defconfig that
    # loads it, so group identical warning text instead of repeating it N times.
    seen: dict[str, list[str]] = {}
    for rel in defconfigs:
        for w in check_defconfig_warnings(ctx, rel):
            seen.setdefault(w, []).append(rel)
    for warning, files in seen.items():
        where = files[0] if len(files) == 1 else f"{files[0]} (+{len(files) - 1} more defconfigs)"
        failures.append((f"warnings:{where}", [warning]))

    if failures:
        sys.stderr.write("check_kconfig_hygiene: FAILED\n")
        for name, errs in failures:
            sys.stderr.write(f"  {name}:\n")
            for e in errs:
                sys.stderr.write(f"    - {e}\n")
        return 1

    sys.stdout.write(
        f"check_kconfig_hygiene: PASS (1 choice sweep + {len(defconfigs)} defconfigs)\n"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
