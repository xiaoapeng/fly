/*
 * Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Adapted for standalone GNU builds from Renesas FSP startup.c and system.c.
 */

#include <stdint.h>
#include <string.h>

extern uint32_t _estack;
extern uint8_t _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
void __libc_init_array(void);
void Reset_Handler(void);

void Default_Handler(void)
{
    for (;;) {
        __asm__ volatile ("wfi");
    }
}

#define WEAK_HANDLER(handler) void handler(void) __attribute__((weak, alias("Default_Handler")))
WEAK_HANDLER(NMI_Handler);
WEAK_HANDLER(HardFault_Handler);
WEAK_HANDLER(SVC_Handler);
WEAK_HANDLER(PendSV_Handler);
WEAK_HANDLER(SysTick_Handler);
#undef WEAK_HANDLER

// RA2E1 has 32 ICU-routed slots, left unconfigured by the bare-metal template.
__attribute__((section(".isr_vector"), used))
const uintptr_t ra2e1_vectors[16 + 32] = {
    [0] = (uintptr_t)&_estack,
    [1] = (uintptr_t)Reset_Handler,
    [2] = (uintptr_t)NMI_Handler,
    [3] = (uintptr_t)HardFault_Handler,
    [11] = (uintptr_t)SVC_Handler,
    [14] = (uintptr_t)PendSV_Handler,
    [15] = (uintptr_t)SysTick_Handler,
    [16 ... 47] = (uintptr_t)Default_Handler,
};

void SystemInit(void)
{
    *(volatile uint32_t *)UINT32_C(0xE000ED08) = (uintptr_t)ra2e1_vectors;
    __asm__ volatile ("dsb\nisb" ::: "memory");

    memcpy(&_sdata, &_sidata, (uintptr_t)&_edata - (uintptr_t)&_sdata);
    memset(&_sbss, 0, (uintptr_t)&_ebss - (uintptr_t)&_sbss);
    __libc_init_array();
}

void Reset_Handler(void)
{
    SystemInit();
    (void)main();
    Default_Handler();
}
