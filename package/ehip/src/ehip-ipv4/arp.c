/**
 * @file arp.c
 * @brief arp协议
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-13
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */


#include <string.h>

#include <eh.h>
#include <eh_debug.h>
#include <eh_types.h>
#include <eh_error.h>
#include <eh_swab.h>
#include <eh_signal.h>
#include <eh_timer.h>
#include <ehip_core.h>
#include <ehip-ipv4/route.h>
#include <ehip_netdev.h>
#include <ehip_module.h>
#include <ehip_buffer.h>
#include <ehip_netdev_type.h>
#include <ehip_protocol_handle.h>
#include <ehip_netdev_trait.h>
#include <ehip-ipv4/arp.h>
#include <ehip-ipv4/ip.h>

enum etharp_state{
    ARP_STATE_NUD_FAILED,                   /* 未使用状态，该状态邻居项无效 */
    // ARP_STATE_NUD_NONE,                     /* 刚建立状态，该状态邻居项无效 */
    ARP_STATE_NUD_INCOMPLETE,               /* 定时发送Solicitation请求，该状态邻居项无效 */
    ARP_STATE_NUD_STALE,                    /* 不新鲜了，随时会被垃圾回收,但若被使用，则会迁移到ARP_STATE_NUD_DELAY，该状态邻居项有效 */
    ARP_STATE_NUD_PROBE,                    /* delay_probe_time超时后，定时发送Solicitation请求，该状态邻居项有效 */
    ARP_STATE_NUD_DELAY,                    /* reachable_time超时但delay_probe_time未超时，当delay_probe_time超时时迁移到ARP_STATE_NUD_PROBE */
    ARP_STATE_NUD_REACHABLE                 /* 绝对可信状态，reachable_time超时后迁移出此状态 */
};

struct arp_entry arp_table[EHIP_ARP_CACHE_MAX_NUM];
EH_DEFINE_STATIC_CUSTOM_SIGNAL(signal_timer_1s, eh_event_timer_t, EH_TIMER_INIT(signal_timer_1s.custom_event));

static void arp_send_dst(uint16_t be_type, enum ehip_ptype ptype, ehip_netdev_t *netdev, 
    const ehip_hw_addr_t *s_hw_addr, const ehip_hw_addr_t *d_hw_addr, 
    ipv4_addr_t s_ipv4_addr, ipv4_addr_t d_ipv4_addr, const ehip_hw_addr_t *target_hw_addr);

static void slot_function_arp_1s_timer_handler(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    for(uint32_t i=0;i<EHIP_ARP_CACHE_MAX_NUM; i++){
        switch (arp_table[i].state){
            case ARP_STATE_NUD_FAILED:
                continue;
            case ARP_STATE_NUD_INCOMPLETE:
                if(arp_table[i].retry_cnt){
                    /* 进行发送报文进行ARP查询 */
                    ipv4_addr_t ipv4_addr = ipv4_netdev_get_addr(ehip_netdev_trait_ipv4_dev(arp_table[i].netdev));
                    if(ipv4_is_zeronet(ipv4_addr)){
                        arp_table[i].state = ARP_STATE_NUD_FAILED;
                        break;
                    }
                    arp_send_dst(
                        eh_hton16(ARPOP_REQUEST), EHIP_PTYPE_ETHERNET_ARP, arp_table[i].netdev, 
                        ehip_netdev_trait_broadcast_hw(arp_table[i].netdev), NULL,
                        ipv4_addr, arp_table[i].ip_addr, NULL );
                }
                arp_table[i].retry_cnt++;
                if(arp_table[i].retry_cnt > EHIP_ARP_MAX_RETRY_CNT)
                    arp_table[i].state = ARP_STATE_NUD_FAILED;
                break;
        }
    }
}

EH_DEFINE_SLOT(slot_timer, slot_function_arp_1s_timer_handler, NULL);





static void arp_send_dst(uint16_t be_type, enum ehip_ptype ptype, ehip_netdev_t *netdev, 
    const ehip_hw_addr_t *s_hw_addr, const ehip_hw_addr_t *d_hw_addr, 
    ipv4_addr_t s_ipv4_addr, ipv4_addr_t d_ipv4_addr, const ehip_hw_addr_t *target_hw_addr){
    int ret;
    struct ehip_buffer* newbuf;
    struct arp_hdr *arp_hdr;
    uint8_t *pos;
    if(s_hw_addr == NULL || netdev) return ;
    newbuf = ehip_buffer_new(netdev->attr.buffer_type, netdev->attr.hw_head_size);
    if(eh_ptr_to_error(newbuf) < 0)
        return ;
    newbuf->netdev = netdev;
    arp_hdr = (struct arp_hdr*)ehip_buffer_payload_append(newbuf, (ehip_buffer_size_t)arp_hdr_len(netdev));
    if(arp_hdr == NULL)
        goto error;
    if(target_hw_addr == NULL && (target_hw_addr = ehip_netdev_trait_broadcast_hw(netdev)) == NULL)
        return ;
    ret = ehip_netdev_trait_hard_header(netdev, newbuf, s_hw_addr, target_hw_addr, ptype, (ehip_buffer_size_t)arp_hdr_len(netdev));
    if(ret < 0)
        goto error;
    
    arp_hdr->ar_hrd = eh_hton16(netdev->type);
    arp_hdr->ar_pro = eh_hton16(EHIP_ETH_P_IP);

    arp_hdr->ar_op = be_type;
    arp_hdr->ar_hln = netdev->attr.hw_addr_len;
    arp_hdr->ar_pln = 4;

    pos = (uint8_t*)(arp_hdr + 1);
    memcpy(pos, s_hw_addr, netdev->attr.hw_addr_len);
    pos += netdev->attr.hw_addr_len;
    memcpy(pos, &s_ipv4_addr, 4);
    pos += 4;
    if(d_hw_addr){
        memcpy(pos, d_hw_addr, netdev->attr.hw_addr_len);
    }else{
        memset(pos, 0, netdev->attr.hw_addr_len);
    }
    pos += netdev->attr.hw_addr_len;
    memcpy(pos, &d_ipv4_addr, 4);

    ehip_queue_tx(newbuf);

    return ;
error:
    ehip_buffer_free(newbuf);
}
static void arp_handle(struct ehip_buffer* buf){
    const struct arp_hdr *arp_hdr;
    const char *arp_pos;
    const ehip_hw_addr_t *s_hw_addr, *d_hw_addr;
    ipv4_addr_t s_ipv4_addr, d_ipv4_addr;
    
    if(ehip_buffer_get_payload_size(buf) < arp_hdr_len(buf->netdev)){
        goto drop;
    }
    arp_hdr = (const struct arp_hdr *)ehip_buffer_get_payload_ptr(buf);
    if(arp_hdr->ar_hln != buf->netdev->attr.hw_addr_len || arp_hdr->ar_pln != 4){
        goto drop;
    }
    
    /* 处理 arp */
    switch (buf->netdev->type) {
        case EHIP_NETDEV_TYPE_ETHERNET:
            if( (arp_hdr->ar_hrd != eh_hton16(EHIP_ARP_ETHER) && 
                arp_hdr->ar_hrd != eh_hton16(EHIP_ARP_IEEE802) ) ||
                arp_hdr->ar_pro != eh_hton16(EHIP_ETH_P_IP)
               )
                goto drop;
            break;
        default: 
            goto drop;
    }

    if(arp_hdr->ar_op != eh_hton16(ARPOP_REPLY) && arp_hdr->ar_op != eh_hton16(ARPOP_REQUEST)) 
        goto drop;

    /* s_hw */
    arp_pos = (const char *)(arp_hdr + 1);
    s_hw_addr = (const ehip_hw_addr_t *)arp_pos;
    
    /* s_ip */
    arp_pos += arp_hdr->ar_hln;
    memcpy(&s_ipv4_addr, arp_pos, sizeof(ipv4_addr_t));

    /* d_hw */
    arp_pos += sizeof(ipv4_addr_t);
    d_hw_addr = (const ehip_hw_addr_t *)arp_pos;
    
    /* d_ip */
    arp_pos += arp_hdr->ar_hln;
    memcpy(&d_ipv4_addr, arp_pos, sizeof(ipv4_addr_t));

    eh_debugfl("ar_hrd: %04hx", arp_hdr->ar_hrd);
    eh_debugfl("ar_pro: %04hx", arp_hdr->ar_pro);
    eh_debugfl("ar_hln: %02hhx", arp_hdr->ar_hln);
    eh_debugfl("ar_pln: %02hhx", arp_hdr->ar_pln);
    eh_debugfl("ar_op: %04hx", arp_hdr->ar_op);
    eh_debugfl("s_hw: %.*hhq", arp_hdr->ar_hln, s_hw_addr);
    eh_debugfl("s_ip: %d.%d.%d.%d", 
        ipv4_addr_to_dec0(s_ipv4_addr), ipv4_addr_to_dec1(s_ipv4_addr),
        ipv4_addr_to_dec2(s_ipv4_addr), ipv4_addr_to_dec3(s_ipv4_addr));
    eh_debugfl("d_hw: %.*hhq", arp_hdr->ar_hln, d_hw_addr);
    eh_debugfl("d_ip: %d.%d.%d.%d", 
        ipv4_addr_to_dec0(d_ipv4_addr), ipv4_addr_to_dec1(d_ipv4_addr),
        ipv4_addr_to_dec2(d_ipv4_addr), ipv4_addr_to_dec3(d_ipv4_addr));

	if(eh_unlikely(ipv4_is_multicast(d_ipv4_addr)))
        goto drop;

    /* 忽略免费ARP */
    if( s_ipv4_addr == d_ipv4_addr )
        goto drop;

    /* 
     * 查路由表看是否接受这个arp包 d_ipv4_addr 是否接受 
     * ARP协议不接受任何转发
     */
    /* 请求 */
    if( arp_hdr->ar_op == eh_hton16(ARPOP_REQUEST) && 
        ( ( s_ipv4_addr && ipv4_route_input(s_ipv4_addr, d_ipv4_addr, buf->netdev, NULL) == ROUTE_TABLE_LOCAL_SELF) ||
          ( s_ipv4_addr == 0 && ipv4_netdev_is_ipv4_addr_valid(ehip_netdev_trait_ipv4_dev(buf->netdev), d_ipv4_addr))
        )
    ){
        arp_send_dst(eh_hton16(ARPOP_REPLY), (uint16_t)EHIP_PTYPE_ETHERNET_ARP, buf->netdev, 
            ehip_netdev_trait_broadcast_hw(buf->netdev), s_hw_addr, d_ipv4_addr, s_ipv4_addr, s_hw_addr);
        goto consume;
    }

    /* 回复 */
    // if( arp_hdr->ar_op == eh_hton16(ARPOP_REPLY) && )

consume:
drop:
    ehip_buffer_free(buf);
}

static struct ehip_protocol_handle arp_protocol_handle = {
    .ptype = EHIP_PTYPE_ETHERNET_ARP,
    .handle = arp_handle,
    .node = EH_LIST_HEAD_INIT(arp_protocol_handle.node),
};


static int __init arp_protocol_parser_init(void){
    int ret;

    eh_timer_advanced_init(eh_signal_to_custom_event(&signal_timer_1s), (eh_sclock_t)eh_msec_to_clock(1000*1), EH_TIMER_ATTR_AUTO_CIRCULATION);
    ret = eh_signal_register(&signal_timer_1s);
    if(ret < 0) return ret;
    eh_signal_slot_connect(&signal_timer_1s, &slot_timer);
    eh_timer_start(eh_signal_to_custom_event(&signal_timer_1s));
    ehip_protocol_handle_register(&arp_protocol_handle);
    return 0;
}

static void __exit arp_protocol_parser_exit(void){
    ehip_protocol_handle_unregister(&arp_protocol_handle);
    eh_timer_stop(eh_signal_to_custom_event(&signal_timer_1s));
    eh_signal_slot_disconnect(&slot_timer);
    eh_signal_unregister(&signal_timer_1s);
}




ehip_protocol_module_export(arp_protocol_parser_init, arp_protocol_parser_exit);