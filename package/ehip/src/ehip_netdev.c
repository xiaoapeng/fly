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
#include <ehip_core.h>

struct eh_list_head s_netdev_head = EH_LIST_HEAD_INIT(s_netdev_head);

ehip_netdev_t* ehip_netdev_register(enum ehip_netdev_type type, const struct ehip_netdev_param *param){
    ehip_netdev_t *netdev = NULL;
    int ret;
    if(type >= EHIP_NETDEV_TYPE_MAX)
        return eh_error_to_ptr(EH_RET_INVALID_PARAM);
    netdev = eh_malloc(sizeof(ehip_netdev_t) + ehip_netdev_trait_size_get(type));
    if(!netdev)
        return eh_error_to_ptr(EH_RET_MALLOC_ERROR);
    memset(netdev, 0, sizeof(ehip_netdev_t));
    netdev->param = param;
    netdev->type = type;
    
    eh_list_head_init(&netdev->protocol_handle_head);

    /* 初始化状态信号 */
    eh_signal_init(&netdev->signal_status);
    eh_event_flags_init(eh_signal_to_custom_event(&netdev->signal_status));
    ret = eh_signal_register(&netdev->signal_status);
    if(ret < 0)
        goto eh_signal_register_error;

    ret = _ehip_core_netdev_init(netdev);
    if(ret < 0){
        goto _ehip_core_netdev_init_error;
    }

    ehip_netdev_trait_reset(netdev);

    /* 添加到网络设备链表 */
    eh_list_add(&netdev->node, &s_netdev_head);

    return netdev;
eh_signal_register_error:
    _ehip_core_netdev_exit(netdev);
_ehip_core_netdev_init_error:
    eh_signal_unregister(&netdev->signal_status);
    /* eh_event_flags_clean、eh_signal_clean 无需调用 */
    eh_free(netdev);
    return eh_error_to_ptr(ret);
}

void ehip_netdev_unregister(ehip_netdev_t *netdev){
    eh_list_del(&netdev->node);
    _ehip_core_netdev_exit(netdev);
    eh_signal_unregister(&netdev->signal_status);
    eh_event_flags_clean(eh_signal_to_custom_event(&netdev->signal_status));
    eh_signal_clean(&netdev->signal_status);
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
    if(ret < 0) 
        return ret;
    
    ret = _ehip_core_netdev_up(netdev);
    if(ret < 0){
        goto _ehip_core_netdev_up_error;
    }

    if( netdev->param->ops && 
        netdev->param->ops->ndo_up && 
        (ret = netdev->param->ops->ndo_up(netdev)) < 0
    ){
        goto ndo_up_error;
    }

    eh_event_flags_set_bits(eh_signal_to_custom_event(&netdev->signal_status), EHIP_NETDEV_STATUS_UP);
    return EH_RET_OK;
ndo_up_error:
    _ehip_core_netdev_down(netdev);
_ehip_core_netdev_up_error:
    ehip_netdev_trait_down(netdev);
    return ret;
}

void ehip_netdev_down(ehip_netdev_t *netdev){
    eh_event_flags_clear_bits(eh_signal_to_custom_event(&netdev->signal_status), EHIP_NETDEV_STATUS_UP);

    if(netdev->param->ops && netdev->param->ops->ndo_down)
        netdev->param->ops->ndo_down(netdev);
    _ehip_core_netdev_down(netdev);
    ehip_netdev_trait_down(netdev);
}