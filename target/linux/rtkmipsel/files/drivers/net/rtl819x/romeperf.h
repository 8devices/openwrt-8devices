/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Performance Profiling Header File for ROME Driver
* Abstract : 
* Author : Yung-Chieh Lo (yjlou@realtek.com.tw)               
* $Id: romeperf.h,v 1.1 2007-12-21 10:28:22 davidhsu Exp $
*/

#ifndef _ROMEPERF_H_
#define _ROMEPERF_H_

#include "net/rtl/rtl_types.h"

#if defined(CONFIG_RTL865X_MODULE_ROMEDRV)
#define	rtl8651_romeperfEnterPoint				module_internal_rtl8651_romeperfEnterPoint
#define	rtl8651_romeperfExitPoint					module_internal_rtl8651_romeperfExitPoint
#endif

struct rtl8651_romeperf_stat_s {
	char *desc;
	uint64 accCycle[4];
	uint64 tempCycle[4];
	uint32 executedNum;
	uint32 hasTempCycle:1; /* true if tempCycle is valid. */
};
typedef struct rtl8651_romeperf_stat_s rtl8651_romeperf_stat_t;


/* for rtl8651_romeperfEnterPoint() and rtl8651_romeperfExitPoint() */
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
#define ROMEPERF_INDEX_TEST0    70
#define ROMEPERF_INDEX_TEST1    71
#define ROMEPERF_INDEX_TEST2    72
#define ROMEPERF_INDEX_TEST3    73
#define ROMEPERF_INDEX_TEST4    74

#define ROMEPERF_INDEX_PCIE_IRQ 75
#define ROMEPERF_INDEX_RX_DSR 76
#define ROMEPERF_INDEX_RX_ISR 77
#define ROMEPERF_INDEX_validate_mpdu 78

#define ROMEPERF_INDEX_swNic_receive 79
#define ROMEPERF_INDEX_rtl_processReceivedInfo 80
#define ROMEPERF_INDEX_rtl_decideRxDevice 81
#define ROMEPERF_INDEX_rtl_processRxFrame 82
#define ROMEPERF_INDEX_rtl_processBridgeShortCut 83
#define ROMEPERF_INDEX_rtl_processRxToProtcolStack 84
#define ROMEPERF_INDEX_netif_receive_skb 85
#define ROMEPERF_INDEX_FastPath_Enter 86
#define ROMEPERF_INDEX_checksum_in_fastpath 87
#define ROMEPERF_INDEX_enter_fast_path 88
#define ROMEPERF_INDEX_ip_finish_output3 89
#define ROMEPERF_INDEX_ndo_start_xmit 90
#define ROMEPERF_INDEX_rtl_preProcess_xmit 91
#define ROMEPERF_INDEX_rtl_fill_txInfo 92
#define ROMEPERF_INDEX_swNic_send 93
#define ROMEPERF_INDEX_swNic_txDone 94
#define ROMEPERF_INDEX_dev_kfree_skb_any 95
#define ROMEPERF_INDEX_fastpath_1 96
#define ROMEPERF_INDEX_fastpath_2 97
#define ROMEPERF_INDEX_fastpath_3 98


#define ROMEPERF_INDEX_MAX 99

int32 rtl8651_romeperfInit( void );
inline int32 rtl8651_romeperfStart(void);
inline int32 rtl8651_romeperfStop(void);
int32 rtl8651_romeperfReset( void );
int rtl8651_romeperfEnterPoint( int index );
int rtl8651_romeperfExitPoint( int index );
int32 rtl8651_romeperfDump( int start, int end );
int32 rtl8651_romeperfPause( void );
int32 rtl8651_romeperfResume( void );
int32 rtl8651_romeperfGet( uint64 *pGet );

extern rtl8651_romeperf_stat_t romePerfStat[ROMEPERF_INDEX_MAX];



#endif/* _ROMEPERF_H_ */
