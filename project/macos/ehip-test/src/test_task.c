/**
 * @file test_task.c
 * @brief 测试任务
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-12-20
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh.h>
#include <eh_sleep.h>
#include <eh_module.h>
#include <eh_debug.h>

static eh_task_t* test_task = NULL;
static bool is_exit = false;
int test_task_function(void* arg){
    (void) arg;
    int i = 0;
    for(i = 0; i < 10; i++){
        __await eh_usleep(1000*1000*2);
        eh_infofl("test_task_function %d", i);
        if(is_exit)
            break;
    }
    return 0;
}

static int __init test_task_init(void){
    test_task = eh_task_create("test_task", 0, 12*1024, NULL, test_task_function);
    if(test_task == NULL){
        return -1;
    }
    return 0;
}

static void __exit test_task_exit(void){
    is_exit = true;
    eh_task_join(test_task, NULL, EH_TIME_FOREVER);
}

eh_module_level8_export(test_task_init, test_task_exit);
