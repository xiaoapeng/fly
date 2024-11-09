/**
 * @file ehip_core_mbox_msg_type.h
 * @brief 邮箱消息类型
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-24
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _EHIP_CORE_MBOX_MSG_TYPE_H_
#define _EHIP_CORE_MBOX_MSG_TYPE_H_

#include "ehip_buffer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

enum ehip_mbox_msg_type{
    EHIP_MBOX_MSG_TYPE_NETDEV_RX,
};

struct ehip_mbox_msg_base{
    enum ehip_mbox_msg_type type;
};

struct ehip_mbox_msg_netdev_rx{
    enum ehip_mbox_msg_type     type;
    ehip_buffer_t               *netdev_buffer;
};




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_CORE_MBOX_MSG_TYPE_H_