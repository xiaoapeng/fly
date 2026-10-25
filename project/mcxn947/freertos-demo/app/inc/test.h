#ifndef _TEST_H_
#define _TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Connects the nsbtn_event handler and starts the LED blink timer;
 * returns a nanosig status. */
int test_init(void);

/* Disconnects the key subscription, stops/releases the blink timer and
 * turns the LED off; call before key_deinit(). */
int test_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
