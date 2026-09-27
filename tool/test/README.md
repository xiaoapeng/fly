# tool/test/

Host-side regression scripts that validate FLY Kconfig / build configuration
without touching toolchains, hardware, or the user's active `.config`.

`tool/python/` is reserved for **build orchestration** (cmake invocation,
fetch helpers, image packaging, Kconfig rendering). This directory is for
**validation** scripts that assert the configuration is correct.

## Running

```bash
python3 tool/test/check_freertos_config.py
```

`tool/test/lib/` is the shared helper layer (Kconfig loader, defconfig
isolation, symbol assertion API). Other scripts in this directory import
from it instead of touching `kconfiglib` directly so the "never overwrite
the user's `.config`" invariant stays in one place.

## Files

| Path | Purpose |
|---|---|
| `lib/__init__.py` | `load_defconfig`, `load_inline_config`, `expect`, `TestContext` |
| `check_freertos_config.py` | V-5 + V-8 sweep: 12 legacy defconfigs + freertos-demo + ch582-demo + malformed `.config` safety net |
| `check_build_arm.py` | Build-time regression: loadconfig + cmake configure + arm-none-eabi-gcc Debug build for 10 arm projects; asserts V-3 (no FreeRTOS API undefined), V-8 (legacy projects contain no FreeRTOS symbols), V-4 (freertos-demo has strong `SysTick_Handler`). Skips RISC-V (`gd32vf103x`, `ch58x`) and x86_64 (`linux`/`macos`/`windows`) projects whose toolchains are not in this host environment. |

## Conventions

* Each check is a callable returning a list of `failure` strings; an empty
  list means the check passed.
* A top-level `main()` builds a single `TestContext`, runs every check, and
  exits 0 only if every check returned an empty failure list.
* Never import from `tool/python/` — this directory is allowed to depend on
  `kconfiglib` only.
* All script entry points are designed to run with the FLY `.venv`'s
  Python (`./.venv/bin/python`) so that the workspace-local `kconfiglib`
  install is used.

## Adding a new check

1. Add a new `check_<topic>.py` in this directory.
2. Import helpers from `lib` (`from .lib import load_defconfig, expect, TestContext`).
3. Define one or more `def check_<name>(ctx: TestContext) -> list[str]` functions.
4. In `main()`, accumulate failures per check and exit 0/1 accordingly.
5. Update this README to list the new file.