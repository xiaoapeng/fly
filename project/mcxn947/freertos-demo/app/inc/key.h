#ifndef _KEY_H_
#define _KEY_H_

#include <stdint.h>

#include <nsbtn/nsbtn.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 板级按键身份。lwbtn 实例、事件信号与扫描定时器都由 package/nsbtn 管理；
 * 本模块只负责引脚配置与把引脚状态喂给 nsbtn 的 get_state 回调。 */

typedef struct key {
    const char *name;   /* "P0_6" / "P0_23"（日志用，避免板级 SW 编号歧义） */
    uint8_t     pin;    /* GPIO0 引脚号（当前两键都在 GPIO0/PORT0） */
} key_t;

extern key_t key_sw1;     /* P0_6  */
extern key_t key_sw2;     /* P0_23 */

/* 配置引脚、nsbtn_init + 注册两键 + 启动扫描。 */
int key_init(void);

/* 停扫描、拆 nsbtn、复位引脚。 */
int key_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
