/**
 * @file ethernet_v2.c
 * @brief 以太网V2帧处理
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-30
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#include <eh.h>
#include <eh_list.h>
#include <eh_debug.h>
#include <ehip_module.h>
#include <ehip_buffer.h>
#include <ehip_protocol_handle.h>
#include <ehip-mac/ethernet_v2.h>

static void ethernet_v2_handle(struct ehip_buffer* buf){
    struct eth_hdr *hdr = (struct eth_hdr *)ehip_buffer_get_payload_ptr(buf);
    ehip_buffer_size_t payload_size = ehip_buffer_get_payload_size(buf);
    if(payload_size < sizeof(struct eth_hdr))
        return;
    eh_debugfl("dst mac: %02x:%02x:%02x:%02x:%02x:%02x", hdr->dest.addr[0], hdr->dest.addr[1], hdr->dest.addr[2], hdr->dest.addr[3], hdr->dest.addr[4], hdr->dest.addr[5]);
    eh_debugfl("src mac: %02x:%02x:%02x:%02x:%02x:%02x", hdr->src.addr[0], hdr->src.addr[1], hdr->src.addr[2], hdr->src.addr[3], hdr->src.addr[4], hdr->src.addr[5]);
    eh_debugfl("ether type: %04x", eh_ntoh16(hdr->type_or_len));
    eh_debugfl("rx buf size %d", ehip_buffer_get_payload_size(buf));
    eh_debugfl("rx data :|%.*hhq|", ehip_buffer_get_payload_size(buf), ehip_buffer_get_payload_ptr(buf));
    
    ehip_buffer_free(buf);
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