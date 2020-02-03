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


#include "./8192cd_cfg.h"

#ifdef _BROADLIGHT_FASTPATH_
int (*send_packet_to_upper_layer)(struct sk_buff *skb) = netif_rx ;
#endif
#include <linux/syscalls.h>
#include <linux/file.h>
#include <asm/unistd.h>

#include <linux/seq_file.h>

#include <linux/syscalls.h>

#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_rx.h"
#include "./8192cd_debug.h"

#include "./WlanHAL/HalMac88XX/halmac_reg2.h"



#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
#include <asm/mips16_lib.h>
#endif


#define MESH_SHIFT			0
#define MESH_NUM_CFG		0



	#if !defined(CONFIG_NET_PCI) && defined(CONFIG_RTL8196C)
	#include <asm/rtl865x/platform.h>
	#endif

#if !defined(USE_RLX_BSP)
#include <platform.h>
#else
#include <bspchip.h>
#endif


#ifdef PERF_DUMP
extern int rtl8651_romeperfEnterPoint(unsigned int index);
extern int rtl8651_romeperfExitPoint(unsigned int index);
extern int (*Fn_rtl8651_romeperfEnterPoint)(unsigned int index);
extern int (*Fn_rtl8651_romeperfExitPoint)(unsigned int index);
#endif

#include "8192cd_net80211.h"

#include "8192cd_cfg80211.h"



// TODO: Filen, move to BSP Setting

struct _device_info_ wlan_device[] =
{
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

    #else
    	//2G in slot 0, 5G in slot 1, or only one slot used
    	#if defined(CONFIG_USE_PCIE_SLOT_1) && defined(CONFIG_BAND_5G_ON_WLAN0)
        {
    	 	(MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_PCI_DIRECT<<TYPE_SHIFT) | ACCESS_SWAP_MEM,
    		BSP_PCIE1_D_CFG0,
    		BSP_PCIE1_D_MEM,
    				BSP_PCIE2_IRQ,
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
    				BSP_PCIE2_IRQ,
        NULL
    	},
    	#endif

    #endif

    #if defined(CONFIG_WLAN_HAL_8197F) && defined(CONFIG_BAND_5G_ON_WLAN0)
    {
        (MESH_NUM_CFG<<MESH_SHIFT) |(WDS_NUM_CFG<<WDS_SHIFT) | (TYPE_EMBEDDED<<TYPE_SHIFT) | ACCESS_SWAP_MEM,
        BSP_WLAN_CONF_ADDR,
        BSP_WLAN_BASE_ADDR,
        BSP_WLAN_MAC_IRQ,
        NULL
    },
    #endif  //CONFIG_WLAN_HAL_8197F

};




static int wlan_index=0;
int drv_registered = FALSE;







// init and remove char device
extern int rtl8192cd_chr_init(void);
extern void rtl8192cd_chr_exit(void);
struct rtl8192cd_priv *rtl8192cd_chr_reg(unsigned int minor, struct rtl8192cd_chr_priv *priv);
void rtl8192cd_chr_unreg(unsigned int minor);
int rtl8192cd_fileopen(const char *filename, int flags, int mode);


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
	unsigned long x;

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
	unsigned long x;

	SAVE_INT_AND_CLI(x);

	rtl8651_romeperfReset();

	RESTORE_INT(x);
    return count;
}
#endif // _INCLUDE_PROC_FS_ && PERF_DUMP


static void rtl8192cd_bcnProc(struct rtl8192cd_priv *priv, unsigned int bcnInt,
				unsigned int bcnOk, unsigned int bcnErr, unsigned int status
				)
{
	int i;
		struct rtl8192cd_priv *priv_root=NULL;

	/* ================================================================
			Process Beacon OK/ERROR interrupt
		================================================================ */
	if ( bcnOk || bcnErr)
	{

		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
						(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}

		//
		// Statistics and LED counting
		//
		if (bcnOk) {
			// for SW LED
			if (priv->pshare->LED_cnt_mgn_pkt)
				priv->pshare->LED_tx_cnt++;
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_ok++;
			SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);

			// disable high queue limitation
			if ((OPMODE & WIFI_AP_STATE) && (priv->pshare->bcnDOk_priv)) {
				if (*((unsigned char *)priv->pshare->bcnDOk_priv->beaconbuf + priv->pshare->bcnDOk_priv->timoffset + 4) & 0x01)  {
					RTL_W16(RD_CTRL, RTL_R16(RD_CTRL) | HIQ_NO_LMT_EN);
				}
			}

		} else if (bcnErr) {
			if (priv->pshare->bcnDOk_priv)
				priv->pshare->bcnDOk_priv->ext_stats.beacon_er++;
		}

		if (priv_root != NULL)
			priv = priv_root;
	}




	/* ================================================================
			Process Beacon interrupt
	    ================================================================ */
	//
	// Update beacon content
	//
	if (bcnInt) {
		unsigned char val8;
		if (
			(status & HIMR_BCNDMA0)) {
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			if (GET_VXD_PRIV(priv)->timoffset) {
				update_beacon(GET_VXD_PRIV(priv));
			}
			} else
			{
				if (priv->timoffset) {
					update_beacon(priv);
				}
			}
		}
		else {
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (
						(status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1))))) {
						if (priv->pvap_priv[i]->timoffset) {
							update_beacon(priv->pvap_priv[i]);
						}
					}
				}
			}
		}

		//
		// Polling highQ as there is multicast waiting for tx...
		//
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}

		if ((OPMODE & WIFI_AP_STATE)) {
			if (

				(status & HIMR_BCNDMA0)) {
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
					if(RTL_R8(BCN_CTRL) & DIS_ATIM)
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
				} else {
					if(!(RTL_R8(BCN_CTRL) & DIS_ATIM))
						RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));
				}
//#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
//#endif
			}
			else if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (
						(status & (HIMR_BCNDMA1 << (priv->pvap_priv[i]->vap_init_seq-1))))) {
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
						if (val8 & 0x01) {
							if(RTL_R8(BCN_CTRL) & DIS_ATIM)
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) & (~DIS_ATIM)));
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
						} else {
							if(!(RTL_R8(BCN_CTRL) & DIS_ATIM))
								RTL_W8(BCN_CTRL, (RTL_R8(BCN_CTRL) | DIS_ATIM));
						}

						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}

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
				ClearMData(phdr);
			}
			tx_poll(priv, MCAST_QNUM);
//				priv->pshare->pkt_in_hiQ = 0;
		}


		if (priv_root != NULL)
			priv = priv_root;

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

static void
rtl88XX_bcnProc(
        struct rtl8192cd_priv *priv,
        unsigned int bcnInt,
        unsigned int bcnOk,
        unsigned int bcnErr
)
{
	int i;

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

		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
						(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}

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
			} else
#endif // SUPPORT_EACH_VAP_INT
			{
				if (priv->pshare->bcnDOk_priv)
				{
					priv->pshare->bcnDOk_priv->ext_stats.beacon_ok++;
#if CFG_HAL_MEASURE_BEACON
					CalcBeaconVariation(priv->pshare->bcnDOk_priv);
#endif
				}
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
			} else
#endif // SUPPORT_EACH_VAP_INT
			{
				if (priv->pshare->bcnDOk_priv)
					priv->pshare->bcnDOk_priv->ext_stats.beacon_er++;
			}
		}

		if (priv_root != NULL)
			priv = priv_root;
	}




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

			if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
				(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
				if (GET_VXD_PRIV(priv)->timoffset) {
					update_beacon(GET_VXD_PRIV(priv));
				}
			} else
			{
				if (priv->timoffset) {
					update_beacon(priv);
				}
			}
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
		}
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

		//
		// Polling highQ as there is multicast waiting for tx...
		//
		struct rtl8192cd_priv *priv_root=NULL;
		if ((OPMODE & WIFI_STATION_STATE) && GET_VXD_PRIV(priv) &&
			(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
			priv_root = priv;
			priv = GET_VXD_PRIV(priv);
		}

		if ((OPMODE & WIFI_AP_STATE)) {
			if ( _TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt) ) {
                // TODO: modify code below
				val8 = *((unsigned char *)priv->beaconbuf + priv->timoffset + 4);
				if (val8 & 0x01) {
					process_mcast_dzqueue(priv);
					priv->pkt_in_dtimQ = 0;
				}
//#ifdef MBSSID
				priv->pshare->bcnDOk_priv = priv;
//#endif
			}
			else if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					if ((priv->pvap_priv[i]->vap_init_seq > 0) && IS_DRV_OPEN(priv->pvap_priv[i])
						&& (_TRUE == GET_HAL_INTERFACE(priv)->GetInterruptHandler(priv, HAL_INT_TYPE_BcnInt1 + (priv->pvap_priv[i]->vap_init_seq-1)))){
						val8 = *((unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->pvap_priv[i]->timoffset + 4);
                       // TODO: modify code below
						if (val8 & 0x01) {
							process_mcast_dzqueue(priv->pvap_priv[i]);
							priv->pvap_priv[i]->pkt_in_dtimQ = 0;
						}
						priv->pshare->bcnDOk_priv = priv->pvap_priv[i];
					}
				}
			}

		}

		if (priv_root != NULL)
			priv = priv_root;

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

__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
static __inline__
VOID
InterruptRxHandle(
    struct rtl8192cd_priv *priv, BOOLEAN caseRxRDU
)
{
#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX)
	if (!priv->pshare->has_triggered_rx_tasklet) {
		priv->pshare->has_triggered_rx_tasklet = 1;
		GET_HAL_INTERFACE(priv)->DisableRxRelatedInterruptHandler(priv);
        tasklet_hi_schedule(&priv->pshare->rx_tasklet);
	}

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
}

__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
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
    if (OPMODE & WIFI_MP_STATE)
        rtl8192cd_tx_dsr((unsigned long)priv);
    else
#endif

    if (GET_HAL_INTERFACE(priv)->QueryTxConditionMatchHandler(priv)) {
        if (!priv->pshare->has_triggered_tx_tasklet) {
            tasklet_schedule(&priv->pshare->tx_tasklet);
            priv->pshare->has_triggered_tx_tasklet = 1;
        }
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

#ifdef SUPPORT_TX_AMSDU
    GET_HAL_INTERFACE(priv)->RemoveInterruptMaskHandler(priv, HAL_INT_TYPE_FS_TIMEOUT0);

    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_TSF_TIMER, (pu1Byte)&current_value);
    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_PS_TIMER, (pu1Byte)&timeout);

    if (TSF_LESS(current_value, timeout))
        setup_timer2(priv, timeout);
    else
        amsdu_timeout(priv, current_value);

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
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
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
			//C2H_isr_88XX(priv);
			GET_HAL_INTERFACE(priv)->C2HHandler(priv);
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
                    tasklet_schedule(&priv->pshare->swq_tasklet);
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

void check_dma_error(struct rtl8192cd_priv *priv, unsigned int status, unsigned int status_ext)
{
	unsigned char reg_rxdma;
	unsigned int reg_txdma;
	int clear_isr=0, check_tx_dma=0, check_rx_dma=0;

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
		{
			RTL_W32(HISR, status);
			RTL_W32(HISRE, status_ext);
		}
	}
}


#define	RTL_WLAN_INT_RETRY_MAX	(2)

#if(CONFIG_WLAN_NOT_HAL_EXIST)
__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
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
#endif


	if(GET_CHIP_VER(priv)!= VERSION_8812E)
	{
		check_dma_error(priv, status, status_ext);
	}

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

	{
		if (status & HIMR_TIMEOUT2)
		caseTimer2 = 1;
	}
	#endif

	if (caseBcnInt || caseBcnStatusOK || caseBcnStatusER || caseBcnDmaOK){
                rtl8192cd_bcnProc(priv, caseBcnInt, caseBcnStatusOK, caseBcnStatusER, status
			);
        }

	//
	// Rx interrupt
	//
	if (caseRxStatus)
	{
		// stop RX first
#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX)
		if (!priv->pshare->has_triggered_rx_tasklet) {
			priv->pshare->has_triggered_rx_tasklet = 1;
			{
				//RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_RDU | HIMR_ROK));

				RTL_W32(HIMR, priv->pshare->InterruptMask & ~(HIMR_RXFOVW | HIMR_ROK));
			}
			tasklet_hi_schedule(&priv->pshare->rx_tasklet);
		}

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
	}

	//
	// Tx interrupt
	//
	phw = GET_HW(priv);
#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE)
		rtl8192cd_tx_dsr((unsigned long)priv);
	else
#endif
	if (
		(CIRC_CNT_RTK(phw->txhead0, phw->txtail0, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead1, phw->txtail1, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead2, phw->txtail2, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead3, phw->txtail3, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead4, phw->txtail4, CURRENT_NUM_TX_DESC) > 10) ||
		(CIRC_CNT_RTK(phw->txhead5, phw->txtail5, CURRENT_NUM_TX_DESC) > 10)
	) {
		if (!priv->pshare->has_triggered_tx_tasklet) {
			tasklet_schedule(&priv->pshare->tx_tasklet);
			priv->pshare->has_triggered_tx_tasklet = 1;
		}
	}

#ifdef SUPPORT_TX_AMSDU
	if (caseTimer2) {

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
                    tasklet_schedule(&priv->pshare->swq_tasklet);
                }
            }
#endif
        }
    }
#endif //defined(SW_TX_QUEUE) || defined(RTK_ATM)


	if ((retry_cnt++)<RTL_WLAN_INT_RETRY_MAX) {

	status_bak = status;
	status_ext_bak = status_ext;

	{
		status = RTL_R32(HISR);
		status_ext = RTL_R32(HISRE);
	}

	if ((status!=0||status_ext!=0)) {
		//watchdog_kick();

		{
			RTL_W32(HISR, status);
			RTL_W32(HISRE, status_ext);
		}

		if((status_bak != status) || (status_ext_bak != status_ext))
		goto retry_process;
	}
	}
int_exit:
	return SUCCESS;
}
#else

__MIPS16
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
__inline__ static int __rtl8192cd_interrupt(void *dev_instance)
{
	return 0;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST


__MIPS16
__IRAM_IN_865X
irqreturn_t rtl8192cd_interrupt(int irq, void *dev_instance)
{
	struct net_device *dev = (struct net_device *)dev_instance;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	if (IS_HAL_CHIP(priv))
	    __rtl_wlan_interrupt(dev_instance);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
	__rtl8192cd_interrupt(dev_instance);
	return IRQ_HANDLED;
}

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
#endif // CONFIG_NETDEV_MULTI_TX_QUEUE

static void rtl8192cd_set_rx_mode(struct net_device *dev)
{

}


struct net_device_stats *rtl8192cd_get_stats(struct net_device *dev)
{
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);



	if(!priv)
	{
		priv = ((struct rtl8192cd_priv *)netdev_priv(dev));
	}
	if(priv == NULL)
	{
	}

	SMP_LOCK(flags);



	SMP_UNLOCK(flags);
	return &(priv->net_stats);
}

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
                #if !defined(CONFIG_SOC_TX_BEAMFORMING)
                priv->pmib->dot11RFEntry.txbf = 0;
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





	if (GET_CHIP_VER(priv) == VERSION_8822B) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|
			WLAN_MULTIMACCLONE_SUPPORT  |
			0;
	}

	if (GET_CHIP_VER(priv) == VERSION_8197F) {
		ability =	\
			WLAN_BEAMFORMING_SUPPORT	|	/*verification ok, 20160112, JIRA ID: WISOCNI-21*/
			WLAN_MULTIMACCLONE_SUPPORT  |
			0;
	}

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
	unsigned long ring_virt_addr;
	unsigned long ring_dma_addr;
	unsigned int  ring_buf_len;
	unsigned long alloc_dma_buf;
	struct sk_buff	*pskb;
	unsigned char	*page_ptr;
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
	if(GET_CHIP_VER(priv) == VERSION_8822B)
	{
		if(priv->pmib->dot11RFEntry.txbf_mu && (priv->pmib->dot11RFEntry.txbf==0)){

			panic_printk("!! error !! txbf_mu = 1 && txbf = 0 \n");
			priv->pmib->dot11RFEntry.txbf_mu = 0;
		}
	}
#endif
#endif //BEAMFORMING_SUPPORT

	if(GET_CHIP_VER(priv) == VERSION_8822B)
		hard_code_8822_mibs(priv);



#ifdef ENABLE_RTL_SKB_STATS
 	rtl_atomic_set(&priv->rtl_tx_skb_cnt, 0);
 	rtl_atomic_set(&priv->rtl_rx_skb_cnt, 0);
#endif

	if (IS_ROOT_INTERFACE(priv))
	{
		/* will not initialize the tasklet if the driver is rebooting due to the detection of radar */
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
				if (!priv->pshare->switch_chan_rp)
				{
					{
#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
						tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_dsr, (unsigned long)priv);
#else
						tasklet_init(&priv->pshare->rx_tasklet, rtl8192cd_rx_tkl_isr, (unsigned long)priv);
#endif
						tasklet_init(&priv->pshare->tx_tasklet, rtl8192cd_tx_dsr, (unsigned long)priv);
						tasklet_init(&priv->pshare->oneSec_tasklet, rtl8192cd_expire_timer, (unsigned long)priv);
					}
				}
			}
		}

		if (priv->pmib->dot11DFSEntry.DFS_detected)
			priv->pmib->dot11DFSEntry.DFS_detected = 0;

		phw = GET_HW(priv);

		// save descriptor virtual address before reset, david
		ring_virt_addr = phw->ring_virt_addr;
		ring_dma_addr = phw->ring_dma_addr;
		ring_buf_len = phw->ring_buf_len;
		alloc_dma_buf = phw->alloc_dma_buf;
		// save RF related settings before reset
		MIMO_TR_hw_support = phw->MIMO_TR_hw_support;
		NumTotalRFPath = phw->NumTotalRFPath;

		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
		phw->ring_virt_addr = ring_virt_addr;
		phw->ring_buf_len = ring_buf_len;
		phw->alloc_dma_buf = alloc_dma_buf;

		phw->MIMO_TR_hw_support = MIMO_TR_hw_support;
		phw->NumTotalRFPath = NumTotalRFPath;

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

	if (IS_ROOT_INTERFACE(priv))
		priv->site_survey->count=0;

#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	{
		if (!priv->pshare->switch_chan_rp)
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

	if (IS_ROOT_INTERFACE(priv))
	{
		// zero all data members below (including) LED_Timer of share_info
		offset = (unsigned long)(&((struct priv_shared_info*)0)->LED_Timer);
		memset((void *)((unsigned long)priv->pshare+ offset), 0, sizeof(struct priv_shared_info)-offset);




#if defined(CONFIG_TCP_ACK_TXAGG) || defined(CONFIG_XMITBUF_TXAGG_ADV)
		mod_timer(&priv->pshare->xmit_check_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));
#endif


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

		page_ptr = (unsigned char *)phw->ring_virt_addr;
		memset(page_ptr, 0, phw->ring_buf_len); // this is vital!


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

	        memset(&phw->tx_desc0_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc1_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc2_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc3_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc4_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
	        memset(&phw->tx_desc5_dma_addr,   0, sizeof(unsigned long) * NUM_TX_DESC);
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
		{//not HAL
			phw->ring_dma_addr = virt_to_bus(page_ptr)+CONFIG_LUNA_SLAVE_PHYMEM_OFFSET;

		phw->rx_ring_addr  = phw->ring_dma_addr;
		phw->tx_ring0_addr = phw->ring_dma_addr + NUM_RX_DESC_IF(priv) * sizeof(struct rx_desc);
		phw->tx_ring1_addr = phw->tx_ring0_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring2_addr = phw->tx_ring1_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring3_addr = phw->tx_ring2_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring4_addr = phw->tx_ring3_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ring5_addr = phw->tx_ring4_addr + NUM_TX_DESC * sizeof(struct tx_desc);
		phw->tx_ringB_addr = phw->tx_ring5_addr + NUM_TX_DESC * sizeof(struct tx_desc);

		phw->rx_descL = (struct rx_desc *)page_ptr;
		phw->tx_desc0 = (struct tx_desc *)(page_ptr + NUM_RX_DESC_IF(priv) * sizeof(struct rx_desc));
		phw->tx_desc1 = (struct tx_desc *)((unsigned long)phw->tx_desc0 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc2 = (struct tx_desc *)((unsigned long)phw->tx_desc1 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc3 = (struct tx_desc *)((unsigned long)phw->tx_desc2 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc4 = (struct tx_desc *)((unsigned long)phw->tx_desc3 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_desc5 = (struct tx_desc *)((unsigned long)phw->tx_desc4 + NUM_TX_DESC * sizeof(struct tx_desc));
		phw->tx_descB = (struct tx_desc *)((unsigned long)phw->tx_desc5 + NUM_TX_DESC * sizeof(struct tx_desc));
		/* To set the DMA address for both RX/TX ring */
		{

			/* RX RING */
			for (i=0; i<NUM_RX_DESC_IF(priv); i++) {
				phw->rx_descL_dma_addr[i] = get_physical_addr(priv, (void *)(&phw->rx_descL[i]),
					sizeof(struct rx_desc), PCI_DMA_TODEVICE);
			}

		}

		DEBUG_INFO("rx_descL=%08lx tx_desc0=%08lx, tx_desc1=%08lx, tx_desc2=%08lx, tx_desc3=%08lx, tx_desc4=%08lx, "
			"tx_desc5=%08lx, tx_descB=%08lx\n",
			(unsigned long)phw->rx_descL, (unsigned long)phw->tx_desc0, (unsigned long)phw->tx_desc1, (unsigned long)phw->tx_desc2,
			(unsigned long)phw->tx_desc3, (unsigned long)phw->tx_desc4, (unsigned long)phw->tx_desc5, (unsigned long)phw->tx_descB);
		}

		rtk_queue_init(&priv->pshare->skb_queue);

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


		}

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
            {
                priv->pshare->atm_swq_use_hw_timer = 1;
            }
        }
#endif

#ifdef SW_TX_QUEUE
        {
            priv->pshare->swq_use_hw_timer = 1;
            tasklet_init(&priv->pshare->swq_tasklet, rtl8192cd_swq_timeout, (unsigned long)priv);
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



	if(GET_CHIP_VER(priv) < VERSION_8812E) {
		priv->pmib->dot11nConfigEntry.dot11nLDPC = 0;
		DEBUG_INFO("Disable LDPC due to old chip\n");
	}
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
	INIT_LIST_HEAD(&priv->wlan_acl_list);
	INIT_LIST_HEAD(&priv->wlan_aclpolllist);

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


	INIT_LIST_HEAD(&(priv->asoc_list));
	INIT_LIST_HEAD(&(priv->auth_list));
	INIT_LIST_HEAD(&(priv->sleep_list));
	INIT_LIST_HEAD(&(priv->defrag_list));
	INIT_LIST_HEAD(&(priv->wakeup_list));
	INIT_LIST_HEAD(&(priv->rx_datalist));
	INIT_LIST_HEAD(&(priv->rx_mgtlist));
	INIT_LIST_HEAD(&(priv->rx_ctrllist));
	INIT_LIST_HEAD(&(priv->addRAtid_list));	// to avoid add RAtid fail
	INIT_LIST_HEAD(&(priv->addrssi_list));



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

		if(!priv->pmib->dot11RFEntry.dot11channel)
			priv->pmib->dot11RFEntry.dot11channel = priv->available_chnl[0];

		if (priv->pmib->dot11RFEntry.dot11channel != 0) {
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
				if ((OPMODE & WIFI_AP_STATE) && !priv->pmib->dot11DFSEntry.disable_DFS && (priv->pshare->dfsSwitchChannel != 0)) {
					priv->pmib->dot11RFEntry.dot11channel = priv->pshare->dfsSwitchChannel;
					priv->pshare->dfsSwitchChannel = 0;
					priv->auto_channel = 0;
					priv->auto_channel_backup = 1;
				}
				else
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
		if (IS_ROOT_INTERFACE(priv))
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
		if (IS_ROOT_INTERFACE(priv))
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

	if (IS_ROOT_INTERFACE(priv))
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
		if (IS_ROOT_INTERFACE(priv))
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

// Add mac clone address manually ----------
		if (priv->pmib->ethBrExtInfo.macclone_enable == 2) {
			extern void mac_clone(struct rtl8192cd_priv *priv, unsigned char *addr);
			mac_clone(priv, priv->pmib->ethBrExtInfo.nat25_dmzMac);
			priv->macclone_completed = 1;
		}
//------------------------- david+2007-5-31

		if (IS_ROOT_INTERFACE(priv))
		{
			if (priv->pshare->ra40MLowerMinus) {
				priv->pshare->rf_ft_var.raGoUp40MLower+=5;
				priv->pshare->rf_ft_var.raGoDown40MLower+=5;
				priv->pshare->ra40MLowerMinus = 0;
			}
		}

	}
#endif

	// initialize event queue
	DOT11_InitQueue(priv->pevent_queue);


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

	if ((priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 0)
		&& (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)
		&& (priv->pmib->dot1180211AuthEntry.dot11WPACipher || priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher))
			rsn_init(priv);

	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {
		psk_init(priv);
#ifdef SUPPORT_TX_AMSDU
		priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 2;
#endif
	} else {
#ifdef SUPPORT_TX_AMSDU
		if ((priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)
			&& (priv->pmib->dot1180211AuthEntry.dot11WPACipher || priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher)
		) {
			priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 2; // 802.1x
		} else {
			if(GET_CHIP_VER(priv) == VERSION_8822B)
				priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 2;
			else
			priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum = 4; // open
		}
#endif
	}


#ifdef WIFI_WPAS
	{
		WPAS_ASSOCIATION_INFO Assoc_Info;
		memset((void *)&Assoc_Info, 0, sizeof(struct _WPAS_ASSOCIATION_INFO));
		Assoc_Info.ReqIELen = priv->pmib->dot11RsnIE.rsnie[1]+ 2;
		memcpy(Assoc_Info.ReqIE, priv->pmib->dot11RsnIE.rsnie, Assoc_Info.ReqIELen);

		//event_indicate_wpas(priv, NULL, WPAS_ASSOC_INFO, (UINT8 *)&Assoc_Info);
	}
#endif




#ifdef CH_LOAD_CAL
#ifdef RTK_MESH_METRIC_REFINE
    if(priv->pmib->meshPathsel.mesh_crossbandEnable)
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

			if (ssid_num >= 5)
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
	if (IS_ROOT_INTERFACE(priv))
		wps_init(priv);
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


	priv->hiddenAP_backup = priv->pmib->dot11OperationEntry.hiddenAP;

#ifdef BT_COEXIST_HAL
	if(GET_CHIP_VER(priv) == VERSION_8197F)
		bt_coexist_hal_init(priv);
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
    int                         halQnum;
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;

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



	if (IS_ROOT_INTERFACE(priv))
	{
#if defined(CONFIG_TCP_ACK_TXAGG) || defined(CONFIG_XMITBUF_TXAGG_ADV)
		if (timer_pending(&priv->pshare->xmit_check_timer))
			del_timer_sync(&priv->pshare->xmit_check_timer);
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

#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
		if (!priv->pshare->switch_chan_rp)
		ODM_StopAllThreads(ODMPTR);
	#ifdef _OUTSRC_COEXIST
		if(IS_OUTSRC_CHIP(priv))
	#endif
			ODM_CancelAllTimers(ODMPTR);


#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
	if (timer_pending(&priv->ps_timer))
		del_timer_sync(&priv->ps_timer);
#endif

#ifdef SDIO_AP_OFFLOAD
	if (timer_pending(&priv->pshare->ps_timer))
		del_timer_sync(&priv->pshare->ps_timer);
#endif



#ifdef RTK_ATM
		if (priv->pshare->atm_swq_use_hw_timer == 0 && timer_pending(&priv->pshare->atm_swq_sw_timer)) {
            del_timer_sync(&priv->pshare->atm_swq_sw_timer);
        }
#endif

#ifdef SW_TX_QUEUE
        if(priv->pshare->swq_use_hw_timer) {
            tasklet_kill(&priv->pshare->swq_tasklet);
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

	if (IS_VXD_INTERFACE(priv)){
		//printk("Try delete ibss beacon timer!! \n");
		if (timer_pending(&priv->pshare->vxd_ibss_beacon))
			del_timer_sync(&priv->pshare->vxd_ibss_beacon);
	}

	if (timer_pending(&priv->ss_timer))
		del_timer_sync(&priv->ss_timer);
	if (timer_pending(&priv->MIC_check_timer))
		del_timer_sync(&priv->MIC_check_timer);
	if (timer_pending(&priv->assoc_reject_timer))
		del_timer_sync(&priv->assoc_reject_timer);
	if (timer_pending(&priv->add_ps_timer))
		del_timer_sync(&priv->add_ps_timer);


#ifdef CLIENT_MODE
	if (timer_pending(&priv->reauth_timer))
		del_timer_sync(&priv->reauth_timer);
	if (timer_pending(&priv->reassoc_timer))
		del_timer_sync(&priv->reassoc_timer);
	if (timer_pending(&priv->idle_timer))
		del_timer_sync(&priv->idle_timer);
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

	if (IS_ROOT_INTERFACE(priv))
	{
		free_txq_pool(&priv->pshare->txq_pool,priv->pshare->txq_pool_addr);
	}
#endif

#if (BEAMFORMING_SUPPORT == 1)
		Beamforming_Release(priv);
#endif


	if (IS_ROOT_INTERFACE(priv))
	{
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

		// for SW LED
		if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE < LEDTYPE_SW_MAX))
			disable_sw_LED(priv);


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
		if (IS_HAL_CHIP(priv)) {
			PHCI_RX_DMA_MANAGER_88XX        prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
		    PHCI_RX_DMA_QUEUE_STRUCT_88XX	cur_q   = &(prx_dma->rx_queue[0]);
			cur_q->host_idx = cur_q->cur_host_idx = 0;
			cur_q->rxbd_ok_cnt = 0;
		}
#endif

		for (i=0; i<NUM_RX_DESC_IF(priv); i++)
		{
			if (phw->rx_infoL[i].pbuf != NULL) {
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
			if(IS_HAL_CHIP(priv) && GET_ROOT(priv)->pmib->miscEntry.vap_enable)
				max_qnum = HIGH_QUEUE7;

			for (j=0; j<=max_qnum; j++) {
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
				{//not HAL
					head = get_txhead(phw, j);
					tail = get_txtail(phw, j);
				}
//				if (i <tail || i >= head)
				if( (tail <= head) ? (i <tail || i >= head) :(i <tail && i >= head))
					continue;

                if (IS_HAL_CHIP(priv)) {
                    if (j >= TXDESC_HIGH_QUEUE_NUM) {
                         if(i>=NUM_TX_DESC_HQ)
                            continue;
                       }
                }


				tx_info = get_txdesc_info(priv->pshare->pdesc_info, j);

#ifdef RESERVE_TXDESC_FOR_EACH_IF
				if (RSVQ_ENABLE && IS_RSVQ(j)) {
					if (tx_info[i].priv)
						tx_info[i].priv->use_txdesc_cnt[RSVQ(j)]--;
				}
#endif

                if (IS_HAL_CHIP(priv)) {
    				if (tx_info[i].buf_pframe[0] && (tx_info[i].buf_type[0] == _SKB_FRAME_TYPE_)) { // should be buf_paddr
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
                {//not HAL
    				if (tx_info[i].pframe && (tx_info[i].type == _SKB_FRAME_TYPE_)) {

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
			if (priv != priv->pshare->aidarray[i]->priv){
				continue;
			} else
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


#if defined(WIFI_WMM) && defined(WMM_APSD)
				kfree(priv->pshare->aidarray[i]->station.VO_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.VI_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BE_dz_queue);
				kfree(priv->pshare->aidarray[i]->station.BK_dz_queue);
#endif

#if defined(WIFI_WMM)
				kfree(priv->pshare->aidarray[i]->station.MGT_dz_queue);
#endif

				kfree(priv->pshare->aidarray[i]->station.wpa_sta_info);
				kfree(priv->pshare->aidarray[i]);
				priv->pshare->aidarray[i] = NULL;
			}
		}
	}




	if (!priv->pmib->dot11OperationEntry.keep_rsnie) {
		priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
		memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
		if (GET_VXD_PRIV(priv))
			GET_VXD_PRIV(priv)->pmib->dot11RsnIE.rsnielen = 0;
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
			!priv->pshare->switch_chan_rp &&
			priv->auto_channel_backup)
			priv->pmib->dot11RFEntry.dot11channel = 0;
	}

	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv) && !priv->pmib->dot11OperationEntry.keep_rsnie) {
			priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
			memset(&priv->pmib->dot11GroupKeysTable, '\0', sizeof(struct Dot11KeyMappingsEntry)); // reset group key
		}
	}

    // mark by Pedro: because priv->bcn_period_bak is zero currently.

	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
			nat25_db_cleanup(priv);
	}

#ifdef A4_STA
#ifdef CHECK_HANGUP
    if (!priv->reset_hangup)
#endif
        a4_sta_cleanup_all(priv);
#endif



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



#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
	flush_rx_queue(priv);
#endif


RESTORE_INT(flags);
	SMP_UNLOCK(flags);


#ifdef WIFI_SIMPLE_CONFIG
	wsc_disconn_list_flush(priv);
#endif

#ifdef BT_COEXIST_HAL
	if(GET_CHIP_VER(priv) == VERSION_8197F)
		bt_coexist_hal_close(priv);
#endif

	return 0;
}


static void rtl8192cd_init_vxd_mib(struct rtl8192cd_priv *priv)
{
	// copy mib_rf from root interface
	memcpy(&priv->pmib->dot11RFEntry, &GET_ROOT(priv)->pmib->dot11RFEntry, sizeof(struct Dot11RFEntry));
}

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


	// copy mib_rf from root interface
	memcpy(&priv->pmib->dot11RFEntry, &GET_ROOT(priv)->pmib->dot11RFEntry, sizeof(struct Dot11RFEntry));


	// special mib that need to set
#ifdef WIFI_WMM
	//QOS_ENABLE = 0;
#ifdef WMM_APSD
	APSD_ENABLE = 0;
#endif
#endif


}

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)


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


		camData[0] = MBIDCAM_POLL | MBIDWRITE_EN | MBIDCAM_VALID |
				(priv->vap_init_seq&MBIDCAM_ADDR_Mask)<<MBIDCAM_ADDR_SHIFT |
				(macAddr[5] << 8) | macAddr[4];
		camData[1] = (macAddr[3] << 24) | (macAddr[2] << 16) | (macAddr[1] << 8) | macAddr[0];
		for (j=1; j>=0; j--) {
			RTL_W32((MBIDCAMCFG+4)-4*j, camData[j]);
		}
//		RTL_W8(_MBIDCAMCFG_, BIT(7) | BIT(6) | ((unsigned char)priv->vap_init_seq & 0x1f));
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

			RTL_W8(BCN_CTRL, EN_BCN_FUNCTION | DIS_SUB_STATE_N | DIS_TSF_UPDATE_N|EN_TXBCN_RPT);

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
			}
		}

		if (RTL_R8(MBID_NUM) & MBID_BCN_NUM_Mask) {
			RTL_W8(MBID_NUM, (RTL_R8(MBID_NUM)-1) & MBID_BCN_NUM_Mask);

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

	if (IS_DRV_OPEN(root_priv->pvxd_priv))
		total_if++;

	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(root_priv->pvap_priv[i]))
			total_if++;
	}

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

	if (IS_DRV_OPEN(root_priv->pvxd_priv))
		total_if++;

	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(root_priv->pvap_priv[i]))
			total_if++;
	}

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
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv))) {
		struct rtl8192cd_priv *priv_vxd = GET_VXD_PRIV(priv);
		rtw_txservq_flush(priv_vxd, &priv_vxd->tx_mgnt_queue);
	}
	if (priv->pmib->miscEntry.vap_enable) {
		int i;
		for (i = 0; i < RTL8192CD_NUM_VWLAN; i++) {
			struct rtl8192cd_priv *priv_vap = priv->pvap_priv[i];
			if (IS_DRV_OPEN(priv_vap)) {
				rtw_txservq_flush(priv_vap, &priv_vap->tx_mgnt_queue);
			}
		}
	}
}

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
#endif // SDIO_AP_OFFLOAD


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
		if (IS_HAL_CHIP(priv))
			RTL_W32(TBTT_PROHIBIT, 0x40004);
		else
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

	if (IS_HAL_CHIP(priv))
		GET_HAL_INTERFACE(priv)->StopMBSSIDHandler(priv);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
		rtl8192cd_stop_mbssid(priv);


}


void init_mbssid(struct rtl8192cd_priv *priv)
{
	//panic_printk("[%s]%s +++\n", priv->dev->name, __FUNCTION__);

	if (IS_HAL_CHIP(priv))
		GET_HAL_INTERFACE(priv)->InitMBSSIDHandler(priv);
	else if(CONFIG_WLAN_NOT_HAL_EXIST)
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
		// set BcnDmaInt & BcnOk of different VAP in IMR
		if (IS_HAL_CHIP(priv))
		{
	    	GET_HAL_INTERFACE(priv)->InitVAPIMRHandler(priv, priv->vap_init_seq);
		}
		else if(CONFIG_WLAN_NOT_HAL_EXIST)
		{ //not HAL
			{
				priv->pshare->InterruptMask |= (HIMR_BCNDMA1 << (priv->vap_init_seq-1));
				RTL_W32(HIMR, priv->pshare->InterruptMask);
			}
		}


		if (GET_ROOT(priv)->auto_channel == 0)
		{
			priv->pmib->dot11RFEntry.dot11channel = GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel;
			priv->ht_cap_len = 0;	// re-construct HT IE

			init_beacon(priv);
		}
	}
}



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




int rtl8192cd_open(struct net_device *dev)
{
	struct rtl8192cd_priv *priv;	// recuresively used, can't be static
	int rc;
    unsigned int errorFlag;

	int i;
	unsigned long x = 0;
#ifdef CHECK_HANGUP
	int is_reset;
#endif
	int init_hw_cnt = 0;


	DBFENTER;

	priv = GET_DEV_PRIV(dev);

    STADEBUG("===>\n");
	if(!is_iface_ready_nl80211(dev, priv))
		return 0;

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


    if(GET_CHIP_VER(priv)==VERSION_8197F) {
        REG32(0xB8000064)|= 0x1F;
    }

#if 0
//#ifdef PCIE_POWER_SAVING
	if (((REG32(CLK_MANAGE) & BIT(11)) == 0)
		&& IS_ROOT_INTERFACE(priv)
	) {
		extern void setBaseAddressRegister(void);
		REG32(CLK_MANAGE) |=  BIT(11);
		delay_ms(10);
		PCIE_reset_procedure(0, 0, 1, priv->pshare->ioaddr);
		setBaseAddressRegister();
	}
#endif

// for Virtual interface...
	if((GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8188E) || (IS_HAL_CHIP(priv))
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

#ifdef CHECK_HANGUP
	is_reset = priv->reset_hangup;
#endif

// init mib from cfg file, we only need to load cfg file once - chris 2010/02
#ifdef CONFIG_RTL_COMAPI_CFGFILE
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



#ifdef SDIO_AP_OFFLOAD
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	}
#endif

	// stop h/w in the very beginning
	if (IS_ROOT_INTERFACE(priv))
	{

#ifdef CONFIG_XMITBUF_TXAGG_ADV
		priv->pshare->need_sched_xmit = 0;
		priv->pshare->low_traffic_xmit = 0;
#endif

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
		{//not HAL
	        if ( check_MAC_IO_Enable(priv) )
	            rtl8192cd_stop_hw(priv);
		}

	}

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
			if (!priv->pmib->dot11OperationEntry.keep_rsnie)
				memset(&priv->pmib->dot11RsnIE, 0, sizeof(struct Dot11RsnIE));


			// Sync regdomain from root AP, ensure connectivity
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11RegDomain;

			priv->drv_state |= DRV_STATE_VXD_INIT;	// indicate the mib of vxd driver has been initialized
		}
#ifdef SDIO_AP_OFFLOAD
		priv->pshare->ps_ctrl = RTW_ACT_SETREP;
#endif
	}

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
#endif

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
				rtl8192cd_init_vap_mib(priv);
			}
		}
	}

	if (IS_VXD_INTERFACE(priv))
		rtl8192cd_init_vxd_mib(priv);

// check phyband and channel match or not
#if 0 //def MP_TEST
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

		if (!priv->pmib->dot11DFSEntry.disable_DFS)
			priv->pmib->dot11DFSEntry.disable_DFS;
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


    if (IS_ROOT_INTERFACE(priv))
    {
        priv->pshare->current_num_tx_desc = NUM_TX_DESC;


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

//#ifdef CONFIG_RTL865X_AC
#if defined(CONFIG_RTL865X_AC) || defined(CONFIG_RTL865X_KLD) || defined(CONFIG_RTL8196B_KLD) || defined(CONFIG_RTL8196C_KLD)
	if (!priv->auto_channel) {
		LOG_START_MSG();
	}
#endif

	validate_fixed_tx_rate(priv);

	if (IS_ROOT_INTERFACE(priv))
	{

#ifdef CHECK_HANGUP
		if (!is_reset)
#endif
		{
		#if defined(CONFIG_RTL_92D_DMDP) || defined(NOT_RTK_BSP) || defined (CONFIG_CT_PUSHKEYEVENT)
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_SHARED, dev->name, dev);
			#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_SHARED|IRQF_DISABLED, dev->name, dev);
			#endif
		#else
			rc = request_irq(dev->irq, rtl8192cd_interrupt, IRQF_DISABLED, dev->name, dev);
		#endif

#if defined(GPIO_WAKEPIN) && IS_EXIST_RTL8822BE
            if(IS_HARDWARE_TYPE_8822B(priv))
                rc |= request_irq_for_wakeup_pin_V2(dev);
#endif

			if (rc) {
				DEBUG_ERR("some issue in request_irq, rc=%d\n", rc);
			}
		}

		//SAVE_INT_AND_CLI(x);
		SMP_LOCK(x);

		watchdog_kick();



do_hw_init:


#if defined(CONFIG_AUTO_PCIE_PHY_SCAN)
#endif
#if !defined(CONFIG_AUTO_PCIE_PHY_SCAN) && !defined(CONFIG_PHY_EAT_40MHZ) && (defined(CONFIG_RTL_8197DL) || defined(CONFIG_RTL8672))
#endif

		rc = rtl8192cd_init_hw_PCI(priv);
		//delay_ms(200);		// TODO: need refinement, for 98 watchdog time out


		// write IDR0, IDR4 here
		{
			if (IS_HAL_CHIP(priv)) {
                if(!IS_MACHAL_CHIP(priv))
                {
        	    GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)dev->dev_addr);
                }
			} else if(CONFIG_WLAN_NOT_HAL_EXIST)
			{//not HAL
				unsigned long reg = 0;
				reg = *(unsigned long *)(dev->dev_addr);
	//			RTL_W32(IDR0, (cpu_to_le32(reg)));
				RTL_W32(MACID, (cpu_to_le32(reg)));
				reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
	//			RTL_W32(IDR4, (cpu_to_le32(reg)));
				RTL_W16(MACID+4, (cpu_to_le16(reg)));
			}
		}

		if (rc && ++init_hw_cnt < 5) {
		if (GET_CHIP_VER(priv)==VERSION_8192D) {
			watchdog_kick();
		}
			goto do_hw_init;
		}

		//if (GET_CHIP_VER(priv)==VERSION_8192D)  {
		watchdog_kick();

		//}

		//RESTORE_INT(x);
		SMP_UNLOCK(x);

		if (rc) {
			DEBUG_ERR("init hw failed!\n");
			force_stop_wlan_hw();
			{
			local_irq_disable();
			*(volatile unsigned long *)(0xB800311c) = 0; /* enable watchdog reset now */
			for(;;)
				;
			}
			goto free_res;
		}
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
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
			rtl8192cd_set_mbidcam(priv, GET_MY_HWADDR, 7);
	}
#endif

#ifdef HW_FILL_MACID
    if(IS_SUPPORT_HW_FILL_MACID(priv)) {
        GET_HAL_INTERFACE(priv)->InitMACIDSearchHandler(priv);
    }
#endif //#ifdef HW_FILL_MACID

	if ((OPMODE & WIFI_AP_STATE) && (GET_ROOT(priv)->pmib->miscEntry.vap_enable)) {
		refine_TBTT(priv);
		init_vap_hw(priv);
		init_vap_beacon(priv);

		if(IS_ROOT_INTERFACE(priv))
		{
			//for  performance
			//panic_printk("\n FORCE ROOT VAP_ENABLE=0 !! \n\n");
			priv->pmib->miscEntry.vap_enable = 0;
		}

	}
	else if (IS_VAP_INTERFACE(priv) && (OPMODE & WIFI_STATION_STATE)) {
		if (IS_HAL_CHIP(priv))
        	GET_HAL_INTERFACE(priv)->InitMBIDCAMHandler(priv);
		else if(CONFIG_WLAN_NOT_HAL_EXIST)
			; //ToDo
	}

    if(IS_ROOT_INTERFACE(priv))
    {
		if (IS_HAL_CHIP(priv)){
	        u1Byte HIQ_En=0;
	        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HIQ_En);
	        HIQ_En = HIQ_En | BIT0;
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_HIQ_NO_LMT_EN, (pu1Byte)&HIQ_En);
		}
    }


// new added to reset keep_rsnie flag
	if (priv->pmib->dot11OperationEntry.keep_rsnie)
		priv->pmib->dot11OperationEntry.keep_rsnie = 0;
//------------------- david+2006-06-30

priv->drv_state |= DRV_STATE_OPEN;      // set driver as has been opened, david


	if (IS_HAL_CHIP(priv)) {
		GET_HAL_INTERFACE(priv)->EnableIMRHandler(priv);
	}

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	recalc_txdesc_limit(priv);
#endif
#ifdef USE_TXQUEUE
	recalc_tx_bufq_limit(priv);
#endif

	//memcpy((void *)dev->dev_addr, priv->pmib->dot11OperationEntry.hwaddr, 6);

	// below is for site_survey timer
	init_timer(&priv->ss_timer);
	priv->ss_timer.data = (unsigned long) priv;
	priv->ss_timer.function = rtl8192cd_ss_timer;


/*cfg p2p cfg p2p rm*/
/*cfg p2p cfg p2p rm*/

#ifdef CONFIG_RTL_COMAPI_WLTOOLS
	init_waitqueue_head(&priv->ss_wait);
#endif

#ifdef CLIENT_MODE
	init_timer(&priv->reauth_timer);
	priv->reauth_timer.data = (unsigned long) priv;
	priv->reauth_timer.function = rtl8192cd_reauth_timer;

	init_timer(&priv->reassoc_timer);
	priv->reassoc_timer.data = (unsigned long) priv;
	priv->reassoc_timer.function = rtl8192cd_reassoc_timer;

	init_timer(&priv->idle_timer);
	priv->idle_timer.data = (unsigned long) priv;
	priv->idle_timer.function = rtl8192cd_idle_timer;
	init_timer(&priv->dfs_cntdwn_timer);
	priv->dfs_cntdwn_timer.data = (unsigned long) priv;
	priv->dfs_cntdwn_timer.function = rtl8192cd_dfs_cntdwn_timer;
#endif


	priv->frag_to = 0;

	init_timer(&priv->frag_to_filter);
	priv->frag_to_filter.data = (unsigned long) priv;
	priv->frag_to_filter.function = rtl8192cd_frag_timer;

	mod_timer(&priv->frag_to_filter, jiffies + FRAG_TO);

	priv->auth_to = AUTH_TO / HZ;
	priv->assoc_to = ASSOC_TO / HZ;

#ifdef PCIE_POWER_SAVING_DEBUG
	priv->expire_to = 60;
#else
	priv->expire_to = (EXPIRETIME > 100)? (EXPIRETIME / 100) : 86400; /*10ms to 1s*/
#endif

	if (IS_ROOT_INTERFACE(priv))
	{

#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
		init_timer(&priv->ps_timer);
		priv->ps_timer.data = (unsigned long) priv;
		priv->ps_timer.function = RF_MIMO_check_timer;

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
		priv->expire_timer.function = rtl8192cd_1sec_timer;
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


#if 0
		init_timer(&priv->pshare->phw->tpt_timer);
		priv->pshare->phw->tpt_timer.data = (unsigned long)priv;
		priv->pshare->phw->tpt_timer.function = rtl8192cd_tpt_timer;
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


#if defined(CH_LOAD_CAL) || defined(HS2_SUPPORT) || defined(DOT11K) || defined(RTK_SMART_ROAMING)
#if  defined(HS2_SUPPORT) || defined(DOT11K) || defined(RTK_SMART_ROAMING)
	priv->pmib->dot11StationConfigEntry.cu_enable=1;
#endif
	if(IS_ROOT_INTERFACE(priv)){


		phydm_CLMInit(ODMPTR, CLM_SAMPLE_NUM2);
	}
#endif


	// to avoid add RAtid fail
	init_timer(&priv->add_ps_timer);
	priv->add_ps_timer.data = (unsigned long) priv;
	priv->add_ps_timer.function = add_ps_timer;



	if (IS_ROOT_INTERFACE(priv))
	{
#if defined(OSK)
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
	}
#endif


#ifdef SUPPORT_SNMP_MIB
		mib_init(priv);
#endif
	}



	//if (OPMODE & WIFI_AP_STATE)  //in case of station mode, queue will start only after assoc.
		netif_start_queue(dev);		// start queue always


#ifdef CHECK_HANGUP
	if (priv->reset_hangup)
		priv->reset_hangup = 0;
#endif


	if (IS_ROOT_INTERFACE(priv))
	if ((OPMODE & WIFI_AP_STATE) && priv->auto_channel) {
		if (((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _TKIP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _CCMP_PRIVACY_) &&
			  (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _WEP_WPA_MIXED_PRIVACY_)) ||
			 (priv->pmib->dot11RsnIE.rsnielen > 0)) {
			priv->ss_ssidlen = 0;
			DEBUG_INFO("start_clnt_ss, trigger by %s, ss_ssidlen=0\n", (char *)__FUNCTION__);
			start_clnt_ss(priv);
		}
	}

#ifdef CLIENT_MODE
	if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) {
		struct net_bridge_port *br_port;

		br_port = GET_BR_PORT(priv->dev);

		if (br_port) {
			memcpy(priv->br_mac, br_port->br->dev->dev_addr, MACADDRLEN);
		}
	if (IS_VAP_INTERFACE(priv)) {
		if((priv->pmib->miscEntry.vap_enable)&&(OPMODE == WIFI_STATION_STATE)) {
            if(
				!strcmp(priv->dev->name,"wlan0-va1")||!strcmp(priv->dev->name,"wlan1-va1")
				){
                STADEBUG("open ,start ss_timer\n");
    			mod_timer(&priv->ss_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));
            }else if(
            !strcmp(priv->dev->name,"wlan0-va2")||!strcmp(priv->dev->name,"wlan1-va2")
            ){
                STADEBUG("open ,start ss_timer\n");
    			mod_timer(&priv->ss_timer, jiffies + RTL_SECONDS_TO_JIFFIES(15));
            }
		}
	}else
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

#if defined(CONFIG_AUTH_RESULT)
					priv->authRes = 0;
#endif

				}
			}
		}
	}
#endif


	if (IS_VXD_INTERFACE(priv) && (OPMODE & WIFI_ADHOC_STATE))
    {
		init_timer(&priv->pshare->vxd_ibss_beacon);
		priv->pshare->vxd_ibss_beacon.data = (unsigned long) priv;
		priv->pshare->vxd_ibss_beacon.function = issue_beacon_ibss_vxd;
		//mod_timer(&priv->pshare->vxd_ibss_beacon, jiffies + RTL_MILISECONDS_TO_JIFFIES(5000));
	}

	if (IS_ROOT_INTERFACE(priv) &&
		netif_running(GET_VXD_PRIV(priv)->dev)) {
		SAVE_INT_AND_CLI(x);
		rtl8192cd_open(GET_VXD_PRIV(priv)->dev);
		RESTORE_INT(x);
	}
	if (IS_VXD_INTERFACE(priv) &&
		(GET_ROOT(priv)->pmib->dot11OperationEntry.opmode&WIFI_STATION_STATE) &&
		(GET_ROOT(priv)->pmib->dot11OperationEntry.opmode&WIFI_ASOC_STATE) &&
		!(GET_ROOT(priv)->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed) &&
		!(priv->drv_state & DRV_STATE_VXD_AP_STARTED) )
		enable_vxd_ap(priv);

	if (IS_ROOT_INTERFACE(priv)) {
		{
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (netif_running(priv->pvap_priv[i]->dev))
					rtl8192cd_open(priv->pvap_priv[i]->dev);
			}
		}
	}



	if (IS_ROOT_INTERFACE(priv))
	{

#if 0
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
    if(!strcmp(priv->dev->name,"wlan0") || !strcmp(priv->dev->name,"wlan0-va1") ||  !strcmp(priv->dev->name,"wlan0-vxd"))
	{
        priv->reperater_idx=1;
    }
	else if(!strcmp(priv->dev->name,"wlan0-va0") || !strcmp(priv->dev->name,"wlan0-va2"))
	{
        priv->reperater_idx=2;
    }


#ifdef SDIO_AP_OFFLOAD
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	} else
	{
#ifdef USE_WAKELOCK_MECHANISM
		rtw_lock_suspend_timeout(priv, 2000);
#endif
	}
#endif // SDIO_AP_OFFLOAD

	DBFEXIT;

	return 0;

free_res:

	if (IS_ROOT_INTERFACE(priv))
	{
#if defined(CONFIG_TCP_ACK_TXAGG) || defined(CONFIG_XMITBUF_TXAGG_ADV)
		if (timer_pending(&priv->pshare->xmit_check_timer))
			del_timer_sync(&priv->pshare->xmit_check_timer);
#endif
#if defined(MULTI_STA_REFINE)
		if (timer_pending(&priv->pshare->PktAging_timer))
			del_timer_sync(&priv->pshare->PktAging_timer);
#endif
		ODM_StopAllThreads(ODMPTR);
	}

#ifdef SDIO_AP_OFFLOAD
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	} else
	{
		// disable "unlock suspend" to avoid card missing issue in Sequans platform
#if 0//def USE_WAKELOCK_MECHANISM
		rtw_lock_suspend_timeout(priv, 2000);
#endif
	}
#endif // SDIO_AP_OFFLOAD

	return rc;
}

int rtl_setMac(struct net_device *dev, char *mac)
{
	struct rtl8192cd_priv *priv = ((struct rtl8192cd_priv *)netdev_priv(dev))->wlan_priv;

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

int  rtl8192cd_set_hwaddr(struct net_device *dev, void *addr)
{
	unsigned long flags = 0;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	unsigned long reg;
	unsigned char *p, zero_mac[MACADDRLEN]={0};
    BOOLEAN     bVal;


	p = ((struct sockaddr *)addr)->sa_data;

	rtl_setMac(dev, p);

	if(!memcmp(zero_mac,p,MACADDRLEN)) {
		panic_printk("\nWARNING!! Tend to configure %s with zero MAC address!!\n",priv->dev->name);

		if(IS_DRV_OPEN(priv)) {
            if(IS_ROOT_INTERFACE(priv)) {
				int intf_count=0;
				panic_printk("Shut wireless down!!\n");
                for(intf_count=0;intf_count<RTL8192CD_NUM_VWLAN;intf_count++) {
					if(IS_DRV_OPEN(priv->pvap_priv[intf_count]))
						priv->pvap_priv[intf_count]->dev->flags &= ~IFF_UP;
				}
          if(IS_DRV_OPEN(priv->pvxd_priv)) {
						priv->pvxd_priv->dev->flags &= ~IFF_UP;
				}
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


	memcpy(priv->dev->dev_addr, p, 6);
	memcpy(GET_MY_HWADDR, p, 6);
	//brian, for setup MAC address from calibration at flash only during system initialization
	if(IS_ROOT_INTERFACE(priv) && !IS_DRV_OPEN(priv)) {
		memcpy(priv->rtk->root_mac, p, MACADDRLEN);
		memcpy(priv->rtk->wiphy->perm_addr, p, MACADDRLEN);
	}
    memset(dev->broadcast, 0xff, ETH_ALEN);
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (IS_VAP_INTERFACE(priv)) {
			RESTORE_INT(flags);
			SMP_UNLOCK(flags);
			return 0;
		}
	}


#ifdef SDIO_AP_OFFLOAD
	ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
#endif

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

	if (IS_HAL_CHIP(priv)) {
		if(!IS_DRV_OPEN(priv)) {
			RESTORE_INT(flags);
			SMP_UNLOCK(flags);
			return;
		}
	    GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_MAC_IO_ENABLE, (pu1Byte)&bVal);
	    if ( bVal ) {
	        GET_HAL_INTERFACE(priv)->SetHwRegHandler(priv, HW_VAR_ETHER_ADDR, (pu1Byte)p);
	    }
	    else {
//        printk("%s(%d): Can't write MACID register\n", __FUNCTION__, __LINE__);
	    }
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
	{//not HAL
	    if (check_MAC_IO_Enable(priv)) {
	        reg = *(unsigned long *)(dev->dev_addr);
	        RTL_W32(MACID, (cpu_to_le32(reg)));
	        reg = *(unsigned short *)((unsigned long)dev->dev_addr + 4);
	        RTL_W16(MACID+4, (cpu_to_le16(reg)));
		}
	}
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if (OPMODE & WIFI_AP_STATE) {
			if (IS_HAL_CHIP(priv)) {
				if ( bVal )
		            GET_HAL_INTERFACE(priv)->InitMBSSIDHandler(priv);
			}
			else if(CONFIG_WLAN_NOT_HAL_EXIST)
			rtl8192cd_init_mbssid(priv);

		}
		else if (IS_VAP_INTERFACE(priv) && (OPMODE & WIFI_STATION_STATE)) {
			if (IS_HAL_CHIP(priv)) {
				if ( bVal )
		            GET_HAL_INTERFACE(priv)->InitMBIDCAMHandler(priv);
			}
			else if(CONFIG_WLAN_NOT_HAL_EXIST)
				; //ToDo
		}
	}

out:
	RESTORE_INT(flags);
	SMP_UNLOCK(flags);

#ifndef WIFI_WPAS_CLI
#ifdef CLIENT_MODE
	if (!(OPMODE & WIFI_AP_STATE) && netif_running(priv->dev)) {
		int link_status = chklink_wkstaQ(priv);
		if (link_status) {
			start_clnt_join(priv);
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
	struct rtl8192cd_priv *priv_vxd;
	unsigned int errorFlag=0;
	unsigned long flags=0;

	int i;

	if (timer_pending(&priv->ch_avail_chk_timer)) {
		del_timer(&priv->ch_avail_chk_timer);
		RTL_W8(TXPAUSE, 0xff);
		event_indicate_cfg80211(priv, NULL, CFG80211_RADAR_CAC_ABORTED, NULL);
	}

#if 0	//prevent drop vxd, vap connection
	if(IS_ROOT_INTERFACE(priv))
		close_vxd_vap(priv);
#endif
	if(IS_ROOT_INTERFACE(priv) && priv->scan_req)
		event_indicate_cfg80211(priv, NULL, CFG80211_SCAN_ABORDED, NULL);

	if(is_WRT_scan_iface(dev->name))
	{
		//printk("No need to call close for scan iface !!\n");
		return 0;
	}

	STADEBUG("===>\n");
	SMP_LOCK(flags);
	DBFENTER;

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


	priv->drv_state &= ~DRV_STATE_OPEN;     // set driver as has been closed, david

#if defined(PCIE_POWER_SAVING) || defined(RF_MIMO_SWITCH)
	if (timer_pending(&priv->ps_timer)) {
		SMP_UNLOCK(flags);
		del_timer_sync(&priv->ps_timer);
		SMP_LOCK(flags);
	}
#endif

#ifdef SDIO_AP_OFFLOAD
	if (!IS_ROOT_INTERFACE(priv)) {
		ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	} else
	{
		del_timer_sync(&priv->pshare->ps_timer);

		if (RTW_STS_SUSPEND == priv->pshare->pwr_state)
			ap_offload_exit(priv);
	}
#endif // SDIO_AP_OFFLOAD

	/*
	priv->rtk->tx_time = 0;
	priv->rtk->rx_time = 0;
	priv->rtk->chbusytime = 0;
	*/
	memset(priv->rtk->survey_info, 0x0, ARRAY_SIZE(realtek_5ghz_a_channels));
	memset(priv->rtk->psd_fft_info, 0x0, sizeof(priv->rtk->psd_fft_info));

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
/*
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
*/
		if (priv->vap_id < 0)
		{
			disable_vxd_ap(priv);
#ifdef SDIO_2_PORT
			RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT1)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT1));
#endif
		}
	}

	netif_stop_queue(dev);





#ifdef CHECK_HANGUP
	if (!priv->reset_hangup)
#endif
	{
		if (OPMODE & WIFI_AP_STATE) {
			int i;
			for(i=0; i<NUM_STAT; i++)
			{
				if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)
				) {
					if (priv != priv->pshare->aidarray[i]->priv)
						continue;
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

		}
#endif    /* WPS2DOTX   */
	}

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

		if(is_last_vap_iface(priv))
		{
			//stop_mbssid(GET_ROOT(priv));
			refine_TBTT(GET_ROOT(priv));
			GET_ROOT(priv)->pmib->miscEntry.vap_enable = 0;
		}
	}

	if (IS_ROOT_INTERFACE(priv))
	{
		free_rtk_queue(priv, &priv->pshare->skb_queue);


#if 1 //kill tasklets before stop hw
		/* prevent killing tasklet issue in interrupt */
		if (!priv->pmib->dot11DFSEntry.DFS_detected)
		{
#ifdef CHECK_HANGUP
			if (!priv->reset_hangup)
#endif
			{
				{
					if (!priv->pshare->switch_chan_rp)
					{

						{


						RESTORE_INT(flags);
						SMP_UNLOCK(flags);
						tasklet_kill(&priv->pshare->rx_tasklet);
						tasklet_kill(&priv->pshare->tx_tasklet);
						tasklet_kill(&priv->pshare->oneSec_tasklet);
						SMP_LOCK(flags);
						SAVE_INT_AND_CLI(flags);
					}
				}
				}

			}
		}
#endif //kill tasklets before stop hw


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
		{//not HAL
			rtl8192cd_stop_hw(priv);
		}

#ifdef RTLWIFINIC_GPIO_CONTROL
	if(GET_CHIP_VER(priv) == VERSION_8822B) {
		RTL_W8(0x1c, (RTL_R8(0x1c)|BIT(5)));
		RTL_W8(0x1c, (RTL_R8(0x1c)|BIT(6)));
	}
#endif

#ifdef CHECK_HANGUP
		if (!priv->reset_hangup)
#endif
		{
			SMP_UNLOCK(flags);
			free_irq(dev->irq, dev);
			SMP_LOCK(flags);
#if 0 //yllin //#ifdef GPIO_WAKEPIN
            int h1irq = gpio_to_irq(BSP_GPIO_PIN_H1);
            free_irq(h1irq, dev);
#endif
		}


		if (GET_VXD_PRIV(priv))
			GET_VXD_PRIV(priv)->drv_state &= ~DRV_STATE_VXD_INIT;
	}
#ifdef SDIO_AP_OFFLOAD
	if ( IS_ROOT_INTERFACE(priv)) {
#ifdef PLATFORM_ARM_BALONG
		free_irq(dev->irq, priv);
		//gpio_free(rtk_wifi_wake_i);
#endif
	}
#endif
	RESTORE_INT(flags);
	SMP_UNLOCK(flags);
	rtl8192cd_stop_sw(priv);
	SMP_LOCK(flags);
	SAVE_INT_AND_CLI(flags);

	if (IS_ROOT_INTERFACE(priv) && (GET_CHIP_VER(priv)==VERSION_8197F)) {
        REG32(0xB8000064)= 0x00;
	}

#ifdef ENABLE_RTL_SKB_STATS
	DEBUG_INFO("skb_tx_cnt =%d\n", rtl_atomic_read(&priv->rtl_tx_skb_cnt));
	DEBUG_INFO("skb_rx_cnt =%d\n", rtl_atomic_read(&priv->rtl_rx_skb_cnt));
#endif


	RESTORE_INT(flags);
	SMP_UNLOCK(flags);

#if 0//def  CONFIG_WLAN_HAL
        if (IS_ROOT_INTERFACE(priv))
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
	if (IS_ROOT_INTERFACE(priv))
	HostPCIe_Close();
#endif

#ifdef SDIO_AP_OFFLOAD
	if (!IS_ROOT_INTERFACE(priv))
		ap_offload_activate(priv, OFFLOAD_PROHIBIT_DEV_OPS);
	else
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
	struct sockaddr addr;
	p = addr.sa_data;

	priv->pmib->mib_version = MIB_VERSION;
	set_mib_default_tbl(priv);



	// others that are not types of byte and int
	strcpy((char *)priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, "RTL8186-default");
	priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = strlen("RTL8186-default");

	if (IS_ROOT_INTERFACE(priv)) {
		read_flash_hw_cal_data(priv);
	}

	char * name;
	unsigned char * mac=NULL;
	unsigned int vap_idx=-1;

	name = priv->dev->name;
	if (!is_WRT_scan_iface(name) && !IS_VXD_INTERFACE(priv)) {
		if (IS_ROOT_INTERFACE(priv)) {
			vap_idx = 0;
			mac=priv->rtk->root_mac;
		} else if ((IS_VAP_INTERFACE(priv)) && (name[strlen(priv->dev->name)-2]=='-')){ //dev->name="wlanX-X"
			vap_idx=name[strlen(name)-1]-'0';
			mac=priv->rtk->vap_mac[vap_idx];
		}

		if(read_flash_hw_mac_vap(mac, vap_idx))
			memcpy(p, "\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);
		else
		        memcpy(p, mac, MACADDRLEN);
	}

#if defined(DOT11D) || defined(DOT11H) || defined(DOT11K)
	// set countryCode for 11d and 11h
	strcpy(priv->pmib->dot11dCountry.dot11CountryString, "US");
#endif

	rtl8192cd_set_hwaddr(priv->dev, (void *)&addr);

	if (IS_ROOT_INTERFACE(priv))
	{
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

		if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_EMBEDDED) {
			// not implement yet
		} else {
#ifdef IO_MAPPING
			priv->pshare->io_mapping = 1;
#endif
		}
	}


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

	if (is_11AC_Chips(priv)){
		AMSDU_ENABLE = 2;
		priv->pmib->dot11BssType.net_work_type = WIRELESS_11A | WIRELESS_11N | WIRELESS_11AC;
		priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
		priv->pmib->dot11RFEntry.dot11channel = 36;
	}
	else {
		AMSDU_ENABLE = 0;
		priv->pmib->dot11BssType.net_work_type = WIRELESS_11B | WIRELESS_11G | WIRELESS_11N;
		priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
		priv->pmib->dot11RFEntry.dot11channel = 1;
	}

#if defined(CONFIG_SHARE_XCAP_SUPPORT)
	priv->pmib->dot11RFEntry.share_xcap = 1;
#endif

	if ((GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B))
		ODMPTR->TH_L2H_default = 0xf2;
	else if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8881A))
		ODMPTR->TH_L2H_default = 0xef;
	else
		ODMPTR->TH_L2H_default = 0xf3;


}


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
static struct ss_res* alloc_site_survey_res(gfp_t flags)
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


static const struct net_device_ops rtl8192cd_netdev_ops = {
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
        .ndo_start_xmit         = rtl8192cd_start_xmit,
#ifdef CONFIG_NETDEV_MULTI_TX_QUEUE
        .ndo_select_queue       = rtl8192cd_select_queue,
#endif
};





#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif


static int MDL_DEVINIT rtl8192cd_init_one(struct pci_dev *pdev,
                  const struct pci_device_id *ent, struct _device_info_ *wdev, int vap_idx)
{
    struct net_device *dev;
    struct rtl8192cd_priv *priv;
    void *regs;
	struct wifi_mib 		*pmib;
	DOT11_QUEUE				*pevent_queue;
	struct rtl8192cd_hw		*phw;
	struct rtl8192cd_tx_desc_info		*ptxdesc;
	struct wlan_hdr_poll	*pwlan_hdr_poll;
	struct wlanllc_hdr_poll	*pwlanllc_hdr_poll;
	struct wlanbuf_poll		*pwlanbuf_poll;
	struct wlanicv_poll		*pwlanicv_poll;
	struct wlanmic_poll		*pwlanmic_poll;
	struct wlan_acl_poll	*pwlan_acl_poll;
	DOT11_EAP_PACKET		*Eap_packet;
	WPA_GLOBAL_INFO			*wpa_global_info;
	struct ss_res			*site_survey;


	unsigned char *page_ptr = NULL;
    unsigned char *page_ptr_amsdu;


	struct priv_shared_info *pshare;	// david

    BOOLEAN     bVal;
	unsigned int errorFlag;


    int rc=0;
    priv = NULL;
    regs = NULL;
	pmib = NULL;
	pevent_queue = NULL;
	phw = NULL;
	ptxdesc = NULL;
	pwlan_hdr_poll = NULL;
	pwlanllc_hdr_poll = NULL;
	pwlanbuf_poll = NULL;
	pwlanicv_poll = NULL;
	pwlanmic_poll = NULL;
	pwlan_acl_poll = NULL;
	Eap_packet = NULL;
	wpa_global_info = NULL;
	site_survey = NULL;



	pshare = NULL;	// david

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


	dev = alloc_etherdev(sizeof(struct rtl8192cd_priv));
	if (!dev) {
		printk(KERN_ERR "alloc_etherdev() error!\n");
		return -ENOMEM;
	}

	// now, allocating memory for pmib
	pmib = (struct wifi_mib *)kmalloc((sizeof(struct wifi_mib)), GFP_ATOMIC);
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

	if (wdev->priv == NULL) // root interface
	{
		phw = (struct rtl8192cd_hw *)kmalloc((sizeof(struct rtl8192cd_hw)), GFP_ATOMIC);
		if (!phw) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for rtl8192cd_hw (size %d)\n", sizeof(struct rtl8192cd_hw));
			goto err_out_free;
		}
		memset((void *)phw, 0, sizeof(struct rtl8192cd_hw));
		ptxdesc = &phw->tx_info;

		pshare = (struct priv_shared_info *)kmalloc(sizeof(struct priv_shared_info), GFP_ATOMIC);
		if (!pshare) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for priv_shared_info (size %d)\n", sizeof(struct priv_shared_info));
			goto err_out_free;
		}
		memset((void *)pshare, 0, sizeof(struct priv_shared_info));

		pwlan_hdr_poll = (struct wlan_hdr_poll *)
						kmalloc((sizeof(struct wlan_hdr_poll)), GFP_ATOMIC);
		if (!pwlan_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlan_hdr_poll (size %d)\n", sizeof(struct wlan_hdr_poll));
			goto err_out_free;
		}

		pwlanllc_hdr_poll = (struct wlanllc_hdr_poll *)
						kmalloc((sizeof(struct wlanllc_hdr_poll)), GFP_ATOMIC);
		if (!pwlanllc_hdr_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanllc_hdr_poll (size %d)\n", sizeof(struct wlanllc_hdr_poll));
			goto err_out_free;
		}

		pwlanbuf_poll = (struct	wlanbuf_poll *)
						kmalloc((sizeof(struct	wlanbuf_poll)), GFP_ATOMIC);
		if (!pwlanbuf_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanbuf_poll (size %d)\n", sizeof(struct wlanbuf_poll));
			goto err_out_free;
		}

		pwlanicv_poll = (struct	wlanicv_poll *)
						kmalloc((sizeof(struct	wlanicv_poll)), GFP_ATOMIC);
		if (!pwlanicv_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanicv_poll (size %d)\n", sizeof(struct wlanicv_poll));
			goto err_out_free;
		}

		pwlanmic_poll = (struct	wlanmic_poll *)
						kmalloc((sizeof(struct	wlanmic_poll)), GFP_ATOMIC);
		if (!pwlanmic_poll) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for wlanmic_poll (size %d)\n", sizeof(struct wlanmic_poll));
			goto err_out_free;
		}
	}
/*cfg p2p cfg p2p*/

	pwlan_acl_poll = (struct wlan_acl_poll *)
					kmalloc((sizeof(struct wlan_acl_poll)), GFP_ATOMIC);
	if (!pwlan_acl_poll) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wlan_acl_poll (size %d)\n", sizeof(struct wlan_acl_poll));
		goto err_out_free;
	}


	Eap_packet = (DOT11_EAP_PACKET *)
					kmalloc((sizeof(DOT11_EAP_PACKET)), GFP_ATOMIC);
	if (!Eap_packet) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for Eap_packet (size %d)\n", sizeof(DOT11_EAP_PACKET));
		goto err_out_free;
	}
	memset((void *)Eap_packet, 0, sizeof(DOT11_EAP_PACKET));

	wpa_global_info = (WPA_GLOBAL_INFO *)
					kmalloc((sizeof(WPA_GLOBAL_INFO)), GFP_ATOMIC);
	if (!wpa_global_info) {
		rc = -ENOMEM;
		printk(KERN_ERR "Can't kmalloc for wpa_global_info (size %d)\n", sizeof(WPA_GLOBAL_INFO));
		goto err_out_free;
	}
	memset((void *)wpa_global_info, 0, sizeof(WPA_GLOBAL_INFO));

	if ((NULL == wdev->priv) || (vap_idx < 0) || vap_idx == 1 || vap_idx == 2)
        /*for multiple repeater usage,  need to allocate site_survey memory for vap_idx=1,2*/
	{
		site_survey = alloc_site_survey_res(GFP_KERNEL /*GFP_ATOMIC*/);
		if (!site_survey) {
			rc = -ENOMEM;
			printk(KERN_ERR "Can't kmalloc for site_survey (size %d)\n", sizeof(struct ss_res));
			goto err_out_free;
		}
	}

	if (wdev->priv) {
	}
	else
		strcpy(dev->name, "wlan%d");

	if (vap_idx >= 0)
	{
		if(vap_idx == 0)
			sprintf(dev->name, "tmp.%s", wdev->priv->dev->name);
		else
			sprintf(dev->name, "%s-%d", wdev->priv->dev->name, vap_idx);
	}
	else if(wdev->priv)
	{
		sprintf(dev->name, "%s-%d", wdev->priv->dev->name, RTL8192CD_NUM_VWLAN);
	}


	/*SET_MODULE_OWNER is obsolete from 2.6.24*/

	priv = (struct rtl8192cd_priv *)netdev_priv(dev);
	priv->wlan_priv = priv;


	priv->pmib = pmib;
#if 0/*defined(CONFIG_RTL_WAPI_SUPPORT)*/
	/*	only for test	*/
	priv->pmib->wapiInfo.wapiType = wapiDisable;
	priv->pmib->wapiInfo.wapiUpdateMCastKeyType = wapi_disable_update;
	priv->pmib->wapiInfo.wapiUpdateUCastKeyTimeout = WAPI_KEY_UPDATE_PERIOD;
	priv->pmib->wapiInfo.wapiUpdateUCastKeyPktNum = WAPI_KEY_UPDATE_PKTCNT;
#endif
	priv->pevent_queue = pevent_queue;
	priv->pwlan_acl_poll = pwlan_acl_poll;
	priv->Eap_packet = Eap_packet;
	priv->wpa_global_info = wpa_global_info;
	priv->site_survey = site_survey;
	priv->vap_id = -1;

	if (wdev->priv) {
		priv->pshare = wdev->priv->pshare;
		priv->proot_priv = wdev->priv;
		if (vap_idx < 0) // create for vxd
			GET_VXD_PRIV(wdev->priv) = priv;
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
        priv->HalFunc = GET_ROOT(priv)->HalFunc;
        priv->HalData = GET_ROOT(priv)->HalData;

        priv->pHalmac_adapter = GET_ROOT(priv)->pHalmac_adapter;
        priv->pHalmac_api = GET_ROOT(priv)->pHalmac_api;

	}
	else
	{
		priv->pshare = pshare;	// david
		priv->pshare->phw = phw;

		priv->pshare->wlandev_idx = wlan_index;
		priv->pshare->pdesc_info = ptxdesc;
		priv->pshare->pwlan_hdr_poll = pwlan_hdr_poll;
		priv->pshare->pwlanllc_hdr_poll = pwlanllc_hdr_poll;
		priv->pshare->pwlanbuf_poll = pwlanbuf_poll;
		priv->pshare->pwlanicv_poll = pwlanicv_poll;
		priv->pshare->pwlanmic_poll = pwlanmic_poll;
		wdev->priv = priv;


		spin_lock_init(&priv->pshare->lock);




		priv->pshare->type = wdev->type;
		priv->pshare->have_hw_mic = 1;
//		priv->pshare->is_giga_exist  = is_giga_board();
	}
/*cfg p2p cfg p2p*/
	priv->dev = dev;

	if (!IS_ROOT_INTERFACE(priv)) { // is a vxd or vap
		dev->base_addr = GET_ROOT(priv)->dev->base_addr;
		goto register_driver;
	}

	{
        regs = (void *)wdev->base_addr;
        dev->base_addr = (unsigned long)wdev->base_addr;
        priv->pshare->ioaddr = (unsigned long)regs;

        if (((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_DIRECT)
        {
            int i ;
			_DEBUG_INFO("INIT PCI config space directly\n");

			int flags;
			SMP_LOCK(flags);


            if (BSP_PCIE0_D_CFG0 == wdev->conf_addr)
            {
				{
					extern  int PCIE_reset_procedure_97F(unsigned int PCIeIdx, unsigned int mdioReset);
					extern int PCIE_link_ok;
					extern unsigned int PCIE_dev_id;

					#define RTL_8822B_PCI_DEVID 0xb82210ec
					if (!PCIE_link_ok || PCIE_dev_id == RTL_8822B_PCI_DEVID) {
						if ((PCIE_reset_procedure_97F(0,1))  != 1) {
							rc = -ENODEV;
							goto err_out_free2;
						}
					}
					else {
						rc = -ENODEV;
						goto err_out_free2;
					}

				}
            }
            else if (BSP_PCIE1_D_CFG0 == wdev->conf_addr)
			{
            }
            else
            {
				SMP_UNLOCK(flags);
                rc = -ENODEV;
                goto err_out_free2;
            }
			SMP_UNLOCK(flags);

			{
				u32 vendor_deivce_id, config_base;
				config_base = wdev->conf_addr;
				vendor_deivce_id = *((volatile unsigned long *)(config_base+0));
				DEBUG_INFO("config_base=%x, vendor_deivce_id=%x\n", config_base, vendor_deivce_id);
				if (
					(vendor_deivce_id != ((unsigned long)((0x8191<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8171<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8178<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8174<<16)|PCI_VENDOR_ID_REALTEK))) &&
					(vendor_deivce_id != ((unsigned long)((0x8176<<16)|PCI_VENDOR_ID_REALTEK)))
                    && (vendor_deivce_id != ((unsigned long)((0xb822<<16)|PCI_VENDOR_ID_REALTEK)))
					) {
					_DEBUG_ERR("vendor_deivce_id=%x not match\n", vendor_deivce_id);
					rc = -EIO;
					goto err_out_free2;
			    }

                if (vendor_deivce_id == ((unsigned long)((0xb822<<16)|PCI_VENDOR_ID_REALTEK))) {
                    priv->pshare->version_id = VERSION_8822B;
                    DEBUG_INFO("\n found 8822B !!! \n");
                    printk("\n found 8822B !!! \n");
                }
                else
				{
					priv->pshare->version_id = 0;
				}
			}

            *((volatile unsigned long *)PCI_CONFIG_BASE1) = virt_to_bus((void *)dev->base_addr);

//#if defined(CONFIG_WLAN_HAL_8822BE) || defined(CONFIG_RTL_8812_SUPPORT)
//			if(priv->pshare->version_id == VERSION_8822B || priv->pshare->version_id == VERSION_8812E)
				*((volatile unsigned long *)PCI_CONFIG_BASE1) = 0x19000004;
//#endif

            //DEBUG_INFO("...config_base1 = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_BASE1));
            for(i=0; i<1000000; i++);
            *((volatile unsigned char *)PCI_CONFIG_COMMAND) = 0x07;
            //DEBUG_INFO("...command = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_COMMAND));
            for(i=0; i<1000000; i++);
            *((volatile unsigned short *)PCI_CONFIG_LATENCY) = 0x2000;
            for(i=0; i<1000000; i++);
            //DEBUG_INFO("...latency = 0x%08lx\n", *((volatile unsigned long *)PCI_CONFIG_LATENCY));

			if ((GET_CHIP_VER(priv)!=VERSION_8188E) && (GET_CHIP_VER(priv)!=VERSION_8192E) && (GET_CHIP_VER(priv)!=VERSION_8812E) && (GET_CHIP_VER(priv)!=VERSION_8814A) && (GET_CHIP_VER(priv)!=VERSION_8822B))
			{
			}
			REG32(0xB800311C) |=  1 << 23;
	       if (GET_CHIP_VER(priv)==VERSION_8822B) {
	           Wlan_HAL_Link(priv);
	       }
	       else
            {
                check_chipID_MIMO(priv);
                //Exception Case
                if ( check_MAC_IO_Enable(priv) ) {
                    rtl8192cd_stop_hw(priv);
                }
            }
	   }
       else if (TYPE_EMBEDDED == ((priv->pshare->type>>TYPE_SHIFT) & TYPE_MASK) ) {
            DEBUG_INFO("TYPE_EMBEDDED\n");

            if((REG32(0xB8000000)&0xfffff000)== 0x8197F000)
            {
                priv->pshare->version_id = VERSION_8197F;
                DEBUG_INFO("\n found 8197F !!! \n");
                REG32(0xB8000064)|= 0x1F;
            }



            Wlan_HAL_Link(priv);
        }
	}
/*	==========>> maybe later
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
		rtl8192cd_ePhyInit(priv);
*/

	{
		dev->irq = wdev->irq;
	}





register_driver:

	dev->netdev_ops = &rtl8192cd_netdev_ops;



	MACFM_software_init(priv);


	priv->rtk = rtk;
	if(vap_idx < 0) {
		if (IS_ROOT_INTERFACE(priv)) //wrt-vxd
		{
			realtek_cfg80211_init(rtk,priv);
			realtek_interface_add(priv, rtk, rtk->root_ifname, NL80211_IFTYPE_STATION, 0, 0);
		}
		else
		{
			priv->dev = dev;
			memcpy(dev->dev_addr, GET_ROOT(priv)->pmib->dot11Bss.bssid, 6);
			dev->dev_addr[4] = dev->dev_addr[4] + 1;
			memcpy(GET_MY_HWADDR, dev->dev_addr, 6);
			realtek_interface_add(priv, rtk, dev->name, NL80211_IFTYPE_STATION, 0, 0);
		}
	}
	else
	{
		priv->dev = dev;

		if(read_flash_hw_mac_vap(rtk->vap_mac[vap_idx], vap_idx))
		{
			memcpy(dev->dev_addr, GET_ROOT(priv)->pmib->dot11Bss.bssid, 6);
			dev->dev_addr[5] = dev->dev_addr[5] + vap_idx + 1;
		} else {
			memcpy(dev->dev_addr, rtk->vap_mac[vap_idx], 6);
		}
		memcpy(GET_MY_HWADDR, dev->dev_addr, 6);
		realtek_interface_add(priv, rtk, dev->name, NL80211_IFTYPE_AP, 0, 0);
	}


	if (rc)
		goto err_out_iomap;



	if (IS_ROOT_INTERFACE(priv))  // is root interface
		DEBUG_INFO("Init %s, base_addr=%08lx, irq=%d\n",
			dev->name, (unsigned long)dev->base_addr,  dev->irq);



	if (IS_ROOT_INTERFACE(priv))  // is root interface
	{
	}

#ifdef _INCLUDE_PROC_FS_
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


	// set some default value of mib
	set_mib_default(priv);

	if(IS_VXD_INTERFACE(priv))
	{
		OPMODE = WIFI_STATION_STATE;
	}

	nat25_filter_default(priv);
	prepare_iface_nl80211(dev, priv);


	if (IS_ROOT_INTERFACE(priv))  // is root interface
	{







        //3 Require Descriptor Memory
        //Method:
        //  1.) Static Memory
        //  2.) Allocate memory from OS
		if (IS_HAL_CHIP(priv)) {
			{
    		page_ptr = kmalloc(DESC_DMA_PAGE_SIZE_IF(priv), GFP_KERNEL);
            page_ptr_amsdu = kmalloc(DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU, GFP_KERNEL);

            _GET_HAL_DATA(priv)->desc_dma_buf = page_ptr;

		    if (page_ptr == NULL) {
    			rc = -ENOMEM;
    			panic_printk(KERN_ERR "can't allocate descriptior page, abort!\n");
                _GET_HAL_DATA(priv)->desc_dma_buf_len   = 0;
    			goto err_out_dev;
			} else {
                _GET_HAL_DATA(priv)->desc_dma_buf_len   = DESC_DMA_PAGE_SIZE_IF(priv);
            }
            _GET_HAL_DATA(priv)->desc_dma_buf_amsdu = page_ptr_amsdu;
            if (page_ptr_amsdu == NULL) {
                kfree(page_ptr);
                rc = -ENOMEM;
                printk(KERN_ERR "can't allocate AMSDU descriptior page, abort!\n");
                goto err_out_dev;
            } else {
                _GET_HAL_DATA(priv)->desc_dma_buf_len_amsdu = DESC_DMA_PAGE_SIZE_HAL_FOR_AMSDU;
            }
			}
			if (GET_HAL_INTERFACE(priv)->InitHCIDMAMemHandler(priv) == RT_STATUS_FAILURE){
				GET_HAL_INTERFACE(priv)->StopSWHandler(priv);
				rc = -ENOMEM;
				printk(KERN_ERR "can't allocate descriptior page, abort!\n");
				if (page_ptr != NULL){
					kfree(page_ptr);
					kfree(page_ptr_amsdu);
				}
				goto err_out_dev;
			}
		} else
		{
		{
		page_ptr = kmalloc(DESC_DMA_PAGE_SIZE_IF(priv), GFP_KERNEL);
		}

		if (page_ptr == NULL) {
			rc = -ENOMEM;
			panic_printk(KERN_ERR "can't allocate descriptior page, abort!\n");
			goto err_out_dev;
		}

		phw->alloc_dma_buf = (unsigned long)page_ptr;
		page_ptr = (unsigned char *)
			(((unsigned long)page_ptr) + (PAGE_SIZE - (((unsigned long)page_ptr) & (PAGE_SIZE-1))));
		phw->ring_buf_len = phw->alloc_dma_buf + DESC_DMA_PAGE_SIZE_IF(priv) - ((unsigned long)page_ptr);
		phw->ring_dma_addr = virt_to_bus(page_ptr)+CONFIG_LUNA_SLAVE_PHYMEM_OFFSET;


		DEBUG_INFO("page_ptr=%lx, size=%ld\n",  (unsigned long)page_ptr, (unsigned long)DESC_DMA_PAGE_SIZE_IF(priv));
		phw->ring_virt_addr = (unsigned long)page_ptr;
		}






	}

	INIT_LIST_HEAD(&priv->asoc_list); // init assoc_list first because webs may get sta_num even it is not open,
																// and it will cause exception if it is not init, david+2008-03-05


	return 0;

err_out_dev:

	dev = priv->dev;
#ifdef _INCLUDE_PROC_FS_
	rtl8192cd_proc_remove(dev);
#endif
	if (IS_ROOT_INTERFACE(priv))  // is root interface
	{


	}

	unregister_netdev(dev);

err_out_iomap:




err_out_free2:

	if (IS_HAL_CHIP(priv)) {
		HalDisAssociateNic(priv, TRUE);
	}

	if (!IS_ROOT_INTERFACE(priv)) { // is a vxd or vap
		if (vap_idx < 0) // create for vxd
			GET_VXD_PRIV(wdev->priv) = NULL;
		if (vap_idx >= 0) // create for vap
			GET_ROOT(priv)->pvap_priv[vap_idx] = NULL;
	} else
	wdev->priv = NULL;

err_out_free:

	if (pmib){
		kfree(pmib);
	}


	if (pevent_queue)
		kfree(pevent_queue);
	if (phw)
		kfree(phw);
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
	if (pwlan_acl_poll)
		kfree(pwlan_acl_poll);


	if (Eap_packet)
		kfree(Eap_packet);
	if (wpa_global_info)
		kfree(wpa_global_info);
	if (site_survey)
		free_site_survey_res(site_survey);

	free_netdev(dev);

	printk("=====>> EXIT %s (%d) <<=====\n", __func__, rc);

    return rc;
}

void rtl8192cd_init_one_cfg80211(struct rtknl *rtk)
{
	int wlan_index=0;

	printk("roo_ifname = %s, rtk->num_vif=%d \n", rtk->root_ifname, rtk->num_vif);

	if(!strcmp(rtk->root_ifname, "wlan1")) //if it is	second wlan phy
		wlan_index =1;

	rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], rtk->num_vif);

}

void rtl8192cd_deinit_one(struct rtl8192cd_priv *priv)
{
	struct net_device *dev;
	int i;
	struct rtl8192cd_priv *vxd_priv = NULL;
	struct rtl8192cd_priv *vap_priv = NULL;
	if (NULL == priv) {
		panic_printk("%s: priv should NOT be a NULL pointer!\n", __func__);
		return;
	} else {
	}



	vxd_priv = priv->pvxd_priv;
	if (vxd_priv) {
		unregister_netdev(vxd_priv->dev);
#ifdef _INCLUDE_PROC_FS_
		rtl8192cd_proc_remove(vxd_priv->dev);
#endif
		kfree(vxd_priv->pmib);
		kfree(vxd_priv->pevent_queue);
		kfree(vxd_priv->pwlan_acl_poll);
		kfree(vxd_priv->Eap_packet);
		kfree(vxd_priv->wpa_global_info);
		free_site_survey_res(vxd_priv->site_survey);
		free_netdev(vxd_priv->dev);
		priv->pvxd_priv = NULL;
	}

	for (i = 0; i < RTL8192CD_NUM_VWLAN; i++) {
		vap_priv = priv->pvap_priv[i];
		if (vap_priv) {
			unregister_netdev(vap_priv->dev);
#ifdef _INCLUDE_PROC_FS_
			rtl8192cd_proc_remove(vap_priv->dev);
#endif
			kfree(vap_priv->pmib);
			kfree(vap_priv->pevent_queue);
			kfree(vap_priv->pwlan_acl_poll);
			kfree(vap_priv->Eap_packet);
			kfree(vap_priv->wpa_global_info);

            free_site_survey_res(vap_priv->site_survey);

			free_netdev(vap_priv->dev);
			priv->pvap_priv[i] = NULL;
		}
	}

	// root interface clean
	dev = priv->dev;
	unregister_netdev(dev);
#ifdef _INCLUDE_PROC_FS_
	rtl8192cd_proc_remove(dev);
#endif


	if (IS_HAL_CHIP(priv)) {
		{
			if (_GET_HAL_DATA(priv)->desc_dma_buf) {
				kfree(_GET_HAL_DATA(priv)->desc_dma_buf);
				_GET_HAL_DATA(priv)->desc_dma_buf = NULL;
			}
            if (_GET_HAL_DATA(priv)->desc_dma_buf_amsdu) {
                kfree(_GET_HAL_DATA(priv)->desc_dma_buf_amsdu);
                _GET_HAL_DATA(priv)->desc_dma_buf_amsdu = NULL;
			}
		}
		GET_HAL_INTERFACE(priv)->StopSWHandler(priv);
	} else
	{
			kfree((void *)priv->pshare->phw->alloc_dma_buf);
	}



	kfree(priv->pmib);
	kfree(priv->pevent_queue);
	kfree(priv->pwlan_acl_poll);
	kfree(priv->Eap_packet);
	kfree(priv->wpa_global_info);
	free_site_survey_res(priv->site_survey);




	kfree(priv->pshare->pwlan_hdr_poll);
	kfree(priv->pshare->pwlanllc_hdr_poll);
	kfree(priv->pshare->pwlanbuf_poll);
	kfree(priv->pshare->pwlanicv_poll);
	kfree(priv->pshare->pwlanmic_poll);
	kfree(priv->pshare->phw);
	kfree(priv->pshare);	// david


	if (IS_HAL_CHIP(priv)) {
		HalDisAssociateNic(priv, TRUE);
	}

	free_netdev(dev);
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

static const struct net_device_ops rtl8192cd_rtl_pseudodev_ops = {
        .ndo_open               = rtl8192cd_open,
        .ndo_stop               = rtl8192cd_close,
        .ndo_set_mac_address    = rtl_pseudo_dev_set_hwaddr,
        .ndo_get_stats          = rtl8192cd_get_stats,
        .ndo_do_ioctl           = rtl8192cd_ioctl,
        .ndo_start_xmit         = rtl8192cd_start_xmit,
};

void rtl_pseudo_dev_init(void* priv)
{
	struct net_device *dev;

/*	printk("[%s][%d] priv of %s\n", __FUNCTION__, __LINE__, ((struct rtl8192cd_priv*)priv)->dev->name);*/
	dev = alloc_etherdev(0);
	if (dev == NULL) {
		printk("alloc_etherdev() pseudo port5 error!\n");
		return;
	}

	dev->netdev_ops = &rtl8192cd_rtl_pseudodev_ops;
	dev->priv = priv;
	strcpy(dev->name, "port5");
	memcpy((char*)dev->dev_addr,"\x00\xe0\x4c\x81\x86\x86", MACADDRLEN);
	if (register_netdev(dev)) {
		printk(KERN_ERR "register_netdev() wds error!\n");
	}
	rtl865x_wlanIF_Init(dev);
}
#endif







int GetCpuCanSuspend(void)
{
	extern int gCpuCanSuspend;
	return gCpuCanSuspend;
}




int MDL_INIT __rtl8192cd_init(unsigned long base_addr)
{
	int rc;
	int i;


#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif

#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = rtl8651_romeperfEnterPoint;
	Fn_rtl8651_romeperfExitPoint = rtl8651_romeperfExitPoint;
#endif

	panic_printk("%s - version %d.%d (%s)\n", DRV_NAME, DRV_VERSION_H, DRV_VERSION_L, DRV_RELDATE);

	panic_printk("DFS function - version %s\n", get_DFS_version());
	panic_printk("Adaptivity function - version %s\n", Get_Adaptivity_Version());
	for (wlan_index=0; wlan_index<ARRAY_SIZE(wlan_device); wlan_index++)
	{
		_DEBUG_INFO(" wlan_index:%d  %d, %d,0x%lx,0x%lx,%d\n", wlan_index, (((wlan_device[wlan_index].type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS),
			wlan_device[wlan_index].type, wlan_device[wlan_index].base_addr,wlan_device[wlan_index].conf_addr, wlan_device[wlan_index].irq);

		if (((wlan_device[wlan_index].type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
			if (FALSE == drv_registered) {
				drv_registered = TRUE;
			}
		}
		else {
			rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);
			if (rc)
				printk("init_one fail!!! rc=%d\n",rc);

			if (rc == 0)
				rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], -1);
			if (rc == 0) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					rc = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_index], i);
					if (rc != 0) {
						printk("Init fail! rc=%d\n", rc);
						break;
					}
				}
			}
		}

	}



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

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 -----------
	rtl8192cd_chr_reg_hook = rtl8192cd_chr_reg;
	rtl8192cd_chr_unreg_hook = rtl8192cd_chr_unreg;
//------------------------------------
	rtl8192cd_chr_init();
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


	return 0;
}


int MDL_INIT rtl8192cd_init(void)
{

#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	static char initated = 0;
	if (initated == 0)
		initated = 1;
	else
		return 0;
#endif

#ifndef CONFIG_DUAL_CPU_SLAVE
#endif

	return __rtl8192cd_init(0);
}


#if !defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
static
#endif
void MDL_EXIT rtl8192cd_exit (void)
{
	int idx;

#ifdef PERF_DUMP
	Fn_rtl8651_romeperfEnterPoint = NULL;
	Fn_rtl8651_romeperfExitPoint = NULL;
 #endif

#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	extern struct rtl8192cd_priv* (*rtl8192cd_chr_reg_hook)(unsigned int minor, struct rtl8192cd_chr_priv *priv);
	extern void (*rtl8192cd_chr_unreg_hook)(unsigned int minor);
//------------------------------------
#endif


	drv_registered = FALSE;

	for (idx = 0; idx < ARRAY_SIZE(wlan_device) ; idx++) {
		if (NULL == wlan_device[idx].priv)
			continue;
		rtl8192cd_deinit_one(wlan_device[idx].priv);
		rtk_remove_dev(wlan_device[idx].priv->rtk, idx);
		wlan_device[idx].priv = NULL;
	}



	ResetHALIndex();


#if defined(EAP_BY_QUEUE) && defined(USE_CHAR_DEV)
// for module, 2005-12-26 ------------
	rtl8192cd_chr_reg_hook = NULL;
	rtl8192cd_chr_unreg_hook = NULL;
//------------------------------------

	rtl8192cd_chr_exit();
#endif

}


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
						if (priv != priv->pshare->aidarray[i]->priv)
							continue;
						else
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





void update_fwtbl_asoclst(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned char tmpbuf[16];
	int i;

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
				netif_rx(skb);
		}
	}

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

#if defined(CONFIG_RTL_819X) && defined(CONFIG_RTL_819X_SWCORE) && !defined(CONFIG_RTL8196C_KLD) && !defined(__OSK__) && !(defined(CONFIG_RTL8672) && defined(CONFIG_OPENWRT_SDK))
	{
	/* 02-17-2012: move the called function "update_hw_l2table" from Bridge module to here to avoid hacking the Linux kernel or the other kernel */
	extern void update_hw_l2table(const char *srcName,const unsigned char *addr);
	update_hw_l2table("wlan", (const unsigned char *)pstat->hwaddr); /* RTL_WLAN_NAME */
	}
#endif
}


// quick fix for warn reboot fail issue
#define CLK_MANAGE     0xb8000010
void force_stop_wlan_hw(void)
{
	int i=0;
    unsigned int errorFlag;
//	int temp;

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D) || defined(CONFIG_RTL_92D_DMDP)
	for (i=0; i<sizeof(wlan_device)/sizeof(struct _device_info_); i++)
#endif
	{
		if (wlan_device[i].priv) {
			struct rtl8192cd_priv *priv = wlan_device[i].priv;
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
			} else
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
		if (!priv->pshare->switch_chan_rp)
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



MODULE_LICENSE("GPL");
#if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT)
	/* don't init wlan while kernel startup */
#else
module_init(rtl8192cd_init);
module_exit(rtl8192cd_exit);
#endif /* #if defined(CONFIG_RTL_ULINKER_WLAN_DELAY_INIT) */

