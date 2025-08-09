/**
 * eventhub_os 用户配置文件
 */

#ifndef _EH_USER_CONFIG_H_
#define _EH_USER_CONFIG_H_


/**
 *  配置事件回调函数的栈大小
 */
#define EH_CONFIG_EVENT_CALLBACK_FUNCTION_STACK_SIZE            (2*1024U)


/**
 *  platform_get_clock_monotonic_time 函数获取到的时钟使用的时钟频率
 */
#include <stdint.h>
extern uint32_t SystemCoreClock;
#define EH_CONFIG_CLOCKS_PER_SEC                                (SystemCoreClock)

/**
 *  EH_CONFIG_USE_LIBC_MEM_MANAGE为1时,将使用libc库的内存管理,未定义
 *   或者定义为0时使用eventhub_os的内存管理,使用libc时请务必实现：
 *    caddr_t _sbrk(int incr); 否则由于协程的特殊性(协程栈在堆上实现)，
 *    会使C库误以为发生堆栈碰撞而malloc失败。
 *  EH_CONFIG_MEM_ALLOC_ALIGN为分配空间的对齐粒度，为2的幂，这里默认为系统指针大小的2倍
 *  EH_CONFIG_MEM_HEAP_SIZE为堆内存大小，默认为20K
 */
#define EH_CONFIG_USE_LIBC_MEM_MANAGE                            0
#if (!defined(EH_CONFIG_USE_LIBC_MEM_MANAGE)) || (EH_CONFIG_USE_LIBC_MEM_MANAGE == 0)
#   define EH_CONFIG_MEM_ALLOC_ALIGN                             (sizeof(void*)*2)
#   define EH_CONFIG_MEM_HEAP_SIZE                               (100*1024U)
#endif

/**
 *  配置标准输出缓存大小,该缓冲为单次输出的最大字节数，并不限制eh_printf的输出字节数
 */
#define EH_CONFIG_STDOUT_MEM_CACHE_SIZE                          (32U)

/** 
 *  DEBUG时的回车符号，当未定义时会根据系统类型自动判断
 */
#define EH_CONFIG_DEBUG_ENTER_SIGN                               "\n"

/**
 *  DEBUG打印的TAG配置
 */
#define EH_CONFIG_DEBUG_FLAGS                                    (EH_DBG_FLAGS_DEBUG_TAG|EH_DBG_FLAGS_MONOTONIC_CLOCK)

/**
 *  中断栈的大小
 *  该栈仅仅在单片机时才能使用到，默认使用1024即可
 */
#define EH_CONFIG_INTERRUPT_STACK_SIZE                          1024

/**
 *  配置任务调度多少次后进行一次轮询
 */
#define EH_CONFIG_TASK_DISPATCH_CNT_PER_POLL                    4

#endif // _EH_USER_CONFIG_H_