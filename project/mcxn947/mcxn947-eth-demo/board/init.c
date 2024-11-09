/**
 * @file init.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-06-28
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */



#include "clock_cnt.h"
#include "clock_config.h"
#include "eh.h"
#include "eh_user_config.h"

extern int board_debug_init(void);




void init(void){
    BOARD_InitBootClocks();
    board_debug_init();
}