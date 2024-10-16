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

enum ehip_buffer_type{
    EHIP_BUFFER_TYPE_ETHERNET_FRAME,
    EHIP_BUFFER_TYPE_MAX
};

eh_static_assert(EHIP_BUFFER_TYPE_MAX <= UINT8_MAX, "ehip_buffer_type must be less than UINT8_MAX");

struct ehip_buffer_ref{
    uint8_t                 *buffer;
    uint16_t                 buffer_size;
    uint16_t                 ref_cnt;
    enum ehip_buffer_type    type;
};

struct ehip_buffer{
    struct ehip_buffer_ref *buffer_ref;
    uint16_t                payload_pos;
    uint16_t                payload_tail;
};

/**
 * @brief                   获取buf的实例缓冲区指针
 * @return                  buffer ptr该缓冲区可用来读写
 */
#define ehip_buffer_get_buffer_ptr(buf) ((buf)->buffer_ref->buffer) 

/**
 * @brief                   获取buffer的总大小
 * @return                  buffer size
 */
#define ehip_buffer_get_buffer_size(buf) ((size_t)(buf)->buffer_ref->buffer_size)


/**
 * @brief                   获取有效数据的开始指针地址，这在读数据时很有用
 * @return                  payload ptr
 */
#define ehip_buffer_get_payload_ptr(buf) ((buf)->payload_pos + ehip_buffer_get_buffer_ptr(buf))

/**
 * @brief                   获取有效数据的大小
 * @return                  payload size
 */ 
#define ehip_buffer_get_payload_size(buf) ((buf)->payload_tail - (buf)->payload_pos)

/**
 * @brief                   获取有效数据的结束指针地址，在访问数据时作为边界条件使用
 * @return                  payload end ptr
 */
#define ehip_buffer_get_payload_end_ptr(buf) (ehip_buffer_get_buffer_ptr(buf) + (buf)->payload_tail)


/**
 * @brief                   新建一个网络数据buf，初始引用计数为1
 * @param  type             数据类型
 * @return ehip_buffer_t*   返回buf句柄
 */
extern ehip_buffer_t* ehip_buffer_new(enum ehip_buffer_type type);


/**
 * @brief                   释放一个网络数据buf
 * @param  buf              buf句柄
 */
extern void ehip_buffer_free(ehip_buffer_t* buf);

/**
 * @brief                   复制一个网络数据buf,到一个新的缓冲区中 (深拷贝)
 * @param  src              源缓冲
 * @return ehip_buffer_t* 
 */
extern ehip_buffer_t* ehip_buffer_dup(ehip_buffer_t* src);

/**
 * @brief                   引用一个网络数据buf，到一个新的缓冲区中 (浅拷贝)
 * @param  buf              源缓冲
 * @return ehip_buffer_t*   返回一个引用源缓冲区的buf句柄
 */
extern ehip_buffer_t* ehip_buffer_ref_dup(ehip_buffer_t* buf);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_BUFFER_H_