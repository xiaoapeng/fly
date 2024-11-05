/**
 * @file ehip_protocol_handle.h
 * @brief ptype 枚举中各种协议包处理
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-26
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#ifndef _EHIP_PROTOCOL_HANDLE_H_
#define _EHIP_PROTOCOL_HANDLE_H_

#include "ehip_ptype.h"
#include "eh_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct ehip_buffer ehip_buffer_t;

struct ehip_protocol_handle {
    struct eh_list_head     node;
    enum ehip_ptype         ptype;
    void (*handle)(struct ehip_buffer* buf);
};
/**
 * @brief                   注册协议包处理函数
 * @param  handle           由用户的静态空间进行定义，一般使用 static struct ehip_protocol_handle xxx_protocol;
 * @return int              注册成功返回0
 */
extern int ehip_protocol_handle_register(struct ehip_protocol_handle *handle);

/**
 * @brief                   注销协议包处理函数
 * @param  handle           之前调用ehip_protocol_handle_register注册过的
 */
extern void ehip_protocol_handle_unregister(struct ehip_protocol_handle *handle);

/**
 * @brief                   进行协议包处理
 * @param  buf              网络协议包
 * @return int 
 */
extern int ehip_protocol_handle_dispatch(ehip_buffer_t *buf);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_PROTOCOL_HANDLE_H_