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

#include "eh.h"
#include "eh_mem_pool.h"
#include "eh_module.h"
#include "eh_error.h"

#include "ehip_buffer.h"
#include "ehip_conf.h"


struct _ehip_buffer{
    struct ehip_buffer  base;
    uint16_t            ref_cnt;
};

static eh_mem_pool_t pool_tab[EHIP_BUFFER_TYPE_MAX];

struct ehip_pool_info {
    uint16_t    size;
    uint16_t    num;
    uint16_t    align;
};
static const struct ehip_pool_info ehip_pool_info_tab[EHIP_BUFFER_TYPE_MAX] = {
    [EHIP_BUFFER_TYPE_ETHERNET_FRAME] = {EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_SIZE, EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_NUM, EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_ALIGN}
};

static __init int ehip_buffer_init(void){
    const struct ehip_pool_info *info;
    int i=0;
    int ret;
    for(i = 0; i < EHIP_BUFFER_TYPE_MAX; i++){
        info = &ehip_pool_info_tab[i];
        pool_tab[i] = eh_mem_pool_create(info->align, info->size, info->num);
        ret = eh_ptr_to_error(pool_tab[i]);
        if(ret < 0)
            goto error;
    }
    return 0;
error:
    while(i--)
        eh_mem_pool_destroy(pool_tab[i]);
    return ret;
}

static __exit void ehip_buffer_exit(void){
    for(int i = 0; i < EHIP_BUFFER_TYPE_MAX; i++){
        eh_mem_pool_destroy(pool_tab[i]);
    }
}

eh_interior_module_export(ehip_buffer_init, ehip_buffer_exit);
