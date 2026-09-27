#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void led_init(void);
void led_set(uint8_t mask);

#ifdef __cplusplus
}
#endif

#endif