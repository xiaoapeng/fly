/*  Copyright (c) 2011 - 2012 ARM LIMITED
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name of ARM nor the names of its contributors may be used
      to endorse or promote products derived from this software without
      specific prior written permission.
    *
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

/* GNU startup for the FLY linker contract, with the V2.7.0 vendor vector order. */
.syntax unified
.cpu cortex-m3
.thumb

.section .text.Reset_Handler,"ax",%progbits
.weak Reset_Handler
.type Reset_Handler, %function
.thumb_func
Reset_Handler:
    cpsid i
    ldr r0, =_estack
    msr msp, r0

    /* Initialize C storage before SystemInit uses it. */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
.Lcopy_data:
    cmp r1, r2
    bhs .Lclear_start
    ldr r3, [r0], #4
    str r3, [r1], #4
    b .Lcopy_data

.Lclear_start:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0
.Lclear_bss:
    cmp r1, r2
    bhs .Lstart
    str r3, [r1], #4
    b .Lclear_bss

.Lstart:
    bl SystemInit
    bl main
    b Default_Handler
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.global Default_Handler
.type Default_Handler, %function
.thumb_func
Default_Handler:
    b Default_Handler
.size Default_Handler, .-Default_Handler

.macro vector handler
    .word \handler
    .weak \handler
    .thumb_set \handler, Default_Handler
.endm

.section .isr_vector,"a",%progbits
.balign 4
.global g_pfnVectors
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    vector NMI_Handler
    vector HardFault_Handler
    vector MemManage_Handler
    vector BusFault_Handler
    vector UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    vector SVC_Handler
    vector DebugMon_Handler
    .word 0
    vector PendSV_Handler
    vector SysTick_Handler
    vector WWDGT_IRQHandler
    vector LVD_IRQHandler
    vector TAMPER_IRQHandler
    vector RTC_IRQHandler
    vector FMC_IRQHandler
    vector RCU_IRQHandler
    vector EXTI0_IRQHandler
    vector EXTI1_IRQHandler
    vector EXTI2_IRQHandler
    vector EXTI3_IRQHandler
    vector EXTI4_IRQHandler
    vector DMA0_Channel0_IRQHandler
    vector DMA0_Channel1_IRQHandler
    vector DMA0_Channel2_IRQHandler
    vector DMA0_Channel3_IRQHandler
    vector DMA0_Channel4_IRQHandler
    vector DMA0_Channel5_IRQHandler
    vector DMA0_Channel6_IRQHandler
    vector ADC0_1_IRQHandler
#if defined(GD32F10X_CL)
    vector CAN0_TX_IRQHandler
    vector CAN0_RX0_IRQHandler
#else
    vector USBD_HP_CAN0_TX_IRQHandler
    vector USBD_LP_CAN0_RX0_IRQHandler
#endif
    vector CAN0_RX1_IRQHandler
    vector CAN0_EWMC_IRQHandler
    vector EXTI5_9_IRQHandler
#if defined(GD32F10X_XD)
    vector TIMER0_BRK_TIMER8_IRQHandler
    vector TIMER0_UP_TIMER9_IRQHandler
    vector TIMER0_TRG_CMT_TIMER10_IRQHandler
#else
    vector TIMER0_BRK_IRQHandler
    vector TIMER0_UP_IRQHandler
    vector TIMER0_TRG_CMT_IRQHandler
#endif
    vector TIMER0_Channel_IRQHandler
    vector TIMER1_IRQHandler
    vector TIMER2_IRQHandler
    vector TIMER3_IRQHandler
    vector I2C0_EV_IRQHandler
    vector I2C0_ER_IRQHandler
    vector I2C1_EV_IRQHandler
    vector I2C1_ER_IRQHandler
    vector SPI0_IRQHandler
    vector SPI1_IRQHandler
    vector USART0_IRQHandler
    vector USART1_IRQHandler
    vector USART2_IRQHandler
    vector EXTI10_15_IRQHandler
    vector RTC_Alarm_IRQHandler
#if defined(GD32F10X_CL)
    vector USBFS_WKUP_IRQHandler
#else
    vector USBD_WKUP_IRQHandler
#endif
#if defined(GD32F10X_MD)
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    vector EXMC_IRQHandler
#else
#if defined(GD32F10X_XD)
    vector TIMER7_BRK_TIMER11_IRQHandler
    vector TIMER7_UP_TIMER12_IRQHandler
    vector TIMER7_TRG_CMT_TIMER13_IRQHandler
#else
    vector TIMER7_BRK_IRQHandler
    vector TIMER7_UP_IRQHandler
    vector TIMER7_TRG_CMT_IRQHandler
#endif
    vector TIMER7_Channel_IRQHandler
#if defined(GD32F10X_CL)
    .word 0
    vector EXMC_IRQHandler
    .word 0
#else
    vector ADC2_IRQHandler
    vector EXMC_IRQHandler
    vector SDIO_IRQHandler
#endif
    vector TIMER4_IRQHandler
    vector SPI2_IRQHandler
    vector UART3_IRQHandler
    vector UART4_IRQHandler
    vector TIMER5_IRQHandler
    vector TIMER6_IRQHandler
    vector DMA1_Channel0_IRQHandler
    vector DMA1_Channel1_IRQHandler
    vector DMA1_Channel2_IRQHandler
#if defined(GD32F10X_CL)
    vector DMA1_Channel3_IRQHandler
    vector DMA1_Channel4_IRQHandler
    vector ENET_IRQHandler
    vector ENET_WKUP_IRQHandler
    vector CAN1_TX_IRQHandler
    vector CAN1_RX0_IRQHandler
    vector CAN1_RX1_IRQHandler
    vector CAN1_EWMC_IRQHandler
    vector USBFS_IRQHandler
#else
    vector DMA1_Channel3_4_IRQHandler
#endif
#endif
.size g_pfnVectors, .-g_pfnVectors
