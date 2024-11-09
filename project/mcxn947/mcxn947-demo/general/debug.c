/**
 * @file debug.h
 * @brief debug用
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-03-17
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#define _SOUCE_DEBUG_C_ 1



DBG_LEVEL dbgLevel = DEBUG_DEFAULT_LEVEL;
static char 	printBuf[DEBUG_BUF_SIZE];

int debug_printf(int is_lock, int is_print_time,const char *format, ...)
{
	va_list aptr;
	int ret;

#if defined(DEBUG_USE_TIME) && (DEBUG_USE_TIME == 1)
	time_t Time;
	struct tm Timestamp;
	char str_time[30];
#endif 

#if !defined(DEBUG_USE_LOCK) || (DEBUG_USE_LOCK == 0)
    (void) is_lock;
#endif

#if !defined(DEBUG_USE_TIME) || (DEBUG_USE_TIME == 0)
    (void) is_print_time;
#endif
	/* 临界区开始 */

#if defined(DEBUG_USE_LOCK) && (DEBUG_USE_LOCK == 1)
    if(is_lock)
	    debug_lock();
#endif
    
	
	va_start(aptr, format);
	ret = vsnprintf(printBuf,DEBUG_BUF_SIZE,format, aptr);
	va_end(aptr);
	if(ret > 0)
	{
    
#if defined(DEBUG_USE_TIME) && (DEBUG_USE_TIME == 1)
		if(is_print_time)
		{
			debug_time(&Time);
			Time += DEBUG_DEFAULT_TIMEZONE;
			localtime_r(&Time, &Timestamp);
			strftime (str_time,sizeof(str_time),"[%Y/%m/%d %H:%M:%S] ",&Timestamp);
			debug_pus(str_time);
		}
#endif
        debug_pus(printBuf);
	}
#if defined(DEBUG_USE_LOCK) && (DEBUG_USE_LOCK == 1)
	if(is_lock)
	    debug_unlock();
#endif
    return ret;
}


void debug_phex(const void *buf,  int len)
{
	const uint8_t *start = buf;
	uint32_t y,x,i=0;
	uint32_t y_n, x_n;
	y_n = (uint32_t)len / 16;
	x_n = (uint32_t)len % 16;

#if defined(DEBUG_USE_LOCK) && (DEBUG_USE_LOCK == 1)
	debug_lock();
#endif

	debug_printf(0, 0, "______________________________________________________________\r\n");
	debug_printf(0, 0, "            | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9| A| B| C| D| E| F||\r\n");
	debug_printf(0, 0, "--------------------------------------------------------------\r\n");
	
	for(y=0;y  < y_n;y++)
	{
		debug_printf(0, 0, "|0x%08x| ", y*16);
		for(x=0;x < 16; x++ )
		{
			debug_printf(0, 0, "%02x ",start[i]);
			i++;
		}
		debug_printf(0, 0, "|\r\n");
	}
	if(x_n)
	{
		debug_printf(0, 0, "|0x%08x| ", y*16);
		for(x=0;x < 16;x++)
		{
			if(x < x_n)
			{
				debug_printf(0, 0, "%02x ",start[i]);
				i++;
			}else{
				debug_printf(0, 0, "   ");
			}
		}
		debug_printf(0, 0, "|\r\n");
	}
	debug_printf(0, 0, "--------------------------------------------------------------\r\n");

#if defined(DEBUG_USE_LOCK) && (DEBUG_USE_LOCK == 1)
	debug_unlock();
#endif

}