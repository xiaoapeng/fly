/**
 * @file ethernet_v2.h
 * @brief 以太网V2帧结构与接口
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-31
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _ETHERNET_V2_H_
#define _ETHERNET_V2_H_

#include "ether.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

struct ethv2_hdr{
	struct eth_addr 	dest;
	struct eth_addr  	src;
    uint16_t type;
}eh_aligned(1);











#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _ETHERNET_V2_H_