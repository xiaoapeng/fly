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
WEAK_HANDLER(MemManage_Handler);
WEAK_HANDLER(BusFault_Handler);
WEAK_HANDLER(UsageFault_Handler);
WEAK_HANDLER(SVC_Handler);
WEAK_HANDLER(DebugMon_Handler);
WEAK_HANDLER(PendSV_Handler);
WEAK_HANDLER(SysTick_Handler);
#undef WEAK_HANDLER

// RA4M1 has 96 ICU-routed slots, left unconfigured by the bare-metal template.
__attribute__((section(".isr_vector"), used))
const uintptr_t ra4m1_vectors[16 + 96] = {
    [0] = (uintptr_t)&_estack,
    [1] = (uintptr_t)Reset_Handler,
    [2] = (uintptr_t)NMI_Handler,
    [3] = (uintptr_t)HardFault_Handler,
    [4] = (uintptr_t)MemManage_Handler,
    [5] = (uintptr_t)BusFault_Handler,
    [6] = (uintptr_t)UsageFault_Handler,
    [11] = (uintptr_t)SVC_Handler,
    [12] = (uintptr_t)DebugMon_Handler,
    [14] = (uintptr_t)PendSV_Handler,
    [15] = (uintptr_t)SysTick_Handler,
    [16 ... 111] = (uintptr_t)Default_Handler,
};

void SystemInit(void)
{
    *(volatile uint32_t *)UINT32_C(0xE000ED08) = (uintptr_t)ra4m1_vectors;
    *(volatile uint32_t *)UINT32_C(0xE000ED88) |= UINT32_C(15) << 20;
    __asm__ volatile ("dsb\nisb" ::: "memory");

    memcpy(&_sdata, &_sidata, (uintptr_t)&_edata - (uintptr_t)&_sdata);
    memset(&_sbss, 0, (uintptr_t)&_ebss - (uintptr_t)&_sbss);
    __libc_init_array();
}

void Reset_Handler(void)
{
    SystemInit();
    /* The Arduino bootloader masks interrupts before entering the APP. */
    __asm__ volatile ("cpsie i" ::: "memory");
    (void)main();
    Default_Handler();
}
