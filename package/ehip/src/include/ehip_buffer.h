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

#include <stddef.h>
#include <stdint.h>

#include "eh_types.h"
#include "ehip_ptype.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct ehip_buffer ehip_buffer_t;
typedef uint16_t ehip_buffer_size_t;
typedef struct ehip_netdev ehip_netdev_t;

enum ehip_buffer_type{
    EHIP_BUFFER_TYPE_ETHERNET_FRAME,
    EHIP_BUFFER_TYPE_MAX
};

eh_static_assert(EHIP_BUFFER_TYPE_MAX <= UINT8_MAX, "ehip_buffer_type must be less than UINT8_MAX");

struct ehip_buffer_ref{
    uint8_t                    *buffer;
    ehip_buffer_size_t          buffer_size;
    ehip_buffer_size_t          ref_cnt;
    enum ehip_buffer_type       type;
};

struct ehip_buffer{
    struct ehip_buffer_ref     *buffer_ref;
    ehip_buffer_size_t          payload_pos;
    ehip_buffer_size_t          payload_tail;
    enum ehip_ptype             protocol;
    ehip_netdev_t              *netdev;
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
#define ehip_buffer_get_buffer_size(buf) ((buf)->buffer_ref->buffer_size)


/**
 * @brief                   获取有效数据的开始指针地址，这在读数据时很有用
 * @return                  payload ptr
 */
#define ehip_buffer_get_payload_ptr(buf) ((buf)->payload_pos + ehip_buffer_get_buffer_ptr(buf))

/**
 * @brief                   获取有效数据的大小
 * @return                  payload size
 */ 
#define ehip_buffer_get_payload_size(buf) ((ehip_buffer_size_t)((buf)->payload_tail - (buf)->payload_pos))

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
 * @param  src              源缓冲句柄
 * @return ehip_buffer_t* 
 */
extern ehip_buffer_t* ehip_buffer_dup(ehip_buffer_t* src);

/**
 * @brief                   引用一个网络数据buf，到一个新的缓冲区中 (浅拷贝)
 * @param  buf              源缓冲句柄
 * @return ehip_buffer_t*   返回一个引用源缓冲区的buf句柄
 */
extern ehip_buffer_t* ehip_buffer_ref_dup(ehip_buffer_t* buf);


/**
 * @brief                   向payload中追数据,
 * @param  buf              缓冲句柄
 * @param  size             本次追加数据的数量
 * @return uint8_t*         返回本次追加数据的开始指针，用户需要自己追加
 */
extern uint8_t* ehip_buffer_payload_append(ehip_buffer_t* buf, ehip_buffer_size_t size);

/**
 * @brief                   向缓冲区头部追加空间
 * @param buf               缓冲句柄
 * @param size              需要追加的空间大小
 * @return uint8_t*         返回新头部的缓冲区头部指针，如果失败返回NULL
 */
extern uint8_t* ehip_buffer_head_append(ehip_buffer_t* buf, ehip_buffer_size_t size);

/**
 * @brief                   减少缓冲区头部空间
 * @param buf               缓冲句柄
 * @param size              需要减少的空间大小
 * @return uint8_t*         返回被移除的空间的开始指针，如果失败返回NULL
 */
extern uint8_t* ehip_buffer_head_reduce(ehip_buffer_t* buf, ehip_buffer_size_t size);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_BUFFER_H_