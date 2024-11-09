/**
 * @file init.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-09-16
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include "SEGGER_RTT.h"
#include "eh.h"
#include "eh_debug.h"
#include "gd32vf103.h"
#include "n200_func.h"


void stdout_write(void *stream, const uint8_t *buf, size_t size){
    (void)stream;
    SEGGER_RTT_Write(0, buf, (unsigned)size);
}

eh_clock_t  platform_get_clock_monotonic_time(void){
    return (eh_clock_t)get_timer_value();
}

void _init();

int init(void){
    _init();
    
    rcu_periph_clock_enable(RCU_AF);

    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);

    // eh_infoln("get_cpu_freq: %d", get_cpu_freq());
    
    return 0;
}