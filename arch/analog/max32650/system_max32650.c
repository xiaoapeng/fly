#include <stdint.h>

#define GCR_SCON      (*(volatile uint32_t *)UINT32_C(0x40000000))
#define GCR_SCON_OVR  (UINT32_C(3) << 16)
#define GCR_OVR_1V1   (UINT32_C(2) << 16)
#define SCB_VTOR      (*(volatile uint32_t *)UINT32_C(0xE000ED08))
#define SCB_CPACR     (*(volatile uint32_t *)UINT32_C(0xE000ED88))
#define CP10_CP11     (UINT32_C(15) << 20)

extern const uint32_t __isr_vector[];

__attribute__((weak)) int PreInit(void)
{
    /* Apply the MSDK power-up SRAM timing workaround before RAM initialization. */
    GCR_SCON = (GCR_SCON & ~GCR_SCON_OVR) | GCR_OVR_1V1;
    return 0;
}

__attribute__((weak)) void SystemInit(void)
{
    SCB_VTOR = (uintptr_t)__isr_vector;
    SCB_CPACR |= CP10_CP11;
    __asm__ volatile ("dsb\nisb\ncpsie i" ::: "memory");
}
