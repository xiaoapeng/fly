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
#include <eh_list.h>
#include <ehip_netdev.h>
#include <ehip_conf.h>
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
#define IPV4_ADDR_ALLRTRS_GROUP		    0xe0000002U	/* 224.0.0.2   */
#define IPV4_ADDR_ALLSNOOPERS_GROUP	    0xe000006aU	/* 224.0.0.106 */
#define IPV4_ADDR_MAX_LOCAL_GROUP		0xe00000ffU	/* 224.0.0.255 */

#define IPV4_ADDR_DEFAULT_CLASS_A_MASK_LEN 8
#define IPV4_ADDR_DEFAULT_CLASS_B_MASK_LEN 16
#define IPV4_ADDR_DEFAULT_CLASS_C_MASK_LEN 24
#define IPV4_ADDR_DEFAULT_CLASS_D_MASK_LEN 3

struct ipv4_netdev{
	struct eh_list_head							node;
    ipv4_addr_t                                 ipv4_addr[EHIP_NETDEV_MAX_IP_NUM];
	uint8_t										ipv4_mask_len[EHIP_NETDEV_MAX_IP_NUM];
	uint8_t 									ipv4_addr_num;
};


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ipv4_mask_len_to_mask(mask_len) ((ipv4_addr_t)(0xffffffffU >> (32 - (mask_len))))
#define ipv4_make_addr(dec0, dec1, dec2, dec3) ((ipv4_addr_t)(((dec0) << 0) | ((dec1) << 8) | ((dec2) << 16) | ((dec3) << 24)))
#define ipv4_addr_to_dec0(addr) ( ((addr) & 0x000000ff) >> 0  )
#define ipv4_addr_to_dec1(addr) ( ((addr) & 0x0000ff00) >> 8  )
#define ipv4_addr_to_dec2(addr) ( ((addr) & 0x00ff0000) >> 16 )
#define ipv4_addr_to_dec3(addr) ( ((addr) & 0xff000000) >> 24 )
#else
#define ipv4_mask_len_to_mask(mask_len) ((ipv4_addr_t)(0xffffffffU << (32 - (mask_len))))
#define ipv4_make_addr(dec0, dec1, dec2, dec3) ((ipv4_addr_t)(((dec0) << 24) | ((dec1) << 16) | ((dec2) << 8) | ((dec3) << 0)))
#define ipv4_addr_to_dec0(addr) ( ((addr) & 0xff000000) >> 24 )
#define ipv4_addr_to_dec1(addr) ( ((addr) & 0x00ff0000) >> 16 )
#define ipv4_addr_to_dec2(addr) ( ((addr) & 0x0000ff00) >> 8  )
#define ipv4_addr_to_dec3(addr) ( ((addr) & 0x000000ff) >> 0  )
#endif


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

static inline bool ipv4_is_global_bcast(ipv4_addr_t addr)
{
	/* limited broadcast */
	return addr == eh_hton32(IPV4_ADDR_BROADCAST);
}

static inline bool ipv4_is_local_broadcast(ipv4_addr_t addr, uint8_t mask_len){
	return (addr | ipv4_mask_len_to_mask(mask_len)) == eh_hton32(IPV4_ADDR_BROADCAST);
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


static inline bool ipv4_is_class_a(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0x80000000)) == 0;
}

static inline bool ipv4_is_class_b(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xc0000000)) == eh_hton32(0x80000000);
}

static inline bool ipv4_is_class_c(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xe0000000)) == eh_hton32(0xc0000000);
}

static inline bool ipv4_is_class_d(ipv4_addr_t addr)
{
	return (addr & eh_hton32(0xf0000000)) == eh_hton32(0xe0000000);
}

extern void ipv4_netdev_reset(struct ipv4_netdev *netdev);

extern void ipv4_netdev_up(struct ipv4_netdev *netdev);

extern void ipv4_netdev_down(struct ipv4_netdev *netdev);



/**
 * @brief 					判断IP地址是否为该设备的有效地址
 * @param  netdev           网络设备
 * @param  ipv4_addr        
 * @return true 
 * @return false 
 */
extern bool ipv4_netdev_is_ipv4_addr_valid(const struct ipv4_netdev* ipv4_dev,ipv4_addr_t ipv4_addr);

/**
 * @brief 					获取最佳匹配的IP地址
 * @param  netdev           网络设备
 * @param  dst_addr        	目标地址
 * @param  mask_len         掩码位数
 * @return ipv4_addr_t 		失败返回IPV4_ADDR_ANY 成功返回最佳接口地址
 */
extern ipv4_addr_t ipv4_netdev_get_best_ipv4_addr(const struct ipv4_netdev* ipv4_dev, ipv4_addr_t dst_addr, 
    uint8_t mask_len);

/**
 * @brief 					获取接口的网络地址
 * @param  ipv4_dev         ipv4设备
 * @return ipv4_addr_t 		失败返回IPV4_ADDR_ANY 成功返回第一个接口地址
 */
extern ipv4_addr_t ipv4_netdev_get_addr(const struct ipv4_netdev* ipv4_dev);

/**
 * @brief 					设置接口主要网络地址
 * @param  ipv4_dev         ipv4设备
 * @param  addr             要设置的IP地址
 * @param  mask_len         掩码，如果设置为0，则根据IP类型使用默认掩码
 */
extern int ipv4_netdev_set_main_addr(struct ipv4_netdev* ipv4_dev, ipv4_addr_t addr, uint8_t mask_len);

/**
 * @brief 					设置接口次要网络地址
 * @param  ipv4_dev         ipv4设备
 * @param  addr             要设置的IP地址
 * @param  mask_len         掩码，如果设置为0，则根据IP类型使用默认掩码
 */
extern int ipv4_netdev_set_sub_addr(struct ipv4_netdev* ipv4_dev, ipv4_addr_t addr, uint8_t mask_len);

/**
 * @brief 					删除接口网络地址
 * @param  ipv4_dev         ipv4设备
 * @param  addr             要删除的IP地址
 */
extern void ipv4_netdev_del_addr(struct ipv4_netdev* ipv4_dev, ipv4_addr_t addr);
/**
 * @brief 					判断地址是否为本地地址
 * @param  addr             要判断的ip地址
 * @return int 
 */
extern bool ipv4_netdev_is_local_addr(ipv4_addr_t addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _IPV4_H_