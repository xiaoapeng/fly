"""Shared helpers for tool/test/ scripts.

Provides a thin wrapper around kconfiglib that:

* loads a fresh `kconfiglib.Kconfig` from the repository root
* loads a defconfig (or any Kconfig snippet) into a temporary copy, never
  touching the user's active `.config`
* exposes a small assertion API for Kconfig symbols

Any test script under tool/test/ should import from here instead of using
kconfiglib directly so that the rules above stay consistent.
"""

from __future__ import annotations

import dataclasses
import shutil
import sys
import tempfile
from pathlib import Path

try:
    import kconfiglib
except ImportError:  # pragma: no cover
    sys.stderr.write(
        "error: kconfiglib not installed "
        "(set FLY_PYTHON or activate the FLY .venv with `pip install kconfiglib`)\n"
    )
    sys.exit(2)


@dataclasses.dataclass
class TestContext:
    repo_root: Path
    tmpdir: Path
    # Hold the TemporaryDirectory so it isn't garbage-collected when
    # make_context() returns; without this the tmpdir is removed before any
    # caller can write to it.
    _tmp: "tempfile.TemporaryDirectory[str]" = dataclasses.field(
        repr=False, compare=False
    )


def make_context(repo_root: Path, prefix: str = "fly_test_") -> TestContext:
    """Return a TestContext whose tmpdir lives until the TestContext is GC'd.

    The temporary directory is bound to the returned TestContext (via its
    `_tmp` field), so `load_defconfig` / `load_inline_config` can safely write
    files into it. The dir is reclaimed when the TestContext itself is
    garbage-collected, which happens at script exit (or when the caller
    drops its reference).
    """
    tmp = tempfile.TemporaryDirectory(prefix=prefix)
    return TestContext(repo_root=repo_root, tmpdir=Path(tmp.name), _tmp=tmp)


def load_defconfig(ctx: TestContext, defconfig_rel: str) -> tuple[kconfiglib.Kconfig, Path]:
    """Return (Kconfig, tmp_config_path) for the given defconfig.

    Loads a private copy from `ctx.tmpdir` so the user's active `.config`
    is never touched. Raises FileNotFoundError if the source defconfig
    does not exist.
    """
    k = kconfiglib.Kconfig(str(ctx.repo_root / "Kconfig"))
    src = ctx.repo_root / defconfig_rel
    if not src.exists():
        raise FileNotFoundError(src)
    safe_name = defconfig_rel.replace("/", "__")
    tmp_cfg = ctx.tmpdir / safe_name
    tmp_cfg.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, tmp_cfg)
    k.load_config(str(tmp_cfg))
    return k, tmp_cfg


def load_inline_config(ctx: TestContext, name: str, body: str) -> kconfiglib.Kconfig:
    """Return a Kconfig loaded with an inline `body` snippet.

    The body is written to a fresh file under `ctx.tmpdir/<name>.config`
    and loaded into a new Kconfig. Use for synthetic / malformed-config
    tests that do not have a real defconfig on disk.
    """
    k = kconfiglib.Kconfig(str(ctx.repo_root / "Kconfig"))
    tmp_cfg = ctx.tmpdir / f"{name}.config"
    tmp_cfg.write_text(body)
    k.load_config(str(tmp_cfg))
    return k


def expect(k: kconfiglib.Kconfig, sym: str, want: str) -> str | None:
    """Assert k.syms[sym].str_value == want; return the actual value on mismatch."""
    actual = k.syms[sym].str_value
    if actual != want:
        return f"{sym}: got {actual!r}, expected {want!r}"
    return None