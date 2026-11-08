#ifndef BAREMETAL_RUNTIME_H
#define BAREMETAL_RUNTIME_H

#include <nanosig/nanosig.h>
#include <nanosig/nanosig_loop.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 初始化 SysTick 时钟及静态 nanosig loop。 */
int runtime_init(void);

/* 业务释放后停止时钟、销毁 loop 并关闭 nanosig。 */
int runtime_deinit(void);

/* 返回主 loop；runtime_init 成功前为 NULL。 */
ns_loop_t *runtime_get(void);

/* 主循环轮询；ISR 只更新时间，不调用 nanosig 或业务回调。 */
int runtime_run(void);

#ifdef __cplusplus
}
#endif

#endif
