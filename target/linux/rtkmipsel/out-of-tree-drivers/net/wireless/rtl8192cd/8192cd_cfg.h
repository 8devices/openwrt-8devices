/*
 *	 Headler file defines some configure options and basic types
 *
 *	 $Id: 8192cd_cfg.h,v 1.59.2.26 2011/01/10 07:49:07 jerryko Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_CFG_H_
#define _8192CD_CFG_H_

#if defined(CONFIG_RTL_ULINKER_BRSC)
#include "linux/ulinker_brsc.h"
#endif

#ifdef __OSK__
	#if !defined(CONFIG_RTL_819X) && !defined(CONFIG_RTL8672)
		#define CONFIG_RTL8672
	#endif
	#define USE_RTL8186_SDK
	#define CONFIG_RTL8196C
	#define CONFIG_RTL_8196C
#endif

//#define _LITTLE_ENDIAN_
//#define _BIG_ENDIAN_

//this is for WLAN HAL driver coexist with not HAL driver for code size reduce
#ifdef CONFIG_RTL_WLAN_HAL_NOT_EXIST
#define CONFIG_WLAN_NOT_HAL_EXIST 1
#else
#define CONFIG_WLAN_NOT_HAL_EXIST 0//96e_92e, 8881a_92e, 8881a_only, 96d_92er, is only HAL driver
#endif

#ifdef __MIPSEB__

#ifndef _BIG_ENDIAN_
	#define _BIG_ENDIAN_
#endif

#ifdef _LITTLE_ENDIAN_
#undef _LITTLE_ENDIAN_
#endif
//### add by sen_liu 2011.4.14 CONFIG_NET_PCI defined in V2.4 and CONFIG_PCI
// define now to replace it. However,some modules still use CONFIG_NET_PCI
#ifdef CONFIG_PCI
#define CONFIG_NET_PCI
#endif

//### end
#endif	//__MIPSEB__


#ifdef __MIPSEL__
#ifndef _LITTLE_ENDIAN_
	#define _LITTLE_ENDIAN_
#endif

#ifdef _BIG_ENDIAN_
#undef _BIG_ENDIAN_
#endif

#endif //__MIPSEL__



#ifdef __KERNEL__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= 0x020614 // linux 2.6.20
	#define LINUX_2_6_20_
#endif

#if LINUX_VERSION_CODE >= 0x020615 // linux 2.6.21
	#define LINUX_2_6_21_
#endif

#if LINUX_VERSION_CODE >= 0x020616 // linux 2.6.22
	#define LINUX_2_6_22_
#endif

#if LINUX_VERSION_CODE >= 0x020618 // linux 2.6.24
	#define LINUX_2_6_24_
#endif

#if LINUX_VERSION_CODE >= 0x02061B // linux 2.6.27
	#define LINUX_2_6_27_
#endif

#if LINUX_VERSION_CODE >= 0x02061D // linux 2.6.29
	#define LINUX_2_6_29_
#endif

#if LINUX_VERSION_CODE > 0x020600
	#define __LINUX_2_6__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
	#define __LINUX_3_2__
	#define CONFIG_RTL_PROC_NEW
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
	#define __LINUX_3_4__
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	#define __LINUX_3_10__
	#define CONFIG_RTL_PROC_NEW
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,21))
	#define __LINUX_3_18_21__
#endif

#if defined(LINUX_2_6_20_) || defined(__LINUX_3_4__)
#ifdef CPTCFG_CFG80211_MODULE
#if (LINUX_VERSION_CODE == KERNEL_VERSION(3,18,21))
#include "../../../../../linux-3.18.21/include/generated/autoconf.h"
#elif (LINUX_VERSION_CODE == KERNEL_VERSION(3,10,49))
#include "../../../../../linux-3.10.49/include/generated/autoconf.h"
#endif
#else //CPTCFG_CFG80211_MODULE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
#include <linux/kconfig.h>
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#endif //CPTCFG_CFG80211_MODULE
#include <linux/jiffies.h>
#include <asm/param.h>
#else
#include <linux/config.h>
#endif
#endif // __KERNEL__

//-------------------------------------------------------------
// Type definition
//-------------------------------------------------------------
#include "typedef.h"

#ifdef __ECOS
	#include <pkgconf/system.h>
	#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
	#include <pkgconf/devs_eth_rltk_819x_wlan.h>
	#include <sys/param.h>
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
    #define CONFIG_RTL8190_PRIV_SKB
#endif
#endif
#ifdef LINUX_2_6_29_ // linux 2.6.29
#define NETDEV_NO_PRIV
#endif

/*
 *	Following Definition Sync 2.4/2.6 SDK Definitions
 */

#if !defined(__LINUX_2_6__) && !defined(__ECOS) && !defined (__OSK__)

#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL8196C) || defined(CONFIG_RTL8198)
#define CONFIG_RTL_819X
#endif
#if defined(CONFIG_RTL8198)
#define CONFIG_RTL_8198
#endif
#if defined(CONFIG_RTL8196C)
#define CONFIG_RTL_8196C
#endif

#define BSP_PCIE0_D_CFG0 	PCIE0_D_CFG0
#define BSP_PCIE1_D_CFG0 	PCIE0_D_CFG0
#define BSP_PCIE0_H_CFG 	PCIE0_H_CFG
#define BSP_PCIE0_H_EXT 	PCIE0_RC_EXT_BASE
#define BSP_PCIE0_H_MDIO	(BSP_PCIE0_H_EXT + 0x00)
#define BSP_PCIE0_H_INTSTR	(BSP_PCIE0_H_EXT + 0x04)
#define BSP_PCIE0_H_PWRCR	(BSP_PCIE0_H_EXT + 0x08)
#define BSP_PCIE0_H_IPCFG	(BSP_PCIE0_H_EXT + 0x0C)
#define BSP_PCIE0_H_MISC	(BSP_PCIE0_H_EXT + 0x10)
#define BSP_PCIE1_H_CFG 	PCIE1_H_CFG
#define BSP_PCIE1_H_EXT 	PCIE1_RC_EXT_BASE
#define BSP_PCIE1_H_MDIO	(BSP_PCIE1_H_EXT + 0x00)
#define BSP_PCIE1_H_INTSTR	(BSP_PCIE1_H_EXT + 0x04)
#define BSP_PCIE1_H_PWRCR	(BSP_PCIE1_H_EXT + 0x08)
#define BSP_PCIE1_H_IPCFG	(BSP_PCIE1_H_EXT + 0x0C)
#define BSP_PCIE1_H_MISC	(BSP_PCIE1_H_EXT + 0x10)

#endif // !defined(__LINUX_2_6__)

#ifdef __ECOS
//add macro for Realsil WLAN modification
//move the macro definition to cdl file now
//#ifndef CONFIG_RTL_819X_ECOS
//#define CONFIG_RTL_819X_ECOS
//#endif
#endif

#if !defined(__ECOS) && !(defined(CONFIG_OPENWRT_SDK) && defined(__LINUX_3_10__)) && !defined(__OSK__) //eric-sync ??
#define CONFIG_RTL_CUSTOM_PASSTHRU
#endif

#ifdef CONFIG_RTL_819X_ECOS
#define CONFIG_RTL_CUSTOM_PASSTHRU
#define CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
//#define CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE

#define IP6_PASSTHRU_MASK 0x1
#if	defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
#define PPPOE_PASSTHRU_MASK 0x1<<1
#endif

//#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)||defined (CONFIG_RTL_8881A)
//dual band
#if (defined (CONFIG_USE_PCIE_SLOT_0)&&defined (CONFIG_USE_PCIE_SLOT_1))||( defined(CONFIG_RTL_8881A)&& !defined(CONFIG_RTL_8881A_SELECTIVE))|| defined(CONFIG_WLAN_HAL_8814AE) || defined (CONFIG_RTL_8197F)
#define WISP_WLAN_IDX_MASK 0xF0
#define WISP_WLAN_IDX_RIGHT_SHIFT 4
#endif

#endif	/* CONFIG_RTL_CUSTOM_PASSTHRU	*/

#if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
#if defined(CONFIG_RTL_92D_SUPPORT)
#define CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D 1
#else
#define CONFIG_RTL_DUAL_PCIESLOT_BIWLAN 1
#endif
#endif

#if defined(CONFIG_RTL_92D_DMDP) \
||  defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)\
|| (defined(CONFIG_USE_PCIE_SLOT_0)  && defined(CONFIG_WLAN_HAL_8881A)) \
|| (defined(CONFIG_USE_PCIE_SLOT_0)  && defined(CONFIG_RTL_8197F)) \
|| (defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1))
#define CONCURRENT_MODE

#if defined(CONFIG_RTL_92D_DMDP) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) && !defined(CONFIG_RTL_92C_SUPPORT) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
#define DUALBAND_ONLY
#endif
#endif


#ifdef CONFIG_RTK_VOIP_BOARD
	#if defined(CONFIG_RTL_8198C)
	#define RTL_MAX_PCIE_SLOT_NUM   2
	#else
	#define	RTL_MAX_PCIE_SLOT_NUM	1
	#endif
	#define	RTL_USED_PCIE_SLOT		0
#elif defined(CONFIG_RTL_8198)
	#define	RTL_MAX_PCIE_SLOT_NUM	2
	#ifdef CONFIG_SLOT_0_92D
		#define RTL_USED_PCIE_SLOT	0
	#else
		#define	RTL_USED_PCIE_SLOT	1
	#endif
#elif defined(CONFIG_RTL_8197D) || defined(CONFIG_RTL_8197DL) || defined(CONFIG_RTL_8198C)
	#define	RTL_MAX_PCIE_SLOT_NUM	2
	#ifdef CONFIG_SLOT_0_92D
		#define RTL_USED_PCIE_SLOT	0
	#elif defined(CONFIG_USE_PCIE_SLOT_1)
		#define	RTL_USED_PCIE_SLOT	1
	#else
		#define RTL_USED_PCIE_SLOT	0
	#endif
#else
	#define	RTL_MAX_PCIE_SLOT_NUM	1
	#define	RTL_USED_PCIE_SLOT		0
#endif

#ifdef __ECOS
#if defined(CONFIG_RTL_MESH_SUPPORT)
#define CONFIG_RTK_MESH
#endif
#endif

#ifdef CONFIG_RTK_MESH
#include "./mesh_ext/mesh_cfg.h"
#define RTL_MESH_TXCACHE
#define RTK_MESH_MANUALMETRIC
#define RTK_MESH_AODV_STANDALONE_TIMER
//#define RTK_MESH_REDIRECT_TO_ROOT
#define RTK_MESH_REMOVE_PATH_AFTER_AODV_TIMEOUT

#if defined(CONFIG_RTL_MESH_METRIC_REFINE)
#define RTK_MESH_METRIC_REFINE
#endif

#endif

#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
	#if defined(CONFIG_RTL8196B_AP_ROOT) || defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196B_GW) || defined(CONFIG_RTL_8196C_GW) || defined(CONFIG_RTL_8198_GW) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196B_TLD) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8196C_AP_HCM) || defined(CONFIG_RTL8198_AP_ROOT) || defined(CONFIG_RTL_8198_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY) || defined(CONFIG_RTL_8198_NFBI_BOARD) || defined(CONFIG_RTL8196C_KLD) || defined(CONFIG_RTL8196C_EC) || defined(CONFIG_RTL_8196C_iNIC) || defined(CONFIG_RTL_8198_INBAND_AP) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B) || defined(__ECOS) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		#define USE_RTL8186_SDK
	#endif
#endif

#if !defined(_BIG_ENDIAN_) && !defined(_LITTLE_ENDIAN_)
	#error "Please specify your ENDIAN type!\n"
#elif defined(_BIG_ENDIAN_) && defined(_LITTLE_ENDIAN_)
	#error "Only one ENDIAN should be specified\n"
#endif

#define PCI_CONFIG_COMMAND			(wdev->conf_addr+4)
#define PCI_CONFIG_LATENCY			(wdev->conf_addr+0x0c)
#define PCI_CONFIG_BASE0			(wdev->conf_addr+0x10)
#define PCI_CONFIG_BASE1			(wdev->conf_addr+0x18)

#define MAX_NUM(_x_, _y_)	(((_x_)>(_y_))? (_x_) : (_y_))
#define MIN_NUM(_x_, _y_)	(((_x_)<(_y_))? (_x_) : (_y_))

#define POWER_MIN_CHECK(a,b)            (((a) > (b)) ? (b) : (a))
#define POWER_MIN_INDEX(a)            (((a) > 0) ? (a) : 0)
#define POWER_RANGE_CHECK(val)		(((val) > 0x3f)? 0x3f : ((val < 0) ? 0 : val))
#define COUNT_SIGN_OFFSET(val, oft)	(((oft & 0x08) == 0x08)? (val - (0x10 - oft)) : (val + oft))

//-------------------------------------------------------------
// Driver version information
//-------------------------------------------------------------
#define DRV_VERSION_H	1
#define DRV_VERSION_L	7
#define DRV_RELDATE		"2015-10-30"
#define DRV_NAME		"Realtek WLAN driver"


//-------------------------------------------------------------
// Will check type for endian issue when access IO and memory
//-------------------------------------------------------------
#define CHECK_SWAP


//-------------------------------------------------------------
// Defined when include proc file system
//-------------------------------------------------------------
#define INCLUDE_PROC_FS
#if defined(CONFIG_PROC_FS) && defined(INCLUDE_PROC_FS)
	#define _INCLUDE_PROC_FS_
#endif


//-------------------------------------------------------------
// Debug function
//-------------------------------------------------------------
//#define _DEBUG_RTL8192CD_		// defined when debug print is used
#define _IOCTL_DEBUG_CMD_		// defined when read/write register/memory command is used in ioctl
//#define _AMPSDU_AMSDU_DEBUG_		// defined when debug for ampdu_amsdu

#ifdef CONFIG_BT_REPEATER_CONFIG
//-------------------------------------------------------------
// defined for client mode associate to remote AP with wrong PSK/WEP shared key,
//the user can get the auth fail reason by ioctl (password error)
//-------------------------------------------------------------
#define CONFIG_AUTH_RESULT
//-------------------------------------------------------------
// defined for realtek bluetooth repeater config, to make driver mib take effect immediately
//-------------------------------------------------------------
#define CONFIG_RTK_BTCONFIG
#endif
//-------------------------------------------------------------
// Defined when internal DRAM is used for sw encryption/decryption
//-------------------------------------------------------------
#ifdef __MIPSEB__
	// disable internal ram for nat speedup
	//#define _USE_DRAM_
#endif

#ifdef CONFIG_RTL_WLAN_DIAGNOSTIC
#define _DEBUG_RTL8192CD_
#endif

//-------------------------------------------------------------
// Support 8188C/8192C test chip
//-------------------------------------------------------------
#if !defined(CONFIG_RTL8196B_GW_8M) && !defined(__OSK__) && !defined(__ECOS)//&&!defined(CONFIG_RTL_92D_SUPPORT)
#define TESTCHIP_SUPPORT
#endif

//-------------------------------------------------------------
// Support software tx queue
// ------------------------------------------------------------
#ifdef CONFIG_PCI_HCI
#define SW_TX_QUEUE
#endif

//-------------------------------------------------------------
// Romeperf support
//-------------------------------------------------------------
#ifdef CONFIG_RTL_ROMEPERF_24K
#define ROMEPERF_SUPPORT
#endif

//-------------------------------------------------------------
// Support RTK ATM
// ------------------------------------------------------------
#if defined(CONFIG_RTL_ATM_SUPPORT)
#define RTK_ATM
#endif

//-------------------------------------------------------------
// RF MIMO SWITCH
// ------------------------------------------------------------
#ifdef CONFIG_PCI_HCI
#define RF_MIMO_SWITCH
#define IDLE_T0			(10*HZ)
#endif

//-------------------------------------------------------------
// Support Tx Report
//-------------------------------------------------------------
#define TXREPORT
#ifdef TXREPORT
#define DETECT_STA_EXISTANCE
#define TXRETRY_CNT
#endif
//#define LEAVESTADETECT

//-------------------------------------------------------------
// Refine for multi-station
//-------------------------------------------------------------
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
#define MULTI_STA_REFINE
#endif
#ifdef MULTI_STA_REFINE
#define PKTAGINGTIME 1200
#define MAXPAUSEDSTA 24
#define MAXPAUSEDQUEUE 7
#define LOWAGGRESTA	8
#define STA_TX_LOWRATE ((txcfg->pstat->ht_cap_buf.support_mcs[2])? _MCS8_RATE_:_MCS4_RATE_)
#endif

//-------------------------------------------------------------
// Support sta rate statistic
//-------------------------------------------------------------
//#define STA_RATE_STATISTIC
#ifdef STA_RATE_STATISTIC
#define CCK_OFDM_RATE_NUM (4+8)//CCK:4 OFDM:8
#define STA_RATE_NUM (CCK_OFDM_RATE_NUM+HT_RATE_NUM+VHT_RATE_NUM)//CCK_OFDM+HT+VHT
#endif

//#define STA_ASSOC_STATISTIC

//-------------------------------------------------------------
// PCIe power saving function
//-------------------------------------------------------------
//#define GPIO_WAKEPIN //yllin
//#define PCIE_PME_WAKEUP_TEST //yllin
//#define PCIE_POWER_SAVING_TEST //yllin


#ifdef CONFIG_PCIE_POWER_SAVING
#if !defined(CONFIG_NET_PCI) && !defined(CONFIG_RTL_8196CS) &&  !defined(CONFIG_RTL_88E_SUPPORT)
#define PCIE_POWER_SAVING
#endif

#endif

#ifdef _SINUX_
#define PCIE_POWER_SAVING
#endif

#ifdef PCIE_POWER_SAVING
	#define GPIO_WAKEPIN
	#define FIB_96C
//	#define PCIE_POWER_SAVING_DEBUG
//	#define ASPM_ENABLE
#ifdef PCIE_POWER_SAVING_DEBUG
	#define PCIE_L2_ENABLE
#endif

#define CONFIG_SLOT0H	0xb8b00000
#define CONFIG_SLOT0S	0xb8b10000
#define CONFIG_SLOT1H	0xb8b20000
#define CONFIG_SLOT1S	0xb8b30000


#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#define CFG_92C_SLOTH		CONFIG_SLOT0H
#define CFG_92C_SLOTS		CONFIG_SLOT0S
#if (RTL_USED_PCIE_SLOT==1)
#define CFG_92D_SLOTH		CONFIG_SLOT1H
#define CFG_92D_SLOTS		CONFIG_SLOT1S
#else
#define CFG_92D_SLOTH		CONFIG_SLOT0H
#define CFG_92D_SLOTS		CONFIG_SLOT0S
#endif
#elif defined(CONFIG_RTL_8196C)
#define CFG_92C_SLOTH		CONFIG_SLOT0H
#define CFG_92C_SLOTS		CONFIG_SLOT0S
#define CFG_92D_SLOTH		CONFIG_SLOT0H
#define CFG_92D_SLOTS		CONFIG_SLOT0S
#endif


#endif


//-------------------------------------------------------------
// WDS function support
//-------------------------------------------------------------
#if defined(CONFIG_RTL_WDS_SUPPORT)
#define WDS
//	#define LAZY_WDS
#endif


//-------------------------------------------------------------
// Pass EAP packet by event queue
//-------------------------------------------------------------
#define EAP_BY_QUEUE
#undef EAPOLSTART_BY_QUEUE	// jimmylin: don't pass eapol-start up
							// due to XP compatibility issue
//#define USE_CHAR_DEV
#define USE_PID_NOTIFY

#ifdef CONFIG_PACP_SUPPORT
#define SUPPORT_MONITOR			// for packet capture function
//#define CONFIG_PKT_FILTER
#if !defined(CONFIG_RTL_COMAPI_WLTOOLS) && !defined(__ECOS)
#define CONFIG_RTL_COMAPI_WLTOOLS
#endif
#endif

//-------------------------------------------------------------
// WPA Supplicant
//-------------------------------------------------------------
//#define WIFI_WPAS

#ifdef WIFI_WPAS
#ifndef WIFI_HAPD
#define WIFI_HAPD
#endif

#ifndef CLIENT_MODE
#define CLIENT_MODE
#endif
#endif

#ifdef WIFI_WPAS_CLI
#define WIFI_WPAS
#ifndef CLIENT_MODE
#define CLIENT_MODE
#endif
#endif

//-------------------------------------------------------------
// Client mode function support
//-------------------------------------------------------------
#if defined(CONFIG_RTL_CLIENT_MODE_SUPPORT)
#define CLIENT_MODE
#endif

#ifdef CLIENT_MODE
	#define RTK_BR_EXT		// Enable NAT2.5 and MAC clone support
	#define CL_IPV6_PASS	// Enable IPV6 pass-through. RTK_BR_EXT must be defined
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	#undef RTK_BR_EXT
	#undef CL_IPV6_PASS
#endif
#endif

#if defined(CONFIG_RTL_MULTI_CLONE_SUPPORT)
	#define MULTI_MAC_CLONE // Enable mac clone to multiple Ethernet MAC address
#endif
#ifdef MULTI_MAC_CLONE
#define REPEATER_TO		RTL_SECONDS_TO_JIFFIES(10)
#endif

#ifdef CONFIG_RTL_SUPPORT_MULTI_PROFILE
	#define SUPPORT_MULTI_PROFILE		// support multiple AP profile
#endif


//-------------------------------------------------------------
// Defined when WPA2 is used
//-------------------------------------------------------------
#define RTL_WPA2
#define RTL_WPA2_PREAUTH

//-------------------------------------------------------------
// MCR test
//-------------------------------------------------------------
//#define MCR_WIRELESS_EXTEND


//-------------------------------------------------------------
// MP test
//-------------------------------------------------------------
#if (!defined(CONFIG_RTL8196B_GW_8M) || defined(CONFIG_RTL8196B_GW_MP))
#define MP_TEST
#ifdef CONFIG_RTL_92D_SUPPORT
#endif
#endif


//-------------------------------------------------------------
// MIC error test
//-------------------------------------------------------------
//#define MICERR_TEST


//-------------------------------------------------------------
// Log event
//-------------------------------------------------------------
#define EVENT_LOG


//-------------------------------------------------------------
// Tx/Rx data path shortcut
//-------------------------------------------------------------
#define TX_SHORTCUT
#define RX_SHORTCUT
#if defined(CONFIG_RTK_MESH) && defined(RX_SHORTCUT)
#define RX_RL_SHORTCUT
#endif
#ifdef __ECOS
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_BRSC
#define BR_SHORTCUT
#endif
#elif !defined(CONFIG_RTL_FASTBRIDGE)
#ifdef CONFIG_RTL8672
#ifndef CONFIG_RTL8672_BRIDGE_FASTPATH
//#define BR_SHORTCUT
#endif
#else
#if !defined(CONFIG_RTL_NO_BR_SHORTCUT)
//#ifndef CONFIG_RPS
//#define BR_SHORTCUT //mark_apo
//#endif
#endif
#if 0	//jwj
#if !defined(CONFIG_RTL_8198B) && !defined(CONFIG_RTL8196C_KLD)
#define BR_SHORTCUT_C2
#define BR_SHORTCUT_C3
#define BR_SHORTCUT_C4
#endif
#endif
#endif
#endif
#if defined(CONFIG_RTK_MESH) && defined(TX_SHORTCUT)
	#define MESH_TX_SHORTCUT
#endif

#define TX_SC_ENTRY_NUM		4
#define RX_SC_ENTRY_NUM		4

//Filen
//#define SHORTCUT_STATISTIC

// Tx path finetune
#define TXSC_CFG
//#define TXSC_HDR
#define TXSC_SKBLEN
//#define TXSC_CACHE_FRLEN
#define TXSC_CMPTXBD

#define TXDESC_INFO
#define TX_LOWESTRATE
//-------------------------------------------------------------
// Mininal Memory usage
//-------------------------------------------------------------
#if defined(CONFIG_MEM_LIMITATION) || defined(CONFIG_RTL_NFJROM_MP)
#define WIFI_LIMITED_MEM
#endif

#ifdef WIFI_LIMITED_MEM
#define WIFI_MIN_IMEM_USAGE	// Mininal IMEM usage
#undef TESTCHIP_SUPPORT
#endif

#if defined(CONFIG_RTL_8196E)
#define WIFI_MIN_IMEM_USAGE	// Mininal IMEM usage
#endif


//-------------------------------------------------------------
// back to back test
//-------------------------------------------------------------
//#define B2B_TEST


//-------------------------------------------------------------
// enable e-fuse read write
//-------------------------------------------------------------
#ifdef CONFIG_ENABLE_EFUSE
#define EN_EFUSE
#endif


//-------------------------------------------------------------
// new Auto channel
//-------------------------------------------------------------
#define CONFIG_RTL_NEW_AUTOCH
#define MAC_RX_COUNT_THRESHOLD		200
#define MAX_NUM_20M_CH			28 // 5G
//#define NHM_ACS2_SUPPORT
#define ACS_CCA_CAP_MAX				18
#define MAX_UP_RESOLUTION 			6
#define IGI_TO_NHM_TH_MULTIPLIER 		2
#define CLM_SAMPLE_NUM				50000

#ifdef CONFIG_SDIO_HCI
#define AUTOCH_SS_SPEEDUP
#endif

//-------------------------------------------------------------
// new IQ calibration for 92c / 88c
//-------------------------------------------------------------
//#define PERIODIC_AUTO_CHANNEL
//#define DYNAMIC_AUTO_CHANNEL//SW#5
//#define PERIODIC_AUTO_CHANNEL_DEBUG
#define CONFIG_RTL_NEW_IQK


//-------------------------------------------------------------
// noise control
//-------------------------------------------------------------
#ifdef CONFIG_RTL_92C_SUPPORT
//#define CONFIG_RTL_NOISE_CONTROL_92C
#endif


//-------------------------------------------------------------
// Universal Repeater (support AP + Infra client concurrently)
//-------------------------------------------------------------
#if defined(CONFIG_RTL_REPEATER_MODE_SUPPORT)
#define UNIVERSAL_REPEATER
#define SMART_REPEATER_MODE
#define	SWITCH_CHAN
#define CASE6_VAP_INDEX 0
#endif

#if defined(CONFIG_RTL_CROSSBAND_REPEATER_SUPPORT)
#define CROSSBAND_REPEATER
#endif

//-------------------------------------------------------------
// Check hangup for Tx queue
//-------------------------------------------------------------
#ifdef CONFIG_PCI_HCI
#define CHECK_HANGUP
//#define CHECK_HANG_DEBUG
#endif

#ifdef CHECK_HANGUP
	#define CHECK_TX_HANGUP				1
	#define CHECK_RX_DMA_ERROR			2
	#define CHECK_RX_TAG_ERROR			4
	#define CHECK_LX_DMA_ERROR			8
	#define CHECK_FW_ERROR				16
	#define CHECK_BEACON_HANGUP			32
	#define FAST_RECOVERY
#endif


//-------------------------------------------------------------
// DFS
//-------------------------------------------------------------
#ifdef CONFIG_RTL_DFS_SUPPORT
#define DFS
#endif


//-------------------------------------------------------------
// Driver based WPA PSK feature
//-------------------------------------------------------------
#define INCLUDE_WPA_PSK
#if (defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8814AE)) && !defined(CLIENT_MODE)
#define HW_ENC_FOR_GROUP_CIPHER
#endif


//eric-sync ?? bind with openwrt-sdk ??
//-------------------------------------------------------------
// NL80211
//-------------------------------------------------------------
//
#ifdef CONFIG_OPENWRT_SDK
#define NETDEV_NO_PRIV 1  //mark_wrt
#undef EVENT_LOG //mark_wrt

#if defined(CPTCFG_CFG80211_MODULE)
#define RTK_NL80211 1
#endif

#ifdef RTK_NL80211
//#define CONFIG_NET_PCI
#undef EAP_BY_QUEUE
#undef INCLUDE_WPA_PSK

#ifndef CONFIG_RTL_VAP_SUPPORT
#define CONFIG_RTL_VAP_SUPPORT 1
#endif

#ifndef CONFIG_RTL_REPEATER_MODE_SUPPORT
#define CONFIG_RTL_REPEATER_MODE_SUPPORT 1
#endif

#ifndef CONFIG_RTL_CLIENT_MODE_SUPPORT
#define CONFIG_RTL_CLIENT_MODE_SUPPORT
#endif

#endif //RTK_NL80211

#ifdef __LINUX_3_18_21__
#define OPENWRT_CC
#endif

#endif //CONFIG_OPENWRT_SDK

//-------------------------------------------------------------
// RF Fine Tune
//-------------------------------------------------------------
//#define RF_FINETUNE


//-------------------------------------------------------------
// Wifi WMM
//-------------------------------------------------------------
#define WIFI_WMM
#ifdef WIFI_WMM
	#define WMM_APSD	// WMM Power Save
	#ifndef WIFI_LIMITED_MEM
	#define RTL_MANUAL_EDCA		// manual EDCA parameter setting
	#endif
#if defined(CONFIG_RTL_8197F) || defined(CONFIG_WLAN_HAL_8822BE)
#define WMM_VIBE_PRI
#endif
#endif
//-------------------------------------------------------------
// Channel Loading Calculation
//-------------------------------------------------------------
#define CH_LOAD_CAL
#define CLM_SAMPLE_NUM2 65535  /*  CLM sample num , 65535*4us = 256 ms*/

/* 45ms[CLM sample time] switch to [CLM sample num , it's unit is 4us]*/
#define RTK80211_TIME_TO_SAMPLE_NUM   ( (45 * 1000) / 4 )

/* [CLM sample num , it's unit is 4us] switch to busy time [ how many busy ms of one second]*/
#define RTK80211_SAMPLE_NUM_TO_TIME(VAL)  (  (VAL*1000) /RTK80211_TIME_TO_SAMPLE_NUM )


//-------------------------------------------------------------
// Hotspot 2.0 -R2(20141216 SYS logo pass)
//-------------------------------------------------------------
#ifdef CONFIG_RTL_HS2_SUPPORT
#define HS2_SUPPORT
//#define HS2_CLIENT_TEST
#define CONFIG_IEEE80211W
//#define DRVMAC_LB
//#define CONFIG_IEEE80211W_TEST
#endif

#ifdef CONFIG_RTL_HS2_SIGMA_SUPPORT
#define HS2_SIGMA_SUPPORT
#endif
//-------------------------------------------------------------
// Hostapd
//-------------------------------------------------------------
#ifdef CONFIG_RTL_HOSTAPD_SUPPORT
#define WIFI_HAPD
#endif
#ifdef CONFIG_RTL_P2P_SUPPORT
#define P2P_SUPPORT  //  support for WIFI_Direct
//#define P2P_DEBUGMSG
#endif

#ifdef CONFIG_PACP_SUPPORT
#define SUPPORT_MONITOR			// for packet capture function
#if !defined(CONFIG_RTL_COMAPI_WLTOOLS) && !defined(__ECOS)
#define CONFIG_RTL_COMAPI_WLTOOLS
#endif
#endif

#ifdef CONFIG_RTL_TDLS_SUPPORT
#define TDLS_SUPPORT
#endif

#ifdef WIFI_HAPD

//#define HAPD_DRV_PSK_WPS

#ifndef CONFIG_WEXT_PRIV
#define CONFIG_WEXT_PRIV
#endif

#if !defined(CONFIG_RTL_COMAPI_WLTOOLS) && !defined(__ECOS)
#define CONFIG_RTL_COMAPI_WLTOOLS
#endif

#ifndef HAPD_DRV_PSK_WPS
#undef EAP_BY_QUEUE
#undef INCLUDE_WPA_PSK
#endif

#endif


//-------------------------------------------------------------
// IO mapping access
//-------------------------------------------------------------
//#define IO_MAPPING


//-------------------------------------------------------------
// Wifi Simple Config support
//-------------------------------------------------------------
#define WIFI_SIMPLE_CONFIG
/* WPS2DOTX   */
#define WPS2DOTX
#define OUI_LEN					4

#ifdef WPS2DOTX
#define SUPPORT_PROBE_REQ_REASSEM	//for AP mode
#define SUPPORT_PROBE_RSP_REASSEM	// for STA mode
//#define WPS2DOTX_DEBUG
#endif

#ifdef	WPS2DOTX_DEBUG	  //0614 for wps2.0  trace
#define SECU_DEBUG(fmt, args...)  printk("[secu]%s %d:"fmt, __FUNCTION__,__LINE__, ## args)
#define SME_DEBUG(fmt, args...) printk("[sme]%s %d:"fmt,__FUNCTION__ , __LINE__ , ## args)

#else
#define SECU_DEBUG(fmt, args...)
#define SME_DEBUG(fmt, args...)
#endif
/* WPS2DOTX   */

//-------------------------------------------------------------
// Support Multiple BSSID
//-------------------------------------------------------------
#if defined(CONFIG_RTL_VAP_SUPPORT)
#define MBSSID
#endif
#ifdef MBSSID
#if (!defined(CONFIG_RTL_SDRAM_GE_32M) && (defined(CONFIG_RTL_8196E))) || (defined(CONFIG_RTL_SHRINK_MEMORY_SIZE) && defined(CONFIG_RTL_8197F))
#define RTL8192CD_NUM_VWLAN  1
#else
#define RTL8192CD_NUM_VWLAN  4
#endif
#else
#define RTL8192CD_NUM_VWLAN  0
#endif


//-------------------------------------------------------------
// Support Tx Descriptor Reservation for each interface
//-------------------------------------------------------------
#ifdef CONFIG_RTL_TX_RESERVE_DESC
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
#define RESERVE_TXDESC_FOR_EACH_IF
#endif
#endif


//-------------------------------------------------------------
// Group BandWidth Control
//-------------------------------------------------------------
#ifdef CONFIG_PCI_HCI
#define GBWC
#endif

//-------------------------------------------------------------
// Support add or remove ACL list at run time
//-------------------------------------------------------------
#define D_ACL

//-------------------------------------------------------------
// Support 802.11 SNMP MIB
//-------------------------------------------------------------
//#define SUPPORT_SNMP_MIB


//-------------------------------------------------------------
// Driver-MAC loopback
//-------------------------------------------------------------
//#define DRVMAC_LB


//-------------------------------------------------------------
// Use perfomance profiling
//-------------------------------------------------------------
//#define PERF_DUMP
#ifdef PERF_DUMP
//3 Definition
//Check List Selection
#define PERF_DUMP_INIT_ORI          0
#define PERF_DUMP_INIT_WLAN_TRX     1

//CP3 of MIPS series count event format selection
#define PERF_DUMP_CP3_OLD     0
#define PERF_DUMP_CP3_NEW     1

//3 Control Setting
#define PERF_DUMP_INIT_SELECT   PERF_DUMP_INIT_WLAN_TRX

// TODO: Filen, I am not sure that which type these are for 97D/96E/96D
#if defined(CONFIG_RTL_8881A)
#define PERF_DUMP_CP3_SELECT    PERF_DUMP_CP3_NEW
#else
// 96C / ...
#define PERF_DUMP_CP3_SELECT    PERF_DUMP_CP3_OLD
#endif

//Dual Counter mode, only new chip support
#if (PERF_DUMP_CP3_SELECT == PERF_DUMP_CP3_NEW)
#define PERF_DUMP_CP3_DUAL_COUNTER_EN
#else
#undef PERF_DUMP_CP3_DUAL_COUNTER_EN
#endif
#endif //PERF_DUMP

//#define PERF_DUMP_1074K

//-------------------------------------------------------------
// 1x1 Antenna Diversity
//-------------------------------------------------------------
#if defined (CONFIG_ANT_SWITCH) || defined(CONFIG_RTL_8881A_ANT_SWITCH) || defined(CONFIG_SLOT_0_ANT_SWITCH) || defined(CONFIG_SLOT_1_ANT_SWITCH) || defined(CONFIG_RTL_8197F_ANT_SWITCH)
//#define SW_ANT_SWITCH
#define HW_ANT_SWITCH
//#define GPIO_ANT_SWITCH
#ifdef HW_ANT_SWITCH
#define HW_DIV_ENABLE	(priv->pshare->rf_ft_var.antHw_enable&1)
#endif
#ifdef SW_ANT_SWITCH
#define SW_DIV_ENABLE	(priv->pshare->rf_ft_var.antSw_enable&1)
#endif
#endif


//-------------------------------------------------------------
// WPAI performance issue
//-------------------------------------------------------------
#ifdef CONFIG_RTL_WAPI_SUPPORT
	//#define IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
	#if defined(CONFIG_RTL8192CD)
	// if CONFIG_RTL_HW_WAPI_SUPPORT defined, { SWCRYPTO=1: sw wapi; SWCRYPTO=0: hw wapi. }
	// if CONFIG_RTL_HW_WAPI_SUPPORT not defined, { SWCRYPTO=1: sw wapi;  SWCRYPTO=0: should not be used! }
	#define	CONFIG_RTL_HW_WAPI_SUPPORT		1
	#endif
#endif


//-------------------------------------------------------------
// Use local ring for pre-alloc Rx buffer.
// If no defined, will use kernel skb que
//-------------------------------------------------------------
#ifndef __ECOS
#define RTK_QUE
#endif


//-------------------------------------------------------------
//Support IP multicast->unicast
//-------------------------------------------------------------
#ifndef __ECOS
#define SUPPORT_TX_MCAST2UNI
#define MCAST2UI_REFINE
#endif

#ifdef CONFIG_RTL_819X_ECOS
#define SUPPORT_TX_MCAST2UNI
//#define MCAST2UI_REFINE
#endif

#ifdef CLIENT_MODE
#define SUPPORT_RX_UNI2MCAST
#endif

/* for cameo feature*/
#ifdef CONFIG_RTL865X_CMO
	#define IGMP_FILTER_CMO
#endif

// Support  IPV6 multicast->unicast
#ifdef	SUPPORT_TX_MCAST2UNI
	#define	TX_SUPPORT_IPV6_MCAST2UNI
#endif
//-------------------------------------------------------------
// Support  IDLE NOISE LEVEL , so far 97F,8812B has tested
//-------------------------------------------------------------
//#define IDLE_NOISE_LEVEL

//-------------------------------------------------------------
// Support  USB tx rate adaptive
//-------------------------------------------------------------
// define it always for object code release
#if defined(CONFIG_USB) && !defined(NOT_RTK_BSP)
	#define USB_PKT_RATE_CTRL_SUPPORT
#endif


//-------------------------------------------------------------
// Support Tx AMSDU
//-------------------------------------------------------------
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE) && !defined(__ECOS)
#define SUPPORT_TX_AMSDU
#endif

#if !defined(CONFIG_RTL_8198C)
#ifdef SUPPORT_TX_AMSDU
#if defined(TX_SHORTCUT) && !defined(CONFIG_RTK_MESH)
// No idea how to process AMSDU shortcut with MESH or WDS, so disable AMSDU shortcut when MESH or WDS enabled
#define SUPPORT_TX_AMSDU_SHORTCUT
#endif
#endif
#endif

//-------------------------------------------------------------
// Support Rx AMSDU
//-------------------------------------------------------------
#if (defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)) && !defined(__ECOS)
#define SUPPORT_RX_AMSDU_AMPDU
#endif

//8812_RX_AMSDU_AMPDU
#if defined(CONFIG_RTL_8812_SUPPORT) && !defined(__ECOS)
#ifndef SUPPORT_RX_AMSDU_AMPDU
#define SUPPORT_RX_AMSDU_AMPDU
#endif
#endif

//-------------------------------------------------------------
// Mesh Network
//-------------------------------------------------------------
#ifdef CONFIG_RTK_MESH
#define _MESH_ACL_ENABLE_

/*need check Tx AMSDU dependency ; 8196B no support now */
#ifdef	SUPPORT_TX_AMSDU
#define MESH_AMSDU
#endif
//#define MESH_ESTABLISH_RSSI_THRESHOLD
//#define MESH_BOOTSEQ_AUTH
#endif // CONFIG_RTK_MESH


//-------------------------------------------------------------
// Realtek proprietary wake up on wlan mode
//-------------------------------------------------------------
//#define RTK_WOW


//-------------------------------------------------------------
// Use static buffer for STA private buffer
//-------------------------------------------------------------
#if !defined(CONFIG_RTL8196B_GW_8M) && !defined(CONFIG_WIRELESS_LAN_MODULE) && !defined(WIFI_LIMITED_MEM) && !defined(__OSK__)
#define PRIV_STA_BUF
#endif
#if defined(CONFIG_WLAN_HAL_8197F) && defined(CONFIG_RTL_NFJROM_MP) && (defined(CONFIG_RTL_8812_SUPPORT) || defined(__ECOS))
#define PRIV_STA_BUF
#endif
//-------------------------------------------------------------
// Do not drop packet immediately when rx buffer empty
//-------------------------------------------------------------

#ifdef __ECOS
#ifdef CONFIG_RTL_DELAY_REFILL
#define DELAY_REFILL_RX_BUF
#endif
#endif


#if defined(CONFIG_RTL8190_PRIV_SKB) || defined(CONFIG_OPENWRT_SDK)
	#define DELAY_REFILL_RX_BUF
#endif


//-------------------------------------------------------------
// WiFi 11n 20/40 coexistence
//-------------------------------------------------------------
#define WIFI_11N_2040_COEXIST
#define WIFI_11N_2040_COEXIST_EXT
//#define WIFI_11N_2040_PERMIT_LOGIC
//-------------------------------------------------------------
// Add TX power by command
//-------------------------------------------------------------
#define ADD_TX_POWER_BY_CMD


//-------------------------------------------------------------
// Do Rx process in tasklet
//-------------------------------------------------------------
//#define RX_TASKLET


//-------------------------------------------------------------
// Support external high power PA
//-------------------------------------------------------------
#if defined(CONFIG_SLOT_0_EXT_PA) || defined(CONFIG_SLOT_1_EXT_PA) || defined(CONFIG_SOC_EXT_PA)
#define HIGH_POWER_EXT_PA
#endif


//-------------------------------------------------------------
// Support external LNA
//-------------------------------------------------------------
#if defined(CONFIG_SLOT_0_EXT_LNA) || defined(CONFIG_SLOT_1_EXT_LNA) || defined(CONFIG_SOC_EXT_LNA)
#define HIGH_POWER_EXT_LNA
#endif


//-------------------------------------------------------------
// Cache station info for bridge
//-------------------------------------------------------------
#define RTL_CACHED_BR_STA


//-------------------------------------------------------------
// Bridge shortcut for AP to AP case
//-------------------------------------------------------------
#define AP_2_AP_BRSC

//-------------------------------------------------------------
// Use default keys of WEP (instead of keymapping keys)
//-------------------------------------------------------------
//#define USE_WEP_DEFAULT_KEY
//#define USE_WEP_4_KEYS

//-------------------------------------------------------------
// Auto test support
//-------------------------------------------------------------
//#ifdef CONFIG_RTL_92C_SUPPORT
#define AUTO_TEST_SUPPORT
//#endif


//-------------------------------------------------------------
// to prevent broadcast storm attacks
//-------------------------------------------------------------
#define PREVENT_BROADCAST_STORM	1

//-------------------------------------------------------------
// to prevent ARP spoofing attacks
//-------------------------------------------------------------
#ifdef __KERNEL__
//#define PREVENT_ARP_SPOOFING
#endif

#ifdef PREVENT_BROADCAST_STORM
/*
 *	NOTE: The driver will skip the other broadcast packets if the system free memory is less than FREE_MEM_LOWER_BOUND
 *		   and the broadcast packet amount is larger than BROADCAST_STORM_THRESHOLD in one second period.
 */

#define BROADCAST_STORM_THRESHOLD		50 //16
#define FREE_MEM_LOWER_BOUND			800 //uint: KBytes
#endif


//-------------------------------------------------------------
// Video streaming refine
//-------------------------------------------------------------
#ifdef CONFIG_RTK_VLC_SPEEDUP_SUPPORT
	#define VIDEO_STREAMING_REFINE
#endif


//-------------------------------------------------------------
// Rx buffer gather feature
//-------------------------------------------------------------
#define RX_BUFFER_GATHER
//#define RX_BUFFER_GATHER_REORDER

//-------------------------------------------------------------
//Check Veriwave BA/ACK bug
//-------------------------------------------------------------
#ifndef CONFIG_VERIWAVE_CHECK
//#define CONFIG_VERIWAVE_CHECK
#endif

//-------------------------------------------------------------
//Check Veriwave MU bug
//-------------------------------------------------------------
#ifndef CONFIG_VERIWAVE_MU_CHECK
//#define CONFIG_VERIWAVE_MU_CHECK
#endif

//-------------------------------------------------------------
// Error access counter
//-------------------------------------------------------------
//#define ERR_ACCESS_CNTR
#define MAX_ERR_ACCESS_CNTR		64

//-------------------------------------------------------------
// sta control support
//-------------------------------------------------------------

#ifdef CONFIG_RTL_STA_CONTROL_SUPPORT
#define STA_CONTROL
#define STA_CONTROL_ALGO1    1
#define STA_CONTROL_ALGO2    2
#define	STA_CONTROL_ALGO3	3
#define STA_CONTROL_ALGO STA_CONTROL_ALGO3
#endif


//-------------------------------------------------------------
// A4 client support
//-------------------------------------------------------------
#if defined(CONFIG_RTL_A4_STA_SUPPORT)
#define A4_STA
#endif

#if defined(A4_STA)
#define A4_STA_OUT_REALTEK 0
#define A4_STA_OUT_CUSTOMER_1 1
#define A4_STA_OUI A4_STA_OUT_CUSTOMER_1
#endif //A4_STA

//-------------------------------------------------------------
// TV MODE support
//-------------------------------------------------------------
//#define TV_MODE


//-------------------------------------------------------------
// Avoid deadlock for SMP(Symmetrical Multi-Processing) architecture
//-------------------------------------------------------------
#if defined(CONFIG_SMP) && !defined(SMP_SYNC)
#define SMP_SYNC
#endif


//-------------------------------------------------------------
// NOT use Realtek specified BSP
//-------------------------------------------------------------
//#define NOT_RTK_BSP


//-------------------------------------------------------------
// No padding between members in the structure for specified CPU
//-------------------------------------------------------------
//#define PACK_STRUCTURE


//-------------------------------------------------------------
// customers proprietary info display
//-------------------------------------------------------------
//#define TLN_STATS


//-------------------------------------------------------------
// Shrink memory for 96D+8194
//-------------------------------------------------------------
#if defined(CONFIG_RTL_819XD) && defined(CONFIG_WLAN_HAL_8814AE)
#if !defined(CONFIG_SLOT_0_8814AE) && !defined(CONFIG_SLOT_1_8814AE)
	#define SDRAM_LE_32M
#endif
#endif

//-------------------------------------------------------------
// Tx early mode
//-------------------------------------------------------------
#ifdef CONFIG_RTL_TX_EARLY_MODE_SUPPORT
#ifdef CONFIG_RTL_88E_SUPPORT
	#define TX_EARLY_MODE
#endif
#endif

//-------------------------------------------------------------
// Dynamically switch LNA for 97d High Power in MP mode to pass Rx Test
//-------------------------------------------------------------
#if defined(CONFIG_RTL_819XD) && !defined(CONFIG_RTL_8196D) && defined(CONFIG_HIGH_POWER_EXT_PA) //for 97d High Power only
//#define MP_SWITCH_LNA
#endif


//-------------------------------------------------------------
// RTL8188E GPIO Control
//-------------------------------------------------------------
#define RTLWIFINIC_GPIO_CONTROL

//-------------------------------------------------------------
// General interface of add user defined IE.
//-------------------------------------------------------------
#define USER_ADDIE

#if defined(CONFIG_WLAN_HAL_8814AE) && defined(CONFIG_WLAN_HAL_8197F)
#define WLAN_NULL_INT_WORKAROUND
#endif



//-------------------------------------------------------------
// TXBF SUPPORT
//-------------------------------------------------------------
#if defined (CONFIG_SLOT_0_TX_BEAMFORMING) || defined (CONFIG_SLOT_1_TX_BEAMFORMING) || defined (CONFIG_SOC_TX_BEAMFORMING)
#define BEAMFORMING_SUPPORT  1
#ifdef CONFIG_WLAN_HAL_8822BE
#define MU_BEAMFORMING_SUPPORT 1
#else
#define MU_BEAMFORMING_SUPPORT 0
#endif
#else
#define BEAMFORMING_SUPPORT  0
#define MU_BEAMFORMING_SUPPORT 0
#endif

#if (BEAMFORMING_SUPPORT == 1) && defined(CONFIG_TXPWR_LMT)
#define BEAMFORMING_AUTO //TXBF auto mechanism
#endif


//-------------------------------------------------------------
// WLAN event indication by netlink
//-------------------------------------------------------------
#ifdef CONFIG_RTK_WLAN_EVENT_INDICATE
#define RTK_WLAN_EVENT_INDICATE
#endif


//-------------------------------------------------------------
// Tx power limit function
//-------------------------------------------------------------
#ifdef CONFIG_TXPWR_LMT
#define TXPWR_LMT
#ifdef CONFIG_RTL_8812_SUPPORT
#define TXPWR_LMT_8812
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
#define TXPWR_LMT_88E
#endif
#ifdef CONFIG_WLAN_HAL_8881A
#define TXPWR_LMT_8881A
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
#define TXPWR_LMT_92EE
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
#define TXPWR_LMT_8814A
#endif
#ifdef CONFIG_WLAN_HAL_8197F
#define TXPWR_LMT_8197F
#endif
#ifdef CONFIG_WLAN_HAL_8822BE
#define TXPWR_LMT_8822B
#endif


#if defined(TXPWR_LMT_8812) || defined(TXPWR_LMT_88E) || defined(CONFIG_WLAN_HAL)
#define TXPWR_LMT_NEWFILE
#endif

#endif // CONFIG_TXPWR_LMT

#if 0
#define PWR_BY_RATE_92E_HP
#endif

//-------------------------------------------------------------
// RADIUS Accounting supportive functions
//-------------------------------------------------------------
//#define RADIUS_ACCOUNTING

//-------------------------------------------------------------
// Client mixed mode security
//-------------------------------------------------------------
#ifdef CLIENT_MODE

#if 1//def CONFIG_SUPPORT_CLIENT_MIXED_SECURITY
#define SUPPORT_CLIENT_MIXED_SECURITY
#endif

#endif


//-------------------------------------------------------------
// Support BT Coexist for 92E
//-------------------------------------------------------------
#ifdef CONFIG_BT_COEXIST_92EE
#define BT_COEXIST
#endif


//-------------------------------------------------------------
// Support BT Coexist for HAL: currently 97F
//-------------------------------------------------------------
#ifdef CONFIG_BT_COEXIST
#define BT_COEXIST_HAL
#endif


//-------------------------------------------------------------
// Support BT Coexist Kernel Socket Commnunication in replace of FW mailbox
//-------------------------------------------------------------
#ifdef CONFIG_BT_COEXIST_SOCKET
#define BT_COEXIST_SOCKET
#endif


//-------------------------------------------------------------
// Support dynamic swap mechanism of SW & HW CAM entries
// (move idle station in HW CAM to SW Encrypt, move busy station in SW Encrypt to HW CAM)
//-------------------------------------------------------------
//#define CAM_SWAP

//-------------------------------------------------------------
// Drop packets in RX
//-------------------------------------------------------------
//#define DROP_RXPKT

//-------------------------------------------------------------
// Client mixed mode security
//-------------------------------------------------------------
#ifdef CLIENT_MODE
#define SUPPORT_CLIENT_MIXED_SECURITY
#endif

//-------------------------------------------------------------
// FW Test Function
//-------------------------------------------------------------
#ifdef CONFIG_WLAN_HAL
#define CONFIG_OFFLOAD_FUNCTION
//#define CONFIG_SAPPS //tingchu
//#define CONFIG_32K  //tingchu
#endif

//-------------------------------------------------------------
//Change interval for LED display when in configuring progress
//-------------------------------------------------------------
#if defined(CONFIG_APPLE_MFI_SUPPORT) || defined(CONFIG_RTL_SIMPLE_CONFIG)
#ifdef CONFIG_RTL_88E_SUPPORT
#define SUPPORT_UCFGING_LED
#endif
#endif

//-------------------------------------------------------------
// 8814 AP MAC function verification
//-------------------------------------------------------------
#ifdef CONFIG_WLAN_HAL_8814AE

//#define RTL8814_FPGA_TEMP
//#define DISABLE_BB_RF
//#define CONFIG_8814_AP_MAC_VERI
//#define VERIFY_AP_FAST_EDCA
//#define RTL_8814_C_CUT_TEMP // only use for 8814A Test Chip PCIe mini card
#define RTL_8814A_MP_TEMP  // TODO: temp setting for porting 8814A MP

// Note: need to change /WlanHAL/Data/8814A folder
//#define RTL_8814_RFE_TYPE0 // PCIE half size mini card
//#define RTL_8814_RFE_TYPE2  // PCIE full size mini card

#if defined(CONFIG_RTL_8198C)
#define TRXBD_CACHABLE_REGION
#endif
#define RX_CRC_EXPTIMER // must enable 0x608[8] = 1b'1 #BIT_ACRC32
//#define MERGE_TXDESC_HEADER_PAYLOAD
//#define MERGE_HEADER_PAYLOAD

//#define FPGA_TEST_BIT_FILE

#if defined(CONFIG_RTL_8198C) || defined(NOT_RTK_BSP)
#define ENABLE_PCIE_MULTI_TAG
#define ENABLE_PCIE_PREFETCH
// #define CONFIG_WLAN_PCIE_SSC //for FCC or CE MASK
#endif


//#undef RF_MIMO_SWITCH

#ifndef __ECOS  // avoid  relocation R_mips_26 problem
#define del_timer_sync del_timer
#endif

#define CONFIG_RTL_PCIE_LINK_PROTECTION 1
#endif

#if defined(CONFIG_WLAN_HAL_8822BE)//eric-8822 ??

#define TRXBD_CACHABLE_REGION

#if defined(CONFIG_RTL_8198C) || defined(NOT_RTK_BSP)
#define ENABLE_PCIE_MULTI_TAG
#endif

#endif

//-------------------------------------------------------------
// Cochannel RTS
//-------------------------------------------------------------
#define COCHANNEL_RTS
//-------------------------------------------------------------
// WLAN HAL
//-------------------------------------------------------------
#ifdef CONFIG_WLAN_HAL

// Special function, implement after 8814A IC
#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8197F) || defined(CONFIG_WLAN_HAL_8822BE)
#define SUPPORT_EACH_VAP_INT

#define SUPPORT_RELEASE_ONE_PACKET

#define DEBUG_8822TX

#if defined(TX_SHORTCUT)
#define WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
//#define WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
// TODO: turn on REUSE + HDR_CONV
// TODO: recycle problem (sw desc) for HDR_CONV
#define WLAN_HAL_HW_SEQ
#define WLAN_HAL_HW_AES_IV
#undef  TX_SC_ENTRY_NUM
#define TX_SC_ENTRY_NUM			1
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
#endif // defined(TX_SHORTCUT)

#endif // #if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8197F) || defined(CONFIG_WLAN_HAL_8822BE)


// Add by Eric and Pedro, this compile flag is temp for 8814 FPGA tunning
// Must remove after driver ready
#ifdef RTL8814_FPGA_TEMP
#define DISABLE_BB_RF
#endif

//-------------------------------------------------------------
// 8197F AP MAC function verification
//-------------------------------------------------------------
#ifdef CONFIG_WLAN_HAL_8197F
#define ENABLE_PCIE_MULTI_TAG
#define TRXBD_CACHABLE_REGION
#define RX_CRC_EXPTIMER
#define SUPPORT_AXI_BUS_EXCEPTION
#endif

//#ifdef CONFIG_RTL_8198C
//#define TRXBD_CACHABLE_REGION
//#endif
//#undef CHECK_HANGUP
//#undef FAST_RECOVERY
//#undef SW_TX_QUEUE
//#undef DELAY_REFILL_RX_BUF
//#undef CONFIG_NET_PCI
#undef TX_EARLY_MODE
//#undef CONFIG_OFFLOAD_FUNCTION
//#undef PREVENT_BROADCAST_STORM
//#undef DETECT_STA_EXISTANCE
//#undef RX_BUFFER_GATHER
//#undef GBWC
#define _TRACKING_TABLE_FILE
#define AVG_THERMAL_NUM_88XX    4
//#define WLANHAL_MACDM

#ifdef SUPPORT_TX_AMSDU
#define WLAN_HAL_TX_AMSDU
// Note: WLAN_HAL_TX_AMSDU_MAX_NUM must be the same as MAX_NUM_OF_MSDU_IN_AMSDU in WlanHAL !!!
#define WLAN_HAL_TX_AMSDU_MAX_NUM   4   // recommended value: 4, 6, 8,
#define WLAN_HAL_TX_AMSDU_NUM       4   // priv->pmib->dot11nConfigEntry.dot11nAMSDUSendNum
#endif

#endif

#ifdef CONFIG_WLAN_HAL_8822BE
#define SUPPORT_AP_OFFLOAD
//#define WLAN_SUPPORT_H2C_PACKET
#define CONFIG_8814_AP_MAC_VERI
//#undef SUPPORT_TX_AMSDU
//#undef WLAN_HAL_TX_AMSDU
//#define ENABLE_LA_MODE

#endif

#ifdef CONFIG_WLAN_HAL_8197F
//#define WLAN_SUPPORT_H2C_PACKET
#define SUPPORT_AP_OFFLOAD

#endif //#ifdef CONFIG_WLAN_HAL_8197F


#ifdef HW_DETEC_POWER_STATE
//#define AP_PS_Offlaod
//#define AP_PS_Offlaod_Phase2
#ifdef AP_PS_Offlaod_Phase2
#ifndef AP_PS_Offlaod
#define AP_PS_Offlaod
#endif
#endif
//#define AP_PS_Offlaod_VERI
#endif

#if defined(CONFIG_WLAN_HAL_8814AE) && defined(TX_SHORTCUT)
//#define WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
//#define WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
// TODO: turn on REUSE + HDR_CONV
// TODO: recycle problem (sw desc) for HDR_CONV
#define WLAN_HAL_HW_SEQ
#define WLAN_HAL_HW_AES_IV
#undef  TX_SC_ENTRY_NUM
#define TX_SC_ENTRY_NUM			1
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
#endif // defined(CONFIG_WLAN_HAL_8814AE) && defined(TX_SHORTCUT)


#ifdef CONFIG_RTL_8881A
#define WLAN_HAL_TXDESC_CHECK_ADDR_LEN    1
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
//#define BEAMFORMING_SUPPORT                     1
#endif

//#define WMM_DSCP_C42

/*********************************************************************/
/* some definitions in 8192cd driver, we set them as NULL definition */
/*********************************************************************/
#ifdef USE_RTL8186_SDK
#if defined(CONFIG_WIRELESS_LAN_MODULE) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define __DRAM_IN_865X
#define __IRAM_IN_865X
#else
#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))
#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))
#endif

#define RTL8190_DIRECT_RX						/* For packet RX : directly receive the packet instead of queuing it */
#define RTL8190_ISR_RX							/* process RXed packet in interrupt service routine: It become useful only when RTL8190_DIRECT_RX is defined */

#ifndef CONFIG_WIRELESS_LAN_MODULE
#ifndef CONCURRENT_MODE
#define RTL8192CD_VARIABLE_USED_DMEM			/* Use DMEM for some critical variables */
#endif
#endif

#else // not USE_RTL8186_SDK

#define __DRAM_IN_865X
#define __IRAM_IN_865X
#endif

#undef __MIPS16
#if CONFIG_RTL_DISABLE_WLAN_MIPS16
#define __MIPS16
#else
#ifdef __ECOS
#if defined(RTLPKG_DEVS_ETH_RLTK_819X_USE_MIPS16) || (!defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F))
#define __MIPS16			__attribute__ ((mips16))
#else
#define __MIPS16
#endif
#else
#if defined(CONFIG_WIRELESS_LAN_MODULE) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)   || (defined(CONFIG_RTL8672) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)))
#define __MIPS16
#else
#define __MIPS16			__attribute__ ((mips16))
#endif
#endif
#endif

#ifdef IRAM_FOR_WIRELESS_AND_WAPI_PERFORMANCE
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define __IRAM_WLAN_HI		//__attribute__ ((section(".iram-wapi")))
#define __DRAM_WLAN_HI		//__attribute__ ((section(".dram-wapi")))
#else
#define __IRAM_WLAN_HI		__attribute__ ((section(".iram-wapi")))
#define __DRAM_WLAN_HI		__attribute__ ((section(".dram-wapi")))
#endif
#endif


//-------------------------------------------------------------
// Kernel 2.6 specific config
//-------------------------------------------------------------
#ifdef __LINUX_2_6__

#define USE_RLX_BSP
#ifndef CONCURRENT_MODE
#define RTL8192CD_VARIABLE_USED_DMEM
#endif

#ifndef RX_TASKLET
	#define	RX_TASKLET
#endif

#endif


#if 0
//-------------------------------------------------------------
// TR define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL8196B_TR


//-------------------------------------------------------------
// AC define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL865X_AC

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
#endif

#ifdef CLIENT_MODE
	#undef CLIENT_MODE
	#undef RTK_BR_EXT
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#endif // CONFIG_RTL865X_AC
#endif //#if 0


//-------------------------------------------------------------
// Config Little Endian CPU
//-------------------------------------------------------------
#if defined(_LITTLE_ENDIAN_) && !defined(CONFIG_RTL_8197F)

#ifndef NOT_RTK_BSP
#define NOT_RTK_BSP
#endif

#ifndef CONFIG_WIRELESS_LAN_MODULE
#define CONFIG_WIRELESS_LAN_MODULE
#endif

#ifdef __MIPSEB__
	#undef __MIPSEB__
#endif

#ifdef _BIG_ENDIAN_
	#undef _BIG_ENDIAN_
#endif

#endif //_LITTLE_ENDIAN_

//-------------------------------------------------------------
// Config if NOT use Realtek specified BSP
//-------------------------------------------------------------
#ifdef NOT_RTK_BSP

#if defined(CONFIG_PCI_HCI) && !defined(CONFIG_NET_PCI)
#define CONFIG_NET_PCI
#endif

#ifndef __LINUX_2_6__
#ifndef __ECOS
#define del_timer_sync del_timer
#endif
#endif

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= 0x02061D // linux 2.6.29
#define NETDEV_NO_PRIV
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29))
#define CONFIG_COMPAT_NET_DEV_OPS
#endif
#endif

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
#undef CONFIG_RTL_CUSTOM_PASSTHRU
#endif

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE
#undef CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE
#endif

#ifdef _USE_DRAM_
	#undef _USE_DRAM_
#endif

#ifdef _BIG_ENDIAN_
#ifndef CHECK_SWAP
	#define CHECK_SWAP
#endif
#else // !_BIG_ENDIAN_
#ifdef CHECK_SWAP
	#undef CHECK_SWAP
#endif
#endif

#ifdef EVENT_LOG
	#undef EVENT_LOG
#endif

#ifdef BR_SHORTCUT
	#undef BR_SHORTCUT
#endif

#if defined(NOT_RTK_BSP) && defined(BR_SHORTCUT_SUPPORT)
#define BR_SHORTCUT
#endif // NOT_RTK_BSP && BR_SHORTCUT_SUPPORT

//#ifdef RTK_BR_EXT
//	#undef RTK_BR_EXT
//#endif

//#ifdef UNIVERSAL_REPEATER
//	#undef UNIVERSAL_REPEATER
//#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef USE_IO_OPS
	#undef USE_IO_OPS
#endif

#ifdef IO_MAPPING
	#undef IO_MAPPING
#endif

#ifdef RTK_QUE
	#undef RTK_QUE
#endif

#ifdef USE_RLX_BSP
#undef USE_RLX_BSP
#endif

#ifdef __ECOS
#ifdef RTLWIFINIC_GPIO_CONTROL
#undef RTLWIFINIC_GPIO_CONTROL
#endif
#endif

#undef TRXBD_CACHABLE_REGION

// 2013/07/04, Lucien, Enable RX_BUFFER_GATHER to have lower RX_BUF_LEN
// Because pci_map_single/pci_unmap_single has higher time consumption in some non-RTK platforms.
//#ifdef RX_BUFFER_GATHER
//#undef RX_BUFFER_GATHER
//#endif
#define POWER_PERCENT_ADJUSTMENT

// If the CPU's crystal is shared with WIFI, unmark this line
//#define DONT_DISABLE_XTAL_ON_CLOSE

// use seq_file to display big file to avoid buffer overrun when using create_proc_entry
#ifdef __KERNEL__
#define CONFIG_RTL_PROC_NEW
#endif

#ifdef CONFIG_PCI_HCI
#undef TXDESC_INFO
#endif


#endif //NOT_RTK_BSP


//-------------------------------------------------------------
// Define flag of EC system
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196C_EC

#ifndef USE_WEP_DEFAULT_KEY
	#define USE_WEP_DEFAULT_KEY
#endif

#ifdef TESTCHIP_SUPPORT
	#undef TESTCHIP_SUPPORT
#endif

#endif


//-------------------------------------------------------------
// MSC define flag
//-------------------------------------------------------------
#ifdef _SINUX_
#define CONFIG_MSC
#endif

#ifdef CONFIG_MSC
#define INCLUDE_WPS

#ifdef CHECK_HANGUP
	#undef CHECK_HANGUP
#endif
#ifndef USE_WEP_DEFAULT_KEY         //2010.4.23 Fred Fu open it to support wep key index 1 2 3
	#define USE_WEP_DEFAULT_KEY
#endif

#ifdef WIFI_SIMPLE_CONFIG
#ifdef	INCLUDE_WPS
#define USE_PORTING_OPENSSL
#endif
#endif

#endif

//-------------------------------------------------------------
// Define flag of 8672 system
//-------------------------------------------------------------
#ifdef CONFIG_RTL8672

#ifndef RX_TASKLET
	#define RX_TASKLET
#endif

#ifdef RTL8190_DIRECT_RX
	#undef RTL8190_DIRECT_RX
#endif

#ifdef RTL8190_ISR_RX
	#undef RTL8190_ISR_RX
#endif

#if defined(USE_RLX_BSP) && !defined(LINUX_2_6_22_)
	#undef USE_RLX_BSP
#endif

#undef TX_SC_ENTRY_NUM
#define TX_SC_ENTRY_NUM		2

#ifdef __DRAM_IN_865X
	#undef __DRAM_IN_865X
#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define __DRAM_IN_865X		//__attribute__ ((section(".dram-rtkwlan")))
#else
#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))
#endif
#ifdef __IRAM_IN_865X
	#undef __IRAM_IN_865X
#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define __IRAM_IN_865X		//__attribute__ ((section(".iram-rtkwlan")))
#else
#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))
#endif

#ifdef __IRAM_IN_865X_HI
	#undef __IRAM_IN_865X_HI
#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define __IRAM_IN_865X_HI	//__attribute__ ((section(".iram-tx")))
#else
#define __IRAM_IN_865X_HI	__attribute__ ((section(".iram-tx")))
#endif

//#define USE_TXQUEUE
#ifdef USE_TXQUEUE
	#define TXQUEUE_SIZE	512
#endif

// Support dynamically adjust TXOP in low throughput feature
#define LOW_TP_TXOP

// Support four different AC stream
#define WMM_VIBE_PRI

// Resist interference
#ifdef CONFIG_RTL_92C_SUPPORT
	#define INTERFERENCE_CONTROL
#endif

#ifdef PCIE_POWER_SAVING
	#undef PCIE_POWER_SAVING
#endif

#ifdef CONFIG_RTL_8196C
	#undef CONFIG_RTL_8196C
#endif
#ifdef CONFIG_RTL8196C_REVISION_B
	#undef CONFIG_RTL8196C_REVISION_B
#endif

#ifdef RTL_MANUAL_EDCA
	#undef RTL_MANUAL_EDCA
#endif

#if (defined(CONFIG_RTL_8812_SUPPORT) && defined(CONFIG_WLAN_HAL_8192EE)) || defined(__OSK__)
#define RX_LOOP_LIMIT
#endif

#define DROP_RXPKT
#define RTLWIFINIC_GPIO_CONTROL

#ifdef CONFIG_RTL8686
#define _FULLY_WIFI_IGMP_SNOOPING_SUPPORT_
#endif

// use seq_file to display big file to avoid buffer overrun when using create_proc_entry
#define CONFIG_RTL_PROC_NEW

/* STA BW control */
#define SBWC

#undef CONFIG_RTL_PCIE_LINK_PROTECTION

#endif // CONFIG_RTL8672

#ifdef __ECOS
//disable it for 97f for BT testing
//#define OSK_LOW_TX_DESC
#ifdef TESTCHIP_SUPPORT
#undef TESTCHIP_SUPPORT
#endif
#endif

#if defined(__OSK__)
#define OSK_LOW_TX_DESC
#ifdef TESTCHIP_SUPPORT
#undef TESTCHIP_SUPPORT
#endif
#if defined(CONFIG_AUTH) && !defined(CONFIG_WIFI_INCLUDE_WPA_PSK)
#undef INCLUDE_WPA_PSK
#endif

#ifdef RTL8190_DIRECT_RX
	#undef RTL8190_DIRECT_RX
#endif

#ifdef RTL8190_ISR_RX
	#undef RTL8190_ISR_RX
#endif

#undef __DRAM_IN_865X
#undef __IRAM_IN_865X
#undef __IRAM_IN_865X_HI
#define __DRAM_IN_865X

#undef __IRAM_IN_865X
#undef __IRAM_IN_865X_HI
#ifdef CONFIG_RTL6028
#define __IRAM_IN_865X		__attribute__ ((section(".iram-wifi-pri2"))) __NOMIPS16
#define __IRAM_IN_865X_HI	__attribute__ ((section(".iram-wifi-pri1"))) __NOMIPS16

#define __IRAM_WIFI_PRI1	__attribute__ ((section(".iram-wifi-pri1"))) __NOMIPS16
#define __IRAM_WIFI_PRI2	__attribute__ ((section(".iram-wifi-pri2"))) __NOMIPS16
#define __IRAM_WIFI_PRI3	__attribute__ ((section(".iram-wifi-pri3"))) __NOMIPS16
#define __IRAM_WIFI_PRI4	__attribute__ ((section(".iram-wifi-pri4"))) __NOMIPS16
#define __IRAM_WIFI_PRI5	__attribute__ ((section(".iram-wifi-pri5"))) __NOMIPS16
#define __IRAM_WIFI_PRI6	__attribute__ ((section(".iram-wifi-pri6"))) __NOMIPS16
#else
#define __IRAM_IN_865X		__attribute__ ((section(".iram-wifi-pri2")))
#define __IRAM_IN_865X_HI	__attribute__ ((section(".iram-wifi-pri1")))

#define __IRAM_WIFI_PRI1	__attribute__ ((section(".iram-wifi-pri1")))
#define __IRAM_WIFI_PRI2	__attribute__ ((section(".iram-wifi-pri2")))
#define __IRAM_WIFI_PRI3	__attribute__ ((section(".iram-wifi-pri3")))
#define __IRAM_WIFI_PRI4	__attribute__ ((section(".iram-wifi-pri4")))
#define __IRAM_WIFI_PRI5	__attribute__ ((section(".iram-wifi-pri5")))
#define __IRAM_WIFI_PRI6	__attribute__ ((section(".iram-wifi-pri6")))
#endif

#define scrlog_printk	printk
#define panic_printk printk

#undef BR_SHORTCUT
#undef SW_TX_QUEUE
#undef DROP_RXPKT

#ifdef MP_TEST
#define ALLOW_TELNET
#endif
#undef RX_TASKLET
#undef EVENT_LOG
#undef CONFIG_RTL8671
#undef CONFIG_RTL_PROC_NEW

/* 20100617 support GDMA for wifi TKIP*/
#if !defined(UNIVERSAL_REPEATER) && !defined(CONFIG_RTL_8196E)
#define SUPPORT_TKIP_OSK_GDMA
#endif

//HF
#ifdef CONFIG_RTL_819X
//#define PCI_SLOT0_CONFIG_ADDR		0xbd100000
//#define PCI_SLOT1_CONFIG_ADDR		0xbd180000
#define PCIE0_H_CFG					BSP_PCIE0_H_CFG
#define PCIE0_D_CFG0				BSP_PCIE0_D_CFG0
#define PCIE0_D_CFG1				BSP_PCIE0_D_CFG1
#define PCIE0_D_IO					BSP_PCIE0_D_IO
#define PCIE1_D_IO					BSP_PCIE1_D_IO
#define PCIE0_D_MEM				BSP_PCIE0_D_MEM
#define PCIE1_D_MEM				BSP_PCIE1_D_MEM
//#define GISR            					0xB8003004
//#define GIMR            					0xB8003000

#define PCIE1_H_CFG					BSP_PCIE1_H_CFG
#define PCIE1_D_CFG0				BSP_PCIE1_D_CFG0
#define PCIE1_D_CFG1				BSP_PCIE1_D_CFG1

//#define PCI_IP          (1 << 14)
#define PCIE_IRQ         BSP_PCIE_IRQ
#else
#define PCI_SLOT0_CONFIG_ADDR		0xbd100000
#define PCI_SLOT1_CONFIG_ADDR		0xbd180000
#define PCIE0_H_CFG					0xB8B00000
#define PCIE0_D_CFG0				0xB8B10000
#define PCIE0_D_CFG1				0xB8B11000
#define PCIE0_D_IO					0xB8C00000
#define PCIE1_D_IO					0xB8E00000
#define PCIE0_D_MEM					0xB9000000
#define PCIE1_D_MEM					0xBA000000
//#define GISR            					0xB8003004
//#define GIMR            					0xB8003000

#define PCIE1_H_CFG					BSP_PCIE1_H_CFG
#define PCIE1_D_CFG0				BSP_PCIE1_D_CFG0
#define PCIE1_D_CFG1				BSP_PCIE1_D_CFG1

//#define PCI_IP          (1 << 14)
#if 0 /*tonyWifi 20150211 comment*/
#define PCIE_IRQ         14
#define BSP_PCIE_IRQ				PCIE_IRQ
#endif
#endif
//HF end
/*
 * System MISC Control Register
 */
#ifdef CONFIG_RLE0412
#define MISCCR_BASE     0xB8000300
#else
#define MISCCR_BASE     0xB8003300
#endif
#define MISC_PINMUX     (MISCCR_BASE+0x00)
#define MISC_PINOCR     (MISCCR_BASE+0x04)
#define MISC_PINSR      (MISCCR_BASE+0x08)

//-------------------czyao adds for PCIE------------------------//
#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))
#define WRITE_MEM16(addr, val)   (*(volatile unsigned short *) (addr)) = (val)
#define READ_MEM16(addr)         (*(volatile unsigned short *) (addr))
#define WRITE_MEM8(addr, val)    (*(volatile unsigned char *)  (addr)) = (val)
#define READ_MEM8(addr)          (*(volatile unsigned char *)  (addr))

#define REVR					0xB8000000
#define RTL8196C_REVISION_A 	0x80000001
#define RTL8196C_REVISION_B 	0x80000002
#define BSP_RTL8198_REVISION_A	0xC0000000
#define BSP_RTL8198_REVISION_B	0xC0000001
#define BSP_RTL8197D			0x8197D000

#undef RTL8192CD_NUM_VWLAN
#define RTL8192CD_NUM_VWLAN    CONFIG_WLAN_MBSSID_NUM

#ifndef true
#define true	(0 == 0)
#define false	(0 != 0)
#endif

#endif // defined(__OSK__)

//-------------------------------------------------------------
// Define flag of 129X PLATFORM
//-------------------------------------------------------------
#ifdef RTK_129X_PLATFORM

#ifdef CONFIG_RTL_819X
#undef CONFIG_RTL_819X
#endif

#ifdef RTK_ATM
#undef RTK_ATM
#endif

#ifdef PRIV_STA_BUF
#undef PRIV_STA_BUF
#endif

#define USE_DMA_ALLOCATE

#endif // RTK_129X_PLATFORM

//-------------------------------------------------------------
// Define flag of special features
//-------------------------------------------------------------
//#define TRX_DATA_LOG

//-------------------------------------------------------------
// Define flag of rtl8192d features
//-------------------------------------------------------------
#ifdef CONFIG_RTL_92D_SUPPORT
#define SW_LCK_92D
#define DPK_92D

#define RX_GAIN_TRACK_92D
//#define CONFIG_RTL_NOISE_CONTROL
#ifdef CONFIG_RTL_92D_DMDP
//#define NON_INTR_ANTDIV
#endif

//#ifdef CONFIG_RTL_92D_INT_PA
#define RTL8192D_INT_PA
//#endif

#ifdef RTL8192D_INT_PA
//Use Gain Table with suffix '_new'  for purpose
//1. refine the large gap between power index 39 &40
//#define RTL8192D_INT_PA_GAIN_TABLE_NEW //for both Non-USB & USB Power

//Use Gain Table with suffix '_new1' for purpose
//1. refine the large gap between power index 39 &40
//2. increase tx power
//#define RTL8192D_INT_PA_GAIN_TABLE_NEW1 //for USB Power only
#endif

#endif

//-------------------------------------------------------------
// OUT SOURCE
//-------------------------------------------------------------
#ifdef CONFIG_RTL_ODM_WLAN_DRIVER
#define USE_OUT_SRC					1
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
#define _OUTSRC_COEXIST
#endif
#endif


//-------------------------------------------------------------
// Define flag of RTL8188E features
//-------------------------------------------------------------
#ifdef CONFIG_RTL_88E_SUPPORT
/* RTL8188E test chip support*/
#define SUPPORT_RTL8188E_TC
#ifdef USE_OUT_SRC
#define RATEADAPTIVE_BY_ODM			1
#define CALIBRATE_BY_ODM			1
#endif

// Support four different AC stream
#ifndef WMM_VIBE_PRI
#define WMM_VIBE_PRI
#endif
#define WMM_BEBK_PRI

#ifndef CALIBRATE_BY_ODM
//for 8188E IQK
#define IQK_BB_REG_NUM		9

//for 88e tx power tracking
#define	index_mapping_NUM_88E	15
#define AVG_THERMAL_NUM_88E		4
#define IQK_Matrix_Settings_NUM	1+24+21
#define	AVG_THERMAL_NUM 8
#define	HP_THERMAL_NUM 8

#define	RF_T_METER_88E			0x42
//===
#endif

#endif // CONFIG_RTL_88E_SUPPORT

#if 0
//-------------------------------------------------------------
// TLD define flag
//-------------------------------------------------------------
#ifdef CONFIG_RTL8196B_TLD

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef HIGH_POWER_EXT_PA
	#undef HIGH_POWER_EXT_PA
#endif

#ifdef ADD_TX_POWER_BY_CMD
	#undef ADD_TX_POWER_BY_CMD
#endif

#endif // CONFIG_RTL8196B_TLD
#endif



//-------------------------------------------------------------
// KLD define flag
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196C_KLD)

#ifndef INCLUDE_WPA_PSK
	#define INCLUDE_WPA_PSK
#endif

#ifdef UNIVERSAL_REPEATER
	#undef UNIVERSAL_REPEATER
	#undef SMART_REPEATER_MODE
#endif

#ifndef WIFI_SIMPLE_CONFIG
	#define WIFI_SIMPLE_CONFIG
#endif

#ifdef GBWC
	#undef GBWC
#endif

#ifdef SUPPORT_SNMP_MIB
	#undef SUPPORT_SNMP_MIB
#endif

#ifdef DRVMAC_LB
	#undef DRVMAC_LB
#endif

#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

//#ifdef HIGH_POWER_EXT_PA
//	#undef HIGH_POWER_EXT_PA
//#endif

//#ifdef ADD_TX_POWER_BY_CMD
//	#undef ADD_TX_POWER_BY_CMD
//#endif

#endif // CONFIG_RTL8196C_KLD


//-------------------------------------------------------------
// eCos define flag
//-------------------------------------------------------------
#ifdef __ECOS
	#undef USE_PID_NOTIFY
	//#undef EAP_BY_QUEUE
#ifdef RTK_SYSLOG_SUPPORT
	#define EVENT_LOG
#else
//	#undef EVENT_LOG
	#define EVENT_LOG
#endif
	//#undef SUPPORT_TX_MCAST2UNI	//support m2u hx
	//#undef AUTO_TEST_SUPPORT
	//#undef SUPPORT_RX_UNI2MCAST
#if !defined(UNIVERSAL_REPEATER) && !defined(MBSSID)
	#define USE_WEP_DEFAULT_KEY
#endif
#ifndef RX_TASKLET
	#define	RX_TASKLET
#endif

#ifdef RTL8192CD_NUM_VWLAN
	#undef RTL8192CD_NUM_VWLAN
#ifdef CONFIG_RTL_VAP_SUPPORT
	#define RTL8192CD_NUM_VWLAN	RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM
#else
	#define RTL8192CD_NUM_VWLAN 0
#endif
#else
	#define RTL8192CD_NUM_VWLAN 0
#endif

#if defined(CYGSEM_HAL_IMEM_SUPPORT) && !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)
	#define __IRAM_IN_865X		__attribute__ ((section(".iram-rtkwlan")))
#else
	#define __IRAM_IN_865X
#endif
#if defined(CYGSEM_HAL_DMEM_SUPPORT) && !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)
	#define __DRAM_IN_865X		__attribute__ ((section(".dram-rtkwlan")))
#else
	#define __DRAM_IN_865X
#endif
	#define dev_kfree_skb_any(skb)	wlan_dev_kfree_skb_any(skb)
#endif /* __ECOS */


//-------------------------------------------------------------
// Dependence check of define flag
//-------------------------------------------------------------
#if defined(B2B_TEST) && !defined(MP_TEST)
	#error "Define flag error, MP_TEST is not defined!\n"
#endif


#if defined(UNIVERSAL_REPEATER) && !defined(CLIENT_MODE)
	#error "Define flag error, CLIENT_MODE is not defined!\n"
#endif


#if defined(TX_EARLY_MODE) && !defined(SW_TX_QUEUE)
	#error "Define flag error, SW_TX_QUEUE is not defined!\n"
#endif


/*=============================================================*/
/*------ Compiler Portability Macros --------------------------*/
/*=============================================================*/
#ifdef EVENT_LOG
#ifdef RTK_SYSLOG_SUPPORT
	extern int wlanlog_printk(const char *fmt, ...);
#else
	extern int scrlog_printk(const char * fmt, ...);
#endif
#ifdef CONFIG_RTK_MESH
/*
 *	NOTE: dot1180211sInfo.log_enabled content from webpage MIB_LOG_ENABLED (bitmap) (in AP/goahead-2.1.1/LINUX/fmmgmt.c  formSysLog)
 */
#ifndef RTK_SYSLOG_SUPPORT
	#define _LOG_MSG(fmt, args...)	if (1 & GET_MIB(priv)->dot1180211sInfo.log_enabled) scrlog_printk(fmt, ## args)
#else
	#define _LOG_MSG(fmt, args...)	if (1 & GET_MIB(priv)->dot1180211sInfo.log_enabled) wlanlog_printk(fmt, ## args)
#endif

	#define LOG_MESH_MSG(fmt, args...)	if (16 & GET_MIB(priv)->dot1180211sInfo.log_enabled) _LOG_MSG("%s: " fmt, priv->mesh_dev->name, ## args)
#else
#ifndef RTK_SYSLOG_SUPPORT
#ifdef __ECOS
	#define _LOG_MSG(fmt, args...)	diag_printf(fmt, ## args)
#else
	#define _LOG_MSG(fmt, args...)	scrlog_printk(fmt, ## args)
#endif
#else
	#define _LOG_MSG(fmt, args...)	wlanlog_printk(fmt, ## args)
#endif
#endif
#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}

#elif defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	#define _NOTICE	"NOTICElog_num:13;msg:"
	#define _DROPT	"DROPlog_num:13;msg:"
	#define _SYSACT "SYSACTlog_num:13;msg:"

	#define LOG_MSG_NOTICE(fmt, args...) _LOG_MSG("%s" fmt, _NOTICE, ## args)
	#define LOG_MSG_DROP(fmt, args...) _LOG_MSG("%s" fmt, _DROPT, ## args)
	#define LOG_MSG_SYSACT(fmt, args...) _LOG_MSG("%s" fmt, _SYSACT, ## args)
	#define LOG_MSG(fmt, args...)	{}

	#define LOG_START_MSG() { \
			char tmpbuf[10]; \
			LOG_MSG_NOTICE("Access Point: %s started at channel %d;\n", \
				priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, \
				priv->pmib->dot11RFEntry.dot11channel); \
			if (priv->pmib->dot11StationConfigEntry.autoRate) \
				strcpy(tmpbuf, "best"); \
			else \
				sprintf(tmpbuf, "%d", get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate)/2); \
			LOG_MSG_SYSACT("AP 2.4GHz mode Ready. Channel : %d TxRate : %s SSID : %s;\n", \
				priv->pmib->dot11RFEntry.dot11channel, \
				tmpbuf, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID); \
	}
#elif defined(CONFIG_RTL8196B_TLD)
	#define LOG_MSG_DEL(fmt, args...)	_LOG_MSG(fmt, ## args)
	#define LOG_MSG(fmt, args...)	{}
#else
	#define LOG_MSG(fmt, args...)	_LOG_MSG("%s: "fmt, priv->dev->name, ## args)
#endif
#else
	#if defined(__GNUC__) || defined(GREEN_HILL)
		#define LOG_MSG(fmt, args...)	{}
	#else
		#define LOG_MSG
	#endif
#endif // EVENT_LOG

#ifdef _USE_DRAM_
	#define DRAM_START_ADDR		0x81000000	// start address of internal data ram
#endif

#ifndef __ECOS
#ifdef __GNUC__
#define __WLAN_ATTRIB_PACK__		__attribute__ ((packed))
#define __PACK
#endif
#endif

#ifdef __arm
#define __WLAN_ATTRIB_PACK__
#define __PACK	__packed
#endif

#if defined(GREEN_HILL) || defined(__ECOS)
#define __WLAN_ATTRIB_PACK__
#define __PACK
#endif


/*=============================================================*/
/*-----------_ Driver module flags ----------------------------*/
/*=============================================================*/
#if defined(CONFIG_WIRELESS_LAN_MODULE) || defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	#define	MODULE_NAME		"Realtek WirelessLan Driver"
#if !defined(NOT_RTK_BSP) && !defined(CONFIG_RTL8672) && !defined(CONFIG_OPENWRT_SDK)
	#define	MODULE_VERSION	"v1.00"
#endif

	#define MDL_DEVINIT
	#define MDL_DEVEXIT
	#define MDL_INIT
	#define MDL_EXIT
	#define MDL_DEVINITDATA
#elif defined(__OSK__) || defined(__ECOS)
	#define MDL_DEVINIT		__devinit
	#define MDL_DEVEXIT		__devexit
	#define MDL_INIT		__init
	#define MDL_EXIT		__exit
	#define MDL_DEVINITDATA	__devinitdata
#else
//#ifdef CONFIG_RTL_8198C // modified by lynn_pu, 2014-10-16
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	#define MDL_DEVINIT		//__devinit
	#define MDL_DEVEXIT		//__devexit
	#define MDL_INIT		//__init
	#define MDL_EXIT		//__exit
	#define MDL_DEVINITDATA	//__devinitdata
#else
	#define MDL_DEVINIT		__devinit
	#define MDL_DEVEXIT		__devexit
	#define MDL_INIT		__init
	#define MDL_EXIT		__exit
	#define MDL_DEVINITDATA	__devinitdata
#endif
#endif


/*=============================================================*/
/*----------- System configuration ----------------------------*/
/*=============================================================*/
#if defined(CONFIG_RTL8196B_GW_8M)
#define NUM_TX_DESC		200
#elif defined(__OSK__)

#ifdef CONFIG_DUAL_CPU_SLAVE
    #define NUM_TX_DESC     512
#else
#ifdef SDRAM_SIZE_8MB
    #define NUM_TX_DESC		256
#else
    #define NUM_TX_DESC		512
#endif
#endif

#ifdef OSK_LOW_TX_DESC
    #define BE_TXDESC			NUM_TX_DESC
    #define NONBE_TXDESC		(BE_TXDESC>>2)
#endif
#else
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD)  || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198B)  || (defined(CONFIG_LUNA_DUAL_LINUX) || defined(CONFIG_ARCH_LUNA_SLAVE)) || defined(CONFIG_RTL8672)
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
#define NUM_TX_DESC		640
#elif defined(__ECOS) && defined(CYGNUM_RAM_SIZE_0x00800000)
#define NUM_TX_DESC 	128
#else
#ifdef CONFIG_RTL_8812_SUPPORT
	#ifdef CONFIG_RTL_8812AR_VN_SUPPORT
		#define NUM_TX_DESC	480
	#else
		#define NUM_TX_DESC	768
	#endif
#elif  defined(CONFIG_RTL_8196E)
	#define NUM_TX_DESC		300//256
#elif defined(CONFIG_WLAN_HAL_8881A)
	#define NUM_TX_DESC		480
#elif (defined(CONFIG_RTL_8198C) && defined(CONFIG_WLAN_HAL_8814AE)) || defined(CONFIG_WLAN_HAL_8822BE)
	#ifdef WMM_DSCP_C42
	#define NUM_TX_DESC    2048
	#else
    #define NUM_TX_DESC    1024 //768
	#endif
#elif defined(CONFIG_RTL_819XD) && defined(CONFIG_WLAN_HAL_8814AE)
	#if defined(CONFIG_SLOT_0_8814AE) || defined(CONFIG_SLOT_1_8814AE)
		#define NUM_TX_DESC     1024
	#else // 8194 only
		#define NUM_TX_DESC     256
	#endif
#elif (defined(CONFIG_SLOT_0_8194AE) || defined(CONFIG_SLOT_1_8194AE)) && defined(CONFIG_RTL8672)
    #define NUM_TX_DESC    768
#elif defined(CONFIG_RTL_8196D) && defined(CONFIG_WLAN_HAL_8192EE)
	#define NUM_TX_DESC		300
#else
	#define NUM_TX_DESC		512
#endif

#ifdef CONFIG_RTL_8723B_SUPPORT
#define NUM_TX_DESC 		480
#endif

#endif
#ifdef __ECOS
	#ifdef OSK_LOW_TX_DESC
    	#define BE_TXDESC           NUM_TX_DESC
		#define NONBE_TXDESC        (BE_TXDESC>>2)
	#endif
#endif
#elif defined(NOT_RTK_BSP)
#if defined(RTK_129X_PLATFORM)
#define NUM_TX_DESC		256
#elif defined(CONFIG_WLAN_HAL_8814AE)|| defined(CONFIG_WLAN_HAL_8822BE)
#define NUM_TX_DESC		2176 //512		// kmalloc max size issue
#else
#define NUM_TX_DESC		1024//512
#endif
#else
#define NUM_TX_DESC		256		// kmalloc max size issue
#endif
#endif

//#define NUM_TX_DESC_HQ    64

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
#define RTK_AC_SUPPORT
#define dybw_tx
#define dybw_rx
#define dybw_beacon
#endif

#define dybw_thrd	3

#ifdef CONFIG_RTL_NFJROM_MP
	#define NUM_RX_DESC		64
#elif defined(__OSK__)

#ifdef CONFIG_DUAL_CPU_SLAVE
    #define NUM_RX_DESC     320 //256
#else
#ifdef SDRAM_SIZE_8MB
    #define NUM_RX_DESC		128
#else
    #define NUM_RX_DESC		320	//256
#endif
#endif

#elif defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8196E) || (defined(CONFIG_LUNA_DUAL_LINUX) || defined(CONFIG_ARCH_LUNA_SLAVE)) || defined(CONFIG_RTL8672)
	#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D)
		#define NUM_RX_DESC		256
	#elif defined(__ECOS) && defined(CYGNUM_RAM_SIZE_0x00800000)
		#define NUM_RX_DESC 	320
	#else
		#ifdef CONFIG_RTL_8812_SUPPORT
			#ifdef CONFIG_RTL_8812AR_VN_SUPPORT
			#define NUM_RX_DESC 	256
			#elif defined(CONFIG_RTL_SHRINK_MEMORY_SIZE)
			#define NUM_RX_DESC 	256
			#else
			#define NUM_RX_DESC 	512
			#endif
		#elif defined(CONFIG_RTL_8196E)
            #ifdef MULTI_MAC_CLONE
			#define NUM_RX_DESC 	512
            #else
			#define NUM_RX_DESC 	256
            #endif
		#elif  defined(CONFIG_WLAN_HAL_8881A)
			#define NUM_RX_DESC		480	//256
        #elif (defined(CONFIG_WLAN_HAL_8814AE) && defined(CONFIG_RTL_8198C)) || defined(CONFIG_WLAN_HAL_8822BE)
			#ifndef CONFIG_RTL_SHRINK_MEMORY_SIZE
			#if defined(CONFIG_WLAN_HAL_8822BE) && defined(CONFIG_OPENWRT_SDK)
				#define NUM_RX_DESC     512
			#else
				#define NUM_RX_DESC     1024 //512 //256
			#endif
			#else
				#define NUM_RX_DESC     256
			#endif
        #elif defined(CONFIG_WLAN_HAL_8814AE) && defined(CONFIG_RTL_819XD)
			#if defined(CONFIG_SLOT_0_8814AE) || defined(CONFIG_SLOT_1_8814AE)
				#define NUM_RX_DESC     1024
			#else // 8194 only
				#define NUM_RX_DESC     256
			#endif
		#elif (defined(CONFIG_SLOT_0_8194AE) || defined(CONFIG_SLOT_1_8194AE)) && defined(CONFIG_RTL8672)
		#define NUM_RX_DESC     768
		#elif defined(CONFIG_RTL_8196D) && defined(CONFIG_OPENWRT_SDK)
			#define NUM_RX_DESC		512
        #elif defined(CONFIG_RTL_8197F)
			#ifndef CONFIG_RTL_SHRINK_MEMORY_SIZE
				#define NUM_RX_DESC		512
			#else
				#define NUM_RX_DESC		128
			#endif
		#elif defined(CONFIG_RTL_8196D) && defined(CONFIG_WLAN_HAL_8192EE)
			#define NUM_RX_DESC     256
		#else
			#ifdef CONFIG_VERIWAVE_CHECK
			//enlarge RX buffer because small packets of VERIWAVE
			#define NUM_RX_DESC		512
			#else
			#define NUM_RX_DESC		128
			#endif
		#endif
	#endif
#elif defined(CONFIG_RTL_92D_SUPPORT)
	#define NUM_RX_DESC		80
#elif defined(__ECOS)
	#define NUM_RX_DESC		64
	#undef NUM_TX_DESC
	#define NUM_TX_DESC		512
#elif defined(NOT_RTK_BSP)
	#if defined(RTK_129X_PLATFORM)
		#define NUM_RX_DESC 	256
	#elif defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
		#define NUM_RX_DESC		2048
	#elif defined(CONFIG_RTL_8812_SUPPORT)
		#define NUM_RX_DESC		2048
	#else
		#define NUM_RX_DESC		512//128
	#endif
#else
	#ifdef RX_BUFFER_GATHER
		#ifdef CONFIG_VERIWAVE_CHECK
		//enlarge RX buffer because small packets of VERIWAVE
		#define NUM_RX_DESC		512
		#else
		#define NUM_RX_DESC		64
		#endif
	#else
		#define NUM_RX_DESC		32
	#endif
#endif

#ifdef CONFIG_RTL_NFJROM_MP
	#define NUM_RX_DESC_2G NUM_RX_DESC
#elif defined(CONFIG_WLAN_HAL_8197F) && defined(CONFIG_WLAN_HAL_8822BE)
	#define NUM_RX_DESC_2G 400  /* wlan1 97F */
#else
	#define NUM_RX_DESC_2G NUM_RX_DESC
#endif



#define NUM_RX_DESC_IF(priv)  (priv->pshare->wlandev_idx == (0^WLANIDX)?NUM_RX_DESC:NUM_RX_DESC_2G)

#define CURRENT_NUM_TX_DESC	priv->pshare->current_num_tx_desc
#if defined (CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_92D_DMDP)
#define MAX_NUM_TX_DESC_DMDP		256
#endif

#ifdef DELAY_REFILL_RX_BUF
	#define REFILL_THRESHOLD	NUM_RX_DESC
	#define REFILL_THRESHOLD_IF(priv)	NUM_RX_DESC_IF(priv)
#endif


#ifdef CONFIG_RTL_88E_SUPPORT
#if defined(__ECOS) || defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#define RTL8188E_NUM_STAT 32
#else
#define RTL8188E_NUM_STAT 64
#endif
#endif

/* do not modify this*/
#define RTL8192CD_NUM_STAT         32         //92c / 92d // 88c
#define FW_NUM_STAT               128         // 8812 / 8192E / 8881A / Other new ic  FW_NUM_STAT

#if (defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)) && defined(MBSSID)
#define NUM_CMD_DESC	2
#else
#define NUM_CMD_DESC	16
#endif

#ifdef HW_FILL_MACID
#define HWMACID_RESULT_SUCCESS          0
#define HWMACID_RESULT_NOT_READY        1
#define HWMACID_RESULT_FAIL             2
#endif

#ifdef HW_DETEC_POWER_STATE
#define HW_MACID_SEARCH_NOT_READY   0x7E
#define HW_MACID_SEARCH_FAIL        0x7F
#define HW_MACID_SEARCH_SUPPORT_NUM (HW_MACID_SEARCH_FAIL-1)
#endif

#ifdef CONFIG_RTL_NFJROM_MP
    #define NUM_STAT		8
#elif defined(__ECOS)
    #define NUM_STAT        16
#elif defined(__OSK__)
    #define NUM_STAT		16
#elif defined(MULTI_MAC_CLONE) && defined(CONFIG_WLAN_HAL_8192EE)
    #define NUM_STAT        63
#elif defined(CONFIG_RTL_88E_SUPPORT)
    #define NUM_STAT		(RTL8188E_NUM_STAT - 1)
#elif defined(__ECOS)
    #define NUM_STAT		(32 - 1)
#elif defined(CONFIG_RTL_SHRINK_MEMORY_SIZE)
	#define NUM_STAT		16	// for reduce ram size
#else
    #define NUM_STAT		64 //127
#endif

//Auto enable CAM_SWAP if (station number > cam entries)
#if (NUM_STAT>32)
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
#ifndef CAM_SWAP
#define CAM_SWAP
#endif
#endif
#endif

#if (NUM_STAT>64)
#ifndef CAM_SWAP
#define CAM_SWAP
#endif
#endif

#define MAX_GUEST_NUM   NUM_STAT

#define NUM_TXPKT_QUEUE			64
#define NUM_APSD_TXPKT_QUEUE	32
#define NUM_DZ_MGT_QUEUE	16

#ifdef CONFIG_WLAN_HAL
#define PRE_ALLOCATED_HDR		(NUM_TX_DESC*2)
#else
#define PRE_ALLOCATED_HDR		NUM_TX_DESC
#endif

#ifdef HS2_SUPPORT
#define PRE_ALLOCATED_MMPDU		32
#define PRE_ALLOCATED_BUFSIZE	((800+128)/4)		// 800 bytes long should be enough for HS2 R2 test cases! Declare as unsigned int
#elif defined(DRVMAC_LB)
#define PRE_ALLOCATED_MMPDU		32
#define PRE_ALLOCATED_BUFSIZE	(2048/4)		// 600 bytes long should be enough for mgt! Declare as unsigned int
#else
#define PRE_ALLOCATED_MMPDU		64
#define PRE_ALLOCATED_BUFSIZE	((600+128)/4)		// 600 bytes long should be enough for mgt! Declare as unsigned int
#endif

#if defined(RTK_NL80211)  //eric-sync ??
#define MAX_BSS_NUM		32
#elif defined(SDRAM_LE_32M)
#define MAX_BSS_NUM		32
#else
#define MAX_BSS_NUM		64
#endif

#define MAX_NUM_WLANIF		4
#define WLAN_MISC_MAJOR		13

#define MAX_FRAG_COUNT		16

#define NUM_MP_SKB		32

#define SUPPORT_CH_NUM	64

// unit of time out: 10 msec
#define AUTH_TO			RTL_SECONDS_TO_JIFFIES(5)
#define ASSOC_TO		RTL_SECONDS_TO_JIFFIES(5)
#define FRAG_TO			RTL_SECONDS_TO_JIFFIES(20)
#define SS_TO_SYN			RTL_MILISECONDS_TO_JIFFIES(50)
#define SS_TO			RTL_MILISECONDS_TO_JIFFIES(50)
#define SS_PSSV_TO		RTL_MILISECONDS_TO_JIFFIES(120)			// passive scan for 120 ms



#define P2P_SEARCH_TIME_V   120
#define P2P_SEARCH_TIME		RTL_MILISECONDS_TO_JIFFIES(P2P_SEARCH_TIME_V)


#ifdef CONFIG_RTL_11W_SUPPORT
#ifdef CONFIG_RTL_11W_CLI_SUPPORT
#define  CONFIG_IEEE80211W_CLI
//#define  CONFIG_IEEE80211W_CLI_DEBUG
#endif
#define CONFIG_IEEE80211W
#define CONFIG_IEEE80211W_CMD
//#define  CONFIG_IEEE80211W_AP_DEBUG
//#define  PMF_DEBUGMSG
#endif

#ifdef CONFIG_RTL_11R_SUPPORT
#define CONFIG_IEEE80211R
#define SUPPORT_FAST_CONFIG
#endif

#ifdef CONFIG_RTL_11V_SUPPORT
#define CONFIG_IEEE80211V
//#define CONFIG_IEEE80211V_CLI	//client mode
//#define DOT11V_DEBUG
#endif

#ifdef CONFIG_RTL_PMKCACHE_SUPPORT
#define CONFIG_PMKCACHE
#endif

#ifdef CONFIG_IEEE80211W
#define SA_QUERY_RETRY_TO				201
#define SA_QUERY_MAX_TO					1000
#define SA_QUERY_MAX_NUM				5
#endif

#ifdef CONFIG_RTL_NEW_AUTOCH
#define SS_AUTO_CHNL_TO		RTL_MILISECONDS_TO_JIFFIES(200)
#ifdef __OSK__
#define SS_AUTO_CHNL_NHM_TO		RTL_MILISECONDS_TO_JIFFIES(150)	// workaround for osk timer issue
#else
#define SS_AUTO_CHNL_NHM_TO		RTL_MILISECONDS_TO_JIFFIES(100)
#endif
#define SS_AUTO_CHNL_ACS2_TO		RTL_MILISECONDS_TO_JIFFIES(200)
#endif

#ifdef CONFIG_RTK_MESH
//GANTOE for automatic site survey 2008/12/10
#define SS_RAND_DEFER		300
#if defined(RTK_MESH_AODV_STANDALONE_TIMER)
#define MESH_AODV_EXPIRE_TO	RTL_MILISECONDS_TO_JIFFIES(100)
#endif
#endif
#ifdef LINUX_2_6_22_
#define EXPIRE_TO		RTL_SECONDS_TO_JIFFIES(1)
#else
#define EXPIRE_TO		RTL_SECONDS_TO_JIFFIES(1)
#endif
#define REAUTH_TO		RTL_SECONDS_TO_JIFFIES(5)
#define REASSOC_TO		RTL_SECONDS_TO_JIFFIES(5)
#define REAUTH_LIMIT	6
#define REASSOC_LIMIT	6

#ifdef CONFIG_VERIWAVE_CHECK
#define CHK_VERIWAVE_TO		100000
#endif

#define DEFAULT_OLBC_EXPIRE		60

#define GBWC_TO			RTL_MILISECONDS_TO_JIFFIES(250)
#define SBWC_TO			RTL_MILISECONDS_TO_JIFFIES(250)

#ifdef __DRAYTEK_OS__
#define SS_COUNT		2
#else
#define SS_COUNT		3
#endif

#define TUPLE_WINDOW	128

#define RC_TIMER_NUM	64
#define RC_ENTRY_NUM	128
#define AMSDU_TIMER_NUM	64
#define AMPDU_TIMER_NUM	64
#define SWQ_TIMER_NUM	(NUM_STAT*4)
#define SWQ_HWTIMER_TOLERANCE	500 // us
#define SWQ_HWTIMER_MINIMUN	1000 // us
#define SWQ_HWTIMER_MAXIMUN	500000 // us
#define SWQ_HWTIMER_DELAY	34 // us
#define SWQ_TIMEOUT_THD  300 // ms
#define SWQ_TIMEOUT_THD_LOWER  3 // count



#define ROAMING_DECISION_PERIOD_INFRA	5
#define ROAMING_DECISION_PERIOD_ADHOC	10
#define ROAMING_DECISION_PERIOD_ARRAY 	(MAX_NUM(ROAMING_DECISION_PERIOD_ADHOC,ROAMING_DECISION_PERIOD_INFRA)+1)
#define ROAMING_THRESHOLD		1	// roaming will be triggered when rx
									// beacon percentage is less than the value
#define FAST_ROAMING_THRESHOLD	40

/* below is for security.h  */
#define MAXDATALEN		1560
#define MAXQUEUESIZE	8	//WPS2DOTX
#define MAXRSNIELEN		128
#define E_DOT11_2LARGE	-1
#define E_DOT11_QFULL	-2
#define E_DOT11_QEMPTY	-3
#ifdef WIFI_SIMPLE_CONFIG
#define PROBEIELEN		260
#endif
#if defined( CONFIG_IEEE80211R) ||defined(CONFIG_IEEE80211V)
#define MAX_R0KHID_LEN				48
#define FT_R0KH_R1KH_PULL_NONCE_LEN	16
#define FT_PMKID_LEN				16
#define FT_PMK_LEN					32
#define MAX_FTACTION_LEN			MAXDATALEN - 20
#define MAX_FTREASSOC_DEADLINE		65535
#define MAX_TRANS_LIST_NUM         	32
#endif

#define GKEY_ID_SECOND 	2

// for SW LED
#define LED_MAX_PACKET_CNT_B	400
#define LED_MAX_PACKET_CNT_AG	1200
#define LED_MAX_SCALE			100
#define LED_NOBLINK_TIME		RTL_SECONDS_TO_JIFFIES(15)/10	// time more than watchdog interval
#define LED_INTERVAL_TIME		RTL_MILISECONDS_TO_JIFFIES(500)	// 500ms
#define LED_ON_TIME				RTL_MILISECONDS_TO_JIFFIES(40)	// 40ms
#define LED_UCFGING_TIME				RTL_MILISECONDS_TO_JIFFIES(120)	// 100ms
#define LED_ON					0
#define LED_OFF					1
#define LED_0 					0
#define LED_1 					1
#define LED_2					2

// for counting association number
#define INCREASE		1
#define DECREASE		0

// DFS
#define CH_AVAIL_CHK_TO			RTL_SECONDS_TO_JIFFIES(62)	 // 62 seconds
#define CH_AVAIL_CHK_TO_CE              RTL_SECONDS_TO_JIFFIES(602)      // 602 seconds


/*adjusted for support AMSDU*/
#if defined(RTK_AC_SUPPORT) //&& !defined(CONFIG_RTL_8198B)

    #ifdef CONFIG_RTL_AC2G_256QAM
    #define AC2G_256QAM
    #endif

    #ifdef __ECOS
    #define MAX_SKB_BUF     MCLBYTES //2048
    #define MAX_RX_BUF_LEN	(MAX_SKB_BUF -sizeof(struct skb_shared_info) - 32)
    #define MIN_RX_BUF_LEN  MAX_RX_BUF_LEN
    #else
        #ifdef RX_BUFFER_GATHER
            #ifdef CONFIG_RTL_8812_SUPPORT
                #if defined(SUPPORT_RX_AMSDU_AMPDU)//8812_RX_AMSDU_AMPDU
                #define MAX_RX_BUF_LEN  8800
                #define MIN_RX_BUF_LEN  8800
                #else
                #define MAX_RX_BUF_LEN  3000
                #define MIN_RX_BUF_LEN  3000
                #endif
            #else
                #if defined(SUPPORT_RX_AMSDU_AMPDU)
                #define MAX_RX_BUF_LEN  8800
                #define MIN_RX_BUF_LEN  8800
                #else
                #define MAX_RX_BUF_LEN  2600
                #define MIN_RX_BUF_LEN  2600
                #endif
            #endif
            #if defined(CONFIG_OPENWRT_SDK) && !defined(SUPPORT_RX_AMSDU_AMPDU)
            #undef MAX_RX_BUF_LEN
            #undef MIN_RX_BUF_LEN
            #define MAX_RX_BUF_LEN  4000
            #define MIN_RX_BUF_LEN  4000
            #endif
        #else
            #define MAX_RX_BUF_LEN  12000
            #define MIN_RX_BUF_LEN  4600
        #endif
    #endif

#else // ! RTK_AC_SUPPORT

    #ifdef RX_BUFFER_GATHER
        #ifdef __LINUX_2_6__
        #define MAX_SKB_BUF     2344 //2280
        #elif defined(__ECOS)
        #define MAX_SKB_BUF     MCLBYTES //2048
        #else
        #define MAX_SKB_BUF     2048
        #endif

        #ifdef __ECOS
        #define MAX_RX_BUF_LEN  (MAX_SKB_BUF -sizeof(struct skb_shared_info) - 32)
        #else
        #define MAX_RX_BUF_LEN  (MAX_SKB_BUF -sizeof(struct skb_shared_info) - 128)
        #endif
        #define MIN_RX_BUF_LEN MAX_RX_BUF_LEN
    #else //#ifdef RX_BUFFER_GATHER
        #if defined(__ECOS) //mark_ecos
        #define MAX_SKB_BUF     MCLBYTES //2048
        #define MAX_RX_BUF_LEN	(MAX_SKB_BUF -sizeof(struct skb_shared_info) - 32)
        #define MIN_RX_BUF_LEN  MAX_RX_BUF_LEN
        #else //
        #define MAX_RX_BUF_LEN	8400
        #define MIN_RX_BUF_LEN	4400
        #endif
    #endif //#ifdef RX_BUFFER_GATHER
#endif

#if defined(CONFIG_WLAN_HAL_8197F) && defined(CONFIG_WLAN_HAL_8822BE)
#ifdef __ECOS
	#define MAX_RX_BUF_LEN_1  (MAX_SKB_BUF -sizeof(struct skb_shared_info) - 32)
	#define MIN_RX_BUF_LEN_1 MAX_RX_BUF_LEN_1  //wlan1 97F
#else
    #define MAX_RX_BUF_LEN_1 2600  //wlan1 97F
    #define MIN_RX_BUF_LEN_1 2600  //wlan1 97F
    #undef MAX_RX_BUF_LEN
    #undef MIN_RX_BUF_LEN
	#ifdef RX_BUFFER_GATHER_REORDER
    #define MAX_RX_BUF_LEN	1820   // wlan0 8822
    #define MIN_RX_BUF_LEN	1820   // wlan0 8822
	#else
    #define MAX_RX_BUF_LEN  4000   // wlan0 8822
    #define MIN_RX_BUF_LEN  4000   // wlan0 8822
    #endif
#if defined(CONFIG_OPENWRT_SDK) // fix openwrt webpage memory issue , rx buf len need to greater than 4000
		#undef MAX_RX_BUF_LEN_1
		#undef MIN_RX_BUF_LEN_1
		#define MAX_RX_BUF_LEN_1 MAX_RX_BUF_LEN //4000
		#define MIN_RX_BUF_LEN_1 MIN_RX_BUF_LEN
#endif
#endif
#else
    #define MAX_RX_BUF_LEN_1 MAX_RX_BUF_LEN
    #define MIN_RX_BUF_LEN_1 MIN_RX_BUF_LEN
#endif


/* for RTL865x suspend mode */
#define TP_HIGH_WATER_MARK 55 //80 /* unit: Mbps */
#define TP_LOW_WATER_MARK 35 //40 /* unit: Mbps */

#define FW_BOOT_SIZE	400
#define FW_MAIN_SIZE	52000
#define FW_DATA_SIZE	850

#ifdef CONFIG_WLAN_HAL_8881A
#define AGC_TAB_SIZE    2400
#else
#define AGC_TAB_SIZE	1600
#endif
//#define PHY_REG_SIZE	2048
#define PHY_REG_SIZE	4096
//#define MAC_REG_SIZE	1200
#define MAC_REG_SIZE	1420
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
#define PHY_REG_PG_SIZE 2560
#else
#define PHY_REG_PG_SIZE 256
#endif

#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_WLAN_HAL_8197F)
// TODO: temporary for 8814AE file size
#undef AGC_TAB_SIZE
#undef PHY_REG_SIZE
#undef MAC_REG_SIZE
#undef PHY_REG_PG_SIZE
#define AGC_TAB_SIZE	5120
#define PHY_REG_SIZE	18432
#define MAC_REG_SIZE	2048
#define PHY_REG_PG_SIZE 9216
#endif

#define PHY_REG_1T2R	256
#define PHY_REG_1T1R	256
#define FW_IMEM_SIZE	40*(1024)
#define FW_EMEM_SIZE	50*(1024)
#define FW_DMEM_SIZE	48

// for PCIe power saving
#define POWER_DOWN_T0	(10*HZ)
#define PKT_PAGE_SZ 	128
#define TX_DESC_SZ 		32


#ifdef SUPPORT_TX_MCAST2UNI
#define MAX_IP_MC_ENTRY		16
#define MAX_IP_SRC_ENTRY		8

#define MAX_FLOODING_MAC_NUM 32
#endif

#ifndef CALIBRATE_BY_ODM
#define IQK_ADDA_REG_NUM	16
#define MAX_TOLERANCE		5
#if defined(HIGH_POWER_EXT_PA) && defined(CONFIG_RTL_92C_SUPPORT)
#define	IQK_DELAY_TIME		20		//ms
#else
#define	IQK_DELAY_TIME		1		//ms
#endif
#define IQK_MAC_REG_NUM		4
#endif


#define SKIP_MIC_NUM	300


// for dynamic mechanism of reserving tx desc
#ifdef RESERVE_TXDESC_FOR_EACH_IF
#define IF_TXDESC_UPPER_LIMIT	70	// percentage
#endif
#ifdef USE_TXQUEUE
#define IF_TXQ_UPPER_LIMIT		85	// percentage
#endif


// for dynamic mechanism of retry count
#define RETRY_TRSHLD_H	3750000
#define RETRY_TRSHLD_L	3125000
#define MP_PSD_SUPPORT 1
//-------------------------------------------------------------
// Define flag for 8M gateway configuration
//-------------------------------------------------------------
#if defined(CONFIG_RTL8196B_GW_8M)

#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

#undef NUM_STAT
#define NUM_STAT		16

#endif // CONFIG_RTL8196B_GW_8M


#if defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8198_AP_ROOT)

#ifdef DOT11D
	#undef DOT11D
#endif


#ifdef MBSSID
	#undef RTL8192CD_NUM_VWLAN
	#define RTL8192CD_NUM_VWLAN  1
#endif

#undef NUM_STAT
#define NUM_STAT		16


#endif //defined(CONFIG_RTL8196C_AP_ROOT)
#ifdef CONFIG_RTL_8198_NFBI_RTK_INBAND_AP //mark_nfbi_inband_ap
#ifdef MBSSID
        #undef RTL8192CD_NUM_VWLAN
        #define RTL8192CD_NUM_VWLAN  7
#endif
#endif


#if defined(CONFIG_RTL8196C_CLIENT_ONLY)

#ifdef DOT11D
	#undef DOT11D
#endif

#endif

#ifdef CONCURRENT_MODE
#define NUM_WLAN_IFACE 2
#endif

#ifdef CONFIG_NET_PCI
#ifndef NUM_WLAN_IFACE
#define NUM_WLAN_IFACE 2
#endif
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_USB_POWER_BUS)
#define USB_POWER_SUPPORT
#endif

#ifdef WIFI_SIMPLE_CONFIG
#define MAX_WSC_IE_LEN		(256+128)
#define MAX_WSC_PROBE_STA	10
#endif

#ifdef __OSK__
#define MAX_PROBE_REQ_STA	16
#elif defined(__ECOS)
#define MAX_PROBE_REQ_STA	16
#else
#define MAX_PROBE_REQ_STA	64
#endif

#ifdef CONFIG_WIRELESS_LAN_MODULE //BE_MODULE
#ifndef __OSK__
#undef __DRAM_IN_865X
#undef __IRAM_IN_865X
#undef __MIPS16

#define __DRAM_IN_865X
#define __IRAM_IN_865X
#define __MIPS16
#endif

#ifndef RTK_NL80211
#undef BR_SHORTCUT
#endif
#if defined(NOT_RTK_BSP) && defined(BR_SHORTCUT_SUPPORT)
#define BR_SHORTCUT
#endif // NOT_RTK_BSP && BR_SHORTCUT_SUPPORT
#undef CONFIG_RTL865X_ETH_PRIV_SKB
#undef CONFIG_RTL_ETH_PRIV_SKB
#undef CONFIG_RTK_VLAN_SUPPORT
#endif
#if defined(CONFIG_RTL8672) && defined(LINUX_2_6_22_) && defined(WIFI_LIMITED_MEM)
	#undef NUM_STAT
	#define NUM_STAT	16
#endif

#if defined(CONFIG_RTL_819X) && defined(__OSK__) && defined(CONFIG_RTL_8196E)
#define CONFIG_AUTO_PCIE_PHY_SCAN
#endif

//-------------------------------------------------------------
// Option: Use kernel thread to execute Tx Power Tracking function.
//-------------------------------------------------------------
#ifdef CONFIG_RTL_TPT_THREAD
#define TPT_THREAD
#endif


//-------------------------------------------------------------
// Define flag of RTL8812 features
//-------------------------------------------------------------
#ifdef CONFIG_RTL_8812_SUPPORT

//#undef DETECT_STA_EXISTANCE

#define USE_OUT_SRC					1

//#ifdef CONFIG_RTL_92C_SUPPORT
//#define _OUTSRC_COEXIST
//#endif

//for 11ac logo +++
//#define BEAMFORMING_SUPPORT			1
#ifndef __ECOS
//#define SUPPORT_TX_AMSDU //disable this, because aput only needs rx amsdu. (but testbed needs both rx & tx amsdu)
#endif
//for 11ac logo ---

#define _TRACKING_TABLE_FILE
#define TX_PG_8812

#endif

#ifdef	CONFIG_RTL_8723B_SUPPORT
#define _TRACKING_TABLE_FILE
#define AVG_THERMAL_NUM_8723B    4
#endif

//-------------------------------------------------------------
// SKB NUM
//-------------------------------------------------------------
#ifdef CONFIG_RTL8190_PRIV_SKB
		#ifdef DELAY_REFILL_RX_BUF
			#if defined(CONFIG_RTL_NFJROM_MP) && defined(CONFIG_RTL_8196E)
				#define MAX_SKB_NUM		100
			#elif defined(CONFIG_RTL8196B_GW_8M)
				#define MAX_SKB_NUM		100
			#elif defined(CONFIG_RTL8672)
				#if defined(WIFI_LIMITED_MEM)
					#if defined(LINUX_2_6_22_)
						#define MAX_SKB_NUM		96
					#else
						#define MAX_SKB_NUM		160
					#endif
				#else
					#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
						#define MAX_SKB_NUM	1800
					#else
					#define MAX_SKB_NUM		768
				#endif
				#endif
			#elif defined(CONFIG_RTL_8196E)
				#if defined(CONFIG_WLAN_HAL_8192EE)
    		        #ifdef MULTI_MAC_CLONE
					#define MAX_SKB_NUM 	(NUM_RX_DESC + 64)
		            #else
					#define MAX_SKB_NUM 	400 //256
		            #endif
				#else
					#define MAX_SKB_NUM	256
				#endif
			#elif defined(CONFIG_WLAN_HAL_8881A)
					#define MAX_SKB_NUM	480
            #elif defined(CONFIG_WLAN_HAL_8197F)
            	#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
					#ifndef CONFIG_RTL_SHRINK_MEMORY_SIZE
                   		 #define MAX_SKB_NUM	1024//2048 //768 //1024 for amsdu
                   	#else
						 #define MAX_SKB_NUM	768
					#endif
				#else
					#ifdef CONFIG_RTL_SHRINK_MEMORY_SIZE
						#define MAX_SKB_NUM	768
					#else
                    	#define MAX_SKB_NUM	1500
					#endif
                #endif
			#elif defined(CONFIG_RTL_8198C)
				#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
                    #define MAX_SKB_NUM	1024 //768
				#else
					#define MAX_SKB_NUM     512
				#endif
			#elif defined(CONFIG_RTL_8198_GW) || defined(CONFIG_RTL_8198_AP_ROOT) || defined(CONFIG_RTL_819XD)
				#if defined(CONFIG_RTL_8812_SUPPORT)
					#ifdef CONFIG_RTL_8812AR_VN_SUPPORT
				    	#define MAX_SKB_NUM	480
					#elif defined(CONFIG_RTL_8198C)
						#define MAX_SKB_NUM	256
					#else
						#define MAX_SKB_NUM	768
					#endif
				#elif defined(CONFIG_RTL_819XD) && defined(CONFIG_WLAN_HAL_8814AE)
					#if defined(CONFIG_SLOT_1_8814AE) || defined(CONFIG_SLOT_1_8814AE)
						#define MAX_SKB_NUM	1024
					#else // 8194 only
						#define MAX_SKB_NUM	256
					#endif
				#else
#ifdef __ECOS
					#define MAX_SKB_NUM	256
#else
					#define MAX_SKB_NUM	480//256
#endif
				#endif
			#elif defined(CONFIG_RTL_8723B_SUPPORT)
					#define MAX_SKB_NUM 480
			#elif defined(CONFIG_RTL_92D_SUPPORT)
				#ifdef CONFIG_RTL_8198_AP_ROOT
					#define MAX_SKB_NUM		210
				#else
					#define MAX_SKB_NUM		256
				#endif
			#elif defined( __ECOS)
				#define MAX_SKB_NUM		256
			#else
				#ifdef UNIVERSAL_REPEATER
					#define MAX_SKB_NUM		256
				#else
					#define MAX_SKB_NUM		160
				#endif
			#endif
		#else
			#define MAX_SKB_NUM		580
		#endif

#if defined(CONFIG_WLAN_HAL_8197F) && defined(CONFIG_WLAN_HAL_8822BE)
	#define MAX_SKB_NUM_1 400  /* wlan1 97F */
#else
	#define MAX_SKB_NUM_1 MAX_SKB_NUM
#endif

#endif


//-------------------------------------------------------------
// NFJROM CONFIG
//-------------------------------------------------------------
#ifdef CONFIG_RTL_NFJROM_MP
#undef NUM_TX_DESC
#undef NUM_RX_DESC
#undef NUM_STAT
#undef NUM_TXPKT_QUEUE
#undef NUM_APSD_TXPKT_QUEUE
#undef NUM_DZ_MGT_QUEUE
#ifdef __ECOS
/*Ecos's skb buffer is limited to cluster size.using the value in MP the same as in normal driver*/
#else
#undef MAX_RX_BUF_LEN
#undef MIN_RX_BUF_LEN
#endif

#define NUM_TX_DESC				64
#define NUM_RX_DESC				64
#define NUM_STAT				1
#define NUM_TXPKT_QUEUE		8
#define NUM_APSD_TXPKT_QUEUE	1
#define NUM_DZ_MGT_QUEUE		1
#ifdef __ECOS
/*Ecos's skb buffer is limited to cluster size. using the value in MP the same as in normal driver*/
#else
#define MAX_RX_BUF_LEN  			3000
#define MIN_RX_BUF_LEN  			3000
#endif
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
#define RTK_5G_SUPPORT
#endif

#ifdef RTK_5G_SUPPORT
#define MAX_CHANNEL_NUM		76
#else
#define MAX_CHANNEL_NUM		MAX_2G_CHANNEL_NUM
#endif

#define NUM_TX_DESC_HQ 	(NUM_TX_DESC>>3)


//-------------------------------------------------------------
//  802.11h TPC
//-------------------------------------------------------------
#ifdef RTK_5G_SUPPORT
#define DOT11H
#endif

//-------------------------------------------------------------
// Support 802.11d
//-------------------------------------------------------------
#if defined(RTK_5G_SUPPORT) || defined(CONFIG_RTL_80211D_SUPPORT)
#define DOT11D
#endif

//-------------------------------------------------------------
// Support 802.11k
//-------------------------------------------------------------
#ifdef CONFIG_RTL_DOT11K_SUPPORT
#define DOT11K
#ifndef CH_LOAD_CAL
#define CH_LOAD_CAL
#endif
#endif
#ifdef DOT11K
#define DOT11D
#endif

//-------------------------------------------------------------
// Support smart roaming
//-------------------------------------------------------------
#ifdef CONFIG_RTK_SMART_ROAMING
#define RTK_SMART_ROAMING
#ifndef CH_LOAD_CAL
#define CH_LOAD_CAL
#endif
#endif
#ifdef RTK_SMART_ROAMING
#define MAX_NEIGHBOR_STA	64
#endif


#if 0//defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
#define CU_TO           RTL_MILISECONDS_TO_JIFFIES(210)         // 200ms to calculate bbp channel load
#define CU_Intval       200
#endif

//#define CHK_RX_ISR_TAKES_TOO_LONG

//This is for LUNA SDK - Apollo to config 8812 in slave CPU and shift mem 33M
#if defined(CONFIG_ARCH_LUNA_SLAVE) && !defined(CONFIG_WLAN_HAL)
#define CONFIG_LUNA_SLAVE_PHYMEM_OFFSET CONFIG_RTL8686_DSP_MEM_BASE
#else
#define CONFIG_LUNA_SLAVE_PHYMEM_OFFSET 0x0
#endif

#if !defined(CONFIG_LUNA_DUAL_LINUX) && defined(CONFIG_ARCH_LUNA_SLAVE)
#define CONFIG_LUNA_DUAL_LINUX
#endif

#ifdef CONFIG_USB_HCI
#define USB_INTERFERENCE_ISSUE // this should be checked in all usb interface
//#define USB_LOCK_ENABLE
//#define CONFIG_USB_VENDOR_REQ_MUTEX
#define CONFIG_VENDOR_REQ_RETRY
//#define CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC
#define CONFIG_USB_VENDOR_REQ_BUFFER_DYNAMIC_ALLOCATE
#define SUPPORTED_BLOCK_IO
#define CONFIG_USE_VMALLOC
//#define CMD_THREAD_FUNC_DEBUG
#define RSSI_MIN_ADV_SEL

// USB TX
//#define CONFIG_IRQ_LEVEL_XMIT_LOCK
#define CONFIG_USB_TX_AGGREGATION
#define CONFIG_XMITBUF_TXAGG_ADV
#define CONFIG_TCP_ACK_TXAGG
#define CONFIG_TCP_ACK_MERGE
#define CONFIG_NETDEV_MULTI_TX_QUEUE
//#define CONFIG_TX_RECYCLE_EARLY

// USB RX
//#define CONFIG_USE_USB_BUFFER_ALLOC_RX
#define CONFIG_PREALLOC_RECV_SKB
#define CONFIG_USB_RX_AGGREGATION
#define DBG_CONFIG_ERROR_DETECT
//#define CONFIG_USB_INTERRUPT_IN_PIPE		// Has bug on 92C
#ifdef CONFIG_RTL_88E_SUPPORT
//#define CONFIG_SUPPORT_USB_INT
#define CONFIG_INTERRUPT_BASED_TXBCN // Tx Beacon when driver BCN_OK ,BCN_ERR interrupt occurs
#endif

#undef TX_SC_ENTRY_NUM
#define TX_SC_ENTRY_NUM		3

#undef RX_SC_ENTRY_NUM
#define RX_SC_ENTRY_NUM		3

#undef SW_TX_QUEUE
#define RTL8190_DIRECT_RX
#undef RTL8190_ISR_RX
#define RX_TASKLET
#undef RX_BUFFER_GATHER	// this is a tip. You must define/undef it in above to make it available
#undef CONFIG_OFFLOAD_FUNCTION
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
#undef RTL_MANUAL_EDCA
#undef CONFIG_OFFLOAD_FUNCTION
#ifdef __KERNEL__
#define CONFIG_USE_VMALLOC
#endif
#define DONT_COUNT_PROBE_PACKET
#ifdef CONFIG_WLAN_HAL_8192EE
#define CONFIG_1RCCA_RF_POWER_SAVING
#endif
#if defined(CONFIG_POWER_SAVE) && (defined(CONFIG_WAKELOCK) || defined(CONFIG_PM_WAKELOCKS))
#define USE_WAKELOCK_MECHANISM
#endif
//#define CMD_THREAD_FUNC_DEBUG
//#define INDICATE_LINK_CHANGE
#define RSSI_MIN_ADV_SEL

// SDIO TX
//#define CONFIG_IRQ_LEVEL_XMIT_LOCK
#define CONFIG_SDIO_TX_AGGREGATION
#define CONFIG_XMITBUF_TXAGG_ADV
#ifdef __KERNEL__ //TBD for ecos
#define CONFIG_TCP_ACK_TXAGG
#define CONFIG_TCP_ACK_MERGE
#endif
#define CONFIG_SDIO_TX_INTERRUPT
#define CONFIG_SDIO_TX_IN_INTERRUPT
#define CONFIG_SDIO_RESERVE_MASSIVE_PUBLIC_PAGE		// only 88E available
#ifdef __KERNEL__
#define CONFIG_NETDEV_MULTI_TX_QUEUE
#endif
#define CONFIG_TX_RECYCLE_EARLY
#ifdef __ECOS
#define CONFIG_SDIO_TX_FILTER_BY_PRI
#endif
//#define SDIO_STATISTICS
//#define SDIO_STATISTICS_TIME

// SDIO RX
#define RTL8190_DIRECT_RX
#undef RTL8190_ISR_RX
#define RX_TASKLET
#undef RX_BUFFER_GATHER	// this is a tip. You must define/undef it in above to make it available

#undef TX_SC_ENTRY_NUM
#define TX_SC_ENTRY_NUM		3

#undef RX_SC_ENTRY_NUM
#define RX_SC_ENTRY_NUM		3

// Validate SDIO flag combination
#if defined(CONFIG_SDIO_TX_IN_INTERRUPT) && !defined(CONFIG_SDIO_TX_INTERRUPT)
#error "CONFIG_SDIO_TX_IN_INTERRUPT must under TX INT mode"
#endif
#if defined(CONFIG_XMITBUF_TXAGG_ADV) && !defined(CONFIG_SDIO_TX_AGGREGATION)
#error "CONFIG_XMITBUF_TXAGG_ADV must under CONFIG_SDIO_TX_AGGREGATION mode"
#endif
#if defined(SDIO_AP_OFFLOAD) && defined(MBSSID) && (RTL8192CD_NUM_VWLAN > 2)
#error "AP offload only support at most three SSIDs"
#endif
#endif // CONFIG_SDIO_HCI

#if defined(WIFI_WPAS_CLI) && defined(SMART_REPEATER_MODE)
#undef SMART_REPEATER_MODE
#endif

// select_queue (of net_device) is available above Linux 2.6.27
#if defined(CONFIG_NETDEV_MULTI_TX_QUEUE) && !defined(LINUX_2_6_27_)
#undef CONFIG_NETDEV_MULTI_TX_QUEUE
#endif

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
#undef __DRAM_IN_865X
#undef __IRAM_IN_865X
#undef __MIPS16

#define __DRAM_IN_865X
#define __IRAM_IN_865X
#define __MIPS16
#undef BR_SHORTCUT
#undef CONFIG_RTL865X_ETH_PRIV_SKB
#undef CONFIG_RTL_ETH_PRIV_SKB
#undef CONFIG_RTK_VLAN_SUPPORT
#endif




//#ifdef BR_SHORTCUT
#define MAX_REPEATER_SC_NUM 2
#define MAX_BRSC_NUM 8
//#endif

#if defined(RTK_NL80211) || defined(WIFI_HAPD)
#ifdef SUPPORT_CLIENT_MIXED_SECURITY
#undef SUPPORT_CLIENT_MIXED_SECURITY //because 8192cd_psk_hapd.c not patched this fun yet
#endif

#ifdef CONFIG_IEEE80211W
#undef CONFIG_IEEE80211W
#endif

#endif


#ifdef RTK_NL80211

#ifdef CPTCFG_CFG80211_MODULE
//#define RTK_NL80211_DMA //eric-sync ?? backfire comapt
#endif

#undef NUM_STAT
#define NUM_STAT 64 //support 64 STA for OpenWrt sdk

#if 0 //ndef RTK_NL80211_DMA
//use SLUB to overcome the kmalloc issue , no need RTK_NL80211_DMA anymore!
#undef NUM_RX_DESC
#undef NUM_TX_DESC
#undef RTL8192CD_NUM_VWLAN

#define NUM_RX_DESC    128
#define NUM_TX_DESC    128
#define RTL8192CD_NUM_VWLAN  4
#else
#define RTL8192CD_NUM_VWLAN  (0) //eric-vap, add one more for open-wrt scan iface
#endif

//#define CUSTOMIZE_FLASH_EFUSE //Only 8812 read EFUSE except MAC address
//#define CUSTOMIZE_SCAN_HIDDEN_AP			//Scan Hidden AP

#endif //RTK_NL80211

#ifdef CONFIG_RTL_ASUSWRT
#define CUSTOMIZE_WLAN_IF_NAME
//#define THERMAL_PROTECTION

#endif

#define LEVEL_92C 2
#define LEVEL_92D 3
#define LEVEL_8812 4
#define LEVEL_88E 5
#define LEVEL_92E 6
#define LEVEL_8881A 7
#define LEVEL_8814 8
#define LEVEL_97F 9
#define LEVEL_8822 10

#ifdef CONFIG_WLAN_HAL_8822BE
#define IC_LEVEL	10
#elif defined(CONFIG_WLAN_HAL_8197F)
#define IC_LEVEL	9
#elif defined(CONFIG_WLAN_HAL_8814AE)
#define IC_LEVEL	8
#elif defined(CONFIG_WLAN_HAL_8881A)
#define IC_LEVEL	7
#elif defined(CONFIG_WLAN_HAL_8192EE)
#define IC_LEVEL	6
#elif defined(CONFIG_RTL_88E_SUPPORT)
#define IC_LEVEL	5
#elif defined(CONFIG_RTL_8812_SUPPORT)
#define IC_LEVEL	4
#elif defined(CONFIG_RTL_92D_SUPPORT)
#define IC_LEVEL	3
#elif defined(CONFIG_RTL_92C_SUPPORT)
#define IC_LEVEL	2
#else
#define IC_LEVEL	1
#endif


#ifdef CONFIG_WLAN_HAL_8822BE
#define SUPPORT_ALL_CUT
#define PHYDM_SUPPORT_8822_CUT_VERSION 2
#endif

#ifdef CONFIG_BAND_2G_ON_WLAN0
	#define WLANIDX	1
#else
	#define WLANIDX	0
#endif

//-------------------------------------------------------------
// for App tr069
//-------------------------------------------------------------
#define CONFIG_WLAN_STATS_EXTENTION 	1

#if 0
#if defined(CONFIG_WLAN_HAL_8197F)
#define POWER_TRIM
#endif

#if defined(CONFIG_WLAN_HAL_8197F)
#define THER_TRIM
#endif
#endif

// Thermal Control
//#define THERMAL_CONTROL

#ifdef THERMAL_CONTROL
#define RTK_STA_BWC
#if defined(CONFIG_WLAN_HAL_8192EE)
#define THER_HIGH_2G 13
#define THER_LOW_2G  12
#else
#define THER_HIGH_2G 11
#define THER_LOW_2G  10
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
#define THER_HIGH_5G 15
#define THER_LOW_5G  14
#else
#define THER_HIGH_5G 10
#define THER_LOW_5G  9
#endif
#define RSSI1 45
#define RSSI2 63
#define TOLERANCE 3
#define	THER_HIT_COUNT	30
#endif

//-------------------------------------------------------------
//  Customize SSID Priority Support
//-------------------------------------------------------------
#define CONFIG_RTK_SSID_PRIORITY 1
#ifdef CONFIG_RTK_SSID_PRIORITY
#define SSID_PRIORITY_SUPPORT
#endif

#if defined(CONFIG_RTL_QOS_PATCH) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) ||defined(CONFIG_RTL_HW_QOS_SUPPORT_WLAN) || defined(CONFIG_RTL_NIC_QUEUE)|| defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	#ifndef QOS_PATCH_RX_FROM_WIRELESS
	#define QOS_PATCH_RX_FROM_WIRELESS      7
	#endif
#endif

#endif // _8192CD_CFG_H_

