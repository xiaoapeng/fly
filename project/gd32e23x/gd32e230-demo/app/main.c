
#include <stdio.h>
#include <SEGGER_RTT.h>
#include <factory-data.h>
#include <gd32e23x.h>
/**
 * @brief                   此函数是为了实现printf
 */
__used int _write (int fd, char *pBuffer, int size)
{
    (void) fd;
    SEGGER_RTT_Write(0, pBuffer, (unsigned)size);
    return size;
}

static void printf_app_info(void){
    const FactoryData* factory_data;
    factory_data = factory_data_detect((const void *)0x8000000, 32 * 1024);
    printf("part_name: %s\r\n", factory_data->part_name);
    printf("firmware_name: %s\r\n", factory_data->firmware_name);
    printf("generated_description: %s\r\n", factory_data->generated_description);
    printf("software_len: %lu\r\n", factory_data->software_len);
    printf("version: V%d.%d.%d\r\n", factory_data->software_version.major, 
            factory_data->software_version.minor, factory_data->software_version.patch);
}

static volatile uint32_t systick_cnt = 0;
void SysTick_Handler(void)
{
    systick_cnt++;
}

uint32_t tick_get(void)
{
    return systick_cnt;
}

void tick_init(void){
    uint32_t tick_cycle;
    tick_cycle = (SystemCoreClock)/1000;
    SysTick_Config(tick_cycle);
    /* 开启中断 */
    NVIC_SetPriority(SysTick_IRQn, 0);
    NVIC_EnableIRQ(SysTick_IRQn);

}




int main(void){
    uint32_t last_time = 0;
    /* 开总中断 */
    __enable_irq();

    printf("SystemCoreClock:%lu\r\n", SystemCoreClock);
    
    printf_app_info();
    tick_init();

    /* LED PB4 一秒闪烁一次 */
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);



    last_time = tick_get();
    while(1){
        if(tick_get() - last_time > 1000){
            last_time += 1000;
            /* 一秒打印一次 hello workd!! 并闪烁LED灯 */
            printf("hello world!!! \r\n");
            gpio_bit_toggle(GPIOB, GPIO_PIN_4);
        }
    }
    return 0;
}