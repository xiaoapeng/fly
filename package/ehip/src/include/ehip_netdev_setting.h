/**
 * @file ehip_netdev_setting.h
 * @brief 网络设备配置
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#ifndef _EHIP_NETDEV_SETTING_H_
#define _EHIP_NETDEV_SETTING_H_

#include <stddef.h>
#include <stdint.h>

#include "eh_list.h"
#include "eh_mem.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


enum ehip_netdev_setting_type{
    EHIP_NETDEV_SETTING_TYPE_HW_ADDR,
    EHIP_NETDEV_SETTING_TYPE_IPV4_ADDR,
    EHIP_NETDEV_SETTING_TYPE_TPV6_ADDR,
};



/* 配置项结构体 */
struct ehip_netdev_setting_item{
    struct eh_list_head             node;
    enum ehip_netdev_setting_type   type;
    uint8_t                         conf[0];
};

struct ehip_netdev_setting{
    struct eh_list_head         item_list; 
};


/**
 * @brief 初始化配置项列表
 * @param setting 配置项列表指针
 */
#define ehip_netdev_setting_init(setting) eh_list_head_init(&(setting)->item_list)

/**
 * @brief 遍历
 * @param setting       配置项列表指针
 * @param item          遍历过程中pos指针
 */
#define ehip_netdev_setting_for_each(setting, item) \
    eh_list_for_each_entry(item, &(setting)->item_list, node)

/**
 * @brief 安全遍历，遍历过程中可进行删除配置项
 * @param setting       配置项列表指针
 * @param item          遍历过程中pos指针
 */
#define ehip_netdev_setting_for_each_safe(setting, item, n) \
    eh_list_for_each_entry_safe(item, &(setting)->item_list, node)

/**
 * @brief                   动态分配一个配置项
 * @param  size             配置的缓冲长度
 * @return struct ehip_netdev_setting_item* 返回NULL时失败
 */
static inline struct ehip_netdev_setting_item* ehip_netdev_setting_item_alloc(size_t size){
    return eh_malloc(sizeof(struct ehip_netdev_setting_item) + size);
}

/**
 * @brief                   销毁一个配置项，销毁前请进行remove
 * @param  item             配置项
 */
static inline void ehip_netdev_setting_item_free(struct ehip_netdev_setting_item *item){
    eh_free(item);
}

/**
 * @brief                   添加一个配置项到setting
 * @param  setting          setting 句柄
 * @param  item             配置项，通过ehip_netdev_setting_item_alloc进行分配
 * @return  int  成功返回0，失败返回负数
 */
extern int ehip_netdev_setting_item_add(struct ehip_netdev_setting *setting, struct ehip_netdev_setting_item *item);


/**
 * @brief                   移除一个配置项，remove后需要手动去调用 ehip_netdev_setting_item_free
 * @param  setting          setting 句柄
 * @param  item             配置项(已经添加到setting中)
 */
static inline void ehip_netdev_setting_item_remove(struct ehip_netdev_setting *setting, struct ehip_netdev_setting_item *item){
    (void) setting;
    eh_list_del(&item->node);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_SETTING_H_