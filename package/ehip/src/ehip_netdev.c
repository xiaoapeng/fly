/**
 * @file ehip_netdev.c
 * @brief 网络设备接口层,网络数据输入输出
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */
#include <string.h>
#include "eh.h"
#include "eh_list.h"
#include "eh_mem.h"
#include "eh_error.h"
#include "eh_signal.h"
#include "eh_event_flags.h"
#include "ehip_module.h"
#include "ehip_netdev.h"
#include "ehip_netdev_trait.h"
#include "ehip_protocol_handle.h"

struct eh_list_head s_netdev_head;

ehip_netdev_t* ehip_netdev_register(enum ehip_netdev_type type, struct ehip_netdev_param *param){
    ehip_netdev_t *netdev = NULL;
    if(type < EHIP_NETDEV_TYPE_MAX)
        return eh_error_to_ptr(EH_RET_INVALID_PARAM);
    netdev = eh_malloc(sizeof(ehip_netdev_t) + ehip_netdev_trait_size_get(type));
    if(!netdev)
        return eh_error_to_ptr(EH_RET_MALLOC_ERROR);
    memset(netdev, 0, sizeof(ehip_netdev_t));
    netdev->param = param;
    netdev->type = type;
    eh_list_add(&netdev->node, &s_netdev_head);
    eh_list_head_init(&netdev->protocol_handle_head);
    eh_signal_init(&netdev->signal);
    eh_event_flags_init(eh_signal_to_custom_event(&netdev->signal));
    eh_signal_register(&netdev->signal);
    ehip_netdev_trait_reset(netdev);
    return netdev;
}

void ehip_netdev_unregister(ehip_netdev_t *netdev){
    eh_signal_unregister(&netdev->signal);
    eh_event_flags_clean(eh_signal_to_custom_event(&netdev->signal));
    eh_signal_clean(&netdev->signal);
    eh_list_del(&netdev->protocol_handle_head);
    eh_list_del(&netdev->node);
    eh_free(netdev);
}


int ehip_netdev_protocol_handle_register(ehip_netdev_t *netdev, struct ehip_protocol_handle *handle){
    extern int ehip_protocol_handle_list_add_handle(struct  eh_list_head *head, struct ehip_protocol_handle *handle);
    return ehip_protocol_handle_list_add_handle(&netdev->protocol_handle_head, handle);
}

void ehip_netdev_protocol_handle_unregister(ehip_netdev_t *netdev, struct ehip_protocol_handle *handle){
    (void) netdev;
    ehip_protocol_handle_unregister(handle);
}

static int __init ehip_netdev_init(void){
    eh_list_head_init(&s_netdev_head);
    return 0;
}

static void __exit ehip_netdev_exit(void){
    eh_list_del_init(&s_netdev_head);
}

ehip_preinit_module_export(ehip_netdev_init, ehip_netdev_exit);
