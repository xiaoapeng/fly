/**
 * @file ehip_protocol_handle.c
 * @brief ptype 枚举中各种协议包处理
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-26
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh_error.h>
#include <eh_list.h>
#include <ehip_module.h>
#include <ehip_ptype.h>
#include <ehip_buffer.h>
#include <ehip_netdev.h>


#include <ehip_protocol_handle.h>

static struct  eh_list_head global_protocol_handle_head;

int ehip_protocol_handle_list_add_handle(struct  eh_list_head *head, struct ehip_protocol_handle *handle){
    struct ehip_protocol_handle *pos;
    eh_param_assert(handle);

    eh_list_for_each_entry(pos, head, node){
        if(pos->ptype > handle->ptype)
            break;
    }
    eh_list_add_tail(&handle->node, &pos->node);
    return 0;
}

int ehip_protocol_handle_list_handle(struct  eh_list_head *head, ehip_buffer_t *buf){
    struct ehip_protocol_handle *pos;
    ehip_buffer_t *ref_buf;
    int ret = 0;
    eh_list_for_each_entry(pos, head, node){
        if(pos->ptype < buf->protocol){
            continue;
        }if(pos->ptype == buf->protocol){
            ref_buf = ehip_buffer_ref_dup(buf);
            ret = eh_ptr_to_error(ref_buf);
            if(ret < 0)
                return ret;
            pos->handle(ref_buf);
            continue;
        }
        break;
    }
    return ret;
}

int ehip_protocol_handle_register(struct ehip_protocol_handle *handle){
    return ehip_protocol_handle_list_add_handle(&global_protocol_handle_head, handle);
}

void ehip_protocol_handle_unregister(struct ehip_protocol_handle *handle){
    eh_list_del(&handle->node);
}

int ehip_protocol_handle_dispatch(ehip_buffer_t *buf){
    int ret = 0;
    ret = ehip_protocol_handle_list_handle(&global_protocol_handle_head, buf);
    if(ret < 0)
        goto error;
    ret = ehip_protocol_handle_list_handle(&buf->netdev->protocol_handle_head, buf);
error:
    ehip_buffer_free(buf);
    return ret;
}

static int __init ehip_protocol_handle_init(void)
{
    eh_list_head_init(&global_protocol_handle_head);
    return 0;
}

ehip_preinit_module_export(ehip_protocol_handle_init, NULL);