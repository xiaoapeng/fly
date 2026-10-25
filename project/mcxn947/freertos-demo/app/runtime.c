/**
 * @file runtime.c
 * @brief Runtime: the app's single nanosig main loop
 */
#include "runtime.h"

#include "SEGGER_RTT.h"

#include <nanosig/ns_debug.h>

/* nanosig routes its debug output (ns_debug / ns_printf) through the weak
 * stdout_write() hook; the FreeRTOS backend forwards it here, so everything
 * nanosig logs ends up on SEGGER RTT channel 0. */
void platform_stdout_write(void *stream, const uint8_t *buf, size_t size)
{
    (void)stream;
    (void)SEGGER_RTT_Write(0, buf, (unsigned)size);
}

static ns_loop_t *main_loop = NULL;

ns_loop_t *runtime_get(void)
{
    return main_loop;
}

int runtime_init(void)
{
    int rc;

    rc = ns_init();
    if (rc != NS_OK) {
        goto out;
    }
    ns_sysln("nanosig debug bridge -> SEGGER RTT");

    rc = ns_loop_create(&main_loop, NULL);
    if (rc != NS_OK) {
        main_loop = NULL;
        goto out_shutdown;
    }
    return NS_OK;

out_shutdown:
    (void)ns_shutdown();
out:
    return rc;
}

int runtime_deinit(void)
{
    int rc = NS_OK;

    if (main_loop != NULL) {
        rc = ns_loop_destroy(main_loop);
        main_loop = NULL;
    }
    if (rc == NS_OK) {
        rc = ns_shutdown();
    }
    return rc;
}

int runtime_run(void)
{
    if (main_loop == NULL) {
        return NS_E_INVAL;
    }
    return ns_loop_run(main_loop);
}
