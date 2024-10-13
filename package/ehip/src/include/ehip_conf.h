/**
 * @file ehip_conf.h
 * @brief ehip_conf的默认配置
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-13
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */
#ifndef _EHIP_CONF_H_
#define _EHIP_CONF_H_

#ifndef EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_SIZE
#define EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_SIZE  1518U
#endif

#ifndef EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_NUM
#define EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_NUM   6U
#endif

#ifndef EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_ALIGN
#define EHIP_NETDEV_TYPE_ETHERNET_POOL_BUFFER_ALIGN 16U
#endif




#endif // _EHIP_CONF_H_