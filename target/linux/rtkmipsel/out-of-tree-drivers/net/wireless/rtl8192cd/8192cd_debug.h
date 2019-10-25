/*
 *  Debug headler file. It defines various print out method
 *
 *  $Id: 8192cd_debug.h,v 1.2.4.2 2010/12/01 13:38:00 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_DEBUG_H_
#define _8192CD_DEBUG_H_

#ifdef CONFIG_RTL_WLAN_DIAGNOSTIC
#define DIAGNOSTIC_LOG_SIZE 0x1000
extern char diag_log_buff[DIAGNOSTIC_LOG_SIZE];
extern char tmp_log[128];
#endif
#if 1
//#define	_MESH_MOD_	
#define printMac(da)	printk("%02X:%02X:%02X:%02X:%02X:%02X\n",  0xff&*(da), 0xff&*(da+1), 0xff&*(da+2), 0xff&*(da+3), 0xff&*(da+4), 0xff&*(da+5));

#define printMac4(pframe) 		{\
		 printMac(GetAddr1Ptr(pframe));\
		 printMac(GetAddr2Ptr(pframe));\
		 printMac(GetAddr3Ptr(pframe));\
		 printMac(GetAddr4Ptr(pframe));\
}


#define printHex(d,n)		{int i; \
	for(i=0; i<n; i++) 	{  printk("%02X:", *(d+i)); \
		if( i%40==39) printk("\n "); \
	} }

#endif

#ifdef _DEBUG_RTL8192CD_

extern unsigned long rtl8192cd_debug_err;		/* err flag */
extern unsigned long rtl8192cd_debug_info;	/* info flag */
extern unsigned long rtl8192cd_debug_trace;	/* trace flag */
extern unsigned long rtl8192cd_debug_warn;	/* warn flag */
#ifdef CONFIG_RTL_WLAN_DIAGNOSTIC
extern unsigned int rtl8192cd_wlan_diagnostic;/*for wlan diagnostic*/
#endif

/* Bit definition for bit31-bit8 of rtl8190_debug */
enum _module_define_ {
	_OSDEP_ =		0x00000001,
	_SME_ =			0x00000002,
	_IOCTL_ =		0x00000004,
	_TX_ =			0x00000008,
	_RX_ =			0x00000010,
	_HW_ =			0x00000020,
	_SECURITY_ =	0x00000040,
	_UTIL_ =		0x00000080,
	_TKIP_ =		0x00000100,
	_AES_ =			0x00000200,
	_HOST_ =		0x00000400,
	_BR_EXT_ =		0x00000800,
	_EEPROM_ =		0x00001000,
	_PSK_ =			0x00002000,
	_MP_ =			0x00004000,
	_MIB_ =			0x00008000,
	_LED_ =			0x00010000,
	_WPS_ =			0x00020000,	
	_DHW_ =			0x00040000,
	_HAL_ =			0x00080000,
	_DM_ =			0x00100000,
	_88E_HW_ =		0x00200000,
	_DFS_ =			0x00400000,
	_OTHER_ =		0x00800000,
	_PROC_ =		0x01000000,
	_MESH_SME_ =	0x02000000,
	_MESH_SECURITY_ =	0x04000000,
	_MESH_TX_ =		0x08000000,
	_MESH_RX_ =		0x10000000,
	_MESH_UTIL_ =	0x20000000,
	_MESH_ROUTE_ =	0x40000000,
	_DM_COM_ =		0x80000000,
};

#if defined(_8192CD_OSDEP_C_)
	#define _MODULE_DEFINE _OSDEP_
	#define _MODULE_NAME	"osdep"

#elif defined(_8192CD_SME_C_)
	#define _MODULE_DEFINE _SME_
	#define _MODULE_NAME	"sme"

#elif defined(_8192CD_IOCTL_C_)
	#define _MODULE_DEFINE _IOCTL_
	#define _MODULE_NAME	"ioctl"

#elif defined(_8192CD_PROC_C_)
	#define _MODULE_DEFINE _PROC_
	#define _MODULE_NAME	"proc"

#elif defined(_8192CD_TX_C_)
	#define _MODULE_DEFINE _TX_
	#define _MODULE_NAME	"tx"

#elif defined(_8192CD_RX_C_)
	#define _MODULE_DEFINE _RX_
	#define _MODULE_NAME	"rx"

#elif defined(_8192CD_HW_C_)
	#define _MODULE_DEFINE _HW_
	#define _MODULE_NAME	"hw"

#elif defined(_8192D_HW_C_)
	#define _MODULE_DEFINE _DHW_
	#define _MODULE_NAME	"dhw"

#elif defined(_8192CD_SECURITY_C_)
	#define _MODULE_DEFINE _SECURITY_
	#define _MODULE_NAME	"security"

#elif defined(_8192CD_UTILS_C_)
	#define _MODULE_DEFINE _UTIL_
	#define _MODULE_NAME	"util"

#elif defined(_8192CD_TKIP_C_)
	#define _MODULE_DEFINE _TKIP_
	#define _MODULE_NAME	"tkip"

#elif defined(_8192CD_AES_C_)
	#define _MODULE_DEFINE _AES_
	#define _MODULE_NAME	"aes"

#elif defined(_8192CD_BR_EXT_C_)
	#define _MODULE_DEFINE _BR_EXT_
	#define _MODULE_NAME	"br_ext"

#elif defined(_8192CD_EEPROM_C_)
	#define _MODULE_DEFINE _EEPROM_
	#define _MODULE_NAME	"eeprom"

#elif defined(_8192CD_PSK_C_)
	#define _MODULE_DEFINE _PSK_
	#define _MODULE_NAME	"psk"

#elif defined(_8192CD_MP_C_)
	#define _MODULE_DEFINE _MP_
	#define _MODULE_NAME	"mp"

#elif defined(_8192CD_MIB_C_)
	#define _MODULE_DEFINE _MIB_
	#define _MODULE_NAME	"mib"

#elif defined(_8192CD_DMEM_C_)
	//not yet

#elif defined(_HAL8192CDM_C_)
	#define _MODULE_DEFINE _HAL_
	#define _MODULE_NAME	"hal"

#elif defined(_8192CD_A4_STA_C_)
	#define _MODULE_DEFINE _A4STA_
	#define _MODULE_NAME	"a4_sta"

#elif defined(_8192CD_WSCD_C_)
	#define _MODULE_DEFINE _WPS_
	#define _MODULE_NAME	"wps"

#elif defined(_MESH_SME_C_)
	#define _MODULE_DEFINE _MESH_SME_
	#define _MODULE_NAME	"mesh_sme"

#elif defined(_MESH_TX_C_)
	#define _MODULE_DEFINE _MESH_TX_
	#define _MODULE_NAME	"mesh_tx"
	
#elif defined(_MESH_RX_C_)
	#define _MODULE_DEFINE _MESH_RX_
	#define _MODULE_NAME	"mehs_rx"

#elif defined(_MESH_SECURITY_C_)
	#define _MODULE_DEFINE _MESH_SECURITY_
	#define _MODULE_NAME	"mesh_secutiry"

#elif defined(_MESH_UTILS_C_)
	#define _MODULE_DEFINE _MESH_UTIL_
	#define _MODULE_NAME	"mesh_util"

#elif defined(_MESH_ROUTE_C_)
	#define _MODULE_DEFINE _MESH_ROUTE_
	#define _MODULE_NAME	"mesh_route"

#elif defined(_8192CD_PWRCTRL_C_)
	#define _MODULE_DEFINE _PWR_CTRL_
	#define _MODULE_NAME	"pwr_ctrl"

#elif defined(_MESH_PROC_C_)
	#define _MODULE_DEFINE _MESH_PROC_
	#define _MODULE_NAME	"mesh_proc"

#elif defined(_MESH_11KV_C_)
	//not yet
	
#elif defined(_HAL8192CDM_C_)
	#define _MODULE_DEFINE _DM_
	#define _MODULE_NAME    "DM"

#elif defined(_8188E_HW_C_)
	#define _MODULE_DEFINE _88E_HW_
	#define _MODULE_NAME    "88E_hw"

#elif defined(_HALDM_COMMON_C_)
	#define _MODULE_DEFINE _DM_COM_
	#define _MODULE_NAME	"DM_COM"

#elif defined(_8812_HW_C_)
	#define _MODULE_DEFINE _OTHER_
    #define _MODULE_NAME    "8812_hw"
	
#elif defined(_8192CD_HOST_C_)
	#define _MODULE_DEFINE _HOST_
	#define _MODULE_NAME	"host"

#elif defined(_8192CD_LED_C_)
	#define _MODULE_DEFINE _LED_
	#define _MODULE_NAME	"led"

#elif defined(_8192CD_DFS_C_)
	#define _MODULE_DEFINE _DFS_
	#define _MODULE_NAME	"dfs"

#else
	#define _MODULE_DEFINE _OTHER_
	#define _MODULE_NAME	"other"

#endif

/* Macro for DEBUG_ERR(), DEBUG_TRACE(), DEBUG_WARN(), DEBUG_INFO() */

#ifdef __GNUC__
#ifdef CONFIG_RTL8671
#define DEBUG_ERR		printk
#define DEBUG_TRACE		printk
#define DEBUG_INFO		printk
#define DEBUG_WARN		printk

#define _DEBUG_ERR		printk
#define _DEBUG_INFO		printk

#define DBFENTER
#define DBFEXIT
#define PRINT_INFO		printk

#else
#ifdef CONFIG_RTL_WLAN_DIAGNOSTIC
#define output_diag_log(log)\
if(rtl8192cd_wlan_diagnostic){\
	if(strlen(diag_log_buff) >= DIAGNOSTIC_LOG_SIZE)\
		memset(diag_log_buff,0,DIAGNOSTIC_LOG_SIZE);\
	if(diag_log_buff[0]=='0')\
		strcpy(diag_log_buff,log);\
	else\
		strcat(diag_log_buff,log);}\
else{\
	printk(log);}
		
#define __DEBUG_ERR(name, fmt, args...) \
	if (rtl8192cd_debug_err&_MODULE_DEFINE) {\
		sprintf(tmp_log,"%s-"_MODULE_NAME"-err: " fmt, name, ## args);\
		output_diag_log(tmp_log)};
#define __DEBUG_TRACE(name) \
	if (rtl8192cd_debug_trace&_MODULE_DEFINE) {\
		sprintf(tmp_log,"%s-"_MODULE_NAME"-trace: %s----->\n", name, (char *)__FUNCTION__);\
		output_diag_log(tmp_log);}
#define __DEBUG_INFO(name, fmt, args...) \
	if (rtl8192cd_debug_info&_MODULE_DEFINE){ \
		sprintf(tmp_log,"%s-"_MODULE_NAME"-info: " fmt, name, ## args);\
		output_diag_log(tmp_log);}
#define __DEBUG_WARN(name, fmt, args...) \
	if (rtl8192cd_debug_warn&_MODULE_DEFINE){ \
		sprintf(tmp_log,"%s-"_MODULE_NAME"-warn: " fmt, name, ## args);\
		output_diag_log(tmp_log);}
#else
#define __DEBUG_ERR(name, fmt, args...) \
	if (rtl8192cd_debug_err&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-err: " fmt, name, ## args);
#define __DEBUG_TRACE(name) \
	if (rtl8192cd_debug_trace&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-trace: %s----->\n", name, (char *)__FUNCTION__);
#define __DEBUG_INFO(name, fmt, args...) \
	if (rtl8192cd_debug_info&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-info: " fmt, name, ## args);
#define __DEBUG_WARN(name, fmt, args...) \
	if (rtl8192cd_debug_warn&_MODULE_DEFINE) \
		printk("%s-"_MODULE_NAME"-warn: " fmt, name, ## args);
#endif
#define DEBUG_ERR(fmt, args...)		__DEBUG_ERR(priv->dev->name, fmt, ## args)
#define DEBUG_INFO(fmt, args...)	__DEBUG_INFO(priv->dev->name, fmt, ## args)
#define DEBUG_TRACE					__DEBUG_TRACE(priv->dev->name)
#define DEBUG_WARN(fmt, args...)	__DEBUG_WARN(priv->dev->name, fmt, ## args)

#define _DEBUG_ERR(fmt, args...)	__DEBUG_ERR("wlan", fmt, ## args)
#define _DEBUG_INFO(fmt, args...)	__DEBUG_INFO("wlan", fmt, ## args)
#define _DEBUG_TRACE				__DEBUG_TRACE("wlan")
#define _DEBUG_WARN(fmt, args...)	__DEBUG_WARN("wlan", fmt, ## args)

#define DBFENTER	printk("----->%s\n", (char *)__FUNCTION__)
#define DBFEXIT		printk("%s----->\n", (char *)__FUNCTION__)
#define PRINT_INFO(fmt, args...)	printk(fmt, ## args)
#endif
#endif	// __GNUC__

/*
#ifdef __DRAYTEK_OS__
#define __FUNCTION__	""

#define DEBUG_ERR		Print
#define DEBUG_INFO		Print
#define DEBUG_TRACE
#define DEBUG_WARN		Print

#define _DEBUG_ERR		DEBUG_ERR
#define _DEBUG_INFO		DEBUG_INFO
#define _DEBUG_TRACE	DEBUG_TRACE
#define _DEBUG_WARN		DEBUG_WARN

#define DBFENTER
#define DBFEXIT
#define PRINT_INFO		Print
#endif // __DRAYTEK_OS__

#ifdef GREEN_HILL
#define DEBUG_ERR		printk
#define DEBUG_INFO		printk
#define DEBUG_TRACE		printk
#define DEBUG_WARN		printk

#define _DEBUG_ERR		printk
#define _DEBUG_INFO		printk
#define _DEBUG_TRACE	printk
#define _DEBUG_WARN		printk

#define DBFENTER		printk
#define DBFEXIT			printk
#define PRINT_INFO		printk
#endif // GREEN_HILL
*/


#else // not _DEBUG_RTL8192CD_


#ifdef __GNUC__
#define DEBUG_ERR(fmt, args...) {}
#define DEBUG_INFO(fmt, args...) {}
#define DEBUG_TRACE {}
#define DEBUG_WARN(fmt, args...) {}

#define _DEBUG_ERR(fmt, args...) {}
#define _DEBUG_INFO(fmt, args...) {}
#define _DEBUG_TRACE {}
#define _DEBUG_WARN(fmt, args...) {}

#define DBFENTER	{}
#define DBFEXIT		{}
#define PRINT_INFO(fmt, args...)	{}
#endif // __GNUC__

/*
#ifdef __DRAYTEK_OS__
#define __FUNCTION__	""

#define DEBUG_ERR
#define DEBUG_INFO
#define DEBUG_TRACE
#define DEBUG_WARN

#define _DEBUG_ERR
#define _DEBUG_INFO
#define _DEBUG_TRACE
#define _DEBUG_WARN

#define DBFENTER
#define DBFEXIT
#define PRINT_INFO
#endif // __DRAYTEK_OS__

#ifdef GREEN_HILL
#define DEBUG_ERR(fmt, args...) {}
#define DEBUG_INFO(fmt, args...) {}
#define DEBUG_TRACE {}
#define DEBUG_WARN(fmt, args...) {}

#define _DEBUG_ERR(fmt, args...) {}
#define _DEBUG_INFO(fmt, args...) {}
#define _DEBUG_TRACE {}
#define _DEBUG_WARN(fmt, args...) {}

#define DBFENTER	{}
#define DBFEXIT		{}
#define PRINT_INFO(fmt, args...)	{}
#endif // GREEN_HILL
*/


#endif // _DEBUG_RTL8192CD_
#endif // _8192CD_DEBUG_H_

