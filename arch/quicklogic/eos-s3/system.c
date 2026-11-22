#include <stdint.h>

void SystemInit(void)
{
    /* The EOS S3 application inherits the clocks from its boot environment. */
    volatile uint32_t *const cpacr = (volatile uint32_t *)0xE000ED88u;
    *cpacr |= 0x00F00000u;
    __asm__ volatile ("dsb\nisb" ::: "memory");
}

_Noreturn void dbg_fatal_error_int(const char *message, int line)
{
    (void)message;
    (void)line;
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
