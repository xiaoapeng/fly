/**
 * @file ethernet_v2.c
 * @brief 以太网V2帧处理
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-30
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#include "eh_list.h"
#include "ehip_module.h"
#include "ehip_protocol_handle.h"
#include "ehip-mac/ethernet_v2.h"

static void ethernet_v2_handle(struct ehip_buffer* buf){
    
}

static struct ehip_protocol_handle ethernet_v2_protocol_handle = {
    .ptype = EHIP_PTYPE_ETHERNET_II_FRAME,
    .handle = ethernet_v2_handle,
    .node = EH_LIST_HEAD_INIT(ethernet_v2_protocol_handle.node),
};

static int __init ethernet_v2_protocol_parser_init(void){
    ehip_protocol_handle_register(&ethernet_v2_protocol_handle);
    return 0;
}

static void __exit ethernet_v2_protocol_parser_exit(void){
    ehip_protocol_handle_unregister(&ethernet_v2_protocol_handle);
}

ehip_protocol_module_export(ethernet_v2_protocol_parser_init, ethernet_v2_protocol_parser_exit);