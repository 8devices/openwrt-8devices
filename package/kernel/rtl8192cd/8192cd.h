/*
 *  Header file defines some private structures and macro
 *
 *  $Id: 8192cd.h,v 1.56.2.38 2011/01/13 14:38:36 pluswang Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef	_8192CD_H_
#define _8192CD_H_

#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <../net/bridge/br_private.h>

#ifdef HS2_SUPPORT
#include <linux/in6.h>
#endif


#include "./8192cd_cfg.h"



#ifdef SUPPORT_SNMP_MIB
#include "./8192cd_mib.h"
#endif


#ifdef BT_COEXIST_SOCKET
#include "btcoexist/halbtc_socket.h"
#include "btcoexist/rtl_btc.h"
#endif

#define TRUE		1
#define FALSE		0

#define CONGESTED	2
#ifdef SUCCESS
#undef SUCCESS
#endif
#define SUCCESS		1
#define FAIL		0

#if 1//def __ECOS
#ifndef FAILED
#define FAILED -1
#endif
#endif

#ifndef CONFIG_RTL_WLAN_STATUS
#define CONFIG_RTL_WLAN_STATUS
#endif

#if 0
typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned long	UINT32;

typedef signed char		INT8;
typedef signed short	INT16;
typedef signed long		INT32;

typedef unsigned int	UINT;
typedef signed int		INT;

typedef unsigned long long	UINT64;
typedef signed long long	INT64;
#endif

struct stat_info;


#include "./ieee802_mib.h"

#include "./wifi.h"
#include "./Wlan_TypeDef.h"
#include "./Wlan_QoSType.h"
#include "./8192cd_security.h"

#include "./8192cd_br_ext.h"

#include "./8192cd_hw.h"


#include "phydm/phydm_types.h"
#include "./odm_inc.h"
#include "phydm/phydm.h"
#include "./EdcaTurboCheck.h"

#include "./8192cd_psk.h"

#ifdef WIFI_SIMPLE_CONFIG
#ifdef INCLUDE_WPS
#include "./wps/wsc.h"
#endif
#endif



#include "WlanHAL/HalMac88XX/halmac_api.h"


//RTK_NL80211
#include <net/cfg80211.h>

#define MAX_WPA_IE_LEN		60+5
#define MAX_RSN_IE_LEN		60+5



#define DWNGRADE_PROBATION_TIME		3
#define UPGRADE_PROBATION_TIME		3
#define TRY_RATE_FREQ				6

#define MAX_PRE_ALLOC_SKB_NUM	32

#define NR_NET80211_UP				8
#define NET80211_TU_TO_US			1024		// unit:us

#if defined(CONFIG_PRINTK)
#define panic_printk         printk
#elif !defined(CONFIG_PANIC_PRINTK)
#define panic_printk		scrlog_printk
#endif

#ifdef HIGH_POWER_EXT_PA
#define HP_OFDM_POWER_MAX		(priv->pshare->rf_ft_var.hp_ofdm_pwr_max)
#define HP_CCK_POWER_MAX		(priv->pshare->rf_ft_var.hp_cck_pwr_max)
#define HP_OFDM_POWER_DEFAULT	8//20
#define HP_CCK_POWER_DEFAULT	10//16
#endif

#ifdef USB_POWER_SUPPORT
#define USB_HT_2S_DIFF			14
#define USB_RA_MASK				0x1e0fffff		// disable MCS 12, 11, 10, 9, 8
#endif

#define NAT25_FILTER_ETH_NUM  20
#define NAT25_FILTER_IPPROTO_NUM  30

#if (BEAMFORMING_SUPPORT == 1)

#define NUM_TXBF_SU		2
#define NUM_TXBF_MU		6
#define NUM_TXBF_ALL	(NUM_TXBF_SU+NUM_TXBF_MU)

#define TXBF_TYPE_SU 0
#define TXBF_TYPE_MU 1

// <Note> Need to be defined by IC
#define SU_SOUNDING_TIMEOUT			10	// unit: ms
#if (MU_BEAMFORMING_SUPPORT == 1)
#define MU_SOUNDING_TIMEOUT			10	// unit: ms
#endif

#define GetInitSoundCnt(_SoundPeriod, _MinSoundPeriod) ((_SoundPeriod) /(_MinSoundPeriod))

#define MAX_NUM_BEAMFORMEE_SU	2
#define MAX_NUM_BEAMFORMER_SU	2
#if (MU_BEAMFORMING_SUPPORT == 1)
#define MAX_NUM_BEAMFORMEE_MU	6
#define MAX_NUM_BEAMFORMER_MU	1
#endif


//#if defined(CONFIG_WLAN_HAL_8822BE)
#if (MU_BEAMFORMING_SUPPORT == 1)
#define BEAMFORMEE_ENTRY_NUM		(MAX_NUM_BEAMFORMEE_SU + MAX_NUM_BEAMFORMEE_MU)
#define BEAMFORMER_ENTRY_NUM		2

#define TOTAL_BEAMFORMEE_ENTRY_NUM	8 //(2+6)
#else
#define BEAMFORMEE_ENTRY_NUM		2
#define BEAMFORMER_ENTRY_NUM		2

#define TOTAL_BEAMFORMEE_ENTRY_NUM	2
#endif


typedef struct _DEL_ENTRY_LIST_BY_MACADDR{
    u1Byte BFerEntry_Idx0[6];
    u1Byte BFerEntry_Idx1[6];
    u1Byte BFeeEntry_Idx0[6];
    u1Byte BFeeEntry_Idx1[6];
}DEL_ENTRY_LIST_BY_MACADDR,*PDEL_ENTRY_LIST_BY_MACADDR;


typedef enum _DEL_ENTRY_TYPE_SEL{
    BFerEntry ,
    BFeeEntry ,
    BFerBFeeEntry ,
}DEL_ENTRY_TYPE_SEL,*PDEL_ENTRY_TYPE_SEL;


typedef enum _ACT_CATEGORY {
	ACT_CAT_SPECTRUM_MGNT = 0,		// Spectrum management
	ACT_CAT_QOS	= 1,				// Qos
	ACT_CAT_DLS	= 2,				// Direct Link Protocol (DLS)
	ACT_CAT_BA = 3,					// Block Ack
	ACT_CAT_PUBLIC = 4,				// Public
	ACT_CAT_RM = 5,					// Radio Measurement (RM)
	ACT_CAT_FT = 6,					// Fast BSS Transition
	ACT_CAT_HT = 7,					// High Throughput
	ACT_CAT_SAQ = 8,				// Security Association Query
	ACT_CAT_SAQ_PD_PUBLIC = 9,		// Protected Dual of Public Action
	ACT_CAT_TDLS 	= 12,				// Tunneled Direct Link Setup
	ACT_CAT_WMM	= 17,				// WMM
	ACT_CAT_VHT	= 21, 				// VHT
	ACT_CAT_VENDOR_PROTECT = 126,	// Vendor-specific Protected
	ACT_CAT_VENDOR = 127,			// Vendor-specific
} ACT_CATEGORY, *PACT_CATEGORY;


typedef	enum	_TYPE_SUBTYPE {
	// Management Frame
	Type_Asoc_Req		= 0x00,
	Type_Asoc_Rsp		= 0x10,
	Type_Reasoc_Req	= 0x20,
	Type_Reasoc_Rsp	= 0x30,
	Type_Probe_Req		= 0x40,
	Type_Probe_Rsp		= 0x50,
	Type_Beacon		= 0x80,
	Type_Atim			= 0x90,
	Type_Disasoc		= 0xa0,
	Type_Auth			= 0xb0,
	Type_Deauth		= 0xc0,
	Type_Action			= 0xd0,
	Type_Action_No_Ack	= 0xe0,

	// Control Frame
	Type_Beamforming_Report_Poll = 0x44, //= MkString(S8(0,0,1,0,0,0,1,0));
	Type_NDPA			= 0x54,//= MkString(S8(0,0,1,0,1,0,1,0));
	Type_BlockAckReq	= 0x84,//= MkString(S8(0,0,1,0,0,0,0,1));
	Type_BlockAck		= 0x94,//= MkString(S8(0,0,1,0,1,0,0,1));
	Type_PS_poll		= 0xa4,//= MkString(S8(0,0,1,0,0,1,0,1));
	Type_RTS			= 0xb4,//= MkString(S8(0,0,1,0,1,1,0,1));
	Type_CTS			= 0xc4,//= MkString(S8(0,0,1,0,0,0,1,1));
	Type_Ack			= 0xd4,//= MkString(S8(0,0,1,0,1,0,1,1));
	Type_Cfend			= 0xe4,//= MkString(S8(0,0,1,0,0,1,1,1));
	Type_Cfend_Ack		= 0xf4,//= MkString(S8(0,0,1,0,1,1,1,1));

	// Data Frame
	Type_Data			= 0x08,//= MkString(S8(0,0,0,1,0,0,0,0));
	Type_Data_Ack		= 0x18,//= MkString(S8(0,0,0,1,1,0,0,0));
	Type_Data_Poll		= 0x28,//= MkString(S8(0,0,0,1,0,1,0,0));
	Type_Data_Poll_Ack	= 0x38,//= MkString(S8(0,0,0,1,1,1,0,0));
	Type_Null_Frame	= 0x48,//= MkString(S8(0,0,0,1,0,0,1,0));
	Type_Cfack			= 0x58,//= MkString(S8(0,0,0,1,1,0,1,0));
	Type_Cfpoll			= 0x68,//= MkString(S8(0,0,0,1,0,1,1,0));
	Type_Cfpoll_Ack		= 0x78,//= MkString(S8(0,0,0,1,1,1,1,0));
	Type_QosData		= 0x88,//= MkString(S8(0,0,0,1,0,0,0,1));
	Type_QData_Ack		= 0x98,//= MkString(S8(0,0,0,1,1,0,0,1));
	Type_QData_Poll		= 0xa8,//= MkString(S8(0,0,0,1,0,1,0,1));
	Type_QData_Poll_Ack = 0xb8, //= MkString(S8(0,0,0,1,1,1,0,1));
	Type_QosNull		= 0xc8,//= MkString(S8(0,0,0,1,0,0,1,1));
	// Note: 0xd8 is reserved in 11e/13.0.
	Type_QosCfpoll		= 0xe8,//= MkString(S8(0,0,0,1,0,1,1,1));
	Type_QosCfpoll_Ack	= 0xf8,//= MkString(S8(0,0,0,1,1,1,1,1));
} TYPE_SUBTYPE, *PTYPE_SUBTYPE;

typedef enum _BEAMFORMING_ENTRY_STATE
{
	BEAMFORMING_ENTRY_STATE_UNINITIALIZE,
	BEAMFORMING_ENTRY_STATE_INITIALIZEING,
	BEAMFORMING_ENTRY_STATE_INITIALIZED,
	BEAMFORMING_ENTRY_STATE_PROGRESSING,
	BEAMFORMING_ENTRY_STATE_PROGRESSED,
}BEAMFORMING_ENTRY_STATE, *PBEAMFORMING_ENTRY_STATE;
typedef enum _BEAMFORMING_STATE
{
	BEAMFORMING_STATE_END,
	BEAMFORMING_STATE_START_1BFee,
	BEAMFORMING_STATE_START_2BFee
} BEAMFORMING_STATE, *PBEAMFORMING_STATE;

typedef enum _BEAMFORMING_NOTIFY_STATE
{
	BEAMFORMING_NOTIFY_NONE,
	BEAMFORMING_NOTIFY_ADD,
	BEAMFORMING_NOTIFY_DELETE,
	BEAMFORMEE_NOTIFY_ADD_SU,
	BEAMFORMEE_NOTIFY_DELETE_SU,
	BEAMFORMEE_NOTIFY_ADD_MU,
	BEAMFORMEE_NOTIFY_DELETE_MU,
	BEAMFORMING_NOTIFY_RESET
} BEAMFORMING_NOTIFY_STATE, *PBEAMFORMING_NOTIFY_STATE;

//eric-txbf ??
typedef enum _BEAMFORMING_CAP
{
	BEAMFORMING_CAP_NONE = 0x0,
	BEAMFORMER_CAP_HT_EXPLICIT = BIT1,
	BEAMFORMEE_CAP_HT_EXPLICIT = BIT2,
	BEAMFORMER_CAP_VHT_SU = BIT5,			/* Self has er Cap, because Reg er  & peer ee */
	BEAMFORMEE_CAP_VHT_SU = BIT6,			/* Self has ee Cap, because Reg ee & peer er */
	BEAMFORM_CAP_VHT_MU_BFER = BIT7,			/* Self has er Cap, because Reg er  & peer ee */
	BEAMFORM_CAP_VHT_MU_BFEE = BIT8,			/* Self has ee Cap, because Reg ee & peer er */
	BEAMFORMER_CAP = BIT9,
	BEAMFORMEE_CAP = BIT10,
}BEAMFORMING_CAP, *PBEAMFORMING_CAP;
typedef enum _SOUNDING_MODE
{
	SOUNDING_SW_VHT_TIMER = 0x0,
	SOUNDING_SW_HT_TIMER = 0x1,
	SOUNDING_STOP_All_TIMER = 0x2,
	SOUNDING_HW_VHT_TIMER = 0x3,
	SOUNDING_HW_HT_TIMER = 0x4,
	SOUNDING_STOP_OID_TIMER = 0x5,
	SOUNDING_AUTO_VHT_TIMER = 0x6,
	SOUNDING_AUTO_HT_TIMER = 0x7,
	SOUNDING_FW_VHT_TIMER = 0x8,
	SOUNDING_FW_HT_TIMER = 0x9
}SOUNDING_MODE, *PSOUNDING_MODE;

// The sounding state is recorded by BFer.
typedef enum _SOUNDING_STATE{
	SOUNDING_STATE_NONE = 0,
	SOUNDING_STATE_INIT = 1,
	SOUNDING_STATE_SU_START = 2,
	SOUNDING_STATE_SU_SOUNDDOWN = 3,
	SOUNDING_STATE_MU_START = 4,
	SOUNDING_STATE_MU_SOUNDDOWN = 5,
	SOUNDING_STATE_SOUNDING_TIMEOUT = 6,
	SOUNDING_STATE_MAX
}SOUNDING_STATE, *PSOUNDING_STATE;

typedef enum _BEAMFORM_ENTRY_HW_STATE{
	BEAMFORM_ENTRY_HW_STATE_NONE,
	BEAMFORM_ENTRY_HW_STATE_ADD_INIT,
	BEAMFORM_ENTRY_HW_STATE_ADDING,
	BEAMFORM_ENTRY_HW_STATE_ADDED,
	BEAMFORM_ENTRY_HW_STATE_DELETE_INIT,
	BEAMFORM_ENTRY_HW_STATE_DELETING,
	//BEAMFORM_ENTRY_HW_STATE_DELETED,
	BEAMFORM_ENTRY_HW_STATE_MAX
}BEAMFORM_ENTRY_HW_STATE, *PBEAMFORM_ENTRY_HW_STATE;

typedef struct _RT_BEAMFORMING_ENTRY {
	BOOLEAN bUsed;
	BOOLEAN	 bTxBF;
	BOOLEAN bSound;
	BOOLEAN	Sounding_En;	// When STA is BFee entry, 1->activate sounding, 0-->deactivate sounding
	u2Byte	AID;
	u2Byte	MacId;		// Used to Set Reg42C in IBSS mode.
	u2Byte	P_AID;
	u1Byte	MacAddr[6];
	ODM_BW_E	BW;	// Sounding BandWidth
	u2Byte					SoundPeriod;
	BEAMFORMING_CAP			BeamformEntryCap;
	BEAMFORMING_ENTRY_STATE	BeamformEntryState;
	BOOLEAN 				bBeamformingInProgress;

	BEAMFORM_ENTRY_HW_STATE	HwState;

	u1Byte			SoundCnt;	// The BFee need to be sounded when count to zero.
	BOOLEAN			bCandidateSoundingPeer;
	BOOLEAN			bSoundingTimeout;
	BOOLEAN			bDeleteSounding;

      // LogStatusFailCnt is used to record if we receive CSI or not
	u1Byte	LogStatusFailCnt;
	u1Byte	CompSteeringNumofBFer;
	struct stat_info	*pSTA;
	struct rtl8192cd_priv *priv;

	u1Byte	su_reg_index;

	/*For MU-MIMO*/
#if (MU_BEAMFORMING_SUPPORT == 1)
	BOOLEAN	is_mu_sta;
	u1Byte	mu_reg_index;
	u1Byte	gid_valid[8];
	u1Byte	user_position[16];
	u1Byte  gid_mgnt_retryCnt;
	u1Byte  mu_tx_rate;
#endif
} RT_BEAMFORMING_ENTRY, *PRT_BEAMFORMING_ENTRY;
typedef struct _RT_BEAMFORMER_ENTRY {
	BOOLEAN 			bUsed;
	u2Byte				P_AID;
	u1Byte				MacAddr[6];
	BEAMFORMING_CAP	BeamformEntryCap;
	u1Byte				NumofSoundingDim;
    u1Byte                      ClockResetTimes;
    // Below variable are used to record if BFer retry NDPA or not
        u1Byte                      NDPAPreLogSeq;
        u1Byte                      NDPALogSeq;
        u1Byte                      NDPALogRetryCnt;
        u1Byte                      NDPALogSuccess;
	u1Byte			su_reg_index;
	BEAMFORM_ENTRY_HW_STATE	HwState;
#if (MU_BEAMFORMING_SUPPORT == 1)
	 /*For MU-MIMO*/
	BOOLEAN 		is_mu_ap;
	u1Byte			gid_valid[8];
	u1Byte			user_position[16];
	u2Byte			AID;
#endif
} RT_BEAMFORMER_ENTRY, *PRT_BEAMFORMER_ENTRY;
typedef struct _RT_BEAMFORMING_TIMER_INFO {
	u1Byte			Mode;
	ODM_BW_E		BW;
	u1Byte			BeamCount;
	u2Byte			BeamPeriod;
} RT_BEAMFORMING_TIMER_INFO, *PRT_BEAMFORMING_TIMER_INFO;
typedef struct _RT_BEAMFORMING_TEST_INFO {
	BOOLEAN			bBeamAutoTest;
	u1Byte			BeamTestCount;
	u1Byte			BeamTestThreshold;
	u2Byte			BeamTestPeriod;
} RT_BEAMFORMING_TEST_INFO, *PRT_BEAMFORMING_TEST_INFO;
typedef struct _RT_BEAMFORMING_PERIOD_INFO {
	u1Byte			Idx;
	SOUNDING_MODE	Mode;
	u1Byte			BW;
	u2Byte			BeamPeriod;
} RT_BEAMFORMING_PERIOD_INFO, *PRT_BEAMFORMING_PERIOD_INFO;

typedef struct _RT_SOUNDING_INFOV2{
	u1Byte				SUSoundingList[MAX_NUM_BEAMFORMEE_SU];
#if (MU_BEAMFORMING_SUPPORT == 1)
	u1Byte				MUSoundingList[MAX_NUM_BEAMFORMEE_MU];
#endif
	//u1Byte				CurSUBFeeCnt;	// For one sounding period, the selecting SU BFee count which has not been sound.
	//u1Byte				CurMUBFeeCnt;
	SOUNDING_STATE		State;
	u1Byte				SUBFeeCurIdx;
#if (MU_BEAMFORMING_SUPPORT == 1)
	u1Byte				CandidateMUBFeeCnt;
#endif
	// For sounding schedule maintenance
	u2Byte				MinSoundingPeriod;
	u1Byte				SoundRemainCntPerPeriod;	// Get from sounding list. Ex: SU STA1, SU STA2, MU STA(1~n) => the value will be 2+1=3.
	u1Byte				SUSoundNumPerPeriod;		// Real number of SU sound per period.
#if (MU_BEAMFORMING_SUPPORT == 1)
	u1Byte				MUSoundNumPerPeriod;		// Real number of MU sound per period.
#endif
}RT_SOUNDING_INFOV2, *PRT_SOUNDING_INFOV2;

typedef struct _RT_BEAMFORMING_INFO {
	BEAMFORMING_CAP			BeamformCap;
	BEAMFORMING_STATE			BeamformState;
	RT_BEAMFORMING_ENTRY 		BeamformeeEntry[TOTAL_BEAMFORMEE_ENTRY_NUM];
	RT_BEAMFORMER_ENTRY		BeamformerEntry[BEAMFORMER_ENTRY_NUM];
	u1Byte						BeamformeeCurIdx;
	RT_TIMER					BeamformingTimer;
	RT_TIMER					BFSoundingTimeoutTimer;
	RT_BEAMFORMING_TIMER_INFO	BeamformingTimerInfo[TOTAL_BEAMFORMEE_ENTRY_NUM];
	RT_BEAMFORMING_PERIOD_INFO	BeamformingPeriodInfo;
	u1Byte						BeamformingPeriodState;   //  add by Gary
	DEL_ENTRY_TYPE_SEL               CurDelBFerBFeeEntrySel;          // add by Jeffery
	DEL_ENTRY_LIST_BY_MACADDR   DelEntryListByMACAddr;
	// For sounding
	RT_SOUNDING_INFOV2		SoundingInfoV2;

	u1Byte					beamformee_su_cnt;
	u1Byte					beamformer_su_cnt;
	u4Byte					beamformee_su_reg_maping;
	u4Byte					beamformer_su_reg_maping;
	/*For MU-MINO*/
#if (MU_BEAMFORMING_SUPPORT == 1)
	u1Byte					beamformee_mu_cnt;
	u1Byte					beamformer_mu_cnt;
	u4Byte					beamformee_mu_reg_maping;
	u4Byte					beamformee_mu_reg_maping_old;
	u1Byte					active_gid[30];
	u1Byte					FirstMUBFeeIndex;

	u4Byte					RegMUTxCtrl;		/* For USB/SDIO interfaces aync I/O  */
	BOOLEAN					is_mu_sounding;
	u1Byte					mu_ap_index;
#endif
	u1Byte					SetHalSoundownOnDemandCnt;
	BOOLEAN					bSetBFHwConfigInProgess;
} RT_BEAMFORMING_INFO, *PRT_BEAMFORMING_INFO;

typedef struct _RT_NDPA_STA_INFO {
	u2Byte	AID: 12;
	u2Byte	FeedbackType: 1;
	u2Byte	NcIndex: 3;
} RT_NDPA_STA_INFO, *PRT_NDPA_STA_INFO;

#endif

typedef enum _WLAN_Support_Ability_Definition
{
    WLAN_BEAMFORMING_SUPPORT	= BIT0,
    WLAN_MULTIMACCLONE_SUPPORT  = BIT1,
}WLAN_ABILITY_E;

#ifdef GPIO_WAKEPIN
#define PABCD_ISR  0xB8003510
#define PEFGH_ISR  0xB800352c
#endif




#if defined(HS2_SUPPORT)
/* Hotspot 2.0 Release 1 */
#define ETHER_TDLS 0x890d
#endif

#define	RF_PATH_A		0		//Radio Path A
#define	RF_PATH_B		1		//Radio Path B
#define	RF_PATH_C		2		//Radio Path C
#define	RF_PATH_D		3		//Radio Path D

// for 802.3 and 802.11 header conversion
#define WLAN_PKT_FORMAT_ENCAPSULATED	0x01
#define WLAN_PKT_FORMAT_SNAP_RFC1042	0x02
#define WLAN_PKT_FORMAT_SNAP_TUNNEL		0x03
#define WLAN_PKT_FORMAT_IPX_TYPE4		0x04
#define WLAN_PKT_FORMAT_APPLETALK		0x05
#define WLAN_PKT_FORMAT_CDP				0x06
#define WLAN_PKT_FORMAT_OTHERS			0x07

// for packet aggregation
#define FG_AGGRE_MPDU				1
#define FG_AGGRE_MPDU_BUFFER_FIRST	2
#define FG_AGGRE_MPDU_BUFFER_MID	3
#define FG_AGGRE_MPDU_BUFFER_LAST	4
#define FG_AGGRE_MSDU_FIRST			5
#define FG_AGGRE_MSDU_MIDDLE		6
#define FG_AGGRE_MSDU_LAST			7

#ifdef SDIO_AP_OFFLOAD
#define OFFLOAD_BUF  ((RTL8192CD_NUM_VWLAN+1)*1024)
#define OFFLOAD_DURATION  128		// UNIT:us, min:128us

enum {
	TRIGGER_TIME_NONE = 0,
	TRIGGER_TIME_8MILISEC,
	TRIGGER_TIME_2SEC,
};

#if OFFLOAD_DURATION <= 8000
#define PULSE_DURATION			(OFFLOAD_DURATION/64)	//duration unit: 64us
#define REPEAT_TRIGGER_UNIT		TRIGGER_TIME_8MILISEC
#define REPEAT_TRIGGER_DURATION	50
#elif OFFLOAD_DURATION < 32000
#define PULSE_DURATION			(128 + 1)				//duration unit: 32ms
#define REPEAT_TRIGGER_UNIT		TRIGGER_TIME_2SEC
#define REPEAT_TRIGGER_DURATION	1
#else
#define PULSE_DURATION			(128 + OFFLOAD_DURATION/32000)
#define REPEAT_TRIGGER_UNIT		TRIGGER_TIME_2SEC
#define REPEAT_TRIGGER_DURATION	(OFFLOAD_DURATION/2000000 + 1)
#endif

enum {
	RTW_STS_SUSPEND=0,
	RTW_STS_NORMAL,
	RTW_STS_REP,
};

enum {
	RTW_PM_AWAKE=0,
	RTW_PM_PREPROCESS,
	RTW_PM_START,
	RTW_PM_SUSPEND,
};

enum {
	RTW_ACT_IDLE=0,
	RTW_ACT_ASSOC,
	RTW_ACT_DISASSOC,
	RTW_ACT_POWERON,
	RTW_ACT_POWERDOWN,
	RTW_ACT_SETREP,
};

enum {
	OFFLOAD_PROHIBIT_USER = BIT0,
	OFFLOAD_PROHIBIT_SITE_SURVEY = BIT1,
	OFFLOAD_PROHIBIT_WPS = BIT2,
	OFFLOAD_PROHIBIT_DEV_OPS = BIT3,
	OFFLOAD_PROHIBIT_MGT = BIT4,
};

struct ap_offload_resource {
	u16 beacon_offset;
	u16 probe_offset;
};

#endif // SDIO_AP_OFFLOAD

#define MANAGEMENT_AID				0

enum wifi_state {
	WIFI_NULL_STATE		=	0x00000000,
	WIFI_ASOC_STATE		=	0x00000001,
	WIFI_REASOC_STATE	=	0x00000002,
	WIFI_SLEEP_STATE	=	0x00000004,
	WIFI_STATION_STATE	=	0x00000008,
	WIFI_AP_STATE		=	0x00000010,
	WIFI_ADHOC_STATE	=	0x00000020,
	WIFI_AUTH_NULL		=	0x00000100,
	WIFI_AUTH_STATE1	= 	0x00000200,
	WIFI_AUTH_SUCCESS	=	0x00000400,
	WIFI_SITE_MONITOR	=	0x00000800,		//to indicate the station is under site surveying

#ifdef MP_TEST
	WIFI_MP_STATE					= 0x00010000,
	WIFI_MP_CTX_BACKGROUND			= 0x00020000,	// in continuous tx background
	WIFI_MP_CTX_BACKGROUND_PENDING	= 0x00040000,	// pending in continuous tx background due to out of skb
	WIFI_MP_CTX_PACKET				= 0x00080000,	// in packet mode
	WIFI_MP_CTX_ST					= 0x00100000,	// in continuous tx with single-tone
	WIFI_MP_CTX_SCR					= 0x00200000,	// in continuous tx with single-carrier
	WIFI_MP_CTX_CCK_CS				= 0x00400000,	// in cck continuous tx with carrier suppression
	WIFI_MP_CTX_OFDM_HW				= 0x00800000,	// in ofdm continuous tx
	WIFI_MP_RX							= 0x01000000,
	WIFI_MP_ARX_FILTER				= 0x02000000,
#if 1//def CONFIG_RTL8672
	WIFI_MP_CTX_BACKGROUND_STOPPING	= 0x04000000,	// stopping ctx
#endif
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef INCLUDE_WPS
	WIFI_WPS			=	0x02000000,
	WIFI_WPS_JOIN		=	0x04000000,
#else
	WIFI_WPS			= 0x01000000,
	WIFI_WPS_JOIN		= 0x02000000,
#endif
#endif

#ifdef A4_STA
	WIFI_A4_STA		=	0x04000000,
#endif



};

enum frag_chk_state {
	NO_FRAG		= 0x0,
	UNDER_FRAG	= 0x1,
	CHECK_FRAG	= 0x2,
};

enum led_type {
	LEDTYPE_HW_TX_RX,
	LEDTYPE_HW_LINKACT_INFRA,
	LEDTYPE_SW_LINK_TXRX,
	LEDTYPE_SW_LINKTXRX,
	LEDTYPE_SW_LINK_TXRXDATA,
	LEDTYPE_SW_LINKTXRXDATA,
	LEDTYPE_SW_ENABLE_TXRXDATA,
	LEDTYPE_SW_ENABLETXRXDATA,
	LEDTYPE_SW_ADATA_GDATA,
	LEDTYPE_SW_ENABLETXRXDATA_1,
	LEDTYPE_SW_CUSTOM1,
	LEDTYPE_SW_LED2_GPIO8_LINKTXRX,					// 11
	LEDTYPE_SW_LED2_GPIO8_ENABLETXRXDATA,			// 12
	LEDTYPE_SW_LED2_GPIO10_LINKTXRX,				// 13
	LEDTYPE_SW_RESERVED,							// 14, redirect to 52
	LEDTYPE_SW_LED2_GPIO8_LINKTXRXDATA,				// 15
	LEDTYPE_SW_LED2_GPIO8_ASOCTXRXDATA,  			// 16, mark_led
	LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA,			// 17 LED Control = LED_TYPE 7
	// Latest 92D customized LED types start from 50
	LEDTYPE_SW_LED2_GPIO10_ENABLETXRXDATA_92D = 50,	// 50 for 92D, LED Control = 92C LED_TYPE 7
	LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D = 51,		// 51 for 92D, LED Control = 92C LED_TYPE 13
	LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D = 52,		// 52 for 92D, LED Control = 92C LED_TYPE 13
	LEDTYPE_SW_MAX,
};

enum Synchronization_Sta_State {
	STATE_Sta_Min				= 0,
	STATE_Sta_No_Bss			= 1,
	STATE_Sta_Bss				= 2,
	STATE_Sta_Ibss_Active		= 3,
	STATE_Sta_Ibss_Idle			= 4,
	STATE_Sta_Auth_Success		= 5,
	STATE_Sta_Roaming_Scan		= 6,
};

// Realtek proprietary IE
enum Realtek_capability_IE_bitmap {
	RTK_CAP_IE_TURBO_AGGR		= 0x01,
	RTK_CAP_IE_USE_LONG_SLOT	= 0x02,
	RTK_CAP_IE_USE_AMPDU		= 0x04,
#ifdef RTK_WOW
	RTK_CAP_IE_USE_WOW			= 0x08,
#endif
	RTK_CAP_IE_SOFTAP			= 0x10,
	RTK_CAP_IE_WLAN_8192SE		= 0x20,
	RTK_CAP_IE_WLAN_88C92C		= 0x40,
	RTK_CAP_IE_AP_CLIENT		= 0x80,
	RTK_CAP_IE_VIDEO_ENH		= 0x01,
	RTK_CAP_IE_8812_BCUT		= 0x02,
	RTK_CAP_IE_8812_CCUT		= 0x04,
};

enum CW_STATE {
	CW_STATE_NORMAL				= 0x00000000,
	CW_STATE_AGGRESSIVE			= 0x00010000,
	CW_STATE_DIFSEXT			= 0x00020000,
	CW_STATE_AUTO_TRUBO			= 0x01000000,
};

enum {TURBO_AUTO = 0, TURBO_ON = 1, TURBO_OFF = 2};

enum NETWORK_TYPE {
	WIRELESS_11B = 1,
	WIRELESS_11G = 2,
	WIRELESS_11A = 4,
	WIRELESS_11N = 8,
	WIRELESS_11AC = 64
};

enum FREQUENCY_BAND {
	BAND_2G,
	BAND_5G
};

enum _HT_CHANNEL_WIDTH {
	HT_CHANNEL_WIDTH_20		= 0,
	HT_CHANNEL_WIDTH_20_40	= 1,
	HT_CHANNEL_WIDTH_80		= 2,
	HT_CHANNEL_WIDTH_160	= 3,
	HT_CHANNEL_WIDTH_10		= 4,
	HT_CHANNEL_WIDTH_5		= 5
};


#ifdef RTK_AC_SUPPORT
enum _HT_CHANNEL_WIDTH_AC {
	HT_CHANNEL_WIDTH_AC_20	= 0,
	HT_CHANNEL_WIDTH_AC_40	= 1,
	HT_CHANNEL_WIDTH_AC_80 	= 2,
	HT_CHANNEL_WIDTH_AC_160	= 3,
	HT_CHANNEL_WIDTH_AC_10	= 4,
	HT_CHANNEL_WIDTH_AC_5 	= 5
};

enum _SUB_CHANNEL_INDEX_80M {
	_20_B_40_A = 1,
	_20_A_40_B = 2,
	_20_A_40_A = 3,
	_20_B_40_B = 4,
	_40_A	= 9,
	_40_B	= 10
};

enum _AC_SIGMA_MODE {
	AC_SIGMA_NONE = 0, //normal mode
	AC_SIGMA_APUT = 1, //running ac logo sigma test, AP DUT mode
	AC_SIGMA_APTB = 2  //running ac logo sigma test, AP Testbed mode
};

//t_stamp[1], b5-b7 = channel width information, mapping to _HT_CHANNEL_WIDTH_AC
#define BSS_BW_SHIFT	5
#define BSS_BW_MASK		0x7
#endif


enum SECONDARY_CHANNEL_OFFSET {
	HT_2NDCH_OFFSET_DONTCARE = 0,
	HT_2NDCH_OFFSET_BELOW    = 1,	// secondary channel is below primary channel, ex. primary:5 2nd:1
	HT_2NDCH_OFFSET_ABOVE    = 2	// secondary channel is above primary channel, ex. primary:5 2nd:9
};

enum AGGREGATION_METHOD {
	AGGRE_MTHD_NONE = 0,
	AGGRE_MTHD_MPDU = 1,
	AGGRE_MTHD_MSDU = 2,
	AGGRE_MTHD_MPDU_AMSDU = 3
};

enum _HT_CURRENT_TX_INFO_ {
	TX_USE_40M_MODE		= BIT(0),
	TX_USE_SHORT_GI		= BIT(1),
	TX_USE_80M_MODE     = BIT(2),
	TX_USE_160M_MODE    = BIT(3)
};

/*
enum _ADD_RATID_UPDATE_CONTENT_ {
	RATID_NONE_UPDATE = 0,
	RATID_GENERAL_UPDATE = 1,
	RATID_INFO_UPDATE = 2
};
*/

enum _DC_TH_CURRENT_STATE_ {
	DC_TH_USE_NONE	= 0,
	DC_TH_USE_UPPER	= 1,
	DC_TH_USE_LOWER	= 2
};

enum _H2C_CMD_ID_ {
	_AP_OFFLOAD_CMD_ = 0 ,  /*0*/
	_SETPWRMODE_CMD_,
	_JOINBSSRPT_CMD_,
	_RSVDPAGE_CMD_,
	_SET_RSSI_4_CMD_,
	H2C_CMD_RSSI		= 5,
	H2C_CMD_MACID		= 6,
	H2C_CMD_PS		= 7,
	_P2P_PS_OFFLOAD_CMD_	= 8,
	SELECTIVE_SUSPEND_ROF = 9,
	AP_OFFLOAD_RFCTRL = 10,
	AP_REQ_RPT = 11,
	BT_COEX_DUTY_CYCLE = 12,
	H2C_CMD_INFO_PKT = 13,
	H2C_CMD_SMCC = 14,
	H2C_PS_SOFTAP = 38,
	H2C_CMD_AP_WPS_CTRL = 64
};

enum _ANTENNA_ {
	ANTENNA_A		= 0x1,
	ANTENNA_B		= 0x2,
	ANTENNA_C		= 0x4,
	ANTENNA_D		= 0x8,
	ANTENNA_AC		= 0x5,
	ANTENNA_AB		= 0x3,
	ANTENNA_BC		= 0x6,
	ANTENNA_BD		= 0xA,
	ANTENNA_CD		= 0xC,
	ANTENNA_ABC		= 0x7,
	ANTENNA_BCD		= 0xE,
	ANTENNA_ABCD	= 0xF
};

enum qos_prio { BK, BE, VI, VO, VI_AG, VO_AG };


enum RTK_CFG80211_EVENT {
	CFG80211_CONNECT_RESULT = 0,
	CFG80211_ROAMED = 1,
	CFG80211_DISCONNECTED = 2,
	CFG80211_IBSS_JOINED = 3,
	CFG80211_NEW_STA = 4,
	CFG80211_SCAN_DONE = 5,
	CFG80211_SCAN_ABORDED = 6,
	CFG80211_DEL_STA = 7,
	CFG80211_RADAR_CAC_FINISHED = 8,
	CFG80211_RADAR_DETECTED = 9,
	CFG80211_RADAR_CAC_ABORTED = 10,
};

enum ACL_MODE {
	ACL_allow = 1,
	ACL_deny = 2
};


enum _PA_TYPE {
	PA_SKYWORKS_5022 = 0,
	PA_RFMD_4501 = 1,
	PA_SKYWORKS_5023 = 2,
	PA_RTC5634 = 3,
	PA_SKYWORKS_85712_HP = 4,
	PA_INTERNAL = 0x10,
};

enum _LNA_TYPE {
	LNA_TYPE_0 = 0,
	LNA_TYPE_1 = 1,
	LNA_TYPE_2 = 2,
	LNA_TYPE_3 = 3,
	LNA_TYPE_MAX
};

#ifdef WIFI_WPAS
enum WPAS_EVENT {
	WPAS_EXIRED = 10,
	WPAS_REGISTERED = 11,
	WPAS_MIC_FAILURE = 12,
	WPAS_ASSOC_INFO = 13,
	WPAS_SCAN_DONE = 14,
#ifdef WIFI_WPAS_CLI
	WPAS_DISCON = 15,
#endif
};

#define REQIELEN		123
#define RESPIELEN		123

typedef struct _WPAS_ASSOCIATION_INFO {
	unsigned short	ReqIELen;
	char			ReqIE[REQIELEN];
	unsigned short  RespIELen;
	char            RespIE[RESPIELEN];
} WPAS_ASSOCIATION_INFO;
#endif

static const struct ParaRecord rtl_ap_EDCA[] = {
//ACM,AIFSN, ECWmin, ECWmax, TXOplimit
	{0,     7,      4,      10,     0},
	{0,     3,      4,      6,      0},
	{0,     1,      3,      4,      188},
	{0,     1,      2,      3,      102},
	{0,     1,      3,      4,      94},
	{0,     1,      2,      3,      47},
};

static const struct ParaRecord rtl_sta_EDCA[] = {
//ACM,AIFSN, ECWmin, ECWmax, TXOplimit
	{0,     7,      4,      10,     0},
	{0,     3,      4,      10,     0},
	{0,     2,      3,      4,      188},
	{0,     2,      2,      3,      102},
	{0,     2,      3,      4,      94},
	{0,     2,      2,      3,      47},
};

#if 1
//
// Indicate different AP vendor for IOT issue.
//
typedef enum _HT_IOT_PEER {
	HT_IOT_PEER_UNKNOWN 			= 0,
	HT_IOT_PEER_REALTEK 			= 1,
	HT_IOT_PEER_REALTEK_92SE 		= 2,
	HT_IOT_PEER_BROADCOM 			= 3,
	HT_IOT_PEER_RALINK 				= 4,
	HT_IOT_PEER_ATHEROS 			= 5,
	HT_IOT_PEER_CISCO 				= 6,
	HT_IOT_PEER_MERU 				= 7,
	HT_IOT_PEER_MARVELL 			= 8,
	HT_IOT_PEER_REALTEK_SOFTAP 		= 9,// peer is RealTek SOFT_AP, by Bohn, 2009.12.17
	HT_IOT_PEER_SELF_SOFTAP 		= 10, // Self is SoftAP
	HT_IOT_PEER_AIRGO 				= 11,
	HT_IOT_PEER_INTEL 				= 12,
	HT_IOT_PEER_RTK_APCLIENT 		= 13,
	HT_IOT_PEER_REALTEK_81XX 		= 14,
	HT_IOT_PEER_REALTEK_WOW 		= 15,
	HT_IOT_PEER_HTC 				= 16,
	HT_IOT_PEER_REALTEK_8812		= 17,
	HT_IOT_PEER_CMW 				= 18,
	HT_IOT_PEER_MAX 				= 19
} HT_IOT_PEER_E, *PHTIOT_PEER_E;
#endif

struct pkt_queue {
	struct sk_buff	*pSkb[NUM_TXPKT_QUEUE];
	int	head;
	int	tail;
};

#if defined(WIFI_WMM) && defined(WMM_APSD)
struct apsd_pkt_queue {
	struct sk_buff	*pSkb[NUM_APSD_TXPKT_QUEUE];
	int				head;
	int				tail;
};
#endif

#ifdef CAM_SWAP
enum TRAFFIC_LEVEL {
	STA_TRAFFIC_DONT_CARE = 0,
	STA_TRAFFIC_IDLE = 1,
	STA_TRAFFIC_LOW = 2,
	STA_TRAFFIC_MID = 3,
	STA_TRAFFIC_HIGH = 4
};

struct traffic_status {
	int level;

	int delta_tx_bytes;
	int delta_rx_bytes;

	int prev_tx_bytes;
	int prev_rx_bytes;
};
#endif

#if defined(WIFI_WMM)
struct dz_mgmt_queue {
	struct tx_insn	*ptx_insn[NUM_DZ_MGT_QUEUE];
	int				head;
	int				tail;
};

#ifdef DZ_ADDBA_RSP
// dz addba
struct dz_addba_info {
	unsigned char used;
	unsigned char dialog_token;
	unsigned char TID;
	unsigned short status_code;
	unsigned short timeout;
};
#endif
#endif

#ifdef USER_ADDIE
#define MAX_USER_IE		4
struct user_ie {
	unsigned int		used;
	unsigned int		ie_len;
	unsigned char		ie[256];
};
#endif

#ifdef RTK_WLAN_EVENT_INDICATE
enum {
	WIFI_CONNECT_SUCCESS = 11,
	WIFI_CONNECT_FAIL = 12,
	WIFI_DISCONNECT = 13,
};
#endif


#ifdef RSSI_MIN_ADV_SEL
#define NR_RSSI_MIN_DATA	10	// must > 1

struct rssi_search_entry {
	unsigned char rssi;
	unsigned char tx_rate;
	unsigned short throughput;
};
#endif

#ifdef WIFI_SIMPLE_CONFIG
enum WSC_ISSUE_DISCON_STATE {
	WSC_DISCON_STATE_RECV_EAP_FAIL = 0,
	WSC_DISCON_STATE_IGNORE = 1,
};

struct wsc_disconn_entry {
	struct list_head list;
	unsigned char addr[MACADDRLEN];
	int state;
	int expire_to;
};
#endif

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack(1)
#endif




/**
 *	@brief MAC Frame format - wlan_hdr (wireless LAN header)
 *
 *	Dcscription: 802.11 MAC Frame (header). See textbook P.46
 *	p.s : memory aligment by BYTE,
 *	      __PACK : not need 4 bytes aligment
 */
__PACK struct wlan_hdr {
	unsigned short	fmctrl;
	unsigned short	duration;
	unsigned char	addr1[MACADDRLEN];
	unsigned char	addr2[MACADDRLEN];
	unsigned char	addr3[MACADDRLEN];
	unsigned short	sequence;
	unsigned char	addr4[MACADDRLEN];
	unsigned short	qosctrl;
	unsigned char	iv[8];
} __WLAN_ATTRIB_PACK__;
/*cfg p2p cfg p2p*/
struct ieee80211_mgmt_hrd {
	u16 frame_control;
	u16 duration;
	u8 da[6];
	u8 sa[6];
	u8 bssid[6];
	u16 seq_ctrl;
} __attribute__ ((packed));
/*cfg p2p cfg p2p*/
#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack()
#endif

struct wlan_hdrnode {
	struct list_head	list;
#ifdef TX_EARLY_MODE
	unsigned char		em_info[8];		// early mode info
#endif
	struct wlan_hdr		hdr;
};

struct wlan_hdr_poll {
	struct wlan_hdrnode hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack(1)
#endif

__PACK struct wlanllc_hdr {
	struct wlan_hdr		wlanhdr;
	struct llc_snap		llcsnaphdr;
} __WLAN_ATTRIB_PACK__;

#if defined(GREEN_HILL) || defined(PACK_STRUCTURE) || defined(__ECOS)
#pragma pack()
#endif

struct wlanllc_node {
	struct list_head	list;
#ifdef TX_EARLY_MODE
	unsigned char		em_info[8];		// early mode info
#endif
	struct wlanllc_hdr	hdr;

	unsigned char amsdu_header[14];

};

struct wlanllc_hdr_poll {
	struct wlanllc_node	hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

struct wlanbuf_node {
	struct list_head	list;
	unsigned int		buf[PRE_ALLOCATED_BUFSIZE]; // 4 bytes alignment!
};

struct wlanbuf_poll {
	struct wlanbuf_node	hdrnode[PRE_ALLOCATED_MMPDU];
	int					count;
};

struct wlanicv_node {
	struct list_head	list;
	unsigned int		icv[2];
};

struct wlanicv_poll {
	struct wlanicv_node	hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

struct wlanmic_node {
	struct list_head	list;
	unsigned int		mic[2];
};

struct wlanmic_poll {
	struct wlanmic_node	hdrnode[PRE_ALLOCATED_HDR];
	int					count;
};

struct wlan_acl_node {
	struct list_head	list;
	unsigned char		addr[MACADDRLEN];
	unsigned char		mode;
};

struct wlan_acl_poll {
	struct wlan_acl_node aclnode[NUM_ACL];
};

struct tx_insn	{
	unsigned int		q_num;
	void				*pframe;
	unsigned char		*phdr;			//in case of mgt frame, phdr is wlan_hdr,
	//in case of data, phdr = wlan + llc
	unsigned int		hdr_len;
	unsigned int		fr_type;
	unsigned int		fr_len;
	unsigned int		frg_num;
	unsigned int		need_ack;
	unsigned int		frag_thrshld;
	unsigned int		rts_thrshld;
	unsigned int		privacy;
	unsigned int		iv;
	unsigned int		icv;
	unsigned int		mic;
	unsigned char		llc;
	unsigned char		tx_rate;
	unsigned char		lowest_tx_rate;
	unsigned char		fixed_rate;
	unsigned char		retry;
	unsigned char		aggre_en;
//	unsigned char		tpt_pkt;
	unsigned char		one_txdesc;
#ifdef CONFIG_IEEE80211W
	unsigned char		isPMF;
#endif
	struct stat_info	*pstat;


#ifdef SUPPORT_TX_MCAST2UNI
	unsigned char		isMC2UC;
#endif
#if (BEAMFORMING_SUPPORT == 1)
	unsigned char		ndpa;
	unsigned char		SND_pkt_sel; //eric-8822
	unsigned char		is_GID;
#endif

};





struct reorder_ctrl_entry {
	struct sk_buff		*packet_q[RC_ENTRY_NUM];
#ifdef SUPPORT_RX_AMSDU_AMPDU
	struct rx_frinfo 	*frinfo_q[RC_ENTRY_NUM];
#else
	struct rx_frinfo 	*frinfo_q[0];
#endif
	unsigned char		start_rcv;
	short				rc_timer_id;
	unsigned short		win_start;
	unsigned short		last_seq;
};

#ifdef SUPPORT_TX_MCAST2UNI
struct ip_mcast_info {
	int					used;
	unsigned char		mcmac[MACADDRLEN];
};
#endif

#ifdef TX_SHORTCUT
struct tx_sc_entry {
	struct tx_insn		txcfg;
	struct wlanllc_hdr 	wlanhdr;
//    PVOID   phw_desc1;
// 48 is SIZE_TXDESC_88XX
	u1Byte  hal_hw_desc[48];
	struct tx_desc		hwdesc1;
	struct tx_desc		hwdesc2;
	struct tx_desc_info	swdesc1;
	struct tx_desc_info	swdesc2;
#ifdef TX_SCATTER
	struct tx_desc		hwdesc3;
	struct tx_desc_info	swdesc3;
	int	has_desc3;
#endif
	int					sc_keyid;
	struct wlan_ethhdr_t	ethhdr;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
	unsigned char			isamsdu;
	struct wlan_amsdullcsnaphdr_t amsdullcsnaphdr;
#endif
	unsigned char		pktpri;
	unsigned short		frlen;
};

#endif // TX_SHORTCUT

#ifdef RX_SHORTCUT
struct rx_sc_entry {
	int rx_payload_offset;
	int rx_trim_pad;
	struct wlan_ethhdr_t rx_ethhdr;
	struct wlan_hdr rx_wlanhdr;
};
#endif

#if defined(SW_TX_QUEUE) || defined(RTK_ATM)
#define CHECK_DEC_AGGN		0
#define CHECK_INC_AGGN		1
#define MAX_BACKOFF_CNT		8

struct sw_tx_q {
    int						q_aggnum[8];
    int						q_TOCount[8];
    unsigned char 			q_used[8];
    struct sk_buff_head     swq_queue[8];
    unsigned short          swq_timer_id[8];
    unsigned char           swq_timer_status[8]; /* 0: success, 1: need to re-set*/
    unsigned char           swq_empty[8];      //0:empty; 1:not empty

    /* below is for old swq*/
    unsigned char			q_aggnumIncSlow[8];

    /* below is for new swq*/
    unsigned char           swq_en[8];
    unsigned long           swq_keeptime[8];
    unsigned char           swq_timeout_change[8];
    unsigned char           swq_prev_timeout[8];
};
#endif

#ifdef ERR_ACCESS_CNTR
struct err_access_list {
	unsigned int		used;
	unsigned char		mac[MACADDRLEN];
	unsigned int		num;
};
#endif

#ifdef CONFIG_IEEE80211V
//set bss transition client
struct target_transition_list
{
    unsigned char addr[MACADDRLEN];
};
// set neighbor report parameter
struct bss_transition_para
{
    unsigned char addr[MACADDRLEN];
    unsigned char chan_until;
    unsigned char FomUser;
};

struct dot11v_bss_candidate_subelement {
    unsigned char subelement_id;
    unsigned char len;
    unsigned char preference;
};

struct dot11v_req_mode_field {
    unsigned char preferred_candidate_list_included;
    unsigned char abridged;
    unsigned char disassoc_imminent;
    unsigned char bss_termniation;
    unsigned char ess_diassoc_imminent;
    unsigned char reseved[3];
};
#endif


#ifdef CONFIG_IEEE80211V_CLI
struct dot11v_stat_info
{
	unsigned char dialog_token;
	unsigned char reply;
	unsigned char num_neighbor_report;
	unsigned char req_mode;
	unsigned short dissoc_timer;
	unsigned char bss_termination_duration[12];
	unsigned char target_bssid[MACADDRLEN];
	unsigned int candidate_valid_time;
	struct dot11k_neighbor_report neighbor_report[MAX_NEIGHBOR_REPORT];
};
#endif


struct stat_info {
	struct list_head	hash_list;	// always keep the has_list as the first item, to accelerat searching
	struct list_head	asoc_list;
	struct list_head	auth_list;
	struct list_head	sleep_list;
	struct list_head	defrag_list;
	struct list_head	wakeup_list;
	struct list_head	frag_list;
	struct list_head	addRAtid_list;	// to avoid add RAtid fail
	struct list_head	addrssi_list;
	struct list_head	addps_list;
	struct sk_buff_head	dz_queue;	// Queue for sleeping mode

#ifdef SW_TX_QUEUE
	struct sw_tx_q  swq;
#endif

#ifdef RTK_ATM
	struct sw_tx_q  atm_swq;

	unsigned int		atm_tx_rate;//tx rate
	unsigned int		atm_tx_rate_id[4];//tx rate idx, [0]=vht/ht/ofdm_cck, [1]=S/L GI [2]=20/40M [3]=phy rate
	unsigned int		atm_sta_time;//sta tx time for mode 0/1
	unsigned int		atm_match_sta_time;//sta tx time for mode 2

	unsigned int		atm_adj_factor;//adjust tx swq length by tx time
	unsigned int		atm_burst_num;//packet num that must sent
	unsigned int		atm_burst_size;//txrate*txtime
	unsigned int		atm_burst_unit;//packet num that must sent
	unsigned int		atm_burst_sent;//packet num that already sent

	unsigned int		atm_tx_time;//real transmission time//ms
	unsigned int		atm_tx_time_orig;//real transmission time//ms without retry
	unsigned int		atm_tx_time_static;//accumuler sta tx time
	unsigned int		atm_wait_cnt;//when burst out but must wait others
	unsigned int		atm_drop_cnt;
	unsigned int		atm_swq_full;
	unsigned int		atm_full_sent_cnt;//when dequeue but burst sent is full
	unsigned int		atm_swq_sent_cnt;//how many packet swq has sent
	unsigned int		atm_txbd_full[4];

	//retry
#ifdef TXRETRY_CNT
	unsigned int		atm_txretry_pre;
	unsigned int		atm_txpkt_pre;
	unsigned int		atm_txretry_ratio;
	unsigned int		atm_txretry_avg;	//avg retry per packet
	unsigned int		atm_txretry_1s;
	unsigned int		atm_txpkt_1s;
#endif
	unsigned int		atm_is_maxsta;
#endif//RTL_ATM


#ifdef A4_STA
	struct list_head	a4_sta_list;
#endif

	WPA_STA_INFO		*wpa_sta_info;

#ifdef CAM_SWAP
	struct traffic_status	traffic;
#endif

#ifdef TLN_STATS
	unsigned int		enterpise_wpa_info;
#endif

	u_int8_t wpa_ie[256];
#ifndef HAPD_DRV_PSK_WPS
	u_int8_t wps_ie[256];
#endif
#if defined(WIFI_WMM) && defined(WMM_APSD)
	unsigned char		apsd_bitmap;	// bit 0: VO, bit 1: VI, bit 2: BK, bit 3: BE
	unsigned int		apsd_pkt_buffering;
#endif

#if defined(WIFI_WMM) && defined(WMM_APSD)
	struct apsd_pkt_queue	*VO_dz_queue;
	struct apsd_pkt_queue	*VI_dz_queue;
	struct apsd_pkt_queue	*BE_dz_queue;
	struct apsd_pkt_queue	*BK_dz_queue;
#endif

#if defined(WIFI_WMM)
	struct dz_mgmt_queue	*MGT_dz_queue;
#ifdef DZ_ADDBA_RSP
	struct dz_addba_info	dz_addba;
#endif
#endif


	struct reorder_ctrl_entry	rc_entry[8];

#ifdef STA_RATE_STATISTIC
	unsigned int		txrate_stat[STA_RATE_NUM];
	unsigned int		rxrate_stat[STA_RATE_NUM];
#endif

	unsigned char		tx_bw_bak;	//backup station's bwndwidth ever used, keep tracking unless freed
	/****************************************************************
	 * from here on, data will be clear in init_stainfo() except "aid" and "hwaddr"  *
	 ****************************************************************/
	unsigned int		auth_seq;
	unsigned char		chg_txt[128];

	unsigned int		frag_to;
	unsigned int		frag_count;
	unsigned int		sleep_to;
	unsigned short		tpcache[8][TUPLE_WINDOW];
	unsigned short		tpcache_mgt;	// mgt cache number

#ifdef CLIENT_MODE
	unsigned short      tpcache_mcast;  // for client mode broadcast or multicast used
#endif

	//AMSDU DEBUG
	unsigned int		tx_amsdu_timer_ovf;
#ifdef _AMPSDU_AMSDU_DEBUG_
	unsigned int		tx_amsdu_to;
	unsigned int		tx_amsdu_buf_overflow;
	unsigned int		tx_amsdu_1pkt;
	unsigned int		tx_amsdu_2pkt;
	unsigned int		tx_amsdu_3pkt;
	unsigned int		tx_amsdu_4pkt;
	unsigned int		tx_amsdu_5pkt;
	unsigned int		tx_amsdu_gt5pkt;

	unsigned int		rx_amsdu_err;
	unsigned int		rx_amsdu_1pkt;
	unsigned int		rx_amsdu_2pkt;
	unsigned int		rx_amsdu_3pkt;
	unsigned int		rx_amsdu_4pkt;
	unsigned int		rx_amsdu_5pkt;
	unsigned int		rx_amsdu_gt5pkt;
#endif

	//RC over window size
	unsigned int		rx_rc4_count;
#ifdef _DEBUG_RTL8192CD_
	unsigned int		rx_rc_drop1;
	unsigned int		rx_rc_drop3;
	unsigned int		rx_rc_drop4;
	unsigned int		rx_rc_reorder3;
	unsigned int		rx_rc_reorder4;
	unsigned int		rx_rc_passup2;
	unsigned int		rx_rc_passup3;
	unsigned int		rx_rc_passup4;
	unsigned int		rx_rc_passupi;
#endif

#if defined(CLIENT_MODE) || defined(WDS)
	unsigned int		beacon_num;
#endif

#ifdef TX_SHORTCUT
    struct tx_sc_entry	tx_sc_ent[TX_SC_ENTRY_NUM];
    int					tx_sc_replace_idx;
    int					tx_sc_idx_cache;
    int 				protection;
    unsigned int		ht_protection;
    unsigned int		tx_sc_pkts_lv1;
    unsigned int		tx_sc_pkts_lv2;
    unsigned int		tx_sc_pkts_slow;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
    unsigned int		tx_sc_amsdu_pkts_lv1;
    unsigned int		tx_sc_amsdu_pkts_lv2;
    unsigned int		tx_sc_amsdu_pkts_slow;
#endif
    #ifdef WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
    unsigned int        tx_sc_hw_idx;
    #endif
#endif

#ifdef RX_SHORTCUT
	struct rx_sc_entry	rx_sc_ent[RX_SC_ENTRY_NUM];
	int				rx_sc_replace_idx;
	int				rx_privacy;
	unsigned int		rx_sc_pkts;
	unsigned int		rx_sc_pkts_slow;
#endif

#ifdef SUPPORT_TX_AMSDU
	struct sk_buff_head	amsdu_tx_que[8];
	int					amsdu_timer_id[8];
	int					amsdu_size[8];
#endif



#ifdef CONFIG_IEEE80211V_CLI
   struct	dot11v_stat_info wnm;
#endif

	/******************************************************************
	 * from here to end, data will be backup when doing FAST_RECOVERY *
	 ******************************************************************/
	unsigned short		aid;
	unsigned short      assocID;

    unsigned short		remapped_aid;// support up to 64 clients
    int                 sta_in_firmware;

	unsigned char		hwaddr[MACADDRLEN];
#ifdef CONFIG_RECORD_CLIENT_HOST
	unsigned char client_host_name[256];
	unsigned char client_host_ip[4];
#endif
#ifdef WIFI_WMM
	unsigned int 		QosEnabled;
	unsigned short		AC_seq[8];
#endif
	enum wifi_state		state;
#ifdef HW_DETEC_POWER_STATE
    unsigned char		pwr_state;
#endif

	unsigned int		cnt_sleep;

	unsigned int		AuthAlgrthm;		// could be open/shared key
	unsigned int		ieee8021x_ctrlport;	// 0 for blocked, 1 for open
	unsigned int		keyid;				// this could only be valid in legacy wep
	unsigned int		keylen;
	struct Dot11KeyMappingsEntry	dot11KeyMapping;
	unsigned char		bssrateset[32];
	unsigned int		bssratelen;
	unsigned int		useShortPreamble;
	unsigned int		expire_to;
#ifdef STA_ROAMING_CHECK
	unsigned int        rtl_link_roaming_value;
#endif
	unsigned char		rssi;
	unsigned char		sq;
	unsigned char		rx_rate;
	unsigned char		rx_bw;
	unsigned char		rx_splcp;
	struct rf_misc_info	rf_info;
	unsigned short		seq_backup;
	unsigned char		rssi_backup;
	unsigned char		sq_backup;
	unsigned char		rx_rate_backup;
	unsigned char		rx_bw_backup;
	unsigned char		rx_splcp_backup;
	struct rf_misc_info rf_info_backup;
	int					cck_mimorssi_total[4];
	unsigned char		cck_rssi_num;
	unsigned char		highest_rx_rate;
	unsigned char		rssi_level;
	unsigned char		hp_level;
	unsigned char		is_realtek_sta;
#if 1
	UINT8				IOTPeer;			// Enum value.	HT_IOT_PEER_E
#else
//#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
	unsigned char		is_apclient;
	unsigned char		is_rtl8192s_sta;
	unsigned char		is_rtl8192cd_apclient;
	unsigned char		is_rtl81xxc_sta;
	unsigned char		is_broadcom_sta;
	unsigned char		is_ralink_sta;
	unsigned char		is_marvell_sta;
	unsigned char		is_intel_sta;
#ifdef RTK_WOW
	unsigned char		is_rtk_wow_sta;
#endif

#endif

	unsigned char		rx_snr[4];

	unsigned char		leave;
	unsigned char		no_rts;
	unsigned char		useCts2self; // 0:rts, 1:cts2self
	unsigned char		disable_ldpc;
	unsigned char		is_2t_mimo_sta;
	unsigned int		check_init_tx_rate;

	unsigned char		is_forced_ampdu;
	unsigned char		is_forced_rts;
	unsigned char		aggre_mthd;
	unsigned char		tx_bw;
	unsigned char		tx_bw_fw;
	unsigned char		ht_current_tx_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char		tmp_rmv_key;
	unsigned long		link_time;
	unsigned char		private_ie[32];
	unsigned int		private_ie_len;
	unsigned int		tx_ra_bitmap;

	unsigned int		tx_bytes;
	unsigned int		rx_bytes;
#ifdef RTK_ATM
	unsigned int		tx_bytes_1s;
	unsigned int		tx_bytes_pre;
#endif
#ifdef RADIUS_ACCOUNTING
	unsigned int		tx_bytes_1m;
	unsigned int		rx_bytes_1m;
#endif
	unsigned int		tx_pkts;
	unsigned int		rx_pkts;
	unsigned int		tx_fail;
	unsigned int		tx_pkts_pre;
	unsigned int		rx_pkts_pre;
	unsigned int		tx_fail_pre;
	unsigned int		current_tx_rate;
	unsigned int		tx_byte_cnt;
	unsigned int		tx_byte_cnt_odm;
	unsigned int		tx_byte_cnt_LowMAW; // TX byte conter with lower moving average weighting
	unsigned int		tx_avarage;
#ifdef RTK_ATM
	unsigned int		tx_avarage_atm;
#endif
	unsigned int		rx_byte_cnt;
	unsigned int		rx_byte_cnt_odm;
	unsigned int		rx_byte_cnt_LowMAW; //RX byte conter with lower moving average weighting
	unsigned int		rx_avarage;
	unsigned int		tx_conti_fail_cnt;	// continuous tx fail count; added by Annie, 2010-08-10.
	unsigned long		tx_last_good_time;	// up_time record for last good tx; added by Annie, 2010-08-10.
	unsigned long		rx_last_good_time;	// up_time record for rx resume time, added by Eric, 2013-01-18.
#ifdef DONT_COUNT_PROBE_PACKET
	unsigned int		tx_probe_rsp_pkts;
#endif
    unsigned char       bDrop;
	// bcm old 11n chipset iot debug, and TXOP enlarge
	unsigned int		current_tx_bytes;
	unsigned int		current_rx_bytes;

#ifdef TXRETRY_CNT
	unsigned int		cur_tx_fail;
	unsigned int		cur_tx_ok;
	unsigned int		cur_tx_retry_pkts;
	unsigned int		cur_tx_retry_cnt;
	unsigned int		total_tx_retry_pkts;
	unsigned int		total_tx_retry_cnt;
	unsigned int		txretry_ratio;
	unsigned int		prev_tx_pkts;
	unsigned int		prev_tx_retry_pkts;

#endif

#ifdef PREVENT_BROADCAST_STORM
	unsigned int		rx_pkts_bc;
#endif


#ifdef TV_MODE
   unsigned char      tv_auto_support; /*0: not support, 1:support*/
#endif


#ifdef RTK_AC_SUPPORT
	struct vht_cap_elmt 	vht_cap_buf;
	unsigned int			vht_cap_len;
	struct vht_oper_elmt	vht_oper_buf;
	unsigned int			vht_oper_len;
	unsigned char			nss;
	unsigned char			sta_mimo_mode;
#endif
#if (BEAMFORMING_SUPPORT == 1)
	unsigned int			p_aid;
	unsigned char			g_id;
	unsigned int			bf_score;
	unsigned char			error_csi; //eric-6s
	unsigned char			csi_nc; // eric-6s
	unsigned char			inTXBFEntry;
#if (MU_BEAMFORMING_SUPPORT == 1)
	struct stat_info	*muPartner;
	unsigned int		mu_tx_rate;
	unsigned int		mu_rate;
	unsigned char		mu_deq_num;
	unsigned char		isSendNDPA;
	unsigned char		isRssiApplyMU;
	unsigned char		muFlagForAMSDU;
#ifdef CONFIG_VERIWAVE_MU_CHECK
	unsigned char		isVeriwaveInValidSTA;
	unsigned char		mu_csi_data[3];
#endif
#endif
#endif

	unsigned char		force_rate;

	unsigned char	 		ratr_idx;
	unsigned char	 		ratr_idx_init;
	unsigned char			H2C_rssi_rpt;

	struct ht_cap_elmt	ht_cap_buf;
	unsigned int		ht_cap_len;
	struct ht_info_elmt	ht_ie_buf;
	unsigned int		ht_ie_len;
	unsigned char		cam_id;
	unsigned char		MIMO_ps;
	unsigned char		dialog_token;
	unsigned char		is_8k_amsdu;
	unsigned char		ADDBA_ready[8];
	unsigned char		ADDBA_req_num[8];
	unsigned char		ADDBA_sent[8];
	unsigned char		AMSDU_AMPDU_support;
	unsigned int		diffAmpduSz;
	unsigned int		amsdu_level;
	unsigned char		tmp_mic_key[8];
	unsigned short		maxAggNum;
	unsigned short		maxAggNumOrig;
	unsigned short		StaRxBuf;

#ifdef GBWC
	unsigned char		GBWC_in_group;
#endif
#ifdef INDICATE_LINK_CHANGE
	unsigned char		link_up;
#endif

	unsigned int		is_legacy_encrpt;			// 1: TKIP, 2: WEP

#ifdef  SUPPORT_TX_MCAST2UNI
	int					ipmc_num;
	struct ip_mcast_info	ipmc[MAX_IP_MC_ENTRY];
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	unsigned int		change_toggle;
#endif

	unsigned int		retry_inc;

    /* Hotspot 2.0 Release 1 */
	unsigned char	sta_ip[4];
	struct in6_addr sta_v6ip[10];
	int v6ipCount;
#ifdef CONFIG_IEEE80211W
	unsigned char 			isPMF;
	unsigned short 			SA_TID[SA_QUERY_MAX_NUM+1];     // Security Association Transaction ID
	struct timer_list		SA_timer;	//SA_timer: Security Association timer
	unsigned long			sa_query_start;
	unsigned long			sa_query_end;
	int						sa_query_timed_out;
	int						sa_query_count;
#endif
#ifdef CONFIG_IEEE80211V
	int 				bssTransStatusCode;
	unsigned char 		bssTransSupport;
	unsigned char 		bssTransRejectionCount;
	unsigned char 		bssTransExpiredTime;
	unsigned char		bssTransTriggered;
	unsigned char 		rcvNeighborReport;
#endif

#ifdef TX_EARLY_MODE
	int empkt_num;	// how many pending packet in next
	int emextra_len;	// extra 802.11 length for each 802.3 pkt
	int empkt_len[10];	// packet length for next pending packet
#endif
	unsigned long def_expired_time;
	unsigned long def_expired_throughput;
	unsigned char txpause_flag;
	unsigned char txpdrop_flag;
	unsigned long txpause_time;
	unsigned int dropPktCurr;
	unsigned int dropPktTotal;
	unsigned int drop_expire;
	UINT8		WirelessMode;		//
	u1Byte      rf_mimo_mode;

	// Driver Write
//	BOOLEAN		bValid;				// record the sta status link or not?
//	UINT8		IOTPeer;			// Enum value.	HT_IOT_PEER_E

	// ODM Write
	//1 PHY_STATUS_INFO
	UINT8		RSSI_Path[4];		//
	UINT8		RSSI_Ave;
	UINT8		RXEVM[4];
	UINT8		RXSNR[4];
	RSSI_STA	rssi_stat;
	unsigned int			low_tp_disable_ampdu;
	unsigned long	uk_timeout;
	unsigned int tx_time_total;
	unsigned int tx_byte_last;
	unsigned int rx_time_total;
	unsigned int rx_byte_last;

#ifdef THERMAL_CONTROL
	unsigned char power;
	unsigned int  tx_tp_base;  // kbps, throughput of the moment when Tx duty cycle mechansim kicks off
	unsigned int  tx_tp_limit;  // kbps, the upper-bound throughput for limitation
	unsigned char got_limit_tp; // 0:hasn't got limit throughput base, 1: got
#ifdef RTK_STA_BWC
	unsigned int		sta_bwctx_cnt;	 //tx Byte count
	unsigned int		sta_bwcthrd_tx;	 //bw control value, kbps
	unsigned int		sta_bwcdrop_cnt;
#endif
#endif

#ifdef CH_LOAD_CAL
	unsigned int total_tx_time;
	unsigned int prev_tx_byte;
	unsigned int total_rx_time;
	unsigned int prev_rx_byte;
#endif
	unsigned char bw_cnt[4];	//Reserved:80M:40M:20M
	unsigned char shrink_ac_bw;	//bit3: 80M bit2: 40M, bit1: 20M, bit0:rollback
	unsigned char shrink_ac_bw_bak;	//backup of shrink_ac_bw;
};


/*
 *	Driver open, alloc whole amount of aid_obj, avoid memory fragmentation
 *	If STA Association , The aid_obj will chain to stat_info.
 */
struct aid_obj {
	struct stat_info	station;
	unsigned int		used;	// used == TRUE => has been allocated, used == FALSE => can be allocated
	struct rtl8192cd_priv *priv;
};

/* Note: always calculate the WLAN average throughput, if the throughput is larger than TP_HIGH_WATER_MARK,
             gCpuCanSuspend will be FALSE. If the throughput is smaller than TP_LOW_WATER_MARK,
             gCpuCanSuspend will be TRUE.
             However, you can undefine the CONFIG_RTL8190_THROUGHPUT. The gCpuCanSuspend will always be
             TRUE in this case.
 */
#define CONFIG_RTL8190_THROUGHPUT

struct extra_stats {
	unsigned long		tx_retrys;
	unsigned long		tx_drops;
#ifdef SW_TX_QUEUE
	unsigned long       swq_enque_pkt;
	unsigned long       swq_xmit_out_pkt;
    unsigned long       swq_drop_pkt;
#endif
	unsigned long		rx_retrys;
	unsigned long		rx_decache;
	unsigned long		rx_data_drops;
	unsigned long		rx_mc_pn_drops;
	unsigned long		rx_cnt_psk_to;
	unsigned long		beacon_ok;
	unsigned long		beacon_er;
	unsigned long		beacon_dma_err;
	unsigned long 		beaconQ_sts;
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8190_THROUGHPUT) || defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC)
	unsigned long		tx_peak;
	unsigned long		rx_peak;
#endif
#if 0
	unsigned long		tx_dma_err;
	unsigned long		rx_dma_err;
	unsigned long		tx_dma_status;
	unsigned long		rx_dma_status;
#endif
#ifdef RX_CRC_EXPTIMER
    unsigned long       rx_crc_exptimer;
    unsigned long       rx_packets_exptimer;
	unsigned long		rx_crc_by_rate[256];
	unsigned long		rx_packets_by_rate[256];
#endif
#ifdef CH_LOAD_CAL
	unsigned long		ch_utilization;
	unsigned long		tx_time;
	unsigned long		rx_time;
#endif
	unsigned long		tx_byte_cnt;
	unsigned long		tx_avarage;
	unsigned long		rx_byte_cnt;
	unsigned long		rx_avarage;
	unsigned long		rx_fifoO;
	unsigned long		rx_rdu;
	unsigned long		rx_reuse;
	unsigned long		freeskb_err;
	unsigned long		reused_skb;
#if defined(CONFIG_RTL_NOISE_CONTROL) || defined(CONFIG_RTL_NOISE_CONTROL_92C)
	unsigned int		tp_average_pre;
#endif

#if (defined(__ECOS) && defined(_DEBUG_RTL8192CD_)) || defined(SHORTCUT_STATISTIC)
	unsigned long		tx_cnt_nosc;
	unsigned long		tx_cnt_sc1;
	unsigned long		tx_cnt_sc2;
	unsigned long		rx_cnt_nosc;
	unsigned long		rx_cnt_sc;
	unsigned long		br_cnt_nosc;
	unsigned long		br_cnt_sc;
#endif
#ifdef TRX_DATA_LOG
	unsigned long		tx_data_packets;
	unsigned long		rx_data_packets;
#endif
#if defined(CONFIG_RTL8672) || defined(CONFIG_WLAN_STATS_EXTENTION)
	unsigned long		tx_ucast_pkts_cnt;
	unsigned long		rx_ucast_pkts_cnt;
	unsigned long		tx_mcast_pkts_cnt;
	unsigned long		rx_mcast_pkts_cnt;
	unsigned long		tx_bcast_pkts_cnt;
	unsigned long		rx_bcast_pkts_cnt;
	unsigned long		unknown_pro_pkts_cnt; // unknown protocal packets
	unsigned long 		total_mic_fail;
	unsigned long 		total_psk_fail;
#endif
	unsigned long 		null_interrupt_cnt1;
	unsigned long 		null_interrupt_cnt2;
};

enum {
	fourWaystep1 = 1,
	fourWaystep2,
	fourWaystep3,
	fourWaystep4
};

#ifdef TLN_STATS
struct tln_wifi_stats {
	unsigned long		connected_sta;
	unsigned long		max_sta;
	unsigned long		max_sta_timestamp;
	unsigned long		rejected_sta;
};


struct tln_ext_wifi_stats {
	unsigned long		rson_UNSPECIFIED_1;
	unsigned long		rson_AUTH_INVALID_2;
	unsigned long		rson_DEAUTH_STA_LEAVING_3;
	unsigned long		rson_INACTIVITY_4;
	unsigned long		rson_RESOURCE_INSUFFICIENT_5;
	unsigned long		rson_UNAUTH_CLS2FRAME_6;
	unsigned long		rson_UNAUTH_CLS3FRAME_7;
	unsigned long		rson_DISASSOC_STA_LEAVING_8;
	unsigned long		rson_ASSOC_BEFORE_AUTH_9;
	unsigned long		rson_INVALID_IE_13;
	unsigned long		rson_MIC_FAILURE_14;
	unsigned long		rson_4WAY_TIMEOUT_15;
	unsigned long		rson_GROUP_KEY_TIMEOUT_16;
	unsigned long		rson_DIFF_IE_17;
	unsigned long		rson_MCAST_CIPHER_INVALID_18;
	unsigned long		rson_UCAST_CIPHER_INVALID_19;
	unsigned long		rson_AKMP_INVALID_20;
	unsigned long		rson_UNSUPPORT_RSNIE_VER_21;
	unsigned long		rson_RSNIE_CAP_INVALID_22;
	unsigned long		rson_802_1X_AUTH_FAIL_23;
	unsigned long		rson_OUT_OF_SCOPE;

	unsigned long		status_FAILURE_1;
	unsigned long		status_CAP_FAIL_10;
	unsigned long		status_NO_ASSOC_11;
	unsigned long		status_OTHER_12;
	unsigned long		status_NOT_SUPPORT_ALG_13;
	unsigned long		status_OUT_OF_AUTH_SEQ_14;
	unsigned long		status_CHALLENGE_FAIL_15;
	unsigned long		status_AUTH_TIMEOUT_16;
	unsigned long		status_RESOURCE_INSUFFICIENT_17;
	unsigned long		status_RATE_FAIL_18;
	unsigned long		status_OUT_OF_SCOPE;
};
#endif


#ifdef WIFI_SIMPLE_CONFIG
#ifndef INCLUDE_WPS
/* this struct no sync with wsc daemon ;
   add ssid for sorting by ssid intent ; wps_ie_info2 is sync with wsc daemon;take care*/
struct wps_ie_info {
	unsigned char rssi;
	unsigned int wps_ie_len;
	unsigned char data[MAX_WSC_IE_LEN];
	char ssid[33];
	unsigned char chan;
};

/* this struct sync with wsc daemon ; when report wsc ie to daemon use this
struct wps_ie_info2{
	unsigned char rssi;
	unsigned char data[MAX_WSC_IE_LEN];
};
*/

#endif

struct wpa_ie_info {
	unsigned char 	rssi;
	unsigned int	wpa_ie_len;
	unsigned char 	data[MAX_WPA_IE_LEN];
    char ssid[33];

};
struct rsn_ie_info {
	unsigned char 	rssi;
	unsigned int	rsn_ie_len	;
	unsigned char 	data[MAX_RSN_IE_LEN];
    char ssid[33];

};
struct p2p_ie_info {
	unsigned char 	rssi;
	unsigned int	p2p_ie_len	;
	unsigned char 	data[MAX_RSN_IE_LEN];
    char ssid[33];

};

#endif

struct ss_res {
	unsigned int		ss_channel;
	unsigned int		count;
	struct bss_desc		bss[MAX_BSS_NUM];
	unsigned int		count_backup;
	struct bss_desc		bss_backup[MAX_BSS_NUM];
	unsigned int		count_target;
	struct bss_desc		bss_target[MAX_BSS_NUM];
#ifdef WIFI_SIMPLE_CONFIG
	struct wps_ie_info	wscie[MAX_BSS_NUM];
	//struct wps_ie_info	*wscie;
	/* this struct sync with wsc daemon ; when report wsc ie to daemon use this*/
	//struct wps_ie_info2	*wscie_backup;
#endif
	struct wpa_ie_info	wpa_ie[MAX_BSS_NUM];
	struct rsn_ie_info	rsn_ie[MAX_BSS_NUM];
    struct p2p_ie_info	rtk_p2p_ie[MAX_BSS_NUM];
	//struct wpa_ie_info	wpa_ie_backup[MAX_BSS_NUM];
    //struct rsn_ie_info	rsn_ie_backup[MAX_BSS_NUM];
#ifdef CONFIG_RTL_NEW_AUTOCH
	unsigned int		to_scan_40M;
	unsigned int		start_ch_40M;
#endif
	unsigned int		hidden_ap_found;
	unsigned int		target_ap_found;

	unsigned char	defered_ss;
	struct propietaryIE *pptyIE;
};

#if defined(CONFIG_RTL_NEW_AUTOCH) && defined(SS_CH_LOAD_PROC)

struct ss_report {
	unsigned char	channel;
	unsigned char	is40M;
	unsigned char	rssi;
	unsigned short	fa_count;
	unsigned short	cca_count;
	unsigned int	ch_load;
};

#endif


//struct stat_info_cache {
//	struct stat_info	*pstat;
//	unsigned char		hwaddr[6];
//};

struct mc2u_flood_mac_entry {
	unsigned char macAddr[MACADDRLEN];
};

#if 0//def RTK_ATM
struct atm_stat_info{
	unsigned char		hwaddr[MACADDRLEN];
	unsigned char		ipaddr[4];
	int			atm_time;
};
#endif//RTK_ATM

struct rf_finetune_var {
	unsigned char		ofdm_1ss_oneAnt;// for 2T2R
	unsigned char		pathB_1T; // for 1T2R, 1T1R
	unsigned char		rssi_dump;
	unsigned char		totaltp;
	unsigned char		rxfifoO;
	unsigned char		raGoDownUpper;
	unsigned char		raGoDown20MLower;
	unsigned char		raGoDown40MLower;
	unsigned char		raGoUpUpper;
	unsigned char		raGoUp20MLower;
	unsigned char		raGoUp40MLower;
	unsigned char		dig_enable;
	unsigned char		adaptivity_enable;
	unsigned char		digGoLowerLevel;
	unsigned char		digGoUpperLevel;
	unsigned char		dcThUpper;
	unsigned char		dcThLower;
	unsigned char		rssiTx20MUpper;
	unsigned char		rssiTx20MLower;
	unsigned char		rssi_expire_to;
	unsigned char       dig_cov_enable;
	unsigned char		dig_upcheck_enable;
	unsigned char		dig_upcheck_initial_value;
	unsigned char		dig_level0_ratio_reciprocal;
	unsigned char		dig_level1_ratio_reciprocal;
	BOOLEAN				fix_expire_to_zero;
//#ifdef INTERFERENCE_CONTROL
	unsigned char		nbi_filter_enable;
//#endif
	unsigned char		rts_init_rate;
	unsigned char		auto_rts_rate;

	unsigned char		cck_pwr_max;
	unsigned char		cck_tx_pathB;
	unsigned char       min_pwr_idex;
	unsigned char       bcn_pwr_max;
	unsigned char       bcn_pwr_idex;
	// dynamic Tx power control
	unsigned char		tx_pwr_ctrl;

	// 11n ap AES debug
	unsigned char		aes_check_th;

	// Tx power tracking
	unsigned int		tpt_period;

	unsigned int		dpk_period;

#ifdef CAM_SWAP
	unsigned int		cam_rotation;

	unsigned int		thrd_low;
	unsigned int		thrd_mid;
	unsigned int		thrd_high;

	unsigned int		camforce;
#endif

	// TXOP enlarge
	unsigned char		txop_enlarge_upper;
	unsigned char		txop_enlarge_lower;
#ifdef LOW_TP_TXOP
	unsigned char		low_tp_txop;
	unsigned int		low_tp_txop_thd_n;
	unsigned int		low_tp_txop_thd_g;
	unsigned int		low_tp_txop_thd_low;
	unsigned char		low_tp_txop_close;
	unsigned char		low_tp_txop_count;
	unsigned char		low_tp_txop_delay;
	unsigned int		cwmax_enhance_thd;
#endif

	// 2.3G support
	unsigned char		use_frq_2_3G;

	// for mp test
#ifdef MP_TEST
	unsigned char		mp_specific;
#endif
	unsigned char		drop_multicast;

	//Support IP multicast->unicast
#ifdef SUPPORT_TX_MCAST2UNI
	unsigned char		mc2u_disable;
#ifdef IGMP_FILTER_CMO
	unsigned char		igmp_deny;
#endif
	unsigned char 		mc2u_drop_unknown;
	unsigned int		mc2u_flood_ctrl;
	struct mc2u_flood_mac_entry mc2u_flood_mac[MAX_FLOODING_MAC_NUM];
	unsigned int		mc2u_flood_mac_num __attribute__ ((packed));


#endif

#ifdef	HIGH_POWER_EXT_PA
	unsigned char		hp_ofdm_pwr_max;
	unsigned char		hp_cck_pwr_max;
#endif
	unsigned char		dropUDP;
	unsigned char		lmtdesc;
	unsigned int		lmt1;
	unsigned int		lmt2;
	unsigned int		lmt3;
#ifdef RF_MIMO_SWITCH
	unsigned char		rf_mode;
#endif
#ifdef HIGH_POWER_EXT_PA
	unsigned char		use_ext_pa;
#endif
#ifdef CONFIG_8881A_2LAYER
	unsigned char		use_8881a_2layer;
#endif
#ifdef HIGH_POWER_EXT_LNA
	unsigned char		use_ext_lna;
	unsigned char		ext_lna_gain;
	unsigned int		lna_type;
#endif
	unsigned char           NDSi_support;

	//	for adaptivity
	u1Byte				IGI_target;
	s1Byte				IGI_Base;
	s1Byte 				TH_L2H_ini;
	s1Byte				TH_L2H_ini_mode2;
	s1Byte				TH_L2H_ini_backup;
	s1Byte				TH_EDCCA_HL_diff;
	s1Byte				TH_EDCCA_HL_diff_mode2;
	s1Byte				TH_EDCCA_HL_diff_backup;
	u2Byte     			NHM_cnt_0;
	u2Byte				NHM_cnt_1;
	BOOLEAN				adaptivity_flag;
	s1Byte				H2L_lb;
	s1Byte				L2H_lb;
	u1Byte				Adaptivity_IGI_upper;
	u1Byte				NHMWait;
	BOOLEAN				bFirstLink;
	BOOLEAN				bCheck;
	u1Byte				dcbackoff;
	BOOLEAN 			txbuf_merge;
	unsigned int		msta_refine;
	BOOLEAN				DynamicLinkAdaptivity;
	BOOLEAN 			Adaptivity_enable;
	BOOLEAN				bLinked;
	BOOLEAN				AcsForAdaptivity;
	BOOLEAN				isCleanEnvironment;
	//	for adaptivity

	unsigned char		manual_ampdu;
#ifdef ADD_TX_POWER_BY_CMD
	char		txPowerPlus_cck_1;
	char		txPowerPlus_cck_2;
	char		txPowerPlus_cck_5;
	char		txPowerPlus_cck_11;
	char		txPowerPlus_ofdm_6;
	char		txPowerPlus_ofdm_9;
	char		txPowerPlus_ofdm_12;
	char		txPowerPlus_ofdm_18;
	char		txPowerPlus_ofdm_24;
	char		txPowerPlus_ofdm_36;
	char		txPowerPlus_ofdm_48;
	char		txPowerPlus_ofdm_54;
	char		txPowerPlus_mcs_0;
	char		txPowerPlus_mcs_1;
	char		txPowerPlus_mcs_2;
	char		txPowerPlus_mcs_3;
	char		txPowerPlus_mcs_4;
	char		txPowerPlus_mcs_5;
	char		txPowerPlus_mcs_6;
	char		txPowerPlus_mcs_7;
	char		txPowerPlus_mcs_8;
	char		txPowerPlus_mcs_9;
	char		txPowerPlus_mcs_10;
	char		txPowerPlus_mcs_11;
	char		txPowerPlus_mcs_12;
	char		txPowerPlus_mcs_13;
	char		txPowerPlus_mcs_14;
	char		txPowerPlus_mcs_15;
#endif

	unsigned char		rootFwBeacon;		// use FW to send beacon
	unsigned char		ledBlinkingFreq;

	unsigned char		diffAmpduSz;
	unsigned char		one_path_cca;		// 0: 2-path, 1: path-A, 2: path-B
	unsigned int		dbg_dig_upper;
	unsigned int		dbg_dig_lower;
	unsigned int		dbg_dig_th1;
	unsigned int		dbg_dig_th2;
	unsigned int		dbg_dig_th3;
	unsigned int		dbg_aaa_base;
	unsigned int		dbg_aaa_step;
#ifdef CONFIG_1RCCA_RF_POWER_SAVING
	unsigned char		one_path_cca_ps;	// 0: disable, 1: enable, 2: auto
	unsigned char		one_path_cca_ps_active;
	unsigned char		one_path_cca_ps_rssi_thd;
#endif
	unsigned char		dfsdbgmode;
	unsigned char		manual_dfs_regdomain;
	unsigned char		dfs_force_TP_mode;
	unsigned char		dfsdelayiqk;
	unsigned int		dfs_next_ch;
	unsigned int		dfsdbgcnt;
	unsigned long		dfsrctime;
	unsigned char		dfs_det_off;
	unsigned char		dfs_det_reset;
	unsigned char		dfs_fa_cnt_lower;
	unsigned char		dfs_fa_cnt_mid;
	unsigned int		dfs_fa_cnt_upper;
	unsigned char		dfs_fa_cnt_inc_ratio;
	unsigned char		dfs_psd_delay;
	unsigned char		dfs_crc32_cnt_lower;
	unsigned char		dfs_fa_ratio_th;
	unsigned char		dfs_det_period;
	unsigned char		dfs_det_period_jp_w53;
	unsigned char		dfs_det_print;
	unsigned char		dfs_det_print1;
	unsigned char		dfs_det_print2;
	unsigned char		dfs_det_print3;
	unsigned char		dfs_det_print4;
	unsigned char		dfs_radar_diff_on;
	unsigned char		dfs_radar_diff_print;
	unsigned char		dfs_print_hist_report;
	unsigned char		dfs_det_print_psd;
	unsigned char		dfs_pulse_print;
	unsigned char		dfs_det_hist_len;
	unsigned char		dfs_det_sum_th;
	unsigned char		dfs_det_flag_offset;
	unsigned int		dfs_dpt_fa_th_upper;
	unsigned char		dfs_dpt_fa_th_lower;
	unsigned int        dfs_fa_hist;
	unsigned char		dfs_dpt_pulse_th_mid;
	unsigned char		dfs_dpt_pulse_th_lower;
	unsigned char		dfs_dpt_st_l2h_max;
	unsigned char		dfs_dpt_st_l2h_min;
	unsigned char		dfs_dpt_st_l2h_add;
	unsigned char		dfs_dpt_st_l2h_idle_offset;
	unsigned char		dpt_ini_gain_th;
	unsigned char		dfs_pwdb_th;
	unsigned char		dfs_pwdb_scalar_factor;
	unsigned char		dfs_psd_pw_th;
	unsigned char		psd_skip_lookup_table;
	unsigned char		dfs_psd_fir_decay;
	unsigned char		dfs_skip_iqk;
	unsigned char		dfs_scan_inband;
	unsigned char		dfs_psd_op;
	unsigned char		dfs_psd_idle_on;
	unsigned char		dfs_psd_TP_on;
	unsigned char		dfs_psd_tp_th;
	unsigned char		dfs_pc0_th_idle_w53;
	unsigned char		dfs_pc0_th_idle_w56;
	unsigned char		dfs_max_sht_pusle_cnt_th;

#ifdef ROMEPERF_SUPPORT
	unsigned char		perf_index;
#endif

#ifdef SW_TX_QUEUE
	int             	swq_en_highthd;
	int             	swq_dis_lowthd;
	int             	swq_enable;
	int             	swq_dbg;
	int					swq_aggnum;
	int					swqturboaggnum;
	int					swqmaxturbotime;
	int					timeout_thd;
	int					timeout_thd2;
	int					timeout_thd3;
	unsigned int 		tri_time1;
	unsigned int 		tri_time2;
	unsigned int 		tri_time3;
	unsigned int 		tri_time4;
	unsigned int 		tri_time5;
	unsigned int 		tri_time6;
	unsigned int 		tri_time7;
    unsigned int 		tri_time8;
    unsigned int 		udp_tri_time1;
    unsigned int 		udp_tri_time2;
    unsigned int 		udp_tri_time3;
    unsigned int 		udp_tri_time4;
	unsigned int 		udp_tri_time5;
	unsigned int 		udp_tri_time6;
	unsigned int 		udp_tri_time7;
    unsigned int 		udp_tri_time8;
#endif

#ifdef RTK_ATM
    int                 atm_en;
    int                 atm_swqf;
    int                 atm_mode;// 1:if auto 2:if manual 3:sta auto 4:sta manual
    struct atm_stainfo  atm_sta_info[NUM_STAT];

    int                 atm_quota;//burst size base
    int                 atm_aggmax;//dequeue num max
    int                 atm_aggmin;//dequeue num min
    int                 atm_enqmax;//enqueue max
    int                 atm_hista_enqbuf;//enqueue max buf
    int                 atm_rhi;//max/min hi
    int                 atm_rlo;//max/min lo
    int                 atm_sto;//short timer
    int                 atm_mto;//middle timer
    int                 atm_lto;//lo timer
    int                 atm_chk_txtime;//check tx time
    int                 atm_chk_hista;
	int					atm_chk_newrty;
	int					atm_adj_time;
	int					atm_rty_ratio;
	int					atm_adj_bto;
#endif


	unsigned int		amsdu_th;
	unsigned int		amsdu_th2;

	unsigned int		rts_rssith;


#ifdef A4_STA
	unsigned char		a4_enable;
#endif

#ifdef SW_ANT_SWITCH
	unsigned char		antSw_enable;
#endif

#ifdef HW_ANT_SWITCH
	unsigned char		antHw_enable;
	unsigned char		CurAntenna;
#endif
#if defined(HW_ANT_SWITCH) || defined(SW_ANT_SWITCH)
	unsigned char		antSw_select;
	unsigned char		ant_dump;
#endif

#ifdef RTL8192D_INT_PA
	unsigned char		use_intpa92d;
#endif
#if defined(CONFIG_RTL_NOISE_CONTROL) || defined(CONFIG_RTL_NOISE_CONTROL_92C)
	unsigned char		dnc_enable;
#endif
#if defined(WIFI_11N_2040_COEXIST_EXT)
	unsigned int		bws_Thd;
	unsigned int		bws_CleanThd;
	unsigned char		bws_enable;
	unsigned char		n_add40m,n_del40m;
#endif
	unsigned char		pwr_by_rate;
#ifdef _TRACKING_TABLE_FILE
	unsigned char		pwr_track_file;
#endif
#ifdef DPK_92D
	unsigned char		dpk_on;
#endif
	unsigned char		disable_pwr_by_rate;
#if defined(TXPWR_LMT)
	unsigned char		disable_txpwrlmt;
	unsigned char		disable_txpwrlmt2path;
#endif
#ifdef WIFI_WMM
	unsigned char		wifi_beq_iot;
#endif
	unsigned char		bcast_to_dzq;
#ifdef TLN_STATS
	unsigned int		stats_time_interval;
#endif
#ifdef TX_EARLY_MODE
	unsigned char		em_enable;
	unsigned char		em_que_num;
	unsigned int		em_swq_thd_high;
	unsigned int		em_swq_thd_low;
#endif
#ifdef CONFIG_TCP_ACK_TXAGG
	unsigned char		tcpack_agg;
	unsigned char		tcpack_merge;
#endif
#ifdef CONFIG_XMITBUF_TXAGG_ADV
	unsigned char		low_traffic_xmit_thd;
#endif
	unsigned char		low_retry_rssi_thd;
	unsigned char		low_rate_low_retry;
#ifdef RSSI_MIN_ADV_SEL
	unsigned char		rssi_min_advsel;
#endif
#ifdef CLIENT_MODE
	unsigned char		sta_mode_ps;
#endif
#ifdef CONFIG_RTL_WLAN_DOS_FILTER
	unsigned int		dos_block_time;
#endif

	unsigned int		intel_rtylmt_tp_margin;

	unsigned char       enable_macid_sleep;

	unsigned char		low_tp_no_aggr;

#if (BEAMFORMING_SUPPORT == 1)
	unsigned int		dumpcsi;
	unsigned int		csi_counter;
	unsigned int		soundingPeriod;
	unsigned char		soundingEnable;
	unsigned char		ndparate;
	unsigned char		applyVmatrix;
	unsigned char		Nsnding;
#ifdef BEAMFORMING_AUTO
	unsigned char		txbf_pwrlmt;
#endif

#endif
	unsigned char		use_cca;

	unsigned char		oper_mode_field;
	unsigned char		opmtest;

// channel switch announcement
	unsigned char		csa;

	unsigned char		sub_pwr;
// power constraint
	unsigned char		lpwrc;
	unsigned int		lgirate;
	unsigned char		no_rtscts;

#ifdef RTK_AC_SUPPORT //eric-8822 add mibs
	unsigned int		cts_width;
	unsigned char		gidforce;
	unsigned char		ndpa_mac[6];
	unsigned char		ndpa_swap;
#endif

	unsigned char		sigma_mode;

	unsigned char		cca_rts;

	unsigned char		txforce;

	unsigned char		mu_retry;

	unsigned char		debug8822;

	unsigned char		wakeforce;
	unsigned char		waketh;
	unsigned char		rts_iot_th;
	unsigned char		lock5d1;
#if (MU_BEAMFORMING_SUPPORT == 1)
	unsigned int		mu_ok[MAX_NUM_BEAMFORMEE_MU];
	unsigned int		mu_fail[MAX_NUM_BEAMFORMEE_MU];
	unsigned int		mu_BB_ok;
	unsigned int		mu_BB_fail;
	unsigned int 		mutime;
	unsigned char		mu_dump;
	unsigned char		muPairResult[MAX_NUM_BEAMFORMEE_MU];
#ifdef CONFIG_VERIWAVE_MU_CHECK
	unsigned char		isExistVeriwaveSTA;
#endif
#endif
	unsigned char 		snd;
	unsigned char 		sndlen;
#if (MU_BEAMFORMING_SUPPORT == 1)
	unsigned int		qtime;
	unsigned int		qlmt;
	unsigned int		dqnum;
	unsigned int		mutp_th_up;
	unsigned int		mutp_th_lower;
	unsigned int		murssi_th_up;
	unsigned int		murssi_th_lower;
	unsigned int		reg_bcn_hold_backup;
#endif

	unsigned char		txa;
	unsigned char		txb;
	unsigned char		no_legacy;
	unsigned int		mloop;
	unsigned int		soundint;
	unsigned char		sgiforce;
	unsigned char		aggforce;
#ifdef RX_CRC_EXPTIMER
	unsigned char		crc_enable;
	unsigned char		crc_dump;
#endif
//mark_priv
	unsigned int	rtk_uci_AcEnable;
	unsigned char	rtk_uci_PrivBandwidth[6]; //5M,10M ,80M-,80M+ .
//	unsigned char		mp_dig_enable;

#ifdef AC2G_256QAM
	unsigned char 		ac2g_enable;		//enable 11ac mode in phyband = 2g
	unsigned char 		ac2g_phy_type;		//0 = LNA, 1 = PA+LNA
	unsigned int		ac2g_thd_ldpc; 		//threshlod to enable/disable ldpc (by rssi level)
#endif


#ifdef CHECK_HANGUP
	unsigned int		check_hang;
#endif
	unsigned char		auto_cipher;
#ifdef BT_COEXIST
	unsigned char		bt_dump;
	unsigned char		btc;
#endif
	unsigned int		dynamic_max_num_stat;
#ifdef MCR_WIRELESS_EXTEND
	unsigned char		mcr_ft;
	unsigned char		disable_ldpc_thd;
#endif
	unsigned char		print_table;
	unsigned char		curr_ther;
	unsigned char 		cv_ch1;
#ifdef WIFI_SIMPLE_CONFIG
	int			prefer_2g;			// 0: disable, 1: prefer 2g band
#endif
	unsigned char		bcn_dont_ignore_edcca;	// 0: disable, 1: when edcca on, bcn will not xmit

	unsigned char		ac_del80m;
	unsigned char		ac_add80m;
	unsigned char		ac_del40m;
	unsigned char		ac_add40m;
	unsigned char		shrink_ac_bw;
	unsigned char		load_fw;
	unsigned char		dym_soml;
	unsigned char		dym_soml_thd;
#ifdef THER_TRIM
	unsigned char		ther_trim_enable;
	char				ther_trim_val;
#endif
#ifdef THERMAL_CONTROL
	unsigned char		ther_dm;   //0: turn off thermal control dynamic mechanism; 1: turn on
	unsigned char		ther_dm_period;  // period to do thermal control
	unsigned char		state; //0: initial, 1: power, 2: limit
	unsigned char		low_power; // 0: as default (depends on tx_pwr_ctrl), 1: turn on dynamic mechanism, 2: maually set power
	unsigned char		power_desc; // 0: 0dB, 1: -3dB, 2:-7dB, 3:-11dB
	unsigned char		current_power_desc;
	unsigned char		path; // 0: as default (depends on tx2path), 1: use 1T, 2: use 2T
	unsigned char		current_path; // 1: current 1T, 2: current 2T
	unsigned char		path_select; // 0: disable, 1: enable path selection
	unsigned char		chosen_path; // 1: path A, 2: path B, 3: path C
	int					del_ther;    // current ther - calibration ther
	unsigned char		ther_hi; // if del_ther >= ther_hi, enable power control mechanism
	unsigned char		ther_low; // if del_ther <= ther_low, disable power control mechanism
	unsigned char 		ther_drop; //
	unsigned char		txduty; // 0: disable, 1: enable Tx ducy cycle limitation
	unsigned int		limit_tp; // kbps; used when man=1 and txduty=1
	unsigned char		txduty_level; // 1: -10%, 2: -20% , ..., 9: -90%
	unsigned char		pa;  // percent of reduced throughput of base Tx throughput for limitation
	unsigned char		limit_90pa; // =1, limit all STA to 90%
	unsigned char		pa_refresh;
	unsigned char		limit_level;  // 1: -10%, 2:-20%, ... ,9:-90%
	unsigned int		monitor_time;  // wait monitor_time seconds to check thermal again
	unsigned int		countdown; // currently left time to monitor
	unsigned int		hitcount_hi;
	unsigned int		hitcount_low;
#ifdef RTK_STA_BWC
	int					sta_bwc_en;
	int					sta_bwc_to;
#endif
	unsigned char		debug; // 0: default, 1: print all debug message
	unsigned char		man;  // 0: default, 1: manually set parameters for testing
#endif

	unsigned char		force_bcn;

};

//Filen
//		MESH		WDS			Type		ACCESS_SWAP
//          [9:8]		[7:4]             [3:2]		[1:0]
//

/* Bit mask value of type */
#define ACCESS_SWAP_IO		0x01	/* Do bye-swap in access IO register */
#define ACCESS_SWAP_MEM		0x02	/* Do byte-swap in access memory space */

#define ACCESS_MASK			0x3
#define TYPE_SHIFT			2
#define TYPE_MASK			0x3

#define WDS_SHIFT			0
#define WDS_NUM_CFG			0


enum {
	TYPE_EMBEDDED	= 0,	/* embedded wlan controller */
	TYPE_PCI_DIRECT	= 1,	/* PCI wlan controller and enable PCI bridge directly */
	TYPE_PCI_BIOS	= 2		/* PCI wlan controller and enable PCI by BIOS */
};


enum {
	DRV_STATE_INIT	 = 1,	/* driver has been init */
	DRV_STATE_OPEN	= 2,	/* driver is opened */
	DRV_STATE_VXD_INIT = 4,	/* vxd driver has been opened */
	DRV_STATE_VXD_AP_STARTED	= 8, /* vxd ap has been started */
};

#ifdef RX_BUFFER_GATHER
enum {
	GATHER_STATE_NO = 0,		/* no data pending */
	GATHER_STATE_FIRST = 1, 	/* first segment */
	GATHER_STATE_MIDDLE = 2, 	/* niether first nor last segment */
	GATHER_STATE_LAST = 3		/* last segment */
};
#endif

#ifdef CHECK_TX_HANGUP
#define PENDING_PERIOD		60	// max time of pending period


struct desc_check_info {
	int pending_tick;	// tick value when pending is detected
	int pending_tail;	// descriptor tail number when pending is detected
	int idle_tick;		// tick value when detect idle (tx desc is not free)
	int adaptivity_cnt;	// counter when edcca on
};
#endif


// MAC access control log definition
#define MAX_AC_LOG		32	// max number of log entry
#define AC_LOG_TIME		300	// log time in sec
#define AC_LOG_EXPIRE	RTL_SECONDS_TO_JIFFIES(3)	// entry expire time

struct ac_log_info {
	int	used;		// <>0: in use
	int	cur_cnt;	// current attack counter
	int	last_cnt;	// counter of last time log
	unsigned long last_attack_time; // jiffies time of last attack
	unsigned char addr[MACADDRLEN];	// mac address
};

#ifdef WIFI_SIMPLE_CONFIG
struct wsc_probe_request_info {
	unsigned char addr[MACADDRLEN]; // mac address
	char ProbeIE[PROBEIELEN];
	unsigned short ProbeIELen;
	unsigned long time_stamp; // jiffies time of last probe request
	unsigned char used;
	unsigned char pbcactived;			/* 0528pbc */
};
#endif

struct sta_mac_rssi {
	unsigned char			addr[MACADDRLEN];
	unsigned char			rssi;
	unsigned char 			used;
	unsigned char 			Entry;
	unsigned char 			status;
};


#ifdef PROC_STA_CONN_FAIL_INFO
struct sta_conn_fail_info {
	unsigned char 		used;
	unsigned char		error_state;
	unsigned char		addr[MACADDRLEN];
};
#endif

struct reorder_ctrl_timer {
	struct rtl8192cd_priv	*priv;
	struct stat_info	*pstat;
	unsigned char		tid;
	unsigned int		timeout;
};

#ifdef SW_TX_QUEUE
struct sw_tx_queue_timer {
	struct rtl8192cd_priv	*priv;
	struct stat_info	*pstat;
	unsigned char		qnum;
	UINT32      		timeout;
};
#endif

struct ring_que {
	int qlen;
	int qmax;
	int head;
	int tail;
	struct sk_buff *ring[MAX_PRE_ALLOC_SKB_NUM + 1];
};

#ifdef USE_TXQUEUE
struct txq_node {
	struct list_head list;
	struct sk_buff *skb;
	struct net_device *dev;
};
struct txq_list_head {
	struct list_head list;
	unsigned int qlen;
};

#define init_txq_head(_q) \
	do { \
		INIT_LIST_HEAD(&((_q)->list)); \
		(_q)->qlen = 0; \
	}while(0)
#define is_txq_empty(_q) ((_q)->qlen ? 0 : 1)
#define txq_len(_q) ((_q)->qlen)
#define add_txq_tail(_q, _n) \
	do { \
		list_add_tail(&((_n)->list), &((_q)->list)); \
		(_q)->qlen ++; \
	}while(0)
#define add_txq_head(_q, _n) \
	do { \
		list_add(&((_n)->list), &((_q)->list)); \
		(_q)->qlen ++; \
	}while(0)
#define unlink_txq(_q, _n) \
	do { \
		list_del(&((_n)->list)); \
		(_q)->qlen --; \
	}while(0)
#define deq_txq(_q) \
	( (_q)->qlen ? (struct txq_node *)((_q)->list.next) : NULL ); \
	unlink_txq(_q, (struct txq_node *)((_q)->list.next));
#endif

#ifdef A4_STA
#define A4_STA_HASH_BITS		3
#define A4_STA_HASH_SIZE		(1 << A4_STA_HASH_BITS)
#define A4_STA_AGEING_TIME	300
#define MAX_A4_TBL_NUM			32

struct a4_sta_db_entry {
	struct a4_sta_db_entry	*next_hash;
	struct a4_sta_db_entry	**pprev_hash;
	unsigned char			mac[6];
	unsigned long			ageing_timer;
	struct stat_info		*stat;
};

struct a4_tbl_entry {
	int used;
	struct a4_sta_db_entry	entry;
};
#endif

#ifdef RTK_MESH_METRIC_REFINE

struct mesh_mibValues{
	unsigned char rssiThreshold;
	unsigned char cuThreshold;
	unsigned char noiseThreshold;
	unsigned char rssiWeight;
	unsigned char cuWeight;
	unsigned char noiseWeight;
};

#endif


#if defined(CROSSBAND_REPEATER) || defined(RTK_MESH_METRIC_REFINE)

#define RECORDING_DURATION 30

struct envinfo_one_second{
    unsigned int rssi_value;
    unsigned int cu_value;
    unsigned int noise_value;
};

struct envinfo_data{
    unsigned char index;
    struct envinfo_one_second info_record[RECORDING_DURATION];
    unsigned int rssi_metric;
    unsigned int cu_metric;
    unsigned int noise_metric;
};

#endif



#ifdef SW_ANT_SWITCH

// 20100503 Joseph:
// Add new function SwAntDivCheck8192C().
// This is the main function of Antenna diversity function before link.
// Mainly, it just retains last scan result and scan again.
// After that, it compares the scan result to see which one gets better RSSI.
// It selects antenna with better receiving power and returns better scan result.
//
#define	TP_MODE			0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW		0
#define	TRAFFIC_HIGH	1
#define	TRAFFIC_THRESHOLD	 6600000	// 16 Mbps
#define	TP_MODE_THD			  200000	//500 kbps

#define SELANT_MAP_SIZE	8

// 20100514 Joseph: Add definition for antenna switching test after link.
// This indicates two different the steps.
// In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air.
// In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK
// with original RSSI to determine if it is necessary to switch antenna.
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1
#define SWAW_STEP_RESET		0xff

typedef struct _SW_Antenna_Switch_ {
	unsigned char 		try_flag;
	unsigned char		CurAntenna;
	unsigned char		RSSI_Trying;
	unsigned char		TestMode;
	int					SelectAntennaMap[2][SELANT_MAP_SIZE];
	unsigned char 		mapIndex;
} SWAT_T;

#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#if defined(SW_ANT_SWITCH) || defined(HW_ANT_SWITCH)

typedef enum tag_SW_Antenna_Switch_Definition {
	Antenna_L = 1,
	Antenna_R = 2,
	Antenna_MAX = 3,
} DM_SWAS_E;
#endif
#endif








#if 0//defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
struct channel_utilization_info {
    unsigned char           cu_enable;
    unsigned char           channel_utilization;
    unsigned int            cu_initialcnt;
    unsigned int            cu_cntdwn;
    int                     chbusytime;
    struct timer_list       cu_cntdwn_timer;
};
#endif

typedef struct _sta_bitmap_{
    unsigned int    _staMap_;
    #if (NUM_STAT >32)
    unsigned int    _staMap_ext_1;
    #if (NUM_STAT >64)
    unsigned int    _staMap_ext_2;
    unsigned int    _staMap_ext_3;
    #endif
    #endif
} STA_BITMAP, *PSTA_BITMAP;

struct propietaryIE {
	unsigned int id;
	unsigned char length;
	unsigned char oui[3];
	unsigned char *content;
};

// common private structure which info are shared between root interface and virtual interface
struct priv_shared_info {
	unsigned int			type;
	unsigned long			ioaddr;
	unsigned int			version_id;
#ifdef IO_MAPPING
	unsigned int			io_mapping;
#endif

#ifdef USE_CHAR_DEV
	struct rtl8192cd_chr_priv	*chr_priv;
#endif
#ifdef USE_PID_NOTIFY
	pid_t					wlanapp_pid;
#endif





#if defined(MULTI_STA_REFINE)
	struct timer_list PktAging_timer;
#endif
#if	1//defined(CONCURRENT_MODE) || defined(CONFIG_RTL_92D_SUPPORT)
	int						wlandev_idx;
#endif

	struct tasklet_struct	rx_tasklet;
	struct tasklet_struct	tx_tasklet;
	struct tasklet_struct	oneSec_tasklet;

#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
	unsigned int rf_phy_bb_backup[26];
#endif

	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct list_head		wlan_hdrlist;

	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct list_head		wlanllc_hdrlist;

	struct wlanbuf_poll		*pwlanbuf_poll;
	struct list_head		wlanbuf_list;

	struct wlanicv_poll		*pwlanicv_poll;
	struct list_head		wlanicv_list;

	struct wlanmic_poll		*pwlanmic_poll;
	struct list_head		wlanmic_list;

	struct rtl8192cd_hw		*phw;
	struct rtl8192cd_tx_desc_info 	*pdesc_info;
	unsigned int			have_hw_mic;

	struct aid_obj			*aidarray[NUM_STAT];
	unsigned int			max_fw_macid;

	unsigned char			agc_tab_buf[AGC_TAB_SIZE];
	unsigned char			mac_reg_buf[MAC_REG_SIZE];
	unsigned char			phy_reg_buf[PHY_REG_SIZE];

#ifdef MP_TEST
	unsigned char			phy_reg_mp_buf[PHY_REG_SIZE];
#endif

	unsigned char			phy_reg_pg_buf[PHY_REG_PG_SIZE];
	unsigned char			txpwr_pg_format_abs;
#ifdef TXPWR_LMT
	unsigned char			txpwr_lmt_buf[MAC_REG_SIZE];
	unsigned int			txpwr_lmt_CCK;
	unsigned int			txpwr_lmt_OFDM;
	unsigned int			txpwr_lmt_HT1S;
	unsigned int			txpwr_lmt_HT2S;
	unsigned char			ch_pwr_lmtCCK[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtOFDM[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT20_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT20_2S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT40_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT40_2S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_CCK[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_OFDM[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT20_1S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT20_2S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT40_1S[SUPPORT_CH_NUM];
	unsigned char			ch_tgpwr_HT40_2S[SUPPORT_CH_NUM];
	unsigned int			tgpwr_CCK;
	unsigned int			tgpwr_OFDM;
	unsigned int			tgpwr_HT1S;
	unsigned int			tgpwr_HT2S;
	unsigned int			txpwr_lmt_VHT1S;
	unsigned int			txpwr_lmt_VHT2S;
	unsigned char			ch_pwr_lmtVHT80_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtVHT80_2S[SUPPORT_CH_NUM];

#ifdef BEAMFORMING_AUTO
	unsigned int			txpwr_lmt_TXBF_HT1S;
	unsigned int			txpwr_lmt_TXBF_HT2S;
	unsigned char			ch_pwr_lmtHT20_TXBF_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT20_TXBF_2S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT40_TXBF_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtHT40_TXBF_2S[SUPPORT_CH_NUM];

	unsigned int			txpwr_lmt_TXBF_VHT1S;
	unsigned int			txpwr_lmt_TXBF_VHT2S;
	unsigned char			ch_pwr_lmtVHT80_TXBF_1S[SUPPORT_CH_NUM];
	unsigned char			ch_pwr_lmtVHT80_TXBF_2S[SUPPORT_CH_NUM];


	unsigned int    txbferVHT2TX;
	unsigned int    txbferHT2TX;
	unsigned int	txbf2TXbackoff;
	unsigned int	txbf3TXbackoff;
#endif // BEAMFORMING_AUTO

#endif // TXPWR_LMT

	unsigned char			tgpwr_CCK_new[2];
	unsigned char			tgpwr_OFDM_new[2];
	unsigned char			tgpwr_HT1S_new[2];
	unsigned char			tgpwr_HT2S_new[2];
	unsigned char			tgpwr_VHT1S_new[2];
	unsigned char			tgpwr_VHT2S_new[2];


#ifdef _TRACKING_TABLE_FILE
	unsigned char			txpwr_tracking_2G_CCK[2*TXPWR_TRACKING_PATH_NUM ][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_2G_OFDM[2*TXPWR_TRACKING_PATH_NUM ][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_5GL[2*TXPWR_TRACKING_PATH_NUM ][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_5GM[2*TXPWR_TRACKING_PATH_NUM ][index_mapping_NUM_MAX];
	unsigned char			txpwr_tracking_5GH[2*TXPWR_TRACKING_PATH_NUM ][index_mapping_NUM_MAX];
	unsigned char			tracking_table_new;
#endif

//	unsigned char			phy_reg_2to1[PHY_REG_1T2R];
	unsigned short			fw_IMEM_len;
	unsigned short			fw_EMEM_len;
	unsigned short			fw_DMEM_len;

	spinlock_t				lock;


	// for RF fine tune
	struct rf_finetune_var	rf_ft_var;

	// bcm 11n chipset iot debug
	unsigned int			reg_810_state;	// 0=default, 1=set for bcm

	unsigned int			skip_mic_chk;
	// TXOP enlarge
	unsigned int			txop_enlarge;	// 0=no txop, 1=half txop enlarged, 2=full txop enlarged, e=for Intel, f=for 8192S sta

	struct stat_info                *txpause_pstat;
	unsigned long                   txpause_time;
	unsigned char					rssi_min;
	unsigned char					mimorssi_min;
#ifdef RSSI_MIN_ADV_SEL
	unsigned char			rssi_min_prev;
	unsigned char			min_rssi_data_cnt;
	struct rssi_search_entry	min_rssi_data[NR_RSSI_MIN_DATA];
#endif
	unsigned char			th_mode; //0: boot, 1: tx mode, 2:rx mode
	unsigned char			txop_turbo; //0: TXOP normal mode, 1: TXOP turbo mode
	unsigned char			amsdu_mode; //0:rx amsdu normal, 1:rx amsdu turbo
	unsigned char			cnt_rxcca_check;

	unsigned char			agg_to;
#ifdef WIFI_WMM
	unsigned char			iot_mode_enable;
	unsigned int			iot_mode_VI_exist;
	unsigned int			iot_mode_VO_exist;
	unsigned char			wifi_beq_lower;
	unsigned int           	 	iot_mode_BE_exist;
	unsigned int            	iot_mode_BK_exist;
#endif

#ifdef LOW_TP_TXOP
	unsigned char			BE_cwmax_enhance;
#endif


#ifdef RTK_AC_SUPPORT//  defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	char					txsc_20;    // { 1, 2, 3, 4 }
	char					txsc_40;    // { 9, 10 }
#endif
//    unsigned int            current_rsp_rate;


	char					txsc_20_92e; // {1, 2}

	int						is_40m_bw;
	int						is_40m_bw_bak;
	char						offset_2nd_chan;
//	int						is_giga_exist;


	unsigned int			curr_band;				// remember the current band to save switching time
	unsigned short		fw_signature;
	unsigned char			fw_category;
	unsigned char			fw_function;
	unsigned short			fw_version;
//	unsigned short		fw_src_version;
	unsigned short			fw_sub_version;
	unsigned char			fw_date_month;
	unsigned char			fw_date_day;
	unsigned char			fw_date_hour;
	unsigned char			fw_date_minute;
	unsigned int			h2c_box_full;
	unsigned int			CamEntryOccupied;		// how many entries in CAM?
	unsigned char			rtk_ie_buf[16];
	unsigned int			rtk_ie_len;
	unsigned char			*rtk_cap_ptr;
	unsigned short			rtk_cap_val;
	unsigned char			use_long_slottime;

	// for Tx power control
/*cfg p2p cfg p2p*/
	unsigned char			working_channel2;	/*the assign ch, no consider 20M/40M/80M band width*/
	unsigned char			working_channel;
	unsigned char			ra40MLowerMinus;
	unsigned char			raThdHP_Minus;

#ifdef	INCLUDE_WPS
	struct wsc_context		WSC_CONT_S;
#endif

	unsigned int			current_num_tx_desc;
#ifdef PERIODIC_AUTO_CHANNEL
	unsigned int			PAT;
#ifdef DYNAMIC_AUTO_CHANNEL//SW#5
	unsigned int			all_cli_idle;
	unsigned int			noise_level_pre;
#endif
#endif
	DM_ODM_T				_dmODM;

	unsigned long			hal_SupportMACfunction;
    unsigned long           hal_MAC_Version;


	unsigned char		use_outsrc;

#ifdef TPT_THREAD
	struct task_struct		*tpt_task;	// Tx Power Tracking task
	atomic_t				do_tpt; 				// Do Tx Power Tracking function
#endif

	unsigned char		use_hal;
    unsigned char		use_macHalAPI;

#if defined(__ECOS) || defined(CONFIG_RTL865X_WTDOG) || defined(CONFIG_RTL_WTDOG)
	unsigned long		wtval;
#endif

	unsigned long		tx_dma_err;
	unsigned long		rx_dma_err;
	unsigned long		tx_dma_status;
	unsigned long		rx_dma_status;

#ifdef SUPPORT_AXI_BUS_EXCEPTION
	unsigned long		axi_exception;
#endif

	volatile struct cfg80211_chan_def *dfs_chan_def;

#if 0	//defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
    struct channel_utilization_info      cu_info;
#endif

	unsigned int		WlanSupportAbility;


	/*********************************************************
	 * from here on, data will be clear in rtl8192cd_init_sw() *
	 *********************************************************/

	// for SW LED
	struct timer_list		LED_Timer;
	unsigned int			LED_Interval;
	unsigned char			LED_Toggle;
	unsigned char			LED_ToggleStart;
	unsigned int			LED_tx_cnt_log;
	unsigned int			LED_rx_cnt_log;
	unsigned int			LED_tx_cnt;
	unsigned int			LED_rx_cnt;
	unsigned char			LED_cnt_mgn_pkt;

	struct stat_info		*highTP_found_pstat;
	struct stat_info		*highTP_found_root_pstat;
	struct stat_info		*highTP_found_vxd_pstat;

	//for TxPwrTracking

	unsigned char			Thermal_idx;
	unsigned char			Thermal_log[8];

	unsigned char			ThermalValue;
	unsigned char			ThermalValue_LCK;
	unsigned char			ThermalValue_IQK;
	unsigned char			CCK_index;
	unsigned char			OFDM_index[2];
	unsigned char			CCK_index0;
	unsigned char			OFDM_index0[2];

#if 1//  defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	unsigned char 			No_RF_Write;
	unsigned char			No_BB_Write;
	unsigned int			RegCB0;
	unsigned int			RegEB0;
	unsigned int			RegCB4;

#endif

	unsigned int			RegE94;
	unsigned int			RegE9C;
	unsigned int			RegEB4;
	unsigned int			RegEBC;

	//for IQK
	unsigned int			RegC04;
	unsigned int			Reg874;
	unsigned int			RegC08;
	unsigned int			ADDA_backup[IQK_ADDA_REG_NUM];
	unsigned int			IQK_MAC_backup[IQK_MAC_REG_NUM];


	//for IQK debug
	unsigned int			IQK_total_cnt;
	unsigned int			IQK_fail_cnt;
	unsigned int			PhyVersion;

	int		    	txop_decision;
#ifdef SW_TX_QUEUE
	unsigned char   record_mac[6];
	int             record_qnum;
	int             swq_txmac_chg;
	int             swq_en;
	unsigned short  record_aid;
	unsigned long   swqen_keeptime;
#ifdef SW_TX_QUEUE_SMALL_PACKET_CHECK
	int             swq_boost_delay;
#endif
#endif

	// MP DIG
	unsigned char			mp_dig_on;
	unsigned char			mp_dig_reg_backup;
	unsigned int			RxPWDBAve;
	unsigned int			NumQryPhyStatus;
	unsigned int			LastNumQryPhyStatusAll;
	unsigned int			NumQryPhyStatusCCK;
	unsigned int			NumQryPhyStatusOFDM;

#ifdef POWER_TRIM
	unsigned char 			kfree_value;
	unsigned int			kfree_reg_backup_pathA;
	unsigned int			kfree_reg_backup_pathB;
	unsigned char			kfree_reg_0x65_pathA;
	unsigned char			kfree_reg_0x65_pathB;
#endif

	unsigned char	Power_tracking_on_8812;
	unsigned char	ThermalValue_AVG_8812[AVG_THERMAL_NUM_8812];
	unsigned char	ThermalValue_AVG_index_8812;

	unsigned char	Power_tracking_on_88XX;
	unsigned char	ThermalValue_AVG_88XX[AVG_THERMAL_NUM_88XX];
	unsigned char	ThermalValue_AVG_index_88XX;
	unsigned int	RxTagPollingCount;
	unsigned int	RxTagMismatchCount;
#ifdef RF_MIMO_SWITCH
	unsigned char	rf_status;
#endif

    unsigned int            current_rsp_rate;
//#ifdef MBSSID
	struct rtl8192cd_priv	*bcnDOk_priv;
//#endif

#if defined(TXREPORT)
	int sta_query_idx;
#endif
#ifdef TXRETRY_CNT
	int sta_query_retry_idx;
	int sta_query_retry_macid[2];
#endif
#if (BEAMFORMING_SUPPORT == 1)
	RT_BEAMFORMING_INFO	BeamformingInfo;
	unsigned char soundingLock;
#endif

	// for Rx dynamic tasklet
	unsigned int			rxInt_useTsklt;
	unsigned int			rxInt_data_delta;

#ifdef CHECK_HANGUP
#ifdef CHECK_TX_HANGUP
	struct desc_check_info	Q_info[13];
#endif
#ifdef CHECK_RX_DMA_ERROR
	unsigned int			rx_dma_err_cnt;
	unsigned short			rxff_pkt;
	unsigned short			rxff_rdptr;
	unsigned int			rx_byte_cnt;
#endif
#ifdef CHECK_FW_ERROR
	unsigned int			mcu_tst2;
	unsigned int			mcu_pending_cnt;
#endif
#ifdef CHECK_RX_HANGUP
	unsigned int			rx_hang_checking;
	unsigned int			rx_cntreg_log;
	unsigned int			rx_stop_pending_tick;
	struct rtl8192cd_priv		*selected_priv;
#endif
#ifdef CHECK_BEACON_HANGUP
	unsigned int			beacon_ok_cnt;
	unsigned int			beacon_pending_cnt;
	unsigned int			beacon_wait_cnt;
#endif
#ifdef CHECK_AFTER_RESET
	unsigned int			reset_monitor_cnt_down;
	unsigned int			reset_monitor_pending;
	unsigned int			reset_monitor_rx_pkt_cnt;
#endif
#endif

#ifdef HW_DETEC_POWER_STATE
    unsigned char           HWPwrState[128];
    unsigned char           HWPwroldState[128];
	unsigned char           HWPwrStateUpdateIndex[128];
    unsigned int            HWPwrDetectFailCnt;
    unsigned char           HWPwrStateUpdate[128];
#endif //#ifdef HW_DETEC_POWER_STATE


#ifdef MP_TEST
	unsigned char			mp_datarate;
	unsigned char			mp_antenna_tx;
	unsigned char			mp_antenna_rx;
	unsigned char			mp_txpwr_patha;
	unsigned char			mp_txpwr_pathb;

	unsigned char			mp_cck_txpwr_odd;

	unsigned char			mp_rx_rate;
	unsigned int			mp_FA_cnt;
	unsigned int			mp_CCA_cnt;
	unsigned int			mp_rssi_weight;

	void 					*skb_pool_ptr;
	struct sk_buff 			*skb_pool[NUM_MP_SKB];
	int						skb_head;
	int						skb_tail;

	unsigned int			mp_ctx_count;
	unsigned int			mp_ctx_pkt;
	unsigned char			mp_rssi;
	unsigned char			mp_sq;
	struct rf_misc_info		mp_rf_info;

	unsigned char			mp_ofdm_swing_idx;
	unsigned char			mp_cck_swing_idx;
	unsigned char			mp_txpwr_tracking;
	unsigned char			mp_tx_dpk_tracking;
	int						mp_pkt_len;
#ifdef MP_SWITCH_LNA
	unsigned char			rx_packet_ss_a;
	unsigned char			rx_packet_ss_b;
#endif

#ifdef B2B_TEST
	volatile unsigned long	mp_rx_ok, mp_rx_sequence, mp_rx_lost_packet, mp_rx_dup;
	volatile unsigned short	mp_cached_seq;
	int 					mp_rx_waiting;
	volatile unsigned int	mp_mac_changed;
	unsigned long			txrx_elapsed_time;
	unsigned long			txrx_start_time;
#endif
//arx parameters
	unsigned int			mp_filter_flag;
	unsigned char 			mp_filter_DA[6];
	unsigned char 			mp_filter_SA[6];
	unsigned char 			mp_filter_BSSID[6];
	UINT64					mp_rx_mac_ok;
	UINT64					mp_rx_mac_crc_err;
	UINT64					mp_rx_phy_ok;
	UINT64					mp_rx_phy_crc_err;
	unsigned char			mimorssi[4];
#endif // MP_TEST

	// monitor Tx and Rx
	unsigned long			tx_packets_pre;
	unsigned long			rx_packets_pre;

	// bcm old 11n chipset iot debug, and TXOP enlarge
	unsigned long			current_tx_bytes;
	unsigned long			current_rx_bytes;

	// for collecting probe request info
	unsigned int			ProbeReqEntryOccupied;
	unsigned int			ProbeReqEntryNum;


	u8Byte					NumTxBytesUnicast;
	u8Byte					NumRxBytesUnicast;
	unsigned char			bScanInProcess;
	u8Byte					dummy;

	unsigned char			CurrentChannelBW;
	unsigned char			*txcmd_buf;
	unsigned long			cmdbuf_phyaddr;
	unsigned int			InterruptMask;
	unsigned int			InterruptMaskExt;
	unsigned int			rx_rpt_ofdm;
	unsigned int			rx_rpt_cck;
	unsigned int			rx_rpt_ht;
	unsigned int			successive_bb_hang;
#ifdef CLIENT_MODE
	int					AP_BW;
#endif

	unsigned long			rxFiFoO_pre;
//	unsigned int			pkt_in_hiQ;

	struct ring_que 		skb_queue;

	struct timer_list			rc_sys_timer;
	struct reorder_ctrl_timer	rc_timer[RC_TIMER_NUM];
	unsigned short				rc_timer_head;
	unsigned short				rc_timer_tail;
	unsigned short				rc_timer_tick;

	struct timer_list			rc_sys_timer_cli;
	struct reorder_ctrl_timer	rc_timer_cli[RC_TIMER_NUM];
	unsigned short				rc_timer_head_cli;
	unsigned short				rc_timer_tail_cli;
	unsigned short				rc_timer_tick_cli;

	struct reorder_ctrl_timer	amsdu_timer[AMSDU_TIMER_NUM];
	unsigned short				amsdu_timer_head;
	unsigned short				amsdu_timer_tail;
#ifdef SW_TX_QUEUE
    struct sw_tx_queue_timer    swq_timer[SWQ_TIMER_NUM];
    unsigned short              swq_timer_head;
    unsigned short              swq_timer_tail;
    UINT32                      swq_last_tsf;
    UINT32                      swq_current_timeout;
    struct timer_list           swq_sw_timer;
    unsigned char               swq_use_hw_timer;
    unsigned char				swq_numActiveSTA;
    unsigned long				swq_turbo_time;

    int						    has_triggered_sw_tx_Q_tasklet;
    struct tasklet_struct	    swq_tasklet;
#endif

#ifdef RTK_ATM
    UINT32                      atm_swq_current_timeout;
    struct timer_list           atm_swq_sw_timer;
    unsigned char               atm_swq_use_hw_timer;

    unsigned int                atm_min_burstsize;
    unsigned int                atm_max_burstsize;
    unsigned int                atm_min_txrate;
    unsigned int                atm_max_txrate;
    unsigned int                atm_ttl_stanum;//sta num of 2.4G/5G interface

    unsigned int                atm_ttl_match_stanum;// num of cli that in client list of interface 2.4G/5G
    unsigned int                atm_ttl_match_statime;// total matched sta time

    //unsigned int              atm_burst_base;
    unsigned int                atm_timer;

    unsigned int                atm_swq_en;
    unsigned int                atm_timeout_count;
    unsigned int                atm_timer_init;//timer init flag
#endif//RTK_ATM


	// ht associated client statistic
#ifdef WIFI_WMM
	unsigned int			ht_sta_num;
	STA_BITMAP  			mimo_ps_dynamic_sta;
#endif

	unsigned int			set_led_in_progress;

	struct stat_info*		CurPstat[4]; // for tx desc break field
//	STA_BITMAP			has_2r_sta; // Used when AP is 2T2R. bitmap of 2R aid
	int						has_triggered_rx_tasklet;
	int						has_triggered_tx_tasklet;

    unsigned short          total_cam_entry;
    unsigned short          fw_support_sta_num;
    unsigned char			fw_free_space;
    unsigned char			remapped_aidarray[NUM_STAT];

	// for DIG
	unsigned char			DIG_on;
	unsigned char			restore;
	unsigned short			threshold0;
	unsigned short			threshold1;
	unsigned short			threshold2;
	unsigned char			FA_upper;
	unsigned char			FA_lower;
	unsigned short			ofdm_FA_cnt1;
	unsigned short			ofdm_FA_cnt2;
	unsigned short			ofdm_FA_cnt3;
	unsigned short			ofdm_FA_cnt4;
	unsigned short			cck_FA_cnt;
#ifdef INTERFERENCE_CONTROL
	unsigned int			ofdm_FA_total_cnt;
#endif
	unsigned int			FA_total_cnt;
	unsigned int			CCA_total_cnt;

#if defined(CONFIG_RTL_NOISE_CONTROL) || defined(CONFIG_RTL_NOISE_CONTROL_92C)
	unsigned char			DNC_on;
#endif


	int 					digDownCount;
	int 					digDeadPoint;
	int 					digDeadPointCandidate;
	int 					digDeadPointHitCount;

	unsigned char			cca_path;
	unsigned char			fw_q_fifo_count;

	unsigned int			current_tx_rate;
	unsigned char			ht_current_tx_info;	// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI


#ifdef RESERVE_TXDESC_FOR_EACH_IF
	unsigned int			num_txdesc_cnt;		// less or equal max available tx desc
	unsigned int			num_txdesc_upper_limit;
	unsigned int			num_txdesc_lower_limit;
#endif
#ifdef USE_TXQUEUE
	unsigned int			num_txq_cnt;
	unsigned int			num_txq_upper_limit;
	unsigned int			num_txq_lower_limit;
#endif


	// Retry Limit register content
	unsigned short		RL_setting;

#ifdef SW_ANT_SWITCH
	SWAT_T				DM_SWAT_Table;
	struct	stat_info  *RSSI_target;
	struct timer_list	swAntennaSwitchTimer;

	unsigned int		RSSI_sum_R;
	unsigned int		RSSI_cnt_R;
	unsigned int		RSSI_sum_L;
	unsigned int		RSSI_cnt_L;
	unsigned int 		RSSI_test;
	unsigned int 		lastTxOkCnt;
	unsigned int 		lastRxOkCnt;
	unsigned int 		TXByteCnt_R;
	unsigned int 		TXByteCnt_L;
	unsigned int 		RXByteCnt_R;
	unsigned int 		RXByteCnt_L;
	unsigned int 		TrafficLoad;
#endif

#ifdef DETECT_STA_EXISTANCE
	unsigned char				bRLShortened;
#endif

	unsigned int			dfsSwitchChannel;
	unsigned int			dfsSwitchChCountDown;
	unsigned int			dfsSwCh_ongoing;

#ifdef RX_BUFFER_GATHER
	unsigned int 			gather_state;
#ifdef RX_BUFFER_GATHER_REORDER
		struct list_head		*gather_list_p;

#else
		struct list_head		gather_list;
#endif

	int					gather_len;
	int                     pkt_total_len;
#endif

#ifdef USE_TXQUEUE
	struct txq_list_head	txq_list[4];
	unsigned int			txq_isr;
	unsigned int			txq_stop;
	unsigned int			txq_check;
	struct list_head		txq_pool;
	unsigned char			*txq_pool_addr;
#endif

    unsigned char  	Reg_RRSR_2;
    unsigned char  	Reg_81b;
    STA_BITMAP      marvellMapBit;
#if defined(WIFI_11N_2040_COEXIST_EXT)
    STA_BITMAP      _40m_staMap;
    STA_BITMAP      _80m_staMap;
#endif
	unsigned char	ctrl40m;

#ifdef TX_EARLY_MODE
	unsigned int	em_waitq_on;
	unsigned int	em_tx_byte_cnt;
	unsigned int	reach_tx_limit_cnt;
#endif
	unsigned short	aggrmax_bak;
	unsigned char	aggrmax_change;

	unsigned int	iqk_2g_done;

	unsigned char 	pre_channel;

	unsigned int	intel_active_sta;
	unsigned int	intel_rty_lmt;
	unsigned int 	paused_sta_num;
	unsigned int 	unlock_counter1;
	unsigned int 	unlock_counter2;
	unsigned int 	lock_counter;
	unsigned int total_assoc_num;

	unsigned int		switch_chan_rp;
	unsigned int		switch_2ndchoff_rp;
	unsigned int		band_width_rp;

	struct timer_list	vxd_ibss_beacon; //timer to issue beacon for vxd_adhoc

#ifdef MCR_WIRELESS_EXTEND
	unsigned int		cmw_link;
#endif
	unsigned long		no_sta_link_tick;			// tick value when no sta connect
	unsigned char		acs_for_adaptivity_flag;	// Set for hold TXPAUSE during NHM-ACS
	unsigned char		reg_tapause_bak;




#ifdef BT_COEXIST_SOCKET
struct rtl_btc_info {
	u8 bt_type;
	u8 btcoexist;
	u8 ant_num;
};

struct rtl_btc_ops {
	void (*btc_init_variables)(struct rtl8192cd_priv *priv);
	void (*btc_init_hal_vars)(struct rtl8192cd_priv *priv);
	void (*btc_init_hw_config)(struct rtl8192cd_priv *priv);
	void (*btc_ips_notify)(struct rtl8192cd_priv *priv, u8 type);
	void (*btc_lps_notify)(struct rtl8192cd_priv *priv, u8 type);
	void (*btc_scan_notify)(struct rtl8192cd_priv *priv, u8 scantype);
	void (*btc_connect_notify)(struct rtl8192cd_priv *priv, u8 action);
	void (*btc_mediastatus_notify)(struct rtl8192cd_priv *priv,	enum rt_media_status mstatus);
	void (*btc_periodical)(struct rtl8192cd_priv *priv);
	void (*btc_halt_notify)(void);
	void (*btc_btinfo_notify)(struct rtl8192cd_priv *priv, u8 *tmp_buf, u8 length);
	bool (*btc_is_limited_dig)(struct rtl8192cd_priv *priv);
	bool (*btc_is_disable_edca_turbo)(struct rtl8192cd_priv *priv);
	bool (*btc_is_bt_disabled)(struct rtl8192cd_priv *priv);
	void (*btc_special_packet_notify)(struct rtl8192cd_priv *priv,u8 pkt_type);
	void (*btc_set_hci_version) (u16 hci_version);
	void (*btc_set_bt_patch_version) (u16 bt_hci_version, u16 bt_patch_version);
	void (*btc_stack_update_profile_info) (void);
	void (*btc_init_socket) (struct rtl8192cd_priv *priv);
	void (*btc_close_socket) (struct rtl8192cd_priv *priv);
};

struct rtl_bt_coexist {
	struct rtl_btc_ops *btc_ops;
	struct rtl_btc_info btc_info;
};

struct workqueue_struct *rtl_wq;
struct delayed_work socket_wq;
struct rtl_bt_coexist btcoexist;
struct bt_coex_info coex_info;

#endif


#if defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined (RTK_SMART_ROAMING)
	    unsigned char           cu_enable; /*channel utilization calculated enable/disable*/
#endif


};


#ifdef SUPPORT_MONITOR
struct rtl_wifi_header {
#ifdef CONFIG_PKT_FILTER
		unsigned short rt_pkt_len;
		unsigned short rt_payload_len;
		unsigned char pkt[PTK_FILTER_LEN];
#else
		unsigned char rt_frame_type;			// Frame type
		unsigned char rt_sub_frame_type;		//Sub frame type
		unsigned char rt_rssi;					//RSSI
		unsigned char rt_addr1[6];        			//DA mac addr
		unsigned char rt_addr2[6];        			//SA mac addr
		unsigned char rt_rate;		 			//Rx_rate
		unsigned char rt_channel_num;			//Channel Num
#endif
} __attribute__ ((packed));
#endif


typedef struct rtl8192cd_priv {
	struct rtl8192cd_priv*	wlan_priv;		//This element shall be put at top of this struct

	void *                           HalFunc;
	void *                           HalData;
    PHALMAC_ADAPTER                  pHalmac_adapter;
    PHALMAC_API                      pHalmac_api;
    PHALMAC_PLATFORM_API             pHalmac_platform_api;
	int						drv_state;		// bit0 - init, bit1 - open/close
	struct net_device		*dev;

	//struct wiphy 			*wiphy;
	struct rtknl 			*rtk;
	struct wireless_dev		wdev;
	volatile struct cfg80211_scan_request *scan_req;
	//struct pci_dev			*pdev;
	//struct net 				init_net;
	//dev_t 					rtk88e;
	//struct class 			*c1;
	//struct device			*tmp_device;
	unsigned char			receive_connect_cmd;
	unsigned char			cfg80211_interface_add;
	u64 					mgmt_action_id;

	struct wifi_mib 		*pmib;

	struct wlan_acl_poll	*pwlan_acl_poll;
	struct list_head		wlan_aclpolllist;	// this is for poll management
	struct list_head		wlan_acl_list;		// this is for auth checking

	DOT11_QUEUE				*pevent_queue;
	DOT11_EAP_PACKET		*Eap_packet;
#ifdef INCLUDE_WPS
#ifndef CONFIG_MSC
	DOT11_EAP_PACKET		*upnp_packet;
#endif
#endif

#ifdef _INCLUDE_PROC_FS_
	struct proc_dir_entry	*proc_root;
	unsigned int			txdesc_num;
	unsigned char			*phypara_file_start;
	unsigned char			*phypara_file_end;
#endif

#ifdef ENABLE_RTL_SKB_STATS
	atomic_t 				rtl_tx_skb_cnt;
	atomic_t				rtl_rx_skb_cnt;
#endif

	struct priv_shared_info	*pshare;		// pointer of shared info, david

	struct rtl8192cd_priv		*proot_priv;	// ptr of private structure of root interface
	struct rtl8192cd_priv		*pvxd_priv;		// ptr of private structure of virtual interface
	struct rtl8192cd_priv		*pvap_priv[RTL8192CD_NUM_VWLAN];	// ptr of private structure of vap interface
	short					vap_id;
	short					vap_init_seq;
	short					vap_count;
	int						bcn_period_bak;

	/*
	 *	timer for the blocked channel if radar detected
	 */
	struct timer_list		ch52_timer;
	struct timer_list		ch56_timer;
	struct timer_list		ch60_timer;
	struct timer_list		ch64_timer;
	struct timer_list		ch100_timer;
	struct timer_list		ch104_timer;
	struct timer_list		ch108_timer;
	struct timer_list		ch112_timer;
	struct timer_list		ch116_timer;
	struct timer_list		ch120_timer;
	struct timer_list		ch124_timer;
	struct timer_list		ch128_timer;
	struct timer_list		ch132_timer;
	struct timer_list		ch136_timer;
	struct timer_list		ch140_timer;
	struct timer_list		ch144_timer;

	/*
	 *	blocked channel will be removed from available_chnl[32] and placed in this list
	 */
	unsigned int			NOP_chnl[15];
	unsigned int			NOP_chnl_num;

	unsigned int			Not_DFS_chnl[32];
	unsigned int			Not_DFS_chnl_num;

#if defined(DOT11D) || defined(DOT11H) || defined(DOT11K)
    unsigned char           countryTableIdx;
#endif

#ifdef CONFIG_IEEE80211V
	struct bss_transition_para bssTransPara;
	struct target_transition_list transition_list[MAX_TRANS_LIST_NUM];
	unsigned char transition_list_bitmask[(MAX_TRANS_LIST_NUM + 7)/8];
	bool 				startCounting;
#endif
#ifdef SUPPORT_MONITOR
	struct timer_list		chan_switch_timer;
	BOOLEAN 					is_monitor_mode;
	unsigned int 			chan_num;
#endif

	WPA_GLOBAL_INFO			*wpa_global_info;
	struct ss_res			*site_survey;

#ifdef CHECK_HANGUP
	int						reset_hangup;
	unsigned int			check_cnt_tx;
	unsigned int			check_cnt_rx;
	unsigned int			check_cnt_isr;
	unsigned int			check_cnt_bcn;
	unsigned int			check_cnt_rst;
	unsigned int			check_cnt_bb;
	unsigned int			check_cnt_fw;
	unsigned int			cnt_sta1_only;
	unsigned int			cnt_sta2_only;
	unsigned int			cnt_sta1_sta2;
	unsigned int			cnt_mu_swqoverflow;
	unsigned int			cnt_mu_swqtimeout;
	unsigned int			check_cnt_adaptivity;
#endif

	unsigned int			auto_channel;			// 0: not auto, 1: auto select this time, 2: auto select next time
	unsigned int			auto_channel_backup;
	unsigned long			up_time;
	unsigned int			auto_channel_step;





#if defined(CONFIG_RTL_NEW_AUTOCH) && defined(SS_CH_LOAD_PROC)
	unsigned int			ch_ss_rpt_cnt;
	struct ss_report		ch_ss_rpt[MAX_BSS_NUM];
#endif

	unsigned int	nat25_filter; //0:disable, 1:accept, 2:deny
	unsigned short  nat25_filter_ethlist[NAT25_FILTER_ETH_NUM]; /*ethernet type list for nat25 filter*/
	unsigned char   nat25_filter_ipprotolist[NAT25_FILTER_IPPROTO_NUM]; /*ip prototcol list for nat25 filter*/

#ifdef RTK_STA_BWC
	struct timer_list	sta_bwc_timer;
#endif


#if defined(CROSSBAND_REPEATER) || defined(RTK_MESH_METRIC_REFINE)
	struct envinfo_data envinfo;
#endif


#ifdef SDIO_AP_OFFLOAD
	struct work_struct ap_cmd_queue;
#endif

#ifdef RTK_ATM
    unsigned int		atm_iftime;
    unsigned int		atm_sta_num;//num of cli of every SSID
    unsigned int		atm_ttl_txtime;//total txtime per interface 2.4G/5G in 1sec
#endif

#if 1//backup derivePSK
	unsigned int		is_PSK_backup;
	unsigned char		Backup_PSK[A_SHA_DIGEST_LEN*2];
	unsigned char		Backup_PassPhrase[65];
	unsigned char		Backup_SSID[32];
	unsigned int		Backup_SSIDLen;
#endif

	/*********************************************************
	 * from here on, data will be clear in rtl8192cd_init_sw() *
	 *********************************************************/
	struct net_device_stats	net_stats;
	struct extra_stats		ext_stats;
#ifdef TLN_STATS
	struct tln_wifi_stats		wifi_stats;
	struct tln_ext_wifi_stats	ext_wifi_stats;
	unsigned int			stats_time_countdown;
#endif
    int reperater_idx;
	int bIntMigration;

	struct timer_list		frag_to_filter;
	unsigned int			frag_to;

	struct timer_list		expire_timer;
#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
	struct timer_list			ps_timer;
#endif

	unsigned int			auth_to;	// second. time to expire in authenticating
	unsigned int			assoc_to;	// second. time to expire before associating
	unsigned int			expire_to;	// second. time to expire after associating

	struct timer_list		ss_timer;	//ss_timer: site_survey timer
	int                     rescantype;
	unsigned int			site_survey_times;
	unsigned char			ss_ssid[32];
	unsigned int			ss_ssidlen;
	unsigned char			ss_req_ongoing;
#if defined( WIFI_SIMPLE_CONFIG	) && defined(UNIVERSAL_REPEATER)
    unsigned char	        wsc_ss_delay;    //for VXD WPS Scan delay
#endif
#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	wait_queue_head_t		ss_wait;
#endif

	unsigned int			auth_seq;
	unsigned char			chg_txt[128];

	struct list_head		stat_hash[NUM_STAT];
	struct list_head		asoc_list;
	struct list_head		auth_list;
	struct list_head		defrag_list;
	struct list_head		sleep_list;
	struct list_head		wakeup_list;
	struct list_head		addRAtid_list;	// to avoid add RAtid fail
	struct list_head		addrssi_list;
	struct list_head		addps_list;
#ifdef WIFI_SIMPLE_CONFIG
	struct list_head		wsc_disconn_list;
	spinlock_t			wsc_disconn_list_lock;
#endif

#ifdef WIFI_SIMPLE_CONFIG
	unsigned int			beaconbuf[MAX_WSC_IE_LEN];
	struct wsc_probe_request_info wsc_sta[MAX_WSC_PROBE_STA];
	unsigned int 			wps_issue_join_req;
	unsigned int			recover_join_req;
	struct bss_desc			dot11Bss_original;
	int						hidden_ap_mib_backup;
	unsigned	char		*pbeacon_ssid;

	unsigned	char 		orig_SSID[33];
	int 					orig_SSID_LEN;

#else
	unsigned int			beaconbuf[128];
#endif

	struct sta_mac_rssi     probe_sta[MAX_PROBE_REQ_STA];
	unsigned int			ProbeReqEntryOccupied;
	unsigned int			ProbeReqEntryNum;


#ifdef STA_ASSOC_STATISTIC
	struct sta_mac_rssi     reject_sta[MAX_PROBE_REQ_STA];
	struct sta_mac_rssi     removed_sta[MAX_PROBE_REQ_STA];
	struct sta_mac_rssi     assoc_sta[MAX_PROBE_REQ_STA];
	unsigned int			RejectAssocEntryOccupied;
	unsigned int			RejectAssocEntryNum;
	unsigned int			RemoveAssocEntryOccupied;
	unsigned int			RemoveAssocEntryNum;
	unsigned int			AssocStatusEntryOccupied;
	unsigned int			AssocStatusEntryNum;
#endif
#ifdef PROC_STA_CONN_FAIL_INFO
	struct sta_conn_fail_info	sta_conn_fail[64];
#endif

	unsigned int			beaconbuf_ibss_vxd[128]; //beacon data for vxd_adhoc
	unsigned int			beaconbuf_ibss_vxd_len;

	struct ht_cap_elmt		ht_cap_buf;
	unsigned int			ht_cap_len;
	struct ht_info_elmt		ht_ie_buf;
	unsigned int			ht_ie_len;
	unsigned int			ht_legacy_obss_to;
	unsigned int			ht_nomember_legacy_sta_to;
	unsigned int			ht_legacy_sta_num;
	unsigned int			ht_protection;
	unsigned int			dc_th_current_state;


#ifdef RTK_AC_SUPPORT //defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
	struct vht_cap_elmt		vht_cap_buf;
	unsigned int			vht_cap_len;
	struct vht_oper_elmt 	vht_oper_buf;
	unsigned int			vht_oper_len;
#endif

	// to avoid add RAtid fail
	struct timer_list		add_ps_timer;


	unsigned short			timoffset;
	unsigned char			dtimcount;
	unsigned char			pkt_in_dtimQ;
	unsigned char			pkt_in_hiQ;

	//struct stat_info_cache	stainfo_cache;
	struct stat_info		*pstat_cache;

	struct list_head		rx_datalist;
	struct list_head		rx_mgtlist;
	struct list_head		rx_ctrllist;
	spinlock_t				defrag_lock;

	int						assoc_num;		// association client number
	int						link_status;

	unsigned short			*pBeaconCapability;		// ptr of capability field in beacon buf
//	unsigned char			*pBeaconErp;			// ptr of ERP field in beacon buf

	unsigned int			available_chnl[MAX_CHANNEL_NUM];		// all available channel we can use
	unsigned int			available_chnl_num;		// record the number
	unsigned int            MultiSTA_available_chnl[MAX_CHANNEL_NUM];        /*when more than one VAP interface as  STA mode*/
	unsigned int            MultiSTA_available_chnl_num;
	unsigned int            MultiSTA_available_backup;

#ifdef CONFIG_RTL_NEW_AUTOCH
	unsigned int			chnl_ss_fa_count[MAX_CHANNEL_NUM];	// record FA count while ss
	unsigned int			chnl_ss_cca_count[MAX_CHANNEL_NUM];	// record CCA count while ss
	unsigned int			chnl_ss_ofdm_fa_count[76];	// record OFDM FA count while ss
	unsigned int			chnl_ss_cck_fa_count[76];	// record CCK FA count while ss
#ifdef SS_CH_LOAD_PROC
	unsigned char			chnl_ss_load[MAX_CHANNEL_NUM];	// record noise level while ss
#endif
	unsigned int			chnl_ss_mac_rx_count[MAX_CHANNEL_NUM];
	unsigned int			chnl_ss_mac_rx_count_40M[MAX_CHANNEL_NUM];
	unsigned int			nhm_cnt[MAX_NUM_20M_CH][11];
	unsigned int			nhm_cnt_round[1][MAX_NUM_20M_CH][11];

	unsigned int			acs2_round_cn;
	unsigned int			cck_th_backup;
	unsigned int			clm_cnt[MAX_NUM_20M_CH];
#endif


	//unsigned int			oper_band;				// remember the operating bands
	unsigned int			supported_rates;
	unsigned int			basic_rates;

	// for MIC check
	struct timer_list		MIC_check_timer;
	struct timer_list		assoc_reject_timer;
	unsigned int			MIC_timer_on;
	unsigned int			assoc_reject_on;
	unsigned char                   assoc_reject_mac[6];

#ifdef CLIENT_MODE
	struct timer_list		reauth_timer;
	unsigned int			reauth_count;

	struct timer_list		reassoc_timer;
	unsigned int			reassoc_count;

	struct timer_list		idle_timer;
	struct timer_list		dfs_cntdwn_timer;

#ifdef HS2_SUPPORT
	struct timer_list		disassoc_timer;
#endif
	unsigned int			join_req_ongoing;
	int						join_index;
	unsigned long			jiffies_pre;
	unsigned int			ibss_tx_beacon;
	unsigned int			rxBeaconNumInPeriod;
	unsigned int			rxBeaconCntArray[ROAMING_DECISION_PERIOD_ARRAY];
	unsigned int			rxBeaconCntArrayIdx;	// current index of array
	unsigned int			rxBeaconCntArrayWindow;	// Is slide windows full
	unsigned int			rxBeaconPercentage;
	unsigned int			rxDataNumInPeriod;
	unsigned int			rxDataCntArray[ROAMING_DECISION_PERIOD_ARRAY];
	unsigned int			rxMlcstDataNumInPeriod;
	unsigned int			rxBeaconMissConti;
//	unsigned int			rxDataNumInPeriod_pre;
//	unsigned int			rxMlcstDataNumInPeriod_pre;
	unsigned int			dual_band;
	unsigned int			supported_rates_alt;
	unsigned int			basic_rates_alt;
	unsigned char 			rx_timestamp[8];
	UINT64					prev_tsf;
    unsigned int            dot114WayStatus;
	unsigned int			is_4Way_finished;
#endif

	int						authModeToggle;		// auth mode toggle referred when auto-auth mode is set under client mode, david
	int						authModeRetry;		// auth mode retry sequence when auto-auth mode is set under client mode

	int						acLogCountdown;		// log count-down time
	struct ac_log_info		acLog[MAX_AC_LOG];

	struct tx_desc			*amsdu_first_desc;

	unsigned int			bcnTxAGC;
	unsigned int            bcnTxAGC_bak;
	unsigned short          tx_beacon_len;

#ifdef WIFI_11N_2040_COEXIST
	struct obss_scan_para_elmt		obss_scan_para_buf;
	unsigned int					obss_scan_para_len;
	unsigned int					bg_ap_timeout;
#ifdef CLIENT_MODE
	unsigned int					bg_ap_timeout_ch[14];
	unsigned int					intolerant_timeout;
	unsigned int					coexist_connection;
#endif
	STA_BITMAP  					force_20_sta;
	STA_BITMAP  					switch_20_sta;
#endif



#ifdef RESERVE_TXDESC_FOR_EACH_IF
	unsigned int			use_txdesc_cnt[4];
#endif
#ifdef USE_TXQUEUE
	unsigned int			use_txq_cnt[4];
#endif
#ifdef COCHANNEL_RTS
	unsigned int					cochannel_to;
#endif

	/*********************************************************************
	 * from here on till EE_Cached will be backup during hang up reset   *
	 *********************************************************************/
#ifdef CLIENT_MODE
	unsigned int			join_res;
	unsigned int			beacon_period;
	unsigned short			aid;
	unsigned int			ps_state;
#if defined(WIFI_WMM) && defined (WMM_APSD)
	unsigned int			uapsd_assoc;
#endif
	unsigned int			macclone_completed;
	struct nat25_network_db_entry	*nethash[NAT25_HASH_SIZE];
	int						pppoe_connection_in_progress;
	unsigned char			pppoe_addr[MACADDRLEN];
	unsigned char			scdb_mac[MACADDRLEN];
	unsigned char			scdb_ip[4];
	struct nat25_network_db_entry	*scdb_entry;
	unsigned char			br_mac[MACADDRLEN];
	unsigned char			br_ip[4];
	unsigned char			ukpro_mac[MACADDRLEN];	// mac address of unknown protocol
	unsigned char			ukpro_mac_valid;		// if the above entry is valid
	unsigned char 			up_flag;
#endif

#ifdef MICERR_TEST
	unsigned int			micerr_flag;
#endif

	struct timer_list		DFS_timer;			/* timer for radar detection */
	struct timer_list		DFS_TXPAUSE_timer;	/* timer for checking whether restarting TX or not*/
	struct timer_list		ch_avail_chk_timer;	/* timer for channel availability check */
	struct timer_list		dfs_chk_timer;		/* timer for dfs trigger */
	struct timer_list		dfs_det_chk_timer;	/* timer for channel busy check under dfs */
	unsigned int			FA_count_pre;
	unsigned int			FA_count_inc_pre;
	unsigned int			VHT_CRC_ok_cnt_pre;
	unsigned int			HT_CRC_ok_cnt_pre;
	unsigned int			LEG_CRC_ok_cnt_pre;
	unsigned char			g_ti_pre[16];
	unsigned char			g_pw_pre[6];
	unsigned char			g_pri_pre[6];
	unsigned char			radar_det_mask_hist[10];
	unsigned char			mask_idx;
	unsigned char			mask_hist_checked;
	unsigned char			pulse_flag_hist[10];
	unsigned char			det_asoc_clear;
	unsigned int			short_pulse_cnt_pre;
	unsigned int			long_pulse_cnt_pre;
	unsigned int			st_L2H_cur;
	unsigned char           idle_flag;
	unsigned char			ini_gain_pre;
	unsigned char			ini_gain_cur;
	unsigned char			peak_th;
	unsigned char			short_pulse_cnt_th;
	unsigned char			long_pulse_cnt_th;
	unsigned char			peak_window;
	unsigned char			nb2wb_th;
	unsigned char			three_peak_opt;
	unsigned char			three_peak_th2;
	unsigned char           ch_120_132_CAC_end;
	int						pwdb_th;
	int						PSD_report_right[10][20];
	int						PSD_report_left[10][20];
	int						max_hold_right[20];
	int						max_hold_left[20];
	int						fa_inc_hist[5];

#ifdef GBWC
	struct timer_list		GBWC_timer;
	struct pkt_queue		GBWC_tx_queue;
	struct pkt_queue		GBWC_rx_queue;
	unsigned int			GBWC_tx_count;
	unsigned int			GBWC_rx_count;
	unsigned int			GBWC_consuming_Q;
#endif


#ifdef SUPPORT_SNMP_MIB
	struct mib_snmp			snmp_mib;
#endif

	struct pkt_queue		dz_queue;	// Queue for multicast pkts when there is sleeping sta
	unsigned char			release_mcast;
    unsigned char           func_off_already;


	int						amsdu_len;
	int						ampdu_len;

    int                     amsdu_num;

#ifdef CHECK_RX_HANGUP
	unsigned long			rx_packets_pre1;
	unsigned long			rx_packets_pre2;
	unsigned int 			rx_start_monitor_running;
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	unsigned int			change_toggle;
	unsigned int			pre_pkt_cnt;
	unsigned int			pkt_nsec_diff;
	unsigned int			poll_usb_cnt;
	unsigned int			auto_rate_mask;
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	int						global_vlan_enable;
	struct vlan_info	vlan_setting;
#endif

#ifdef SUPPORT_TX_MCAST2UNI
	int 							stop_tx_mcast2uni;
#endif

#ifdef CONFIG_OFFLOAD_FUNCTION
	unsigned int offload_function_ctrl;
	unsigned short offload_bcn_page;
	unsigned short offload_proc_page;
#ifdef CONFIG_32K
    unsigned char offload_32k_flag;
#endif
#endif //#ifdef CONFIG_OFFLOAD_FUNCTION

#ifdef CONFIG_8814_AP_MAC_VERI
	unsigned char macID_temp;
    unsigned char lowestRate_TXDESCen;
    unsigned int lowestRate;
    unsigned char RXMACIDTestEn;
    unsigned char pwrState[128];
    unsigned char pwrHWState[128];
    unsigned char pwroldHWState[128];
    unsigned int pwrStateCnt[128];
    unsigned int pwrStateHWCnt[128];
    unsigned char testResult;
    unsigned short hw_seq[128];
    unsigned short sw_seq;
    unsigned char test_Compare;
    unsigned char sw_Carrier;
    unsigned char hw_Carrier;
    unsigned char tx_pwrBit;
    unsigned char BMC_RTY_LMT;
    unsigned char ProtectionType;
#endif //#ifdef CONFIG_OFFLOAD_FUNCTION

#ifdef AP_PS_Offlaod
    unsigned char APPS_phase;
#endif




/* merge from osk 8192su driver to limit tx sleep queue 20100818*/

	int				em_txop_tp_high;
#ifdef A4_STA
	struct list_head			a4_sta_list;
	struct a4_sta_db_entry		*machash[A4_STA_HASH_SIZE];
	struct a4_tbl_entry 		a4_ent[MAX_A4_TBL_NUM];
#endif

#ifdef TV_MODE
   unsigned char      tv_mode_status; /*0: disable, 1:enable, 2:auto(disable), 3:auto(enable)*/
#endif


#ifdef WIFI_WPAS
	unsigned char 	wpas_manual_assoc; //_Eric ??
#endif
	int update_bcn_period;

#ifdef SUPPORT_MULTI_PROFILE
	int	profile_idx;			// indicate next used profile.
	int mask_n_band;
#endif
#ifdef SWITCH_CHAN
	int	chan_backup;
	int	bw_backup;
	int	offset_backup;
	int func_backup;
#endif
	char bFWReady;
//#ifdef HS2_SUPPORT
    /* Hotspot 2.0 Release 1 */
	unsigned char			timeadvt_dtimcount;
	unsigned int			dgaf_disable;
	unsigned int			OSU_Present;
	unsigned int			proxy_arp;
//#endif


#ifdef CONFIG_IEEE80211W_CLI	// all ap's cap
	unsigned char 			bss_support_sha256;
	unsigned char 			bss_support_pmf;
	unsigned int 			bss_support_akmp;
#endif
#ifdef CONFIG_RTL_WLAN_STATUS
	int wlan_status_flag;
#endif
	unsigned char take_over_hidden;
	unsigned char hiddenAP_backup;
#ifdef ERR_ACCESS_CNTR
	struct err_access_list	err_ac_list[MAX_ERR_ACCESS_CNTR];
#endif

	struct propietaryIE *pptyIE;
#ifdef THERMAL_PROTECTION
        unsigned int thermal_hit_counter;//THERMAL_HIT_COUNTER
        unsigned int thermal_fireoff;
        unsigned int read_flag;
#endif
#ifdef USER_ADDIE
	struct user_ie 			user_ie_list[MAX_USER_IE];
#endif
#ifdef CONFIG_AUTH_RESULT
	unsigned int authRes;
#endif
#ifdef CONFIG_RTK_BTCONFIG
	unsigned int btconfig_timeout;
#endif
} RTL8192CD_PRIV, *PRTL8192CD_PRIV;

struct rtl8192cd_chr_priv {
	unsigned int			major;
	unsigned int			minor;
	struct rtl8192cd_priv*	wlan_priv;
	struct fasync_struct*	asoc_fasync;	// asynch notification
};

struct rtl8192cd_wds_priv {
	struct rtl8192cd_priv*	wlan_priv;      //This element shall be put at top of this struct
};

/* station info, reported to web server */
typedef struct _sta_info_2_web {
	unsigned short	aid;
	unsigned char	addr[6];
	unsigned long	tx_packets;
	unsigned long	rx_packets;
	unsigned long	expired_time;	// 10 msec unit
	unsigned short	flags;
	unsigned char	TxOperaRate;
	unsigned char	rssi;
	unsigned long	link_time;		// 1 sec unit
	unsigned long	tx_fail;
	unsigned long	tx_bytes;
	unsigned long	rx_bytes;
	unsigned char	network;
	unsigned char	ht_info;		// bit0: 0=20M mode, 1=40M mode; bit1: 0=longGI, 1=shortGI
	unsigned char	RxOperaRate;
#ifdef TLN_STATS
	unsigned char	auth_type;
	unsigned char	enc_type;
	unsigned char 	resv[1];
#else
	unsigned char 	resv[3];
#endif
	unsigned short	acTxOperaRate;
} sta_info_2_web;

#define NULL_MAC_ADDR		("\x0\x0\x0\x0\x0\x0")

// Macros
#define GET_MIB(priv)		(priv->pmib)
#define GET_HW(priv)		(priv->pshare->phw)

#define AP_BSSRATE			((GET_MIB(priv))->dot11StationConfigEntry.dot11OperationalRateSet)
#define AP_BSSRATE_LEN		((GET_MIB(priv))->dot11StationConfigEntry.dot11OperationalRateSetLen)

#ifdef RTK_AC_SUPPORT  //vht rate
#define AP_BSSRATE_AC			(priv->pshare->rf_ft_var.dot11OperationalRateSet_AC)
#define AP_BSSRATE_LEN_AC		(priv->pshare->rf_ft_var.dot11OperationalRateSetLen_AC)
#define AC_SIGMA_MODE			(priv->pshare->rf_ft_var.sigma_mode)
#endif


#define STAT_OPRATE			(pstat->bssrateset)
#define STAT_OPRATE_LEN		(pstat->bssratelen)

#define BSSID			((GET_MIB(priv))->dot11StationConfigEntry.dot11Bssid)

#define SSID			((GET_MIB(priv))->dot11StationConfigEntry.dot11DesiredSSID)

#define SSID_LEN		((GET_MIB(priv))->dot11StationConfigEntry.dot11DesiredSSIDLen)

#define ACTIVE_ID 		(0)
#define OPMODE_VXD		((GET_MIB((GET_VXD_PRIV(priv))))->dot11OperationEntry.opmode)
#define OPMODE			((GET_MIB(priv))->dot11OperationEntry.opmode)
#define OPMODE_VAL(mod) do {\
		(GET_MIB(priv))->dot11OperationEntry.opmode = mod;\
		} while (0)

#ifdef CAM_SWAP
#define TRAFFIC_THRESHOLD_LOW	((priv->pshare->rf_ft_var.cam_rotation)*(priv->pshare->rf_ft_var.thrd_low))
#define TRAFFIC_THRESHOLD_MID	((priv->pshare->rf_ft_var.cam_rotation)*(priv->pshare->rf_ft_var.thrd_mid))
#define TRAFFIC_THRESHOLD_HIGH	((priv->pshare->rf_ft_var.cam_rotation)*(priv->pshare->rf_ft_var.thrd_high))
#endif

#define TAKEOVER_HIDDEN_AP		(priv->take_over_hidden)

#define HIDDEN_AP		((GET_MIB(priv))->dot11OperationEntry.hiddenAP)

#define RTSTHRSLD		((GET_MIB(priv))->dot11OperationEntry.dot11RTSThreshold)

#define FRAGTHRSLD		((GET_MIB(priv))->dot11OperationEntry.dot11FragmentationThreshold)

#define EXPIRETIME		((GET_MIB(priv))->dot11OperationEntry.expiretime)

#define LED_TYPE		((GET_MIB(priv))->dot11OperationEntry.ledtype)

#ifdef RTL8190_SWGPIO_LED
#define LED_ROUTE		((GET_MIB(priv))->dot11OperationEntry.ledroute)
#endif

#define IAPP_ENABLE		((GET_MIB(priv))->dot11OperationEntry.iapp_enable)

#define SWCRYPTO		((GET_MIB(priv))->dot11StationConfigEntry.dot11swcrypto)

#define IEEE8021X_FUN	((GET_MIB(priv))->dot118021xAuthEntry.dot118021xAlgrthm)

#define ACCT_FUN		((GET_MIB(priv))->dot118021xAuthEntry.acct_enabled)

#define ACCT_FUN_TIME	((GET_MIB(priv))->dot118021xAuthEntry.acct_timeout_period)

#define ACCT_FUN_TP		((GET_MIB(priv))->dot118021xAuthEntry.acct_timeout_throughput)

#define ACCT_TP_INT		60

#define SHORTPREAMBLE	((GET_MIB(priv))->dot11RFEntry.shortpreamble)

#define SSID2SCAN		((GET_MIB(priv))->dot11StationConfigEntry.dot11SSIDtoScan)

#define SSID2SCAN_LEN	((GET_MIB(priv))->dot11StationConfigEntry.dot11SSIDtoScanLen)

#define RX_BUF_LEN		((GET_MIB(priv))->dot11nConfigEntry.dot11nAMSDURecvMax?((priv->pshare->wlandev_idx == (0^WLANIDX))?(MAX_RX_BUF_LEN):(MAX_RX_BUF_LEN_1)):((priv->pshare->wlandev_idx == (0^WLANIDX))?(MIN_RX_BUF_LEN):(MIN_RX_BUF_LEN_1)))
#define RX_DESC_NUM		((priv->pshare->wlandev_idx == (0^WLANIDX))?(NUM_RX_DESC):(NUM_RX_DESC_2G))
#define RX_MAX_SKB_NUM		((priv->pshare->wlandev_idx == (0^WLANIDX))?(MAX_SKB_NUM):(MAX_SKB_NUM_1))



#ifdef WIFI_WMM
#define QOS_ENABLE 		((GET_MIB(priv))->dot11QosEntry.dot11QosEnable)

#define APSD_ENABLE 	((GET_MIB(priv))->dot11QosEntry.dot11QosAPSD)

#define GET_WMM_IE		((GET_MIB(priv))->dot11QosEntry.WMM_IE)

#define GET_WMM_PARA_IE		((GET_MIB(priv))->dot11QosEntry.WMM_PARA_IE)

#define GET_EDCA_PARA_UPDATE 	((GET_MIB(priv))->dot11QosEntry.EDCAparaUpdateCount)

#define GET_STA_AC_BE_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_BE_paraRecord)

#define GET_STA_AC_BK_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_BK_paraRecord)

#define GET_STA_AC_VI_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_VI_paraRecord)

#define GET_STA_AC_VO_PARA	((GET_MIB(priv))->dot11QosEntry.STA_AC_VO_paraRecord)
#if defined(UNIVERSAL_REPEATER) && defined(WIFI_11N_2040_PERMIT_LOGIC)
#define COEXIST_ENABLE		(((IS_DRV_OPEN(GET_VXD_PRIV(GET_ROOT(priv)))) && ((GET_VXD_PRIV(GET_ROOT(priv))->pmib->dot11OperationEntry.opmode & (WIFI_STATION_STATE|WIFI_ASOC_STATE|WIFI_AUTH_SUCCESS)) == (WIFI_STATION_STATE|WIFI_ASOC_STATE|WIFI_AUTH_SUCCESS)))? 0 :(GET_MIB(priv)->dot11nConfigEntry.dot11nCoexist))
#else
#define COEXIST_ENABLE		((GET_MIB(priv))->dot11nConfigEntry.dot11nCoexist)
#endif
#endif

#define COUNTRY_CODE_ENABLED 	((GET_MIB(priv))->dot11dCountry.dot11CountryCodeSwitch)



#define AMPDU_ENABLE	((GET_MIB(priv))->dot11nConfigEntry.dot11nAMPDU)
#define AMSDU_ENABLE	((GET_MIB(priv))->dot11nConfigEntry.dot11nAMSDU)

#define TSF_LESS(a, b)	(((a - b) & 0x80000000) != 0)
#define TSF_DIFF(a, b)	((a >= b)? (a - b):(0xffffffff - b + a + 1))

#define GET_GROUP_MIC_KEYLEN	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKeyLen)
#define GET_GROUP_IDX2_MIC_KEYLEN	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey2.dot11TMicKeyLen)
#define GET_GROUP_TKIP_MIC1_KEY	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey1.skey)
#define GET_GROUP_TKIP_MIC2_KEY	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey2.skey)
#define GET_GROUP_TKIP_IDX2_MIC1_KEY	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey2.dot11TMicKey1.skey)
#define GET_GROUP_TKIP_IDX2_MIC2_KEY	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey2.dot11TMicKey2.skey)


#define GET_UNICAST_MIC_KEYLEN		(pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKeyLen)
#define GET_UNICAST_TKIP_MIC1_KEY	(pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey1.skey)
#define GET_UNICAST_TKIP_MIC2_KEY	(pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey2.skey)

#define GET_GROUP_ENCRYP_KEY		((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey)
#define GET_GROUP_ENCRYP2_KEY		((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey2.dot11TTKey.skey)

#define GET_UNICAST_ENCRYP_KEY		(pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey)
#ifdef CONFIG_IEEE80211W
#define GET_IGROUP_ENCRYP_KEY		((GET_MIB(priv))->dot11IGTKTable.dot11EncryptKey.dot11TTKey.skey)
#endif

#define GET_GROUP_ENCRYP_KEYLEN			((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen)
#define GET_GROUP_IDX2_ENCRYP_KEYLEN	((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey2.dot11TTKeyLen)

#define GET_UNICAST_ENCRYP_KEYLEN		(pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen)

#define GET_MY_HWADDR		((GET_MIB(priv))->dot11OperationEntry.hwaddr)
#define AUTH_SEQ			(priv->auth_seq)
#define AUTH_SEQ_VAL(res)	(priv->auth_seq=res)
#define REAUTH_COUNT		(priv->reauth_count)
#define REAUTH_COUNT_VAL(res) (priv->reauth_count=res)
#define REASSOC_COUNT		(priv->reassoc_count)
#define REASSOC_COUNT_VAL(res) (priv->reassoc_count=res)
#define AUTH_MODE_TOGGLE	(priv->authModeToggle)
#define AUTH_MODE_TOGGLE_VAL(res) (priv->authModeToggle=res)
#define AUTH_MODE_RETRY		(priv->authModeRetry)
#define AUTH_MODE_RETRY_VAL(res) (priv->authModeRetry=res)
#define JOIN_RES			(priv->join_res)
#define JOIN_RES_VAL(res)	(priv->join_res=res)
#define JOIN_REQ_ONGOING	(priv->join_req_ongoing)
#define JOIN_REQ_ONGOING_VAL(res) (priv->join_req_ongoing=res)
#define CHG_TXT				(priv->chg_txt)
#define _AID				(priv->aid)
#define AID_VAL(res)		(priv->aid=res)
#define PENDING_REAUTH_TIMER	(timer_pending(&priv->reauth_timer))
#define DELETE_REAUTH_TIMER	(del_timer(&priv->reauth_timer))
#define PENDING_REASSOC_TIMER	(timer_pending(&priv->reassoc_timer))
#define DELETE_REASSOC_TIMER	(del_timer(&priv->reassoc_timer))
#define MOD_REAUTH_TIMER(t)	(mod_timer(&priv->reauth_timer, jiffies+t))
#define MOD_REASSOC_TIMER(t)	(mod_timer(&priv->reassoc_timer, jiffies+t))

#define GET_GROUP_ENCRYP_PN			(&((GET_MIB(priv))->dot11GroupKeysTable.dot11EncryptKey.dot11TXPN48))
#define GET_UNICAST_ENCRYP_PN		(&(pstat->dot11KeyMapping.dot11EncryptKey.dot11TXPN48))
#ifdef CONFIG_IEEE80211W
#define GET_UNICAST_ENCRYP_MPN		(&(pstat->dot11KeyMapping.dot11EncryptKey.dot11MTXPN48))
#define GET_IGROUP_ENCRYP_PN			(&((GET_MIB(priv))->dot11IGTKTable.dot11EncryptKey.dot11TXPN48))
#endif

#define SET_SHORTSLOT_IN_BEACON_CAP		\
	do {	\
		if (priv->pBeaconCapability != NULL)	\
			*priv->pBeaconCapability |= cpu_to_le16(BIT(10));	\
	} while(0)

#define RESET_SHORTSLOT_IN_BEACON_CAP	\
	do {	\
		if (priv->pBeaconCapability != NULL)	\
			*priv->pBeaconCapability &= ~cpu_to_le16(BIT(10));	\
	} while(0)

#define IS_DRV_OPEN(priv) ((priv==NULL) ? 0 : ((priv->drv_state & DRV_STATE_OPEN) ? 1 : 0))

	#define GET_DEV_PRIV(dev) ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv

	#define PROC_GET_DEV() (struct net_device *)(s->private)


//#define GET_ROOT_PRIV(priv)			(priv->proot_priv)
#define IS_ROOT_INTERFACE(priv)	 	((priv->proot_priv) ? 0 : 1)
#define GET_ROOT(priv)				((priv->proot_priv) ? priv->proot_priv : priv)
#define GET_VXD_PRIV(priv)			(GET_ROOT(priv)->pvxd_priv)
#define IS_VXD_INTERFACE(priv)		(((priv->proot_priv) ? 1 : 0) && (priv->vap_id < 0))
#define GET_VAP_PRIV(priv, i)		(GET_ROOT(priv)->pvap_priv[i])
#define IS_VAP_INTERFACE(priv)		(!IS_ROOT_INTERFACE(priv) && (priv->vap_id >= 0))

#define MANAGE_QUE_NUM		MGNT_QUEUE

#define DFS_TO					RTL_10MILISECONDS_TO_JIFFIES(priv->pmib->dot11DFSEntry.DFS_timeout)
#define NONE_OCCUPANCY_PERIOD	RTL_10MILISECONDS_TO_JIFFIES(priv->pmib->dot11DFSEntry.NOP_timeout)
#define DFS_TXPAUSE_TO			RTL_10MILISECONDS_TO_JIFFIES(priv->pmib->dot11DFSEntry.DFS_TXPAUSE_timeout)
#define IS_METEOROLOGY_CHANNEL(_ch_)	\
	( (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80 && (_ch_) >= 116 && (_ch_) <= 128) || \
	  (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40 && (_ch_) >= 116 && (_ch_) <= 128) || \
	  (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20 && (_ch_) >= 120 && (_ch_) <= 128) )

#ifdef TX_EARLY_MODE
#define GET_TX_EARLY_MODE			(priv->pshare->rf_ft_var.em_enable)
#define GET_EM_SWQ_ENABLE			(priv->pshare->em_waitq_on)
#define MAX_EM_QUE_NUM				(priv->pshare->rf_ft_var.em_que_num)
#define EM_TP_UP_BOUND				(priv->pshare->rf_ft_var.em_swq_thd_high)
#define EM_TP_LOW_BOUND				(priv->pshare->rf_ft_var.em_swq_thd_low)
#define WAIT_TP_TIME				3	/* wait TP limit for this period in sec */
#endif

#ifdef CONFIG_IEEE80211V
#define WNM_ENABLE				(priv->pmib->wnmEntry.dot11vBssTransEnable)
#define EVENT_TIMEOUT			(priv->pmib->wnmEntry.bssTransExpiredTime)
#endif




#ifdef _DEBUG_RTL8192CD_
#define ASSERT(expr) \
        if(!(expr)) {					\
  			printk( "\033[33;41m%s:%d: assert(%s)\033[m\n",	\
	        __FILE__,__LINE__,#expr);		\
        }
#else
#define ASSERT(expr)
#endif

#define	ODMPTR					(&(priv->pshare->_dmODM))

#ifdef RTL8190_SWGPIO_LED
/* =====================================================================
		LED route configuration:

			Currently, LOW 10 bits of this MIB are used.

			+---+---+---+---+---+---+---+---+---+---+
			| E1| H1|  Route 1  | E0| H0|  Route 0  |
			+---+---+---+---+---+---+---+---+---+---+

			E0		: Indicates if the field route 0 is valid or not.
			E1		: Indicates if the field route 1 is valid or not.
			H0		: Indicate the GPIO indicated by route 0 is Active HIGH or Active LOW. ( 0: Active LOW, 1: Active HIGH)
			H1		: Indicate the GPIO indicated by route 1 is Active HIGH or Active LOW. ( 0: Active LOW, 1: Active HIGH)
			Route0	: The GPIO number (0~6) which used by LED0. Only used when E0=0b'1
			Route1	: The GPIO number (0~6) which used by LED1. Only used when E1=0b'1

			Unused bits	: reserved for further extension, must set to 0.

			Currently RTL8185 AP driver supports LED0/LED1, and RTL8185 has 7 GPIOs in it.
			So we need a routing-mechanism to decide what GPIO is used for LED0/LED1.
			The bit-field pairs {E0, H0, Route0}, {E1, H0, Route1} is used to set it.
			Ex.
				One customer only use GPIO0 for LED1 (Active LOW) and don't need LED0,
				he can set the route being:
				----------------------------------
				E0 = 0
				E1 = 1
				H0 = 0 ( Driver would ignore it )
				H1 = 0  ( Driver would ignore it )
				Route 0 = 0 ( Driver would ignore it )
				Route 1 = 0 ( GPIO0 )

				ledroute = 0x10 << 5;		: LED1 -Active LOW, GPIO0
				ledroute |= 0;				: LED0 -Disabled
				----------------------------------
     ===================================================================== */
#define	SWLED_GPIORT_CNT			2					/* totally we have max 3 GPIOs reserved for LED route usage */
#define	SWLED_GPIORT_RTBITMSK		0x07				/* bit mask of routing field = 0b'111 */
#define	SWLED_GPIORT_HLMSK			0x08				/* bit mask of Active high/low field = 0b'1000 */
#define	SWLED_GPIORT_ENABLEMSK		0x10				/* bit mask of enable filed = 0b'10000 */
#define	SWLED_GPIORT_ITEMBITCNT		5					/* total bit count of each item */
#define	SWLED_GPIORT_ITEMBITMASK	(	SWLED_GPIORT_RTBITMSK |\
										SWLED_GPIORT_HLMSK |\
										SWLED_GPIORT_ENABLEMSK)	/* bit mask of each item */
#define	SWLED_GPIORT_ITEM(ledroute, x)	(((ledroute) >> ((x)*SWLED_GPIORT_ITEMBITCNT)) & SWLED_GPIORT_ITEMBITMASK)
#endif // RTL8190_SWGPIO_LED

#ifdef SUPPORT_SNMP_MIB
#define SNMP_MIB(f)					(priv->snmp_mib.f)
#define SNMP_MIB_ASSIGN(f,v)		(SNMP_MIB(f)=v)
#define SNMP_MIB_COPY(f,v,len)		(memcpy(&SNMP_MIB(f), v, len))
#define SNMP_MIB_INC(f,v)			(SNMP_MIB(f)+=v)
#define SNMP_MIB_DEC(f,v)			(SNMP_MIB(f)-=v)

#else

#define SNMP_MIB(f)
#define SNMP_MIB_ASSIGN(f,v)
#define SNMP_MIB_COPY(f,v,len)
#define SNMP_MIB_INC(f,v)
#define SNMP_MIB_DEC(f,v)
#endif //SUPPORT_SNMP_MIB


#ifdef USB_PKT_RATE_CTRL_SUPPORT
typedef unsigned int (*usb_pktCnt_fn)(void);
typedef unsigned int (*register_usb_pkt_cnt_fn)(void *);
extern register_usb_pkt_cnt_fn register_usb_hook;
#endif


#ifdef CONFIG_RTK_VLAN_SUPPORT
#if defined(CONFIG_RTK_BRIDGE_VLAN_SUPPORT) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
extern int rx_vlan_process(struct net_device *dev, struct VlanConfig *info_ori, struct sk_buff *skb, struct sk_buff **new_skb);
extern int tx_vlan_process(struct net_device *dev, struct VlanConfig *info_ori, struct sk_buff *skb, int wlan_pri);
#else
extern int  rx_vlan_process(struct net_device *dev, struct VlanConfig *info, struct sk_buff *skb);
extern int  tx_vlan_process(struct net_device *dev, struct VlanConfig *info, struct sk_buff *skb, int wlan_pri);
#endif
#endif

#if defined(CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT)
extern int rtl_process_vlan_passthrough_tx(struct sk_buff **ppskb);
#endif


//#ifdef  SUPPORT_TX_MCAST2UNI
#define IP_MCAST_MAC(mac)		((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
#define IPV6_MCAST_MAC(mac)	((mac[0]==0x33)&&(mac[1]==0x33))
#define IS_UDP_PROTO(mac)	((mac[12]==0x08) && (mac[13]==0x00) && (mac[23]==0x11))
/*match is  (1)ipv4 && (2)(IGMP control/management packet) */
#define IS_IGMP_PROTO(mac)	((mac[12]==0x08) && (mac[13]==0x00) && (mac[23]==0x02))
/* for Hotspot 2.0 Release 1 */
#define IS_ICMPV4_PROTO(mac) ((mac[12]==0x08) && (mac[13]==0x00) && (mac[23]==0x01))
#define IS_ICMPV4_ECHO_TYPE(mac) (mac[34]==0x08 || mac[34]==0x00)

#define IS_ICMPV6_PROTO(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd) && ((mac[20]==0x3a)||(mac[54]==0x3a)))
#define IS_MDNSV4_MAC(mac) ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e)&& (mac[3]==0x00)&&(mac[4]==0x00)&&(mac[5]==0xFB)&&(mac[12]==0x08)&&(mac[13]==0x00))
#define IS_MDNSV6_MAC(mac) ((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]==0x00)&& (mac[3]==0x00)&&(mac[4]==0x00)&&(mac[5]==0xFB)&&(mac[12]==0x86)&&(mac[13]==0xdd))

//#ifdef	TX_SUPPORT_IPV6_MCAST2UNI
#define ICMPV6_MCAST_MAC(mac)	((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]!=0xff))
/* for Hotspot 2.0 Release 1 */
#define ICMPV6_MCAST_SOLI_MAC(mac)	((mac[0]==0x33)&&(mac[1]==0x33)&&(mac[2]==0xff))

#define ICMPV6_PROTO1A_VALN(mac)		( (mac[12+4]==0x86)&&(mac[13+4]==0xdd)&& (mac[54+4]==0x3a))
#define ICMPV6_PROTO1B_VALN(mac)		( (mac[12+4]==0x86)&&(mac[13+4]==0xdd)&& (mac[20+4]==0x3a))
#define ICMPV6_PROTO2X_VALN(mac)		( (mac[12+4]==0x86)&&(mac[13+4]==0xdd)&& (mac[54+4]==0x3a || mac[20+4]==0x3a))

#define ICMPV6_PROTO1A(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd)&& (mac[54]==0x3a))
#define ICMPV6_PROTO1B(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd)&& (mac[20]==0x3a))
#define ICMPV6_PROTO2X(mac)		( (mac[12]==0x86)&&(mac[13]==0xdd)&& (mac[54]==0x3a || mac[20]==0x3a))

//#endif
//#endif

#if defined(TXREPORT) //&& (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
struct tx_rpt {
	unsigned short		txfail;
	unsigned short      txok;
	unsigned short      macid;
	unsigned char     	initil_tx_rate;	//add for 8812
};
#endif

#if 1 //eric-gid struct
struct ccx_rpt {
	unsigned short		txfail;
	unsigned short      txok;
	unsigned short      macid;
	unsigned char     	initil_tx_rate;	//add for 8812
};
#endif

#ifdef TXRETRY_CNT
struct tx_retry {
	unsigned char		stat_idx;
	unsigned short      retry_pkt_macid[2];
	unsigned short      retry_cnt_macid[2];
};
#endif

struct _device_info_ {
	int type;
	unsigned long conf_addr;
	unsigned long base_addr;
	int irq;
	struct rtl8192cd_priv *priv;
};

#define HIDE_AP_FOUND			1
#define HIDE_AP_FOUND_DO_ACTIVE_SSAN	2


#ifdef GBWC
#define GBWC_MODE_DISABLE			0
#define GBWC_MODE_LIMIT_MAC_INNER	1 // limit bw by mac address
#define GBWC_MODE_LIMIT_MAC_OUTTER	2 // limit bw by excluding the mac
#define GBWC_MODE_LIMIT_IF_TX		3 // limit bw by interface tx
#define GBWC_MODE_LIMIT_IF_RX		4 // limit bw by interface rx
#define GBWC_MODE_LIMIT_IF_TRX		5 // limit bw by interface tx/rx
#endif


// andrew, define a compatible data macro
#define SKB_MAC_HEADER(s) skb_mac_header(s)
#define SKB_IP_HEADER(s) (struct iphdr *)(SKB_MAC_HEADER(s) + ETH_HLEN);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35)
#define GET_BR_PORT(netdev)	(netdev)->br_port
#else
#define GET_BR_PORT(netdev)	br_port_get_rcu(netdev)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
static inline void skb_reset_network_header(struct sk_buff *skb)
{
        skb->nh.raw = skb->data;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#include <linux/if_arp.h>
static inline int arp_hdr_len(struct net_device *dev)
{
        /* ARP header, plus 2 device addresses, plus 2 IP addresses. */
        return sizeof(struct arphdr) + (dev->addr_len + sizeof(u32)) * 2;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
static inline int dev_hard_header(struct sk_buff *skb, struct net_device *dev,
                                  unsigned short type,
                                  void *daddr, void *saddr,
                                  unsigned len)
{
        return dev->hard_header(skb, dev, type, daddr, saddr, len);
}
#endif

#if (BEAMFORMING_SUPPORT == 1)

#define	IsCtrlNDPA(pdu)			( ((EF1Byte(pdu[0]) & 0xFC) == Type_NDPA) ? TRUE : FALSE)
#define	IsMgntActionNoAck(pdu)	( ((EF1Byte(pdu[0]) & 0xFC) == Type_Action_No_Ack ) ? TRUE : FALSE)
#define GET_80211_HDR_ORDER(_hdr)					LE_BITS_TO_2BYTE(_hdr, 15, 1)

enum _TXBF_TXPWRLMT_ {
	TXBF_TXPWRLMT_DISABLE	= 0,
	TXBF_TXPWRLMT_ENABLE	= 1,
	TXBF_TXPWRLMT_AUTO		= 2,
};
#endif


#include "WlanHAL/Output/HalLib.h"

//#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
enum _HW_TX_SHORTCUT_ {
	// for header conversion ( 802.3 -> 802.11 )
	HW_TX_SC_NORMAL 		= 0,
	HW_TX_SC_BACKUP_HEADER 	= 1,
	HW_TX_SC_HEADER_CONV 	= 2,
};
//#endif



#define RESCAN  1
#define DONTRESCAN 0

#define RESCAN_ROAMING  2

#define RESCAN_BY_NEXTTIME 0

enum {
	SSFROM_WEB = 1,
	SSFROM_WSC,
	SSFROM_REPEATER_VXD,
	SSFROM_11K_BEACONREQ,
	SSFROM_WPAS
};

/*sorting by profile*/
enum {
	COMPARE_BSS = 0,
	COMPARE_WSCIE = 1,
	COMPARE_WPAIE = 2,
	COMPARE_RSNIE = 3
};

enum {
	SS_LV_WSTA = 0,
	SS_LV_WOSTA = 1,
	SS_LV_ROOTFUNCOFF = 2
};


#define CHECK_VXD_AP_TIMEOUT            RTL_SECONDS_TO_JIFFIES(60)
#define CHECK_VXD_RUN_DELAY             RTL_SECONDS_TO_JIFFIES(15)
#define CHECK_VXD_24G_AP_NOSTA_TIMEOUT  RTL_SECONDS_TO_JIFFIES(15)
#define CHECK_VXD_5G_AP_NOSTA_TIMEOUT   RTL_SECONDS_TO_JIFFIES(30)

struct brsc_cache_t {
	int occupy;
	unsigned long timestamp;
	unsigned char cached_br_sta_mac[MACADDRLEN];
	struct net_device *cached_br_sta_dev;
};




#if defined(RESERVE_TXDESC_FOR_EACH_IF) || defined(USE_TXQUEUE)
#define RSVQ(_q)				((_q)-BK_QUEUE)
#define IS_RSVQ(_q)			((_q)>=BK_QUEUE && (_q)<=VO_QUEUE)
#endif
#ifdef RESERVE_TXDESC_FOR_EACH_IF
#define RSVQ_ENABLE				GET_ROOT(priv)->pmib->miscEntry.rsv_txdesc
#endif
#ifdef USE_TXQUEUE
#define BUFQ_ENABLE				GET_ROOT(priv)->pmib->miscEntry.use_txq
#endif

#endif // _8192CD_H_

