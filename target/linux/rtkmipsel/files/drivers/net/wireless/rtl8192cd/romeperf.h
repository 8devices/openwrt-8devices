/*
 *  Performance Profiling Header File
 *
 *  $Id: romeperf.h,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _ROMEPERF_H_
#define _ROMEPERF_H_

#include <net/rtl/rtl_types.h>
#include <asm/rlxregs.h>

/*
typedef unsigned long long	uint64;
typedef signed long long		int64;
typedef unsigned int	uint32;
typedef signed int		int32;
typedef unsigned short	uint16;
typedef signed short	int16;
typedef unsigned char	uint8;
typedef signed char		int8;
*/

#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif


#define MAX_CP3_COUNTER 8

#if defined(CONFIG_RTL865X_MODULE_ROMEDRV)
#define	rtl8651_romeperfEnterPoint				module_internal_rtl8651_romeperfEnterPoint
#define	rtl8651_romeperfExitPoint					module_internal_rtl8651_romeperfExitPoint
#endif

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
	uint32 executedNum;
	uint32 hasTempCycle:1; /* true if tempCycle is valid. */
};
typedef struct rtl8651_romeperf_stat_s rtl8651_romeperf_stat_t;


/* for rtl8651_romeperfEnterPoint() and rtl8651_romeperfExitPoint() */
#if (PERF_DUMP_INIT_SELECT == PERF_DUMP_INIT_ORI)
#define ROMEPERF_INDEX_MIN 0
#define ROMEPERF_INDEX_NAPT_ADD 0
#define ROMEPERF_INDEX_NAPT_ADD_1 1
#define ROMEPERF_INDEX_NAPT_ADD_2 2
#define ROMEPERF_INDEX_NAPT_ADD_3 3
#define ROMEPERF_INDEX_NAPT_ADD_4 4
#define ROMEPERF_INDEX_NAPT_ADD_5 5
#define ROMEPERF_INDEX_NAPT_ADD_6 6
#define ROMEPERF_INDEX_NAPT_ADD_7 7
#define ROMEPERF_INDEX_NAPT_ADD_8 8
#define ROMEPERF_INDEX_NAPT_ADD_9 9
#define ROMEPERF_INDEX_NAPT_ADD_10 10
#define ROMEPERF_INDEX_NAPT_ADD_11 11
#define ROMEPERF_INDEX_NAPT_DEL 12
#define ROMEPERF_INDEX_NAPT_FIND_OUTBOUND 13
#define ROMEPERF_INDEX_NAPT_FIND_INBOUND 14
#define ROMEPERF_INDEX_NAPT_UPDATE 15
#define ROMEPERF_INDEX_UNTIL_RXTHREAD 20
#define ROMEPERF_INDEX_RECVLOOP 21
#define ROMEPERF_INDEX_FWDENG_INPUT 22
#define ROMEPERF_INDEX_BEFORE_CRYPTO_ENCAP 23
#define ROMEPERF_INDEX_ENCAP 24
#define ROMEPERF_INDEX_ENCAP_CRYPTO_ENGINE 25
#define ROMEPERF_INDEX_ENCAP_AUTH_ENGINE 26
#define ROMEPERF_INDEX_BEFORE_CRYPTO_DECAP 27
#define ROMEPERF_INDEX_DECAP 28
#define ROMEPERF_INDEX_DECAP_CRYPTO_ENGINE 29
#define ROMEPERF_INDEX_DECAP_AUTH_ENGINE 30
#define ROMEPERF_INDEX_FASTPATH 31
#define ROMEPERF_INDEX_SLOWPATH 32
#define ROMEPERF_INDEX_UNTIL_ACLDB 33
#define ROMEPERF_INDEX_GET_MTU_AND_SOURCE_MAC 34
#define ROMEPERF_INDEX_PPTPL2TP_1 35
#define ROMEPERF_INDEX_PPPOE_ARP_CACHE 36
#define ROMEPERF_INDEX_PPTPL2TP_SEND 37
#define ROMEPERF_INDEX_FRAG 38
#define ROMEPERF_INDEX_FWDENG_SEND 39
#define ROMEPERF_INDEX_EGRESS_ACL 40
#define ROMEPERF_INDEX_PPTPL2TP_ENCAP 41
#define ROMEPERF_INDEX_FROM_PS 42
#define ROMEPERF_INDEX_EXTDEV_SEND 43
#define ROMEPERF_INDEX_FRAG_2ND_HALF 44
#define ROMEPERF_INDEX_TXPKTPOST 45
#define ROMEPERF_INDEX_MBUFPAD 46
#define ROMEPERF_INDEX_TXALIGN 47
#define ROMEPERF_INDEX_ISRTXRECYCLE 48
#define ROMEPERF_INDEX_16 49
#define ROMEPERF_INDEX_17 50
#define ROMEPERF_INDEX_18 51
#define ROMEPERF_INDEX_19 52
#define ROMEPERF_INDEX_20 53
#define ROMEPERF_INDEX_21 54
#define ROMEPERF_INDEX_22 55
#define ROMEPERF_INDEX_23 56
#define ROMEPERF_INDEX_24 57
#define ROMEPERF_INDEX_25 58
#define ROMEPERF_INDEX_FLUSHDCACHE 59
#define ROMEPERF_INDEX_IRAM_1 60
#define ROMEPERF_INDEX_IRAM_2 61
#define ROMEPERF_INDEX_IRAM_3 62
#define ROMEPERF_INDEX_IRAM_4 63
#define ROMEPERF_INDEX_DRAM_1 64
#define ROMEPERF_INDEX_DRAM_2 65
#define ROMEPERF_INDEX_DRAM_3 66
#define ROMEPERF_INDEX_DRAM_4 67
#define ROMEPERF_INDEX_BMP 68
#define ROMEPERF_INDEX_MDCMDIO 69
#define ROMEPERF_INDEX_MAX 70

#elif (PERF_DUMP_INIT_SELECT == PERF_DUMP_INIT_WLAN_TRX)
#define ROMEPERF_INDEX_MIN      0
enum _ROMEPERF_INDEX_LIST_ {
    //tx
    ROMEPERF_INDEX_TX_PREWORK   = 0,
    ROMEPERF_INDEX_TX_XMIT_OUT,
    ROMEPERF_INDEX_TX_XMIT_OUT_2,

    //Rx     
    ROMEPERF_INDEX_RX_ONE_PKT,
    ROMEPERF_INDEX_RX_ONE_PKT_2,
    ROMEPERF_INDEX_TX_START_XMIT,
    ROMEPERF_INDEX_TX_START_XMIT_2,
    ROMEPERF_INDEX_TX_START_XMIT_3,    
    ROMEPERF_INDEX_TX_START_XMIT_4,    
    ROMEPERF_INDEX_TX_START_XMIT_5,        
    ROMEPERF_INDEX_MAX
};


#else
#error "PERF_DUMP_INIT_SELECT flag error"
#endif


#if (PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_OLD)
enum CP3_COUNTER
{
	CP3CNT_CYCLES = 0,
	CP3CNT_NEW_INST_FECTH,
	CP3CNT_NEW_INST_FETCH_CACHE_MISS,
	CP3CNT_NEW_INST_MISS_BUSY_CYCLE,
	CP3CNT_DATA_STORE_INST,
	CP3CNT_DATA_LOAD_INST,
	CP3CNT_DATA_LOAD_OR_STORE_INST,
	CP3CNT_EXACT_RETIRED_INST,
	CP3CNT_RETIRED_INST_FOR_PIPE_A,
	CP3CNT_RETIRED_INST_FOR_PIPE_B,
	CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS,
	CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE,
	CP3CNT_RESERVED12,
	CP3CNT_RESERVED13,
	CP3CNT_RESERVED14,
	CP3CNT_RESERVED15,
};
#elif (PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_NEW)
enum CP3_COUNTER
{
    //0x0
	CP3CNT_STOP_COUNT               = 0,
	CP3CNT_INST_FECTH,                  
	CP3CNT_ICACHE_MISS,
	CP3CNT_ICACHE_MISS_CYCLE,
	CP3CNT_STORE_INST,

    //0x5
	CP3CNT_LOAD_INST,
	CP3CNT_LOAD_OR_STORE_INST,
	CP3CNT_COMPLETE_INST,
	CP3CNT_CYCLES,
	CP3CNT_ICACHE_SOFT_MISS,
	
	//0xA
	CP3CNT_DCACHE_MISS,
	CP3CNT_DCACHE_MISS_CYCLES,
	CP3CNT_L2CACHE_HIT,
	CP3CNT_L2CACHE_HIT_CYCLES,
	CP3CNT_L2CACHE_MISS,
	CP3CNT_L2CACHE_MISS_CYCLES,

    //0X10
    CP3CNT_BRANCH_PREDICTION,
    CP3CNT_BRANCH_PREDICTION_MISS,
    // TODO: Filen, enumalation below should be added
    
    //0x15
    //0x1A
    //0x20
    //0x25
    //0x2A
    //0x30

    CP3_CNT_MAX = 0x34
};

#else
#error "PERF_DUMP_CP3_SELECT flag error"
#endif

int32 rtl8651_romeperfInit( void );
inline int32 rtl8651_romeperfStart(void);
inline int32 rtl8651_romeperfStop(void);
int32 rtl8651_romeperfReset( void );
int32 rtl8651_romeperfEnterPoint( uint32 index );
int32 rtl8651_romeperfExitPoint( uint32 index );
int32 rtl8651_romeperfDump( int start, int end );
int32 rtl8651_romeperfPause( void );
int32 rtl8651_romeperfResume( void );
int32 rtl8651_romeperfGet( uint64 *pGet );

//int32 rtl_romeperfEnterPoint_dual(uint32 index, int cnt_num);
int32 rtl_romeperfEnterPoint_dual(uint32 index, int cnt_num,char *event);
int32 rtl_romeperfExitPoint_dual(uint32 index);
int getEventIndex(int i);
int getAvailableCnt(void);
int setEvent(int counter,char evnet);


extern rtl8651_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];



#endif/* _ROMEPERF_H_ */
