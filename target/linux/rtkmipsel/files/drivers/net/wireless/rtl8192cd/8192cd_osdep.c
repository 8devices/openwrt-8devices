/*
 *  Routines to handle OS dependent jobs and interfaces
 *
 *  $Id: 8192cd_osdep.c,v 1.61.2.28 2011/01/11 13:48:37 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_OSDEP_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/fcntl.h>
#include <linux/signal.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/reboot.h>
#endif

#ifdef __DRAYTEK_OS__
#include <draytek/softimer.h>
#include <draytek/skbuff.h>
#include <draytek/wl_dev.h>
#endif

#include "./8192cd_cfg.h"

#ifdef _BROADLIGHT_FASTPATH_
int (*send_packet_to_upper_layer)(struct sk_buff *skb) = netif_rx ;
#endif
#ifdef __KERNEL__
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/unistd.h>
#endif

#ifdef CONFIG_RTL_PROC_NEW
#include <linux/seq_file.h>
#endif

#if defined(RTK_BR_EXT) || defined(BR_SHORTCUT)
#ifdef __LINUX_2_6__
#include <linux/syscalls.h>
#else
#include <linux/fs.h>
#endif
#endif
#elif defined(__ECOS)
#include <cyg/hal/plf_intr.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#ifdef CONFIG_RTL_REPORT_LINK_STATUS
#include <cyg/io/eth/rltk/819x/wrapper/if_status.h>
#endif
#ifdef CONFIG_SDIO_HCI
#include <cyg/io/sdio_linux.h>
#endif
#else
#include "./sys-support.h"
#endif

#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_rx.h"
#include "./8192cd_debug.h"

#if defined(CONFIG_WLAN_HAL)
#include "./WlanHAL/HalMac88XX/halmac_reg2.h"
#endif
#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"
#endif
#ifdef CONFIG_RTL_VLAN_8021Q
#include <linux/if_vlan.h>
extern int linux_vlan_enable;
#endif

#ifdef CONFIG_PUMA_UDMA_SUPPORT
#include <linux/udma_api.h>
#endif

#ifdef CONFIG_RTL_92D_DMDP
u32 if_priv[NUM_WLAN_IFACE];
#ifdef NOT_RTK_BSP
static int rtl8192D_idx=0;
#endif
#endif

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif

#if defined(STA_CONTROL) && STA_CONTROL_ALGO == STA_CONTROL_ALGO3	//20170103
u32 if_priv_stactrl[NUM_WLAN_IFACE];
#endif

#ifndef CONFIG_RTK_MESH	//move below from mesh_route.h ; plus 0119
#define MESH_SHIFT			0
#define MESH_NUM_CFG		0
#else
#define MESH_SHIFT			8 // ACCESS_MASK uses 2 bits, WDS_MASK use 4 bits
#define MESH_MASK			0x3
#define MESH_NUM_CFG		NUM_MESH
#endif


#if defined(CONFIG_RTL_8198)
#define CONFIG_RTL8198_REVISION_B 1
//#define CONFIG_PHY_EAT_40MHZ 1
#endif
#ifdef CONFIG_RTL8672
	#ifdef USE_RLX_BSP
		#include <bspchip.h>
		#ifdef __LINUX_3_10__
		#include <bspgpio.h>
		#else
		#include <gpio.h>
		#endif

		#ifdef CONFIG_RTL_8196C
		#undef CONFIG_RTL_8196C
		#endif
		#ifdef CONFIG_RTL8196C_REVISION_B
		#undef CONFIG_RTL8196C_REVISION_B
		#endif
	#elif !defined(__OSK__)
		#include <platform.h>
		#include "../../../arch/mips/realtek/rtl8672/gpio.h"
	#endif
#else
	#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196B)
	#include <asm/rtl865x/platform.h>
	#endif

	#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196C)
	#ifdef __KERNEL__
	#include <asm/rtl865x/platform.h>
	#endif
	#endif
#endif

#if defined(CONFIG_RTL_819X) && defined(__LINUX_2_6__)
#if !defined(USE_RLX_BSP)
#ifndef __ECOS
#include <platform.h>
#endif
#else
#if (defined(CONFIG_OPENWRT_SDK) && !defined(CONFIG_ARCH_CPU_RLX)) || defined(CONFIG_RTL_8197F)
#include <bspchip.h>
#else
#include <bsp/bspchip.h>
#endif //CONFIG_OPENWRT_SDK
#endif
#endif

#if defined(__OSK__) && defined(CONFIG_RLE0412)
#include "asicregs.h"
#include "gpio.h"
#endif

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#include "wapiCrypto.h"
#endif
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU //mark_test
#ifdef __KERNEL__
#include <linux/proc_fs.h> 
#endif
#endif
#if defined(CONFIG_WIRELESS_LAN_MODULE) && !defined(NOT_RTK_BSP)
extern int (*wirelessnet_hook)(void);
#ifdef BR_SHORTCUT
extern struct net_device* (*wirelessnet_hook_shortcut)(unsigned char *da);
#endif
#ifdef PERF_DUMP
extern int rtl8651_romeperfEnterPoint(unsigned int index);
extern int rtl8651_romeperfExitPoint(unsigned int index);
extern int (*Fn_rtl8651_romeperfEnterPoint)(unsigned int index);
extern int (*Fn_rtl8651_romeperfExitPoint)(unsigned int index);
#endif
#ifdef CONFIG_RTL8190_PRIV_SKB
extern int (*wirelessnet_hook_is_priv_buf)(void);
extern void (*wirelessnet_hook_free_priv_buf)(unsigned char *head);
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE

#if defined(WIFI_HAPD) || defined (RTK_NL80211)
#include "8192cd_net80211.h" 
#endif

#ifdef RTK_NL80211
#include "8192cd_cfg80211.h" 
#endif

#ifdef BR_SHORTCUT
#ifdef CLIENT_MODE
extern unsigned char cached_sta_mac[MAX_REPEATER_SC_NUM][MACADDRLEN];
extern struct net_device *cached_sta_dev[MAX_REPEATER_SC_NUM];
#endif

#ifdef RTL_CACHED_BR_STA
extern struct brsc_cache_t brsc_cache_arr[MAX_BRSC_NUM];
#endif
#endif // BR_SHORTCUT

#if defined(CONFIG_LUNA_DUAL_LINUX) && (defined(CONFIG_USE_PCIE_SLOT_1) && !defined(CONFIG_ARCH_LUNA_SLAVE))
	#ifdef CONFIG_RTL8672
		#define SLOT1_IRQ	BSP_PCIE2_IRQ
	#else
		#define SLOT1_IRQ	BSP_PCIE1_IRQ
	#endif
#endif

// TODO: Filen, move to BSP Setting
#if defined(CONFIG_WLAN_HAL_8881A)
/*
 * Wlan LBus Address
 */

#define BSP_WLAN_BASE_ADDR      0xB8640000
#define BSP_WLAN_CONF_ADDR      NULL
#endif  //CONFIG_SOC_RTL8881A

#ifdef CONFIG_PCI_HCI
struct _device_info_ wlan_device[] =
{
#if (defined(CONFIG_RTL8686) || defined(CONFIG_RTL8685) || defined(CONFIG_RTL8685S)) 
    #if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
        #if (defined(CONFIG_RTL_5G_SLOT_0) && defined(CONFIG_WLAN0_5G_WLAN1_2G)) || \
	        (defined(CONFIG_RTL_5G_SLOT_1) && defined(CONFIG_WLAN0_2G_WLAN1_5G))
	        //5G in wlan0/slot0, 2G in wlan1/slot1 or 5G in wlan1/slot1, 2G in wlan0/slot0
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM,	BSP_PCIE1_IRQ, NULL },
        #else
	        //5G in wlan1/slot0, 2G in wlan0/slot1 or 5G in wlan0/slot1, 2G in wlan1/slot0
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM,	BSP_PCIE1_IRQ, NULL },
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
        #endif
    #elif defined(CONFIG_USE_PCIE_SLOT_0)
	    {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
    #else
	    {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE1_IRQ, NULL},
    #endif
#elif defined(CONFIG_WLAN_HAL)
    #if defined(CONFIG_WLAN_HAL_8881A) && defined(CONFIG_BAND_5G_ON_WLAN0)
    {
        (MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
        BSP_WLAN_CONF_ADDR, 
        BSP_WLAN_BASE_ADDR, 
        BSP_WLAN_MAC_IRQ, 
        NULL
    },
    #endif  //CONFIG_WLAN_HAL_8881A
    #if defined(CONFIG_WLAN_HAL_8197F) && defined(CONFIG_BAND_2G_ON_WLAN0)
    {
        (MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
        BSP_WLAN_CONF_ADDR, 
        BSP_WLAN_BASE_ADDR, 
        BSP_WLAN_MAC_IRQ, 
        NULL
    },
    #endif  //CONFIG_WLAN_HAL_8197F

    #if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1) && \
   	    (defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_92D))
   	     //5G in slot 0, 2G in slot 1
        #if defined(CONFIG_BAND_5G_ON_WLAN0)
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
        #else
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
	        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
        #endif	
	
    #elif defined(NOT_RTK_BSP)
	    {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	    {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
	    {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
    #else
    	//2G in slot 0, 5G in slot 1, or only one slot used
    	#if defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_BAND_5G_ON_WLAN0)
        {
    	 	(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM,
    		BSP_PCIE1_D_CFG0,
    		BSP_PCIE1_D_MEM,
    		#ifdef CONFIG_LUNA_DUAL_LINUX
    	      #if defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_WLAN_HAL_8192EE)
    				BSP_PCIE1_IRQ,//92er slave
    	      #elif !defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_WLAN_HAL_8192EE)
    				BSP_PCIE1_IRQ,//92er master
    				#elif defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_RTL_8812_SUPPORT)
    				BSP_PCIE2_IRQ,//8812 slave
    				#elif !defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_RTL_8812_SUPPORT)
    				BSP_PCIE2_IRQ,//8812 master
    	      #endif
    	  #else
    				BSP_PCIE2_IRQ,
    		#endif
        NULL
    	},
    	#endif  
    	#if defined(CONFIG_USE_PCIE_SLOT_0) 
    	{
    		(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
    		BSP_PCIE0_D_CFG0, 
    		BSP_PCIE0_D_MEM, 
    		BSP_PCIE_IRQ, 
    		NULL
    	},
    	#endif
    	#if defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_BAND_2G_ON_WLAN0)
        {
    	 	(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM,
    		BSP_PCIE1_D_CFG0,
    		BSP_PCIE1_D_MEM,
    		#ifdef CONFIG_LUNA_DUAL_LINUX
    	      #if defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_WLAN_HAL_8192EE)
    				BSP_PCIE1_IRQ,//92er slave
    	      #elif !defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_WLAN_HAL_8192EE)
    				BSP_PCIE1_IRQ,//92er master
    				#elif defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_RTL_8812_SUPPORT)
    				BSP_PCIE2_IRQ,//8812 slave
    				#elif !defined(CONFIG_ARCH_LUNA_SLAVE) && defined(CONFIG_RTL_8812_SUPPORT)
    				BSP_PCIE2_IRQ,//8812 master
    	      #endif
    	  #else
    				BSP_PCIE2_IRQ,
    		#endif
        NULL
    	},
    	#endif  	
	
    #endif

    #if defined(CONFIG_WLAN_HAL_8881A) && !defined(CONFIG_BAND_5G_ON_WLAN0)
    {
        (MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
        BSP_WLAN_CONF_ADDR, 
        BSP_WLAN_BASE_ADDR, 
        BSP_WLAN_MAC_IRQ, 
        NULL
    },
    #endif  //CONFIG_WLAN_HAL_8881A
    #if defined(CONFIG_WLAN_HAL_8197F) && defined(CONFIG_BAND_5G_ON_WLAN0)
    {
        (MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 
        BSP_WLAN_CONF_ADDR, 
        BSP_WLAN_BASE_ADDR, 
        BSP_WLAN_MAC_IRQ,
        NULL
    },
    #endif  //CONFIG_WLAN_HAL_8197F

#else

    #if defined(USE_RTL8186_SDK)
    	//{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT), 0, 0xbd400000, 2, NULL},
    	#ifdef CONFIG_NET_PCI
    		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
    	#else
    		#ifdef IO_MAPPING
    			{(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT1_CONFIG_ADDR, 0xb8C00000, 3, NULL},
    		#else
    			#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8196E)
    			    #if defined(__LINUX_2_6__) && defined(USE_RLX_BSP) 
        	            #if defined(CONFIG_RTL_92D_SUPPORT)
                            #if (RTL_USED_PCIE_SLOT==0)
                                {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
                                  #ifdef CONFIG_RTL_92D_DMDP
                        				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_F1_D_MEM, BSP_PCIE_IRQ, NULL},
                                  #endif
                                  #if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
                                  		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
                                  #endif
                            #elif (RTL_USED_PCIE_SLOT==1)
        				        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
                				#ifdef CONFIG_RTL_92D_DMDP
                				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_F1_D_MEM, BSP_PCIE2_IRQ, NULL},
                				#endif
                				#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
                				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
                		 		#endif
                    		#else
                    				#error "define pcie error"
                            #endif
        	            #else
							#if defined(CONFIG_RTL_88E_SUPPORT)||defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
                                #if defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1)
                                    #if defined(CONFIG_SLOT_0_8812) || defined(CONFIG_SLOT_0_92D)
                                		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
                                		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},
                                    #else
                                		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},		
                                		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
                                    #endif
                                #elif (RTL_USED_PCIE_SLOT==1)
                            		{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},		
                                #else
                        		    {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
                                #endif
                            #else
                        		#if (!defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1))
                        				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL},		
                        		#else		
                        				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE0_D_CFG0, BSP_PCIE0_D_MEM, BSP_PCIE_IRQ, NULL},
                        		#endif		
                        		 #if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
                        				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, BSP_PCIE1_D_CFG0, BSP_PCIE1_D_MEM, BSP_PCIE2_IRQ, NULL}
                        		 #endif
                            #endif
    	                #endif
                    #else
				        {(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCIE0_D_CFG0, PCIE0_D_MEM, PCIE_IRQ, NULL},
                	    #ifdef CONFIG_RTL_92D_DMDP
                				{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCIE0_D_CFG0, PCIE0_F1_D_MEM, PCIE_IRQ, NULL},
                		#endif
                    #endif
				//{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
			    #else
				    {(MESH_NUM_CFG<<MESH_SHIFT) | (WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT1_CONFIG_ADDR, 0xb9000000, 3, NULL},
			    #endif
		    #endif
	    #endif
    #elif defined(NOT_RTK_BSP)
    	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
    	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL},
    	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, 0, 0, NULL}
    #elif defined(CONFIG_RTL8671)
    	#ifdef IO_MAPPING
    	{(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT0_CONFIG_ADDR, 0xbd200000, 7, NULL},
    	#else
    	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM, PCI_SLOT0_CONFIG_ADDR, 0xbd300000, 7, NULL},
    	#endif
    #else
	    #error No_System_Defined
    #endif
#endif  //CONFIG_WLAN_HAL
};
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
struct _device_info_ wlan_device[] =
{
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, ((0x8191<<16)|USB_VENDER_ID_REALTEK), 0, NULL},
};
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
struct _device_info_ wlan_device[] =
{
	{(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_BIOS<<TYPE_SHIFT) | ACCESS_SWAP_MEM, 0, ((0x8179<<16)|0x024c), 0, NULL},
};
#endif // CONFIG_SDIO_HCI

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
static int rtw_shutdown(struct notifier_block *nb, unsigned long cmd, void *p)
{
	struct net_device *dev;
	
	if (wlan_device[0].priv) {
		dev = dev_get_by_name(&init_net, wlan_device[0].priv->dev->name);
		if (dev) {
			printk("[%s] close %s ...\n", __func__, wlan_device[0].priv->dev->name);
			rtnl_lock();
			dev_close(dev);
			rtnl_unlock();
			
			dev_put(dev);
		}
	}
	return 0;
}

static struct notifier_block rtw_reboot_notifier = {
	.notifier_call = rtw_shutdown,
};
#endif

#ifndef __ECOS
static int wlan_index=0;
int drv_registered = FALSE;
#endif

#ifdef __OSK__
struct rtl8192cd_priv *wifi_root_priv = NULL;
#endif

#if defined(CONFIG_RTK_MESH) && defined(CONFIG_RTL_MESH_SINGLE_IFACE)
static struct rtl8192cd_priv *tmp_priv = NULL;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE

#ifdef STA_CONTROL
static struct rtl8192cd_priv *stactrl_tmp_priv = NULL;
#endif

#ifdef CROSSBAND_REPEATER
static struct rtl8192cd_priv *crossband_tmp_priv = NULL;
#endif

#if defined(CONFIG_RTL8672) && defined(__OSK__)
// Added by Mason Yu
// MBSSID Port Mapping
#ifdef CONFIG_RTL_92D_DMDP
struct port_map wlanDev[(RTL8192CD_NUM_VWLAN+2)*2];
#else
struct port_map wlanDev[RTL8192CD_NUM_VWLAN+2];		// Root(1)+vxd(1)+VAPs(4)
#endif
int wlanDevNum=0;
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
static struct rtl8192cd_hw hw_info[NUM_WLAN_IFACE];
static struct priv_shared_info shared_info[NUM_WLAN_IFACE];
static struct wlan_hdr_poll hdr_pool[NUM_WLAN_IFACE];
static struct wlanllc_hdr_poll llc_pool[NUM_WLAN_IFACE];
static struct wlanbuf_poll buf_pool[NUM_WLAN_IFACE];
static struct wlanicv_poll icv_pool[NUM_WLAN_IFACE];
static struct wlanmic_poll mic_pool[NUM_WLAN_IFACE];
static unsigned char desc_buf[NUM_WLAN_IFACE][DESC_DMA_PAGE_SIZE];
#ifdef WLAN_SUPPORT_H2C_PACKET
static unsigned char h2c_buf[NUM_WLAN_IFACE][H2C_BUFFER_SIZE];
#endif //WLAN_SUPPORT_H2C_PACKET
#ifdef WLAN_HAL_TX_AMSDU
static unsigned char desc_buf_amsdu[NUM_WLAN_IFACE][DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU];
#endif //WLAN_HAL_TX_AMSDU
static int wlandev_idx = 0;

#else
static struct rtl8192cd_hw hw_info;
static struct priv_shared_info shared_info;
static struct wlan_hdr_poll hdr_pool;
static struct wlanllc_hdr_poll llc_pool;
static struct wlanbuf_poll buf_pool;
static struct wlanicv_poll icv_pool;
static struct wlanmic_poll mic_pool;
static unsigned char desc_buf[DESC_DMA_PAGE_SIZE];
#ifdef WLAN_SUPPORT_H2C_PACKET
static unsigned char h2c_buf[H2C_BUFFER_SIZE];
#endif //WLAN_SUPPORT_H2C_PACKET

#ifdef WLAN_HAL_TX_AMSDU
static unsigned char desc_buf_amsdu[DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU];
#endif
#endif
#endif

// init and remove char device
#ifdef CONFIG_WIRELESS_LAN_MODULE
extern int rtl8192cd_chr_init(void);
extern void rtl8192cd_chr_exit(void);
#else
int rtl8192cd_chr_init(void);
void rtl8192cd_chr_exit(void);
#endif
struct rtl8192cd_priv *rtl8192cd_chr_reg(unsigned int minor, struct rtl8192cd_chr_priv *priv);
void rtl8192cd_chr_unreg(unsigned int minor);
int rtl8192cd_fileopen(const char *filename, int flags, int mode);

#ifdef USE_DMA_ALLOCATE
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
static struct device * cma_dev;
static const struct file_operations cma_dev_fops = {
	.owner =    THIS_MODULE,
	.read  =    NULL,
	.write =    NULL,
};
static struct miscdevice cma_dev_misc = {
	.name = "cma_dev",
	.fops = &cma_dev_fops,
};
#endif

#ifdef RTK_WLAN_EVENT_INDICATE
extern struct sock *get_nl_eventd_sk(void);
extern struct sock* rtk_eventd_netlink_init();
#endif

void force_stop_wlan_hw(void);


#if defined(_INCLUDE_PROC_FS_) && defined(PERF_DUMP)
#include "romeperf.h"
static int read_perf_dump(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
#ifndef SMP_SYNC
	unsigned long x;
#endif

	SAVE_INT_AND_CLI(x);

	rtl8651_romeperfDump(ROMEPERF_INDEX_MIN, ROMEPERF_INDEX_MAX);

	RESTORE_INT(x);
    return count;
}


static int flush_perf_dump(struct file *file, const char *buffer,
              unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
#ifndef SMP_SYNC
	unsigned long x;
#endif

	SAVE_INT_AND_CLI(x);

	rtl8651_romeperfReset();

	RESTORE_INT(x);
    return count;
}
#endif // _INCLUDE_PROC_FS_ && PERF_DUMP

#ifdef MULTI_MAC_CLONE
void init_Multi_Mac_Clone(struct rtl8192cd_priv *priv)
{
    int i=0;
    if (priv==NULL || priv->pshare==NULL){
        DEBUG_ERR("init mclone_sta_fixed_addr failed, because the interface is not ready\n");
        return;
    }

    #if defined(CONFIG_WLAN_HAL_8197F)
    if(GET_CHIP_VER(priv)==VERSION_8197F) {
        priv->pshare->mclone_num_max = MAX_MBIDCAM_NUM_8197F;
    }
    else
    #endif
    {
        priv->pshare->mclone_num_max = MAX_MBIDCAM_NUM_DEFAULT;
    }

    priv->pshare->mclone_num_max -= MAX_MBIDCAM_RESERVED;

    for (i=0; i<priv->pshare->mclone_num_max; i++){
        priv->pshare->mclone_sta[i].opmode = WIFI_STATION_STATE;
        priv->pshare->mclone_sta[i].usedStaAddrId = 0xff;
        memcpy(priv->pshare->mclone_sta[i].sa_addr, NULL_MAC_ADDR, ETH_ALEN);
        memcpy(priv->pshare->mclone_sta[i].hwaddr, NULL_MAC_ADDR, ETH_ALEN);
        priv->pshare->mclone_sta[i].isTimerInit = 0;
        priv->pshare->mclone_sta[i].aid = -1;
        priv->pshare->mclone_sta[i].priv = NULL;
    }

    STADEBUG("init mclone_sta_fixed_addr\n");
    /*first copy from interface's HWADDR then increase from 3th Byte*/
    if (!memcmp(priv->pshare->mclone_sta_fixed_addr[0].clone_addr, NULL_MAC_ADDR, MACADDRLEN)) {
        memcpy(priv->pshare->mclone_sta_fixed_addr[0].clone_addr, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);
        priv->pshare->mclone_sta_fixed_addr[0].clone_addr[2]=0;
        STADEBUG("use defualt MAC\n");
    } else {
        priv->pshare->mclone_sta_fixed_addr[0].clone_addr[5] &= 0xf0;//max is 16 clone sta.
        priv->pshare->mclone_sta_fixed_addr[0].clone_addr[0] &= 0xfe;//should not mcast addr.
    }

    priv->pshare->mclone_sta_fixed_addr[0].used = 0;
    for (i=1; i<priv->pshare->mclone_num_max; i++){
        memcpy(priv->pshare->mclone_sta_fixed_addr[i].clone_addr, priv->pshare->mclone_sta_fixed_addr[0].clone_addr, MACADDRLEN);
        priv->pshare->mclone_sta_fixed_addr[i].clone_addr[2] += i;
        priv->pshare->mclone_sta_fixed_addr[i].used = 0;
    }
   
}

void clear_Multi_Mac_Clone(struct rtl8192cd_priv *priv, int idx)
{
	int i, id;
	if (idx==-1){//clear all and re-init all
	    DEBUG_INFO("clear all mclone_sta_fixed_addr\n");
	    id = ACTIVE_ID;
	    for (i=0; i<priv->pshare->mclone_num_max; i++) {
	    	ACTIVE_ID = i + 1;
			if (PENDING_REAUTH_TIMER) DELETE_REAUTH_TIMER;
			if (PENDING_REASSOC_TIMER) DELETE_REASSOC_TIMER;
	    }
	    ACTIVE_ID = id;
		init_Multi_Mac_Clone(priv);
		MCLONE_NUM = 0;
		return;
	} 
	
	if (priv->pshare->mclone_sta[idx].usedStaAddrId != 0xff){
		priv->pshare->mclone_sta_fixed_addr[priv->pshare->mclone_sta[idx].usedStaAddrId].used = 0;
		priv->pshare->mclone_sta[idx].usedStaAddrId = 0xff;
	}

	priv->pshare->mclone_sta[idx].opmode = WIFI_STATION_STATE;
	memcpy(priv->pshare->mclone_sta[idx].sa_addr, NULL_MAC_ADDR, ETH_ALEN);
	memcpy(priv->pshare->mclone_sta[idx].hwaddr, NULL_MAC_ADDR, ETH_ALEN);
	if (PENDING_REAUTH_TIMER) DELETE_REAUTH_TIMER;
	if (PENDING_REASSOC_TIMER) DELETE_REASSOC_TIMER;
	priv->pshare->mclone_sta[idx].aid = -1;
	priv->pshare->mclone_sta[idx].priv = NULL;
	MCLONE_NUM--;

	return;
}
#endif

#ifdef CONFIG_PCI_HCI
static void rtl8192cd_bcnProc(struct rtl8192cd_priv *priv, unsigned int bcnInt,
				unsigned int bcnOk, unsigned int bcnErr, unsigned int status
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				, unsigned int status_ext
#endif
				)
{
#ifdef MBSSID
	int i;
#endif
#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
#endif

	/* ================================================================
			Process Beacon OK/ERROR interrupt
		================================================================ */
	if ( bcnOk || bcnErr)
	{

#ifdef UNIVERSAL_REPEATER
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
						(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		//
		// Statistics and LED counting
		//
		if (bcnOk) {
			// for SW LED
			if (priv->pshare->LED_cnt_mgn_pkt)
				priv->pshare->LED_tx_cnt++;
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_ok++;
#else
			priv->ext_stats.beacon_ok++;
#endif
			SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);

			// disable high queue limitation
			if ((OPMODE & WIFI_AP_STATE) && (priv->pshare->bcnDOk_priv)) {
				if (*((unsigned char *)priv->pshare->bcnDOk_priv->beaconbuf + priv->pshare->bcnDOk_priv->timoffset + 4) & 0x01)  {
					RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | HIQ_NO_LMT_EN);
				}
			}

		} else if (bcnErr) {
#ifdef MBSSID
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_er++;
#else
			priv->ext_stats.beacon_er++;
#endif
		}

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif
	}


#ifdef PCIE_POWER_SAVING
		if ((OPMODE & WIFI_AP_STATE) && (status & HIMR_BCNDOK0)) {
			if ((priv->offload_ctrl & 1) && (priv->offload_ctrl >> 7) && priv->pshare->rf_ft_var.power_save) {
				priv->offload_ctrl &= (~1);
					update_beacon(priv);
				delay_us(100);
				RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);
				return;
			}
		}
#endif	
	

	/* ================================================================
			Process Beacon interrupt
	    ================================================================ */
	//
	// Update beacon content
	//
	if (bcnInt) {
		unsigned char val8;
		if (
#ifdef CONFIG_RTL_88E_SUPPORT
			(GET_CHIP_VER(priv)==VERSION_8188E)?(status & HIMR_88E_BcnInt):
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B)?(status & HIMR_92E_BcnInt):
#endif
			(status & HIMR_BCNDMA0)) {
#ifdef UNIVERSAL_REPEATER
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			if (GET_VXD_PRIV(priv)->timoffset) {
				update_beacon(GET_VXD_PRIV(priv));
			}
			} else
#endif
			{
				if (priv->timoffset) {
					update_beacon(priv);
				}
			}
		}
#ifdef MBSSID
		else {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (
#ifdef CONFIG_RTL_88E_SUPPORT
						(GET_CHIP_VER(priv)==VERSION_8188E)?(status_ext & (HIMRE_88E_BCNDMAINT1 <<
						(priv->pvap_priv[i]->vap_init_seq-1))):
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
						(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B)?(status_ext & (HIMRE_92E_BCNDMAINT1 <<
						(priv->pvap_priv[i]->vap_init_seq-1))):
#endif
						(status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1))))) {
						if (priv->pvap_priv[i]->timoffset) {
							update_beacon(priv->pvap_priv[i]);
						}
					}
				}
			}
		}
#endif

		//
		// Polling highQ as there is multicast waiting for tx...
		//
#ifdef UNIVERSAL_REPEATER
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		if ((OPMODE & WIFI_AP_STATE)) {
			if (
#ifdef CONFIG_RTL_88E_SUPPORT
				(GET_CHIP_VER(priv)==VERSION_8188E)?(status & HIMR_88E_BcnInt):
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B)?(status & HIMR_92E_BcnInt):
#endif

				(status & HIMR_BCNDMA0)) {
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
					if(RTL_R8(BCN_CTRL) & DIS_ATIM)
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
#ifdef __ECOS
					priv->pshare->has_triggered_process_mcast_dzqueue = 1;
					priv->pshare->call_dsr = 1;
#else
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
#endif
				} else {
					if(!(RTL_R8(BCN_CTRL) & DIS_ATIM))
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));				
				}	
//#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
//#endif
			}
#ifdef MBSSID
			else if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (
#ifdef CONFIG_RTL_88E_SUPPORT
						(GET_CHIP_VER(priv)==VERSION_8188E)?(status_ext & (HIMRE_88E_BCNDMAINT1 <<
						(priv->pvap_priv[i]->vap_init_seq-1))):
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
						(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B)?(status_ext & (IMR_BCNDMAINT1_8812 <<
						(priv->pvap_priv[i]->vap_init_seq-1))):
#endif
						(status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1))))) {
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
						if (val8 & 0x01) {
							if(RTL_R8(BCN_CTRL) & DIS_ATIM)
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
#ifdef __ECOS
							priv->pshare->has_triggered_vap_process_mcast_dzqueue[i] = 1;
							priv->pshare->call_dsr = 1;
#else
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
#endif
						} else {
							if(!(RTL_R8(BCN_CTRL) & DIS_ATIM))
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));
						}

						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}
#endif

		}

//		if (priv->pshare->pkt_in_hiQ) {
		if (priv->pshare->bcnDOk_priv && priv->pshare->bcnDOk_priv->pkt_in_hiQ) {
			int pre_head = get_txhead(priv->pshare->phw, MCAST_QNUM);
			do {
				txdesc_rollback(&pre_head);
			} while (((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->type != _PRE_ALLOCLLCHDR_) &&
					(get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN));

			if (get_desc((get_txdesc(priv->pshare->phw, MCAST_QNUM) + pre_head)->Dword0) & TX_OWN) {
				unsigned short *phdr = (unsigned short *)((get_txdesc_info(priv->pshare->pdesc_info, MCAST_QNUM) + pre_head)->pframe);
#ifdef __MIPSEB__
				phdr = (unsigned short *)KSEG1ADDR(phdr);
#endif
				ClearMData(phdr);
			}
			tx_poll(priv, MCAST_QNUM);
//				priv->pshare->pkt_in_hiQ = 0;
		}


#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif

	}

	
#ifdef CLIENT_MODE
	//
	// Ad-hoc beacon status
	//
	if (OPMODE & WIFI_ADHOC_STATE) {
		if (bcnOk)
			priv->ibss_tx_beacon = TRUE;
		if (bcnErr)
			priv->ibss_tx_beacon = FALSE;
	}
#endif
}
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_WLAN_HAL

#if CFG_HAL_MEASURE_BEACON
static VOID
CalcBeaconVariation(
    struct rtl8192cd_priv *priv
)
{
    u4Byte		tsfVal,tsf,beaconVarationTime,i;
    static u4Byte      maxVal[8]= {0};
	tsf = RTL_R32(REG_TSFTR);
    tsf = tsf - RTL_R8(0x556)*1024*(priv->vap_init_seq);

    if(priv->vap_init_seq ==2)
    {
        beaconVarationTime = (tsf%102400);
        if(beaconVarationTime > maxVal[priv->vap_init_seq])
        {
             maxVal[priv->vap_init_seq] = beaconVarationTime;
            if(priv->vap_init_seq == 0 ) {
//                printk("Root maxVal = %d \n", maxVal[0]);
            } else {
//                printk("VAP[%d] maxVal = %d \n",priv->vap_init_seq,maxVal[priv->vap_init_seq]);
            }          
        }

        //RTL_W32(0x1b8,beaconVarationTime);
//        printk("VAP[%d] beaconVarationTime = %d TSF =%d\n",priv->vap_init_seq,beaconVarationTime,tsf);
    }
}

#endif  // #if CFG_HAL_MEASURE_BEACON
#endif //#ifdef CONFIG_WLAN_HAL

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_WLAN_HAL
static void 
rtl88XX_bcnProc(
        struct rtl8192cd_priv *priv, 
        unsigned int bcnInt,
        unsigned int bcnOk, 
        unsigned int bcnErr
)
{
#ifdef MBSSID
	int i;
#endif

	/* ================================================================
			Process Beacon OK/ERROR interrupt
		================================================================ */
	if ( bcnOk || bcnErr)
	{
        // clear OWN bit after beacon ok interrupt, include root & VAPs
		//if (priv->pshare->bcnDOk_priv) {
		//	GET_HAL_INTERFACE(priv)->SetBeaconDownloadHandler(priv->pshare->bcnDOk_priv, HW_VAR_BEACON_DISABLE_DOWNLOAD);
		//}
#if (BEAMFORMING_SUPPORT == 1)
		priv->pshare->soundingLock=0;
#endif

#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
						(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		//
		// Statistics and LED counting
		//
		if (bcnOk) {
			// for SW LED
			if (priv->pshare->LED_cnt_mgn_pkt)
				priv->pshare->LED_tx_cnt++;

#ifdef SUPPORT_EACH_VAP_INT
			// The TXOK interrupt seperate for each VAP in RTL8814A 
			if (IS_SUPPORT_EACH_VAP_INT(priv)) {
#ifdef MBSSID
				if ((OPMODE & WIFI_AP_STATE)) {
					if ( TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TBDOK) ) {
						priv->ext_stats.beacon_ok++;
					}else {
						for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
							if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
								&& (_TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXBCN1OK + (priv->pvap_priv[i]->vap_init_seq-1))))
							{
								priv->pvap_priv[i]->ext_stats.beacon_ok++;
							}
						}
					}
				}
#else
				priv->ext_stats.beacon_ok++;
#endif // MBSSID
			} else
#endif // SUPPORT_EACH_VAP_INT
			{
#ifdef MBSSID
				if (priv->pshare->bcnDOk_priv)
				{
					priv->pshare->bcnDOk_priv->ext_stats.beacon_ok++;
#if CFG_HAL_MEASURE_BEACON
					CalcBeaconVariation(priv->pshare->bcnDOk_priv);
#endif
				}
#else // !MBSSID
				priv->ext_stats.beacon_ok++;
#if CFG_HAL_MEASURE_BEACON
				CalcBeaconVariation(priv);
#endif
#endif // MBSSID
				SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);

				// disable high queue limitation
				if ((OPMODE & WIFI_AP_STATE) && (priv->pshare->bcnDOk_priv)) {
					if (*((unsigned char *)priv->pshare->bcnDOk_priv->beaconbuf + priv->pshare->bcnDOk_priv->timoffset + 4) & 0x01)  {
						RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | HIQ_NO_LMT_EN);
					}
				}
			}
		} else if (bcnErr) {
#ifdef SUPPORT_EACH_VAP_INT
                // The TXOK interrupt seperate for each VAP in RTL8814A 
                if (IS_SUPPORT_EACH_VAP_INT(priv)) {
#ifdef MBSSID
				if ((OPMODE & WIFI_AP_STATE)) {
					if ( TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TBDER) ) {
						priv->ext_stats.beacon_er++;
					}else {
						for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
							if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
								&& (_TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXBCN1ERR + (priv->pvap_priv[i]->vap_init_seq-1))))
							{
								priv->pvap_priv[i]->ext_stats.beacon_er++;
							}
						}
					}
				}
#else
				priv->ext_stats.beacon_er++;
#endif // MBSSID
			} else
#endif // SUPPORT_EACH_VAP_INT
			{
#ifdef MBSSID
				if (priv->pshare->bcnDOk_priv)
					priv->pshare->bcnDOk_priv->ext_stats.beacon_er++;
#else
				priv->ext_stats.beacon_er++;
#endif // MBSSID
			}
		}

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif
	}


#ifdef PCIE_POWER_SAVING
// TODO: we should modify code below
		if ((OPMODE & WIFI_AP_STATE) && (status & HIMR_BCNDOK0)) {
			if ((priv->offload_ctrl & 1) && (priv->offload_ctrl >> 7) && priv->pshare->rf_ft_var.power_save) {
				priv->offload_ctrl &= (~1);
					update_beacon(priv);
				delay_us(100);
				RTL_W16(CR , RTL_R16(CR) & ~ENSWBCN);
				return;
			}
		}
#endif	
	

	/* ================================================================
			Process Beacon interrupt
	    ================================================================ */
	//
	// Update beacon content
	//
	if (bcnInt) {

		unsigned char val8;
#if (BEAMFORMING_SUPPORT == 1)		
		priv->pshare->soundingLock=1;
#endif

		if ( _TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt) ) {
            GET_HAL_INTERFACE(priv)->SetBeaconDownloadHandler(priv, HW_VAR_BEACON_DISABLE_DOWNLOAD);

#ifdef UNIVERSAL_REPEATER
			if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
				(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
				if (GET_VXD_PRIV(priv)->timoffset) {
					update_beacon(GET_VXD_PRIV(priv));
				}
			} else
#endif
			{
				if (priv->timoffset) {
					update_beacon(priv);
				}
			}
#ifdef MBSSID
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])) {
							struct rtl8192cd_priv	*p = priv->pvap_priv[i];
							if (p->pmib->miscEntry.func_off) {
								if (!p->func_off_already && ((!GET_ROOT(priv)->pmib->miscEntry.func_off) || (p->vap_init_seq%2))) {
									RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) & ~(1 << p->vap_init_seq));
									p->func_off_already = 1;
								}
							}
						}
					}
				}
#endif
		}
#ifdef MBSSID
		else {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (_TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt1 + (priv->pvap_priv[i]->vap_init_seq-1)))) 
						{
	                        GET_HAL_INTERFACE(priv)->SetBeaconDownloadHandler(priv->pvap_priv[i], HW_VAR_BEACON_DISABLE_DOWNLOAD);
							if (priv->pvap_priv[i]->timoffset) {
								update_beacon(priv->pvap_priv[i]);
							{
								int k;
								if (priv->pvap_priv[i]->vap_init_seq%2) {
									for (k=0; k<RTL8192CD_NUM_VWLAN; k++) {
										if ((priv->pvap_priv[k]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[k])) {
											struct rtl8192cd_priv	*p2 = priv->pvap_priv[k];
											if (p2->pmib->miscEntry.func_off) {
												if (!p2->func_off_already && (p2->vap_init_seq%2)==0) {
													RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) & ~(1 << p2->vap_init_seq));
													p2->func_off_already = 1;
												}
											}					
										}
									}
									if (priv->pmib->miscEntry.func_off) {	
										 if (!priv->func_off_already) {
											if(GET_CHIP_VER(priv) < VERSION_8814A)
												RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) & ~(BIT(0)));
											 priv->func_off_already = 1;
										 }			 
									 }									
								}else {
									for (k=0; k<RTL8192CD_NUM_VWLAN; k++) {
										if ((priv->pvap_priv[k]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[k])) {
											struct rtl8192cd_priv	*p2 = priv->pvap_priv[k];
											if (p2->pmib->miscEntry.func_off) {
												if (!p2->func_off_already && (p2->vap_init_seq%2)) {
													RTL_W8(REG_MBSSID_CTRL, RTL_R8(REG_MBSSID_CTRL) & ~(1 << p2->vap_init_seq));
													p2->func_off_already = 1;
												}
											}					
										}
									}
								}
							}								
						}
					}
				}
			}
		}
#endif

		//
		// Polling highQ as there is multicast waiting for tx...
		//
#ifdef UNIVERSAL_REPEATER
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}
#endif

		if ((OPMODE & WIFI_AP_STATE)) {
			if ( _TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt) ) {
                // TODO: modify code below
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
#ifdef __ECOS //mark_ecos
					priv->pshare->has_triggered_process_mcast_dzqueue = 1;
					priv->pshare->call_dsr = 1;
#else
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
#endif					
				}	
//#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
//#endif
			}
#ifdef MBSSID
			else if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (_TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt1 + (priv->pvap_priv[i]->vap_init_seq-1)))){
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
                       // TODO: modify code below
						if (val8 & 0x01) {
#ifdef __ECOS
							priv->pshare->has_triggered_vap_process_mcast_dzqueue[i] = 1;
							priv->pshare->call_dsr = 1;
#else
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
#endif
						}
						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}
#endif

		}

#ifdef UNIVERSAL_REPEATER
		if (priv_root != NULL)
			priv = priv_root;
#endif

	}

	
#ifdef CLIENT_MODE
	//
	// Ad-hoc beacon status
	//
	if (OPMODE & WIFI_ADHOC_STATE) {
		if (bcnOk)
			priv->ibss_tx_beacon = TRUE;
		if (bcnErr)
			priv->ibss_tx_beacon = FALSE;
	}
#endif
}

#if defined(__OSK__)
__IRAM_IN_865X_HI
#else
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
#endif
static __inline__ 
VOID
InterruptRxHandle(
    struct rtl8192cd_priv *priv, BOOLEAN caseRxRDU
)
{
#if defined(__KERNEL__) || defined(__ECOS)
#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX)
#if defined(RX_TASKLET)
#ifdef __ECOS
	GET_HAL_INTERFACE(priv)->DisableRxRelatedInterruptHandler(priv);
	if (!priv->pshare->has_triggered_rx_tasklet) {
		priv->pshare->has_triggered_rx_tasklet = 1;
		priv->pshare->call_dsr = 1;
	}
#else
	if (!priv->pshare->has_triggered_rx_tasklet) {
		priv->pshare->has_triggered_rx_tasklet = 1;
		GET_HAL_INTERFACE(priv)->DisableRxRelatedInterruptHandler(priv);
        tasklet_hi_schedule(&priv->pshare->rx_tasklet);
	}
#endif
#else
    rtl8192cd_rx_isr(priv);
#endif

#else	// !(defined RTL8190_ISR_RX && RTL8190_DIRECT_RX)
    if (caseRxRDU) {
        rtl8192cd_rx_isr(priv);
        if (priv->pshare->rxInt_useTsklt)
        tasklet_hi_schedule(&priv->pshare->rx_tasklet);
        else
            rtl8192cd_rx_dsr((unsigned long)priv);
    }
    else {
        if (priv->pshare->rxInt_useTsklt)
            tasklet_hi_schedule(&priv->pshare->rx_tasklet);
        else
            rtl8192cd_rx_dsr((unsigned long)priv);
    }
#endif
#else	// !__KERNEL__
    rtl8192cd_rx_dsr((unsigned long)priv);
#endif
}

#if defined(__OSK__)
__IRAM_IN_865X_HI
#else
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
#endif
static __inline__ 
VOID
InterruptTxHandle(
   struct rtl8192cd_priv *priv
)
{
    struct rtl8192cd_hw *phw;
    
    phw = GET_HW(priv);
    
#ifdef MP_TEST
#if defined(SMP_SYNC) || defined(__ECOS)
    if (OPMODE & WIFI_MP_STATE){
        if (!priv->pshare->has_triggered_tx_tasklet) {
#ifdef __KERNEL__
            tasklet_schedule(&priv->pshare->tx_tasklet);
            priv->pshare->has_triggered_tx_tasklet = 1;
#elif defined(__ECOS)
            priv->pshare->has_triggered_tx_tasklet = 1;
            priv->pshare->call_dsr = 1;
#endif
        }
    }
#else
    if (OPMODE & WIFI_MP_STATE)
        rtl8192cd_tx_dsr((unsigned long)priv);
#endif
    else
#endif
    
    if (GET_HAL_INTERFACE(priv)->QueryTxConditionMatchHandler(priv)) {
#if defined(__KERNEL__) || defined(__ECOS)
        if (!priv->pshare->has_triggered_tx_tasklet) {
#ifdef __ECOS
            priv->pshare->call_dsr = 1;
#else
            tasklet_schedule(&priv->pshare->tx_tasklet);
#endif
            priv->pshare->has_triggered_tx_tasklet = 1;
        }
#else
#ifdef SMP_SYNC
        if (!priv->pshare->has_triggered_tx_tasklet) {
            tasklet_schedule(&priv->pshare->tx_tasklet);
            priv->pshare->has_triggered_tx_tasklet = 1;
        }
#else
        rtl8192cd_tx_dsr((unsigned long)priv);
#endif
#endif
    }     
}


static __inline__ VOID
InterruptPSTimer2Handle(
   struct rtl8192cd_priv *priv
)
{
#if defined (SUPPORT_TX_AMSDU)
        unsigned long current_value, timeout;
#endif

#ifdef CONFIG_WLAN_HAL
#ifdef SUPPORT_TX_AMSDU
    GET_HAL_INTERFACE(priv)->RemoveInterruptMaskHandler(priv, HAL_INT_TYPE_FS_TIMEOUT0);

    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_TSF_TIMER, (pu1Byte)&current_value);
    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PS_TIMER, (pu1Byte)&timeout);

    if (TSF_LESS(current_value, timeout))
        setup_timer2(priv, timeout);
    else
        amsdu_timeout(priv, current_value);

#endif
#endif
    
#if 0   // TODO: Modify Code  below
#ifdef SUPPORT_TX_AMSDU
    RTL_W32(IMR, RTL_R32(IMR) & ~IMR_TIMEOUT2);

    current_value = RTL_R32(TSFR) ;
    timeout = RTL_R32(TIMER1);
    if (TSF_LESS(current_value, timeout))
        setup_timer2(priv, timeout);
    else
        amsdu_timeout(priv, current_value);
#endif

#ifdef P2P_SUPPORT
	/*cfg p2p cfg p2p*/
    if( rtk_p2p_is_enabled(priv) &&  (rtk_p2p_chk_role(priv,P2P_CLIENT))) {
        RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);
        p2p_noa_timer(priv);
    }
#endif
#endif
}


#if 0 //Filen_Test
#define PRINT_DATA(_TitleString, _HexData, _HexDataLen)						\
{												\
	char			*szTitle = _TitleString;					\
	pu1Byte		pbtHexData = _HexData;							\
	u4Byte		u4bHexDataLen = _HexDataLen;						\
	u4Byte		__i;									\
	DbgPrint("%s", szTitle);								\
	for (__i=0;__i<u4bHexDataLen;__i++)								\
	{											\
		if ((__i & 15) == 0) 								\
		{										\
			DbgPrint("\n");								\
		}										\
		DbgPrint("%02X%s", pbtHexData[__i], ( ((__i&3)==3) ? "  " : " ") );			\
	}											\
	DbgPrint("\n");										\
}
#endif



#define	RTL_WLAN_INT_RETRY_CNT_MAX	(1)
#if defined(__OSK__)
__IRAM_IN_865X_HI
#else
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
#endif
__inline__ static int __rtl_wlan_interrupt(void *dev_instance)

{
    struct net_device       *dev;
    struct rtl8192cd_priv   *priv;

    unsigned int status, status_ext, retry_cnt = 0;
    BOOLEAN caseBcnInt, caseBcnStatusOK, caseBcnStatusER, caseBcnDMAER;
    BOOLEAN caseRxRDU, caseRxOK, caseRxFOVW, caseTxFOVW , caseTxErr , caseRxErr;
#if defined(HW_DETEC_POWER_STATE)
    BOOLEAN casePwrInt0, casePwrInt1, casePwrInt2, casePwrInt3 , casePwrInt4;    
#endif
	BOOLEAN caseC2HIsr;

#if defined(SW_TX_QUEUE) ||defined(RTK_ATM)
    BOOLEAN caseGTimer4Int;
#endif

#if defined(CONFIG_VERIWAVE_CHECK)
    BOOLEAN caseTimer1 = FALSE;
#endif

#if 1   //Filen: temp
    static unsigned int caseRxRDUCnt=0, caseRxOKCnt=0;
#endif

#if	defined(SUPPORT_AXI_BUS_EXCEPTION) 
    BOOLEAN caseAXIException;
#endif

#if	defined(SUPPORT_AP_OFFLOAD) 
    //yllin
    BOOLEAN caseCPWM2;
#endif

#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
    BOOLEAN caseTimer2;
#endif

	BOOLEAN casePSTimer2 = FALSE;
    dev = (struct net_device *)dev_instance;
	priv = GET_DEV_PRIV(dev);

    if(_FALSE == GET_HAL_INTERFACE(priv)->InterruptRecognizedHandler(priv, NULL, 0)) {
#ifdef WLAN_NULL_INT_WORKAROUND
		delay_us(40); 
		priv->ext_stats.null_interrupt_cnt1++;
		
		if(_FALSE == GET_HAL_INTERFACE(priv)->InterruptRecognizedHandler(priv, NULL, 0)) 
#endif			
		{
#ifdef PCIE_PME_WAKEUP_TEST//yllin         
        unsigned long wakestatus;
        u1Byte reg_val;
        u4Byte u4_val;

        char PME_flag = FALSE;
        wakestatus=REG32(0xb8b00090);
        if(wakestatus & 0x10000){
            PME_flag = TRUE;
            REG32(0xb8b00090)= wakestatus ;  //write 1 clear RC pme status
        }
        wakestatus=REG32(0xb8b10044);
        if(wakestatus & 0x8000){
            wakestatus=REG32(0xb8b10044);
            REG32(0xb8b10044)= wakestatus; //write 1 clear device pme status
            
        }
        //stop apofflaod
        GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 0, 1, 0, 0, NULL, NULL);
        //restore setting
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
    	reg_val = reg_val | BIT(6);
    	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);

        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
    	u4_val = u4_val & ~BIT(8);
    	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
        //printk("0x284=%x\n",RTL_R32(0x284));
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
    	u4_val = u4_val & ~BIT(18);
    	GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);
#endif
#if !defined(NOT_RTK_BSP) && !defined(RTK_NL80211) && !defined(__OSK__)
	// don't print for non-RTK platforms, interrupt line may be shared among devices
#ifdef PCIE_PME_WAKEUP_TEST //yllin
        if(PME_flag)
            printk("DETECT PCIE PME WAKEUP\n");
#ifndef WLAN_NULL_INT_WORKAROUND
			else
#endif
#endif	
#ifndef WLAN_NULL_INT_WORKAROUND
	       	panic_printk("NULL Wlan Interrupt !?\n");		
#endif
#endif
#ifdef WLAN_NULL_INT_WORKAROUND
			priv->ext_stats.null_interrupt_cnt2++;
#endif
        return SUCCESS;
    }
	}
    //Break Condition, satisfy one of condtion below:
    //   1.) retry cnt until our setting value
    //   2.) No interupt pending
    //while(1) 
int_retry_process:    
	{
        //4 Initialize
        caseBcnInt          = FALSE;
        caseBcnStatusOK     = FALSE;
        caseBcnStatusER     = FALSE;
        caseBcnDMAER        = FALSE;

        caseRxRDU           = FALSE;
        caseRxOK            = FALSE;
        caseRxFOVW          = FALSE;
        
        #if defined(SW_TX_QUEUE) ||defined(RTK_ATM)
        caseGTimer4Int      = FALSE;
        #endif

#if	defined(SUPPORT_AXI_BUS_EXCEPTION) 
        caseAXIException    = FALSE;
#endif

#if defined(CONFIG_VERIWAVE_CHECK)
        caseTimer1 = FALSE;
#endif

#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
        caseTimer2 = FALSE;
#endif

        //4 Check interrupt handler
        // 1.) Beacon
        caseBcnInt      = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt_MBSSID);
        caseBcnDMAER    = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BCNDERR0);


#ifdef  SUPPORT_EACH_VAP_INT
        if (IS_SUPPORT_EACH_VAP_INT(priv)) {  
            caseBcnStatusOK = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXBCNOK_MBSSID);
            caseBcnStatusER = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXBCNERR_MBSSID);
        }else
#endif
        {
           caseBcnStatusOK = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TBDOK);
           caseBcnStatusER = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TBDER);
        }

        if(TRUE == caseBcnDMAER)
        {
            priv->ext_stats.beacon_dma_err++;
        }

        if (TRUE == caseBcnInt || TRUE == caseBcnStatusOK || TRUE == caseBcnStatusER) {
            rtl88XX_bcnProc(priv, caseBcnInt, caseBcnStatusOK, caseBcnStatusER);
        }

#ifdef TXREPORT
		caseC2HIsr = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_C2HCMD);
		if (TRUE == caseC2HIsr) {
#ifdef __ECOS
			priv->pshare->has_triggered_C2H_isr = 1;
			priv->pshare->call_dsr = 1;
#else
			//C2H_isr_88XX(priv);
			GET_HAL_INTERFACE(priv)->C2HHandler(priv);
#endif
		}
#endif

        // 2.) Rx
        caseRxRDU = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RDU);
        if (TRUE == caseRxRDU) {
            caseRxRDUCnt++; //filen: temp
            priv->ext_stats.rx_rdu++;
            priv->pshare->skip_mic_chk = SKIP_MIC_NUM;            
        }
        
        caseRxOK = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RX_OK);
        if (TRUE == caseRxOK) {
            caseRxOKCnt++; //filen: temp
        }
        
        caseRxFOVW = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RXFOVW);
        if (TRUE == caseRxFOVW) {
            priv->ext_stats.rx_fifoO++;
            priv->pshare->skip_mic_chk = SKIP_MIC_NUM;            
        }

#if 1
                // 5.) check PS INT
#ifdef HW_DETEC_POWER_STATE
            if (IS_SUPPORT_HW_DETEC_POWER_STATE(priv)) {    
                casePwrInt0 = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_PwrInt0);
                casePwrInt1 = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_PwrInt1);        
                casePwrInt2 = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_PwrInt2);        
                casePwrInt3 = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_PwrInt3);        
                casePwrInt4 = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_PwrInt4);        
                int  i;

                //printk("casePwrInt0 = %x pwr = %x \n",casePwrInt0,RTL_R32(0x1140));          
                
                
                if (TRUE == casePwrInt0 || TRUE == casePwrInt1 || TRUE == casePwrInt2 || TRUE == casePwrInt3 || TRUE == casePwrInt4) {
#ifdef HW_DETEC_POWER_STATE                      
                    if(TRUE == casePwrInt0)
                    {
                        detect_hw_pwr_state(priv,0);
                    }

                    if(TRUE == casePwrInt1)
                    {
                        detect_hw_pwr_state(priv,1);
                    }                    

                    if(TRUE == casePwrInt2)
                    {
                        detect_hw_pwr_state(priv,2);
                    }                    

                    if(TRUE == casePwrInt3)
                    {
                        detect_hw_pwr_state(priv,3);
                    }                    
#endif //#ifdef HW_DETEC_POWER_STATE     
                }
            }
#endif //defined(HW_DETEC_POWER_STATE)
#endif 

        if (TRUE == caseRxRDU || TRUE == caseRxOK || TRUE == caseRxFOVW) {
            InterruptRxHandle(priv, caseRxRDU);
        }

        // 3.) Tx
        InterruptTxHandle(priv);

        // 4.) check DMA error
        caseTxFOVW = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXFOVW);
        caseTxErr  = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_TXERR);
        caseRxErr  = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_RXERR);

        if (TRUE == caseTxFOVW || TRUE == caseTxErr) {
            // check Tx DMA error
            u4Byte  TxDMAStatus = 0;
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_TXDMA_STATUS, (pu1Byte)&TxDMAStatus);

            if(TxDMAStatus)
            {
               printk("[%s]TXDMA Error TxDMAStatus =%x\n", priv->dev->name, TxDMAStatus);
               priv->pshare->tx_dma_status |= TxDMAStatus;
               priv->pshare->tx_dma_err++;		               
               GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_NUM_TXDMA_STATUS, (pu1Byte)&TxDMAStatus);            
#ifdef CHECK_LX_DMA_ERROR
			   check_hangup(priv);
#endif
            }

#ifdef CONFIG_WLAN_HAL_8814AE
            if (caseTxFOVW) 
            {
                priv->ext_stats.tx_fovw++;
            }
#endif

        }        

        if (TRUE == caseRxFOVW || TRUE == caseRxErr) {
            // check Rx DMA error            
            u1Byte  RxDMAStatus = 0;
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_NUM_RXDMA_STATUS, (pu1Byte)&RxDMAStatus);

            if(RxDMAStatus)
            {
               printk("RXDMA Error RxDMAStatus =%x\n",RxDMAStatus);
               priv->pshare->rx_dma_status |= RxDMAStatus;               
               priv->pshare->rx_dma_err++;		               
               GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_NUM_RXDMA_STATUS, (pu1Byte)&RxDMAStatus);            
#ifdef CHECK_LX_DMA_ERROR
			   check_hangup(priv);
#endif			   
            }        
        }

#if defined(CONFIG_VERIWAVE_CHECK)
        caseTimer1 = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_PSTIMEOUT1);
        if (caseTimer1) {
            unsigned int current_value = 0;
            unsigned int timeout = 0;
            cancel_timer1(priv);
		
		    current_value = RTL_R32(TSFTR) ;
		    timeout = RTL_R32(TIMER0);
		    if (TSF_LESS(current_value, timeout)){
		        //printk("TIMER1 %d, jiffies:%d \n", current_value, jiffies);
			    setup_timer1(priv, timeout);
		    }else {
			    //printk("TIMER1 %d,  2 jiffies:%d \n", current_value, jiffies);
			    rtl8192cd_check_veriwave_timer((unsigned long)priv);
			    setup_timer1(priv, current_value + CHK_VERIWAVE_TO);
		    }
		}
#endif

        // 4.) TX_AMSDU & P2P

#if defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)        
        caseTimer2  = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_FS_TIMEOUT0);
        if ( TRUE == caseTimer2 ) {
            InterruptPSTimer2Handle(priv);
        }

#endif

#if 0
        //4 Check Break Condition
        if(_FALSE == GET_HAL_INTERFACE(priv)->InterruptRecognizedHandler(priv, NULL, 0)) {
            break;
        }
        else {
            retry_cnt++;

            if ( retry_cnt >= RTL_WLAN_INT_RETRY_CNT_MAX ) {
                break;
            }
            else {
				watchdog_kick();
            }
        }    
#endif
    // 6.) SW TX QUEUE ,  RTK_ATM
    #if defined(SW_TX_QUEUE) ||defined(RTK_ATM)
    caseGTimer4Int = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_GTIMER4);
    if(caseGTimer4Int == TRUE) {
        
        #ifdef RTK_ATM  
        if(priv->pshare->rf_ft_var.atm_en) {
            if(priv->pshare->atm_swq_use_hw_timer)
                rtl8192cd_atm_swq_timeout((unsigned long) priv);
        } else
        #endif //RTK_ATM
        {

            #ifdef SW_TX_QUEUE
            if(priv->pshare->swq_use_hw_timer) {
                if (!priv->pshare->has_triggered_sw_tx_Q_tasklet) {
                    priv->pshare->has_triggered_sw_tx_Q_tasklet = 1;
                    #ifdef __ECOS                
                    priv->pshare->call_dsr = 1; /*by DSR(ECOS) at 8192cd_ecos.c  / tasklet(Linux)*/ 
                    #else
                    tasklet_schedule(&priv->pshare->swq_tasklet);
                    #endif
                }
            }
            #endif //SW_TX_QUEUE

        }


    }
    #endif //defined(SW_TX_QUEUE) ||defined(RTK_ATM)

	// 7.) AXI exception
	#if	defined(SUPPORT_AXI_BUS_EXCEPTION) 
        caseAXIException    = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_AXI_EXCEPTION);
        if(caseAXIException == TRUE) {
            priv->pshare->axi_exception++;		   
        }
    #endif
    // 8.) CPWM2 //yllin
    #if defined(SUPPORT_AP_OFFLOAD)
        caseCPWM2  = GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_CPWM2);
        if ( TRUE == caseCPWM2 ) {
            u1Byte reg_val;
            u4Byte u4_val;
            u1Byte rxdone_check_count=0;
    #ifdef CONFIG_SAPPS
            u1Byte H2CCommand[2]={0};
    #endif
            //static u4Byte wakeup_counter = 0;
            printk("get CPWM2 INT!!!!!\n");
            //wakeup_counter++;

    #ifdef CONFIG_32K
        //disable 32K
            priv->offload_32k_flag = 0;
            RTL_W8(0x3d9,(RTL_R8(0x3d9)&~BIT0)^BIT7);
            printk("disable 32k\n");

    #endif
            GET_HAL_INTERFACE(priv)->SetAPOffloadHandler(priv, 0, 1, 0, 0, NULL, NULL);
            //restore setting
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);
    		reg_val = reg_val | BIT(6);
    		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CCK_CHECK, (pu1Byte)&reg_val);

            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
    		u4_val = u4_val & ~BIT(8);
    		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_REG_CR, (pu4Byte)&u4_val);
            //printk("0x284=%x\n",RTL_R32(0x284));
        #if 0
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
    		u4_val = u4_val & ~BIT(18);
    		GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);
        #endif
            //printk("0x284=%x\n",RTL_R32(0x284));
    #ifdef CONFIG_SAPPS
            FillH2CCmd88XX(priv,0x26,2,H2CCommand);
            printk("leave SAPPS 1c4=0x%x\n",RTL_R8(0x1c4));
    
    #endif
          while(RTL_R32(0x120) & BIT(16)){
    			if(rxdone_check_count > 20){
    				printk("error, 0x120 BIT16=1\n");
                    return FAIL;
    				//break;
    			}
    			//printk("wait apoffload disable\n");
    			//delay_ms(1);
                delay_us(100);
    			rxdone_check_count++;
		    }
            printk("apoffload disable success\n");
            

            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val); //release RXDMA
            u4_val = u4_val & ~BIT(18);
            GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_RXPKT_NUM, (pu4Byte)&u4_val);

        }
    #endif 

	if ((retry_cnt++) <= RTL_WLAN_INT_RETRY_CNT_MAX ) {
		if (GET_HAL_INTERFACE(priv)->InterruptRecognizedHandler(priv, NULL, 0)) {
			PHAL_DATA_TYPE              pHalData = _GET_HAL_DATA(priv);
			if ((pHalData->IntArray_bak[0] != pHalData->IntArray[0]) || (pHalData->IntArray_bak[1] != pHalData->IntArray[1]))
				goto int_retry_process;
		}
	}	
    }
    return SUCCESS;
}
#endif  //CONFIG_WLAN_HAL

void check_dma_error(struct rtl8192cd_priv *priv, unsigned int status, unsigned int status_ext)
{
	unsigned char reg_rxdma;
	unsigned int reg_txdma;
	int clear_isr=0, check_tx_dma=0, check_rx_dma=0;

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		if ((status_ext & HIMRE_88E_RXFOVW) | (status_ext & HIMRE_88E_RXERR))
			check_rx_dma++;
		if ((status_ext & HIMRE_88E_TXFOVW) | (status_ext & HIMRE_88E_TXERR))
			check_tx_dma++;
	} else
#endif
	{
		if ((status & HIMR_RXFOVW) | (status_ext & HIMRE_RXERR))
			check_rx_dma++;

		if ((status & HIMR_TXFOVW) | (status_ext & HIMRE_TXERR))
			check_tx_dma++;
	}

	if (check_rx_dma) {
		reg_rxdma = RTL_R8(RXDMA_STATUS);
		if (reg_rxdma) {
			RTL_W8(RXDMA_STATUS, reg_rxdma);
			DEBUG_ERR("RXDMA_STATUS %02x\n", reg_rxdma);
			priv->pshare->rx_dma_err++;
            priv->pshare->rx_dma_status |= reg_rxdma;               			
			clear_isr = 1;
		}
	}

	if (check_tx_dma) {
		reg_txdma = RTL_R32(TXDMA_STATUS);
		if (reg_txdma) {
			RTL_W32(TXDMA_STATUS, reg_txdma);
			DEBUG_ERR("TXDMA_STATUS %08x\n", reg_txdma);
			priv->pshare->tx_dma_err++;			
            priv->pshare->tx_dma_status |= reg_txdma;
			clear_isr = 1;
		}
	}

	if (clear_isr) {
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			RTL_W32(REG_88E_HISR, status);
			RTL_W32(REG_88E_HISRE, status_ext);
		} else
#endif
		{
			RTL_W32(HISR, status);
			RTL_W32(HISRE, status_ext);
		}
	}
}


#define	RTL_WLAN_INT_RETRY_MAX	(2)

#if(CONFIG_WLAN_NOT_HAL_EXIST)
#if defined(__OSK__)
__IRAM_IN_865X_HI
#else
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
#endif
__inline__ static int __rtl8192cd_interrupt(void *dev_instance)
{
	struct net_device *dev;
	struct rtl8192cd_priv *priv;
	struct rtl8192cd_hw *phw;

	unsigned int status, status_ext, retry_cnt;
	unsigned int status_bak, status_ext_bak;
	unsigned int caseBcnInt, caseBcnStatusOK, caseBcnStatusER, caseBcnDmaOK=0;
	unsigned int caseRxStatus, caseRxRDU;


    
    #if defined(SW_TX_QUEUE) || defined(RTK_ATM)
	unsigned int caseGTimer4Int;
    #endif
	
	#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
	unsigned int caseTimer2;
	#endif
#if defined (SUPPORT_TX_AMSDU)
	unsigned long current_value, timeout;
#endif

	dev = (struct net_device *)dev_instance;
	priv = GET_DEV_PRIV(dev);

#if defined(CONFIG_RTL_92D_DMDP) && !defined(NOT_RTK_BSP)
	if (GET_CHIP_VER(priv)==VERSION_8192D) {
	#if (RTL_USED_PCIE_SLOT==1)
	 	if (!((REG32(0xb8b21004)& 0x01) && (priv->pshare->wlandev_idx ==0)) &&
			!((REG32(0xb8b21004)& 0x02) && (priv->pshare->wlandev_idx ==1))) {
			//printk("INT=[%02x] WLAN(%d)\n",(REG32(0xb8b21004)& 0x0f),(priv->pshare->wlandev_idx));
			goto int_exit;
		}
	#else
	 	if (!((REG32(0xb8b01004)& 0x01) && (priv->pshare->wlandev_idx ==0)) &&
			!((REG32(0xb8b01004)& 0x02) && (priv->pshare->wlandev_idx ==1))) {
			//printk("INT=[%02x] WLAN(%d)\n",(REG32(0xb8b21004)& 0x0f),(priv->pshare->wlandev_idx));
			goto int_exit;
		}
	#endif
	}
#endif 

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) {
#ifdef CONFIG_RTL_92D_DMDP
		REG32(0xb8003000) = REG32(0xb8003000)|BIT(22);
#endif
		goto int_exit;
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		status = RTL_R32(REG_88E_HISR);
		RTL_W32(REG_88E_HISR, status);
		if (status & HIMR_88E_HISR1_IND_INT) {
			status_ext = RTL_R32(REG_88E_HISRE);
			RTL_W32(REG_88E_HISRE, status_ext);
		} else {
			status_ext = 0;
		}
	} else
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B) {
		status = RTL_R32(REG_HISR0_8812);
		RTL_W32(REG_HISR0_8812, status);
			
		status_ext = RTL_R32(REG_HISR1_8812);
		RTL_W32(REG_HISR1_8812, status_ext);
	} else
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
	if (GET_CHIP_VER(priv)==VERSION_8192E) {
		status = RTL_R32(REG_92E_HISR);
		RTL_W32(REG_92E_HISR, status);
		
		status_ext = RTL_R32(REG_92E_HISRE);
		RTL_W32(REG_92E_HISRE, status_ext);
	} else
#endif
	{
		status = RTL_R32(HISR);
		RTL_W32(HISR, status);
		status_ext = RTL_R32(HISRE);
		RTL_W32(HISRE, status_ext);
	}
	if (status == 0 && status_ext == 0) {
		goto int_exit;
	}
	
	retry_cnt = 0;
retry_process:
	
	caseBcnInt = caseBcnStatusOK = caseBcnStatusER = caseBcnDmaOK = 0;
	caseRxStatus = caseRxRDU = 0;

    
    #if defined(SW_TX_QUEUE) || defined(RTK_ATM)
    caseGTimer4Int = 0;
	#endif
    
	#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)
	caseTimer2 = 0;
	#endif
	
#if defined(TXREPORT) 
#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
	if (CHIP_VER_92X_SERIES(priv)) {		
		if(status_ext & BIT(9))
#ifdef __ECOS
			priv->pshare->has_triggered_C2H_isr = 1;
			priv->pshare->call_dsr = 1;
#else
			C2H_isr(priv);
#endif

	}
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B){		
//		if(status & HIMR_92E_C2HCMD){
//			C2H_isr_8812(priv);
//		}
#if (BEAMFORMING_SUPPORT == 1)	
		if(status & IMR_GTINT3_8812)
			Beamforming_TimerCallback(priv);
#endif		
	}
#endif	
#endif


	if(GET_CHIP_VER(priv)!= VERSION_8812E)
	{
		check_dma_error(priv, status, status_ext);
	}

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		if ((status & HIMR_88E_BcnInt) || (status_ext & (HIMRE_88E_BCNDMAINT1 | HIMRE_88E_BCNDMAINT2
			| HIMRE_88E_BCNDMAINT3 | HIMRE_88E_BCNDMAINT4 | HIMRE_88E_BCNDMAINT5
			| HIMRE_88E_BCNDMAINT6 | HIMRE_88E_BCNDMAINT7))) {
			caseBcnInt = 1;
		}

		if (status & HIMR_88E_TBDOK)
			caseBcnStatusOK = 1;

		if (status & HIMR_88E_TBDER)
			caseBcnStatusER = 1;

		if (status & (HIMR_88E_ROK | HIMR_88E_RDU)) {
			caseRxStatus = 1;

			if (status & HIMR_88E_RDU) {
				priv->ext_stats.rx_rdu++;
				caseRxRDU = 1;
				priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
			}
		}

		if (status_ext & HIMRE_88E_RXFOVW) {
			priv->ext_stats.rx_fifoO++;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}
	} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B) {
	
		if ((status & HIMR_92E_BcnInt) || (status_ext & (HIMRE_92E_BCNDMAINT1 | HIMRE_92E_BCNDMAINT2
			| HIMRE_92E_BCNDMAINT3 | HIMRE_92E_BCNDMAINT4 | HIMRE_92E_BCNDMAINT5
			| HIMRE_92E_BCNDMAINT6 | HIMRE_92E_BCNDMAINT7))) {
			caseBcnInt = 1;
		}
	
        #if defined(SW_TX_QUEUE) || defined(RTK_ATM)
        if(status & IMR_GTINT4_8812) {
           caseGTimer4Int = 1;
        }
        #endif
		if (status & HIMR_92E_TBDOK)
			caseBcnStatusOK = 1;
	
		if (status & HIMR_92E_TBDER)
			caseBcnStatusER = 1;
	
		if (status & (HIMR_92E_ROK | HIMR_92E_RDU)) {
			caseRxStatus = 1;
	
    		if (status & HIMR_92E_RDU) {
	    		priv->ext_stats.rx_rdu++;
		    	caseRxRDU = 1;
			    priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
			}
		}
	
		if (status_ext & HIMRE_92E_RXFOVW) {
			priv->ext_stats.rx_fifoO++;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}
	} else
#endif
	{
		if (status & (HIMR_BCNDMA0 | HIMR_BCNDMA1 | HIMR_BCNDMA2 | HIMR_BCNDMA3 | HIMR_BCNDMA4 | HIMR_BCNDMA5 | HIMR_BCNDMA6 | HIMR_BCNDMA7))
			caseBcnInt = 1;
		if (status & (HIMR_BCNDOK0 | HIMR_BCNDOK1 | HIMR_BCNDOK2 | HIMR_BCNDOK3 | HIMR_BCNDOK4 | HIMR_BCNDOK5 | HIMR_BCNDOK6 | HIMR_BCNDOK7))
			caseBcnDmaOK = 1;

		if (status & HIMR_TXBCNOK)
			caseBcnStatusOK = 1;

		if (status & HIMR_TXBCNERR)
			caseBcnStatusER = 1;

		if (status & (HIMR_ROK | HIMR_RDU))
			caseRxStatus = 1;

		if (status & HIMR_RDU) {
			priv->ext_stats.rx_rdu++;
			caseRxRDU = 1;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}

		if (status & HIMR_RXFOVW) {
			priv->ext_stats.rx_fifoO++;
			priv->pshare->skip_mic_chk = SKIP_MIC_NUM;
		}
	}

	#if	defined(SUPPORT_TX_AMSDU) || defined(P2P_SUPPORT)

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if ( (GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8723B)) {
		if (status & IMR_TIMER2_8812)
			caseTimer2 = 1;
	} else 
#endif		
	{
		if (status & HIMR_TIMEOUT2)
		caseTimer2 = 1;
	}
	#endif

	if (caseBcnInt || caseBcnStatusOK || caseBcnStatusER || caseBcnDmaOK){
                rtl8192cd_bcnProc(priv, caseBcnInt, caseBcnStatusOK, caseBcnStatusER, status
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			, status_ext
#endif
			);
        }

	//
	// Rx interrupt
	//
	if (caseRxStatus)
	{
		// stop RX first
#if defined(__KERNEL__) || defined(__ECOS)
#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX)
#if defined(RX_TASKLET)
		if (!priv->pshare->has_triggered_rx_tasklet) {
			priv->pshare->has_triggered_rx_tasklet = 1;
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
				RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask & ~HIMR_88E_ROK);
				RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt & ~HIMRE_88E_RXFOVW);
			} else
#endif
			{
				//RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_RDU | HIMR_ROK));

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
				if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv)== VERSION_8723B){
					RTL_W32(REG_HIMR0_8812, priv->pshare->InterruptMask & ~(HIMR_92E_ROK ));
					RTL_W32(REG_HIMR1_8812, priv->pshare->InterruptMaskExt & ~( HIMRE_92E_RXFOVW));
				}
				else
#endif
				RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_ROK));
			}
#ifdef __ECOS
			priv->pshare->call_dsr = 1;
#else
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
#endif
		}
#else
		rtl8192cd_rx_isr(priv);
#endif

#else	// !(defined RTL8190_ISR_RX && RTL8190_DIRECT_RX)
		if (caseRxRDU) {
			rtl8192cd_rx_isr(priv);
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}
		else {
			if (priv->pshare->rxInt_useTsklt)
				tasklet_hi_schedule(&priv->pshare->rx_tasklet);
			else
				rtl8192cd_rx_dsr((unsigned long)priv);
		}
#endif
#else	// !__KERNEL__
		rtl8192cd_rx_dsr((unsigned long)priv);
#endif
	}

	//
	// Tx interrupt
	//
	phw = GET_HW(priv);
#ifdef MP_TEST
#if defined(SMP_SYNC) || defined(__ECOS)
	if (OPMODE & WIFI_MP_STATE){
		if (!priv->pshare->has_triggered_tx_tasklet) {
#ifdef __KERNEL__
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
#elif defined(__ECOS)
			priv->pshare->has_triggered_tx_tasklet = 1;
			priv->pshare->call_dsr = 1;
#endif
		}
	}
#else
	if (OPMODE & WIFI_MP_STATE)
		rtl8192cd_tx_dsr((unsigned long)priv);
#endif
	else
#endif
	if (
#ifdef OSK_LOW_TX_DESC
		(CIRC_CNT_RTK(phw->txhead0, phw->txtail0, NONBE_TXDESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead1, phw->txtail1, NONBE_TXDESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead2, phw->txtail2, BE_TXDESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead3, phw->txtail3, NONBE_TXDESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead4, phw->txtail4, NONBE_TXDESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead5, phw->txtail5, BE_TXDESC) > 10)
#else
		(CIRC_CNT_RTK(phw->txhead0, phw->txtail0, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead1, phw->txtail1, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead2, phw->txtail2, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead3, phw->txtail3, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead4, phw->txtail4, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead5, phw->txtail5, CURRENT_NUM_TX_DESC) > 10)
#endif
	) {
#if defined(__KERNEL__) || defined(__ECOS)
		if (!priv->pshare->has_triggered_tx_tasklet) {
#ifdef __ECOS
			priv->pshare->call_dsr = 1;
#else
			tasklet_schedule(&priv->pshare->tx_tasklet);
#endif
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
#else
#ifdef SMP_SYNC
		if (!priv->pshare->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
#else
		rtl8192cd_tx_dsr((unsigned long)priv);
#endif
#endif
	}

#ifdef SUPPORT_TX_AMSDU
	if (caseTimer2) {

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv)== VERSION_8723B)
			RTL_W32(REG_HIMR0_8812, RTL_R32(REG_HIMR0_8812) & ~	IMR_TIMER2_8812	);
		else
#endif
		RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);

		current_value = RTL_R32(TSFTR) ;
		timeout = RTL_R32(TIMER1);
		if (TSF_LESS(current_value, timeout))
			setup_timer2(priv, timeout);
		else
			amsdu_timeout(priv, current_value);
	}
#endif


#if defined(SW_TX_QUEUE) || defined(RTK_ATM)
    if(caseGTimer4Int)
    {
#ifdef RTK_ATM
    	if(priv->pshare->rf_ft_var.atm_en) {
            if(priv->pshare->atm_swq_use_hw_timer)
        		rtl8192cd_atm_swq_timeout((unsigned long) priv);
        }
        else
#endif
        {
#ifdef SW_TX_QUEUE   
            if(priv->pshare->swq_use_hw_timer) {
                if (!priv->pshare->has_triggered_sw_tx_Q_tasklet) {
                    priv->pshare->has_triggered_sw_tx_Q_tasklet = 1;
                    #ifdef __ECOS                
                    priv->pshare->call_dsr = 1; /*by DSR(ECOS) at 8192cd_ecos.c  / tasklet(Linux)*/ 
                    #else
                    tasklet_schedule(&priv->pshare->swq_tasklet);
                    #endif
                }
            }            
#endif            
        }
    }
#endif //defined(SW_TX_QUEUE) || defined(RTK_ATM)

#ifdef P2P_SUPPORT
	if( rtk_p2p_is_enabled(priv) &&	(rtk_p2p_chk_role(priv,P2P_CLIENT))) {
		if (caseTimer2) {
			RTL_W32(HIMR, RTL_R32(HIMR) & ~HIMR_TIMEOUT2);
			p2p_noa_timer(priv);
		}
	}
#endif

	if ((retry_cnt++)<RTL_WLAN_INT_RETRY_MAX) {

	status_bak = status;
	status_ext_bak = status_ext;

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		status = RTL_R32(REG_88E_HISR);
		if (status & HIMR_88E_HISR1_IND_INT)
			status_ext = RTL_R32(REG_88E_HISRE);
		else
			status_ext = 0;
	} else
#endif                                
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B) {
		status = RTL_R32(REG_HISR0_8812);
		if (status & HIMR_92E_HISR1_IND_INT)
			status_ext = RTL_R32(REG_HISR1_8812);
		else
			status_ext = 0;
	} else
#endif
	{
		status = RTL_R32(HISR);
		status_ext = RTL_R32(HISRE);
	}

	if ((status!=0||status_ext!=0)) {
		//watchdog_kick();

#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			RTL_W32(REG_88E_HISR, status);
			RTL_W32(REG_88E_HISRE, status_ext);
		} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if (GET_CHIP_VER(priv)==VERSION_8812E || GET_CHIP_VER(priv)==VERSION_8723B) {
			RTL_W32(REG_HISR0_8812, status);
			RTL_W32(REG_HISR1_8812, status_ext);
		} else
#endif
		{
			RTL_W32(HISR, status);
			RTL_W32(HISRE, status_ext);
		}

		if((status_bak != status) || (status_ext_bak != status_ext))
		goto retry_process;
	}
	}
int_exit:
#ifdef __ECOS
	return (priv->pshare->call_dsr);
#else
	return SUCCESS;
#endif
}
#else

#if defined(__OSK__)
__IRAM_IN_865X_HI
#else
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
#endif
__inline__ static int __rtl8192cd_interrupt(void *dev_instance)
{
	return 0;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST


#ifdef __ECOS
__MIPS16
__IRAM_IN_865X
int rtl8192cd_interrupt(struct net_device *dev_instance)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev_instance->priv;
	priv->pshare->call_dsr = 0;
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) 
		__rtl_wlan_interrupt((void *)dev_instance);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		__rtl8192cd_interrupt(dev_instance);
	return (priv->pshare->call_dsr);
}
#elif defined(__LINUX_2_6__) || defined(__OSK__)
#if defined(__OSK__)
__IRAM_IN_865X_HI
#else
__MIPS16
__IRAM_IN_865X
#endif
irqreturn_t rtl8192cd_interrupt(int irq, void *dev_instance)
{
#ifdef  CONFIG_WLAN_HAL
	struct net_device *dev = (struct net_device *)dev_instance;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	if (IS_HAL_CHIP(priv)) 
	    __rtl_wlan_interrupt(dev_instance);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif	
	__rtl8192cd_interrupt(dev_instance);
	return IRQ_HANDLED;
}
#else
__MIPS16
__IRAM_IN_865X
void rtl8192cd_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
{
#ifdef  CONFIG_WLAN_HAL
	struct net_device *dev = (struct net_device *)dev_instance;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	if (IS_HAL_CHIP(priv)) 
        __rtl_wlan_interrupt(dev_instance);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif  //CONFIG_WLAN_HAL
        __rtl8192cd_interrupt(dev_instance);

	return;
}
#endif /* ! __ECOS */
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_NETDEV_MULTI_TX_QUEUE) || defined(CONFIG_SDIO_TX_FILTER_BY_PRI)
const u16 rtw_1d_to_queue[8] = {
	_NETDEV_TX_QUEUE_BE,	/* 0 */
	_NETDEV_TX_QUEUE_BK,	/* 1 */
	_NETDEV_TX_QUEUE_BK,	/* 2 */
	_NETDEV_TX_QUEUE_BE,	/* 3 */
	_NETDEV_TX_QUEUE_VI,		/* 4 */
	_NETDEV_TX_QUEUE_VI,		/* 5 */
	_NETDEV_TX_QUEUE_VO,	/* 6 */
	_NETDEV_TX_QUEUE_VO		/* 7 */
};

#if defined(__KERNEL__)
/* Given a data frame determine the 802.1p/1d tag to use. */
unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263)
		return skb->priority - 256;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
u16 rtl8192cd_select_queue(struct net_device *dev, struct sk_buff *skb,
	void *accel_priv, select_queue_fallback_t fallback)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0))
u16 rtl8192cd_select_queue(struct net_device *dev, struct sk_buff *skb,
	void *accel_priv)
#else
u16 rtl8192cd_select_queue(struct net_device *dev, struct sk_buff *skb)
#endif
{
	skb->priority = rtw_classify8021d(skb);

	return rtw_1d_to_queue[skb->priority];
}
#elif defined(__ECOS)
/* Given a data frame determine the 802.1p/1d tag to use. */
unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;
	struct ip *ip;
	struct ether_header *eh = SKB_MAC_HEADER(skb);

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (eh->ether_type == htons(ETH_P_IP)) {
		ip = (struct ip *)((char *)eh + sizeof(struct ether_header)); //fixme for TX_SCATTER
		dscp = ip->ip_tos & 0xfc;
		return dscp >> 5;
	}
	return 0;
}

u16 rtl8192cd_select_queue(struct net_device *dev, struct sk_buff *skb)
{
	unsigned int priority;

	priority = rtw_classify8021d(skb);
	return rtw_1d_to_queue[priority];
}
#endif
#endif // CONFIG_NETDEV_MULTI_TX_QUEUE

#ifdef __KERNEL__
static void rtl8192cd_set_rx_mode(struct net_device *dev)
{

}
#endif /* __KERNEL__ */


struct net_device_stats *rtl8192cd_get_stats(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif

#ifdef WDS
	int idx;
	struct stat_info *pstat;
#endif

#ifdef __KERNEL__
	if(!priv)
	{
		priv = ((struct rtl8192cd_priv *)netdev_priv(dev));
	}
	if(priv == NULL)
	{
	}
#endif
	
	SMP_LOCK(flags);

#ifdef WDS
	if (dev->base_addr == 0) {
		idx = getWdsIdxByDev(priv, dev);
		if (idx < 0) {
			memset(&priv->wds_stats[NUM_WDS-1], 0, sizeof(struct net_device_stats));
			SMP_UNLOCK(flags);
			return &priv->wds_stats[NUM_WDS-1];
		}

		if (netif_running(dev) && netif_running(priv->dev)) {
			pstat = get_stainfo(priv, priv->pmib->dot11WdsInfo.entry[idx].macAddr);
			if (pstat == NULL) {
				DEBUG_ERR("%s: get_stainfo() wds fail!\n", (char *)__FUNCTION__);
				memset(&priv->wds_stats[idx], 0, sizeof(struct net_device_stats));
			}
			else {
				priv->wds_stats[idx].tx_packets = pstat->tx_pkts;
				priv->wds_stats[idx].tx_errors = pstat->tx_fail;
				priv->wds_stats[idx].tx_bytes = pstat->tx_bytes;
				priv->wds_stats[idx].rx_packets = pstat->rx_pkts;
				priv->wds_stats[idx].rx_bytes = pstat->rx_bytes;
			}
		}
		SMP_UNLOCK(flags);
		return &priv->wds_stats[idx];
	}
#endif

#ifdef CONFIG_RTK_MESH

	if (dev->base_addr == 1) {
		if(priv->mesh_dev != dev)
		{
			SMP_UNLOCK(flags);
			return NULL;
		}

		SMP_UNLOCK(flags);
		return &priv->mesh_stats;
	}
#endif // CONFIG_RTK_MESH

	SMP_UNLOCK(flags);
	return &(priv->net_stats);
}

#if defined(CONFIG_WLAN_HAL) && defined(CONFIG_PCI_HCI)
static void
rtl88xx_init_swtxdec(
    struct rtl8192cd_priv *priv
)
{
    struct rtl8192cd_hw *phw=NULL; 
    struct tx_desc_info *tx_info;
    u4Byte max_qnum = HIGH_QUEUE7;
    u4Byte QueueIdx;
    u4Byte i;

    phw = GET_HW(priv);

    for (QueueIdx=0; QueueIdx<=max_qnum; QueueIdx++) {
        tx_info         = get_txdesc_info(&phw->tx_info, QueueIdx);
        tx_info->type   = _RESERVED_FRAME_TYPE_;

        for (i=0; i<(TXBD_ELE_NUM-2);i++) {
            tx_info->buf_type[i] = _RESERVED_FRAME_TYPE_;
        }
    }
}
#endif // CONFIG_WLAN_HAL && CONFIG_PCI_HCI

#if defined(MBSSID) 
int vap_opened_num(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *priv_root = GET_ROOT(priv);
	int i, num = 0;
	
	num = 0;
	
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) 
	{
		if (IS_DRV_OPEN(priv_root->pvap_priv[i]))
			num++;
	}

	return num;
}

#if defined(CONFIG_RTL_8812_SUPPORT)

void close_iface_txbf(struct rtl8192cd_priv *priv)
{
	//set txbf mib = 0
	priv->pmib->dot11RFEntry.txbf = 0;
	
	//regen ht ie
	priv->ht_cap_len = 0;
	priv->ht_ie_len = 0;
}

void close_all_iface_txbf(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *priv_root = GET_ROOT(priv);
	int i = 0;
	
//Close Root TXBF
	close_iface_txbf(priv_root);

//Close all VAP TXBF
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) 
	{
		close_iface_txbf(priv_root->pvap_priv[i]);
	}

//Close VXD TXBF	
#ifdef UNIVERSAL_REPEATER
	close_iface_txbf(GET_VXD_PRIV(priv_root));
#endif

}

#endif //CONFIG_RTL_8812_SUPPORT

#endif //MBSSID

#if (BEAMFORMING_SUPPORT == 1)

void check_txbf_support_cap(struct rtl8192cd_priv *priv)
{

    if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) { /* 5G band*/
        #if defined(CONFIG_RTL_5G_SLOT_0)
            #if !defined(CONFIG_SLOT_0_TX_BEAMFORMING)
            priv->pmib->dot11RFEntry.txbf = 0;
            #endif
        #elif defined(CONFIG_RTL_5G_SLOT_1) 
            #if !defined(CONFIG_SLOT_1_TX_BEAMFORMING)
            priv->pmib->dot11RFEntry.txbf = 0;        
            #endif
        #else
            #if !defined(CONFIG_SOC_TX_BEAMFORMING)
            priv->pmib->dot11RFEntry.txbf = 0;        
            #endif
        #endif
    }
    else { /* 2G band*/
        #if defined(CONFIG_RTL_5G_SLOT_0)
            #if !defined(CONFIG_SLOT_1_TX_BEAMFORMING) && !defined(CONFIG_SOC_TX_BEAMFORMING)
            priv->pmib->dot11RFEntry.txbf = 0;
            #endif
        #elif defined(CONFIG_RTL_5G_SLOT_1)
            #if !defined(CONFIG_SLOT_0_TX_BEAMFORMING)
            priv->pmib->dot11RFEntry.txbf = 0;        
            #endif
        #else
            #if defined(CONFIG_WLAN_HAL_8197F)
                #if !defined(CONFIG_SOC_TX_BEAMFORMING)
                priv->pmib->dot11RFEntry.txbf = 0;
                #endif
            #else
                #if !defined(CONFIG_SLOT_0_TX_BEAMFORMING)
                priv->pmib->dot11RFEntry.txbf = 0;        
                #endif
            #endif
        #endif
    }


	if(priv->pmib->dot11RFEntry.txbf)
	{
		if(!(priv->pshare->WlanSupportAbility & WLAN_BEAMFORMING_SUPPORT))
		{
			panic_printk("Disable TXBF due to IC NOT support !\n");
			priv->pmib->dot11RFEntry.txbf = 0;
			priv->pmib->dot11RFEntry.txbfee=0;
			priv->pmib->dot11RFEntry.txbfer=0;
		}
		else if((get_rf_mimo_mode(priv) == MIMO_1T1R) || (get_rf_mimo_mode(priv) == MIMO_1T2R))
		{
			panic_printk("Disable TXBFer due to 1T only !\n");
			priv->pmib->dot11RFEntry.txbfer=0;
			if(GET_CHIP_VER(priv)== VERSION_8881A)
				priv->pmib->dot11RFEntry.txbf=0;
		}
	}
}
#endif

void WLAN_ability_init(struct rtl8192cd_priv *priv)
{

	unsigned long ability;

	ability =	\
			0;

#ifdef CONFIG_WLAN_HAL_8881A
	if (GET_CHIP_VER(priv) == VERSION_8881A) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|
			0;
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|
			WLAN_MULTIMACCLONE_SUPPORT  |
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|
			WLAN_MULTIMACCLONE_SUPPORT  |
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
	if (GET_CHIP_VER(priv) == VERSION_8822B) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|
			WLAN_MULTIMACCLONE_SUPPORT  |
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8197F
	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|	/*verification ok, 20160112, JIRA ID: WISOCNI-21*/
			WLAN_MULTIMACCLONE_SUPPORT  |
			0;
	}
#endif

	priv->pshare->WlanSupportAbility = ability;
	panic_printk("WlanSupportAbility = 0x%x\n", priv->pshare->WlanSupportAbility);

}

#ifdef BT_COEXIST_HAL
void bt_coexist_hal_init(struct rtl8192cd_priv *priv)
{
#ifdef BT_COEXIST_SOCKET
		if(GET_CHIP_VER(priv) == VERSION_8197F){
			priv->pshare->btcoexist.btc_ops = rtl_btc_get_ops_pointer();
			priv->pshare->btcoexist.btc_ops->btc_init_socket(priv);
		}
#endif
}

void bt_coexist_hal_close(struct rtl8192cd_priv *priv)
{
#ifdef BT_COEXIST_SOCKET
		if(GET_CHIP_VER(priv) == VERSION_8197F){
			priv->pshare->btcoexist.btc_ops->btc_close_socket(priv);
		}
#endif
}

#endif //BT_COEXIST_HAL

#if defined(CONFIG_WLAN_HAL_8822BE)
void hard_code_8822_mibs(struct rtl8192cd_priv *priv)
{
	panic_printk("[%s] +++ \n", __FUNCTION__);
	panic_printk("MAX_RX_BUF_LEN = %d \n", MAX_RX_BUF_LEN);

#ifdef LIFETIME_FEATURE
#if (MU_BEAMFORMING_SUPPORT == 1)	
	if(priv->pshare->rf_ft_var.txbf_mu)
		priv->pmib->dot11OperationEntry.lifetime = 0; //lifetime + MU will cause TXDMA error
#endif		
#endif

    priv->pmib->dot11nConfigEntry.dot11nAMSDURecvMax = 0; /* temporary disable receiving 8K amsdu*/
}
#endif

static int rtl8192cd_init_sw(struct rtl8192cd_priv *priv)
{
	// All the index/counters should be reset to zero...
	struct rtl8192cd_hw *phw=NULL;
	unsigned long offset;
	unsigned int  i;
	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct wlanbuf_poll		*pwlanbuf_poll;
	struct wlanicv_poll		*pwlanicv_poll;
	struct wlanmic_poll		*pwlanmic_poll;
	struct wlan_acl_poll	*pwlan_acl_poll;
#ifdef _MESH_ACL_ENABLE_
	struct mesh_acl_poll	*pmesh_acl_poll;
#endif
#ifdef CONFIG_PCI_HCI
	unsigned long ring_virt_addr;
	unsigned long ring_dma_addr;
	unsigned int  ring_buf_len;
#ifndef PRIV_STA_BUF
	unsigned long alloc_dma_buf;
#endif
	struct sk_buff	*pskb;
	unsigned char	*page_ptr;
#ifndef USE_RTL8186_SDK
	unsigned long tx_dma_start;
	struct tx_desc *tx_desc_ptr;
#endif
#endif // CONFIG_PCI_HCI
	unsigned char MIMO_TR_hw_support;
	unsigned int NumTotalRFPath;
#if defined(CLIENT_MODE) && defined(CHECK_HANGUP)
	unsigned char *pbackup=NULL;
	unsigned long backup_len=0;
#endif

#ifdef CONFIG_IEEE80211W_CLI
	if((priv->pmib->dot1180211AuthEntry.dot11IEEE80211W != NO_MGMT_FRAME_PROTECTION)
		 && (OPMODE & WIFI_STATION_STATE)){

		//PMF client mode, must enable SHA256
		priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 1;
	}
#endif

#ifdef RTK_AC_SUPPORT //for 11ac logo
	if(priv->pshare->rf_ft_var.no_rtscts)
	{
		DEBUG_INFO("\n	NO CTS & RTS !!	\n\n");
		if(GET_CHIP_VER(priv) == VERSION_8812E)
		{
			priv->pshare->rf_ft_var.cca_rts = 0;
			priv->pmib->dot11StationConfigEntry.protectionDisabled = 1;
		}
	}
#endif

#if (BEAMFORMING_SUPPORT == 1)
	check_txbf_support_cap(priv);

#if (MU_BEAMFORMING_SUPPORT == 1)
#if defined(CONFIG_WLAN_HAL_8822BE) //eric-mu2 init entry number
	if(GET_CHIP_VER(priv) == VERSION_8822B)
	{
		if(priv->pmib->dot11RFEntry.txbf_mu && (priv->pmib->dot11RFEntry.txbf==0)){

			panic_printk("!! error !! txbf_mu = 1 && txbf = 0 \n");
			priv->pmib->dot11RFEntry.txbf_mu = 0;
		}	
	} 
#endif
#endif
#endif //BEAMFORMING_SUPPORT

#if defined(CONFIG_WLAN_HAL_8822BE)
	if(GET_CHIP_VER(priv) == VERSION_8822B)
		hard_code_8822_mibs(priv);
#endif

#if defined(CONFIG_RTL_SIMPLE_CONFIG) 
	priv->simple_config_could_fix = 0;
	priv->pmib->dot11StationConfigEntry.sc_fix_channel = 0;
#endif

#if defined(DFS) && !defined(RTK_NL80211)
	/*
	 *	For JAPAN : prevent switching to channels 52, 56, 60, and 64 in adhoc mode
	 */
	if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK) ||
		 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
		(OPMODE & WIFI_ADHOC_STATE)) {
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
			/* block channels 52~64 and place them in NOP_chnl */
			if (!timer_pending(&priv->ch52_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 52);
			if (!timer_pending(&priv->ch56_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 56);
			if (!timer_pending(&priv->ch60_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 60);
			if (!timer_pending(&priv->ch64_timer))
				InsertChannel(priv->NOP_chnl, &priv->NOP_chnl_num, 64);
		}

		/* if users select an illegal channel, the driver will switch to channel 36~48 */
		if ((priv->pmib->dot11RFEntry.dot11channel >= 52) && (priv->pmib->dot11RFEntry.dot11channel <= 64)) {
			PRINT_INFO("Channel %d is illegal in ad-hoc mode in Japan!\n", priv->pmib->dot11RFEntry.dot11channel);
			priv->pmib->dot11RFEntry.dot11channel = DFS_SelectChannel(priv);
			PRINT_INFO("Swiching to channel %d!\n", priv->pmib->dot11RFEntry.dot11channel);
		}
	}

	/* if users select a blocked channel, the driver will switch to unblocked channel */
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		((timer_pending(&priv->ch52_timer) && (priv->pmib->dot11RFEntry.dot11channel == 52)) ||
		 (timer_pending(&priv->ch56_timer) && (priv->pmib->dot11RFEntry.dot11channel == 56)) ||
		 (timer_pending(&priv->ch60_timer) && (priv->pmib->dot11RFEntry.dot11channel == 60)) ||
		 (timer_pending(&priv->ch64_timer) && (priv->pmib->dot11RFEntry.dot11channel == 64)) || 
		 (timer_pending(&priv->ch100_timer) && (priv->pmib->dot11RFEntry.dot11channel == 100)) ||
		 (timer_pending(&priv->ch104_timer) && (priv->pmib->dot11RFEntry.dot11channel == 104)) ||
		 (timer_pending(&priv->ch108_timer) && (priv->pmib->dot11RFEntry.dot11channel == 108)) ||
		 (timer_pending(&priv->ch112_timer) && (priv->pmib->dot11RFEntry.dot11channel == 112)) ||
		 (timer_pending(&priv->ch116_timer) && (priv->pmib->dot11RFEntry.dot11channel == 116)) ||
		 (timer_pending(&priv->ch120_timer) && (priv->pmib->dot11RFEntry.dot11channel == 120)) ||
		 (timer_pending(&priv->ch124_timer) && (priv->pmib->dot11RFEntry.dot11channel == 124)) ||
		 (timer_pending(&priv->ch128_timer) && (priv->pmib->dot11RFEntry.dot11channel == 128)) ||
		 (timer_pending(&priv->ch132_timer) && (priv->pmib->dot11RFEntry.dot11channel == 132)) ||
		 (timer_pending(&priv->ch136_timer) && (priv->pmib->dot11RFEntry.dot11channel == 136)) ||
		 (timer_pending(&priv->ch140_timer) && (priv->pmib->dot11RFEntry.dot11channel == 140)) ||
		 (timer_pending(&priv->ch144_timer) && (priv->pmib->dot11RFEntry.dot11channel == 144)))) {
		PRINT_INFO("Channel %d is still in none occupancy period!\n", priv->pmib->dot11RFEntry.dot11channel);
		priv->pmib->dot11RFEntry.dot11channel = DFS_SelectChannel(priv);
		PRINT_INFO("Swiching to channel %d!\n", priv->pmib->dot11RFEntry.dot11channel);
	}

	/* disable all of the transmissions during channel availability check */
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		priv->pmib->dot11DFSEntry.disable_tx = 0;
		if (!priv->pmib->dot11DFSEntry.disable_DFS &&
			(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
			  (priv->pmib->dot11RFEntry.dot11channel <= 64))  || 
			 ((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
			  (priv->pmib->dot11RFEntry.dot11channel <= 140)) ||
			 ((priv->pmib->dot11RFEntry.dot11channel == 0) &&
			  (priv->pmib->dot11RFEntry.band5GSelected & PHY_BAND_5G_1) == 0)) &&
			(OPMODE & WIFI_AP_STATE))
			priv->pmib->dot11DFSEntry.disable_tx = 1;
	}
#endif

#ifdef ENABLE_RTL_SKB_STATS
 	rtl_atomic_set(&priv->rtl_tx_skb_cnt, 0);
 	rtl_atomic_set(&priv->rtl_rx_skb_cnt, 0);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if !defined(__OSK__)
#ifdef DFS
		/* will not initialize the tasklet if the driver is rebooting due to the detection of radar */
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
#endif
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
#ifdef SMART_REPEATER_MODE
				if (!priv->pshare->switch_chan_rp)
#endif
				{
#ifdef CONFIG_RTL_SIMPLE_CONFIG
					if(!priv->pmib->dot11StationConfigEntry.sc_enabled || ((priv->simple_config_status==0) && (IS_ROOT_INTERFACE(priv) 
#ifdef UNIVERSAL_REPEATER
&& (GET_VXD_PRIV(priv)->simple_config_status==0)
#endif
)))
#endif
					{
#ifdef PCIE_POWER_SAVING
#ifdef __ECOS
						priv->pshare->ps_tasklet = 1;
#else
						tasklet_init(&priv->pshare->ps_tasklet, PCIe_power_save_tasklet, (unsigned long)priv);
#endif
#endif
#ifdef CONFIG_PCI_HCI
#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
						tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_dsr, (unsigned long)priv);
#else
#ifdef RX_TASKLET
#ifdef __ECOS
						priv->pshare->rx_tasklet = 1;
						priv->pshare->has_triggered_rx_tasklet = 0;
#else
						tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_tkl_isr, (unsigned long)priv);
#endif
#endif
#endif
#ifdef __ECOS
						priv->pshare->tx_tasklet = 1;
						priv->pshare->has_triggered_tx_tasklet = 0;
#else
						tasklet_init(&priv->pshare->tx_tasklet, rtl8192cd_tx_dsr, (unsigned long)priv);
						tasklet_init(&priv->pshare->oneSec_tasklet, rtl8192cd_expire_timer, (unsigned long)priv);
#endif
#endif // CONFIG_PCI_HCI
					}
				}
			}
		}
#endif  //__OSK__

#ifdef DFS
		if (priv->pmib->dot11DFSEntry.DFS_detected)
			priv->pmib->dot11DFSEntry.DFS_detected = 0;
#endif

		phw = GET_HW(priv);

		// save descriptor virtual address before reset, david
#ifdef CONFIG_PCI_HCI
		ring_virt_addr = phw->ring_virt_addr;
		ring_dma_addr = phw->ring_dma_addr;
		ring_buf_len = phw->ring_buf_len;
#ifndef PRIV_STA_BUF
		alloc_dma_buf = phw->alloc_dma_buf;
#endif
#endif // CONFIG_PCI_HCI
		// save RF related settings before reset
		MIMO_TR_hw_support = phw->MIMO_TR_hw_support;
		NumTotalRFPath = phw->NumTotalRFPath;

		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
#ifdef CONFIG_PCI_HCI
		phw->ring_virt_addr = ring_virt_addr;
		phw->ring_buf_len = ring_buf_len;
#ifndef PRIV_STA_BUF
		phw->alloc_dma_buf = alloc_dma_buf;
#endif

#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE)
			phw->ring_dma_addr = ring_dma_addr;
#endif
#endif // CONFIG_PCI_HCI
		phw->MIMO_TR_hw_support = MIMO_TR_hw_support;
		phw->NumTotalRFPath = NumTotalRFPath;

#if defined(DUALBAND_ONLY) && defined(CONFIG_RTL8190_PRIV_SKB)
		if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY) 
			split_pool(priv);
		else
			merge_pool(priv);
#endif
	}

#if defined(CLIENT_MODE) && defined(CHECK_HANGUP) && defined(RTK_BR_EXT)
	if (priv->reset_hangup &&
			(OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))) {
		backup_len = ((unsigned long)&((struct rtl8192cd_priv *)0)->br_ip) -
				 ((unsigned long)&((struct rtl8192cd_priv *)0)->join_res)+4;
		pbackup = kmalloc(backup_len, GFP_ATOMIC);
		if (pbackup)
			memcpy(pbackup, &priv->join_res, backup_len);
	}
#endif

	offset = (unsigned long)(&((struct rtl8192cd_priv *)0)->net_stats);
	// zero all data members below (including) stats
	memset((void *)((unsigned long)priv + offset), 0, sizeof(struct rtl8192cd_priv)-offset);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
		priv->site_survey->count=0;
#endif

#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	{
#ifdef SMART_REPEATER_MODE
		if (!priv->pshare->switch_chan_rp)
#endif
		{
		
			priv->up_time = 0;
		}
	}

#if defined(CLIENT_MODE) && defined(CHECK_HANGUP)
	if (priv->reset_hangup && pbackup) {
		memcpy(&priv->join_res, pbackup, backup_len);
		kfree(pbackup);
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		// zero all data members below (including) LED_Timer of share_info
		offset = (unsigned long)(&((struct priv_shared_info*)0)->LED_Timer);
		memset((void *)((unsigned long)priv->pshare+ offset), 0, sizeof(struct priv_shared_info)-offset);

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		// re-initialize timer_evt_queue to make sure timer_evt_queue is empty before cmd_thread activate
		_rtw_init_listhead(&priv->pshare->timer_evt_queue.queue);
		priv->pshare->timer_evt_queue.qlen = 0;
		// re-initialize beacon_timer
		_rtw_init_listhead(&priv->pshare->beacon_timer_event.list);
#endif

#ifdef CONFIG_USB_HCI
#ifdef SMART_REPEATER_MODE
		if (!priv->pshare->switch_chan_rp)
#endif
		{
			priv->pshare->cmd_thread = kthread_run(rtw_cmd_thread, priv, "rtw_cmd/%s", priv->dev->name);
			if (IS_ERR(priv->pshare->cmd_thread)) {
				priv->pshare->cmd_thread = NULL;
				printk("[%s] create cmd_thread fail\n", __FUNCTION__);
				return 1;
			}
		}
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
#ifdef __ECOS
		priv->pshare->xmit_thread = create_rtw_xmit_thread(priv);
		if (*priv->pshare->xmit_thread == 0) {
			priv->pshare->xmit_thread = NULL;
			printk("[%s] create xmit_thread fail\n", __FUNCTION__);
			return 1;
		}
#else
		priv->pshare->xmit_thread = kthread_run(rtw_xmit_thread, priv, "rtw_xmit/%s", priv->dev->name);
		if (IS_ERR(priv->pshare->xmit_thread)) {
			priv->pshare->xmit_thread = NULL;
			printk("[%s] create xmit_thread fail\n", __FUNCTION__);
			return 1;
		}
#endif
		
#ifdef SMART_REPEATER_MODE
		if (!priv->pshare->switch_chan_rp)
#endif
#ifdef P2P_SUPPORT
        if (!priv->p2pPtr->change_role_is_ongoing)
#endif
		{
#ifdef __ECOS
			priv->pshare->cmd_thread = create_rtw_cmd_thread(priv);
			if (*priv->pshare->cmd_thread == 0) {
				priv->pshare->cmd_thread = NULL;
				printk("[%s] create cmd_thread fail\n", __FUNCTION__);
				return 1;
			}
#else
			priv->pshare->cmd_thread = kthread_run(rtw_cmd_thread, priv, "rtw_cmd/%s", priv->dev->name);
			if (IS_ERR(priv->pshare->cmd_thread)) {
				priv->pshare->cmd_thread = NULL;
				printk("[%s] create cmd_thread fail\n", __FUNCTION__);
				return 1;
			}
#endif
		}
#endif // CONFIG_SDIO_HCI
		
#if defined(CONFIG_TCP_ACK_TXAGG) || defined(CONFIG_XMITBUF_TXAGG_ADV)
		mod_timer(&priv->pshare->xmit_check_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));
#endif

#ifdef CONFIG_RTK_MESH
		memset((void *)&priv->pshare->meshare, 0, sizeof(struct MESH_Share));
#ifdef __ECOS
		{
			unsigned char random_buf[4];
			get_random_bytes(random_buf, 4);
			priv->pshare->meshare.seq = random_buf[3];
		}
#else
		get_random_bytes((void *)&priv->pshare->meshare.seq, sizeof(priv->pshare->meshare.seq));
#endif

#endif // CONFIG_RTK_MESH

        pwlan_hdr_poll = priv->pshare->pwlan_hdr_poll;
		pwlanllc_hdr_poll = priv->pshare->pwlanllc_hdr_poll;
		pwlanbuf_poll = priv->pshare->pwlanbuf_poll;
		pwlanicv_poll = priv->pshare->pwlanicv_poll;
		pwlanmic_poll = priv->pshare->pwlanmic_poll;

		pwlan_hdr_poll->count = PRE_ALLOCATED_HDR;
		pwlanllc_hdr_poll->count = PRE_ALLOCATED_HDR;
		pwlanbuf_poll->count = PRE_ALLOCATED_MMPDU;
		pwlanicv_poll->count = PRE_ALLOCATED_HDR;
		pwlanmic_poll->count = PRE_ALLOCATED_HDR;

		// initialize all the hdr/buf node, and list to the poll_list
		INIT_LIST_HEAD(&priv->pshare->wlan_hdrlist);
		INIT_LIST_HEAD(&priv->pshare->wlanllc_hdrlist);
		INIT_LIST_HEAD(&priv->pshare->wlanbuf_list);
		INIT_LIST_HEAD(&priv->pshare->wlanicv_list);
		INIT_LIST_HEAD(&priv->pshare->wlanmic_list);


		for(i=0; i< PRE_ALLOCATED_HDR; i++)
		{
			INIT_LIST_HEAD(&(pwlan_hdr_poll->hdrnode[i].list));
			list_add_tail(&(pwlan_hdr_poll->hdrnode[i].list), &priv->pshare->wlan_hdrlist);

			INIT_LIST_HEAD(&(pwlanllc_hdr_poll->hdrnode[i].list));
			list_add_tail( &(pwlanllc_hdr_poll->hdrnode[i].list), &priv->pshare->wlanllc_hdrlist);

			INIT_LIST_HEAD(&(pwlanicv_poll->hdrnode[i].list));
			list_add_tail( &(pwlanicv_poll->hdrnode[i].list), &priv->pshare->wlanicv_list);

			INIT_LIST_HEAD(&(pwlanmic_poll->hdrnode[i].list));
			list_add_tail( &(pwlanmic_poll->hdrnode[i].list), &priv->pshare->wlanmic_list);
		}

		for(i=0; i< PRE_ALLOCATED_MMPDU; i++)
		{
			INIT_LIST_HEAD(&(pwlanbuf_poll->hdrnode[i].list));
			list_add_tail( &(pwlanbuf_poll->hdrnode[i].list), &priv->pshare->wlanbuf_list);
		}

		DEBUG_INFO("hdrlist=%lx, llc_hdrlist=%lx, buf_list=%lx, icv_list=%lx, mic_list=%lx\n",
			(unsigned long)&priv->pshare->wlan_hdrlist, (unsigned long)&priv->pshare->wlanllc_hdrlist,
			(unsigned long)&priv->pshare->wlanbuf_list, (unsigned long)&priv->pshare->wlanicv_list,
			(unsigned long)&priv->pshare->wlanmic_list);

#ifdef CONFIG_PCI_HCI
		page_ptr = (unsigned char *)phw->ring_virt_addr;
		memset(page_ptr, 0, phw->ring_buf_len); // this is vital!


#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        phw->ring_dma_addr   = 0;
	        phw->rx_ring_addr    = 0;
	        phw->tx_ring0_addr   = 0;
	        phw->tx_ring1_addr   = 0;
	        phw->tx_ring2_addr   = 0;
	        phw->tx_ring3_addr   = 0;
	        phw->tx_ring4_addr   = 0;
	        phw->tx_ring5_addr   = 0;
	        phw->tx_ringB_addr   = 0;
        
#ifdef OSK_LOW_TX_DESC
	        memset(&phw->tx_desc0_dma_addr,   0, sizeof(unsigned long) * NONBE_TXDESC);
	        memset(&phw->tx_desc1_dma_addr,   0, sizeof(unsigned long) * NONBE_TXDESC);
	        memset(&phw->tx_desc2_dma_addr,   0, sizeof(unsigned long) * BE_TXDESC);
	        memset(&phw->tx_desc3_dma_addr,   0, sizeof(unsigned long) * NONBE_TXDESC);
	        memset(&phw->tx_desc4_dma_addr,   0, sizeof(unsigned long) * NONBE_TXDESC);
	        memset(&phw->tx_desc5_dma_addr,   0, sizeof(unsigned long) * BE_TXDESC);
#else
	        memset(&phw->tx_desc0_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc1_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc2_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc3_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc4_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc5_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
#endif
	        memset(&phw->tx_descB_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->rx_descL_dma_addr,   0, sizeof(unsigned long) * NUM_RX_DESC_IF(priv));
            
	        phw->rx_descL   = NULL;
	        phw->tx_desc0   = NULL;
	        phw->tx_desc1   = NULL;
	        phw->tx_desc2   = NULL;
	        phw->tx_desc3   = NULL;
	        phw->tx_desc4   = NULL;
	        phw->tx_desc5   = NULL;
	        phw->tx_descB   = NULL;
        
	        memset(&phw->rx_descL_dma_addr,   0, sizeof(unsigned long) * NUM_RX_DESC_IF(priv)) ;
     
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
#ifdef CONFIG_NET_PCI
		if (!IS_PCIBIOS_TYPE)
#endif
			phw->ring_dma_addr = virt_to_bus(page_ptr)+CONFIG_LUNA_SLAVE_PHYMEM_OFFSET;

		phw->rx_ring_addr  = phw->ring_dma_addr;
		phw->tx_ring0_addr = phw->ring_dma_addr + NUM_RX_DESC_IF(priv) * sizeof(struct rx_desc);
#ifdef OSK_LOW_TX_DESC
		phw->tx_ring1_addr = phw->tx_ring0_addr + NONBE_TXDESC* sizeof(struct tx_desc);
		phw->tx_ring2_addr = phw->tx_ring1_addr + NONBE_TXDESC * sizeof(struct tx_desc);
		phw->tx_ring3_addr = phw->tx_ring2_addr + BE_TXDESC * sizeof(struct tx_desc);
		phw->tx_ring4_addr = phw->tx_ring3_addr + NONBE_TXDESC * sizeof(struct tx_desc);
		phw->tx_ring5_addr = phw->tx_ring4_addr + NONBE_TXDESC * sizeof(struct tx_desc);
		phw->tx_ringB_addr = phw->tx_ring5_addr + BE_TXDESC * sizeof(struct tx_desc);
#else   //OSK_LOW_TX_DESC
		phw->tx_ring1_addr = phw->tx_ring0_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring2_addr = phw->tx_ring1_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring3_addr = phw->tx_ring2_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring4_addr = phw->tx_ring3_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring5_addr = phw->tx_ring4_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ringB_addr = phw->tx_ring5_addr + NUM_TX_DESC * sizeof(struct tx_desc);
#endif

		phw->rx_descL = (struct rx_desc *)page_ptr;
		phw->tx_desc0 = (struct tx_desc *)(page_ptr + NUM_RX_DESC_IF(priv) * sizeof(struct rx_desc));
#ifdef OSK_LOW_TX_DESC
		phw->tx_desc1 = (struct tx_desc *)((unsigned long)phw->tx_desc0 + NONBE_TXDESC * sizeof(struct tx_desc));
		phw->tx_desc2 = (struct tx_desc *)((unsigned long)phw->tx_desc1 + NONBE_TXDESC * sizeof(struct tx_desc));
		phw->tx_desc3 = (struct tx_desc *)((unsigned long)phw->tx_desc2 + BE_TXDESC * sizeof(struct tx_desc));
		phw->tx_desc4 = (struct tx_desc *)((unsigned long)phw->tx_desc3 + NONBE_TXDESC * sizeof(struct tx_desc));
		phw->tx_desc5 = (struct tx_desc *)((unsigned long)phw->tx_desc4 + NONBE_TXDESC * sizeof(struct tx_desc));
		phw->tx_descB = (struct tx_desc *)((unsigned long)phw->tx_desc5 + BE_TXDESC * sizeof(struct tx_desc));
#else   //OSK_LOW_TX_DESC
		phw->tx_desc1 = (struct tx_desc *)((unsigned long)phw->tx_desc0 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc2 = (struct tx_desc *)((unsigned long)phw->tx_desc1 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc3 = (struct tx_desc *)((unsigned long)phw->tx_desc2 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc4 = (struct tx_desc *)((unsigned long)phw->tx_desc3 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc5 = (struct tx_desc *)((unsigned long)phw->tx_desc4 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_descB = (struct tx_desc *)((unsigned long)phw->tx_desc5 + NUM_TX_DESC * sizeof(struct tx_desc));
#endif
		/* To set the DMA address for both RX/TX ring */
		{
#ifndef USE_RTL8186_SDK
			int txDescRingIdx;

			struct tx_desc *tx_desc_array[] = {
				phw->tx_desc0,
				phw->tx_desc1,
				phw->tx_desc2,
				phw->tx_desc3,
				phw->tx_desc4,
				phw->tx_desc5,
				phw->tx_descB,
				0
				};
#ifdef CONFIG_NET_PCI
			unsigned long *tx_desc_dma_array[] = {
				(unsigned long*)(phw->tx_desc0_dma_addr),
				(unsigned long*)(phw->tx_desc1_dma_addr),
				(unsigned long*)(phw->tx_desc2_dma_addr),
				(unsigned long*)(phw->tx_desc3_dma_addr),
				(unsigned long*)(phw->tx_desc4_dma_addr),
				(unsigned long*)(phw->tx_desc5_dma_addr),
				(unsigned long*)(phw->tx_descB_dma_addr),
				(unsigned long*)0
				};
#endif
#endif // !USE_RTL8186_SDK

			/* RX RING */
#if defined(NOT_RTK_BSP)
			for (i=0; i<NUM_RX_DESC_IF(priv); i++) {
				phw->rx_descL_dma_addr[i] = phw->rx_ring_addr + i*(sizeof(struct rx_desc));
			}
#else
			for (i=0; i<NUM_RX_DESC_IF(priv); i++) {
				phw->rx_descL_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rx_descL[i]),
					sizeof(struct rx_desc), PCI_DMA_TODEVICE);
			}
#endif

#ifndef USE_RTL8186_SDK

#if defined(NOT_RTK_BSP)
			tx_dma_start = phw->ring_dma_addr + NUM_RX_DESC_IF(priv) * sizeof(struct rx_desc);
#endif

			/* TX RING */
			txDescRingIdx = 0;

			while (tx_desc_array[txDescRingIdx] != 0) {
#ifdef CONFIG_NET_PCI
				unsigned long *tx_desc_dma_ptr = tx_desc_dma_array[txDescRingIdx];
#endif
				tx_desc_ptr = tx_desc_array[txDescRingIdx];

#if defined(NOT_RTK_BSP)
				for (i=0; i<CURRENT_NUM_TX_DESC; i++) {
					tx_desc_dma_ptr[i]= tx_dma_start + txDescRingIdx*NUM_TX_DESC * sizeof(struct tx_desc) + i*sizeof(struct tx_desc);
				}
#else
				for (i=0; i<CURRENT_NUM_TX_DESC; i++) {
					tx_desc_dma_ptr[i] = get_physical_addr(priv, (void *)(&(tx_desc_ptr[i])),
						sizeof(struct tx_desc), PCI_DMA_TODEVICE);
				}
#endif

				txDescRingIdx ++;
			}
#endif	// !USE_RTL8186_SDK
		}

		DEBUG_INFO("rx_descL=%08lx tx_desc0=%08lx, tx_desc1=%08lx, tx_desc2=%08lx, tx_desc3=%08lx, tx_desc4=%08lx, "
			"tx_desc5=%08lx, tx_descB=%08lx\n",
			(unsigned long)phw->rx_descL, (unsigned long)phw->tx_desc0, (unsigned long)phw->tx_desc1, (unsigned long)phw->tx_desc2,
			(unsigned long)phw->tx_desc3, (unsigned long)phw->tx_desc4, (unsigned long)phw->tx_desc5, (unsigned long)phw->tx_descB);
		}
#endif // CONFIG_PCI_HCI

#if !(defined(__ECOS) && defined(CONFIG_SDIO_HCI))
#ifdef RTK_QUE
		rtk_queue_init(&priv->pshare->skb_queue);
#else
		skb_queue_head_init(&priv->pshare->skb_queue);
#endif
#endif

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        // this three functions must be called in this seqence...it cannot be moved arbitrarily
	        //GET_HAL_INTERFACE(priv)->InitHCIDMAMemHandler(priv);
	        // RX_BUF_LEN must include RX_DESC and Payload
	        if ( GET_HAL_INTERFACE(priv)->PrepareRXBDHandler(priv, RX_BUF_LEN, init_rxdesc_88XX) == RT_STATUS_FAILURE ) {
				panic_printk("PrepareRXBDHandler fail\n");
				return 1;
			}
	        GET_HAL_INTERFACE(priv)->PrepareTXBDHandler(priv);

            //Filen, init SW TXDESC Type
            //To avoid recycle error
            rtl88xx_init_swtxdec(priv);

		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
		// Now for Rx desc...
		for(i=0; i<NUM_RX_DESC_IF(priv); i++)
		{
			pskb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_, 1);
			if (pskb == NULL) {
				panic_printk("can't allocate skbuff for RX (i=%d NUM_RX_DESC=%d), abort!\n", i, NUM_RX_DESC_IF(priv));
				return 1;
			}
			init_rxdesc(pskb, i, priv);
		}

#ifdef OSK_LOW_TX_DESC
		for(i=0; i<BE_TXDESC; i++){
			init_txdesc(priv, phw->tx_desc2, phw->tx_ring2_addr, i);
			init_txdesc(priv, phw->tx_desc5, phw->tx_ring5_addr, i);
		}
		for(i=0; i<NONBE_TXDESC; i++)
		{
			init_nonbe_txdesc(priv, phw->tx_desc0, phw->tx_ring0_addr, i);
			init_nonbe_txdesc(priv, phw->tx_desc1, phw->tx_ring1_addr, i);
			init_nonbe_txdesc(priv, phw->tx_desc3, phw->tx_ring3_addr, i);
			init_nonbe_txdesc(priv, phw->tx_desc4, phw->tx_ring4_addr, i);
		}
#else   //OSK_LOW_TX_DESC
		// Nothing to do for Tx desc...
		for(i=0; i<CURRENT_NUM_TX_DESC; i++)
		{
			init_txdesc(priv, phw->tx_desc0, phw->tx_ring0_addr, i);
			init_txdesc(priv, phw->tx_desc1, phw->tx_ring1_addr, i);
			init_txdesc(priv, phw->tx_desc2, phw->tx_ring2_addr, i);
			init_txdesc(priv, phw->tx_desc3, phw->tx_ring3_addr, i);
			init_txdesc(priv, phw->tx_desc4, phw->tx_ring4_addr, i);
			init_txdesc(priv, phw->tx_desc5, phw->tx_ring5_addr, i);
		}
#endif

#ifdef MBSSID
		for(i=0; i<(RTL8192CD_NUM_VWLAN+1); i++) {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				if (i == RTL8192CD_NUM_VWLAN)
					(phw->tx_descB + i)->Dword10 = set_desc(phw->tx_ringB_addr);
					//(phw->tx_descB + i)->NextTxDescAddress = set_desc(phw->tx_ringB_addr);
				else
					(phw->tx_descB + i)->Dword10 = set_desc(phw->tx_ringB_addr + (i+1) * sizeof(struct tx_desc));
					//(phw->tx_descB + i)->NextTxDescAddress = set_desc(phw->tx_ringB_addr + (i+1) * sizeof(struct tx_desc));
			}
		}
#endif


		}
#endif // CONFIG_PCI_HCI

		priv->pshare->amsdu_timer_head = priv->pshare->amsdu_timer_tail = 0;

#ifdef RX_BUFFER_GATHER
#ifdef RX_BUFFER_GATHER_REORDER
				priv->pshare->gather_list_p=NULL;
#else
				INIT_LIST_HEAD(&priv->pshare->gather_list);
#endif
#endif	

#ifdef USE_TXQUEUE
		if (init_txq_pool(&priv->pshare->txq_pool, &priv->pshare->txq_pool_addr)) {
			panic_printk("Can not init tx queue pool.\n");
			return 1;
		}
		for (i=BK_QUEUE; i<=VO_QUEUE; i++)
			init_txq_head(&(priv->pshare->txq_list[RSVQ(i)]));
		priv->pshare->txq_isr = 0;
		priv->pshare->txq_stop = 0;
		priv->pshare->txq_check = 0;
#endif	

		
#if (defined(CONFIG_SLOT_0_ANT_SWITCH)|| defined(CONFIG_SLOT_1_ANT_SWITCH)|| defined(CONFIG_RTL_8881A_ANT_SWITCH)) || defined(CONFIG_RTL_8197F_ANT_SWITCH)
	if(!CHIP_VER_92X_SERIES(priv))
		priv->pshare->rf_ft_var.antHw_enable=0;
#endif

		
#ifdef TX_EARLY_MODE
		//if ((GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8188C)
		//		&& GET_TX_EARLY_MODE)
		if ((GET_CHIP_VER(priv) != VERSION_8188E) && GET_TX_EARLY_MODE)
			GET_TX_EARLY_MODE = 0;			
#endif

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
        if(GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8192D || GET_CHIP_VER(priv) == VERSION_8188C) {
            priv->pshare->fw_support_sta_num = RTL8192CD_NUM_STAT - 1; // One for MAGANEMENT_AID
            priv->pshare->total_cam_entry = 32;
        }
        else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
        if (GET_CHIP_VER(priv)==VERSION_8188E) {
            priv->pshare->fw_support_sta_num = RTL8188E_NUM_STAT - 1; // One for MAGANEMENT_AID
            priv->pshare->total_cam_entry = 64;
        }
        else
#endif
        {
            priv->pshare->fw_support_sta_num = FW_NUM_STAT - 1; // One for MAGANEMENT_AID       
            priv->pshare->total_cam_entry = 64;
        }
        priv->pshare->fw_free_space =  priv->pshare->fw_support_sta_num;
        if(NUM_STAT > priv->pshare->fw_support_sta_num) {
            priv->pshare->fw_free_space--;
        }

#ifdef RTK_ATM
        if(priv->pshare->rf_ft_var.atm_en) {
            #if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT)
            if(GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8192D ||
               GET_CHIP_VER(priv) == VERSION_8188C || GET_CHIP_VER(priv) == VERSION_8188E) {
				priv->pshare->atm_swq_use_hw_timer = 0;
	            init_timer(&priv->pshare->atm_swq_sw_timer);
	            priv->pshare->atm_swq_sw_timer.data = (unsigned long) priv;
				priv->pshare->atm_swq_sw_timer.function = rtl8192cd_atm_swq_timeout;

            } else 
            #endif
            {
                priv->pshare->atm_swq_use_hw_timer = 1;
            }
        }
#endif

#ifdef SW_TX_QUEUE
        #if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT)
        if(GET_CHIP_VER(priv) == VERSION_8192C || GET_CHIP_VER(priv) == VERSION_8192D ||
           GET_CHIP_VER(priv) == VERSION_8188C || GET_CHIP_VER(priv) == VERSION_8188E) {
            priv->pshare->swq_use_hw_timer = 0;
    		init_timer(&priv->pshare->swq_sw_timer);
    		priv->pshare->swq_sw_timer.data = (unsigned long) priv;
    		priv->pshare->swq_sw_timer.function = rtl8192cd_swq_timeout;                    
        }
        else
        #endif
        {
            priv->pshare->swq_use_hw_timer = 1;
            #ifndef __ECOS
            tasklet_init(&priv->pshare->swq_tasklet, rtl8192cd_swq_timeout, (unsigned long)priv);
            #endif
        }
#endif
	}

	//
	//hw features support examinations for root, vxd, and vap interfaces
	//

	// Only 2.4G need to care about 20/40 coexist
	if ((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
		priv->pmib->dot11nConfigEntry.dot11nCoexist = 0;

	if ((priv->pmib->dot11RFEntry.tx2path || priv->pmib->dot11RFEntry.bcn2path) && ((get_rf_mimo_mode(priv) == MIMO_1T1R) ||
		((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))) {
		DEBUG_INFO("Disable tx2path due to 1T1R or MP mode.\n");
		priv->pmib->dot11RFEntry.tx2path = 0;
		priv->pmib->dot11RFEntry.bcn2path= 0;
	}
		
	if (priv->pmib->dot11RFEntry.tx2path) {
		if(CHIP_VER_92X_SERIES(priv)){
			if (priv->pmib->dot11RFEntry.txbf) {
				priv->pmib->dot11RFEntry.tx2path = 0;
				DEBUG_INFO("Disable tx2path due to txbf\n");
			}
		}
	}
	//bcn2path only 92C & 92E support
	if(priv->pmib->dot11RFEntry.bcn2path && !(GET_CHIP_VER(priv)==VERSION_8192C || GET_CHIP_VER(priv)==VERSION_8192E))
		priv->pmib->dot11RFEntry.bcn2path = 0;
	
	//when bcn2path is enable, it should always transmit beacon with two path, close power saving rf_mode
	if(priv->pmib->dot11RFEntry.bcn2path && get_rf_mimo_mode(priv) == MIMO_2T2R){
		priv->pmib->dot11RFEntry.tx2path = 0;		
		DEBUG_INFO("Disable tx2path due to bcn2path\n");
#ifdef RF_MIMO_SWITCH
		priv->pshare->rf_ft_var.rf_mode = 2;
		DEBUG_INFO("Disable rf_mode due to bcn2path\n");
#endif
	}

#if defined(CONFIG_RTL_8812_SUPPORT) && defined(MBSSID) 
	if((GET_CHIP_VER(priv) == VERSION_8812E) && (vap_opened_num(priv) >= 5))
	{
		close_all_iface_txbf(priv);	
	}
#endif

	if(CHIP_VER_92X_SERIES(priv) || (GET_CHIP_VER(priv) == VERSION_8188E)){
	//if ((GET_CHIP_VER(priv) != VERSION_8812E) && (GET_CHIP_VER(priv) != VERSION_8192E)) {
		if (priv->pmib->dot11RFEntry.txbf) {
			if (priv->pmib->dot11nConfigEntry.dot11nSTBC) {
				priv->pmib->dot11nConfigEntry.dot11nSTBC = 0;
				DEBUG_INFO("Disable stbc due to txbf\n");
			}
		}
	}

//	if(GET_CHIP_VER(priv) == VERSION_8881A && priv->pmib->dot11nConfigEntry.dot11nSTBC)
//		priv->pmib->dot11nConfigEntry.dot11nSTBC = 0;

#ifdef CONFIG_RTL_8723B_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8723B) {
		priv->pmib->dot11nConfigEntry.dot11nLDPC = 0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if (GET_CHIP_VER(priv) == VERSION_8192E)
	{
#ifdef CONFIG_RTL8672
		priv->pmib->dot11nConfigEntry.dot11nLDPC = 0;	//	Enable LDPC will cause PCIE disconnection.
		DEBUG_INFO("Disable LDPC due to ADSL platform+92E\n");
#else
		priv->pmib->dot11nConfigEntry.dot11nLDPC = priv->pshare->rf_ft_var.ldpc_92e;
#endif
	}
#endif

	if(GET_CHIP_VER(priv) < VERSION_8812E) {
		priv->pmib->dot11nConfigEntry.dot11nLDPC = 0;
		DEBUG_INFO("Disable LDPC due to old chip\n");
	}
#ifdef CONFIG_RTL_8812_SUPPORT
	if( IS_B_CUT_8812(priv))
	{
		DEBUG_INFO("8812 B CUT, Disable LDPC\n");
		priv->pmib->dot11nConfigEntry.dot11nLDPC = 0;
	}
#endif		
#ifdef MULTI_STA_REFINE 
	if(priv->pshare->rf_ft_var.msta_refine&2 )  {
		priv->pshare->rf_ft_var.txbuf_merge=1;
		priv->pmib->dot11RFEntry.tx2path=0;
		priv->pshare->rf_ft_var.tx_pwr_ctrl=0;
	}
#endif		

#ifdef SUPPORT_TX_AMSDU
	if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) &&
		((GET_CHIP_VER(priv) == VERSION_8814A)|| (GET_CHIP_VER(priv) == VERSION_8822B)))
	{
		if(AMSDU_ENABLE ==3)
			AMSDU_ENABLE =0;
		else if(AMPDU_ENABLE)
			AMSDU_ENABLE =2;
	}	
#endif

#if 0
// disable txbf when antenna diversity is enabled
		if(GET_CHIP_VER(priv) == VERSION_8192E) {
#if (defined(CONFIG_SLOT_0_ANT_SWITCH) && defined(CONFIG_SLOT_0_8192EE)) || (defined(CONFIG_SLOT_1_ANT_SWITCH) && defined(CONFIG_SLOT_1_8192EE))
		priv->pmib->dot11RFEntry.txbf = 0;
#endif
		}
		if(GET_CHIP_VER(priv) == VERSION_8812E) {
#if (defined(CONFIG_SLOT_0_ANT_SWITCH) && defined(CONFIG_SLOT_0_8812)) || (defined(CONFIG_SLOT_1_ANT_SWITCH) && defined(CONFIG_SLOT_1_8812))
		priv->pmib->dot11RFEntry.txbf = 0;
#endif
		}
#endif
/*cfg p2p cfg p2p*/
#if defined(CONFIG_RTL_P2P_SUPPORT)
    if(priv->p2pPtr){
        init_timer(&priv->p2pPtr->p2p_search_timer_t);
        init_timer(&priv->p2pPtr->p2p_find_timer_t);        
        priv->p2pPtr->p2p_find_timer_t.data = (unsigned long) priv;
        priv->p2pPtr->p2p_find_timer_t.function = p2p_find_timer;   
        priv->p2pPtr->p2p_search_timer_t.data = (unsigned long) priv;
        priv->p2pPtr->p2p_search_timer_t.function = P2P_search_timer;

        #ifdef RTK_NL80211
        init_timer(&priv->p2pPtr->remain_on_ch_timer);        
        priv->p2pPtr->remain_on_ch_timer.data = (unsigned long) priv;
        priv->p2pPtr->remain_on_ch_timer.function = realtek_cfg80211_RemainOnChExpire;             

        init_timer(&priv->p2pPtr->scan_deny_timer);        
        priv->p2pPtr->scan_deny_timer.data = (unsigned long) priv;
        priv->p2pPtr->scan_deny_timer.function = rtk_p2p_scan_deny_expire;                         
        #endif //CONFIG_P2P            
    }

    if(rtk_p2p_is_enabled(priv)){
        if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G){
            panic_printk("no Support P2P under 5G mode\n");       
        }else{
            if(rtk_p2p_is_enabled(priv)==PROPERTY_P2P){
                panic_printk("[%s][PROPERTY_P2P]\n",priv->dev->name);
                rtk_p2p_init(priv);            
            }
        }
    }    
#endif
	INIT_LIST_HEAD(&priv->wlan_acl_list);
	INIT_LIST_HEAD(&priv->wlan_aclpolllist);
#ifdef SMP_SYNC
	spin_lock_init(&(priv->wlan_acl_list_lock));
#endif

	pwlan_acl_poll = priv->pwlan_acl_poll;
	for(i=0; i< NUM_ACL; i++)
	{
		INIT_LIST_HEAD(&(pwlan_acl_poll->aclnode[i].list));
		list_add_tail(&(pwlan_acl_poll->aclnode[i].list), &priv->wlan_aclpolllist);
	}

	// copy acl from mib to link list
	for (i=0; i<priv->pmib->dot11StationConfigEntry.dot11AclNum; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->wlan_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist,	struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot11StationConfigEntry.dot11AclAddr[i], MACADDRLEN);
		paclnode->mode = (unsigned char)priv->pmib->dot11StationConfigEntry.dot11AclMode;
		list_add_tail(pnewlist, &priv->wlan_acl_list);
	}

	for(i=0; i<NUM_STAT; i++)
		INIT_LIST_HEAD(&(priv->stat_hash[i]));

#ifdef	CONFIG_RTK_MESH
	/*
	 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
	*/
#ifdef	_MESH_ACL_ENABLE_	// copy acl from mib to link list (below code copy above ACL code)
	INIT_LIST_HEAD(&priv->mesh_acl_list);
	INIT_LIST_HEAD(&priv->mesh_aclpolllist);
#if defined(SMP_SYNC) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	spin_lock_init(&(priv->mesh_acl_list_lock));
#endif

    priv->meshAclCacheMode = 0; /*clear mesh acl cache*/
	pmesh_acl_poll = priv->pmesh_acl_poll;
	for(i=0; i< NUM_MESH_ACL; i++)
	{
		INIT_LIST_HEAD(&(pmesh_acl_poll->meshaclnode[i].list));
		list_add_tail(&(pmesh_acl_poll->meshaclnode[i].list), &priv->mesh_aclpolllist);
	}

	for (i=0; i<priv->pmib->dot1180211sInfo.mesh_acl_num; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->mesh_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist, struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot1180211sInfo.mesh_acl_addr[i], MACADDRLEN);
		paclnode->mode = (unsigned char)priv->pmib->dot1180211sInfo.mesh_acl_mode;

		list_add_tail(pnewlist, &priv->mesh_acl_list);
	}
#endif

#ifdef MESH_BOOTSEQ_AUTH
	INIT_LIST_HEAD(&(priv->mesh_auth_hdr));
#endif

	INIT_LIST_HEAD(&(priv->mesh_unEstablish_hdr));
	INIT_LIST_HEAD(&(priv->mesh_mp_hdr));
#if defined(SMP_SYNC) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	spin_lock_init(&(priv->mesh_mp_hdr_lock));
#endif

	priv->mesh_profile[0].used = FALSE; // Configure by WEB in the future, Maybe delete, Preservation before delete
#endif

	INIT_LIST_HEAD(&(priv->asoc_list));
	INIT_LIST_HEAD(&(priv->auth_list));
	INIT_LIST_HEAD(&(priv->sleep_list));
	INIT_LIST_HEAD(&(priv->defrag_list));
	INIT_LIST_HEAD(&(priv->wakeup_list));
	INIT_LIST_HEAD(&(priv->rx_datalist));
	INIT_LIST_HEAD(&(priv->rx_mgtlist));
	INIT_LIST_HEAD(&(priv->rx_ctrllist));
#ifdef CONFIG_PCI_HCI
	INIT_LIST_HEAD(&(priv->addRAtid_list));	// to avoid add RAtid fail
	INIT_LIST_HEAD(&(priv->addrssi_list));
#endif

#ifdef SMP_SYNC
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef __ECOS
	_rtw_mutex_init(&(priv->hash_list_lock));
	_rtw_mutex_init(&(priv->asoc_list_lock));
	_rtw_mutex_init(&(priv->auth_list_lock));
	_rtw_mutex_init(&(priv->sleep_list_lock));
	_rtw_mutex_init(&(priv->wakeup_list_lock));
#else
	spin_lock_init(&(priv->hash_list_lock));
	spin_lock_init(&(priv->asoc_list_lock));
	spin_lock_init(&(priv->auth_list_lock));
	spin_lock_init(&(priv->sleep_list_lock));
	spin_lock_init(&(priv->wakeup_list_lock));
#endif
#elif defined(CONFIG_PCI_HCI)
	spin_lock_init(&(priv->hash_list_lock));
	spin_lock_init(&(priv->asoc_list_lock));
	spin_lock_init(&(priv->auth_list_lock));
	spin_lock_init(&(priv->sleep_list_lock));
	spin_lock_init(&(priv->wakeup_list_lock));

#endif
	spin_lock_init(&(priv->rx_datalist_lock));
	spin_lock_init(&(priv->rx_mgtlist_lock));
	spin_lock_init(&(priv->rx_ctrllist_lock));
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	_rtw_mutex_init(&(priv->rc_packet_q_lock));
#else
	spin_lock_init(&(priv->rc_packet_q_lock));
	spin_lock_init(&(priv->psk_gkrekey_lock));
	spin_lock_init(&(priv->psk_resend_lock));
#endif
	spin_lock_init(&priv->defrag_lock);
#endif

	
#ifdef A4_STA
	INIT_LIST_HEAD(&(priv->a4_sta_list));
#endif

#ifdef WIFI_SIMPLE_CONFIG
	INIT_LIST_HEAD(&priv->wsc_disconn_list);
	spin_lock_init(&priv->wsc_disconn_list_lock);
#endif

#ifdef TV_MODE
    if(OPMODE & WIFI_AP_STATE) {
        if(priv->pmib->miscEntry.tv_mode > 2) {
            priv->pmib->miscEntry.tv_mode = 2;
        }
    }
    else if(OPMODE & WIFI_STATION_STATE) {
        if(priv->pmib->miscEntry.tv_mode > 1) {
            priv->pmib->miscEntry.tv_mode = 1;
        }
    }
    priv->tv_mode_status = priv->pmib->miscEntry.tv_mode;
#endif

#ifdef CHECK_BEACON_HANGUP
	if (priv->reset_hangup)
	   	priv->pshare->beacon_wait_cnt = 1;
	else
	   	priv->pshare->beacon_wait_cnt = 2;
#endif

#ifdef CHECK_HANGUP
	if (priv->reset_hangup) {
		get_available_channel(priv);
		validate_oper_rate(priv);
		get_oper_rate(priv);
		DOT11_InitQueue(priv->pevent_queue);
#ifdef CONFIG_IEEE80211R
		DOT11_InitQueue(priv->pftevent_queue);
#endif
#if defined(TXREPORT) 
		priv->pshare->sta_query_idx=-1;
		priv->pmib->staDetectInfo.txRprDetectPeriod = 1;
#endif
		return 0;
	}
#endif

#if defined(CONFIG_RTL_8812AR_VN_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) { // 8812AR-VN can not support AC mode
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
			priv->pmib->dot11BssType.net_work_type &= ~WIRELESS_11AC;
	}
#endif

	// construct operation and basic rates set
	{
		// validate region domain
		if ((priv->pmib->dot11StationConfigEntry.dot11RegDomain < DOMAIN_FCC) ||
				(priv->pmib->dot11StationConfigEntry.dot11RegDomain >= DOMAIN_MAX)) {
			PRINT_INFO("invalid region domain, use default value [DOMAIN_FCC]!\n");
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_FCC;
		}

		// validate band
		if (priv->pmib->dot11BssType.net_work_type == 0) {
			PRINT_INFO("operation band is not set, use G+B as default!\n");
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G;
		}
		if ((OPMODE & WIFI_AP_STATE) && (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11B | WIRELESS_11G))) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
				priv->pmib->dot11BssType.net_work_type &= (WIRELESS_11B | WIRELESS_11G);
				PRINT_INFO("operation band not appropriate, use G/B as default!\n");
			}
		}

		if (should_forbid_Nmode(priv)) {
#ifdef SUPPORT_MULTI_PROFILE
			if (!((OPMODE & WIFI_STATION_STATE) && 
				priv->pmib->ap_profile.enable_profile && priv->pmib->ap_profile.profile_num > 0))
#endif						
				priv->pmib->dot11BssType.net_work_type &= ~(WIRELESS_11N|WIRELESS_11AC);

			// validate band
			// for the N mode only + Security = WEP combination, the work_type will become 0
			if (priv->pmib->dot11BssType.net_work_type == 0) {
				PRINT_INFO("operation band is not set, use G+B as default!\n");
				priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G;
			}
		}

		// validate channel number
		if (get_available_channel(priv) == FAIL) {
			PRINT_INFO("can't get operation channels, abort!\n");
			return 1;
		}

		#if defined(RTK_NL80211)	//brian, do not propietary ACS while open with dot11channel=0
		if(!priv->pmib->dot11RFEntry.dot11channel)
			priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];
		#endif

		if (priv->pmib->dot11RFEntry.dot11channel != 0) {
#ifdef DFS			
			if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80 && 
				(priv->pmib->dot11RFEntry.band5GSelected == PHY_BAND_5G_3 ||
				priv->pmib->dot11RFEntry.band5GSelected == 7)) 
			{
				if(!is80MChannel(priv->available_chnl,priv->available_chnl_num,priv->pmib->dot11RFEntry.dot11channel))					
					//priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];
					priv->pmib->dot11RFEntry.dot11channel = DFS_SelectChannel(priv);
				if(!is80MChannel(priv->available_chnl,priv->available_chnl_num,priv->pmib->dot11RFEntry.dot11channel)) {
					if(is40MChannel(priv->available_chnl,priv->available_chnl_num,priv->pmib->dot11RFEntry.dot11channel))
						priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20_40;
					else
						priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20;
				}
			}
			else 
#endif
			{
				for (i=0; i<priv->available_chnl_num; i++)
					if (priv->pmib->dot11RFEntry.dot11channel == priv->available_chnl[i])
						break;
				if (i == priv->available_chnl_num) {
					priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];

					PRINT_INFO("invalid channel number, use default value [%d]!\n",
						priv->pmib->dot11RFEntry.dot11channel);
				}

				// Check whether dot11channel is legal when dot11Use40M = 1 or 2 is set.
				if(priv->pmib->dot11RFEntry.dot11channel > 14) {
					if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80) {
						if(!is80MChannel(priv->available_chnl,priv->available_chnl_num,priv->pmib->dot11RFEntry.dot11channel)) {
							if(is40MChannel(priv->available_chnl,priv->available_chnl_num,priv->pmib->dot11RFEntry.dot11channel))
								priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20_40;
							else
								priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20;
						}						
					}
					else if(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40) {			
						if(!is40MChannel(priv->available_chnl,priv->available_chnl_num,priv->pmib->dot11RFEntry.dot11channel))
							priv->pmib->dot11nConfigEntry.dot11nUse40M = HT_CHANNEL_WIDTH_20;
					}
				}				
				priv->auto_channel = 0;
				priv->auto_channel_backup = priv->auto_channel;
			}
			
#ifdef SIMPLE_CH_UNI_PROTOCOL
			SET_PSEUDO_RANDOM_NUMBER(priv->mesh_ChannelPrecedence);
#endif
		}
		else {
#ifdef SIMPLE_CH_UNI_PROTOCOL
            if(GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
                priv->auto_channel = 1;
                priv->auto_channel_backup = priv->auto_channel;    
                priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];
                priv->mesh_ChannelPrecedence = 0;
                priv->pmib->dot11RFEntry.acs_type = 0; /* disable acs_type when doing mesh scan*/
            }
            else
#endif
			{
#ifdef DFS
				if ((OPMODE & WIFI_AP_STATE) && !priv->pmib->dot11DFSEntry.disable_DFS && (priv->pshare->dfsSwitchChannel != 0)) {
					priv->pmib->dot11RFEntry.dot11channel = priv->pshare->dfsSwitchChannel;
					priv->pshare->dfsSwitchChannel = 0;
					priv->auto_channel = 0;
					priv->auto_channel_backup = 1;
				}
				else
#endif
				{
					if (OPMODE & WIFI_AP_STATE)
						priv->auto_channel = 1;
					else
						priv->auto_channel = 2;
					priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];
					priv->auto_channel_backup = priv->auto_channel;
				}
			}
		}
		//priv->auto_channel_backup = priv->auto_channel;

		if ( !((GET_CHIP_VER(priv) == VERSION_8188C) 
				|| (GET_CHIP_VER(priv) == VERSION_8192C) 
				|| (GET_CHIP_VER(priv) == VERSION_8188E) 
				|| (GET_CHIP_VER(priv) == VERSION_8192E) 
				|| ((GET_CHIP_VER(priv) == VERSION_8814A) && (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G)) ))
		{
			DEBUG_INFO("Only 88C/92C/88E/92E/8194 will use NHM-ACS currently\n");
			priv->pmib->dot11RFEntry.acs_type = 0;
		}

#ifdef NHM_ACS2_SUPPORT
		if ((GET_CHIP_VER(priv) == VERSION_8192E) || (GET_CHIP_VER(priv) == VERSION_8812E) ||(GET_CHIP_VER(priv) == VERSION_8197F)||(GET_CHIP_VER(priv) == VERSION_8822B))
			priv->pmib->dot11RFEntry.acs_type = 2;
#endif

		if (priv->pmib->dot11RFEntry.dot11channel <= 14){
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
			priv->pshare->curr_band = BAND_2G;
		}else{
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
			priv->pshare->curr_band = BAND_5G;
		}
		// validate hi and low channel
		if (priv->pmib->dot11RFEntry.dot11ch_low != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11ch_low == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11ch_low = priv->available_chnl[0];

				PRINT_INFO("invalid low channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11ch_low);
			}
		}
		if (priv->pmib->dot11RFEntry.dot11ch_hi != 0) {
			for (i=0; i<priv->available_chnl_num; i++)
				if (priv->pmib->dot11RFEntry.dot11ch_hi == priv->available_chnl[i])
					break;
			if (i == priv->available_chnl_num) {
				priv->pmib->dot11RFEntry.dot11ch_hi = priv->available_chnl[priv->available_chnl_num-1];

				PRINT_INFO("invalid hi channel number, use default value [%d]!\n",
					priv->pmib->dot11RFEntry.dot11ch_hi);
			}
		}

// Mark the code to auto disable N mode in WEP encrypt
#if 0
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
				(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
					priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_))
			priv->pmib->dot11BssType.net_work_type &= ~WIRELESS_11N;
#endif
//------------------------------ david+2008-01-11

		// support cck only in channel 14
		if ((priv->pmib->dot11RFEntry.disable_ch14_ofdm) &&
			(priv->pmib->dot11RFEntry.dot11channel == 14)) {
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B;
			PRINT_INFO("support cck only in channel 14!\n");
		}

		// validate and get support and basic rates
		validate_oper_rate(priv);
		get_oper_rate(priv);

		if ((priv->pmib->dot11nConfigEntry.dot11nUse40M==2) &&
			(!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC))) {
			PRINT_INFO("enable 80M but not in AC mode! back to 40M\n");
			priv->pmib->dot11nConfigEntry.dot11nUse40M = 1;
		}

		if (priv->pmib->dot11nConfigEntry.dot11nUse40M == 1 &&
			(!(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N | WIRELESS_11AC)))) {
			PRINT_INFO("enable 40M but not in N | AC mode! back to 20M\n");
			priv->pmib->dot11nConfigEntry.dot11nUse40M = 0;
		}

		// check deny band
		if ((priv->pmib->dot11BssType.net_work_type & (~priv->pmib->dot11StationConfigEntry.legacySTADeny)) == 0) {
			PRINT_INFO("legacySTADeny %d not suitable! set to 0\n", priv->pmib->dot11StationConfigEntry.legacySTADeny);
			priv->pmib->dot11StationConfigEntry.legacySTADeny = 0;
		}
	}

#ifdef PERIODIC_AUTO_CHANNEL
		/*periodic channel ; periodicAutochannel need depend on priv->auto_channel
		if priv->auto_channel==0 finally then  set periodicAutochannel to 0*/
		PDEBUG("set priv->auto_channel to %d\n",priv->auto_channel);		
		if(OPMODE & WIFI_AP_STATE){
			if(priv->auto_channel && priv->pmib->dot11RFEntry.periodicAutochannel){
				priv->pshare->PAT = priv->pmib->dot11RFEntry.periodicAutochannel;
				PDEBUG("set PAT=%d\n",priv->pshare->PAT);
			}
		}
#endif

	if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)) {
		if (AMSDU_ENABLE)
			AMSDU_ENABLE = 0;
		if (AMPDU_ENABLE)
			AMPDU_ENABLE = 0;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			if(priv->pmib->dot11nConfigEntry.dot11nUse40M == 4)
				priv->pshare->is_40m_bw = 4;
			else if(priv->pmib->dot11nConfigEntry.dot11nUse40M == 5) 
				priv->pshare->is_40m_bw = 5;
			else
				priv->pshare->is_40m_bw = 0;
			priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
		}
	}
	else {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
			if (priv->pshare->is_40m_bw == 0|| priv->pshare->is_40m_bw == 4 || priv->pshare->is_40m_bw ==5)
				priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_DONTCARE;
			else {
#if defined(RTK_5G_SUPPORT) 
				if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) {
					if((priv->pmib->dot11RFEntry.dot11channel>144) ? ((priv->pmib->dot11RFEntry.dot11channel-1)%8) : (priv->pmib->dot11RFEntry.dot11channel%8)) {
						priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE;
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
					} else {
						priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW;
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
					}
				}
				else 
#endif				
				{
					if ((priv->pmib->dot11RFEntry.dot11channel < 5) &&
							(priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_BELOW))
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_ABOVE;
					else if ((priv->pmib->dot11RFEntry.dot11channel > 9) &&
							(priv->pmib->dot11nConfigEntry.dot11n2ndChOffset == HT_2NDCH_OFFSET_ABOVE))
						priv->pshare->offset_2nd_chan = HT_2NDCH_OFFSET_BELOW;
					else
						priv->pshare->offset_2nd_chan = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
				}
			}
		}

		// force wmm enabled if n mode
		// so hostapd should always set wmm_enabled=1 if n mode.
		//QOS_ENABLE = 1;
	}

	// set wep key length
	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_)
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen = 8;
	else if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)
		priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyLen = 16;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		init_crc32_table();	// for sw encryption
	}

#ifdef WIFI_WMM
	if (QOS_ENABLE) {
		if ((OPMODE & WIFI_AP_STATE)
#ifdef CLIENT_MODE
			|| (OPMODE & WIFI_ADHOC_STATE)
#endif
			) {
			GET_EDCA_PARA_UPDATE = 0;
/*			
			//BK
			GET_STA_AC_BK_PARA.AIFSN = 7;
			GET_STA_AC_BK_PARA.TXOPlimit = 0;
			GET_STA_AC_BK_PARA.ACM = 0;
			GET_STA_AC_BK_PARA.ECWmin = 4;
			GET_STA_AC_BK_PARA.ECWmax = 10;
			//BE
			GET_STA_AC_BE_PARA.AIFSN = 3;
			GET_STA_AC_BE_PARA.TXOPlimit = 0;
			GET_STA_AC_BE_PARA.ACM = 0;
			GET_STA_AC_BE_PARA.ECWmin = 4;
			GET_STA_AC_BE_PARA.ECWmax = 10;
			//VI
			GET_STA_AC_VI_PARA.AIFSN = 2;
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
				(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
				GET_STA_AC_VI_PARA.TXOPlimit = 94; // 3.008ms
			else
				GET_STA_AC_VI_PARA.TXOPlimit = 188; // 6.016ms
			GET_STA_AC_VI_PARA.ACM = 0;
			GET_STA_AC_VI_PARA.ECWmin = 3;
			GET_STA_AC_VI_PARA.ECWmax = 4;
			//VO
			GET_STA_AC_VO_PARA.AIFSN = 2;
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) ||
				(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))
				GET_STA_AC_VO_PARA.TXOPlimit = 47; // 1.504ms
			else
				GET_STA_AC_VO_PARA.TXOPlimit = 102; // 3.264ms
			GET_STA_AC_VO_PARA.ACM = 0;
			GET_STA_AC_VO_PARA.ECWmin = 2;
			GET_STA_AC_VO_PARA.ECWmax = 3;
*/
			default_WMM_para(priv);

			//init WMM Para ie in beacon
			init_WMM_Para_Element(priv, priv->pmib->dot11QosEntry.WMM_PARA_IE);
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE) {
			init_WMM_Para_Element(priv, priv->pmib->dot11QosEntry.WMM_IE);  //  WMM STA
		}
#endif

		if (AMSDU_ENABLE || AMPDU_ENABLE) {
			if (priv->pmib->dot11nConfigEntry.dot11nTxNoAck) {
				priv->pmib->dot11nConfigEntry.dot11nTxNoAck = 0;
				PRINT_INFO("Tx No Ack is off because aggregation is enabled.\n");
			}
		}
	}
#endif

	i = priv->pmib->dot11ErpInfo.ctsToSelf;
	memset(&priv->pmib->dot11ErpInfo, '\0', sizeof(struct erp_mib)); // reset ERP mib
	priv->pmib->dot11ErpInfo.ctsToSelf = i;

	if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G|WIRELESS_11A))
		priv->pmib->dot11ErpInfo.shortSlot = 1;
	else
		priv->pmib->dot11ErpInfo.shortSlot = 0;

	if (OPMODE & WIFI_AP_STATE) {
		memcpy(priv->pmib->dot11StationConfigEntry.dot11Bssid,
				priv->pmib->dot11OperationEntry.hwaddr, 6);
		//priv->oper_band = priv->pmib->dot11BssType.net_work_type;
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			if (!priv->pshare->ra40MLowerMinus && (priv->pshare->rf_ft_var.raGoUp40MLower > 5) &&
				(priv->pshare->rf_ft_var.raGoDown40MLower > 5)) {
				priv->pshare->rf_ft_var.raGoUp40MLower-=5;
				priv->pshare->rf_ft_var.raGoDown40MLower-=5;
				priv->pshare->ra40MLowerMinus++;
			}
#if 0//def HIGH_POWER_EXT_PA
			if (!priv->pshare->raThdHP_Minus) {

				if( priv->pshare->rf_ft_var.use_ext_pa )  {
					if(priv->pshare->rf_ft_var.raGoDownUpper > RSSI_DIFF_PA) {
						priv->pshare->rf_ft_var.raGoDownUpper -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUpUpper -= RSSI_DIFF_PA;
					}
					if(priv->pshare->rf_ft_var.raGoDown20MLower > RSSI_DIFF_PA) {					
						priv->pshare->rf_ft_var.raGoDown20MLower -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUp20MLower -= RSSI_DIFF_PA;
					}
					if(priv->pshare->rf_ft_var.raGoDown40MLower > RSSI_DIFF_PA) {	
						priv->pshare->rf_ft_var.raGoDown40MLower -= RSSI_DIFF_PA;
						priv->pshare->rf_ft_var.raGoUp40MLower -= RSSI_DIFF_PA;		
					}
					if( priv->pshare->rf_ft_var.txop_enlarge_lower > RSSI_DIFF_PA) {
						 priv->pshare->rf_ft_var.txop_enlarge_upper -= RSSI_DIFF_PA;
						 priv->pshare->rf_ft_var.txop_enlarge_lower -= RSSI_DIFF_PA;						 	
					}
				}
				++priv->pshare->raThdHP_Minus;
			}
#endif
		}
	}
#ifdef CLIENT_MODE
	else {
		if (priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen == 0) {
			priv->pmib->dot11StationConfigEntry.dot11DefaultSSIDLen = 11;
			memcpy(priv->pmib->dot11StationConfigEntry.dot11DefaultSSID, "defaultSSID", 11);
		}
		memset(priv->pmib->dot11StationConfigEntry.dot11Bssid, 0, 6);
		JOIN_RES_VAL(STATE_Sta_No_Bss);
#if defined(CONFIG_RTL8672) && !defined(__OSK__)
		wlan_sta_state_notify(priv, STATE_Sta_No_Bss);
#endif

// Add mac clone address manually ----------
#ifdef RTK_BR_EXT
		if (priv->pmib->ethBrExtInfo.macclone_enable == 2) {
			extern void mac_clone(struct rtl8192cd_priv *priv, unsigned char *addr);
			mac_clone(priv, priv->pmib->ethBrExtInfo.nat25_dmzMac);
			priv->macclone_completed = 1;
		}
#endif
//------------------------- david+2007-5-31

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			if (priv->pshare->ra40MLowerMinus) {
				priv->pshare->rf_ft_var.raGoUp40MLower+=5;
				priv->pshare->rf_ft_var.raGoDown40MLower+=5;
				priv->pshare->ra40MLowerMinus = 0;
			}
		}

#ifdef MULTI_MAC_CLONE
        if((priv->pshare->WlanSupportAbility & WLAN_MULTIMACCLONE_SUPPORT) && priv->pmib->ethBrExtInfo.macclone_enable) {
    		if ((priv->pshare->mclone_ok == 0) && (OPMODE & WIFI_STATION_STATE)) 
            {
    			init_Multi_Mac_Clone(priv);
    			priv->pshare->mclone_ok = 1;
    		}
            #ifdef UNIVERSAL_REPEATER
    		if (IS_VXD_INTERFACE(priv)) {			
    			RTL_W32(RCR, RTL_R32(RCR) | BIT(24));	// MBSSID enable
    		}
    	    // for root + Vxd repeater, need to add root into MBSSIDCam to make multiple mac clone work.
            #endif
        }
        else { /*IC do not support, disable it*/
            priv->pmib->ethBrExtInfo.macclone_enable = 0;
        }
#endif
	}
#endif

	// initialize event queue
	DOT11_InitQueue(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_InitQueue(priv->wapiEvent_queue);
#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable)  {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			DOT11_InitQueue(priv->pvap_priv[i]->wapiEvent_queue);
	}
#endif

#endif
#ifdef CONFIG_IEEE80211R
	DOT11_InitQueue(priv->pftevent_queue);
#endif
#ifdef RSSI_MONITOR_NCR
	DOT11_InitQueue(priv->rssimEvent_queue);
#endif
#ifdef CONFIG_RTK_MESH
	if(GET_MIB(priv)->dot1180211sInfo.mesh_enable == 1)	// plus add 0217, not mesh mode should not do below function
	{
        #ifdef A4_STA
        priv->pshare->rf_ft_var.a4_enable = 0;
        #endif

	DOT11_InitQueue2(priv->pathsel_queue, MAXQUEUESIZE2, MAXDATALEN2);
        priv->pmib->dot11sKeysTable.keyid = 0;
        priv->pmib->dot11sKeysTable.keyInCam = TRUE;
        memset(&priv->pmib->dot11sKeysTable.dot11EncryptKey, 0, sizeof(struct Dot11EncryptKey));
    }
#endif

#ifdef __DRAYTEK_OS__
	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _TKIP_PRIVACY_ &&
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _CCMP_PRIVACY_ &&
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_WPA_MIXED_PRIVACY_) {
#ifdef UNIVERSAL_REPEATER
		if (IS_ROOT_INTERFACE(priv))
#endif
		{
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
			memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
#ifdef UNIVERSAL_REPEATER
			if (GET_VXD_PRIV(priv))
				GET_VXD_PRIV(priv)->pmib->dot11RsnIE.rsnielen = 0;
#endif
		}
	}
#endif

	i = RC_ENTRY_NUM;
	for (;;) {
		if (priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz >= i) {
			priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz = i;
			break;
		}
		else if (i > 8)
			i = i / 2;
		else {
			priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz = 8;
			break;
		}
	}

	// Realtek proprietary IE
	memcpy(&(priv->pshare->rtk_ie_buf[0]), Realtek_OUI, 3);
	priv->pshare->rtk_ie_buf[3] = 2;
#ifdef CONFIG_WLAN_HAL_8814AE
	if (GET_CHIP_VER(priv) == VERSION_8814A) {
		#if defined(CONFIG_SLOT_0_8814_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8814_2T2R_SUPPORT)
			if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
				printk("Force 2T2R for 8814 !!!\n");
				priv->pmib->dot11RFEntry.MIMO_TR_mode = 3; // 2T2R
			}
		#endif	
		#if defined(CONFIG_SLOT_0_8194_2T2R_SUPPORT) || defined(CONFIG_SLOT_1_8194_2T2R_SUPPORT)
			if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G) {
				printk("Force 2T2R for 8194 !!!\n");
				priv->pmib->dot11RFEntry.MIMO_TR_mode = 3; // 2T2R
			}
		#endif
		priv->pshare->rtk_ie_buf[4] = 2;
		priv->pshare->rtk_ie_buf[5] = 0;
#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE)
			priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_AP_CLIENT;
#endif
		priv->pshare->rtk_ie_buf[6] = 0;
		priv->pshare->rtk_ie_len = 7;
	}
	else
#endif	
#ifdef CONFIG_RTL_8812_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		priv->pshare->rtk_ie_buf[4] = 2;
		priv->pshare->rtk_ie_buf[5] = 0;
		priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_WLAN_88C92C | RTK_CAP_IE_WLAN_8192SE;
#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE)
			priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_AP_CLIENT;
#endif
		priv->pshare->rtk_ie_buf[6] = 0;
		if (IS_B_CUT_8812(priv))
			priv->pshare->rtk_ie_buf[6] |= RTK_CAP_IE_8812_BCUT;
		else
			priv->pshare->rtk_ie_buf[6] |= RTK_CAP_IE_8812_CCUT;
		priv->pshare->rtk_ie_len = 7;
	}
	else
#endif
	{
		priv->pshare->rtk_ie_buf[4] = 1;
		priv->pshare->rtk_ie_buf[5] = 0;
		priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_WLAN_88C92C | RTK_CAP_IE_WLAN_8192SE;
#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE)
			priv->pshare->rtk_ie_buf[5] |= RTK_CAP_IE_AP_CLIENT;
#endif
		priv->pshare->rtk_ie_len = 6;
	}

#ifdef UNIVERSAL_REPEATER
	if (IS_VXD_INTERFACE(priv)) {
		void *ptr_root, *ptr_vxd;
		unsigned int len;
		ptr_root = (void *)&GET_ROOT(priv)->pmib->dot11RFEntry.pwrlevelCCK_A[0];
		ptr_vxd  = (void *)&priv->pmib->dot11RFEntry.pwrlevelCCK_A[0];
		len = (unsigned int)(&priv->pmib->dot11RFEntry.defaultAntennaB) - (unsigned int)(&priv->pmib->dot11RFEntry.pwrlevelCCK_A[0]);
		if (memcmp(ptr_vxd, ptr_root, len)) {
			panic_printk("Power settings of VXD are not the same as ROOT. Sync!\n");
			memcpy(ptr_vxd, ptr_root, len);
		}
	}
#endif

#if defined(WIFI_HAPD) || defined (RTK_NL80211)//OPENWRT_RADIUS
	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 0)
		&& (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)
		&& (priv->pmib->dot1180211AuthEntry.dot11WPACipher || priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher))
			rsn_init(priv);
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {
		psk_init(priv);
#ifdef SUPPORT_TX_AMSDU
		priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 2;
#endif
	} else {
#if !defined(RTK_NL80211) //make OpenWrt support VAP+RADIUS
#ifdef MBSSID
		if(IS_VAP_INTERFACE(priv) && (!priv->pmib->dot11OperationEntry.keep_rsnie))
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length			
#endif		
#endif
#ifdef SUPPORT_TX_AMSDU
		if ((priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)
			&& (priv->pmib->dot1180211AuthEntry.dot11WPACipher || priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher)
		) {
			priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 2; // 802.1x
		} else {
			#if defined(CONFIG_WLAN_HAL_8822BE) //eric-mu2 init entry number
			if(GET_CHIP_VER(priv) == VERSION_8822B)
				priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 2;
			else
			#endif	
			priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 4; // open
		}		
#endif
	}

#ifdef WDS
#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
        #ifdef A4_STA
        if (priv->pmib->dot11WdsInfo.wdsEnabled) {
            priv->pshare->rf_ft_var.a4_enable = 0;
        }
        #endif
    
#ifdef LAZY_WDS
		if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) {
			priv->pmib->dot11WdsInfo.wdsNum = 0;
			memset(priv->pmib->dot11WdsInfo.entry, '\0', sizeof(struct wdsEntry)*NUM_WDS);
		}
#endif	
		if ((priv->pmib->dot11WdsInfo.wdsEnabled) && (priv->pmib->dot11WdsInfo.wdsNum > 0) &&
			((priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_) ||
			 (priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_)))
			wds_psk_init(priv);

#if defined(WIFI_11N_2040_COEXIST_EXT)
		if (priv->pmib->dot11WdsInfo.wdsEnabled && priv->pshare->rf_ft_var.bws_enable)
			priv->pshare->rf_ft_var.bws_enable = 0;
#endif
	}
#endif
#endif

#ifdef WIFI_WPAS
	{
		WPAS_ASSOCIATION_INFO Assoc_Info;
		memset((void *)&Assoc_Info, 0, sizeof(struct _WPAS_ASSOCIATION_INFO));
		Assoc_Info.ReqIELen = priv->pmib->dot11RsnIE.rsnie[1]+ 2;
		memcpy(Assoc_Info.ReqIE, priv->pmib->dot11RsnIE.rsnie, Assoc_Info.ReqIELen);

		//event_indicate_wpas(priv, NULL, WPAS_ASSOC_INFO, (UINT8 *)&Assoc_Info);
	}
#endif 

#ifdef STA_CONTROL
    stactrl_init(priv);
#endif

#ifdef RTK_SMART_ROAMING
	smart_roaming_block_init(priv);
#endif

#ifdef CROSSBAND_REPEATER
	struct rtl8192cd_priv *crossband_priv_sc;
	crossband_priv_sc = GET_ROOT(priv)->crossband.crossband_rootpriv_sc;
	priv->crossband.crossband_priv_sc = crossband_priv_sc;

	priv->crossband.crossband_status = priv->pmib->crossBand.crossband_enable;
	priv->pmib->crossBand.crossband_prefer = 1;
	priv->pmib->crossBand.crossband_pathReady = 0;
	priv->pmib->crossBand.crossband_assoc = 0;
#endif

#ifdef CH_LOAD_CAL
#ifdef RTK_MESH_METRIC_REFINE
    if(priv->pmib->meshPathsel.mesh_crossbandEnable)
        priv->pmib->dot11StationConfigEntry.cu_enable = 1;
#endif
#ifdef CROSSBAND_REPEATER
    if(priv->crossband.crossband_status)
        priv->pmib->dot11StationConfigEntry.cu_enable = 1;
#endif
#endif

#ifdef GBWC
	priv->GBWC_tx_queue.head = 0;
	priv->GBWC_tx_queue.tail = 0;
	priv->GBWC_rx_queue.head = 0;
	priv->GBWC_rx_queue.tail = 0;
	priv->GBWC_tx_count = 0;
	priv->GBWC_rx_count = 0;
	priv->GBWC_consuming_Q = 0;
#endif

	priv->release_mcast = 0;

#ifdef USB_PKT_RATE_CTRL_SUPPORT //mark_test
	priv->change_toggle = 0;
	priv->pre_pkt_cnt = 0;
	priv->pkt_nsec_diff = 0;
	priv->poll_usb_cnt = 0;
	priv->auto_rate_mask = 0;
#endif


#ifdef CONFIG_RTK_VLAN_SUPPORT
	#if !defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if (priv->pmib->vlan.global_vlan)
		priv->pmib->dot11OperationEntry.disable_brsc = 1;
	#endif
#endif

#if defined(CONFIG_RTL_819X_ECOS)&&defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
	if (rtl_vlan_support_enable)
		priv->pmib->dot11OperationEntry.disable_brsc = 1;
#endif

#if 0//def CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		/*	set NMK	*/
		GenerateRandomData(priv->wapiNMK, WAPI_KEY_LEN);
		priv->wapiMCastKeyId = 0;
		priv->wapiMCastKeyUpdate = 0;
		priv->wapiWaiTxSeq = 0;
		wapiInit(priv);
	}

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable)  {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			if (priv->pvap_priv[i]->pmib->wapiInfo.wapiType!=wapiDisable) {
				/*  set NMK */
				GenerateRandomData(priv->pvap_priv[i]->wapiNMK, WAPI_KEY_LEN);
				priv->pvap_priv[i]->wapiMCastKeyId = 0;
				priv->pvap_priv[i]->wapiMCastKeyUpdate = 0;
				priv->wapiWaiTxSeq = 0;
				wapiInit(priv->pvap_priv[i]);
			}
	}
#endif
#endif

#ifdef MBSSID
	// if vap enabled, set beacon int to 100 at minimun when guest ssid num <= 4
    // if vap enabled, set beacon int to 200 at minimun when guest ssid num > 4
	{
		int ssid_num = 1, minbcn_period;
		priv->bcn_period_bak = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;

		if ((OPMODE & WIFI_AP_STATE) && GET_ROOT(priv)->pmib->miscEntry.vap_enable)
		{
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			{
				if (GET_ROOT(priv)->pvap_priv[i] && IS_DRV_OPEN(GET_ROOT(priv)->pvap_priv[i]))
				{
					ssid_num++;
				}
			}

#if defined(CONFIG_PCI_HCI)
			if (ssid_num >= 5)
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			if (ssid_num >= 4)
#endif
				minbcn_period = 200;
			else
				minbcn_period = 100;

			// if vap enabled, set beacon int to 100 at minimun
			if (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod < minbcn_period)
				priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = minbcn_period;

			for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			{
				if (GET_ROOT(priv)->pvap_priv[i])
				{
					GET_ROOT(priv)->pvap_priv[i]->pmib->dot11StationConfigEntry.dot11BeaconPeriod = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;
					GET_ROOT(priv)->pvap_priv[i]->update_bcn_period = 1;
				}
			}

			GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11BeaconPeriod = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;
			GET_ROOT(priv)->update_bcn_period = 1;
		}
	}
#endif

#ifdef DOT11H
    if ( priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
        priv->pmib->dot11hTPCEntry.tpc_enable = 0; /*2G do not have tpc, disable it*/
    }
#endif

#ifdef DOT11D
    if ( priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
        COUNTRY_CODE_ENABLED = 0; /*disable 11d  in 2G*/
    }
#endif

#if defined(DOT11D) || defined(DOT11H) || defined(DOT11K)
    check_country_channel_table(priv);    
#endif

#ifdef DOT11K
    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated) {
        init_timer(&priv->rm.delay_timer);
        priv->rm.delay_timer.data = (unsigned long) priv;
        priv->rm.delay_timer.function = rm_do_next_measure;
    }
#endif

#ifdef RTL_MANUAL_EDCA
	for (i=0; i<8; i++) {
		if ((priv->pmib->dot11QosEntry.TID_mapping[i] < 1) || (priv->pmib->dot11QosEntry.TID_mapping[i] > 4))
			priv->pmib->dot11QosEntry.TID_mapping[i] = 2;
	}
#endif

#if defined(TXREPORT) 
	priv->pshare->sta_query_idx=-1;
	// Init StaDetectInfo to detect disappearing STA. Added by Annie, 2010-08-10.
	priv->pmib->staDetectInfo.txRprDetectPeriod = 1;
#endif

#ifdef INCLUDE_WPS
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
		wps_init(priv);
#endif

#ifdef CONFIG_USB_HCI
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{	// The QUEUE-to-OUT Endpoint mapping must match _InitQueuePriority() settings
		_MappingOutEP(priv, priv->pshare->RtNumOutPipes, IS_TEST_CHIP(priv));
	}
	_init_txservq(&priv->tx_mc_queue, BE_QUEUE);
	_init_txservq(&priv->tx_mgnt_queue, MGNT_QUEUE);
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
#ifdef CONFIG_RTL_88E_SUPPORT
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		rtl8188es_interface_configure(priv);
	}
#endif
	_init_txservq(&priv->tx_mc_queue, BE_QUEUE);
	_init_txservq(&priv->tx_mgnt_queue, MGNT_QUEUE);
#endif // CONFIG_SDIO_HCI

#ifdef CONFIG_POWER_SAVE
	if (IS_ROOT_INTERFACE(priv)) {
		rtw_ap_ps_init(priv);
	}
#endif

#ifdef CONFIG_RTK_MESH
    if (IS_ROOT_INTERFACE(priv)) {
        init_mesh(priv);
    }
#endif

#ifdef TLN_STATS
	if (priv->pshare->rf_ft_var.stats_time_interval)
		priv->stats_time_countdown = priv->pshare->rf_ft_var.stats_time_interval;
#endif
	
#if defined (SUPPORT_TX_MCAST2UNI)
	/*ipv4 mdns*/
	priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[0]=0x01;
	priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[1]=0x00;
	priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[2]=0x5e;
	priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[3]=0x00;
	priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[4]=0x00;
	priv->pshare->rf_ft_var.mc2u_flood_mac[0].macAddr[5]=0xfb;

	/*ipv4 upnp&m-search*/
	priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[0]=0x01;
	priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[1]=0x00;
	priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[2]=0x5e;
	priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[3]=0x7f;
	priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[4]=0xff;
	priv->pshare->rf_ft_var.mc2u_flood_mac[1].macAddr[5]=0xfa;

	/*ipv6 mdns*/
	priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[0]=0x33;
	priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[1]=0x33;
	priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[2]=0x00;
	priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[3]=0x00;
	priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[4]=0x00;
	priv->pshare->rf_ft_var.mc2u_flood_mac[2].macAddr[5]=0xfb;

	/*ipv6 upnp&m-search*/
	priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[0]=0x33;
	priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[1]=0x33;
	priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[2]=0x7f;
	priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[3]=0xff;
	priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[4]=0xff;
	priv->pshare->rf_ft_var.mc2u_flood_mac[3].macAddr[5]=0xfa;
	
	priv->pshare->rf_ft_var.mc2u_flood_mac_num=4;
#endif

#ifdef _TRACKING_TABLE_FILE
	if (GET_CHIP_VER(priv) == VERSION_8812E)
		priv->pshare->rf_ft_var.pwr_track_file = 1;
#endif
#ifdef CONFIG_IEEE80211W
	if(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _NO_PRIVACY_ 
	||priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_){
		priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = 0;
		priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 0;
	}
#endif

#ifdef CONFIG_IEEE80211R
	if (FT_ENABLE) {
		unsigned char tmp[MDID_LEN];
		if (*(unsigned char *)MDID == 0) {	
			panic_printk("MDID is null !!, set default MDID\n"); 
			int retval = rtl_string_to_hex(DEFAULT_MDID, tmp, 4);
			if(retval == 0)
				printk(" rtl_string_to_hex() -> Weird! \n");

			memcpy(MDID, tmp, MDID_LEN);
		}
		
#ifdef SUPPORT_FAST_CONFIG
		unsigned char FTR0KeyHolderID[49] = {0};
		unsigned char macaddr[MACADDRLEN] = {0};
			
		if (*(unsigned char *)R0KH_ID == 0) {	
			memcpy(macaddr, GET_MY_HWADDR, MACADDRLEN);
			sprintf(FTR0KeyHolderID, "%02x%02x%02x%02x%02x%02x.%s",
				macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5], FT_R0KH_ID_STR);
			memcpy(R0KH_ID, FTR0KeyHolderID, strlen(FTR0KeyHolderID));
		}
#else		
		if (*(unsigned char *)R0KH_ID == 0)
			memcpy(R0KH_ID, FT_R0KH_ID_STR, strlen(FT_R0KH_ID_STR));	
#endif

		R0KH_ID_LEN = strlen(R0KH_ID);
		INIT_LIST_HEAD(&(priv->r0kh));
		INIT_LIST_HEAD(&(priv->r1kh));
	}
#endif

	priv->hiddenAP_backup = priv->pmib->dot11OperationEntry.hiddenAP;

#ifdef BT_COEXIST_HAL
	if(GET_CHIP_VER(priv) == VERSION_8197F)
		bt_coexist_hal_init(priv);
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A)
#ifdef CLIENT_MODE
	priv->pshare->ori_ther = priv->pmib->dot11RFEntry.ther ;
	priv->pshare->ori_ther2 = priv->pmib->dot11RFEntry.ther2 ;
	priv->pshare->ori_xcap = priv->pmib->dot11RFEntry.xcap;
	priv->pshare->ori_xcap2 = priv->pmib->dot11RFEntry.xcap2 ;
#endif
#endif
#if (PHYDM_TDMA_DIG_SUPPORT == 1)	
	if (IS_OUTSRC_CHIP(priv)) {
		IN PDM_ODM_T pDM_Odm = ODMPTR;
		if (!IS_TDMA_DIG_CHIP(priv))
			pDM_Odm->original_dig_restore = 1;			
	}
#endif	
	return 0;
}


static int rtl8192cd_stop_sw(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw *phw;
	unsigned long	flags;
	int	i;
	struct list_head	*phead, *plist;
	struct wlan_acl_node	*paclnode;
#ifdef CONFIG_WLAN_HAL
    int                         halQnum;
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
#endif // CONFIG_WLAN_HAL

	// we hope all this can be done in critical section
	SMP_LOCK(flags);
	SAVE_INT_AND_CLI(flags);

    if(OPMODE&WIFI_STATION_STATE){
        priv->site_survey->count = 0;
		priv->site_survey->count_target=0;
    }

#ifdef INCLUDE_WPS
   	priv->pshare->WSC_CONT_S.wait_reinit = 1 ;
#endif

	SMP_UNLOCK(flags);
	if (timer_pending(&priv->frag_to_filter))
	{
		del_timer_sync(&priv->frag_to_filter);
	}

#if defined(RTK_BR_EXT) && defined(EN_EFUSE)
#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	if (priv->pmib->ethBrExtInfo.macclone_enable && priv->macclone_completed 
		&& (priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE)
		&& (OPMODE & WIFI_STATION_STATE))
	{		
		ReadMacAddressFromEfuse(priv);
	}
#endif


#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if defined(CONFIG_TCP_ACK_TXAGG) || defined(CONFIG_XMITBUF_TXAGG_ADV)
		if (timer_pending(&priv->pshare->xmit_check_timer))
			del_timer_sync(&priv->pshare->xmit_check_timer);
#endif
#ifdef CONFIG_USB_HCI
#if defined(CONFIG_RTL_92C_SUPPORT) || (!defined(CONFIG_SUPPORT_USB_INT) || !defined(CONFIG_INTERRUPT_BASED_TXBCN))
		if (timer_pending(&priv->pshare->beacon_timer))
			del_timer_sync(&priv->pshare->beacon_timer);
#endif
#endif // CONFIG_USB_HCI
#ifdef CONFIG_SDIO_HCI
		if (timer_pending(&priv->pshare->beacon_timer))
			del_timer_sync(&priv->pshare->beacon_timer);
#endif
		if (timer_pending(&priv->expire_timer))
			del_timer_sync(&priv->expire_timer);
#ifdef 	SW_ANT_SWITCH
		if (timer_pending(&priv->pshare->swAntennaSwitchTimer))
			del_timer_sync(&priv->pshare->swAntennaSwitchTimer);
#endif		
		if (timer_pending(&priv->pshare->rc_sys_timer))
			del_timer_sync(&priv->pshare->rc_sys_timer);
		if (timer_pending(&priv->pshare->rc_sys_timer_cli))
			del_timer_sync(&priv->pshare->rc_sys_timer_cli);
#if 0
		if (timer_pending(&priv->pshare->phw->tpt_timer))
			del_timer_sync(&priv->pshare->phw->tpt_timer);
#endif

#ifdef USE_OUT_SRC
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
#ifdef SMART_REPEATER_MODE
		if (!priv->pshare->switch_chan_rp)
#endif
		ODM_StopAllThreads(ODMPTR);
	#ifdef _OUTSRC_COEXIST
		if(IS_OUTSRC_CHIP(priv))
	#endif
			ODM_CancelAllTimers(ODMPTR);
#else
	#ifdef TPT_THREAD
		kTPT_task_stop(priv);
	#endif
#endif // USE_OUT_SRC

#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (priv->pmib->wapiInfo.wapiType!=wapiDisable
#ifdef CHECK_HANGUP
			&&(!priv->reset_hangup)
#endif
#ifdef SMART_REPEATER_MODE
			&&(!priv->pshare->switch_chan_rp)
#endif
			)
		{
			wapiExit(priv);
		}


#ifdef MBSSID
	if( priv->pmib->miscEntry.vap_enable) {
	  for (i=0; i<RTL8192CD_NUM_VWLAN; i++){
		  if (priv->pvap_priv[i]->pmib->wapiInfo.wapiType!=wapiDisable
#ifdef CHECK_HANGUP
			&&(!priv->pvap_priv[i]->reset_hangup)
#endif
			) {
			  wapiExit(priv->pvap_priv[i]);
		  }
	  }
  }
#endif
#endif // CONFIG_RTL_WAPI_SUPPORT

#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);
#endif

#ifdef SDIO_AP_OFFLOAD
	if (timer_pending(&priv->pshare->ps_timer))
		del_timer_sync(&priv->pshare->ps_timer);
#endif
	
#ifdef CONFIG_RTK_MESH
		/*
		 * CAUTION !! These statement meshX(virtual interface) ONLY, Maybe modify....
		 */
		if (timer_pending(&priv->mesh_peer_link_timer))
			del_timer_sync(&priv->mesh_peer_link_timer);

#ifdef MESH_BOOTSEQ_AUTH
		if (timer_pending(&priv->mesh_auth_timer))
			del_timer_sync(&priv->mesh_auth_timer);
#endif
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(DPK_92D)
		if (GET_CHIP_VER(priv) == VERSION_8192D){
			if (timer_pending(&priv->pshare->DPKTimer))
				del_timer_sync(&priv->pshare->DPKTimer);
		}
#endif

#ifdef RTK_ATM
		if (priv->pshare->atm_swq_use_hw_timer == 0 && timer_pending(&priv->pshare->atm_swq_sw_timer)) {
            del_timer_sync(&priv->pshare->atm_swq_sw_timer);
        }
#endif 

#ifdef SW_TX_QUEUE
        if(priv->pshare->swq_use_hw_timer) {
            #ifndef __ECOS
            tasklet_kill(&priv->pshare->swq_tasklet);
            #endif
        }
        else {
            if (timer_pending(&priv->pshare->swq_sw_timer))
                del_timer_sync(&priv->pshare->swq_sw_timer);
        }  
#endif   

#if defined(MULTI_STA_REFINE)
		if (timer_pending(&priv->pshare->PktAging_timer))
			del_timer_sync(&priv->pshare->PktAging_timer);
#endif
	}

#ifdef RTK_NL80211 // wrt-adhoc
	if (IS_VXD_INTERFACE(priv)){
		//printk("Try delete ibss beacon timer!! \n");
		if (timer_pending(&priv->pshare->vxd_ibss_beacon)) 		
			del_timer_sync(&priv->pshare->vxd_ibss_beacon);
	}
#endif

	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);
	if (timer_pending(&priv->MIC_check_timer))
		del_timer_sync(&priv->MIC_check_timer);
	if (timer_pending(&priv->assoc_reject_timer))
		del_timer_sync(&priv->assoc_reject_timer);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	#ifdef CONFIG_PCI_HCI
	// to avoid add RAtid fail
	if (timer_pending(&priv->add_RATid_timer))
		del_timer_sync(&priv->add_RATid_timer);
	if (timer_pending(&priv->add_rssi_timer))
		del_timer_sync(&priv->add_rssi_timer);
	#endif
#endif
#ifdef CONFIG_PCI_HCI
	if (timer_pending(&priv->add_ps_timer))
		del_timer_sync(&priv->add_ps_timer);
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		if (timer_pending(&priv->dnc_timer))
			del_timer_sync(&priv->dnc_timer);
	}
#endif

#ifdef CLIENT_MODE
	if (timer_pending(&priv->reauth_timer))
		del_timer_sync(&priv->reauth_timer);
	if (timer_pending(&priv->reassoc_timer))
		del_timer_sync(&priv->reassoc_timer);
	if (timer_pending(&priv->idle_timer))
		del_timer_sync(&priv->idle_timer);
#endif


#ifdef DOT11K
    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated) {
        if (timer_pending(&priv->rm.delay_timer))
            del_timer_sync(&priv->rm.delay_timer);
    }
#endif

SMP_LOCK(flags);

#ifdef GBWC
	if (timer_pending(&priv->GBWC_timer))
		del_timer_sync(&priv->GBWC_timer);
	while (CIRC_CNT(priv->GBWC_tx_queue.head, priv->GBWC_tx_queue.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = priv->GBWC_tx_queue.pSkb[priv->GBWC_tx_queue.tail];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		priv->GBWC_tx_queue.tail++;
		priv->GBWC_tx_queue.tail = priv->GBWC_tx_queue.tail & (NUM_TXPKT_QUEUE - 1);
	}
	while (CIRC_CNT(priv->GBWC_rx_queue.head, priv->GBWC_rx_queue.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = priv->GBWC_rx_queue.pSkb[priv->GBWC_rx_queue.tail];
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
		priv->GBWC_rx_queue.tail++;
		priv->GBWC_rx_queue.tail = priv->GBWC_rx_queue.tail & (NUM_TXPKT_QUEUE - 1);
	}
#endif

#ifdef RTK_STA_BWC
	if (timer_pending(&priv->sta_bwc_timer))
		del_timer_sync(&priv->sta_bwc_timer);
#endif

#ifdef SBWC
	if (timer_pending(&priv->SBWC_timer))
		del_timer_sync(&priv->SBWC_timer);
#endif

#if defined(INCLUDE_WPA_PSK) && !defined(RTK_NL80211)
	if (timer_pending(&priv->wpa_global_info->GKRekeyTimer))
		del_timer_sync(&priv->wpa_global_info->GKRekeyTimer);
#endif

#ifdef USE_TXQUEUE
	for (i=BK_QUEUE; i<=VO_QUEUE; i++) {
		struct txq_node *pnode, *phead, *pnext;

		phead = (struct txq_node *)&(priv->pshare->txq_list[RSVQ(i)].list);
		pnode = phead->list.next;
		pnext = pnode;
		while (pnext != phead)
		{
			pnode = pnext;
			pnext = pnext->list.next;
			if (pnode->skb && pnode->dev && pnode->dev->priv == priv) {
				unlink_txq(&(priv->pshare->txq_list[RSVQ(i)]), pnode);
				rtl_kfree_skb(priv, pnode->skb, _SKB_TX_);
				pnode->skb = pnode->dev = NULL;
				list_add_tail(&pnode->list, &priv->pshare->txq_pool);
			}
		}
		priv->use_txq_cnt[RSVQ(i)] = 0;
	}

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
		free_txq_pool(&priv->pshare->txq_pool,priv->pshare->txq_pool_addr);
	}
#endif

#if (BEAMFORMING_SUPPORT == 1)
		Beamforming_Release(priv);
#endif


#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef DFS
		SMP_UNLOCK(flags);
		if (timer_pending(&priv->DFS_TXPAUSE_timer))
			del_timer_sync(&priv->DFS_TXPAUSE_timer);

		if (timer_pending(&priv->DFS_timer))
			del_timer_sync(&priv->DFS_timer);

		if (timer_pending(&priv->ch_avail_chk_timer))
			del_timer_sync(&priv->ch_avail_chk_timer);

		if (timer_pending(&priv->dfs_chk_timer))
			del_timer_sync(&priv->dfs_chk_timer);

		if (timer_pending(&priv->dfs_det_chk_timer))
			del_timer_sync(&priv->dfs_det_chk_timer);

		/*
		 *	when we disable the DFS function dynamically, we also remove the channel
		 *	from NOP_chnl while the driver is rebooting
		 */
		if (timer_pending(&priv->ch52_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch52_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 52);
		}

		if (timer_pending(&priv->ch56_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch56_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 56);
		}

		if (timer_pending(&priv->ch60_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch60_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 60);
		}

		if (timer_pending(&priv->ch64_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch64_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 64);
		}

		if (timer_pending(&priv->ch100_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch100_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 100);
		}

		if (timer_pending(&priv->ch104_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch104_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 104);
		}

		if (timer_pending(&priv->ch108_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch108_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 108);
		}

		if (timer_pending(&priv->ch112_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch112_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 112);
		}

		if (timer_pending(&priv->ch116_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch116_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 116);
		}

		if (timer_pending(&priv->ch120_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch120_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 120);
		}

		if (timer_pending(&priv->ch124_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch124_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 124);
		}

		if (timer_pending(&priv->ch128_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch128_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 128);
		}

		if (timer_pending(&priv->ch132_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch128_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 128);
		}

		if (timer_pending(&priv->ch136_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch136_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 136);
		}

		if (timer_pending(&priv->ch140_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch140_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 140);
		}

		if (timer_pending(&priv->ch144_timer) &&
			(priv->pmib->dot11DFSEntry.disable_DFS)) {
			del_timer_sync(&priv->ch144_timer);
			RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 144);
		}
		SMP_LOCK(flags);

		/*
		 *	For JAPAN in adhoc mode
		 */
		if (((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK)	||
			 (priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_MKK3)) &&
			 (OPMODE & WIFI_ADHOC_STATE)) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
				if (!timer_pending(&priv->ch52_timer))
					RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 52);
				if (!timer_pending(&priv->ch56_timer))
					RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 56);
				if (!timer_pending(&priv->ch60_timer))
					RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 60);
				if (!timer_pending(&priv->ch64_timer))
					RemoveChannel(priv, priv->NOP_chnl, &priv->NOP_chnl_num, 64);
			}
		}
#endif // DFS

		// for SW LED
		if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX))
			disable_sw_LED(priv);

#ifndef RTK_NL80211 //kill tasklets before stop hw
//#ifdef __KERNEL__
#ifndef __OSK__
#ifdef DFS
		/* prevent killing tasklet issue in interrupt */
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
#endif
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
				{
#ifdef SMART_REPEATER_MODE
					if (!priv->pshare->switch_chan_rp)
#endif
					{				
#ifdef CONFIG_RTL_SIMPLE_CONFIG
						if(!priv->pmib->dot11StationConfigEntry.sc_enabled && ((priv->simple_config_status==0) && (IS_ROOT_INTERFACE(priv) 
#ifdef UNIVERSAL_REPEATER
&& (GET_VXD_PRIV(priv)->simple_config_status == 0)
#endif
)))
#endif

						{


#if defined(CONFIG_PCI_HCI)
							RESTORE_INT(flags);
							SMP_UNLOCK(flags);
#ifdef __ECOS
							priv->pshare->rx_tasklet = 0;
							priv->pshare->tx_tasklet = 0;
							priv->pshare->has_triggered_rx_tasklet = 0;
							priv->pshare->has_triggered_tx_tasklet = 0;
#else
							tasklet_kill(&priv->pshare->rx_tasklet);
							tasklet_kill(&priv->pshare->tx_tasklet);				
							tasklet_kill(&priv->pshare->oneSec_tasklet);
#endif
							SMP_LOCK(flags);
							SAVE_INT_AND_CLI(flags);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef __ECOS
						priv->recvpriv.recv_tasklet = 0;
						priv->pshare->xmit_tasklet = 0;
						priv->pshare->has_triggered_rx_tasklet = 0;
						priv->pshare->has_triggered_tx_tasklet = 0;
#else
							tasklet_kill(&priv->recvpriv.recv_tasklet);
							tasklet_kill(&priv->pshare->xmit_tasklet);
#endif
#endif
						}
					}
				}
			}
		}
#endif //__OSK__
#endif //kill tasklets before stop hw

		if (priv->pptyIE
#ifdef CHECK_HANGUP
			&& !priv->reset_hangup
#endif
			) {
			if(priv->pptyIE->content) {
				kfree(priv->pptyIE->content);
				priv->pptyIE->content=NULL;
			}

			kfree(priv->pptyIE);
			priv->pptyIE=NULL;
		}

		phw = GET_HW(priv);

#ifdef DELAY_REFILL_RX_BUF
		priv->pshare->phw->cur_rx_refill = priv->pshare->phw->cur_rx = 0;       // avoid refill to rx ring
#if defined(CONFIG_WLAN_HAL)
		if (IS_HAL_CHIP(priv)) {
			PHCI_RX_DMA_MANAGER_88XX        prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
		    PHCI_RX_DMA_QUEUE_STRUCT_88XX	cur_q   = &(prx_dma->rx_queue[0]);
			cur_q->host_idx = cur_q->cur_host_idx = 0;
			cur_q->rxbd_ok_cnt = 0;
		}
#endif
#endif

#ifdef CONFIG_PCI_HCI
		for (i=0; i<NUM_RX_DESC_IF(priv); i++)
		{
			if (phw->rx_infoL[i].pbuf != NULL) {
#ifdef CONFIG_NET_PCI
				// if pci bios, then pci_unmap_single and dev_kfree_skb
				if (IS_PCIBIOS_TYPE)
					pci_unmap_single(priv->pshare->pdev, phw->rx_infoL[i].paddr, (RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);
#endif
				rtl_kfree_skb(priv, (struct sk_buff*)(phw->rx_infoL[i].pbuf), _SKB_RX_);
			}
		}

		// free the skb buffer in Low and Hi queue
		DEBUG_INFO("free tx Q0 head %d tail %d\n", phw->txhead0, phw->txtail0);
		DEBUG_INFO("free tx Q1 head %d tail %d\n", phw->txhead1, phw->txtail1);
		DEBUG_INFO("free tx Q2 head %d tail %d\n", phw->txhead2, phw->txtail2);
		DEBUG_INFO("free tx Q3 head %d tail %d\n", phw->txhead3, phw->txtail3);
		DEBUG_INFO("free tx Q4 head %d tail %d\n", phw->txhead4, phw->txtail4);
		DEBUG_INFO("free tx Q5 head %d tail %d\n", phw->txhead5, phw->txtail5);

		for (i=0; i<CURRENT_NUM_TX_DESC; i++)
		{
			// free tx queue skb
			struct tx_desc_info *tx_info;
			int j;
			int	head, tail;
			int max_qnum = HIGH_QUEUE;
#if defined(CONFIG_WLAN_HAL) && defined(MBSSID)
			if(IS_HAL_CHIP(priv) && GET_ROOT(priv)->pmib->miscEntry.vap_enable)
				max_qnum = HIGH_QUEUE7;
#endif		

			for (j=0; j<=max_qnum; j++) {
#ifdef CONFIG_WLAN_HAL
				if (IS_HAL_CHIP(priv)) {
	                halQnum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, j);
	                ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
                    #if 0
	                head    = GET_HAL_INTERFACE(priv)->GetTxQueueHWIdxHandler(priv, j);
                    #else
	                head    = ptx_dma->tx_queue[halQnum].host_idx;
                    #endif
	                tail    = ptx_dma->tx_queue[halQnum].hw_idx;
				} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif				
				{//not HAL
#ifdef OSK_LOW_TX_DESC
					if ((j!=BE_QUEUE && j!=HIGH_QUEUE) && i>NONBE_TXDESC)
						continue;
#endif
					head = get_txhead(phw, j);
					tail = get_txtail(phw, j);
				}
//				if (i <tail || i >= head)
				if( (tail <= head) ? (i <tail || i >= head) :(i <tail && i >= head))
					continue;

#ifdef CONFIG_WLAN_HAL
                if (IS_HAL_CHIP(priv)) {
                    if (j >= TXDESC_HIGH_QUEUE_NUM) {
                         if(i>=NUM_TX_DESC_HQ)
                            continue;                            
                       }
                }
#endif                


				tx_info = get_txdesc_info(priv->pshare->pdesc_info, j);

#ifdef RESERVE_TXDESC_FOR_EACH_IF
				if (RSVQ_ENABLE && IS_RSVQ(j)) {
					if (tx_info[i].priv)
						tx_info[i].priv->use_txdesc_cnt[RSVQ(j)]--;
				}
#endif
				
#if defined(CONFIG_WLAN_HAL)
                if (IS_HAL_CHIP(priv)) {
    				if (tx_info[i].buf_pframe[0] && (tx_info[i].buf_type[0] == _SKB_FRAME_TYPE_)) { // should be buf_paddr
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
    					if (IS_PCIBIOS_TYPE && (tx_info[i].buf_len[0]!=0 && tx_info[i].buf_paddr[0]!=0))
    						pci_unmap_single(priv->pshare->pdev, tx_info[i].buf_paddr[0],(tx_info[i].buf_len[0])&0xffff, PCI_DMA_TODEVICE);
#endif
#ifdef MP_TEST
    					if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
    						priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
    					}
    					else
#endif
    					{
    						rtl_kfree_skb(priv, tx_info[i].buf_pframe[0], _SKB_TX_);
    						DEBUG_INFO("free skb in queue %d\n", j);
    					}		
    				}
                } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // defined(CONFIG_WLAN_HAL)
                {//not HAL
    				if (tx_info[i].pframe && (tx_info[i].type == _SKB_FRAME_TYPE_)) {
#ifdef CONFIG_NET_PCI
    					if (IS_PCIBIOS_TYPE)
    						pci_unmap_single(priv->pshare->pdev, tx_info[i].paddr, (tx_info[i].len), PCI_DMA_TODEVICE);
#endif

#if 1//def CONFIG_RTL8672
#ifdef MP_TEST
    					if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
    						priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
    					}
    					else
#endif
    					{
    						rtl_kfree_skb(priv, tx_info[i].pframe, _SKB_TX_);
    						DEBUG_INFO("free skb in queue %d\n", j);
    					}
#else //CONFIG_RTL8672
    					rtl_kfree_skb(priv, tx_info[i].pframe, _SKB_TX_);
    					DEBUG_INFO("free skb in queue %d\n", j);
#endif //CONFIG_RTL8672
    				}
                }
			}
		} // TX descriptor Free


#if 1//def CONFIG_RTL8672
#ifdef MP_TEST
		if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
			DEBUG_INFO("[%s %d] skb_head/skb_tail=%d/%d\n",
					__FUNCTION__, __LINE__, priv->pshare->skb_head, priv->pshare->skb_tail);
			
			OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_BACKGROUND);
			
			for (i=0; i<NUM_MP_SKB; i++)
				kfree(priv->pshare->skb_pool[i]->head);
			kfree(priv->pshare->skb_pool_ptr);
		}
#endif
#endif

		// unmap  beacon buffer
#if defined(CONFIG_NET_PCI)
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
		    if (IS_PCIBIOS_TYPE) {
			    PHCI_TX_DMA_MANAGER_88XX ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
			    u32 halQNum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, BEACON_QUEUE);
			    PHCI_TX_DMA_QUEUE_STRUCT_88XX cur_q = &(ptx_dma->tx_queue[halQNum]);
			    PTX_BUFFER_DESCRIPTOR cur_txbd = cur_q->pTXBD_head;
			    
				#ifdef NOT_RTK_BSP
				if (cur_txbd->TXBD_ELE[1].Dword1 != 0)
				#endif	
					pci_unmap_single(priv->pshare->pdev, get_desc(cur_txbd->TXBD_ELE[1].Dword1), 128*sizeof(unsigned int), PCI_DMA_TODEVICE);
		    }
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{
			// Not HAL
		    if (IS_PCIBIOS_TYPE) {
#ifdef CONFIG_RTL_8812_SUPPORT
				if(GET_CHIP_VER(priv)== VERSION_8812E) {
					#ifdef NOT_RTK_BSP
					if (phw->tx_descB->Dword10 != 0)
					#endif	
						pci_unmap_single(priv->pshare->pdev, get_desc(phw->tx_descB->Dword10),
							128*sizeof(unsigned int), PCI_DMA_TODEVICE);
				} else
#endif
				{
					#ifdef NOT_RTK_BSP
					if (phw->tx_descB->Dword8 != 0)
					#endif	
						pci_unmap_single(priv->pshare->pdev, get_desc(phw->tx_descB->Dword8), 128*sizeof(unsigned int), PCI_DMA_TODEVICE);
				}
		    }
		}
#endif // defined(CONFIG_NET_PCI)
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef MP_TEST
		if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
			DEBUG_INFO("[%s %d] skb_head/skb_tail=%d/%d\n",
					__FUNCTION__, __LINE__, priv->pshare->skb_head, priv->pshare->skb_tail);
			
			OPMODE_VAL(OPMODE & ~WIFI_MP_CTX_BACKGROUND);
			
			for (i=0; i<NUM_MP_SKB; i++)
				kfree(priv->pshare->skb_pool[i]->head);
			kfree(priv->pshare->skb_pool_ptr);
		}
#endif
		rtw_flush_recvbuf_pending_queue(priv);
#ifdef CONFIG_SDIO_HCI
		rtw_flush_xmit_pending_queue(priv);
#endif
#endif

#ifdef RX_BUFFER_GATHER
		flush_rx_list(priv);
#endif
	} // if (IS_ROOT_INTERFACE(priv))

	priv->pmib->dot11StationConfigEntry.dot11AclNum=0;
	phead = &priv->wlan_acl_list;
	if (!list_empty(phead)) { 
		plist = phead->next;
		while(plist != phead)	{
			paclnode = list_entry(plist, struct wlan_acl_node, list);
			plist = plist->next;
			memcpy((void *)priv->pmib->dot11StationConfigEntry.dot11AclAddr[priv->pmib->dot11StationConfigEntry.dot11AclNum], (void *)paclnode->addr, MACADDRLEN);
			priv->pmib->dot11StationConfigEntry.dot11AclNum++;
		};
	}
	
	for (i=0; i<NUM_STAT; i++)
	{
		if (priv->pshare->aidarray[i]) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (priv != priv->pshare->aidarray[i]->priv){
				continue;
			} else
#endif
			{
				if (priv->pshare->aidarray[i]->used == TRUE) {
					if (priv->pshare->aidarray[i]->station.expire_to > 0)
					{
						cnt_assoc_num(priv, &(priv->pshare->aidarray[i]->station), DECREASE, (char *)__FUNCTION__);
						check_sta_characteristic(priv, &(priv->pshare->aidarray[i]->station), DECREASE);
					}					
					if (free_stainfo(priv, &(priv->pshare->aidarray[i]->station)) == FAIL)
					DEBUG_ERR("free station %d fails\n", i);
				}

				
#ifdef CONFIG_PCI_HCI
#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifdef PRIV_STA_BUF
				free_sta_que(priv, priv->pshare->aidarray[i]->station.VO_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.VI_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.BE_dz_queue);
				free_sta_que(priv, priv->pshare->aidarray[i]->station.BK_dz_queue);

#else
				kfree(priv->pshare->aidarray[i]->station.VO_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.VI_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BE_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BK_dz_queue);
#endif
#endif

#if defined(WIFI_WMM)
#ifdef PRIV_STA_BUF
				free_sta_mgt_que(priv, priv->pshare->aidarray[i]->station.MGT_dz_queue);
#else
				kfree(priv->pshare->aidarray[i]->station.MGT_dz_queue);
#endif
#endif
#endif // CONFIG_PCI_HCI

#ifdef INCLUDE_WPA_PSK
#ifdef PRIV_STA_BUF
				free_wpa_buf(priv, priv->pshare->aidarray[i]->station.wpa_sta_info);
#else
				kfree(priv->pshare->aidarray[i]->station.wpa_sta_info);
#endif
#endif
#ifdef RTL8192CD_VARIABLE_USED_DMEM
			{
				unsigned int index = (unsigned int)i;
				rtl8192cd_dmem_free(AID_OBJ, &index);
			}
#else
#ifdef PRIV_STA_BUF
				free_sta_obj(priv, priv->pshare->aidarray[i]);
#else
				kfree(priv->pshare->aidarray[i]);
#endif
#endif
				priv->pshare->aidarray[i] = NULL;
			}
		}
	}
	
#ifdef CONFIG_USB_HCI
	rtw_flush_rx_mgt_queue(priv);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	rtw_flush_h2c_cmd_queue(priv, NULL);
#endif
	rtw_flush_cmd_queue(priv, NULL);
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
	rtw_flush_rx_mgt_queue(priv);
	rtw_flush_cmd_queue(priv, NULL);
#endif

#ifndef __DRAYTEK_OS__
#ifndef RTK_NL80211 //wrt-wps
	// reset rsnie and group key from open to here, david
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef WIFI_SIMPLE_CONFIG
		if (!priv->pmib->dot11OperationEntry.keep_rsnie) {
			priv->pmib->wscEntry.beacon_ielen = 0;
			priv->pmib->wscEntry.probe_rsp_ielen = 0;
			priv->pmib->wscEntry.probe_req_ielen = 0;
			priv->pmib->wscEntry.assoc_ielen = 0;
		}

//		if (!(OPMODE & WIFI_AP_STATE))
//			priv->pmib->dot11OperationEntry.keep_rsnie = 1;
#endif
	}
#endif

	if (!priv->pmib->dot11OperationEntry.keep_rsnie) {
		priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
		memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
#ifdef UNIVERSAL_REPEATER
		if (GET_VXD_PRIV(priv))
			GET_VXD_PRIV(priv)->pmib->dot11RsnIE.rsnielen = 0;
#endif
		priv->auto_channel_backup = 0;
	}
	else {
		// When wlan scheduling and auto-chan case, it will disable/enable
		// wlan interface directly w/o re-set mib. Therefore, we need use
		// "keep_rsnie" flag to keep auto-chan value

		if (
#ifdef CHECK_HANGUP
			!priv->reset_hangup &&
#endif
#ifdef SMART_REPEATER_MODE
			!priv->pshare->switch_chan_rp &&
#endif
			priv->auto_channel_backup)
			priv->pmib->dot11RFEntry.dot11channel = 0;
	}
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv) && !priv->pmib->dot11OperationEntry.keep_rsnie) {
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
			memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
		}
	}

    // mark by Pedro: because priv->bcn_period_bak is zero currently.
#if !defined(RTK_NL80211)
#ifdef  CONFIG_WLAN_HAL
	if (!IS_HAL_CHIP(priv))
#endif		
	priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = priv->bcn_period_bak;
#endif
#endif

#ifdef RTK_BR_EXT
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
			nat25_db_cleanup(priv);
	}
#endif

#ifdef A4_STA
#ifdef CHECK_HANGUP
    if (!priv->reset_hangup)
#endif
        a4_sta_cleanup_all(priv);
#endif

#ifdef STA_CONTROL
    #ifdef CHECK_HANGUP
    if (!priv->reset_hangup)
    #endif
    {
        stactrl_deinit(priv);
    }
#endif

#ifdef RTK_SMART_ROAMING
	smart_roaming_block_deinit(priv);
#endif

#ifdef CONFIG_PCI_HCI
	{
		int				hd, tl;
		struct sk_buff	*pskb;

		hd = priv->dz_queue.head;
		tl = priv->dz_queue.tail;
		while (CIRC_CNT(hd, tl, NUM_TXPKT_QUEUE))
		{
			pskb = priv->dz_queue.pSkb[tl];
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
			tl++;
			tl = tl & (NUM_TXPKT_QUEUE - 1);
		}
		priv->dz_queue.head = 0;
		priv->dz_queue.tail = 0;
	}
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
	// release tx_mc_queue
	rtw_txservq_flush(priv, &priv->tx_mc_queue);
	_rtw_spinlock_free(&priv->tx_mc_queue.xframe_queue.lock);
	
	// release tx_mgnt_queue
	rtw_txservq_flush(priv, &priv->tx_mgnt_queue);
	_rtw_spinlock_free(&priv->tx_mgnt_queue.xframe_queue.lock);
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
	// release tx_mc_queue
	rtw_txservq_flush(priv, &priv->tx_mc_queue);
	_rtw_spinlock_free(&priv->tx_mc_queue.xframe_queue.lock);
	
	// release tx_mgnt_queue
	rtw_txservq_flush(priv, &priv->tx_mgnt_queue);
	_rtw_spinlock_free(&priv->tx_mgnt_queue.xframe_queue.lock);

#ifdef CONFIG_POWER_SAVE
	if(IS_ROOT_INTERFACE(priv))
		rtw_ap_ps_deinit(priv);
#endif

#endif // CONFIG_SDIO_HCI

#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
	flush_rx_queue(priv);
#endif

#ifdef CONFIG_IEEE80211R
	if (FT_ENABLE) {
		free_r0kh(priv);
		free_r1kh(priv);
	}
#endif

RESTORE_INT(flags);
	SMP_UNLOCK(flags);

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	priv->ap_ready = 0;
	_rtw_mutex_free(&(priv->hash_list_lock));
	_rtw_mutex_free(&(priv->asoc_list_lock));
	_rtw_mutex_free(&(priv->auth_list_lock));
	_rtw_mutex_free(&(priv->sleep_list_lock));
	_rtw_mutex_free(&(priv->wakeup_list_lock));
	_rtw_mutex_free(&(priv->rc_packet_q_lock));
#endif

#ifdef WIFI_SIMPLE_CONFIG
	wsc_disconn_list_flush(priv);
#endif

#ifdef BT_COEXIST_HAL
	if(GET_CHIP_VER(priv) == VERSION_8197F)
		bt_coexist_hal_close(priv);
#endif

	return 0;
}


#ifdef RTK_NL80211
#ifdef UNIVERSAL_REPEATER
static void rtl8192cd_init_vxd_mib(struct rtl8192cd_priv *priv)
{
	// copy mib_rf from root interface
	memcpy(&priv->pmib->dot11RFEntry, &GET_ROOT(priv)->pmib->dot11RFEntry, sizeof(struct Dot11RFEntry));
}
#endif
#endif

#ifdef MBSSID
static void rtl8192cd_init_vap_mib(struct rtl8192cd_priv *priv)
{
#if 0
	unsigned char tmpbuf[36], hwaddr[6];
	int len;
	//unsigned int AclMode;
	//unsigned char AclAddr[NUM_ACL][MACADDRLEN];
	//unsigned int AclNum;
	struct Dot1180211AuthEntry dot1180211AuthEntry;
	struct Dot118021xAuthEntry dot118021xAuthEntry;
	struct Dot11DefaultKeysTable dot11DefaultKeysTable;
	struct Dot11RsnIE dot11RsnIE;

	// backup mib that can differ from root interface
	memcpy(hwaddr, GET_MY_HWADDR, 6);
	len = SSID_LEN;
	memcpy(tmpbuf, SSID, len);
	//AclMode = priv->pmib->dot11StationConfigEntry.dot11AclMode;
	//memcpy(AclAddr, priv->pmib->dot11StationConfigEntry.dot11AclAddr, sizeof(AclAddr));
	//AclNum = priv->pmib->dot11StationConfigEntry.dot11AclNum;
	memcpy(&dot1180211AuthEntry, &priv->pmib->dot1180211AuthEntry, sizeof(struct Dot1180211AuthEntry));
	memcpy(&dot118021xAuthEntry, &priv->pmib->dot118021xAuthEntry, sizeof(struct Dot118021xAuthEntry));
	memcpy(&dot11DefaultKeysTable, &priv->pmib->dot11DefaultKeysTable, sizeof(struct Dot11DefaultKeysTable));
	memcpy(&dot11RsnIE, &priv->pmib->dot11RsnIE, sizeof(struct Dot11RsnIE));

	// copy mib from root interface
	memcpy(priv->pmib, GET_ROOT_PRIV(priv)->pmib, sizeof(struct wifi_mib));

	// restore the different part
	memcpy(GET_MY_HWADDR, hwaddr, 6);
	SSID_LEN = len;
	memcpy(SSID, tmpbuf, len);
	SSID2SCAN_LEN = len;
	memcpy(SSID2SCAN, SSID, len);
	//priv->pmib->dot11StationConfigEntry.dot11AclMode = AclMode;
	//memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr, AclAddr, sizeof(AclAddr));
	//priv->pmib->dot11StationConfigEntry.dot11AclNum = AclNum;
	memcpy(&priv->pmib->dot1180211AuthEntry, &dot1180211AuthEntry, sizeof(struct Dot1180211AuthEntry));
	memcpy(&priv->pmib->dot118021xAuthEntry, &dot118021xAuthEntry, sizeof(struct Dot118021xAuthEntry));
	memcpy(&priv->pmib->dot11DefaultKeysTable, &dot11DefaultKeysTable, sizeof(struct Dot11DefaultKeysTable));
	memcpy(&priv->pmib->dot11RsnIE, &dot11RsnIE, sizeof(struct Dot11RsnIE));
#endif

#ifdef NOT_RTK_BSP
	unsigned int shortpreamble = priv->pmib->dot11RFEntry.shortpreamble;
#endif

	// copy mib_rf from root interface
	memcpy(&priv->pmib->dot11RFEntry, &GET_ROOT(priv)->pmib->dot11RFEntry, sizeof(struct Dot11RFEntry));

#ifdef NOT_RTK_BSP
	priv->pmib->dot11RFEntry.shortpreamble = shortpreamble;
#endif

	// special mib that need to set
#ifdef WIFI_WMM
	//QOS_ENABLE = 0;
#ifdef WMM_APSD
	APSD_ENABLE = 0;
#endif
#endif

#ifdef WDS
	// always disable wds in vap
	priv->pmib->dot11WdsInfo.wdsEnabled = 0;
	priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif
#ifdef CONFIG_RTK_MESH
	// in current release, mesh can be only run upon wlan0, so we disable the following flag in vap
	priv->pmib->dot1180211sInfo.mesh_enable = 0;
#endif

#ifdef __OSK__
    priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nShortGIfor20M;
    priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nShortGIfor40M;
    priv->pmib->dot11nConfigEntry.dot11nAMPDU = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nAMPDU;
#endif
}

#if defined(CONFIG_PCI_HCI) || defined(__ECOS)
#if(CONFIG_WLAN_NOT_HAL_EXIST==1)

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)//eric_8812 ??
static void rtl8192cd_set_DTIM(struct rtl8192cd_priv *priv)
{
//	printk("init swq=%d\n", priv->vap_init_seq);
	if(GET_CHIP_VER(priv)==VERSION_8192E|| GET_CHIP_VER(priv)==VERSION_8812E)
	{
		switch (priv->vap_init_seq)
		{
			case 1:
				RTL_W16(REG_92E_ATIMWND1, 0x03); //0x3C);		
				RTL_W8(REG_92E_DTIM_COUNT_VAP1, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 2:
				RTL_W8(REG_92E_ATIMWND2, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP2, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 3:
				RTL_W8(REG_92E_ATIMWND3, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP3, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 4:
				RTL_W8(REG_92E_ATIMWND4, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP4, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 5:
				RTL_W8(REG_92E_ATIMWND5, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP5, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 6:
				RTL_W8(REG_92E_ATIMWND6, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP6, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
			case 7:
				RTL_W8(REG_92E_ATIMWND7, 0x3C);
				RTL_W8(REG_92E_DTIM_COUNT_VAP7, priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod);
				break;
		}
	}
}
#endif

static void rtl8192cd_init_mbssid(struct rtl8192cd_priv *priv)
{
	int i, j;
	unsigned int camData[2];
	unsigned char *macAddr = GET_MY_HWADDR;

	if (IS_ROOT_INTERFACE(priv))
	{
		//camData[0] = 0x00800000 | (macAddr[5] << 8) | macAddr[4];
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID | (macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
//		for (j=0; j<2; j++) {
		for (j=1; j>=0; j--) {
			//RTL_W32((_MBIDCAMCONTENT_+4)-4*j, camData[j]);
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
		//RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6));
#if 0
		// clear the rest area of CAM
		//camData[0] = 0;
		camData[1] = 0;
		for (i=1; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
//			for (j=0; j<2; j++) {
			for (j=1; j>=0; j--) {
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
			}
//			RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | (unsigned char)i);
		}
#endif
		// set MBIDCTRL & MBID_BCN_SPACE by cmd
//		set_fw_reg(priv, 0xf1000101, 0, 0);
		RTL_W32(MBSSID_BCN_SPACE,
			(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);

#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION |EN_MBSSID| DIS_SUB_STATE | DIS_TSF_UPDATE|EN_TXBCN_RPT);
		else
#endif
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable
/*
#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_STATION_STATE) || (OPMODE & WIFI_ADHOC_STATE))
			RTL_W32(RCR, RTL_R32(RCR) | RCR_CBSSID);
#endif
*/
		priv->vap_count = 0;//RTK_ATM
	}
	else if (IS_VAP_INTERFACE(priv))
	{
//		priv->vap_init_seq = (RTL_R8(_MBIDCTRL_) & (BIT(4) | BIT(5) | BIT(6))) >> 4;
//		priv->vap_init_seq++;
//		set_fw_reg(priv, 0xf1000001 | ((priv->vap_init_seq + 1)&0xffff)<<8, 0, 0);

		priv->vap_init_seq = RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask;
		priv->vap_init_seq++;
		GET_ROOT(priv)->vap_count++;//RTK_ATM

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)//eric_8812 ??
		rtl8192cd_set_DTIM(priv);
#endif
		
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
				(priv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
				(macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
		for (j=1; j>=0; j--) {
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
//		RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | ((unsigned char)priv->vap_init_seq & 0x1f));
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			unsigned int vap_bcn_offset = (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1))*priv->vap_init_seq)-1)
				& BCN_SPACE2_Mask;

			if (vap_bcn_offset > 200)
				vap_bcn_offset = 200;
			RTL_W32(MBSSID_BCN_SPACE, (vap_bcn_offset & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
		} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8723B)) {
			unsigned int vap_bcn_offset = (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1))*priv->vap_init_seq)-1)
				& BCN_SPACE2_Mask;

			if (vap_bcn_offset > 200)
				vap_bcn_offset = 200;
			RTL_W32(MBSSID_BCN_SPACE, (vap_bcn_offset & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
		} else
#endif
		{
			RTL_W32(MBSSID_BCN_SPACE,
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1))*priv->vap_init_seq))
				& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(priv->vap_init_seq+1)) & BCN_SPACE1_Mask)
				<<BCN_SPACE1_SHIFT);
		}
		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);

#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION |EN_MBSSID| DIS_SUB_STATE | DIS_TSF_UPDATE|EN_TXBCN_RPT);
		else
#endif
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)//eric_8812 ??
		if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8723B))
		RTL_W8(MBID_NUM, (RTL_R8(MBID_NUM) & ~MBID_BCN_NUM_Mask) | (priv->vap_init_seq & MBID_BCN_NUM_Mask));
		else
#endif
		RTL_W8(MBID_NUM, priv->vap_init_seq & MBID_BCN_NUM_Mask);
		
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable
	}
}

static void rtl8192cd_stop_mbssid(struct rtl8192cd_priv *priv)
{
	int i, j;
	unsigned int camData[2];
	camData[1] = 0;

	if (IS_ROOT_INTERFACE(priv))
	{
		// clear the rest area of CAM
		for (i=0; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
			for (j=1; j>=0; j--) {
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
			}
//			RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | (unsigned char)i);
		}

//		set_fw_reg(priv, 0xf1000001, 0, 0);
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);	// MBSSID disable
		RTL_W32(MBSSID_BCN_SPACE,
			(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);
#ifdef CONFIG_RTL_92C_SUPPORT
		if (IS_TEST_CHIP(priv))
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
		else
#endif
			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

	}
	else if (IS_VAP_INTERFACE(priv) && (priv->vap_init_seq >= 0))
	{
		GET_ROOT(priv)->vap_count--;//RTK_ATM
	
		struct rtl8192cd_priv *tmpPriv;
		unsigned char *macAddr;
//		set_fw_reg(priv, 0xf1000001 | (((RTL_R8(_MBIDCTRL_) & (BIT(4) | BIT(5) | BIT(6))) >> 4)&0xffff)<<8, 0, 0);
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN |
			(priv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
		for (j=1; j>=0; j--) {
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
//		RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | ((unsigned char)priv->vap_init_seq & 0x1f));

		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			tmpPriv = GET_ROOT(priv)->pvap_priv[i];
			if ((tmpPriv) && (tmpPriv->vap_init_seq > priv->vap_init_seq)) {
				camData[0] = MBIDCAM_POLL | MBIDWRITE_EN |
					(tmpPriv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
				for (j=1; j>=0; j--) {
					RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
				}
			}
		}

		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			tmpPriv = GET_ROOT(priv)->pvap_priv[i];
			if ((tmpPriv) && (tmpPriv->vap_init_seq > priv->vap_init_seq)) {
				struct rtl8192cd_hw	*phw=GET_HW(tmpPriv);
				struct tx_desc		*pdesc;
				extern void fill_bcn_desc(struct rtl8192cd_priv *priv, struct tx_desc *pdesc, void *dat_content, unsigned short txLength, char forceUpdate);

				macAddr = ((GET_MIB(tmpPriv))->dot11OperationEntry.hwaddr);
				tmpPriv->vap_init_seq -= 1;		

				camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
					(tmpPriv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
					(macAddr[5] << 8) | macAddr[4];
				camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
				for (j=1; j>=0; j--) {
					RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
				}
			
				pdesc = phw->tx_descB + tmpPriv->vap_init_seq;
				fill_bcn_desc(tmpPriv, pdesc, (void*)tmpPriv->beaconbuf, tmpPriv->tx_beacon_len, 1);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)//eric_8812 ??
				rtl8192cd_set_DTIM(tmpPriv);
#endif
			}
		}

		if (RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask) {
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)//eric_8812 ??
			if ((GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8188E) || (GET_CHIP_VER(priv)==VERSION_8723B))
				RTL_W8(MBID_NUM, ((RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask)-1) & MBID_BCN_NUM_Mask);
			else
#endif			
			RTL_W8(MBID_NUM, (RTL_R8(MBID_NUM)-1) & MBID_BCN_NUM_Mask);
			
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
				unsigned int vap_bcn_offset = (priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
					((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1))
					*RTL_R8(MBID_NUM))-1) & BCN_SPACE2_Mask;

				if (vap_bcn_offset > 200)
					vap_bcn_offset = 200;
				RTL_W32(MBSSID_BCN_SPACE, (vap_bcn_offset & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT|
					(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
			} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)//eric_8812 ??
			if ((GET_CHIP_VER(priv)==VERSION_8192E) || (GET_CHIP_VER(priv)==VERSION_8812E) || (GET_CHIP_VER(priv)==VERSION_8723B)) {
				RTL_W32(MBSSID_BCN_SPACE,
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/((RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask)+1))*(RTL_R8(MBID_NUM)&MBID_BCN_NUM_Mask)))
				& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod) & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
			} else
#endif
			{
				RTL_W32(MBSSID_BCN_SPACE,
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod-
				((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1))*RTL_R8(MBID_NUM)))
				& BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
				|((priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod/(RTL_R8(MBID_NUM)+1)) & BCN_SPACE1_Mask)
				<<BCN_SPACE1_SHIFT);
			}

			RTL_W8(BCN_CTRL, 0);
			RTL_W8(0x553, 1);
#ifdef CONFIG_RTL_92C_SUPPORT
			if (IS_TEST_CHIP(priv))
				RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
			else
#endif
				RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

		}
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);
		priv->vap_init_seq = -1;
	}
}

void rtl8192cd_set_mbssid(struct rtl8192cd_priv *priv, unsigned char *macAddr, unsigned char index)
{
	int i, j;
	unsigned int camData[2];

	if (index > 2)
		return;
	camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
				((7-index)&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
					(macAddr[5] << 8) | macAddr[4];
	camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
	for (j=1; j>=0; j--) 
		RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
}

void rtl8192cd_clear_mbssid(struct rtl8192cd_priv *priv, unsigned char index)
{
	int i, j;
	unsigned int camData[2];

	if (index > 1)
		return;
	camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID | ((7-index)&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
	camData[1] = 0;
	for (j=1; j>=0; j--) 
		RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
}
#else // !CONFIG_WLAN_NOT_HAL_EXIST
static void rtl8192cd_init_mbssid(struct rtl8192cd_priv *priv)
{
	return;
}
void rtl8192cd_set_mbssid(struct rtl8192cd_priv *priv, unsigned char *macAddr, unsigned char index)
{
	return;
}
void rtl8192cd_clear_mbssid(struct rtl8192cd_priv *priv, unsigned char index)
{
	return;
}

static void rtl8192cd_stop_mbssid(struct rtl8192cd_priv *priv)
{
}
#endif // CONFIG_WLAN_NOT_HAL_EXIST
#endif // CONFIG_PCI_HCI || __ECOS
#endif //MBSSID


#ifdef MULTI_MAC_CLONE
 void mclone_set_mbssid(struct rtl8192cd_priv *priv, unsigned char *macAddr)
{
	int i, j;
	unsigned int camData[2];

	// for debug
	DEBUG_INFO("%s, %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, 
				macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);

	//wlan0
	if (!memcmp(macAddr, (GET_MIB(priv))->dot11OperationEntry.hwaddr, ETH_ALEN)) {
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID | (macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];		
		for (j=1; j>=0; j--) 
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);

		// clear the rest area of CAM				
		camData[1] = 0;
		for (i=1; i<8; i++) {
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
			for (j=1; j>=0; j--) 
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
		
		// set MBIDCTRL & MBID_BCN_SPACE by cmd
		RTL_W32(MBSSID_BCN_SPACE,
			(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);
		
		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);
		
		RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable

	}
	else//otehr mac from client
	{
		priv->pshare->mclone_init_seq  = RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask;
		priv->pshare->mclone_init_seq++;		
			
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
					(priv->pshare->mclone_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
						(macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
		for (j=1; j>=0; j--) 
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);

		RTL_W32(MBSSID_BCN_SPACE,
			(priv->beacon_period & BCN_SPACE2_Mask)<<BCN_SPACE2_SHIFT
			|(0 & BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

		RTL_W8(BCN_CTRL, 0);
		RTL_W8(0x553, 1);

		RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);
		
		PHY_SetBBReg(priv, BCN_CTRL, 0x70000, priv->pshare->mclone_init_seq & MBID_BCN_NUM_Mask);	//MBID_NUM: 0x552[2:0]

		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);	// MBSSID enable
	}
}


void mclone_stop_mbssid(struct rtl8192cd_priv *priv, int entIdx)
{
	int i, j, k;
	unsigned int camData[2];
	camData[1] = 0;

	if (MCLONE_NUM <= 0)
		return;

	// clear the rest area of CAM
	for (i=0; i<8; i++) {
		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT;
		for (j=1; j>=0; j--) 
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
	}
	RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);	// MBSSID disable
	RTL_W32(MBSSID_BCN_SPACE,
			(priv->beacon_period& BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);

	RTL_W8(BCN_CTRL, 0);
	RTL_W8(0x553, 1);
	if (IS_TEST_CHIP(priv))
		RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
	else
		RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);

	if (MCLONE_NUM > 0 && priv->pshare->mclone_init_seq >= 0) 
	{
		for (k=0; k<MCLONE_NUM; k++, priv->pshare->mclone_init_seq--) 
		{	
			camData[0] = MBIDCAM_POLL | MBIDWRITE_EN |
				(priv->pshare->mclone_init_seq & MBIDCAM_ADDR_Mask) <<MBIDCAM_ADDR_SHIFT;
			for (j=1; j>=0; j--) 
				RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);		

			if (RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask) 
			{
				RTL_W8(MBID_NUM, (RTL_R8(MBID_NUM)-1) & MBID_BCN_NUM_Mask);
				RTL_W32(MBSSID_BCN_SPACE, (priv->beacon_period& BCN_SPACE1_Mask)<<BCN_SPACE1_SHIFT);		

				PHY_SetBBReg(priv, BCN_CTRL, 0x70000, (RTL_R8(MBID_NUM)-1) & MBID_BCN_NUM_Mask);				
				
				RTL_W8(BCN_CTRL, 0);
				RTL_W8(0x553, 1);
				
				if (IS_TEST_CHIP(priv))
					RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE | DIS_TSF_UPDATE| EN_TXBCN_RPT);
				else
					RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N| EN_TXBCN_RPT);
	
			}
		}
		RTL_W32(RCR, RTL_R32(RCR) & ~RCR_MBID_EN);
		RTL_W32(RCR, RTL_R32(RCR) | RCR_MBID_EN);
		priv->pshare->mclone_init_seq = -1;
	}
}
#endif /* MULTI_MAC_CLONE */


#ifdef WDS
#ifdef LAZY_WDS
void delete_wds_entry(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i;
	
	for (i=0; i<NUM_WDS; i++) {
		if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr, pstat->hwaddr, MACADDRLEN)) {
			memcpy(priv->pmib->dot11WdsInfo.entry[i].macAddr, 
							NULL_MAC_ADDR, MACADDRLEN);			
			free_stainfo(priv, pstat);				
			priv->pmib->dot11WdsInfo.wdsNum--;			
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv) == VERSION_8188E) 
				priv->pshare->total_assoc_num--;
#endif											
			break;
		}	
	}
}
#endif

struct stat_info *add_wds_entry(struct rtl8192cd_priv *priv, int idx, unsigned char *mac)
{
	struct stat_info *pstat;
	DOT11_SET_KEY Set_Key;
#ifdef LAZY_WDS
	int i;
	
	if (mac != NULL) {
		for (i=0; i<NUM_WDS; i++) {
			if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr, 
						NULL_MAC_ADDR, MACADDRLEN)) {		
				memcpy(	priv->pmib->dot11WdsInfo.entry[i].macAddr, mac, MACADDRLEN);
				idx = i;				
				priv->pmib->dot11WdsInfo.wdsNum++;					
				break;
			}
		}	
		if (i == NUM_WDS) {
			DEBUG_ERR("WDS table is full!!!\n");
			return NULL;	
		}
	}
#endif

	pstat = alloc_stainfo(priv, priv->pmib->dot11WdsInfo.entry[idx].macAddr, -1);
	if (pstat == NULL) {
		DEBUG_ERR("alloc_stainfo() fail!\n");
		return NULL;
	}


	
	// use self supported rate for wds
	memcpy(pstat->bssrateset, AP_BSSRATE, AP_BSSRATE_LEN);
	pstat->bssratelen = AP_BSSRATE_LEN;
	pstat->state = WIFI_WDS;
	
	pstat->wds_probe_done=0;
	pstat->state &= ~WIFI_WDS_RX_BEACON;
	
	//WDEBUG("priv->pmib->dot11WdsInfo.entry[%d].txRate=%04x\n\n",idx , priv->pmib->dot11WdsInfo.entry[idx].txRate);
	
	if (  priv->pmib->dot11WdsInfo.entry[idx].txRate & BIT31 ) {
		//WDEBUG("under AC data rate\n");
	} else {
		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0x0000fff;	// mask HT rates

		if (!(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G | WIRELESS_11A)))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0xffff00f;	// mask OFDM rates

		if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))
			priv->pmib->dot11WdsInfo.entry[idx].txRate &= 0xffffff0;	// mask CCK rates
	}

	//WDEBUG("priv->pmib->dot11WdsInfo.entry[%d].txRate=%04x\n\n", idx , priv->pmib->dot11WdsInfo.entry[idx].txRate);

#ifdef LAZY_WDS
	if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE)
		pstat->state |= WIFI_WDS_LAZY;		

	if ((priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) &&
		(priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
			priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_))
		wds_psk_set(priv, idx, NULL);
#endif

			if (priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_40_PRIVACY_ ||
				priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_104_PRIVACY_ ) {
				memcpy(Set_Key.MACAddr, priv->pmib->dot11WdsInfo.entry[idx].macAddr, 6);
				Set_Key.KeyType = DOT11_KeyType_Pairwise;
				Set_Key.EncType = priv->pmib->dot11WdsInfo.wdsPrivacy;
				Set_Key.KeyIndex = priv->pmib->dot11WdsInfo.wdsKeyId;
				DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, priv->pmib->dot11WdsInfo.wdsWepKey);
			}
			else if ((priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_ ||
						priv->pmib->dot11WdsInfo.wdsPrivacy == _CCMP_PRIVACY_) &&
					(priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx]&0x80000000) ) {
		priv->pmib->dot11WdsInfo.wdsMappingKeyLen[idx] &= ~0x80000000;
		memcpy(Set_Key.MACAddr, priv->pmib->dot11WdsInfo.entry[idx].macAddr, 6);
				Set_Key.KeyType = DOT11_KeyType_Pairwise;
				Set_Key.EncType = priv->pmib->dot11WdsInfo.wdsPrivacy;
				Set_Key.KeyIndex = priv->pmib->dot11WdsInfo.wdsKeyId;
		DOT11_Process_Set_Key(priv->dev, NULL, &Set_Key, priv->pmib->dot11WdsInfo.wdsMapingKey[idx]);
			}

#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
				add_RATid(priv, pstat);
#endif
			} else
#endif
			{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)			
				add_update_RATid(priv, pstat);
#endif
			}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			update_STA_RATid(priv, pstat);
#endif
	pstat->wds_idx = idx;
			assign_tx_rate(priv, pstat, NULL);
			assign_aggre_mthod(priv, pstat);
			assign_aggre_size(priv, pstat);

			asoc_list_add(priv, pstat);
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv)==VERSION_8188E) {				
				priv->pshare->total_assoc_num++;
				RTL8188E_AssignTxReportMacId(priv);
				RTL8188E_ResumeTxReport(priv);				
			}
#endif			

	return pstat;
}

static void create_wds_tbl(struct rtl8192cd_priv *priv)
{
	int i;
	struct stat_info *pstat;

#ifdef FAST_RECOVERY
	if (priv->reset_hangup)
		return;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv))
		return;
#endif

	if (priv->pmib->dot11WdsInfo.wdsEnabled) {
		for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
			pstat = add_wds_entry(priv, i, NULL);
			if (pstat == NULL)
				break;
		}
	}
}
#endif //  WDS


void validate_fixed_tx_rate(struct rtl8192cd_priv *priv)
{
	if (!priv->pmib->dot11StationConfigEntry.autoRate) {
		
		if(priv->pmib->dot11StationConfigEntry.fixedTxRate & BIT(31)) //ac rates, vht 1ss, 2ss, 3ss...
		{
			unsigned char vht_mcs = 0; 
		
			if(!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC))
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;

			vht_mcs = (priv->pmib->dot11StationConfigEntry.fixedTxRate & 0xff);
				
			if((get_rf_mimo_mode(priv) == MIMO_1T1R) && (vht_mcs >= 10))
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
			else if((get_rf_mimo_mode(priv) == MIMO_2T2R) && (vht_mcs >= 20))
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
			else if((get_rf_mimo_mode(priv) == MIMO_3T3R) && (vht_mcs >= 30))
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
		}
		else if((priv->pmib->dot11StationConfigEntry.fixedTxRate & BIT(28))
				|| (priv->pmib->dot11StationConfigEntry.fixedTxRate & 0x0ffff000)) //n rates. mcs0 - mcs 31
		{

			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))				
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;

			if((get_rf_mimo_mode(priv) == MIMO_1T1R)||(get_rf_mimo_mode(priv) == MIMO_1T2R))
			{
				if(priv->pmib->dot11StationConfigEntry.fixedTxRate & BIT(28))
					priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
				if(priv->pmib->dot11StationConfigEntry.fixedTxRate & 0x0ff00000)
					priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
			}
			else if(get_rf_mimo_mode(priv) == MIMO_2T2R)
			{
				if(priv->pmib->dot11StationConfigEntry.fixedTxRate & BIT(28))
					priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
			}
			else if(get_rf_mimo_mode(priv) == MIMO_3T3R)
			{
				if(priv->pmib->dot11StationConfigEntry.fixedTxRate & BIT(28))
				{
					if((priv->pmib->dot11StationConfigEntry.fixedTxRate&0xff) > 8)
						priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
				}
			}
		}
		else if(priv->pmib->dot11StationConfigEntry.fixedTxRate & 0x00000ff0) // a, g rates, ofdm
		{
			if(!(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11A | WIRELESS_11G)))
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
		}
		else //b rates, cck 
		{
			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))				
				priv->pmib->dot11StationConfigEntry.fixedTxRate = 0;
		}

		if (priv->pmib->dot11StationConfigEntry.fixedTxRate==0) {
			priv->pmib->dot11StationConfigEntry.autoRate=1;
			panic_printk("invalid fixed tx rate, use auto rate!\n");
		}
		else
			priv->pshare->current_tx_rate = get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate);
		
	}
}


#ifdef RESERVE_TXDESC_FOR_EACH_IF
void recalc_txdesc_limit(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *root_priv = NULL;
	int i, num, total_if = 0;

	if (IS_ROOT_INTERFACE(priv))
		root_priv = priv;
	else
		root_priv = GET_ROOT(priv);

	if (IS_DRV_OPEN(root_priv))
		total_if++;

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(root_priv->pvxd_priv))
		total_if++;
#endif

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(root_priv->pvap_priv[i]))
			total_if++;
	}
#endif

	if (total_if <= 1) {
		root_priv->pshare->num_txdesc_cnt = CURRENT_NUM_TX_DESC - 2;  // 2 for space...
		root_priv->pshare->num_txdesc_upper_limit = CURRENT_NUM_TX_DESC - 2;
		root_priv->pshare->num_txdesc_lower_limit = 0;
		return;
	}
	
	num = (CURRENT_NUM_TX_DESC * IF_TXDESC_UPPER_LIMIT) / 100;
	root_priv->pshare->num_txdesc_upper_limit = num;
	
	num = ((CURRENT_NUM_TX_DESC - 2) - num) / (total_if - 1);
	root_priv->pshare->num_txdesc_lower_limit = num;

	num = root_priv->pshare->num_txdesc_upper_limit + 
			root_priv->pshare->num_txdesc_lower_limit * (total_if - 1);
	root_priv->pshare->num_txdesc_cnt = num;
}
#endif

#ifdef USE_TXQUEUE
void recalc_tx_bufq_limit(struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_priv *root_priv = NULL;
	int i, num, total_if = 0;

	if (IS_ROOT_INTERFACE(priv))
		root_priv = priv;
	else
		root_priv = GET_ROOT(priv);

	if (IS_DRV_OPEN(root_priv))
		total_if++;

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(root_priv->pvxd_priv))
		total_if++;
#endif

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(root_priv->pvap_priv[i]))
			total_if++;
	}
#endif

	if (total_if <= 1) {
		root_priv->pshare->num_txq_cnt = TXQUEUE_SIZE;
		root_priv->pshare->num_txq_upper_limit = TXQUEUE_SIZE;
		root_priv->pshare->num_txq_lower_limit = 0;
		return;
	}
	
	num = (TXQUEUE_SIZE * IF_TXQ_UPPER_LIMIT) / 100;
	root_priv->pshare->num_txq_upper_limit = num;

	num = (TXQUEUE_SIZE - num) / (total_if - 1);
	root_priv->pshare->num_txq_lower_limit = num;

	num = root_priv->pshare->num_txq_upper_limit +
			root_priv->pshare->num_txq_lower_limit * (total_if - 1);
	root_priv->pshare->num_txq_cnt = num;
}
#endif


#ifdef CONFIG_WLAN_HAL_8881A
// Branch 3.4 has no sys_bonding_type()
int get_bonding_type_8881A()
{
	int type = -1;
	type = REG32(0xb800000c);
	switch(type)
	{
		case 0:
			type = BOND_8881AB;
			break;
		case 1:
			type = BOND_8881AQ;
			break;
		case 3:
			type = BOND_8881AL;
			break;
		case 9:
			type = BOND_8881AN;
			break;
		case 10:
		case 13:
		case 14:
			type = BOND_8881AM;
			break;			
		default:
			break;
	}
	return type;
}
#endif

#if defined(CONFIG_RTL_8196E)	
#if 1
int get_96e_bonding_type()
{
#define 	BOND_ID_MASK	(0xF)
#define 	BOND_OPTION	(SYSTEM_BASE+0x000C)

	return  REG32(BOND_OPTION) & BOND_ID_MASK;;
}

#else
#if defined(__ECOS) //mark_ecos
extern int bonding_type; //from switch
int sys_bonding_type(void)
{
	return bonding_type;
}
#else
extern int sys_bonding_type(void); //from rtl_gpio.c
#endif
#endif

#define 	BOND_8196ES	(0xD)
void rtl_8196es_gpio_init(void)
{
	//printk("rtl_8196es_gpio_init\n");
	// 8196ES GPIO 
	// WAKE# --> GPIOB5 , in    (not yet , FIXME )
	// 8188ER GPIO definition 	, Out = 0x10  In= 0x01
	// WPS button  --> GPIO7 ,In
#ifdef RTLWIFINIC_GPIO_CONTROL
	RTLWIFINIC_GPIO_config(7, 0x01);
	
	// WPS LED /  Reset LED   --> GPIO4 ,out
	RTLWIFINIC_GPIO_config(4, 0x10);

	// Reset button  --> GPIO0 , in
	RTLWIFINIC_GPIO_config(0, 0x01);
#endif	
}
#endif

#ifdef SDIO_AP_OFFLOAD
int is_ps_cond_match(struct rtl8192cd_priv *priv)
{
	if (0 == priv->pshare->total_assoc_num)
		return 1;
	else	
		return 0;
}

void rtw_flush_all_tx_mgt_queue(struct rtl8192cd_priv *priv)
{
	rtw_txservq_flush(priv, &priv->tx_mgnt_queue);
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv))) {
		struct rtl8192cd_priv *priv_vxd = GET_VXD_PRIV(priv);
		rtw_txservq_flush(priv_vxd, &priv_vxd->tx_mgnt_queue);
	}
#endif
#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable) {
		int i;
		for (i = 0; i < RTL8192CD_NUM_VWLAN; i++) {
			struct rtl8192cd_priv *priv_vap = priv->pvap_priv[i];
			if (IS_DRV_OPEN(priv_vap)) {
				rtw_txservq_flush(priv_vap, &priv_vap->tx_mgnt_queue);
			}
		}
	}
#endif // MBSSID
}

#ifndef CONFIG_POWER_SAVE
#define ACT_STS(x, y) ((x<<4)|y )

void sdio_power_save_timer(unsigned long task_priv)
{
        struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	        
        unsigned char act = priv->pshare->ps_ctrl;
        unsigned char sts = priv->pshare->pwr_state;

        if (priv->pshare->offload_prohibited) 
            goto out1;

	printk("acli: get %d %d %d\n", priv->pshare->total_assoc_num, act, sts);

        switch ( ACT_STS(act, sts) )
        {
        	  case ACT_STS(RTW_ACT_IDLE, RTW_STS_NORMAL):
	      	  	if ( is_ps_cond_match(priv))  {
	          	    /* send offload command */
	    	  	    priv->pshare->offload_function_ctrl = RTW_PM_PREPROCESS;
	    	  	    priv->pshare->pwr_state = RTW_STS_SUSPEND;
    			
		            printk("starting offload \n");
				// Flush all tx_mgnt_queues and pending_xmitbuf_queue
				// This purpose is to stop submitting any packet in AP offload (PS) state
				rtw_flush_all_tx_mgt_queue(priv);
				rtw_flush_xmit_pending_queue(priv);
	    	  	 }
    	                break;

        	  case ACT_STS(RTW_ACT_ASSOC, RTW_STS_SUSPEND):
        	     //GET_ROOT(priv)->offload_function_ctrl = RTW_PM_AWAKE;
        	     priv->pshare->pwr_state = RTW_STS_NORMAL;
        	     mod_timer(&priv->pshare->ps_timer, jiffies + POWER_DOWN_T0 + POWER_DOWN_T0);
        	     goto out;
        	     break;

	          case ACT_STS(RTW_ACT_SETREP, RTW_STS_NORMAL):
        	     priv->pshare->pwr_state = RTW_STS_REP;
        	     goto out;
        	     break;

	          case ACT_STS(RTW_ACT_SETREP, RTW_STS_SUSPEND):
        	     //GET_ROOT(priv)->offload_function_ctrl = RTW_PM_AWAKE;
        	     //cmd_set_ap_offload(priv, 0);
        	     priv->pshare->pwr_state = RTW_STS_REP;
        	     goto out;
        	     break;

	          default:
        	     break;
       }	
out1:	
       mod_timer(&priv->pshare->ps_timer, jiffies + POWER_DOWN_T0);

out:
       priv->pshare->ps_ctrl = RTW_ACT_IDLE;
       return;	
}
#endif // !CONFIG_POWER_SAVE
#endif // SDIO_AP_OFFLOAD

#if defined(MBSSID)

int is_last_vap_iface(struct rtl8192cd_priv *priv)
{
	if(IS_VAP_INTERFACE(priv) && (vap_opened_num(priv)==0))
		return 1;
	else	
		return 0;

}

int is_first_vap_iface(struct rtl8192cd_priv *priv)
{
	if(IS_VAP_INTERFACE(priv) && (vap_opened_num(priv)==0))
		return 1;
	else	
		return 0;

}


void refine_TBTT(struct rtl8192cd_priv *priv)
{
	//panic_printk("[%s]%s +++\n", priv->dev->name, __FUNCTION__);

	if(IS_ROOT_INTERFACE(priv))
	{
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv))
			RTL_W32(TBTT_PROHIBIT, 0x40004);
		else
#endif
		if(priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod <= 40)
			RTL_W32(TBTT_PROHIBIT, 0x1df04);
		else
			RTL_W32(TBTT_PROHIBIT, 0x40004);
	}
	else if(IS_VAP_INTERFACE(priv))
	{
		RTL_W32(TBTT_PROHIBIT, 0x1df04); //vap interfacef really opened, set TBTT for MBSSID function
	}

}


void stop_mbssid(struct rtl8192cd_priv *priv)
{
	//panic_printk("[%s]%s +++\n", priv->dev->name, __FUNCTION__);

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv))
		GET_HAL_INTERFACE(priv)->StopMBSSIDHandler(priv);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif  //CONFIG_WLAN_HAL
		rtl8192cd_stop_mbssid(priv);


}


void init_mbssid(struct rtl8192cd_priv *priv)
{
	//panic_printk("[%s]%s +++\n", priv->dev->name, __FUNCTION__);

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv))
		GET_HAL_INTERFACE(priv)->InitMBSSIDHandler(priv);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		rtl8192cd_init_mbssid(priv);

}

void init_vap_hw(struct rtl8192cd_priv *priv)
{
	if(IS_ROOT_INTERFACE(priv))
	{	
		// clear the rest area of CAM
		int i=0;
		unsigned int camData[2];
		camData[1] = 0;
		for (i=0; i<8; i++) {
			camData[0] = (MBIDCAM_POLL | MBIDWRITE_EN | (i&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT);
			RTL_W32( MBIDCAMCFG, camData[1]);
			RTL_W32((MBIDCAMCFG+4), camData[0]);	
		}
	}
	else if(IS_VAP_INTERFACE(priv))
	{
		if(is_first_vap_iface(priv))
			init_mbssid(GET_ROOT(priv));

		init_mbssid(priv);
	}

}

void init_vap_beacon(struct rtl8192cd_priv *priv)
{
	//panic_printk("[%s]%s +++\n", priv->dev->name, __FUNCTION__);
	
 	if (IS_VAP_INTERFACE(priv)) 
	{
#ifdef CONFIG_PCI_HCI
		// set BcnDmaInt & BcnOk of different VAP in IMR
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) 
		{
	    	GET_HAL_INTERFACE(priv)->InitVAPIMRHandler(priv, priv->vap_init_seq);
		} 
		else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{ //not HAL
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E) 
			{
				priv->pshare->InterruptMaskExt |= (HIMRE_88E_BCNDMAINT1 << (priv->vap_init_seq-1));
				RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt);
			} 
			else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
			{
				priv->pshare->InterruptMaskExt |= (HIMRE_92E_BCNDMAINT1 << (priv->vap_init_seq-1));
				RTL_W32(REG_HIMR1_8812, priv->pshare->InterruptMaskExt);
			} 
			else
#endif
			{
				priv->pshare->InterruptMask |= (HIMR_BCNDMA1 << (priv->vap_init_seq-1));
				RTL_W32(HIMR, priv->pshare->InterruptMask);
			}
		}
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
#ifdef CONFIG_RTL_88E_SUPPORT
#if defined(CONFIG_SUPPORT_USB_INT) && defined(CONFIG_INTERRUPT_BASED_TXBCN)
		priv->pshare->InterruptMaskExt |= (HIMRE_88E_BCNDMAINT1 << (priv->vap_init_seq-1));
		RTL_W32(REG_88E_HIMRE, priv->pshare->InterruptMaskExt);
#endif
#endif // CONFIG_RTL_88E_SUPPORT
#endif // CONFIG_USB_HCI

		if (GET_ROOT(priv)->auto_channel == 0) 
		{
			priv->pmib->dot11RFEntry.dot11channel = GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel;
			priv->ht_cap_len = 0;	// re-construct HT IE

			init_beacon(priv);
		}
	}
}

#endif //MBSSID

#ifdef RTK_NL80211

int is_iface_ready_nl80211(struct net_device *dev, struct rtl8192cd_priv *priv)
{

	if (priv->drv_state & DRV_STATE_OPEN)
	{
		DEBUG_INFO(" [%s] DO NOT double OPEN !!\n", dev->name);
		return 0;
	}
	else
	{
		DEBUG_INFO("+++ OPEN[%s] for priv = 0x%x (root=%d vxd=%d vap=%d) +++ \n\n", 
				dev->name, 
				priv, IS_ROOT_INTERFACE(priv), IS_VXD_INTERFACE(priv), IS_VAP_INTERFACE(priv));
		//printk("rtk->num_vap = %d rtk->num_vxd = %d \n", priv->rtk->num_vap, priv->rtk->num_vxd);
	}

	if(!IS_ROOT_INTERFACE(priv))
	{
		if(!IS_DRV_OPEN(GET_ROOT(priv)))	
		{
			DEBUG_INFO("Root interface NOT OPEN yet !!\n");
			return 0;
		}

		if(IS_VXD_INTERFACE(priv) && (priv->rtk->num_vxd < 1))
		{
			DEBUG_INFO("VXD if already deleted or NOT added !!\n");
			return 0;
		}

		if(IS_VAP_INTERFACE(priv) && (priv->rtk->num_vap < 1))
		{
			if(!is_WRT_scan_iface(dev->name)) //eric-vap
			{
				DEBUG_INFO("VAP already deleted or NOT added !!\n");
				return 0;
			}
		}

	}

	if(is_WRT_scan_iface(dev->name))
	{
		DEBUG_INFO("No need to call open for scan iface !!\n");
		return 0;
	}

	return 1;
	
}

void prepare_iface_nl80211(struct net_device *dev, struct rtl8192cd_priv *priv)
{

	if(priv->dev->priv_flags & IFF_DONT_BRIDGE)
	{
		//printk("Unable to be bridged !! Unlock for this iface !!\n");
		priv->dev->priv_flags &= ~(IFF_DONT_BRIDGE);
	}

	check_5M10M_config(priv);

	if(IS_VAP_INTERFACE(priv))
	{
		//panic_printk("\n FORCE ROOT VAP_ENABLE=1 (VAP OPENED)!! \n\n");
		GET_ROOT(priv)->pmib->miscEntry.vap_enable = 1;
	}		

	//Clear scan_req from NL80211
	priv->scan_req = NULL; 

	/* Because end of realtek_start_ap will perform close-open to initialize,
		previos configured bandwidth will lost if always setup with N bandwidth.
	*/
	if(!priv->rtk->keep_legacy) {
		//always enable N bandwidth to make client mode support WMM
		priv->pmib->dot11BssType.net_work_type |= WIRELESS_11N;
	} else {
		priv->rtk->keep_legacy = 0;
	}
}


#endif //RTK_NL80211


int rtl8192cd_open(struct net_device *dev)
{
	struct rtl8192cd_priv *priv;	// recuresively used, can't be static
	int rc;
#ifdef  CONFIG_WLAN_HAL	
    unsigned int errorFlag;
#endif	

	int i;
#ifdef __ECOS
	unsigned long		timeout;
#endif
	unsigned long x = 0;
#ifdef CHECK_HANGUP
	int is_reset;
#endif
	int init_hw_cnt = 0;

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	unsigned int wispWlanIndex;
#endif

	DBFENTER;

	priv = GET_DEV_PRIV(dev);

    STADEBUG("===>\n");
#ifdef RTK_NL80211
	if(!is_iface_ready_nl80211(dev, priv))
		return 0;
	
	prepare_iface_nl80211(dev, priv);
#endif
#if defined (CONFIG_AUTH_RESULT)
	priv->authRes = 0;
#endif
#ifdef AC2G_256QAM
	if( (is_ac2g(priv)) && (GET_CHIP_VER(priv)==VERSION_8812E) )
	{
		if(priv->pshare->rf_ft_var.ac2g_phy_type == 0)
		{
			DEBUG_INFO("8812 2G support AC mode, PCIE_ALNA!!\n");
#ifdef HIGH_POWER_EXT_PA
			priv->pshare->rf_ft_var.use_ext_pa = 0; 
#endif
#ifdef HIGH_POWER_EXT_LNA
			priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
		}
		else
		{
			DEBUG_INFO("8812 2G support AC mode, PCIE_APA_ALNA!!\n");
#ifdef HIGH_POWER_EXT_PA
			priv->pshare->rf_ft_var.use_ext_pa = 1; 
#endif
#ifdef HIGH_POWER_EXT_LNA
			priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
		}		
	}
#endif

	#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	if(dev==wlan_device[passThruWanIdx].priv->pWlanDev)
	{
		if(priv->drv_state&DRV_STATE_OPEN)
		{
			netif_start_queue(dev); 
			return 0;
		}
	}
	#endif

#ifdef CONFIG_WLAN_HAL_8881A
    if(GET_CHIP_VER(priv)==VERSION_8881A) {
        //Enable MAC_System(BIT(0)), MAC_Lextra_Bus(BIT(1))
        REG32(0xB80000DC)= 0x03;
    }
#endif //CONFIG_WLAN_HAL_8881A
#ifdef CONFIG_WLAN_HAL_8197F
    if(GET_CHIP_VER(priv)==VERSION_8197F) {
        REG32(0xB8000064)|= 0x1F;
    }
#endif //CONFIG_WLAN_HAL_8197F
	
#if 0
//#ifdef PCIE_POWER_SAVING
	if (((REG32(CLK_MANAGE) & BIT(11)) == 0)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		&& IS_ROOT_INTERFACE(priv) 
#endif
	) {
		extern void setBaseAddressRegister(void);
		REG32(CLK_MANAGE) |=  BIT(11);
		delay_ms(10);
		PCIE_reset_procedure(0, 0, 1, priv->pshare->ioaddr);
		setBaseAddressRegister();		
	}
#endif

// for Virtual interface...
#ifdef USE_OUT_SRC
	if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8188E) || (IS_HAL_CHIP(priv))
		#ifdef CONFIG_RTL_8723B_SUPPORT
		|| (GET_CHIP_VER(priv)== VERSION_8723B)
		#endif
		) //
	{
		priv->pshare->use_outsrc = 1;
//		printk("use out source!!\n");
	}
	else
	{
		priv->pshare->use_outsrc = 0;
//		printk("NOT use out source!!\n");
	}
#endif

#ifdef CHECK_HANGUP
	is_reset = priv->reset_hangup;
#endif

// init mib from cfg file, we only need to load cfg file once - chris 2010/02
#ifdef CONFIG_RTL_COMAPI_CFGFILE
#ifdef WDS
	if (dev->base_addr) //root
#endif
#ifdef CHECK_HANGUP
	if(!is_reset)
#endif
	{
		//printk(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>dev %s set_default\n", dev->name);
		//memset(priv->pmib, 0, sizeof(struct wifi_mib));
		//set_mib_default(priv);

		CfgFileProc(dev);
	}
#endif

// register iw_handler - chris 2010/02
#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	dev->wireless_handlers = (struct iw_handler_def *) &rtl8192cd_iw_handler_def;
#endif

	memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, 6);

#ifdef WDS
	if (dev->base_addr == 0)
	{
#ifdef BR_SHORTCUT
		extern struct net_device *cached_wds_dev;
		cached_wds_dev = NULL;
#endif

		netif_start_queue(dev);
		return 0;
	}
#endif

#ifdef CONFIG_RTK_MESH

    if (dev->base_addr == 1) {
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
        if(priv->mesh_priv_sc) {/*dual band*/
            /* reallocate private to mesh dev*/
            if(
                #if defined(CUSTOMIZE_WLAN_IF_NAME)
                priv->dev->name[ROOT_IFNAME_INDEX] == '0' 
                #else
                priv->dev->name[4] == '0' 
                #endif
                && (!IS_DRV_OPEN(priv) || GET_MIB(priv)->dot1180211sInfo.mesh_enable == 0)) {
                
                priv = priv->mesh_priv_sc;
                #ifdef NETDEV_NO_PRIV
                ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv = priv;
                #else
                dev->priv = priv;
                #endif                
                memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);
            }
            else if(
                #if defined(CUSTOMIZE_WLAN_IF_NAME)
                priv->dev->name[ROOT_IFNAME_INDEX] == '1' 
                #else
                priv->dev->name[4] == '1' 
                #endif
                && (IS_DRV_OPEN(priv->mesh_priv_sc) && GET_MIB(priv->mesh_priv_sc)->dot1180211sInfo.mesh_enable)) {
                
                priv = priv->mesh_priv_sc;
                #ifdef NETDEV_NO_PRIV
                ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv = priv;
                #else
                dev->priv = priv;
                #endif                    
                memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);
            }

            #if !defined(CONFIG_RTL_MESH_CROSSBAND)
            priv->mesh_priv_sc->mesh_priv_first = priv;
            priv->mesh_priv_first = priv;
            #endif
        }
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
        #ifdef SMP_SYNC
        spin_lock_init(&(priv->mesh_path_lock));        
        spin_lock_init(&(priv->mesh_queue_lock));
        spin_lock_init(&(priv->mesh_preq_lock));
        spin_lock_init(&(priv->mesh_proxy_lock));
        spin_lock_init(&(priv->mesh_proxyupdate_lock));

        priv->mesh_proxy_lock_owner = -1;
        priv->mesh_proxyupdate_lock_owner = -1;
        priv->mesh_preq_lock_owner = -1;
        priv->mesh_queue_lock_owner = -1;
        priv->mesh_path_lock_owner = -1;         
        #endif

        #if defined(RTK_MESH_AODV_STANDALONE_TIMER)
		init_timer(&priv->mesh_expire_timer);
        #ifdef __KERNEL__
        priv->mesh_expire_timer.expires = jiffies+MESH_AODV_EXPIRE_TO;
        #endif
		priv->mesh_expire_timer.data = (unsigned long) priv;
		priv->mesh_expire_timer.function = mesh_standalone_timer_expire;
        #ifdef __KERNEL__
		add_timer(&priv->mesh_expire_timer);
        #else
	    mod_timer(&priv->mesh_expire_timer, timeout);
        #endif
        #endif //RTK_MESH_AODV_STANDALONE_TIMER

        #if defined(CONFIG_RTL_MESH_CROSSBAND)
        if(IS_DRV_OPEN(priv->mesh_priv_sc) && GET_MIB(priv->mesh_priv_sc)->dot1180211sInfo.mesh_enable) {

            #ifdef SMP_SYNC
            spin_lock_init(&(priv->mesh_priv_sc->mesh_path_lock));        
            spin_lock_init(&(priv->mesh_priv_sc->mesh_queue_lock));            
            spin_lock_init(&(priv->mesh_priv_sc->mesh_preq_lock));             
            spin_lock_init(&(priv->mesh_priv_sc->mesh_proxy_lock));
            spin_lock_init(&(priv->mesh_priv_sc->mesh_proxyupdate_lock));

            priv->mesh_priv_sc->mesh_proxy_lock_owner = -1;
            priv->mesh_priv_sc->mesh_proxyupdate_lock_owner = -1;
            priv->mesh_priv_sc->mesh_preq_lock_owner = -1;
            priv->mesh_priv_sc->mesh_queue_lock_owner = -1;
            priv->mesh_priv_sc->mesh_path_lock_owner = -1;            
            #endif

            #if defined(RTK_MESH_AODV_STANDALONE_TIMER)
            init_timer(&priv->mesh_priv_sc->mesh_expire_timer);
            #ifdef __KERNEL__
            priv->mesh_priv_sc->mesh_expire_timer.expires = jiffies+MESH_AODV_EXPIRE_TO;
            #endif
            priv->mesh_priv_sc->mesh_expire_timer.data = (unsigned long) priv->mesh_priv_sc;
            priv->mesh_priv_sc->mesh_expire_timer.function = mesh_standalone_timer_expire;
            #ifdef __KERNEL__
            add_timer(&priv->mesh_priv_sc->mesh_expire_timer);
            #else
			mod_timer(&priv->mesh_priv_sc->mesh_expire_timer, timeout);
            #endif
            #endif //RTK_MESH_AODV_STANDALONE_TIMER

        }
        #endif

		netif_start_queue(dev);
		return 0;
	}
#endif // CONFIG_RTK_MESH

#ifdef PCIE_POWER_SAVING
		if((priv->pwr_state == L1) || (priv->pwr_state == L2)) {
			PCIeWakeUp(priv, (POWER_DOWN_T0<<3));
		}
#endif
#ifdef SDIO_AP_OFFLOAD
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	}
#endif
#endif

	// stop h/w in the very beginning
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef CONFIG_USB_HCI
		priv->pshare->bDriverStopped = FALSE;
		priv->pshare->bSurpriseRemoved = FALSE;
		priv->pshare->bReadPortCancel = FALSE;
		priv->pshare->bWritePortCancel = FALSE;
		for (i = 0; i < BITS_TO_LONGS(MAX_RTW_CMD_CODE); ++i)
			priv->pshare->pending_cmd[i] = 0;
#endif

#ifdef CONFIG_SDIO_HCI
#ifdef SDIO_AP_OFFLOAD
		priv->pshare->offload_function_ctrl = RTW_PM_AWAKE;
		priv->pshare->ps_ctrl = RTW_ACT_IDLE;
		priv->pshare->pwr_state = RTW_STS_NORMAL;
		priv->pshare->offload_prohibited &= OFFLOAD_PROHIBIT_USER;
		INIT_WORK(&priv->ap_cmd_queue, handle_ap_cmd);
#endif
		priv->pshare->bDriverStopped = FALSE;
		priv->pshare->bSurpriseRemoved = FALSE;
		for (i = 0; i < BITS_TO_LONGS(MAX_RTW_CMD_CODE); ++i)
			priv->pshare->pending_cmd[i] = 0;
		priv->pshare->pHalData->SdioTxIntStatus = 0;	// Polling mode
		priv->pshare->pHalData->SdioRxFIFOCnt = 0;
#ifdef CONFIG_SDIO_TX_INTERRUPT
		priv->pshare->freepage_updated = 0;
#endif
#endif // CONFIG_SDIO_HCI
#ifdef CONFIG_XMITBUF_TXAGG_ADV
		priv->pshare->need_sched_xmit = 0;
		priv->pshare->low_traffic_xmit = 0;
#endif
	
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
			BOOLEAN     bVal;
        
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, (pu1Byte)&bVal);

			if ( bVal || (RTL_R8(SYS_FUNC_EN+1) & BIT2)) {
				panic_printk("MAC IO enabled already! Reset HW!\n");
				GET_HAL_INTERFACE(priv)->ResetHWForSurpriseHandler(priv);

				if (RT_STATUS_SUCCESS != GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
					GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
					errorFlag |= DRV_ER_CLOSE_STOP_HW;
					GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
					panic_printk("StopHW Failed\n");
			
				}
			}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif  //CONFIG_WLAN_HAL
		{//not HAL
	        if ( check_MAC_IO_Enable(priv) ) 
	            rtl8192cd_stop_hw(priv);
		}

	}

#ifdef UNIVERSAL_REPEATER
	// If vxd interface, see if some mandatory mib is set. If ok, backup these
	// mib, and copy all mib from root interface. Then, restore the backup mib
	// to current.

	if (IS_VXD_INTERFACE(priv)) {
		DEBUG_INFO("Open request from vxd\n");
		if (!IS_DRV_OPEN(GET_ROOT(priv))) {
			printk("Open vxd error! Root interface should be opened in advanced.\n");
#ifdef SDIO_AP_OFFLOAD
			ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
#endif
			return 0;
		}

#ifdef PCIE_POWER_SAVING
		if((GET_ROOT(priv)->pwr_state == L1) || (GET_ROOT(priv)->pwr_state == L2)) {
			PCIeWakeUp(GET_ROOT(priv), (POWER_DOWN_T0<<3));
		}
#endif	

		if (!(priv->drv_state & DRV_STATE_VXD_INIT)) {
// Mark following code. MIB copy will be executed through ioctl -------------
#if 0
			unsigned char tmpbuf[36];
			int len, encyption, is_1x, mac_clone, nat25;
			struct Dot11RsnIE rsnie;

			len = SSID_LEN;
			memcpy(tmpbuf, SSID, len);
			encyption = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
			is_1x = IEEE8021X_FUN;
			mac_clone = priv->pmib->ethBrExtInfo.macclone_enable;
			nat25 = priv->pmib->ethBrExtInfo.nat25_disable;
			memcpy((char *)&rsnie, (char *)&priv->pmib->dot11RsnIE, sizeof(rsnie));

			memcpy(priv->pmib, GET_ROOT_PRIV(priv)->pmib, sizeof(struct wifi_mib));

			SSID_LEN = len;
			memcpy(SSID, tmpbuf, len);
			SSID2SCAN_LEN = len;
			memcpy(SSID2SCAN, SSID, len);
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = encyption;
			IEEE8021X_FUN = is_1x;
			priv->pmib->ethBrExtInfo.macclone_enable = mac_clone;
			priv->pmib->ethBrExtInfo.nat25_disable = nat25;
			memcpy((char *)&priv->pmib->dot11RsnIE, (char *)&rsnie, sizeof(rsnie));
#ifdef WDS
			// always disable wds in vxd
			priv->pmib->dot11WdsInfo.wdsEnabled = 0;
			priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif

			// if root interface is AP mode, set infra-client in vxd
			// if root interfeace is a infra-client, set AP in vxd
			if (OPMODE & WIFI_AP_STATE) {
				OPMODE = WIFI_STATION_STATE;
#if defined(WIFI_WMM) && defined(WMM_APSD)
				APSD_ENABLE = 0;
#endif
				DEBUG_INFO("Set vxd as an infra-client\n");
			}
			else if (OPMODE & WIFI_STATION_STATE) {
				OPMODE = WIFI_AP_STATE;
				priv->auto_channel = 0;
				DEBUG_INFO("Set vxd as an AP\n");
			}
			else {
				DEBUG_ERR("Invalid opmode for vxd!\n");
				return 0;
			}
#endif
//---------------------------------------------------------- david+2008-03-17

			// correct RSN IE will be set later for WPA/WPA2
#ifdef CHECK_HANGUP
			if (!is_reset)
#endif
#ifdef SMART_REPEATER_MODE
			if (!priv->pmib->dot11OperationEntry.keep_rsnie)
#endif
				memset(&priv->pmib->dot11RsnIE, 0, sizeof(struct Dot11RsnIE));

#ifdef WDS
			// always disable wds in vxd
			priv->pmib->dot11WdsInfo.wdsEnabled = 0;
			priv->pmib->dot11WdsInfo.wdsPure = 0;
#endif

			// Sync regdomain from root AP, ensure connectivity
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11RegDomain;

#ifdef CONFIG_RTK_MESH
			// always disable mesh in vxd (for dev)
			GET_MIB(priv)->dot1180211sInfo.mesh_enable = 0;
#endif // CONFIG_RTK_MESH
			priv->drv_state |= DRV_STATE_VXD_INIT;	// indicate the mib of vxd driver has been initialized
		}
#ifdef SDIO_AP_OFFLOAD
		priv->pshare->ps_ctrl = RTW_ACT_SETREP;
#endif		
	}
#endif // UNIVERSAL_REPEATER

#ifdef CHECK_HANGUP
	if (!is_reset)
#endif
	{
/*cfg p2p cfg p2p rm*/
/*cfg p2p cfg p2p rm*/
		if (OPMODE & WIFI_AP_STATE) {
			OPMODE_VAL(WIFI_AP_STATE);
		}
#ifdef CLIENT_MODE
		else if (OPMODE & WIFI_STATION_STATE) {
			OPMODE_VAL(WIFI_STATION_STATE);
		} else if (OPMODE & WIFI_ADHOC_STATE) {
			OPMODE_VAL(WIFI_ADHOC_STATE);
#if defined(WIFI_WMM) && defined(WMM_APSD)
			APSD_ENABLE = 0;
#endif
		}
#endif
		else {
			printk("Undefined state... using AP mode as default\n");
			OPMODE_VAL(WIFI_AP_STATE);
		}

/*cfg p2p cfg p2p rm*/
/*cfg p2p cfg p2p rm*/
	}

#if defined(UNIVERSAL_REPEATER) && defined(CLIENT_MODE)
	if (IS_VXD_INTERFACE(priv) &&
		((GET_MIB(GET_ROOT(priv)))->dot11OperationEntry.opmode & WIFI_STATION_STATE)) {
		if (!chklink_wkstaQ(GET_ROOT(priv))) {
			printk("Root interface does not link yet!\n");
#ifdef SDIO_AP_OFFLOAD
			ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
#endif
			return 0;
		}
	}
#endif

#ifdef WIFI_WMM
#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) 
			if (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY && priv->pmib->dot11OperationEntry.wifi_specific)
				priv->pshare->rf_ft_var.wifi_beq_iot = 1;
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
	if (GET_CHIP_VER(priv) == VERSION_8188E || GET_CHIP_VER(priv) == VERSION_8812E)
		if (priv->pmib->dot11OperationEntry.wifi_specific)
			priv->pshare->rf_ft_var.wifi_beq_iot = 1;
#endif
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv)) {
			if (!IS_DRV_OPEN(GET_ROOT(priv))) {
				printk("Open vap error! Root interface should be opened in advanced.\n");
#ifdef SDIO_AP_OFFLOAD
				ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
#endif
				return -1;
			}

			if ((GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) == 0) {
				printk("Fail to open VAP under non-AP mode!\n");
#ifdef SDIO_AP_OFFLOAD
				ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
#endif
				return -1;
			}
			else {
#if defined(__OSK__)
				delay_ms(100);
#elif defined(CONFIG_RTL8672) 
				do {
					if (GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & WIFI_WAIT_FOR_CHANNEL_SELECT) {
						DEBUG_INFO("wait for root interface ss_timer!!\n");
						delay_ms(1);
					}
					else {
						DEBUG_INFO("channel=%x\n", GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel);
						break;
					}
				}while(1);
#endif
				rtl8192cd_init_vap_mib(priv);
			}
		}
	}
#endif

#ifdef RTK_NL80211
#ifdef UNIVERSAL_REPEATER
	if (IS_VXD_INTERFACE(priv))
		rtl8192cd_init_vxd_mib(priv);
#endif
#endif

// check phyband and channel match or not
#ifdef MP_TEST
	if(priv->pshare->rf_ft_var.mp_specific) //For MP nfjrom to open WLAN0 successfully
	{
		if ((priv->pmib->dot11RFEntry.dot11channel <= 14) && (priv->pmib->dot11RFEntry.phyBandSelect != PHY_BAND_2G))
		{
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11A | WIRELESS_11N;
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
			priv->pmib->dot11RFEntry.dot11channel = 36;
		}
		else if((priv->pmib->dot11RFEntry.dot11channel > 14) && (priv->pmib->dot11RFEntry.phyBandSelect != PHY_BAND_5G))
		{
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G | WIRELESS_11N;
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
			priv->pmib->dot11RFEntry.dot11channel = 1;
		}

		if((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))
		{
			priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G | WIRELESS_11N;
			priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
			priv->pmib->dot11RFEntry.dot11channel = 1;
		}

#ifdef DFS
		if (!priv->pmib->dot11DFSEntry.disable_DFS)
			priv->pmib->dot11DFSEntry.disable_DFS;
#endif
        // TODO: Should we add some code here??  By Filen
	} 
#endif

#ifdef RTK_AC_SUPPORT //for 11ac logo
	if(AC_SIGMA_MODE != AC_SIGMA_NONE)
	{
		priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_TEST;
		DEBUG_INFO("Enable all channels, domain = %d\n", priv->pmib->dot11StationConfigEntry.dot11RegDomain);
	}
#endif

#if defined(BR_SHORTCUT)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if(IS_ROOT_INTERFACE(priv))
#endif
		clear_shortcut_cache();
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
    if (IS_ROOT_INTERFACE(priv))
#endif
    {
        priv->pshare->current_num_tx_desc = NUM_TX_DESC;

        #if defined (CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_92D_DMDP)
        if ((GET_CHIP_VER(priv) == VERSION_8192D) && 
             (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY)) 
            priv->pshare->current_num_tx_desc = (NUM_TX_DESC>MAX_NUM_TX_DESC_DMDP)?
                (MAX_NUM_TX_DESC_DMDP):(NUM_TX_DESC);
        #endif

        #if 0 //defined(CONFIG_WLAN_HAL) 
        MACFM_software_init(priv);
        #endif

        /* Init WLAN ablility*/
        WLAN_ability_init(priv);	/*Add by BB Yu Chen for Beamforming ability, need to check code structure*/
    }

    rc = rtl8192cd_init_sw(priv);
    if (rc) {
        printk("ERROR : rtl8192cd_init_sw failure\n");
        goto free_res;
    }

#if defined(CONFIG_RTL8196B_TR) || defined(CONFIG_RTL8196C_EC)
	if (!priv->auto_channel) {
		LOG_START_MSG();
	}
#endif
//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if (!priv->auto_channel) {
		LOG_START_MSG();
	}
#endif

	validate_fixed_tx_rate(priv);

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef CONFIG_SDIO_HCI
		if ((rc = sdio_alloc_irq(priv)) != 0) {
			goto free_res;
		}
#ifdef CONFIG_POWER_SAVE
		rc = init_wifi_wakeup_gpio(dev, priv);
		if (rc < 0) 
			goto free_res;
#endif
#endif // CONFIG_SDIO_HCI

#ifdef CONFIG_PCI_HCI
#if defined(__KERNEL__) || defined(__OSK__)
#ifdef CHECK_HANGUP
		if (!is_reset)
#endif
		{
#if !defined(CONFIG_RTL8671)
	#if defined(__LINUX_2_6__)
		#if defined(CONFIG_RTL_92D_DMDP) || defined(NOT_RTK_BSP) || defined (CONFIG_CT_PUSHKEYEVENT)
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_SHARED, dev->name, dev);
			#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_SHARED|IRQF_DISABLED, dev->name, dev);
			#endif
		#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_DISABLED, dev->name, dev);
		#endif
	#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, SA_SHIRQ, dev->name, dev);
	#endif
#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, SA_INTERRUPT, dev->name, dev);
#endif

#if defined(PCIE_POWER_SAVING) && defined(GPIO_WAKEPIN)
			rc |= request_irq_for_wakeup_pin(dev);
#endif
#if defined(GPIO_WAKEPIN) && IS_EXIST_RTL8822BE
            if(IS_HARDWARE_TYPE_8822B(priv))
                rc |= request_irq_for_wakeup_pin_V2(dev);
#endif

			if (rc) {
				DEBUG_ERR("some issue in request_irq, rc=%d\n", rc);
			}
		}
#endif
#endif // CONFIG_PCI_HCI

		//SAVE_INT_AND_CLI(x);
		SMP_LOCK(x);

#if defined(CONFIG_RTL_8197F) //eric-dog 
		watchdog_kick(); 
#else
		watchdog_stop(priv);
#endif


#ifdef EN_EFUSE
		if (priv->pmib->efuseEntry.enable_efuse & EFUSE_ENABLE) {
			int i, readEfuse=0;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
			int idx= 0;
			if ((GET_CHIP_VER(priv) == VERSION_8192D) || 
				(GET_CHIP_VER(priv) == VERSION_8812E) || 
				(GET_CHIP_VER(priv) == VERSION_8814A) || 
				(GET_CHIP_VER(priv) == VERSION_8822B)) 
			{
				for (i=0 ; i < MAX_5G_CHANNEL_NUM ; i++) {
					if (priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i]){						
						idx++;
					}
				}

				if (!idx)
					readEfuse = 1;
			}			
			else
#endif
			{
				for(i=0;i<MAX_2G_CHANNEL_NUM;i++)	{
					if(	priv->pmib->dot11RFEntry.pwrlevelCCK_A[i]==0 
					||	priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] ==0) {
						readEfuse = 1;
						break;
					}
				}
			}
			if(readEfuse)
				ReadTxPowerInfoFromHWPG(priv);

			if(priv->pmib->dot11RFEntry.ther==0) {
				ReadThermalMeterFromEfuse(priv);

#ifdef CONFIG_RTL_92D_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8192D) {
					ReadCrystalCalibrationFromEfuse(priv);
					ReadDeltaValFromEfuse(priv);
					ReadTRSWPAPEFromEfuse(priv);
				}
#endif
#ifdef CONFIG_SDIO_HCI
#ifdef CONFIG_RTL_88E_SUPPORT
				if (GET_CHIP_VER(priv)==VERSION_8188E)
					ReadCrystalCalibrationFromEfuse(priv);
#endif
#endif
			}
#if defined(CONFIG_RTL_8812_SUPPORT)
			if (GET_CHIP_VER(priv)==VERSION_8812E)
				ReadTxBBSwingFromEfuse(priv);
#endif
		}
#endif

do_hw_init:


#if defined(CONFIG_AUTO_PCIE_PHY_SCAN)
#ifdef CONFIG_RTL_88E_SUPPORT
		if(GET_CHIP_VER(priv)==VERSION_8188E)
		{
	#if defined(CONFIG_RTL_8881A)
		if ((REG32(0xb8000008) & 0x1000000) != 0x1000000) {
			RTL_W8(0x2c, ((RTL_R8(0x2c) & 0xf0) | 0x1));
            printk("\n\n 8881A+88E 25M TEST 0x11=0x5b, 0x2c[0:3]=0x1 \n\n");
		} else {
			printk("\n\n 8881A+88E 40M TEST 0x11=0x5b \n\n");
		}
	#elif defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
		if ((REG32(0xb8000008)&0x2000000)==0x2000000)  //40MHz
		{
			printk("\n\n 88E 40M TEST 0x11=0x5b \n\n");
			
			//RTL_W8(0x11, 0x5b);
			//RTL_W8(0x2c, ((RTL_R8(0x2c) & 0xf0) | 0x1));
		}
		else //25MHz
		{
			printk("\n\n 88E 25M TEST 0x11=0x5b, 0x2c[0:3]=0x1 \n\n");
			
			//RTL_W8(0x11, 0x5b);
			RTL_W8(0x2c, ((RTL_R8(0x2c) & 0xf0) | 0x1));
		}
	#endif
		}
#endif
#endif
#if !defined(CONFIG_AUTO_PCIE_PHY_SCAN) && !defined(CONFIG_PHY_EAT_40MHZ) && (defined(CONFIG_RTL_8197DL) || defined(CONFIG_RTL8672))
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			//25MHz
			//printk("\n\n 88E 25M TEST 0x11=0x5b, 0x2c[0:3]=0x1 \n\n");
			printk("\n\n 88E 25M,  0x2c[0:3]=0x1 \n\n");
			
			//RTL_W8(0x11, 0x5b);
			RTL_W8(0x2c, ((RTL_R8(0x2c) & 0xf0) | 0x1));
		}
#endif
#endif
		
		rc = rtl8192cd_init_hw_PCI(priv);
		//delay_ms(200);		// TODO: need refinement, for 98 watchdog time out

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		if (priv->pshare->bSurpriseRemoved) {
			rc = 1;
			goto free_res;
		}
#endif

		// write IDR0, IDR4 here
		{
#ifdef  CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {
                if(!IS_MACHAL_CHIP(priv))
                {
        	    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)dev->dev_addr);
                }                       
			} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			{//not HAL
				unsigned long reg = 0;
				reg = *(unsigned long *)(dev->dev_addr);
	//			RTL_W32(IDR0, (cpu_to_le32(reg)));
				RTL_W32(MACID, (cpu_to_le32(reg)));
				reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
	//			RTL_W32(IDR4, (cpu_to_le32(reg)));
				RTL_W16(MACID+4, (cpu_to_le16(reg)));
	#ifdef CONFIG_RTL_92D_SUPPORT
				if (GET_CHIP_VER(priv) == VERSION_8192D) {
					RTL_W32(MACID1 , RTL_R32(MACID));
					RTL_W16(MACID1+4, RTL_R16(MACID+4));
				}
	#endif
			}
		}

		if (rc && ++init_hw_cnt < 5) {
#ifndef NOT_RTK_BSP
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			watchdog_kick();
		}
#endif
			goto do_hw_init;
		}

		//if (GET_CHIP_VER(priv)==VERSION_8192D)  {
#if defined(CONFIG_RTL_8197F) //eric-dog 
		watchdog_kick(); 
#else
		watchdog_resume(priv);
#endif

		//}

		//RESTORE_INT(x);
		SMP_UNLOCK(x);

		if (rc) {
			DEBUG_ERR("init hw failed!\n");
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8192D){
				RTL_W8(RSV_MAC0_CTRL, RTL_R8(RSV_MAC0_CTRL)&(~MAC0_EN));
				RTL_W8(RSV_MAC1_CTRL, RTL_R8(RSV_MAC1_CTRL)&(~MAC1_EN));
			}
#endif //CONFIG_RTL_92D_SUPPORT
			force_stop_wlan_hw();
#if !defined(NOT_RTK_BSP) && !defined(CONFIG_RTL8672)
#ifdef CONFIG_RTL_92D_SUPPORT
			if (GET_CHIP_VER(priv)!=VERSION_8192D)	/* do not trigger wtdog if 92D open fail */
#endif
			{
#ifdef LINUX_2_6_20_
			local_irq_disable();
#elif !defined(__OSK__)
			cli();
#endif
#ifdef CONFIG_RTL_8198B
			REG32(BSP_WDTCTRLR) = BSP_WDT_ENABLE;
#elif !defined(__OSK__)
			*(volatile unsigned long *)(0xB800311c) = 0; /* enable watchdog reset now */
#endif
			for(;;)
				;
			}
#endif // !NOT_RTK_BSP
			goto free_res;
		}
#ifdef CONFIG_WLAN_HAL_8814AE // temp add in 8813 FPGA by Eric
		else
		{
            #ifdef DRVMAC_LB
                    if(GET_CHIP_VER(priv)==VERSION_8814A) {
                        // disable early mode, multi-tag
                        RTL_W32(0x3fc, ((RTL_R32(0x3fc)&0xFFFF0FFF) & (~BIT6))&(~BIT5) );
                    }
            #endif
            #if 0 //def RTL8814_FPGA_TEMP
                if(GET_CHIP_VER(priv)==VERSION_8814A) {

                    // Open BB RF
                    //RTL_W8(0x2, RTL_R8(0x2)|BIT0|BIT1);

                    // ACK RATE SETTING
                	//RTL_W8(0x6de, RTL_R8(0x6de)|BIT7);

                    //===============Disable New Early Mode================

                    // disable new early mode, and AMPDU burst mode
                    //RTL_W32(0x4bc, ((RTL_R32(0x4bc)&0xFFFFFFF0)&(~BIT7)));

                    // disable early mode, multi-tag
                    RTL_W32(0x3fc, ((RTL_R32(0x3fc)&0xFFFF0FFF) & (~BIT6))&(~BIT5) );
    
                    // BIT_UDF_THSD[8]=1,enable underflow recovery;r_UDF_THSD[8]=0,disable underflow recovery;
                    // BIT_UDF_THSD[6:0]: when mac_wrptr - PHY_rdptr is less than this value, set underflow;
                	//RTL_W8(0x632, 0);                                

                    // Disable Rx BA or ACK
                    //RTL_W8(0x7A3,RTL_R8(0x7A3)& (~BIT0));          
                    
                    // enable Rx data/management/control packet
                    //RTL_W32(0x6a0,0xffffffff);
                    //RTL_W32(0x6a4,0xffffffff);     

                	//RTL_W8(0x60C,0x20);    

                    // Enable MAC security engine 0x100 bit9, sync with RS driver 2013/07/11
                    //RTL_W8(0x101,RTL_R8(0x101)|BIT1);  

                    // REG_TRXFF_BNDY 0x114 bit shift error, sync with RS driver 2013/07/11
                    // set RX boundary 24K
                    //RTL_W32(0x114, (RTL_R32(0x114)&0xFC0000FF)|(0x5fff)<<8);
                    // set RXFIFO overflow Thre.
                    //RTL_W32(0x114, (RTL_R32(0x114)&0x3FFFFFF)|(0xf)<<28);                    

                    // disable PTCL will write BCN TRXRPT when BCNOK/BCNERR INT. 
                    //RTL_W8(0x420, RTL_R8(0x420)&(~BIT2)); 
                    //RTL_W8(0x421, RTL_R8(0x421)&(~BIT5));                     

                    // REG_AMPDU_MAX_LENGTH 256K
                    //RTL_W32(0x458,0x3FFFF);

                    // RESERVED
                    //RTL_W32(0x460,0x3FFFF);

                    // REG_SW_AMPDU_BURST_MODE_CTRL
                    //RTL_W8(0x4BC, RTL_R8(0x4BC)| 0x3F);                                         
                    
                    // Enable single sub-frame APMDU.
                    //RTL_W8(0x4C7, RTL_R8(0x4C7)|BIT7);                                         

                    // REG_PROT_MODE_CTRL ?????
                    //RTL_W32(0x4c8,0x003F0800);

                    // RESERVED
                    //RTL_W8(0x4E3, RTL_R8(0x4e3)& (~BIT5));                                         

                    // REG_SIFS, sync with 8814 NIC driver
                    //RTL_W32(0x514,0x0E100E10);

                    // If txd fifo underflow when txtype is cmpba, reset mactx state machine ,bit14
                    // If txd fifo underflow, reset mactx state machine , bit 13
                    // Phytxend_ps comes but mactx still active, reset mactx state machine ,bit12                 
                    //RTL_W32(0x604,RTL_R32(0x604)& (~(BIT12|BIT13|BIT14)));

                    // REG_MAC_SPEC_SIFS (Specification SIFS Register) sync error setting with RS driver
                    //RTL_W16(0x63a,0x0e10);                    

                    // REG_UDF_THSD
                    //RTL_W32(0x630,RTL_R32(0x630)|BIT19);

                    // REG_RESP_SIFS_CCK (Response SIFS for CCK Register)
                    // REG_RESP_SIFS_OFDM (Response SIFS for OFDM Register)
                    //RTL_W32(0x63C, 0x0E0E1010);

                    // temp add init CPU fw initialize ready, set by CPU
                   // RTL_W32(0x80,RTL_R32(0x80)|BIT15);

                    // set CRC5 buffer addr  
                    //RTL_W32(0x6bc, 0x03000000);  // set CRC5 buffer addr                 

                    // set 
                    //RTL_W32(0x7d4,RTL_R32(0x7D4)|BIT17);  // set CRC5 buffer addr                                     
                }
				#endif //#ifdef RTL8814_FPGA_TEMP    
		}
#endif //CONFIG_WLAN_HAL_8814AE
	}
#if 0 // defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	else {
		if (get_rf_mimo_mode(priv) == MIMO_1T1R)
			GET_MIB(priv)->dot11nConfigEntry.dot11nSupportedMCS &= 0x00ff;
	}
#endif

#ifdef SDIO_2_PORT
	if (IS_VXD_INTERFACE(priv)) {
		unsigned long reg;

		reg = *(unsigned long *)(dev->dev_addr);
		RTL_W32(MACID1, (cpu_to_le32(reg)));
		reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
		RTL_W16(MACID1+4, (cpu_to_le16(reg)));
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			rtl8192cd_set_mbidcam(priv, GET_MY_HWADDR, 7);
#endif
	}
#endif

#ifdef HW_FILL_MACID
    if(IS_SUPPORT_HW_FILL_MACID(priv)) {
        GET_HAL_INTERFACE(priv)->InitMACIDSearchHandler(priv);
    }
#endif //#ifdef HW_FILL_MACID

#ifdef MBSSID
	if ((OPMODE & WIFI_AP_STATE) && (GET_ROOT(priv)->pmib->miscEntry.vap_enable)) {
		refine_TBTT(priv);
#if defined(CONFIG_PCI_HCI)
		init_vap_hw(priv);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		init_mbssid(priv);
#endif
		init_vap_beacon(priv);

#ifdef RTK_NL80211
		if(IS_ROOT_INTERFACE(priv))
		{
			//for  performance 
			//panic_printk("\n FORCE ROOT VAP_ENABLE=0 !! \n\n"); 
			priv->pmib->miscEntry.vap_enable = 0;
		}
#endif

	}
	else if (IS_VAP_INTERFACE(priv) && (OPMODE & WIFI_STATION_STATE)) {
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv))
        	GET_HAL_INTERFACE(priv)->InitMBIDCAMHandler(priv);
		else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			; //ToDo
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
    if(IS_ROOT_INTERFACE(priv))
#endif 
    {
#if defined(CONFIG_WLAN_HAL) 
		if (IS_HAL_CHIP(priv)){
#if defined(CONFIG_PCI_HCI)			
	        u1Byte HIQ_En=0;
	        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HIQ_En);
	        HIQ_En = HIQ_En | BIT0;
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HIQ_En);
#endif			
#ifdef MULTI_MAC_CLONE
			GET_HAL_INTERFACE(priv)->InitMBIDCAMHandler(priv);
			//default add into mbssid cam, vxd or vap as client mode will enable BIT 24 if macclone or vap is enabled.
#endif
		}
#endif
    }


// new added to reset keep_rsnie flag
	if (priv->pmib->dot11OperationEntry.keep_rsnie)
		priv->pmib->dot11OperationEntry.keep_rsnie = 0;
//------------------- david+2006-06-30

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		/*	set NMK	*/
		GenerateRandomData(priv->wapiNMK, WAPI_KEY_LEN);
		priv->wapiMCastKeyId = 0;
		priv->wapiMCastKeyUpdate = 0;
		priv->wapiWaiTxSeq = 0;
		wapiInit(priv);
	}
#endif
priv->drv_state |= DRV_STATE_OPEN;      // set driver as has been opened, david

#if defined(CONFIG_VERIWAVE_CHECK)
	{
		unsigned int now = RTL_R32(TSFTR);
		setup_timer1(priv, now + CHK_VERIWAVE_TO * 10);//after sometime and then start to do this timer
	}
#endif

#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
#ifdef CONFIG_PCI_HCI
		GET_HAL_INTERFACE(priv)->EnableIMRHandler(priv);
#endif
	}
#endif

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	recalc_txdesc_limit(priv);
#endif
#ifdef USE_TXQUEUE
	recalc_tx_bufq_limit(priv);
#endif

	//memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, 6);

	// below is for site_survey timer
	init_timer(&priv->ss_timer);
#if defined(CONFIG_PCI_HCI)
	priv->ss_timer.data = (unsigned long) priv;
	priv->ss_timer.function = rtl8192cd_ss_timer;
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	priv->ss_timer.data = (unsigned long) &priv->ss_timer_event;
	priv->ss_timer.function = timer_event_timer_fn;
	INIT_TIMER_EVENT_ENTRY(&priv->ss_timer_event,rtl8192cd_ss_timer, (unsigned long)priv);
#endif


/*cfg p2p cfg p2p rm*/
/*cfg p2p cfg p2p rm*/

#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	init_waitqueue_head(&priv->ss_wait);
#endif

#ifdef CLIENT_MODE
	init_timer(&priv->reauth_timer);
#if defined(CONFIG_PCI_HCI)
	priv->reauth_timer.data = (unsigned long) priv;
	priv->reauth_timer.function = rtl8192cd_reauth_timer;
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	priv->reauth_timer.data = (unsigned long) &priv->reauth_timer_event;
	priv->reauth_timer.function = timer_event_timer_fn;
	INIT_TIMER_EVENT_ENTRY(&priv->reauth_timer_event, rtl8192cd_reauth_timer, (unsigned long)priv);
#endif

	init_timer(&priv->reassoc_timer);
#if defined(CONFIG_PCI_HCI)
	priv->reassoc_timer.data = (unsigned long) priv;
	priv->reassoc_timer.function = rtl8192cd_reassoc_timer;
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	priv->reassoc_timer.data = (unsigned long) &priv->reassoc_timer_event;
	priv->reassoc_timer.function = timer_event_timer_fn;
	INIT_TIMER_EVENT_ENTRY(&priv->reassoc_timer_event, rtl8192cd_reassoc_timer, (unsigned long)priv);
#endif

	init_timer(&priv->idle_timer);
#if defined(CONFIG_PCI_HCI)
	priv->idle_timer.data = (unsigned long) priv;
	priv->idle_timer.function = rtl8192cd_idle_timer;
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	priv->idle_timer.data = (unsigned long) &priv->idle_timer_event;
	priv->idle_timer.function = timer_event_timer_fn;
	INIT_TIMER_EVENT_ENTRY(&priv->idle_timer_event, rtl8192cd_idle_timer, (unsigned long)priv);
#endif
#ifdef DFS
	init_timer(&priv->dfs_cntdwn_timer);
	priv->dfs_cntdwn_timer.data = (unsigned long) priv;
	priv->dfs_cntdwn_timer.function = rtl8192cd_dfs_cntdwn_timer;
#endif
#endif

#ifdef MULTI_MAC_CLONE
	for (i=0; i<priv->pshare->mclone_num_max; i++) {
	    if (priv->pshare->mclone_sta[i].isTimerInit == 0) {// if it is used, do not re-init
			priv->pshare->mclone_sta[i].timer_data.priv = GET_ROOT(priv);
			priv->pshare->mclone_sta[i].timer_data.active_id = i+1;
	    	
			init_timer(&priv->pshare->mclone_sta[i].reauth_timer);
			priv->pshare->mclone_sta[i].reauth_timer.data = (unsigned long)&priv->pshare->mclone_sta[i].timer_data;
			priv->pshare->mclone_sta[i].reauth_timer.function = rtl8192cd_mclone_reauth_timer;
	
			init_timer(&priv->pshare->mclone_sta[i].reassoc_timer);
			priv->pshare->mclone_sta[i].reassoc_timer.data = (unsigned long)&priv->pshare->mclone_sta[i].timer_data;
			priv->pshare->mclone_sta[i].reassoc_timer.function = rtl8192cd_mclone_reassoc_timer;
			priv->pshare->mclone_sta[i].isTimerInit = 1;
		}		
	}
#endif

	priv->frag_to = 0;

	init_timer(&priv->frag_to_filter);
	priv->frag_to_filter.data = (unsigned long) priv;
	priv->frag_to_filter.function = rtl8192cd_frag_timer;

	mod_timer(&priv->frag_to_filter, jiffies + FRAG_TO);

	priv->auth_to = AUTH_TO / HZ;
	priv->assoc_to = ASSOC_TO / HZ;
#ifdef MULTI_MAC_CLONE
	priv->repeater_to = REPEATER_TO / HZ;
#endif

#ifdef PCIE_POWER_SAVING_DEBUG
	priv->expire_to = 60;
#else
	priv->expire_to = (EXPIRETIME > 100)? (EXPIRETIME / 100) : 86400; /*10ms to 1s*/
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if defined(DETECT_STA_EXISTANCE) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) )
		priv->pshare->bRLShortened = FALSE;
#endif
		
#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
		init_timer(&priv->ps_timer);
		priv->ps_timer.data = (unsigned long) priv;
#if defined(PCIE_POWER_SAVING)		
		priv->ps_timer.function = PCIe_power_save_timer;
#else
		priv->ps_timer.function = RF_MIMO_check_timer;
#endif

		mod_timer(&priv->ps_timer, jiffies + POWER_DOWN_T0);
//		priv->ps_ctrl = 0x11;
#endif

#ifdef SDIO_AP_OFFLOAD
		init_timer(&priv->pshare->ps_timer);
		priv->pshare->ps_timer.data = (unsigned long) priv;
		priv->pshare->ps_timer.function = sdio_power_save_timer;
#ifdef MP_TEST
		if (!priv->pshare->rf_ft_var.mp_specific)
#endif
		mod_timer(&priv->pshare->ps_timer, jiffies + POWER_DOWN_T0);
#endif

		init_timer(&priv->expire_timer);
		priv->expire_timer.data = (unsigned long) priv;
#if defined(CONFIG_PCI_HCI)
		priv->expire_timer.function = rtl8192cd_1sec_timer;
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		priv->expire_timer.function = pre_rtl8192cd_1sec_timer;
		INIT_TIMER_EVENT_ENTRY(&priv->expire_timer_event, rtl8192cd_1sec_timer, (unsigned long)priv);
#endif
#ifdef 	SW_ANT_SWITCH
		init_timer(&priv->pshare->swAntennaSwitchTimer);
		priv->pshare->swAntennaSwitchTimer.data = (unsigned long) priv;
		priv->pshare->swAntennaSwitchTimer.function = dm_SW_AntennaSwitchCallback;
#endif
#if (BEAMFORMING_SUPPORT == 1)
		if (priv->pmib->dot11RFEntry.txbf) //eric-mu
		Beamforming_Init(priv);
#endif

		mod_timer(&priv->expire_timer, jiffies + EXPIRE_TO);

		init_timer(&priv->pshare->rc_sys_timer);
		priv->pshare->rc_sys_timer.data = (unsigned long) priv;
		priv->pshare->rc_sys_timer.function = reorder_ctrl_timeout;

		priv->pshare->rc_timer_tick = priv->pmib->reorderCtrlEntry.ReorderCtrlTimeout / RTL_JIFFIES_TO_MICROSECOND;
		if (priv->pshare->rc_timer_tick == 0)
			priv->pshare->rc_timer_tick = 1;

		init_timer(&priv->pshare->rc_sys_timer_cli);
		priv->pshare->rc_sys_timer_cli.data = (unsigned long) priv;
		priv->pshare->rc_sys_timer_cli.function = reorder_ctrl_timeout_cli;

		priv->pshare->rc_timer_tick_cli = priv->pmib->reorderCtrlEntry.ReorderCtrlTimeoutCli / RTL_JIFFIES_TO_MICROSECOND;
		if (priv->pshare->rc_timer_tick_cli == 0)
			priv->pshare->rc_timer_tick_cli = 1;

#ifndef USE_OUT_SRC
		#ifdef TPT_THREAD
		kTPT_task_init(priv);
		#endif
#endif

#if 0
		init_timer(&priv->pshare->phw->tpt_timer);
		priv->pshare->phw->tpt_timer.data = (unsigned long)priv;
		priv->pshare->phw->tpt_timer.function = rtl8192cd_tpt_timer;
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(DPK_92D)
		if (GET_CHIP_VER(priv) == VERSION_8192D){
			init_timer(&priv->pshare->DPKTimer);
			priv->pshare->DPKTimer.data = (unsigned long) priv;
			priv->pshare->DPKTimer.function = rtl8192cd_DPK_timer;
		}
#endif

#if defined(MULTI_STA_REFINE)
		if (IS_HAL_CHIP(priv) || (GET_CHIP_VER(priv)==VERSION_8812E)){
			init_timer(&priv->pshare->PktAging_timer);
			priv->pshare->PktAging_timer.data = (unsigned long) priv;
			priv->pshare->PktAging_timer.function = TxPktBuf_AgingTimer;
			mod_timer(&priv->pshare->PktAging_timer, jiffies + RTL_SECONDS_TO_JIFFIES(10));
		}
#endif

	}

#if (MU_BEAMFORMING_SUPPORT == 1) &&  defined(UNIVERSAL_REPEATER)
	if (priv->pmib->dot11RFEntry.txbf_mu && under_apmode_repeater(priv)) {				
		if(IS_VXD_INTERFACE(priv)) {
			panic_printk("!! Disable Repeater-VXD MU-MIMO function (Not Support)\n");
			priv->pmib->dot11RFEntry.txbf_mu = 0;			
		}
	}
#endif

	// for MIC check
	init_timer(&priv->MIC_check_timer);
	priv->MIC_check_timer.data = (unsigned long) priv;
	priv->MIC_check_timer.function = DOT11_Process_MIC_Timerup;
	init_timer(&priv->assoc_reject_timer);
	priv->assoc_reject_timer.data = (unsigned long) priv;
	priv->assoc_reject_timer.function = DOT11_Process_Reject_Assoc_Timerup;

	priv->MIC_timer_on = FALSE;
	priv->assoc_reject_on = FALSE;

#ifdef GBWC
	init_timer(&priv->GBWC_timer);
	priv->GBWC_timer.data = (unsigned long) priv;
	priv->GBWC_timer.function = rtl8192cd_GBWC_timer;

	if (priv->pmib->gbwcEntry.GBWCMode != GBWC_MODE_DISABLE)
		mod_timer(&priv->GBWC_timer, jiffies + GBWC_TO);
#endif

#ifdef RTK_STA_BWC
	init_timer(&priv->sta_bwc_timer);
	priv->sta_bwc_timer.data = (unsigned long) priv;
	priv->sta_bwc_timer.function = rtl8192cd_sta_bwc_timer;

	if (priv->pshare->rf_ft_var.sta_bwc_en)
		mod_timer(&priv->sta_bwc_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.sta_bwc_to));
#endif

#ifdef SBWC
	if (OPMODE & WIFI_AP_STATE) {
		init_timer(&priv->SBWC_timer);
		priv->SBWC_timer.data = (unsigned long) priv;
		priv->SBWC_timer.function = rtl8192cd_SBWC_timer;
		mod_timer(&priv->SBWC_timer, jiffies + SBWC_TO);
	}
#endif

#if defined(CH_LOAD_CAL) || defined(HS2_SUPPORT) || defined(DOT11K) || defined(RTK_SMART_ROAMING)
#if  defined(HS2_SUPPORT) || defined(DOT11K) || defined(RTK_SMART_ROAMING)
	priv->pmib->dot11StationConfigEntry.cu_enable=1;
#endif
	if(IS_ROOT_INTERFACE(priv)){
		#ifdef USE_OUT_SRC
		

		phydm_CLMInit(ODMPTR, CLM_SAMPLE_NUM2);	    
		#else
		start_bbp_ch_load(priv,CLM_SAMPLE_NUM2);
		#endif	    
	}
#endif


#ifdef CONFIG_PCI_HCI
	// to avoid add RAtid fail
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	init_timer(&priv->add_RATid_timer);
	priv->add_RATid_timer.data = (unsigned long) priv;
	priv->add_RATid_timer.function = add_RATid_timer;

	init_timer(&priv->add_rssi_timer);
	priv->add_rssi_timer.data = (unsigned long) priv;
	priv->add_rssi_timer.function = add_rssi_timer;
#endif
	init_timer(&priv->add_ps_timer);
	priv->add_ps_timer.data = (unsigned long) priv;
	priv->add_ps_timer.function = add_ps_timer;
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		init_timer(&priv->dnc_timer);
		priv->dnc_timer.data = (unsigned long) priv;
		priv->dnc_timer.function = dnc_timer;
	}
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) 
	init_timer(&priv->pshare->MP_DIGTimer);
	priv->pshare->MP_DIGTimer.data = (unsigned long) priv;
	priv->pshare->MP_DIGTimer.function = MP_DIG_process;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if defined(OSK)
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) 
	if ((GET_CHIP_VER(priv) == VERSION_8188E)||(GET_CHIP_VER(priv) == VERSION_8192E))/*tony 20150203 add: chip detect*/
		RTLWIFINIC_GPIO_init_priv(priv); //init root_priv here
#endif
#endif

	if (GET_CHIP_VER(priv) == VERSION_8822B)
		RTLWIFINIC_GPIO_init_priv(priv);

		// for HW/SW LED
		if ((LED_TYPE >= LEDTYPE_HW_TX_RX) && (LED_TYPE <= LEDTYPE_HW_LINKACT_INFRA))
			enable_hw_LED(priv, LED_TYPE);
		else if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX)) {
			if (LED_TYPE == LEDTYPE_SW_RESERVED)
				LED_TYPE = LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D;

			if ((LED_TYPE == LEDTYPE_SW_LINK_TXRX) ||
				(LED_TYPE <= LEDTYPE_SW_LINKTXRX) ||
				(LED_TYPE == LEDTYPE_SW_LED2_GPIO8_LINKTXRX) ||
				(LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX) ||
				(LED_TYPE == LEDTYPE_SW_LED1_GPIO9_LINKTXRX_92D) ||
				(LED_TYPE == LEDTYPE_SW_LED2_GPIO10_LINKTXRX_92D))
				priv->pshare->LED_cnt_mgn_pkt = 1;

			enable_sw_LED(priv, 1);
		}

#ifdef CONFIG_RTL_ULINKER
		{
			extern void enable_sys_LED(struct rtl8192cd_priv *priv);
			enable_sys_LED(priv);
		}
#endif
		
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
	if(!IS_OUTSRC_CHIP(priv))
#endif
	{
#ifdef SW_ANT_SWITCH
		dm_SW_AntennaSwitchInit(priv);	// SW Ant Switch use LED pin to control TRX Antenna
#endif
#if defined(HW_ANT_SWITCH)&&( defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
		dm_HW_AntennaSwitchInit(priv);
#endif
	}
#endif

#if defined(DFS) && !defined(RTK_NL80211)
		if (!priv->pmib->dot11DFSEntry.disable_DFS &&
			(OPMODE & WIFI_AP_STATE) &&
			(((priv->pmib->dot11RFEntry.dot11channel >= 52) &&
			(priv->pmib->dot11RFEntry.dot11channel <= 64)) || 
			((priv->pmib->dot11RFEntry.dot11channel >= 100) &&
			(priv->pmib->dot11RFEntry.dot11channel <= 140)))) {

			init_timer(&priv->ch_avail_chk_timer);
			priv->ch_avail_chk_timer.data = (unsigned long) priv;
			priv->ch_avail_chk_timer.function = rtl8192cd_ch_avail_chk_timer;

			if ((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) &&
				(IS_METEOROLOGY_CHANNEL(priv->pmib->dot11RFEntry.dot11channel)))
				mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO_CE);
			else
				mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO);

			init_timer(&priv->DFS_timer);
			priv->DFS_timer.data = (unsigned long) priv;
			priv->DFS_timer.function = rtl8192cd_DFS_timer;

			init_timer(&priv->DFS_TXPAUSE_timer);
			priv->DFS_TXPAUSE_timer.data = (unsigned long) priv;
			priv->DFS_TXPAUSE_timer.function = rtl8192cd_DFS_TXPAUSE_timer;

			/* DFS activated after 5 sec; prevent switching channel due to DFS false alarm */
			mod_timer(&priv->DFS_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));

			init_timer(&priv->dfs_det_chk_timer);
			priv->dfs_det_chk_timer.data = (unsigned long) priv;
			priv->dfs_det_chk_timer.function = rtl8192cd_dfs_det_chk_timer;

			mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

			DFS_SetReg(priv);

			if (!priv->pmib->dot11DFSEntry.CAC_enable) {
				del_timer_sync(&priv->ch_avail_chk_timer);
				mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
			}
		}else if(priv->pmib->dot11DFSEntry.disable_tx)
			/* Recovery DFS disable_tx if there is no need to do DFS */
			priv->pmib->dot11DFSEntry.disable_tx = 0;
#endif

#ifdef SUPPORT_SNMP_MIB
		mib_init(priv);
#endif
#if defined(CONFIG_RTL_8196E)
   	    if(get_96e_bonding_type() == BOND_8196ES ) 
    		rtl_8196es_gpio_init(); 		
#endif
	}


#if	defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
	{
		memset(brsc_cache_arr,0,sizeof(struct brsc_cache_t)*MAX_BRSC_NUM);		
	}
#endif

	//if (OPMODE & WIFI_AP_STATE)  //in case of station mode, queue will start only after assoc.
#if defined(CONFIG_PCI_HCI) || !defined(CONFIG_NETDEV_MULTI_TX_QUEUE)
		netif_start_queue(dev);		// start queue always
#else
		netif_tx_start_all_queues(dev);
#endif
		
#ifdef WDS
	create_wds_tbl(priv);
#endif

#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->reset_hangup = 0;
#endif

#if defined(INCLUDE_WPA_PSK) && defined(CLIENT_MODE) && !defined(RTK_NL80211)
	if (OPMODE & WIFI_ADHOC_STATE)
		if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
			ToDrv_SetGTK(priv);
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv))
#endif
	if ((OPMODE & WIFI_AP_STATE) && priv->auto_channel) {
		if (((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _TKIP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _CCMP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_WPA_MIXED_PRIVACY_)) ||
			 (priv->pmib->dot11RsnIE.rsnielen > 0)) {
			priv->ss_ssidlen = 0;
			DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=0\n", (char *)__FUNCTION__);
#ifdef CONFIG_RTL8672
			OPMODE_VAL(OPMODE | WIFI_WAIT_FOR_CHANNEL_SELECT);
#endif
			start_clnt_ss(priv);
		}
	}

#ifdef CLIENT_MODE
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
		struct net_bridge_port *br_port;
		
#ifdef RTK_BR_EXT
#ifdef __ECOS
		memcpy(priv->br_mac, GET_MY_HWADDR, MACADDRLEN);
#else
		br_port = GET_BR_PORT(priv->dev);

		if (br_port) {
#if defined(__LINUX_2_6__) || defined(__OSK__)
			memcpy(priv->br_mac, br_port->br->dev->dev_addr, MACADDRLEN);
#else
			memcpy(priv->br_mac, priv->dev->br_port->br->dev.dev_addr, MACADDRLEN);
#endif
		}
#endif
#endif
#ifdef MBSSID
	if (IS_VAP_INTERFACE(priv)) {
		if((priv->pmib->miscEntry.vap_enable)&&(OPMODE == WIFI_STATION_STATE)) {
            if(
#if defined(CUSTOMIZE_WLAN_IF_NAME)
				!strcmp(priv->dev->name, WLAN0_VIRTUAL2_IFNAME)||!strcmp(priv->dev->name, WLAN1_VIRTUAL2_IFNAME)
#else
				!strcmp(priv->dev->name,"wlan0-va1")||!strcmp(priv->dev->name,"wlan1-va1")
#endif
				){
                STADEBUG("open ,start ss_timer\n");
    			mod_timer(&priv->ss_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));
            }else if(
#if defined(CUSTOMIZE_WLAN_IF_NAME)
			!strcmp(priv->dev->name, WLAN0_VIRTUAL3_IFNAME)||!strcmp(priv->dev->name,WLAN1_VIRTUAL3_IFNAME)
#else
            !strcmp(priv->dev->name,"wlan0-va2")||!strcmp(priv->dev->name,"wlan1-va2")
#endif
            ){
                STADEBUG("open ,start ss_timer\n");            
    			mod_timer(&priv->ss_timer, jiffies + RTL_SECONDS_TO_JIFFIES(15));
            }
		}
	}else
#endif
#if defined(SMART_REPEATER_MODE) && !defined(RTK_NL80211)
//	if (IS_VXD_INTERFACE(priv) && !GET_ROOT_PRIV(priv)->pmib->miscEntry.func_off) 
	if (IS_VXD_INTERFACE(priv))         
    {
		if (OPMODE == WIFI_STATION_STATE) {
			#if	0	//def DFS
			if (GET_ROOT(priv)->pmib->dot11DFSEntry.CAC_enable && (timer_pending(&GET_ROOT(priv)->ch_avail_chk_timer) ||
				((GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel == 0) &&
				 (GET_ROOT(priv)->pmib->dot11RFEntry.band5GSelected & PHY_BAND_5G_1) == 0))) {
				if ((GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) &&
					(IS_METEOROLOGY_CHANNEL(priv->pmib->dot11RFEntry.dot11channel)))
 #if defined(CONFIG_RTL_SIMPLE_CONFIG)               
					if(priv->pmib->dot11StationConfigEntry.sc_enabled && priv->simple_config_status>0)
					{
						mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(1500)); 			   
					}
					else
						mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000) + CH_AVAIL_CHK_TO_CE);
#else
					mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000) + CH_AVAIL_CHK_TO_CE);
#endif
				else
 #if defined(CONFIG_RTL_SIMPLE_CONFIG)               
					if(priv->pmib->dot11StationConfigEntry.sc_enabled  && priv->simple_config_status>0)
					{
						mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(1500)); 			   
					}
					else
						mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000) + CH_AVAIL_CHK_TO);
#else
					mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000) + CH_AVAIL_CHK_TO);
#endif
			}
			else
#endif
            {
#ifdef CONFIG_RTK_MESH
                if(netif_running(GET_ROOT(priv)->dev) &&
                    GET_ROOT(priv)->pmib->dot1180211sInfo.mesh_enable && GET_ROOT(priv)->auto_channel)
                {   /*if mesh is doing auto channel scanning, delay 15 sec*/        
                    mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(15000)); 
                }
                else                 
#endif
                {
 #if defined(CONFIG_RTL_SIMPLE_CONFIG)               
					if(priv->pmib->dot11StationConfigEntry.sc_enabled && priv->simple_config_status>0)
					{
						mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(1500));                
					}
					else
					    mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000));                
#else
				    mod_timer(&priv->ss_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000));                
#endif
                }
            }
		}
	} else 
#endif	
	{
        	/*pure client mode*/ 
			if (!IEEE8021X_FUN || (IEEE8021X_FUN && (priv->pmib->dot11RsnIE.rsnielen > 0))) {
#ifdef CHECK_HANGUP
				if (!is_reset || JOIN_RES == STATE_Sta_No_Bss ||
						JOIN_RES == STATE_Sta_Roaming_Scan || JOIN_RES == 0)
#endif
				{
#ifdef CHECK_HANGUP
					if (is_reset)
						OPMODE_VAL(OPMODE & ~WIFI_SITE_MONITOR);
#endif

#if !defined(WIFI_WPAS_CLI) && !defined(RTK_NL80211)
					start_clnt_lookup(priv, RESCAN);
#endif
#if defined(CONFIG_AUTH_RESULT)
					priv->authRes = 0;
#endif

				}
			}
		}
	}
#endif


#ifdef RTK_NL80211 // wrt-adhoc
	if (IS_VXD_INTERFACE(priv) && (OPMODE & WIFI_ADHOC_STATE))         
    {
#ifdef __ECOS
		init_timer(&priv->pshare->vxd_ibss_beacon, (unsigned long)priv, issue_beacon_ibss_vxd);
#else
		init_timer(&priv->pshare->vxd_ibss_beacon);
		priv->pshare->vxd_ibss_beacon.data = (unsigned long) priv;
		priv->pshare->vxd_ibss_beacon.function = issue_beacon_ibss_vxd;
#endif   
		//mod_timer(&priv->pshare->vxd_ibss_beacon, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000));	
	} 
#endif	

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv) &&
#ifdef __ECOS
		GET_VXD_PRIV(priv) &&
#endif
		netif_running(GET_VXD_PRIV(priv)->dev)) {
		SAVE_INT_AND_CLI(x);
		rtl8192cd_open(GET_VXD_PRIV(priv)->dev);
		RESTORE_INT(x);
	}
	if (IS_VXD_INTERFACE(priv) &&
		(GET_ROOT(priv)->pmib->dot11OperationEntry.opmode&WIFI_STATION_STATE) &&
		(GET_ROOT(priv)->pmib->dot11OperationEntry.opmode&WIFI_ASOC_STATE) &&
#ifdef RTK_BR_EXT
		!(GET_ROOT(priv)->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed) &&
#endif
		!(priv->drv_state & DRV_STATE_VXD_AP_STARTED) )
		enable_vxd_ap(priv);
#endif

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
#ifndef RTK_NL80211
		if (priv->pmib->miscEntry.vap_enable)
#endif
		{
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (netif_running(priv->pvap_priv[i]->dev))
					rtl8192cd_open(priv->pvap_priv[i]->dev);
			}
		}
	}
#endif

#ifdef PCIE_POWER_SAVING
#ifdef CHECK_HANGUP
	if(!is_reset)
#endif
#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL8672)
		if (1)
#else
		if (REG32(REVR) == RTL8196C_REVISION_B)
#endif
			init_pcie_power_saving(priv);
		else
			priv->pshare->rf_ft_var.power_save = 0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8192EE	
	if (GET_CHIP_VER(priv) == VERSION_8192E) {
#if defined(UNIVERSAL_REPEATER) 
		if (IS_ROOT_INTERFACE(priv))
#endif		
		if (((OPMODE & WIFI_AP_STATE) &&
			((priv->pshare->CurrentChannelBW==0 && priv->pmib->dot11RFEntry.dot11channel==13) 
			||(priv->pshare->CurrentChannelBW==1 && priv->pmib->dot11RFEntry.dot11channel>=11)))
#ifdef MP_TEST
			|| ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)	
#endif
#ifdef CLIENT_MODE
			|| (OPMODE & WIFI_STATION_STATE) 
#endif
		) 
		{
			Check_92E_Spur_Valid(priv, false);	
		}
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef CONFIG_USB_HCI
		rtl8192cu_inirp_init(priv);
#endif

#if 0
#if defined(CONFIG_SDIO_HCI) || (defined(CONFIG_PCI_HCI) && (defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8197F)))
#ifdef EN_EFUSE
		if(IS_D_CUT_8192E(priv) || GET_CHIP_VER(priv) == VERSION_8197F)
			if (priv->pmib->dot11RFEntry.kfree_enable)
				power_trim(priv, 0);
			else			
				panic_printk("Kfree not enabled\n");
#endif
#endif
#endif
#if defined(CONFIG_SDIO_HCI)
#ifdef __ECOS
		priv->recvpriv.recv_tasklet= 1;
		priv->pshare->xmit_tasklet = 1;
		priv->pshare->has_triggered_rx_tasklet = 0;
		priv->pshare->has_triggered_tx_tasklet = 0;
#endif

		EnableSdioInterrupt(priv);
#endif
	}

#if 0   //defined(MULTI_MAC_CLONE)
	if (netif_running(GET_ROOT(priv)->pvxd_priv->dev) && (GET_ROOT(priv)->pvxd_priv->drv_state&DRV_STATE_VXD_INIT)) {
		priv->pshare->root_repeater = GET_ROOT(priv)->pvxd_priv;
	}
	else {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {	
			if (GET_ROOT(priv)->pvap_priv[i]!=NULL &&
				(GET_ROOT(priv)->pvap_priv[i]->pmib->dot11OperationEntry.opmode&WIFI_STATION_STATE) &&
				netif_running(GET_ROOT(priv)->pvap_priv[i]->dev)) {
				priv->pshare->root_repeater = GET_ROOT(priv)->pvap_priv[i];
				break;
			}
			priv->pshare->root_repeater = NULL;
		}
	}
#endif
#ifdef UNIVERSAL_REPEATER
#if defined(CUSTOMIZE_WLAN_IF_NAME)
    if(!strcmp(priv->dev->name,WLAN0_IFNAME) || !strcmp(priv->dev->name,WLAN0_VIRTUAL1_IFNAME) ||  !strcmp(priv->dev->name, WLAN0_REPEATER_IFNAME))
#else
    if(!strcmp(priv->dev->name,"wlan0") || !strcmp(priv->dev->name,"wlan0-va1") ||  !strcmp(priv->dev->name,"wlan0-vxd"))
#endif		
	{		
        priv->reperater_idx=1;
        #if defined(BR_SHORTCUT) && defined(CLIENT_MODE)
        if (OPMODE & WIFI_STATION_STATE) {
            cached_sta_dev[0] = NULL;
        }
        #endif        
    }
#if defined(CUSTOMIZE_WLAN_IF_NAME)
	else if(!strcmp(priv->dev->name,WLAN0_VIRTUAL1_IFNAME) || !strcmp(priv->dev->name,WLAN0_VIRTUAL3_IFNAME))
#else
	else if(!strcmp(priv->dev->name,"wlan0-va0") || !strcmp(priv->dev->name,"wlan0-va2"))
#endif
	{
        priv->reperater_idx=2;        
        #if defined(BR_SHORTCUT) && defined(CLIENT_MODE)
        if (OPMODE & WIFI_STATION_STATE) {
            cached_sta_dev[1] = NULL;
        }
        #endif                
    }
#endif

#ifdef RTK_SMART_ROAMING
	//Add timer list 
	timer_ready(priv);

	//Accept Destination Address packets.
	RTL_W32(RCR, RTL_R32(RCR) | RCR_AAP); 

#endif

#if defined(CONFIG_RTL_SIMPLE_CONFIG)
	rtk_sc_init(priv);
#endif
#ifdef SDIO_AP_OFFLOAD
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	} else
#endif
	{
#ifdef USE_WAKELOCK_MECHANISM
		rtw_lock_suspend_timeout(priv, 2000);
#endif
	}
#endif // SDIO_AP_OFFLOAD

	DBFEXIT;

	return 0;
	
free_res:
	
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		rtw_dev_unload(priv);
#endif
#if defined(CONFIG_TCP_ACK_TXAGG) || defined(CONFIG_XMITBUF_TXAGG_ADV)
		if (timer_pending(&priv->pshare->xmit_check_timer))
			del_timer_sync(&priv->pshare->xmit_check_timer);
#endif
#ifdef CONFIG_USB_HCI
#if defined(CONFIG_RTL_92C_SUPPORT) || (!defined(CONFIG_SUPPORT_USB_INT) || !defined(CONFIG_INTERRUPT_BASED_TXBCN))
		if (timer_pending(&priv->pshare->beacon_timer))
			del_timer_sync(&priv->pshare->beacon_timer);
#endif
#endif // CONFIG_USB_HCI
#ifdef CONFIG_SDIO_HCI
		if (timer_pending(&priv->pshare->beacon_timer))
			del_timer_sync(&priv->pshare->beacon_timer);
#endif
#if defined(MULTI_STA_REFINE)
		if (timer_pending(&priv->pshare->PktAging_timer))
			del_timer_sync(&priv->pshare->PktAging_timer);
#endif
#ifdef USE_OUT_SRC
		ODM_StopAllThreads(ODMPTR);
#endif
	}

#ifdef SDIO_AP_OFFLOAD
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	} else
#endif
	{
		// disable "unlock suspend" to avoid card missing issue in Sequans platform
#if 0//def USE_WAKELOCK_MECHANISM
		rtw_lock_suspend_timeout(priv, 2000);
#endif
	}
#endif // SDIO_AP_OFFLOAD
	
	return rc;
}

#ifdef RTK_NL80211
int rtl_setMac(struct net_device *dev, char *mac)
{
#ifdef NETDEV_NO_PRIV
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;
#else
	struct rtl8192cd_priv	*priv = (struct rtl8192cd_priv *)dev->priv;
#endif

	int vap_idx = -1;

	if((!priv) || (!priv->rtk))
		return -1;

	//printk("[%s][%s] %02x:%02x:%02x:%02x:%02x:%02x \n", __FUNCTION__, dev->name, 
		//mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if((strcmp(dev->name, "wlan0")==0) || (strcmp(dev->name, "wlan1")==0)) {
		memcpy(priv->rtk->root_mac, mac, 6);
		return 0;
	}
	
	if((strcmp(dev->name, "wlan0-1")==0) || (strcmp(dev->name, "wlan1-1")==0))
			vap_idx = 0;
	else if((strcmp(dev->name, "wlan0-2")==0) || (strcmp(dev->name, "wlan1-2")==0))
			vap_idx = 1;
	else if((strcmp(dev->name, "wlan0-3")==0) || (strcmp(dev->name, "wlan1-3")==0))
			vap_idx = 2;
	else if((strcmp(dev->name, "wlan0-4")==0) || (strcmp(dev->name, "wlan1-4")==0))
			vap_idx = 3;
	else if((strcmp(dev->name, "wlan0-5")==0) || (strcmp(dev->name, "wlan1-5")==0))
			vap_idx = 4;
	else if((strcmp(dev->name, "wlan0-6")==0) || (strcmp(dev->name, "wlan1-6")==0))
			vap_idx = 5;
	else if((strcmp(dev->name, "wlan0-7")==0) || (strcmp(dev->name, "wlan1-7")==0))
			vap_idx = 6;

	if((vap_idx >= 0) && (vap_idx < (RTL8192CD_NUM_VWLAN-1))) {
		memcpy(priv->rtk->vap_mac[vap_idx], mac, 6);
		return 0;

	}

	return -1;
}
#endif

int  rtl8192cd_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags = 0;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	unsigned long reg;
	unsigned char *p, zero_mac[MACADDRLEN]={0};
#ifdef WDS
	int i;
#endif
#ifdef  CONFIG_WLAN_HAL
    BOOLEAN     bVal;
#endif  //CONFIG_WLAN_HAL


#ifdef __KERNEL__
	p = ((struct sockaddr *)addr)->sa_data;
#else
	p = (unsigned char *)addr;
#endif

#ifdef RTK_NL80211 
	rtl_setMac(dev, p);
#endif

	if(!memcmp(zero_mac,p,MACADDRLEN)) {
		panic_printk("\nWARNING!! Tend to configure %s with zero MAC address!!\n",priv->dev->name);

		if(IS_DRV_OPEN(priv)) {
            if(IS_ROOT_INTERFACE(priv)) {
				int intf_count=0;
				panic_printk("Shut wireless down!!\n");
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
                for(intf_count=0;intf_count<RTL8192CD_NUM_VWLAN;intf_count++) {
					if(IS_DRV_OPEN(priv->pvap_priv[intf_count]))
						priv->pvap_priv[intf_count]->dev->flags &= ~IFF_UP;
				}
#if defined(UNIVERSAL_REPEATER)
          if(IS_DRV_OPEN(priv->pvxd_priv)) {
						priv->pvxd_priv->dev->flags &= ~IFF_UP;
				}
#endif				
#endif				
			} else {
				panic_printk("Shut %s down!!\n",priv->dev->name);
			}
			priv->dev->flags &= ~IFF_UP;
			rtl8192cd_close(priv->dev);
		}

		return 0;
	}

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

#ifdef MULTI_MAC_CLONE
	ACTIVE_ID = 0;
#endif

	memcpy(priv->dev->dev_addr, p, 6);
	memcpy(GET_MY_HWADDR, p, 6);
#if defined(RTK_NL80211)
	//brian, for setup MAC address from calibration at flash only during system initialization
	if(IS_ROOT_INTERFACE(priv) && !IS_DRV_OPEN(priv)) {
		memcpy(priv->rtk->root_mac, p, MACADDRLEN);
		memcpy(priv->rtk->wiphy->perm_addr, p, MACADDRLEN);
	}
#endif
#ifndef __ECOS
    memset(dev->broadcast, 0xff, ETH_ALEN);
#endif
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv)) {
			RESTORE_INT(flags);
			SMP_UNLOCK(flags);
			return 0;
		}
	}
#endif

#ifdef WDS
	for (i=0; i<NUM_WDS; i++)
		if (priv->wds_dev[i])
			memcpy(priv->wds_dev[i]->dev_addr, p, 6);
#endif
#ifdef CONFIG_RTK_MESH
	if(NUM_MESH>0)
		if (priv->mesh_dev)
			memcpy(priv->mesh_dev->dev_addr, p, 6);
#endif

#ifdef SDIO_AP_OFFLOAD
	ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
#endif

#ifdef UNIVERSAL_REPEATER
#ifdef SDIO_2_PORT
	if (IS_VXD_INTERFACE(priv)) {
		reg = *(unsigned long *)(dev->dev_addr);
                RTL_W32(MACID1, (cpu_to_le32(reg)));
                reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
                RTL_W16(MACID1+4, (cpu_to_le16(reg)));

		goto out;
	}
#else // !SDIO_2_PORT
	if (IS_ROOT_INTERFACE(priv)) {
		if (GET_VXD_PRIV(priv)) {
			memcpy(GET_VXD_PRIV(priv)->dev->dev_addr, p, 6);
			memcpy(GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.hwaddr, p, 6);
		}
	}
	else if (IS_VXD_INTERFACE(priv)) {
		memcpy(GET_ROOT(priv)->dev->dev_addr, p, 6);
		memcpy(GET_ROOT(priv)->pmib->dot11OperationEntry.hwaddr, p, 6);
	}
#endif // SDIO_2_PORT
#endif // UNIVERSAL_REPEATER

#ifdef     CONFIG_WLAN_HAL 
	if (IS_HAL_CHIP(priv)) {
	    #ifdef CONFIG_WLAN_HAL_8197F		
		if(!IS_DRV_OPEN(priv)) {
			RESTORE_INT(flags);
			SMP_UNLOCK(flags);
			return;
		}
	    #endif		
	    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, (pu1Byte)&bVal);
	    if ( bVal ) {
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)p);
	    }
	    else {
//        printk("%s(%d): Can't write MACID register\n", __FUNCTION__, __LINE__);
	    }
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
	    if (check_MAC_IO_Enable(priv)) {
	        reg = *(unsigned long *)(dev->dev_addr);
	        RTL_W32(MACID, (cpu_to_le32(reg)));
	        reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
	        RTL_W16(MACID+4, (cpu_to_le16(reg)));
	#ifdef CONFIG_RTL_92D_SUPPORT
		if (GET_CHIP_VER(priv) == VERSION_8192D) {
			RTL_W32(MACID1 , RTL_R32(MACID));
			RTL_W16(MACID1+4, RTL_R16(MACID+4));
		}
	#endif
		}
	}
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (OPMODE & WIFI_AP_STATE) {
#ifdef     CONFIG_WLAN_HAL 
			if (IS_HAL_CHIP(priv)) {
				if ( bVal ) 
		            GET_HAL_INTERFACE(priv)->InitMBSSIDHandler(priv);
			}
			else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			rtl8192cd_init_mbssid(priv);

		}
		else if (IS_VAP_INTERFACE(priv) && (OPMODE & WIFI_STATION_STATE)) {
#ifdef     CONFIG_WLAN_HAL 
			if (IS_HAL_CHIP(priv)) {
				if ( bVal ) 
		            GET_HAL_INTERFACE(priv)->InitMBIDCAMHandler(priv);
			}
			else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
				; //ToDo
		}
	}
#endif

out:	
	RESTORE_INT(flags);
	SMP_UNLOCK(flags);

#ifndef WIFI_WPAS_CLI
#ifdef CLIENT_MODE
	if (!(OPMODE & WIFI_AP_STATE) && netif_running(priv->dev)) {
		int link_status = chklink_wkstaQ(priv);
		if (link_status) {
#ifdef MULTI_MAC_CLONE
			OPMODE_VAL(WIFI_STATION_STATE);
			JOIN_RES_VAL(STATE_Sta_No_Bss);
#if defined(CONFIG_RTL8672) && !defined(__OSK__)
			wlan_sta_state_notify(priv, STATE_Sta_No_Bss);
#endif
			start_clnt_auth(priv);				
#else			
			start_clnt_join(priv);
#endif
		}
	}
#endif
#endif

#ifdef SDIO_AP_OFFLOAD
	ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
#endif
	
	return 0;
}


int rtl8192cd_close(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
#ifdef UNIVERSAL_REPEATER
	struct rtl8192cd_priv *priv_vxd;
#endif
#ifdef CONFIG_WLAN_HAL
    unsigned int errorFlag=0;
#endif	
	unsigned long flags=0;

	int i;

#ifdef RTK_NL80211
#if defined(MBSSID) || defined(UNIVERSAL_REPEATER)
	#if 0	//prevent drop vxd, vap connection
	if(IS_ROOT_INTERFACE(priv))
		close_vxd_vap(priv);
	#endif

	if(is_WRT_scan_iface(dev->name))
	{
		//printk("No need to call close for scan iface !!\n");
		return 0;
	}
#endif
#endif

    STADEBUG("===>\n");
	SMP_LOCK(flags);
	DBFENTER;

#if defined(WDS) || defined(CONFIG_RTK_MESH)
    if(dev->base_addr != 0 && dev->base_addr != 1)
#endif
    {
    	if (!(priv->drv_state & DRV_STATE_OPEN))
        {
		DBFEXIT;
		SMP_UNLOCK(flags);
		return 0;
	}
    }

	SAVE_INT_AND_CLI(flags);

	if (OPMODE & WIFI_SITE_MONITOR) {
		SwChnl(GET_ROOT(priv), GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel, GET_ROOT(priv)->pshare->offset_2nd_chan);
	}

#ifdef MULTI_MAC_CLONE
	ACTIVE_ID = 0;
#endif

#if defined(WDS) || defined(CONFIG_RTK_MESH)
    if(dev->base_addr != 0 && dev->base_addr != 1)
#endif
		priv->drv_state &= ~DRV_STATE_OPEN;     // set driver as has been closed, david

#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
	if (timer_pending(&priv->ps_timer)) {
		SMP_UNLOCK(flags);
		del_timer_sync(&priv->ps_timer);
		SMP_LOCK(flags);
	}
#endif	
#ifdef PCIE_POWER_SAVING
	if((priv->pwr_state == L1) || (priv->pwr_state == L2)) {
		PCIeWakeUp(priv, (POWER_DOWN_T0<<3));
	}
#endif

#ifdef SDIO_AP_OFFLOAD
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	} else
#endif
#ifdef WDS
	if (dev->base_addr)
#endif
	{
#ifdef CONFIG_POWER_SAVE
		rtw_lock_suspend(priv);
#endif
		del_timer_sync(&priv->pshare->ps_timer);

		if (RTW_STS_SUSPEND == priv->pshare->pwr_state)
			ap_offload_exit(priv);
	}
#endif // SDIO_AP_OFFLOAD

#ifdef RTK_NL80211//survey_dump
	/*
	priv->rtk->tx_time = 0;
	priv->rtk->rx_time = 0;
	priv->rtk->chbusytime = 0;
	*/
	memset(priv->rtk->survey_info, 0x0, ARRAY_SIZE(realtek_5ghz_a_channels));
	memset(priv->rtk->psd_fft_info, 0x0, sizeof(priv->rtk->psd_fft_info));
#endif

#if 0//defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
    rtl8192cd_cu_stop(priv);
#endif

#ifdef HS2_SUPPORT
/* Hotspot 2.0 Release 1 */
	//clean hs2 relative ie
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.hs2_ielen)
    {
		priv->pmib->hs2Entry.hs2_ielen = 0;	
	}	
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.interworking_ielen)
    {
		priv->pmib->hs2Entry.interworking_ielen = 0;
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.advt_proto_ielen)
	{
		priv->pmib->hs2Entry.advt_proto_ielen = 0;
	}	
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.roam_ielen)
	{
		priv->pmib->hs2Entry.roam_ielen = 0;
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.timeadvt_ielen)
	{
		priv->pmib->hs2Entry.timeadvt_ielen = 0;
	}
	if ((OPMODE & WIFI_AP_STATE) && priv->pmib->hs2Entry.timezone_ielen)
	{
		priv->pmib->hs2Entry.timezone_ielen = 0;
	}	
#endif

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	recalc_txdesc_limit(priv);
#endif
#ifdef USE_TXQUEUE
	recalc_tx_bufq_limit(priv);
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv))
	{
		priv_vxd = GET_VXD_PRIV(priv);

		// if vxd interface is opened, close it first
		if (IS_DRV_OPEN(priv_vxd)) {
			SMP_UNLOCK(flags);
			rtl8192cd_close(priv_vxd->dev);
			SMP_LOCK(flags);
		}
	}
	else
	{
#ifdef MBSSID
/*
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
*/
		if (priv->vap_id < 0)
#endif
		{
			disable_vxd_ap(priv);
#ifdef SDIO_2_PORT
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT1)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT1));
#endif
		}
	}
#endif

#if defined(CONFIG_PCI_HCI) || !defined(CONFIG_NETDEV_MULTI_TX_QUEUE)
	netif_stop_queue(dev);
#else
	netif_tx_stop_all_queues(dev);
#endif

#ifdef CONFIG_PUMA_UDMA_SUPPORT
	#ifdef CONCURRENT_MODE
		udma_flush(priv->pshare->wlandev_idx);
	#else
		udma_flush(0);
	#endif
#endif

#ifdef WDS
	if (dev->base_addr == 0)
	{
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
		DBFEXIT;
		return 0;
	}
#endif

#ifdef CONFIG_RTK_MESH
    if (dev->base_addr == 1)
    {
        mesh_close(GET_ROOT(priv));
        RESTORE_INT(flags);
        SMP_UNLOCK(flags);
        return 0;
    }
#endif // CONFIG_RTK_MESH

#ifdef RTK_SMART_ROAMING
	timer_del(priv);
#endif

#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	{
		if (OPMODE & WIFI_AP_STATE) {
			int i;
			for(i=0; i<NUM_STAT; i++)
			{
				if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)
#ifdef WDS
					&& !(priv->pshare->aidarray[i]->station.state & WIFI_WDS)
#endif
				) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
					if (priv != priv->pshare->aidarray[i]->priv)
						continue;
#endif
					issue_deauth(priv, priv->pshare->aidarray[i]->station.hwaddr, _RSON_DEAUTH_STA_LEAVING_);
				}
			}

			delay_ms(10);
		}
#ifdef CLIENT_MODE	/* WPS2DOTX   */
		else if ((OPMODE & WIFI_STATION_STATE) == WIFI_STATION_STATE){
			if ((OPMODE & (WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE))==(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE)){
				//issue_disassoc(priv,BSSID,_RSON_DEAUTH_STA_LEAVING_);
				//OS_DEBUG("issue_deauth to AP\n");
				//printMac(BSSID);
				issue_deauth(priv,BSSID,_RSON_DEAUTH_STA_LEAVING_);			
				delay_ms(50);//make sure before issue_disassoc then TX be close		
				OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE));
			}

#ifdef MULTI_MAC_CLONE
			if (MCLONE_NUM > 0) {
				for (i=0; i<priv->pshare->mclone_num_max; i++) {
					ACTIVE_ID = i+1;
					if (priv->pshare->mclone_sta[i].priv==priv) {
						DEBUG_INFO("[%s] mclone_sta[%d]\n", __func__, i);
						if ((OPMODE & (WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE))==(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE)) {
							issue_deauth(priv, BSSID, _RSON_DEAUTH_STA_LEAVING_);
							delay_ms(50);//make sure before issue_disassoc then TX be close		
							OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS|WIFI_ASOC_STATE));	
						}

						#ifdef  CONFIG_WLAN_HAL
						if (IS_HAL_CHIP(priv))
			            	GET_HAL_INTERFACE(priv)->McloneStopMBSSIDHandler(priv, i);
						else
						#endif
							mclone_stop_mbssid(priv, i);
						clear_Multi_Mac_Clone(priv, i);
						priv->pshare->mclone_sta[i].isTimerInit = 0;
					}
				}		
				ACTIVE_ID = 0;

				if (MCLONE_NUM == 0)
					priv->pshare->mclone_ok = 0;
			} else {
			    priv->pshare->mclone_ok = 0;
			}
			
			
            #if 0
			if (netif_running(GET_ROOT(priv)->pvxd_priv->dev) && (GET_ROOT(priv)->pvxd_priv->drv_state&DRV_STATE_VXD_INIT)) {
				priv->pshare->root_repeater = GET_ROOT(priv)->pvxd_priv;
			}
			else {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {	
					if (GET_ROOT(priv)->pvap_priv[i]!=NULL &&
						(GET_ROOT(priv)->pvap_priv[i]->pmib->dot11OperationEntry.opmode&WIFI_STATION_STATE) &&
						netif_running(GET_ROOT(priv)->pvap_priv[i]->dev)) {
						priv->pshare->root_repeater = GET_ROOT(priv)->pvap_priv[i];
						break;
					}
					priv->pshare->root_repeater = NULL;
				}
			}
            #endif
#endif
		}
#endif    /* WPS2DOTX   */
	}

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)) {
		if (priv->pmib->miscEntry.vap_enable) {
			SMP_UNLOCK(flags);
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					rtl8192cd_close(priv->pvap_priv[i]->dev);
			}
			SMP_LOCK(flags);
		}
	}

	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
#ifdef SDIO_2_PORT
		if (IS_VXD_INTERFACE(priv))
			rtl8192cd_clear_mbidcam(priv, 7);
#endif
		stop_mbssid(priv);

#ifdef CONFIG_PCI_HCI
		if(is_last_vap_iface(priv))
		{
			//stop_mbssid(GET_ROOT(priv));
			refine_TBTT(GET_ROOT(priv));
#if defined(RTK_NL80211)
			GET_ROOT(priv)->pmib->miscEntry.vap_enable = 0;
#endif
		}
#endif
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#if !(defined(__ECOS) && defined(CONFIG_SDIO_HCI))
#ifdef RTK_QUE
		free_rtk_queue(priv, &priv->pshare->skb_queue);
#else
		free_skb_queue(priv, &priv->pshare->skb_queue);
#endif
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		rtw_dev_unload(priv);
#endif

#if 1 //kill tasklets before stop hw
#ifdef DFS
		/* prevent killing tasklet issue in interrupt */
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
#endif
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
				{
#ifndef __OSK__	
#ifdef SMART_REPEATER_MODE
					if (!priv->pshare->switch_chan_rp)
#endif
					{				
#ifdef CONFIG_RTL_SIMPLE_CONFIG
						if(!priv->pmib->dot11StationConfigEntry.sc_enabled && ((priv->simple_config_status==0) && (IS_ROOT_INTERFACE(priv)
#ifdef UNIVERSAL_REPEATER
 && (GET_VXD_PRIV(priv)->simple_config_status == 0)
#endif
)))
#endif

						{

                    
#if defined(CONFIG_PCI_HCI)
						RESTORE_INT(flags);
						SMP_UNLOCK(flags);
#ifdef __ECOS
						priv->pshare->rx_tasklet = 0;
						priv->pshare->tx_tasklet = 0;
						priv->pshare->has_triggered_rx_tasklet = 0;
						priv->pshare->has_triggered_tx_tasklet = 0;
#else
						tasklet_kill(&priv->pshare->rx_tasklet);
						tasklet_kill(&priv->pshare->tx_tasklet);
						tasklet_kill(&priv->pshare->oneSec_tasklet);
#endif
						SMP_LOCK(flags);
						SAVE_INT_AND_CLI(flags);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#ifdef __ECOS
						priv->recvpriv.recv_tasklet = 0;
						priv->pshare->xmit_tasklet = 0;
						priv->pshare->has_triggered_rx_tasklet = 0;
						priv->pshare->has_triggered_tx_tasklet = 0;
#else
						tasklet_kill(&priv->recvpriv.recv_tasklet);
						tasklet_kill(&priv->pshare->xmit_tasklet);
#endif
#endif
					}
				}
#endif //__OSK__
				}

			}
		}
#endif //kill tasklets before stop hw


#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {

            //Check Error Flag
            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
            if (errorFlag != 0x0) {

                if(!((GET_CHIP_VER(priv) == VERSION_8822B)&&(errorFlag == 0xffff0000))) //8822 default value, shall not print
                panic_printk("Error Flag: 0x%x\n", errorFlag);
            }
                       
        	if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                DEBUG_INFO("StopHW Succeed\n");
        	}
            else {
                #if 0 //Filen, we can't dump event after StopHW. except dump register is at PON section
                GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                errorFlag |= DRV_ER_CLOSE_STOP_HW;
                GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                #endif
                panic_printk("StopHW Failed\n");
            }        
		}
        else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			rtl8192cd_stop_hw(priv);
		}

#ifdef RTLWIFINIC_GPIO_CONTROL 
	if(GET_CHIP_VER(priv) == VERSION_8822B) {
		RTL_W8(0x1c, (RTL_R8(0x1c)|BIT(5)));
		RTL_W8(0x1c, (RTL_R8(0x1c)|BIT(6)));
	}
#endif

#if defined(__KERNEL__) || defined(__OSK__)
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
		{
			SMP_UNLOCK(flags);
			free_irq(dev->irq, dev);
			SMP_LOCK(flags);
#ifdef PCIE_POWER_SAVING
#ifdef GPIO_WAKEPIN			
#ifdef RTL8676_WAKE_GPIO
		{
			int gpio_num, irq_num;

			get_wifi_wake_pin(&gpio_num);
			irq_num = gpioGetBspIRQNum(gpio_num);				
			REG32(BSP_GIMR) &= ~ BIT(irq_num);

			free_irq(irq_num, dev);
			gpioClearISR(gpio_num); // clear GPIO interrupt status
		}
	#else

#if defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
			REG32(0xb8003000) &= ~ BIT(16);		// GIMR
#else
			if (REG32(REVR) == RTL8196C_REVISION_B) 	
				REG32(0xb8003000) &= ~ BIT(9);		// GIMR
#endif
#if defined(__LINUX_2_6__)
			free_irq(BSP_GPIO_ABCD_IRQ, dev);
#else
			free_irq(1, dev);
#endif	

#endif	
#endif
#endif
#if 0 //yllin //#ifdef GPIO_WAKEPIN
#if defined(__LINUX_2_6__)
            int h1irq = gpio_to_irq(BSP_GPIO_PIN_H1);
            free_irq(h1irq, dev);
#else
            free_irq(2, dev);
#endif
#endif
		}			
#endif
#endif // CONFIG_PCI_HCI


#ifdef UNIVERSAL_REPEATER
		if (GET_VXD_PRIV(priv))
			GET_VXD_PRIV(priv)->drv_state &= ~DRV_STATE_VXD_INIT;
#endif
	}
#ifdef SDIO_AP_OFFLOAD
	if ( IS_ROOT_INTERFACE(priv)) {
#ifdef PLATFORM_ARM_BALONG
		free_irq(dev->irq, priv);
		//gpio_free(rtk_wifi_wake_i);
#endif
#ifdef CONFIG_POWER_SAVE
		free_wifi_wakeup_gpio(dev, priv);
#endif
	}
#endif
	RESTORE_INT(flags);
	SMP_UNLOCK(flags);
	rtl8192cd_stop_sw(priv);
	SMP_LOCK(flags);
	SAVE_INT_AND_CLI(flags);

#ifdef CONFIG_WLAN_HAL_8881A
	if (IS_ROOT_INTERFACE(priv) && (GET_CHIP_VER(priv)==VERSION_8881A)) {
		//Disable MAC_System(BIT(0)), MAC_Lextra_Bus(BIT(1))                
		REG32(0xB80000DC)= 0x00;
	}
#endif //#ifdef CONFIG_WLAN_HAL_8881A
#ifdef CONFIG_WLAN_HAL_8197F
	if (IS_ROOT_INTERFACE(priv) && (GET_CHIP_VER(priv)==VERSION_8197F)) {
        REG32(0xB8000064)= 0x00;
	}
#endif //CONFIG_WLAN_HAL_8197F

#ifdef ENABLE_RTL_SKB_STATS
	DEBUG_INFO("skb_tx_cnt =%d\n", rtl_atomic_read(&priv->rtl_tx_skb_cnt));
	DEBUG_INFO("skb_rx_cnt =%d\n", rtl_atomic_read(&priv->rtl_rx_skb_cnt));
#endif

#if defined(CONFIG_RTL_92D_DMDP) && defined(CHECK_HANGUP)
#ifdef MBSSID
        if (IS_ROOT_INTERFACE(priv))
#endif
        if ((GET_CHIP_VER(priv) == VERSION_8192D)
			&& (priv->pshare->rf_ft_var.peerReinit)
        	&& (priv->pmib->dot11RFEntry.macPhyMode == DUALMAC_DUALPHY))
                reset_dmdp_peer(priv);
#endif

	RESTORE_INT(flags);
	SMP_UNLOCK(flags);

#if 0//def  CONFIG_WLAN_HAL
#if defined( MBSSID) || defined(UNIVERSAL_REPEATER)
        if (IS_ROOT_INTERFACE(priv))
#endif
	if (IS_HAL_CHIP(priv)){
		GET_HAL_INTERFACE(priv)->StopSWHandler(priv);
	}
#endif  //CONFIG_WLAN_HAL

#if 0
//#ifdef PCIE_POWER_SAVING
	if(!IS_UMC_A_CUT_88C(priv))
#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	HostPCIe_Close();
#endif

#ifdef SDIO_AP_OFFLOAD
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv))
		ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	else
#endif
	{
		// disable "unlock suspend" to avoid card missing issue in Sequans platform
#if 0//def USE_WAKELOCK_MECHANISM
		rtw_unlock_suspend(priv);
#endif
	}
#endif // SDIO_AP_OFFLOAD

	DBFEXIT;
    return 0;
}

#ifdef EN_EFUSE
static void efuse_ability_init(struct rtl8192cd_priv *priv){

/* Set efuse_enable due to config */
#if defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
#ifdef CONFIG_RTL_5G_SLOT_0
#ifdef CONFIG_SLOT_0_ENABLE_EFUSE
	if(priv->pshare->wlandev_idx == (0^WLANIDX)){
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
		panic_printk("5G on slot0, slot0 efuse, wlan0^idx, enable_efuse=1\n");
		}
#endif
#ifdef CONFIG_SLOT_1_ENABLE_EFUSE
	if(priv->pshare->wlandev_idx == (1^WLANIDX)){
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
		panic_printk("5G on slot0, slot1 efuse, wlan1^idx, enable_efuse=1\n");
		}
#endif
#endif
#ifdef CONFIG_RTL_5G_SLOT_1
#ifdef CONFIG_SLOT_0_ENABLE_EFUSE
	if(priv->pshare->wlandev_idx == (1^WLANIDX)){
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
	panic_printk("5G on slot1, slot0 efuse, wlan1^idx, enable_efuse=1\n");
	}
#endif
#ifdef CONFIG_SLOT_1_ENABLE_EFUSE
	if(priv->pshare->wlandev_idx == (0^WLANIDX)){
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
	panic_printk("5G on slot1, slot1 efuse, wlan0^idx, enable_efuse=1\n");
	}
#endif
#endif
#if !defined(CONFIG_RTL_5G_SLOT_0) && !defined(CONFIG_RTL_5G_SLOT_1)
#ifdef CONFIG_SLOT_0_ENABLE_EFUSE
	if(priv->pshare->wlandev_idx == (0^WLANIDX)){
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
	panic_printk("no 5G on slots, slot0 efuse, wlan0^idx, enable_efuse=1\n");
	}
#endif
#ifdef CONFIG_SLOT_1_ENABLE_EFUSE
	if(priv->pshare->wlandev_idx == (1^WLANIDX)){
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
		panic_printk("no 5G on slots, slot1 efuse, wlan1^idx, enable_efuse=1\n");
	}
#endif
#endif
#else //defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
	//Only ONE SLOT
	if(GET_CHIP_VER(priv) == VERSION_8197F){
#ifdef CONFIG_SOC_ENABLE_EFUSE
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
		panic_printk("wisoc, enable_efuse=1\n");
#else
		priv->pmib->efuseEntry.enable_efuse = 0;
		panic_printk("wisoc, enable_efuse=0\n");
#endif
	}else{
#if defined(CONFIG_SLOT_0_ENABLE_EFUSE) || defined(CONFIG_SLOT_1_ENABLE_EFUSE)
		priv->pmib->efuseEntry.enable_efuse = EFUSE_ENABLE;
		panic_printk("not two slots, enable_efuse=1\n");
#endif
	}
#endif //defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)

/* set efuse enabled ability*/
#ifdef CONFIG_WLAN_HAL_8197F
	if(GET_CHIP_VER(priv) == VERSION_8197F){
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_KFREE	|
			0;
		
	}
#endif

#ifdef CONFIG_WLAN_HAL_8822BE
	if(GET_CHIP_VER(priv) == VERSION_8822B){
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_POWER		|
			EFUSE_THERMAL	|
			EFUSE_MACADDR	|
			EFUSE_CRYSTAL	|
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8192EE
	if(GET_CHIP_VER(priv) == VERSION_8192E){
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_KFREE		|
			EFUSE_POWER		|
			EFUSE_THERMAL	|
			EFUSE_MACADDR	|
			EFUSE_CRYSTAL	|
			EFUSE_LNATYPE	|
			0;
	}
#endif

#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv) == VERSION_8814A){
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_POWER 	|
			EFUSE_THERMAL	|
			EFUSE_MACADDR	|
			EFUSE_CRYSTAL	|
			0;
	}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_POWER 	|
			EFUSE_THERMAL	|
			EFUSE_MACADDR	|
			EFUSE_CRYSTAL	|
			0;
	}
#endif

#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv) == VERSION_8812E){
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_POWER 	|
			EFUSE_THERMAL	|
			EFUSE_MACADDR	|
			EFUSE_CRYSTAL	|
			0;
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if(GET_CHIP_VER(priv)==VERSION_8192D){
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_POWER		|
			EFUSE_THERMAL	|
			EFUSE_MACADDR	|
			EFUSE_CRYSTAL	|
			0;
	}
#endif

#ifdef CONFIG_RTL_92C_SUPPORT
	if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C)) {
		priv->pmib->efuseEntry.enable_efuse |= \
			EFUSE_POWER 	|
			EFUSE_THERMAL	|
			EFUSE_MACADDR	|
			EFUSE_CRYSTAL	|
			0;
	}
#endif


}
#endif

unsigned char is_11AC_Chips(struct rtl8192cd_priv *priv)
{
	if((GET_CHIP_VER(priv) == VERSION_8812E)
		|| (GET_CHIP_VER(priv) == VERSION_8881A)
		|| (GET_CHIP_VER(priv) == VERSION_8814A)
		|| (GET_CHIP_VER(priv) == VERSION_8822B))
		return 1;
	else
		return 0;
}


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static void MDL_DEVINIT set_mib_default(struct rtl8192cd_priv *priv)
{
	unsigned char *p;
#ifdef __KERNEL__
	struct sockaddr addr;
	p = addr.sa_data;
#else
	unsigned char tmpbuf[10];
	p = (unsigned char *)tmpbuf;
#endif

	priv->pmib->mib_version = MIB_VERSION;
	set_mib_default_tbl(priv);

#if defined(STA_CONTROL) && defined(MBSSID)
    if(IS_VAP_INTERFACE(priv)) {
        priv->pmib->staControl.stactrl_groupID = priv->vap_id+1;
    }
#endif


	// others that are not types of byte and int
	strcpy((char *)priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, "RTL8186-default");
	priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen("RTL8186-default");
	memcpy(p, "\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);

#if defined(DOT11D) || defined(DOT11H) || defined(DOT11K)
	// set countryCode for 11d and 11h
	strcpy(priv->pmib->dot11dCountry.dot11CountryString, "US");
#endif

#ifdef 	__KERNEL__
	rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);
#else
	rtl8192cd_set_hwaddr(priv->dev, (void *)p);
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	{
#ifdef DFS
		init_timer(&priv->ch52_timer);
		priv->ch52_timer.data = (unsigned long) priv;
		priv->ch52_timer.function = rtl8192cd_ch52_timer;

		init_timer(&priv->ch56_timer);
		priv->ch56_timer.data = (unsigned long) priv;
		priv->ch56_timer.function = rtl8192cd_ch56_timer;

		init_timer(&priv->ch60_timer);
		priv->ch60_timer.data = (unsigned long) priv;
		priv->ch60_timer.function = rtl8192cd_ch60_timer;

		init_timer(&priv->ch64_timer);
		priv->ch64_timer.data = (unsigned long) priv;
		priv->ch64_timer.function = rtl8192cd_ch64_timer;

		init_timer(&priv->ch100_timer);
		priv->ch100_timer.data = (unsigned long) priv;
		priv->ch100_timer.function = rtl8192cd_ch100_timer;

		init_timer(&priv->ch104_timer);
		priv->ch104_timer.data = (unsigned long) priv;
		priv->ch104_timer.function = rtl8192cd_ch104_timer;

		init_timer(&priv->ch108_timer);
		priv->ch108_timer.data = (unsigned long) priv;
		priv->ch108_timer.function = rtl8192cd_ch108_timer;

		init_timer(&priv->ch112_timer);
		priv->ch112_timer.data = (unsigned long) priv;
		priv->ch112_timer.function = rtl8192cd_ch112_timer;

		init_timer(&priv->ch116_timer);
		priv->ch116_timer.data = (unsigned long) priv;
		priv->ch116_timer.function = rtl8192cd_ch116_timer;

		init_timer(&priv->ch120_timer);
		priv->ch120_timer.data = (unsigned long) priv;
		priv->ch120_timer.function = rtl8192cd_ch120_timer;

		init_timer(&priv->ch124_timer);
		priv->ch124_timer.data = (unsigned long) priv;
		priv->ch124_timer.function = rtl8192cd_ch124_timer;

		init_timer(&priv->ch128_timer);
		priv->ch128_timer.data = (unsigned long) priv;
		priv->ch128_timer.function = rtl8192cd_ch128_timer;

		init_timer(&priv->ch132_timer);
		priv->ch132_timer.data = (unsigned long) priv;
		priv->ch132_timer.function = rtl8192cd_ch132_timer;

		init_timer(&priv->ch136_timer);
		priv->ch136_timer.data = (unsigned long) priv;
		priv->ch136_timer.function = rtl8192cd_ch136_timer;

		init_timer(&priv->ch140_timer);
		priv->ch140_timer.data = (unsigned long) priv;
		priv->ch140_timer.function = rtl8192cd_ch140_timer;

		init_timer(&priv->ch144_timer);
		priv->ch144_timer.data = (unsigned long) priv;
		priv->ch144_timer.function = rtl8192cd_ch144_timer;
#endif

		if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_EMBEDDED) {
			// not implement yet
		} else {
#ifdef IO_MAPPING
			priv->pshare->io_mapping = 1;
#endif
		}
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pshare->version_id == VERSION_8192D) {
#ifdef HIGH_POWER_EXT_LNA
		if (priv->pshare->rf_ft_var.use_ext_lna)
			priv->pshare->rf_ft_var.use_ext_lna = 0;
#endif		
	}
#endif

#ifdef HIGH_POWER_EXT_PA

#if defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
//=========================
//SLOT0=5G, wlan0=pcie0, wlan1=pcie1
#ifdef CONFIG_RTL_5G_SLOT_0
#ifdef CONFIG_SLOT_0_EXT_PA
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_PA
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif
//=========================
//SLOT0=5G, wlan0=pcie1, wlan1=pcie0
#ifdef CONFIG_RTL_5G_SLOT_1
#ifdef CONFIG_SLOT_0_EXT_PA
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_PA
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif
//=========================
//No 5G interface, wlan0=pcie0, wlan1=pcie1
#if !defined(CONFIG_RTL_5G_SLOT_0) && !defined(CONFIG_RTL_5G_SLOT_1)
#ifdef CONFIG_SLOT_0_EXT_PA
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_PA
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif
//==========================
//Only ONE SLOT, always enable HIGH Power 
#else //defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
#ifdef CONFIG_SOC_WIFI

#if defined(CONFIG_SLOT_0_EXT_PA)
#if defined(CONFIG_RTL_5G_SLOT_0) // slot 0 as 5G
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#elif !defined(CONFIG_RTL_5G_SLOT_0) //slot 0 as 2G
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif //CONFIG_SLOT_0_EXT_PA

#ifdef CONFIG_SOC_EXT_PA
#if CONFIG_USE_PCIE_SLOT_0
#if	defined(CONFIG_RTL_5G_SLOT_0) // slot 0 as 5G
	if(priv->pshare->wlandev_idx == (1^WLANIDX)) //soc wifi as 2G
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#elif !defined(CONFIG_RTL_5G_SLOT_0) //slot 0 as 2G
	if(priv->pshare->wlandev_idx == (0^WLANIDX)) //soc wifi as 5G
		priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#else
	priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif
#endif

#else //CONFIG_SOC_WIFI
	priv->pshare->rf_ft_var.use_ext_pa = 1;
#endif //CONFIG_SOC_WIFI
#endif //defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
//===========================
	if ((GET_CHIP_VER(priv) == VERSION_8881A) && priv->pshare->rf_ft_var.use_ext_pa)
		priv->pshare->rf_ft_var.use_ext_pa = 0;

#endif //HIGH_POWER_EXT_PA

#ifdef HIGH_POWER_EXT_LNA
#if defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)

//=========================
//SLOT0=5G, wlan0=pcie0, wlan1=pcie1
#ifdef CONFIG_RTL_5G_SLOT_0
#ifdef CONFIG_SLOT_0_EXT_LNA
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_LNA
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif
//=========================
//SLOT0=5G, wlan0=pcie1, wlan1=pcie0
#ifdef CONFIG_RTL_5G_SLOT_1
#ifdef CONFIG_SLOT_0_EXT_LNA
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_LNA
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif
//=========================
//No 5G interface, wlan0=pcie0, wlan1=pcie1
#if !defined(CONFIG_RTL_5G_SLOT_0) && !defined(CONFIG_RTL_5G_SLOT_1)
#ifdef CONFIG_SLOT_0_EXT_LNA
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#ifdef CONFIG_SLOT_1_EXT_LNA
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif
//==========================
//Only ONE SLOT, always enable HIGH Power 
#else // defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
#ifdef CONFIG_SOC_WIFI

#if defined(CONFIG_SLOT_0_EXT_LNA)
#if defined(CONFIG_RTL_5G_SLOT_0) // slot 0 as 5G
	if(priv->pshare->wlandev_idx == (0^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#elif !defined(CONFIG_RTL_5G_SLOT_0) //slot 0 as 2G
	if(priv->pshare->wlandev_idx == (1^WLANIDX))
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif //CONFIG_SLOT_0_EXT_LNA

#ifdef CONFIG_SOC_EXT_LNA
#if CONFIG_USE_PCIE_SLOT_0
#if	defined(CONFIG_RTL_5G_SLOT_0) // slot 0 as 5G
	if(priv->pshare->wlandev_idx == (1^WLANIDX)) //soc wifi as 2G
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#elif !defined(CONFIG_RTL_5G_SLOT_0) //slot 0 as 2G
	if(priv->pshare->wlandev_idx == (0^WLANIDX)) //soc wifi as 5G
		priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#else
	priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif
#endif
#else //CONFIG_SOC_WIFI
	priv->pshare->rf_ft_var.use_ext_lna = 1;
#endif //CONFIG_SOC_WIFI
#endif // defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
//===========================
	if ((GET_CHIP_VER(priv) == VERSION_8881A) && priv->pshare->rf_ft_var.use_ext_lna)
		priv->pshare->rf_ft_var.use_ext_lna = 0;
#endif //HIGH_POWER_EXT_LNA

#if defined(CONFIG_RTL_8812_SUPPORT) && defined(HIGH_POWER_EXT_PA)
	if (GET_CHIP_VER(priv) == VERSION_8812E && priv->pshare->rf_ft_var.use_ext_pa) {
#ifdef CONFIG_PA_RFMD_4501
		priv->pmib->dot11RFEntry.pa_type = PA_RFMD_4501;
#elif defined(CONFIG_PA_SKYWORKS_5023)
		priv->pmib->dot11RFEntry.pa_type = PA_SKYWORKS_5023;
#elif defined(CONFIG_PA_SKYWORKS_85712_HP)
		priv->pmib->dot11RFEntry.pa_type = PA_SKYWORKS_85712_HP;
#else
		priv->pmib->dot11RFEntry.pa_type = PA_SKYWORKS_5022;
#endif
	}
#endif

#if	(defined(CONFIG_SLOT_0_8192EE) && defined(CONFIG_SLOT_0_EXT_LNA)) || (defined(CONFIG_SLOT_1_8192EE) && defined(CONFIG_SLOT_1_EXT_LNA))
	if (GET_CHIP_VER(priv) == VERSION_8192E && priv->pshare->rf_ft_var.use_ext_lna) {
#ifdef CONFIG_LNA_TYPE_0
		priv->pshare->rf_ft_var.lna_type = LNA_TYPE_0;
#elif defined(CONFIG_LNA_TYPE_1)
		priv->pshare->rf_ft_var.lna_type = LNA_TYPE_1;
#elif defined(CONFIG_LNA_TYPE_2)
		priv->pshare->rf_ft_var.lna_type = LNA_TYPE_2;
#elif defined(CONFIG_LNA_TYPE_3)
		priv->pshare->rf_ft_var.lna_type = LNA_TYPE_3;
#endif
	}
#endif

#ifdef CONFIG_8881A_2LAYER
    if (GET_CHIP_VER(priv) == VERSION_8881A) {
     	priv->pshare->rf_ft_var.use_8881a_2layer = 1;
     	panic_printk("8881A_2Layer\n");
    }
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) && defined(CONFIG_PA_RTC5634)
	if (GET_CHIP_VER(priv) == VERSION_8812E) {
		//panic_printk("Set PA_RTC5634\n");
		priv->pmib->dot11RFEntry.pa_type = PA_RTC5634;
	}
#endif
#ifdef CONFIG_WLAN_HAL_8881A
	if (GET_CHIP_VER(priv) == VERSION_8881A) {
#if defined(CONFIG_8881A_INT_PA) || defined(CONFIG_8881A_RTC5634)		
		priv->pmib->dot11RFEntry.pa_type = PA_INTERNAL;
#else
		priv->pmib->dot11RFEntry.pa_type = 0;
#endif		

#if defined(CONFIG_8881A_INT_PA)
		panic_printk("\n**********************************\n");
		panic_printk("\n** NOTE!! RTL8881A INTERNAL PA!    **\n");
		panic_printk("\n**********************************\n");
#ifdef CONFIG_8881A_INT_PA_RTC5634
		priv->pshare->rf_ft_var.use_intpa8881A= 2;
#else
		priv->pshare->rf_ft_var.use_intpa8881A= 1;
#endif
#else
		priv->pshare->rf_ft_var.use_intpa8881A= 0;		
#endif		
	}
#endif

#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B)){	
#if defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
//=========================
//SLOT0=5G, wlan0=pcie0, wlan1=pcie1
#ifdef CONFIG_RTL_5G_SLOT_0
		if(priv->pshare->wlandev_idx == (0^WLANIDX))
		{
#ifdef CONFIG_SLOT_0_RFE_TYPE_1
			priv->pmib->dot11RFEntry.rfe_type = 1;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_2)
			priv->pmib->dot11RFEntry.rfe_type = 2;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_3)
			priv->pmib->dot11RFEntry.rfe_type = 3;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_4)
			priv->pmib->dot11RFEntry.rfe_type = 4;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_5)
			priv->pmib->dot11RFEntry.rfe_type = 5;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_6)
			priv->pmib->dot11RFEntry.rfe_type = 6;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_7)
			priv->pmib->dot11RFEntry.rfe_type = 7;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_8)
			priv->pmib->dot11RFEntry.rfe_type = 8;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_9)
			priv->pmib->dot11RFEntry.rfe_type = 9;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_10)
			priv->pmib->dot11RFEntry.rfe_type = 10;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_11)
			priv->pmib->dot11RFEntry.rfe_type = 11;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_13)
			priv->pmib->dot11RFEntry.rfe_type = 13;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_14)
			priv->pmib->dot11RFEntry.rfe_type = 14;
#else
			priv->pmib->dot11RFEntry.rfe_type = 0;
#endif
		}
		if(priv->pshare->wlandev_idx == (1^WLANIDX))
		{
#ifdef  CONFIG_SLOT_1_RFE_TYPE_1
			priv->pmib->dot11RFEntry.rfe_type = 1;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_2)
			priv->pmib->dot11RFEntry.rfe_type = 2;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_3)
			priv->pmib->dot11RFEntry.rfe_type = 3;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_4)
			priv->pmib->dot11RFEntry.rfe_type = 4;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_5)
			priv->pmib->dot11RFEntry.rfe_type = 5;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_6)
			priv->pmib->dot11RFEntry.rfe_type = 6;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_7)
			priv->pmib->dot11RFEntry.rfe_type = 7;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_8)
			priv->pmib->dot11RFEntry.rfe_type = 8;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_9)
			priv->pmib->dot11RFEntry.rfe_type = 9;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_10)
			priv->pmib->dot11RFEntry.rfe_type = 10;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_11)
			priv->pmib->dot11RFEntry.rfe_type = 11;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_13)
			priv->pmib->dot11RFEntry.rfe_type = 13;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_14)
			priv->pmib->dot11RFEntry.rfe_type = 14;
#else
			priv->pmib->dot11RFEntry.rfe_type = 0;
#endif
		}
#endif
//=========================
//SLOT1=5G, wlan0=pcie1, wlan1=pcie0
#ifdef CONFIG_RTL_5G_SLOT_1
		if(priv->pshare->wlandev_idx == (1^WLANIDX))
		{
#ifdef CONFIG_SLOT_0_RFE_TYPE_1
			priv->pmib->dot11RFEntry.rfe_type = 1;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_2)
			priv->pmib->dot11RFEntry.rfe_type = 2;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_3)
			priv->pmib->dot11RFEntry.rfe_type = 3;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_4)
			priv->pmib->dot11RFEntry.rfe_type = 4;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_5)
			priv->pmib->dot11RFEntry.rfe_type = 5;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_6)
			priv->pmib->dot11RFEntry.rfe_type = 6;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_7)
			priv->pmib->dot11RFEntry.rfe_type = 7;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_8)
			priv->pmib->dot11RFEntry.rfe_type = 8;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_9)
			priv->pmib->dot11RFEntry.rfe_type = 9;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_10)
			priv->pmib->dot11RFEntry.rfe_type = 10;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_11)
			priv->pmib->dot11RFEntry.rfe_type = 11;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_13)
			priv->pmib->dot11RFEntry.rfe_type = 13;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_14)
			priv->pmib->dot11RFEntry.rfe_type = 14;
#else
			priv->pmib->dot11RFEntry.rfe_type = 0;
#endif
		}
		if(priv->pshare->wlandev_idx == (0^WLANIDX))
		{
#ifdef  CONFIG_SLOT_1_RFE_TYPE_1
			priv->pmib->dot11RFEntry.rfe_type = 1;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_2)
			priv->pmib->dot11RFEntry.rfe_type = 2;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_3)
			priv->pmib->dot11RFEntry.rfe_type = 3;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_4)
			priv->pmib->dot11RFEntry.rfe_type = 4;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_5)
			priv->pmib->dot11RFEntry.rfe_type = 5;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_6)
			priv->pmib->dot11RFEntry.rfe_type = 6;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_7)
			priv->pmib->dot11RFEntry.rfe_type = 7;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_8)
			priv->pmib->dot11RFEntry.rfe_type = 8;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_9)
			priv->pmib->dot11RFEntry.rfe_type = 9;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_10)
			priv->pmib->dot11RFEntry.rfe_type = 10;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_11)
			priv->pmib->dot11RFEntry.rfe_type = 11;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_13)
			priv->pmib->dot11RFEntry.rfe_type = 13;
#elif defined(CONFIG_SLOT_1_RFE_TYPE_14)
			priv->pmib->dot11RFEntry.rfe_type = 14;
#else
			priv->pmib->dot11RFEntry.rfe_type = 0;
#endif
		}
#endif
//==========================
//Only ONE SLOT, always enable HIGH Power 
#else //defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
#if defined(CONFIG_SLOT_0_RFE_TYPE_1) || defined(CONFIG_SLOT_1_RFE_TYPE_1)
		priv->pmib->dot11RFEntry.rfe_type = 1;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_2) || defined(CONFIG_SLOT_1_RFE_TYPE_2)
		priv->pmib->dot11RFEntry.rfe_type = 2;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_3) || defined(CONFIG_SLOT_1_RFE_TYPE_3)
		priv->pmib->dot11RFEntry.rfe_type = 3;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_4) || defined(CONFIG_SLOT_1_RFE_TYPE_4)
		priv->pmib->dot11RFEntry.rfe_type = 4;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_5) || defined(CONFIG_SLOT_1_RFE_TYPE_5)
		priv->pmib->dot11RFEntry.rfe_type = 5;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_6) || defined(CONFIG_SLOT_1_RFE_TYPE_6)
		priv->pmib->dot11RFEntry.rfe_type = 6;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_7) || defined(CONFIG_SLOT_1_RFE_TYPE_7)
		priv->pmib->dot11RFEntry.rfe_type = 7;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_8) || defined(CONFIG_SLOT_1_RFE_TYPE_8)
		priv->pmib->dot11RFEntry.rfe_type = 8;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_9) || defined(CONFIG_SLOT_1_RFE_TYPE_9)
		priv->pmib->dot11RFEntry.rfe_type = 9;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_10) || defined(CONFIG_SLOT_1_RFE_TYPE_10)
		priv->pmib->dot11RFEntry.rfe_type = 10;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_11) || defined(CONFIG_SLOT_1_RFE_TYPE_11)
		priv->pmib->dot11RFEntry.rfe_type = 11;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_13) || defined(CONFIG_SLOT_1_RFE_TYPE_13)
		priv->pmib->dot11RFEntry.rfe_type = 13;
#elif defined(CONFIG_SLOT_0_RFE_TYPE_14) || defined(CONFIG_SLOT_1_RFE_TYPE_14)
		priv->pmib->dot11RFEntry.rfe_type = 14;
#else
		priv->pmib->dot11RFEntry.rfe_type = 0;


#endif
#endif //defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_USE_PCIE_SLOT_0)
	}
#endif //CONFIG_WLAN_HAL_8814AE

#if defined(CONFIG_SOC_WIFI)
	if (GET_CHIP_VER(priv) == VERSION_8197F){	
#ifdef CONFIG_SOC_RFE_TYPE_0
		priv->pmib->dot11RFEntry.rfe_type = 0;
#elif defined(CONFIG_SOC_RFE_TYPE_1)
		priv->pmib->dot11RFEntry.rfe_type = 1;
#elif defined(CONFIG_SOC_RFE_TYPE_2)
		priv->pmib->dot11RFEntry.rfe_type = 2;
#elif defined(CONFIG_SOC_RFE_TYPE_3)
		priv->pmib->dot11RFEntry.rfe_type = 3;
#elif defined(CONFIG_SOC_RFE_TYPE_4)
		priv->pmib->dot11RFEntry.rfe_type = 4;
#elif defined(CONFIG_SOC_RFE_TYPE_5)
		priv->pmib->dot11RFEntry.rfe_type = 5;
#elif defined(CONFIG_SOC_RFE_TYPE_6)
    priv->pmib->dot11RFEntry.rfe_type = 6;		
#endif
		panic_printk("RFE TYPE =%d\n",priv->pmib->dot11RFEntry.rfe_type);
	}
#endif //CONFIG_SOC_WIFI

	if(is_11AC_Chips(priv))
		AMSDU_ENABLE = 2;
	else
		AMSDU_ENABLE = 0;
	
#if defined(CONFIG_SHARE_XCAP_SUPPORT)
	priv->pmib->dot11RFEntry.share_xcap = 1;
#endif

	if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B))
		ODMPTR->TH_L2H_default = 0xf2;
	else if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		ODMPTR->TH_L2H_default = 0xef;
	else
		ODMPTR->TH_L2H_default = 0xf3;

	
#ifdef EN_EFUSE
	efuse_ability_init(priv);
#endif
}


#ifdef CONFIG_WLAN_HAL
BOOLEAN
Wlan_HAL_Link(
    struct rtl8192cd_priv *priv
)
{
    BOOLEAN         status = TRUE;
    BOOLEAN         bVal;
	unsigned int    errorFlag;

#if CONFIG_WLAN_MACHAL_API
    if (GET_CHIP_VER(priv)==VERSION_8822B) {

       associate_halMac_API(priv);
       if(HALMAC_RET_SUCCESS != halmac_init_adapter(priv,priv->pHalmac_platform_api,HALMAC_INTERFACE_PCIE,&(priv->pHalmac_adapter),&(priv->pHalmac_api)))
       {
            printk("halmac_init_adapter Failed \n");
            errorFlag |= DRV_ER_INIT_PON;
            status = FALSE;
       }else    {
            printk("halmac_init_adapter Succss \n");
       }
    }
#endif
    

	if (RT_STATUS_SUCCESS == HalAssociateNic(priv, TRUE)) {
        DEBUG_INFO("HalAssociateNic OK \n");

	} else {
	    panic_printk("HalAssociateNic Failed \n");
        status = FALSE;
	}

	GET_HW(priv)->MIMO_TR_hw_support = GET_HAL_INTERFACE(priv)->GetChipIDMIMOHandler(priv);

	GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, (pu1Byte)&bVal);
	if ( bVal || (RTL_R8(SYS_FUNC_EN+1) & BIT2)) {

        // If the watchdog reboot the system without stop the DMA HW, it may got DMA hang
        // Do reset to recovery this case
        GET_HAL_INTERFACE(priv)->ResetHWForSurpriseHandler(priv);
        
		if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
			DEBUG_INFO("StopHW Succeed\n");
		} else {
			GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);            
			errorFlag |= DRV_ER_CLOSE_STOP_HW;
			GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                     
			panic_printk("StopHW Failed\n");
            status = FALSE;
		}                
	} else {
            //printk("Can't write MACID register\n");
	}

    check_chipID_MIMO(priv);

    return status;
}
#endif // CONFIG_WLAN_HAL
#if defined(__ECOS)
static struct ss_res* alloc_site_survey_res(int flags)
#else
static struct ss_res* alloc_site_survey_res(gfp_t flags)
#endif
{
	struct ss_res *res;
	
	res = (struct ss_res *)rtw_vmalloc_flag(sizeof(struct ss_res), flags);
	if (NULL == res) return NULL;
	
	memset(res, 0, sizeof(struct ss_res));
	return res;
}

static void free_site_survey_res(struct ss_res *res)
{
	rtw_vmfree(res, sizeof(struct ss_res));
}


#if defined(__LINUX_2_6__) && !defined(CONFIG_COMPAT_NET_DEV_OPS)
static const struct net_device_ops rtl8192cd_netdev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl8192cd_set_hwaddr,
#if LINUX_VERSION_CODE<KERNEL_VERSION(2,6,42) || \
    (LINUX_VERSION_CODE>=KERNEL_VERSION(3,0,0) && LINUX_VERSION_CODE<KERNEL_VERSION(3,2,0))
#if defined(CONFIG_RTL8685) || defined(CONFIG_RTL8685S)
        .ndo_change_rx_flags = rtl8192cd_set_rx_mode,
#else
        .ndo_set_multicast_list = rtl8192cd_set_rx_mode,
#endif
#else
        .ndo_set_rx_mode        = rtl8192cd_set_rx_mode,
#endif
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
#ifdef CONFIG_NETDEV_MULTI_TX_QUEUE
        .ndo_select_queue       = rtl8192cd_select_queue,
#endif
};


#if defined(CONFIG_RTK_MESH)
static const struct net_device_ops mesh_netdev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl8192cd_set_hwaddr,
#if LINUX_VERSION_CODE<KERNEL_VERSION(2,6,42) || \
    (LINUX_VERSION_CODE>=KERNEL_VERSION(3,0,0) && LINUX_VERSION_CODE<KERNEL_VERSION(3,2,0))
        .ndo_set_multicast_list = rtl8192cd_set_rx_mode,
#else
        .ndo_set_rx_mode        = rtl8192cd_set_rx_mode,
#endif
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = mesh_start_xmit,
};
#endif

#endif

#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
// wlan0
struct net_device *wlan_root_netdev=NULL;
struct net_device *wlan_vap_netdev[4]={0};
struct net_device *wlan_wds_netdev[8]={0};
struct net_device *wlan_vxd_netdev=NULL;
// wlan1
struct net_device *wlan1_root_netdev=NULL;
struct net_device *wlan1_vap_netdev[4]={0};
struct net_device *wlan1_wds_netdev[8]={0};
struct net_device *wlan1_vxd_netdev=NULL;
#endif

#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
#ifdef __OSK__
static int MDL_DEVINIT rtl8192cd_init_one(struct osk_pci_dev *pdev,
                  const struct pci_device_id *ent, struct _device_info_ *wdev, int vap_idx)
#elif defined(__KERNEL__)

#ifdef DROP_RXPKT
struct rtl8192cd_priv *G5_priv = NULL;
static void disable_G5_rx_drop(unsigned long data) {
	//printk("Try turning off 5G rx drop\n");
	if(G5_priv != NULL) {
		//printk("G5_priv=%p\n", G5_priv);
#if 0
		if (G5_priv->pmib->miscEntry.drop_rxpkt_en) {
			G5_priv->pmib->miscEntry.drop_rxpkt_en = 0;
			printk("Turned off 5G rx drop\n");
		}
#else
		G5_priv->pmib->miscEntry.drop_rxpkt_rate = G5_priv->pmib->miscEntry.G5_drop_rxpkt_rate; 
		printk("G5_priv->pmib->miscEntry.drop_rxpkt_rate=%lu\n", G5_priv->pmib->miscEntry.drop_rxpkt_rate);
		
#endif
	}
}
struct timer_list G5_rx_drop_timer;
void init_G5_rx_drop_timer(void) {
	init_timer(&G5_rx_drop_timer);	
	G5_rx_drop_timer.function = disable_G5_rx_drop;
	printk("===========================================\n");
	printk("G5_rx_drop_timer initialized\n");
	printk("===========================================\n");
}
#endif

#ifdef CONFIG_PCI_HCI
static int MDL_DEVINIT rtl8192cd_init_one(struct pci_dev *pdev,
                  const struct pci_device_id *ent, struct _device_info_ *wdev, int vap_idx)
#endif
#ifdef CONFIG_USB_HCI
int MDL_DEVINIT rtl8192cd_init_one(struct usb_interface *pusb_intf,
                  const struct usb_device_id *ent, struct _device_info_ *wdev, int vap_idx)
#endif
#ifdef CONFIG_SDIO_HCI
int MDL_DEVINIT rtl8192cd_init_one(struct sdio_func *psdio_func,
                  const struct sdio_device_id *ent, struct _device_info_ *wdev, int vap_idx)
#endif
#else // !__KERNEL__
#ifdef CONFIG_PCI_HCI
void *rtl8192cd_init_one(void *pdev, void *ent, struct _device_info_ *wdev, int vap_idx, int wlan_index)
#endif
#ifdef CONFIG_SDIO_HCI
void *rtl8192cd_init_one(struct sdio_func *psdio_func, void *ent, struct _device_info_ *wdev, int vap_idx, int wlan_index)
#endif
#endif // __KERNEL__
{
    struct net_device *dev;
#ifdef __ECOS
    struct net_device *tmp_dev;
#endif
    struct rtl8192cd_priv *priv;
    void *regs;
	struct wifi_mib 		*pmib;
	DOT11_QUEUE				*pevent_queue;
#ifdef RSSI_MONITOR_NCR
	DOT11_QUEUE				*rssim_queue;	
#endif	
#ifdef CONFIG_RTL_WAPI_SUPPORT
	DOT11_QUEUE				*wapiEvent_queue;
	#if	0 //defined(MBSSID)
	DOT11_QUEUE				*wapiVapEvent_queue;
	#endif
#endif
#ifdef CONFIG_IEEE80211R
	DOT11_QUEUE				*pftevent_queue;
#endif
	struct rtl8192cd_hw		*phw;
#ifdef CONFIG_PCI_HCI
	struct rtl8192cd_tx_desc_info		*ptxdesc;
#endif
	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct wlanbuf_poll		*pwlanbuf_poll;
	struct wlanicv_poll		*pwlanicv_poll;
	struct wlanmic_poll		*pwlanmic_poll;
	struct wlan_acl_poll	*pwlan_acl_poll;
#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
    struct p2p_context     *p2p_context_ptr;
#endif
	DOT11_EAP_PACKET		*Eap_packet;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	WPA_GLOBAL_INFO			*wpa_global_info;
#endif
	struct ss_res			*site_survey;
#ifdef  CONFIG_RTK_MESH
#ifdef _MESH_ACL_ENABLE_
	struct mesh_acl_poll	*pmesh_acl_poll = NULL;
#endif

#ifndef WDS
	char baseDevName[8];
#endif
	int mesh_num;

	struct hash_table		*proxy_table = NULL;
#ifdef PU_STANDARD
	//pepsi
	struct hash_table		*proxyupdate_table = NULL;
#endif
	struct mpp_tb			*pann_mpp_tb = NULL;
	struct hash_table		*mesh_rreq_retry_queue = NULL;
	// add by chuangch 2007.09.13
	struct hash_table		*pathsel_table = NULL;

	DOT11_QUEUE2			*pathsel_queue = NULL;

#endif	// CONFIG_RTK_MESH


#ifdef CONFIG_PCI_HCI
	unsigned char *page_ptr = NULL;
#ifdef WLAN_HAL_TX_AMSDU
    unsigned char *page_ptr_amsdu;
#endif
#ifdef WLAN_SUPPORT_H2C_PACKET
    unsigned char *page_ptr_h2c;
#endif // WLAN_SUPPORT_H2C_PACKET          
#if defined(NOT_RTK_BSP)	
	unsigned long page_align_phy=0;
#endif
#ifdef CONFIG_NET_PCI
    u8 cache_size;
    u16 pci_command;
#ifndef USE_IO_OPS
	resource_size_t pciaddr;
#endif
	unsigned long pmem_len;
#endif
#endif // CONFIG_PCI_HCI

#ifdef WDS
	int wds_num;
	char baseDevName[8];
#endif
#if defined(CONFIG_RTK_MESH) || defined(WDS) || defined(EN_EFUSE)
	int i;
#endif

	struct priv_shared_info *pshare;	// david

#ifdef CONFIG_WLAN_HAL
    BOOLEAN     bVal;
	unsigned int errorFlag;
#endif  //CONFIG_WLAN_HAL

#ifdef USE_DMA_ALLOCATE
	dma_addr_t	dma_phys = NULL;
	void		*dma_virt = NULL;
#endif

    int rc=0;
    priv = NULL;
    regs = NULL;
	pmib = NULL;
	pevent_queue = NULL;
#ifdef RSSI_MONITOR_NCR
	rssim_queue = NULL;	
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	wapiEvent_queue = NULL;
	#if	0 //defined(MBSSID)
	wapiVapEvent_queue = NULL;
	#endif
#endif
#ifdef CONFIG_IEEE80211R
	pftevent_queue = NULL;
#endif
	phw = NULL;
#ifdef CONFIG_PCI_HCI
	ptxdesc = NULL;
#endif
	pwlan_hdr_poll = NULL;
	pwlanllc_hdr_poll = NULL;
	pwlanbuf_poll = NULL;
	pwlanicv_poll = NULL;
	pwlanmic_poll = NULL;
	pwlan_acl_poll = NULL;
	Eap_packet = NULL;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	wpa_global_info = NULL;
#endif
	site_survey = NULL;

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_NET_PCI
    cache_size=0;
#ifndef USE_IO_OPS
    pciaddr=0;
#endif
	pmem_len=0;
#endif
#endif // CONFIG_PCI_HCI


	pshare = NULL;	// david

#ifdef NOT_RTK_BSP
	printk("=====>> INSIDE %s <<=====\n", __func__);
#endif
#ifdef RTK_NL80211
	struct rtknl *rtk=NULL; //mark_dual

	if(vap_idx < 0)
	{
		if(wdev->priv == NULL)
		{
			NDEBUG2("init for root device\n");
			rtk = realtek_cfg80211_create();
               }
		else // mark_rpt
		{		
		    rtk = wdev->priv->rtk ; // find the root rtk
		    NDEBUG2("init for vxd device\n");
		}	
	}
	else 
	{
		NDEBUG2("init for virtual device\n");
		
		if (wdev->priv != NULL) 
		{
			rtk = wdev->priv->rtk ; // find the root rtk
		}
		else 
		{	
			printk("can't find the root if for this virtual interface \n");
			return NULL;
		}		
#if 0 //wrt-vap
		if(rtk->ndev_name[rtk->num_vif][0]==0)
		{
			printk("virtual interface shall be created by cfg80211\n");
			return NULL;
		}
#endif
	}

	NDEBUG2("sizeof(struct rtl8192cd_priv)[%d]\n", sizeof(struct rtl8192cd_priv));

#endif

#if defined(CONFIG_PCI_HCI) || !defined(CONFIG_NETDEV_MULTI_TX_QUEUE)
	dev = alloc_etherdev(sizeof(struct rtl8192cd_priv));
#else
	dev = alloc_etherdev_mq(sizeof(struct rtl8192cd_priv), _NETDEV_TX_QUEUE_ALL);
#endif
	if (!dev) {
		printk(KERN_ERR "alloc_etherdev() error!\n");
#ifdef __KERNEL__
		return -ENOMEM;
#else
		return NULL;
#endif
	}

#ifdef __ECOS
	tmp_dev = dev;
#endif
	// now, allocating memory for pmib
#ifdef RTL8192CD_VARIABLE_USED_DMEM
	pmib = (struct wifi_mib *)rtl8192cd_dmem_alloc(PMIB, NULL);
#else
	pmib = (struct wifi_mib *)kmalloc((sizeof(struct wifi_mib)), GFP_ATOMIC);
#endif
	if (!pmib) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wifi_mib (size %d)\n", sizeof(struct wifi_mib));
		goto err_out_free;
	}
	memset(pmib, 0, sizeof(struct wifi_mib));

	pevent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_ATOMIC);
	if (!pevent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)pevent_queue, 0, sizeof(DOT11_QUEUE));
#ifdef RSSI_MONITOR_NCR
	rssim_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_ATOMIC);
	if (!rssim_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)rssim_queue, 0, sizeof(DOT11_QUEUE));
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	wapiEvent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_ATOMIC);
	if (!wapiEvent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)wapiEvent_queue, 0, sizeof(DOT11_QUEUE));
	#if 0 //def MBSSID
	wapiVapEvent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN), GFP_ATOMIC);
	if (!wapiVapEvent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN);
		goto err_out_free;
	}
	memset((void *)wapiVapEvent_queue, 0, sizeof(DOT11_QUEUE)*RTL8192CD_NUM_VWLAN);
	#endif
#endif
#ifdef CONFIG_IEEE80211R
	pftevent_queue = (DOT11_QUEUE *)kmalloc((sizeof(DOT11_QUEUE)), GFP_ATOMIC);
	if (!pftevent_queue) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for DOT11_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
		goto err_out_free;
	}
	memset((void *)pftevent_queue, 0, sizeof(DOT11_QUEUE));
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv == NULL) // root interface
#endif
	{
#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		phw = &hw_info[wlandev_idx];
#else
		phw = &hw_info;
#endif
#else
#ifndef USE_DMA_ALLOCATE
		phw = (struct rtl8192cd_hw *)kmalloc((sizeof(struct rtl8192cd_hw)), GFP_ATOMIC);
		if (!phw) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for rtl8192cd_hw (size %d)\n", sizeof(struct rtl8192cd_hw));
			goto err_out_free;
		}
#else
		dma_virt = dma_alloc_coherent(cma_dev, sizeof(struct rtl8192cd_hw), &dma_phys, GFP_KERNEL);
		phw = (struct rtl8192cd_hw *)dma_virt;
		if (!dma_virt) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't dma_alloc_coherent for rtl8192cd_hw (size %d)\n", sizeof(struct rtl8192cd_hw));
			goto err_out_free;
		}
		else {
			printk("****** %s %d, allocated addr %p\n", __FUNCTION__, __LINE__, &dma_virt);
		}
#endif
#endif
		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
#ifdef CONFIG_PCI_HCI
		ptxdesc = &phw->tx_info;
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pshare = &shared_info[wlandev_idx];
#else
		pshare = &shared_info;
#endif
#else
		pshare = (struct priv_shared_info *)kmalloc(sizeof(struct priv_shared_info), GFP_ATOMIC);
		if (!pshare) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for priv_shared_info (size %d)\n", sizeof(struct priv_shared_info));
			goto err_out_free;
		}
#endif
		memset((void *)pshare, 0, sizeof(struct priv_shared_info));

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlan_hdr_poll = &hdr_pool[wlandev_idx];
#else
		pwlan_hdr_poll = &hdr_pool;
#endif
#else
		pwlan_hdr_poll = (struct wlan_hdr_poll *)
						kmalloc((sizeof(struct wlan_hdr_poll)), GFP_ATOMIC);
		if (!pwlan_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlan_hdr_poll (size %d)\n", sizeof(struct wlan_hdr_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanllc_hdr_poll = &llc_pool[wlandev_idx];
#else
		pwlanllc_hdr_poll = &llc_pool;
#endif
#else
		pwlanllc_hdr_poll = (struct wlanllc_hdr_poll *)
						kmalloc((sizeof(struct wlanllc_hdr_poll)), GFP_ATOMIC);
		if (!pwlanllc_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanllc_hdr_poll (size %d)\n", sizeof(struct wlanllc_hdr_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanbuf_poll = &buf_pool[wlandev_idx];
#else
		pwlanbuf_poll = &buf_pool;
#endif
#else
		pwlanbuf_poll = (struct	wlanbuf_poll *)
						kmalloc((sizeof(struct	wlanbuf_poll)), GFP_ATOMIC);
		if (!pwlanbuf_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanbuf_poll (size %d)\n", sizeof(struct wlanbuf_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanicv_poll = &icv_pool[wlandev_idx];
#else
		pwlanicv_poll = &icv_pool;
#endif
#else
		pwlanicv_poll = (struct	wlanicv_poll *)
						kmalloc((sizeof(struct	wlanicv_poll)), GFP_ATOMIC);
		if (!pwlanicv_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanicv_poll (size %d)\n", sizeof(struct wlanicv_poll));
			goto err_out_free;
		}
#endif

#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		pwlanmic_poll = &mic_pool[wlandev_idx];
		wlandev_idx++;
#else
		pwlanmic_poll = &mic_pool;
#endif
#else
		pwlanmic_poll = (struct	wlanmic_poll *)
						kmalloc((sizeof(struct	wlanmic_poll)), GFP_ATOMIC);
		if (!pwlanmic_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanmic_poll (size %d)\n", sizeof(struct wlanmic_poll));
			goto err_out_free;
		}
#endif
	}
/*cfg p2p cfg p2p*/	
#ifdef P2P_SUPPORT
    p2p_context_ptr = (struct p2p_context*)kmalloc(sizeof(struct p2p_context),GFP_ATOMIC); 
    if(p2p_context_ptr == NULL){
        rc = -ENOMEM;
        panic_printk(KERN_ERR "Can't kmalloc for p2p_context (size %d)\n", sizeof(struct p2p_context));
        goto err_out_free;                
    }else{          
        memset(p2p_context_ptr,0,sizeof(struct p2p_context));      
    }        
#endif

	pwlan_acl_poll = (struct wlan_acl_poll *)
					kmalloc((sizeof(struct wlan_acl_poll)), GFP_ATOMIC);
	if (!pwlan_acl_poll) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wlan_acl_poll (size %d)\n", sizeof(struct wlan_acl_poll));
		goto err_out_free;
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	pmesh_acl_poll = (struct mesh_acl_poll *)
					kmalloc((sizeof(struct mesh_acl_poll)), GFP_ATOMIC);
	if (!pmesh_acl_poll) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for Mesh wlan_acl_poll (size %d)\n", sizeof(struct mesh_acl_poll));
		goto err_out_free;
	}
#endif

	Eap_packet = (DOT11_EAP_PACKET *)
					kmalloc((sizeof(DOT11_EAP_PACKET)), GFP_ATOMIC);
	if (!Eap_packet) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for Eap_packet (size %d)\n", sizeof(DOT11_EAP_PACKET));
		goto err_out_free;
	}
	memset((void *)Eap_packet, 0, sizeof(DOT11_EAP_PACKET));

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	wpa_global_info = (WPA_GLOBAL_INFO *)
					kmalloc((sizeof(WPA_GLOBAL_INFO)), GFP_ATOMIC);
	if (!wpa_global_info) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wpa_global_info (size %d)\n", sizeof(WPA_GLOBAL_INFO));
		goto err_out_free;
	}
	memset((void *)wpa_global_info, 0, sizeof(WPA_GLOBAL_INFO));
#endif

#if (defined(UNIVERSAL_REPEATER) || defined(MBSSID)) && !defined(MULTI_MAC_CLONE)
	if ((NULL == wdev->priv) || (vap_idx < 0) || vap_idx == 1 || vap_idx == 2) 
        /*for multiple repeater usage,  need to allocate site_survey memory for vap_idx=1,2*/
#endif
	{
		site_survey = alloc_site_survey_res(GFP_KERNEL /*GFP_ATOMIC*/);
		if (!site_survey) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for site_survey (size %d)\n", sizeof(struct ss_res));
			goto err_out_free;
		}
	}

#ifndef __DRAYTEK_OS__
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv) {
#ifndef RTK_NL80211
#ifdef UNIVERSAL_REPEATER
		if (vap_idx < 0)
		{
#if defined(CUSTOMIZE_WLAN_IF_NAME)
			sprintf(dev->name, "%s-vxd", wdev->priv->dev->name);
#else
			sprintf(dev->name, "%s-vxd", wdev->priv->dev->name);
#endif
		}
#endif
#ifdef MBSSID
		if (vap_idx >= 0)
#if defined(CONFIG_RTL8672) || defined(__OSK__)
			sprintf(dev->name, "%s-vap%d", wdev->priv->dev->name, vap_idx);
#elif defined(CUSTOMIZE_WLAN_IF_NAME)
			sprintf(dev->name, "%s.%d", wdev->priv->dev->name, vap_idx);
#else
			sprintf(dev->name, "%s-va%d", wdev->priv->dev->name, vap_idx);
#endif
#endif
#endif
	}
	else
#endif
#if defined(CUSTOMIZE_WLAN_IF_NAME)
		strcpy(dev->name, "wl%d");
#else
		strcpy(dev->name, "wlan%d");
#endif
#endif

#ifdef RTK_NL80211
#ifdef MBSSID
	if (vap_idx >= 0)
	{
		if(vap_idx == 0)
			sprintf(dev->name, "tmp.%s", wdev->priv->dev->name);
		else
			sprintf(dev->name, "%s-%d", wdev->priv->dev->name, vap_idx);
	}
#endif
#ifdef UNIVERSAL_REPEATER
	else if(wdev->priv)
	{
		sprintf(dev->name, "%s-%d", wdev->priv->dev->name, RTL8192CD_NUM_VWLAN);
	}
#endif
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
#if defined(CUSTOMIZE_WLAN_IF_NAME)
		if(strcmp(dev->name, WLAN0_VIRTUAL1_IFNAME)==0)				
			dev->vlan_member_map=WLAN0_VA0_MASK_BIT;
		else if(strcmp(dev->name,WLAN0_VIRTUAL2_IFNAME)==0)
			dev->vlan_member_map=WLAN0_VA1_MASK_BIT;
		else if(strcmp(dev->name,WLAN0_VIRTUAL3_IFNAME)==0)
			dev->vlan_member_map=WLAN0_VA2_MASK_BIT; 
		else if(strcmp(dev->name,WLAN0_VIRTUAL4_IFNAME)==0)
			dev->vlan_member_map=WLAN0_VA3_MASK_BIT; 
		else if(strcmp(dev->name,WLAN1_VIRTUAL1_IFNAME)==0)
			dev->vlan_member_map=WLAN1_VA0_MASK_BIT;
		else if(strcmp(dev->name,WLAN1_VIRTUAL2_IFNAME)==0)
			dev->vlan_member_map=WLAN1_VA1_MASK_BIT;
		else if(strcmp(dev->name,WLAN1_VIRTUAL3_IFNAME)==0)
			dev->vlan_member_map=WLAN1_VA2_MASK_BIT; 
		else if(strcmp(dev->name,WLAN1_VIRTUAL4_IFNAME)==0)
			dev->vlan_member_map=WLAN1_VA3_MASK_BIT; 
		else if(strcmp(dev->name,WLAN0_REPEATER_IFNAME)==0)
			dev->vlan_member_map=WLAN0_VXD_MASK_BIT; 
		else if(strcmp(dev->name,WLAN1_REPEATER_IFNAME)==0)
			dev->vlan_member_map=WLAN1_VXD_MASK_BIT;	
#else
		if(strcmp(dev->name,"wlan0-va0")==0)				
			dev->vlan_member_map=WLAN0_VA0_MASK_BIT;
		else if(strcmp(dev->name,"wlan0-va1")==0)
			dev->vlan_member_map=WLAN0_VA1_MASK_BIT;
		else if(strcmp(dev->name,"wlan0-va2")==0)
			dev->vlan_member_map=WLAN0_VA2_MASK_BIT; 
		else if(strcmp(dev->name,"wlan0-va3")==0)
			dev->vlan_member_map=WLAN0_VA3_MASK_BIT; 
		else if(strcmp(dev->name,"wlan1-va0")==0)
			dev->vlan_member_map=WLAN1_VA0_MASK_BIT;
		else if(strcmp(dev->name,"wlan1-va1")==0)
			dev->vlan_member_map=WLAN1_VA1_MASK_BIT;
		else if(strcmp(dev->name,"wlan1-va2")==0)
			dev->vlan_member_map=WLAN1_VA2_MASK_BIT; 
		else if(strcmp(dev->name,"wlan1-va3")==0)
			dev->vlan_member_map=WLAN1_VA3_MASK_BIT; 
		else if(strcmp(dev->name,"wlan0-vxd")==0)
			dev->vlan_member_map=WLAN0_VXD_MASK_BIT; 
		else if(strcmp(dev->name,"wlan1-vxd")==0)
			dev->vlan_member_map=WLAN1_VXD_MASK_BIT; 		
#endif
#endif

#ifdef LINUX_2_6_24_
	/*SET_MODULE_OWNER is obsolete from 2.6.24*/
#else
	SET_MODULE_OWNER(dev);
#endif

#ifdef NETDEV_NO_PRIV
	priv = (struct rtl8192cd_priv *)netdev_priv(dev);
	priv->wlan_priv = priv;
#else
	priv = dev->priv;
#endif


	priv->pmib = pmib;
#if 0/*defined(CONFIG_RTL_WAPI_SUPPORT)*/
	/*	only for test	*/
	priv->pmib->wapiInfo.wapiType = wapiDisable;
	priv->pmib->wapiInfo.wapiUpdateMCastKeyType = wapi_disable_update;
	priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout = WAPI_KEY_UPDATE_PERIOD;
	priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum = WAPI_KEY_UPDATE_PKTCNT;
#endif
	priv->pevent_queue = pevent_queue;
#ifdef RSSI_MONITOR_NCR
	priv->rssimEvent_queue = rssim_queue;
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	priv->wapiEvent_queue= wapiEvent_queue;
#endif
#ifdef CONFIG_IEEE80211R
	priv->pftevent_queue = pftevent_queue;
#endif
	priv->pwlan_acl_poll = pwlan_acl_poll;
	priv->Eap_packet = Eap_packet;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	priv->wpa_global_info = wpa_global_info;
#endif
	priv->site_survey = site_survey;
#ifdef MBSSID
	priv->vap_id = -1;
#endif
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	priv->pmesh_acl_poll = pmesh_acl_poll;
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (wdev->priv) {
		priv->pshare = wdev->priv->pshare;
		priv->proot_priv = wdev->priv;
#ifdef UNIVERSAL_REPEATER
		if (vap_idx < 0) // create for vxd
			GET_VXD_PRIV(wdev->priv) = priv;
#endif
#ifdef MBSSID
		if (vap_idx >= 0)  { // create for vap
			GET_ROOT(priv)->pvap_priv[vap_idx] = priv;
			priv->vap_id = vap_idx;
			priv->vap_init_seq = -1;

			#if 0 /*defined(CONFIG_RTL_WAPI_SUPPORT)*/
			priv->pmib->wapiInfo.wapiType = wapiDisable;
			priv->pmib->wapiInfo.wapiUpdateMCastKeyType = wapi_disable_update;
			priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout = WAPI_KEY_UPDATE_PERIOD;
			priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum = WAPI_KEY_UPDATE_PKTCNT;
			priv->wapiEvent_queue= &wapiVapEvent_queue[vap_idx];
			printk("dev[%s]:wapiType[%d] UCastKeyType[%d] psk[%s] len[%d]\n",
				priv->dev->name, priv->pmib->wapiInfo.wapiType,
				priv->pmib->wapiInfo.wapiUpdateMCastKeyType,
				priv->pmib->wapiInfo.wapiPsk.octet,
				priv->pmib->wapiInfo.wapiPsk.len);
			#endif
		}
#endif
#ifdef  CONFIG_WLAN_HAL
        priv->HalFunc = GET_ROOT(priv)->HalFunc;
        priv->HalData = GET_ROOT(priv)->HalData;
#endif

#ifdef  CONFIG_WLAN_MACHAL_API
        priv->pHalmac_adapter = GET_ROOT(priv)->pHalmac_adapter;
        priv->pHalmac_api = GET_ROOT(priv)->pHalmac_api;
#endif

	}
	else
#endif
	{
		priv->pshare = pshare;	// david
		priv->pshare->phw = phw;
#ifdef USE_DMA_ALLOCATE
		priv->pshare->hw_dma_phys = dma_phys;
#endif

#ifdef CONCURRENT_MODE
		priv->pshare->wlandev_idx = wlan_index;
#endif
#ifdef CONFIG_PCI_HCI
		priv->pshare->pdesc_info = ptxdesc;
#endif
		priv->pshare->pwlan_hdr_poll = pwlan_hdr_poll;
		priv->pshare->pwlanllc_hdr_poll = pwlanllc_hdr_poll;
		priv->pshare->pwlanbuf_poll = pwlanbuf_poll;
		priv->pshare->pwlanicv_poll = pwlanicv_poll;
		priv->pshare->pwlanmic_poll = pwlanmic_poll;
		wdev->priv = priv;

#ifdef __OSK__
		wifi_root_priv = priv;
#endif

#ifdef __KERNEL__
		spin_lock_init(&priv->pshare->lock);
#endif

#ifdef SMP_SYNC
		spin_lock_init(&priv->pshare->lock_xmit);
		spin_lock_init(&priv->pshare->lock_skb);
#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
		_rtw_mutex_init(&(priv->pshare->lock_buf));
#else
		spin_lock_init(&priv->pshare->lock_buf);
#endif
		spin_lock_init(&priv->pshare->lock_recv);
#ifdef RTK_129X_PLATFORM
		spin_lock_init(&priv->pshare->lock_io_129x);
		priv->pshare->lock_io_129x_owner=-1;
#endif
		priv->pshare->lock_owner=-1;
		priv->pshare->lock_recv_owner=-1;
		priv->pshare->lock_xmit_owner=-1;
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		priv->pshare->nr_bcn = 1;
		priv->pshare->bcn_priv[0] = priv;
#ifdef MBSSID
		_rtw_mutex_init(&priv->pshare->mbssid_lock);
#endif
#endif

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_NET_PCI
		if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
			priv->pshare->pdev = pdev;
#endif
#endif // CONFIG_PCI_HCI

		priv->pshare->type = wdev->type;
#ifdef USE_RTL8186_SDK
#if defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X)
		priv->pshare->have_hw_mic = 1;
#else
		priv->pshare->have_hw_mic = 0;
#endif
#else
		priv->pshare->have_hw_mic = 0;
#endif
//		priv->pshare->is_giga_exist  = is_giga_board();
#ifdef MULTI_MAC_CLONE
		priv->pshare->mclone_init_seq = -1;
#endif
	}
/*cfg p2p cfg p2p*/
#ifdef P2P_SUPPORT  //assign priv->p2pPtr for all interface
    priv->p2pPtr = p2p_context_ptr;
#endif
	priv->dev = dev;

#ifndef __DRAYTEK_OS__
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv)) { // is a vxd or vap
		dev->base_addr = GET_ROOT(priv)->dev->base_addr;
		goto register_driver;
	}
#endif

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
	{
		rc = pci_enable_device(pdev);
		if (rc)
			goto err_out_free2;
#ifndef USE_IO_OPS
		rc = pci_request_regions(pdev, DRV_NAME);
#endif
		if (rc)
			goto err_out_disable;

		if (pdev->irq < 2) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "invalid irq (%d) for pci dev\n", pdev->irq);
#else
			printk(KERN_ERR "invalid irq (%d) for pci dev %s\n", pdev->irq, pdev->slot_name);
#endif
			goto err_out_res;
		}
#ifdef USE_IO_OPS
		{
			resource_size_t pio_start;
			unsigned long pio_len, pio_flags;

			pio_start = pci_resource_start(pdev, 0);
			pio_len = (unsigned long)pci_resource_len(pdev, 0);
			pio_flags = (unsigned long)pci_resource_flags(pdev, 0);
//			pio_start = pci_resource_start(pdev, 2);
//			pio_len = (unsigned long)pci_resource_len(pdev, 2);
//			pio_flags = (unsigned long)pci_resource_flags(pdev, 2);


			if (!(pio_flags & IORESOURCE_IO)) {
				rc = -EIO;
#ifdef __LINUX_2_6__
				printk(KERN_ERR "pci: region #0 not a PIO resource, aborting\n");
#else
				printk(KERN_ERR "%s: region #0 not a PIO resource, aborting\n", pdev->slot_name);
#endif
				goto err_out_res;
			}

			if (!request_region(pio_start, pio_len, DRV_NAME)) {
				rc = -EIO;
				printk(KERN_ERR "request_region failed!\n");
				goto err_out_res;
			}

			if (pio_len < RTL8192CD_REGS_SIZE) {
				rc = -EIO;
#ifdef __LINUX_2_6__
				printk(KERN_ERR "PIO resource (%lx) too small on pci dev\n", pio_len);
#else
				printk(KERN_ERR "PIO resource (%lx) too small on pci dev %s\n", pio_len, pdev->slot_name);
#endif
				goto err_out_res;
			}

			dev->base_addr = pio_start;
			priv->pshare->ioaddr = pio_start; // device I/O address
		}
#else
#ifdef RTK_129X_PLATFORM
		if(pdev->bus->number == 0x00) {
			pciaddr = PCIE_SLOT1_MEM_START;
			pmem_len = PCIE_SLOT1_MEM_LEN;
			printk("RTD129X: PCIE SLOT1\n");
		} else if(pdev->bus->number == 0x01) {
			pciaddr = PCIE_SLOT2_MEM_START;
			pmem_len = PCIE_SLOT2_MEM_LEN;
			printk("RTD129X: PCIE SLOT2\n");
		} else {
			printk(KERN_ERR "RTD129X: Wrong Slot Num\n");
			goto err_out_res;
		}

		if(pdev->bus->number == 0x00)
			priv->pshare->mask_addr = (unsigned long)ioremap( PCIE_SLOT1_MASK, 0x100);
		else if(pdev->bus->number == 0x01)
			priv->pshare->mask_addr = (unsigned long)ioremap( PCIE_SLOT2_MASK, 0x100);

		if(priv->pshare->mask_addr == 0) {
			printk(KERN_ERR "Can't map TRAN/MASK mem\n");
			goto err_out_res;
		}

		priv->pshare->tran_addr = priv->pshare->mask_addr + PCIE_TRANSLATE_OFFSET;
#else //RTK_129X_PLATFORM
#ifdef IO_MAPPING
		pciaddr = pci_resource_start(pdev, 0);
//		pciaddr = pci_resource_start(pdev, 2);
#else
//		pciaddr = pci_resource_start(pdev, 1);
		pciaddr = pci_resource_start(pdev, 2);
#endif
		if (!pciaddr) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "no MMIO resource for pci dev");
#else
			printk(KERN_ERR "no MMIO resource for pci dev %s\n", pdev->slot_name);
#endif
			goto err_out_res;
		}

//		if ((pmem_len = pci_resource_len(pdev, 1)) < RTL8192CD_REGS_SIZE) {
		if ((pmem_len = pci_resource_len(pdev, 2)) < RTL8192CD_REGS_SIZE) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "MMIO resource () too small on pci dev\n");
#else
			printk(KERN_ERR "MMIO resource (%lx) too small on pci dev %s\n", (unsigned long)pmem_len, pdev->slot_name);
#endif
			goto err_out_res;
		}
#endif //#ifdef RTK_129X_PLATFORM

		regs = ioremap_nocache(pciaddr, pmem_len);
		if (!regs) {
			rc = -EIO;
#ifdef __LINUX_2_6__
			printk(KERN_ERR "Cannot map PCI MMIO () on pci dev \n");
#else
			printk(KERN_ERR "Cannot map PCI MMIO (%lx@%lx) on pci dev %s\n", (unsigned long)pmem_len, (long)pciaddr, pdev->slot_name);
#endif
#ifdef RTK_129X_PLATFORM
			goto err_out_iomap_mask;
#else
			goto err_out_res;
#endif
		}

		dev->base_addr = (unsigned long)regs;
		priv->pshare->ioaddr = (unsigned long)regs;
		//check_chipID_MIMO(priv);
#endif // USE_IO_OPS
#if !defined(NOT_RTK_BSP)
{
		struct pci_dev *pdev = priv->pshare->pdev;
		u16 vendor_id,device_id;
		pci_read_config_word( pdev , PCI_VENDOR_ID	, &vendor_id);
		pci_read_config_word( pdev , PCI_DEVICE_ID	, &device_id);

		DEBUG_INFO("vendor_id=0x%04X, device_id=0x%04X\n", vendor_id, device_id);

#ifdef CONFIG_RTL_92D_SUPPORT
		if( (vendor_id==0x10ec) && (device_id==0x8193) )
		{
			u32 vendor_deivce_id, config_base;		
			vendor_deivce_id = dev->base_addr;
			if(dev->base_addr==0xba000000)
			{
				vendor_deivce_id= *((volatile unsigned long *)(0xb8b30000));
			}
			else if(dev->base_addr==0xb9000000)
			{
				vendor_deivce_id= *((volatile unsigned long *)(0xb8b10000));
			}
			DEBUG_INFO("   vendor_deivce_id=%x\n", vendor_deivce_id);
			if (vendor_deivce_id == ((unsigned long)((0x8193<<16)|PCI_VENDOR_ID_REALTEK)))
				priv->pshare->version_id = VERSION_8192D;
			else
				priv->pshare->version_id =0x1234;
		}
#endif

#ifdef CONFIG_RTL_88E_SUPPORT
		if ((vendor_id == 0x10ec) && (device_id == 0x8179))
		{
			DEBUG_INFO("version_id=VERSION_8188E\n");
			priv->pshare->version_id = VERSION_8188E;
			priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
		} 
#endif

}
#else/*NOT_RTK_BSP*/

		{
			struct pci_dev *pdev = priv->pshare->pdev;
			u16 vendor_id,device_id;
			pci_read_config_word( pdev , PCI_VENDOR_ID  , &vendor_id);
			pci_read_config_word( pdev , PCI_DEVICE_ID  , &device_id);
			
			DEBUG_INFO("vendor_id=0x%04X, device_id=0x%04X\n", vendor_id, device_id);
#ifdef CONFIG_RTL_92D_SUPPORT
			if( (vendor_id==0x10ec) && (device_id==0x8193) ) 
			{
				DEBUG_INFO("version_id=VERSION_8192D\n");
				priv->pshare->version_id = VERSION_8192D;
				#ifdef CONFIG_RTL_92D_DMDP
					priv->pshare->wlandev_idx = rtl8192D_idx;/*timmy_modify*/
					if( rtl8192D_idx == 0 )
					{
						priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
						priv->pmib->dot11BssType.net_work_type = WIRELESS_11A | WIRELESS_11N ;
					}
					else
					{
						priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
						priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G;
					}
					rtl8192D_idx++;
					DEBUG_INFO("macPhyMode=DUALMAC_DUALPHY  MIMO_1T1R\n");
					priv->pmib->dot11RFEntry.macPhyMode = DUALMAC_DUALPHY;/*timmy_dbg*/
					priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
				#else
					DEBUG_INFO("macPhyMode=DUALMAC_DUALPHY MIMO_2T2R\n");
					priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;/*timmy_dbg*/
					priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
				#endif
			} else
#endif // CONFIG_RTL_92D_SUPPORT
#ifdef CONFIG_RTL_8812_SUPPORT
			if( (vendor_id==0x10ec) && (device_id==0x8812) ) 
			{
				DEBUG_INFO("version_id=VERSION_8812E found\n");
				priv->pshare->version_id = VERSION_8812E;
				priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
				priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
			} else
#endif // CONFIG_RTL_8812_SUPPORT
#ifdef CONFIG_RTL_8723B_SUPPORT
			if(vendor_id == 0xb723) 
			{
				DEBUG_INFO("version_id=VERSION_8723B found\n");
				priv->pshare->version_id = VERSION_8723B;
				priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
				priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
			} else
#endif
#ifdef CONFIG_WLAN_HAL_8192EE
			if( (vendor_id==0x10ec) && (device_id==0x8170 || device_id==0x818b) ) 
			{
				DEBUG_INFO("version_id=VERSION_8192E found\n");
				priv->pshare->version_id = VERSION_8192E;
				priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
#ifdef __OSK__
				priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
				priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G | WIRELESS_11N;
#endif
				priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
				Wlan_HAL_Link(priv);
			} else
#endif // CONFIG_WLAN_HAL_8192EE		
#ifdef  CONFIG_WLAN_HAL_8814AE	
			if( (vendor_id==0x10ec) && (device_id==0x8813) ) 
			{
				DEBUG_INFO("version_id=VERSION_8814AE found\n");
				priv->pshare->version_id = VERSION_8814A;
				priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
				// TODO: Filen, temporarily use 2T2R
				// 8814 merge issue
#if 0
				// 8814 merge issue (Disable?)
				//Disable Multi-tag
				RTL_W8(REG_HCI_MIX_CFG, RTL_R8(REG_HCI_MIX_CFG) & ~BIT6 );
#endif
				Wlan_HAL_Link(priv);
			} else
#endif // CONFIG_WLAN_HAL_8814AE
#ifdef CONFIG_RTL_88E_SUPPORT
			if ((vendor_id == 0x10ec) && (device_id == 0x8179))
			{
				DEBUG_INFO("version_id=VERSION_8188E\n");
				priv->pshare->version_id = VERSION_8188E;
				priv->pshare->phw->MIMO_TR_hw_support = MIMO_1T1R;
			} else
#endif
#ifdef CONFIG_WLAN_HAL_8822BE
			if ((vendor_id == 0x10ec) && (device_id == 0xb822))
			{
				DEBUG_INFO("version_id=VERSION_8822B\n");
				priv->pshare->version_id = VERSION_8822B;
				priv->pmib->dot11RFEntry.macPhyMode = SINGLEMAC_SINGLEPHY;
				priv->pshare->phw->MIMO_TR_hw_support = MIMO_2T2R;
				Wlan_HAL_Link(priv);
			} else
#endif
			{
				printk(KERN_ERR "%s(%d): can't recognize wifi chip(vendor_id=0x%04x, device_id=0x%04x)!!\n", 
					__func__, __LINE__, vendor_id, device_id);
				rc = -ENODEV;
				goto err_out_res;
			}
		}
#endif /*NOT_RTK_BSP*/
		check_chipID_MIMO(priv);
	}
	else
#endif // CONFIG_NET_PCI
	{
        regs = (void *)wdev->base_addr;
        dev->base_addr = (unsigned long)wdev->base_addr;
        priv->pshare->ioaddr = (unsigned long)regs;

        if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_DIRECT)
        {
            int i ;
            #ifdef CONFIG_RTL_92D_SUPPORT
            unsigned long  config_base, base_addr;
            config_base = wdev->conf_addr;
            base_addr = wdev->base_addr;
            #endif
			_DEBUG_INFO("INIT PCI config space directly\n");

#ifdef RTK_NL80211 //Do protection before PCIE Reset (for watchdog reboot)
			int flags;
			SMP_LOCK(flags);
#endif

#if !defined(CONFIG_NET_PCI) && (defined(__OSK__) || defined(CONFIG_RTL8196B) || defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F))
			#if defined(CONFIG_RTL8196C) || defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)

            if (BSP_PCIE0_D_CFG0 == wdev->conf_addr)
            {
				#ifdef CONFIG_RTL_8197F
				{
					extern  int PCIE_reset_procedure_97F(unsigned int PCIeIdx, unsigned int mdioReset);
					if ((PCIE_reset_procedure_97F(0,1))  != 1) {
						rc = -ENODEV;
						goto err_out_free2;
					}
				}
				#else
                #ifdef CONFIG_RTL_8198B //mark_apo
                PCIE_reset_procedure(1, 0, 1, BSP_PCIE1_D_CFG0); 
                #endif				
                printk("CFG0 \n");
                if ((PCIE_reset_procedure(0, 0, 1, wdev->conf_addr))== FAIL)
                {
#ifdef RTK_NL80211
					SMP_UNLOCK(flags);
#endif
                    rc = -ENODEV;
#if defined(CONFIG_WLAN_HAL_8814AE) && defined(CONFIG_USE_PCIE_SLOT_0)
					panic_printk("!!Reboot by PCIE0 (8814) init unlink 0x%x\r\n", REG32(0xb8b00728));
					watchdog_reboot();
#else
                    goto err_out_free2;
#endif
                }
		#endif
            }
            else if (BSP_PCIE1_D_CFG0 == wdev->conf_addr)
			{
		#ifndef CONFIG_RTL_8197F
                printk("CFG1 \n");
                if ((PCIE_reset_procedure(1, 0, 1, wdev->conf_addr))== FAIL)
                {
#ifdef RTK_NL80211
					SMP_UNLOCK(flags);
#endif
                    rc = -ENODEV;
#if defined(CONFIG_WLAN_HAL_8814AE) && defined(CONFIG_USE_PCIE_SLOT_1)
					panic_printk("!!Reboot by PCIE1 (8814) init unlink 0x%x\r\n", REG32(0xb8b20728));
					watchdog_reboot();
#else
                    goto err_out_free2;
#endif
                }
		#endif
            }
            else
            {
#ifdef RTK_NL80211
				SMP_UNLOCK(flags);
#endif
                rc = -ENODEV;
                goto err_out_free2;
            }
            #else
            if (rtl8196b_pci_reset(wdev->conf_addr) == FAIL) {
#ifdef RTK_NL80211
				SMP_UNLOCK(flags);
#endif
                rc = -ENODEV;
                goto err_out_free2;
            }
			#endif
#ifdef RTK_NL80211
			SMP_UNLOCK(flags);
#endif
#endif
            #ifdef __ECOS
			/* Fix the issue to use memory under 1M */
			REG32(0xb8b00000+0x1c)=(2<<4) | (0<<12);   // [7:4]=base [15:12]=limit
			REG32(0xb8b00000+0x20)=(2<<4) | (0<<20);   // [7:4]=base [15:12]=limit
			REG32(0xb8b00000+0x24)=(2<<4) | (0<<20);   // [7:4]=base [15:12]=limit
            #endif

			{
				u32 vendor_deivce_id, config_base;
				config_base = wdev->conf_addr;
				vendor_deivce_id = *((volatile unsigned long *)(config_base+0));
				DEBUG_INFO("config_base=%x, vendor_deivce_id=%x\n", config_base, vendor_deivce_id);
				if (
                    #if defined(CONFIG_RTL_92D_SUPPORT)
					(vendor_deivce_id != ((unsigned long)((0x8193<<16)|PCI_VENDOR_ID_REALTEK))) &&
                    #endif
					(vendor_deivce_id != ((unsigned long)((0x8191<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8171<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8178<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8174<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8176<<16)|PCI_VENDOR_ID_REALTEK)))
                    #ifdef CONFIG_RTL_88E_SUPPORT
					&& (vendor_deivce_id != ((unsigned long)((0x8179<<16)|PCI_VENDOR_ID_REALTEK)))
                    #endif
					#ifdef CONFIG_RTL_8723B_SUPPORT
					&& (vendor_deivce_id != ((unsigned long)((0xb723<<16)|PCI_VENDOR_ID_REALTEK)))
					#endif
                    #ifdef CONFIG_RTL_8812_SUPPORT
					&& (vendor_deivce_id != ((unsigned long)((0x8812<<16)|PCI_VENDOR_ID_REALTEK)))
                    #endif
                    #if defined(CONFIG_WLAN_HAL_8192EE)
					&& (vendor_deivce_id != ((unsigned long)((0x8170<<16)|PCI_VENDOR_ID_REALTEK)))
					&& (vendor_deivce_id != ((unsigned long)((0x818b<<16)|PCI_VENDOR_ID_REALTEK)))
                    #endif
                    #if defined(CONFIG_WLAN_HAL_8814AE)
                    && (vendor_deivce_id != ((unsigned long)((0x8813<<16)|PCI_VENDOR_ID_REALTEK)))
                    #endif
                    #if defined(CONFIG_WLAN_HAL_8822BE)
                    && (vendor_deivce_id != ((unsigned long)((0xb822<<16)|PCI_VENDOR_ID_REALTEK)))
                    #endif
					) {
					_DEBUG_ERR("vendor_deivce_id=%x not match\n", vendor_deivce_id);
					rc = -EIO;
					goto err_out_free2;
			    }
					
#if defined (CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_RTL_8723B_SUPPORT) || defined(CONFIG_WLAN_HAL_8822BE)
#ifdef CONFIG_RTL_92D_SUPPORT
                if (vendor_deivce_id == ((unsigned long)((0x8193<<16)|PCI_VENDOR_ID_REALTEK)))
                    priv->pshare->version_id = VERSION_8192D;
                else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
                if (vendor_deivce_id == ((unsigned long)((0x8179<<16)|PCI_VENDOR_ID_REALTEK))) {
                    //					DEBUG_INFO("\n found 8188E !!! \n");
                    priv->pshare->version_id = VERSION_8188E;
                } else
#endif
#ifdef CONFIG_RTL_8812_SUPPORT
                if (vendor_deivce_id == ((unsigned long)((0x8812<<16)|PCI_VENDOR_ID_REALTEK)))
                {
                    DEBUG_INFO("\n found 8812 !!! \n");
                    priv->pshare->version_id = VERSION_8812E;
#ifdef DROP_RXPKT
					G5_priv = priv;
					init_G5_rx_drop_timer();
#endif
                }
                else
#endif
#ifdef CONFIG_RTL_8723B_SUPPORT
				if (vendor_deivce_id == ((unsigned long)((0xb723<<16)|PCI_VENDOR_ID_REALTEK)))
				{
					DEBUG_INFO("\n found 8723B !!! \n");
					priv->pshare->version_id = VERSION_8723B;
				}
				else
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
                if (vendor_deivce_id == ((unsigned long)((0x8170<<16)|PCI_VENDOR_ID_REALTEK))
                    || vendor_deivce_id == ((unsigned long)((0x818b<<16)|PCI_VENDOR_ID_REALTEK))) {
                    priv->pshare->version_id = VERSION_8192E;
#ifdef __OSK__
					priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
					priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G | WIRELESS_11N;
#endif
                    DEBUG_INFO("\n found 8192E !!! \n");
                }
                else
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
                if (vendor_deivce_id == ((unsigned long)((0x8813<<16)|PCI_VENDOR_ID_REALTEK))) {
                    priv->pshare->version_id = VERSION_8814A;
                    DEBUG_INFO("\n found 8814A !!! \n");
					#if defined(CONFIG_OPENWRT_SDK)
					if (BSP_PCIE1_D_CFG0 == wdev->conf_addr) {
						#if defined(CONFIG_RTL_5G_SLOT_1)
						priv->pshare->is5g = 1;
						#else
						priv->pshare->is5g = 0;
						#endif
					} else {
						#if defined(CONFIG_RTL_5G_SLOT_1)
						priv->pshare->is5g = 0;
						#else
						priv->pshare->is5g = 1;
						#endif
                	}
					#endif
                } 
				else
#endif // defined(CONFIG_WLAN_HAL_8814AE)
#if defined(CONFIG_WLAN_HAL_8822BE)
                if (vendor_deivce_id == ((unsigned long)((0xb822<<16)|PCI_VENDOR_ID_REALTEK))) {
                    priv->pshare->version_id = VERSION_8822B;
                    DEBUG_INFO("\n found 8822B !!! \n");
                    printk("\n found 8822B !!! \n");
#ifdef CONFIG_RTL8672
					// temp workaround for xDSL platform
					printk("avoid 8822B hang\n");
					printk("debug set soc-memory 0xb8b01000 0x0b000f01\n");
					REG32(0xb8b01000)=0x0b000f01;				
#endif
                }
                else
#endif
				{
					priv->pshare->version_id = 0;
				}
#endif
			}

            *((volatile unsigned long *)PCI_CONFIG_BASE1) = virt_to_bus((void *)dev->base_addr);

#ifdef CONFIG_RTL_8197F //eric-8822 97f
//#if defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_RTL_8812_SUPPORT)
//			if(priv->pshare->version_id == VERSION_8822B || priv->pshare->version_id == VERSION_8812E) 
				*((volatile unsigned long *)PCI_CONFIG_BASE1) = 0x19000004;
//#endif
#endif

            //DEBUG_INFO("...config_base1 = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_BASE1));
            for(i=0; i<1000000; i++);
            *((volatile unsigned char *)PCI_CONFIG_COMMAND) = 0x07;
            //DEBUG_INFO("...command = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_COMMAND));
            for(i=0; i<1000000; i++);
            *((volatile unsigned short *)PCI_CONFIG_LATENCY) = 0x2000;
            for(i=0; i<1000000; i++);
            //DEBUG_INFO("...latency = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_LATENCY));

#ifndef NOT_RTK_BSP
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) || defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
			if ((GET_CHIP_VER(priv)!=VERSION_8188E) && (GET_CHIP_VER(priv)!=VERSION_8192E) && (GET_CHIP_VER(priv)!=VERSION_8812E) && (GET_CHIP_VER(priv)!=VERSION_8814A) && (GET_CHIP_VER(priv)!=VERSION_8822B))
#endif
			{
#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8196C)
#if defined(CONFIG_NET_PCI)
#define REVR                                    0xB8000000
#define RTL8196C_REVISION_A     0x80000001
#define RTL8196C_REVISION_B     0x80000002
#endif
				if (REG32(REVR) == RTL8196C_REVISION_B) {
					REG32(0xb8b01000)=0x0f0f0f01; //Enhance for RTL8192c signal  
					for(i=0; i<1000000; i++);
					i=REG32(0xb8b01000);
					REG32(0xb9000354)=0xc940; //Card PCIE PHY initial  parameter for rtl8196c revision B
					REG32(0xb9000358)=0x24;
					for(i=0; i<1000000; i++);
					REG32(0xb9000354)=0x4270;
					REG32(0xb9000358)=0x25;
					for(i=0; i<1000000; i++);
					REG32(0xb9000354)=0x019E; //Card PCIE PHY initial  parameter for rtl8196c revision B
					REG32(0xb9000358)=0x23;
				}
#endif
#if defined(CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#if !defined(CONFIG_RTL_92D_SUPPORT)
#if defined(CONFIG_NET_PCI)
#define BSP_REVR        0xB8000000
#define BSP_RTL8198_REVISION_A	0xC0000000
#define BSP_RTL8198_REVISION_B	0xC0000001 
#endif
#if !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)

#if defined(CONFIG_RTL_8196E)
				if ((REG32(BSP_REVR) & 0xFFFFF000) == BSP_RTL8196E)
#else
				if ((REG32(BSP_REVR) >= BSP_RTL8198_REVISION_B) || ((REG32(BSP_REVR) & 0xFFFFF000) == BSP_RTL8197D)) 
#endif
				{
					if (priv->pshare->version_id!= VERSION_8192D) {
						REG32(dev->base_addr+0x354)=0xc940; //Card PCIE PHY initial  parameter for rtl8196c revision B
						REG32(dev->base_addr+0x358)=0x24;
					for(i=0; i<1000000; i++);
							REG32(dev->base_addr+0x354)=0x4270;
						REG32(dev->base_addr+0x358)=0x25;
					for(i=0; i<1000000; i++);
							REG32(dev->base_addr+0x354)=0x019E; //Card PCIE PHY initial  parameter for rtl8196c revision B
						REG32(dev->base_addr+0x358)=0x23;
					}
				}
#endif
#endif
#endif
			}
			#if defined(CONFIG_RTL_88E_SUPPORT)
                        else
			{
				REG32(dev->base_addr+0x354)=0x4104; //Card PCIE PHY initial  parameter for rtl8196c revision B
				for(i=0; i<1000000; i++);
					REG32(dev->base_addr+0x358)=0x24;
			}
			#endif
#ifdef CONFIG_RTL_8198B
			REG32(BSP_WDTCNTRR) |= BSP_WDT_KICK;
#elif !defined(CONFIG_ARCH_LUNA_SLAVE) && !defined(__OSK__)
			REG32(0xB800311C) |=  1 << 23;
#endif
#endif // !NOT_RTK_BSP
#ifdef  CONFIG_WLAN_HAL_8192EE
            if (GET_CHIP_VER(priv)==VERSION_8192E) {

                #ifndef NOT_RTK_BSP
                if (PHY_QueryBBReg(priv,0xf0,BIT(23)) == 0) {
                    //printk("\n found 8192E MP chip!!! 0xf0=0x%x\n", RTL_R32(0xf0));
                    #ifdef CONFIG_RTL_8198C			
                    REG32(0xb8b10078)= 0x00100010;	// burst size        
                    #else
                    if (((RTL_R32(0xf0) >> 12 ) & 0xf) == 0){ 			

                        unsigned int tmp_reg = REG32(0xb8b10078);

                        DEBUG_INFO("\n8192E MP chip A-cut!!! 0xb8b10078=0x%x\n", tmp_reg );
                        if (((tmp_reg & 0xf000) >> 12) != 2) {						
                            tmp_reg &= 0xffff0fff;
                            tmp_reg |= 0x2000;
					
							DEBUG_INFO("==> 0xb8b10078=0x%x\n", tmp_reg);
                            RTL_W32(0x3e8, tmp_reg);					
                            RTL_W32(0x3f0, 0x1f078);
                        }
                    }
                    #endif			
                } 		
                #endif		
                Wlan_HAL_Link(priv);	   
            } else
#endif
#ifdef  CONFIG_WLAN_HAL_8814AE
	       if (GET_CHIP_VER(priv)==VERSION_8814A) {
	           Wlan_HAL_Link(priv);
			   spread_spectrum(priv);
	       }
	       else
#endif
#ifdef  CONFIG_WLAN_HAL_8822BE
	       if (GET_CHIP_VER(priv)==VERSION_8822B) {
	           Wlan_HAL_Link(priv);
	       }
	       else
#endif
            {
                check_chipID_MIMO(priv);
                //Exception Case
                if ( check_MAC_IO_Enable(priv) ) {
                    #if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
                    if((GET_CHIP_VER(priv)==VERSION_8192C) || (GET_CHIP_VER(priv)==VERSION_8188C) || (GET_CHIP_VER(priv)==VERSION_8192D) || (GET_CHIP_VER(priv)==VERSION_8812E)) {
                        RTL_W8(0x1c, RTL_R8(0x1c)& ~BIT(1));
                        RTL_W8(CR,0);
                        RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) & 0xfe);
                        RTL_W8(SYS_FUNC_EN+1, RTL_R8(SYS_FUNC_EN+1) |1);
                    }
                    #endif			
                    rtl8192cd_stop_hw(priv);
                }
            }
	   }
       else if (TYPE_EMBEDDED == ((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) ) {
            DEBUG_INFO("TYPE_EMBEDDED\n");
#if defined(CONFIG_WLAN_HAL_8881A)             
				priv->pshare->version_id = VERSION_8881A;
//#ifdef  CONFIG_SOC_RTL8881A
#if !defined(NOT_RTK_BSP)
            // TODO: Filen_debug, code below is temporary
            //Reset Letrx Bus and WL MAC
            #if 0 //96D
            //REG32(0xB8000010)|= 0x00008000;
            #else //8881A
            REG32(0xB8000010)|= 0x00003800;
            //Enable MAC_System(BIT(0)), MAC_Lextra_Bus(BIT(1))
            REG32(0xB80000DC)= 0x03;
            #endif
#endif  //CONFIG_SOC_RTL8881A
#endif //defined(CONFIG_WLAN_HAL_8881A)  

#if defined(CONFIG_WLAN_HAL_8197F) 
            if((REG32(0xB8000000)&0xfffff000)== 0x8197F000)
            {
                priv->pshare->version_id = VERSION_8197F;
                DEBUG_INFO("\n found 8197F !!! \n");
                REG32(0xB8000064)|= 0x1F;
            }
#endif // #if defined(CONFIG_WLAN_HAL_8197F) 



#ifdef  CONFIG_WLAN_HAL
            Wlan_HAL_Link(priv);
#endif  //CONFIG_WLAN_HAL
        }		
	}
/*	==========>> maybe later
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
		rtl8192cd_ePhyInit(priv);
*/

#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
		dev->irq = pdev->irq;
		pci_set_drvdata(pdev, dev);
	}
	else
#endif
	{
		dev->irq = wdev->irq;
	}

#ifdef CONFIG_PUMA_UDMA_SUPPORT
	if ((rc = udma_register_handler(wlan_index, dev, &rtl8192cd_start_xmit)) != 0) {
		goto err_out_free2;
	}
#endif

#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
	dev->base_addr = (unsigned long)wdev->base_addr;
	usb_set_intfdata(pusb_intf, dev);
	
	priv->pshare->pusbintf = pusb_intf;
	priv->pshare->pusbdev = interface_to_usbdev(pusb_intf);
	priv->pshare->bDriverStopped = TRUE;
	
#ifdef CONFIG_RTL_92C_SUPPORT
	priv->pshare->bCardDisableWOHSM = FALSE;
	priv->pshare->BoardType = BOARD_USB_DONGLE;
#endif
	
	if (usb_dvobj_init(priv) != SUCCESS) {
		rc = -ENOMEM;
		goto err_out_free2;
	}
	
	if (rtw_init_drv_sw(priv) != SUCCESS) {
		usb_dvobj_deinit(priv);
		rc = -ENOMEM;
		goto err_out_free2;
	}
#endif // CONFIG_USB_HCI

#ifdef CONFIG_SDIO_HCI
	dev->base_addr = (unsigned long)wdev->base_addr;
#ifdef __ECOS
	psdio_func->dev.driver_data = (void *)dev; //for nova
//diag_printf("###%s %d func=%p driver_data=%p dev=%p###\n",__FUNCTION__,__LINE__, psdio_func, psdio_func->dev.driver_data, dev);
#else
	sdio_set_drvdata(psdio_func, dev);
#endif
	priv->pshare->psdio_func = psdio_func;
	priv->pshare->bDriverStopped = TRUE;

	if ((rc = sdio_dvobj_init(priv)) != 0) {
		goto err_out_free2;
	}
	
	if (rtw_init_drv_sw(priv) != SUCCESS) {
		sdio_dvobj_deinit(priv);
		rc = -ENOMEM;
		goto err_out_free2;
	}
	
	DisableSdioInterrupt(priv);
#endif // CONFIG_SDIO_HCI

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
register_driver:
#endif

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
	dev->open = rtl8192cd_open;
	dev->stop = rtl8192cd_close;
#if defined(__KERNEL__)
	dev->set_multicast_list = rtl8192cd_set_rx_mode;
#endif
	dev->hard_start_xmit = rtl8192cd_start_xmit;
	dev->get_stats = rtl8192cd_get_stats;
	dev->do_ioctl = rtl8192cd_ioctl;
#if defined(__KERNEL__) || defined(__OSK__)
	dev->set_mac_address = rtl8192cd_set_hwaddr;
#endif
#ifdef CONFIG_NETDEV_MULTI_TX_QUEUE
	dev->select_queue = rtl8192cd_select_queue;
#endif
#else
	dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#if defined(CONFIG_RTL8672) && !defined(__OSK__)
	dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

#ifdef __ECOS
#ifdef CONFIG_SDIO_HCI
	dev->isr = NULL;
#else
	dev->isr = rtl8192cd_interrupt;
#endif
	dev->dsr= interrupt_dsr;
	dev->can_xmit = can_xmit;
#ifdef CONFIG_RLTK819X_SW_QUEUE 
	dev->sw_queue = NULL;
#endif
#endif

#if defined(CONFIG_WLAN_HAL) 
	MACFM_software_init(priv);
#endif

#if defined(__KERNEL__) || defined(__OSK__)

#ifdef RTK_NL80211
	priv->rtk = rtk; 
	if(vap_idx < 0) {
		if (IS_ROOT_INTERFACE(priv)) //wrt-vxd
		{
			realtek_cfg80211_init(rtk,priv);
			realtek_interface_add(priv, rtk, rtk->root_ifname, NL80211_IFTYPE_STATION, 0, 0); 
		}
#ifdef  UNIVERSAL_REPEATER //wrt-vxd
		else
		{
			priv->dev = dev; 
			memcpy(dev->dev_addr, GET_ROOT(priv)->pmib->dot11Bss.bssid, 6);
			dev->dev_addr[4] = dev->dev_addr[4] + 1;
			memcpy(GET_MY_HWADDR, dev->dev_addr, 6);
			realtek_interface_add(priv, rtk, dev->name, NL80211_IFTYPE_STATION, 0, 0);
		}
#endif
	}
	else 
	{
		priv->dev = dev; 

#if defined(VAP_MAC_DRV_READ_FLASH)
		if(read_flash_hw_mac_vap(rtk->vap_mac[vap_idx], vap_idx))
		{
			memcpy(dev->dev_addr, GET_ROOT(priv)->pmib->dot11Bss.bssid, 6);
			dev->dev_addr[5] = dev->dev_addr[5] + vap_idx + 1;
		} else {
			memcpy(dev->dev_addr, rtk->vap_mac[vap_idx], 6);
		}
#endif
		memcpy(GET_MY_HWADDR, dev->dev_addr, 6);
		realtek_interface_add(priv, rtk, dev->name, NL80211_IFTYPE_AP, 0, 0);
	}

#elif defined(__OSK__)
	rc = osk_register_netdev(dev);
#else
	rc = register_netdev(dev);
#endif

	if (rc)
		goto err_out_iomap;
#endif

#if (defined(CONFIG_RTL8672) && defined(__OSK__)) && !defined(CONFIG_RTL_819X)
	wlanDev[wlanDevNum].dev_pointer = dev;
	wlanDev[wlanDevNum].dev_ifgrp_member = 0;
	wlanDevNum++;
#endif

#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
	if(dev->name[strlen(dev->name)-2]=='p') //dev->name="wlanX-vapX"
	{
		switch(dev->name[strlen(dev->name)-1])
		{
			case '0':			
				//printk("#### vap0=%x ####\n",(u32)dev);		
				if(dev->name[strlen(dev->name)-6]=='0')
                                        wlan_vap_netdev[0]=dev;
                                else
                                        wlan1_vap_netdev[0]=dev;
				break;
			case '1':
				//printk("#### vap1=%x ####\n",(u32)dev);			
				if(dev->name[strlen(dev->name)-6]=='0')
                                        wlan_vap_netdev[1]=dev;
                                else
                                        wlan1_vap_netdev[1]=dev;
				break;
			case '2':
				//printk("#### vap2=%x ####\n",(u32)dev);
				if(dev->name[strlen(dev->name)-6]=='0')
                                        wlan_vap_netdev[2]=dev;
                                else
                                        wlan1_vap_netdev[2]=dev;
				break;
			case '3':
				//printk("#### vap3=%x ####\n",(u32)dev);			
				if(dev->name[strlen(dev->name)-6]=='0')
                                        wlan_vap_netdev[3]=dev;
                                else
                                        wlan1_vap_netdev[3]=dev;
				break;
		}
	}
	else if(dev->name[strlen(dev->name)-1]=='d') //dev->name="wlanX-vxd"
	{
		printk("#### vxd=%x ####\n",(u32)dev);
		#ifdef CONFIG_WLAN0_5G_WLAN1_2G
		if(priv->pshare->use_hal) // 92E
			wlan1_vxd_netdev=dev;
		else
			wlan_vxd_netdev=dev;
		#else
		if(priv->pshare->use_hal) // 92E
			wlan_vxd_netdev=dev;
		else
			wlan1_vxd_netdev=dev;
		#endif
	}
	else if(dev->name[strlen(dev->name)-2]=='n') //dev->name="wlanX"
	{
		printk("#### root=%x ####\n",(u32)dev);
		if(dev->name[strlen(dev->name)-1]=='0')
                        wlan_root_netdev=dev;
                else
                        wlan1_root_netdev=dev;
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
		DEBUG_INFO("Init %s, base_addr=%08lx, irq=%d\n",
			dev->name, (unsigned long)dev->base_addr,  dev->irq);

#else //  __DRAYTEK_OS__
	regs = (void *)wdev->base_addr;
	dev->base_addr = (unsigned long)wdev->base_addr;
	priv->pshare->ioaddr = (unsigned long)regs;

#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
		DEBUG_INFO("Init %s, base_addr=%08lx\n",
			dev->name, (unsigned long)dev->base_addr);

#endif // __DRAYTEK_OS__


#ifdef CONFIG_PCI_HCI
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
	{
#ifdef CONFIG_NET_PCI

#if defined(CONFIG_RTL8196C_REVISION_B) || defined(CONFIG_RTL_8196C)
			#if defined(CONFIG_NET_PCI)
				#define REVR                                    0xB8000000
				#define RTL8196C_REVISION_A     0x80000001
				#define RTL8196C_REVISION_B     0x80000002
			#endif
					if (REG32(REVR) == RTL8196C_REVISION_B) {
						REG32(0xb9000354)=0xc940; //Card PCIE PHY initial  parameter for rtl8196c revision B
								REG32(0xb9000358)=0x24;
						for(i=0; i<1000000; i++);
								REG32(0xb9000354)=0x4270;
								REG32(0xb9000358)=0x25;
						for(i=0; i<1000000; i++);
						REG32(0xb9000354)=0x019E; //Card PCIE PHY initial  parameter for rtl8196c revision B
								REG32(0xb9000358)=0x23;
					}
#endif
		if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
			if (cache_size != SMP_CACHE_BYTES) {
				panic_printk("%s: PCI cache line size set incorrectly (%i bytes) by BIOS/FW, ", dev->name, cache_size);
		        if (cache_size > SMP_CACHE_BYTES) {
	    	        DEBUG_INFO("expecting %i\n", SMP_CACHE_BYTES);
	        	} else {
	            	DEBUG_INFO("correcting to %i\n", SMP_CACHE_BYTES);
					pci_write_config_byte(pdev, PCI_CACHE_LINE_SIZE, SMP_CACHE_BYTES >> 2);
	    	    }
		    }

	    	/* enable busmastering and memory-write-invalidate */
		    pci_read_config_word(pdev, PCI_COMMAND, &pci_command);
	    	if (!(pci_command & PCI_COMMAND_INVALIDATE)) {
	        	pci_command |= PCI_COMMAND_INVALIDATE;
		        pci_write_config_word(pdev, PCI_COMMAND, pci_command);
	    	}
		    pci_set_master(pdev);
		}
#endif
	}
#endif // CONFIG_PCI_HCI

#ifdef __ECOS
#ifdef RTLWIFINIC_GPIO_CONTROL
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_WLAN_HAL_8192EE) //mark_ecos
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
		RTLWIFINIC_GPIO_init_priv(priv);
#endif
#endif
#else // !__ECOS
#ifdef _INCLUDE_PROC_FS_
#ifdef __KERNEL__
	rtl8192cd_proc_init(dev);
#ifdef PERF_DUMP
	{
		#include <linux/proc_fs.h>

		struct proc_dir_entry *res;
		res = create_proc_entry("perf_dump", 0, NULL);
		if (res) {
			res->read_proc = read_perf_dump;
			res->write_proc = flush_perf_dump;
			res->data = (void *)dev;
		}
	}
#endif
#endif
#endif
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
#if defined(CUSTOMIZE_WLAN_IF_NAME)
		if(strcmp(dev->name, WLAN0_IFNAME)==0)				
			dev->vlan_member_map=WLAN0_MASK_BIT;
		else if(strcmp(dev->name,WLAN1_IFNAME)==0)
			dev->vlan_member_map=WLAN1_MASK_BIT;

#else
        if(strcmp(dev->name,"wlan0")==0)                
            dev->vlan_member_map=WLAN0_MASK_BIT;
        else if(strcmp(dev->name,"wlan1")==0)
            dev->vlan_member_map=WLAN1_MASK_BIT;
#endif
#endif

	// set some default value of mib
	set_mib_default(priv);

#if defined(RTK_NL80211) && defined(UNIVERSAL_REPEATER)
	if(IS_VXD_INTERFACE(priv))	
	{
		OPMODE = WIFI_STATION_STATE;
	}
#endif

#ifdef RTK_BR_EXT
    nat25_filter_default(priv);
#endif

#ifdef DOT11K
    memset(priv->rm_ap_channel_report, 0x00, MAX_AP_CHANNEL_REPORT * sizeof(struct dot11k_ap_channel_report));
    memset(priv->rm_neighbor_bitmask, 0x00, sizeof(priv->rm_neighbor_bitmask));
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
	{
#ifndef __DRAYTEK_OS__
#ifdef WDS
		wds_num = (priv->pshare->type>>WDS_SHIFT) & WDS_MASK;
		strcpy(baseDevName, dev->name);

		for (i=0; i<wds_num; i++) {
#ifdef NETDEV_NO_PRIV
			struct rtl8192cd_wds_priv* priv_wds;
			dev = alloc_etherdev(sizeof(struct rtl8192cd_wds_priv));
#else
			dev = alloc_etherdev(0);
#endif
			if (!dev) {
				printk(KERN_ERR "alloc_etherdev() wds error!\n");
				rc = -ENOMEM;
	    	   	goto err_out_dev;
			}

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
		    dev->open = rtl8192cd_open;
		    dev->stop = rtl8192cd_close;
		    dev->hard_start_xmit = rtl8192cd_start_xmit;
		    dev->get_stats = rtl8192cd_get_stats;
#if defined(__KERNEL__) || defined(__OSK__)
		    dev->set_mac_address = rtl8192cd_set_hwaddr;
#endif
#else
		    dev->netdev_ops = &rtl8192cd_netdev_ops;
#endif

#if defined(CONFIG_RTL8672) && !defined(__OSK__)
			dev->priv_flags = IFF_DOMAIN_WLAN;
#endif

#ifdef __ECOS
			//dev->isr = rtl8192cd_interrupt;
			//dev->dsr= interrupt_dsr;
			dev->can_xmit = can_xmit;
#endif

			priv->wds_dev[i] = dev;
			strcpy(dev->name, baseDevName);
			strcat(dev->name, "-wds%d");
			
#ifdef NETDEV_NO_PRIV
			priv_wds = (struct rtl8192cd_wds_priv*)netdev_priv(dev);
			priv_wds->wlan_priv = priv;
#else
			dev->priv = priv;
#endif
#if defined(__KERNEL__) || defined(__OSK__)
#ifdef __OSK__
			rc = osk_register_netdev(dev);
#else
		    rc = register_netdev(dev);
#endif
			if (rc) {
				printk(KERN_ERR "register_netdev() wds error!\n");
				goto err_out_dev;
			}
#endif

#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
					if(dev->name[strlen(dev->name)-2]=='s') //dev->name="wlanX-wdsX"
					{
						int idx=dev->name[strlen(dev->name)-1]-'0';
						if((idx>=0)&&(idx<=7))
						{
							if(dev->name[strlen(dev->name)-6]=='0')
                                                                wlan_wds_netdev[idx]=dev;
                                                        else
                                                                wlan1_wds_netdev[idx]=dev;
						}
					}
#endif


		}
#endif // WDS

#ifdef CONFIG_RTK_MESH
        mesh_num = (priv->pshare->type>>MESH_SHIFT) & MESH_MASK;

        #ifndef WDS
        strcpy(baseDevName, dev->name);
        #endif
        if(mesh_num>0) {/*the first wlan interface with mesh enabled*/
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
			if(tmp_priv == NULL)
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
            {
                GET_MIB(priv)->dot1180211sInfo.mesh_enable = 1;
                dev = alloc_etherdev(0);	// mesh allocate ethernet device BUT don't have priv memory (Because share root priv)
                if (!dev) {
                    printk(KERN_ERR "alloc_etherdev() mesh error!\n");
                    rc = -ENOMEM;
                    goto err_out_dev;
                }
                dev->base_addr = 1;
                #if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
                dev->open = rtl8192cd_open;
                dev->stop = rtl8192cd_close;
                dev->hard_start_xmit = mesh_start_xmit;
                dev->get_stats = rtl8192cd_get_stats;
#if defined(__KERNEL__) || defined(__OSK__)
                dev->set_mac_address = rtl8192cd_set_hwaddr;
                #endif
                dev->do_ioctl = rtl8192cd_ioctl;
                #else
                dev->netdev_ops = &mesh_netdev_ops;
                #endif

#if defined(CONFIG_RTL8672) && !defined(__OSK__)
                dev->priv_flags = IFF_DOMAIN_WLAN;
#endif
                #ifdef __ECOS
                dev->can_xmit = can_xmit;
                #endif
                priv->mesh_dev = dev; // NO priv zone dev
                #ifdef __ECOS    			
                strcpy(dev->name, "wlan-msh0"); 
                #else
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
                strcpy(dev->name, "wlan-msh");
#else	//!CONFIG_RTL_MESH_SINGLE_IFACE
				strcpy(dev->name, baseDevName);
				strcat(dev->name, "-msh0");
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
                #endif                        

                // mesh priv pointer to root's priv
                #ifdef NETDEV_NO_PRIV
                ((struct rtl8192cd_priv*)netdev_priv(dev))->wlan_priv = priv;
                #else
                dev->priv = priv;
                #endif

#if defined(__KERNEL__) || defined(__OSK__)
#ifdef __OSK__
				rc = osk_register_netdev(dev);
#else
                rc = register_netdev(dev);
#endif
                if (rc) {
                    printk(KERN_ERR "register_netdev() mesh error!\n");
                    goto err_out_dev;
                }
                #endif
            }
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
            if(tmp_priv == NULL) {
                tmp_priv = priv;
            }
            else {
                priv->mesh_dev = tmp_priv->mesh_dev;
                tmp_priv->mesh_priv_sc = priv;
                priv->mesh_priv_sc = tmp_priv;
            }
            priv->mesh_priv_first = priv;
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
        } // end of if(mesh_num>0)

        priv->rreq_head = NULL;
        priv->rreq_tail = NULL;

        pann_mpp_tb = (struct mpp_tb*)kmalloc(sizeof(struct mpp_tb), GFP_ATOMIC);
        if(!pann_mpp_tb)
        {
            rc = -ENOMEM;
            printk("allocate pann_mpp_tb error!!\n");
            goto err_out_dev;
        }
        init_mpp_pool(pann_mpp_tb);
        proxy_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
        if(!proxy_table)
        {
            rc = -ENOMEM;
            printk("allocate proxy_table error!!\n");
            goto err_out_dev;
        }
        memset((void*)proxy_table, 0, sizeof(struct hash_table));

#ifdef PU_STANDARD
        //pepsi
        proxyupdate_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
        if(!proxyupdate_table)
        {
            rc = -ENOMEM;
            printk("allocate proxyupdate_table error!!\n");
            goto err_out_dev;
        }
        memset((void*)proxyupdate_table, 0, sizeof(struct hash_table));
#endif

        pathsel_queue = (DOT11_QUEUE2 *)kmalloc((sizeof(DOT11_QUEUE2)), GFP_ATOMIC);
        if (!pathsel_queue) {
            rc = -ENOMEM;
            printk(KERN_ERR "Can't kmalloc for PATHSELECTION_QUEUE (size %d)\n", sizeof(DOT11_QUEUE));
            goto err_out_dev;
        }
        memset((void *)pathsel_queue, 0, sizeof (DOT11_QUEUE2));

        pathsel_table = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
        if(!pathsel_table)
        {
            rc = -ENOMEM;
            printk("allocate pathsel_table error!!\n");
            goto err_out_dev;
        }
        memset((void*)pathsel_table, 0, sizeof(struct hash_table));

        mesh_rreq_retry_queue = (struct hash_table*)kmalloc(sizeof(struct hash_table), GFP_ATOMIC);
        if(!mesh_rreq_retry_queue)
        {
            rc = -ENOMEM;
            printk("allocate mesh_rreq_retry_queue error!!\n");
            goto err_out_dev;
        }
        memset((void*)mesh_rreq_retry_queue, 0, sizeof(struct hash_table));

        rc = init_hash_table(proxy_table, PROXY_TABLE_SIZE, MACADDRLEN, sizeof(struct proxy_table_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
        if(rc == HASH_TABLE_FAILED)
        {
            printk("init_hash_table \"proxy_table\" error!!\n");
        }

#ifdef PU_STANDARD
        //pepsi
        rc = init_hash_table(proxyupdate_table, 8, sizeof(UINT8), sizeof(struct proxyupdate_table_entry), PU_hashing, search_default, insert_default, delete_default,traverse_default);
        if(rc == HASH_TABLE_FAILED)
        {
            printk("init_hash_table \"proxyupdate_table\" error!!\n");
        }
#endif
        rc = init_hash_table(pathsel_table, PATHSEL_TABLE_SIZE, MACADDRLEN, sizeof(struct path_sel_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
        if(rc == HASH_TABLE_FAILED)
        {
            printk("init_hash_table \"pathsel_table\" error!!\n");
        }

        rc = init_hash_table(mesh_rreq_retry_queue, DATA_SKB_BUFFER_SIZE, MACADDRLEN, sizeof(struct mesh_rreq_retry_entry), crc_hashing, search_default, insert_default, delete_default,traverse_default);
        if(rc == HASH_TABLE_FAILED)
        {
            printk("init_hash_table \"mesh_rreq_retry_queue\" error!!\n");
        }

#ifdef PU_STANDARD
        priv->proxyupdate_table = proxyupdate_table;
#endif

        priv->proxy_table = proxy_table;
        priv->pathsel_queue = pathsel_queue;
        priv->pann_mpp_tb = pann_mpp_tb;
        priv->pathsel_table = pathsel_table;
        priv->mesh_rreq_retry_queue = mesh_rreq_retry_queue;
        //=========================================================
#endif // CONFIG_RTK_MESH


#ifdef STA_CONTROL
        if(stactrl_tmp_priv == NULL) {
            stactrl_tmp_priv = priv;
        }
        else {
            stactrl_tmp_priv->stactrl.stactrl_rootpriv_sc = priv;
            priv->stactrl.stactrl_rootpriv_sc = stactrl_tmp_priv;
        }
#endif

#ifdef CROSSBAND_REPEATER
		if(crossband_tmp_priv == NULL){
			crossband_tmp_priv = priv;
		}
		else{
			crossband_tmp_priv->crossband.crossband_rootpriv_sc = priv;
			priv->crossband.crossband_rootpriv_sc = crossband_tmp_priv;
		}
#endif

#endif  // __DRAYTEK_OS__


        //3 Require Descriptor Memory
        //Method:
        //  1.) Static Memory
        //  2.) Allocate memory from OS
#ifdef CONFIG_PCI_HCI
#ifdef  CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
#ifdef CONFIG_NET_PCI
			if (!IS_PCIBIOS_TYPE)
#endif
			{
#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
            _GET_HAL_DATA(priv)->desc_dma_buf       = desc_buf[priv->pshare->wlandev_idx];
            _GET_HAL_DATA(priv)->desc_dma_buf_len   = DESC_DMA_PAGE_SIZE_IF(priv);
#ifdef WLAN_SUPPORT_H2C_PACKET
            _GET_HAL_DATA(priv)->h2c_buf            = h2c_buf[priv->pshare->wlandev_idx];
            _GET_HAL_DATA(priv)->h2c_buf_len        = H2C_BUFFER_SIZE;
#endif // WLAN_SUPPORT_H2C_PACKET
#ifdef WLAN_HAL_TX_AMSDU
            _GET_HAL_DATA(priv)->desc_dma_buf_amsdu     = desc_buf_amsdu[priv->pshare->wlandev_idx];
            _GET_HAL_DATA(priv)->desc_dma_buf_len_amsdu = DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU;
#endif //WLAN_HAL_TX_AMSDU
#else
            _GET_HAL_DATA(priv)->desc_dma_buf       = desc_buf;
            _GET_HAL_DATA(priv)->desc_dma_buf_len   = DESC_DMA_PAGE_SIZE_IF(priv);
#ifdef WLAN_SUPPORT_H2C_PACKET
            _GET_HAL_DATA(priv)->h2c_buf            = h2c_buf;
            _GET_HAL_DATA(priv)->h2c_buf_len        = H2C_BUFFER_SIZE;
#endif // WLAN_SUPPORT_H2C_PACKET          
#ifdef WLAN_HAL_TX_AMSDU
            _GET_HAL_DATA(priv)->desc_dma_buf_amsdu     = desc_buf_amsdu;
            _GET_HAL_DATA(priv)->desc_dma_buf_len_amsdu = DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU;
#endif //WLAN_HAL_TX_AMSDU
#endif //CONCURRENT_MODE
#else
            #ifdef __DRAYTEK_OS__
		    page_ptr = rtl8185_malloc(DESC_DMA_PAGE_SIZE_IF(priv), 1);	// allocate non-cache buffer
#ifdef WLAN_SUPPORT_H2C_PACKET
            page_ptr_h2c = rtl8185_malloc(H2C_BUFFER_SIZE, 1);
#endif // WLAN_SUPPORT_H2C_PACKET          
#ifdef WLAN_HAL_TX_AMSDU
            page_ptr_amsdu = rtl8185_malloc(DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU, 1);
#endif
            #else
    		page_ptr = kmalloc(DESC_DMA_PAGE_SIZE_IF(priv), GFP_KERNEL);
#ifdef WLAN_SUPPORT_H2C_PACKET
            page_ptr_h2c = kmalloc(H2C_BUFFER_SIZE, GFP_KERNEL);
#endif // WLAN_SUPPORT_H2C_PACKET                     
#ifdef WLAN_HAL_TX_AMSDU
            page_ptr_amsdu = kmalloc(DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU, GFP_KERNEL);
#endif
            #endif

            _GET_HAL_DATA(priv)->desc_dma_buf = page_ptr;

		    if (page_ptr == NULL) {
    			rc = -ENOMEM;
    			panic_printk(KERN_ERR "can't allocate descriptior page, abort!\n");
                _GET_HAL_DATA(priv)->desc_dma_buf_len   = 0;
    			goto err_out_dev;
			} else {
                _GET_HAL_DATA(priv)->desc_dma_buf_len   = DESC_DMA_PAGE_SIZE_IF(priv);
            }
#ifdef WLAN_SUPPORT_H2C_PACKET
            _GET_HAL_DATA(priv)->h2c_buf = page_ptr_h2c;
            if (page_ptr_h2c == NULL) {
                kfree(page_ptr);
                rc = -ENOMEM;
                printk(KERN_ERR "can't allocate h2c buffer, abort!\n");
                goto err_out_dev;
            } else {
                _GET_HAL_DATA(priv)->h2c_buf_len = H2C_BUFFER_SIZE;
            }

#endif // WLAN_SUPPORT_H2C_PACKET                               
#ifdef WLAN_HAL_TX_AMSDU
            _GET_HAL_DATA(priv)->desc_dma_buf_amsdu = page_ptr_amsdu;
            if (page_ptr_amsdu == NULL) {
                kfree(page_ptr);
                rc = -ENOMEM;
                printk(KERN_ERR "can't allocate AMSDU descriptior page, abort!\n");
                goto err_out_dev;
            } else {
                _GET_HAL_DATA(priv)->desc_dma_buf_len_amsdu = DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU;
            }
#endif			
#endif //PRIV_STA_BUF
			}
			if (GET_HAL_INTERFACE(priv)->InitHCIDMAMemHandler(priv) == RT_STATUS_FAILURE){
				GET_HAL_INTERFACE(priv)->StopSWHandler(priv);
				rc = -ENOMEM;
				printk(KERN_ERR "can't allocate descriptior page, abort!\n");
#ifndef PRIV_STA_BUF
				if (page_ptr != NULL){
					kfree(page_ptr);
#ifdef WLAN_HAL_TX_AMSDU
					kfree(page_ptr_amsdu);
#endif
				}
#endif
				goto err_out_dev;
			}
		} else
#endif // CONFIG_WLAN_HAL
		{
#ifdef PRIV_STA_BUF
#ifdef CONCURRENT_MODE
		page_ptr = (unsigned char *)
			(((unsigned long)&desc_buf[priv->pshare->wlandev_idx]) + (PAGE_SIZE - (((unsigned long)&desc_buf[priv->pshare->wlandev_idx]) & (PAGE_SIZE-1))));
		phw->ring_buf_len = ((unsigned long)&desc_buf[priv->pshare->wlandev_idx]) + (sizeof(desc_buf)/NUM_WLAN_IFACE) - ((unsigned long)page_ptr);
		phw->ring_dma_addr = virt_to_bus(page_ptr);
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#else
		page_ptr = (unsigned char *)
			(((unsigned long)desc_buf) + (PAGE_SIZE - (((unsigned long)desc_buf) & (PAGE_SIZE-1))));
		phw->ring_buf_len = (unsigned long)desc_buf + sizeof(desc_buf) - (unsigned long)page_ptr;
		phw->ring_dma_addr = virt_to_bus(page_ptr);
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#endif		
#else
#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE) {
			dma_addr_t addr;
			// Avoid the combination of (64-bit dma_addr_t + big endian) to get invalid phw->ring_dma_addr
			page_ptr = pci_alloc_consistent(priv->pshare->pdev, DESC_DMA_PAGE_SIZE_IF(priv), &addr);
			phw->ring_dma_addr = addr;
		} else
#endif
		{
#ifdef __DRAYTEK_OS__
		page_ptr = rtl8185_malloc(DESC_DMA_PAGE_SIZE_IF(priv), 1);	// allocate non-cache buffer
#else
		page_ptr = kmalloc(DESC_DMA_PAGE_SIZE_IF(priv), GFP_KERNEL);
#endif
		}

		if (page_ptr == NULL) {
			rc = -ENOMEM;
			panic_printk(KERN_ERR "can't allocate descriptior page, abort!\n");
			goto err_out_dev;
		}

		phw->alloc_dma_buf = (unsigned long)page_ptr;
#if defined(NOT_RTK_BSP)		
		page_align_phy = (PAGE_SIZE - (((unsigned long)page_ptr) & (PAGE_SIZE-1)));
#endif	
		page_ptr = (unsigned char *)
			(((unsigned long)page_ptr) + (PAGE_SIZE - (((unsigned long)page_ptr) & (PAGE_SIZE-1))));
		phw->ring_buf_len = phw->alloc_dma_buf + DESC_DMA_PAGE_SIZE_IF(priv) - ((unsigned long)page_ptr);
#if defined(NOT_RTK_BSP)
		phw->ring_dma_addr = phw->ring_dma_addr + page_align_phy;  
#else
		phw->ring_dma_addr = virt_to_bus(page_ptr)+CONFIG_LUNA_SLAVE_PHYMEM_OFFSET;
#endif

#ifdef __MIPSEB__
		page_ptr = (unsigned char *)KSEG1ADDR(page_ptr);
#endif
#endif

		DEBUG_INFO("page_ptr=%lx, size=%ld\n",  (unsigned long)page_ptr, (unsigned long)DESC_DMA_PAGE_SIZE_IF(priv));
		phw->ring_virt_addr = (unsigned long)page_ptr;
		}
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_RTL8190_PRIV_SKB
		init_priv_skb_buf(priv);
#endif

#ifdef PRIV_STA_BUF
		init_priv_sta_buf(priv);
#endif

#ifdef CONFIG_RTL_92D_DMDP
		if_priv[priv->pshare->wlandev_idx] = (u32)priv;
#endif

#if defined(STA_CONTROL) && STA_CONTROL_ALGO == STA_CONTROL_ALGO3	//20170103
		if_priv_stactrl[priv->pshare->wlandev_idx] = (u32)priv;
#endif


#ifdef RX_LOOP_LIMIT
		if (GET_CHIP_VER(priv) == VERSION_8812E)
			priv->pmib->dot11StationConfigEntry.limit_rxloop = 150;
#endif
	}

	INIT_LIST_HEAD(&priv->asoc_list); // init assoc_list first because webs may get sta_num even it is not open,
#if defined(SMP_SYNC) //&& (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	spin_lock_init(&(priv->asoc_list_lock));
#endif
																// and it will cause exception if it is not init, david+2008-03-05
#ifdef EN_EFUSE
	rc = ReadAdapterInfo8192CE(priv);
	if (rc)
		goto err_out_dev;
#endif
#ifdef NOT_RTK_BSP
	printk("=====>> EXIT %s <<=====\n", __func__);
#endif

#if defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
	if(priv && RTL_R32(GPIO_PIN_CTRL))
	{
//		printk("<%s>LZQ: GPIO_PIN_CTRL[0x%x] \n",__FUNCTION__, GPIO_PIN_CTRL);
//		printk("<%s>LZQ: before read tmpReg[0x%x] \n",__FUNCTION__, RTL_R32(GPIO_PIN_CTRL));
		RTL_W32(GPIO_PIN_CTRL,RTL_R32(GPIO_PIN_CTRL)&(0x0));
		DEBUG_INFO("<%s>LZQ: after read tmpReg[0x%x] \n",__FUNCTION__, RTL_R32(GPIO_PIN_CTRL));
	}
#endif
	
#if defined(__KERNEL__) || defined(__OSK__)
	return 0;
#else
	//return (void *)dev;
	return (void *)tmp_dev;
#endif

err_out_dev:
	
	dev = priv->dev;
#ifdef _INCLUDE_PROC_FS_
	rtl8192cd_proc_remove(dev);
#endif
#if defined(__KERNEL__) || defined(__OSK__)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))  // is root interface
#endif
	{
#ifdef EN_EFUSE
		for (i = 0; i < 2; i++)
			if (priv->EfuseMap[i])
				kfree(priv->EfuseMap[i]);
#endif

#ifndef __DRAYTEK_OS__
#ifdef WDS
		wds_num = (priv->pshare->type>>WDS_SHIFT) & WDS_MASK;
		for (i=0; i<wds_num; i++) {
			if (priv->wds_dev[i]) {
				unregister_netdev(priv->wds_dev[i]);
#ifdef __LINUX_2_6__
				free_netdev(priv->wds_dev[i]);
#else
				kfree(priv->wds_dev[i]);
#endif
			}
		}
#endif // WDS

#ifdef CONFIG_RTK_MESH
		mesh_num = (priv->pshare->type>>MESH_SHIFT) & MESH_MASK;
		if(mesh_num>0) {
			if (priv->mesh_dev) {
				unregister_netdev(priv->mesh_dev);
#ifdef __LINUX_2_6__
				free_netdev(priv->mesh_dev);
#else
				kfree(priv->mesh_dev);
#endif
			}
		}
#endif // CONFIG_RTK_MESH
#endif // !__DRAYTEK_OS__
	}
	
	unregister_netdev(dev);

err_out_iomap:
#endif // __KERNEL__

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_NET_PCI
	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifndef USE_IO_OPS
	    iounmap(regs);
#endif
	}

#ifdef RTK_129X_PLATFORM
err_out_iomap_mask:
	iounmap((void *)priv->pshare->mask_addr);
#endif

err_out_res:

	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
#ifdef USE_IO_OPS
		release_region(dev->base_addr, pci_resource_len(pdev, 0));
////	release_region(dev->base_addr, pci_resource_len(pdev, 2));
#else
	    pci_release_regions(pdev);
#endif
	}

err_out_disable:

	if (((wdev->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
	    pci_disable_device(pdev);
#endif // CONFIG_NET_PCI
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
	rtw_free_drv_sw(priv);
	usb_dvobj_deinit(priv);
#endif

#ifdef CONFIG_SDIO_HCI
	rtw_free_drv_sw(priv);
	sdio_dvobj_deinit(priv);
#endif

err_out_free2:

#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		HalDisAssociateNic(priv, TRUE);
	}
#endif

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv)) { // is a vxd or vap
#ifdef UNIVERSAL_REPEATER
		if (vap_idx < 0) // create for vxd
			GET_VXD_PRIV(wdev->priv) = NULL;
#endif
#ifdef MBSSID
		if (vap_idx >= 0) // create for vap
			GET_ROOT(priv)->pvap_priv[vap_idx] = NULL;
#endif
	} else
#endif
	wdev->priv = NULL;

err_out_free:

	if (pmib){
#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(PMIB, pmib);
#else
		kfree(pmib);
#endif
	}

#ifdef  CONFIG_RTK_MESH
	if(proxy_table)
	{
		remove_hash_table(proxy_table);
		kfree(proxy_table);
	}
	if(mesh_rreq_retry_queue)
	{
		remove_hash_table(mesh_rreq_retry_queue);
		kfree(mesh_rreq_retry_queue);
	}

	// add by chuangch 2007.09.13
	if(pathsel_table)
	{
		remove_hash_table(pathsel_table);
		kfree(pathsel_table);
	}

	if(pann_mpp_tb)
		kfree(pann_mpp_tb);
	
#ifdef PU_STANDARD
	if (proxyupdate_table) {
		remove_hash_table(proxyupdate_table);
		kfree(proxyupdate_table);
	}
#endif
	if (pathsel_queue)
		kfree(pathsel_queue);
#endif	// CONFIG_RTK_MESH
#ifdef P2P_SUPPORT/*cfg p2p cfg p2p*/
	if(priv->p2pPtr)
		kfree(priv->p2pPtr);
#endif		

	if (pevent_queue)
		kfree(pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (wapiEvent_queue)
		kfree(wapiEvent_queue);
	#if 0 //defined(MBSSID)
	if (wapiVapEvent_queue)
		kfree(wapiVapEvent_queue);
	#endif
#endif
#ifdef CONFIG_IEEE80211R
	if (pftevent_queue)
		kfree(pftevent_queue);
#endif
#ifdef RSSI_MONITOR_NCR
	if (rssim_queue)
		kfree(rssim_queue);
#endif
#ifndef PRIV_STA_BUF
#ifndef USE_DMA_ALLOCATE
	if (phw)
		kfree(phw);
#else
	if (phw) {
		printk("****** %s %d, free allocated addr %pa\n", __FUNCTION__, __LINE__, &phw);
		dma_free_coherent(cma_dev, sizeof(struct rtl8192cd_hw), phw, dma_phys);
		phw = NULL;
		dma_phys = NULL;
	}
#endif
	if (pshare)	// david
		kfree(pshare);
	if (pwlan_hdr_poll)
		kfree(pwlan_hdr_poll);
	if (pwlanllc_hdr_poll)
		kfree(pwlanllc_hdr_poll);
	if (pwlanbuf_poll)
		kfree(pwlanbuf_poll);
	if (pwlanicv_poll)
		kfree(pwlanicv_poll);
	if (pwlanmic_poll)
		kfree(pwlanmic_poll);
#endif
	if (pwlan_acl_poll)
		kfree(pwlan_acl_poll);

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)	// below code copy above ACL code
	if (pmesh_acl_poll)
		kfree(pmesh_acl_poll);
#endif

	if (Eap_packet)
		kfree(Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	if (wpa_global_info)
		kfree(wpa_global_info);
#endif
	if (site_survey)
		free_site_survey_res(site_survey);

#ifdef __LINUX_2_6__
	free_netdev(dev);
#else
    kfree(dev);
#endif

	printk("=====>> EXIT %s (%d) <<=====\n", __func__, rc);

#if defined(__KERNEL__) || defined(__OSK__)
    return rc;
#else
	return NULL;
#endif
}

#ifdef RTK_NL80211
void rtl8192cd_init_one_cfg80211(struct rtknl *rtk)
{
	int wlan_index=0;

	printk("roo_ifname = %s, rtk->num_vif=%d \n", rtk->root_ifname, rtk->num_vif);
	
	if(!strcmp(rtk->root_ifname, "wlan1")) //if it is	second wlan phy
		wlan_index =1;

	rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], rtk->num_vif);
	
}
#endif

void rtl8192cd_deinit_one(struct rtl8192cd_priv *priv)
{
	struct net_device *dev;
#if defined(WDS) || defined(MBSSID) || defined(CONFIG_RTK_MESH) || defined(EN_EFUSE)
	int i;
#endif
#if defined(WDS) || defined(CONFIG_RTK_MESH)
	int num;
#endif
#ifdef UNIVERSAL_REPEATER
	struct rtl8192cd_priv *vxd_priv = NULL;
#endif
#ifdef MBSSID
	struct rtl8192cd_priv *vap_priv = NULL;
#endif
	if (NULL == priv) {
		panic_printk("%s: priv should NOT be a NULL pointer!\n", __func__);
		return;
	} else {
#ifdef NOT_RTK_BSP
		printk("=====>> INSIDE %s <<=====\n", __func__);
#endif
	}

#ifdef WDS
	num = (priv->pshare->type >> WDS_SHIFT) & WDS_MASK;
	for (i = 0; i < num; i++) {
#if !defined(__OSK__)
		unregister_netdev(priv->wds_dev[i]);
#ifndef NETDEV_NO_PRIV
		priv->wds_dev[i]->priv = NULL;
#endif
#ifdef __LINUX_2_6__
		free_netdev(priv->wds_dev[i]);
#else
		kfree(priv->wds_dev[i]);
#endif
#endif
	}
#endif // WDS

#ifdef CONFIG_RTK_MESH
    num = (priv->pshare->type >> MESH_SHIFT) & MESH_MASK;
    if (num > 0) { // mesh_num is always 0 or 1
        unregister_netdev(priv->mesh_dev);
        // mesh priv pointer to root's priv
        #ifdef NETDEV_NO_PRIV
        ((struct rtl8192cd_priv*)netdev_priv(priv->mesh_dev))->wlan_priv = NULL;
        #else
        priv->mesh_dev->priv = NULL; // mesh priv pointer to root's priv
        #endif

        #ifdef __LINUX_2_6__
        free_netdev(priv->mesh_dev);
        #else
        kfree(priv->mesh_dev);
        #endif
    }
#endif // CONFIG_RTK_MESH

#ifdef UNIVERSAL_REPEATER
	vxd_priv = priv->pvxd_priv;
	if (vxd_priv) {
		unregister_netdev(vxd_priv->dev);
#ifdef _INCLUDE_PROC_FS_
		rtl8192cd_proc_remove(vxd_priv->dev);
#endif
#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(PMIB, vxd_priv->pmib);
#else
		kfree(vxd_priv->pmib);
#endif
		kfree(vxd_priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
		kfree(vxd_priv->wapiEvent_queue);
#endif
#ifdef CONFIG_IEEE80211R
		kfree(vxd_priv->pftevent_queue);
#endif
#ifdef RSSI_MONITOR_NCR
		kfree(vxd_priv->rssimEvent_queue);
#endif
		kfree(vxd_priv->pwlan_acl_poll);
		kfree(vxd_priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		kfree(vxd_priv->wpa_global_info);
#endif
		free_site_survey_res(vxd_priv->site_survey);
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
		kfree(vxd_priv->pmesh_acl_poll);
#endif
#ifdef __LINUX_2_6__
		free_netdev(vxd_priv->dev);
#else
		kfree(vxd_priv->dev);
#endif
		priv->pvxd_priv = NULL;
	}
#endif // UNIVERSAL_REPEATER

#ifdef MBSSID
	for (i = 0; i < RTL8192CD_NUM_VWLAN; i++) {
		vap_priv = priv->pvap_priv[i];
		if (vap_priv) {
			unregister_netdev(vap_priv->dev);
#ifdef _INCLUDE_PROC_FS_
			rtl8192cd_proc_remove(vap_priv->dev);
#endif
#ifdef RTL8192CD_VARIABLE_USED_DMEM
			rtl8192cd_dmem_free(PMIB, vap_priv->pmib);
#else
			kfree(vap_priv->pmib);
#endif
			kfree(vap_priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
			kfree(vap_priv->wapiEvent_queue);
#endif
#ifdef CONFIG_IEEE80211R
			kfree(vap_priv->pftevent_queue);
#endif
#ifdef RSSI_MONITOR_NCR
			kfree(vap_priv->rssimEvent_queue);
#endif
			kfree(vap_priv->pwlan_acl_poll);
			kfree(vap_priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
			kfree(vap_priv->wpa_global_info);
#endif

            free_site_survey_res(vap_priv->site_survey);

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
			kfree(vap_priv->pmesh_acl_poll);
#endif
#ifdef __LINUX_2_6__
			free_netdev(vap_priv->dev);
#else
			kfree(vap_priv->dev);
#endif
			priv->pvap_priv[i] = NULL;
		}
	}
#endif // MBSSID

	// root interface clean
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
	if (priv->pWlanDev) {
		unregister_netdev(priv->pWlanDev);
#ifdef __LINUX_2_6__
		free_netdev(priv->pWlanDev);
#else
		kfree(priv->pWlanDev);
#endif
		priv->pWlanDev = NULL;
	}
#endif
	dev = priv->dev;
	unregister_netdev(dev);
#ifdef _INCLUDE_PROC_FS_
	rtl8192cd_proc_remove(dev);
#endif

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_NET_PCI
#ifdef USE_IO_OPS
	release_region(dev->base_addr, pci_resource_len(priv->pshare->pdev, 0));
#else
#ifdef RTK_129X_PLATFORM
	iounmap((void *)priv->pshare->mask_addr);
#endif
	iounmap((void *)dev->base_addr);
	pci_release_regions(priv->pshare->pdev);
#endif
	pci_disable_device(priv->pshare->pdev);
#endif // CONFIG_NET_PCI

#ifndef PRIV_STA_BUF
#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
#ifdef CONFIG_NET_PCI
		if (!IS_PCIBIOS_TYPE)
#endif
		{
			if (_GET_HAL_DATA(priv)->desc_dma_buf) {
				kfree(_GET_HAL_DATA(priv)->desc_dma_buf);
				_GET_HAL_DATA(priv)->desc_dma_buf = NULL;
			}
#ifdef WLAN_SUPPORT_H2C_PACKET
            if (_GET_HAL_DATA(priv)->h2c_buf) {
				kfree(_GET_HAL_DATA(priv)->h2c_buf);
				_GET_HAL_DATA(priv)->h2c_buf = NULL;
			}
#endif //#WLAN_SUPPORT_H2C_PACKET
#ifdef WLAN_HAL_TX_AMSDU
            if (_GET_HAL_DATA(priv)->desc_dma_buf_amsdu) {
                kfree(_GET_HAL_DATA(priv)->desc_dma_buf_amsdu);
                _GET_HAL_DATA(priv)->desc_dma_buf_amsdu = NULL;
			}
#endif
		}
		GET_HAL_INTERFACE(priv)->StopSWHandler(priv);
	} else
#endif // CONFIG_WLAN_HAL
	{
#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE) {
			unsigned long page_align_phy = (PAGE_SIZE - (((unsigned long)priv->pshare->phw->alloc_dma_buf) & (PAGE_SIZE-1)));
			pci_free_consistent(priv->pshare->pdev, DESC_DMA_PAGE_SIZE_IF(priv), (void*)priv->pshare->phw->alloc_dma_buf,
				(dma_addr_t)(priv->pshare->phw->ring_dma_addr-page_align_phy));
		} else
#endif
			kfree((void *)priv->pshare->phw->alloc_dma_buf);
	}
#endif // !PRIV_STA_BUF
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
	rtw_free_drv_sw(priv);
	usb_dvobj_deinit(priv);
#endif
#ifdef CONFIG_SDIO_HCI
	rtw_free_drv_sw(priv);
	sdio_dvobj_deinit(priv);
#endif

#ifdef MBSSID
#if defined(__KERNEL__) && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	_rtw_mutex_free(&priv->pshare->mbssid_lock);
#endif
#endif // MBSSID

#ifdef RTL8192CD_VARIABLE_USED_DMEM
	rtl8192cd_dmem_free(PMIB, priv->pmib);
#else
	kfree(priv->pmib);
#endif
	kfree(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	kfree(priv->wapiEvent_queue);
#endif
#ifdef CONFIG_IEEE80211R
	kfree(priv->pftevent_queue);
#endif
#ifdef RSSI_MONITOR_NCR
	kfree(priv->rssimEvent_queue);
#endif
	kfree(priv->pwlan_acl_poll);
	kfree(priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	kfree(priv->wpa_global_info);
#endif
	free_site_survey_res(priv->site_survey);

#ifdef P2P_SUPPORT
	if(priv->p2pPtr)
		kfree(priv->p2pPtr);
#endif

#ifdef STA_CONTROL
    if(priv->stactrl.stactrl_preferband_ent)
        kfree(priv->stactrl.stactrl_preferband_ent);
    if(priv->stactrl.stactrl_block_ent) 
        kfree(priv->stactrl.stactrl_block_ent);
#endif

#ifdef CONFIG_RTK_MESH
	remove_hash_table(priv->proxy_table);
	kfree(priv->proxy_table);

	remove_hash_table(priv->pathsel_table);
	kfree(priv->pathsel_table);

	remove_hash_table(priv->mesh_rreq_retry_queue);
	kfree(priv->mesh_rreq_retry_queue);
#ifdef PU_STANDARD
	remove_hash_table(priv->proxyupdate_table);
	kfree(priv->proxyupdate_table);
#endif
	kfree(priv->pann_mpp_tb);
	kfree(priv->pathsel_queue);

#ifdef	_MESH_ACL_ENABLE_
	kfree(priv->pmesh_acl_poll);
#endif
#endif // CONFIG_RTK_MESH

#ifndef PRIV_STA_BUF
	kfree(priv->pshare->pwlan_hdr_poll);
	kfree(priv->pshare->pwlanllc_hdr_poll);
	kfree(priv->pshare->pwlanbuf_poll);
	kfree(priv->pshare->pwlanicv_poll);
	kfree(priv->pshare->pwlanmic_poll);
#ifndef USE_DMA_ALLOCATE
	kfree(priv->pshare->phw);
#else
	if (priv->pshare->phw) {
		dma_free_coherent(cma_dev, sizeof(struct rtl8192cd_hw), priv->pshare->phw, priv->pshare->hw_dma_phys);
		priv->pshare->phw = NULL;
		priv->pshare->hw_dma_phys = NULL;
	}
#endif
	kfree(priv->pshare);	// david
#endif

#ifdef EN_EFUSE
	for( i = 0 ; i < 2 ; i++ )
		kfree(priv->EfuseMap[i]);
#endif

#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
		HalDisAssociateNic(priv, TRUE);
	}
#endif

#ifdef __LINUX_2_6__
	free_netdev(dev);
#else
	kfree(dev);
#endif
	printk("=====>> EXIT %s <<=====\n", __func__);
}


#if 0//def CONFIG_RTL_STP
extern int rtl865x_wlanIF_Init(struct net_device *dev);
static int rtl_pseudo_dev_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;

	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);
	for (i = 0; i<MACADDRLEN; ++i) {
		dev->dev_addr[i] = p[i];
	}
	local_irq_restore(flags);
	return SUCCESS;
}

#if defined(__LINUX_2_6__) && !defined(CONFIG_COMPAT_NET_DEV_OPS)
static const struct net_device_ops rtl8192cd_rtl_pseudodev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl_pseudo_dev_set_hwaddr,
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
};
#endif

void rtl_pseudo_dev_init(void* priv)
{
	struct net_device *dev;

/*	printk("[%s][%d] priv of %s\n", __FUNCTION__, __LINE__, ((struct rtl8192cd_priv*)priv)->dev->name);*/
	dev = alloc_etherdev(0);
	if (dev == NULL) {
		printk("alloc_etherdev() pseudo port5 error!\n");
		return;
	}

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
	dev->open = rtl8192cd_open;
	dev->stop = rtl8192cd_close;
	dev->hard_start_xmit = rtl8192cd_start_xmit;
	dev->get_stats = rtl8192cd_get_stats;
	dev->do_ioctl = rtl8192cd_ioctl;
	dev->set_mac_address = rtl_pseudo_dev_set_hwaddr;
#else
	dev->netdev_ops = &rtl8192cd_rtl_pseudodev_ops;
#endif
	dev->priv = priv;
	strcpy(dev->name, "port5");
	memcpy((char*)dev->dev_addr,"\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);
	if (register_netdev(dev)) {
		printk(KERN_ERR "register_netdev() wds error!\n");
	}
	rtl865x_wlanIF_Init(dev);
}
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
__DRAM_IN_865X int passThruStatusWlan;
int passThruWanIdx;
#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *resPassThruWlan=NULL;
static char passThru_flag_wlan[10];
#endif

static int rtl_passthru_pseudo_dev_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags;
	int i;
	unsigned char *p;

	p = ((struct sockaddr *)addr)->sa_data;
 	local_irq_save(flags);
	for (i = 0; i<MACADDRLEN; ++i) {
		dev->dev_addr[i] = p[i];
	}
	local_irq_restore(flags);
	return SUCCESS;
}

#if defined(__LINUX_2_6__) && !defined(CONFIG_COMPAT_NET_DEV_OPS)
static const struct net_device_ops rtl8192cd_pseudodev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl_passthru_pseudo_dev_set_hwaddr,
#if defined(CONFIG_RTL8685) || defined(CONFIG_RTL8685S)
        .ndo_change_rx_flags = rtl8192cd_set_rx_mode,
#else
#if LINUX_VERSION_CODE<KERNEL_VERSION(2,6,42) || \
				(LINUX_VERSION_CODE>=KERNEL_VERSION(3,0,0) && LINUX_VERSION_CODE<KERNEL_VERSION(3,2,0))
		.ndo_set_multicast_list = rtl8192cd_set_rx_mode,
#else
		.ndo_set_rx_mode		= rtl8192cd_set_rx_mode,
#endif
#endif
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
};
#endif

void rtl_passthru_pseudo_dev_init(void *priv)
{
	struct net_device *dev;
	//struct rtl8192cd_priv *dp;

#ifdef __ECOS
	if(((struct rtl8192cd_priv *)priv)->pWlanDev) {
		diag_printf("pwlan0 exist already!\n");
		return;
	}
#endif

	dev = alloc_etherdev(sizeof(struct rtl8192cd_priv));
	if (dev == NULL) {
		panic_printk("alloc_etherdev() pseudo pwlan0 error!\n");
		return;
	}

#if !defined(__LINUX_2_6__) || defined(CONFIG_COMPAT_NET_DEV_OPS)
		dev->open = rtl8192cd_open;
		dev->stop = rtl8192cd_close;
#if defined(__KERNEL__)
		dev->set_multicast_list = rtl8192cd_set_rx_mode;
#endif
		dev->hard_start_xmit = rtl8192cd_start_xmit;
		dev->get_stats = rtl8192cd_get_stats;
		dev->do_ioctl = rtl8192cd_ioctl;
#ifndef __ECOS
		dev->set_mac_address = rtl_passthru_pseudo_dev_set_hwaddr;
#endif
#else
		dev->netdev_ops = &rtl8192cd_pseudodev_ops;
#endif

#ifdef __ECOS
	dev->isr = __rtl8192cd_interrupt;
	dev->dsr= interrupt_dsr;
	dev->can_xmit = can_xmit;
#endif

#ifdef NETDEV_NO_PRIV
	((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv = priv;
#else
	dev->priv = priv;
#endif

	strcpy(dev->name, "pwlan0");

	memcpy((char*)dev->dev_addr,"\x00\xe0\x4c\x81\x96\x96", MACADDRLEN);
		
#ifdef __KERNEL__
	if (register_netdev(dev)) {
		printk(KERN_ERR "register_netdev() pwlan0 error!\n");
	}
#endif

	((struct rtl8192cd_priv *)priv)->pWlanDev=dev;	//pWlanDev point to the virtual pwlan0

#ifdef __ECOS
	diag_printf("[%s] added...\n", dev->name);
#endif
}


static unsigned long atoi_dec(char *s)
{
	unsigned long k = 0;

	k = 0;
	while (*s != '\0' && *s >= '0' && *s <= '9') {
		k = 10 * k + (*s - '0');
		s++;
	}
	return k;
}

#ifdef CONFIG_PROC_FS
#ifdef CONFIG_RTL_PROC_NEW
static int wlan_custom_Passthru_read_proc(struct seq_file *s, void *data)
{
	seq_printf(s, "%s\n", passThru_flag_wlan);
	return 0;
}
#else
static int wlan_custom_Passthru_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
	int len;	

	len = sprintf(page, "%s\n", passThru_flag_wlan);
	if (len <= off+count) 
		*eof = 1;

	*start = page + off;
	len -= off;

	if (len>count) 
		len = count;

	if (len<0) len = 0;

	return len;
}
#endif
static int wlan_custom_Passthru_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	int flag /*,i*/;

	if (buffer && !copy_from_user(&passThru_flag_wlan, buffer, count))
	{			
		flag=(int)atoi_dec(passThru_flag_wlan);
		
		passThruStatusWlan=flag;
		
		return count;
	}
	return -EFAULT;
}

#ifdef CONFIG_RTL_PROC_NEW
int wlan_custom_Passthru_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, wlan_custom_Passthru_read_proc,NULL));
}
static ssize_t wlan_custom_Passthru_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return wlan_custom_Passthru_write_proc(file, userbuf,count, off);
}

struct file_operations wlan_custom_Passthru_proc_fops = {
	 .open            = wlan_custom_Passthru_single_open,	
	 .write           = wlan_custom_Passthru_single_write,
     .read           = seq_read,
     .llseek         = seq_lseek,
     .release        = single_release,
};
#endif

#endif //CONFIG_PROC_FS

int rtl_wlan_customPassthru_init(void)
{
	int	wlan_idx;

	for(wlan_idx=0;wlan_idx<sizeof(wlan_device)/sizeof(struct _device_info_);wlan_idx++)
	{
		if (wlan_device[wlan_idx].priv!=NULL)
			break;
	}

	if (wlan_idx==sizeof(wlan_device)/sizeof(struct _device_info_)) {
#ifdef __ECOS
		panic_printk("======EINVAL!!!wlan_idx:%d,[%s]:[%d].\n",wlan_idx,__FUNCTION__,__LINE__);
#endif
		return -EINVAL;
	}
	passThruWanIdx = wlan_idx;

#ifdef CONFIG_PROC_FS
	//initial
	memset(passThru_flag_wlan,0, sizeof(passThru_flag_wlan));
	//strcpy(passThru_flag_wlan,"0");
	passThruStatusWlan=0;
#ifdef CONFIG_RTL_PROC_NEW
	proc_create_data("custom_Passthru_wlan", 0, NULL,&wlan_custom_Passthru_proc_fops, NULL);
#else
	resPassThruWlan = create_proc_entry("custom_Passthru_wlan", 0, NULL);	
	if(resPassThruWlan)
	{
		resPassThruWlan->read_proc = wlan_custom_Passthru_read_proc;
		resPassThruWlan->write_proc = wlan_custom_Passthru_write_proc;
	}
#endif
#endif

	rtl_passthru_pseudo_dev_init(wlan_device[passThruWanIdx].priv);
	
	return 0;
}

void __exit rtl_wlan_customPassthru_exit(void)
{
#ifdef CONFIG_PROC_FS
	if (resPassThruWlan) {
		remove_proc_entry("custom_Passthru_wlan", resPassThruWlan);				
		resPassThruWlan = NULL;
	}
#endif	
}
#endif



#if defined(__DRAYTEK_OS__) && defined(WDS)
int rtl8192cd_add_wds(struct net_device *dev, struct net_device *wds_dev, unsigned char *addr)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;
	int wds_num=priv->pmib->dot11WdsInfo.wdsNum;

	priv->pmib->dot11WdsInfo.dev[wds_num] = wds_dev;
	memcpy(priv->pmib->dot11WdsInfo.entry[wds_num].macAddr, addr, 6);
	wds_dev->priv = priv;
	wds_dev->base_addr = 0;
	priv->pmib->dot11WdsInfo.wdsNum++;

	if (!priv->pmib->dot11WdsInfo.wdsEnabled)
		priv->pmib->dot11WdsInfo.wdsEnabled = 1;

	if (netif_running(priv->dev))
		create_wds_tbl(priv);

	DEBUG_INFO("\r\nAdd WDS: %02x%02x	%02x%02x%02x%02x\n",
		addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
}
#endif

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_NET_PCI
static int MDL_DEVINIT rtl8192cd_init_pci(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int ret;
#ifdef MBSSID
	int i;
#endif

#ifdef RTL8192CD_VARIABLE_USED_DMEM
	/* For D-MEM allocation system's initialization : It would before ALL processes */
	rtl8192cd_dmem_init();
#endif
	
#ifdef NOT_RTK_BSP
	if (wlan_index >= ARRAY_SIZE(wlan_device)) {
		printk("PCI device %d can't be supported\n", wlan_index);
		return -1;
	}
#endif
	ret = rtl8192cd_init_one(pdev, ent, &wlan_device[wlan_index], -1);
	if (ret)
		goto error;

#ifdef UNIVERSAL_REPEATER
	ret = rtl8192cd_init_one(pdev, ent, &wlan_device[wlan_index], -1);
	if (ret)
		goto error;
#endif

#ifdef MBSSID
	for (i = 0; i < RTL8192CD_NUM_VWLAN; i++) {
		ret = rtl8192cd_init_one(pdev, ent, &wlan_device[wlan_index], i);
		if (ret)
			goto error;
	}
#endif

	wlan_index++;

	return 0;

error:
	if (NULL != wlan_device[wlan_index].priv) {
		rtl8192cd_deinit_one(wlan_device[wlan_index].priv);
#ifdef RTK_NL80211
		rtk_remove_dev(wlan_device[wlan_index].priv->rtk, wlan_index);
#endif
		wlan_device[wlan_index].priv = NULL;
	}

	return ret;
}


static void MDL_DEVEXIT rtk_remove_one(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	if (!dev)
		BUG();
	
	pci_set_drvdata(pdev, NULL);

	wlan_index--;

	if (NULL != wlan_device[wlan_index].priv) {
		rtl8192cd_deinit_one(wlan_device[wlan_index].priv);
#ifdef RTK_NL80211
		rtk_remove_dev(wlan_device[wlan_index].priv->rtk, wlan_index);
#endif
		wlan_device[wlan_index].priv = NULL;
	}
}


static struct pci_device_id MDL_DEVINITDATA rtl8192cd_pci_tbl[] =
{
/*
	{ PCI_VENDOR_ID_REALTEK, 0x8190,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
*/
#ifdef CONFIG_RTL_92D_SUPPORT
	{ PCI_VENDOR_ID_REALTEK, 0x8193,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
	{ PCI_VENDOR_ID_REALTEK, 0x8191,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8171,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8178,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8176,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

	{ PCI_VENDOR_ID_REALTEK, 0x8174,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },

#ifdef CONFIG_RTL_88E_SUPPORT
    { PCI_VENDOR_ID_REALTEK, 0x8179,
      PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
#if defined(CONFIG_WLAN_HAL_8192EE)
    { PCI_VENDOR_ID_REALTEK, 0x8170,
      PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
    { PCI_VENDOR_ID_REALTEK, 0x818b,//0x817b
      PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },      
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
{ PCI_VENDOR_ID_REALTEK, 0x8813,
  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
#if defined(CONFIG_WLAN_HAL_8822BE) //eric-8282 ??
{ PCI_VENDOR_ID_REALTEK, 0x8822,
  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
{ PCI_VENDOR_ID_REALTEK, 0xB822,
  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
#ifdef CONFIG_RTL_8812_SUPPORT //eric_8812 ??
	{ PCI_VENDOR_ID_REALTEK, 0x8812,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
#endif
	{ },
};


MODULE_DEVICE_TABLE(pci, rtl8192cd_pci_tbl);

static struct pci_driver rtl8192cd_driver = {
	name:		DRV_NAME,
	id_table:	rtl8192cd_pci_tbl,
	probe:		rtl8192cd_init_pci,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
	remove:		__devexit_p(rtk_remove_one),
#else
	remove: 	rtk_remove_one,
#endif
};
#endif // CONFIG_NET_PCI
#endif // CONFIG_PCI_HCI


#ifdef CONFIG_WIRELESS_LAN_MODULE
int GetCpuCanSuspend(void)
{
	extern int gCpuCanSuspend;
	return gCpuCanSuspend;
}
#endif


#if defined(CONFIG_RTL8196B)
//System identification for CHIP
#define CHIP_OEM_ID	0xb8000000
#define DDR_SELECT	0xb8000008
#define C_CUT		2
#define DDR_BOOT	2
int no_ddr_patch;
#endif

#if defined(CONFIG_RTL_8196D) || defined(CONFIG_RTL_8196E)
unsigned int get_8192cd_gpio0_7(void)
{
	int i;
	struct rtl8192cd_priv *priv;
	struct net_device *dev;
	unsigned int reg = 0; 
	dev = NULL;
	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)) && (dev==NULL); i++) 
	{
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev)) 
		{
			priv = wlan_device[i].priv;
			reg = RTL_R32(GPIO_PIN_CTRL);
			DEBUG_INFO("<%s>LZQ: read 8192cd gpio reg [0x%x]!\n", __FUNCTION__, reg);			
			return reg;
		}
	}	
	panic_printk("<%s>LZQ: read 8192cd gpio reg ERROR!\n", __FUNCTION__);
	return 0;
}
#endif

#ifndef __ECOS
int MDL_INIT __rtl8192cd_init(unsigned long base_addr)
{
	int rc;
#ifdef MBSSID
	int i;
#endif

#if defined(CONFIG_RTL8196B)
	//System identification for CHIP
	no_ddr_patch = !((REG32(CHIP_OEM_ID)<C_CUT)&(REG32(DDR_SELECT)&&DDR_BOOT));
#endif

#ifdef __KERNEL__
#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif

#if defined(CONFIG_WIRELESS_LAN_MODULE) && !defined(NOT_RTK_BSP)
	wirelessnet_hook = GetCpuCanSuspend;
#ifdef BR_SHORTCUT
	wirelessnet_hook_shortcut = get_shortcut_dev;
#endif
#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = rtl8651_romeperfEnterPoint;
	Fn_rtl8651_romeperfExitPoint = rtl8651_romeperfExitPoint;
#endif
#ifdef CONFIG_RTL8190_PRIV_SKB
	wirelessnet_hook_is_priv_buf = is_rtl8190_priv_buf;
	wirelessnet_hook_free_priv_buf = free_rtl8190_priv_buf;
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE && !NOT_RTK_BSP
#endif // __KERNEL__

#ifndef GREEN_HILL
#ifdef CONFIG_RTL8671
	printk("%s driver version %d.%d.%d (%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_VERSION_SUBL, DRV_RELDATE);
#else
#if defined(SVN_REV)
	panic_printk("%s - version %d.%d (%s)(SVN:%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_RELDATE, SVN_REV);
#else
	panic_printk("%s - version %d.%d (%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_RELDATE);
#endif
#endif
#endif

#ifdef DFS
	panic_printk("DFS function - version %s\n", get_DFS_version());
#endif
#ifdef USE_OUT_SRC
	panic_printk("Adaptivity function - version %s\n", Get_Adaptivity_Version());
#endif
#if (defined(__KERNEL__) && !defined(CONFIG_WIRELESS_LAN_MODULE)) || defined(__OSK__)
	for (wlan_index = 0; wlan_index < ARRAY_SIZE(wlan_device); wlan_index++)
#else
#if defined(RTK_NL80211) && !defined(RTK_129X_PLATFORM)//dual phy support //eric-sync ??
	for (wlan_index=0; wlan_index<ARRAY_SIZE(wlan_device); wlan_index++)
#else
	if (wlan_index < ARRAY_SIZE(wlan_device))
#endif
#endif
	{
		_DEBUG_INFO(" wlan_index:%d  %d, %d,0x%lx,0x%lx,%d\n", wlan_index, (((wlan_device[wlan_index].type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS),
			wlan_device[wlan_index].type, wlan_device[wlan_index].base_addr,wlan_device[wlan_index].conf_addr, wlan_device[wlan_index].irq);
		
		if (((wlan_device[wlan_index].type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
			if (FALSE == drv_registered) {
				drv_registered = TRUE;
#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_NET_PCI
#ifdef LINUX_2_6_20_
				rc = pci_register_driver(&rtl8192cd_driver);
				if (rc) {
					panic_printk("pci_register_driver() fail!(%d)\n", rc);
				}
#else
				pci_module_init(&rtl8192cd_driver);
#endif
#endif
#endif // CONFIG_PCI_HCI
#ifdef CONFIG_USB_HCI
				rc = usb_register(&rtl8192cd_usb_driver);
				if (rc) {
					printk("usb_register() fail!(%d)\n", rc);
				}
#endif // CONFIG_USB_HCI
#ifdef CONFIG_SDIO_HCI
				rc = sdio_register_driver(&rtl8192cd_sdio_driver);
				if (rc) {
					printk("sdio_register_driver() fail!(%d)\n", rc);
				}
#endif // CONFIG_SDIO_HCI
			}
		}
		else {
#ifdef __DRAYTEK_OS__
			wlan_device[wlan_index].base_addr = base_addr;
			wlan_device[wlan_index].type = (TYPE_PCI_DIRECT<<TYPE_SHIFT);
#endif
			rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);
			if (rc)
				printk("init_one fail!!! rc=%d\n",rc);

#ifdef UNIVERSAL_REPEATER
			if (rc == 0)
				rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);
#endif
#ifdef MBSSID
			if (rc == 0) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], i);
					if (rc != 0) {
						printk("Init fail! rc=%d\n", rc);
						break;
					}
				}
			}
#endif
#if defined(CONFIG_RTL8672) && defined(CONFIG_RTL_92C_SUPPORT)
			if (rc == 0) {
				// switch XTAL_BSEL to NAND only for ADSL platform because external 40M crystal only used for wifi chip
				struct rtl8192cd_priv *priv = wlan_device[wlan_index].priv;

				if ((GET_CHIP_VER(priv) == VERSION_8188C) || (GET_CHIP_VER(priv) == VERSION_8192C))
				{
					if (RTL_R8(AFE_XTAL_CTRL) & BIT(1)) {	
						unsigned long	flags;

						SAVE_INT_AND_CLI(flags);
						
						rtl8192cd_open(priv->dev);
						rtl8192cd_close(priv->dev);
						
						RESTORE_INT(flags);
					}
				}
			}
#endif
		}

#if defined(__DRAYTEK_OS__)
		if (rc != 0)
			return rc;
#endif
#if !defined(__KERNEL__) && !defined(__OSK__)
		wlan_index++;
#endif
	}

#ifdef CONFIG_POWER_SAVE
	rtw_suspend_lock_init();
#endif
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	register_reboot_notifier(&rtw_reboot_notifier);
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	rtl_wlan_customPassthru_init();
#endif

#if 0
//#ifdef PCIE_POWER_SAVING	
	HostPCIe_Close();	
#endif


#if 0//def CONFIG_RTL_STP
	rtl_pseudo_dev_init(wlan_device[0].priv);
#endif

#ifdef _USE_DRAM_
	{
	extern unsigned char *en_cipherstream;
	extern unsigned char *tx_cipherstream;
	extern char *rc4sbox, *rc4kbox;
	extern unsigned char *pTkip_Sbox_Lower, *pTkip_Sbox_Upper;
	extern unsigned char Tkip_Sbox_Lower[256], Tkip_Sbox_Upper[256];

#ifdef CONFIG_RTL8671
	extern void r3k_enable_DRAM(void);    //6/7/04' hrchen, for 8671 DRAM init
	r3k_enable_DRAM();    //6/7/04' hrchen, for 8671 DRAM init
#endif

	en_cipherstream = (unsigned char *)(DRAM_START_ADDR);
	tx_cipherstream = en_cipherstream;

	rc4sbox = (char *)(DRAM_START_ADDR + 2048);
	rc4kbox = (char *)(DRAM_START_ADDR + 2048 + 256);
	pTkip_Sbox_Lower = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*2);
	pTkip_Sbox_Upper = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*3);

	memcpy(pTkip_Sbox_Lower, Tkip_Sbox_Lower, 256);
	memcpy(pTkip_Sbox_Upper, Tkip_Sbox_Upper, 256);
	}
#endif

#ifdef __KERNEL__
#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	rtl8192cd_chr_reg_hook = rtl8192cd_chr_reg;
	rtl8192cd_chr_unreg_hook = rtl8192cd_chr_unreg;
//------------------------------------
	rtl8192cd_chr_init();
#endif
#endif

#ifdef CONFIG_RTL8671
	//turn off AP LED
	{
		unsigned char wlanreg = *(volatile unsigned char *)0xbd30005e;
		*(volatile unsigned char *)0xbd30005e = (wlanreg | ((1<<5)));
	}
#endif

#ifdef PERF_DUMP
	rtl8651_romeperfInit();
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	register_usb_hook = (register_usb_pkt_cnt_fn)(register_usb_pkt_cnt_f);
#endif

#ifdef RTK_WLAN_EVENT_INDICATE
	if (!get_nl_eventd_sk())
		rtk_eventd_netlink_init();
#endif

#ifdef RTK_SMART_ROAMING
	rtl_netlink_init();
#endif

	return 0;
}
#endif


#ifdef __DRAYTEK_OS__
int rtl8192cd_init(unsigned long base_addr)
{
	return __rtl8192cd_init(base_addr);
}
#elif !defined(__ECOS)
int MDL_INIT rtl8192cd_init(void)
{
#ifdef USE_DMA_ALLOCATE
	int ret;
	printk("****** %s %d\n", __FUNCTION__, __LINE__);
	ret = misc_register(&cma_dev_misc);
	if (unlikely(ret)) {
		printk("****** %s %d, failed(%d) to register cma_dev_fops misc device!\n", __FUNCTION__, __LINE__, ret);
		cma_dev = NULL;
	}
	else {
		cma_dev = cma_dev_misc.this_device;
		cma_dev->coherent_dma_mask = ~0;
		printk("****** %s %d, cma_dev %pa\n", __FUNCTION__, __LINE__, &cma_dev);
	}
#endif

#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	static char initated = 0;
	if (initated == 0)
		initated = 1;
	else
		return 0;
#endif

#ifndef CONFIG_DUAL_CPU_SLAVE
#ifdef CONFIG_RTL8671
	gpioConfig(10,2);
	gpioClear(10);
	delay_ms(10);
	gpioSet(10);
#endif
#endif

	return __rtl8192cd_init(0);
}
#endif


#ifndef __ECOS
#if defined(__KERNEL__) || defined(__OSK__)
#if !defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
static
#endif
void MDL_EXIT rtl8192cd_exit (void)
{
#if defined(CONFIG_PCI_HCI) && !defined(CONFIG_NET_PCI)
	int idx;
#endif
#ifdef USE_DMA_ALLOCATE
	misc_deregister(&cma_dev_misc);
#endif

#if defined(CONFIG_WIRELESS_LAN_MODULE) && !defined(NOT_RTK_BSP)
	wirelessnet_hook = NULL;
#ifdef BR_SHORTCUT
	wirelessnet_hook_shortcut = NULL;
#endif
#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = NULL;
	Fn_rtl8651_romeperfExitPoint = NULL;
 #endif
#ifdef CONFIG_RTL8190_PRIV_SKB
	wirelessnet_hook_is_priv_buf = NULL;
	wirelessnet_hook_free_priv_buf = NULL;
#endif
#endif // CONFIG_WIRELESS_LAN_MODULE && !NOT_RTK_BSP

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	unregister_reboot_notifier(&rtw_reboot_notifier);
#endif

	drv_registered = FALSE;

#ifdef CONFIG_PCI_HCI
#ifndef CONFIG_NET_PCI
	for (idx = 0; idx < ARRAY_SIZE(wlan_device) ; idx++) {
		if (NULL == wlan_device[idx].priv)
			continue;
		rtl8192cd_deinit_one(wlan_device[idx].priv);
#ifdef RTK_NL80211
		rtk_remove_dev(wlan_device[idx].priv->rtk, idx);
#endif
		wlan_device[idx].priv = NULL;
	}
#else
	pci_unregister_driver(&rtl8192cd_driver);
#endif
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_USB_HCI
	usb_deregister(&rtl8192cd_usb_driver);
#endif

#ifdef CONFIG_SDIO_HCI
	sdio_unregister_driver(&rtl8192cd_sdio_driver);
#endif

#ifdef CONFIG_WLAN_HAL
	ResetHALIndex();
#endif

#ifdef CONFIG_POWER_SAVE
	rtw_suspend_lock_deinit();
#endif

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	rtl8192cd_chr_reg_hook = NULL;
	rtl8192cd_chr_unreg_hook = NULL;
//------------------------------------

	rtl8192cd_chr_exit();
#endif

#ifdef RTK_SMART_ROAMING
	rtl_netlink_exit();
#endif
}
#else // not __KERNEL__
void MDL_EXIT rtl8192cd_exit(void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8192cd_priv *priv = dev->priv;
	int idx, i;

	for (idx=0; idx<sizeof(wlan_device)/sizeof(struct _device_info_); idx++)
		if (wlan_device[idx].priv == priv)
			break;

	if (idx == sizeof(wlan_device)/sizeof(struct _device_info_))
		return;		// wrong argument!!

#ifdef WDS
	{
		int num;

		num = (wlan_device[idx].type >> WDS_SHIFT) & WDS_MASK;
		for (i=0; i<num; i++) {
			wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]->priv = NULL;
			unregister_netdev(wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]);
			kfree(wlan_device[idx].priv->pmib->dot11WdsInfo.dev[i]);
		}
	}
#endif

	unregister_netdev(dev);

	kfree(priv->pmib);

#ifdef	CONFIG_RTK_MESH

	if(priv->proxy_table)
	{
		remove_hash_table(priv->proxy_table);
		kfree(priv->proxy_table);
	}
	if(priv->mesh_rreq_retry_queue)
	{
		remove_hash_table(priv->mesh_rreq_retry_queue);
		kfree(priv->mesh_rreq_retry_queue);
	}

	// add by chuangch 2007.09.13
	if(priv->pathsel_table)
	{
		remove_hash_table(priv->pathsel_table);
		kfree(priv->pathsel_table);
	}

	if(priv->pann_mpp_tb)
		kfree(priv->pann_mpp_tb);

	kfree(priv->pathsel_queue);
#ifdef	_MESH_ACL_ENABLE_
	kfree(priv->pmesh_acl_poll);
#endif
#endif	// CONFIG_RTK_MESH

	kfree(priv->pevent_queue);
#ifdef CONFIG_RTL_WAPI_SUPPORT
//	kfree(vxd_priv->wapiEvent_queue);
	kfree(priv->wapiEvent_queue);
	#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv)&&priv->pmib->miscEntry.vap_enable)	{
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
			kfree(priv->pvap_priv[i]->wapiEvent_queue);
	}
	#endif
#endif
#ifdef CONFIG_IEEE80211R
	kfree(priv->pftevent_queue);
#endif

#ifndef PRIV_STA_BUF
	kfree((void *)priv->pshare->phw->alloc_dma_buf);

#ifndef USE_DMA_ALLOCATE
	kfree(priv->pshare->phw);
#else
	if (priv->pshare->phw) {
		dma_free_coherent(cma_dev, sizeof(struct rtl8192cd_hw), priv->pshare->phw, priv->pshare->hw_dma_phys);
		priv->pshare->phw = NULL;
		priv->pshare->hw_dma_phys = NULL;
	}
#endif
	kfree(priv->pshare->pwlan_hdr_poll);
	kfree(priv->pshare->pwlanllc_hdr_poll);
	kfree(priv->pshare->pwlanbuf_poll);
	kfree(priv->pshare->pwlanicv_poll);
	kfree(priv->pshare->pwlanmic_poll);
#endif
	kfree(priv->pwlan_acl_poll);
	kfree(priv->Eap_packet);
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	kfree(priv->wpa_global_info);
#endif
	free_site_survey_res(priv->site_survey);
#ifndef PRIV_STA_BUF
	kfree(priv->pshare);	// david
#endif
	kfree(dev);

#ifdef EN_EFUSE
	for( i = 0 ; i < 2 ; i++ )
		kfree(priv->EfuseMap[i]);
	kfree(priv->EfuseCmd);
#endif

#ifdef CONFIG_WLAN_HAL
    if (IS_HAL_CHIP(priv)) {
        HalDisAssociateNic(priv, TRUE);
        DecreaseHALIndex();
    }
#endif //CONFIG_WLAN_HAL

	wlan_device[idx].priv = NULL;

	wlan_index--;
}
#endif
#endif // !__EOCS


#ifdef __KERNEL__
#ifdef USE_CHAR_DEV
struct rtl8192cd_priv *rtl8192cd_chr_reg(unsigned int minor, struct rtl8192cd_chr_priv *priv)
{
	if (wlan_device[minor].priv)
		wlan_device[minor].priv->pshare->chr_priv = priv;
	return wlan_device[minor].priv;
}


void rtl8192cd_chr_unreg(unsigned int minor)
{
	if (wlan_device[minor].priv)
		wlan_device[minor].priv->pshare->chr_priv = NULL;
}
#endif


#ifdef RTL_WPA2_PREAUTH
void wpa2_kill_fasync(void)
{
	int wlan_index = 0;
	struct _device_info_ *wdev = &wlan_device[wlan_index];
	struct rtl8192cd_priv *priv = wdev->priv;
	event_indicate(priv, NULL, -1);
}


void wpa2_preauth_packet(struct sk_buff	*pskb)
{
	// ****** NOTICE **********
	int wlan_index = 0;
	struct _device_info_ *wdev = &wlan_device[wlan_index];
	// ****** NOTICE **********

	struct rtl8192cd_priv *priv = wdev->priv;

	unsigned char		szEAPOL[] = {0x02, 0x01, 0x00, 0x00};
	DOT11_EAPOL_START	Eapol_Start;

	if (priv == NULL) {
		PRINT_INFO("%s: priv == NULL\n", (char *)__FUNCTION__);
		return;
	}

#ifndef WITHOUT_ENQUEUE
	if (!memcmp(pskb->data, szEAPOL, sizeof(szEAPOL)))
	{
		Eapol_Start.EventId = DOT11_EVENT_EAPOLSTART_PREAUTH;
		Eapol_Start.IsMoreEvent = FALSE;
		memcpy(&Eapol_Start.MACAddr, SKB_MAC_HEADER(pskb) + MACADDRLEN, WLAN_ETHHDR_LEN);
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)&Eapol_Start, sizeof(DOT11_EAPOL_START));
	}
	else
	{
		unsigned short		pkt_len;

		pkt_len = WLAN_ETHHDR_LEN + pskb->len;
		priv->Eap_packet->EventId = DOT11_EVENT_EAP_PACKET_PREAUTH;
		priv->Eap_packet->IsMoreEvent = FALSE;
		memcpy(&(priv->Eap_packet->packet_len), &pkt_len, sizeof(unsigned short));
		memcpy(&(priv->Eap_packet->packet[0]), SKB_MAC_HEADER(pskb), WLAN_ETHHDR_LEN);
		memcpy(&(priv->Eap_packet->packet[WLAN_ETHHDR_LEN]), pskb->data, pskb->len);
		DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)priv->Eap_packet, sizeof(DOT11_EAP_PACKET));
	}
#endif // WITHOUT_ENQUEUE

	event_indicate(priv, NULL, -1);

	// let dsr to free this skb
}
#endif // RTL_WPA2_PREAUTH
#endif // __KERNEL__

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
__MIPS16 __IRAM_FWD  extern int is_rtl865x_eth_priv_buf(unsigned char *head);
extern void dump_sta_dz_queue_num(struct rtl8192cd_priv *priv, struct stat_info *pstat);

int dump_wlan_dz_queue_num(const char *name)
{
	int i,j,txCnt=0;
	//int queueCnt,idx;
	struct rtl8192cd_priv *priv;
	//struct tx_desc_info *pdescinfoH,*pdescinfo;
	//struct tx_desc	*pdescH, *pdesc;
	//struct sk_buff *skb = NULL;
	//struct rtl8192cd_hw	*phw;
	int 			hd, tl;
					
	for (j=0; (j<sizeof(wlan_device)/sizeof(struct _device_info_)); j++)
	{
		//if(counted)
			//break;
		
		if (wlan_device[j].priv && netif_running(wlan_device[j].priv->dev) && strcmp(wlan_device[j].priv->dev->name,name)==0)
		{
			priv = wlan_device[j].priv;
			if (OPMODE & WIFI_AP_STATE) 
			{
				hd = priv->dz_queue.head;
				tl = priv->dz_queue.tail;
				printk("priv->dz_queue:%d\n",CIRC_CNT(hd, tl, NUM_TXPKT_QUEUE));

				for (i=0; i<NUM_STAT; i++)
				{
					if (priv->pshare->aidarray[i]) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
						if (priv != priv->pshare->aidarray[i]->priv)
							continue;
						else
#endif
						{
							if (priv->pshare->aidarray[i]->used == TRUE)
							{
								dump_sta_dz_queue_num(priv, &(priv->pshare->aidarray[i]->station));
							}

						}
					}
				}
			}
		}
	}

	return txCnt;
}
int get_nic_buf_in_wireless_tx(const char *name)
{
	int i,txCnt;
	int queueCnt,idx;
	struct rtl8192cd_priv *priv;
	struct tx_desc_info *pdescinfoH,*pdescinfo;
	struct tx_desc	*pdescH, *pdesc;
	struct sk_buff *skb = NULL;
	struct rtl8192cd_hw	*phw;

	txCnt = 0;
	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)); i++)
	{
		//if(counted)
			//break;
		
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev) && strcmp(wlan_device[i].priv->dev->name,name)==0)
		{
			priv = wlan_device[i].priv;
			if (OPMODE & WIFI_AP_STATE) 
			{
				for(queueCnt = 0; queueCnt <= HIGH_QUEUE;queueCnt++)
				{
					phw=GET_HW(priv);
					pdescH		= get_txdesc(phw, queueCnt);					
					pdescinfoH = get_txdesc_info(priv->pshare->pdesc_info,queueCnt);
					for(idx = 0; idx < CURRENT_NUM_TX_DESC; idx++)
					{
						pdesc = pdescH + idx;
						pdescinfo = pdescinfoH + idx;
						//if (!pdesc || (get_desc(pdesc->Dword0) & TX_OWN))
							//continue;
						if(pdescinfo->type == _SKB_FRAME_TYPE_ || pdescinfo->type == _RESERVED_FRAME_TYPE_)
							skb = (struct sk_buff *)(pdescinfo->pframe);
						else
							continue;
						
						if(skb && is_rtl865x_eth_priv_buf(skb->head))
							txCnt++;
					}
				}

				//counted = 1;
			}
		}
	}

	return txCnt;
}
#endif


#if defined(CONFIG_RTK_VLAN_SUPPORT) && defined(CONFIG_RTK_VLAN_FOR_CABLE_MODEM)
struct net_device* get_dev_by_vid(int vid)
{
	int i;
	struct rtl8192cd_priv *priv;

	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)); i++)
	{
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev))
		{
			priv = wlan_device[i].priv;
			if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable) {
					int j;
					for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
						if (IS_DRV_OPEN(priv->pvap_priv[j])) {
							if(priv->pvap_priv[j]->pmib->vlan.vlan_enable && priv->pvap_priv[j]->pmib->vlan.vlan_id == vid)
								return priv->pvap_priv[j]->dev;
						}
					}
				}
		}
	}

	return NULL;
}
#endif

#ifdef BR_SHORTCUT
#ifdef RTL_CACHED_BR_STA
struct brsc_cache_t* get_free_brsc_cache(void){
	int idx;
	int idxofoldest=0;

	/*find free cache*/ 
	for(idx=0;idx<MAX_BRSC_NUM;idx++) {
		if(brsc_cache_arr[idx].occupy==0)
			return &brsc_cache_arr[idx];    // found free cache
	}

	/*not found free cache , return oldest aging*/ 
	for(idx=0;idx<MAX_BRSC_NUM;idx++){
		if(TSF_LESS(brsc_cache_arr[idx].timestamp,brsc_cache_arr[idxofoldest].timestamp)) { // time stamp more less more old
			idxofoldest=idx;
		}
	}
	return &brsc_cache_arr[idxofoldest];    // found free cache

};

int query_brsc_cache(unsigned char* da)
{
	int idx;
	/*find free cache*/ 
	for(idx=0;idx<MAX_BRSC_NUM;idx++){
		if(brsc_cache_arr[idx].occupy && isEqualMACAddr(brsc_cache_arr[idx].cached_br_sta_mac , da))
			return idx;    // found matched cache
	}
	return -1;
};

void add_to_brsc_cache(struct net_device *dev, unsigned char* da)
{
	struct brsc_cache_t* tmpcache=NULL;
	tmpcache = get_free_brsc_cache();
	if(tmpcache) {
		memcpy(tmpcache->cached_br_sta_mac, da, MACADDRLEN);
		tmpcache->cached_br_sta_dev = dev;
		tmpcache->occupy = 1;
		tmpcache->timestamp = jiffies;
	}
};

void release_brsc_cache(unsigned char* da)
{
	int idx = query_brsc_cache(da);
	if(idx>=0) {
		memset(&brsc_cache_arr[idx],0,sizeof(struct brsc_cache_t));
	}
};
#endif // RTL_CACHED_BR_STA

//#ifndef NOT_RTK_BSP
__MIPS16
__IRAM_IN_865X
struct net_device *get_shortcut_dev(unsigned char *da)
{
    int i;
#ifdef MBSSID
    int j;
#endif
    struct rtl8192cd_priv *priv;
#ifdef UNIVERSAL_REPEATER
    struct rtl8192cd_priv *vxd_priv;
#endif
    struct stat_info *pstat;
    struct net_device *dev;
    
    #if !defined( _SINUX_ ) && (defined(__KERNEL__) || defined(__OSK__))
    struct net_bridge_port *br_port=NULL;
    #endif
    
    int br_forwarding = 1;

    #ifdef CONFIG_RTK_MESH	//11 mesh no support shortcut now
    {
        extern unsigned char cached_mesh_mac[2][MACADDRLEN];
        extern struct net_device *cached_mesh_dev[2];

        if (cached_mesh_dev[0] && !memcmp(da, cached_mesh_mac[0], MACADDRLEN)) {
            return cached_mesh_dev[0];
        }
    }
    #endif

    #ifdef WDS
    {
        extern unsigned char cached_wds_mac[MACADDRLEN];
        extern struct net_device *cached_wds_dev;
        if (cached_wds_dev && !memcmp(da, cached_wds_mac, MACADDRLEN))
            return cached_wds_dev;
    }
    #endif

    #ifdef RTL_CACHED_BR_STA
    for(i=0;i<MAX_BRSC_NUM;i++){
        if(brsc_cache_arr[i].occupy && isEqualMACAddr(brsc_cache_arr[i].cached_br_sta_mac , da))
            return brsc_cache_arr[i].cached_br_sta_dev;    // found matched cache
    }
    #endif

    dev = NULL;
    for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)) && (dev==NULL); i++) {
        if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev)) {
            priv = wlan_device[i].priv;

#ifdef CLIENT_MODE
            #ifdef CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT
            if (priv->reperater_idx == 1) {
                if (cached_sta_dev[0] && isEqualMACAddr(da, cached_sta_mac[0])) {
                    return cached_sta_dev[0];
                }
            } else if ((priv->reperater_idx == 2)) {
                if (cached_sta_dev[1] && isEqualMACAddr(da, cached_sta_mac[1])) {
                    return cached_sta_dev[1];
                }
            }
            #else
            if (cached_sta_dev[0] && isEqualMACAddr(da, cached_sta_mac[0])) {
                return cached_sta_dev[0];
            }
            #endif
#endif

            #if !defined( _SINUX_ ) && (defined(__KERNEL__) || defined(__OSK__))
            br_port = GET_BR_PORT(priv->dev);
            if(br_port) {
                if(br_port->br->stp_enabled && br_port->state != BR_STATE_FORWARDING) {
                    br_forwarding = 0;
                }
            }
            #endif

            if (IS_DRV_OPEN(priv) && br_forwarding)
            {
                if (!priv->pmib->dot11OperationEntry.disable_brsc) {
                    pstat = get_stainfo(priv, da);

                    #ifdef A4_STA
                    if( pstat == NULL && priv->pshare->rf_ft_var.a4_enable) {
                        pstat = a4_sta_lookup(priv, da);    
                    }
                    #endif
                    if (pstat && (pstat->tx_avarage > 1) && pstat->expire_to) {	/* Make sure it must have some packets go theough bridge module before shortcut */
                        #ifdef WDS
                        if (!(pstat->state & WIFI_WDS))	// if WDS peer
                        #endif
                        {
                            #if defined(CONFIG_RTK_MESH) && defined(CONFIG_RTL_MESH_SINGLE_IFACE)
                            if( isMeshPoint(pstat)) {                                
                                if(priv->mesh_priv_first->pmib->dot1180211sInfo.mesh_portal_enable)
                                    dev = priv->mesh_dev;
                            }
                            else
                            #endif
                                {dev = priv->dev;}


                            //STADEBUG("  1. got![%02X%02X%02X:%02X%02X%02X]\n",da[0],da[1],da[2],da[3],da[4],da[5]);
                            break;
                        }
                    }
                }
                #ifdef MBSSID
                if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable) {
                    for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
                        if ((priv->pvap_priv[j]->assoc_num > 0) && IS_DRV_OPEN(priv->pvap_priv[j]) &&
                            !(priv->pvap_priv[j]->pmib->dot11OperationEntry.disable_brsc)) {
                            pstat = get_stainfo(priv->pvap_priv[j], da);
                            #ifdef A4_STA
                            if( pstat == NULL && priv->pvap_priv[j]->pshare->rf_ft_var.a4_enable) {
                                pstat = a4_sta_lookup(priv->pvap_priv[j], da);    
                            }
                            #endif                            
                            if (pstat && (pstat->tx_avarage > 1) && pstat->expire_to) {
                                //STADEBUG(" 2.vap%d ,got![%02X%02X%02X:%02X%02X%02X]\n",j,da[0],da[1],da[2],da[3],da[4],da[5]);                                      
                                dev = priv->pvap_priv[j]->dev;
                                break;
                            }
                        }
                    }
                }
                #endif
                #ifdef UNIVERSAL_REPEATER
                vxd_priv = GET_VXD_PRIV(priv);
                if((OPMODE & WIFI_STATION_STATE) && (vxd_priv->assoc_num > 0) && IS_DRV_OPEN(vxd_priv) &&
                    !vxd_priv->pmib->dot11OperationEntry.disable_brsc) {		
                    pstat = get_stainfo(vxd_priv, da);
                    #ifdef A4_STA
                    if( pstat == NULL && vxd_priv->pshare->rf_ft_var.a4_enable) {
                        pstat = a4_sta_lookup(vxd_priv, da);    
                    }
                    #endif                      
                    if (pstat && (pstat->tx_avarage > 1) && pstat->expire_to) {
                        dev = vxd_priv->dev;
                        break;
                    }	
                }
                #endif
            }
        }
    }

#ifdef RTL_CACHED_BR_STA
    if (dev!=NULL) {
        add_to_brsc_cache(dev,da);		
    }
#endif

    return dev;
}
//#endif // !NOT_RTK_BSP

void clear_shortcut_cache(void)
{
#ifdef CONFIG_RTK_MESH
	{
		extern struct net_device *cached_mesh_dev[2];
		extern unsigned char cached_mesh_mac[2][MACADDRLEN];
		cached_mesh_dev[0]= NULL;
		memset(cached_mesh_mac[0],0,MACADDRLEN);
        cached_mesh_dev[1]= NULL;
		memset(cached_mesh_mac[1],0,MACADDRLEN);
	}
#endif

#ifdef WDS
	{
		extern struct net_device *cached_wds_dev;
		extern unsigned char cached_wds_mac[MACADDRLEN];
		cached_wds_dev= NULL;
		memset(cached_wds_mac,0,MACADDRLEN);
	}
#endif

#ifdef CLIENT_MODE
	{
		cached_sta_dev[0] = NULL;
		cached_sta_dev[1] = NULL;        
		memset(cached_sta_mac[0],0,MACADDRLEN);
		memset(cached_sta_mac[1],0,MACADDRLEN);        
	}
#endif

#ifdef RTL_CACHED_BR_STA
     memset(brsc_cache_arr,0,sizeof(struct brsc_cache_t)*MAX_BRSC_NUM);	
#endif

#if	defined(CONFIG_RTL_819X_SWCORE)	
#if defined(CONFIG_RTL_819X) && (defined(__LINUX_2_6__) || defined(__ECOS))
	{
		extern 	 unsigned char cached_eth_addr[MACADDRLEN];
		extern struct net_device *cached_dev;
		cached_dev = NULL;
		memset(cached_eth_addr,0,MACADDRLEN);
	}
#endif

#if !defined(NOT_RTK_BSP) && !defined(__ECOS)
#if defined(BR_SHORTCUT_C2)
	{
		extern 	 unsigned char cached_eth_addr2[MACADDRLEN];
		extern struct net_device *cached_dev2;
		cached_dev2 = NULL;
		memset(cached_eth_addr2,0,MACADDRLEN);
	}
#endif
#ifdef BR_SHORTCUT_C3
	{
		extern 	 unsigned char cached_eth_addr3[MACADDRLEN];
		extern struct net_device *cached_dev3;
		cached_dev3 = NULL;
		memset(cached_eth_addr3,0,MACADDRLEN);
	}
#endif
#ifdef BR_SHORTCUT_C4
	{
		extern 	 unsigned char cached_eth_addr4[MACADDRLEN];
		extern struct net_device *cached_dev4;
		cached_dev4 = NULL;
		memset(cached_eth_addr4,0,MACADDRLEN);
	}
#endif	
#endif // !NOT_RTK_BSP
#endif // end of defined(CONFIG_RTL_819X_SWCORE)	

#ifdef CONFIG_RTL8672
#ifndef CONFIG_ARCH_LUNA_SLAVE
	{
		extern void clear_cached_eth_mac_addr(void);
		clear_cached_eth_mac_addr();
	}
#endif
#endif
}
#endif // BR_SHORTCUT


void update_fwtbl_asoclst(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char tmpbuf[16];
	int i;

#if defined(__KERNEL__) && !defined(NOT_RTK_BSP) 
	struct sk_buff *skb = NULL;
	struct wlan_ethhdr_t *e_hdr;
	unsigned char xid_cmd[] = {0, 0, 0xaf, 0x81, 1, 2};

	// update forwarding table of bridge module
	if (GET_BR_PORT(priv->dev)) {
		skb = dev_alloc_skb(64);
		if (skb != NULL) {
			skb->dev = priv->dev;
			skb_put(skb, 60);
			e_hdr = (struct wlan_ethhdr_t *)skb->data;
			memset(e_hdr, 0, 64);
			memcpy(e_hdr->daddr, priv->dev->dev_addr, MACADDRLEN);
			memcpy(e_hdr->saddr, pstat->hwaddr, MACADDRLEN);
			e_hdr->type = 8;
			memcpy(&skb->data[14], xid_cmd, sizeof(xid_cmd));
			skb->protocol = eth_type_trans(skb, priv->dev);
			#if defined(__LINUX_2_6__) && defined(RX_TASKLET)&& !defined(CONFIG_RTL8672) && !(defined(__LINUX_3_10__) || defined(__LINUX_3_2__))
				netif_receive_skb(skb);
			#else
				netif_rx(skb);
			#endif
		}
	}
#endif

	// update association lists of the other WLAN interfaces
	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++) {
		if (wlan_device[i].priv && (wlan_device[i].priv != priv)) {
			if (wlan_device[i].priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
				sprintf((char *)tmpbuf, "%02x%02x%02x%02x%02x%02x",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
				del_sta(wlan_device[i].priv, tmpbuf);
			}
		}
	}

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (GET_ROOT(priv)->pvap_priv[i] && IS_DRV_OPEN(GET_ROOT(priv)->pvap_priv[i]) && (GET_ROOT(priv)->pvap_priv[i] != priv) &&
				(priv->vap_init_seq >= 0)) {
				if (GET_ROOT(priv)->pvap_priv[i]->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
					sprintf((char *)tmpbuf, "%02x%02x%02x%02x%02x%02x",
						pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
					del_sta(GET_ROOT(priv)->pvap_priv[i], tmpbuf);
				}
			}
		}
	}
#endif

#ifdef __ECOS
#ifdef CONFIG_RTL_819X_SWCORE
	{
	/* 02-17-2012: move the called function "update_hw_l2table" from Bridge module to here to avoid hacking the Linux kernel or the other kernel */	
	extern void update_hw_l2table(const char *srcName,const unsigned char *addr);
	update_hw_l2table("wlan", (const unsigned char *)pstat->hwaddr); /* RTL_WLAN_NAME */
	}
#endif
#else
#if defined(CONFIG_RTL_819X) && defined(CONFIG_RTL_819X_SWCORE) && !defined(CONFIG_RTL8196C_KLD) && !defined(__OSK__) && !(defined(CONFIG_RTL8672) && defined(CONFIG_OPENWRT_SDK))
#ifndef CONFIG_RTL_8198B
	{
	/* 02-17-2012: move the called function "update_hw_l2table" from Bridge module to here to avoid hacking the Linux kernel or the other kernel */	
	extern void update_hw_l2table(const char *srcName,const unsigned char *addr);
	update_hw_l2table("wlan", (const unsigned char *)pstat->hwaddr); /* RTL_WLAN_NAME */
	}
#endif
#endif
#endif
}
#if defined(CONFIG_RTL_REPORT_LINK_STATUS) && defined(__ECOS)	
	int get_wlan_if_type(struct rtl8192cd_priv *priv)
	{
		int i,find = 0,if_idx = -1;;
		struct rtl8192cd_priv *root_priv;
		root_priv = GET_ROOT(priv);
		if(root_priv == NULL)
			return if_idx;
		for (i=0; i<(sizeof(wlan_device)/sizeof(struct _device_info_)); i++)
		{
			if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev) 
				&& wlan_device[i].priv == root_priv)
			{
				find = 1;
				break;
			}
		}
		if(!find)
			return if_idx;	
		if(i == 0){
#ifdef UNIVERSAL_REPEATER
			if(IS_VXD_INTERFACE(priv))
				if_idx = WLAN0_VXD_PORT;
			else
#endif
				if_idx = WLAN0_PORT;
		}
		else if(i == 1){
#ifdef UNIVERSAL_REPEATER
			if(IS_VXD_INTERFACE(priv))
				if_idx = WLAN1_VXD_PORT;
			else
#endif
				if_idx = WLAN1_PORT;
		}
		return if_idx;
	}
	
#endif


// quick fix for warn reboot fail issue
#ifndef CONFIG_RTL_8198B
#define CLK_MANAGE     0xb8000010
#endif
void force_stop_wlan_hw(void)
{
	int i=0;
#ifdef CONFIG_WLAN_HAL	
    unsigned int errorFlag;
#endif	
//	int temp;

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D) || defined(CONFIG_RTL_92D_DMDP)
	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++) 
#endif
	{
		if (wlan_device[i].priv) {
			struct rtl8192cd_priv *priv = wlan_device[i].priv;
#ifndef NOT_RTK_BSP
#ifdef PCIE_POWER_SAVING
			if ((REG32(CLK_MANAGE) & BIT(11)) == 0)
			{
				extern void setBaseAddressRegister(void);
				REG32(CLK_MANAGE) |= BIT(11);
				delay_ms(10);
				PCIE_reset_procedure(0, 0, 1, wlan_device[i].base_addr);
				setBaseAddressRegister();
			}
#endif
#endif
#ifdef  CONFIG_WLAN_HAL
#ifndef NOT_RTK_BSP
            if (GET_CHIP_VER(priv)==VERSION_8197F) {
                if(REG32(0xB8000064)&BIT0)
                {
                    if(RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                        printk("StopHW Succeed\n");
                    }
                }else
                {
                    return;
                }                
            } else
#endif
			if (IS_HAL_CHIP(priv)) {
	            BOOLEAN     bVal;

	            GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, (pu1Byte)&bVal);
	            if ( bVal ) {
                    if (RT_STATUS_SUCCESS == GET_HAL_INTERFACE(priv)->StopHWHandler(priv)) {
                        printk("StopHW Succeed\n");
                    }
                    else {  
                        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);
                        errorFlag |= DRV_ER_CLOSE_STOP_HW;
                        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_DRV_DBG, (pu1Byte)&errorFlag);                                         
                        panic_printk("StopHW Failed\n");
                    }                    
	            }
	            else {
//	                printk("(%d), Can't write MACID register\n", __LINE__);
	            }
#ifdef __OSK__
#if defined(CONFIG_PCI_HCI) && defined(CONFIG_WLAN_HAL_8881A)
	            if(GET_CHIP_VER(priv)==VERSION_8881A) {
	                //Disable MAC_System(BIT(0)), MAC_Lextra_Bus(BIT(1))                
	                REG32(0xB80000DC)= 0x00;
	            }
#endif //#ifdef CONFIG_WLAN_HAL_8881A
#endif
			} else
#endif
			{
				if ( check_MAC_IO_Enable(priv) ) {
					rtl8192cd_stop_hw(priv);
				}
			}
		}
	}
}

#ifdef TPT_THREAD
int kTPT_thread(void *p)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)p;
	//printk("kTPT_thread start!\n");
	
	do {                    
		msleep_interruptible(1000);
		if (!IS_DRV_OPEN(priv))
			continue;

#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
#ifdef SMART_REPEATER_MODE
		if (!priv->pshare->switch_chan_rp)
#endif
		if(rtl_atomic_read(&priv->pshare->do_tpt)) {
			ODM_TXPowerTrackingCheck(ODMPTR);
			rtl_atomic_set(&priv->pshare->do_tpt, 0);
		}

	} while (!kthread_should_stop() );

	//printk("kTPT_thread exiting\n");
	return 0;
}

int kTPT_task_init(struct rtl8192cd_priv *priv)
{
	unsigned char thread_name[10] = {0};

	sprintf( thread_name, "kTPTd" );

	rtl_atomic_set(&priv->pshare->do_tpt, 0);
	if (NULL == priv->pshare->tpt_task) {
		priv->pshare->tpt_task = kthread_run(kTPT_thread, (void *)priv, thread_name);
		if (IS_ERR(priv->pshare->tpt_task)) {
			printk("%s Thread create failed!\n", thread_name);
			priv->pshare->tpt_task = NULL;
			return -1;
		}
	}

	//printk("%s Thread create successfully!\n", thread_name );
	return 0;
}

void kTPT_task_stop(struct rtl8192cd_priv *priv)
{	
	if (priv->pshare->tpt_task)
	{
		kthread_stop(priv->pshare->tpt_task);
		priv->pshare->tpt_task = NULL;
	}
	rtl_atomic_set(&priv->pshare->do_tpt, 0);
}
#endif /* TPT_THREAD */

#ifdef _BROADLIGHT_FASTPATH_
void replace_upper_layer_packet_destination( void * xi_destination_ptr )
{
	printk(KERN_INFO"start fastpath\n");
	send_packet_to_upper_layer = xi_destination_ptr ;
}
EXPORT_SYMBOL(replace_upper_layer_packet_destination) ;
#endif

#ifdef __ECOS
struct rtl8192cd_priv *wlan_search_priv(char *name)
{
	int i;
#ifdef MBSSID
	int j;
#endif

	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++) {
		if (!strcmp(wlan_device[i].priv->dev->name, name))
			return wlan_device[i].priv;
#ifdef MBSSID
		for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
			if (wlan_device[i].priv->pvap_priv[j] && 
					!strcmp(wlan_device[i].priv->pvap_priv[j]->dev->name, name))
				return wlan_device[i].priv->pvap_priv[j];
		}
#endif

#ifdef UNIVERSAL_REPEATER
		if (wlan_device[i].priv->pvxd_priv && 
			!strcmp(wlan_device[i].priv->pvxd_priv->dev->name, name))
			return wlan_device[i].priv->pvxd_priv;
#endif
	}
	return NULL;
}

void wlan_dev_kfree_skb_any(struct sk_buff *skb)
{
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN1
	if (skb->key && skb->dev) {
		if (((Rltk819x_t *)(skb->dev->info))->device_num & 0xffff0000)
			kfree_skb_chk_key(skb, wlan_device[1].priv->dev);
		else
			kfree_skb_chk_key(skb, wlan_device[0].priv->dev);
	}
	else {
		kfree_skb_chk_key(skb, wlan_device[0].priv->dev);
	}
#else
	kfree_skb_chk_key(skb, wlan_device[0].priv->dev);
#endif
}
#endif

#if defined (CONFIG_RTL_EXT_PORT_SUPPORT)
struct net_device * rtl_get_wlan_dev_by_mac(unsigned char *da)
{
	struct net_device *dev=NULL;
	struct rtl8192cd_priv *priv=NULL;
	struct rtl8192cd_priv *vxd_priv=NULL;
	struct stat_info *pstat;
	int i=0;
#ifdef MBSSID
	int j;
#endif

	//ret=rtl_check_assoc_sta(priv,1,da);
	for (i=0; (i<sizeof(wlan_device)/sizeof(struct _device_info_)) && (dev==NULL); i++) {
		if (wlan_device[i].priv && netif_running(wlan_device[i].priv->dev)) {
			priv = wlan_device[i].priv;

            
			struct net_bridge_port *br_port=NULL;
#if !defined( _SINUX_ ) 
			// if sinux, no linux bridge, so should don't depend on br_port if use br_shortcut (John Qian 2010/6/24) 
#ifdef __KERNEL__
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,35))
			br_port = priv->dev->br_port;
#else
			br_port = br_port_get_rcu(priv->dev);
#endif
#endif
#endif
			if (IS_DRV_OPEN(priv)
				//2010-5-10
#ifdef __KERNEL__
				#if !defined( _SINUX_ )
				// if sinux, no linux bridge, so should don't depend on br_port if use br_shortcut (John Qian 2010/6/24) 
				&& (br_port)
				&& ((!br_port->br->stp_enabled) || ((br_port->br->stp_enabled)&&(br_port->state == BR_STATE_FORWARDING)))
				#endif
#endif				
			) {
				if (!priv->pmib->dot11OperationEntry.disable_brsc) {
					pstat = get_stainfo(priv, da);
					if (pstat && (pstat->tx_pkts > 1) && pstat->expire_to) {	/* Make sure it must have some packets go theough bridge module before shortcut */
						#ifdef WDS
						if (!(pstat->state & WIFI_WDS))	// if WDS peer
						#endif
						{
							#ifdef CONFIG_RTK_MESH
							if( isMeshPoint(pstat))
								{dev = priv->mesh_dev;}
							else
							#endif
								{dev = priv->dev;}


                            STADEBUG("  1. got![%02X%02X%02X:%02X%02X%02X]\n",da[0],da[1],da[2],da[3],da[4],da[5]);
							break;
						}
					}
				}
				#ifdef MBSSID
			      if ((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.vap_enable) {
					for (j=0; j<RTL8192CD_NUM_VWLAN; j++) {
						if ((priv->pvap_priv[j]->assoc_num > 0) && IS_DRV_OPEN(priv->pvap_priv[j]) &&
							!(priv->pvap_priv[j]->pmib->dot11OperationEntry.disable_brsc)) {
							pstat = get_stainfo(priv->pvap_priv[j], da);
							if (pstat && (pstat->tx_pkts > 1) && pstat->expire_to) {
                                STADEBUG(" 2.vap%d ,got![%02X%02X%02X:%02X%02X%02X]\n",j,da[0],da[1],da[2],da[3],da[4],da[5]);                                      
								dev = priv->pvap_priv[j]->dev;
								break;
							}
						}
					}
				}
				#endif
				#ifdef UNIVERSAL_REPEATER
				vxd_priv = GET_VXD_PRIV(priv);
				if((OPMODE & WIFI_STATION_STATE) && (vxd_priv->assoc_num > 0) && IS_DRV_OPEN(vxd_priv) &&
					!vxd_priv->pmib->dot11OperationEntry.disable_brsc) {		
					pstat = get_stainfo(vxd_priv, da);
					if (pstat && (pstat->tx_pkts > 1) && pstat->expire_to) {
						dev = vxd_priv->dev;
						break;
					}	
				}
				#endif
			}
		}
	}
	
	
	return dev;
	
}
#endif

#ifdef __KERNEL__
#if defined(CONFIG_WIRELESS_LAN_MODULE)
MODULE_LICENSE("GPL");
#endif
#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	/* don't init wlan while kernel startup */
#else
module_init(rtl8192cd_init);
module_exit(rtl8192cd_exit);
#endif /* #if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT) */
#endif

