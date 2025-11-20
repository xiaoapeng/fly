/**
 * @file main.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-03
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */
#include <stdio.h>
#include <eh.h>
#include <eh_debug.h>
#include <eh_signal.h>
int main(void){
    int ret;
    ret = eh_global_init();
    if(ret < 0){
        eh_errfl("eh_global_init failed, ret = %d", ret);
        return ret;
    }
    eh_signal_dispatch_loop();
    eh_global_exit();
    return 0;
}