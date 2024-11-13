/**
 * @file ether.h
 * @brief 以太网层数据链路层部分定义
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-30
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _ETHER_H_
#define _ETHER_H_

#include <stdint.h>
#include <stdbool.h>
#include <ehip_ptype.h>
#include <eh_swab.h>
#include "hw_addr.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#define EHIP_ETH_HWADDR_LEN                6

typedef struct ehip_general_hw_addr ehip_eth_addr_t;

struct eth_hdr{
    ehip_eth_addr_t         dest;
    ehip_eth_addr_t         src;
    uint16_t                type_or_len;
}eh_aligned(1);

extern const ehip_eth_addr_t ehip_eth_mac_addr_broadcast;

#define EHIP_ETH_FRAME_MAX_LEN          (sizeof(struct eth_hdr) + 1500U + 4U) /* 1518:  1500 bytes payload + 14 bytes header + 4 bytes CRC*/
#define EHIP_ETH_FRAME_MIN_LEN          (sizeof(struct eth_hdr) + 46U + 4U)   /* 64:      46 bytes payload + 14 bytes header + 4 bytes CRC*/

#define EHIP_ETH_MAC_ADDR_BROADCAST  (&ehip_eth_mac_addr_broadcast)

/**
 * @brief                   根据以太网头判断以太网帧类型
 * @param  eth_hdr          以太网头、含以太网帧类型
 * @return enum ehip_ptype 
 */
extern enum ehip_ptype eth_hdr_ptype_get(struct eth_hdr *eth_hdr);


/**
 * @brief                   判断两个以太网地址是否相等
 * @param  addr1            
 * @param  addr2            
 * @return true 相等
 */
static inline bool eth_addr_equal_64bits(const ehip_eth_addr_t *addr1, const ehip_eth_addr_t *addr2)
{
	uint64_t fold = (*(const uint64_t *)addr1) ^ (*(const uint64_t *)addr2);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return (fold >> 16) == 0;
#else
	return (fold << 16) == 0;
#endif
}


static inline bool eth_is_multicast_ether_addr_64bits(const ehip_eth_addr_t *addr)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return 0x01 & ((*(const u64 *)addr) >> 56);
#else
	return 0x01 & (*(const uint64_t *)addr);
#endif
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _ETHER_H_