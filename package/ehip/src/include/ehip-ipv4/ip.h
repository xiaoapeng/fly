/**
 * @file ip.h
 * @brief  ipv4基本定义
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _IPV4_H_
#define _IPV4_H_

#include <stdint.h>
#include <stdbool.h>
#include <eh_swab.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef uint32_t ipv4_addr_t;

#define	IPV4_ADDR_BROADCAST	            0xffffffffU
#define IPV4_ADDR_ANY                   0x00000000U
#define	IPV4_ADDR_NONE		            0xffffffffU
#define	IPV4_ADDR_DUMMY		            0xc0000008U
#define	IPV4_ADDR_LOOPBACKNET		    127
#define	IPV4_ADDR_LOOPBACK		        0x7f000001U	/* 127.0.0.1   */

#define IPV4_ADDR_UNSPEC_GROUP		    0xe0000000U	/* 224.0.0.0   */
#define IPV4_ADDR_ALLHOSTS_GROUP		0xe0000001U	/* 224.0.0.1   */
#define IPV4_ADDR_ALLRTRS_GROUP		    0xe0000002U	/* 224.0.0.2 */
#define IPV4_ADDR_ALLSNOOPERS_GROUP	    0xe000006aU	/* 224.0.0.106 */
#define IPV4_ADDR_MAX_LOCAL_GROUP		0xe00000ffU	/* 224.0.0.255 */



static inline bool ipv4_is_multicast(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xf0000000)) == eh_hton32(0xe0000000);
}

static inline bool ipv4_is_loopback(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xff000000)) == eh_hton32(0x7f000000);
}

static inline bool ipv4_is_local_multicast(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xffffff00)) == eh_hton32(0xe0000000);
}

static inline bool ipv4_is_lbcast(ipv4_addr_t addr)
{
	/* limited broadcast */
	return addr == eh_hton32(IPV4_ADDR_BROADCAST);
}

static inline bool ipv4_is_all_snoopers(ipv4_addr_t addr)
{
	return addr == eh_hton32(IPV4_ADDR_ALLSNOOPERS_GROUP);
}

static inline bool ipv4_is_zeronet(ipv4_addr_t addr)
{
	return (addr == 0);
}

/* Special-Use IPv4 Addresses (RFC3330) */

static inline bool ipv4_is_private_10(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xff000000)) == eh_hton32(0x0a000000);
}

static inline bool ipv4_is_private_172(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xfff00000)) == eh_hton32(0xac100000);
}

static inline bool ipv4_is_private_192(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xffff0000)) == eh_hton32(0xc0a80000);
}

static inline bool ipv4_is_linklocal_169(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xffff0000)) == eh_hton32(0xa9fe0000);
}

static inline bool ipv4_is_anycast_6to4(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xffffff00)) == eh_hton32(0xc0586300);
}

static inline bool ipv4_is_test_192(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xffffff00)) == eh_hton32(0xc0000200);
}

static inline bool ipv4_is_test_198(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xfffe0000)) == eh_hton32(0xc6120000);
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _IPV4_H_