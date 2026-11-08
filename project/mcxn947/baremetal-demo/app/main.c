/**
 * @file main.c
 * @brief 裸机入口：初始化业务后在主循环派发按键和灯效。
 */

#include <stdio.h>

#include "fsl_common.h"
#include "SEGGER_RTT.h"

#include "key.h"
#include "runtime.h"
#include "test.h"

extern void init(void);

int main(void)
{
    char message[64];
    int rc;

    init();
    SEGGER_RTT_WriteString(0, "baremetal-demo\n");

    rc = runtime_init();
    if(rc != NS_OK) {
        goto out;
    }
    rc = key_init();
    if(rc != NS_OK) {
        goto out_runtime;
    }
    rc = test_init();
    if(rc != NS_OK) {
        goto out_key;
    }

    rc = runtime_run();
    (void)test_deinit();
out_key:
    (void)key_deinit();
out_runtime:
    (void)runtime_deinit();
out:
    if(snprintf(message, sizeof(message), "runtime stopped rc=%d\n", rc) > 0) {
        SEGGER_RTT_WriteString(0, message);
    }
    for(;;) {
        __WFI();
    }
}
