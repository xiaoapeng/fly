#include "board.h"

#include "mid/fly_log.h"

#include <stdint.h>
#include <string.h>

static void configure_core_clock(void)
{
    enum {
        HFCG_CONTROL = 0,
        HFCG_M_LOW = 2,
        HFCG_M_HIGH = 4,
        HFCG_N = 6,
        HFCG_PRESCALER = 8,
        HFCG_LOAD = 1u << 0,
        HFCG_CHANGING = 1u << 7,
        HFCG_CORE_DIVIDE_BY_SIX = 5u << 4,
    };
    volatile uint8_t *const hfcg = (volatile uint8_t *)UINT32_C(0x400B5000);

    /* Configure the NPCX9 EVB's 15 MHz core clock from 90 MHz OFMCLK. */
    hfcg[HFCG_PRESCALER] = (hfcg[HFCG_PRESCALER] & 0x0Fu) | HFCG_CORE_DIVIDE_BY_SIX;
    if (hfcg[HFCG_M_LOW] != 0xBAu || hfcg[HFCG_M_HIGH] != 0x0Au || hfcg[HFCG_N] != 0x82u) {
        hfcg[HFCG_M_LOW] = 0xBAu;
        hfcg[HFCG_M_HIGH] = 0x0Au;
        hfcg[HFCG_N] = 0x82u;
        hfcg[HFCG_CONTROL] |= HFCG_LOAD;
        while ((hfcg[HFCG_CONTROL] & HFCG_CHANGING) != 0u) {
        }
    }
}

void board_init(void)
{
    configure_core_clock();
    __asm__ volatile ("cpsie i" ::: "memory");
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __asm__ volatile ("wfi");
}
