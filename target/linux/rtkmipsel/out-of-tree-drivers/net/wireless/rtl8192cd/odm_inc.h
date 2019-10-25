//============================================================
// File Name: AP_precomp.h
//
// Description:
//
//============================================================


#ifndef	__AP_PRECOMP_H__
#define __AP_PRECOMP_H__

#define		DM_ODM_SUPPORT_TYPE						ODM_AP
#if defined(CONFIG_PCI_HCI)
#define		DEV_BUS_TYPE 							RT_PCI_INTERFACE
#elif defined(CONFIG_USB_HCI)
#define DEV_BUS_TYPE		RT_USB_INTERFACE
#elif defined(CONFIG_SDIO_HCI)
#define DEV_BUS_TYPE		RT_SDIO_INTERFACE
#endif
#ifdef __ECOS
#define 	DBG										0
#else
#define 	DBG										1
#endif
//-----------------------------------------------------------------------------------------
// Use one of the following value to define the flag, RT_PLATFORM.
//-----------------------------------------------------------------------------------------
#define PLATFORM_WINDOWS		0
#define PLATFORM_LINUX			1
#define PLATFORM_FREEBSD		3
#define PLATFORM_MACOSX			4

#define	RT_PLATFORM				PLATFORM_LINUX


//2 [HAL\HWIMG\HalHWImg.h]

#define		RTL8192CU_HWIMG_SUPPORT					0
#define		RTL8192CU_TEST_HWIMG_SUPPORT			0

#define 	RTL8192D_HWIMG_SUPPORT					0

#define 	RTL8723_FPGA_VERIFICATION				0
#define 	RTL8723S_HWIMG_SUPPORT					0
#define 	RTL8723U_HWIMG_SUPPORT					0
#define 	RTL8723A_HWIMG_SUPPORT					0

#ifdef CONFIG_RTL_88E_SUPPORT
#define		RTL8188E_SUPPORT						1
#define 	RTL8188ES_HWIMG_SUPPORT					1
#define		RATE_ADAPTIVE_SUPPORT					1

#if !(DM_ODM_SUPPORT_TYPE & ODM_AP) || defined(RATEADAPTIVE_BY_ODM)
#define 	RATE_ADAPTIVE_SUPPORT					1
#endif
#define		RTL8188E_FOR_TEST_CHIP					0

#else
#define		RTL8188E_SUPPORT						0
#define 	RTL8188ES_HWIMG_SUPPORT					0
#define		RATE_ADAPTIVE_SUPPORT					0
#endif

#if defined(CONFIG_RTL_92C_SUPPORT) && !defined(_OUTSRC_COEXIST)
#define		RTL8192C_SUPPORT						1
#define		RTL8192C_HWIMG_SUPPORT					1
#define		RTL8192CE_HWIMG_SUPPORT					1
#define		RTL8192CE_TEST_HWIMG_SUPPORT			1
#else
#define		RTL8192C_SUPPORT						0
#define		RTL8192C_HWIMG_SUPPORT					0
#define		RTL8192CE_HWIMG_SUPPORT					0
#define		RTL8192CE_TEST_HWIMG_SUPPORT			0
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && !defined(_OUTSRC_COEXIST)
#define 	RTL8192D_SUPPORT						1
#else
#define 	RTL8192D_SUPPORT						0
#endif

#ifdef CONFIG_RTL_8812_SUPPORT //FOR_8812_IQK
#define		RTL8812A_SUPPORT						1
#else
#define		RTL8812A_SUPPORT						0
#endif

#ifdef CONFIG_WLAN_HAL_8881A
#define		RTL8881A_SUPPORT						1
#else
#define		RTL8881A_SUPPORT						0
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
#define		RTL8192E_SUPPORT						1
#else
#define		RTL8192E_SUPPORT						0
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
#define		RTL8814A_SUPPORT						1
#else
#define		RTL8814A_SUPPORT						0
#endif

#ifdef CONFIG_WLAN_HAL_8195A
#define		RTL8195A_SUPPORT						1
#else
#define		RTL8195A_SUPPORT						0
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
#define		RTL8822B_SUPPORT						1
#else
#define		RTL8822B_SUPPORT						0
#endif

#ifdef CONFIG_WLAN_HAL_8197F
#define		RTL8197F_SUPPORT						1
#else
#define		RTL8197F_SUPPORT						0
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
#define		RTL8723A_SUPPORT						0
#ifdef CONFIG_RTL_8723B_SUPPORT
#define		RTL8723B_SUPPORT						1		
#else
#define		RTL8723B_SUPPORT						0
#endif
#define		RTL8821A_SUPPORT						0
#define		BT_30_SUPPORT							0
#define		RTL8188F_SUPPORT						0
#define		RTL8703B_SUPPORT						0
#define		RTL8723D_SUPPORT						0
#define		RTL8821C_SUPPORT						0
#endif

//2 [HEADER\TypeDef.h]

#define IN
#define OUT

//#define	TRUE		1
//#define	FALSE		0

#define TxPwrTrk_OFDM_SwingTbl_Len						37
#define TxPwrTrk_CCK_SwingTbl_Len						23
#define TxPwrTrk_E_Val								3

#define IQK_MAC_REG_NUM								4
#define IQK_ADDA_REG_NUM							16
#define IQK_BB_REG_NUM_MAX							10
#define HP_THERMAL_NUM								8
#define	DM_Type_ByFW								0
#define	DM_Type_ByDriver							1
#define MAX_TX_COUNT								4
#define index_mapping_NUM							13
#define index_mapping_DPK_NUM							15
#define Rx_index_mapping_NUM							15
#define CCK_TABLE_SIZE_92D 							33


#define RT_MEM_SIZE_LEVEL							2		// ?
#define RT_MEM_SIZE_MINIMUM 							3		// ?

#define USE_WORKITEM 								0

//
// Customer ID, note that: 
// This variable is initiailzed through EEPROM or registry, 
// however, its definition may be different with that in EEPROM for 
// EEPROM size consideration. So, we have to perform proper translation between them.
// Besides, CustomerID of registry has precedence of that of EEPROM.
// defined below. 060703, by rcnjko.
//
typedef enum _RT_CUSTOMER_ID
{
	RT_CID_DEFAULT = 0,
	RT_CID_8187_ALPHA0 = 1,
	RT_CID_8187_SERCOMM_PS = 2,
	RT_CID_8187_HW_LED = 3,
	RT_CID_8187_NETGEAR = 4,
	RT_CID_WHQL = 5,
	RT_CID_819x_CAMEO  = 6, 
	RT_CID_819x_RUNTOP = 7,
	RT_CID_819x_Senao = 8,
	RT_CID_TOSHIBA = 9,	// Merge by Jacken, 2008/01/31.
	RT_CID_819x_Netcore = 10,
	RT_CID_Nettronix = 11,
	RT_CID_DLINK = 12,
	RT_CID_PRONET = 13,
	RT_CID_COREGA = 14,
	RT_CID_CHINA_MOBILE = 15,
	RT_CID_819x_ALPHA = 16,
	RT_CID_819x_Sitecom = 17,
	RT_CID_CCX = 18, // It's set under CCX logo test and isn't demanded for CCX functions, but for test behavior like retry limit and tx report. By Bruce, 2009-02-17.
	RT_CID_819x_Lenovo = 19,	
	RT_CID_819x_QMI = 20,
	RT_CID_819x_Edimax_Belkin = 21,		
	RT_CID_819x_Sercomm_Belkin = 22,			
	RT_CID_819x_CAMEO1 = 23,
	RT_CID_819x_MSI = 24,
	RT_CID_819x_Acer = 25,
	RT_CID_819x_AzWave_ASUS = 26,
	RT_CID_819x_AzWave = 27, // For AzWave in PCIe, The ID is AzWave use and not only Asus
	RT_CID_819x_HP = 28,
	RT_CID_819x_WNC_COREGA = 29,
	RT_CID_819x_Arcadyan_Belkin = 30,
	RT_CID_819x_SAMSUNG = 31,
	RT_CID_819x_CLEVO = 32,
	RT_CID_819x_DELL = 33,
	RT_CID_819x_PRONETS = 34,
	RT_CID_819x_Edimax_ASUS = 35,
	RT_CID_NETGEAR = 36,
	RT_CID_PLANEX = 37,
	RT_CID_CC_C = 38,
}RT_CUSTOMER_ID, *PRT_CUSTOMER_ID;


typedef enum _RF_RADIO_PATH{
	RF_PATH_A = 0,			//Radio Path A
	RF_PATH_B = 1,			//Radio Path B
	RF_PATH_C = 2,			//Radio Path C
	RF_PATH_D = 3,			//Radio Path D
	RF_PATH_MAX				//Max RF number 90 support 
}RF_RADIO_PATH_E, *PRF_RADIO_PATH_E;


// Rx smooth factor
#define	Rx_Smooth_Factor				20

typedef		struct _HAL_DATA_TYPE{
	u1Byte		temp;
} HAL_DATA_TYPE;

#define GET_HAL_DATA(pa)		(pa->temp2)

typedef		struct _RFD {
	u1Byte		temp;
} RFD, *PRT_RFD;

typedef struct _RSSI_STA{
	s4Byte	UndecoratedSmoothedPWDB;
	s4Byte	UndecoratedSmoothedCCK;
	s4Byte	UndecoratedSmoothedOFDM;
	u4Byte	OFDM_pkt;
	u1Byte	CCK_pkt;
	u2Byte	CCK_sum_power;
	u1Byte	bsend_rssi;
	u8Byte	PacketMap;
	u1Byte	ValidBit;
}RSSI_STA, *PRSSI_STA;


/* Define different debug flag for dedicated service modules in debug flag array. */
// Each module has independt 32 bit debug flag you cnn define the flag as yout require.
typedef enum tag_DBGP_Flag_Type_Definition
{
	FQoS				= 0,	
	FTX					= 1,
	FRX					= 2,	
	FSEC				= 3,
	FMGNT				= 4,
	FMLME				= 5,
	FRESOURCE			= 6,
	FBEACON				= 7,
	FISR				= 8,
	FPHY				= 9,
	FMP					= 10,
	FEEPROM				= 11,
	FPWR				= 12,
	FDM					= 13,
	FDBG_CTRL			= 14,
	FC2H				= 15,
	FBT					= 16,
	FINIT				= 17,
	FIOCTL				= 18,
	FSHORT_CUT			= 19,
	DBGP_TYPE_MAX
}DBGP_FLAG_E;


//extern u8Byte GlobalDebugComponents;

#define RTL8192DE_HWIMG_SUPPORT 0
#define RTL8192DU_HWIMG_SUPPORT 0
#define	RTL8723U_HWIMG_SUPPORT	0
#define	RTL8723S_HWIMG_SUPPORT	0

#define VISTA_USB_RX_REVISE 0


#if 0
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
#endif

 typedef enum _RF_CONTENT{
	radioa_txt = 0x1000,
	radiob_txt = 0x1001,
	radioc_txt = 0x1002,
	radiod_txt = 0x1003
} RF_CONTENT;

/*
#define REG_EDCA_VI_PARAM	EDCA_VI_PARA
#define REG_EDCA_VO_PARAM	EDCA_VO_PARA
#define REG_EDCA_BE_PARAM	EDCA_BE_PARA
#define REG_EDCA_BK_PARAM	EDCA_BK_PARA
*/

#define		bRFRegOffsetMask						0xfffff

#define 	odm_ConfigMAC_8192C(...)				{}
#define		odm_ConfigRF_RadioA_8192C(...)			{}
#define		odm_ConfigRF_RadioB_8192C(...)			{}
#define		odm_ConfigBB_PHY_8192C(...)				{}
#define		odm_ConfigBB_AGC_8192C(...)				{}

#define		PlatformStallExecution					ODM_StallExecution

#ifndef WLAN_HAL_INTERNAL_USED
#define		REG_ARFR0		ARFR0
#define		REG_ARFR1		ARFR1
#define		REG_ARFR2		ARFR2
#define		REG_ARFR3		ARFR3

#define		REG_TX_RPT_TIME		REG_88E_TXRPT_TIM
#endif //#ifndef WLAN_HAL_INTERNAL_USED

/*put the line at the end*/
#include "phydm/phydm_features.h"



#endif
