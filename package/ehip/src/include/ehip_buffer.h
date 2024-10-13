/**
 * @file ehip_buffer.h
 * @brief 网络数据buf实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-10
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#ifndef _EHIP_BUFFER_H_
#define _EHIP_BUFFER_H_

#include <stdint.h>

typedef struct ehip_buffer ehip_buffer_t;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

struct ehip_buffer{
    uint8_t *buffer;
    uint16_t buffer_size;
    uint16_t payload_pos;
    uint16_t payload_tail;
};

enum ehip_buffer_type{
    EHIP_BUFFER_TYPE_ETHERNET_FRAME,
    EHIP_BUFFER_TYPE_MAX
};

extern ehip_buffer_t* ehip_buffer_new(enum ehip_buffer_type type);
extern int ehip_buffer_ref(ehip_buffer_t *buf);
extern int ehip_buffer_unref(ehip_buffer_t *buf);









#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_BUFFER_H_