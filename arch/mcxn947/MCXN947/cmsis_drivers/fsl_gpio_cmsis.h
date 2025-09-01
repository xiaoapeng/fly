/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FSL_GPIO_CMSIS_H_
#define FSL_GPIO_CMSIS_H_

#include "fsl_common.h"
#include "RTE_Device.h"
#include "Driver_GPIO.h"

#if defined(GPIO0) && defined(RTE_GPIO_PORT0) && RTE_GPIO_PORT0
extern ARM_DRIVER_GPIO Driver_GPIO_PORT0;
#endif

#if defined(GPIO1) && defined(RTE_GPIO_PORT1) && RTE_GPIO_PORT1
extern ARM_DRIVER_GPIO Driver_GPIO_PORT1;
#endif

#if defined(GPIO2) && defined(RTE_GPIO_PORT2) && RTE_GPIO_PORT2
extern ARM_DRIVER_GPIO Driver_GPIO_PORT2;
#endif

#if defined(GPIO3) && defined(RTE_GPIO_PORT3) && RTE_GPIO_PORT3
extern ARM_DRIVER_GPIO Driver_GPIO_PORT3;
#endif

#if defined(GPIO4) && defined(RTE_GPIO_PORT4) && RTE_GPIO_PORT4
extern ARM_DRIVER_GPIO Driver_GPIO_PORT4;
#endif

#if defined(GPIO5) && defined(RTE_GPIO_PORT5) && RTE_GPIO_PORT5
extern ARM_DRIVER_GPIO Driver_GPIO_PORT5;
#endif

#if defined(GPIOA) && defined(RTE_GPIO_PORTA) && RTE_GPIO_PORTA
extern ARM_DRIVER_GPIO Driver_GPIO_PORTA;
#endif

#if defined(GPIOB) && defined(RTE_GPIO_PORTB) && RTE_GPIO_PORTB
extern ARM_DRIVER_GPIO Driver_GPIO_PORTB;
#endif

#if defined(GPIOC) && defined(RTE_GPIO_PORTC) && RTE_GPIO_PORTC
extern ARM_DRIVER_GPIO Driver_GPIO_PORTC;
#endif

#if defined(GPIOD) && defined(RTE_GPIO_PORTD) && RTE_GPIO_PORTD
extern ARM_DRIVER_GPIO Driver_GPIO_PORTD;
#endif

#if defined(GPIOE) && defined(RTE_GPIO_PORTE) && RTE_GPIO_PORTE
extern ARM_DRIVER_GPIO Driver_GPIO_PORTE;
#endif
#endif
