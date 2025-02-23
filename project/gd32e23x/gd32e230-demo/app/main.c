
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

    /* 开总中断 */
    __enable_irq();

    printf("SystemCoreClock:%lu\r\n", SystemCoreClock);
    // SystemInit();
    
    printf_app_info();
    tick_init();


    while(1){
         printf("hello world %lu\r\n", tick_get());
    }
    return 0;
}