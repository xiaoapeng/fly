/**
 * @file ehip_netdev.c
 * @brief 网络设备接口层,网络数据输入输出
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * */
#include <string.h>
#include <eh.h>
#include <eh_list.h>
#include <eh_mem.h>
#include <eh_error.h>
#include <eh_signal.h>
#include <eh_event_flags.h>
#include <ehip_module.h>
#include <ehip_netdev.h>
#include <ehip_netdev_trait.h>
#include <ehip_protocol_handle.h>

struct eh_list_head s_netdev_head = EH_LIST_HEAD_INIT(s_netdev_head);

ehip_netdev_t* ehip_netdev_register(enum ehip_netdev_type type, const struct ehip_netdev_param *param){
    ehip_netdev_t *netdev = NULL;
    if(type >= EHIP_NETDEV_TYPE_MAX)
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


ehip_netdev_t * ehip_netdev_find(char *netdev_name){
    ehip_netdev_t *pos;
    eh_list_for_each_entry(pos, &s_netdev_head, node){
        if(strcmp(pos->param->name, netdev_name) == 0)
            return pos;
    }
    return NULL;
}


int ehip_netdev_up(ehip_netdev_t *netdev){
    int ret;
    ret = ehip_netdev_trait_up(netdev);
    if(ret < 0) return ret;
    if( netdev->param->ops && 
        netdev->param->ops->ndo_up && 
        (ret = netdev->param->ops->ndo_up(netdev)) < 0
    ){
        ehip_netdev_trait_down(netdev);
        return ret;
    }
    eh_event_flags_set_bits(eh_signal_to_custom_event(&netdev->signal), EHIP_NETDEV_STATUS_UP);
    return EH_RET_OK;
}

void ehip_netdev_down(ehip_netdev_t *netdev){
    if(netdev->param->ops && netdev->param->ops->ndo_down)
        netdev->param->ops->ndo_down(netdev);
    ehip_netdev_trait_down(netdev);
    eh_event_flags_clear_bits(eh_signal_to_custom_event(&netdev->signal), EHIP_NETDEV_STATUS_UP);
}