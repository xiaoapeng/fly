/**
 * @file ehip_buffer.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-13
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#include <stdint.h>
#include <string.h>

#include "eh.h"
#include "eh_debug.h"
#include "eh_mem_pool.h"
#include "eh_error.h"

#include "ehip_module.h"
#include "ehip_buffer.h"
#include "ehip_conf.h"

#define EHIP_BUFFER_REF_MAX_NUM UINT16_MAX

static eh_mem_pool_t pool_tab[EHIP_BUFFER_TYPE_MAX];
static eh_mem_pool_t pool_ehip_buffer_ref;
static eh_mem_pool_t pool_ehip_buffer;
struct ehip_pool_info {
    uint16_t    size;
    uint16_t    num;
    uint16_t    align;
};
static const struct ehip_pool_info ehip_pool_info_tab[EHIP_BUFFER_TYPE_MAX] = {
    [EHIP_BUFFER_TYPE_ETHERNET_FRAME] = {EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_SIZE, EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_NUM, EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_ALIGN}
};

void ehip_buffer_free(ehip_buffer_t* buf){
    if(buf->buffer_ref->ref_cnt > 0){
        buf->buffer_ref->ref_cnt--;
    }else{
        eh_warnfl("ref_cnt == 0");
    }
    if(buf->buffer_ref->ref_cnt == 0){
        eh_mem_pool_free(pool_tab[buf->buffer_ref->type], buf->buffer_ref->buffer);
        eh_mem_pool_free(pool_ehip_buffer_ref, buf->buffer_ref);
    }
    eh_mem_pool_free(pool_ehip_buffer, buf);
}

ehip_buffer_t* ehip_buffer_new(enum ehip_buffer_type type){
    ehip_buffer_t *buf;
    struct ehip_buffer_ref *buf_ref;
    int ret;
    if((uint32_t)type >= EHIP_BUFFER_TYPE_MAX)
        return eh_error_to_ptr(EH_RET_INVALID_PARAM);
    
    buf = eh_mem_pool_alloc(pool_ehip_buffer);
    if(buf == NULL){
        eh_errfl("type: pool_ehip_buffer alloc fail");
        return eh_error_to_ptr(EH_RET_MEM_POOL_EMPTY);
    }
    
    buf_ref = eh_mem_pool_alloc(pool_ehip_buffer_ref);
    if(buf_ref == NULL){
        eh_errfl("type: pool_ehip_buffer_ref alloc fail");
        ret = EH_RET_MEM_POOL_EMPTY;
        goto eh_mem_pool_alloc_buffer_ref_fail;
    }
    
    buf_ref->buffer = eh_mem_pool_alloc(pool_tab[type]);
    if(buf_ref->buffer == NULL){
        eh_errfl("type: %d alloc fail", type);
        ret = EH_RET_MEM_POOL_EMPTY;
        goto eh_mem_pool_alloc_buffer_fail;
    }

    buf->buffer_ref = buf_ref;
    buf->payload_pos = 0;
    buf->payload_tail = 0;
    buf_ref->buffer = buf_ref->buffer;
    buf_ref->ref_cnt = 1;
    buf_ref->type = type;
    buf_ref->buffer_size = ehip_pool_info_tab[type].size;
    
    return buf;
eh_mem_pool_alloc_buffer_fail:
    eh_mem_pool_free(pool_ehip_buffer_ref, buf_ref);
eh_mem_pool_alloc_buffer_ref_fail:
    eh_mem_pool_free(pool_ehip_buffer, buf);
    return eh_error_to_ptr(ret);;
}

ehip_buffer_t* ehip_buffer_dup(ehip_buffer_t* src){
    ehip_buffer_t* new_buffer = ehip_buffer_new(src->buffer_ref->type);
    if(eh_ptr_to_error(new_buffer) < 0)
        return new_buffer;
    new_buffer->payload_pos = src->payload_pos;
    new_buffer->payload_tail = src->payload_tail;
    memcpy(ehip_buffer_get_payload_ptr(new_buffer), ehip_buffer_get_payload_ptr(src), ehip_buffer_get_payload_size(src));
    return new_buffer;
}

ehip_buffer_t* ehip_buffer_ref_dup(ehip_buffer_t* buf){
    ehip_buffer_t* new_buffer;
    
    if(buf->buffer_ref->ref_cnt == EHIP_BUFFER_REF_MAX_NUM)
        return eh_error_to_ptr(EH_RET_INVALID_STATE);
    new_buffer = eh_mem_pool_alloc(pool_ehip_buffer);
    if(new_buffer == NULL)
        return eh_error_to_ptr(EH_RET_MEM_POOL_EMPTY);
    buf->buffer_ref->ref_cnt++;
    *new_buffer = *buf;
    return new_buffer;
}

static __init int ehip_buffer_init(void){
    const struct ehip_pool_info *info;
    int i=0;
    int ret;
    int sum_num = 0;

    for(i = 0; i < EHIP_BUFFER_TYPE_MAX; i++){
        info = &ehip_pool_info_tab[i];
        sum_num += info->num;
        pool_tab[i] = eh_mem_pool_create(info->align, info->size, info->num);
        ret = eh_ptr_to_error(pool_tab[i]);
        if(ret < 0)
            goto eh_mem_pool_create_fail;
    }

    pool_ehip_buffer_ref = eh_mem_pool_create(1, sizeof(struct ehip_buffer_ref), sum_num);
    ret = eh_ptr_to_error(pool_ehip_buffer_ref);
    if(ret < 0)
        goto create_ehip_buffer_ref_pool_fail;
    pool_ehip_buffer = eh_mem_pool_create(1, sizeof(struct ehip_buffer), EHIP_NETDEV_POLL_BUFFER_MAX_NUM);
    ret = eh_ptr_to_error(pool_ehip_buffer);
    if(ret < 0)
        goto create_ehip_buffer_fail;

    return 0;

create_ehip_buffer_fail:
    eh_mem_pool_destroy(pool_ehip_buffer_ref);
create_ehip_buffer_ref_pool_fail:
eh_mem_pool_create_fail:
    while(i--)
        eh_mem_pool_destroy(pool_tab[i]);
    return ret;
}

static __exit void ehip_buffer_exit(void){
    eh_mem_pool_destroy(pool_ehip_buffer);
    for(int i = 0; i < EHIP_BUFFER_TYPE_MAX; i++){
        eh_mem_pool_destroy(pool_tab[i]);
    }
}

ehip_preinit_module_export(ehip_buffer_init, ehip_buffer_exit);
