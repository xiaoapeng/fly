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
#include <eh_error.h>
#include <eh_types.h>
#include <eh_list.h>
#include <eh_debug.h>
#include <ehip_core.h>
#include <ehip_module.h>
#include <ehip_buffer.h>
#include <ehip_protocol_handle.h>
#include <ehip_netdev_trait.h>
#include <ehip-mac/ethernet.h>
#include <ehip-mac/ethernet_v2.h>

static void ethernet_v2_handle(struct ehip_buffer* buf){
    struct eth_hdr *hdr = (struct eth_hdr *)ehip_buffer_get_payload_ptr(buf);
    const ehip_eth_addr_t *if_mac = (const ehip_eth_addr_t *)ehip_netdev_trait_get_hw_addr(buf->netdev);
    ehip_buffer_size_t payload_size = ehip_buffer_get_payload_size(buf);
    if(payload_size < sizeof(struct eth_hdr) || eh_ptr_to_error((void*)if_mac) < 0)
        goto drop;
    
    eh_debugfl("dst mac: %02x:%02x:%02x:%02x:%02x:%02x", hdr->dest.addr[0], hdr->dest.addr[1], hdr->dest.addr[2], hdr->dest.addr[3], hdr->dest.addr[4], hdr->dest.addr[5]);
    eh_debugfl("src mac: %02x:%02x:%02x:%02x:%02x:%02x", hdr->src.addr[0], hdr->src.addr[1], hdr->src.addr[2], hdr->src.addr[3], hdr->src.addr[4], hdr->src.addr[5]);
    eh_debugfl("ether type: %04x", eh_ntoh16(hdr->type_or_len));
    eh_debugfl("rx buf size %d", ehip_buffer_get_payload_size(buf));
    eh_debugfl("rx data :|%.*hhq|", ehip_buffer_get_payload_size(buf), ehip_buffer_get_payload_ptr(buf));
    
    if(eh_likely(eth_addr_equal_64bits(&hdr->dest, if_mac))){
        buf->packet_type = EHIP_PACKET_TYPE_HOST;
    }else if(eh_unlikely(eth_is_multicast_ether_addr_64bits(&hdr->dest))){
        if(eth_addr_equal_64bits(&hdr->dest, EHIP_ETH_MAC_ADDR_BROADCAST)){
            buf->packet_type = EHIP_PACKET_TYPE_BROADCAST;
        }else{
            buf->packet_type = EHIP_PACKET_TYPE_MCASTLOOP;
        }
    }else{
        buf->packet_type = EHIP_PACKET_TYPE_OTHERHOST;
    }

    /* 更新下一层协议 */
    buf->protocol = hdr->type_or_len;
    ehip_buffer_head_reduce(buf, sizeof(struct eth_hdr));
    ehip_rx(buf);
    return ;
drop:
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