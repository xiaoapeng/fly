#include "CH58x_common.h"

int main(void)
{
    _Alignas(4) uint8_t id[8];
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    GPIOB_ModeCfg(GPIO_Pin_1, GPIO_ModeOut_PP_5mA);
    GET_UNIQUE_ID(id);
    if (id[0] != 0) {
        GPIOB_SetBits(GPIO_Pin_1);
    }
    for (;;) {
        __asm__ volatile ("nop");
    }
}
