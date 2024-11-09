/**
 * @file main.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-08-08
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include "fsl_clock.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "eh.h"
#include "eh_debug.h"
#include "eh_co.h"
#include "pfifo.h"
#include "autoconf.h"
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
