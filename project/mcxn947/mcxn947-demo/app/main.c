/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "fsl_clock.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "eh.h"
#include "eh_debug.h"
#include "eh_co.h"
#include "pfifo.h"

extern void init(void);

extern  int task_main(void);


int main(void)
{
    init();
    
    eh_global_init();
    task_main();

    eh_global_exit();

    while(1){    }
    return 0;
}
