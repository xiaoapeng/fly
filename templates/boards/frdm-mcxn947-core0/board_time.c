#include "board_time.h"

#include "fsl_common.h"

static volatile uint64_t elapsed_us;

void SysTick_Handler(void)
{
    elapsed_us += UINT64_C(1000);
}

bool board_time_start(void)
{
    return SysTick_Config(SystemCoreClock / 1000u) == 0u;
}

uint64_t board_time_us(void)
{
    uint32_t interrupt_state = __get_PRIMASK();

    /* A 32-bit core cannot read the 64-bit counter atomically. */
    __disable_irq();
    uint64_t now_us = elapsed_us;
    __set_PRIMASK(interrupt_state);
    return now_us;
}
