/**
 * @file ehip_netdev_trait.h
 * @brief 获取网卡特征接口
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _NETDEV_TRAIT_H_
#define _NETDEV_TRAIT_H_


#include <stddef.h>
#include <eh_types.h>
#include <ehip_conf.h>
#include <ehip_netdev.h>
#include <ehip_ptype.h>
#include <ehip_buffer.h>
#include <ehip_netdev_type.h>
#include <ehip-mac/ethernet.h>
#include <ehip-mac/hw_addr.h>
#include <ehip-ipv4/ip.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef void *ehip_netdev_trait_t;

#define __tait_type_ptr__
#define __tait_r__

struct ehip_netdev_trait_ops{
    size_t  trait_size;
    int (*up)(ehip_netdev_t *netdev);
    void (*down)(ehip_netdev_t *netdev);
    void (*reset)(ehip_netdev_t *netdev);
    int (*change)(ehip_netdev_t *netdev, const void *type_ptr, const void *src_ptr);
    int (*hard_header)(ehip_netdev_t *netdev, ehip_buffer_t *buf, 
        const ehip_hw_addr_t *src_hw_addr, const ehip_hw_addr_t *dst_hw_addr, 
        enum ehip_ptype ptype, ehip_buffer_size_t len);
    uint16_t hw_addr_offset;
    uint16_t mac_ptype_offset;
    uint16_t ipv4_dev_offset;
    uint16_t multicast_hw_offset;
    uint16_t broadcast_hw_offset;
};

#define EHIP_NETDEV_TRAIT_MAX_SIZE  ((size_t)(UINT16_MAX - sizeof(ehip_netdev_t)))

#define ehip_netdev_trait_static_assert(type)   \
    eh_static_assert(sizeof(type) <= EHIP_NETDEV_TRAIT_MAX_SIZE, "type size > EHIP_NETDEV_TRAIT_MAX_SIZE");

#define  ehip_netdev_trait_offsetof(type, member)  (uint16_t)(eh_offsetof(type, member) + sizeof(ehip_netdev_t))
#define  ehip_netdev_attr_offsetof(attr_name) (uint16_t)(eh_offsetof(ehip_netdev_t, attr.attr_name))
#define  ehip_netdev_attr_type_ptr(netdev, attr_name)  (void*)((netdev)->attr.attr_name)


#define ehip_netdev_to_trait(netdev)        ((ehip_netdev_trait_t)(netdev + 1) )
#define ehip_trait_to_netdev(netdev_trait)  ((ehip_netdev_t*)(netdev_trait) - 1)
    
/**
 * @brief                   获取给定网络设备类型的特征属性大小
 * @param  type             网络设备类型
 * @return size_t 
 */
extern size_t ehip_netdev_trait_size_get(enum ehip_netdev_type type);

/**
 * @brief               安装网卡特征属性
 * @param type          网卡类型
 * @param ops           网卡特征操作接口
 * @return int 
 */
extern int ehip_netdev_trait_type_install(enum ehip_netdev_type type, const struct ehip_netdev_trait_ops *ops);

/**
 * @brief               网卡特征属性重置
 * @param   netdev           
 * @return  int 
 */
extern int ehip_netdev_trait_reset(ehip_netdev_t *netdev);

/**
 * @brief               网卡特征属性up,当网络设备状态被up时调用
 */
extern int ehip_netdev_trait_up(ehip_netdev_t *netdev);

/**
 * @brief               网卡特征属性down,当网络设备状态被down时调用
 */
extern void ehip_netdev_trait_down(ehip_netdev_t *netdev);


/**
 * @brief               网卡特征属性hard_header,当网络设备发送数据包前调用，填充头部
 * @param  netdev       网络设备句柄
 * @param  buf          ehip_buffer 数据包结构
 * @param  src_hw_addr  源网卡MAC地址
 * @param  dst_hw_addr  目标网卡MAC地址
 * @param  ptype        上层协议
 * @param  len          除去头部的数据长度
 * @return int          0表示成功，负数表示失败
 */
extern int ehip_netdev_trait_hard_header(ehip_netdev_t *netdev, ehip_buffer_t *buf, 
    const ehip_hw_addr_t *src_hw_addr, const ehip_hw_addr_t *dst_hw_addr, 
    enum ehip_ptype ptype, ehip_buffer_size_t len);


/**
 * @brief                   修改某一个特征
 * @param  netdev           网卡设备
 * @param  type_ptr         要修改的属性类型指针，由标有 __tait_type_ptr__ 的属性函数返回的指针进行填充
 *                              或者ehip_netdev_attr_type_ptr的返回值
 * @param  src_ptr          要修改为目的数据的指针
 * @return int              成功返回0，错误返回负数
 */
extern int ehip_netdev_trait_change(ehip_netdev_t *netdev, const void *type_ptr, const void *src_ptr);

extern __tait_type_ptr__ const ehip_hw_addr_t* ehip_netdev_trait_hw_addr(ehip_netdev_t *netdev);
extern __tait_type_ptr__ const enum ehip_ptype* ehip_netdev_trait_mac_ptype(ehip_netdev_t *netdev);
extern __tait_type_ptr__ struct ipv4_netdev* ehip_netdev_trait_ipv4_dev(ehip_netdev_t *netdev);
extern __tait_type_ptr__ const ehip_hw_addr_t* ehip_netdev_trait_broadcast_hw(ehip_netdev_t *netdev);
extern __tait_type_ptr__ const ehip_hw_addr_t* ehip_netdev_trait_multicast_hw(ehip_netdev_t *netdev, int index);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _NETDEV_TRAIT_H_

