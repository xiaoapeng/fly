#include "board_time.h"

#define SYSTICK_CTRL    (*(volatile uint32_t *)UINT32_C(0xe000e010))
#define SYSTICK_RELOAD  (*(volatile uint32_t *)UINT32_C(0xe000e014))
#define SYSTICK_CURRENT (*(volatile uint32_t *)UINT32_C(0xe000e018))
#define SCB_ICSR        (*(volatile uint32_t *)UINT32_C(0xe000ed04))
#define SCB_SHPR3       (*(volatile uint32_t *)UINT32_C(0xe000ed20))

#define SYSTICK_ENABLE          (UINT32_C(1) << 0)
#define SYSTICK_TICKINT         (UINT32_C(1) << 1)
#define SYSTICK_CORE_CLOCK      (UINT32_C(1) << 2)
#define SYSTICK_MAX_RELOAD      UINT32_C(0x00ffffff)
#define SYSTICK_CLEAR_PENDING   (UINT32_C(1) << 25)
#define SYSTICK_LOWEST_PRIORITY (UINT32_C(0xff) << 24)

static volatile uint64_t elapsed_us;

void SysTick_Handler(void)
{
    elapsed_us += UINT64_C(1000);
}

bool board_time_start(void)
{
    const uint32_t ticks_per_ms = FLY_BOARD_CLOCK_HZ / 1000u;
    if (ticks_per_ms == 0u || ticks_per_ms - 1u > SYSTICK_MAX_RELOAD
            || FLY_BOARD_CLOCK_HZ % 1000u != 0u) {
        return false;
    }

    SYSTICK_CTRL = 0u;
    SCB_ICSR = SYSTICK_CLEAR_PENDING;
    elapsed_us = 0u;
    SYSTICK_RELOAD = ticks_per_ms - 1u;
    SYSTICK_CURRENT = 0u;
    SCB_SHPR3 |= SYSTICK_LOWEST_PRIORITY;
    SYSTICK_CTRL = SYSTICK_CORE_CLOCK | SYSTICK_TICKINT | SYSTICK_ENABLE;
    return true;
}

uint64_t board_time_us(void)
{
    uint32_t interrupt_state;
    /* A 32-bit core cannot read the timer's 64-bit counter atomically. */
    __asm__ volatile ("mrs %0, primask\n\tcpsid i" : "=r"(interrupt_state) :: "memory");
    const uint64_t now_us = elapsed_us;
    __asm__ volatile ("msr primask, %0" :: "r"(interrupt_state) : "memory");
    return now_us;
}
