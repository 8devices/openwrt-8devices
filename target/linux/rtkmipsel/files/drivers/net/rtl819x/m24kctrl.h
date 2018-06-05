/*
 *  Performance Profiling Header File
 *
 *  $Id: m24kctrl.h,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2015 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <net/rtl/rtl_types.h>
#include <linux/math64.h>
//#include <asm/rlxregs.h>

/***
typedef unsigned long long	uint64;
typedef signed long long		int64;
typedef unsigned int	uint32;
typedef signed int		int32;
typedef unsigned short	uint16;
typedef signed short	int16;
typedef unsigned char	uint8;
typedef signed char		int8;
***/

struct rtl8651_romeperf_stat_s {
	char *desc;
#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN
	uint64 accCycle[8];
	uint64 tempCycle[8];
	unsigned char Counter[8];    
	unsigned char Event[8];        
	unsigned char numOfCount;
	unsigned char bUsed;    
#else
	uint64 accCycle[4];
	uint64 tempCycle[4];
#endif // PERF_DUMP_CP3_DUAL_COUNTER_EN
	uint32 reEnterIdx;    
	uint64 executedNum;
	uint32 hasTempCycle:1; /* true if tempCycle is valid. */
};
typedef struct rtl8651_romeperf_stat_s rtl8651_romeperf_stat_t;

#define CP3ROMEPERF_INDEX_MAX 20
extern rtl8651_romeperf_stat_t cp3romePerfStat[CP3ROMEPERF_INDEX_MAX];
extern uint64 avrgCycle[CP3ROMEPERF_INDEX_MAX];

extern unsigned long long int cp3Value[4]; 
extern unsigned long int cp3Count[4]; 

extern void stopCP3Ctrl(int i, unsigned int index);
extern void startCP3Ctrl(unsigned int i);

// see Table 6.46 and 6.47 of MD00343-2B-24K-SUM-03.11.pdf for all of the event and counter0/1 meaning
#define PERF_EVENT_CYCLE				0x0100	// Counter 0: Cycles; Counter 1: Instructions completed
#define PERF_EVENT_ICACHE_MISS			0x0909	// Counter 0: Instruction Cache accesses; Counter 1: Instruction cache misses
#define PERF_EVENT_ICACHE_MISS_CYCLE	0x2525	// Counter 0: I$ Miss stall cycles; Counter 1: D$ miss stall cycles
#define PERF_EVENT_DCACHE_MISS			0x010B	// Counter 0: Data cache misses; Counter 1: Instructions completed


#if 0
// example:
// part 1: add test code
file_1
#include "m24kctrl.h"
FUNC_A() {

	startCP3Ctrl(PERF_EVENT_CYCLE); // assign Event and start counting
	...
	if (...) {
		...
		stopCP3Ctrl(1, 0);
		return;
	}
	...
	stopCP3Ctrl(1, 0); // stop and record to global array 0
	return;
}

file_2
#include "m24kctrl.h"
FUNC_B() {

	startCP3Ctrl(PERF_EVENT_ICACHE_MISS);
	...
	if (...) {
		...
		stopCP3Ctrl(1, 1);
		return;
	}
	...
	stopCP3Ctrl(1, 1);
	return;
}

// part 2: test and get cp3 result
0. after DUT power on, the global array is summed when FUNC_A or FUNC_B is executed.
1. starting test
2. clear the global array:
	echo clear > /proc/rtl865x/stats
3. dump cp3 counter	result till now.
	echo dump 0 1 > /proc/rtl865x/stats

result:
index[0] accCycle[0]=190909834 executedNum=168829 average=1130
index[0] accCycle[1]=100639009 executedNum=168867 average=595
index[1] accCycle[0]=73693100 executedNum=336742 average=218
index[1] accCycle[1]=8998 executedNum=336781 average=0

	FUNC_A: index[0]
		accCycle[0]: Cycles
		accCycle[1]: Instructions completed
		
	FUNC_B: index[1]
		accCycle[0]: Instruction Cache accesses
		accCycle[1]: Instruction cache misses	
#endif

