#ifndef FLY_FIRMWARE_INFO_H
#define FLY_FIRMWARE_INFO_H

#include "autoconf.h"

/* Print the validated firmware metadata at startup. */
#if defined(CONFIG_FACTORY_DATA_SUPPORTED)
void fly_log_firmware_info(void);
#else
static inline void fly_log_firmware_info(void)
{
}
#endif

#endif
