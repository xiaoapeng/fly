/**
 * @file typedef.h
 * @brief 平台定义
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-04-01
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */


#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdint.h>

extern uint32_t systick_get(void);
static inline void _delay_us(uint32_t _us){
    uint32_t i;
    while(_us--){
        for(i=0;i<100;i++)
            asm("NOP");
    }
}
static inline void _delay_ms(uint32_t _ms){
    uint32_t c_time = systick_get();
    while(systick_get() - c_time < _ms){
        asm("NOP");
    }
}

#define MALLOC(__size)  (malloc(__size))
#define FREE(__ptr)     (free(__ptr))
#define DELAY(__ms)     _delay_ms(__ms)
#define DELAY_US(__us) _delay_us(__us)
#define GET_TICK()	   systick_get()



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _TYPEDEF_H_
