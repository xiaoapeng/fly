#ifndef _RUNTIME_H_
#define _RUNTIME_H_

#include <nanosig/nanosig.h>
#include <nanosig/nanosig_loop.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The app's one main loop; NULL until runtime_init() succeeds. */
ns_loop_t *runtime_get(void);

/* ns_init() + ns_loop_create(); returns a nanosig status code. */
int runtime_init(void);

/* ns_loop_destroy() + ns_shutdown(); call after runtime_run() returns. */
int runtime_deinit(void);

/* Runs the main loop on the calling thread; returns when it is quit. */
int runtime_run(void);

#ifdef __cplusplus
}
#endif

#endif
