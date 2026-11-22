#ifndef FLY_APP_RUNTIME_H
#define FLY_APP_RUNTIME_H

#include <nanosig/nanosig_loop.h>

/* Initialize from the application context after the board timebase is ready. */
int app_runtime_init(void);

/* Borrow the native loop between successful init and deinit. */
ns_loop_t *app_runtime_get_loop(void);

/* Run on the application context until a callback requests a stop. */
int app_runtime_run(void);

/* Request a stop from a callback on this loop. */
int app_runtime_request_stop(void);

/* Call after run returns and all application connections and timers are released. */
int app_runtime_deinit(void);

#endif
