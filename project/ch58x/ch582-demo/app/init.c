/**
 * @file init.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-09-02
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#include "CH58x_common.h"
#include "core_riscv.h"

int init(void){
    
    SetSysClock(CLK_SOURCE_PLL_60MHz);
    return 0;
}
