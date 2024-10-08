/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_common.h"
#include "fsl_lpuart_cmsis.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Definition of peripheral ID */
#define LP_FLEXCOMM4_PERIPHERAL Driver_USART4
/* Definition of the clock source frequency */
#define LP_FLEXCOMM4_CLOCK_SOURCE_FREQ 12000000UL
/* Alias for GPIO0 peripheral */
#define GPIO0_GPIO GPIO0
/* Alias for PORT0 */
#define GPIO0_PORT PORT0
/* GPIO0 interrupt vector ID (number). */
#define GPIO0_INT_0_IRQN GPIO00_IRQn
/* GPIO0 interrupt vector priority. */
#define GPIO0_INT_0_IRQ_PRIORITY 0
/* GPIO0 interrupt handler identifier. */
#define GPIO0_INT_0_IRQHANDLER GPIO00_IRQHandler

/***********************************************************************************************************************
 * Global functions
 **********************************************************************************************************************/
/* Signal event function for component LP_FLEXCOMM4*/
extern void LPUART4_SignalEvent(uint32_t event);
/* Get clock source frequency function for component LP_FLEXCOMM4 */
uint32_t LPUART4_GetFreq(void);

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/

void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
