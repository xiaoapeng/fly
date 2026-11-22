#ifndef FLY_LOG_H
#define FLY_LOG_H

#include <stddef.h>

/* Nonblocking. Returns how many bytes the selected sink accepted. */
size_t fly_log_write(const void *data, size_t size);

#endif
