/**
 * @file ehip_core.c
 * @brief ehip 核心程序
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#include "eh.h"
#include "eh_list.h"
#include "eh_module.h"

#include "ehip_netdev.h"

struct eh_list_head s_netdev_head;

struct ehip_netdev{
    struct eh_list_head node;
    struct ehip_netdev_param *param;
};

static int __init eh_ehipcore_init(void){
    eh_list_head_init(&s_netdev_head);

    return 0;
}

static void __exit eh_ehipcore_exit(void){

    eh_list_del_init(&s_netdev_head);
}

eh_ehipcore_module_export(eh_ehipcore_init, eh_ehipcore_exit);