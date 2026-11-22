#include "runtime.h"

#include "autoconf.h"
#include "mid/fly_log.h"

#include <nanosig/nanosig.h>
#include <stdbool.h>

#if defined(CONFIG_PACKAGE_SCHEDULER_NONE)
#include "board.h"
#include "board_time.h"
#include <nanosig_baremetal_api.h>

static bool stop_requested;
#endif

static ns_loop_t *app_loop;
static bool running;

#if defined(CONFIG_PACKAGE_SCHEDULER_NONE)
void stdout_write(void *stream, const uint8_t *data, size_t size)
#else
void platform_stdout_write(void *stream, const uint8_t *data, size_t size)
#endif
{
    (void)stream;
    (void)fly_log_write(data, size);
}

int app_runtime_init(void)
{
#if defined(CONFIG_PACKAGE_SCHEDULER_NONE)
    ns_baremetal_set_now_us(board_time_us);
#endif
    int status = ns_init();
    if (status != NS_OK) {
        return status;
    }

    status = ns_loop_create(&app_loop, NULL);
    if (status != NS_OK) {
        (void)ns_shutdown();
    }
    return status;
}

ns_loop_t *app_runtime_get_loop(void)
{
    return app_loop;
}

int app_runtime_run(void)
{
    if (app_loop == NULL) {
        return NS_E_SHUTDOWN;
    }
    if (running) {
        return NS_E_BUSY;
    }

    running = true;
#if defined(CONFIG_PACKAGE_SCHEDULER_NONE)
    int status = NS_OK;
    stop_requested = false;
    while (!stop_requested) {
        status = ns_loop_poll(app_loop, 0u);
        if (status != NS_OK || stop_requested) {
            break;
        }
        board_idle();
    }
#else
    int status = ns_loop_run(app_loop);
#endif
    running = false;
    return status;
}

int app_runtime_request_stop(void)
{
    if (!running) {
        return NS_E_INVAL;
    }
#if defined(CONFIG_PACKAGE_SCHEDULER_NONE)
    stop_requested = true;
    return NS_OK;
#else
    return ns_loop_quit(app_loop);
#endif
}

int app_runtime_deinit(void)
{
    if (running) {
        return NS_E_BUSY;
    }
    if (app_loop == NULL) {
        return NS_OK;
    }

    int status = ns_loop_destroy(app_loop);
    if (status != NS_OK) {
        return status;
    }
    app_loop = NULL;
    return ns_shutdown();
}
