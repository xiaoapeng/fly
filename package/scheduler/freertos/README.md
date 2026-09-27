# package/scheduler/freertos — FreeRTOS-Kernel v1 integration

This sub-package (living under `package/scheduler/freertos/`) fetches [FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) into `dl/freertos-kernel-src/`, builds the curated `V11.1.0` tag by default, and exposes a 4-target split that links into the application via `target_link_package_lib()`. It is gated by `if(CONFIG_PACKAGE_SCHEDULER_FREERTOS)` in its own `CMakeLists.txt` and is only loaded by the scheduler choice at the Kconfig layer.

## Package boundary

The package deliberately contains **no** knowledge of:

- concrete architecture names (`ARCH_MCXN947`, `ARCH_STM32F4XX`, ...)
- the eventhub-os scheduler or its `PACKAGE_SCHEDULER_EVENTHUB_OS` choice member
- FreeRTOS-specific architecture capability symbols

It depends only on the generic fact `ARCH_CPU_FAMILY_CORTEX_M`. Generic architecture facts (`ARCH_CORE`, `ARCH_TRUSTZONE_MODE`, `ARCH_HAS_HARD_FPU`, `ARCH_FLOAT_ABI`, `ARCH_NVIC_PRIO_BITS`, ...) flow from `arch/<arch>/Kconfig` through `auto-generate/autoconf.h`. Mutual exclusion with eventhub-os is enforced by the `PACKAGE_SCHEDULER` choice in `package/scheduler/Kconfig`, not by this package.

## 4-target split

| target | type | role |
|---|---|---|
| `freertos-config` | INTERFACE | include directories and global defines (`_ARMV8M_LM_PRESENT`, `_ARMV8M_MAINLINE_PRESENT`); exposed through the public link chain rather than registered with `package_lib_add` |
| `freertos-heap` | OBJECT | `portable/MemMang/<heap>.c` (heap_4 default) |
| `freertos-kernel` | OBJECT | `tasks.c` `queue.c` `list.c` `event_groups.c` `stream_buffer.c` (+ `timers.c` if `PACKAGE_FREERTOS_USE_TIMERS=y`) |
| `freertos-port` | OBJECT | `portable/GCC/ARM_CM33_NTZ/non_secure/{port.c,portasm.c}` |

PUBLIC link chain: `freertos-port → freertos-kernel → freertos-heap → freertos-config`. The executable reaches `freertos-config` through this chain.

## Why `freertos-config` is not registered

`cmake/package.cmake:9-13` (`package_lib_compile_options`) applies `target_compile_options(<lib> PRIVATE ...)` to every lib in `FLY_ALL_PACKAGE_LIB_LIST`. CMake treats `PRIVATE` options on an INTERFACE target as a hard configure error. Since `freertos-config` is INTERFACE, we expose it through the PUBLIC chain instead of `package_lib_add()`-ing it. It carries only include dirs + definitions; the strict global flag pass is therefore safely skipped for this target.

## Configuration bridge

No standalone renderer script. `tool/python/build.py` already writes `auto-generate/autoconf.h` via `kconfiglib.write_autoconf()` during `menuconfig` / `saveconfig` / `loadconfig`; this package's CMakeLists additionally refreshes it inline at plain-`build` time.

`inc/FreeRTOSConfig.h` is the **single authoritative template**:

```c
#include <autoconf.h>

#ifndef configTICK_RATE_HZ
#  ifdef CONFIG_PACKAGE_FREERTOS_TICK_RATE_HZ
#    define configTICK_RATE_HZ  CONFIG_PACKAGE_FREERTOS_TICK_RATE_HZ
#  else
#    define configTICK_RATE_HZ  1000
#  endif
#endif
```

Three-layer fallback per tunable:

1. caller already defined → keep caller's value
2. Kconfig exposed the key → use it
3. otherwise → curated default keeps the build green

Architecture capabilities (`configENABLE_FPU`, `configENABLE_TRUSTZONE`, `__NVIC_PRIO_BITS`) are bridged from `CONFIG_ARCH_*` symbols the same way.

The template lives only in `package/scheduler/freertos/inc/`. Projects must not provide their own `FreeRTOSConfig.h`; the packaged template is made available through the `freertos-config` INTERFACE target's include directories.

## Assertion policy

`configASSERT` is armed in the packaged `FreeRTOSConfig.h` and routes to `vAssertCalled()`. The package provides a **weak** default implementation (`freertos_assert.c`) that parks with interrupts masked and no output — the package has no knowledge of any debug backend (Segger RTT, UART, ...). Projects that want the assert location printed should define their own **strong** `vAssertCalled(const char *pcFile, unsigned long ulLine)` and route it to whatever output they already use (the freertos-demo prints it over RTT).

## Generic-fact → port mapping

The package owns the mapping table:

```cmake
if(CONFIG_ARCH_CORE STREQUAL "cortex-m33")
    if(CONFIG_ARCH_TRUSTZONE_MODE STREQUAL "non-secure")
        set(FREERTOS_PORT_DIR "portable/GCC/ARM_CM33_NTZ/non_secure")
    else()
        message(FATAL_ERROR ...)
    endif()
    set(FREERTOS_PORT_GLOBAL_DEFS _ARMV8M_LM_PRESENT _ARMV8M_MAINLINE_PRESENT)
elseif(CONFIG_ARCH_CORE STREQUAL "cortex-m4")
    if(CONFIG_ARCH_HAS_HARD_FPU AND CONFIG_ARCH_FLOAT_ABI STREQUAL "hard")
        set(FREERTOS_PORT_DIR "portable/GCC/ARM_CM4F")
    else()
        message(FATAL_ERROR ...)
    endif()
else()
    message(FATAL_ERROR ...)
endif()
```

Users do **not** select a port. Architecture Kconfig only declares generic hardware/startup facts; this package maps them to the portable path.

## Mutex with other OS packages

`package/scheduler/Kconfig` is the **only** scheduler mutual-exclusion authority. This package performs no cross-OS validation; if a hand-edited `.config` accidentally pins both scheduler legs to `y`, kconfiglib normalizes the choice to the leg declared later in `scheduler/Kconfig` (FreeRTOS) and the eventhub-os sub-config stays hidden.

## Version selection

`PACKAGE_FREERTOS_VERSION` → `PACKAGE_FREERTOS_GIT_TAG` → `fetch_git_project(GIT_TAG)`. Default tag is `V11.1.0`. Switch with `./build.sh package_update freertos-kernel` (forces re-fetch) or `./build.sh menuconfig` + rebuild (re-pulls the new tag). Re-verify the chosen tag exists with `git ls-remote --tags https://github.com/FreeRTOS/FreeRTOS-Kernel.git` before pinning.