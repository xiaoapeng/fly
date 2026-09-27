#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Pull in kconfiglib's standard product: one #define CONFIG_<sym> <val>
 * per Kconfig symbol in .config. auto-generate/ is on the global include
 * path (top-level CMakeLists.txt:33). */
#include <autoconf.h>

/* ---- user-tunable bridge: Kconfig value > curated default ----
 * #ifndef protects against a future caller that defines configXXX first;
 * #ifdef CONFIG_PACKAGE_FREERTOS_XXX forwards the Kconfig value;
 * #else keeps the build green when Kconfig does not expose the key. */
#ifndef configTICK_RATE_HZ
#  ifdef CONFIG_PACKAGE_FREERTOS_TICK_RATE_HZ
#    define configTICK_RATE_HZ               CONFIG_PACKAGE_FREERTOS_TICK_RATE_HZ
#  else
#    define configTICK_RATE_HZ               1000
#  endif
#endif

#ifndef configMAX_PRIORITIES
#  ifdef CONFIG_PACKAGE_FREERTOS_MAX_PRIORITIES
#    define configMAX_PRIORITIES             CONFIG_PACKAGE_FREERTOS_MAX_PRIORITIES
#  else
#    define configMAX_PRIORITIES             5
#  endif
#endif

#ifndef configTOTAL_HEAP_SIZE
#  ifdef CONFIG_PACKAGE_FREERTOS_TOTAL_HEAP_SIZE
#    define configTOTAL_HEAP_SIZE            CONFIG_PACKAGE_FREERTOS_TOTAL_HEAP_SIZE
#  else
#    define configTOTAL_HEAP_SIZE            8192
#  endif
#endif

#ifndef configMINIMAL_STACK_SIZE
#  ifdef CONFIG_PACKAGE_FREERTOS_MINIMAL_STACK_SIZE
#    define configMINIMAL_STACK_SIZE         CONFIG_PACKAGE_FREERTOS_MINIMAL_STACK_SIZE
#  else
#    define configMINIMAL_STACK_SIZE         128
#  endif
#endif

#ifndef configCPU_CLOCK_HZ
#  ifdef CONFIG_PACKAGE_FREERTOS_SYSTICK_CLOCK_HZ
#    define configCPU_CLOCK_HZ               CONFIG_PACKAGE_FREERTOS_SYSTICK_CLOCK_HZ
#  else
#    define configCPU_CLOCK_HZ               120000000UL
#  endif
#endif

#ifndef configUSE_TIMERS
#  ifdef CONFIG_PACKAGE_FREERTOS_USE_TIMERS
#    define configUSE_TIMERS                 CONFIG_PACKAGE_FREERTOS_USE_TIMERS
#  else
#    define configUSE_TIMERS                 0
#  endif
#endif

#ifndef configCHECK_FOR_STACK_OVERFLOW
#  ifdef CONFIG_PACKAGE_FREERTOS_CHECK_STACK_OVERFLOW
#    define configCHECK_FOR_STACK_OVERFLOW   CONFIG_PACKAGE_FREERTOS_CHECK_STACK_OVERFLOW
#  else
#    define configCHECK_FOR_STACK_OVERFLOW   2
#  endif
#endif

#ifndef configUSE_TICKLESS_IDLE
#  ifdef CONFIG_PACKAGE_FREERTOS_TICKLESS_IDLE
#    define configUSE_TICKLESS_IDLE          CONFIG_PACKAGE_FREERTOS_TICKLESS_IDLE
#  else
#    define configUSE_TICKLESS_IDLE          0
#  endif
#endif

/* ---- stable boilerplate (cross-version subset; curated set guarantees compat) ---- */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_CO_ROUTINES                   0
#define configMAX_COROUTINE_PRIORITIES          2
#define configUSE_MUTEXES                       1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_RECURSIVE_MUTEXES             1
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_16_BIT_TICKS                  0
#define configUSE_TASK_NOTIFICATIONS            1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1

/* ---- INCLUDE_* gates (opt-in in newer kernels; turn on the demo needs) ---- */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_xTaskDelayUntil                 1

/* ---- Assertions & port self-checks ----
 * configASSERT routes to a hard fault-style trap that keeps RTT alive so the
 * failing file/line stays visible over rttlog. This also arms V11.1.0's
 * configCHECK_HANDLER_INSTALLATION self-test in xPortStartScheduler
 * (vector-table entries for SVC/PendSV). */
extern void vAssertCalled(const char *pcFile, unsigned long ulLine);
#define configASSERT(x)    do { if ((x) == 0) { vAssertCalled(__FILE__, __LINE__); } } while (0)
#define configCHECK_HANDLER_INSTALLATION  1

/* ---- Architecture capabilities from autoconf.h ----
 * The architecture Kconfig owns CPU/core/FPU/ABI/NVIC/TrustZone facts. The
 * generic package only supplies fallback values for a port that did not
 * provide them. The Cortex-M33 non-secure port sets TrustZone off and
 * selects ARM_CM33_NTZ/non_secure, which omits SecureContext refs. */
#ifndef configENABLE_FPU
#  ifdef CONFIG_ARCH_HAS_HARD_FPU
#    define configENABLE_FPU                   CONFIG_ARCH_HAS_HARD_FPU
#  else
#    define configENABLE_FPU                   0
#  endif
#endif
#ifndef configENABLE_MPU
#  define configENABLE_MPU                     0
#endif
/* ---- Security state (inherent to the NTZ port, not per-arch tunable) ----
 * This package only builds the NTZ (non-TrustZone) ports: ARM_CM33_NTZ,
 * ARM_CM0, ARM_CM3, ARM_CM4F. The NTZ semantics mean the kernel runs
 * entirely in the Secure state and never switches to the non-secure world,
 * so:
 *   - configENABLE_TRUSTZONE / _SECURE_CONTEXT must be 0 (no SecureContext
 *     references exist in the NTZ port);
 *   - configRUN_FREERTOS_SECURE_ONLY must be 1 so the initial EXC_RETURN is
 *     the Secure-only variant (0xFFFFFFFD). The NS variant (0xFFFFFFBC)
 *     faults on the first exception return on security-extension cores
 *     (UsageFault, xTickCount stays 0, scheduler dies before the first
 *     tick); on cores without the security extension (M0/M3/M4) the ES/S
 *     bits are ignored and the value is harmless either way.
 * A dual-state secure build would require the ARM_CM33 port plus explicit
 * arch support and is out of scope for this package. */
#ifndef configENABLE_TRUSTZONE
#  define configENABLE_TRUSTZONE               0
#endif
#ifndef configENABLE_TRUSTZONE_SECURE_CONTEXT
#  define configENABLE_TRUSTZONE_SECURE_CONTEXT 0
#endif
#ifndef configRUN_FREERTOS_SECURE_ONLY
#  define configRUN_FREERTOS_SECURE_ONLY       1
#endif

#ifndef __NVIC_PRIO_BITS
#  ifdef CONFIG_ARCH_NVIC_PRIO_BITS
#    define __NVIC_PRIO_BITS                   CONFIG_ARCH_NVIC_PRIO_BITS
#  else
#    error "ARCH_NVIC_PRIO_BITS is required by the selected architecture"
#  endif
#endif
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 5U << ( 8U - __NVIC_PRIO_BITS ) )

/* NOTE: this port does NOT use vPortSVCHandler/xPortPendSVHandler and does
 * NOT do #include_next in FreeRTOSConfig.h. The non_secure port defines the
 * vector symbols directly: SysTick_Handler in port.c and
 * SVC_Handler/PendSV_Handler in portasm.c. Nothing to define here. */

#endif /* FREERTOS_CONFIG_H */