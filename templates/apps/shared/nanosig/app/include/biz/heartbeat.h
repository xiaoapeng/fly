#ifndef FLY_HEARTBEAT_H
#define FLY_HEARTBEAT_H

#include <nanosig/nanosig_loop.h>

int heartbeat_init(ns_loop_t *loop);
int heartbeat_deinit(void);

#endif
