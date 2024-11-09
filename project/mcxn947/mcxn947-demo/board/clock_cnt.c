/**
 * @file clock_cnt.c
 * @brief 系统时钟计次器
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-06-30
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#include <eh.h>
#include <eh_module.h>
#include "eh_user_config.h"

#include "clock_cnt.h"
#include "fsl_common.h"

#define TICK_PER_SEC                                  (100U)
volatile eh_clock_t sys_clock_cnt = 0;
uint32_t tick_cycle;

void SysTick_Handler(void)
{
    sys_clock_cnt += tick_cycle;
}

/* 超精度时钟 */
eh_clock_t  platform_get_clock_monotonic_time(void){
    eh_clock_t sys_clock_cnt_a , sys_clock_cnt_b;
    sys_clock_cnt_a = sys_clock_cnt + (SysTick->LOAD - SysTick->VAL);
    sys_clock_cnt_b = sys_clock_cnt + (SysTick->LOAD - SysTick->VAL);
    return sys_clock_cnt_a > sys_clock_cnt_b ? sys_clock_cnt_a : sys_clock_cnt_b;
}

static __init int systick_init(void)
{
    tick_cycle = SystemCoreClock/TICK_PER_SEC;
    SysTick_Config(tick_cycle);
    return 0;
}

eh_core_module_export(systick_init, NULL);