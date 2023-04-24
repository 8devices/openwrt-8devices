/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/


#ifndef	__HALDMOUTSRC_H__
#define __HALDMOUTSRC_H__

//============================================================
// include files
//============================================================
#include "phydm_pre_define.h"
#include "phydm_dig.h"
#include "phydm_edcaturbocheck.h"
#include "phydm_pathdiv.h"
#include "phydm_antdiv.h"
#include "phydm_antdect.h"
#include "phydm_soml.h"
#include "phydm_dynamicbbpowersaving.h"
#include "phydm_rainfo.h"
#include "phydm_dynamictxpower.h"
#include "phydm_cfotracking.h"
#include "phydm_acs.h"
#include "phydm_adaptivity.h"
#include "phydm_iqk.h"
#include "phydm_dfs.h"
#include "phydm_ccx.h"
#include "txbf/phydm_hal_txbf_api.h"
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
#include "phydm_beamforming.h"
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
#include "halphyrf_ap.h"
#include "phydm_adc_sampling.h"
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
#include "halphyrf_ce.h"
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN)) 
#include "halphyrf_win.h"
#endif
#include "phydm_noisemonitor.h"

//============================================================
// Definition 
//============================================================
//
// 2011/09/22 MH Define all team supprt ability.
//

/* Traffic load decision */
#define	TRAFFIC_ULTRA_LOW	1
#define	TRAFFIC_LOW			2
#define	TRAFFIC_MID			3
#define	TRAFFIC_HIGH			4

#define	NONE			0

/*NBI API------------------------------------*/
#define	NBI_ENABLE 1
#define	NBI_DISABLE 2

#define	NBI_TABLE_SIZE_128	27
#define	NBI_TABLE_SIZE_256	59

#define	NUM_START_CH_80M	7
#define	NUM_START_CH_40M	14

#define	CH_OFFSET_40M		2
#define	CH_OFFSET_80M		6

/*CSI MASK API------------------------------------*/
#define	CSI_MASK_ENABLE 1
#define	CSI_MASK_DISABLE 2

/*------------------------------------------------*/

#define	FFT_128_TYPE	1
#define	FFT_256_TYPE	2

#define	SET_SUCCESS	1
#define	SET_ERROR		2
#define	SET_NO_NEED	3

#define	FREQ_POSITIVE	1
#define	FREQ_NEGATIVE	2


#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
#define PHYDM_WATCH_DOG_PERIOD	1
#else
#define PHYDM_WATCH_DOG_PERIOD	2
#endif

#define MAX_2(_x_, _y_)	(((_x_) > (_y_)) ? (_x_) : (_y_))
#define MIN_2(_x_, _y_)	(((_x_) < (_y_)) ? (_x_) : (_y_))
#define DIFF_2(_x_, _y_)	((_x_ >= _y_) ? (_x_ - _y_) : (_y_ - _x_))

#define IS_GREATER(_x_, _y_)	(((_x_) >= (_y_)) ? true : false)
#define IS_LESS(_x_, _y_)	(((_x_) < (_y_)) ? true : false)

#define BYTE_2_DWORD(B3, B2, B1, B0)	((B3 << 24) | (B2 << 16) | (B1 << 8) | B0)
#define BIT_2_BYTE(B3, B2, B1, B0)	((B3 << 3) | (B2 << 2) | (B1 << 1) | B0)

//============================================================
// structure and define
//============================================================

//
// 2011/09/20 MH Add for AP/ADSLpseudo DM structuer requirement.
// We need to remove to other position???
//
#if(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
typedef		struct rtl8192cd_priv {
	u1Byte		temp;

}rtl8192cd_priv, *prtl8192cd_priv;
#endif


#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
typedef		struct _ADAPTER{
	u1Byte		temp;
	#ifdef AP_BUILD_WORKAROUND
	HAL_DATA_TYPE*		temp2;
	prtl8192cd_priv		priv;
	#endif
}ADAPTER, *PADAPTER;
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)

typedef		struct _WLAN_STA{
	u1Byte		temp;
} WLAN_STA, *PRT_WLAN_STA;

#endif

typedef struct _Dynamic_Primary_CCA{
	u1Byte		PriCCA_flag;
	u1Byte		intf_flag;
	u1Byte		intf_type;  
	u1Byte		DupRTS_flag;
	u1Byte		Monitor_flag;
	u1Byte		CH_offset;
	u1Byte  	MF_state;
}Pri_CCA_T, *pPri_CCA_T;


#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	#ifdef ADSL_AP_BUILD_WORKAROUND
	#define MAX_TOLERANCE			5
	#define IQK_DELAY_TIME			1		/*ms*/
	#endif
#endif	/*#if(DM_ODM_SUPPORT_TYPE & (ODM_AP))*/

#define		DM_Type_ByFW			0
#define		DM_Type_ByDriver		1

//
// Declare for common info
//

#define IQK_THRESHOLD			8
#define DPK_THRESHOLD			4


#if (DM_ODM_SUPPORT_TYPE &  (ODM_AP))
__PACK typedef struct _ODM_Phy_Status_Info_
{
	u1Byte		RxPWDBAll;
	u1Byte		SignalQuality;					/* in 0-100 index. */
	u1Byte		RxMIMOSignalStrength[4];		/* in 0~100 index */
	s1Byte		RxMIMOSignalQuality[4];		/* EVM */
	u1Byte		RxMIMOEVMdbm[4];			/* per-path's EVM dbm */
	s1Byte		RxSNR[4];					/* per-path's SNR */
#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	u1Byte		RxCount:2;					/* RX path counter---*/
	u1Byte		BandWidth:2;
	u1Byte		rxsc:4;						/* sub-channel---*/
#else
	u1Byte		BandWidth;
#endif
#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	u1Byte		channel;						/* channel number---*/
	BOOLEAN		bMuPacket;					/* is MU packet or not---*/
	BOOLEAN		bBeamformed;				/* BF packet---*/
#endif
} __WLAN_ATTRIB_PACK__ ODM_PHY_INFO_T, *PODM_PHY_INFO_T;

typedef struct _ODM_Phy_Status_Info_Append_
{
	u1Byte		MAC_CRC32;	

}ODM_PHY_INFO_Append_T,*PODM_PHY_INFO_Append_T;

#else

typedef struct _ODM_Phy_Status_Info_
{
	//
	// Be care, if you want to add any element please insert between 
	// RxPWDBAll & SignalStrength.
	//
#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN))
	u4Byte		RxPWDBAll;	
#else
	u1Byte		RxPWDBAll;	
#endif
	u1Byte		SignalQuality;				/* in 0-100 index. */
	s1Byte		RxMIMOSignalQuality[4];		/* per-path's EVM */
	u1Byte		RxMIMOEVMdbm[4];			/* per-path's EVM dbm */
	u1Byte		RxMIMOSignalStrength[4];	/* in 0~100 index */
	s2Byte		Cfo_short[4];				/* per-path's Cfo_short */
	s2Byte		Cfo_tail[4];					/* per-path's Cfo_tail */
	s1Byte		RxPower;					/* in dBm Translate from PWdB */
	s1Byte		RecvSignalPower;			/* Real power in dBm for this packet, no beautification and aggregation. Keep this raw info to be used for the other procedures. */
	u1Byte		BTRxRSSIPercentage;
	u1Byte		SignalStrength;				/* in 0-100 index. */
	s1Byte		RxPwr[4];					/* per-path's pwdb */
	s1Byte		RxSNR[4];					/* per-path's SNR	*/
#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	u1Byte		RxCount:2;					/* RX path counter---*/
	u1Byte		BandWidth:2;
	u1Byte		rxsc:4;						/* sub-channel---*/
#else
	u1Byte		BandWidth;
#endif
#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))
	u1Byte		btCoexPwrAdjust;
#endif
#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	u1Byte		channel;						/* channel number---*/
	BOOLEAN		bMuPacket;					/* is MU packet or not---*/
	BOOLEAN		bBeamformed;				/* BF packet---*/
#endif
}ODM_PHY_INFO_T,*PODM_PHY_INFO_T;
#endif

typedef struct _ODM_Per_Pkt_Info_
{
	//u1Byte		Rate;	
	u1Byte		DataRate;
	u1Byte		StationID;
	BOOLEAN		bPacketMatchBSSID;
	BOOLEAN		bPacketToSelf;
	BOOLEAN		bPacketBeacon;
	BOOLEAN		bToSelf;
}ODM_PACKET_INFO_T,*PODM_PACKET_INFO_T;


typedef struct _ODM_Phy_Dbg_Info_
{
	//ODM Write,debug info
	s1Byte		RxSNRdB[4];
	u4Byte		NumQryPhyStatus;
	u4Byte		NumQryPhyStatusCCK;
	u4Byte		NumQryPhyStatusOFDM;
#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	u4Byte		NumQryMuPkt;
	u4Byte		NumQryBfPkt;
	u4Byte		NumQryMuVhtPkt[40];
	u4Byte		NumQryVhtPkt[40];
#endif
	u1Byte		NumQryBeaconPkt;
	//Others
	s4Byte		RxEVM[4];	
	
}ODM_PHY_DBG_INFO_T;


typedef struct _ODM_Mac_Status_Info_
{
	u1Byte	test;
	
}ODM_MAC_INFO;

//
// 2011/20/20 MH For MP driver RT_WLAN_STA =  STA_INFO_T
// Please declare below ODM relative info in your STA info structure.
//
#if 1
typedef		struct _ODM_STA_INFO{
	// Driver Write
	BOOLEAN		bUsed;				// record the sta status link or not?
	//u1Byte		WirelessMode;		// 
	u1Byte		IOTPeer;			// Enum value.	HT_IOT_PEER_E

	// ODM Write
	//1 PHY_STATUS_INFO
	u1Byte		RSSI_Path[4];		// 
	u1Byte		RSSI_Ave;
	u1Byte		RXEVM[4];
	u1Byte		RXSNR[4];

	// ODM Write
	//1 TX_INFO (may changed by IC)
	//TX_INFO_T		pTxInfo;				// Define in IC folder. Move lower layer.
#if 0
	u1Byte		ANTSEL_A;			//in Jagar: 4bit; others: 2bit
	u1Byte		ANTSEL_B;			//in Jagar: 4bit; others: 2bit
	u1Byte		ANTSEL_C;			//only in Jagar: 4bit
	u1Byte		ANTSEL_D;			//only in Jagar: 4bit
	u1Byte		TX_ANTL;			//not in Jagar: 2bit
	u1Byte		TX_ANT_HT;			//not in Jagar: 2bit
	u1Byte		TX_ANT_CCK;			//not in Jagar: 2bit
	u1Byte		TXAGC_A;			//not in Jagar: 4bit
	u1Byte		TXAGC_B;			//not in Jagar: 4bit
	u1Byte		TXPWR_OFFSET;		//only in Jagar: 3bit
	u1Byte		TX_ANT;				//only in Jagar: 4bit for TX_ANTL/TX_ANTHT/TX_ANT_CCK
#endif

	//
	// 	Please use compile flag to disabe the strcutrue for other IC except 88E.
	//	Move To lower layer.
	//
	// ODM Write Wilson will handle this part(said by Luke.Lee)
	//TX_RPT_T		pTxRpt;				// Define in IC folder. Move lower layer.
#if 0	
	//1 For 88E RA (don't redefine the naming)
	u1Byte		rate_id;
	u1Byte		rate_SGI;
	u1Byte		rssi_sta_ra;
	u1Byte		SGI_enable;
	u1Byte		Decision_rate;
	u1Byte		Pre_rate;
	u1Byte		Active;

	// Driver write Wilson handle.
	//1 TX_RPT (don't redefine the naming)
	u2Byte		RTY[4];				// ???
	u2Byte		TOTAL;				// ???
	u2Byte		DROP;				// ???
	//
	// Please use compile flag to disabe the strcutrue for other IC except 88E.
	//
#endif

}ODM_STA_INFO_T, *PODM_STA_INFO_T;
#endif

//
// 2011/10/20 MH Define Common info enum for all team.
//
typedef enum _ODM_Common_Info_Definition
{
//-------------REMOVED CASE-----------//
	//ODM_CMNINFO_CCK_HP,
	//ODM_CMNINFO_RFPATH_ENABLE,		// Define as ODM write???	
	//ODM_CMNINFO_BT_COEXIST,				// ODM_BT_COEXIST_E
	//ODM_CMNINFO_OP_MODE,				// ODM_OPERATION_MODE_E
//-------------REMOVED CASE-----------//

	//
	// Fixed value:
	//

	//-----------HOOK BEFORE REG INIT-----------//
	ODM_CMNINFO_PLATFORM = 0,
	ODM_CMNINFO_ABILITY,					// ODM_ABILITY_E
	ODM_CMNINFO_INTERFACE,				// ODM_INTERFACE_E
	ODM_CMNINFO_MP_TEST_CHIP,
	ODM_CMNINFO_IC_TYPE,					// ODM_IC_TYPE_E
	ODM_CMNINFO_CUT_VER,					// ODM_CUT_VERSION_E
	ODM_CMNINFO_FAB_VER,					// ODM_FAB_E
	ODM_CMNINFO_RF_TYPE,					// ODM_RF_PATH_E or ODM_RF_TYPE_E?
	ODM_CMNINFO_RFE_TYPE, 
	ODM_CMNINFO_BOARD_TYPE,				// ODM_BOARD_TYPE_E
	ODM_CMNINFO_PACKAGE_TYPE,
	ODM_CMNINFO_EXT_LNA,					// TRUE
	ODM_CMNINFO_5G_EXT_LNA,	
	ODM_CMNINFO_EXT_PA,
	ODM_CMNINFO_5G_EXT_PA,
	ODM_CMNINFO_GPA,
	ODM_CMNINFO_APA,
	ODM_CMNINFO_GLNA,
	ODM_CMNINFO_ALNA,
	ODM_CMNINFO_EXT_TRSW,
	ODM_CMNINFO_EXT_LNA_GAIN,
	ODM_CMNINFO_PATCH_ID,				//CUSTOMER ID
	ODM_CMNINFO_BINHCT_TEST,
	ODM_CMNINFO_BWIFI_TEST,
	ODM_CMNINFO_SMART_CONCURRENT,
	ODM_CMNINFO_CONFIG_BB_RF,
	ODM_CMNINFO_DOMAIN_CODE_2G,
	ODM_CMNINFO_DOMAIN_CODE_5G,
	ODM_CMNINFO_IQKFWOFFLOAD,
	ODM_CMNINFO_IQKPAOFF,
	ODM_CMNINFO_HUBUSBMODE,
	ODM_CMNINFO_FWDWRSVDPAGEINPROGRESS,
	ODM_CMNINFO_TX_TP,
	ODM_CMNINFO_RX_TP,
	ODM_CMNINFO_SOUNDING_SEQ,
	ODM_CMNINFO_REGRFKFREEENABLE,
	ODM_CMNINFO_RFKFREEENABLE,
	ODM_CMNINFO_NORMAL_RX_PATH_CHANGE,
	//-----------HOOK BEFORE REG INIT-----------//	


	//
	// Dynamic value:
	//
//--------- POINTER REFERENCE-----------//
	ODM_CMNINFO_MAC_PHY_MODE,			// ODM_MAC_PHY_MODE_E
	ODM_CMNINFO_TX_UNI,
	ODM_CMNINFO_RX_UNI,
	ODM_CMNINFO_WM_MODE,				// ODM_WIRELESS_MODE_E
	ODM_CMNINFO_BAND,					// ODM_BAND_TYPE_E
	ODM_CMNINFO_SEC_CHNL_OFFSET,		// ODM_SEC_CHNL_OFFSET_E
	ODM_CMNINFO_SEC_MODE,				// ODM_SECURITY_E
	ODM_CMNINFO_BW,						// ODM_BW_E
	ODM_CMNINFO_CHNL,
	ODM_CMNINFO_FORCED_RATE,
	
	ODM_CMNINFO_DMSP_GET_VALUE,
	ODM_CMNINFO_BUDDY_ADAPTOR,
	ODM_CMNINFO_DMSP_IS_MASTER,
	ODM_CMNINFO_SCAN,
	ODM_CMNINFO_POWER_SAVING,
	ODM_CMNINFO_ONE_PATH_CCA,			// ODM_CCA_PATH_E
	ODM_CMNINFO_DRV_STOP,
	ODM_CMNINFO_PNP_IN,
	ODM_CMNINFO_INIT_ON,
	ODM_CMNINFO_ANT_TEST,
	ODM_CMNINFO_NET_CLOSED,
	//ODM_CMNINFO_RTSTA_AID,				// For win driver only?
	ODM_CMNINFO_FORCED_IGI_LB,
	ODM_CMNINFO_P2P_LINK,
	ODM_CMNINFO_FCS_MODE,
	ODM_CMNINFO_IS1ANTENNA,
	ODM_CMNINFO_RFDEFAULTPATH,
	ODM_CMNINFO_DFS_MASTER_ENABLE,
	ODM_CMNINFO_FORCE_TX_ANT_BY_TXDESC,
//--------- POINTER REFERENCE-----------//

//------------CALL BY VALUE-------------//
	ODM_CMNINFO_WIFI_DIRECT,
	ODM_CMNINFO_WIFI_DISPLAY,
	ODM_CMNINFO_LINK_IN_PROGRESS,			
	ODM_CMNINFO_LINK,
	ODM_CMNINFO_STATION_STATE,
	ODM_CMNINFO_RSSI_MIN,
	ODM_CMNINFO_MACID_RSSI_MIN,
	ODM_CMNINFO_RSSI_MIN_BY_PATH,
	ODM_CMNINFO_DBG_COMP,				/* u4SByte*/
	ODM_CMNINFO_DBG_LEVEL,				/* u4Byte*/
	ODM_CMNINFO_RA_THRESHOLD_HIGH,		/* u1Byte*/
	ODM_CMNINFO_RA_THRESHOLD_LOW,		/* u1Byte*/
	ODM_CMNINFO_RF_ANTENNA_TYPE,			/* u1Byte*/
	ODM_CMNINFO_WITH_EXT_ANTENNA_SWITCH,
	ODM_CMNINFO_BE_FIX_TX_ANT,
	ODM_CMNINFO_BT_ENABLED,
	ODM_CMNINFO_BT_HS_CONNECT_PROCESS,
	ODM_CMNINFO_BT_HS_RSSI,
	ODM_CMNINFO_BT_OPERATION,
	ODM_CMNINFO_BT_LIMITED_DIG,					//Need to Limited Dig or not
	ODM_CMNINFO_BT_DIG,
	ODM_CMNINFO_BT_BUSY,					//Check Bt is using or not//neil	
	ODM_CMNINFO_BT_DISABLE_EDCA,
#if(DM_ODM_SUPPORT_TYPE & ODM_AP)		// for repeater mode add by YuChen 2014.06.23
#ifdef UNIVERSAL_REPEATER
	ODM_CMNINFO_VXD_LINK,
#endif
#endif
	ODM_CMNINFO_AP_TOTAL_NUM,
	ODM_CMNINFO_POWER_TRAINING,
	ODM_CMNINFO_DFS_REGION_DOMAIN,
//------------CALL BY VALUE-------------//

	//
	// Dynamic ptr array hook itms.
	//
	ODM_CMNINFO_STA_STATUS,
	ODM_CMNINFO_PHY_STATUS,
	ODM_CMNINFO_MAC_STATUS,
	
	ODM_CMNINFO_MAX,


}ODM_CMNINFO_E;


typedef enum _PHYDM_Info_query_type {
	PHYDM_INFO_FA_OFDM,
	PHYDM_INFO_FA_CCK,
	PHYDM_INFO_FA_TOTAL,
	PHYDM_INFO_CCA_OFDM,
	PHYDM_INFO_CCA_CCK,
	PHYDM_INFO_CCA_ALL,
	PHYDM_INFO_CRC32_OK_VHT,
	PHYDM_INFO_CRC32_OK_HT,
	PHYDM_INFO_CRC32_OK_LEGACY,
	PHYDM_INFO_CRC32_OK_CCK,
	PHYDM_INFO_CRC32_ERROR_VHT,
	PHYDM_INFO_CRC32_ERROR_HT,
	PHYDM_INFO_CRC32_ERROR_LEGACY,
	PHYDM_INFO_CRC32_ERROR_CCK,
	PHYDM_INFO_EDCCA_FLAG,
	PHYDM_INFO_OFDM_ENABLE,
	PHYDM_INFO_CCK_ENABLE,
	PHYDM_INFO_DBG_PORT_0
} PHYDM_INFO_QUERY_E;

typedef enum _PHYDM_API_Definition {

	PHYDM_API_NBI			= 1,
	PHYDM_API_CSI_MASK,

	
} PHYDM_API_E;


//
// 2011/10/20 MH Define ODM support ability.  ODM_CMNINFO_ABILITY
//
typedef enum _ODM_Support_Ability_Definition
{
	//
	// BB ODM section BIT 0-19
	//
	ODM_BB_DIG					= BIT0,
	ODM_BB_RA_MASK				= BIT1,
	ODM_BB_DYNAMIC_TXPWR		= BIT2,
	ODM_BB_FA_CNT				= BIT3,
	ODM_BB_RSSI_MONITOR			= BIT4,
	ODM_BB_CCK_PD				= BIT5,
	ODM_BB_ANT_DIV				= BIT6,
	ODM_BB_PWR_TRAIN			= BIT8,
	ODM_BB_RATE_ADAPTIVE		= BIT9,
	ODM_BB_PATH_DIV				= BIT10,
	ODM_BB_LNA_SAT_CHK			= BIT11,	
	ODM_BB_LNA_SAT_CHK_TYPE2	= BIT12,
	ODM_BB_ADAPTIVITY			= BIT13,
	ODM_BB_CFO_TRACKING			= BIT14,
	ODM_BB_NHM_CNT				= BIT15,
	ODM_BB_PRIMARY_CCA			= BIT16,
	ODM_BB_TXBF					= BIT17,
	ODM_BB_DYNAMIC_ARFR			= BIT18,
	ODM_BB_DYNAMIC_PSDTOOL   	= BIT19,
	//
	// MAC DM section BIT 20-23
	//
	ODM_MAC_EDCA_TURBO			= BIT20,
	ODM_MAC_EARLY_MODE			= BIT21,
	ODM_BB_ADAPTIVE_SOML		= BIT23,
	
	//
	// RF ODM section BIT 24-31
	//
	ODM_RF_TX_PWR_TRACK			= BIT24,
	ODM_RF_RX_GAIN_TRACK			= BIT25,
	ODM_RF_CALIBRATION			= BIT26,
	
}ODM_ABILITY_E;


// ODM_CMNINFO_ONE_PATH_CCA
typedef enum tag_CCA_Path
{
	ODM_CCA_2R		= 0,
	ODM_CCA_1R_A		= 1,
	ODM_CCA_1R_B		= 2,
}ODM_CCA_PATH_E;

typedef enum CCA_PATHDIV_EN {
	CCA_PATHDIV_DISABLE		= 0,
	CCA_PATHDIV_ENABLE		= 1,

} CCA_PATHDIV_EN_E;


typedef enum _BASEBAND_CONFIG_PHY_REG_PG_VALUE_TYPE{
	PHY_REG_PG_RELATIVE_VALUE = 0,
	PHY_REG_PG_EXACT_VALUE = 1
} PHY_REG_PG_TYPE;

//
// 2011/09/22 MH Copy from SD4 defined structure. We use to support PHY DM integration.
//
#if(DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (RT_PLATFORM != PLATFORM_LINUX)
typedef 
#endif
	
struct DM_Out_Source_Dynamic_Mechanism_Structure
#else// for AP,ADSL,CE Team
typedef  struct DM_Out_Source_Dynamic_Mechanism_Structure
#endif
{
	//	Add for different team use temporarily
	//
	PADAPTER		Adapter;		// For CE/NIC team
	prtl8192cd_priv	priv;			// For AP/ADSL team
	// WHen you use Adapter or priv pointer, you must make sure the pointer is ready.
	BOOLEAN			odm_ready;

#if(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
	rtl8192cd_priv		fake_priv;
#endif
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	// ADSL_AP_BUILD_WORKAROUND
	ADAPTER			fake_adapter;
#endif
	
	PHY_REG_PG_TYPE		PhyRegPgValueType;
	u1Byte				PhyRegPgVersion;

	u4Byte			DebugComponents;
	u4Byte			fw_DebugComponents;
	u4Byte			DebugLevel;
	
	u4Byte			NumQryPhyStatusAll; 	//CCK + OFDM
	u4Byte			LastNumQryPhyStatusAll; 
	u4Byte			RxPWDBAve;
	BOOLEAN			MPDIG_2G; 		//off MPDIG
	u1Byte			Times_2G;
	BOOLEAN			bInitHwInfoByRfe;
	
//------ ODM HANDLE, DRIVER NEEDS NOT TO HOOK------//
	BOOLEAN			bCckHighPower; 
	u1Byte			RFPathRxEnable;		// ODM_CMNINFO_RFPATH_ENABLE
	u1Byte			ControlChannel;
//------ ODM HANDLE, DRIVER NEEDS NOT TO HOOK------//

//--------REMOVED COMMON INFO----------//
	//u1Byte				PseudoMacPhyMode;
	//BOOLEAN			*BTCoexist;
	//BOOLEAN			PseudoBtCoexist;
	//u1Byte				OPMode;
	//BOOLEAN			bAPMode;
	//BOOLEAN			bClientMode;
	//BOOLEAN			bAdHocMode;
	//BOOLEAN			bSlaveOfDMSP;
//--------REMOVED COMMON INFO----------//


//1  COMMON INFORMATION

	//
	// Init Value
	//
//-----------HOOK BEFORE REG INIT-----------//	
	// ODM Platform info AP/ADSL/CE/MP = 1/2/3/4
	u1Byte			SupportPlatform;
	// ODM Platform info WIN/AP/CE 
	u1Byte			Normalrxpath;
	// ODM Support Ability DIG/RATR/TX_PWR_TRACK/ �K�K = 1/2/3/�K
	u4Byte			SupportAbility;
	// ODM PCIE/USB/SDIO = 1/2/3
	u1Byte			SupportInterface;			
	// ODM composite or independent. Bit oriented/ 92C+92D+ .... or any other type = 1/2/3/...
	u4Byte			SupportICType;	
	// Cut Version TestChip/A-cut/B-cut... = 0/1/2/3/...
	u1Byte			CutVersion;
	// Fab Version TSMC/UMC = 0/1
	u1Byte			FabVersion;
	// RF Type 4T4R/3T3R/2T2R/1T2R/1T1R/...
	u1Byte			RFType;
	u1Byte			RFEType;	
	// Board Type Normal/HighPower/MiniCard/SLIM/Combo/... = 0/1/2/3/4/...
	u1Byte			BoardType;
	u1Byte			PackageType;
	u2Byte			TypeGLNA;
	u2Byte			TypeGPA;
	u2Byte			TypeALNA;
	u2Byte			TypeAPA;
	// with external LNA  NO/Yes = 0/1
	u1Byte			ExtLNA; // 2G
	u1Byte			ExtLNA5G; //5G
	// with external PA  NO/Yes = 0/1
	u1Byte			ExtPA; // 2G
	u1Byte			ExtPA5G; //5G
	// with external TRSW  NO/Yes = 0/1
	u1Byte			ExtTRSW;
	u1Byte			ExtLNAGain; // 2G
	u1Byte			PatchID; //Customer ID
	BOOLEAN			bInHctTest;
	u1Byte			WIFITest;

	BOOLEAN			bDualMacSmartConcurrent;
	u4Byte			BK_SupportAbility;
	u1Byte			AntDivType;
	u1Byte			with_extenal_ant_switch;
	BOOLEAN			ConfigBBRF;
	u1Byte			odm_Regulation2_4G;
	u1Byte			odm_Regulation5G;
	u1Byte			IQKFWOffload;
	BOOLEAN			cck_new_agc;
	BOOLEAN			use_new_agc;
	s8				cck_lna_gain_table[8];
	u1Byte			phydm_period;
	u4Byte			phydm_sys_up_time;
	u1Byte			num_rf_path;
	u2Byte			consecutive_idlel_time;	/*unit: second*/
//-----------HOOK BEFORE REG INIT-----------//	

	//
	// Dynamic Value
	//	
//--------- POINTER REFERENCE-----------//

	u1Byte			u1Byte_temp;
	BOOLEAN			BOOLEAN_temp;
	PADAPTER		PADAPTER_temp;
	
	// MAC PHY Mode SMSP/DMSP/DMDP = 0/1/2
	u1Byte			*pMacPhyMode;
	//TX Unicast byte count
	u8Byte			*pNumTxBytesUnicast;
	//RX Unicast byte count
	u8Byte			*pNumRxBytesUnicast;
	// Wireless mode B/G/A/N = BIT0/BIT1/BIT2/BIT3
	u1Byte			*pWirelessMode; //ODM_WIRELESS_MODE_E
	// Frequence band 2.4G/5G = 0/1
	u1Byte			*pBandType;
	// Secondary channel offset don't_care/below/above = 0/1/2
	u1Byte			*pSecChOffset;
	// Security mode Open/WEP/AES/TKIP = 0/1/2/3
	u1Byte			*pSecurity;
	// BW info 20M/40M/80M = 0/1/2
	u1Byte			*pBandWidth;
 	// Central channel location Ch1/Ch2/....
	u1Byte			*pChannel;	//central channel number
	BOOLEAN			DPK_Done;
	// Common info for 92D DMSP
	
	BOOLEAN			*pbGetValueFromOtherMac;
	PADAPTER		*pBuddyAdapter;
	BOOLEAN			*pbMasterOfDMSP; //MAC0: master, MAC1: slave
	// Common info for Status
	BOOLEAN			*pbScanInProcess;
	BOOLEAN			*pbPowerSaving;
	// CCA Path 2-path/path-A/path-B = 0/1/2; using ODM_CCA_PATH_E.
	u1Byte			*pOnePathCCA;
	//pMgntInfo->AntennaTest
	u1Byte			*pAntennaTest;
	BOOLEAN			*pbNet_closed;
	//u1Byte			*pAidMap;
	u1Byte			*pu1ForcedIgiLb;
	BOOLEAN			*pIsFcsModeEnable;
/*--------- For 8723B IQK-----------*/
	BOOLEAN			*pIs1Antenna;
	u1Byte			*pRFDefaultPath;
	// 0:S1, 1:S0
	
//--------- POINTER REFERENCE-----------//
	pu2Byte			pForcedDataRate;
	pu1Byte			HubUsbMode;
	BOOLEAN			*pbFwDwRsvdPageInProgress;
	u4Byte			*pCurrentTxTP;
	u4Byte			*pCurrentRxTP;
	u1Byte			*pSoundingSeq;
//------------CALL BY VALUE-------------//
	BOOLEAN			bLinkInProcess;
	BOOLEAN			bWIFI_Direct;
	BOOLEAN			bWIFI_Display;
	BOOLEAN			bLinked;
	BOOLEAN			bsta_state;
#if(DM_ODM_SUPPORT_TYPE & ODM_AP)		// for repeater mode add by YuChen 2014.06.23
#ifdef UNIVERSAL_REPEATER
	BOOLEAN			VXD_bLinked;
#endif
#endif									// for repeater mode add by YuChen 2014.06.23	
	u1Byte			RSSI_Min;
	u1Byte			macid_rssi_min;
	u1Byte			RSSI_Min_ByPath;
	u1Byte			InterfaceIndex; /*Add for 92D  dual MAC: 0--Mac0 1--Mac1*/
	BOOLEAN			bIsMPChip;
	BOOLEAN			bOneEntryOnly;
	BOOLEAN			mp_mode;
	u4Byte			OneEntry_MACID;
	u32				one_entry_tp;
	u32				pre_one_entry_tp;
	u1Byte			pre_number_linked_client;	
	u1Byte			number_linked_client;
	u1Byte			pre_number_active_client;	
	u1Byte			number_active_client;
	// Common info for BTDM
	BOOLEAN			bBtEnabled;			// BT is enabled
	BOOLEAN			bBtConnectProcess;	// BT HS is under connection progress.
	u1Byte			btHsRssi;				// BT HS mode wifi rssi value.
	BOOLEAN			bBtHsOperation;		// BT HS mode is under progress
	u1Byte			btHsDigVal;			// use BT rssi to decide the DIG value
	BOOLEAN			bBtDisableEdcaTurbo;	// Under some condition, don't enable the EDCA Turbo
	BOOLEAN			bBtBusy;   			// BT is busy.
	BOOLEAN			bBtLimitedDig;   		// BT is busy.
	BOOLEAN			bDisablePhyApi;
//------------CALL BY VALUE-------------//
	u1Byte			RSSI_A;
	u1Byte			RSSI_B;
	u1Byte			RSSI_C;
	u1Byte			RSSI_D;
	u8Byte			RSSI_TRSW;	
	u8Byte			RSSI_TRSW_H;
	u8Byte			RSSI_TRSW_L;	
	u8Byte			RSSI_TRSW_iso;
	u1Byte			TXAntStatus;
	u1Byte			RXAntStatus;
	u1Byte			cck_lna_idx;
	u1Byte			cck_vga_idx;
	u1Byte			curr_station_id;
	u1Byte			ofdm_agc_idx[4];

	u1Byte			RxRate;
	u8				rate_ss;
	BOOLEAN			bNoisyState;
	u1Byte			TxRate;
	u1Byte			LinkedInterval;
	u1Byte			preChannel;
	u4Byte			TxagcOffsetValueA;
	BOOLEAN			IsTxagcOffsetPositiveA;
	u4Byte			TxagcOffsetValueB;
	BOOLEAN			IsTxagcOffsetPositiveB;
	u4Byte			tx_tp;
	u4Byte			rx_tp;
	u4Byte			total_tp;
	u8Byte			curTxOkCnt;
	u8Byte			curRxOkCnt;	
	u8Byte			lastTxOkCnt;
	u8Byte			lastRxOkCnt;
	u4Byte			BbSwingOffsetA;
	BOOLEAN			IsBbSwingOffsetPositiveA;
	u4Byte			BbSwingOffsetB;
	BOOLEAN			IsBbSwingOffsetPositiveB;
	u1Byte			antdiv_rssi;
	u1Byte			fat_comb_a;
	u1Byte			fat_comb_b;
	u1Byte			antdiv_intvl;
	u1Byte			AntType;
	u1Byte			pre_AntType;
	u1Byte			antdiv_period;
	u1Byte			evm_antdiv_period;
	u1Byte			antdiv_tp_period;
	u1Byte			antdiv_select;
	u1Byte			path_select;
	u8				antdiv_train_num;/*training time for each antenna in EVM method*/
	u8				stop_antdiv_rssi_th;
	u16				stop_antdiv_tp_diff_th;
	u16				stop_antdiv_tp_th;
	u16				tp_active_th;
	u8				tp_active_occur;
	u1Byte			antdiv_evm_en;
	u1Byte			bdc_holdstate;
	u1Byte			NdpaPeriod;
	BOOLEAN			H2C_RARpt_connect;
	BOOLEAN			cck_agc_report_type;
	
	u1Byte			dm_dig_max_TH;
	u1Byte 			dm_dig_min_TH;
	u1Byte 			print_agc;
	u1Byte			TrafficLoad;
	u1Byte			pre_TrafficLoad;
	u1Byte			la_mode;


	//For Adaptivtiy
	u2Byte			NHM_cnt_0;
	u2Byte			NHM_cnt_1;
	s1Byte			TH_L2H_default;
	s1Byte			TH_EDCCA_HL_diff_default;
	s1Byte			TH_L2H_ini;
	s1Byte			TH_EDCCA_HL_diff;
	s1Byte			TH_L2H_ini_mode2;
	s1Byte			TH_EDCCA_HL_diff_mode2;
	BOOLEAN			Carrier_Sense_enable;
	u1Byte			Adaptivity_IGI_upper;
	BOOLEAN			adaptivity_flag;
	u1Byte			DCbackoff;
	BOOLEAN			Adaptivity_enable;
	u1Byte			APTotalNum;
	BOOLEAN			EDCCA_enable;
	ADAPTIVITY_STATISTICS	Adaptivity;
	//For Adaptivtiy
	u1Byte			LastUSBHub;
	u1Byte			TxBfDataRate;

	u1Byte			nbi_set_result;
	u1Byte			csi_set_result;
	u1Byte			csi_set_result_2;
	
	u1Byte			c2h_cmd_start;
	u1Byte			fw_debug_trace[60]; 
	u1Byte			pre_c2h_seq;
	BOOLEAN			fw_buff_is_enpty;
	u4Byte			data_frame_num;

	/*for noise detection*/
	BOOLEAN			NoisyDecision; /*b_noisy*/
	BOOLEAN			pre_b_noisy;	
	u4Byte			NoisyDecision_Smooth;
	BOOLEAN			is_disable_dym_ecs;
	u4Byte			original_pd_threshold;
	BOOLEAN			is_disable_dym_ant_weighting;
	BOOLEAN			is_disable_primary_cca;
	u1Byte			lna_sat_chk_cnt;
	u1Byte			lna_sat_chk_duty_cycle;
	u4Byte			lna_sat_chk_period_ms;
	BOOLEAN			is_disable_lna_sat_chk;
	BOOLEAN			is_disable_gain_table_switch;

	/*TDMA-DIG*/
	u1Byte			force_low_igi;
	u1Byte			force_high_igi;
	bool			rx_gain_min_dbg_switch;
	u1Byte			tdma_low_fa_cca_ratio_l2h;
	u1Byte			tdma_low_fa_cca_ratio_h2l;
	u1Byte			tdma_high_fa_cca_ratio_l2h;
	u1Byte			tdma_high_fa_cca_ratio_h2l;
	u1Byte			tdma_is_enough_fa_th;
	bool			is_disable_rx_gain_min_check;
	u1Byte			rssi_to_igi_upper_offset;
	bool			tdma_dig_turbo;
	u1Byte			tdma_dig_timer_ms;
	bool 			original_dig_restore;
	u8				tdma_dig_low_upper_bond;
	u8				total_state_number;

#if (PHYDM_LNA_SAT_CHK_TYPE2 == 1)
		u1Byte			snr_var_thd;
		u1Byte			delta_snr_mean; 
		u1Byte			delta_snr_var;
		u1Byte			total_bit_shift;
		u1Byte			is_force_lna_sat_table;
		u1Byte			is_force_lna_sat_macid;
		u1Byte			is_snr_detail_en;
		u1Byte			lwr_snr_ratio_bit_shift;
		u1Byte			force_change_period;
#endif

	ODM_NOISE_MONITOR noise_level;//[ODM_MAX_CHANNEL_NUM];

	//
	//2 Define STA info.
	// _ODM_STA_INFO
	// 2012/01/12 MH For MP, we need to reduce one array pointer for default port.??
	PSTA_INFO_T		pODM_StaInfo[ODM_ASSOCIATE_ENTRY_NUM];
	u2Byte			platform2phydm_macid_table[ODM_ASSOCIATE_ENTRY_NUM];		/* platform_macid_table[platform_macid] = phydm_macid */
#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	s4Byte			AccumulatePWDB[ODM_ASSOCIATE_ENTRY_NUM];
#endif

#if (RATE_ADAPTIVE_SUPPORT == 1)
	u2Byte 			CurrminRptTime;
	ODM_RA_INFO_T   RAInfo[ODM_ASSOCIATE_ENTRY_NUM]; //Use MacID as array index. STA MacID=0, VWiFi Client MacID={1, ODM_ASSOCIATE_ENTRY_NUM-1} //YJ,add,120119
#endif
	//
	// 2012/02/14 MH Add to share 88E ra with other SW team.
	// We need to colelct all support abilit to a proper area.
	//
	BOOLEAN				RaSupport88E;

	// Define ...........

	// Latest packet phy info (ODM write)
	ODM_PHY_DBG_INFO_T	 PhyDbgInfo;
	//PHY_INFO_88E		PhyInfo;

	// Latest packet phy info (ODM write)
	ODM_MAC_INFO		*pMacInfo;
	//MAC_INFO_88E		MacInfo;

	// Different Team independt structure??

	//
	//TX_RTP_CMN		TX_retrpo;
	//TX_RTP_88E		TX_retrpo;
	//TX_RTP_8195		TX_retrpo;

	//
	//ODM Structure
	//
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	BDC_T					DM_BdcTable;
	#endif
	
	#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
	SAT_T						dm_sat_table;
	#endif
	
#endif
	FAT_T						DM_FatTable;
	DIG_T						DM_DigTable;
#if (defined(CONFIG_BB_POWER_SAVING))
	PS_T						DM_PSTable;
#endif
	Pri_CCA_T					DM_PriCCA;
	RA_T						DM_RA_Table;  
	FALSE_ALARM_STATISTICS		FalseAlmCnt;
#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	FALSE_ALARM_STATISTICS_ACC	FalseAlmCnt_Acc;
#endif
	FALSE_ALARM_STATISTICS		FlaseAlmCntBuddyAdapter;
	SWAT_T						DM_SWAT_Table;
	CFO_TRACKING    			DM_CfoTrack;
	ACS							DM_ACS;
	CCX_INFO					DM_CCX_INFO;
	LNA_SATURATION_INFO			DM_LNA_SAT_INFO;

#if (PHYDM_LNA_SAT_CHK_TYPE2 == 1)
	LNA_SAT_TYPE2_INFO			DM_LNA_SAT_TYPE2_INFO;
#endif

#if (CONFIG_ADAPTIVE_SOML)
	struct _ADAPTIVE_SOML_			dm_soml_table;
#endif
#if (PHYDM_LA_MODE_SUPPORT == 1)
	RT_ADCSMP					adcsmp;
#endif

#if (RTL8814A_SUPPORT == 1 || RTL8822B_SUPPORT == 1 || RTL8821C_SUPPORT == 1)
	IQK_INFO	IQK_info;
#endif

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	//Path Div Struct
	PATHDIV_PARA	pathIQK;
#endif
#if(defined(CONFIG_PATH_DIVERSITY))
	PATHDIV_T	DM_PathDiv;
#endif	

	EDCA_T		DM_EDCA_Table;
	u4Byte		WMMEDCA_BE;

	// Copy from SD4 structure
	//
	// ==================================================
	//

	//common
	//u1Byte		DM_Type;	
	//u1Byte    PSD_Report_RXHP[80];   // Add By Gary
	//u1Byte    PSD_func_flag;               // Add By Gary
	//for DIG
	//u1Byte		bDMInitialGainEnable;
	//u1Byte		binitialized; // for dm_initial_gain_Multi_STA use.

	BOOLEAN			*pbDriverStopped;
	BOOLEAN			*pbDriverIsGoingToPnpSetPowerSleep;
	BOOLEAN			*pinit_adpt_in_progress;

	//PSD
	BOOLEAN			bUserAssignLevel;
	u1Byte			RSSI_BT;				/*come from BT*/
	BOOLEAN			bPSDinProcess;
	BOOLEAN			bPSDactive;
	BOOLEAN			bDMInitialGainEnable;

	//MPT DIG
	RT_TIMER 		MPT_DIGTimer;
	
	//for rate adaptive, in fact,  88c/92c fw will handle this
	u1Byte			bUseRAMask;

	ODM_RATE_ADAPTIVE	RateAdaptive;
	#if (defined(CONFIG_ANT_DETECTION))
	ANT_DETECTED_INFO	AntDetectedInfo;	/* Antenna detected information for RSSI tool*/
	#endif
	ODM_RF_CAL_T	RFCalibrateInfo;
	u4Byte			nIQK_Cnt;
	u4Byte			nIQK_OK_Cnt;
	u4Byte			nIQK_Fail_Cnt;

#if (DM_ODM_SUPPORT_TYPE &  (ODM_WIN|ODM_CE))	
	//
	// Power Training
	//
	u1Byte			ForcePowerTrainingState;
	BOOLEAN			bChangeState;
	u4Byte			PT_score;
	u8Byte			OFDM_RX_Cnt;
	u8Byte			CCK_RX_Cnt;
#endif
	BOOLEAN			bDisablePowerTraining;
	u1Byte			DynamicTxHighPowerLvl;
	u1Byte			LastDTPLvl;
	u4Byte			tx_agc_ofdm_18_6;
	u1Byte			rx_pkt_type;

	//
	// ODM system resource.
	//

	// ODM relative time.
	RT_TIMER	PathDivSwitchTimer;
	//2011.09.27 add for Path Diversity
	RT_TIMER	CCKPathDiversityTimer;
	RT_TIMER 	FastAntTrainingTimer;
	#ifdef ODM_EVM_ENHANCE_ANTDIV
	RT_TIMER 			EVM_FastAntTrainingTimer;
	#endif
	RT_TIMER		sbdcnt_timer;

	// ODM relative workitem.
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#if USE_WORKITEM
	RT_WORK_ITEM			PathDivSwitchWorkitem;
	RT_WORK_ITEM			CCKPathDiversityWorkitem;
	RT_WORK_ITEM			FastAntTrainingWorkitem;
	RT_WORK_ITEM			MPT_DIGWorkitem;
	RT_WORK_ITEM			RaRptWorkitem;
	RT_WORK_ITEM			sbdcnt_workitem;
#endif
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (BEAMFORMING_SUPPORT == 1)
	RT_BEAMFORMING_INFO BeamformingInfo;
#endif 
#endif

#ifdef CONFIG_PHYDM_DFS_MASTER
	u1Byte DFS_RegionDomain;
	pu1Byte dfs_master_enabled;

	/*====== phydm_radar_detect_with_dbg_parm start ======*/
	u1Byte radar_detect_dbg_parm_en;
	u4Byte radar_detect_reg_918;
	u4Byte radar_detect_reg_91c;
	u4Byte radar_detect_reg_920;
	u4Byte radar_detect_reg_924;
	/*====== phydm_radar_detect_with_dbg_parm end ======*/
#endif

#if(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	
#if (RT_PLATFORM != PLATFORM_LINUX)
} DM_ODM_T, *PDM_ODM_T;		// DM_Dynamic_Mechanism_Structure
#else
};
#endif	

#else// for AP,ADSL,CE Team
} DM_ODM_T, *PDM_ODM_T;		// DM_Dynamic_Mechanism_Structure
#endif


typedef enum _PHYDM_STRUCTURE_TYPE{
	PHYDM_FALSEALMCNT,
	PHYDM_CFOTRACK,
	PHYDM_ADAPTIVITY,
	PHYDM_ROMINFO,
	
}PHYDM_STRUCTURE_TYPE;



 typedef enum _ODM_RF_CONTENT{
	odm_radioa_txt = 0x1000,
	odm_radiob_txt = 0x1001,
	odm_radioc_txt = 0x1002,
	odm_radiod_txt = 0x1003
} ODM_RF_CONTENT;

typedef enum _ODM_BB_Config_Type{
	CONFIG_BB_PHY_REG,   
	CONFIG_BB_AGC_TAB,   
	CONFIG_BB_AGC_TAB_2G,
	CONFIG_BB_AGC_TAB_5G, 
	CONFIG_BB_PHY_REG_PG,
	CONFIG_BB_PHY_REG_MP,
	CONFIG_BB_AGC_TAB_DIFF,
} ODM_BB_Config_Type, *PODM_BB_Config_Type;

typedef enum _ODM_RF_Config_Type{ 
	CONFIG_RF_RADIO,
    CONFIG_RF_TXPWR_LMT,
} ODM_RF_Config_Type, *PODM_RF_Config_Type;

typedef enum _ODM_FW_Config_Type{
    CONFIG_FW_NIC,
    CONFIG_FW_NIC_2,
    CONFIG_FW_AP,
    CONFIG_FW_AP_2,
    CONFIG_FW_MP,
    CONFIG_FW_WoWLAN,
    CONFIG_FW_WoWLAN_2,
    CONFIG_FW_AP_WoWLAN,
    CONFIG_FW_BT,
} ODM_FW_Config_Type;

// Status code
#if (DM_ODM_SUPPORT_TYPE != ODM_WIN)
typedef enum _RT_STATUS{
	RT_STATUS_SUCCESS,
	RT_STATUS_FAILURE,
	RT_STATUS_PENDING,
	RT_STATUS_RESOURCE,
	RT_STATUS_INVALID_CONTEXT,
	RT_STATUS_INVALID_PARAMETER,
	RT_STATUS_NOT_SUPPORT,
	RT_STATUS_OS_API_FAILED,
}RT_STATUS,*PRT_STATUS;
#endif // end of RT_STATUS definition

#ifdef REMOVE_PACK
#pragma pack()
#endif

//3===========================================================
//3 AGC RX High Power Mode
//3===========================================================
#define          LNA_Low_Gain_1                      0x64
#define          LNA_Low_Gain_2                      0x5A
#define          LNA_Low_Gain_3                      0x58

#define          FA_RXHP_TH1                           5000
#define          FA_RXHP_TH2                           1500
#define          FA_RXHP_TH3                             800
#define          FA_RXHP_TH4                             600
#define          FA_RXHP_TH5                             500

typedef enum tag_1R_CCA_Type_Definition
{
	CCA_1R =0,
	CCA_2R = 1,
	CCA_MAX = 2,
}DM_1R_CCA_E;

typedef enum tag_RF_Type_Definition
{
	RF_Save =0,
	RF_Normal = 1,
	RF_MAX = 2,
}DM_RF_E;

//
// check Sta pointer valid or not
//
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
#define IS_STA_VALID(pSta)		(pSta && pSta->expire_to)
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#define IS_STA_VALID(pSta)		(pSta && pSta->bUsed)
#else
#define IS_STA_VALID(pSta)		(pSta)
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_AP))

BOOLEAN
ODM_CheckPowerStatus(
	IN	PADAPTER		Adapter
	);

#endif



u4Byte odm_ConvertTo_dB(u4Byte Value);

u4Byte odm_ConvertTo_linear(u4Byte Value);

#if((DM_ODM_SUPPORT_TYPE==ODM_WIN)||(DM_ODM_SUPPORT_TYPE==ODM_CE))

u4Byte
GetPSDData(
	PDM_ODM_T	pDM_Odm,
	unsigned int 	point,
	u1Byte initial_gain_psd);

#endif

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)	
VOID
ODM_DMWatchdog_LPS(
	IN		PDM_ODM_T		pDM_Odm
);
#endif


s4Byte
ODM_PWdB_Conversion(
    IN  s4Byte X,
    IN  u4Byte TotalBit,
    IN  u4Byte DecimalBit
    );

s4Byte
ODM_SignConversion(
    IN  s4Byte value,
    IN  u4Byte TotalBit
    );

void
phydm_seq_sorting( 
	IN		PVOID	pDM_VOID,
	IN OUT	u4Byte	*p_value,
	IN OUT	u4Byte	*rank_idx,	
	IN OUT	u4Byte	*p_idx_out,	
	IN		u1Byte	seq_length
);

VOID 
ODM_DMInit(
 IN	PDM_ODM_T	pDM_Odm
);

VOID
ODM_DMReset(
	IN	PDM_ODM_T	pDM_Odm
	);

VOID
phydm_support_ability_debug(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*const dm_value,
	IN		u4Byte			*_used,
	OUT		char				*output,
	IN		u4Byte			*_out_len
	);

VOID
phydm_config_trx_path(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		*const dm_value,
	IN		u4Byte			*_used,
	OUT		char			*output,
	IN		u4Byte			*_out_len
	);

VOID
ODM_DMWatchdog(
	IN		PDM_ODM_T			pDM_Odm			// For common use in the future
	);

VOID
ODM_CmnInfoInit(
	IN		PDM_ODM_T		pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		u4Byte			Value	
	);

VOID
ODM_CmnInfoHook(
	IN		PDM_ODM_T		pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		PVOID			pValue	
	);

VOID
ODM_CmnInfoPtrArrayHook(
	IN		PDM_ODM_T		pDM_Odm,
	IN		ODM_CMNINFO_E	CmnInfo,
	IN		u2Byte			Index,
	IN		PVOID			pValue	
	);

VOID
ODM_CmnInfoUpdate(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u4Byte			CmnInfo,
	IN		u8Byte			Value	
	);

u4Byte
PHYDM_CmnInfoQuery(
	IN		PDM_ODM_T					pDM_Odm,
	IN		PHYDM_INFO_QUERY_E			info_type
	);

#if(DM_ODM_SUPPORT_TYPE==ODM_AP)
VOID 
ODM_InitAllThreads(
    IN PDM_ODM_T	pDM_Odm 
    );

VOID
ODM_StopAllThreads(
	IN PDM_ODM_T	pDM_Odm 
	);
#endif

VOID 
ODM_InitAllTimers(
    IN PDM_ODM_T	pDM_Odm 
    );

VOID 
ODM_CancelAllTimers(
    IN PDM_ODM_T    pDM_Odm 
    );

VOID
ODM_ReleaseAllTimers(
    IN PDM_ODM_T	pDM_Odm 
    );




#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
VOID ODM_InitAllWorkItems(IN PDM_ODM_T	pDM_Odm );
VOID ODM_FreeAllWorkItems(IN PDM_ODM_T	pDM_Odm );



u8Byte
PlatformDivision64(
	IN u8Byte	x,
	IN u8Byte	y
);

//====================================================
//3 PathDiV End
//====================================================


#define DM_ChangeDynamicInitGainThresh		ODM_ChangeDynamicInitGainThresh

typedef enum tag_DIG_Connect_Definition
{
	DIG_STA_DISCONNECT = 0,	
	DIG_STA_CONNECT = 1,
	DIG_STA_BEFORE_CONNECT = 2,
	DIG_MultiSTA_DISCONNECT = 3,
	DIG_MultiSTA_CONNECT = 4,
	DIG_CONNECT_MAX
}DM_DIG_CONNECT_E;


//
// 2012/01/12 MH Check afapter status. Temp fix BSOD.
//
#define	HAL_ADAPTER_STS_CHK(pDM_Odm)\
	if (pDM_Odm->Adapter == NULL)\
	{\
		return;\
	}\

#endif	// #if (DM_ODM_SUPPORT_TYPE == ODM_WIN)

VOID
ODM_AsocEntry_Init(
	IN		PDM_ODM_T		pDM_Odm
	);


PVOID
PhyDM_Get_Structure(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte			Structure_Type
);

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN) ||(DM_ODM_SUPPORT_TYPE == ODM_CE)
/*===========================================================*/
/* The following is for compile only*/
/*===========================================================*/

#define	IS_HARDWARE_TYPE_8723A(_Adapter)			FALSE
#define	IS_HARDWARE_TYPE_8723AE(_Adapter)			FALSE
#define	IS_HARDWARE_TYPE_8192C(_Adapter)			FALSE
#define	IS_HARDWARE_TYPE_8192D(_Adapter)			FALSE
#define	RF_T_METER_92D					0x42


#define GET_RX_STATUS_DESC_RX_MCS(__pRxStatusDesc)				LE_BITS_TO_1BYTE( __pRxStatusDesc+12, 0, 6)

#define		rConfig_ram64x16				0xb2c

#define TARGET_CHNL_NUM_2G_5G	59

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
u1Byte GetRightChnlPlaceforIQK(u1Byte chnl);
#endif

//===========================================================
#endif

#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
void odm_dtc(PDM_ODM_T pDM_Odm);
#endif /* #if (DM_ODM_SUPPORT_TYPE == ODM_CE) */


VOID phydm_NoisyDetection(IN	PDM_ODM_T	pDM_Odm	);


#endif

VOID
phydm_api_debug(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		function_map,
	IN		u4Byte		*const dm_value,
	IN		u4Byte		*_used,
	OUT		char			*output,
	IN		u4Byte		*_out_len
);

u1Byte
phydm_csi_mask_setting(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		enable,
	IN		u4Byte		channel,
	IN		u4Byte		bw,
	IN		u4Byte		f_interference,
	IN		u4Byte		Second_ch
);

u1Byte
phydm_nbi_setting(
	IN		PVOID		pDM_VOID,
	IN		u4Byte		enable,
	IN		u4Byte		channel,
	IN		u4Byte		bw,
	IN		u4Byte		f_interference,
	IN		u4Byte		Second_ch
);

void
phydm_receiver_blocking(
	void *p_dm_void
);

