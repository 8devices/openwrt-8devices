/*
 *  Performance Profiling routines
 *
 *  $Id: romeperf.c,v 1.1 2009/11/06 12:26:48 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include "8192cd_cfg.h"

#ifdef PERF_DUMP

#include "romeperf.h"
//#include <asm/rtl865x/rtl_glue.h>
#define KERNEL_SYSeALLS
#include <asm/unistd.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/rlxregs.h>
#ifdef CONFIG_WIRELESS_LAN_MODULE
#define __IRAM
#else
#define __IRAM		__attribute__ ((section(".iram-gen")))
#endif
#define __OPT_0		__attribute__((optimize("O0")))

#define rtlglue_malloc(size)	kmalloc(size, 0x1f0)
#define rtlglue_free(p)	kfree(p)
#define rtlglue_printf panic_printk


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

/* Local variables */
//static 
uint64 tempVariable64;
static uint32 tempVariable32;
#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN
static uint32 tempVariable32_2;
unsigned char bCounterUsed[8];
unsigned char numOfUsedCounter = 0;
#endif
static uint64 currCnt[4];
unsigned int countTemp = 0xff;


/* Global variables */
#ifdef CONFIG_WIRELESS_LAN_MODULE
static uint64 cnt1, cnt2;
static rtl8651_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];
static uint32 rtl8651_romeperf_inited = 0;
static uint32 rtl8651_romeperf_enable = TRUE;
#else
uint64 cnt1, cnt2;
rtl8651_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];
uint32 rtl8651_romeperf_inited = 0;
uint32 rtl8651_romeperf_enable = TRUE;
#endif

unsigned char *mapping[]=
{   
#if (PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_OLD)
    "CP3CNT_CYCLES",
    "CP3CNT_NEW_INST_FECTH",
    "CP3CNT_NEW_INST_FETCH_CACHE_MISS",
    "CP3CNT_NEW_INST_MISS_BUSY_CYCLE",
    "CP3CNT_DATA_STORE_INST",
    "CP3CNT_DATA_LOAD_INST",
    "CP3CNT_DATA_LOAD_OR_STORE_INST",
    "CP3CNT_EXACT_RETIRED_INST",
    "CP3CNT_RETIRED_INST_FOR_PIPE_A",
    "CP3CNT_RETIRED_INST_FOR_PIPE_B",
    "CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS",
    "CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE",
    "CP3CNT_RESERVED12",
    "CP3CNT_RESERVED13",
    "CP3CNT_RESERVED14",
    "CP3CNT_RESERVED15",
};
#else
    "CP3CNT_STOP_COUNT",
    "CP3CNT_INST_FECTH",
    "CP3CNT_ICACHE_MISS",
    "CP3CNT_ICACHE_MISS_CYCLE",
    "CP3CNT_STORE_INST",
    "CP3CNT_LOAD_INST",
    "CP3CNT_LOAD_OR_STORE_INST",
    "CP3CNT_COMPLETE_INST",
    "CP3CNT_CYCLES",
    "CP3CNT_ICACHE_SOFT_MISS",
    "CP3CNT_DCACHE_MISS",
    "CP3CNT_DCACHE_MISS_CYCLES",
    "CP3CNT_L2CACHE_HIT",
    "CP3CNT_L2CACHE_HIT_CYCLES",
    "CP3CNT_L2CACHE_MISS",
    "CP3CNT_L2CACHE_MISS_CYCLES",
    "CP3CNT_BRANCH_PREDICTION",
    "CP3CNT_BRANCH_PREDICTION_MISS",
   };
#endif     


__IRAM void CP3_COUNTER0_INIT( void )
{
    __asm__ __volatile__ 
    (
    "mfc0	$8, $12			\n\t"
	"la		$9, 0x80000000	\n\t"
	"or		$8, $9			\n\t"
	"mtc0	$8, $12			\n\t"
	:
    :
    :"$8","$9"
    );
}

__IRAM uint32 CP3_COUNTER0_IS_INITED( void )
{
    __asm__ __volatile__ 
    (
	"mfc0	$8, $12			    \n\t"
	"la		$9, tempVariable32  \n\t"
	"sw		$8, 0($9)		    \n\t"
	:
    :
    :"$8","$9"
    );
	return tempVariable32;
}


__IRAM void CP3_COUNTER0_START( void )
{

#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN
    __asm__ __volatile__ 
    (
	"la		$8, tempVariable32  	\n\t"
	"lw		$8, 0($8)			    \n\t"
	"ctc3 	$8, $0				    \n\t"
    "la		$8, tempVariable32_2    \n\t"
    "lw		$8, 0($8)			    \n\t"
    "ctc3 	$8, $1				    \n\t" 
    "li     $8, 0xf                 \n\t"
    "ctc3	$8, $2                  \n\t"
    :
    :
    :"$8"
    );
#else
    __asm__ __volatile__ 
    (
	"la		$8, tempVariable32	\n\t"
	"lw		$8, 0($8)			\n\t"
	"ctc3 	$8, $0				\n\t"
    :
    :
    :"$8"	
    );
#endif // PERF_DUMP_CP3_DUAL_COUNTER_EN
}


__IRAM void CP3_COUNTER0_ASSIGN_EVENT( void )
{
#if (PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_OLD)
#if 1 /* Inst */
	tempVariable32 = /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_NEW_INST_FECTH)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_NEW_INST_FETCH_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_NEW_INST_MISS_BUSY_CYCLE)<<24);
#elif 1 /* Data (LOAD+STORE) */
	tempVariable32 = /* Counter0 */((0x10|CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_INST)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
#elif 1 /* Data (STORE) */
	tempVariable32 = /* Counter0 */((0x10|CP3CNT_DATA_LOAD_INST)<< 0) |
	                 /* Counter1 */((0x10|CP3CNT_DATA_STORE_INST)<< 8) |
	                 /* Counter2 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_CACHE_MISS)<<16) |
	                 /* Counter3 */((0x10|CP3CNT_DATA_LOAD_OR_STORE_MISS_BUSY_CYCLE)<<24);
#else
#error
#endif

#elif (PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_NEW)

#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN
    #if 1 /* Inst */
	tempVariable32 = /* Counter0 */((CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((CP3CNT_INST_FECTH)<< 8) |
	                 /* Counter2 */((CP3CNT_ICACHE_MISS)<<16) |
	                 /* Counter3 */((CP3CNT_ICACHE_MISS_CYCLE)<<24);
	tempVariable32_2 = /* Counter0 */((CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((CP3CNT_INST_FECTH)<< 8) |
	                 /* Counter2 */((CP3CNT_ICACHE_MISS)<<16) |
	                 /* Counter3 */((CP3CNT_ICACHE_MISS_CYCLE)<<24);    
    #elif 1 /* Data (LOAD+STORE) */
	tempVariable32 = /* Counter0 */((CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((CP3CNT_LOAD_OR_STORE_INST)<< 8) |
	                 /* Counter2 */((CP3CNT_STORE_INST)<<16) |
	                 /* Counter3 */((CP3CNT_LOAD_INST)<<24);
    #else
    #error
    #endif
#else
    #if 1 /* Inst */
	tempVariable32 = /* Counter0 */((CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((CP3CNT_INST_FECTH)<< 8) |
	                 /* Counter2 */((CP3CNT_ICACHE_MISS)<<16) |
	                 /* Counter3 */((CP3CNT_ICACHE_MISS_CYCLE)<<24);
    #elif 1 /* Data (LOAD+STORE) */
	tempVariable32 = /* Counter0 */((CP3CNT_CYCLES)<< 0) |
	                 /* Counter1 */((CP3CNT_LOAD_OR_STORE_INST)<< 8) |
	                 /* Counter2 */((CP3CNT_STORE_INST)<<16) |
	                 /* Counter3 */((CP3CNT_LOAD_INST)<<24);
    #else
    #error
    #endif
#endif // PERF_DUMP_CP3_DUAL_COUNTER_EN

#else
#error "PERF_DUMP_CP3_SELECT flag error"
#endif

}


__IRAM void CP3_COUNTER0_RESET_ONE(int cnt_num)
{
    switch(cnt_num)
    {
        case 0:
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $8				\n\t"            
            );            
            
        break;            
        case 1:            
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $10				\n\t"            
            );            

        break;            
        case 2:
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $12				\n\t"            
            );            
            
        break;            
        case 3:
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $14				\n\t"            
            );            
            
        break;            
        case 4:        
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $9  			\n\t"            
            );            

        break;            
        case 5:
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $11				\n\t"            
            );            
        
        break;
        case 6:       
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $13				\n\t"            
            );            
        
        break;
        case 7:
            __asm__ __volatile__ 
            (
            	"mtc3 	$0, $15				\n\t"            
            );            
        
        break;
        default:
            printk("CP3 RESET ERROR COUNTER = %x \n",cnt_num);
        break;
    }
}

__IRAM void CP3_COUNTER0_RESET( void )
{
    __asm__ __volatile__ 
    (
    	"mtc3 	$0, $8				\n\t"
        "mtc3 	$0, $9				\n\t"
        "mtc3 	$0, $10				\n\t"
        "mtc3 	$0, $11				\n\t"
        "mtc3 	$0, $12				\n\t"
        "mtc3 	$0, $13				\n\t"
        "mtc3 	$0, $14				\n\t"
        "mtc3 	$0, $15				\n\t"
    );
}

__IRAM void CP3_COUNTER0_STOP( void )
{
#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN
    __asm__ __volatile__ 
    (   
	    "ctc3 	$0, $0			\n\t"
	    "ctc3 	$0, $1			\n\t"	    
    );
#else
    __asm__ __volatile__ 
    (
    	"ctc3 	$0, $0			\n\t"
    );
#endif
}

__IRAM uint64 CP3_COUNTER0_GET( void )
{
    __asm__ __volatile__ 
    (
    	"la		$8, tempVariable64  \n\t"
    	"mfc3	$9, $9			    \n\t"
    	"sw		$9, 0($8)		    \n\t"
    	"mfc3	$9, $8			    \n\t"
    	"sw		$9, 4($8)		    \n\t"
    	:
    	:
        :"$8","$9"
    );
	return tempVariable64;
}

__IRAM void CP3_COUNTER0_GET_ALL( void )
{
__asm__ __volatile__ (
	"mfc3	$9, $9			\n\t"
	"sw		$9, 0x00(%0)	\n\t"
	"mfc3	$9, $8			\n\t"
	"sw		$9, 0x04(%0)	\n\t"
	"mfc3	$9, $11			\n\t"
	"sw		$9, 0x08(%0)	\n\t"
	"mfc3	$9, $10			\n\t"
	"sw		$9, 0x0C(%0)	\n\t"
	"mfc3	$9, $13			\n\t"
	"sw		$9, 0x10(%0)	\n\t"
	"mfc3	$9, $12			\n\t"
	"sw		$9, 0x14(%0)	\n\t"
	"mfc3	$9, $15			\n\t"
	"sw		$9, 0x18(%0)	\n\t"
	"mfc3	$9, $14			\n\t"
	"sw		$9, 0x1C(%0)	\n\t"
    :
    :"r"(currCnt)
    );
}

int32 rtl8651_romeperfInit()
{
	CP3_COUNTER0_INIT();
    CP3_COUNTER0_ASSIGN_EVENT();

	rtl8651_romeperf_inited = TRUE;
	rtl8651_romeperf_enable = TRUE;
	memset( &romePerfStat, 0, sizeof( romePerfStat ) );

#if (PERF_DUMP_INIT_SELECT == PERF_DUMP_INIT_ORI)
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD].desc = "NAPT add_all";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_1].desc = "NAPT add_checkIntIP";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_2].desc = "NAPT add_localServer";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_3].desc = "NAPT add_checkExtIp";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_4].desc = "NAPT add_dupCheck1";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_5].desc = "NAPT add_dupCheck2";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_6].desc = "NAPT add_bPortReused";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_7].desc = "NAPT add_routeCache";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_8].desc = "NAPT add_tooManyConn";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_9].desc = "NAPT add_initConn";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_10].desc = "NAPT add_decisionFlo";
	romePerfStat[ROMEPERF_INDEX_NAPT_ADD_11].desc = "NAPT add_ambiguous";
	romePerfStat[ROMEPERF_INDEX_NAPT_DEL].desc = "NAPT del";
	romePerfStat[ROMEPERF_INDEX_NAPT_FIND_OUTBOUND].desc = "NATP outbound";
	romePerfStat[ROMEPERF_INDEX_NAPT_FIND_INBOUND].desc = "NAPT inbound";
	romePerfStat[ROMEPERF_INDEX_NAPT_UPDATE].desc = "NAPT update";
	romePerfStat[ROMEPERF_INDEX_UNTIL_RXTHREAD].desc = "IntDispatch-RxThread";
	romePerfStat[ROMEPERF_INDEX_RECVLOOP].desc = "RecvLoop-FwdInput";
	romePerfStat[ROMEPERF_INDEX_FWDENG_INPUT].desc = "FwdEng_Input()";
	romePerfStat[ROMEPERF_INDEX_BEFORE_CRYPTO_ENCAP].desc = "FwdInput-Crypto(En)";
	romePerfStat[ROMEPERF_INDEX_ENCAP].desc = "IPSEC Encap";
	romePerfStat[ROMEPERF_INDEX_ENCAP_CRYPTO_ENGINE].desc = "Encap Crypto";
	romePerfStat[ROMEPERF_INDEX_ENCAP_AUTH_ENGINE].desc = "Encap Authtication";
	romePerfStat[ROMEPERF_INDEX_BEFORE_CRYPTO_DECAP].desc = "FwdInput-Crypto(De)";
	romePerfStat[ROMEPERF_INDEX_DECAP].desc = "IPSEC Decap";
	romePerfStat[ROMEPERF_INDEX_DECAP_CRYPTO_ENGINE].desc = "Decap Crypto";
	romePerfStat[ROMEPERF_INDEX_DECAP_AUTH_ENGINE].desc = "Decap Authtication";
	romePerfStat[ROMEPERF_INDEX_FASTPATH].desc = "Fast Path";
	romePerfStat[ROMEPERF_INDEX_SLOWPATH].desc = "Slow Path";
	romePerfStat[ROMEPERF_INDEX_FWDENG_SEND].desc = "FwdEngSend()";
	romePerfStat[ROMEPERF_INDEX_UNTIL_ACLDB].desc = "FwdInput() Until ACLDB";
	romePerfStat[ROMEPERF_INDEX_GET_MTU_AND_SOURCE_MAC].desc = "L3Route_MTU_srcMAC";
	romePerfStat[ROMEPERF_INDEX_PPTPL2TP_1].desc = "L3Route_PPTPL2TP_1";
	romePerfStat[ROMEPERF_INDEX_PPPOE_ARP_CACHE].desc = "L3Route_PPPoE_ArpCache";
	romePerfStat[ROMEPERF_INDEX_PPTPL2TP_SEND].desc = "L3Route_PptpL2tpSend()";
	romePerfStat[ROMEPERF_INDEX_FRAG].desc = "L3Route_Fragment";
	romePerfStat[ROMEPERF_INDEX_EGRESS_ACL].desc = "FwdSend_EgressACL";
	romePerfStat[ROMEPERF_INDEX_PPTPL2TP_ENCAP].desc = "FwdSend_PPTP/L2TP_Encap";
	romePerfStat[ROMEPERF_INDEX_FROM_PS].desc = "FwdSend_FromPS";
	romePerfStat[ROMEPERF_INDEX_EXTDEV_SEND].desc = "FwdSend_ExtDevSend()";
	romePerfStat[ROMEPERF_INDEX_FRAG_2ND_HALF].desc = "FwdSend_Frag_2ndHalf()";
	romePerfStat[ROMEPERF_INDEX_TXPKTPOST].desc = "rtl8651_txPktPostProcessing()";
	romePerfStat[ROMEPERF_INDEX_MBUFPAD].desc = "mBuf_padding()";
	romePerfStat[ROMEPERF_INDEX_TXALIGN].desc = "_swNic_txAlign";
	romePerfStat[ROMEPERF_INDEX_ISRTXRECYCLE].desc = "_swNic_isrTxRecycle";
	romePerfStat[ROMEPERF_INDEX_16].desc = "FwdEng_temp_16";
	romePerfStat[ROMEPERF_INDEX_17].desc = "FwdEng_temp_17";
	romePerfStat[ROMEPERF_INDEX_18].desc = "FwdEng_temp_18";
	romePerfStat[ROMEPERF_INDEX_19].desc = "FwdEng_temp_19";
	romePerfStat[ROMEPERF_INDEX_20].desc = "FwdEng_temp_20";
	romePerfStat[ROMEPERF_INDEX_21].desc = "FwdEng_temp_21";
	romePerfStat[ROMEPERF_INDEX_22].desc = "FwdEng_temp_22";
	romePerfStat[ROMEPERF_INDEX_23].desc = "FwdEng_temp_23";
	romePerfStat[ROMEPERF_INDEX_24].desc = "FwdEng_temp_24";
	romePerfStat[ROMEPERF_INDEX_25].desc = "FwdEng_temp_25";
	romePerfStat[ROMEPERF_INDEX_FLUSHDCACHE].desc = "rtlglue_flushDCache";
	romePerfStat[ROMEPERF_INDEX_IRAM_1].desc = "IRAM Cached within IRAM";
	romePerfStat[ROMEPERF_INDEX_IRAM_2].desc = "IRAM Uncached within IRAM";
	romePerfStat[ROMEPERF_INDEX_IRAM_3].desc = "test ICACHE  (1024*100)";
	romePerfStat[ROMEPERF_INDEX_IRAM_4].desc = "test Uncached (1024*10)";
	romePerfStat[ROMEPERF_INDEX_DRAM_1].desc = "DRAM Cached within DRAM";
	romePerfStat[ROMEPERF_INDEX_DRAM_2].desc = "DRAM Uncached within DRAM";
	romePerfStat[ROMEPERF_INDEX_DRAM_3].desc = "test DCACHE  (1024*100)";
	romePerfStat[ROMEPERF_INDEX_DRAM_4].desc = "test Uncached (1024*10)";
	romePerfStat[ROMEPERF_INDEX_BMP].desc = "KMP Algorithm";
    romePerfStat[ROMEPERF_INDEX_MDCMDIO].desc = "MDCMDIO PHY Register ACCESS";
#elif (PERF_DUMP_INIT_SELECT == PERF_DUMP_INIT_WLAN_TRX)
    romePerfStat[ROMEPERF_INDEX_TX_PREWORK].desc    = "XMIT_PREWORK";
    romePerfStat[ROMEPERF_INDEX_TX_XMIT_OUT].desc   = "XMIT_OUT";
    romePerfStat[ROMEPERF_INDEX_TX_XMIT_OUT_2].desc   = "XMIT_OUT_2";
    romePerfStat[ROMEPERF_INDEX_RX_ONE_PKT].desc    = "RX_ONE_PKT";
    romePerfStat[ROMEPERF_INDEX_RX_ONE_PKT_2].desc   = "RX_ONE_PKT_2";
    romePerfStat[ROMEPERF_INDEX_TX_START_XMIT].desc    = "XMT_1";    
    romePerfStat[ROMEPERF_INDEX_TX_START_XMIT_2].desc    = "XMIT_2";        
    romePerfStat[ROMEPERF_INDEX_TX_START_XMIT_3].desc    = "XMIT_3";            
    romePerfStat[ROMEPERF_INDEX_TX_START_XMIT_4].desc    = "XMIT_4";            
    romePerfStat[ROMEPERF_INDEX_TX_START_XMIT_5].desc    = "XMIT_5";            
#else
#error "PERF_DUMP_INIT_SELECT flag error"
#endif

	return SUCCESS;
}

int32 rtl8651_romeperfReset()
{
	rtl8651_romeperfInit();

	return SUCCESS;
}

#if 0/* old fashion function, for reference only. */
int32 rtl8651_romeperfStart()
{
	if ( rtl8651_romeperf_inited == FALSE ) rtl8651_romeperfInit();

	START_AND_GET_CP3_COUNTER0( cnt1 );

	return SUCCESS;
}

int32 rtl8651_romeperfStop( uint64 *pDiff )
{
	if ( rtl8651_romeperf_inited == FALSE ) rtl8651_romeperfInit();

	STOP_AND_GET_CP3_COUNTER0( cnt2 );

	*pDiff = cnt2 - cnt1;
	return SUCCESS;
}
#endif

int32 rtl8651_romeperfGet( uint64 *pGet )
{
	if ( rtl8651_romeperf_inited == FALSE ) return FAILED;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	*pGet = CP3_COUNTER0_GET();
	CP3_COUNTER0_START();

	return SUCCESS;
}

int32 rtl8651_romeperfPause( void )
{
	if ( rtl8651_romeperf_inited == FALSE ) return FAILED;

	rtl8651_romeperf_enable = FALSE;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();

	return SUCCESS;
}

int32 rtl8651_romeperfResume( void )
{
	if ( rtl8651_romeperf_inited == FALSE ) return FAILED;

	rtl8651_romeperf_enable = TRUE;

	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_START();

	return SUCCESS;
}


#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN

__IRAM int32 rtl_romeperfEnterPoint_dual(uint32 index, int cnt_num,char *event)
{    
    unsigned char i,j,counter;  

	/* Louis patch: someone will disable CP3 in somewhere. */
    CP3_COUNTER0_INIT();
	CP3_COUNTER0_STOP(); 
      
	if ( (rtl8651_romeperf_inited == FALSE) || (rtl8651_romeperf_enable == FALSE) ) {
        //CP3_COUNTER0_START();
        return FAILED;    
    }
    
	if ( index >= (sizeof(romePerfStat)/sizeof(rtl8651_romeperf_stat_t)) )	{
        printk("CP3 index error! \n");        
        CP3_COUNTER0_START();
        return FAILED;
    }

    if(cnt_num + numOfUsedCounter > MAX_CP3_COUNTER) {
        printk("Index=[%x]CP3 counter not enough, request:%x, availableCnt:%x \n",index,cnt_num,(MAX_CP3_COUNTER-numOfUsedCounter));
        CP3_COUNTER0_START();        
        return FAILED;
    }

    if(TRUE == romePerfStat[index].bUsed) {
        printk("CP3 Error : reue the index=%x \n",index);
        CP3_COUNTER0_START();        
        return FAILED;
    }

    // check reCall enter function, but not exit
    if((countTemp!=0xff)&(countTemp != index))
    {
        romePerfStat[index].reEnterIdx = countTemp;
    }    

    countTemp = index; 
    
    romePerfStat[index].numOfCount = cnt_num;        
    
    //printk("ENTER romePerfStat[%x].startCounter =%x \n",index,romePerfStat[index].startCounter);
    
    for(i=0; i<cnt_num ;i++)  
    {
        counter = getAvailableCnt();
        romePerfStat[index].Counter[i] = counter;
        romePerfStat[index].Event[i] = event[i];        
        bCounterUsed[counter] = TRUE;
      //  printk("ENTER index= %x counter =[%x]requestCntNum :%x\n",index,counter,cnt_num);
        CP3_COUNTER0_RESET_ONE(counter);

        setEvent(counter,event[i]);
        numOfUsedCounter++;    
     }

   // printk("ENTER index =%x total numOfUsedCounter =%x\n",index,numOfUsedCounter);

    romePerfStat[index].bUsed = TRUE;
    romePerfStat[index].hasTempCycle = TRUE;
	CP3_COUNTER0_START();

	return SUCCESS;
}

__IRAM int32 rtl_romeperfExitPoint_dual(uint32 index)
{
    unsigned char i,j,counter;  

    CP3_COUNTER0_INIT();
    CP3_COUNTER0_STOP();  

    if ( (rtl8651_romeperf_inited == FALSE) || (rtl8651_romeperf_enable == FALSE) )
    {
    	//CP3_COUNTER0_START();                        
        return FAILED;
    }
    
	if ( index >= (sizeof(romePerfStat)/sizeof(rtl8651_romeperf_stat_t)) )
	{
        printk("CP3 index error! \n");     
    	CP3_COUNTER0_START();                
		return FAILED;
	}
    
	if ( romePerfStat[index].hasTempCycle == FALSE )
	{
        printk("CP3 EXIT error! romePerfStat[%x].hasTempCycle == FALSE \n",index);
    	CP3_COUNTER0_START();        
		return FAILED;
	}

   
    if(countTemp == index)
    {
        countTemp = 0xff;
    }
    
    CP3_COUNTER0_GET_ALL();        

    for(i=0; i< romePerfStat[index].numOfCount; i++)
    {       
        counter = romePerfStat[index].Counter[i];
        //printk("LEAVE counter=[%x] cnt_index =%x romePerfStat[index].numOfCount =%x \n",counter,index,romePerfStat[index].numOfCount);
        if (counter <4) {
            romePerfStat[index].accCycle[i] += (currCnt[counter] & 0x00ffffff);
        } else {  // conter > 4
            romePerfStat[index].accCycle[i] += ((currCnt[counter-4]>>32) & 0x00ffffff);         
        }

        // counter available
        bCounterUsed[counter] = FALSE;
        numOfUsedCounter--;   
    }

    // printk("LEAVE numOfUsedCounter =%x \n",numOfUsedCounter);

    romePerfStat[index].bUsed = FALSE;
	romePerfStat[index].hasTempCycle = FALSE;
	romePerfStat[index].executedNum++;

    //CP3_COUNTER0_RESET_DUAL(cnt_num);
	CP3_COUNTER0_START();

	return SUCCESS;
}

int getAvailableCnt(void)
{
    int i;

    for(i=0;i<MAX_CP3_COUNTER;i++)
    {
        if( bCounterUsed[i] == false)
        {
            return i;
        }
    }

    printk("Not avalable counter !!! \n");
}

int setEvent(int counter,char event)
{   
   if(event >= CP3_CNT_MAX)
   {
        printk("Event over max = %x \n",event);               
        return FAILED;
   }
   
   if(counter < 4)
   {
       tempVariable32 = (tempVariable32 & (~(0xff << (counter*8)))) | ( event << (counter*8));
   }
   else
   {
       tempVariable32_2 = (tempVariable32_2 & (~(0xff << ((counter-4)*8)))) | ( event << ((counter-4)*8));
   }    
}

#else


__IRAM int32 rtl8651_romeperfEnterPoint( uint32 index )
{
	if ( rtl8651_romeperf_inited == FALSE ||
	     rtl8651_romeperf_enable == FALSE ) return FAILED;
	if ( index >= (sizeof(romePerfStat)/sizeof(rtl8651_romeperf_stat_t)) )
		return FAILED;

    if((countTemp!=0xff)&(countTemp != index))
    {
        romePerfStat[index].reEnterIdx = countTemp;
    }
    
    countTemp = index;
    
	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
    CP3_COUNTER0_RESET();    
	CP3_COUNTER0_GET_ALL();

	romePerfStat[index].tempCycle[0] = currCnt[0];
	romePerfStat[index].tempCycle[1] = currCnt[1];
	romePerfStat[index].tempCycle[2] = currCnt[2];
	romePerfStat[index].tempCycle[3] = currCnt[3];

	romePerfStat[index].hasTempCycle = TRUE;

	CP3_COUNTER0_START();

	return SUCCESS;
}


__IRAM int32 rtl8651_romeperfExitPoint( uint32 index )
{
	if ( rtl8651_romeperf_inited == FALSE ||
	     rtl8651_romeperf_enable == FALSE ) return FAILED;
	if ( index >= (sizeof(romePerfStat)/sizeof(rtl8651_romeperf_stat_t)) )
		return FAILED;
	if ( romePerfStat[index].hasTempCycle == FALSE )
		return FAILED;

    if((countTemp == index)||(countTemp == 0))
    {
        countTemp = 0;
    }
  
	/* Louis patch: someone will disable CP3 in somewhere. */
	CP3_COUNTER0_INIT();

	CP3_COUNTER0_STOP();
	CP3_COUNTER0_GET_ALL();

	romePerfStat[index].accCycle[0] += (currCnt[0] - romePerfStat[index].tempCycle[0]);
	romePerfStat[index].accCycle[1] += (currCnt[1] - romePerfStat[index].tempCycle[1]);
	romePerfStat[index].accCycle[2] += (currCnt[2] - romePerfStat[index].tempCycle[2]);
	romePerfStat[index].accCycle[3] += (currCnt[3] - romePerfStat[index].tempCycle[3]);


	romePerfStat[index].hasTempCycle = FALSE;
	romePerfStat[index].executedNum++;

    CP3_COUNTER0_RESET();

	return SUCCESS;
}

#endif //#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN

int32 rtl8651_romeperfDump( int start, int end )
{
#if 0
	int i;

	rtlglue_printf( "index %30s %12s %8s %10s\n", "description", "accCycle", "totalNum", "Average" );
	for( i = start; i <= end; i++ )
	{
		if ( romePerfStat[i].executedNum == 0 )
		{
			rtlglue_printf( "[%3d] %30s %12s %8s %10s\n", i, romePerfStat[i].desc, "--", "--", "--" );
		}
		else
		{
			int j;
			rtlglue_printf( "[%3d] %30s ",
			                i, romePerfStat[i].desc );
			for( j =0; j < sizeof(romePerfStat[i].accCycle)/sizeof(romePerfStat[i].accCycle[0]);
			     j++ )
			{
				uint32 *pAccCycle = (uint32*)&romePerfStat[i].accCycle[j];
				uint32 avrgCycle = /* Hi-word */ (pAccCycle[0]*(0xffffffff/romePerfStat[i].executedNum)) +
				                   /* Low-word */(pAccCycle[1]/romePerfStat[i].executedNum);

				rtlglue_printf( "%12llu %8u %10u\n",
				                romePerfStat[i].accCycle[j],
				                romePerfStat[i].executedNum,
				                avrgCycle
				                );
				rtlglue_printf( " %3s  %30s ", "", "" );
			}
			rtlglue_printf( "\r" );
		}
	}

	return SUCCESS;
#else
	int i,bReturn;

	rtl8651_romeperf_stat_t* statSnapShot = rtlglue_malloc(sizeof(rtl8651_romeperf_stat_t) * (end - start + 1) );
	if( statSnapShot == NULL )
	{
		rtlglue_printf("statSnapShot mem alloc failed\n");
		return FAILED;
	}

	rtlglue_printf( "index %30s %30s %12s %8s %10s %8s\n", "description","Event","accCycle", "totalNum", "Average","reENTERIdx" );

	for( i = start; i <= end; i++ )
	{
		int j;
		for( j =0; j < sizeof(romePerfStat[i].accCycle)/sizeof(romePerfStat[i].accCycle[0]); j++ )
		{
			statSnapShot[i].accCycle[j]  = romePerfStat[i].accCycle[j];
			statSnapShot[i].tempCycle[j] = romePerfStat[i].tempCycle[j];
		}
		statSnapShot[i].executedNum  = romePerfStat[i].executedNum;
		statSnapShot[i].hasTempCycle = romePerfStat[i].hasTempCycle;
	}

	for( i = start; i < end; i++ )
	{
		if ( statSnapShot[i].executedNum == 0 )
		{
			rtlglue_printf( "[%3d] %30s %30s %12s %8s %10s %8s\n", i, romePerfStat[i].desc, "--", "--", "--", "--", "--","--" );
		}
		else
		{
			int j;
			rtlglue_printf( "[%3d] %30s ", i, romePerfStat[i].desc );
#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN            
            for( j =0; j < romePerfStat[i].numOfCount; j++ )
#else
			for( j =0; j < sizeof(statSnapShot[i].accCycle)/sizeof(statSnapShot[i].accCycle[0]); j++ )
#endif //#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN                            
			{
				uint32 *pAccCycle = (uint32*)&statSnapShot[i].accCycle[j];
				uint32 avrgCycle = /* Hi-word */ (pAccCycle[0]*(0xffffffff/statSnapShot[i].executedNum)) +
				                   /* Low-word */(pAccCycle[1]/statSnapShot[i].executedNum);

    				rtlglue_printf( "%30s %12llu %8u %10u %8x\n",
#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN  
                    mapping[romePerfStat[i].Event[j]],
#else
                    mapping[getEventIndex(j)],                    
#endif //#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN                            
				statSnapShot[i].accCycle[j],
				statSnapShot[i].executedNum,
    				avrgCycle,
    				romePerfStat[i].reEnterIdx);

				rtlglue_printf( " %3s  %30s ", "", "" );
			}
			rtlglue_printf( "\r" );
		}
	}

	rtlglue_free(statSnapShot);

	return SUCCESS;
#endif
}

////////////////////////////////////////////////////////////////////////////////

int getEventIndex(int i)
{
    int index;
#if (PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_OLD)
    index = (tempVariable32 >> (i*8))&0xf;    
#else 
#ifdef PERF_DUMP_CP3_DUAL_COUNTER_EN
    if(i < 4)
    {
        index = (tempVariable32 >> (i*8))&0xff;    
    }
    else
    {
        index = (tempVariable32_2 >> ((i-4)*8))&0xff;            
    }
#else
    index = (tempVariable32 >> (i*8))&0xff;    
#endif // PERF_DUMP_CP3_DUAL_COUNTER_EN
#endif //(PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_OLD)
    return index;
}


#endif // PERF_DUMP

