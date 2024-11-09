/**
 * @file main.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-08-08
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include "eh_mem.h"
#include "fsl_clock.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "eh.h"
#include "eh_debug.h"
#include "eh_co.h"
#include "autoconf.h"
extern void init(void);

extern  int task_main(void);


int main(void)
{
    init();
    
    EH_DBG_ERROR_EXEC(eh_global_init() != 0 , goto error);
    
    task_main();

    eh_global_exit();
    
    eh_debugfl("exit!!");
    
error:
    while(1){    }
    return 0;
}
