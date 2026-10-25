#ifndef _TASK_MAIN_H_
#define _TASK_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Entry of the app's sole thread; created by main(). */
void main_task(void *p);

#ifdef __cplusplus
}
#endif

#endif
