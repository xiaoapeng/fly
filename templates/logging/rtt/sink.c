#include "mid/fly_log.h"

#include "SEGGER_RTT.h"

#include <limits.h>
#include <stdint.h>

size_t fly_log_write(const void *data, size_t size)
{
    if (data == NULL || size == 0u) {
        return 0u;
    }
#if SIZE_MAX > UINT_MAX
    if (size > UINT_MAX) {
        size = UINT_MAX;
    }
#endif
    return SEGGER_RTT_Write(0u, data, (unsigned int)size);
}
