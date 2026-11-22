#include <stdint.h>

extern const uint32_t __Vectors[];

void SystemInit(void)
{
    volatile uint32_t *const vtor = (volatile uint32_t *)0xE000ED08u;
    volatile uint32_t *const cpacr = (volatile uint32_t *)0xE000ED88u;
    *vtor = (uint32_t)(uintptr_t)__Vectors;
    *cpacr |= 0x00F00000u;
    __asm__ volatile ("dsb\nisb" ::: "memory");
}
