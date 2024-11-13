/**
 * @file ethernet.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#include <stdbool.h>
#include <eh_swab.h>
#include <ehip-mac/ethernet.h>

const ehip_eth_addr_t ehip_eth_mac_addr_broadcast = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

static inline bool eth_proto_is_ethernet_v2(uint16_t proto)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	/* if CPU is little endian mask off bits representing LSB */
	proto &= eh_hton16(0xFF00);
#endif
	/* cast both to u16 and compare since LSB can be ignored */
	return (uint16_t)proto >= (uint16_t)eh_hton16(EHIP_ETH_P_ETHERNET_II_MIN);
}

enum ehip_ptype eth_hdr_ptype_get(struct eth_hdr *eth_hdr){
    if(eh_likely(eth_proto_is_ethernet_v2(eth_hdr->type_or_len))){
        return EHIP_PTYPE_ETHERNET_II_FRAME;
    }
    return EHIP_PTYPE_UNKNOWN;
}