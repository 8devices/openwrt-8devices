/*
 *  TX handle routines
 *
 *  $Id: 8192cd_tx.c,v 1.39.2.31 2011/01/19 15:20:34 victoryman Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_TX_C_

#ifdef __KERNEL__
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/tcp.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#ifdef __DRAYTEK_OS__
#include <draytek/wl_dev.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "./sys-support.h"
#endif

#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"
#endif

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#include "wapiCrypto.h"
#endif
#if defined (CONFIG_RTL_VLAN_8021Q) || defined (CONFIG_PUMA_VLAN_8021Q)
#include <linux/if_vlan.h>
#endif

#ifdef CONFIG_RTL867X_VLAN_MAPPING
#include "../../re_vlan.h"
#endif

#ifdef PERF_DUMP
#include "romeperf.h"
#endif

#ifdef PERF_DUMP_1074K
#include "m1074kctrl.c"
#endif

struct rtl_arphdr
{
	//for corss platform
    __be16          ar_hrd;         /* format of hardware address   */
    __be16          ar_pro;         /* format of protocol address   */
    unsigned char   ar_hln;         /* length of hardware address   */
    unsigned char   ar_pln;         /* length of protocol address   */
    __be16          ar_op;          /* ARP opcode (command)         */
};

#define AMSDU_TX_DESC_TH		2	// A-MSDU tx desc threshold, A-MSDU will be
									// triggered when more than this threshold packet in hw queue

#define RET_AGGRE_BYPASS		0
#define RET_AGGRE_ENQUE			1
#define RET_AGGRE_DESC_FULL		2

#define TX_NORMAL				0
#define TX_NO_MUL2UNI			1
#define TX_AMPDU_BUFFER_SIG		2
#define TX_AMPDU_BUFFER_FIRST	3
#define TX_AMPDU_BUFFER_MID		4
#define TX_AMPDU_BUFFER_LAST	5

#if 0//def CONFIG_RTL_STP
extern unsigned char STPmac[6];
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
extern int linux_vlan_enable;
extern linux_vlan_ctl_t *vlan_ctl_p;
#endif

#ifdef RTL_MANUAL_EDCA
unsigned int PRI_TO_QNUM(struct rtl8192cd_priv *priv, int priority)
{
#if defined(SSID_PRIORITY_SUPPORT)
	if(priv->pmib->miscEntry.manual_priority && (OPMODE & WIFI_AP_STATE))
		priority = priv->pmib->miscEntry.manual_priority;
#endif

	if (priv->pmib->dot11QosEntry.ManualEDCA) {
		return priv->pmib->dot11QosEntry.TID_mapping[priority];
	}
	else {
		if ((priority == 0) || (priority == 3)) {
			if (!((OPMODE & WIFI_STATION_STATE) && GET_STA_AC_BE_PARA.ACM))
				return BE_QUEUE;
			else
				return BK_QUEUE;
		} else if ((priority == 7) || (priority == 6)) {
			if (!((OPMODE & WIFI_STATION_STATE) && GET_STA_AC_VO_PARA.ACM)) {
				return VO_QUEUE;
			} else {
				if (!GET_STA_AC_VI_PARA.ACM)
					return VI_QUEUE;
				else if (!GET_STA_AC_BE_PARA.ACM)
					return BE_QUEUE;
				else
					return BK_QUEUE;
			}
		} else if ((priority == 5) || (priority == 4)) {
			if (!((OPMODE & WIFI_STATION_STATE) && GET_STA_AC_VI_PARA.ACM)) {
				return VI_QUEUE;
			} else {
				if (!GET_STA_AC_BE_PARA.ACM)
					return BE_QUEUE;
				else
					return BK_QUEUE;
			}
		} else {
			return BK_QUEUE;
		}
	}
}
#endif

#if defined(CONFIG_RTL8672) && defined(__OSK__)
extern int enable_port_mapping;
extern int enable_IGMP_SNP;
extern int check_IGMP_report(CCB *ccb);
extern int check_wlan_mcast_tx(struct sk_buff *skb, int port);
#ifdef INET6
extern int enable_MLD_SNP;
extern int check_mld_report(CCB *ccb);
extern int check_wlan_ipv6_mcast_tx(struct sk_buff *skb,int port);
#endif

#ifdef CONFIG_RTL_92D_DMDP
extern struct port_map wlanDev[(RTL8192CD_NUM_VWLAN+2)*2];
#else
extern struct port_map wlanDev[RTL8192CD_NUM_VWLAN+2];		// Root(1)+vxd(1)+VAPs(4)
#endif
#elif !defined(CONFIG_RTL_8676HWNAT) && defined(CONFIG_RTL8672) && !defined(CONFIG_RTL8686) && !defined(CONFIG_RTL8685) && !defined(CONFIG_RTL8685S)
extern int check_IGMP_report(struct sk_buff *skb);
extern int check_wlan_mcast_tx(struct sk_buff *skb);
#endif

#define BG_TABLE_SIZE 21
#define MCS_40M_TABLE_SIZE 18
#define MCS_20M_TABLE_SIZE 22

unsigned short BG_TABLE[2][BG_TABLE_SIZE] = {{73,68,63,57,52,47,42,37,31,30,27,25,23,22,20,18,16,14,11,9,8},
				{108,108,108,108,108,108,108,108,108,108,108,108,72,72,48,48,36,12,11,11,4}};
unsigned short MCS_40M_TABLE[2][MCS_40M_TABLE_SIZE] = {{73,68,63,57,53,45,39,37,31,29,28,26,25,23,21,19,17,12},
					{7,7,7,7,7,7,7,7,5,4,4,4,2,2,2,0,0,0}};
unsigned short MCS_20M_TABLE[2][MCS_20M_TABLE_SIZE]={{73,68,65,60,55,50,45,40,31,32,27,26,24,22,21,19,18,16,14,11,9,8},
					{7,7,7,7,7,7,7,7,7,6,5,5,5,4,3,3,3,2,3,1,0,0}};

static int tkip_mic_padding(struct rtl8192cd_priv *priv,
			unsigned char *da, unsigned char *sa, unsigned char priority,
			unsigned char *llc,struct sk_buff *pskb, struct tx_insn* txcfg);

#ifdef CONFIG_PCI_HCI
static int rtl8192cd_tx_queueDsr(struct rtl8192cd_priv *priv, unsigned int txRingIdx);
static void rtl8192cd_tx_restartQueue(struct rtl8192cd_priv *priv);
#endif

#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
int __rtl8192cd_start_xmit(struct sk_buff*skb, struct net_device *dev, int tx_fg);

#ifdef TX_SCATTER
extern struct sk_buff *copy_skb(struct sk_buff *skb);
#endif



unsigned int get_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
    if (pstat->sta_in_firmware == 1) {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
		if (CHIP_VER_92X_SERIES(priv) && (priv->pmib->dot11RFEntry.txbf == 1))
			check_txrate_by_reg(priv, pstat);
#endif		
		return pstat->current_tx_rate;
	} else {
			// firmware does not keep the aid ...
			//use default rate instead
			// eric_8814 ?? VHT rates ?? 1SS ?? 2SS ?? 3SS ??
			if (pstat->ht_cap_len) {	// is N client
				if (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40) {//(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))){ //40Mhz
					int i = 0;
					pstat->current_tx_rate = MCS_40M_TABLE[1][MCS_40M_TABLE_SIZE-1] + HT_RATE_ID;
					for (i=1; i < MCS_40M_TABLE_SIZE; i++) {
						if (pstat->rssi > MCS_40M_TABLE[0][i]){
							pstat->current_tx_rate = MCS_40M_TABLE[1][i-1] + HT_RATE_ID;
							break;
						}
					}
					return pstat->current_tx_rate;
				} else { // 20Mhz
					int i = 0;
					pstat->current_tx_rate = MCS_20M_TABLE[1][MCS_20M_TABLE_SIZE-1] + HT_RATE_ID;
					for (i=1; i < MCS_20M_TABLE_SIZE; i++) {
						if (pstat->rssi > MCS_20M_TABLE[0][i]){
							pstat->current_tx_rate = MCS_20M_TABLE[1][i-1] + HT_RATE_ID;
							break;
						}
					}
					return pstat->current_tx_rate;
				}
				return pstat->current_tx_rate;

			} 
#ifdef RTK_AC_SUPPORT
			else if (pstat->vht_cap_len) {	// is N client
				if (pstat->tx_bw == HT_CHANNEL_WIDTH_80) {//(pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SUPPORT_CH_WDTH_))){ //40Mhz
					int i = 0;
					pstat->current_tx_rate = MCS_40M_TABLE[1][MCS_40M_TABLE_SIZE-1] + VHT_RATE_ID;
					for (i=1; i < MCS_40M_TABLE_SIZE; i++) {
						if (pstat->rssi > MCS_40M_TABLE[0][i]){
							pstat->current_tx_rate = MCS_40M_TABLE[1][i-1] + VHT_RATE_ID;
							break;
						}
					}
					return pstat->current_tx_rate;
				} else { // 20Mhz
					int i = 0;
					pstat->current_tx_rate = MCS_20M_TABLE[1][MCS_20M_TABLE_SIZE-1] + VHT_RATE_ID;
					for (i=1; i < MCS_20M_TABLE_SIZE; i++) {
						if (pstat->rssi > MCS_20M_TABLE[0][i]){
							pstat->current_tx_rate = MCS_20M_TABLE[1][i-1] + VHT_RATE_ID;
							break;
						}
					}
					return pstat->current_tx_rate;
				}
				return pstat->current_tx_rate;

			} 
#endif
			else { // is BG client
				int i = 0;
				pstat->current_tx_rate = BG_TABLE[1][BG_TABLE_SIZE-1]; // eric_8814 ?? conflict with HT RATES ??
				for (i = 0; i < BG_TABLE_SIZE; i++) {
					if (pstat->rssi > BG_TABLE[0][i]){
						pstat->current_tx_rate = BG_TABLE[1][i-1];
						break;
				    }
				}
				return pstat->current_tx_rate;
			}

	}

}


unsigned int get_lowest_tx_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat,
				unsigned int tx_rate)
{
	unsigned int lowest_tx_rate;

	if (is_auto_rate(priv , pstat))
	{
			lowest_tx_rate = find_rate(priv, pstat, 0, 0);
	}
	else
		lowest_tx_rate = tx_rate;

	return lowest_tx_rate;
}

#ifdef __OSK__
__IRAM_WIFI_PRI5
#else
__MIPS16 __IRAM_IN_865X 
#endif
void assign_wlanseq(struct rtl8192cd_hw *phw, unsigned char *pframe, struct stat_info *pstat, struct wifi_mib *pmib
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
	, unsigned char is_11s
#endif
	)
{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	u16 seq;
#endif

#ifdef WIFI_WMM
	unsigned char qosControl[2];
	int tid;

	if (is_qos_data(pframe)) {
		memcpy(qosControl, GetQosControl(pframe), 2);
		tid = qosControl[0] & 0x07;

		if (pstat) {
			SetSeqNum(pframe, pstat->AC_seq[tid]);
			pstat->AC_seq[tid] = (pstat->AC_seq[tid] + 1) & 0xfff;
		}
		else {
//			SetSeqNum(pframe, phw->AC_seq[tid]);
//			phw->AC_seq[tid] = (phw->AC_seq[tid] + 1) & 0xfff;
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
			if (is_11s)
			{
				SetSeqNum(pframe, phw->seq);
				phw->seq = (phw->seq + 1) & 0xfff;
			}
			else
#endif
				printk("Invalid seq num setting for Multicast or Broadcast pkt!!\n");
		}

		{
			if ((tid == 7) || (tid == 6))
				phw->VO_pkt_count++;
			else if ((tid == 5) || (tid == 4))
				phw->VI_pkt_count++;
			else if ((tid == 2) || (tid == 1))
				phw->BK_pkt_count++;
#ifdef WMM_VIBE_PRI
			else
				phw->BE_pkt_count++;
#endif
		}
	}
	else
#endif
	{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		seq = atomic_inc_return(&phw->seq);
		seq = seq & 0x0fff;
		SetSeqNum(pframe, seq);
#elif defined(CONFIG_PCI_HCI)
		SetSeqNum(pframe, phw->seq);
		phw->seq = (phw->seq + 1) & 0xfff;
#endif
	}
}


#ifdef CONFIG_RTK_MESH
static unsigned int get_skb_priority3(struct rtl8192cd_priv *priv, struct sk_buff *skb, int is_11s, struct stat_info *pstat)
#else
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X unsigned int get_skb_priority(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat)
#endif
{
	unsigned int pri=0, parsing=0;
	unsigned char protocol[2];

#ifdef WIFI_WMM
    if (QOS_ENABLE) {
        parsing = 1;
#ifdef CONFIG_PCI_HCI
        if(pstat && !pstat->QosEnabled)
            parsing = 0;
#endif
    }
#endif

	if (parsing) {		
#if defined(SSID_PRIORITY_SUPPORT)
		if(priv->pmib->miscEntry.manual_priority && (OPMODE & WIFI_AP_STATE))
			pri = priv->pmib->miscEntry.manual_priority;
		else
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT) ||defined(CONFIG_RTL_VLAN_SUPPORT) 
		if (skb->cb[0])
			pri =  skb->cb[0];
		else
#endif
		{
			protocol[0] = skb->data[12];
			protocol[1] = skb->data[13];

			if ((protocol[0] == 0x08) && (protocol[1] == 0x00))
			{
#ifdef CONFIG_RTK_MESH
				if(is_11s & RELAY_11S)
				{
					pri = (skb->data[31] & 0xe0) >> 5;
				}
				else
#endif
#ifdef HS2_SUPPORT
				if(priv->pmib->hs2Entry.QoSMap_ielen[priv->pmib->hs2Entry.curQoSMap]!=0) {
					pri=getDSCP2UP(priv, (skb->data[15] & 0xFC) >> 2);
				}
				else
#endif
				{
#if defined(CONFIG_SDIO_HCI) && defined(TX_SCATTER)
					pri = (get_skb_data_u8(skb, 15) & 0xe0) >> 5;
#else
					pri = (skb->data[15] & 0xe0) >> 5;
#endif
				}
			}
#ifndef NOT_RTK_BSP
			else if ((skb->cb[0]>0) && (skb->cb[0]<8))	// Ethernet driver will parse priority and put in cb[0]
				pri = skb->cb[0];
#endif
			else
				pri = 0;
		}

#ifdef CLIENT_MODE
		if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE)) {
			if (GET_STA_AC_VO_PARA.ACM) {
				if (!GET_STA_AC_VI_PARA.ACM) 
					pri = 5;
				else if (!GET_STA_AC_BE_PARA.ACM)
					pri = 0;
				else
					pri = 1;
			} else if (GET_STA_AC_VI_PARA.ACM) {
				if (!GET_STA_AC_BE_PARA.ACM)
					pri = 0;
				else
					pri = 1;
			} else if (GET_STA_AC_BE_PARA.ACM) {
				pri = 1;	// DSCP_BK tag = 1;
			}
		}
#endif	
		skb->cb[1] = pri;

		return pri;
	}
	else {
		// default is no priority
		skb->cb[1] = 0;
		return 0;
	}
}


#ifdef CONFIG_RTK_MESH
#define get_skb_priority(priv, skb, pstat)  get_skb_priority3(priv, skb, 0, pstat)
#endif

#ifdef CONFIG_PCI_HCI
static int dz_queue(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct sk_buff *pskb)
{
	unsigned int ret;

	if (pstat)
	{
		if(0 == pstat->expire_to)
			return FALSE;
#if defined(WIFI_WMM) && defined(WMM_APSD)
		if ((QOS_ENABLE) && (APSD_ENABLE) && (pstat->QosEnabled) && (pstat->apsd_bitmap & 0x0f)) {
			int pri = 0;

			pri = get_skb_priority(priv, pskb, pstat);

			if (((pri == 7) || (pri == 6)) && (pstat->apsd_bitmap & 0x01)) {
				ret = enque(priv, &(pstat->VO_dz_queue->head), &(pstat->VO_dz_queue->tail),
					(unsigned long)(pstat->VO_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE, (void *)pskb);
				if (ret)
					DEBUG_INFO("enque VO pkt\n");
			}
			else if (((pri == 5) || (pri == 4)) && (pstat->apsd_bitmap & 0x02)) {
				ret = enque(priv, &(pstat->VI_dz_queue->head), &(pstat->VI_dz_queue->tail),
					(unsigned long)(pstat->VI_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE, (void *)pskb);
				if (ret)
					DEBUG_INFO("enque VI pkt\n");
			}
			else if (((pri == 0) || (pri == 3)) && (pstat->apsd_bitmap & 0x08)) {
				ret = enque(priv, &(pstat->BE_dz_queue->head), &(pstat->BE_dz_queue->tail),
					(unsigned long)(pstat->BE_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE, (void *)pskb);
				if (ret)
					DEBUG_INFO("enque BE pkt\n");
			}
			else if (pstat->apsd_bitmap & 0x04) {
				ret = enque(priv, &(pstat->BK_dz_queue->head), &(pstat->BK_dz_queue->tail),
					(unsigned long)(pstat->BK_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE, (void *)pskb);
				if (ret)
					DEBUG_INFO("enque BK pkt\n");
			}
			else
				goto legacy_ps;

			if (!pstat->apsd_pkt_buffering)
				pstat->apsd_pkt_buffering = 1;

			if (ret == FALSE) {
				DEBUG_ERR("sleep Q full for priority = %d!\n", pri);
				return CONGESTED;
			}
			return TRUE;
		}
		else
legacy_ps:
#endif
		if (pstat->dz_queue.qlen<NUM_TXPKT_QUEUE){
			skb_queue_tail(&pstat->dz_queue, pskb);
			return TRUE;
		}
	}
	else {	// Multicast or Broadcast
		ret = enque(priv, &(priv->dz_queue.head), &(priv->dz_queue.tail),
			(unsigned long)(priv->dz_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
		if (ret == TRUE) {
		        if (!priv->pkt_in_dtimQ)
			        priv->pkt_in_dtimQ = 1;
		        return TRUE;
                }
	}

	return FALSE;
}
#endif // CONFIG_PCI_HCI


/*        Function to process different situations in TX flow             */
/* ====================================================================== */
#define TX_PROCEDURE_CTRL_STOP			0
#define TX_PROCEDURE_CTRL_CONTINUE		1
#define TX_PROCEDURE_CTRL_SUCCESS		2

#ifdef WDS
static int rtl8192cd_tx_wdsDevProc(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device **dev_p,
				struct net_device **wdsDev_p, struct tx_insn *txcfg)
{
	struct stat_info *pstat;

	txcfg->wdsIdx = getWdsIdxByDev(priv, *dev_p);
	if (txcfg->wdsIdx < 0) {
		priv->ext_stats.tx_drops++;
		DEBUG_ERR("TX DROP: getWdsIdxByDev() fail!\n");
		goto free_and_stop;
	}

	if (!netif_running(priv->dev)) {
		priv->ext_stats.tx_drops++;
		DEBUG_ERR("TX DROP: Can't send WDS packet due to wlan interface is down!\n");
		goto free_and_stop;
	}
	pstat = get_stainfo(priv, priv->pmib->dot11WdsInfo.entry[txcfg->wdsIdx].macAddr);
	if (NULL == pstat) {
		priv->ext_stats.tx_drops++;
		DEBUG_ERR("TX DROP: %s: get_stainfo() for wds failed [%d]!\n", (char *)__FUNCTION__, txcfg->wdsIdx);
		goto free_and_stop;
	}
	if (pstat->current_tx_rate==0) {
		priv->ext_stats.tx_drops++;
		DEBUG_ERR("TX DROP: Can't send packet due to tx rate is not supported in peer WDS AP!\n");
		goto free_and_stop;
	}
	*wdsDev_p = *dev_p;
	*dev_p = priv->dev;

	/* Reply caller function : Continue process */
	return TX_PROCEDURE_CTRL_CONTINUE;

free_and_stop:		/* Free current packet and stop TX process */

	rtl_kfree_skb(priv,skb,_SKB_TX_);

	/* Reply caller function : STOP process */
	return TX_PROCEDURE_CTRL_STOP;
}
#endif


#ifdef CLIENT_MODE
static int rtl8192cd_tx_clientMode(struct rtl8192cd_priv *priv, struct sk_buff **pskb)
{
	struct sk_buff *skb=*pskb;
	int DontEnterNat25=0;

	{
#ifdef RTK_BR_EXT
		int res, is_vlan_tag=0, i, do_nat25=1;
		unsigned short vlan_hdr=0;
		int lltd_flag=0;

		if (!priv->pmib->wscEntry.wsc_enable)
#ifdef MULTI_MAC_CLONE			
			if (mac_clone_handle_frame(priv, skb)) {
				/*if STAs number exceed max macclone support number then let it run nat25 path*/
                ACTIVE_ID = 0;
				//priv->ext_stats.tx_drops++;
				//DEBUG_ERR("TX DROP: exceed max clone address!\n");
				//goto free_and_stop;
			}
#else
			mac_clone_handle_frame(priv, skb);
		if(priv->pmib->ethBrExtInfo.macclone_enable && priv->macclone_completed){
			if(!memcmp(skb->data+ETH_ALEN, GET_MY_HWADDR, ETH_ALEN))	{
				DontEnterNat25=1;
			}
		}
#endif
		if ((!priv->pmib->ethBrExtInfo.nat25_disable && DontEnterNat25==0) 
#ifdef MULTI_MAC_CLONE
			&& ((ACTIVE_ID == 0) || (ACTIVE_ID > 0 && priv->pshare->mclone_sta[ACTIVE_ID-1].usedStaAddrId != 0xff))//(ACTIVE_ID == 0)//change for wifi-sta
#endif		
			)
		{
			if (*((unsigned short *)(skb->data+MACADDRLEN*2)) == __constant_htons(ETH_P_8021Q)) {
				is_vlan_tag = 1;
				vlan_hdr = *((unsigned short *)(skb->data+MACADDRLEN*2+2));
				for (i=0; i<6; i++)
					*((unsigned short *)(skb->data+MACADDRLEN*2+2-i*2)) = *((unsigned short *)(skb->data+MACADDRLEN*2-2-i*2));
				skb_pull(skb, 4);
			}

            if(priv->nat25_filter) {
                if(nat25_filter(priv, skb)== 1) {
                    priv->ext_stats.tx_drops++;
                    DEBUG_ERR("TX DROP: nat25 filter out!\n");
                    goto free_and_stop;
                }
            }


			if ((*((unsigned short *)(skb->data+MACADDRLEN*2)) == __constant_htons(ETH_P_IP)) && !IS_MCAST(skb->data)) {
				if (memcmp(priv->scdb_mac, skb->data+MACADDRLEN, MACADDRLEN)) {
					if ((priv->scdb_entry = (struct nat25_network_db_entry *)scdb_findEntry(priv,
								skb->data+MACADDRLEN, skb->data+WLAN_ETHHDR_LEN+12)) != NULL) {
						memcpy(priv->scdb_mac, skb->data+MACADDRLEN, MACADDRLEN);
						memcpy(priv->scdb_ip, skb->data+WLAN_ETHHDR_LEN+12, 4);
						priv->scdb_entry->ageing_timer = jiffies;
						do_nat25 = 0;
					}
				}
				else {
					if (priv->scdb_entry) {
						priv->scdb_entry->ageing_timer = jiffies;
						do_nat25 = 0;
					}
					else {
						memset(priv->scdb_mac, 0, MACADDRLEN);
						memset(priv->scdb_ip, 0, 4);
					}
				}
			}

			if (*((unsigned short *)(skb->data+MACADDRLEN*2)) == __constant_htons(0x88d9)) {
				if(skb->data[0] & 0x1)
				{
					do_nat25=0;
					lltd_flag=1;
				}
			}

			if (do_nat25)
			{
				if (nat25_db_handle(priv, skb, NAT25_CHECK) == 0) {
					struct sk_buff *newskb;

					if (is_vlan_tag) {
						skb_push(skb, 4);
						for (i=0; i<6; i++)
							*((unsigned short *)(skb->data+i*2)) = *((unsigned short *)(skb->data+4+i*2));
						*((unsigned short *)(skb->data+MACADDRLEN*2)) = __constant_htons(ETH_P_8021Q);
						*((unsigned short *)(skb->data+MACADDRLEN*2+2)) = vlan_hdr;
					}

					newskb = skb_copy(skb, GFP_ATOMIC);
					if (newskb == NULL) {
						priv->ext_stats.tx_drops++;
						DEBUG_ERR("TX DROP: skb_copy fail!\n");
						goto free_and_stop;
					}
					dev_kfree_skb_any(skb);
					*pskb = skb = newskb;
					if (is_vlan_tag) {
						vlan_hdr = *((unsigned short *)(skb->data+MACADDRLEN*2+2));
						for (i=0; i<6; i++)
							*((unsigned short *)(skb->data+MACADDRLEN*2+2-i*2)) = *((unsigned short *)(skb->data+MACADDRLEN*2-2-i*2));
						skb_pull(skb, 4);
					}
				}

				res = nat25_db_handle(priv, skb, NAT25_INSERT);
				if (res < 0) {
					if (res == -2) {
						priv->ext_stats.tx_drops++;
						DEBUG_ERR("TX DROP: nat25_db_handle fail!\n");
						goto free_and_stop;
					}
					// we just print warning message and let it go
					DEBUG_WARN("nat25_db_handle INSERT fail!\n");
				}
			}

			if(lltd_flag != 1)
			{
				memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
			}

			if (is_vlan_tag) {
				skb_push(skb, 4);
				for (i=0; i<6; i++)
					*((unsigned short *)(skb->data+i*2)) = *((unsigned short *)(skb->data+4+i*2));
				*((unsigned short *)(skb->data+MACADDRLEN*2)) = __constant_htons(ETH_P_8021Q);
				*((unsigned short *)(skb->data+MACADDRLEN*2+2)) = vlan_hdr;
			}
		}
		else{

            /*even nat25 is disabled, we still do nat25 for bridge's ip/mac*/
            if (!memcmp(skb->data+MACADDRLEN, priv->br_mac, MACADDRLEN)) {
                unsigned short ethtype = *((unsigned short *)(skb->data+MACADDRLEN*2));
                i = 0;
                if(ethtype == __constant_htons(ETH_P_IP)) {
                    i = 12;
                }
                else if(ethtype == __constant_htons(ETH_P_ARP)) {
                    i= 14;
                }

                if(i) {                           
                    memcpy(priv->br_ip, skb->data+WLAN_ETHHDR_LEN+i, 4);                    
                    res = nat25_db_handle(priv, skb, NAT25_INSERT);
                    if (res < 0) {
                        if (res == -2) {
                            priv->ext_stats.tx_drops++;
                            DEBUG_ERR("TX DROP: nat25_db_handle fail!\n");
                            goto free_and_stop;
                        }
                        // we just print warning message and let it go
                        DEBUG_WARN("nat25_db_handle INSERT fail!\n");
                    }    
                    memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
                }
            }

#ifdef TX_SUPPORT_IPV6_MCAST2UNI            
			if (*((unsigned short *)(skb->data+MACADDRLEN*2)) == __constant_htons(ETH_P_8021Q)) {
				is_vlan_tag = 1;
			}

			if(is_vlan_tag){
				if(ICMPV6_MCAST_MAC(skb->data) && ICMPV6_PROTO1A_VALN(skb->data)){
					memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
				}
			}else
			{
				if(ICMPV6_MCAST_MAC(skb->data) && ICMPV6_PROTO1A(skb->data)){
					memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
				}
			}
#endif            
		}



		// check if SA is equal to our MAC
		if (memcmp(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN)) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: untransformed frame SA:%02X%02X%02X%02X%02X%02X!\n",
				skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11]);
			goto free_and_stop;
		}
#endif // RTK_BR_EXT
	}

	/* Reply caller function : Continue process */
	return TX_PROCEDURE_CTRL_CONTINUE;

#ifdef RTK_BR_EXT
free_and_stop:		/* Free current packet and stop TX process */

	rtl_kfree_skb(priv, skb, _SKB_TX_);

//stop_proc:
	/* Reply caller function : STOP process */
	return TX_PROCEDURE_CTRL_STOP;
#endif

}
#endif // CLIENT_MODE


#ifdef GBWC
static int rtl8192cd_tx_gbwc(struct rtl8192cd_priv *priv, struct stat_info	*pstat, struct sk_buff *skb)
{
	if (((priv->pmib->gbwcEntry.GBWCMode == GBWC_MODE_LIMIT_MAC_INNER) && (pstat->GBWC_in_group)) ||
		((priv->pmib->gbwcEntry.GBWCMode == GBWC_MODE_LIMIT_MAC_OUTTER) && !(pstat->GBWC_in_group)) ||
		(priv->pmib->gbwcEntry.GBWCMode == GBWC_MODE_LIMIT_IF_TX) ||
		(priv->pmib->gbwcEntry.GBWCMode == GBWC_MODE_LIMIT_IF_TRX)) {
		if ((priv->GBWC_tx_count + skb->len) > ((priv->pmib->gbwcEntry.GBWCThrd_tx * 1024 / 8) / (HZ / GBWC_TO))) {
			// over the bandwidth
			if (priv->GBWC_consuming_Q) {
				// in rtl8192cd_GBWC_timer context
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: BWC bandwidth over!\n");
				rtl_kfree_skb(priv, skb, _SKB_TX_);
			}
			else {
				// normal Tx path
				int ret = enque(priv, &(priv->GBWC_tx_queue.head), &(priv->GBWC_tx_queue.tail),
						(unsigned long)(priv->GBWC_tx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)skb);
				if (ret == FALSE) {
					priv->ext_stats.tx_drops++;
					DEBUG_ERR("TX DROP: BWC tx queue full!\n");
					rtl_kfree_skb(priv, skb, _SKB_TX_);
				}
			}
			goto stop_proc;
		}
		else {
			// not over the bandwidth
			if (CIRC_CNT(priv->GBWC_tx_queue.head, priv->GBWC_tx_queue.tail, NUM_TXPKT_QUEUE) &&
					!priv->GBWC_consuming_Q) {
				// there are already packets in queue, put in queue too for order
				int ret = enque(priv, &(priv->GBWC_tx_queue.head), &(priv->GBWC_tx_queue.tail),
						(unsigned long)(priv->GBWC_tx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)skb);
				if (ret == FALSE) {
					priv->ext_stats.tx_drops++;
					DEBUG_ERR("TX DROP: BWC tx queue full!\n");
					rtl_kfree_skb(priv, skb, _SKB_TX_);
				}
				goto stop_proc;
			}
			else {
				// can transmit directly
				priv->GBWC_tx_count += skb->len;
			}
		}
	}

	/* Reply caller function : Continue process */
	return TX_PROCEDURE_CTRL_CONTINUE;

stop_proc:
	/* Reply caller function : STOP process */
	return TX_PROCEDURE_CTRL_STOP;
}
#endif

#ifdef SBWC
static int rtl8192cd_tx_sbwc(struct rtl8192cd_priv *priv, struct stat_info	*pstat, struct sk_buff *skb)
{
	SBWC_MODE mode = pstat->SBWC_mode;
	unsigned int tx_count, tx_limit;
	unsigned char is_consuming_q = pstat->SBWC_consuming_q;
	int *head, *tail;
	struct sk_buff *pSkb;
	BOOLEAN ret;

	if (!(OPMODE & WIFI_AP_STATE))
		return TX_PROCEDURE_CTRL_CONTINUE;

	if (!(mode & SBWC_MODE_LIMIT_STA_TX))
		return TX_PROCEDURE_CTRL_CONTINUE;

	tx_count = (pstat->SBWC_tx_count + skb->len);
	tx_limit = pstat->SBWC_tx_limit_byte;

	if (tx_count > tx_limit)
	{
		// over the bandwidth
		if (is_consuming_q)
		{
			// in rtl8192cd_SBWC_timer context
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: SBWC bandwidth over!\n");
			rtl_kfree_skb(priv, skb, _SKB_TX_);
		}
		else
		{
			ret = enque(priv, &(pstat->SBWC_txq.head), &(pstat->SBWC_txq.tail),
						(unsigned long)(pstat->SBWC_txq.pSkb), NUM_TXPKT_QUEUE, (void *)skb);
			if (ret == FALSE) 
			{
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: SBWC tx queue full!\n");
				rtl_kfree_skb(priv, skb, _SKB_TX_);
			}
		}
		goto stop_proc;
	}
	else
	{
		// not over the bandwidth
		if (CIRC_CNT(pstat->SBWC_txq.head, pstat->SBWC_txq.tail, NUM_TXPKT_QUEUE) && !is_consuming_q) 
		{
			// there are already packets in queue, put in queue too for order
			ret = enque(priv, &(pstat->SBWC_txq.head), &(pstat->SBWC_txq.tail),
						(unsigned long)(pstat->SBWC_txq.pSkb), NUM_TXPKT_QUEUE, (void *)skb);

			if (ret == FALSE) 
			{
				priv->ext_stats.tx_drops++;
				DEBUG_WARN("TX DROP: SBWC tx queue full!\n");
				rtl_kfree_skb(priv, skb, _SKB_TX_);
			}
			goto stop_proc;
		}
		else 
		{
			// can transmit directly
			pstat->SBWC_tx_count = tx_count;
		}
	}

	/* Reply caller function : Continue process */
	return TX_PROCEDURE_CTRL_CONTINUE;
	
stop_proc:
	/* Reply caller function : STOP process */
	return TX_PROCEDURE_CTRL_STOP;
}
#endif

#ifdef THERMAL_CONTROL
#ifdef RTK_STA_BWC
static int rtl8192cd_tx_sta_bwc(struct rtl8192cd_priv *priv, struct stat_info	*pstat, struct sk_buff *skb)
{
	int TP_thd = (((pstat->sta_bwcthrd_tx*1024)/8)/1000)*priv->pshare->rf_ft_var.sta_bwc_to;
//	if(priv->pshare->rf_ft_var.swq_dbg == 200)
//		TP_thd = (((pstat->sta_bwcthrd_tx*1024)/8)/1000)*priv->pshare->rf_ft_var.sta_bwc_to;

	//current bps > bwc bps thrd	
	if ((pstat->sta_bwctx_cnt+skb->len) > TP_thd) 
	{
		// over the bandwidth
		// drop packet
		priv->ext_stats.tx_drops++;
		pstat->sta_bwcdrop_cnt++;
		DEBUG_ERR("TX DROP: BWC bandwidth over!\n");
		rtl_kfree_skb(priv, skb, _SKB_TX_);
		goto stop_proc;
	}
	else 
	{
		// can transmit directly
		pstat->sta_bwctx_cnt += skb->len;
	}
	/* Reply caller function : Continue process */
	return TX_PROCEDURE_CTRL_CONTINUE;

stop_proc:
	/* Reply caller function : STOP process */
	return TX_PROCEDURE_CTRL_STOP;
}
#endif

#endif


#ifdef TX_SHORTCUT
static int rtl8192cd_tx_tkip(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info*pstat, struct tx_insn *txcfg)
{
    struct wlan_ethhdr_t *pethhdr;
    struct llc_snap	*pllc_snap = NULL;
    unsigned char * da;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
   if(txcfg->aggre_en >= FG_AGGRE_MSDU_FIRST)
   	pethhdr = (struct wlan_ethhdr_t *)(skb->data);
   else
#endif
    pethhdr = (struct wlan_ethhdr_t *)(skb->data - WLAN_ETHHDR_LEN);

    da = pethhdr->daddr;
#ifdef MCAST2UI_REFINE
    memcpy(pethhdr->daddr, &skb->cb[10], 6);
#endif

#ifdef A4_STA
    if(pstat && (pstat->state & WIFI_A4_STA)) {
        da = GetAddr3Ptr(txcfg->phdr);
    }
#endif

    if(txcfg->llc) {
        pllc_snap = (struct llc_snap *)((UINT8 *)(txcfg->phdr) + txcfg->hdr_len + txcfg->iv);
    }
    
#ifdef WIFI_WMM
    if ((tkip_mic_padding(priv, da, pethhdr->saddr, ((QOS_ENABLE) && (pstat) && (pstat->QosEnabled))?skb->cb[1]:0, (UINT8 *)pllc_snap,
            skb, txcfg)) == FALSE)
#else
    if ((tkip_mic_padding(priv, da, pethhdr->saddr, 0, (UINT8 *)pllc_snap,
            skb, txcfg)) == FALSE)
#endif
    {
        priv->ext_stats.tx_drops++;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
	if(txcfg->aggre_en == FG_AGGRE_MSDU_LAST)
		printk("%s(%d) impossible failed on AMSDU packets\n", __func__, __LINE__);
#endif
        DEBUG_ERR("TX DROP: Tkip mic padding fail!\n");
        rtl_kfree_skb(priv, skb, _SKB_TX_);
        release_wlanllchdr_to_poll(priv, txcfg->phdr);
        goto stop_proc;
    }
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
	if ((txcfg->aggre_en < FG_AGGRE_MSDU_FIRST) || (txcfg->aggre_en == FG_AGGRE_MSDU_LAST))
#endif
	{
    skb_put((struct sk_buff *)txcfg->pframe, 8);
    txcfg->fr_len += 8;	// for Michael padding.
	}

    /* Reply caller function : Continue process */
    return TX_PROCEDURE_CTRL_CONTINUE;

stop_proc:
    /* Reply caller function : STOP process */
    return TX_PROCEDURE_CTRL_STOP;
}


#ifdef CONFIG_PCI_HCI
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
#if defined(CONFIG_WLAN_HAL_8197F) 
static inline int get_tx_sc_index(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *hdr)
#else
int get_tx_sc_index(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *hdr)
#endif
{
    struct tx_sc_entry *ptxsc_entry;
    int i;
    int idx = pstat->tx_sc_idx_cache;

    ptxsc_entry = pstat->tx_sc_ent;

#ifdef MCAST2UI_REFINE          
    if  ((OPMODE & WIFI_AP_STATE)
        #ifdef WDS			
        && !(pstat->state & WIFI_WDS)
        #endif				
    ) 
    {	    
        for (i=0; i<TX_SC_ENTRY_NUM; i++, idx = ((idx+1) % TX_SC_ENTRY_NUM)) {        
            if (!rtk_memcmp(hdr+6, (unsigned char *)(&ptxsc_entry[idx].ethhdr.saddr), sizeof(struct wlan_ethhdr_t)-6)) {
                pstat->tx_sc_idx_cache = idx;
                return idx;                         
            }
        }
    }
    else 
#endif
    {
        for (i=0; i<TX_SC_ENTRY_NUM; i++, idx = ((idx+1) % TX_SC_ENTRY_NUM)) {
            if (!rtk_memcmp(hdr, (unsigned char *)(&ptxsc_entry[idx].ethhdr), sizeof(struct wlan_ethhdr_t))) {
                pstat->tx_sc_idx_cache = idx;
                return idx;
            }
        }

    }
    return -1;
}


#ifdef CONFIG_RTL8672
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
#endif

#if defined(CONFIG_WLAN_HAL_8197F) 
static inline int get_tx_sc_free_entry(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *hdr)
#else
int get_tx_sc_free_entry(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *hdr)
#endif
{
	struct tx_sc_entry *ptxsc_entry;
	int i;

	i = get_tx_sc_index(priv, pstat, hdr);
	if (i >= 0)
		return i;
	
	ptxsc_entry = pstat->tx_sc_ent;
	
	for (i=0; i<TX_SC_ENTRY_NUM; i++) {
		if (ptxsc_entry[i].txcfg.fr_len == 0)
			return i;
	}
	
	// no free entry
	i = pstat->tx_sc_replace_idx;
	pstat->tx_sc_replace_idx = (++pstat->tx_sc_replace_idx) % TX_SC_ENTRY_NUM;
	return i;
}
#endif // CONFIG_PCI_HCI
#endif // TX_SHORTCUT


/*
	Always STOP process after calling this Procedure.
*/
#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static void rtl8192cd_tx_xmitSkbFail(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device *dev,
				struct net_device *wdsDev, struct tx_insn *txcfg)
{
/*
 * Comment-out the following lines to disable flow-control in any case to fix
 * WDS interface may be blocked sometimes during root interface is up/down
 * continously.
 */
#if 0
#ifdef WDS
	if (wdsDev)
		netif_stop_queue(wdsDev);
	else
#endif
	{
#ifdef WIFI_WMM
		if (!QOS_ENABLE)
#endif
			netif_stop_queue(dev);
	}
#endif

#if 1
if(txcfg->pstat)
{
	int q_num= txcfg->q_num;
		if(q_num == VI_QUEUE) 
			priv->pshare->phw->VI_droppkt_count++;
		else if(q_num == VO_QUEUE) 
			priv->pshare->phw->VO_droppkt_count++;
		else if(q_num == BK_QUEUE) 
			priv->pshare->phw->BK_droppkt_count++;
		else if(q_num == BE_QUEUE) 
			priv->pshare->phw->BE_droppkt_count++;
}
#endif
	priv->ext_stats.tx_drops++;
	DEBUG_WARN("TX DROP: Congested!\n");
	if (txcfg->phdr)
		release_wlanllchdr_to_poll(priv, txcfg->phdr);
	if (skb)
		rtl_kfree_skb(priv, skb, _SKB_TX_);

	return;
}


int rtl8192cd_tx_slowPath(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat,
				struct net_device *dev, struct net_device *wdsDev, struct tx_insn *txcfg)
{


#ifdef A4_STA
    int use4Addr = 0;
#endif

    //SMP_LOCK_XMIT(flags);
#ifdef CONFIG_RTL_WAPI_SUPPORT
    if ((pstat && pstat->wapiInfo
        && (pstat->wapiInfo->wapiType!=wapiDisable)
        && skb->protocol != __constant_htons(ETH_P_WAPI)
        && (!pstat->wapiInfo->wapiUCastTxEnable))
    )
    {
        rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
        goto stop_proc;
    }
#endif

    if (IEEE8021X_FUN && pstat && (pstat->ieee8021x_ctrlport == DOT11_PortStatus_Unauthorized) &&
        #ifdef WDS
        (! (pstat->state & WIFI_WDS)) &&
        #endif
        #ifdef CONFIG_RTK_MESH
        (txcfg->is_11s == 0) && /* is not mesh packet*/
        #endif
        (*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2)) != htons(0x888e)) &&
        (!(OPMODE & WIFI_ADHOC_STATE))
    )
    {
        //DEBUG_ERR("TX DROP: control port not authorized!\n");
        rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
        goto stop_proc;
    }

    // Note: Don't change txcfg->q_num for USB. If do it, you may hold two ownerships, but only release one.
#ifdef CONFIG_PCI_HCI
    if (txcfg->aggre_en < FG_AGGRE_MSDU_FIRST) {
        txcfg->q_num = BE_QUEUE;	// using low queue for data queue        
#ifdef SSID_PRIORITY_SUPPORT
		if(priv->pmib->miscEntry.manual_priority && (OPMODE & WIFI_AP_STATE))
			skb->cb[1] = priv->pmib->miscEntry.manual_priority;
		else
#endif
       		skb->cb[1] = 0;
    }
#endif
    txcfg->fr_type = _SKB_FRAME_TYPE_;
    txcfg->pframe = skb;

    if ((txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE) || (txcfg->aggre_en == FG_AGGRE_MSDU_LAST))
        txcfg->phdr = NULL;
    else
    {
        txcfg->phdr = (UINT8 *)get_wlanllchdr_from_poll(priv);

        if (txcfg->phdr == NULL) {
            DEBUG_ERR("Can't alloc wlan header!\n");
            rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
            goto stop_proc;
        }

        if (skb->len > priv->pmib->dot11OperationEntry.dot11RTSThreshold)
            txcfg->retry = priv->pmib->dot11OperationEntry.dot11LongRetryLimit;
        else
            txcfg->retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

        if(*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2)) == htons(0x888e))
            txcfg->retry = 0x20;

        memset((void *)txcfg->phdr, 0, sizeof(struct wlanllc_hdr));

#ifdef CONFIG_RTK_MESH
        if(txcfg->is_11s)
        {
            SetFrameSubType(txcfg->phdr, WIFI_11S_MESH);
            SetToDs(txcfg->phdr);
        }
        else
#endif
#ifdef WIFI_WMM
        if ((pstat) && (QOS_ENABLE) && (pstat->QosEnabled))
            SetFrameSubType(txcfg->phdr, WIFI_QOS_DATA);
        else
#endif
            SetFrameType(txcfg->phdr, WIFI_DATA);


        #ifdef A4_STA
        if (pstat && (pstat->state & WIFI_A4_STA)) {
            if(priv->pshare->rf_ft_var.a4_enable == 2) {
                use4Addr = 1;
            }
            else { /*when a4_enable = 1, use 3-address to send EAP packet*/
                if(*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2)) != htons(0x888e))
                    use4Addr = 1;
            }                
            txcfg->pstat = pstat;
        }
        #endif

        if (OPMODE & WIFI_AP_STATE) {
            SetFrDs(txcfg->phdr);
            #ifdef WDS
            if (wdsDev)
                SetToDs(txcfg->phdr);
            #endif
            #ifdef A4_STA
            if(use4Addr)
                SetToDs(txcfg->phdr);
            #endif                     
        }
#ifdef CLIENT_MODE
        else if (OPMODE & WIFI_STATION_STATE) {
            SetToDs(txcfg->phdr);
            #ifdef A4_STA
            if(use4Addr)
                SetFrDs(txcfg->phdr);
            #endif         
        }
        else if (OPMODE & WIFI_ADHOC_STATE)
        /* toDS=0, frDS=0 */;
#endif
		else
			DEBUG_WARN("non supported mode yet!\n");
	}

#ifdef USE_TXQUEUE
    if (BUFQ_ENABLE && priv->pshare->iot_mode_enable)
        rtl8192cd_tx_dsr((unsigned long)priv);
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
    txcfg->next_txpath = TXPATH_FIRETX;
#endif
	
	//SMP_UNLOCK_XMIT(flags);
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
    if (__rtl8192cd_firetx(priv, txcfg) == CONGESTED)
#else
    if (rtl8192cd_wlantx(priv, txcfg) == CONGESTED)
#endif
    {
        //SMP_LOCK_XMIT(flags);
        rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
        goto stop_proc;
    }
    //SMP_LOCK_XMIT(flags);

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
    rtw_handle_xmit_fail(priv, txcfg);
#endif

    /* Reply caller function : process done successfully */
    //SMP_UNLOCK_XMIT(flags);
    return TX_PROCEDURE_CTRL_SUCCESS;

stop_proc:
	
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
    rtw_handle_xmit_fail(priv, txcfg);
#endif
    //SMP_UNLOCK_XMIT(flags);

    /* Reply caller function : STOP process */
    return TX_PROCEDURE_CTRL_STOP;
}


__IRAM_IN_865X
int rtl8192cd_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int ret;
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

#ifdef PCIE_POWER_SAVING_TEST //yllin
	if((priv->pwr_state >= L2) || (priv->pwr_state == L1)) {
		dev_kfree_skb_any(skb);
  		return 0; 
 	}
#endif

#ifdef CONFIG_32K 
    if(priv->offload_32k_flag==1){//would run this code when enable 32K
        dev_kfree_skb_any(skb);
        printk("32k rtl8192cd_start_xmit return\n");
  		return 0; 
    }
#endif

#if !defined(CONFIG_USB_HCI) && !defined(CONFIG_SDIO_HCI)
	unsigned long x;
#endif

#if defined(RTK_NL80211) && defined(MBSSID)
	struct net_device *dev_vap = NULL;
	if(ntohs(*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2))) == 0x888e)
	{
		NDEBUG3("Tx EAP packets,len[%d],centCh[%d],Ch[%d] \n", 
             skb->len,priv->pshare->working_channel,priv->pshare->working_channel2);
        
		if(IS_ROOT_INTERFACE(priv) && priv->pmib->miscEntry.vap_enable)
		{	
			struct stat_info	*pstat=NULL;
			unsigned char		*da;
			unsigned char		*sa;
			
			da = skb->data;
			sa = skb->data+ETH_ALEN;

			//dump_mac2(da);
			//dump_mac2(sa);
			pstat = get_stainfo(priv, da);

			if(pstat == NULL)
			{
				int i;
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {	
					if(IS_DRV_OPEN(priv->pvap_priv[i]))
					{
						pstat = get_stainfo(priv->pvap_priv[i], da);
						if(pstat)
						{
							priv = priv->pvap_priv[i];
							dev_vap = priv->dev;
							memcpy(sa, dev_vap->dev_addr, ETH_ALEN);
							printk("Use [%s] Tx EAP packets \n", dev_vap->name);
							//dump_mac2(sa);
							break;
						}
					}
				}				
			}
		}
	}
#endif

#ifdef CROSSBAND_REPEATER			
			if(IS_VXD_INTERFACE(priv) && IS_DRV_OPEN(priv))
			{	
//Check if crossband enabled && valid switch path available && this is not preferred interface
				if(GET_ROOT(priv)->crossband.crossband_status
					&& GET_ROOT(priv)->pmib->crossBand.crossband_pathReady
					&& !GET_ROOT(priv)->pmib->crossBand.crossband_prefer) 
				{			
						if(IS_DRV_OPEN(GET_ROOT(priv)->crossband.crossband_priv_sc))
						{					
							priv = GET_VXD_PRIV(GET_ROOT(priv)->crossband.crossband_priv_sc);
							dev = priv->dev; //switch network device for transmission
							priv->ext_stats.cb_pathswitch_pkts++;
						}
				}
			}

#endif

	if (!(priv->drv_state & DRV_STATE_OPEN)){ 
		dev_kfree_skb_any(skb);
  		return 0; 
 	}

#ifdef __OSK__	
 	extern unsigned int AvailableMemSize;  // 20100818 defined in oskmem.c,  bad declaration, but still here for bad include on OSK.
	if(AvailableMemSize < (100*1024))
	{	// 20100818 this case should not happened, but we still add here to avoid some potential queuing more buffer in wifi driver
		dev_kfree_skb_any(skb);
		printk("TX: Memory is not enoughput\n");
		return 0; 
	}
#endif

#ifdef CONFIG_POWER_SAVE
	if (rtw_ap_ps_xmit_monitor(priv)) {
		dev_kfree_skb_any(skb);	
		return 0;
	}
#endif

	SAVE_INT_AND_CLI(x);
	SMP_LOCK_XMIT(x);

#ifdef MCAST2UI_REFINE
	memcpy(&skb->cb[10], skb->data, 6);
#endif

	STARTROMEPERF(priv,0);	
	
#if defined(RTK_NL80211) && defined(MBSSID)
	if(dev_vap)
	ret = __rtl8192cd_start_xmit(skb, dev_vap, TX_NORMAL);
	else
#endif
	ret = __rtl8192cd_start_xmit(skb, dev, TX_NORMAL);

	STOPROMEPERF(priv,1,0);
	RESTORE_INT(x);
	SMP_UNLOCK_XMIT(x);

	return ret;
}


#ifdef SUPPORT_TX_MCAST2UNI

__IRAM_IN_865X
int rtl8192cd_start_xmit_noM2U(struct sk_buff *skb, struct net_device *dev)
{
	int ret;

#if !defined(CONFIG_USB_HCI) && !defined(CONFIG_SDIO_HCI)
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
	unsigned long x;

	SAVE_INT_AND_CLI(x);
	SMP_LOCK_XMIT(x);
#endif

	ret = __rtl8192cd_start_xmit(skb, dev, TX_NO_MUL2UNI);

#if !defined(CONFIG_USB_HCI) && !defined(CONFIG_SDIO_HCI)
	RESTORE_INT(x);
	SMP_UNLOCK_XMIT(x);
#endif

	return ret;
}

#endif

#ifdef SUPPORT_TX_AMSDU
static int amsdu_xmit(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct tx_insn *txcfg, int tid,
				int from_isr, struct net_device *wdsDev, struct net_device *dev)
{
	int q_num, max_size, is_first=1, total_len=0, total_num=0;
	struct sk_buff *pskb;
	unsigned long	flags;
	int *tx_head, *tx_tail, space=0;
	struct rtl8192cd_hw	*phw = GET_HW(priv);

	txcfg->pstat = pstat;
	q_num = txcfg->q_num;

	tx_head = get_txhead_addr(phw, q_num);
	tx_tail = get_txtail_addr(phw, q_num);

	max_size = pstat->amsdu_level;

#ifdef _AMPSDU_AMSDU_DEBUG_
	if(from_isr)
		pstat->tx_amsdu_to++;
#endif

	// start to transmit queued skb
	SAVE_INT_AND_CLI(flags);
	while (skb_queue_len(&pstat->amsdu_tx_que[tid]) > 0) {
		pskb = __skb_dequeue(&pstat->amsdu_tx_que[tid]);
		if (pskb == NULL)
			break;
		total_len += (pskb->len + sizeof(struct llc_snap) + 3);
#ifdef SUPPORT_TX_AMSDU_SHORTCUT		
		pskb->cb[47]=q_num;
#endif
		if (is_first) {
			if (skb_queue_len(&pstat->amsdu_tx_que[tid]) > 0) {
                if (
#ifdef  CONFIG_WLAN_HAL
                    (IS_HAL_CHIP(priv)) ? (compareAvailableTXBD(priv, 10, q_num, 2)) :
#endif		
#ifdef OSK_LOW_TX_DESC
					(CIRC_SPACE_RTK(*tx_head, *tx_tail, (q_num==BE_QUEUE || q_num==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC) < 10)
#else
					(CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC) < 10)
#endif
                ) {
#ifdef SMP_SYNC
					if (!priv->pshare->has_triggered_tx_tasklet) {
						tasklet_schedule(&priv->pshare->tx_tasklet);
						priv->pshare->has_triggered_tx_tasklet = 1;
					}
#else
					rtl8192cd_tx_dsr((unsigned long)priv);
#endif

                    if (
#ifdef  CONFIG_WLAN_HAL
                        (IS_HAL_CHIP(priv)) ? (compareAvailableTXBD(priv, 10, q_num, 2)) :
#endif		
#ifdef OSK_LOW_TX_DESC
						(CIRC_SPACE_RTK(*tx_head, *tx_tail, (q_num==BE_QUEUE || q_num==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC) < 10)
#else
						(CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC) < 10)
#endif		
					) {
						// printk("Tx desc not enough for A-MSDU!\n");
						__skb_queue_head(&pstat->amsdu_tx_que[tid], pskb);
						RESTORE_INT(flags);
#ifdef _AMPSDU_AMSDU_DEBUG_
						pstat->tx_amsdu_buf_overflow++;
#endif						
						return 0;
					}
				}
				txcfg->aggre_en = FG_AGGRE_MSDU_FIRST;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT	
				pskb->cb[47] |=BIT(FG_AGGRE_MSDU_FIRST);	//AMSDU first
#endif
				is_first = 0;
				total_num++;
			}
			else {
				if (!from_isr) {
					__skb_queue_head(&pstat->amsdu_tx_que[tid], pskb);
					RESTORE_INT(flags);
#ifdef _AMPSDU_AMSDU_DEBUG_
					pstat->tx_amsdu_buf_overflow++;
#endif
					return 0;
				}
				txcfg->aggre_en = 0;
			}
		}
		else if ((skb_queue_len(&pstat->amsdu_tx_que[tid]) == 0) ||
				((total_len + ((struct sk_buff *)pstat->amsdu_tx_que[tid].next)->len + sizeof(struct llc_snap) + 3) > max_size) ||
				(
#ifdef CONFIG_WLAN_HAL
                (IS_HAL_CHIP(priv)) ? (compareAvailableTXBD(priv, 1, q_num, 2)) : 
#endif
				(total_num >= (space - 4)) )|| // 1 for header, 1 for ICV when sw encrypt, 2 for spare 
				(!pstat->is_realtek_sta && (total_num >= (priv->pmib->dot11nConfigEntry.dot11nAMSDUSendNum-1)))
#ifdef CONFIG_WLAN_HAL
                 || (total_num >= (priv->pmib->dot11nConfigEntry.dot11curAMSDUSendNum-1))
#endif
        ) {
			txcfg->aggre_en = FG_AGGRE_MSDU_LAST;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT				
			pskb->cb[47] |=BIT(FG_AGGRE_MSDU_LAST);	//AMSDU last
#endif
			total_len = 0;
			is_first = 1;
#ifdef _AMPSDU_AMSDU_DEBUG_
			switch(total_num){
				case 0:
					pstat->tx_amsdu_1pkt++;
					break;
				case 1:
					pstat->tx_amsdu_2pkt++;
					break;
				case 2:
					pstat->tx_amsdu_3pkt++;
					break;
				case 3:
					pstat->tx_amsdu_4pkt++;
					break;
				case 4:
					pstat->tx_amsdu_5pkt++;
					break;
				default:
					pstat->tx_amsdu_gt5pkt++;
					break;
			}
#endif
			total_num = 0;
		}
		else {
			txcfg->aggre_en = FG_AGGRE_MSDU_MIDDLE;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
			pskb->cb[47] |=BIT(FG_AGGRE_MSDU_MIDDLE);	//AMSDU middle
#endif
			total_num++;
		}

		pstat->amsdu_size[tid] -= (pskb->len + sizeof(struct llc_snap));
#ifdef MESH_AMSDU
		if( isMeshPoint(pstat))
		{
			txcfg->is_11s = 8;
			dev = priv->mesh_dev;
			memcpy(txcfg->nhop_11s, pstat->hwaddr, MACADDRLEN);
		}
		else
			txcfg->is_11s = 0;

#endif
		//SMP_UNLOCK_XMIT(flags);
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
		if(!priv->pmib->dot11OperationEntry.disable_amsdu_txsc)
			__rtl8192cd_start_xmit_out(pskb, pstat);
		else
		{
			pstat->tx_sc_amsdu_pkts_slow++;
#endif
		rtl8192cd_tx_slowPath(priv, pskb, pstat, dev, wdsDev, txcfg);
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
		}
#endif
		//SMP_LOCK_XMIT(flags);

	}
	RESTORE_INT(flags);

	return 1;
}


static int amsdu_timer_add(struct rtl8192cd_priv *priv, struct stat_info *pstat, int tid, int from_timeout)
{
	unsigned int now, timeout, new_timer=0;
	int setup_timer;
	int current_idx, next_idx;

	current_idx = priv->pshare->amsdu_timer_head;

	while (CIRC_CNT(current_idx, priv->pshare->amsdu_timer_tail, AMSDU_TIMER_NUM)) {
		if (priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].pstat == NULL) {
			priv->pshare->amsdu_timer_tail = (priv->pshare->amsdu_timer_tail + 1) & (AMSDU_TIMER_NUM - 1);
			new_timer = 1;
		}
		else
			break;
	}

	if (CIRC_CNT(current_idx, priv->pshare->amsdu_timer_tail, AMSDU_TIMER_NUM) == 0) {
		cancel_timer2(priv);
		setup_timer = 1;
	}
	else if (CIRC_SPACE(current_idx, priv->pshare->amsdu_timer_tail, AMSDU_TIMER_NUM) == 0) {
		//printk("%s: %s, amsdu timer overflow!\n", priv->dev->name, __FUNCTION__ );
		pstat->tx_amsdu_timer_ovf++;
		return -1;
	}
	else {	// some items in timer queue
		setup_timer = 0;
		if (new_timer)
			new_timer = priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].timeout;
	}

	next_idx = (current_idx + 1) & (AMSDU_TIMER_NUM - 1);

	priv->pshare->amsdu_timer[current_idx].priv = priv;
	priv->pshare->amsdu_timer[current_idx].pstat = pstat;
	priv->pshare->amsdu_timer[current_idx].tid = (unsigned char)tid;
	priv->pshare->amsdu_timer_head = next_idx;

#ifdef CONFIG_WLAN_HAL
    if ( IS_HAL_CHIP(priv) ) {
        GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_TSF_TIMER, (pu1Byte)&now);
    } else
#endif
    {
	    now = RTL_R32(TSFTR);
    }

	timeout = now + priv->pmib->dot11nConfigEntry.dot11nAMSDUSendTimeout;
	priv->pshare->amsdu_timer[current_idx].timeout = timeout;

	if (!from_timeout) {
		if (setup_timer)
			setup_timer2(priv, timeout);
		else if (new_timer) {
			if (TSF_LESS(new_timer, now))
				setup_timer2(priv, timeout);
			else
				setup_timer2(priv, new_timer);
		}
	}

	return current_idx;
}


void amsdu_timeout(struct rtl8192cd_priv *priv, unsigned int current_time)
{
	struct tx_insn tx_insn;
	struct stat_info *pstat;
	struct net_device *wdsDev=NULL;
	struct rtl8192cd_priv *priv_this=NULL;
	int tid=0, head;
	//DECLARE_TXCFG(txcfg, tx_insn);
	unsigned long flags;
	
	SMP_LOCK_XMIT(flags);
	
	head = priv->pshare->amsdu_timer_head;
	while (CIRC_CNT(head, priv->pshare->amsdu_timer_tail, AMSDU_TIMER_NUM))
	{
		DECLARE_TXCFG(txcfg, tx_insn);	// will be reused in this while loop

		pstat = priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].pstat;
		if (pstat) {
			tid = priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].tid;
			priv_this = priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].priv;
			priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].pstat = NULL;
		}

		priv->pshare->amsdu_timer_tail = (priv->pshare->amsdu_timer_tail + 1) & (AMSDU_TIMER_NUM - 1);

		if (pstat) {
#ifdef WDS
			wdsDev = NULL;
			if (pstat->state & WIFI_WDS) {
				wdsDev = getWdsDevByAddr(priv, pstat->hwaddr);
				txcfg->wdsIdx = getWdsIdxByDev(priv, wdsDev);
			}
#endif
#ifdef RTL_MANUAL_EDCA
			txcfg->q_num = PRI_TO_QNUM(priv, tid);
#else
			PRI_TO_QNUM(tid, txcfg->q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif

			if (pstat->state & WIFI_SLEEP_STATE)
				pstat->amsdu_timer_id[tid] = amsdu_timer_add(priv_this, pstat, tid, 1) + 1;
			else
			{
				int ret;

				ret = amsdu_xmit(priv_this, pstat, txcfg, tid, 1, wdsDev, priv->dev);
				
				if (ret == 0) // not finish
					pstat->amsdu_timer_id[tid] = amsdu_timer_add(priv_this, pstat, tid, 1) + 1;
			    else
				    pstat->amsdu_timer_id[tid] = 0;
		    }
	    }
	}

	if (CIRC_CNT(priv->pshare->amsdu_timer_head, priv->pshare->amsdu_timer_tail, AMSDU_TIMER_NUM)) {
		setup_timer2(priv, priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].timeout);
		if (TSF_LESS(priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].timeout, current_time))
			printk("Setup timer2 %d too late (now %d)\n", priv->pshare->amsdu_timer[priv->pshare->amsdu_timer_tail].timeout, current_time);
	}
	SMP_UNLOCK_XMIT(flags);	
}


 int amsdu_check(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat, struct tx_insn *txcfg)
{
	int q_num;
	unsigned int priority;
	unsigned short protocol;
	int *tx_head, *tx_tail, cnt, add_timer=1;
	struct rtl8192cd_hw	*phw;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	struct net_device *wdsDev=NULL;

	protocol = ntohs(*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2)));
#ifdef CONFIG_RTL_WAPI_SUPPORT
	if ((protocol == 0x888e)||(protocol == ETH_P_WAPI))
#else
	if (protocol == 0x888e)
#endif
	{
		return RET_AGGRE_BYPASS;
	}

	if (((protocol + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) &&
				 (skb_headroom(skb) < sizeof(struct llc_snap))) {
		return RET_AGGRE_BYPASS;
	}
//----------------------

#ifdef CONFIG_RTK_MESH
	priority = get_skb_priority3(priv, skb, txcfg->is_11s, pstat);
#else
	priority = get_skb_priority(priv, skb, pstat);
#endif
#ifdef RTL_MANUAL_EDCA
	q_num = PRI_TO_QNUM(priv, priority);
#else
	PRI_TO_QNUM(priority, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif

#ifdef  CONFIG_WLAN_HAL
    if (IS_HAL_CHIP(priv)) {
        if (compareAvailableTXBD(priv, 10, q_num, 2))
            return RET_AGGRE_DESC_FULL;
    } else 
#endif
    {
    	phw = GET_HW(priv);
    	tx_head = get_txhead_addr(phw, q_num);
    	tx_tail = get_txtail_addr(phw, q_num);

#ifdef OSK_LOW_TX_DESC
		cnt = CIRC_CNT_RTK(*tx_head, *tx_tail, (q_num==BE_QUEUE || q_num==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC);
#else
    	cnt = CIRC_CNT_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC);
#endif

#if 0
    	if (cnt <= AMSDU_TX_DESC_TH)
    		return RET_AGGRE_BYPASS;
#endif

#ifdef OSK_LOW_TX_DESC
		if (cnt == (((q_num==BE_QUEUE || q_num==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC) - 1))
#else
    	if (cnt == (CURRENT_NUM_TX_DESC - 1))
#endif
    		return RET_AGGRE_DESC_FULL;
    }

#ifdef MESH_AMSDU
	if (txcfg->is_11s&1)
	{
		short j, popen =  ((txcfg->mesh_header.mesh_flag &1) ? 16 : 4);
		if (skb_headroom(skb) < popen || skb_cloned(skb)) {
			struct sk_buff *skb2 = dev_alloc_skb(skb->len);
			if (skb2 == NULL) {
				printk("%s: %s, dev_alloc_skb() failed!\n", priv->mesh_dev->name, __FUNCTION__);
				return RET_AGGRE_BYPASS;
			}
         	memcpy(skb_put(skb2, skb->len), skb->data, skb->len);
			dev_kfree_skb_any(skb);
			skb = skb2;
			txcfg->pframe = (void *)skb;
		}
		skb_push(skb, popen);
		for(j=0; j<sizeof(struct wlan_ethhdr_t); j++)
			skb->data[j]= skb->data[j+popen];
		memcpy(skb->data+j, &(txcfg->mesh_header), popen);
	}
#endif // MESH_AMSDU


	SAVE_INT_AND_CLI(flags);
	__skb_queue_tail(&pstat->amsdu_tx_que[priority], skb);
	pstat->amsdu_size[priority] += (skb->len + sizeof(struct llc_snap));

	if ((pstat->amsdu_size[priority] >= pstat->amsdu_level) ||
			(!pstat->is_realtek_sta && (skb_queue_len(&pstat->amsdu_tx_que[priority]) >= priv->pmib->dot11nConfigEntry.dot11nAMSDUSendNum)))
	{
#ifdef WDS
		wdsDev = NULL;
		if (pstat->state & WIFI_WDS) {
			wdsDev = getWdsDevByAddr(priv, pstat->hwaddr);
			txcfg->wdsIdx = getWdsIdxByDev(priv, wdsDev);
		}
#endif
		// delete timer entry
		if (pstat->amsdu_timer_id[priority] > 0) {
			priv->pshare->amsdu_timer[pstat->amsdu_timer_id[priority] - 1].pstat = NULL;
			pstat->amsdu_timer_id[priority] = 0;
		}
		txcfg->q_num = q_num;
		if (amsdu_xmit(priv, pstat, txcfg, priority, 0, wdsDev, priv->dev) == 0) // not finish
			pstat->amsdu_timer_id[priority] = amsdu_timer_add(priv, pstat, priority, 0) + 1;
		else
			add_timer = 0;
	}

	if (add_timer) {
		if (pstat->amsdu_timer_id[priority] == 0)
			pstat->amsdu_timer_id[priority] = amsdu_timer_add(priv, pstat, priority, 0) + 1;
	}

	RESTORE_INT(flags);

	return RET_AGGRE_ENQUE;
}
#endif // SUPPORT_TX_AMSDU

#ifdef SUPPORT_TX_MCAST2UNI
#ifndef CONFIG_RTK_MESH
static 
#endif
int isICMPv6Mng(struct sk_buff *skb)
{
#ifdef __ECOS
	if((skb->data[12] == 0x86) &&(skb->data[13] == 0xdd)&&
#else
	if((skb->protocol == __constant_htons(0x86dd)) &&
#endif
		((skb->data[20] == 0x3a) || (skb->data[54] == 0x3a)) //next header is icmpv6
		//&& skb->data[54] == 0x86 //RA
		/*128,129,133,.....255 SHOULD BE MANAGMENT PACKET
		 REF:http://en.wikipedia.org/wiki/ICMPv6 */
	)
	{
		return 1;		
	}
	else
		return 0;
}

#ifndef CONFIG_RTK_MESH
static 
#endif
inline int isMDNS(unsigned char *data)
{
	if((data[3] == 0x00) && (data[4] == 0x00) && (data[5] == 0xfb) &&
			(((data[12] == 0x08) && (data[13] == 0x00) // IPv4
			&& (data[23] == 0x11) // UDP
			//&& (data[30] == 0xe0) // 224.0.0.251
			&& (data[36] == 0x14) && (data[37] == 0xe9)) // port 5353
			||
			((data[12] == 0x86) && (data[13] == 0xdd) // IPv6
			&& (data[20] == 0x11) // next header is UDP
			&& (data[56] == 0x14) && (data[57] == 0xe9)) // port 5353
			)) {
		return 1;
	}
		
	return 0;
}

#ifdef CONFIG_PCI_HCI
static int rtl8192cd_tx_recycle(struct rtl8192cd_priv *priv, unsigned int txRingIdx, int *recycleCnt_p);
static inline void check_tx_queue(struct rtl8192cd_priv *priv)
{
		int *tx_head, *tx_tail;
		struct rtl8192cd_hw	*phw = GET_HW(priv);

#ifdef  CONFIG_WLAN_HAL
        PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
		if (IS_HAL_CHIP(priv)) {      
	        ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			tx_head = get_txhead_addr(phw, BE_QUEUE);// use BE queue to send multicast
			tx_tail = get_txtail_addr(phw, BE_QUEUE);		
		}
        if (
#ifdef  CONFIG_WLAN_HAL
            (IS_HAL_CHIP(priv)) ? (compareAvailableTXBD(priv, (CURRENT_NUM_TX_DESC/4), BE_QUEUE, 2)) :
#endif		
			(CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC) < (CURRENT_NUM_TX_DESC/4)) 
		)	// OSK: because BE_Queue = CURRENT_NUM_TX_DESC, so do not need to change
		{
			rtl8192cd_tx_queueDsr(priv, BE_QUEUE);
			//int recycleCnt;
			//rtl8192cd_tx_recycle(priv, BE_QUEUE, &recycleCnt);
		}
		
		return;
}
#endif // CONFIG_PCI_HCI


#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
	extern struct sk_buff *priv_skb_copy(struct sk_buff *skb);
	extern int eth_skb_free_num;
#endif
#ifdef CONFIG_RTL8196B_GW_8M
#define ETH_SKB_FREE_TH 50
#else
#define ETH_SKB_FREE_TH 100
#endif

int isSpecialFloodMac(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	int i;
	if(priv->pshare->rf_ft_var.mc2u_flood_ctrl==0)
	{
		return 0;
	}
	
	for(i=0; i< priv->pshare->rf_ft_var.mc2u_flood_mac_num; i++)
	{
		if(memcmp(skb->data, priv->pshare->rf_ft_var.mc2u_flood_mac[i].macAddr,MACADDRLEN)==0)
		{
			return 1;
		}

	}
	return 0;
}

int mlcst2unicst(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	struct stat_info *pstat, *sa_stat;
	struct list_head *phead, *plist;
	struct sk_buff *newskb;
	unsigned char origDest[6];
	int i= 0;
	int m2uCnt =0;
	int fwdCnt=0;
	struct stat_info *pstat_found = NULL;
#ifdef MCAST2UI_REFINE
	unsigned int privacy;
#endif

#ifdef CONFIG_MAXED_MULTICAST	
	int M2Uanyway=0;
#endif

#ifdef HS2_SUPPORT
	// if AP 
	// 1.support HS2 and 
	// 2.dgaf disable=0 
	// it means to let icmpv6 mgmt multicast to clients
	if ((priv->pmib->hs2Entry.hs2_ielen != 0) && (priv->dgaf_disable == 0)) {
		if (isICMPv6Mng(skb) || IS_ICMPV6_PROTO(skb->data))
			return 0;
    }
#endif
	
	memcpy(origDest, skb->data, 6);

    sa_stat = get_stainfo(priv, skb->data+MACADDRLEN);
#ifdef A4_STA
    if (priv->pshare->rf_ft_var.a4_enable) {
        if(sa_stat == NULL) {
            sa_stat = a4_sta_lookup(priv, skb->data+MACADDRLEN);
        }
    }
#endif  

	// all multicast managment packet try do m2u
	if( isSpecialFloodMac(priv,skb) || IS_MDNSV4_MAC(skb->data)||IS_MDNSV6_MAC(skb->data)||IS_IGMP_PROTO(skb->data) || isICMPv6Mng(skb) || IS_ICMPV6_PROTO(skb->data)|| isMDNS(skb->data))
	{
		/*added by qinjunjie,do multicast to unicast conversion, and send to every associated station */
		phead = &priv->asoc_list;
		plist = phead;
		while ((plist = asoc_list_get_next(priv, plist)) != phead) {
			pstat = list_entry(plist, struct stat_info, asoc_list);

            #ifdef CONFIG_RTK_MESH
            if(isPossibleNeighbor(pstat)) {
                continue;
            }
            #endif

			/* avoid   come from STA1 and send back STA1 */ 
			if (sa_stat == pstat) {
				continue; 
			}
			
            if(pstat->leave || pstat->expire_to==0)
				continue; 			
#ifdef MCAST2UI_REFINE
			privacy = get_sta_encrypt_algthm(priv, pstat);
			if ((privacy == _NO_PRIVACY_ || (privacy == _CCMP_PRIVACY_
#ifdef CONFIG_IEEE80211W
				&& !UseSwCrypto(priv, pstat, FALSE,0)))
#else
				&& !UseSwCrypto(priv, pstat, FALSE)))
#endif
				&& (newskb = skb_clone(skb, GFP_ATOMIC)))
			{
				memcpy(&newskb->cb[10], pstat->hwaddr, 6);
				newskb->cb[2] = (char)0xff;
				__rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
			}
			else
#endif
			{
				if((plist->next == phead) && !skb_cloned(skb)) {
					asoc_list_unref(priv, pstat);
#ifdef MCAST2UI_REFINE                                                  
					memcpy(&skb->cb[10], pstat->hwaddr, 6);
#else
					memcpy(skb->data, pstat->hwaddr, 6);
#endif
					skb->cb[2] = (char)0xff;			// not do aggregation
#ifdef ENABLE_RTL_SKB_STATS
					rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif
					__rtl8192cd_start_xmit(skb, priv->dev, TX_NO_MUL2UNI);
					return TRUE;
				}
				
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
				newskb = priv_skb_copy(skb);
#else
				newskb = skb_copy(skb, GFP_ATOMIC);
#endif
				if (newskb) {
#ifdef MCAST2UI_REFINE                                                  
					memcpy(&newskb->cb[10], pstat->hwaddr, 6);
#else
					memcpy(newskb->data, pstat->hwaddr, 6);
#endif
					newskb->cb[2] = (char)0xff;			// not do aggregation
					__rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
				}
				else {
					asoc_list_unref(priv, pstat);
					
					DEBUG_ERR("%s: muti2unit skb_copy() failed!\n", priv->dev->name);
					priv->stop_tx_mcast2uni = 2;
					priv->ext_stats.tx_drops++;
					DEBUG_ERR("TX DROP: %s: run out ether buffer!\n", __FUNCTION__);
					rtl_kfree_skb(priv, skb, _SKB_TX_);
					return TRUE;
				}

			}
		}
		rtl_kfree_skb(priv, skb, _SKB_TX_);
		return TRUE;
	}

#ifdef TV_MODE
    if((priv->tv_mode_status & BIT0) == 0) { /*TV mode is disabled*/
#ifdef A4_STA
        if(priv->pshare->rf_ft_var.a4_enable == 0)
#endif
            return FALSE;
    }
#endif

//#ifdef VIDEO_STREAMING_REFINE
	// for video streaming refine 
	phead = &priv->asoc_list;
	plist = phead;
	while ((plist = asoc_list_get_next(priv, plist)) != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);

        #ifdef CONFIG_RTK_MESH
        if(isPossibleNeighbor(pstat)) {
            continue;
        }
        #endif

		/* avoid   come from STA1 and send back STA1 */ 
		if (sa_stat == pstat){		
			continue; 
		}		
        if(pstat->leave || pstat->expire_to==0)
            continue; 
		for (i=0; i<MAX_IP_MC_ENTRY; i++) {
			if (pstat->ipmc[i].used && !memcmp(&pstat->ipmc[i].mcmac[0], origDest, 6)) {
				pstat_found = pstat;
				m2uCnt++;
				break;
			}
		}
	}

#ifdef TV_MODE
    if(priv->tv_mode_status & BIT0)/*TV mode is enabled*/    
#endif
    {
        if (m2uCnt == 1 && !skb_cloned(skb)) {
#ifdef MCAST2UI_REFINE                                                  
            memcpy(&skb->cb[10], pstat_found->hwaddr, 6);
#else
            memcpy(skb->data, pstat_found->hwaddr, 6);
#endif
#ifdef ENABLE_RTL_SKB_STATS
            rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif
            __rtl8192cd_start_xmit(skb, priv->dev, TX_NO_MUL2UNI);		
            return TRUE;
        }
    }

	fwdCnt = m2uCnt;
	
	if(m2uCnt == 0){

#ifdef CONFIG_MAXED_MULTICAST	
		/*case: when STA <=3 do M2U anyway ; 
		if STA number > 3 by orig method(multicast);*/
		if(priv->assoc_num <=3){
			M2Uanyway=1;
			fwdCnt = priv->assoc_num;			
		}else
#endif
		if(!priv->pshare->rf_ft_var.mc2u_drop_unknown) 
		{
			/*case: if M2U can't success then 
			  forward by multicast(orig method),
			  defect: may affect system performance
		    	 advantage:better compatibility*/ 
			return FALSE;		
		}
		else
		{

		/*case: if M2U can't success then drop this packet ;
		    defect:maybe some management packet will lose
		    advantage:better performance*/ 
	   	    DEBUG_WARN("TX DROP: %s %d !\n", __FUNCTION__,__LINE__);
			priv->ext_stats.tx_drops++;
			rtl_kfree_skb(priv, skb, _SKB_TX_);
			return TRUE;
			
		}
	}
//#endif


	
	// Do multicast to unicast conversion
	phead = &priv->asoc_list;
	plist = phead;
	while ((plist = asoc_list_get_next(priv, plist)) != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);

        #ifdef CONFIG_RTK_MESH
        if(isPossibleNeighbor(pstat)) {
            continue;
        }
        #endif

		/* avoid   come from STA1 and send back STA1 */ 
		if (sa_stat == pstat) {
			continue;
		}		
        
        if(pstat->leave || pstat->expire_to==0)
            continue;

#ifdef TV_MODE
        if((priv->tv_mode_status & BIT0) == 0) { /*TV mode is disabled*/
#ifdef A4_STA
            if((pstat->state & WIFI_A4_STA) == 0)
#endif
                continue;
        }
#endif

#ifdef CONFIG_PCI_HCI
		{
			int *tx_head, *tx_tail, q_num;
			struct rtl8192cd_hw	*phw = GET_HW(priv);
			q_num = BE_QUEUE;	// use BE queue to send multicast
#ifdef CONFIG_WLAN_HAL
	        //PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
			if (IS_HAL_CHIP(priv)) {
				//ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
			} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			{//not HAL
				tx_head = get_txhead_addr(phw, q_num);
				tx_tail = get_txtail_addr(phw, q_num);
			}
			
			if (priv->stop_tx_mcast2uni) {
				rtl8192cd_tx_queueDsr(priv, q_num);

				if (priv->stop_tx_mcast2uni  == 1) {
#ifdef CONFIG_WLAN_HAL
					if(IS_HAL_CHIP(priv)) { // OSK: because qum = BE_Queue = CURRENT_NUM_TX_DESC, so do not need to change
                        	if(compareAvailableTXBD(priv, ((CURRENT_NUM_TX_DESC*1)/4), q_num, 1)) 
								priv->stop_tx_mcast2uni = 0;
                        } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
						{//not HAL
							if(CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC) > (CURRENT_NUM_TX_DESC*1)/4)	
								priv->stop_tx_mcast2uni = 0;
						}
				}

#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
				else if ((priv->stop_tx_mcast2uni == 2) && (eth_skb_free_num > ETH_SKB_FREE_TH))
				{
					priv->stop_tx_mcast2uni = 0;
				}
#endif
				else {
					asoc_list_unref(priv, pstat);
					
					priv->ext_stats.tx_drops++;
					DEBUG_ERR("TX DROP: %s: run out ether buffer!\n", __FUNCTION__);
					rtl_kfree_skb(priv, skb, _SKB_TX_);
					return TRUE;
				}
			} else {
#ifdef CONFIG_WLAN_HAL
                if(IS_HAL_CHIP(priv)) {// OSK: because qum = BE_Queue = CURRENT_NUM_TX_DESC, so do not need to change
					if(compareAvailableTXBD(priv, 20, q_num, 2)) {
						asoc_list_unref(priv, pstat);
						#ifdef __ECOS
						#else
						priv->stop_tx_mcast2uni = 1;
						#endif
						priv->ext_stats.tx_drops++;
						DEBUG_ERR("TX DROP: %s: txdesc full!\n", __FUNCTION__);
						rtl_kfree_skb(priv, skb, _SKB_TX_);
						return TRUE;
					}
				} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif			
				{//not HAL
					if (CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC) < 20) {
						asoc_list_unref(priv, pstat);
						#ifdef __ECOS
						#else
						priv->stop_tx_mcast2uni = 1;
						#endif
						priv->ext_stats.tx_drops++;
						DEBUG_ERR("TX DROP: %s: txdesc full!\n", __FUNCTION__);
						rtl_kfree_skb(priv, skb, _SKB_TX_);
						return TRUE;
					}
				}
			}
		}
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_MAXED_MULTICAST

		if(M2Uanyway){
			if((fwdCnt== 1) && !skb_cloned(skb))
			{
				asoc_list_unref(priv, pstat);
#ifdef MCAST2UI_REFINE                                                  
				memcpy(&skb->cb[10], pstat->hwaddr, 6);
#else
				memcpy(skb->data, pstat->hwaddr, 6);
#endif
				skb->cb[2] = (char)0xff;			// not do aggregation
#ifdef ENABLE_RTL_SKB_STATS
				rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif
				__rtl8192cd_start_xmit(skb, priv->dev, TX_NO_MUL2UNI);
				return TRUE;
			}
			
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
			newskb = priv_skb_copy(skb);
#else
			newskb = skb_copy(skb, GFP_ATOMIC);
#endif
			if (newskb) {
#ifdef MCAST2UI_REFINE                                                  
				memcpy(&newskb->cb[10], pstat->hwaddr, 6);
#else
				memcpy(newskb->data, pstat->hwaddr, 6);
#endif
				newskb->cb[2] = (char)0xff;			// not do aggregation
				__rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
			}
			else {
				asoc_list_unref(priv, pstat);
				
				DEBUG_ERR("%s: muti2unit skb_copy() failed!\n", priv->dev->name);
				#ifdef __ECOS
				#else
				priv->stop_tx_mcast2uni = 2;
				#endif
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: %s: run out ether buffer!\n", __FUNCTION__);
				rtl_kfree_skb(priv, skb, _SKB_TX_);
				return TRUE;
			}
			fwdCnt--;
			continue;
		}
#endif
		for (i=0; i<MAX_IP_MC_ENTRY; i++) {
			if (pstat->ipmc[i].used && !memcmp(&pstat->ipmc[i].mcmac[0], origDest, 6)) {


				if((fwdCnt== 1) && !skb_cloned(skb))
				{
					asoc_list_unref(priv, pstat);
#ifdef MCAST2UI_REFINE                                                  
					memcpy(&skb->cb[10], pstat->hwaddr, 6);
#else
					memcpy(skb->data, pstat->hwaddr, 6);
#endif
					skb->cb[2] = (char)0xff; 		// not do aggregation
#ifdef ENABLE_RTL_SKB_STATS
					rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif
					__rtl8192cd_start_xmit(skb, priv->dev, TX_NO_MUL2UNI);
					return TRUE;
				}
				else {
#ifdef MCAST2UI_REFINE
					privacy = get_sta_encrypt_algthm(priv, pstat);
					if ((privacy == _NO_PRIVACY_ || (privacy == _CCMP_PRIVACY_
#ifdef CONFIG_IEEE80211W
							&& !UseSwCrypto(priv, pstat, FALSE,0)))
#else
							&& !UseSwCrypto(priv, pstat, FALSE)))
#endif
							&& (newskb = skb_clone(skb, GFP_ATOMIC)))
					{
						memcpy(&newskb->cb[10], pstat->hwaddr, 6);
						newskb->cb[2] = (char)0xff;
						__rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
					}
					else
#endif
					{	
						#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)

						newskb = priv_skb_copy(skb);
						#else
						newskb = skb_copy(skb, GFP_ATOMIC);
						#endif
						if (newskb) {
#ifdef MCAST2UI_REFINE                                                  
							memcpy(&newskb->cb[10], pstat->hwaddr, 6);
#else
							memcpy(newskb->data, pstat->hwaddr, 6);
#endif
							newskb->cb[2] = (char)0xff;			// not do aggregation
							__rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
						}
						else {
							asoc_list_unref(priv, pstat);
							
							DEBUG_ERR("%s: muti2unit skb_copy() failed!\n", priv->dev->name);
							#ifdef __ECOS
							#else
							priv->stop_tx_mcast2uni = 2;
							#endif
							priv->ext_stats.tx_drops++;
							DEBUG_ERR("TX DROP: %s: run out ether buffer!\n", __FUNCTION__);
							rtl_kfree_skb(priv, skb, _SKB_TX_);
							return TRUE;
						}
					}
					fwdCnt--;
					break;
				}
			}
		}
	}
	
#ifdef TV_MODE
    if((priv->tv_mode_status & BIT0) == 0 && fwdCnt) {
        /*TV mode is disabled and has joint legacy sta*/
        if(!skb_cloned(skb)) {   
            #ifdef ENABLE_RTL_SKB_STATS
            rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
            #endif            
            __rtl8192cd_start_xmit(skb, priv->dev, TX_NO_MUL2UNI);
        } else 
        {
            #if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
            newskb = priv_skb_copy(skb);
            #else
            newskb = skb_copy(skb, GFP_ATOMIC);
            #endif
            if (newskb) {
                __rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
            }
            else {            
                asoc_list_unref(priv, pstat);                
                DEBUG_ERR("%s: muti2unit skb_copy() failed!\n", priv->dev->name);
                #ifdef __ECOS
                #else
                priv->stop_tx_mcast2uni = 2;
                #endif
                priv->ext_stats.tx_drops++;
                DEBUG_ERR("TX DROP: %s: run out ether buffer!\n", __FUNCTION__);
            }
            rtl_kfree_skb(priv, skb, _SKB_TX_);
        }
        return TRUE;
    }
#endif
	/*
	 *	Device interested in this MC IP cannot be found, drop packet.
	 */
	rtl_kfree_skb(priv, skb, _SKB_TX_);
	return TRUE;
}

#endif // TX_SUPPORT_MCAST2U


#ifdef RESERVE_TXDESC_FOR_EACH_IF
int check_rsv_txdesc(struct rtl8192cd_priv *priv, int q_num, int txdesc_need)
{
	struct rtl8192cd_priv *root_priv = NULL;
	unsigned int lower_limit = priv->pshare->num_txdesc_lower_limit;
	unsigned int avail_cnt = priv->pshare->num_txdesc_cnt;
	unsigned int used, accu = 0, i;

	if (!IS_RSVQ(q_num))
		return 0;
	
	used = priv->use_txdesc_cnt[RSVQ(q_num)];

	if (IS_ROOT_INTERFACE(priv))
		root_priv = priv;
	else
		root_priv = GET_ROOT(priv);
	
	if (IS_ROOT_INTERFACE(priv)) {
		if (IS_DRV_OPEN(priv))
			accu += used;
	} else {
		if (IS_DRV_OPEN(root_priv))
			accu += MAX_NUM(root_priv->use_txdesc_cnt[RSVQ(q_num)], lower_limit);
	}
	
#ifdef UNIVERSAL_REPEATER
	if (IS_VXD_INTERFACE(priv)) {
		if (IS_DRV_OPEN(priv))
			accu += used;
	} else {
		if (IS_DRV_OPEN(root_priv->pvxd_priv))
			accu += MAX_NUM(root_priv->pvxd_priv->use_txdesc_cnt[RSVQ(q_num)], lower_limit);
	}
#endif

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(root_priv->pvap_priv[i])) {
			if (root_priv->pvap_priv[i] == priv)
				accu += used;
	else
				accu += MAX_NUM(root_priv->pvap_priv[i]->use_txdesc_cnt[RSVQ(q_num)], lower_limit);
		}
	}
#endif
	
	if (accu + txdesc_need <= avail_cnt)
		return 0;
	
	return -1;
}
#endif

#ifdef USE_TXQUEUE
int check_rsv_txbufq(struct rtl8192cd_priv *priv, int q_num)
{
	struct rtl8192cd_priv *root_priv = NULL;
	unsigned int lower_limit = priv->pshare->num_txq_lower_limit;
	unsigned int avail_cnt = priv->pshare->num_txq_cnt;
	unsigned int used, accu = 0, i;

	if (!IS_RSVQ(q_num))
		return 0;

	used = priv->use_txq_cnt[RSVQ(q_num)];
	
	if (IS_ROOT_INTERFACE(priv))
		root_priv = priv;
	else
		root_priv = GET_ROOT(priv);
		
	if (IS_ROOT_INTERFACE(priv))
	{
		if ( IS_DRV_OPEN(priv) )
			accu += used;
	}
	else
	{
		if ( IS_DRV_OPEN(root_priv) )
			accu += MAX_NUM(root_priv->use_txq_cnt[RSVQ(q_num)], lower_limit);
	}
		
#ifdef UNIVERSAL_REPEATER
	if (IS_VXD_INTERFACE(priv))
	{
		if ( IS_DRV_OPEN(priv) )
			accu += used;
	}
	else
	{
		if ( IS_DRV_OPEN(root_priv->pvxd_priv) )
			accu += MAX_NUM(root_priv->pvxd_priv->use_txq_cnt[RSVQ(q_num)], lower_limit);
	}
#endif
	
#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++)
	{
		if ( IS_DRV_OPEN(root_priv->pvap_priv[i]) )
		{
			if (root_priv->pvap_priv[i] == priv)
				accu += used;
			else
				accu += MAX_NUM(root_priv->pvap_priv[i]->use_txq_cnt[RSVQ(q_num)], lower_limit);
		}
	}
#endif
		
	if (accu < avail_cnt)
		return 0;

	return -1;

}
#endif


#if (BEAMFORMING_SUPPORT == 1)
BOOLEAN
IsMgntNDPA(
	pu1Byte		pdu
)
{
	BOOLEAN ret = 0;
	if(IsMgntActionNoAck(pdu) && GET_80211_HDR_ORDER(pdu))
	{
		if(GET_HT_CTRL_NDP_ANNOUNCEMENT(pdu+sMacHdrLng) == 1)
			ret = 1;
	}
	return ret;
}
#endif

void RtsCheck(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, u4Byte fr_len,
	BOOLEAN *bRtsEnable, BOOLEAN *bCts2SelfEnable, BOOLEAN *bHwRts, BOOLEAN *bErpProtect, BOOLEAN *bNProtect)
{
	*bRtsEnable = FALSE;
	*bCts2SelfEnable = FALSE;
	*bHwRts = FALSE;
	*bErpProtect = FALSE;
	*bNProtect = FALSE;

	if(!txcfg->pstat)
		return; 

#if (MU_BEAMFORMING_SUPPORT == 1)
	if( priv->pmib->dot11RFEntry.txbf_mu && txcfg->pstat->muPartner){
		*bRtsEnable = FALSE;
		*bCts2SelfEnable = FALSE;
		*bHwRts = FALSE;		
		return;
	}
#endif

	if ((txcfg->rts_thrshld <= get_mpdu_len(txcfg, fr_len)) ||
		(txcfg->pstat && txcfg->pstat->is_forced_rts))
		*bRtsEnable = TRUE;
	else {
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
			is_MCS_rate(txcfg->tx_rate) &&
			(priv->ht_protection /*|| txcfg->pstat->is_rtl8190_sta*/))
		{
			*bNProtect = 1;
			if (priv->pmib->dot11ErpInfo.protection)
				*bErpProtect = 1;
#ifdef MULTI_STA_REFINE 					
			if (priv->pshare->total_assoc_num > 10 )	{
				*bRtsEnable = FALSE;
				*bRtsEnable = TRUE;
			} else			
#endif			
			if (priv->pmib->dot11ErpInfo.ctsToSelf)
				*bCts2SelfEnable = TRUE;
			else					
				*bRtsEnable = TRUE;
				
		}
		else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
			(!is_CCK_rate(txcfg->tx_rate)) && // OFDM mode
			priv->pmib->dot11ErpInfo.protection)
		{
			*bErpProtect = 1;
			if (priv->pmib->dot11ErpInfo.ctsToSelf)
				*bCts2SelfEnable = TRUE;
			else						
				*bRtsEnable = TRUE;
			
		}
		else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
			(txcfg->pstat) && (txcfg->pstat->MIMO_ps & _HT_MIMO_PS_DYNAMIC_) &&
			(is_2T_rate(txcfg->tx_rate) || (txcfg->pstat->tx_avarage>=(3<<17))))
		{	// when HT MIMO Dynamic power save is set, RTS is needed
			*bRtsEnable = TRUE;
		
		} else {
			/*
			 * Auto rts mode, use rts depends on packet length and packet tx time
			 */

			if(is_MCS_rate(txcfg->tx_rate) && (txcfg->pstat->IOTPeer==HT_IOT_PEER_INTEL) && priv->pshare->rf_ft_var.rts_iot_th){

				*bNProtect = 1;
				if(!txcfg->pstat->no_rts){
					*bHwRts = TRUE;
					*bRtsEnable = TRUE;
				}
				if(txcfg->pstat->useCts2self)
					*bCts2SelfEnable = TRUE;					
			} else
			if (is_MCS_rate(txcfg->tx_rate) && ((txcfg->pstat->IOTPeer!=HT_IOT_PEER_INTEL) || !txcfg->pstat->no_rts)) 
			{
#ifdef COCHANNEL_RTS
					if (priv->pmib->miscEntry.auto_rts_mode == 1 && priv->cochannel_to)
					{
						*bNProtect = 1;
						*bRtsEnable = TRUE;
						*bCts2SelfEnable = FALSE;
						*bHwRts = FALSE;
					}
					else						
#endif
					{
						*bNProtect = 1;
						*bHwRts = TRUE;
						if(txcfg->pstat->useCts2self)
							*bCts2SelfEnable = TRUE;
						else
							*bRtsEnable = TRUE;
					}
			
			}
		}
	}
}

#ifdef RTK_AC_SUPPORT
static void RtsCheckAC(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, BOOLEAN *bRtsEnable, BOOLEAN *bCts2SelfEnable, BOOLEAN *bHwRts)
{
	if((priv->pshare->rf_ft_var.cca_rts) && (txcfg->pstat->vht_cap_len > 0)) {
		*bHwRts = FALSE;
		*bCts2SelfEnable = FALSE;
		*bRtsEnable = TRUE;
	}
}
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
static void rtl8192cd_fill_fwinfo_8812(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, struct tx_desc  *pdesc, unsigned int frag_idx)
{
	char n_txshort = 0, bg_txshort = 0;
	//int erp_protection = 0, n_protection = 0;
//	unsigned char rate;
	unsigned char txRate = 0;
#ifdef DRVMAC_LB
	static unsigned int rate_select = 0;
#endif
	BOOLEAN			bRtsEnable;
	BOOLEAN			bErpProtect;
	BOOLEAN			bNProtect;
	BOOLEAN			bHwRts;
	BOOLEAN			bCts2SelfEnable;
	unsigned char RtsRate;
	unsigned int	reduction_level = 0;

#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE) {
		#ifdef RTK_AC_SUPPORT
		if (is_VHT_rate(txcfg->tx_rate)) {
			txRate = txcfg->tx_rate - VHT_RATE_ID;
			txRate += 44;
		} else 
		#endif
		if (is_MCS_rate(txcfg->tx_rate)) {	// HT rates
			txRate = txcfg->tx_rate - HT_RATE_ID;
			txRate += 12;
		} else{
			txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);
		}

		if (priv->pshare->is_40m_bw == 2) {
			pdesc->Dword5 |= set_desc((0 << TXdesc_92E_DataScSHIFT) | (0 << TXdesc_92E_RtsScSHIFT));
			pdesc->Dword5 |= set_desc(0x2 << TXdesc_92E_DataBwSHIFT);

			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M)
				n_txshort = 1;
		} else if (priv->pshare->is_40m_bw == 1) {	
			pdesc->Dword5 |= set_desc((0 << TXdesc_92E_DataScSHIFT) | (0 << TXdesc_92E_RtsScSHIFT));
			pdesc->Dword5 |= set_desc(0x1 << TXdesc_92E_DataBwSHIFT);

			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
				n_txshort = 1;
		} else {
			pdesc->Dword5 |= set_desc((0 << TXdesc_92E_DataScSHIFT) | (0 << TXdesc_92E_RtsScSHIFT));
			pdesc->Dword5 |= set_desc(0 << TXdesc_92E_DataBwSHIFT);
			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M)
				n_txshort = 1;
		}

		if (txcfg->retry) {	
			pdesc->Dword4 |= set_desc(TXdesc_92E_RtyLmtEn);
            		pdesc->Dword4 |= set_desc((txcfg->retry  & TXdesc_92E_DataRtyLmtMask) << TXdesc_92E_DataRtyLmtSHIFT);
		}

		pdesc->Dword4 |= set_desc((txRate & TXdesc_92E_DataRateMask) << TXdesc_92E_DataRateSHIFT);
		
		
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask_8812) << TX_RtsRateSHIFT_8812);

		if(n_txshort == 1)
			pdesc->Dword5 |= set_desc(TXdesc_92E_DataShort);	

		return;
	}
#endif


	if (is_MCS_rate(txcfg->tx_rate))	// HT rates
	{
		if( txcfg->tx_rate >= VHT_RATE_ID)
		{
			txRate = (txcfg->tx_rate - VHT_RATE_ID) + 44;
		}
		else
		{
			txRate = (txcfg->tx_rate - HT_RATE_ID) + 12;
		}

		if (priv->pshare->is_40m_bw==2)
		{
//			get_txsc_AC(priv, priv->pmib->dot11RFEntry.dot11channel);
				
			#ifdef RTK_AC_SUPPORT 
			if(txcfg->pstat && is_VHT_rate(txcfg->tx_rate) && (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_80 
				&& (is_auto_rate(priv, txcfg->pstat) || is_fixedVHTTxRate(priv, txcfg->pstat))
			) )
			{
				pdesc->Dword5 |= set_desc((0 << TXdesc_92E_DataScSHIFT) | (0 << TXdesc_92E_RtsScSHIFT));
				pdesc->Dword5 |= set_desc(0x2 << TXdesc_92E_DataBwSHIFT);

				if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M &&
					txcfg->pstat && (txcfg->pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(BIT(5))))
						n_txshort = 1;
			}
			else
			#endif
			if(txcfg->pstat && is_MCS_rate(txcfg->tx_rate) && (txcfg->pstat->tx_bw >= HT_CHANNEL_WIDTH_20_40
					&& (is_auto_rate(priv, txcfg->pstat) || is_fixedMCSTxRate(priv, txcfg->pstat))		
				) )
			{
				#ifdef RTK_AC_SUPPORT
				if(GET_CHIP_VER(priv) != VERSION_8723B)
					pdesc->Dword5 |= set_desc((priv->pshare->txsc_40 << TXdesc_92E_DataScSHIFT) | (priv->pshare->txsc_40 << TXdesc_92E_RtsScSHIFT));
				#endif
				pdesc->Dword5 |= set_desc(0x1 << TXdesc_92E_DataBwSHIFT);

				if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M &&
					txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)))
						n_txshort = 1;
			}
			else
			{
				#ifdef RTK_AC_SUPPORT
				if(GET_CHIP_VER(priv) != VERSION_8723B)
					pdesc->Dword5 |= set_desc((priv->pshare->txsc_20 << TXdesc_92E_DataScSHIFT) | (priv->pshare->txsc_20 << TXdesc_92E_RtsScSHIFT));
				#endif
				if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
					txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
						n_txshort = 1;
			}
		}
		else
		if (priv->pshare->is_40m_bw) {
			if (txcfg->pstat && (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
				&& !(COEXIST_ENABLE && (((OPMODE & WIFI_AP_STATE) && 
				(priv->bg_ap_timeout || orForce20_Switch20Map(priv)
				))
#ifdef CLIENT_MODE
				|| ((OPMODE & WIFI_STATION_STATE) && priv->coexist_connection && 
				(txcfg->pstat->ht_ie_len) && !(txcfg->pstat->ht_ie_buf.info0 & _HTIE_STA_CH_WDTH_))
#endif
				))
#endif

				) {
					
				pdesc->Dword5 |= set_desc((1 << TXdesc_92E_DataBwSHIFT) | (3 << TXdesc_92E_DataScSHIFT));

				{
					if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M &&
						txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)))
						n_txshort = 1;
				}
			}
			else {
				if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
					pdesc->Dword5 |= set_desc((2 << TXdesc_92E_DataScSHIFT) | (2 << TXdesc_92E_RtsScSHIFT));
				else
					pdesc->Dword5 |= set_desc((1 << TXdesc_92E_DataScSHIFT) | (1 << TXdesc_92E_RtsScSHIFT));

				{
					if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
						txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
						n_txshort = 1;
				}
			}
		} else {
			{
				if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
					txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
					n_txshort = 1;
			}
		}

		
		if (
			( txcfg->pstat && 
			((txcfg->pstat->aggre_mthd == AGGRE_MTHD_MPDU_AMSDU) || (txcfg->pstat->aggre_mthd == AGGRE_MTHD_MPDU))
			&& txcfg->aggre_en )	|| 
			((txcfg->aggre_en >= FG_AGGRE_MPDU) && (txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST)) 
		){
			int TID = ((struct sk_buff *)txcfg->pframe)->cb[1];
			if (txcfg->pstat->ADDBA_ready[TID] && !txcfg->pstat->low_tp_disable_ampdu) {


					pdesc->Dword2 |= set_desc(TXdesc_92E_AggEn);


				/*
				 * assign aggr size
				 */

				// assign aggr density
				if (txcfg->privacy) {
					//8812_11n_iot, set TxAmpduDsty=7 for 20M WPA2
					if ((priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) && (!priv->pshare->is_40m_bw))
						pdesc->Dword2 |= set_desc(7 << TX_AmpduDstySHIFT);	// according to DWA-160 A2
					else
						pdesc->Dword2 |= set_desc(5 << TX_AmpduDstySHIFT);	// according to WN111v2
				}
				else {
					pdesc->Dword2 |= set_desc(((txcfg->pstat->ht_cap_buf.ampdu_para & _HTCAP_AMPDU_SPC_MASK_) >> _HTCAP_AMPDU_SPC_SHIFT_) << TX_AmpduDstySHIFT);
				}

                pdesc->Dword3 |= set_desc( ((txcfg->pstat->maxAggNum >> 1) & TXdesc_92E_MAX_AGG_NUMMask)<< TXdesc_92E_MAX_AGG_NUMSHIFT);      // MAX_AGG_NUM=4
                
				if(	((txcfg->pstat->current_tx_rate >= _MCS0_RATE_) && (txcfg->pstat->current_tx_rate <= _MCS2_RATE_)) 
				||((txcfg->pstat->current_tx_rate >= _MCS8_RATE_) && (txcfg->pstat->current_tx_rate <= _MCS10_RATE_)) 
#ifdef RTK_AC_SUPPORT					
				||((txcfg->pstat->current_tx_rate >= _NSS1_MCS0_RATE_) && (txcfg->pstat->current_tx_rate <= _NSS1_MCS2_RATE_)) 
				||((txcfg->pstat->current_tx_rate >= _NSS2_MCS0_RATE_) && (txcfg->pstat->current_tx_rate <= _NSS2_MCS2_RATE_))
#endif					
				)
					pdesc->Dword3 |= set_desc( (0x02 & TXdesc_92E_MAX_AGG_NUMMask)<< TXdesc_92E_MAX_AGG_NUMSHIFT);		// MAX_AGG_NUM=4
#ifdef MULTI_STA_REFINE 					
				else if (priv->pshare->total_assoc_num > 10 )
					pdesc->Dword3 |= set_desc( (0x08 & TXdesc_92E_MAX_AGG_NUMMask)<< TXdesc_92E_MAX_AGG_NUMSHIFT);		// MAX_AGG_NUM=16
#endif
			}
		}

		// for STBC
		if (priv->pmib->dot11nConfigEntry.dot11nSTBC &&	txcfg->pstat )	// 2012 10 31  for test
		{
			if((txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_RX_STBC_CAP_)) 
				#ifdef RTK_AC_SUPPORT
				||(txcfg->pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(_VHTCAP_RX_STBC_CAP_))
				#endif
			){

#if (BEAMFORMING_SUPPORT == 1)
				u1Byte					Idx = 0;
				PRT_BEAMFORMING_ENTRY	pEntry; 
				pEntry = Beamforming_GetEntryByMacId(priv, txcfg->pstat->aid, &Idx);
				if((pEntry == NULL) || (!pEntry->Sounding_En))
#endif 
				pdesc->Dword5 |= set_desc(1 << TXdesc_92E_DataStbcSHIFT);

			}

		}
	}
	else	// legacy rate
	{
		txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);
		if (is_CCK_rate(txcfg->tx_rate) && (txcfg->tx_rate != 2)) {
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
					(priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
				; // txfw->txshort = 0
			else {
				if (txcfg->pstat)
					bg_txshort = (priv->pmib->dot11RFEntry.shortpreamble) &&
									(txcfg->pstat->useShortPreamble);
				else
					bg_txshort = priv->pmib->dot11RFEntry.shortpreamble;
			}
		}
		#ifdef RTK_AC_SUPPORT	
		if (priv->pshare->is_40m_bw==2 && (GET_CHIP_VER(priv) != VERSION_8723B)) {
//			get_txsc_AC(priv, priv->pmib->dot11RFEntry.dot11channel);
			pdesc->Dword5 |= set_desc((priv->pshare->txsc_20 << TXdesc_92E_DataScSHIFT) | (priv->pshare->txsc_20  << TXdesc_92E_RtsScSHIFT));
		}
		else
		#endif
		if (priv->pshare->is_40m_bw) {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				pdesc->Dword5 |= set_desc((2 << TXdesc_92E_DataScSHIFT) | (2 << TXdesc_92E_RtsScSHIFT));
			else
				pdesc->Dword5 |= set_desc((1 << TXdesc_92E_DataScSHIFT) | (1 << TXdesc_92E_RtsScSHIFT));	
		}

		if (bg_txshort)
			pdesc->Dword5 |= set_desc(TXdesc_92E_DataShort);
			
	}
	#ifdef RTK_AC_SUPPORT
	if(AC_SIGMA_MODE != AC_SIGMA_NONE) //for 11ac logo
	if(txcfg->pstat)
	{
		txcfg->pstat->no_rts=0;
	}
	#endif
	if (txcfg->need_ack) { // unicast
		if (frag_idx == 0) {
#if defined(CONFIG_PCI_HCI)
			u4Byte fr_len = (get_desc(pdesc->Dword0)&TX_PktSizeMask) - (get_desc(pdesc->Dword7)&TX_TxBufSizeMask);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			u4Byte fr_len = txcfg->fr_len;	// for backward compatibile
#endif
			RtsCheck(priv, txcfg, fr_len, &bRtsEnable, &bCts2SelfEnable, &bHwRts, &bErpProtect, &bNProtect);
		}
	}

	RtsRate = find_rts_rate(priv, txcfg->tx_rate, bErpProtect);
	if(bRtsEnable && CheckCts2SelfEnable(RtsRate))
	{
		bRtsEnable = FALSE;
		bCts2SelfEnable = TRUE;
	}

#ifdef COCHANNEL_RTS
	if (priv->pmib->miscEntry.auto_rts_mode == 2)
	{
		bRtsEnable = FALSE;
		bCts2SelfEnable = FALSE;
		bHwRts = FALSE;
	}
	else if (priv->pmib->miscEntry.auto_rts_mode == 3)
	{
		bRtsEnable = TRUE;
		bCts2SelfEnable = FALSE;
		bHwRts = FALSE;
	}
#endif

	#ifdef RTK_AC_SUPPORT
	if(txcfg->pstat && (GET_CHIP_VER(priv) != VERSION_8723B)) //8812_11n_iot, only vht clnt support cca_rts, //for 11ac logo
		RtsCheckAC(priv, txcfg, &bRtsEnable, &bCts2SelfEnable, &bHwRts);
	#endif
	if(bRtsEnable)
		pdesc->Dword3 |= set_desc(TX_RtsEn);	
	if(bCts2SelfEnable)
		pdesc->Dword3 |= set_desc(TX_CTS2Self);	
	if(bHwRts)
		pdesc->Dword3 |= set_desc(TX_HwRtsEn);	

	if (bRtsEnable || bCts2SelfEnable ) {

			unsigned int rtsTxRateIdx   = get_rate_index_from_ieee_value(RtsRate);
			if (bErpProtect) {
				unsigned char  rtsShort = 0;
				if (is_CCK_rate(RtsRate) && (RtsRate != 2)) {
					if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
							(priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
						rtsShort = 0; // do nothing
					else {
						if (txcfg->pstat)
							rtsShort = (priv->pmib->dot11RFEntry.shortpreamble) &&
											(txcfg->pstat->useShortPreamble);
						else
							rtsShort = priv->pmib->dot11RFEntry.shortpreamble;
					}
				}
				pdesc->Dword5 |= (rtsShort == 1)? set_desc(TXdesc_92E_RtsShort): 0;
				
			} 
			pdesc->Dword4 |= set_desc((rtsTxRateIdx & TXdesc_92E_RtsRateMask) << TXdesc_92E_RtsRateSHIFT);
			pdesc->Dword4 |= set_desc((0xf & TXdesc_92E_RtsRateFBLmtMask) << TXdesc_92E_RtsRateFBLmtSHIFT);
				
	}

	if(txcfg->pstat && priv->pshare->rf_ft_var.txforce != 0xFF)
	{
		if(priv->pshare->rf_ft_var.sgiforce == 1)
			n_txshort = 1;
		else if(priv->pshare->rf_ft_var.sgiforce == 0)
			n_txshort = 0;
	}

	if(n_txshort == 1)
	if((txcfg->pstat->sta_in_firmware == 1) || (priv->pshare->rf_ft_var.sgiforce != 0xFF))
		pdesc->Dword5 |= set_desc(TXdesc_92E_DataShort);		
	

#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb && (priv->pmib->miscEntry.lb_mlmp == 4)) {
		txRate = rate_select;
		if (rate_select++ > 0x1b)
			rate_select = 0;

		pdesc->Dword3 |= set_desc(TXdesc_92E_DisDataFB|TXdesc_92E_DisRtsFB|TXdesc_92E_UseRate);
	}
#endif


	if((priv->pshare->rf_ft_var.txforce != 0xff)
#if (BEAMFORMING_SUPPORT == 1)
		&& (!txcfg->ndpa) 
#endif
	) {
				pdesc->Dword3 |= set_desc(TXdesc_92E_DisDataFB|TXdesc_92E_DisRtsFB|TXdesc_92E_UseRate);
				pdesc->Dword4 |= set_desc((priv->pshare->rf_ft_var.txforce & TXdesc_92E_DataRateMask) << TXdesc_92E_DataRateSHIFT);
	}
	else
	pdesc->Dword4 |= set_desc((txRate & TXdesc_92E_DataRateMask) << TXdesc_92E_DataRateSHIFT);


#if 1
	if (priv->pshare->rf_ft_var.rts_init_rate) {
		pdesc->Dword4 &= set_desc(~(TX_RtsRateMask_8812 << TX_RtsRateSHIFT_8812));
		pdesc->Dword4 |= set_desc(((priv->pshare->rf_ft_var.rts_init_rate) & TX_RtsRateMask_8812) << TX_RtsRateSHIFT_8812);
	}		
	if ((priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) && 
		(TX_RtsRateMask_8812&(get_desc(pdesc->Dword4)>>TX_RtsRateSHIFT_8812)) <4	)
		pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask_8812) << TX_RtsRateSHIFT_8812);
#else		
	if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
		pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask_8812) << TX_RtsRateSHIFT_8812);

#endif

	if (txcfg->need_ack) {
		// give retry limit to management frame
		if (txcfg->q_num == MANAGE_QUE_NUM) {
			pdesc->Dword4 |= set_desc(TXdesc_92E_RtyLmtEn);
			
			if (GetFrameSubType(txcfg->phdr) == WIFI_PROBERSP) {
				;	// 0 no need to set
			}
#ifdef WDS
			else if ((GetFrameSubType(txcfg->phdr) == WIFI_PROBEREQ) && (txcfg->pstat->state & WIFI_WDS)) {
				pdesc->Dword4 |= set_desc((2 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
			}
#endif
			else {
				pdesc->Dword4 |= set_desc((6 & TXdesc_92E_DataRtyLmtMask) << TXdesc_92E_DataRtyLmtSHIFT);
			}
		}
#ifdef WDS
		else if (txcfg->wdsIdx >= 0) {
			if (txcfg->pstat->rx_avarage == 0) {
				pdesc->Dword4 |= set_desc(TX_RtyLmtEn);
				pdesc->Dword4 |= set_desc((3 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
			}
		}
#endif
#ifdef MULTI_STA_REFINE
		else if( priv->pshare->total_assoc_num > 30) {
		        pdesc->Dword4 |= set_desc(TXdesc_92E_RtyLmtEn);
                	pdesc->Dword4 |= set_desc((0x0a & TXdesc_92E_DataRtyLmtMask) << TXdesc_92E_DataRtyLmtSHIFT);
		}
#endif
		else if (is_MCS_rate(txcfg->pstat->current_tx_rate) && (txcfg->pstat->IOTPeer==HT_IOT_PEER_INTEL) && (txcfg->pstat->retry_inc)
			&& !(txcfg->pstat->leave) && priv->pshare->intel_rty_lmt) {			
			pdesc->Dword4 |= set_desc(TXdesc_92E_RtyLmtEn);
			pdesc->Dword4 |= set_desc((priv->pshare->intel_rty_lmt & TXdesc_92E_DataRtyLmtMask) << TXdesc_92E_DataRtyLmtSHIFT);
		}

		else if ((txcfg->pstat->IOTPeer==HT_IOT_PEER_BROADCOM) && (txcfg->pstat->retry_inc) && !(txcfg->pstat->leave)) {
			pdesc->Dword4 |= set_desc(TXdesc_92E_RtyLmtEn);
    		pdesc->Dword4 |= set_desc((0x20 & TXdesc_92E_DataRtyLmtMask) << TXdesc_92E_DataRtyLmtSHIFT);
        }

		if (priv->pshare->rf_ft_var.tx_pwr_ctrl && txcfg->pstat && (txcfg->fr_type == _SKB_FRAME_TYPE_)) {
			if (txcfg->pstat->hp_level == 1) {
				pdesc->Dword5 |= set_desc((priv->pshare->phw->TXPowerOffset & TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT);
			}
		}	

#if (BEAMFORMING_SUPPORT == 1)
		if(txcfg->ndpa) {
			unsigned char *pFrame = (unsigned char*)txcfg->phdr;
			if(IsCtrlNDPA(pFrame) || IsMgntNDPA(pFrame)) {
				//SET_TX_DESC_DATA_RETRY_LIMIT_8812(pDesc, 5);
				//SET_TX_DESC_RETRY_LIMIT_ENABLE_8812(pDesc, 1);
				pdesc->Dword4 &= set_desc(~(TXdesc_92E_DataRtyLmtMask << TXdesc_92E_DataRtyLmtSHIFT));									
	            pdesc->Dword4 |= set_desc(TXdesc_92E_RtyLmtEn|((0x05 & TXdesc_92E_DataRtyLmtMask) << TXdesc_92E_DataRtyLmtSHIFT));	

					if(IsMgntNDPA(pFrame))		//0xe0
					{
						pdesc->Dword3 |= set_desc((1 &TXdesc_92E_NDPAMASK)<<TXdesc_92E_NDPASHIFT);
					}	
					else		// 0x54
					{
						if(!IS_TEST_CHIP(priv))
						{
							if ((txcfg->pstat) && (txcfg->pstat->WirelessMode & WIRELESS_MODE_AC_5G) && (txcfg->pstat->IOTPeer!=HT_IOT_PEER_BROADCOM))
								pdesc->Dword3 |= set_desc((2 &TXdesc_92E_NDPAMASK)<<TXdesc_92E_NDPASHIFT);
							else
								pdesc->Dword3 |= set_desc((1 &TXdesc_92E_NDPAMASK)<<TXdesc_92E_NDPASHIFT);
						}
						else
						{
							pdesc->Dword3 |= set_desc((1 &TXdesc_92E_NDPAMASK)<<TXdesc_92E_NDPASHIFT);				
						}
					}	
	//				panic_printk("LINE:%d, %x\n", __LINE__, get_desc(pdesc->Dword3));
			}
		} else
#endif

		{
			if((priv->pmib->dot11nConfigEntry.dot11nLDPC) && (txcfg->pstat) && (!txcfg->pstat->disable_ldpc) &&
			((txcfg->pstat->ht_cap_len && cpu_to_le16(txcfg->pstat->ht_cap_buf.ht_cap_info) & _HTCAP_SUPPORT_RX_LDPC_)
		#ifdef RTK_AC_SUPPORT
			  || (txcfg->pstat->vht_cap_len && (cpu_to_le32(txcfg->pstat->vht_cap_buf.vht_cap_info) & BIT(RX_LDPC_E)))
		#endif
			  )	
			) {
				pdesc->Dword5 |= set_desc(TXdesc_92E_DataLDPC);	
			}
		}
	}

	if((txcfg->q_num == MANAGE_QUE_NUM) && (txcfg->fr_type == _PRE_ALLOCMEM_) && (GetFrameSubType(txcfg->phdr) == WIFI_PROBERSP))
	{
		if(priv->pmib->dot11RFEntry.bcnagc==1)  {
			if(priv->pshare->rf_ft_var.bcn_pwr_idex+6 <= priv->pshare->rf_ft_var.bcn_pwr_max)
				pdesc->Dword5 |= set_desc((4 & TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT);
		}
		else if ( priv->pmib->dot11RFEntry.bcnagc==2) {
			if(priv->pshare->rf_ft_var.bcn_pwr_idex+12 <= priv->pshare->rf_ft_var.bcn_pwr_max)
				pdesc->Dword5 |= set_desc((5 & TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT);
			else if(priv->pshare->rf_ft_var.bcn_pwr_idex+6 <=priv->pshare->rf_ft_var.bcn_pwr_max)
				pdesc->Dword5 |= set_desc((4 & TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT);				
		}
	} 
	if (priv->pmib->dot11RFEntry.txpwr_reduction) {
		if (priv->pmib->dot11RFEntry.txpwr_reduction <= 3) {
			if (reduction_level < priv->pmib->dot11RFEntry.txpwr_reduction) {
				reduction_level = priv->pmib->dot11RFEntry.txpwr_reduction;
				pdesc->Dword5 &= set_desc(~((TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT));
				pdesc->Dword5 |= set_desc((reduction_level & TXdesc_8812_TxPwrOffetMask) << TXdesc_8812_TxPwrOffetSHIFT);
			}
		}
	}
}

#endif

#ifdef CONFIG_WLAN_HAL
static void 
rtl88XX_fill_fwinfo(
    struct rtl8192cd_priv   *priv, 
    struct tx_insn          *txcfg, 
    unsigned int            frag_idx, 
    PTX_DESC_DATA_88XX      pdesc_data
)
{
    char n_txshort = 0, bg_txshort = 0;
//    int erp_protection = 0, n_protection = 0;
//    unsigned char rate;
    unsigned char txRate = 0;
#ifdef DRVMAC_LB
    static unsigned int rate_select = 0;
#endif
//	BOOLEAN			bRtsEnable;
	BOOLEAN			bErpProtect;
	BOOLEAN			bNProtect;
//	BOOLEAN			bHwRts;
//	BOOLEAN			bCts2SelfEnable;
	unsigned char RtsRate;

#ifdef MP_TEST
    if (OPMODE & WIFI_MP_STATE) {
    #ifdef RTK_AC_SUPPORT
        if (is_VHT_rate(txcfg->tx_rate)) {
            txRate = (txcfg->tx_rate - VHT_RATE_ID) + 44;
        } 
        else 
    #endif            
        if (is_MCS_rate(txcfg->tx_rate)) {  // HT rates
            txRate = (txcfg->tx_rate - HT_RATE_ID) + 12;
        } 
        else{
            txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);
        }

        if (priv->pshare->is_40m_bw == 2) {
            pdesc_data->dataSC  = 0x0;
            pdesc_data->RTSSC   = 0x0;
            pdesc_data->dataBW  = 0x2;

            if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M)
                n_txshort = 1;
        } 
        else if (priv->pshare->is_40m_bw == 1) {
            pdesc_data->dataSC  = 0x0;
            pdesc_data->RTSSC   = 0x0;
            pdesc_data->dataBW  = 0x1;

            if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
                n_txshort = 1;
        } 
        else {
            pdesc_data->dataSC  = 0x0;
            pdesc_data->RTSSC   = 0x0;
            pdesc_data->dataBW  = 0x0;
            
            if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M)
                n_txshort = 1;
        }

        if (txcfg->retry) { 
            pdesc_data->rtyLmtEn    = TRUE;
            pdesc_data->dataRtyLmt  = txcfg->retry;
        }

        pdesc_data->dataRate = txRate;
        if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
            pdesc_data->RTSRate = 4;

		if(n_txshort == 1)
			pdesc_data->dataShort = 1;
		
        return;
    }
#endif

    if (is_MCS_rate(txcfg->tx_rate))    // HT rates
    {
#ifdef RTK_AC_SUPPORT
        if(is_VHT_rate(txcfg->tx_rate))
        {
            txRate = (txcfg->tx_rate - VHT_RATE_ID) + 44;
        }
        else
#endif
        {
            txRate = (txcfg->tx_rate - HT_RATE_ID) + 12;
        }

#ifdef RTK_AC_SUPPORT
        if (priv->pshare->is_40m_bw==2)
        {
            if(txcfg->pstat && (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_80
                && (is_auto_rate(priv, txcfg->pstat) || is_fixedVHTTxRate(priv, txcfg->pstat))
            ) )
            {
                pdesc_data->dataSC = 0;
                pdesc_data->RTSSC = 0;
                pdesc_data->dataBW = 2;                
                if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M &&
                    (txcfg->pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(BIT(5))))
                        n_txshort = 1;
            } // TODO: Pedro, in 8812: is_MCS_rate(txcfg->tx_rate)
            else if(txcfg->pstat && (txcfg->pstat->tx_bw >= HT_CHANNEL_WIDTH_20_40
                    && (is_auto_rate(priv, txcfg->pstat) || is_fixedMCSTxRate(priv, txcfg->pstat))        
                ) )
            {
                pdesc_data->dataSC = priv->pshare->txsc_40;
                pdesc_data->RTSSC  = priv->pshare->txsc_40;
                pdesc_data->dataBW = 1;
                if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M &&
                    (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)))
                        n_txshort = 1;
            }
            else
            {
                pdesc_data->dataSC = priv->pshare->txsc_20;
                pdesc_data->RTSSC  = priv->pshare->txsc_20;
                if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
                    txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
                        n_txshort = 1;
            }
        }
        else
#endif
        if (priv->pshare->is_40m_bw) {
            if (txcfg->pstat && (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
                && !(COEXIST_ENABLE && (((OPMODE & WIFI_AP_STATE) && 
                (priv->bg_ap_timeout || orForce20_Switch20Map(priv)
                ))
#ifdef CLIENT_MODE
                || ((OPMODE & WIFI_STATION_STATE) && priv->coexist_connection && 
                (txcfg->pstat->ht_ie_len) && !(txcfg->pstat->ht_ie_buf.info0 & _HTIE_STA_CH_WDTH_))
#endif
                ))
#endif

                ) {
                pdesc_data->dataBW = 1;
                pdesc_data->dataSC = 3;
                {
                    if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M &&
                        (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)))
                        n_txshort = 1;
                }
            }
            else {
                if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW) {
                    pdesc_data->dataSC = 2;
                    pdesc_data->RTSSC  = 2;
                } else {
                    pdesc_data->dataSC = 1;
                    pdesc_data->RTSSC  = 1;
                }

                {
                    if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
                        txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
                        n_txshort = 1;
                }
            }
        } else {
            {
                if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
                    txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
                    n_txshort = 1;
            }
        }

        
        if (
            ( txcfg->pstat && 
                ((txcfg->pstat->aggre_mthd == AGGRE_MTHD_MPDU_AMSDU) || (txcfg->pstat->aggre_mthd == AGGRE_MTHD_MPDU))
                && txcfg->aggre_en ) || 
            ((txcfg->aggre_en >= FG_AGGRE_MPDU) && (txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST)) 
        ){
            int TID = ((struct sk_buff *)txcfg->pframe)->cb[1];
            if (txcfg->pstat->ADDBA_ready[TID] && !txcfg->pstat->low_tp_disable_ampdu) {
                pdesc_data->aggEn = TRUE;

                /*
                 * assign aggr size
                 */

#ifdef CONFIG_WLAN_HAL
                // TODO: 這裡是指非AC chip ???
                // TODO: check 是否有在其他位置..
#else
                if(GET_CHIP_VER(priv) != VERSION_8812E)
                {
                     if (priv->pshare->rf_ft_var.diffAmpduSz) {
                        pdesc->Dword6 |= set_desc((txcfg->pstat->diffAmpduSz & 0xffff) << TX_MCS1gMaxSHIFT | TX_UseMaxLen);
                        
                        if (GET_CHIP_VER(priv)!=VERSION_8812E)     
                        pdesc->Dword7 |= set_desc(txcfg->pstat->diffAmpduSz & 0xffff0000);
                     }  
                }
#endif // CONFIG_WLAN_HAL

                // assign aggr density
                if (txcfg->privacy) {
#if 1   // TODO: for test two STA TP
#if defined(CONFIG_WLAN_HAL_8814AE) 			
					if ((GET_CHIP_VER(priv) == VERSION_8814A))
						pdesc_data->ampduDensity = ((txcfg->pstat->ht_cap_buf.ampdu_para & _HTCAP_AMPDU_SPC_MASK_) >> _HTCAP_AMPDU_SPC_SHIFT_);
					else
#endif	
#if defined(CONFIG_WLAN_HAL_8822BE) 			
					if ((GET_CHIP_VER(priv) == VERSION_8822B)) //eric-wpa2
						pdesc_data->ampduDensity = 4;
					else
#endif	
                    pdesc_data->ampduDensity = 7;
#else
                    pdesc_data->ampduDensity = 5;
#endif
                }
                else {
#if 0   // TODO: for test two STA TP
                    pdesc_data->ampduDensity = 5;
#else
					pdesc_data->ampduDensity = ((txcfg->pstat->ht_cap_buf.ampdu_para & _HTCAP_AMPDU_SPC_MASK_) >> _HTCAP_AMPDU_SPC_SHIFT_);
#endif
                }
				pdesc_data->maxAggNum = txcfg->pstat->maxAggNum >> 1;

				if(priv->pshare->rf_ft_var.txforce == 0xff) 
				if(	((txcfg->pstat->current_tx_rate >= _MCS0_RATE_) && (txcfg->pstat->current_tx_rate <= _MCS2_RATE_)) 
					||((txcfg->pstat->current_tx_rate >= _MCS8_RATE_) && (txcfg->pstat->current_tx_rate <= _MCS10_RATE_)) 
#if 0//def RTK_AC_SUPPORT						
					||((txcfg->pstat->current_tx_rate >= _NSS1_MCS0_RATE_) && (txcfg->pstat->current_tx_rate <= _NSS1_MCS2_RATE_)) 
					||((txcfg->pstat->current_tx_rate >= _NSS2_MCS0_RATE_) && (txcfg->pstat->current_tx_rate <= _NSS2_MCS2_RATE_))
#endif						
				)
					pdesc_data->maxAggNum = 2;

#if defined(CONFIG_WLAN_HAL_8814AE) 	// MAC HW TxRPT lost for 8814A, truncate aggregation number to 30 in small RSSI
				if (txcfg->pstat->rssi < 33) {
					if (pdesc_data->maxAggNum>15)
						pdesc_data->maxAggNum=15;
				}
#endif		
#if 1 //eric-8822 tp
				if(priv->pshare->rf_ft_var.aggforce)
					pdesc_data->maxAggNum = priv->pshare->rf_ft_var.aggforce;
#endif

#ifdef MULTI_STA_REFINE 					
				{
					if (txcfg->tx_rate < STA_TX_LOWRATE && pdesc_data->maxAggNum>2)
						pdesc_data->maxAggNum = 2;
				}		
#ifdef SW_TX_QUEUE
				if (priv->pshare->swq_numActiveSTA > LOWAGGRESTA && pdesc_data->maxAggNum>4) {
					if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
						pdesc_data->maxAggNum = 8;			// max agg# 16	
					else
						pdesc_data->maxAggNum = 4;			// max agg# 8
				}
#endif						
#endif	
            }
        }

        // for STBC
        if (priv->pmib->dot11nConfigEntry.dot11nSTBC && (txcfg->pstat) &&
			((txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_RX_STBC_CAP_)) 
#ifdef RTK_AC_SUPPORT			
				|| (txcfg->pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(_VHTCAP_RX_STBC_CAP_))
#endif
		)){
#if (BEAMFORMING_SUPPORT == 1)
			u1Byte					Idx = 0;	

			PRT_BEAMFORMING_ENTRY	pEntry;
			pEntry = Beamforming_GetEntryByMacId(priv, txcfg->pstat->aid, &Idx);
			if(pEntry == NULL || pEntry->Sounding_En == 0)
#endif
			if((get_rf_mimo_mode(priv) == MIMO_2T2R) || (get_rf_mimo_mode(priv) == MIMO_3T3R))
           		pdesc_data->dataStbc = 1;
        }
		// LDPC
#if (BEAMFORMING_SUPPORT == 1)
		if(!txcfg->ndpa) 
#endif		
		if((priv->pmib->dot11nConfigEntry.dot11nLDPC) && (txcfg->pstat) && (!txcfg->pstat->disable_ldpc) &&
		((txcfg->pstat->ht_cap_len && cpu_to_le16(txcfg->pstat->ht_cap_buf.ht_cap_info) & _HTCAP_SUPPORT_RX_LDPC_)
#ifdef RTK_AC_SUPPORT		
		||	(txcfg->pstat->vht_cap_len && (cpu_to_le32(txcfg->pstat->vht_cap_buf.vht_cap_info) & BIT(RX_LDPC_E)))
#endif		
		)) {		
				pdesc_data->dataLdpc = 1;
		}
    }
    else    // legacy rate
    {
        txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);
        if (is_CCK_rate(txcfg->tx_rate) && (txcfg->tx_rate != 2)) {
            if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
                    (priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
                ; // txfw->txshort = 0
            else {
                if (txcfg->pstat)
                    bg_txshort = (priv->pmib->dot11RFEntry.shortpreamble) &&
                                    (txcfg->pstat->useShortPreamble);
                else
                    bg_txshort = priv->pmib->dot11RFEntry.shortpreamble;
            }
        }

#ifdef RTK_AC_SUPPORT
        if (priv->pshare->is_40m_bw==2) {
            pdesc_data->dataSC = priv->pshare->txsc_20;
            pdesc_data->RTSSC  = priv->pshare->txsc_20;
        }
        else
#endif
        if (priv->pshare->is_40m_bw) {
            if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW) {
                pdesc_data->dataSC = 2;
                pdesc_data->RTSSC  = 2;
            } else {
                pdesc_data->dataSC = 1;
                pdesc_data->RTSSC  = 1;
            }
        }

        if (bg_txshort) {
            pdesc_data->dataShort = 1;
        }
            
    }
	
#if defined(CONFIG_WLAN_HAL_8192EE)
	if (GET_CHIP_VER(priv)==VERSION_8192E) {
		if( priv->pmib->dot11RFEntry.bcn2path )
			pdesc_data->TXAnt = 1; // use path A for 1ss CCK rate tx
		else if( (txcfg->pstat) && (txcfg->pstat->IOTPeer==HT_IOT_PEER_BROADCOM)&& (txcfg->pstat->ratr_idx == 1))
			pdesc_data->TXAnt = 1;
		else if( priv->pmib->dot11RFEntry.tx2path )
			pdesc_data->TXAnt = 3;
		else
			pdesc_data->TXAnt = 1;
	}
#endif

    if (txcfg->need_ack) { // unicast
        if (frag_idx == 0) { // TODO: ..........how to do for hw tx shortcut (8814)
			u4Byte fr_len = pdesc_data->frLen;
			RtsCheck(priv, txcfg, fr_len, &pdesc_data->RTSEn, &pdesc_data->CTS2Self, &pdesc_data->HWRTSEn, &bErpProtect, &bNProtect);
		}
	}

#ifdef RTK_AC_SUPPORT 
	if(txcfg->pstat) //8812_11n_iot, only vht clnt support cca_rts, //for 11ac logo
		RtsCheckAC(priv, txcfg, &pdesc_data->RTSEn, &pdesc_data->CTS2Self, &pdesc_data->HWRTSEn);

	pdesc_data->cca_rts = priv->pshare->rf_ft_var.cca_rts; 

	if(priv->pshare->rf_ft_var.no_rtscts){
		pdesc_data->RTSEn		= 0;
		pdesc_data->CTS2Self    = 0;
		pdesc_data->HWRTSEn     = 0;
	}	
#endif
	
	RtsRate = find_rts_rate(priv, txcfg->tx_rate, bErpProtect);
	if(pdesc_data->RTSEn && CheckCts2SelfEnable(RtsRate))
	{
		pdesc_data->RTSEn = FALSE;
		pdesc_data->CTS2Self = TRUE;
	}

#ifdef COCHANNEL_RTS
	if (priv->pmib->miscEntry.auto_rts_mode == 2)
	{
		pdesc_data->RTSEn = FALSE;
		pdesc_data->CTS2Self = FALSE;
		pdesc_data->HWRTSEn = FALSE;
	}
	else if (priv->pmib->miscEntry.auto_rts_mode == 3)
	{
		pdesc_data->RTSEn = TRUE;
		pdesc_data->CTS2Self = FALSE;
		pdesc_data->HWRTSEn = FALSE;
	}
#endif

    if (pdesc_data->CTS2Self || pdesc_data->RTSEn
#if (MU_BEAMFORMING_SUPPORT == 1)
		|| (priv->pmib->dot11RFEntry.txbf_mu && txcfg->pstat && txcfg->pstat->muPartner)
#endif	
	) 
    {
            unsigned int rtsTxRateIdx   = get_rate_index_from_ieee_value(RtsRate);
            if (bErpProtect) {
                unsigned char  rtsShort = 0;
                if (is_CCK_rate(RtsRate) && (RtsRate != 2)) {
                    if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
                        (priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
                        rtsShort = 0; // do nothing
                    else {
                        if (txcfg->pstat)
                            rtsShort = (priv->pmib->dot11RFEntry.shortpreamble) &&
                                            (txcfg->pstat->useShortPreamble);
                        else
                            rtsShort = priv->pmib->dot11RFEntry.shortpreamble;
                    }
                }
                pdesc_data->RTSShort = (rtsShort == 1) ? TRUE : FALSE;                
            } 

            pdesc_data->RTSRate = rtsTxRateIdx;
            if (GET_CHIP_VER(priv) >= VERSION_8814A) {
                if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
                    pdesc_data->RTSRateFBLmt = 4;
                } else {
                    pdesc_data->RTSRateFBLmt = 0;
                }
            } else            
                pdesc_data->RTSRateFBLmt = 0xf;

    }

	if(txcfg->pstat && priv->pshare->rf_ft_var.txforce != 0xFF)
	{
		if(priv->pshare->rf_ft_var.sgiforce == 1)
			n_txshort = 1;
		else if(priv->pshare->rf_ft_var.sgiforce == 0)
			n_txshort = 0;
	}
    
	if(n_txshort == 1)
	if((txcfg->pstat->sta_in_firmware == 1) || (priv->pshare->rf_ft_var.sgiforce != 0xFF))    
        pdesc_data->dataShort = 1;
    

#ifdef DRVMAC_LB
    if (priv->pmib->miscEntry.drvmac_lb && (priv->pmib->miscEntry.lb_mlmp == 4)) {
        txRate = rate_select;
        if (rate_select++ > 0x1b)
            rate_select = 0;

        pdesc_data->disDataFB = TRUE;
        pdesc_data->disRTSFB  = TRUE;
        pdesc_data->useRate   = TRUE;
    }
#endif

	if((priv->pshare->rf_ft_var.txforce != 0xff || (txcfg->pstat && txcfg->pstat->force_rate)) 
#if (BEAMFORMING_SUPPORT == 1)
		&& (!txcfg->ndpa) 
#endif
	){
        pdesc_data->useRate  = TRUE;
		if(txcfg->pstat && txcfg->pstat->force_rate)
			pdesc_data->dataRate = 	txcfg->pstat->force_rate;
		else
	        pdesc_data->dataRate = priv->pshare->rf_ft_var.txforce;       
	} else {
        pdesc_data->dataRate = txRate;
    }


#if 1
    if (priv->pshare->rf_ft_var.rts_init_rate) {
        pdesc_data->RTSRate = priv->pshare->rf_ft_var.rts_init_rate;
    }      
    if ((priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) && 
        (pdesc_data->RTSRate)<4) {
        pdesc_data->RTSRate = 4;
    }
#else		
    if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
        pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask_8812) << TX_RtsRateSHIFT_8812);

#endif

    if (txcfg->need_ack) {
        // give retry limit to management frame
        if (txcfg->q_num == MANAGE_QUE_NUM) {
            pdesc_data->rtyLmtEn = TRUE;
            // TODO: ???? for hw tx sc... how to do
            if (GetFrameSubType(txcfg->phdr) == WIFI_PROBERSP) {
                ;   // 0 no need to set
            }
            #ifdef WDS
            else if ((GetFrameSubType(txcfg->phdr) == WIFI_PROBEREQ) && (txcfg->pstat->state & WIFI_WDS)) {
                pdesc_data->dataRtyLmt = 2;
            }
            #endif
            else {
                pdesc_data->dataRtyLmt = 6;
            }
        }
        #ifdef WDS
        else if (txcfg->wdsIdx >= 0) {
            if (txcfg->pstat->rx_avarage == 0) {
                pdesc_data->rtyLmtEn = TRUE;
                pdesc_data->dataRtyLmt = 3;
            }
        }
        #endif
        else if(txcfg->pstat)
        {
#if defined(CONFIG_PCI_HCI)
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
            if (priv->pshare->rf_ft_var.low_retry_rssi_thd && txcfg->pstat->rssi
                && (txcfg->pstat->rssi < priv->pshare->rf_ft_var.low_retry_rssi_thd)) {
                if ((txcfg->pstat->current_tx_rate <= _9M_RATE_) ||
                    ((_MCS0_RATE_ == txcfg->pstat->current_tx_rate) || (_MCS1_RATE_ == txcfg->pstat->current_tx_rate))) {
                    pdesc_data->rtyLmtEn = TRUE;
                    pdesc_data->dataRtyLmt = 0x04;
                } else {
                    pdesc_data->rtyLmtEn = TRUE;
                    pdesc_data->dataRtyLmt = 0x0c;
                }
            } else if (priv->pshare->rf_ft_var.low_rate_low_retry
                    && ((txcfg->pstat->current_tx_rate <= _9M_RATE_)
                    || (_MCS0_RATE_ == txcfg->pstat->current_tx_rate) || (_MCS1_RATE_ == txcfg->pstat->current_tx_rate))) {
                pdesc_data->rtyLmtEn = TRUE;
                pdesc_data->dataRtyLmt = 0x08;
            } else
#endif
            if (is_MCS_rate(txcfg->pstat->current_tx_rate) && (txcfg->pstat->IOTPeer==HT_IOT_PEER_INTEL) && (txcfg->pstat->retry_inc)
                && !(txcfg->pstat->leave) && priv->pshare->intel_rty_lmt) {        
                pdesc_data->rtyLmtEn = TRUE;
                pdesc_data->dataRtyLmt = priv->pshare->intel_rty_lmt;
            } else if ((txcfg->pstat->IOTPeer==HT_IOT_PEER_BROADCOM) && (txcfg->pstat->retry_inc) && !(txcfg->pstat->leave)) {
                pdesc_data->rtyLmtEn = TRUE;
                pdesc_data->dataRtyLmt = 0x20;
            }
            #if defined(MULTI_STA_REFINE)
            else if((priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) && (priv->pshare->total_assoc_num > 30)) {
                pdesc_data->rtyLmtEn = TRUE;
                pdesc_data->dataRtyLmt = 0x0a;				 
            }
            #endif
        }

#ifdef THERMAL_CONTROL	
		/* fill in descriptor with -11dB if low_poewr = 1*/
		if(priv->pshare->rf_ft_var.low_power == 1){ /* decided by STA*/
			if(txcfg->pstat && txcfg->fr_type == _SKB_FRAME_TYPE_ && txcfg->pstat->power <= 3){
				pdesc_data->TXPowerOffset = txcfg->pstat->power;
			}
		}else if(priv->pshare->rf_ft_var.low_power == 2){ /* decided by power_desc*/		
			if(txcfg->pstat && txcfg->fr_type == _SKB_FRAME_TYPE_ && priv->pshare->rf_ft_var.current_power_desc <= 3){
				pdesc_data->TXPowerOffset = priv->pshare->rf_ft_var.current_power_desc;
			}
		}else
#endif		
		{		

		// High power
		if (priv->pshare->rf_ft_var.tx_pwr_ctrl && txcfg->pstat && (txcfg->fr_type == _SKB_FRAME_TYPE_)) {
			if (txcfg->pstat->hp_level == 1)
				pdesc_data->TXPowerOffset = priv->pshare->phw->TXPowerOffset;
//eric-7260
#if defined(CONFIG_WLAN_HAL_8822BE) && defined(TXPWR_LMT)
			else if((GET_CHIP_VER(priv)==VERSION_8822B) && (txcfg->pstat->IOTPeer==HT_IOT_PEER_INTEL) && (priv->pshare->rf_ft_var.disable_txpwrlmt)){
				if((txcfg->pstat->current_tx_rate >= _NSS2_MCS3_RATE_) || ((txcfg->pstat->current_tx_rate >= _NSS1_MCS6_RATE_)&&(txcfg->pstat->current_tx_rate <= _NSS1_MCS9_RATE_)))
				{		 
					pdesc_data->TXPowerOffset = 1;
				}
			}
#endif	
		}
	}
#if (BEAMFORMING_SUPPORT == 1)
#if (MU_BEAMFORMING_SUPPORT == 1)
		pdesc_data->is_GID = txcfg->is_GID;
#endif		

        if(txcfg->ndpa) {
            unsigned char *pFrame = (unsigned char*)txcfg->phdr;
            if (1) { //(IsCtrlNDPA(pFrame) || IsMgntNDPA(pFrame)) { //eric-8822 tx hang
                pdesc_data->dataRtyLmt = 5;
                pdesc_data->rtyLmtEn   = TRUE;	
                if(IsMgntNDPA(pFrame))		//0xe0
                {
                    pdesc_data->ndpa = 1;
                }	
                else		// 0x54
                {
                    if(!IS_TEST_CHIP(priv) && (txcfg->pstat)) //eric-8822
                    {
                        if ((txcfg->pstat->WirelessMode & WIRELESS_MODE_AC_5G) && (txcfg->pstat->IOTPeer!=HT_IOT_PEER_BROADCOM))
                            pdesc_data->ndpa= 1; //eric-8822					
                        else
                            pdesc_data->ndpa= 1;

                    }
                    else
                    {
                        pdesc_data->ndpa = 1;
                    }
                }	
            }
            pdesc_data->SND_pkt_sel = txcfg->SND_pkt_sel; //eric-8822
        }
#endif

    }

	if((txcfg->q_num == MANAGE_QUE_NUM) && (txcfg->fr_type == _PRE_ALLOCMEM_) && (GetFrameSubType(txcfg->phdr) == WIFI_PROBERSP))
	{
		if(priv->pmib->dot11RFEntry.bcnagc==1) {
			if(priv->pshare->rf_ft_var.bcn_pwr_idex+6 <= priv->pshare->rf_ft_var.bcn_pwr_max)
				pdesc_data->TXPowerOffset = 4; // +3 dB
		}
		else if ( priv->pmib->dot11RFEntry.bcnagc==2) {
			if(priv->pshare->rf_ft_var.bcn_pwr_idex+12 <= priv->pshare->rf_ft_var.bcn_pwr_max)
				pdesc_data->TXPowerOffset = 5; // +6 dB
			else if(priv->pshare->rf_ft_var.bcn_pwr_idex+6 <= priv->pshare->rf_ft_var.bcn_pwr_max)
				pdesc_data->TXPowerOffset = 4; // +3 dB
		}
	} 

	if (priv->pmib->dot11RFEntry.txpwr_reduction) {
		if (priv->pmib->dot11RFEntry.txpwr_reduction <= 3) {
			if (pdesc_data->TXPowerOffset < priv->pmib->dot11RFEntry.txpwr_reduction) {
				pdesc_data->TXPowerOffset = priv->pmib->dot11RFEntry.txpwr_reduction;
			}
		}
	}
}
#endif // CONFIG_WLAN_HAL
#if (CONFIG_WLAN_NOT_HAL_EXIST == 1)
void rtl8192cd_fill_fwinfo(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, struct tx_desc  *pdesc, unsigned int frag_idx)
{
	char n_txshort = 0, bg_txshort = 0;
	//int erp_protection = 0, n_protection = 0;
	//unsigned char rate;
	unsigned char txRate = 0;
#ifdef DRVMAC_LB
	static unsigned int rate_select = 0;
#endif
	BOOLEAN			bRtsEnable = FALSE;
	BOOLEAN			bErpProtect = FALSE;
	BOOLEAN			bNProtect = FALSE;
	BOOLEAN			bHwRts = FALSE;
	BOOLEAN			bCts2SelfEnable = FALSE;
	unsigned char RtsRate;

#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE) {
		if (is_MCS_rate(txcfg->tx_rate)) {	// HT rates
			txRate = (txcfg->tx_rate - HT_RATE_ID) + 12;
		}
		else{
			txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);
		}

		if (priv->pshare->CurrentChannelBW) {
			pdesc->Dword4 |= set_desc(TX_DataBw | (3&TX_DataScMask) << TX_DataScSHIFT);
			if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40 && priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
				n_txshort = 1;
			else if (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80 && priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M)
				n_txshort = 1;
		}
		else {
			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M)
				n_txshort = 1;
		}

		if (txcfg->retry)
			pdesc->Dword5 |= set_desc((txcfg->retry & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT | TX_RtyLmtEn);

		if(n_txshort == 1)
			pdesc->Dword5 |= set_desc(TX_SGI);

		pdesc->Dword5 |= set_desc((txRate & TX_DataRateMask) << TX_DataRateSHIFT);
		
#ifdef CONFIG_RTL_92D_SUPPORT
		if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
			pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask) << TX_RtsRateSHIFT);
			//if (is_CCK_rate(txRate))
				//pdesc->Dword5 |= set_desc((4 & TX_DataRateMask) << TX_DataRateSHIFT);
		}
#endif

		return;
	}
#endif

	if (priv->pmib->dot11RFEntry.txbf == 1) {
		pdesc->Dword2 &= set_desc(0x03ffffff); // clear related bits
		pdesc->Dword2 |= set_desc(1 << TX_TxAntCckSHIFT); 	// Set Default CCK rate with 1T
		pdesc->Dword2 |= set_desc(1 << TX_TxAntlSHIFT); 	// Set Default Legacy rate with 1T
		pdesc->Dword2 |= set_desc(1 << TX_TxAntHtSHIFT); 	// Set Default Ht rate		
	}
	if(priv->pmib->dot11RFEntry.bcn2path){
		pdesc->Dword2 &= set_desc(0x03ffffff); // clear related bits
		pdesc->Dword2 |= set_desc(1 << TX_TxAntCckSHIFT);	// Set Default CCK rate with 1T
	}

	if (is_MCS_rate(txcfg->tx_rate))	// HT rates
	{
		txRate = (txcfg->tx_rate - HT_RATE_ID) + 12;
		
		if (priv->pmib->dot11RFEntry.txbf == 1) {
			if (txRate <= 0x12) {
				pdesc->Dword2 |= set_desc(3 << TX_TxAntHtSHIFT); // Set Ht rate < MCS6 with 2T
			}
		}

		if (priv->pshare->is_40m_bw) {
			if (txcfg->pstat && (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
#ifdef WIFI_11N_2040_COEXIST
				&& !(COEXIST_ENABLE && (((OPMODE & WIFI_AP_STATE) && 
				(priv->bg_ap_timeout || orForce20_Switch20Map(priv)
				))
#ifdef CLIENT_MODE
				|| ((OPMODE & WIFI_STATION_STATE) && priv->coexist_connection && 
				(txcfg->pstat->ht_ie_len) && !(txcfg->pstat->ht_ie_buf.info0 & _HTIE_STA_CH_WDTH_))
#endif
				))
#endif

				) {
				pdesc->Dword4 |= set_desc(TX_DataBw | (3 << TX_DataScSHIFT));

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
				if ((txcfg->fixed_rate) || (GET_CHIP_VER(priv)!=VERSION_8188E))
#endif
				{
					if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M &&
						txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_)))
						n_txshort = 1;
				}
			}
			else {
				if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
					pdesc->Dword4 |= set_desc((2 << TX_DataScSHIFT) | (2 << TX_RtsScSHIFT));
				else
					pdesc->Dword4 |= set_desc((1 << TX_DataScSHIFT) | (1 << TX_RtsScSHIFT));

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
				if ((txcfg->fixed_rate) || (GET_CHIP_VER(priv)!=VERSION_8188E))
#endif
				{
					if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
						txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
						n_txshort = 1;
				}
			}
		} else {
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if ((txcfg->fixed_rate) || (GET_CHIP_VER(priv)!=VERSION_8188E))
#endif
			{
				if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
					txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
					n_txshort = 1;
			}
		}

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
		if ((GET_CHIP_VER(priv)==VERSION_8188E) && !(txcfg->fixed_rate)) {
			if (txcfg->pstat->ht_current_tx_info & TX_USE_SHORT_GI)
				n_txshort = 1;
		}
#endif

		if ((txcfg->aggre_en >= FG_AGGRE_MPDU) && (txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST)) {
			int TID = ((struct sk_buff *)txcfg->pframe)->cb[1];
			if (txcfg->pstat->ADDBA_ready[TID] && !txcfg->pstat->low_tp_disable_ampdu) {
#ifdef CONFIG_RTL_88E_SUPPORT
				if (GET_CHIP_VER(priv)==VERSION_8188E)
					pdesc->Dword2 |= set_desc(TXdesc_88E_AggEn);
				else
#endif
					pdesc->Dword1 |= set_desc(TX_AggEn);

				/*
				 * assign aggr size
				 */
				 if (priv->pshare->rf_ft_var.diffAmpduSz) {
					pdesc->Dword6 |= set_desc((txcfg->pstat->diffAmpduSz & 0xffff) << TX_MCS1gMaxSHIFT | TX_UseMaxLen);
					
#ifdef CONFIG_RTL_88E_SUPPORT
					if (GET_CHIP_VER(priv)!=VERSION_8188E)
#endif						
					pdesc->Dword7 |= set_desc(txcfg->pstat->diffAmpduSz & 0xffff0000);
				 } else if(	((txcfg->pstat->current_tx_rate >= _MCS0_RATE_) && (txcfg->pstat->current_tx_rate <= _MCS2_RATE_)) 
						||((txcfg->pstat->current_tx_rate >= _MCS8_RATE_) && (txcfg->pstat->current_tx_rate <= _MCS10_RATE_)) )				 	
				 {
					pdesc->Dword6 |= set_desc((0x4444) << TX_MCS1gMaxSHIFT | TX_UseMaxLen);					
#ifdef CONFIG_RTL_88E_SUPPORT
					if (GET_CHIP_VER(priv)!=VERSION_8188E)
#endif						
					pdesc->Dword7 |= set_desc(0x44440000);									 
				 }
				// assign aggr density
				if (txcfg->privacy) {
#ifdef CONFIG_RTL_92D_SUPPORT
					if ((priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) && (!priv->pshare->is_40m_bw))
						pdesc->Dword2 |= set_desc(7 << TX_AmpduDstySHIFT);	// according to DWA-160 A2
					else
#endif
						pdesc->Dword2 |= set_desc(5 << TX_AmpduDstySHIFT);	// according to WN111v2
				}
				else {
					pdesc->Dword2 |= set_desc(((txcfg->pstat->ht_cap_buf.ampdu_para & _HTCAP_AMPDU_SPC_MASK_) >> _HTCAP_AMPDU_SPC_SHIFT_) << TX_AmpduDstySHIFT);
				}
			}
			//set Break
			if((txcfg->q_num >=1 && txcfg->q_num <=4)){
				if((txcfg->pstat != priv->pshare->CurPstat[txcfg->q_num-1])) {
#ifdef CONFIG_RTL_88E_SUPPORT
					if (GET_CHIP_VER(priv)==VERSION_8188E)
						pdesc->Dword2 |= set_desc(TXdesc_88E_BK);
					else
#endif
						pdesc->Dword1 |= set_desc(TX_BK);
					priv->pshare->CurPstat[txcfg->q_num-1] = txcfg->pstat;
				}				
			} else {
#ifdef CONFIG_RTL_88E_SUPPORT
				if (GET_CHIP_VER(priv)==VERSION_8188E)
					pdesc->Dword2 |= set_desc(TXdesc_88E_BK);
				else
#endif
					pdesc->Dword1 |= set_desc(TX_BK);
			}
		}

		// for STBC
#ifdef CONFIG_RTL_92C_SUPPORT
		if (GET_CHIP_VER(priv) != VERSION_8192C)
#endif			
		if (priv->pmib->dot11nConfigEntry.dot11nSTBC &&
			txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_RX_STBC_CAP_)) &&
			((get_rf_mimo_mode(priv) == MIMO_2T2R) || (get_rf_mimo_mode(priv) == MIMO_3T3R)))
			pdesc->Dword4 |= set_desc(1 << TX_DataStbcSHIFT);
	}
	else	// legacy rate
	{
		txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);
		if (is_CCK_rate(txcfg->tx_rate) && (txcfg->tx_rate != 2)) {
			if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
					(priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
				; // txfw->txshort = 0
			else {
				if (txcfg->pstat)
					bg_txshort = (priv->pmib->dot11RFEntry.shortpreamble) &&
									(txcfg->pstat->useShortPreamble);
				else
					bg_txshort = priv->pmib->dot11RFEntry.shortpreamble;
			}
		}
		if (priv->pshare->is_40m_bw) {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				pdesc->Dword4 |= set_desc((2 << TX_DataScSHIFT) | (2 << TX_RtsScSHIFT));
			else
				pdesc->Dword4 |= set_desc((1 << TX_DataScSHIFT) | (1 << TX_RtsScSHIFT));	
		}

		if (bg_txshort)
			pdesc->Dword4 |= set_desc(TX_DataShort);
	}

	if (txcfg->need_ack) { // unicast
		if (frag_idx == 0) {
#if defined(CONFIG_PCI_HCI)
			u4Byte fr_len = (get_desc(pdesc->Dword0)&TX_PktSizeMask) - (get_desc(pdesc->Dword7)&TX_TxBufSizeMask); 
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			u4Byte fr_len = txcfg->fr_len;	// for backward compatibile
#endif
			RtsCheck(priv, txcfg, fr_len, &bRtsEnable, &bCts2SelfEnable, &bHwRts, &bErpProtect, &bNProtect);
		}
	}

	RtsRate = find_rts_rate(priv, txcfg->tx_rate, bErpProtect);
	if(bRtsEnable && CheckCts2SelfEnable(RtsRate))
	{
		bRtsEnable = FALSE;
		bCts2SelfEnable = TRUE;
	}

//	if(bErpProtect)
//		priv->pshare->phw->bErpProtection = TRUE;

	if(bRtsEnable)
		pdesc->Dword4 |= set_desc(TX_RtsEn);
	if(bCts2SelfEnable)
		pdesc->Dword4 |= set_desc(TX_CTS2Self);
	if(bHwRts)
		pdesc->Dword4 |=set_desc(TX_HwRtsEn);
		
	if (bRtsEnable || bCts2SelfEnable) {
			unsigned int rtsTxRateIdx  = get_rate_index_from_ieee_value(RtsRate);
			if (bErpProtect) {
				unsigned char  rtsShort = 0;
				if (is_CCK_rate(RtsRate) && (RtsRate != 2)) {
					if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
							(priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
						rtsShort = 0; // do nothing
					else {
						if (txcfg->pstat)
							rtsShort = (priv->pmib->dot11RFEntry.shortpreamble) &&
											(txcfg->pstat->useShortPreamble);
						else
							rtsShort = priv->pmib->dot11RFEntry.shortpreamble;
					}
				}
				pdesc->Dword4 |= (rtsShort == 1)? set_desc(TX_RtsShort): 0;
			} 
			
			pdesc->Dword4 |= set_desc((rtsTxRateIdx & TX_RtsRateMask) << TX_RtsRateSHIFT);
			pdesc->Dword5 |= set_desc((0xf & TX_RtsRateFBLmtMask) << TX_RtsRateFBLmtSHIFT);
			//8192SE Must specified BW mode while sending RTS ...
			if (priv->pshare->is_40m_bw)
				pdesc->Dword4 |= set_desc(TX_RtsBw);

	}

	if(txcfg->pstat && priv->pshare->rf_ft_var.txforce != 0xFF)
	{
		if(priv->pshare->rf_ft_var.sgiforce == 1)
			n_txshort = 1;
		else if(priv->pshare->rf_ft_var.sgiforce == 0)
			n_txshort = 0;
	}	
	
	if(n_txshort == 1)
	if((txcfg->pstat->sta_in_firmware == 1) || (priv->pshare->rf_ft_var.sgiforce != 0xFF))
		pdesc->Dword5 |= set_desc(TX_SGI);


#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb && (priv->pmib->miscEntry.lb_mlmp == 4)) {
		txRate = rate_select;
		if (rate_select++ > 0x1b)
			rate_select = 0;

		pdesc->Dword4 |= set_desc(TX_UseRate);
		pdesc->Dword4 |= set_desc(TX_DisDataFB);
		pdesc->Dword4 |= set_desc(TX_DisRtsFB);// disable RTS fall back
	}
#endif

	if(priv->pshare->rf_ft_var.txforce != 0xff)	{
		pdesc->Dword4 |= set_desc(TX_UseRate);
		pdesc->Dword5 |= set_desc((priv->pshare->rf_ft_var.txforce & TX_DataRateMask) << TX_DataRateSHIFT);
	} else {
		pdesc->Dword5 |= set_desc((txRate & TX_DataRateMask) << TX_DataRateSHIFT);
	}

#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
		pdesc->Dword4 |= set_desc((4 & TX_RtsRateMask) << TX_RtsRateSHIFT);
		//if (is_CCK_rate(txRate))
			//pdesc->Dword5 |= set_desc((4 & TX_DataRateMask) << TX_DataRateSHIFT);
	}
#endif

	if (txcfg->need_ack) {
		// give retry limit to management frame
#ifndef DRVMAC_LB		
		if (txcfg->q_num == MANAGE_QUE_NUM) {
			pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
			if (GetFrameSubType(txcfg->phdr) == WIFI_PROBERSP) {
				;	// 0 no need to set
			}
#ifdef WDS
			else if ((GetFrameSubType(txcfg->phdr) == WIFI_PROBEREQ) && (txcfg->pstat->state & WIFI_WDS)) {
				pdesc->Dword5 |= set_desc((2 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
			}
#endif
			else {
				pdesc->Dword5 |= set_desc((6 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
			}
		}
#ifdef WDS
		else if (txcfg->wdsIdx >= 0) {
			if (txcfg->pstat->rx_avarage == 0) {
				pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
				pdesc->Dword5 |= set_desc((3 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
			}
		}
#endif
#if defined(CONFIG_PCI_HCI)
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
		else if (priv->pshare->rf_ft_var.low_retry_rssi_thd && txcfg->pstat->rssi
				&& (txcfg->pstat->rssi < priv->pshare->rf_ft_var.low_retry_rssi_thd)) {
			if ((txcfg->pstat->current_tx_rate <= _9M_RATE_) ||
				((_MCS0_RATE_ == txcfg->pstat->current_tx_rate) || (_MCS1_RATE_ == txcfg->pstat->current_tx_rate))) {
				pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
				pdesc->Dword5 |= set_desc((0x04 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
			} else {
				pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
				pdesc->Dword5 |= set_desc((0x08 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
			}
		} else if (priv->pshare->rf_ft_var.low_rate_low_retry
				&& ((txcfg->pstat->current_tx_rate <= _9M_RATE_)
				|| (_MCS0_RATE_ == txcfg->pstat->current_tx_rate) || (_MCS1_RATE_ == txcfg->pstat->current_tx_rate))) {
			pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
			pdesc->Dword5 |= set_desc((0x08 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
		}
#endif
		else if (is_MCS_rate(txcfg->pstat->current_tx_rate) && (txcfg->pstat->IOTPeer==HT_IOT_PEER_INTEL) && (txcfg->pstat->retry_inc)
			&& !(txcfg->pstat->leave) && priv->pshare->intel_rty_lmt) {
			pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
			pdesc->Dword5 |= set_desc((priv->pshare->intel_rty_lmt & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
		}

		else if ((txcfg->pstat->IOTPeer==HT_IOT_PEER_BROADCOM) && (txcfg->pstat->retry_inc) && !(txcfg->pstat->leave)) {
                pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
                pdesc->Dword5 |= set_desc((0x20 & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
        }
#endif //end DRVMAC_LB

		// High power mechanism
		//if ((GET_CHIP_VER(priv) == VERSION_8192C)||(GET_CHIP_VER(priv) == VERSION_8188C))

		{
			if (priv->pshare->rf_ft_var.tx_pwr_ctrl && txcfg->pstat && (txcfg->fr_type == _SKB_FRAME_TYPE_)) {
				if ((txcfg->pstat->hp_level == 1)
#if defined(CONFIG_RTL_92D_SUPPORT) && defined(CONFIG_RTL_NOISE_CONTROL)
					|| ((priv->pshare->DNC_on) && (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)) 
#endif
					) {
						int pwr = (priv->pshare->rf_ft_var.min_pwr_idex > 16) ? 16: priv->pshare->rf_ft_var.min_pwr_idex;
						pwr &= 0x1e;
						pdesc->Dword6 |= set_desc(((-pwr) & TX_TxAgcAMask) << TX_TxAgcASHIFT);
						pdesc->Dword6 |= set_desc(((-pwr) & TX_TxAgcBMask) << TX_TxAgcBSHIFT);
					}
			}
		}
	}
}
#endif


#ifdef TX_EARLY_MODE
__MIPS16
__IRAM_IN_865X
static void insert_emcontent(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, unsigned char *buf)
{
	struct stat_info *pstat = txcfg->pstat;
	unsigned int dw[2];

	dw[0] = set_desc((pstat->empkt_num & 0xf) |
							(((pstat->empkt_len[0]+pstat->emextra_len) << 4) & 0xfff0) |
							(((pstat->empkt_len[1]+pstat->emextra_len) << 16) & 0xfff0000) |
							(((pstat->empkt_len[2]+pstat->emextra_len) << 28) & 0xf0000000) 	
							);
	dw[1] = set_desc((((pstat->empkt_len[2]+pstat->emextra_len) >> 4) & 0xff) |
							(((pstat->empkt_len[3]+pstat->emextra_len) << 8) & 0xfff00) |							
							(((pstat->empkt_len[4]+pstat->emextra_len) << 20) & 0xfff00000) 
							);
	memcpy(buf, dw, 8);
}
#endif


#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_WLAN_HAL
void rtl88XX_signin_txdesc(struct rtl8192cd_priv *priv, struct tx_insn* txcfg, unsigned char convHdr)
{
	struct tx_desc_info	*pswdescinfo, *pdescinfo;
	unsigned int 		fr_len, tx_len, i, keyid;
	u2Byte              *tx_head;
    u4Byte              q_num;
	unsigned char		*da, *pbuf, *pwlhdr, *pmic, *picv;
	struct rtl8192cd_hw	*phw;
#ifdef TX_SHORTCUT
	int					fit_shortcut=0, idx=0;
#endif


    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    u32                             halQNum;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_BUFFER_DESCRIPTOR           cur_txbd;    
    TX_DESC_DATA_88XX               desc_data;
    BOOLEAN use_hwtxsc = FALSE;

#ifdef CONFIG_IEEE80211W
	unsigned int	isBIP = 0;
#endif
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
    BOOLEAN isWiFiHdr = TRUE;
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV

	keyid=0;
	pmic=NULL;
	picv=NULL;

	if (txcfg->tx_rate == 0) {
		DEBUG_ERR("tx_rate=0!\n");
		txcfg->tx_rate = find_rate(priv, NULL, 0, 1);
	}

	q_num = txcfg->q_num;

	phw	= GET_HW(priv);
	
    halQNum     = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, (u32)q_num);
    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[halQNum]);
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;
    memset(&desc_data, 0, sizeof(TX_DESC_DATA_88XX));

    tx_head     = &(cur_q->host_idx);

	pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
	if (isWiFiHdr)
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
	{
		da = get_da((unsigned char *)txcfg->phdr);
	}

#ifdef CONFIG_IEEE80211W
	if(txcfg->isPMF && IS_MCAST(da)) 
	{
		isBIP = 1;
		txcfg->iv = 0;
		txcfg->fr_len += 10; // 10: MMIE length
	}
#endif

	tx_len = txcfg->fr_len;

	if (txcfg->fr_type == _SKB_FRAME_TYPE_)
		pbuf = ((struct sk_buff *)txcfg->pframe)->data;
	else
		pbuf = (unsigned char*)txcfg->pframe;

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
	// this condition is 802.3 header
    if ((GET_CHIP_VER(priv)==VERSION_8814A) && (convHdr==HW_TX_SC_HEADER_CONV)) {
        isWiFiHdr = FALSE;
    }
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV


	// TODO: hw tx shorcut, reuse txdesc only support AES...
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
	if (isWiFiHdr)
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
 
	// in case of default key, then find the key id
	if (GetPrivacy((txcfg->phdr)))
	{
#ifdef WDS
		if (txcfg->wdsIdx >= 0) {
			if (txcfg->pstat)
			keyid = txcfg->pstat->keyid;
			else
				keyid = 0;
		}
		else
#endif

#ifdef __DRAYTEK_OS__
		if (!IEEE8021X_FUN)
			keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		else {
			if (IS_MCAST(GetAddr1Ptr ((unsigned char *)txcfg->phdr)) || !txcfg->pstat)
				keyid = priv->pmib->dot11GroupKeysTable.keyid;
			else
				keyid = txcfg->pstat->keyid;
		}
#else

		if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_40_PRIVACY_ ||
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_104_PRIVACY_) {
			if(IEEE8021X_FUN && txcfg->pstat) {
#ifdef A4_STA
				if (IS_MCAST(da) && !(txcfg->pstat->state & WIFI_A4_STA))
#else
				if(IS_MCAST(da))
#endif					
					keyid = 0;
				else
					keyid = txcfg->pstat->keyid;
			}
			else {
				keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		    }
		}
#endif
	}


	for (i=0; i < txcfg->frg_num; i++)
	{
        pdescinfo = pswdescinfo + *tx_head;

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		// TODO: hw tx shortcut no support fragment ? Qos Control bit ??
		if (isWiFiHdr)
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		if (i != 0)
		{
			memset(&desc_data, 0, sizeof(TX_DESC_DATA_88XX));
			
			// we have to allocate wlan_hdr
			pwlhdr = (UINT8 *)get_wlanhdr_from_poll(priv);
			if (pwlhdr == (UINT8 *)NULL)
			{
				DEBUG_ERR("System-bug... should have enough wlan_hdr\n");
				return;
			}
			// other MPDU will share the same seq with the first MPDU
			memcpy((void *)pwlhdr, (void *)(txcfg->phdr), txcfg->hdr_len); // data pkt has 24 bytes wlan_hdr
		}
		else
		{
#ifdef WIFI_WMM
			if (txcfg->pstat /*&& (is_qos_data(txcfg->phdr))*/) {
				if ((GetFrameSubType(txcfg->phdr) & (WIFI_DATA_TYPE | BIT(6) | BIT(7))) == (WIFI_DATA_TYPE | BIT(7))) {
					unsigned char tempQosControl[2];
					memset(tempQosControl, 0, 2);
					tempQosControl[0] = ((struct sk_buff *)txcfg->pframe)->cb[1];
#ifdef WMM_APSD
					if (
#ifdef CLIENT_MODE
						(OPMODE & WIFI_AP_STATE) &&
#endif
						(APSD_ENABLE) && (txcfg->pstat) && (txcfg->pstat->state & WIFI_SLEEP_STATE) &&
						(!GetMData(txcfg->phdr)) &&
						((((tempQosControl[0] == 7) || (tempQosControl[0] == 6)) && (txcfg->pstat->apsd_bitmap & 0x01)) ||
						 (((tempQosControl[0] == 5) || (tempQosControl[0] == 4)) && (txcfg->pstat->apsd_bitmap & 0x02)) ||
						 (((tempQosControl[0] == 3) || (tempQosControl[0] == 0)) && (txcfg->pstat->apsd_bitmap & 0x08)) ||
						 (((tempQosControl[0] == 2) || (tempQosControl[0] == 1)) && (txcfg->pstat->apsd_bitmap & 0x04))))
						tempQosControl[0] |= BIT(4);
#endif
					if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
						tempQosControl[0] |= BIT(7);

					if (priv->pmib->dot11nConfigEntry.dot11nTxNoAck)
						tempQosControl[0] |= BIT(5);

					memcpy((void *)GetQosControl((txcfg->phdr)), tempQosControl, 2);
				}
			}
#endif

#if (BEAMFORMING_SUPPORT == 1)
			if(!txcfg->ndpa)
#endif	
			assign_wlanseq(GET_HW(priv), txcfg->phdr, txcfg->pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
				, txcfg->is_11s
#endif
				);
			pwlhdr = txcfg->phdr;
		}
#if (BEAMFORMING_SUPPORT == 1)
		if(!txcfg->ndpa)
#endif	
        {
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
 	       	if (isWiFiHdr)
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
            {
		SetDuration(pwlhdr, 0);
            }
		}

#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE) {
			if (GetFrameSubType(txcfg->phdr) == WIFI_PSPOLL)
        desc_data.navUseHdr = _TRUE;

			if (priv->ps_state)
				SetPwrMgt(pwlhdr);
			else
				ClearPwrMgt(pwlhdr);
		}
#endif
#if (BEAMFORMING_SUPPORT == 1)
		if(txcfg->ndpa)
			desc_data.navUseHdr = _TRUE;
#endif

if((priv->pshare->rf_ft_var.txforce != 0xff || (txcfg->pstat && txcfg->pstat->force_rate))
            #if (BEAMFORMING_SUPPORT == 1)
            && (!txcfg->ndpa) 
            #endif
        ){
        	if(txcfg->pstat && txcfg->pstat->force_rate)
				desc_data.dataRate  = txcfg->pstat->force_rate;
			else
	            desc_data.dataRate  = priv->pshare->rf_ft_var.txforce;
            desc_data.disDataFB = _TRUE;
            desc_data.disRTSFB  = _TRUE;
            desc_data.useRate   = _TRUE;
        }
			

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		if (isWiFiHdr)
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		if (i != (txcfg->frg_num - 1))
		{
			SetMFrag(pwlhdr);
			if (i == 0) {
				fr_len = (txcfg->frag_thrshld - txcfg->llc);
				tx_len -= (txcfg->frag_thrshld - txcfg->llc);
			}
			else {
				fr_len = txcfg->frag_thrshld;
				tx_len -= txcfg->frag_thrshld;
			}
		}
		else	// last seg, or the only seg (frg_num == 1)
		{
			fr_len = tx_len;
			ClearMFrag(pwlhdr);
		}

		desc_data.frLen  = fr_len;

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		if(txcfg->pstat)
		{
			ODM_SetTxAntByTxInfo_HAL(priv, (PVOID)&desc_data, txcfg->pstat->aid);   //HAL Set TX antenna	
		}
#endif	//#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		// TODO: how to fill some field in rtl88XX_fill_fwinfo
		rtl88XX_fill_fwinfo(priv, txcfg, i, &desc_data);
        
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
        if (isWiFiHdr)
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
        {
		SetFragNum((pwlhdr), i);
        }

		if (((i == 0) && (txcfg->fr_type == _SKB_FRAME_TYPE_))
#if (BEAMFORMING_SUPPORT == 1)
			|| txcfg->ndpa
#endif
		) {
			pdescinfo->type = _PRE_ALLOCLLCHDR_;
		}
		else {
			pdescinfo->type = _PRE_ALLOCHDR_;
		}

		if(txcfg->fr_type == _SKB_FRAME_TYPE_) {
	       desc_data.tid = ((struct sk_buff *)txcfg->pframe)->cb[1];
#ifdef WMM_DSCP_C42
			{
				unsigned int 	   q_num;
#ifdef RTL_MANUAL_EDCA
				q_num = PRI_TO_QNUM(priv, desc_data.tid);
#else
				PRI_TO_QNUM(desc_data.tid, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
				if(q_num ^ txcfg->q_num)
					desc_data.tid = 0x04;
			}
#elif defined(CONFIG_WLAN_HAL_8197F)
			{
				unsigned int	   q_num;
#ifdef RTL_MANUAL_EDCA
				q_num = PRI_TO_QNUM(priv, desc_data.tid);
#else
				PRI_TO_QNUM(desc_data.tid, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
			}
#endif
		}


        if (i != (txcfg->frg_num - 1)) {
            desc_data.frag = _TRUE;
        }

		if (txcfg->pstat) {
			if (txcfg->pstat->aid != MANAGEMENT_AID) {
                desc_data.rateId = txcfg->pstat->ratr_idx;
                desc_data.macId = REMAP_AID(txcfg->pstat);
			}
#if (BEAMFORMING_SUPPORT == 1)
			if((priv->pmib->dot11RFEntry.txbf == 1) &&
				((txcfg->pstat->ht_cap_len && (txcfg->pstat->ht_cap_buf.txbf_cap)) 
#ifdef RTK_AC_SUPPORT
				||(txcfg->pstat->vht_cap_len && (cpu_to_le32(txcfg->pstat->vht_cap_buf.vht_cap_info) & BIT(SU_BFEE_S)))
#endif
			)){
				desc_data.p_aid = txcfg->pstat->p_aid;
				desc_data.g_id  = txcfg->pstat->g_id;
			}
#endif
		} else {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
			if ((txcfg->fr_type == _SKB_FRAME_TYPE_) && IS_MCAST(GetAddr1Ptr ((unsigned char *)txcfg->phdr)) && 
				((txcfg->privacy == _TKIP_PRIVACY_) || (txcfg->privacy == _CCMP_PRIVACY_))) {
				desc_data.enDescId = 1;
				desc_data.macId = priv->vap_init_seq;
			}
#endif
            desc_data.rateId = ARFR_BMC;
			
        }

#if 1 //eric-8822 ?? assign gid of txdesc = 0x3f
#if (BEAMFORMING_SUPPORT == 1)
		if((GET_CHIP_VER(priv)==VERSION_8822B) && txcfg->ndpa){
			if(txcfg->pstat){
				Beamforming_GidPAid(priv, txcfg->pstat);
				desc_data.p_aid = txcfg->pstat->p_aid;
			}
		}
#endif

		desc_data.g_id = 0x3f;
#endif	

        if (GET_CHIP_VER(priv) >= VERSION_8814A) {
            if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
                desc_data.dataRateFBLmt = 4;
            } else {
                desc_data.dataRateFBLmt = 0;
            }
        } else
#if defined(MULTI_STA_REFINE)
        if((priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) && (priv->pshare->total_assoc_num > 10) 
            && (txcfg->pstat) && ((txcfg->pstat->tx_ra_bitmap & 0xff00000) == 0)) {
            desc_data.dataRateFBLmt = 3;
        } else
#endif
        {
            desc_data.dataRateFBLmt = 0x1F;
        }

        if (txcfg->fixed_rate) {
            desc_data.disDataFB = _TRUE;
            desc_data.disRTSFB  = _TRUE;
            desc_data.useRate   = _TRUE;
        }


        if(txcfg->pstat && txcfg->pstat->sta_in_firmware != 1)
        desc_data.useRate = _TRUE;


#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		if (isWiFiHdr)
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		if (txcfg->privacy) {
            desc_data.secType = txcfg->privacy;
#ifdef CONFIG_IEEE80211W			
			if (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF)) {
#else		
			if (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE))) {
#endif				
                desc_data.swCrypt = TRUE;
                desc_data.icv = txcfg->icv;
                desc_data.mic = txcfg->mic;
                desc_data.iv  = txcfg->iv;
			} else {
				// hw encrypt
				desc_data.swCrypt = FALSE;
				switch(txcfg->privacy) {
				case _WEP_104_PRIVACY_:
				case _WEP_40_PRIVACY_:
                    desc_data.icv = 0;
                    desc_data.mic = 0;
                    desc_data.iv  = txcfg->iv;
					wep_fill_iv(priv, pwlhdr, txcfg->hdr_len, keyid);
					break;

				case _TKIP_PRIVACY_:
                    desc_data.icv = 0;
                    desc_data.mic = txcfg->mic;
                    desc_data.iv  = txcfg->iv;
					tkip_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
					break;

				#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				case _WAPI_SMS4_:
                    desc_data.icv = 0;
                    desc_data.mic = 0;
                    desc_data.iv  = txcfg->iv;
					break;
				#endif
                
				case _CCMP_PRIVACY_:
					//michal also hardware...
                    desc_data.icv = 0;
                    desc_data.mic = 0;
                    desc_data.iv  = txcfg->iv;
					aes_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
					break;

				default:
					DEBUG_ERR("Unknow privacy\n");
					break;
				}
			}
		}

#ifdef WLAN_HAL_HW_AES_IV
        // TODO: add this mechanism in rtl88XX_signin_txdesc_shortcut !!!
        if ((txcfg->privacy == _CCMP_PRIVACY_) && 
            (
#ifdef CONFIG_IEEE80211W
            (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF)) 
#else
            (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
#endif
                == FALSE)) {
            desc_data.secType = txcfg->privacy;
            desc_data.swCrypt = FALSE;
            desc_data.icv     = 0;
            desc_data.mic     = 0;
            desc_data.iv      = 0;
            desc_data.hwAESIv = TRUE;
			desc_data.iv = txcfg->iv;

            #if 0
            // temp mark by Eric, no need check isWifiHdr
            if (isWiFiHdr == TRUE) {
                desc_data.iv = txcfg->iv;
                // when desc_data.hwAESIv == TRUE, hw auto fill AES iv
                // aes_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
            }
            #endif 
        }
#endif // WLAN_HAL_HW_AES_IV

		// below is for sw desc info
		// TODO: hw tx shortcut recycle....sw desc.
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		if (isWiFiHdr == FALSE) {
			pdescinfo->pframe = NULL;
		} else
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		{
		pdescinfo->pframe = pwlhdr;
		}

#if defined(WIFI_WMM) && defined(WMM_APSD)
		pdescinfo->priv = priv;
#ifndef TXDESC_INFO
		pdescinfo->pstat = txcfg->pstat;
#endif		
#endif

#ifdef TX_SHORTCUT
		if (!priv->pmib->dot11OperationEntry.disable_txsc && txcfg->pstat &&
				(txcfg->fr_type == _SKB_FRAME_TYPE_) &&
				(txcfg->frg_num == 1) &&
				((txcfg->privacy == 0)
#ifdef CONFIG_RTL_WAPI_SUPPORT
				|| (txcfg->privacy == _WAPI_SMS4_)
#endif
#ifdef CONFIG_IEEE80211W
				|| (!UseSwCrypto(priv, txcfg->pstat, FALSE, txcfg->isPMF))
#else
				|| (!UseSwCrypto(priv, txcfg->pstat, FALSE))
#endif
				) &&
				(
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
				(isWiFiHdr == FALSE) ? TRUE : 
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
				(!GetMData(txcfg->phdr))
				) &&
#ifdef SUPPORT_TX_AMSDU_SHORTCUT				
				txcfg->aggre_en <= FG_AGGRE_MSDU_FIRST
#else
				txcfg->aggre_en < FG_AGGRE_MSDU_FIRST
#endif
				) {
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
				struct wlan_ethhdr_t ethhdr;
#endif
#ifdef CONFIG_RTK_MESH
			if( txcfg->is_11s) {
				idx = get_tx_sc_free_entry(priv, txcfg->pstat, &priv->ethhdr);
			} else
#endif
			{
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
				if (isWiFiHdr == FALSE) {
					idx = get_tx_sc_free_entry(priv, txcfg->pstat, pbuf);
				} else
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
                {
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
                			if(txcfg->aggre_en == FG_AGGRE_MSDU_FIRST){
						memcpy((void *)&ethhdr, pbuf, 12);
						memcpy((void *)&(ethhdr.type), pbuf+20, 2);
						idx = get_tx_sc_free_entry(priv, txcfg->pstat, &ethhdr);
                			}
					else
#endif
					idx = get_tx_sc_free_entry(priv, txcfg->pstat, pbuf - sizeof(struct wlan_ethhdr_t));
				}
			}
#ifdef CONFIG_RTK_MESH
			if( txcfg->is_11s) {
				memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, &priv->ethhdr, sizeof(struct wlan_ethhdr_t));
			} else
#endif
			{
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
				if (isWiFiHdr == FALSE) 
				{
					memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, pbuf, sizeof(struct wlan_ethhdr_t));
				} else
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
                {
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
//					txcfg->pstat->tx_sc_ent[idx].isamsdu = (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST);
                			if(txcfg->aggre_en == FG_AGGRE_MSDU_FIRST){
						memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].amsdullcsnaphdr, pbuf, sizeof(struct wlan_amsdullcsnaphdr_t));
						memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, &ethhdr, sizeof(struct wlan_ethhdr_t));
                			}
					else
#endif
					memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, pbuf - sizeof(struct wlan_ethhdr_t), sizeof(struct wlan_ethhdr_t));
				}
			}

			txcfg->pstat->protection = priv->pmib->dot11ErpInfo.protection;
			txcfg->pstat->ht_protection = priv->ht_protection;
			txcfg->pstat->tx_sc_ent[idx].sc_keyid = keyid;
			txcfg->pstat->tx_sc_ent[idx].pktpri = ((struct sk_buff *)txcfg->pframe)->cb[1];
			fit_shortcut = 1;

#ifdef WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
            if(IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC(priv))   {
                desc_data.txwifiCp = TRUE;
                txcfg->pstat->tx_sc_hw_idx = idx;
            }
#endif // WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
            if(IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV(priv))   {
				if (convHdr == HW_TX_SC_BACKUP_HEADER) {
					desc_data.macCp = TRUE;		// backup 802.11 header info.
					desc_data.smhEn = FALSE;
				} else if (convHdr == HW_TX_SC_HEADER_CONV) {
					desc_data.macCp = FALSE;
					desc_data.smhEn = TRUE;		// auto conv hdr  (802.3 -> 802.11)	
					// TODO: consider AES for HW_TX_SHORTCUT_HDR_CONV
					// Eth hdr 14 bytes => gen llc 8 bytes
					//                              gen iv 8 bytes
					// skb: ethHdr + data
					// ethHdr + (iv + llc)  + data
					// BD[0]: txDesc, BD[1]: ethHdr, BD[2]: iv+llc, BD[3]: data
				}
            }
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		}
		else {
			if (txcfg->pstat) {
				for (idx=0; idx<TX_SC_ENTRY_NUM; idx++) {
                    GET_HAL_INTERFACE(priv)->SetShortCutTxBuffSizeHandler(priv, txcfg->pstat->tx_sc_ent[idx].hal_hw_desc, 0);
				}
			}
		}
#endif

#ifndef TXDESC_INFO			
		if (txcfg->privacy)
		{
			if (txcfg->privacy == _WAPI_SMS4_)
			{
				pdescinfo->pstat = txcfg->pstat;
				pdescinfo->rate = txcfg->tx_rate;
			}
#ifdef CONFIG_IEEE80211W			
			else if (!UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF))
#else
			else if (!UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))			
#endif
			{
				pdescinfo->pstat = txcfg->pstat;
				pdescinfo->rate = txcfg->tx_rate;
			}
            // TODO: why swCrypto doesn't set these two swDesc ?
		}
		else
		{
			pdescinfo->pstat = txcfg->pstat;
			pdescinfo->rate = txcfg->tx_rate;
		}
#endif    
	if (0 == txcfg->fr_len)
            goto swenc_end;
	
	#ifdef CONFIG_RTL_WAPI_SUPPORT
	//panic_printk("%s:%d privacy=%d\n", __FUNCTION__, __LINE__,txcfg->privacy);
	if (txcfg->privacy == _WAPI_SMS4_)
	{
		//panic_printk("%s:%d\n", __FUNCTION__, __LINE__);
		SecSWSMS4Encryption(priv, txcfg);
		desc_data.pMic = ((struct sk_buff *)txcfg->pframe)->data+txcfg->fr_len;
	} 
	#endif    
        /*** start sw encryption ***/
#ifdef CONFIG_IEEE80211W	
		if (txcfg->privacy && UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE),txcfg->isPMF))
#else
		if (txcfg->privacy && UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
#endif
		{
            if (txcfg->privacy == _TKIP_PRIVACY_ ||
                txcfg->privacy == _WEP_40_PRIVACY_ ||
                txcfg->privacy == _WEP_104_PRIVACY_) {
                
                //picvdescinfo = pswdescinfo + *tx_head;
        
                // append ICV first...
                picv = get_icv_from_poll(priv);
                if (picv == NULL) {
                    DEBUG_ERR("System-Buf! can't alloc picv\n");
                    BUG();
                }
                
                pdescinfo->buf_type[1]   = _PRE_ALLOCICVHDR_;
                pdescinfo->buf_pframe[1] = picv;
                pdescinfo->buf_len[1] = txcfg->icv;

                desc_data.pIcv = picv;
                
                if (i == 0) {
                    tkip_icv(picv,
                          pwlhdr + txcfg->hdr_len + txcfg->iv, txcfg->llc,
                          pbuf,                                txcfg->fr_len);
                } else {                
                    tkip_icv(picv,
                        NULL, 0,
                        pbuf, txcfg->fr_len);
                }
        
                if ((i == 0) && (txcfg->llc != 0)) {
                    if (txcfg->privacy == _TKIP_PRIVACY_) {
                        tkip_encrypt(priv, pwlhdr, txcfg->hdr_len,
                            pwlhdr + txcfg->hdr_len + 8, sizeof(struct llc_snap),
                            pbuf,                        txcfg->fr_len, 
                            picv,                        txcfg->icv);
                    } else {
                        wep_encrypt(priv, pwlhdr, txcfg->hdr_len,
                            pwlhdr + txcfg->hdr_len + 4, sizeof(struct llc_snap),
                            pbuf,                        txcfg->fr_len, 
                            picv,                        txcfg->icv,
                            txcfg->privacy);
                    }
                } else { // not first segment or no snap header
                    if (txcfg->privacy == _TKIP_PRIVACY_) {
                        tkip_encrypt(priv, pwlhdr, txcfg->hdr_len, 
                            NULL, 0,
                            pbuf, txcfg->fr_len, 
                            picv, txcfg->icv);
                    } else {
                        wep_encrypt(priv, pwlhdr, txcfg->hdr_len, 
                            NULL, 0,
                            pbuf, txcfg->fr_len,
                            picv, txcfg->icv,
                            txcfg->privacy);
                    }
                }
                        
            } else if (txcfg->privacy == _CCMP_PRIVACY_) {
                //pmicdescinfo = pswdescinfo + *tx_head;

                // append MIC first...
                pmic = get_mic_from_poll(priv);
                if (pmic == NULL) {
                    DEBUG_ERR("System-Buf! can't alloc pmic\n");
                    BUG();
                }

                pdescinfo->buf_type[1]   = _PRE_ALLOCMICHDR_;
                pdescinfo->buf_pframe[1] = pmic;
                pdescinfo->buf_len[1] = txcfg->mic;

                desc_data.pMic = pmic;
                
                // then encrypt all (including ICV) by AES
                if ((i == 0)&&(txcfg->llc != 0)) { // encrypt 3 segments ==> llc, mpdu, and mic
#ifdef CONFIG_IEEE80211W
					if(isBIP) {
						BIP_encrypt(priv, pwlhdr, 
									pwlhdr + txcfg->hdr_len + 8,
									pbuf, txcfg->fr_len, 
									pmic,txcfg->isPMF);
					} 
					else {
						aesccmp_encrypt(priv, pwlhdr, 
										pwlhdr + txcfg->hdr_len + 8,
										pbuf, txcfg->fr_len, 
										pmic,txcfg->isPMF);
					}
#else
						aesccmp_encrypt(priv, pwlhdr, 
										pwlhdr + txcfg->hdr_len + 8,
										pbuf, txcfg->fr_len, 
										pmic);
#endif            
                } else { // encrypt 2 segments ==> mpdu and mic
#ifdef CONFIG_IEEE80211W
					if(isBIP) {
						BIP_encrypt(priv, pwlhdr, 
									NULL,
									pbuf, txcfg->fr_len, 
									pmic,txcfg->isPMF);
					} 
					else {
						aesccmp_encrypt(priv, pwlhdr, 
										NULL,
										pbuf, txcfg->fr_len, 
										pmic,txcfg->isPMF);
					}
#else
					aesccmp_encrypt(priv, pwlhdr, 
									NULL,
									pbuf, txcfg->fr_len, 
									pmic);
#endif                    
               }       
            }
        }
        /*** end sw encryption ***/
swenc_end:

        desc_data.pHdr   = pwlhdr;
        desc_data.hdrLen = txcfg->hdr_len;
        desc_data.llcLen = (i==0 ? txcfg->llc : 0);
        
        if (fr_len != 0) {
            desc_data.pBuf = pbuf;
        }

        switch (halQNum) {
        case HCI_TX_DMA_QUEUE_HI0:
#if CFG_HAL_SUPPORT_MBSSID
        case HCI_TX_DMA_QUEUE_HI1:
        case HCI_TX_DMA_QUEUE_HI2:
        case HCI_TX_DMA_QUEUE_HI3:
        case HCI_TX_DMA_QUEUE_HI4:
        case HCI_TX_DMA_QUEUE_HI5:
        case HCI_TX_DMA_QUEUE_HI6:
        case HCI_TX_DMA_QUEUE_HI7:
#endif
            desc_data.moreData = 1;
            break;
        default:
            break;
        }

#ifdef CONFIG_8814_AP_MAC_VERI
    if (txcfg->fr_len != 0)	//for mgt frame
    {
        if((desc_data.pBuf[0]==0x55) && (desc_data.pBuf[1]==0x55))
        {
            desc_data.macId = priv->macID_temp;
            desc_data.disDataFB = _FALSE;
            desc_data.disRTSFB  = _FALSE;
            desc_data.useRate= _TRUE;  

            desc_data.dataRate = 0x13; // HT MCS6
            desc_data.RTSRate = 0x13;
            desc_data.rateId = 5;

            if(priv->lowestRate_TXDESCen)
            {
                desc_data.moreData = 1;                
                desc_data.dataRateFBLmt = priv->lowestRate;
                desc_data.RTSRateFBLmt = priv->lowestRate;                
                desc_data.rtyLmtEn = 1;                                         
                desc_data.dataRtyLmt = 16;    
                desc_data.RTSEn = true;
            }
            else
            {
                desc_data.rtyLmtEn = 0;
            }

            if(priv->ProtectionType == 0){
                desc_data.HWRTSEn = 0;
                desc_data.RTSEn = 0;                
                desc_data.CTS2Self = 0;                
            }else if(priv->ProtectionType == 1) {
                desc_data.HWRTSEn = 0;
                desc_data.RTSEn = 1;                              
                desc_data.CTS2Self = 0;       
                desc_data.rtyLmtEn = 1;         
                desc_data.dataRtyLmt = 16;                 
            }else if(priv->ProtectionType == 2) {
                desc_data.HWRTSEn = 0;
                desc_data.RTSEn = 0;                
                desc_data.CTS2Self = 1;                          
            }

            desc_data.BMCRtyLmt = priv->BMC_RTY_LMT;

#ifdef VERIFY_AP_FAST_EDCA
			desc_data.aggEn = 0;	// no agg.
			desc_data.dataRate = 4;	// OFDM rate
			desc_data.RTSEn = FALSE;
#endif
        }
    }
#endif 


#ifdef WLAN_HAL_TX_AMSDU
    desc_data.aggreEn = txcfg->aggre_en;
    if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
        priv->amsdu_len = desc_data.hdrLen + desc_data.llcLen + desc_data.frLen;

#ifdef CONFIG_IEEE80211W
        if (txcfg->privacy && (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF)==FALSE))
#else
        if (txcfg->privacy && (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE))==FALSE))
#endif
        {
            priv->amsdu_len += (desc_data.icv + desc_data.mic + desc_data.iv);
        }
    }
#endif
#ifdef TX_SHORTCUT
        if (fit_shortcut) {
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
			if (isWiFiHdr == FALSE) {
				desc_data.pHdr	 = txcfg->phdr;
				desc_data.hdrLen = txcfg->fr_len;
				desc_data.llcLen = 0;
				desc_data.frLen  = 0;
			}
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
            GET_HAL_INTERFACE(priv)->FillShortCutTxHwCtrlHandler(
                priv, halQNum, (void *)&desc_data, txcfg->pstat->tx_sc_ent[idx].hal_hw_desc, 0x01, use_hwtxsc);
#ifdef TXSC_CACHE_FRLEN					
			txcfg->pstat->tx_sc_ent[idx].frlen = fr_len;
#endif
        } else
#endif
        {        	
            GET_HAL_INTERFACE(priv)->FillTxHwCtrlHandler(priv, halQNum, (void *)&desc_data);
        }

        if (txcfg->fr_len != 0) {
            if (i == 0) {
                pdescinfo->buf_type[0] = txcfg->fr_type;
            } else {
                pdescinfo->buf_type[0] = _RESERVED_FRAME_TYPE_;
            }

            pdescinfo->buf_pframe[0]   = txcfg->pframe;
            pdescinfo->buf_len[0]      = txcfg->fr_len;
#ifdef WLAN_HAL_TX_AMSDU
            if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
                pdescinfo->buf_type_amsdu[0]    = txcfg->fr_type;
                pdescinfo->buf_pframe_amsdu[0]  = txcfg->pframe;
                
                pdescinfo->buf_type[0] = _RESERVED_FRAME_TYPE_;
                pdescinfo->buf_pframe[0] = NULL;
                
            } else if (txcfg->aggre_en > FG_AGGRE_MSDU_FIRST) {
                printk("%s(%d): amsdu error !!! %d \n", __FUNCTION__, __LINE__, txcfg->aggre_en);
            } else
#endif // WLAN_HAL_TX_AMSDU
            {
#ifndef TXDESC_INFO
                pdescinfo->buf_paddr[0] = get_desc(cur_txbd->TXBD_ELE[2].Dword1);//payload
#endif
            }
            if (desc_data.pIcv || desc_data.pMic)
                pdescinfo->buf_paddr[1] = get_desc(cur_txbd->TXBD_ELE[3].Dword1);// icv/mic
        }

#ifdef WLAN_HAL_TX_AMSDU
        if (txcfg->aggre_en >= FG_AGGRE_MSDU_FIRST) {
        } else 
#endif // WLAN_HAL_TX_AMSDU
        {
#ifndef TXDESC_INFO        
            pdescinfo->paddr = get_desc(cur_txbd->TXBD_ELE[1].Dword1);//header address
#endif
        }

        pdescinfo->len = get_desc(cur_txbd->TXBD_ELE[1].Dword0) & 0xffff; // header len


#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
		// TODO: temp for swdesc...should check it..
		if (isWiFiHdr == FALSE) {
			pdescinfo->type				= _SKB_FRAME_TYPE_;	// 802.3 header + payload
			pdescinfo->pframe 			= txcfg->pframe; // skb

			pdescinfo->buf_type[0]	 	= 0;
			pdescinfo->buf_pframe[0] 	= 0; // no packet payload
		}
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV


#ifdef MERGE_TXDESC_HEADER_PAYLOAD
//if (q_num == BE_QUEUE) {
        pdescinfo->type             = _SKB_FRAME_TYPE_; // txdesc + 802.3 header + payload
        pdescinfo->pframe           = txcfg->pframe;    // skb

        pdescinfo->buf_type[0]      = 0;
        pdescinfo->buf_pframe[0]    = 0;

        if (((i == 0) && (txcfg->fr_type == _SKB_FRAME_TYPE_))) {
            //pdescinfo->type = _PRE_ALLOCLLCHDR_;
            release_wlanllchdr_to_poll(priv, txcfg->phdr);
        } else {
            //pdescinfo->type = _PRE_ALLOCHDR_;
            release_wlanhdr_to_poll(priv, txcfg->phdr);
        }
//}
#endif

#ifdef TX_SHORTCUT
        if (fit_shortcut) {
			//descinfo_copy(&txcfg->pstat->tx_sc_ent[idx].swdesc1, pdescinfo);
    		txcfg->pstat->tx_sc_ent[idx].swdesc1.type = pdescinfo->type;
#ifndef TXDESC_INFO			
	    	txcfg->pstat->tx_sc_ent[idx].swdesc1.len  = pdescinfo->len;
    		txcfg->pstat->tx_sc_ent[idx].swdesc1.rate = pdescinfo->rate;
#endif			
            //txcfg->pstat->tx_sc_ent[idx].swdesc1.buf_type[0] = pdescinfo->buf_type[0]
        }
#endif

		if (txcfg->fr_len == 0)
		{
//            printk("%s(%d): fr_len == 0 !!! \n", __FUNCTION__, __LINE__);
			goto init_deschead;
		}

		pbuf += fr_len;

#ifdef RESERVE_TXDESC_FOR_EACH_IF
		if (RSVQ_ENABLE && IS_RSVQ(q_num))
			priv->use_txdesc_cnt[RSVQ(q_num)] ++;
#endif
	}


init_deschead:

#ifdef WLAN_HAL_TX_AMSDU
    if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
        priv->amsdu_num = 1;
        return;
    }
#endif

    if(desc_data.SND_pkt_sel != 1) //SND_pkt_sel = 1 = send VHTNDPA_MU, needs Tx together with Report Poll
    GET_HAL_INTERFACE(priv)->SyncSWTXBDHostIdxToHWHandler(priv, halQNum);
	return;
}
#endif // CONFIG_WLAN_HAL

#if	defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT) 
void rtl8192cd_signin_txdesc_8812(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	struct tx_desc		*phdesc, *pdesc, *pndesc, *picvdesc, *pmicdesc, *pfrstdesc;
	struct tx_desc_info	*pswdescinfo, *pdescinfo, *pndescinfo, *picvdescinfo, *pmicdescinfo;
	unsigned int 		fr_len, tx_len, i, keyid;
	int					*tx_head, q_num;
	unsigned long		tmpphyaddr;
	unsigned char		*da, *pbuf, *pwlhdr, *pmic, *picv;
	struct rtl8192cd_hw	*phw;
	unsigned char		 q_select;
#ifdef TX_SHORTCUT
	int					fit_shortcut=0, idx=0;
#endif
	unsigned long		pfrst_dma_desc=0;
	unsigned long		*dma_txhead;

	unsigned long		flush_addr[20];
	int					flush_len[20];
	int					flush_num=0;
#ifdef CONFIG_IEEE80211W
	unsigned int	isBIP = 0;
#endif

	picvdesc=NULL;
	keyid=0;
	pmic=NULL;
	picv=NULL;
	q_select=0;

	if (txcfg->tx_rate == 0) {
		DEBUG_ERR("tx_rate=0!\n");
		txcfg->tx_rate = find_rate(priv, NULL, 0, 1);
	}

	q_num = txcfg->q_num;

	phw	= GET_HW(priv);

	dma_txhead	= get_txdma_addr(phw, q_num);
	tx_head		= get_txhead_addr(phw, q_num);
	phdesc   	= get_txdesc(phw, q_num);
	pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);

	da = get_da((unsigned char *)txcfg->phdr);
	
#ifdef CONFIG_IEEE80211W
	if(txcfg->isPMF && IS_MCAST(da)) 
	{
		isBIP = 1;
		txcfg->iv = 0;
		txcfg->fr_len += 10; // 10: MMIE length
	}
#endif
	tx_len = txcfg->fr_len;

	if (txcfg->fr_type == _SKB_FRAME_TYPE_)
		pbuf = ((struct sk_buff *)txcfg->pframe)->data;
	else
		pbuf = (unsigned char*)txcfg->pframe;



	tmpphyaddr = get_physical_addr(priv, pbuf, tx_len, PCI_DMA_TODEVICE);

	// in case of default key, then find the key id
	if (GetPrivacy((txcfg->phdr)))
	{
#ifdef WDS
		if (txcfg->wdsIdx >= 0) {
			if (txcfg->pstat)
				keyid = txcfg->pstat->keyid;
			else
				keyid = 0;
		}
		else
#endif

#ifdef __DRAYTEK_OS__
		if (!IEEE8021X_FUN)
			keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		else {
			if (IS_MCAST(GetAddr1Ptr ((unsigned char *)txcfg->phdr)) || !txcfg->pstat)
				keyid = priv->pmib->dot11GroupKeysTable.keyid;
			else
				keyid = txcfg->pstat->keyid;
		}
#else

		if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_40_PRIVACY_ ||
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_104_PRIVACY_) {
			if(IEEE8021X_FUN && txcfg->pstat) {
#ifdef A4_STA
				if (IS_MCAST(da) && !(txcfg->pstat->state & WIFI_A4_STA))
#else
				if(IS_MCAST(da))
#endif					
					keyid = 0;
				else
					keyid = txcfg->pstat->keyid;
			}
			else
				keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		}
#endif


	}

	for(i=0, pfrstdesc= phdesc + (*tx_head); i < txcfg->frg_num; i++)
	{
		/*------------------------------------------------------------*/
		/*           fill descriptor of header + iv + llc             */
		/*------------------------------------------------------------*/
		pdesc     = phdesc + (*tx_head);
		pdescinfo = pswdescinfo + *tx_head;

		//clear all bits

		memset(pdesc, 0, 40);

		if (i != 0)
		{
			// we have to allocate wlan_hdr
			pwlhdr = (UINT8 *)get_wlanhdr_from_poll(priv);
			if (pwlhdr == (UINT8 *)NULL)
			{
				DEBUG_ERR("System-bug... should have enough wlan_hdr\n");
				return;
			}
			// other MPDU will share the same seq with the first MPDU
			memcpy((void *)pwlhdr, (void *)(txcfg->phdr), txcfg->hdr_len); // data pkt has 24 bytes wlan_hdr
			pdesc->Dword9 |= set_desc((GetSequence(txcfg->phdr) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
		else
		{
#ifdef WIFI_WMM
			if (txcfg->pstat && (is_qos_data(txcfg->phdr))) {
				if ((GetFrameSubType(txcfg->phdr) & (WIFI_DATA_TYPE | BIT(6) | BIT(7))) == (WIFI_DATA_TYPE | BIT(7))) {
					unsigned char tempQosControl[2];
					memset(tempQosControl, 0, 2);
					tempQosControl[0] = ((struct sk_buff *)txcfg->pframe)->cb[1];
#ifdef WMM_APSD
					if (
#ifdef CLIENT_MODE
						(OPMODE & WIFI_AP_STATE) &&
#endif
						(APSD_ENABLE) && (txcfg->pstat) && (txcfg->pstat->state & WIFI_SLEEP_STATE) &&
						(!GetMData(txcfg->phdr)) &&
						((((tempQosControl[0] == 7) || (tempQosControl[0] == 6)) && (txcfg->pstat->apsd_bitmap & 0x01)) ||
						 (((tempQosControl[0] == 5) || (tempQosControl[0] == 4)) && (txcfg->pstat->apsd_bitmap & 0x02)) ||
						 (((tempQosControl[0] == 3) || (tempQosControl[0] == 0)) && (txcfg->pstat->apsd_bitmap & 0x08)) ||
						 (((tempQosControl[0] == 2) || (tempQosControl[0] == 1)) && (txcfg->pstat->apsd_bitmap & 0x04))))
						tempQosControl[0] |= BIT(4);
#endif
					if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
						tempQosControl[0] |= BIT(7);

					if (priv->pmib->dot11nConfigEntry.dot11nTxNoAck)
						tempQosControl[0] |= BIT(5);

					memcpy((void *)GetQosControl((txcfg->phdr)), tempQosControl, 2);
				}
			}
#endif

#if (BEAMFORMING_SUPPORT == 1)
			if(!txcfg->ndpa)
#endif	
			assign_wlanseq(GET_HW(priv), txcfg->phdr, txcfg->pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
				, txcfg->is_11s
#endif
				);

			pdesc->Dword9 |= set_desc((GetSequence(txcfg->phdr) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
			pwlhdr = txcfg->phdr;
		}
#if (BEAMFORMING_SUPPORT == 1)
		if(!txcfg->ndpa)
#endif			
		SetDuration(pwlhdr, 0);		

		if (priv->pmib->dot11nConfigEntry.dot11nTxNoAck){
			pdesc->Dword0 |= set_desc(TX_BMC);
			pdesc->Dword2 &= set_desc(~(TXdesc_92E_AggEn));
		}

		pdesc->Dword0 |= set_desc(40 << TX_OffsetSHIFT); // tx_desc size
		
		if (IS_MCAST(GetAddr1Ptr((unsigned char *)txcfg->phdr)))
			pdesc->Dword0 |= set_desc(TX_BMC);

#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE) {
			if (GetFrameSubType(txcfg->phdr) == WIFI_PSPOLL)
				pdesc->Dword3 |= set_desc(TXdesc_92E_NAVUSEHDR);


			if (priv->ps_state)
				SetPwrMgt(pwlhdr);
			else
				ClearPwrMgt(pwlhdr);
		}
#endif
#if (BEAMFORMING_SUPPORT == 1)
		if(txcfg->ndpa)
			pdesc->Dword3 |= set_desc(TXdesc_92E_NAVUSEHDR);
#endif		

		if (i != (txcfg->frg_num - 1))
		{
			SetMFrag(pwlhdr);
			if (i == 0) {
				fr_len = (txcfg->frag_thrshld - txcfg->llc);
				tx_len -= (txcfg->frag_thrshld - txcfg->llc);
			}
			else {
				fr_len = txcfg->frag_thrshld;
				tx_len -= txcfg->frag_thrshld;
			}
		}
		else	// last seg, or the only seg (frg_num == 1)
		{
			fr_len = tx_len;
			ClearMFrag(pwlhdr);
		}
		SetFragNum((pwlhdr), i);

		if ((i == 0) && (txcfg->fr_type == _SKB_FRAME_TYPE_))
		{
			pdesc->Dword7 |= set_desc((txcfg->hdr_len + txcfg->llc) << TX_TxBufSizeSHIFT);
			pdesc->Dword0 |= set_desc((fr_len + (get_desc(pdesc->Dword7) & TX_TxBufSizeMask)) << TX_PktSizeSHIFT);
			pdesc->Dword0 |= set_desc(TX_FirstSeg);
			pdescinfo->type = _PRE_ALLOCLLCHDR_;
		}
		else
		{
			pdesc->Dword7 |= set_desc(txcfg->hdr_len << TX_TxBufSizeSHIFT);
			pdesc->Dword0 |= set_desc((fr_len + (get_desc(pdesc->Dword7) & TX_TxBufSizeMask)) << TX_PktSizeSHIFT);
			pdesc->Dword0 |= set_desc(TX_FirstSeg);

#if (BEAMFORMING_SUPPORT == 1)
			if(txcfg->ndpa)
				pdescinfo->type = _PRE_ALLOCLLCHDR_;
			else
#endif				
			pdescinfo->type = _PRE_ALLOCHDR_;
		}

		rtl8192cd_fill_fwinfo_8812(priv, txcfg, pdesc, i);
		//Disable all RTS/CTS //for 11ac logo
		if(priv->pshare->rf_ft_var.no_rtscts) {
			pdesc->Dword3 &= set_desc(~(TXdesc_92E_HwRtsEn|TXdesc_92E_RtsEn|TXdesc_92E_CTS2Self));
		} else {
			if(txcfg->pstat) //8812_11n_iot, only vht clnt support cca_rts
				if((priv->pshare->rf_ft_var.cca_rts)
				#ifdef RTK_AC_SUPPORT
					&& (txcfg->pstat->vht_cap_len > 0)
				#endif
				)
					pdesc->Dword2 |= set_desc((priv->pshare->rf_ft_var.cca_rts & 0x03) << TX_8812_CcaRtsSHIFT);	//10b:  RTS support dynamic mode CCA secondary
		}

		switch (q_num) {
		case HIGH_QUEUE:
			q_select = 0x11;// High Queue
			break;
		case MGNT_QUEUE:
			q_select = 0x12;
			break;
#if defined(DRVMAC_LB) && defined(WIFI_WMM)
		case BE_QUEUE:
			q_select = 0;
			break;
#endif
		default:
			// data packet
#ifdef RTL_MANUAL_EDCA
			if (priv->pmib->dot11QosEntry.ManualEDCA) {
				switch (q_num) {
				case VO_QUEUE:
					q_select = 6;
					break;
				case VI_QUEUE:
					q_select = 4;
					break;
				case BE_QUEUE:
					q_select = 0;
					break;
				default:
					q_select = 1;
					break;
				}
			}
			else
#endif
			q_select = ((struct sk_buff *)txcfg->pframe)->cb[1];
				break;
		}
		pdesc->Dword1 |= set_desc((q_select & TX_QSelMask)<< TX_QSelSHIFT);

		if (i != (txcfg->frg_num - 1))
			pdesc->Dword2 |= set_desc(TX_MoreFrag);

// Set RateID
		if (txcfg->pstat) {
			if (txcfg->pstat->aid != MANAGEMENT_AID) {
				u8 ratid;

				ratid = txcfg->pstat->ratr_idx;
								
				pdesc->Dword1 |= set_desc((ratid & TXdesc_92E_RateIDMask) << TX_RateIDSHIFT);
				
	// Set MacID
				pdesc->Dword1 |= set_desc(REMAP_AID(txcfg->pstat) & TXdesc_92E_MacIdMask);

#if (BEAMFORMING_SUPPORT == 1)
				if((priv->pmib->dot11RFEntry.txbf == 1) &&
					((txcfg->pstat->ht_cap_len && (txcfg->pstat->ht_cap_buf.txbf_cap)) ||
					(txcfg->pstat->vht_cap_len && (cpu_to_le32(txcfg->pstat->vht_cap_buf.vht_cap_info) & BIT(SU_BFEE_S))))
				){
					pdesc->Dword2 |= set_desc((txcfg->pstat->p_aid & TX_8812_PAIDMask) << TX_8812_PAIDSHIFT);
					pdesc->Dword2 |= set_desc((txcfg->pstat->g_id & TX_8812_GIDMask) << TX_8812_GIDSHIFT);
				}
#endif
			}
		} else {
#if defined(MBSSID) && defined(HW_ENC_FOR_GROUP_CIPHER)
			if ((txcfg->fr_type == _SKB_FRAME_TYPE_) && IS_MCAST(GetAddr1Ptr ((unsigned char *)txcfg->phdr)) && 
				((txcfg->privacy == _TKIP_PRIVACY_) || (txcfg->privacy == _CCMP_PRIVACY_))) {
				pdesc->Dword1 |= set_desc(TX_EnDescID);
				pdesc->Dword1 |= set_desc((priv->vap_init_seq & TX_MacIdMask) << TX_MacIdSHIFT);
			}
#endif
			pdesc->Dword1 |= set_desc((ARFR_BMC &TXdesc_92E_RateIDMask)<<TX_RateIDSHIFT);

#if (BEAMFORMING_SUPPORT == 1)
				if (priv->pmib->dot11RFEntry.txbf == 1) 
					pdesc->Dword2 |= set_desc((TX_8812_GIDMask) << TX_8812_GIDSHIFT);
#endif				

		}
#if defined(MULTI_STA_REFINE)
		if((priv->pshare->total_assoc_num > 10) && (txcfg->pstat) && ((txcfg->pstat->tx_ra_bitmap & 0xff00000) == 0))
			pdesc->Dword4 |= set_desc((0x03 & TXdesc_92E_DataRateFBLmtMask) << TXdesc_92E_DataRateFBLmtSHIFT);
		else
#endif			
		pdesc->Dword4 |= set_desc((0x1f & TXdesc_92E_DataRateFBLmtMask) << TXdesc_92E_DataRateFBLmtSHIFT);
		
		if (txcfg->fixed_rate) {
			pdesc->Dword3 |= set_desc(TXdesc_92E_DisDataFB|TXdesc_92E_DisRtsFB|TXdesc_92E_UseRate);
		}

		if(txcfg->pstat && txcfg->pstat->sta_in_firmware != 1)
			pdesc->Dword3 |= set_desc(TXdesc_92E_UseRate);

#ifdef CONFIG_IEEE80211W
		if (!txcfg->need_ack && txcfg->privacy && UseSwCrypto(priv, NULL, TRUE, txcfg->isPMF))
#else
		if (!txcfg->need_ack && txcfg->privacy && UseSwCrypto(priv, NULL, TRUE))
#endif // CONFIG_IEEE80211W
			pdesc->Dword1 &= set_desc( ~(TX_SecTypeMask<< TX_SecTypeSHIFT));

		if (txcfg->privacy) {
#ifdef CONFIG_IEEE80211W
			if (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF)) {
#else
			if (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE))) {
#endif // CONFIG_IEEE80211W
				pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0)+ txcfg->icv + txcfg->mic + txcfg->iv);
				pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7)+ txcfg->iv);
			} else {
				// hw encrypt
				switch(txcfg->privacy) {
				case _WEP_104_PRIVACY_:
				case _WEP_40_PRIVACY_:
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					wep_fill_iv(priv, pwlhdr, txcfg->hdr_len, keyid);
					pdesc->Dword1 |= set_desc(0x1 << TX_SecTypeSHIFT);					
					break;

				case _TKIP_PRIVACY_:
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv + txcfg->mic);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					tkip_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
					pdesc->Dword1 |= set_desc(0x1 << TX_SecTypeSHIFT);
					break;
				#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				case _WAPI_SMS4_:
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					
					pdesc->Dword1 |= set_desc(0x2 << TX_SecTypeSHIFT);
					break;
				#endif
				case _CCMP_PRIVACY_:
					//michal also hardware...
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					aes_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
					pdesc->Dword1 |= set_desc(0x3 << TX_SecTypeSHIFT);
					break;

				default:
					DEBUG_ERR("Unknow privacy\n");
					break;
				}
			}
		}

#ifdef TX_EARLY_MODE
		if (GET_TX_EARLY_MODE && pwlhdr && i == 0) {
			pdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & 0xff00ffff) | (0x28 << TX_OffsetSHIFT));
			pdesc->Dword1 = set_desc(get_desc(pdesc->Dword1) | (1 << TX_PktOffsetSHIFT));
			pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + 8);

			memset(pwlhdr-8, '\0', 8);			
			if (txcfg->pstat && txcfg->pstat->empkt_num > 0) 			
				insert_emcontent(priv, txcfg, pwlhdr-8);

			pdesc->Dword10 = set_desc(get_physical_addr(priv, pwlhdr-8,
				get_desc(pdesc->Dword7)&TX_TxBufSizeMask, PCI_DMA_TODEVICE));	
		}
		else
#endif
		{
			pdesc->Dword10 = set_desc(get_physical_addr(priv, pwlhdr,
				(get_desc(pdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE));

		}

		// below is for sw desc info
#ifndef TXDESC_INFO		
		pdescinfo->paddr  = get_desc(pdesc->Dword10);
#endif		
		pdescinfo->pframe = pwlhdr;
#if defined(WIFI_WMM) && defined(WMM_APSD)
		pdescinfo->priv = priv;
#ifndef TXDESC_INFO		
		pdescinfo->pstat = txcfg->pstat;
#endif		
#endif

#ifdef TX_SHORTCUT
		if (!priv->pmib->dot11OperationEntry.disable_txsc && txcfg->pstat &&
				(txcfg->fr_type == _SKB_FRAME_TYPE_) &&
				(txcfg->frg_num == 1) &&
				((txcfg->privacy == 0)
#ifdef CONFIG_RTL_WAPI_SUPPORT
				|| (txcfg->privacy == _WAPI_SMS4_)
#endif
#ifdef CONFIG_IEEE80211W
				|| !UseSwCrypto(priv, txcfg->pstat, FALSE, txcfg->isPMF)) &&
#else
				|| !UseSwCrypto(priv, txcfg->pstat, FALSE)) &&
#endif // CONFIG_IEEE80211W
				!GetMData(txcfg->phdr) ) {

#ifdef CONFIG_RTK_MESH
			if( txcfg->is_11s) {
				idx = get_tx_sc_free_entry(priv, txcfg->pstat, &priv->ethhdr);
			} else
#endif
				idx = get_tx_sc_free_entry(priv, txcfg->pstat, pbuf - sizeof(struct wlan_ethhdr_t));

#ifdef CONFIG_RTK_MESH
			if( txcfg->is_11s) {
				memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, &priv->ethhdr, sizeof(struct wlan_ethhdr_t));
			} else
#endif
				memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, pbuf - sizeof(struct wlan_ethhdr_t), sizeof(struct wlan_ethhdr_t));

			memcpy(&txcfg->pstat->tx_sc_ent[idx].hwdesc1, pdesc, 40);
			descinfo_copy(&txcfg->pstat->tx_sc_ent[idx].swdesc1, pdescinfo);
			txcfg->pstat->protection = priv->pmib->dot11ErpInfo.protection;
			txcfg->pstat->ht_protection = priv->ht_protection;
			txcfg->pstat->tx_sc_ent[idx].sc_keyid = keyid;
			txcfg->pstat->tx_sc_ent[idx].pktpri = ((struct sk_buff *)txcfg->pframe)->cb[1];
			fit_shortcut = 1;
		}
		else {
			if (txcfg->pstat) {
				for (idx=0; idx<TX_SC_ENTRY_NUM; idx++)
					txcfg->pstat->tx_sc_ent[idx].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
			}
		}
#endif

		pfrst_dma_desc = dma_txhead[*tx_head];

		if (i != 0) {
			pdesc->Dword0 |= set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
			rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
		}

		flush_addr[flush_num]  = (unsigned long)bus_to_virt(get_desc(pdesc->Dword10));
		flush_len[flush_num++]= (get_desc(pdesc->Dword7) & TX_TxBufSizeMask);

/*
		//printk desc content
		{
			unsigned int *ppdesc = (unsigned int *)pdesc;
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc)), get_desc(*(ppdesc+1)), get_desc(*(ppdesc+2)));
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc+3)), get_desc(*(ppdesc+4)), get_desc(*(ppdesc+5)));
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc+6)), get_desc(*(ppdesc+7)), get_desc(*(ppdesc+8)));
			printk("%08x\n", *(ppdesc+9));
			printk("===================================================\n");
		}
*/

#ifdef OSK_LOW_TX_DESC
		if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
			txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
		else
#endif
		txdesc_rollover(pdesc, (unsigned int *)tx_head);

		if (txcfg->fr_len == 0)
		{
			if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
				pdesc->Dword0 |= set_desc(TX_LastSeg);
			goto init_deschead;
		}

		/*------------------------------------------------------------*/
		/*              fill descriptor of frame body                 */
		/*------------------------------------------------------------*/
		pndesc     = phdesc + *tx_head;
		pndescinfo = pswdescinfo + *tx_head;
		//clear all bits

		memset(pndesc, 0,40);
		pndesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | (TX_OWN));



#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	if(txcfg->pstat)
	{
		ODM_SetTxAntByTxInfo(priv, pdesc, txcfg->pstat->aid);  //AC-series Set TX antenna
	}
#endif


		if (txcfg->privacy)
		{
			if (txcfg->privacy == _WAPI_SMS4_)
			{
				if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
					pndesc->Dword0 |= set_desc(TX_LastSeg);
#ifndef TXDESC_INFO						
				pndescinfo->pstat = txcfg->pstat;
				pndescinfo->rate = txcfg->tx_rate;
#endif				
			}
#ifdef CONFIG_IEEE80211W			
			else if (!UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE),txcfg->isPMF))
#else
			else if (!UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
#endif // CONFIG_IEEE80211W
			{
				if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
					pndesc->Dword0 |= set_desc(TX_LastSeg);
#ifndef TXDESC_INFO						
				pndescinfo->pstat = txcfg->pstat;
				pndescinfo->rate = txcfg->tx_rate;
#endif				
			}
		}
		else
		{
			if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
				pndesc->Dword0 |= set_desc(TX_LastSeg);
#ifndef TXDESC_INFO					
			pndescinfo->pstat = txcfg->pstat;
			pndescinfo->rate = txcfg->tx_rate;
#endif			
		}

#ifdef CONFIG_RTL_WAPI_SUPPORT
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
#ifdef CONFIG_IEEE80211W
		if ((txcfg->privacy == _WAPI_SMS4_)&&(UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF)))
#else
		if ((txcfg->privacy == _WAPI_SMS4_)&&(UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE))))
#endif
#else
		if (txcfg->privacy == _WAPI_SMS4_)
#endif
		{
			pndesc->Dword7 |= set_desc( (fr_len+SMS4_MIC_LEN) & TX_TxBufSizeMask);
		}
		else
#endif
		pndesc->Dword7 |= set_desc(fr_len & TX_TxBufSizeMask);

		if (i == 0)
			pndescinfo->type = txcfg->fr_type;
		else
			pndescinfo->type = _RESERVED_FRAME_TYPE_;

#if defined(CONFIG_RTK_MESH) && defined(MESH_USE_METRICOP)
		if( (txcfg->fr_type == _PRE_ALLOCMEM_) && (txcfg->is_11s & 128)) // for 11s link measurement frame
			pndescinfo->type =_RESERVED_FRAME_TYPE_;
#endif


		pndesc->Dword10 = set_desc(tmpphyaddr); //TxBufferAddr
#ifndef TXDESC_INFO		
		pndescinfo->paddr = get_desc(pndesc->Dword10);
#endif		
		pndescinfo->pframe = txcfg->pframe;
#ifndef TXDESC_INFO		
		pndescinfo->len = txcfg->fr_len;	// for pci_unmap_single
#endif		
		pndescinfo->priv = priv;

		pbuf += fr_len;
		tmpphyaddr += fr_len;

#ifdef TX_SHORTCUT
		if (fit_shortcut) {
			memcpy(&txcfg->pstat->tx_sc_ent[idx].hwdesc2, pndesc, 40);
			descinfo_copy(&txcfg->pstat->tx_sc_ent[idx].swdesc2, pndescinfo);
#ifdef TXSC_CACHE_FRLEN			
			txcfg->pstat->tx_sc_ent[idx].frlen = fr_len;
#endif			
		}
#endif

#ifndef USE_RTL8186_SDK
		rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

		flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(pndesc->Dword10));
		flush_len[flush_num++] = get_desc(pndesc->Dword7) & TX_TxBufSizeMask;

#ifdef OSK_LOW_TX_DESC
		if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
			txdesc_rollover_nonbe(pndesc, (unsigned int *)tx_head);
		else
#endif
		txdesc_rollover(pndesc, (unsigned int *)tx_head);

		// retrieve H/W MIC and put in payload
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (txcfg->privacy == _WAPI_SMS4_)
		{
			SecSWSMS4Encryption(priv, txcfg);
#if 0
			if (txcfg->mic>0)
			{
				pndesc->Dword7 &= set_desc(~TX_TxBufSizeMask);
				pndesc->Dword7 |= set_desc((fr_len+txcfg->mic)& TX_TxBufSizeMask);
				flush_len[flush_num-1]= (get_desc(pndesc->Dword7) & TX_TxBufSizeMask);
			}
			else
			{
				txcfg->mic = SMS4_MIC_LEN;
			}
#endif
		
		}
#endif

#ifndef NOT_RTK_BSP
		if ((txcfg->privacy == _TKIP_PRIVACY_) &&
			(priv->pshare->have_hw_mic) &&
			!(priv->pmib->dot11StationConfigEntry.swTkipMic) &&
			(i == (txcfg->frg_num-1)) )	// last segment
		{
			register unsigned long int l,r;
			unsigned char *mic;
			volatile int i;

			while ((*(volatile unsigned int *)GDMAISR & GDMA_COMPIP) == 0)
				for (i=0; i<10; i++)
					;

			l = *(volatile unsigned int *)GDMAICVL;
			r = *(volatile unsigned int *)GDMAICVR;

			mic = ((struct sk_buff *)txcfg->pframe)->data + txcfg->fr_len - 8;
			mic[0] = (unsigned char)(l & 0xff);
			mic[1] = (unsigned char)((l >> 8) & 0xff);
			mic[2] = (unsigned char)((l >> 16) & 0xff);
			mic[3] = (unsigned char)((l >> 24) & 0xff);
			mic[4] = (unsigned char)(r & 0xff);
			mic[5] = (unsigned char)((r >> 8) & 0xff);
			mic[6] = (unsigned char)((r >> 16) & 0xff);
			mic[7] = (unsigned char)((r >> 24) & 0xff);

#ifdef MICERR_TEST
			if (priv->micerr_flag) {
				mic[7] ^= mic[7];
				priv->micerr_flag = 0;
			}
#endif
		}
#endif // !NOT_RTK_BSP

		/*------------------------------------------------------------*/
		/*                insert sw encrypt here!                     */
		/*------------------------------------------------------------*/
#ifdef CONFIG_IEEE80211W
		if (txcfg->privacy && UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF))
#else
		if (txcfg->privacy && UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
#endif			
		{
			if (txcfg->privacy == _TKIP_PRIVACY_ ||
				txcfg->privacy == _WEP_40_PRIVACY_ ||
				txcfg->privacy == _WEP_104_PRIVACY_)
			{
				picvdesc     = phdesc + *tx_head;
				picvdescinfo = pswdescinfo + *tx_head;
				//clear all bits
				memset(picvdesc, 0,32);

				if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST){
					picvdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN);
				}
				else{
					picvdesc->Dword0   = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN | TX_LastSeg);
				}

				picvdesc->Dword7 |= (set_desc(txcfg->icv & TX_TxBufSizeMask)); //TxBufferSize

				// append ICV first...
				picv = get_icv_from_poll(priv);
				if (picv == NULL)
				{
					DEBUG_ERR("System-Buf! can't alloc picv\n");
					BUG();
				}

				picvdescinfo->type = _PRE_ALLOCICVHDR_;
				picvdescinfo->pframe = picv;
#ifndef TXDESC_INFO					
				picvdescinfo->pstat = txcfg->pstat;
				picvdescinfo->rate = txcfg->tx_rate;
#endif				
				picvdescinfo->priv = priv;
				//TxBufferAddr

				picvdesc->Dword10 = set_desc(get_physical_addr(priv, picv, txcfg->icv, PCI_DMA_TODEVICE));

				if (i == 0)
					tkip_icv(picv,
						pwlhdr + txcfg->hdr_len + txcfg->iv, txcfg->llc,
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask));
				else
					tkip_icv(picv,
						NULL, 0,
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask));

				if ((i == 0) && (txcfg->llc != 0)) {
					if (txcfg->privacy == _TKIP_PRIVACY_)
						tkip_encrypt(priv, pwlhdr, txcfg->hdr_len,
							pwlhdr + txcfg->hdr_len + 8, sizeof(struct llc_snap),
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv);
					else
						wep_encrypt(priv, pwlhdr, txcfg->hdr_len,
							pwlhdr + txcfg->hdr_len + 4, sizeof(struct llc_snap),
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv,
							txcfg->privacy);
				}
				else { // not first segment or no snap header
					if (txcfg->privacy == _TKIP_PRIVACY_)
						tkip_encrypt(priv, pwlhdr, txcfg->hdr_len, NULL, 0,
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv);
					else
						wep_encrypt(priv, pwlhdr, txcfg->hdr_len, NULL, 0,
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv,
							txcfg->privacy);
				}
#ifndef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

				flush_addr[flush_num]  = (unsigned long)bus_to_virt(get_desc(picvdesc->Dword10)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET);
				flush_len[flush_num++]=(get_desc(picvdesc->Dword7) & TX_TxBufSizeMask);

#ifdef OSK_LOW_TX_DESC
				if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
					txdesc_rollover_nonbe(picvdesc, (unsigned int *)tx_head);
				else
#endif
				txdesc_rollover(picvdesc, (unsigned int *)tx_head);
			}

			else if (txcfg->privacy == _CCMP_PRIVACY_)
			{
				pmicdesc = phdesc + *tx_head;
				pmicdescinfo = pswdescinfo + *tx_head;

				//clear all bits
				memset(pmicdesc, 0,32);
				
				if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
					pmicdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN);
				else
				  pmicdesc->Dword0   = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN | TX_LastSeg);

				// set TxBufferSize
				pmicdesc->Dword7 = set_desc(txcfg->mic & TX_TxBufSizeMask);

				// append MIC first...
				pmic = get_mic_from_poll(priv);
				if (pmic == NULL)
				{
					DEBUG_ERR("System-Buf! can't alloc pmic\n");
					BUG();
				}

				pmicdescinfo->type = _PRE_ALLOCMICHDR_;
				pmicdescinfo->pframe = pmic;
#ifndef TXDESC_INFO					
				pmicdescinfo->pstat = txcfg->pstat;
				pmicdescinfo->rate = txcfg->tx_rate;
#endif				
				pmicdescinfo->priv = priv;
				// set TxBufferAddr
				pmicdesc->Dword10= set_desc(get_physical_addr(priv, pmic, txcfg->mic, PCI_DMA_TODEVICE));

				// then encrypt all (including ICV) by AES
				if ((i == 0)&&(txcfg->llc != 0)) // encrypt 3 segments ==> llc, mpdu, and mic
				{
				
#ifdef CONFIG_IEEE80211W
					if(isBIP) {
						BIP_encrypt(priv, pwlhdr, pwlhdr + txcfg->hdr_len + 8,
									pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					} 
					else {
						aesccmp_encrypt(priv, pwlhdr, pwlhdr + txcfg->hdr_len + 8,
										pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					}
#else
					aesccmp_encrypt(priv, pwlhdr, pwlhdr + txcfg->hdr_len + 8,
									pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic);
#endif
				
				}
				else {// encrypt 2 segments ==> mpdu and mic
#ifdef CONFIG_IEEE80211W
					if(isBIP) 
					{
						BIP_encrypt(priv, pwlhdr, NULL,
									pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					}
					else
					{
						aesccmp_encrypt(priv, pwlhdr, NULL,
										pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					}
#else // !defined(CONFIG_IEEE80211W)
					aesccmp_encrypt(priv, pwlhdr, NULL,
									pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic);
#endif // CONFIG_IEEE80211W
				}
#ifndef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

				flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(pmicdesc->Dword10)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET);
				flush_len[flush_num++]= (get_desc(pmicdesc->Dword7) & TX_TxBufSizeMask);

#ifdef OSK_LOW_TX_DESC
				if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
					txdesc_rollover_nonbe(pmicdesc, (unsigned int *)tx_head);
				else
#endif
				txdesc_rollover(pmicdesc, (unsigned int *)tx_head);
			}
		}

#ifdef RESERVE_TXDESC_FOR_EACH_IF
		if (RSVQ_ENABLE && IS_RSVQ(q_num)) {
			if ( txcfg->privacy
#if defined(CONFIG_RTL_WAPI_SUPPORT)
				&& (_WAPI_SMS4_ != txcfg->privacy)
#endif		
#ifdef CONFIG_IEEE80211W
				&& UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF) ){
#else
				&& UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)) ){
#endif						
				priv->use_txdesc_cnt[RSVQ(q_num)] += 3;
			}
			else
				priv->use_txdesc_cnt[RSVQ(q_num)] += 2;
		}
#endif

	}


init_deschead:
#if 0
	switch (q_select) {
	case 0:
	case 3:
	   if (q_num != BE_QUEUE)
    		printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
	   break;
	case 1:
	case 2:
		if (q_num != BK_QUEUE)
		    printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
	   break;
	case 4:
	case 5:
		if (q_num != VI_QUEUE)
		    printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	case 6:
	case 7:
		if (q_num != VO_QUEUE)
			printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	case 0x11 :
		 if (q_num != HIGH_QUEUE)
			printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	case 0x12 :
		if (q_num != MGNT_QUEUE)
			printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	default :
		printk("%s %d warning : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
	break;
	}
#endif

	for (i=0; i<flush_num; i++)
		rtl_cache_sync_wback(priv, flush_addr[i], flush_len[i], PCI_DMA_TODEVICE);

	if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
		priv->amsdu_first_desc = pfrstdesc;
#ifndef USE_RTL8186_SDK
		priv->amsdu_first_dma_desc = pfrst_dma_desc;
#endif
		priv->amsdu_len = get_desc(pfrstdesc->Dword0) & 0xffff; // get pktSize
		return;
	}

	pfrstdesc->Dword0 |= set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(pfrst_dma_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

	if (q_num == HIGH_QUEUE) {
//		priv->pshare->pkt_in_hiQ = 1;
		priv->pkt_in_hiQ = 1;

		return;
	} else {
		tx_poll(priv, q_num);
	}

	return;
}
#endif // CONFIG_RTL_8812_SUPPORT

// I AM not sure that if our Buffersize and PKTSize is right,
// If there are any problem, fix this first
#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
void rtl8192cd_signin_txdesc(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	struct tx_desc		*phdesc, *pdesc, *pndesc, *picvdesc, *pmicdesc, *pfrstdesc;
	struct tx_desc_info	*pswdescinfo, *pdescinfo, *pndescinfo, *picvdescinfo, *pmicdescinfo;
	unsigned int 		fr_len, tx_len, i, keyid;
	int					*tx_head, q_num;
	unsigned long		tmpphyaddr;
	unsigned char		*da, *pbuf, *pwlhdr, *pmic, *picv;
	struct rtl8192cd_hw	*phw;
	unsigned char		 q_select;
#ifdef TX_SHORTCUT
	int					fit_shortcut=0, idx=0;
#endif
	unsigned long		pfrst_dma_desc=0;
	unsigned long		*dma_txhead;

	unsigned long		flush_addr[20];
	int					flush_len[20];
	int					flush_num=0;

#ifdef TX_SCATTER
	int				actual_size = 0;
	struct sk_buff	*skb=NULL;
#endif
#ifdef CONFIG_IEEE80211W
	unsigned int	isBIP = 0;
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT) 
	if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv)== VERSION_8723B){
		return rtl8192cd_signin_txdesc_8812(priv, txcfg);
	}
#endif

	picvdesc=NULL;
	keyid=0;
	pmic=NULL;
	picv=NULL;
	q_select=0;

	if (txcfg->tx_rate == 0) {
		DEBUG_ERR("tx_rate=0!\n");
		txcfg->tx_rate = find_rate(priv, NULL, 0, 1);
	}

	q_num = txcfg->q_num;

	phw	= GET_HW(priv);

	dma_txhead	= get_txdma_addr(phw, q_num);
	tx_head		= get_txhead_addr(phw, q_num);
	phdesc   	= get_txdesc(phw, q_num);
	pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);

	da = get_da((unsigned char *)txcfg->phdr);
	
#ifdef CONFIG_IEEE80211W
	if(txcfg->isPMF && IS_MCAST(da)) 
	{
		isBIP = 1;
		txcfg->iv = 0;
		txcfg->fr_len += 10; // 10: MMIE length
	}
#endif
	tx_len = txcfg->fr_len;

	if (txcfg->fr_type == _SKB_FRAME_TYPE_)
		pbuf = ((struct sk_buff *)txcfg->pframe)->data;
	else
		pbuf = (unsigned char*)txcfg->pframe;

#ifdef TX_SCATTER
	if (txcfg->fr_type == _SKB_FRAME_TYPE_ &&
			((struct sk_buff *)txcfg->pframe)->list_num > 0) {
		skb = (struct sk_buff *)txcfg->pframe;
		actual_size = skb->len;
	} else {
		actual_size = tx_len;
	}

	tmpphyaddr = get_physical_addr(priv, pbuf, actual_size, PCI_DMA_TODEVICE);
#else
	tmpphyaddr = get_physical_addr(priv, pbuf, tx_len, PCI_DMA_TODEVICE);
#endif

	// in case of default key, then find the key id
	if (GetPrivacy((txcfg->phdr)))
	{
#ifdef WDS
		if (txcfg->wdsIdx >= 0) {
			if (txcfg->pstat)
				keyid = txcfg->pstat->keyid;
			else
				keyid = 0;
		}
		else
#endif

#ifdef __DRAYTEK_OS__
		if (!IEEE8021X_FUN)
			keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		else {
			if (IS_MCAST(GetAddr1Ptr ((unsigned char *)txcfg->phdr)) || !txcfg->pstat)
				keyid = priv->pmib->dot11GroupKeysTable.keyid;
			else
				keyid = txcfg->pstat->keyid;
		}
#else

		if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_40_PRIVACY_ ||
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_WEP_104_PRIVACY_) {
			if(IEEE8021X_FUN && txcfg->pstat) {
#ifdef A4_STA
				if (IS_MCAST(da) && !(txcfg->pstat->state & WIFI_A4_STA))
#else
				if(IS_MCAST(da))
#endif					
					keyid = 0;
				else
					keyid = txcfg->pstat->keyid;
			}
			else
				keyid = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
		}
#endif


	}

	for(i=0, pfrstdesc= phdesc + (*tx_head); i < txcfg->frg_num; i++)
	{
		/*------------------------------------------------------------*/
		/*           fill descriptor of header + iv + llc             */
		/*------------------------------------------------------------*/
		pdesc     = phdesc + (*tx_head);
		pdescinfo = pswdescinfo + *tx_head;

		//clear all bits
		memset(pdesc, 0, 32);

		if (i != 0)
		{
			// we have to allocate wlan_hdr
			pwlhdr = (UINT8 *)get_wlanhdr_from_poll(priv);
			if (pwlhdr == (UINT8 *)NULL)
			{
				DEBUG_ERR("System-bug... should have enough wlan_hdr\n");
				return;
			}
			// other MPDU will share the same seq with the first MPDU
			memcpy((void *)pwlhdr, (void *)(txcfg->phdr), txcfg->hdr_len); // data pkt has 24 bytes wlan_hdr
			pdesc->Dword3 |= set_desc((GetSequence(txcfg->phdr) & TX_SeqMask)  << TX_SeqSHIFT);
		}
		else
		{
#ifdef WIFI_WMM
			if (txcfg->pstat && (is_qos_data(txcfg->phdr))) {
				if ((GetFrameSubType(txcfg->phdr) & (WIFI_DATA_TYPE | BIT(6) | BIT(7))) == (WIFI_DATA_TYPE | BIT(7))) {
					unsigned char tempQosControl[2];
					memset(tempQosControl, 0, 2);
					tempQosControl[0] = ((struct sk_buff *)txcfg->pframe)->cb[1];
#ifdef WMM_APSD
					if (
#ifdef CLIENT_MODE
						(OPMODE & WIFI_AP_STATE) &&
#endif
						(APSD_ENABLE) && (txcfg->pstat) && (txcfg->pstat->state & WIFI_SLEEP_STATE) &&
						(!GetMData(txcfg->phdr)) &&
						((((tempQosControl[0] == 7) || (tempQosControl[0] == 6)) && (txcfg->pstat->apsd_bitmap & 0x01)) ||
						 (((tempQosControl[0] == 5) || (tempQosControl[0] == 4)) && (txcfg->pstat->apsd_bitmap & 0x02)) ||
						 (((tempQosControl[0] == 3) || (tempQosControl[0] == 0)) && (txcfg->pstat->apsd_bitmap & 0x08)) ||
						 (((tempQosControl[0] == 2) || (tempQosControl[0] == 1)) && (txcfg->pstat->apsd_bitmap & 0x04))))
						tempQosControl[0] |= BIT(4);
#endif
					if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
						tempQosControl[0] |= BIT(7);

					if (priv->pmib->dot11nConfigEntry.dot11nTxNoAck)
						tempQosControl[0] |= BIT(5);

					memcpy((void *)GetQosControl((txcfg->phdr)), tempQosControl, 2);
				}
			}
#endif

			assign_wlanseq(GET_HW(priv), txcfg->phdr, txcfg->pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
				, txcfg->is_11s
#endif
				);
			pdesc->Dword3 |= set_desc((GetSequence(txcfg->phdr) & TX_SeqMask)  << TX_SeqSHIFT);
			pwlhdr = txcfg->phdr;
		}
		SetDuration(pwlhdr, 0);

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		if(txcfg->pstat)
		{
			ODM_SetTxAntByTxInfo(priv, pdesc,txcfg->pstat->aid);   //N-series Set TX antenna	
		}
#endif
#if defined (HW_ANT_SWITCH) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
		pdesc->Dword2 &= set_desc(~ BIT(24));
		pdesc->Dword2 &= set_desc(~ BIT(25));
		if(!(priv->pshare->rf_ft_var.CurAntenna & 0x80) && (txcfg->pstat)) {
			pdesc->Dword2 |= set_desc(((txcfg->pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)<<24);
			pdesc->Dword2 |= set_desc(((txcfg->pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)<<25);
		}
#endif

		pdesc->Dword0 |= set_desc(32 << TX_OffsetSHIFT); // tx_desc size

		if (IS_MCAST(GetAddr1Ptr((unsigned char *)txcfg->phdr)))
			pdesc->Dword0 |= set_desc(TX_BMC);

#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE) {
			if (GetFrameSubType(txcfg->phdr) == WIFI_PSPOLL)
				pdesc->Dword1 |= set_desc(TX_NAVUSEHDR);

			if (priv->ps_state)
				SetPwrMgt(pwlhdr);
			else
				ClearPwrMgt(pwlhdr);
		}
#endif

		if (i != (txcfg->frg_num - 1))
		{
			SetMFrag(pwlhdr);
			if (i == 0) {
				fr_len = (txcfg->frag_thrshld - txcfg->llc);
				tx_len -= (txcfg->frag_thrshld - txcfg->llc);
			}
			else {
				fr_len = txcfg->frag_thrshld;
				tx_len -= txcfg->frag_thrshld;
			}
		}
		else	// last seg, or the only seg (frg_num == 1)
		{
			fr_len = tx_len;
			ClearMFrag(pwlhdr);
		}
		SetFragNum((pwlhdr), i);

		if ((i == 0) && (txcfg->fr_type == _SKB_FRAME_TYPE_))
		{
			pdesc->Dword7 |= set_desc((txcfg->hdr_len + txcfg->llc) << TX_TxBufSizeSHIFT);
			pdesc->Dword0 |= set_desc((fr_len + (get_desc(pdesc->Dword7) & TX_TxBufSizeMask)) << TX_PktSizeSHIFT);
			pdesc->Dword0 |= set_desc(TX_FirstSeg);
			pdescinfo->type = _PRE_ALLOCLLCHDR_;
		}
		else
		{
			pdesc->Dword7 |= set_desc(txcfg->hdr_len << TX_TxBufSizeSHIFT);
			pdesc->Dword0 |= set_desc((fr_len + (get_desc(pdesc->Dword7) & TX_TxBufSizeMask)) << TX_PktSizeSHIFT);
			pdesc->Dword0 |= set_desc(TX_FirstSeg);
			pdescinfo->type = _PRE_ALLOCHDR_;
		}

		rtl8192cd_fill_fwinfo(priv, txcfg, pdesc, i);

		switch (q_num) {
		case HIGH_QUEUE:
			q_select = 0x11;// High Queue
			break;
		case MGNT_QUEUE:
			q_select = 0x12;
			break;
#if defined(DRVMAC_LB) && defined(WIFI_WMM)
		case BE_QUEUE:
			q_select = 0;
			break;
#endif
		default:
			// data packet
#ifdef RTL_MANUAL_EDCA
			if (priv->pmib->dot11QosEntry.ManualEDCA) {
				switch (q_num) {
				case VO_QUEUE:
					q_select = 6;
					break;
				case VI_QUEUE:
					q_select = 4;
					break;
				case BE_QUEUE:
					q_select = 0;
					break;
				default:
					q_select = 1;
					break;
				}
			}
			else
#endif
			q_select = ((struct sk_buff *)txcfg->pframe)->cb[1];
				break;
		}
		pdesc->Dword1 |= set_desc((q_select & TX_QSelMask)<< TX_QSelSHIFT);

		if (i != (txcfg->frg_num - 1))
			pdesc->Dword2 |= set_desc(TX_MoreFrag);

// Set RateID
		if (txcfg->pstat) {
			if (txcfg->pstat->aid != MANAGEMENT_AID) {
				u8 ratid;

#ifdef CONFIG_RTL_92D_SUPPORT
				if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G){
					if (txcfg->pstat->tx_ra_bitmap & 0xffff000) {
						if (priv->pshare->is_40m_bw)
							ratid = ARFR_2T_Band_A_40M;
						else
							ratid = ARFR_2T_Band_A_20M;
					} else {
						ratid = ARFR_G_ONLY;
					}
				} else 
#endif
				{			
					if (txcfg->pstat->tx_ra_bitmap & 0xff00000) {
						if (priv->pshare->is_40m_bw)
							ratid = ARFR_2T_40M;
						else
							ratid = ARFR_2T_20M;
					} else if (txcfg->pstat->tx_ra_bitmap & 0xff000) {
						if (priv->pshare->is_40m_bw)
							ratid = ARFR_1T_40M;
						else
							ratid = ARFR_1T_20M;
					} else if (txcfg->pstat->tx_ra_bitmap & 0xff0) {
						ratid = ARFR_BG_MIX;
					} else {
						ratid = ARFR_B_ONLY;
					}


#ifdef P2P_SUPPORT	/*tx to GC no user B rate*/
					if(txcfg->pstat->is_p2p_client){
						switch(ratid) {
							case  ARFR_BG_MIX :
								ratid = ARFR_G_ONLY;
								break;
							default:
								ratid = ARFR_2T_Band_A_40M;		
						}
					}
#endif				
				}
				pdesc->Dword1 |= set_desc((ratid & TX_RateIDMask) << TX_RateIDSHIFT);
	// Set MacID
#ifdef CONFIG_RTL_88E_SUPPORT
				if (GET_CHIP_VER(priv)==VERSION_8188E)
					pdesc->Dword1 |= set_desc(REMAP_AID(txcfg->pstat) & TXdesc_88E_MacIdMask);
				else
#endif
					pdesc->Dword1 |= set_desc(REMAP_AID(txcfg->pstat) & TX_MacIdMask);
			}
		} else {
	
#ifdef CONFIG_RTL_92D_SUPPORT
		if (priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G)
			pdesc->Dword1 |= set_desc((ARFR_Band_A_BMC &TX_RateIDMask)<<TX_RateIDSHIFT);
		else
#endif
			pdesc->Dword1 |= set_desc((ARFR_BMC &TX_RateIDMask)<<TX_RateIDSHIFT);
	}

		pdesc->Dword5 |= set_desc((0x1f & TX_DataRateFBLmtMask) << TX_DataRateFBLmtSHIFT);
		if (txcfg->fixed_rate)
			pdesc->Dword4 |= set_desc(TX_DisDataFB|TX_DisRtsFB|TX_UseRate);
#ifdef CONFIG_RTL_88E_SUPPORT
		else if (GET_CHIP_VER(priv)==VERSION_8188E)
			pdesc->Dword4 |= set_desc(TX_UseRate);
#endif

		if(txcfg->pstat &&  txcfg->pstat->sta_in_firmware != 1)
			pdesc->Dword4 |= set_desc(TX_UseRate);

		if (!txcfg->need_ack && txcfg->privacy && 
#ifdef CONFIG_IEEE80211W
			UseSwCrypto(priv, NULL, TRUE, txcfg->isPMF)
#else
			UseSwCrypto(priv, NULL, TRUE)
#endif		
	 	)
			pdesc->Dword1 &= set_desc( ~(TX_SecTypeMask<< TX_SecTypeSHIFT));

		if (txcfg->privacy) {
#ifdef CONFIG_IEEE80211W
			if (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF
#else
			if (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)
#endif				
			)){
				pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0)+ txcfg->icv + txcfg->mic + txcfg->iv);
				pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7)+ txcfg->iv);
			} else {
				// hw encrypt
				switch(txcfg->privacy) {
				case _WEP_104_PRIVACY_:
				case _WEP_40_PRIVACY_:
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					wep_fill_iv(priv, pwlhdr, txcfg->hdr_len, keyid);
					pdesc->Dword1 |= set_desc(0x1 << TX_SecTypeSHIFT);
					break;

				case _TKIP_PRIVACY_:
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv + txcfg->mic);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					tkip_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
					pdesc->Dword1 |= set_desc(0x1 << TX_SecTypeSHIFT);
					break;
				#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
				case _WAPI_SMS4_:
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					
					pdesc->Dword1 |= set_desc(0x2 << TX_SecTypeSHIFT);
					break;
				#endif
				case _CCMP_PRIVACY_:
					//michal also hardware...
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					aes_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
					pdesc->Dword1 |= set_desc(0x3 << TX_SecTypeSHIFT);
					break;

				default:
					DEBUG_ERR("Unknow privacy\n");
					break;
				}
			}
		}

#ifdef TX_EARLY_MODE
		if (GET_TX_EARLY_MODE && pwlhdr && i == 0) {
			pdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & 0xff00ffff) | (0x28 << TX_OffsetSHIFT));
			pdesc->Dword1 = set_desc(get_desc(pdesc->Dword1) | (1 << TX_PktOffsetSHIFT));
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E) {
				pdesc->Dword6 &= set_desc(~ (0xf << TX_MaxAggNumSHIFT));
				pdesc->Dword6 |= set_desc(0xf << TX_MaxAggNumSHIFT);
			}
#endif
			pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + 8);

			memset(pwlhdr-8, '\0', 8);			
			if (txcfg->pstat && txcfg->pstat->empkt_num > 0) 			
				insert_emcontent(priv, txcfg, pwlhdr-8);
			pdesc->Dword8 = set_desc(get_physical_addr(priv, pwlhdr-8,
				get_desc(pdesc->Dword7)&TX_TxBufSizeMask, PCI_DMA_TODEVICE));	
		}
		else
#endif
		{
#if defined(TX_EARLY_MODE) && defined(CONFIG_RTL_88E_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8188E)
				pdesc->Dword6 &= set_desc(~ (0xf << TX_MaxAggNumSHIFT));
#endif
			pdesc->Dword8 = set_desc(get_physical_addr(priv, pwlhdr,
				(get_desc(pdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE));
		}

		// below is for sw desc info
#ifndef TXDESC_INFO
		pdescinfo->paddr  = get_desc(pdesc->Dword8);
#endif
		pdescinfo->pframe = pwlhdr;
#if defined(WIFI_WMM) && defined(WMM_APSD)
		pdescinfo->priv = priv;
#ifndef TXDESC_INFO		
		pdescinfo->pstat = txcfg->pstat;
#endif		
#endif

#ifdef TX_SHORTCUT
		if (!priv->pmib->dot11OperationEntry.disable_txsc && txcfg->pstat &&
				(txcfg->fr_type == _SKB_FRAME_TYPE_) &&
				(txcfg->frg_num == 1) &&
				((txcfg->privacy == 0)
#ifdef CONFIG_RTL_WAPI_SUPPORT
				|| (txcfg->privacy == _WAPI_SMS4_)
#endif
#ifdef CONFIG_IEEE80211W
				|| !UseSwCrypto(priv, txcfg->pstat, FALSE,txcfg->isPMF)) &&
#else
				|| !UseSwCrypto(priv, txcfg->pstat, FALSE)) &&
#endif // CONFIG_IEEE80211W
				!GetMData(txcfg->phdr) ) {

#ifdef CONFIG_RTK_MESH
			if( txcfg->is_11s) {
				idx = get_tx_sc_free_entry(priv, txcfg->pstat, &priv->ethhdr);
			} else
#endif
				idx = get_tx_sc_free_entry(priv, txcfg->pstat, pbuf - sizeof(struct wlan_ethhdr_t));

#ifdef CONFIG_RTK_MESH
			if( txcfg->is_11s) {
				memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, &priv->ethhdr, sizeof(struct wlan_ethhdr_t));
			} else
#endif
#ifdef TX_SCATTER
			if (((struct sk_buff *)txcfg->pframe)->list_num > 0)
				memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, ((struct sk_buff *)txcfg->pframe)->list_buf[0].buf, sizeof(struct wlan_ethhdr_t));
			else	
#endif
				memcpy((void *)&txcfg->pstat->tx_sc_ent[idx].ethhdr, pbuf - sizeof(struct wlan_ethhdr_t), sizeof(struct wlan_ethhdr_t));

			desc_copy(&txcfg->pstat->tx_sc_ent[idx].hwdesc1, pdesc);
			descinfo_copy(&txcfg->pstat->tx_sc_ent[idx].swdesc1, pdescinfo);
			txcfg->pstat->protection = priv->pmib->dot11ErpInfo.protection;
			txcfg->pstat->ht_protection = priv->ht_protection;
			txcfg->pstat->tx_sc_ent[idx].sc_keyid = keyid;
			txcfg->pstat->tx_sc_ent[idx].pktpri = ((struct sk_buff *)txcfg->pframe)->cb[1];
			fit_shortcut = 1;
		}
		else {
			if (txcfg->pstat) {
				for (idx=0; idx<TX_SC_ENTRY_NUM; idx++) {
					txcfg->pstat->tx_sc_ent[idx].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
#ifdef TX_SCATTER
					txcfg->pstat->tx_sc_ent[idx].has_desc3 = 0;
#endif
				}
			}
		}
#endif

		pfrst_dma_desc = dma_txhead[*tx_head];

		if (i != 0) {
			pdesc->Dword0 |= set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
			rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
		}

		flush_addr[flush_num]  = (unsigned long)bus_to_virt(get_desc(pdesc->Dword8)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET);
		flush_len[flush_num++]= (get_desc(pdesc->Dword7) & TX_TxBufSizeMask);

/*
		//printk desc content
		{
			unsigned int *ppdesc = (unsigned int *)pdesc;
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc)), get_desc(*(ppdesc+1)), get_desc(*(ppdesc+2)));
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc+3)), get_desc(*(ppdesc+4)), get_desc(*(ppdesc+5)));
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc+6)), get_desc(*(ppdesc+7)), get_desc(*(ppdesc+8)));
			printk("%08x\n", *(ppdesc+9));
			printk("===================================================\n");
		}
*/

#ifdef OSK_LOW_TX_DESC
		if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
			txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
		else
#endif
		txdesc_rollover(pdesc, (unsigned int *)tx_head);

		if (txcfg->fr_len == 0)
		{
			if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
				pdesc->Dword0 |= set_desc(TX_LastSeg);
			goto init_deschead;
		}

#ifdef TX_SCATTER
fill_body:
#endif
		/*------------------------------------------------------------*/
		/*              fill descriptor of frame body                 */
		/*------------------------------------------------------------*/
		pndesc     = phdesc + *tx_head;
		pndescinfo = pswdescinfo + *tx_head;
		//clear all bits
		memset(pndesc, 0,32);
		pndesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | (TX_OWN));


#if defined (HW_ANT_SWITCH) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
		pndesc->Dword2 &= set_desc(~ BIT(24));
		pndesc->Dword2 &= set_desc(~ BIT(25));
		if(!(priv->pshare->rf_ft_var.CurAntenna & 0x80) && (txcfg->pstat)) {
			pndesc->Dword2 |= set_desc(((txcfg->pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)<<24);
			pndesc->Dword2 |= set_desc(((txcfg->pstat->CurAntenna^priv->pshare->rf_ft_var.CurAntenna)&1)<<25);
		}
#endif


		if (txcfg->privacy)
		{
			if (txcfg->privacy == _WAPI_SMS4_)
			{
				if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
					pndesc->Dword0 |= set_desc(TX_LastSeg);
#ifndef TXDESC_INFO	
				pndescinfo->pstat = txcfg->pstat;
				pndescinfo->rate = txcfg->tx_rate;
#endif				
			}


#ifdef CONFIG_IEEE80211W
			else if (!UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF))
#else
			else if (!UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
#endif		
			{
				if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST) {
#ifdef TX_SCATTER
					if (!skb || (skb && ((skb->list_idx+1) == skb->list_num))) 
#endif
						pndesc->Dword0 |= set_desc(TX_LastSeg);
				}
#ifndef TXDESC_INFO					
				pndescinfo->pstat = txcfg->pstat;
				pndescinfo->rate = txcfg->tx_rate;
#endif				
			}
		}
		else
		{
			if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST) {
#ifdef TX_SCATTER
				if (!skb || (skb && ((skb->list_idx+1) == skb->list_num)))
#endif
					pndesc->Dword0 |= set_desc(TX_LastSeg);
			}
#ifndef TXDESC_INFO				
			pndescinfo->pstat = txcfg->pstat;
			pndescinfo->rate = txcfg->tx_rate;
#endif			
		}
#ifdef TX_SCATTER
		if (skb != NULL)
			fr_len = actual_size;
#endif		

#ifdef CONFIG_RTL_WAPI_SUPPORT
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
#ifdef CONFIG_IEEE80211W
		if ((txcfg->privacy == _WAPI_SMS4_)&&(UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF)))
#else
		if ((txcfg->privacy == _WAPI_SMS4_)&&(UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE))))
#endif	// CONFIG_IEEE80211W
#else
		if (txcfg->privacy == _WAPI_SMS4_)
#endif
		{
			pndesc->Dword7 |= set_desc( (fr_len+SMS4_MIC_LEN) & TX_TxBufSizeMask);
		}
		else
#endif
		pndesc->Dword7 |= set_desc(fr_len & TX_TxBufSizeMask);

#ifdef TX_SCATTER
		if (skb && (skb->list_num > 0)) {
			if (get_desc(pndesc->Dword0) & TX_LastSeg)
				pndescinfo->type = txcfg->fr_type;
			else
				pndescinfo->type = _RESERVED_FRAME_TYPE_;
		} else
#endif
		{
			if (i == 0)
				pndescinfo->type = txcfg->fr_type;
			else
				pndescinfo->type = _RESERVED_FRAME_TYPE_;
		}

#if defined(CONFIG_RTK_MESH) && defined(MESH_USE_METRICOP)
		if( (txcfg->fr_type == _PRE_ALLOCMEM_) && (txcfg->is_11s & 128)) // for 11s link measurement frame
			pndescinfo->type =_RESERVED_FRAME_TYPE_;
#endif

		pndesc->Dword8 = set_desc(tmpphyaddr); //TxBufferAddr
#ifndef TXDESC_INFO
		pndescinfo->paddr = get_desc(pndesc->Dword8);
#endif		
		pndescinfo->pframe = txcfg->pframe;
#ifndef TXDESC_INFO		
		pndescinfo->len = txcfg->fr_len;	// for pci_unmap_single
#endif		
		pndescinfo->priv = priv;

		pbuf += fr_len;
		tmpphyaddr += fr_len;

#ifdef TX_SHORTCUT
		if (fit_shortcut) {
#ifdef TX_SCATTER
			if (txcfg->pstat->tx_sc_ent[idx].has_desc3) {
				desc_copy(&txcfg->pstat->tx_sc_ent[idx].hwdesc3, pndesc);
				descinfo_copy(&txcfg->pstat->tx_sc_ent[idx].swdesc3, pndescinfo);
			} else
#endif
			{
				desc_copy(&txcfg->pstat->tx_sc_ent[idx].hwdesc2, pndesc);
				descinfo_copy(&txcfg->pstat->tx_sc_ent[idx].swdesc2, pndescinfo);
#ifdef TXSC_CACHE_FRLEN				
				txcfg->pstat->tx_sc_ent[idx].frlen = fr_len;
#endif				
			}
		}
#endif

#ifndef USE_RTL8186_SDK
		rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

		flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(pndesc->Dword8)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET);
		flush_len[flush_num++] = get_desc(pndesc->Dword7) & TX_TxBufSizeMask;

#ifdef OSK_LOW_TX_DESC
		if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
			txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
		else
#endif
		txdesc_rollover(pndesc, (unsigned int *)tx_head);

		// retrieve H/W MIC and put in payload
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (txcfg->privacy == _WAPI_SMS4_)
		{
			SecSWSMS4Encryption(priv, txcfg);
#if 0
			if (txcfg->mic>0)
			{
				pndesc->Dword7 &= set_desc(~TX_TxBufSizeMask);
				pndesc->Dword7 |= set_desc((fr_len+txcfg->mic)& TX_TxBufSizeMask);
				flush_len[flush_num-1]= (get_desc(pndesc->Dword7) & TX_TxBufSizeMask);
			}
			else
			{
				txcfg->mic = SMS4_MIC_LEN;
			}
#endif
		
		}
#endif

#if !defined(NOT_RTK_BSP) && !defined(SUPPORT_TKIP_OSK_GDMA)
		if ((txcfg->privacy == _TKIP_PRIVACY_) &&
			(priv->pshare->have_hw_mic) &&
			!(priv->pmib->dot11StationConfigEntry.swTkipMic) &&
			(i == (txcfg->frg_num-1)) )	// last segment
		{
			register unsigned long int l,r;
			unsigned char *mic;
			volatile int i;

			while ((*(volatile unsigned int *)GDMAISR & GDMA_COMPIP) == 0)
				for (i=0; i<10; i++)
					;

			l = *(volatile unsigned int *)GDMAICVL;
			r = *(volatile unsigned int *)GDMAICVR;

			mic = ((struct sk_buff *)txcfg->pframe)->data + txcfg->fr_len - 8;
			mic[0] = (unsigned char)(l & 0xff);
			mic[1] = (unsigned char)((l >> 8) & 0xff);
			mic[2] = (unsigned char)((l >> 16) & 0xff);
			mic[3] = (unsigned char)((l >> 24) & 0xff);
			mic[4] = (unsigned char)(r & 0xff);
			mic[5] = (unsigned char)((r >> 8) & 0xff);
			mic[6] = (unsigned char)((r >> 16) & 0xff);
			mic[7] = (unsigned char)((r >> 24) & 0xff);

#ifdef MICERR_TEST
			if (priv->micerr_flag) {
				mic[7] ^= mic[7];
				priv->micerr_flag = 0;
			}
#endif
		}
#endif // !NOT_RTK_BSP && !SUPPORT_TKIP_OSK_GDMA

		/*------------------------------------------------------------*/
		/*                insert sw encrypt here!                     */
		/*------------------------------------------------------------*/
		if (txcfg->privacy &&
#ifdef CONFIG_IEEE80211W
			UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF))
#else
			UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
#endif
		{
			if (txcfg->privacy == _TKIP_PRIVACY_ ||
				txcfg->privacy == _WEP_40_PRIVACY_ ||
				txcfg->privacy == _WEP_104_PRIVACY_)
			{
				picvdesc     = phdesc + *tx_head;
				picvdescinfo = pswdescinfo + *tx_head;
				//clear all bits
				memset(picvdesc, 0,32);

				if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST){
					picvdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN);
				}
				else{
					picvdesc->Dword0   = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN | TX_LastSeg);
				}

				picvdesc->Dword7 |= (set_desc(txcfg->icv & TX_TxBufSizeMask)); //TxBufferSize

				// append ICV first...
				picv = get_icv_from_poll(priv);
				if (picv == NULL)
				{
					DEBUG_ERR("System-Buf! can't alloc picv\n");
					BUG();
				}

				picvdescinfo->type = _PRE_ALLOCICVHDR_;
				picvdescinfo->pframe = picv;
#ifndef TXDESC_INFO					
				picvdescinfo->pstat = txcfg->pstat;
				picvdescinfo->rate = txcfg->tx_rate;
#endif				
				picvdescinfo->priv = priv;
				//TxBufferAddr
				picvdesc->Dword8 = set_desc(get_physical_addr(priv, picv, txcfg->icv, PCI_DMA_TODEVICE));

				if (i == 0)
					tkip_icv(picv,
						pwlhdr + txcfg->hdr_len + txcfg->iv, txcfg->llc,
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask));
				else
					tkip_icv(picv,
						NULL, 0,
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask));

				if ((i == 0) && (txcfg->llc != 0)) {
					if (txcfg->privacy == _TKIP_PRIVACY_)
						tkip_encrypt(priv, pwlhdr, txcfg->hdr_len,
							pwlhdr + txcfg->hdr_len + 8, sizeof(struct llc_snap),
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv);
					else
						wep_encrypt(priv, pwlhdr, txcfg->hdr_len,
							pwlhdr + txcfg->hdr_len + 4, sizeof(struct llc_snap),
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv,
							txcfg->privacy);
				}
				else { // not first segment or no snap header
					if (txcfg->privacy == _TKIP_PRIVACY_)
						tkip_encrypt(priv, pwlhdr, txcfg->hdr_len, NULL, 0,
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv);
					else
						wep_encrypt(priv, pwlhdr, txcfg->hdr_len, NULL, 0,
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), picv, txcfg->icv,
							txcfg->privacy);
				}
#ifndef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

				flush_addr[flush_num]  = (unsigned long)bus_to_virt(get_desc(picvdesc->Dword8)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET);
				flush_len[flush_num++]=(get_desc(picvdesc->Dword7) & TX_TxBufSizeMask);
#ifdef OSK_LOW_TX_DESC
				if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
					txdesc_rollover_nonbe(picvdesc, (unsigned int *)tx_head);
				else
#endif
				txdesc_rollover(picvdesc, (unsigned int *)tx_head);
			}

			else if (txcfg->privacy == _CCMP_PRIVACY_)
			{
				pmicdesc = phdesc + *tx_head;
				pmicdescinfo = pswdescinfo + *tx_head;

				//clear all bits
				memset(pmicdesc, 0,32);
				
				if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
					pmicdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN);
				else
				  pmicdesc->Dword0   = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN | TX_LastSeg);

				// set TxBufferSize
				pmicdesc->Dword7 = set_desc(txcfg->mic & TX_TxBufSizeMask);

				// append MIC first...
				pmic = get_mic_from_poll(priv);
				if (pmic == NULL)
				{
					DEBUG_ERR("System-Buf! can't alloc pmic\n");
					BUG();
				}

				pmicdescinfo->type = _PRE_ALLOCMICHDR_;
				pmicdescinfo->pframe = pmic;
#ifndef TXDESC_INFO					
				pmicdescinfo->pstat = txcfg->pstat;
				pmicdescinfo->rate = txcfg->tx_rate;
#endif				
				pmicdescinfo->priv = priv;
				// set TxBufferAddr
				pmicdesc->Dword8= set_desc(get_physical_addr(priv, pmic, txcfg->mic, PCI_DMA_TODEVICE));

				// then encrypt all (including ICV) by AES
				if ((i == 0)&&(txcfg->llc != 0)) // encrypt 3 segments ==> llc, mpdu, and mic
				{
#ifdef CONFIG_IEEE80211W
					if(isBIP) 
					{
						BIP_encrypt(priv, pwlhdr, pwlhdr + txcfg->hdr_len + 8,
									pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					}
					else {
						aesccmp_encrypt(priv, pwlhdr, pwlhdr + txcfg->hdr_len + 8,
										pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					}
#else
					aesccmp_encrypt(priv, pwlhdr, pwlhdr + txcfg->hdr_len + 8,
									pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic);
#endif	
				}
				else { // encrypt 2 segments ==> mpdu and mic
#ifdef CONFIG_IEEE80211W
					if(isBIP) 
					{
						BIP_encrypt(priv, pwlhdr, NULL,
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					}
					else {
						aesccmp_encrypt(priv, pwlhdr, NULL,
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic,txcfg->isPMF);
					}
#else // !defined(CONFIG_IEEE80211W)
					aesccmp_encrypt(priv, pwlhdr, NULL,
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufSizeMask), pmic);
#endif // CONFIG_IEEE80211W
				}
#ifndef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
				flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(pmicdesc->Dword8)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET);
				flush_len[flush_num++]= (get_desc(pmicdesc->Dword7) & TX_TxBufSizeMask);
#ifdef OSK_LOW_TX_DESC
				if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
					txdesc_rollover_nonbe(pmicdesc, (unsigned int *)tx_head);
				else
#endif
				txdesc_rollover(pmicdesc, (unsigned int *)tx_head);
			}
		}

#ifdef RESERVE_TXDESC_FOR_EACH_IF
		if (RSVQ_ENABLE && IS_RSVQ(q_num)) {
			if ( txcfg->privacy
#if defined(CONFIG_RTL_WAPI_SUPPORT)
				&& (_WAPI_SMS4_ != txcfg->privacy)
#endif		
#ifdef CONFIG_IEEE80211W
				&& UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF) ){
#else
				&& UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)) ){
#endif						
				priv->use_txdesc_cnt[RSVQ(q_num)] += 3;
			}
			else
				priv->use_txdesc_cnt[RSVQ(q_num)] += 2;
		}
#endif

#ifdef TX_SCATTER
		if (skb && ++skb->list_idx < skb->list_num) {
			skb_assign_buf(skb, skb->list_buf[skb->list_idx].buf, skb->list_buf[skb->list_idx].len);
			skb->len = skb->list_buf[skb->list_idx].len;
			pbuf = skb->data;
			actual_size = skb->len;
			tmpphyaddr = get_physical_addr(priv, pbuf, actual_size, PCI_DMA_TODEVICE);
#ifdef TX_SHORTCUT
			if (txcfg->pstat) {
				if (txcfg->pstat->tx_sc_ent[idx].has_desc3) {
					fit_shortcut = 0;
					txcfg->pstat->tx_sc_ent[idx].has_desc3 = 0;
					for (idx=0; idx<TX_SC_ENTRY_NUM; idx++)
						txcfg->pstat->tx_sc_ent[idx].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
				} else {
					txcfg->pstat->tx_sc_ent[idx].has_desc3 = 1;
				}
			}
#endif
			goto fill_body;
		}
#endif
	}


init_deschead:
#if 0
	switch (q_select) {
	case 0:
	case 3:
	   if (q_num != BE_QUEUE)
    		printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
	   break;
	case 1:
	case 2:
		if (q_num != BK_QUEUE)
		    printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
	   break;
	case 4:
	case 5:
		if (q_num != VI_QUEUE)
		    printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	case 6:
	case 7:
		if (q_num != VO_QUEUE)
			printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	case 0x11 :
		 if (q_num != HIGH_QUEUE)
			printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	case 0x12 :
		if (q_num != MGNT_QUEUE)
			printk("%s %d error : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
		break;
	default :
		printk("%s %d warning : q_select[%d], q_num[%d]\n", __FUNCTION__, __LINE__, q_select, q_num);
	break;
	}
#endif

	for (i=0; i<flush_num; i++)
		rtl_cache_sync_wback(priv, flush_addr[i], flush_len[i], PCI_DMA_TODEVICE);

	if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
		priv->amsdu_first_desc = pfrstdesc;
#ifndef USE_RTL8186_SDK
		priv->amsdu_first_dma_desc = pfrst_dma_desc;
#endif
		priv->amsdu_len = get_desc(pfrstdesc->Dword0) & 0xffff; // get pktSize
		return;
	}

	pfrstdesc->Dword0 |= set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(pfrst_dma_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

	if (q_num == HIGH_QUEUE) {
//		priv->pshare->pkt_in_hiQ = 1;
		priv->pkt_in_hiQ = 1;

		return;
	} else {
		tx_poll(priv, q_num);
	}

	return;
}
#else
void rtl8192cd_signin_txdesc(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	return;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST
#endif // CONFIG_PCI_HCI


#ifdef SUPPORT_TX_AMSDU
#ifdef CONFIG_WLAN_HAL
int rtl88XX_signin_txdesc_amsdu(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
    struct tx_desc *phdesc, *pdesc, *pfrstdesc;
    struct tx_desc_info *pswdescinfo, *pdescinfo;
    unsigned int  tx_len;
	u2Byte  *tx_head;
    u4Byte  q_num;
    unsigned long   tmpphyaddr;
    unsigned char *pbuf;
    struct rtl8192cd_hw *phw;
    unsigned long *dma_txhead;

#ifdef CONFIG_WLAN_HAL
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    u4Byte                          halQNum;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_BUFFER_DESCRIPTOR           cur_txbd;
    TX_DESC_DATA_88XX               desc_data;
#endif

    q_num = txcfg->q_num;
    phw = GET_HW(priv);

#ifdef CONFIG_WLAN_HAL
    halQNum     = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, (u4Byte)q_num);
    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[halQNum]);
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;
    memset(&desc_data, 0, sizeof(TX_DESC_DATA_88XX));
    
    tx_head     = &(ptx_dma->tx_queue[halQNum].host_idx);
    pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);
#else
    dma_txhead  = get_txdma_addr(phw, q_num);
    tx_head     = get_txhead_addr(phw, q_num);
    phdesc      = get_txdesc(phw, q_num);
    pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);
#endif
       
    pbuf = ((struct sk_buff *)txcfg->pframe)->data;    
    tx_len = ((struct sk_buff *)txcfg->pframe)->len;

#ifdef CONFIG_WLAN_HAL
    // Do nothing here...
#else
    tmpphyaddr = get_physical_addr(priv, pbuf, tx_len, PCI_DMA_TODEVICE);
#ifdef USE_RTL8186_SDK
    rtl_cache_sync_wback(priv, (unsigned long)pbuf, tx_len, PCI_DMA_TODEVICE);
#endif
#endif // CONFIG_WLAN_HAL

    pdesc     = phdesc + (*tx_head);
    pdescinfo = pswdescinfo + *tx_head;

#ifdef CONFIG_WLAN_HAL
        // Do nothing here...
#else
    //clear all bits
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT) 
    if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
    {
        memset(pdesc, 0, 40);                   //clear all bits
        pdesc->Dword10 = set_desc(tmpphyaddr); // TXBufferAddr
        pdesc->Dword7 |= (set_desc(tx_len & TX_TxBufSizeMask));
        pdesc->Dword0 |= set_desc(40 << TX_OffsetSHIFT); // tx_desc size
    }
    else
#endif
    {
        memset(pdesc, 0, 32);
        pdesc->Dword8 = set_desc(tmpphyaddr); // TXBufferAddr
        pdesc->Dword7 |= (set_desc(tx_len & TX_TxBufSizeMask));
        pdesc->Dword0 |= set_desc(32 << TX_OffsetSHIFT); // tx_desc size
    }

    if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST){
        pdesc->Dword0 = set_desc(TX_OWN | TX_LastSeg);
    }
    else
        pdesc->Dword0 = set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
    rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
#endif // CONFIG_WLAN_HAL

    //pdescinfo->type = _SKB_FRAME_TYPE_;

#ifdef CONFIG_WLAN_HAL
    // Do nothing here...
#else
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
    if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
   		pdescinfo->paddr = get_desc(pdesc->Dword10); // TXBufferAddr
    else
#endif
    pdescinfo->paddr = get_desc(pdesc->Dword8); // TXBufferAddr
#endif // CONFIG_WLAN_HAL

#ifdef CONFIG_WLAN_HAL
#ifdef WLAN_HAL_TX_AMSDU
    pdescinfo->buf_type_amsdu[priv->amsdu_num] = _SKB_FRAME_TYPE_;
    pdescinfo->buf_pframe_amsdu[priv->amsdu_num] = txcfg->pframe;
#endif
#else
    pdescinfo->pframe = txcfg->pframe;
#endif
    pdescinfo->len = txcfg->fr_len;
    pdescinfo->priv = priv;

#ifdef CONFIG_WLAN_HAL
    // Do nothing here...
#else
#ifdef OSK_LOW_TX_DESC
	if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
		txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
	else
#endif
    txdesc_rollover(pdesc, (unsigned int *)tx_head);
#endif // CONFIG_WLAN_HAL

    priv->amsdu_len += tx_len;

#ifdef WLAN_HAL_TX_AMSDU
    priv->amsdu_num++;
    desc_data.aggreEn   = txcfg->aggre_en;
#endif
    desc_data.pBuf      = pbuf;
    desc_data.frLen     = tx_len;

#ifdef WLAN_HAL_TX_AMSDU
    if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {
        desc_data.amsduLen  = priv->amsdu_len;
        //printk("%s(%d): num:%d\n", __FUNCTION__, __LINE__, priv->amsdu_num);
    }
#endif

    GET_HAL_INTERFACE(priv)->FillTxHwCtrlHandler(priv, halQNum, (void *)&desc_data);

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	if (RSVQ_ENABLE && IS_RSVQ(q_num))
		priv->use_txdesc_cnt[RSVQ(q_num)] ++;
#endif

    if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {

#ifdef CONFIG_WLAN_HAL
        GET_HAL_INTERFACE(priv)->SyncSWTXBDHostIdxToHWHandler(priv, halQNum);
#else
        pfrstdesc = priv->amsdu_first_desc;
        pfrstdesc->Dword0 = set_desc((get_desc(pfrstdesc->Dword0) &0xff0000) | priv->amsdu_len | TX_FirstSeg | TX_OWN);

#ifndef USE_RTL8186_SDK
        rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(priv->amsdu_first_dma_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

        tx_poll(priv, q_num);
#endif // CONFIG_WLAN_HAL
    }

    return 0;
}
#endif // CONFIG_WLAN_HAL
int rtl8192cd_signin_txdesc_amsdu(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	struct tx_desc *phdesc, *pdesc, *pfrstdesc;
	struct tx_desc_info *pswdescinfo, *pdescinfo;
	unsigned int  tx_len;
	int *tx_head, q_num;
	unsigned long	tmpphyaddr;
	unsigned char *pbuf;
	struct rtl8192cd_hw *phw;
	unsigned long *dma_txhead;

	q_num = txcfg->q_num;
	phw	= GET_HW(priv);

	dma_txhead	= get_txdma_addr(phw, q_num);
	tx_head		= get_txhead_addr(phw, q_num);
	phdesc   	= get_txdesc(phw, q_num);
	pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);

	pbuf = ((struct sk_buff *)txcfg->pframe)->data;
	tx_len = ((struct sk_buff *)txcfg->pframe)->len;
	tmpphyaddr = get_physical_addr(priv, pbuf, tx_len, PCI_DMA_TODEVICE);
#ifdef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)pbuf, tx_len, PCI_DMA_TODEVICE);
#endif

	pdesc     = phdesc + (*tx_head);
	pdescinfo = pswdescinfo + *tx_head;

	//clear all bits
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)
	{
		memset(pdesc, 0, 40);					//clear all bits
		pdesc->Dword10 = set_desc(tmpphyaddr); // TXBufferAddr
		pdesc->Dword7 |= (set_desc(tx_len & TX_TxBufSizeMask));
		pdesc->Dword0 |= set_desc(40 << TX_OffsetSHIFT); // tx_desc size
	}
	else
#endif
	{
		memset(pdesc, 0, 32);
		pdesc->Dword8 = set_desc(tmpphyaddr); // TXBufferAddr
		pdesc->Dword7 |= (set_desc(tx_len & TX_TxBufSizeMask));
		pdesc->Dword0 |= set_desc(32 << TX_OffsetSHIFT); // tx_desc size
	}

	if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST){
		pdesc->Dword0 = set_desc(TX_OWN | TX_LastSeg);
	}
	else
		pdesc->Dword0 = set_desc(TX_OWN);

#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

	pdescinfo->type = _SKB_FRAME_TYPE_;

//#ifdef CONFIG_RTL_8812_SUPPORT
//	if(GET_CHIP_VER(priv)== VERSION_8812E)
//	pdescinfo->paddr = get_desc(pdesc->Dword10); // TXBufferAddr
//	else
//#endif
//	pdescinfo->paddr = get_desc(pdesc->Dword8); // TXBufferAddr

	pdescinfo->pframe = txcfg->pframe;
//	pdescinfo->len = txcfg->fr_len;
	pdescinfo->priv = priv;

	txdesc_rollover(pdesc, (unsigned int *)tx_head);

	priv->amsdu_len += tx_len;

	if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {
		pfrstdesc = priv->amsdu_first_desc;
		pfrstdesc->Dword0 = set_desc((get_desc(pfrstdesc->Dword0) &0xff0000) | priv->amsdu_len | TX_FirstSeg | TX_OWN);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
		if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv)== VERSION_8723B)
			pfrstdesc->Dword2 |= set_desc(TXdesc_92E_BK);
#endif

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	if (RSVQ_ENABLE && IS_RSVQ(q_num))
		priv->use_txdesc_cnt[RSVQ(q_num)] ++;
#endif

#ifndef USE_RTL8186_SDK
		rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(priv->amsdu_first_dma_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
		tx_poll(priv, q_num);
	}

	return 0;
}
#endif // SUPPORT_TX_AMSDU


#if 0

#ifdef CONFIG_PCI_HCI
//-----------------------------------------------------------------------------
// Procedure:    Fill Tx Command Packet Descriptor
//
// Description:   This routine fill command packet descriptor. We assum that command packet
//				require only one descriptor.
//
// Arguments:   This function is only for Firmware downloading in current stage
//
// Returns:
//-----------------------------------------------------------------------------
int rtl8192cd_SetupOneCmdPacket(struct rtl8192cd_priv *priv, unsigned char *dat_content, unsigned short txLength, unsigned char LastPkt)
/*
	IN	PADAPTER		Adapter,
	IN	PRT_TCB			pTcb,
	IN	u1Byte			QueueIndex,
	IN	u2Byte			index,
	IN	BOOLEAN			bFirstSeg,
	IN	BOOLEAN			bLastSeg,
	IN	pu1Byte			VirtualAddress,
	IN	u4Byte			PhyAddressLow,
	IN	u4Byte			BufferLen,
	IN	BOOLEAN     		bSetOwnBit,
	IN	BOOLEAN			bLastInitPacket,
	IN    u4Byte			DescPacketType,
	IN	u4Byte			PktLen
	)
*/
{

	unsigned char	ih=0;
	unsigned char	DescNum;
	unsigned short	DebugTimerCount;

	struct tx_desc	*pdesc;
	struct tx_desc	*phdesc;
	volatile unsigned int *ppdesc  ; //= (unsigned int *)pdesc;
	int	*tx_head, *tx_tail;
	struct rtl8192cd_hw	*phw = GET_HW(priv);

	tx_head	= (int *)&phw->txcmdhead;
	tx_tail = (int *)&phw->txcmdtail;
	phdesc = phw->txcmd_desc;

	DebugTimerCount = 0; // initialize debug counter to exit loop
	DescNum = 1;

//TODO: Fill the dma check here

//	printk("data lens: %d\n", txLength );

	for (ih=0; ih<DescNum; ih++) {
		pdesc      = (phdesc + (*tx_head));
		ppdesc = (unsigned int *)pdesc;
		// Clear all status
		memset(pdesc, 0, 36);
//		rtl_cache_sync_wback(priv, phw->txcmd_desc_dma_addr[*tx_head], 32, PCI_DMA_TODEVICE);
		// For firmware downlaod we only need to set LINIP
		if (LastPkt)
			pdesc->Dword0 |= set_desc(TX_LINIP);

		// From Scott --> 92SE must set as 1 for firmware download HW DMA error
		pdesc->Dword0 |= set_desc(TX_FirstSeg);;//bFirstSeg;
		pdesc->Dword0 |= set_desc(TX_LastSeg);;//bLastSeg;

		// 92SE need not to set TX packet size when firmware download
		pdesc->Dword7 |=  (set_desc((unsigned short)(txLength) << TX_TxBufSizeSHIFT));

		memcpy(priv->pshare->txcmd_buf, dat_content, txLength);

		rtl_cache_sync_wback(priv, (unsigned long)priv->pshare->txcmd_buf, txLength, PCI_DMA_TODEVICE);

#ifdef CONFIG_RTL_8812_SUPPORT
		if(GET_CHIP_VER(priv)== VERSION_8812E)
			pdesc->Dword10 =  set_desc(priv->pshare->cmdbuf_phyaddr);
		else
#endif
		pdesc->Dword8 =  set_desc(priv->pshare->cmdbuf_phyaddr);


//		pdesc->Dword0	|= set_desc((unsigned short)(txLength) << TX_PktIDSHIFT);
		pdesc->Dword0	|= set_desc((unsigned short)(txLength) << TX_PktSizeSHIFT);
		//if (bSetOwnBit)
		{
			pdesc->Dword0 |= set_desc(TX_OWN);
//			*(ppdesc) |= set_desc(BIT(31));
		}

#ifndef USE_RTL8186_SDK
		rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(phw->txcmd_desc_dma_addr[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
		*tx_head = (*tx_head + 1) & (NUM_CMD_DESC - 1);
	}

	return TRUE;
}
#endif // CONFIG_PCI_HCI
#endif

#ifdef CONFIG_RTK_MESH
#ifndef __LINUX_2_6__
__MIPS16
#endif
int reuse_meshhdr(struct rtl8192cd_priv *priv, struct tx_insn *txcfg)
{
	const short meshhdrlen= (txcfg->mesh_header.mesh_flag & 0x01) ? 16 : 4;
	struct sk_buff *pskb = (struct sk_buff *)txcfg->pframe;
	if (skb_cloned(pskb))
	{
		struct sk_buff	*newskb = skb_copy(pskb, GFP_ATOMIC);
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		if (newskb == NULL) {
			priv->ext_stats.tx_drops++;
			release_wlanllchdr_to_poll(priv, txcfg->phdr);
			DEBUG_ERR("TX DROP: Can't copy the skb!\n");
			return 0;
		}
		txcfg->pframe = pskb = newskb;
#ifdef ENABLE_RTL_SKB_STATS
		rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif
	}
	memcpy(skb_push(pskb,meshhdrlen), &(txcfg->mesh_header), meshhdrlen);
	txcfg->fr_len += meshhdrlen;
	return 1;
}
#endif //CONFIG_RTK_MESH


#if defined(WIFI_WMM) && defined(DRVMAC_LB)
void SendLbQosNullData(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned char *hwaddr;
	unsigned char tempQosControl[2];
	DECLARE_TXINSN(txinsn);


	pmib = GET_MIB(priv);
	hwaddr = pmib->dot11OperationEntry.hwaddr;

	if(!pmib->miscEntry.drvmac_lb) {
		printk("LB mode disabled, cannot SendLbQosNullData!!!\n");
		return;
	}

	if(!memcmp(pmib->miscEntry.lb_da, "\x0\x0\x0\x0\x0\x0", 6)) {
		printk("LB addr is NULL, cannot SendLbQosNullData!!!\n");
		return;
	}

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
	txinsn.q_num = BE_QUEUE;
//	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE	
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
	txinsn.fr_type = _PRE_ALLOCHDR_;
	txinsn.phdr = get_wlanhdr_from_poll(priv);
	txinsn.pframe = NULL;

	if (txinsn.phdr == NULL)
		goto send_qos_null_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	SetFrameSubType(txinsn.phdr, BIT(7) | WIFI_DATA_NULL);
	SetFrDs(txinsn.phdr);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pmib->miscEntry.lb_da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	txinsn.hdr_len = WLAN_HDR_A3_QOS_LEN;

	memset(tempQosControl, 0, 2);
//	tempQosControl[0] = 0x07;		//set priority to VO
//	tempQosControl[0] |= BIT(4);	//set EOSP
	memcpy((void *)GetQosControl((txinsn.phdr)), tempQosControl, 2);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

send_qos_null_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
}


void SendLbQosData(struct rtl8192cd_priv *priv)
{
	struct wifi_mib *pmib;
	unsigned char *hwaddr;
	unsigned char tempQosControl[2];
	unsigned char	*pbuf;
	static unsigned int pkt_length = 1;
	DECLARE_TXINSN(txinsn);

	pmib = GET_MIB(priv);
	hwaddr = pmib->dot11OperationEntry.hwaddr;

	if(!pmib->miscEntry.drvmac_lb) {
		printk("LB mode disabled, cannot SendLbQosData!!!\n");
		return;
	}

	if(!memcmp(pmib->miscEntry.lb_da, "\x0\x0\x0\x0\x0\x0", 6)) {
		printk("LB addr is NULL, cannot SendLbQosData!!!\n");
		return;
	}

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
	txinsn.q_num = BE_QUEUE;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
	txinsn.fixed_rate = 1;
//	txinsn.pframe = NULL;

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto send_qos_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto send_qos_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));
	memset((void *)pbuf, 0, PRE_ALLOCATED_BUFSIZE);

	SetFrameSubType(txinsn.phdr, WIFI_QOS_DATA);
	SetFrDs(txinsn.phdr);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pmib->miscEntry.lb_da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	txinsn.hdr_len = WLAN_HDR_A3_QOS_LEN;

	memset(tempQosControl, 0, 2);
	memcpy((void *)GetQosControl((txinsn.phdr)), tempQosControl, 2);

//	printk("--0x%02x%02x%02x%02x 0x%02x%02x%02x%02x--\n", *pbuf, *(pbuf+1), *(pbuf+2), *(pbuf+3), *(pbuf+4), *(pbuf+5), *(pbuf+6), *(pbuf+7));

	if (pmib->miscEntry.lb_mlmp == 1) {
		// all zero in payload
		memset((void *)pbuf, 0x00, pkt_length);
		pbuf += pkt_length;
		txinsn.fr_len += pkt_length;
	}
	else if (pmib->miscEntry.lb_mlmp == 2) {
		// all 0xff in payload
		memset((void *)pbuf, 0xff, pkt_length);
//		printk("~~0x%02x%02x%02x%02x 0x%02x%02x%02x%02x~~\n", *pbuf, *(pbuf+1), *(pbuf+2), *(pbuf+3), *(pbuf+4), *(pbuf+5), *(pbuf+6), *(pbuf+7));
		pbuf += pkt_length;
		txinsn.fr_len += pkt_length;
	}
	else if ((pmib->miscEntry.lb_mlmp == 3) || (pmib->miscEntry.lb_mlmp == 4)) {
		// all different value in payload, 0x00~0xff
		unsigned int i = 0;
//		printk("~~");
		for (i = 0; i <pkt_length; i++) {
			memset((void *)pbuf, i%0x100, 1);
//			printk("%02x", *pbuf);
			pbuf += 1;
			txinsn.fr_len += 1;
		}
//		printk("~~\n");
	}
	else {
		printk("wrong LB muli-length-multi-packet setting!!\n");
		goto send_qos_fail;
	}

//	if (pkt_length++ >= 600)
	if (pkt_length++ >= 2048)
		pkt_length = 1;

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return;

send_qos_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}
#endif

#ifdef TX_EARLY_MODE
__MIPS16
__IRAM_IN_865X
static void get_tx_early_info(struct rtl8192cd_priv *priv, struct stat_info  *pstat,  struct sk_buff_head *pqueue)
{	
	struct sk_buff *next_skb;

	pstat->empkt_num = 0;
	memset(pstat->empkt_len, '\0', sizeof(pstat->empkt_len));
	
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8188E) {
		int tmplen = 0, extra_len = 0;
		
		if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _CCMP_PRIVACY_) {
			extra_len = 16;
		} else if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _TKIP_PRIVACY_) {
			extra_len = 20;
		} else if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_ ||
			priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_) {
			extra_len = 8;
		}
		
		skb_queue_walk(pqueue, next_skb) {
			if ((pstat->empkt_num %2) == 0) {
				pstat->empkt_len[pstat->empkt_num/2] = next_skb->len+WLAN_HDR_A3_LEN+WLAN_CRC_LEN+extra_len;
			} else {
				tmplen = pstat->empkt_len[(pstat->empkt_num-1)/2];
				tmplen += ((tmplen%4)?(4-tmplen%4):0)+4;
				tmplen += next_skb->len+WLAN_HDR_A3_LEN+WLAN_CRC_LEN+extra_len;	
				pstat->empkt_len[(pstat->empkt_num-1)/2] = tmplen;
			}
		
			pstat->empkt_num++;
			if (skb_queue_is_last(pqueue, next_skb))
				break;

			if (pstat->empkt_num >= 10)
				break;
		}	
	} else 
#endif	
	skb_queue_walk(pqueue, next_skb) {
		pstat->empkt_len[pstat->empkt_num++] = next_skb->len;

		if (skb_queue_is_last(pqueue, next_skb))
			break;

		if (pstat->empkt_num >= 5)
			break;
	}	
}
#endif


#if defined(SW_TX_QUEUE) || defined(RTK_ATM)
__inline__ static int rtl8192cd_swq_bdfull(struct rtl8192cd_priv *priv, unsigned int num,  unsigned char qnum)
{
    int *tx_head=NULL, *tx_tail=NULL;

#ifdef CONFIG_WLAN_HAL	 	 
    if(IS_HAL_CHIP(priv))    {
        if(compareAvailableTXBD(priv, num+4, qnum, 2))
            return 1;
    } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
    {
        tx_head = get_txhead_addr(GET_HW(priv), qnum);
        tx_tail = get_txtail_addr(GET_HW(priv), qnum);
        if((num+4) > CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC))
            return 1;
    }

    return 0;
}
#endif


#ifdef SW_TX_QUEUE
/* when using hw timer,  the parameter "add_timer_timeout" is TSF time
     when using sw timer, the parameter "add_timer_timeout" is jiffies time
*/
__inline__ static unsigned char rtl8192cd_swq_settimer(struct rtl8192cd_priv *priv, UINT32 timer_timeout)
{
    UINT32 set_timeout;    
    UINT32 current_time;
    unsigned char set_timer = 0;

    if(timer_timeout == 0) {
        if(priv->pshare->swq_current_timeout) {
            //priv->pshare->swq_current_timeout = 0;
            if(priv->pshare->swq_use_hw_timer) {  
				/*
                current_time = RTL_R32(TSFTR1);  
                if(current_time - priv->pshare->swq_last_tsf < SWQ_HWTIMER_DELAY) {
                    //delay_us(SWQ_HWTIMER_DELAY - (current_time - priv->pshare->swq_last_tsf));
                    return;
                }
                RTL_W32(TC4_CTRL, 0);
                priv->pshare->swq_last_tsf = RTL_R32(TSFTR1);
                */
				;
           	}
            else {      
                if (timer_pending(&priv->pshare->swq_sw_timer))
			        del_timer_sync(&priv->pshare->swq_sw_timer);
            }            
        }
    }
    else {
        if(priv->pshare->swq_use_hw_timer) {  
            current_time = RTL_R32(TSFTR1);   
            set_timeout = current_time + SWQ_HWTIMER_MINIMUN;
            if(timer_timeout - set_timeout > SWQ_HWTIMER_MAXIMUN) {                
                timer_timeout = set_timeout;
                set_timeout = SWQ_HWTIMER_MINIMUN;                
            }            
            else {
                set_timeout = timer_timeout - current_time;
            }

            if(priv->pshare->swq_current_timeout) {
                if(timer_timeout + SWQ_HWTIMER_TOLERANCE - priv->pshare->swq_current_timeout > SWQ_HWTIMER_MAXIMUN) {                
                    set_timer = 1;
                }
            }
            else {
                set_timer = 1;
            }

            if(set_timer) {
                if(current_time - priv->pshare->swq_last_tsf < SWQ_HWTIMER_DELAY) {
                    return 1;
                }
                priv->pshare->swq_current_timeout = (timer_timeout == 0?1:timer_timeout);            
                set_timeout = RTL_MICROSECONDS_TO_GTIMERCOUNTER(set_timeout);                
                set_timeout = BIT26 | BIT24 | (set_timeout & 0x00FFFFFF);  //TC40INT_EN | TC4EN | TC4Data
                RTL_W32(TC4_CTRL, set_timeout);
                priv->pshare->swq_last_tsf = RTL_R32(TSFTR1);
            }            
        }
        else {
            if(priv->pshare->swq_current_timeout) {                          
                if(timer_timeout < priv->pshare->swq_current_timeout) {                
                    set_timer = 1;
                }                
            }
            else {            
                set_timer = 1;
            }

            if(set_timer) {                                
                priv->pshare->swq_current_timeout = timer_timeout;            
                mod_timer(&priv->pshare->swq_sw_timer, (unsigned long)timer_timeout);            
            }            
        }
    }
    return 0;
}


/* when using hw timer,  the parameter "add_timer_timeout" is TSF time
     when using sw timer, the parameter "add_timer_timeout" is jiffies time
*/
__inline__ static int rtl8192cd_swq_addtimer(struct rtl8192cd_priv *priv, struct stat_info* pstat, unsigned char qnum, UINT32 add_timer_timeout)
{
    unsigned short timer_index = (priv->pshare->swq_timer_head + 1) % SWQ_TIMER_NUM; 
    if(priv->pshare->swq_timer_tail == timer_index) { /* queue is full */           
        /* find an empty slot */
        for(timer_index = 0 ; timer_index < SWQ_TIMER_NUM; timer_index++) {
            if(timer_index == priv->pshare->swq_timer_tail) {
                continue;
            }
            if(priv->pshare->swq_timer[timer_index].pstat == NULL) {
                 break;
            }
        }

        if(timer_index == SWQ_TIMER_NUM) {
            printk("%s: %s, swq timer overflow!\n", priv->dev->name, __FUNCTION__);
            return -1;
        }
    }

    priv->pshare->swq_timer_head = timer_index;                 
    priv->pshare->swq_timer[timer_index].priv = priv;
    priv->pshare->swq_timer[timer_index].pstat = pstat;
    priv->pshare->swq_timer[timer_index].qnum = qnum;
    priv->pshare->swq_timer[timer_index].timeout = add_timer_timeout;
    pstat->swq.swq_timer_id[qnum] = timer_index + 1;

    return 0;
}


void rtl8192cd_swq_deltimer(struct rtl8192cd_priv *priv, struct stat_info* pstat, unsigned char qnum)
{
    unsigned char set_timer = 0;
    unsigned short temp_tail;
    unsigned short timer_index = pstat->swq.swq_timer_id[qnum]-1;
    UINT32 set_timer_timeout = 0;
    priv->pshare->swq_timer[timer_index].pstat = NULL;
    pstat->swq.swq_timer_id[qnum] = 0; 

    /*dequeu dummy slot */    
     while(priv->pshare->swq_timer_head != priv->pshare->swq_timer_tail) /*check if queue is empty*/
    {
        temp_tail = (priv->pshare->swq_timer_tail + 1) % SWQ_TIMER_NUM;
        if(priv->pshare->swq_timer[temp_tail].pstat != NULL) {
            break;
        }            
        priv->pshare->swq_timer_tail = temp_tail;
    }
  
    if (priv->pshare->swq_timer_head != priv->pshare->swq_timer_tail) // if queue is not empty
    {
        temp_tail = priv->pshare->swq_timer_tail;
        do {            
            temp_tail = (temp_tail + 1) % SWQ_TIMER_NUM;
            if(priv->pshare->swq_timer[temp_tail].pstat != NULL) {
                if(priv->pshare->swq_use_hw_timer) { 
                    if(priv->pshare->swq_timer[temp_tail].timeout - (priv->pshare->swq_current_timeout + SWQ_HWTIMER_MINIMUN) > SWQ_HWTIMER_MAXIMUN) {
                        set_timer = 0;
                        break;
                    }
                    else {
                        if(set_timer == 0) {
                            set_timer_timeout = priv->pshare->swq_timer[temp_tail].timeout;
                        }
                        else {
                            if(priv->pshare->swq_timer[temp_tail].timeout - set_timer_timeout > SWQ_HWTIMER_MAXIMUN) {
                                set_timer_timeout = priv->pshare->swq_timer[temp_tail].timeout;
                            }                     
                        }
                        
                        set_timer = 1;
                    }
                }
                else {
                    if(priv->pshare->swq_timer[temp_tail].timeout <= priv->pshare->swq_current_timeout) {
                        set_timer = 0;
                        break;
                    }
                    else {
                        if(set_timer == 0) {
                            set_timer_timeout = priv->pshare->swq_timer[temp_tail].timeout;
                        }
                        else {
                            if(priv->pshare->swq_timer[temp_tail].timeout < set_timer_timeout) {
                                set_timer_timeout = priv->pshare->swq_timer[temp_tail].timeout;
                            }
                        }                        
                        set_timer = 1;
                    }

                }                
            }
        }while(temp_tail != priv->pshare->swq_timer_head);

        if(set_timer) {
            rtl8192cd_swq_settimer(priv, 0);
            rtl8192cd_swq_settimer(priv, set_timer_timeout);   /*need to re-setup timer*/              
        }
    }
    else {
        rtl8192cd_swq_settimer(priv, 0);   /*disable timer*/
    }

}


/*return: 1: not finished,  0: the queue is empty now*/
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
__inline__ static int rtl8192cd_swq_dequeue(struct rtl8192cd_priv *priv, struct stat_info *pstat,  unsigned char qnum, unsigned int dqnum, unsigned char releaseALL)
{
    int count = 0;
    
    while (1)
    {

		if(!releaseALL && rtl8192cd_swq_bdfull(priv, 0, qnum)) 
			break;
		
        struct sk_buff *tmpskb;
        tmpskb = skb_dequeue(&pstat->swq.swq_queue[qnum]);
        if (tmpskb == NULL)
            break;

#ifdef TX_EARLY_MODE
        if (GET_TX_EARLY_MODE && GET_EM_SWQ_ENABLE) 
            get_tx_early_info(priv, pstat, &pstat->swq.swq_queue[qnum]);
#endif

		priv->ext_stats.swq_xmit_out_pkt++;

#if defined(CONFIG_RTK_MESH)
        __rtl8192cd_start_xmit_out(tmpskb, pstat, NULL);
#else
        __rtl8192cd_start_xmit_out(tmpskb, pstat);
#endif
        count++;

		if(!releaseALL && count >= dqnum)
			break;
		
    }

    if(skb_queue_len(&pstat->swq.swq_queue[qnum]) == 0) {
        if(priv->pshare->swq_use_hw_timer && pstat->swq.swq_empty[qnum] == 1)
            priv->pshare->swq_numActiveSTA--;
        pstat->swq.swq_empty[qnum] = 0;        
    }
        
    return count;
}


#define MBPS_TO_BIT(mbps) (mbps*125000)
#define KBPS_TO_BIT(kbps) (kbps*125)  
__inline__ static UINT32 get_swq_tri_time_using_sw_timer(struct stat_info *pstat)
{
    UINT32 tri_time;
    
    if (pstat->tx_avarage > MBPS_TO_BIT(15))         //15M~
        tri_time = 1;
    else if (pstat->tx_avarage > MBPS_TO_BIT(4))    //4M~15M
        tri_time = 30;
    else if (pstat->tx_avarage > MBPS_TO_BIT(2))    //2M~4M
        tri_time = 90;
    else if (pstat->tx_avarage > KBPS_TO_BIT(800))  //800K~2M
        tri_time = 120;              
    else
        tri_time = 10;    

    tri_time = jiffies + RTL_MILISECONDS_TO_JIFFIES(tri_time);
        
    return tri_time;
}


#define GET_TRI_TIME_VALUE(tri_time,skb,priv,num_of_tri_time)              \
    do {                                                                     \
		if (IS_UDP_PROTO(skb->data))                                         \
			 tri_time =priv->pshare->rf_ft_var.udp_tri_time##num_of_tri_time; \
		else                                                                  \
			 tri_time =priv->pshare->rf_ft_var.tri_time##num_of_tri_time;     \
	} while (0)    

__inline__ static UINT32 get_swq_tri_time_using_hw_timer(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat)
{
    /*please reference to JIRA:APDEVELOP-181*/
    
    UINT32 tri_time;

    if(priv->pshare->swq_numActiveSTA > 8 && (priv->up_time - priv->pshare->swq_turbo_time) < priv->pshare->rf_ft_var.swqmaxturbotime)
		tri_time = 20;
	else if (pstat->tx_avarage > MBPS_TO_BIT(110)){      // 110M~
        GET_TRI_TIME_VALUE(tri_time,skb,priv,1);
    }
	else if (pstat->tx_avarage > MBPS_TO_BIT(15)){       //15M~110M
		GET_TRI_TIME_VALUE(tri_time,skb,priv,2);
	}
	else if (pstat->tx_avarage > MBPS_TO_BIT(5)){        //5M~15M
		GET_TRI_TIME_VALUE(tri_time,skb,priv,3);
	}
	else if (pstat->tx_avarage > KBPS_TO_BIT(2200)){      //2.2M~5M
		GET_TRI_TIME_VALUE(tri_time,skb,priv,4);
	}
    else if (pstat->tx_avarage > MBPS_TO_BIT(2)){        //2M~2.2M
		GET_TRI_TIME_VALUE(tri_time,skb,priv,5);
    }
	else if (pstat->tx_avarage > KBPS_TO_BIT(800)){      //800K~2M
		GET_TRI_TIME_VALUE(tri_time,skb,priv,6);
	}
	else if (pstat->tx_avarage > KBPS_TO_BIT(200)){      //200K~800K
		GET_TRI_TIME_VALUE(tri_time,skb,priv,7);
	}
	else{
		GET_TRI_TIME_VALUE(tri_time,skb,priv,8);
	}

    return tri_time;
}

__inline__ static UINT32 get_swq_tri_time(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat, int q_num)
{
    UINT32 tri_time;
    
    if(priv->pshare->swq_use_hw_timer) {
        tri_time = get_swq_tri_time_using_hw_timer(priv,skb,pstat);
           
    	if(pstat->swq.swq_prev_timeout[q_num] != tri_time) {
    		pstat->swq.swq_prev_timeout[q_num] = tri_time;
    		pstat->swq.swq_timeout_change[q_num] = 1;                                 
    	}  

        //get hw timer setting
        tri_time = RTL_R32(TSFTR1) + tri_time*1000;
    } 
    else {
        tri_time = get_swq_tri_time_using_sw_timer(pstat);
    }

    return tri_time;   
}

__inline__ static int rtl8192cd_swq_enqueue(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat)
{
    int q_len, pri, q_num;
    unsigned char need_deque = 0;
#if (MU_BEAMFORMING_SUPPORT == 1)	
	unsigned char need_deque_mu = 0;
	PRT_BEAMFORMING_INFO	pBeamInfo = &(priv->pshare->BeamformingInfo);
#endif	
    UINT32 tri_time;
    int assoc_num;
    int i;

#if (MU_BEAMFORMING_SUPPORT == 1)
    struct stat_info *psta_mu = pstat ? pstat->muPartner: NULL;
#endif

    pri = get_skb_priority(priv, skb, pstat);
#if defined(RTL_MANUAL_EDCA)
    q_num = PRI_TO_QNUM(priv, pri);
#else
    PRI_TO_QNUM(pri, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif

    if(priv->pshare->swq_use_hw_timer) {
        assoc_num = GET_ROOT(priv)->assoc_num;
        #ifdef MBSSID
        if (GET_ROOT(priv)->pmib->miscEntry.vap_enable){
            struct rtl8192cd_priv *vap_priv;
            for (i=0; i<RTL8192CD_NUM_VWLAN; ++i) {
                vap_priv = GET_ROOT(priv)->pvap_priv[i];
                if (vap_priv && 
                    IS_DRV_OPEN(vap_priv) && 
                    (GET_MIB(vap_priv)->dot11OperationEntry.opmode & WIFI_AP_STATE)
                    ) 
                {
                    assoc_num += vap_priv->assoc_num;
                }
            }
        }
        #endif	
  
#if (MU_BEAMFORMING_SUPPORT == 1)
		if(psta_mu){
			if(pstat->swq.swq_en[q_num]) {
				//if(pstat->tx_avarage < (priv->pshare->rf_ft_var.mutp_th_lower * 125000) || psta_mu->tx_avarage < (priv->pshare->rf_ft_var.mutp_th_lower * 125000) || pstat->mu_deq_num != psta_mu->mu_deq_num) {
				if(pstat->tx_avarage < (MBPS_TO_BIT(priv->pshare->rf_ft_var.mutp_th_lower)) || psta_mu->tx_avarage < (MBPS_TO_BIT(priv->pshare->rf_ft_var.mutp_th_lower))) 
				{
					pstat->swq.swq_en[q_num] = 0; /*disable swq*/  
					pstat->swq.swq_en[q_num] = 0; /*disable swq*/
					if (pstat->swq.swq_timer_id[q_num])
						rtl8192cd_swq_deltimer(priv, pstat, q_num);
					if (psta_mu->swq.swq_timer_id[q_num])
						rtl8192cd_swq_deltimer(priv, psta_mu, q_num);
					if(pstat->swq.swq_empty[q_num]) {
						rtl8192cd_swq_dequeue(priv, pstat, q_num, 0, 1);                          
					}
					if(psta_mu->swq.swq_empty[q_num]) {
            	    	rtl8192cd_swq_dequeue(priv, psta_mu, q_num, 0, 1);                          
            		}
				}
			} else {
				if(pstat->tx_avarage > (MBPS_TO_BIT(priv->pshare->rf_ft_var.mutp_th_up)) && psta_mu->tx_avarage > (MBPS_TO_BIT(priv->pshare->rf_ft_var.mutp_th_up))) {
					pstat->swq.swq_en[q_num] = 1; /*enable swq*/ 
		        	pstat->swq.swq_prev_timeout[q_num] = 1;
		        	pstat->swq.swq_timeout_change[q_num] = 0;
		        	pstat->swq.swq_keeptime[q_num] = 0;   
					psta_mu->swq.swq_en[q_num] = 1; /*enable swq*/ 
		        	psta_mu->swq.swq_prev_timeout[q_num] = 1;
		        	psta_mu->swq.swq_timeout_change[q_num] = 0;
		        	psta_mu->swq.swq_keeptime[q_num] = 0;
				}
			}
		} else
#endif
		{
			/* IOT for some STAs Rx test */
            /* Rx Check : Rx > 10M && Rx > 5 * Tx  */
			if ((MBPS_TO_BIT(10) < pstat->rx_avarage) &&  ((pstat->tx_avarage * 5) <  pstat->rx_avarage))  
		    {
    			//Rx mode, force data dequeue
    			if (pstat->swq.swq_en[q_num]) {

                   //force dequeue
                    need_deque = 1;
                }
    		} 
			else {
				/*check if we need to enable/disable the swq for the specific q_num and pstat*/
				if(pstat->swq.swq_en[q_num] && assoc_num <= 8) {
					if(pstat->tx_avarage < MBPS_TO_BIT(5) || MBPS_TO_BIT(160) < pstat->tx_avarage) { /* 5M < TP  or 160M < TP*/
						pstat->swq.swq_en[q_num] = 0; /*disable swq*/  
						if (pstat->swq.swq_timer_id[q_num])
							rtl8192cd_swq_deltimer(priv, pstat, q_num);
						if(pstat->swq.swq_empty[q_num]) {
							rtl8192cd_swq_dequeue(priv, pstat, q_num, 0 ,1);                          
						}
					}
				} else {
					if(assoc_num > 8 || (KBPS_TO_BIT(5500) < pstat->tx_avarage && pstat->tx_avarage < MBPS_TO_BIT(155))) { /* 5.5M < TP < 155M*/
						pstat->swq.swq_en[q_num] = 1; /*enable swq*/ 
						pstat->swq.swq_prev_timeout[q_num] = 1;
						pstat->swq.swq_timeout_change[q_num] = 0;
						pstat->swq.swq_keeptime[q_num] = 0;         
					}
				}
			}
		}

	    if(pstat->swq.swq_en[q_num] == 0) {	
			priv->ext_stats.swq_xmit_out_pkt++;

            #if defined(CONFIG_RTK_MESH)
            __rtl8192cd_start_xmit_out(skb, pstat, NULL);
            #else
            __rtl8192cd_start_xmit_out(skb, pstat);
            #endif
            goto exit;
        }
    }
    
    pstat->swq.q_used[q_num] = 1;    
    if (!pstat->ADDBA_ready[pri])
    {
        need_deque = 1;
    }
    
    if(need_deque == 0) {    
        q_len = skb_queue_len(&pstat->swq.swq_queue[q_num]);
        #ifdef TX_EARLY_MODE
        if (q_len + 1 >= (GET_EM_SWQ_ENABLE ? MAX_EM_QUE_NUM : pstat->swq.q_aggnum[q_num])) {
        #else
        if (q_len + 1 >= pstat->swq.q_aggnum[q_num]) {
        #endif		
            need_deque = 1;
        } 
    }

#if (MU_BEAMFORMING_SUPPORT == 1)
	if (pBeamInfo->beamformee_mu_cnt >= 2 && psta_mu) {
		unsigned int idx = 0;
		struct stat_info* psta;
		unsigned int qlen, qlen1;
		unsigned char	need_fire = 1;		

		qlen = skb_queue_len(&pstat->swq.swq_queue[q_num]);
		qlen1 = skb_queue_len(&psta_mu->swq.swq_queue[q_num]);
		
		if(qlen <= pstat->mu_deq_num || qlen1 <= psta_mu->mu_deq_num){
			need_fire = 0;
		}	

		if(need_fire){
			need_deque_mu = 1;
		}
	}

	if(need_deque_mu) {
		struct stat_info *psta[2];
		unsigned int qlen, qlen2;
		if(psta_mu) {
			psta[0] = pstat;
			psta[1] = psta_mu;	

			skb_queue_tail(&psta[0]->swq.swq_queue[q_num], skb);
			psta[0]->swq.swq_empty[q_num] = 1;									   								
			
			qlen = skb_queue_len(&psta[0]->swq.swq_queue[q_num]);
			qlen2 = skb_queue_len(&psta[1]->swq.swq_queue[q_num]);
	
			if (psta[0]->swq.swq_timer_id[q_num])
				rtl8192cd_swq_deltimer(priv, psta[0], q_num);
			if (psta[1]->swq.swq_timer_id[q_num])
				rtl8192cd_swq_deltimer(priv, psta[1], q_num);
			

			
			if (priv->pshare->rf_ft_var.dqnum){
				if(qlen >= psta[0]->mu_deq_num && qlen2 >= psta[1]->mu_deq_num){
					int count = psta[0]->mu_deq_num, count2 = psta[1]->mu_deq_num;
					int deq_num, sta_deq_num[2];

					priv->cnt_sta1_sta2++;

					if(count > 0 && count2 > 0) {
						if(count > count2)
							deq_num = (count / count2) + 1;
						else
							deq_num = (count2 / count) + 1;
					} else {
						panic_printk("mu_deq_num = 0\n");
					}

					while (1)
				    {						
				    	unsigned long drop_tmp;	
				    	if(rtl8192cd_swq_bdfull(priv, 0, q_num)) {           
							break;
				    	}
						if(count > 0 && count2 > 0) {
							if(count == count2) {
								sta_deq_num[0] = 1;
								sta_deq_num[1] = 1;
								count--;
								count2--;
							} else if(count > count2 && count >= deq_num) {
								sta_deq_num[0] = deq_num;
								sta_deq_num[1] = 1;
								count -= deq_num;
								count2 -= 1;
							} else if(count2 > count && count2 >= deq_num) {
								sta_deq_num[0] = 1;
								sta_deq_num[1] = deq_num;
								count -= 1;
								count2 -= deq_num;
							} else {
								sta_deq_num[0] = count;
								sta_deq_num[1] = count2;
								count = 0;
								count2 = 0;
							}
							if(rtl8192cd_swq_bdfull(priv, sta_deq_num[0]+sta_deq_num[1], q_num)) {           
								break;
				    		}
							rtl8192cd_swq_dequeue(priv, psta[0], q_num, sta_deq_num[0], 0);
							rtl8192cd_swq_dequeue(priv, psta[1], q_num, sta_deq_num[1], 0);
								
						} else if(count > 0) {
							rtl8192cd_swq_dequeue(priv, psta[0], q_num, count, 0);
							count = 0;
						} else if(count2 > 0) {
							rtl8192cd_swq_dequeue(priv, psta[1], q_num, count2, 0);
							count2 = 0;
						} else if(count == 0 && count2 == 0) {
							qlen = skb_queue_len(&psta[0]->swq.swq_queue[q_num]);
							qlen2 = skb_queue_len(&psta[1]->swq.swq_queue[q_num]);
							if(qlen < psta[0]->mu_deq_num || qlen2 < psta[1]->mu_deq_num)
								break;
							else {
								count = psta[0]->mu_deq_num;
								count2 = psta[1]->mu_deq_num;
							}
						} else {
							panic_printk("%s (%d) Wrong case, count=%d, count2=%d\n", __FUNCTION__, __LINE__, count, count2);
							break;
						}
				    }					
				}
				else if(qlen >= (priv->pshare->rf_ft_var.dqnum)){					
					priv->cnt_sta1_only++;
					rtl8192cd_swq_dequeue(priv, psta[0], q_num, psta[0]->mu_deq_num, 0);					
				} else if(qlen2 >= (priv->pshare->rf_ft_var.dqnum)){
					priv->cnt_sta2_only++;
					rtl8192cd_swq_dequeue(priv, psta[1], q_num, psta[1]->mu_deq_num, 0);
				}
			}
			else{
				if(psta[0]->swq.swq_empty[q_num]) {
					rtl8192cd_swq_dequeue(priv, psta[0], q_num, psta[0]->mu_deq_num, 0);			
				}
				if(psta[1]->swq.swq_empty[q_num]) {
					rtl8192cd_swq_dequeue(priv, psta[1], q_num, psta[1]->mu_deq_num, 0);			
				}

				priv->ext_stats.swq_xmit_out_pkt++;
			}
		}
	} else
#endif
    if(need_deque) {
        if (pstat->swq.swq_timer_id[q_num])
            rtl8192cd_swq_deltimer(priv, pstat, q_num);
        if(rtl8192cd_swq_bdfull(priv, pstat->swq.q_aggnum[q_num], q_num)) {                   
            skb_queue_tail(&pstat->swq.swq_queue[q_num], skb);
            if(priv->pshare->swq_use_hw_timer && pstat->swq.swq_empty[q_num] == 0) {
                priv->pshare->swq_numActiveSTA++;
                if(priv->pshare->swq_numActiveSTA == 9) {
                    turbo_swq_setting(priv);
                    priv->pshare->swq_turbo_time = priv->up_time;
                }
            }
            pstat->swq.swq_empty[q_num] = 1;                                       
        }
        else {
            if(pstat->swq.swq_empty[q_num]) {
				priv->cnt_mu_swqoverflow++;
                rtl8192cd_swq_dequeue(priv, pstat, q_num, skb_queue_len(&pstat->swq.swq_queue[q_num]), 0);
            }

			priv->ext_stats.swq_xmit_out_pkt++;

			if(pstat->swq.swq_empty[q_num]) {
	            skb_queue_tail(&pstat->swq.swq_queue[q_num], skb);
			} else {
				#if defined(CONFIG_RTK_MESH)
	            __rtl8192cd_start_xmit_out(skb, pstat, NULL);
	            #else
	            __rtl8192cd_start_xmit_out(skb, pstat);        
	            #endif
			}
        }
    }
    else {
        skb_queue_tail(&pstat->swq.swq_queue[q_num], skb);
        if(priv->pshare->swq_use_hw_timer && pstat->swq.swq_empty[q_num] == 0) {
            priv->pshare->swq_numActiveSTA++;
            if(priv->pshare->swq_numActiveSTA == 9) {
                turbo_swq_setting(priv);
                priv->pshare->swq_turbo_time = priv->up_time;
            }
        }
        pstat->swq.swq_empty[q_num] = 1;  
    }

#if (MU_BEAMFORMING_SUPPORT == 1)
	if(psta_mu) {
		if(pstat->swq.swq_empty[q_num]) { /*queue not empty, add timer for it*/        
	        if (pstat->swq.swq_timer_id[q_num] == 0)
	        {
				if(priv->pshare->swq_use_hw_timer) {
					if(priv->pshare->rf_ft_var.qtime)
						tri_time = priv->pshare->rf_ft_var.qtime;
					if(pstat->swq.swq_prev_timeout[q_num] != tri_time) {
	                    pstat->swq.swq_prev_timeout[q_num] = tri_time;
	                    pstat->swq.swq_timeout_change[q_num] = 1;                                 
	                }  
	                tri_time = RTL_R32(TSFTR1) + tri_time*1000;
				} else {
					tri_time = jiffies + RTL_MILISECONDS_TO_JIFFIES(tri_time);
				}
				rtl8192cd_swq_addtimer(priv, pstat, q_num, tri_time);           
	            pstat->swq.swq_timer_status[q_num] = rtl8192cd_swq_settimer(priv, tri_time);   /*need to re-setup timer*/                        
	        } else if(pstat->swq.swq_timer_status[q_num]) { /*re-set the timer*/
            	pstat->swq.swq_timer_status[q_num] = rtl8192cd_swq_settimer(priv, priv->pshare->swq_timer[pstat->swq.swq_timer_id[q_num] - 1].timeout); 
        	}			
		}
		if(psta_mu->swq.swq_empty[q_num]) { /*queue not empty, add timer for it*/        
	        if (psta_mu->swq.swq_timer_id[q_num] == 0)
	        {
				if(priv->pshare->swq_use_hw_timer) {
					if(priv->pshare->rf_ft_var.qtime)
						tri_time = priv->pshare->rf_ft_var.qtime;
					if(psta_mu->swq.swq_prev_timeout[q_num] != tri_time) {
	                    psta_mu->swq.swq_prev_timeout[q_num] = tri_time;
	                    psta_mu->swq.swq_timeout_change[q_num] = 1;                                 
	                }  
	                tri_time = RTL_R32(TSFTR1) + tri_time*1000;
				} else {
					tri_time = jiffies + RTL_MILISECONDS_TO_JIFFIES(tri_time);
				}
				rtl8192cd_swq_addtimer(priv, psta_mu, q_num, tri_time);           
	            psta_mu->swq.swq_timer_status[q_num] = rtl8192cd_swq_settimer(priv, tri_time);   /*need to re-setup timer*/                        
	        } else if(psta_mu->swq.swq_timer_status[q_num]) { /*re-set the timer*/
            	psta_mu->swq.swq_timer_status[q_num] = rtl8192cd_swq_settimer(priv, priv->pshare->swq_timer[psta_mu->swq.swq_timer_id[q_num] - 1].timeout); 
        	}
		}
		
	} 
	else
#endif
	{
		if(pstat->swq.swq_empty[q_num]) { /*queue not empty, add timer for it*/        
			if (pstat->swq.swq_timer_id[q_num] == 0)
			{   
                tri_time = get_swq_tri_time(priv,skb,pstat,q_num);

                rtl8192cd_swq_addtimer(priv, pstat, q_num, tri_time);           
				pstat->swq.swq_timer_status[q_num] = rtl8192cd_swq_settimer(priv, tri_time);   /*need to re-setup timer*/                        
			} else if(pstat->swq.swq_timer_status[q_num]) { /*re-set the timer*/
				pstat->swq.swq_timer_status[q_num] = rtl8192cd_swq_settimer(priv, priv->pshare->swq_timer[pstat->swq.swq_timer_id[q_num] - 1].timeout); 
			}
		}
	}

exit:  
    return 0;
 }

void rtl8192cd_swq_timeout(unsigned long task_priv)
{
    struct rtl8192cd_priv        *priv = (struct rtl8192cd_priv *)task_priv;
    struct sw_tx_queue_timer* swq_timer;
    int head;
    unsigned char add_timer;
    unsigned char need_dequeue;  
    unsigned char set_timer = 0;
    
    UINT32 add_timer_timeout;
    UINT32 set_timer_timeout = 0;
    struct stat_info * pstat;
    struct rtl8192cd_priv *priv_this=NULL;
    unsigned char qnum;    
    UINT32 current_time;
    int bdfull;
    unsigned long x = 0;
    SMP_LOCK_XMIT(x);
    SAVE_INT_AND_CLI(x);
    
    priv->pshare->swq_current_timeout = 0;
    if(priv->pshare->swq_use_hw_timer) {     
        current_time = RTL_R32(TSFTR1);
    }
    else {
        current_time = jiffies;
    }
  

    head = priv->pshare->swq_timer_head;
    while (head != priv->pshare->swq_timer_tail) //check if queue is empty
    {
        priv->pshare->swq_timer_tail = (priv->pshare->swq_timer_tail + 1) % SWQ_TIMER_NUM;    
        swq_timer = &priv->pshare->swq_timer[priv->pshare->swq_timer_tail];
        if (swq_timer->pstat) {           
            pstat = swq_timer->pstat;            
            priv_this = swq_timer->priv;
            qnum = swq_timer->qnum;
            add_timer_timeout = swq_timer->timeout;

            swq_timer->pstat = NULL;
            pstat->swq.swq_timer_id[qnum] = 0;


            if (!(priv_this->drv_state & DRV_STATE_OPEN)) {
                continue;
            }   
           
            need_dequeue = 0;
            if(priv->pshare->swq_use_hw_timer) {     
                if(add_timer_timeout -  (current_time + SWQ_HWTIMER_TOLERANCE) > SWQ_HWTIMER_MAXIMUN) {
                    need_dequeue = 1;
                }
            }
            else {
                if(add_timer_timeout <= current_time) {
                    need_dequeue = 1;
                }
            }
            
            add_timer = 0;
            if(need_dequeue) {  
                bdfull = rtl8192cd_swq_bdfull(priv, 0, qnum);
                if(bdfull && skb_queue_len(&pstat->swq.swq_queue[qnum]) < pstat->swq.q_aggnum[qnum]*2 ) {
                    add_timer = 1;                             
                    if(priv->pshare->swq_use_hw_timer) {        
                        add_timer_timeout = 1;   
                        add_timer_timeout = add_timer_timeout*1000; // to us
                    }
                    else {
                        add_timer_timeout = 30;   
                        add_timer_timeout = RTL_MILISECONDS_TO_JIFFIES(add_timer_timeout); 
                    }
                    add_timer_timeout += current_time; 

                }
                else {
                        if(priv->pshare->swq_use_hw_timer) {
							priv->cnt_mu_swqtimeout++;
                        	rtl8192cd_swq_dequeue(priv_this, pstat, qnum, skb_queue_len(&pstat->swq.swq_queue[qnum]), 0);
                            pstat->swq.q_TOCount[qnum]++;                        
                        }
                        else {
                        	pstat->swq.q_TOCount[qnum] += rtl8192cd_swq_dequeue(priv_this, pstat, qnum, skb_queue_len(&pstat->swq.swq_queue[qnum]), 0);  
                        }
						if(pstat->swq.swq_empty[qnum])
							add_timer = 1;
                }
                
                if(!bdfull) {
                    adjust_swq_setting(priv, pstat, qnum, CHECK_DEC_AGGN);
                }
            }
            else {                        
                add_timer = 1;
            }


            if(add_timer) { 
                rtl8192cd_swq_addtimer(priv_this, pstat, qnum, add_timer_timeout);  
                if(set_timer == 0) {
                    set_timer_timeout = add_timer_timeout;
                }
                else {
                    if(priv->pshare->swq_use_hw_timer) { 
                        if(add_timer_timeout - set_timer_timeout > SWQ_HWTIMER_MAXIMUN) {
                            set_timer_timeout = add_timer_timeout;
                        }
                    }
                    else {
                        if(add_timer_timeout < set_timer_timeout) {
                            set_timer_timeout = add_timer_timeout;
                        }                    
                    }
                }                
                
                set_timer = 1;
            }
        }


    }

    if(set_timer) {
        rtl8192cd_swq_settimer(priv, set_timer_timeout);   /*need to re-setup timer*/
    }    

    priv->pshare->has_triggered_sw_tx_Q_tasklet = 0;

    RESTORE_INT(x);		
    SMP_UNLOCK_XMIT(x);    
}    
#endif //SW_TX_QUEUE



#ifdef RTK_ATM
__inline__ static void rtl8192cd_atm_swq_settimer(struct rtl8192cd_priv *priv, UINT32 timer_timeout)
{
    UINT32 set_timeout, current_time;    
    unsigned char set_timer = 0;

	//delet timer when timeout == 0
    if(timer_timeout == 0) {
		if(priv->pshare->atm_swq_current_timeout) {
            priv->pshare->atm_swq_current_timeout = 0;
	    	if(priv->pshare->atm_swq_use_hw_timer) {                          
	        	RTL_W32(TC4_CTRL, 0);           
	        } else {      
	        	if (timer_pending(&priv->pshare->atm_swq_sw_timer))
					del_timer_sync(&priv->pshare->atm_swq_sw_timer);
	        }
		}
		priv->pshare->atm_timer_init = 0;//reset timer
    }
    else {
        if(priv->pshare->atm_swq_use_hw_timer) {			
        	current_time = RTL_R32(TSFTR1);   
            set_timeout = current_time + SWQ_HWTIMER_MINIMUN;
            if(timer_timeout - set_timeout > SWQ_HWTIMER_MAXIMUN) {                
                timer_timeout = set_timeout;
                set_timeout = SWQ_HWTIMER_MINIMUN;                
            }            
            else {
                set_timeout = timer_timeout - current_time;
            }

            if(priv->pshare->atm_swq_current_timeout) {
                if((timer_timeout+SWQ_HWTIMER_TOLERANCE-priv->pshare->atm_swq_current_timeout)>SWQ_HWTIMER_MAXIMUN)                 
                    set_timer = 1;
            } else
                set_timer = 1;

            if(set_timer) {                         
                priv->pshare->atm_swq_current_timeout = (timer_timeout == 0?1:timer_timeout);            
                set_timeout = RTL_MICROSECONDS_TO_GTIMERCOUNTER(set_timeout);                
                set_timeout = BIT26 | BIT24 | (set_timeout & 0x00FFFFFF);  //TC40INT_EN | TC4EN | TC4Data
                RTL_W32(TC4_CTRL, set_timeout);           
            }                      
        } else {//sw timer jiffies
        	if(priv->pshare->atm_swq_current_timeout) {                          
                if(timer_timeout < priv->pshare->atm_swq_current_timeout)               
                    set_timer = 1;
            } else            
                set_timer = 1;

            if(set_timer){
				priv->pshare->atm_swq_current_timeout = timer_timeout;
        		mod_timer(&priv->pshare->atm_swq_sw_timer, (unsigned long)timer_timeout);
            }
        }
    }
}

void rtl8192cd_atm_swq_inittimer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	UINT32 tri_time;

	if(priv->pshare->atm_swq_use_hw_timer)
		tri_time = RTL_R32(TSFTR1) + priv->pshare->atm_timer*1000;
	else
		tri_time = jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->atm_timer);

	priv->pshare->atm_swq_current_timeout = 0;
	rtl8192cd_atm_swq_settimer(priv, tri_time);	
}

__inline__ static int rtl8192cd_atm_swq_dequeue(struct rtl8192cd_priv *priv);


void rtl8192cd_atm_swq_timeout(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	UINT32 tri_time;
	unsigned long x = 0;

	priv->pshare->atm_timeout_count++;

    SMP_LOCK_XMIT(x);
    SAVE_INT_AND_CLI(x);
	
	//atm swq dequeue
	rtl8192cd_atm_swq_dequeue(priv);

	//set next timer
	if(priv->pshare->atm_swq_use_hw_timer)
		tri_time = RTL_R32(TSFTR1) + priv->pshare->atm_timer*1000;
	else
		tri_time = jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->atm_timer);

	priv->pshare->atm_swq_current_timeout = 0;
	if(priv->pshare->atm_swq_en)
		rtl8192cd_atm_swq_settimer(priv, tri_time);	
	else
		rtl8192cd_atm_swq_settimer(priv, 0);
	
	RESTORE_INT(x);		
    SMP_UNLOCK_XMIT(x); 
	
	return;
}

void atm_check_refill(struct rtl8192cd_priv *priv)
{
	int i;
	int is_all_burst=1, min_burst_sent=0xffff;
	struct stat_info *pstat=NULL;
	
	for(i=0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)){
			pstat = &(priv->pshare->aidarray[i]->station);
			//its means someone has quota to send
			if(pstat->atm_burst_sent<pstat->atm_burst_num && is_all_burst==1){
				if((pstat->atm_burst_sent > pstat->atm_burst_num/1000) && pstat->atm_is_maxsta)
					is_all_burst = 0;//not all sta is burst out
				else
					;//sta sent only 1/1000 packets, so others keep sending burst packet
			}		
		}
	}

	//if all other client is burst
	if(is_all_burst==1){	
		for(i=0; i<NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)){
				pstat = &(priv->pshare->aidarray[i]->station);

				if(pstat->atm_burst_sent >= pstat->atm_burst_num)
					pstat->atm_burst_sent -= pstat->atm_burst_num;
				else
					pstat->atm_burst_sent = 0;
			}
		}
	}
}

//#define ATM_MAX_AGGR_NUM 16

__inline__ static int rtl8192cd_atm_swq_enqueue(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat)
{

	int pri, q_num, q_len;
	//check qnum
	pri = get_skb_priority(priv, skb, pstat);
#if defined(RTL_MANUAL_EDCA)
    q_num = PRI_TO_QNUM(priv, pri);
#else
    PRI_TO_QNUM(pri, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif

	//q_len >= atm_enqmax and burst_sent is over than burst_num, drop 1 packet
	{	
		q_len = skb_queue_len(&pstat->atm_swq.swq_queue[q_num]);
		if(q_len >= priv->pshare->rf_ft_var.atm_enqmax)
		{
			pstat->atm_swq_full++;
			if(/*pstat->atm_burst_sent >= pstat->atm_burst_num &&*/
				(priv->pshare->atm_ttl_stanum>1 && !pstat->atm_is_maxsta) ||
				(pstat->atm_is_maxsta && 
					q_len>=priv->pshare->rf_ft_var.atm_enqmax+priv->pshare->rf_ft_var.atm_hista_enqbuf))
			{
		        struct sk_buff *tmpskb;
		    	tmpskb = skb_dequeue(&pstat->atm_swq.swq_queue[q_num]);		
		        if (tmpskb) {
					priv->ext_stats.tx_drops++;
					pstat->atm_drop_cnt++;
					rtl_kfree_skb(priv, tmpskb, _SKB_TX_);				
	        	}
			}
		}
	}

	//enqueue
	{
        skb_queue_tail(&pstat->atm_swq.swq_queue[q_num], skb);
		pstat->atm_swq.swq_empty[q_num] = 1;
    }

	//check refill
	//if sta already rearch my target burst, check other if burst out, too
	{		
		if(pstat->atm_burst_sent >= pstat->atm_burst_num){
			pstat->atm_wait_cnt++;			
			atm_check_refill(priv);
		}		
	}

	//init timer
	{
		if(!priv->pshare->atm_timer_init){
			rtl8192cd_atm_swq_inittimer(priv);
			priv->pshare->atm_timer_init = 1;			
			priv->pshare->atm_timeout_count = 0;
		}
	}
	
	return 0;
}

__inline__ static int atm_skb_dequeue(struct rtl8192cd_priv *priv, struct stat_info *pstat, int deq_size, int qnum)
{	
	int count = 0, flush_all = 0;
	struct sk_buff *tmpskb;
    
	if(deq_size == priv->pshare->rf_ft_var.atm_aggmax)
		deq_size = 0xff;
    
    while (1)
    {        
        tmpskb = skb_dequeue(&pstat->atm_swq.swq_queue[qnum]);
        if (tmpskb == NULL)
            break;

#ifdef TX_EARLY_MODE
        if (GET_TX_EARLY_MODE && GET_EM_SWQ_ENABLE) 
            get_tx_early_info(priv, pstat, &pstat->atm_swq.swq_queue[qnum]);
#endif

#if defined(CONFIG_RTK_MESH)
        __rtl8192cd_start_xmit_out(tmpskb, pstat, NULL);
#else
        __rtl8192cd_start_xmit_out(tmpskb, pstat);
#endif

        count++;
		pstat->atm_burst_sent++;

		if(deq_size == 0xff)
			continue;

		//if full, sent 1 packet
		if(pstat->atm_burst_sent >= pstat->atm_burst_num){
			pstat->atm_full_sent_cnt++;
			break;
		}

		if(count == deq_size)			
			break;
	}

	pstat->atm_swq_sent_cnt += count;

    if(skb_queue_len(&pstat->atm_swq.swq_queue[qnum]) == 0)
        pstat->atm_swq.swq_empty[qnum] = 0;	
	
    return count;	
}

#define MAX_DEQUEUE_NUM 16

unsigned int sta_idx[4]={0,0,0,0};

__inline__ static int rtl8192cd_atm_swq_dequeue(struct rtl8192cd_priv *priv)
{
	int i, qnum, q_len, deq_num=0, *pempty, burst_num=0;
	struct stat_info *pstat;
	int sta_cnt = 0, sta_used_cnt = 0, bd_full = 0;

	if(priv->pshare->rf_ft_var.atm_en==0 || priv->pshare->atm_swq_en==0)
		return 0;
	
	if(priv->pshare->atm_ttl_stanum==0)
		return 0;		
#if 0
	//VO dequeue -> VI -> BE -> BK
	for(qnum = VO_QUEUE; qnum >= BK_QUEUE ; qnum--){
		for(i=0; i<NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)){
				pstat = &(priv->pshare->aidarray[i]->station);

				if(rtl8192cd_swq_bdfull(priv, pstat, qnum))
					pstat->atm_txbd_full[qnum-1]++;

				//burst_num = (priv->pshare->rf_ft_var.atm_aggmax*pstat->atm_burst_num)/priv->pshare->rf_ft_var.atm_quota;
				burst_num = (priv->pshare->rf_ft_var.atm_aggmax*pstat->atm_burst_size)/priv->pshare->atm_max_burstsize;

				if(burst_num < priv->pshare->rf_ft_var.atm_aggmin)
				burst_num = priv->pshare->rf_ft_var.atm_aggmin;
						
				pstat->atm_burst_unit = burst_num;
				deq_num = atm_skb_dequeue(priv, pstat, burst_num, qnum);
			}
		}
	}
#else
	for(qnum = VO_QUEUE; qnum >= BK_QUEUE ; qnum--){
		sta_cnt = 0;//0~63
		sta_used_cnt = 0;//used count

		i = sta_idx[qnum-1];
		
		while(sta_cnt < NUM_STAT){
			
			if (priv->pshare->aidarray[(i%NUM_STAT)] && (priv->pshare->aidarray[(i%NUM_STAT)]->used == TRUE)){
				pstat = &(priv->pshare->aidarray[(i%NUM_STAT)]->station);

				bd_full = rtl8192cd_swq_bdfull(priv, 0, qnum);
				if(!bd_full){					
					burst_num = (priv->pshare->rf_ft_var.atm_aggmax*pstat->atm_burst_size)/priv->pshare->atm_max_burstsize;
					if(burst_num < priv->pshare->rf_ft_var.atm_aggmin)
						burst_num = priv->pshare->rf_ft_var.atm_aggmin;
							
					pstat->atm_burst_unit = burst_num;
					deq_num = atm_skb_dequeue(priv, pstat, burst_num, qnum);					
				}else
					pstat->atm_txbd_full[qnum-1]++;

				//find 2nd sta
				if(sta_used_cnt == 1)
					sta_idx[qnum-1] = i%NUM_STAT;
								
				//printk("sta[%d][%d] send.\n", qnum, i%NUM_STAT);

				sta_used_cnt++;
			}

			//next
			i++;
			sta_cnt++;//0~63
		}
		//printk("\n");
	}

	//printk("-----------------------------------------\n");
#endif	
	return 0;
}
#endif//RTK_ATM


#ifdef CONFIG_PCI_HCI

#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
int __rtl8192cd_start_xmit(struct sk_buff *skb, struct net_device *dev, int tx_flag)
{
	struct rtl8192cd_priv *priv;
	struct stat_info	*pstat=NULL;
	unsigned char		*da;
	unsigned int		ret;
#if defined(__KERNEL__) || defined(__OSK__)
#if defined(HS2_SUPPORT) || !defined(CONFIG_PCI_HCI)
	struct sk_buff *newskb = NULL;
#endif
#endif
#ifdef TX_SCATTER
	struct sk_buff *newskb = NULL;
#endif
	struct net_device *wdsDev = NULL;
#if defined(SW_TX_QUEUE) || defined(RTK_ATM)
	int swq_out = 0;
#endif
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	struct vlan_info *vlan=NULL;
#endif
	struct tx_insn tx_insn;
#ifdef CONFIG_RTL_EAP_RELAY
	int real_len;
#endif
#ifdef CONFIG_RTL_VLAN_8021Q
    uint16 vid;
    struct sk_buff *newskb;
#endif
#ifdef CONFIG_PUMA_VLAN_8021Q
    struct sk_buff *newskb;
#endif
    int temp;
#ifdef SMP_SYNC
	unsigned long flags;
#endif


	DECLARE_TXCFG(txcfg, tx_insn);

	priv = GET_DEV_PRIV(dev);

#ifdef ENABLE_RTL_SKB_STATS
	rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif

	if (skb->len < 15)
    {
        _DEBUG_ERR("TX DROP: SKB len small:%d\n", skb->len);
        goto free_and_stop;
    }

    #ifdef CONFIG_RTL_NETSNIPER_SUPPORT
    rtl_netsniper_check(skb, dev);
    #endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	if (dev==wlan_device[passThruWanIdx].priv->pWlanDev ||skb->dev==wlan_device[passThruWanIdx].priv->pWlanDev)
	{
#ifdef __ECOS
		if (SUCCESS==rtl_isWlanPassthruFrame(skb->data))
#else
		if (SUCCESS==rtl_isPassthruFrame(skb->data))
#endif
		{
#ifdef UNIVERSAL_REPEATER			
			if(passThruStatusWlan & 0x8) //WISP Mode Enable. default is vxd
			{
			    // TODO: need to add 8881A?, check it
         		   //#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)||defined (CONFIG_RTL_8881A)
			//dual band			
				#if (defined (CONFIG_USE_PCIE_SLOT_0)&&(defined (CONFIG_USE_PCIE_SLOT_1) || defined (CONFIG_RTL_8197F)))||( defined(CONFIG_RTL_8881A)&& !defined(CONFIG_RTL_8881A_SELECTIVE))|| defined(CONFIG_WLAN_HAL_8814AE)
				unsigned int wispWlanIndex=(passThruStatusWlan&WISP_WLAN_IDX_MASK)>>WISP_WLAN_IDX_RIGHT_SHIFT;
				  if ((wlan_device[wispWlanIndex].priv->drv_state & DRV_STATE_OPEN )&&
				  	 ((GET_MIB(GET_VXD_PRIV((wlan_device[wispWlanIndex].priv)))->dot11OperationEntry.opmode) & WIFI_STATION_STATE))
				  {
					#if 0//def NETDEV_NO_PRIV
				    	//dev=skb->dev=((struct rtl8192cd_priv *)(((struct rtl8192cd_priv *)netdev_priv(wlan_device[wispWlanIndex].priv->pWlanDev))->wlan_priv))->pvxd_priv->dev;
						dev=skb->dev=(((struct rtl8192cd_priv *)netdev_priv(wlan_device[wispWlanIndex].priv->dev))->wlan_priv)->pvxd_priv->dev;
					#else	
						dev=skb->dev=GET_VXD_PRIV(wlan_device[wispWlanIndex].priv)->dev;
					#endif
				  } else {
					goto free_and_stop;
				  }
				#else		
					#if 0//def NETDEV_NO_PRIV
					//dev=skb->dev=((struct rtl8192cd_priv *)(((struct rtl8192cd_priv *)netdev_priv(wlan_device[passThruWanIdx].priv->pWlanDev))->wlan_priv))->pvxd_priv->dev;
					dev=skb->dev=(((struct rtl8192cd_priv *)netdev_priv(wlan_device[wispWlanIndex].priv->dev))->wlan_priv)->pvxd_priv->dev;
					#else	
					dev=skb->dev= GET_VXD_PRIV(wlan_device[passThruWanIdx].priv)->dev;
					#endif
				#endif
			} else
#endif
			{
			    // TODO: need to add 8881A?, check it
				//#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)||defined (CONFIG_RTL_8881A)
				//dual band
				#if (defined (CONFIG_USE_PCIE_SLOT_0)&&(defined (CONFIG_USE_PCIE_SLOT_1) || defined (CONFIG_RTL_8197F)))||( defined(CONFIG_RTL_8881A)&& !defined(CONFIG_RTL_8881A_SELECTIVE))|| defined(CONFIG_WLAN_HAL_8814AE)
				unsigned int wispWlanIndex=(passThruStatusWlan&WISP_WLAN_IDX_MASK)>>WISP_WLAN_IDX_RIGHT_SHIFT;
				  if ((wlan_device[wispWlanIndex].priv->drv_state & DRV_STATE_OPEN )&&
				  	 (((GET_MIB(wlan_device[wispWlanIndex].priv))->dot11OperationEntry.opmode) & WIFI_STATION_STATE))
				  {
					#if 0//def NETDEV_NO_PRIV
				        dev=skb->dev=((struct rtl8192cd_priv *)netdev_priv(wlan_device[passThruWanIdx].priv->pWlanDev))->wlan_priv->dev;
					#else	
						dev=skb->dev=wlan_device[wispWlanIndex].priv->dev;
					#endif
				  } else {
					goto free_and_stop;
				  }
				#else		
					#ifdef NETDEV_NO_PRIV
					dev=skb->dev=((struct rtl8192cd_priv *)netdev_priv(wlan_device[passThruWanIdx].priv->pWlanDev))->wlan_priv->dev;				
					#else	
					dev=skb->dev=((struct rtl8192cd_priv *)(wlan_device[passThruWanIdx].priv->pWlanDev->priv))->dev;
					#endif
				#endif
			}

		SMP_UNLOCK_XMIT(flags);
			
		rtl8192cd_start_xmit(skb, dev);
		
		SMP_LOCK_XMIT(flags);
		return 0;	

		} else {
			goto free_and_stop;
		}
	}
#endif	/* defined(CONFIG_RTL_CUSTOM_PASSTHRU) */

#if 0//def CONFIG_RTL_STP
	//port5: virtual device for wlan0 stp BPDU process
	if(memcmp((void *)(dev->name), "port5", 5)==0)
	{
		if ((skb->data[0]&0x01) && !memcmp(&(skb->data[0]), STPmac, 5) && !(skb->data[5] & 0xF0))
		{
			//To virtual device port5, tx bpdu.
		}
		else
		{
			//To virtual device port5, drop tx pkt becasue not bpdu!
			goto free_and_stop;
		}
	}
#endif

#ifdef CONFIG_RTL_EAP_RELAY
//mark_test , remove EAP padding by ethernet
	if (*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2)) == __constant_htons(0x888e))
	{			
		real_len = ETH_ALEN*2+2+4+ntohs(*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2+2+2)));
		skb_trim(skb,real_len);
		//printk("2 wlan tx EAP ,skb->len=%d,skb->data=%d \n",skb->len,skb->data_len);		
	}
#endif

//#ifdef SW_TX_QUEUE
        skb->dev = dev;
//#endif

#ifdef WDS
	if (!dev->base_addr && skb_cloned(skb)
		&& (priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_)
		) {
		struct sk_buff *mcast_skb = NULL;
		mcast_skb = skb_copy(skb, GFP_ATOMIC);
		if (mcast_skb == NULL) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: Can't copy the skb!\n");
			goto free_and_stop;
		}
		dev_kfree_skb_any(skb);
		skb = mcast_skb;
	}
#endif

#ifdef	IGMP_FILTER_CMO
	if (priv->pshare->rf_ft_var.igmp_deny)
	{
		if ((OPMODE & WIFI_AP_STATE) &&	IS_MCAST(skb->data))
		{
			if (IP_MCAST_MAC(skb->data)
				#ifdef	TX_SUPPORT_IPV6_MCAST2UNI
				|| ICMPV6_MCAST_MAC(skb->data)
				#endif
				)
			{
				if(!IS_IGMP_PROTO(skb->data)){
					priv->ext_stats.tx_drops++;										
					DEBUG_ERR("TX DROP: Multicast packet filtered\n");
					goto free_and_stop;
				}
			}
		}
	}
#endif

#ifdef HS2_SUPPORT
	if (priv->proxy_arp)	{

		unsigned short protocol;

		if(isDHCPpkt(skb))	{
			staip_snooping_bydhcp(skb, priv);
		}

		if (IS_MCAST(skb->data))	{
			protocol = *((unsigned short *)(skb->data + 2 * ETH_ALEN));
	
			//proxy arp	
			if (protocol == __constant_htons(ETH_P_ARP))	{
				if (proxy_arp_handle(priv, skb))	{
					//reply ok and return
			        	goto stop_proc;
				}
				//drop packets
		        	goto free_and_stop;
			}
			//icmpv6 
			if (ICMPV6_MCAST_SOLI_MAC(skb->data))	{
			//if (ICMPV6_MCAST_MAC(skb->data))
				if (proxy_icmpv6_ndisc(priv,skb))	{
					goto stop_proc;
				}
				//drop packets
				goto free_and_stop;
			} 
			//drop unsolicited neighbor advertisement when proxy arp=1
			if (ICMPV6_MCAST_MAC(skb->data))	{
				if (check_nei_advt(priv,skb))	{
					 //drop packets
			             goto free_and_stop;
				}
			}
		}
	} 
	
	if(priv->dgaf_disable)
	{
		//dhcp m2u check
		if (IS_MCAST(skb->data))
		{
			if (isDHCPpkt(skb))
			{
				struct list_head *phead, *plist;
                HS2_DEBUG_INFO("DHCP multicast to unicast\n");
				phead = &priv->asoc_list;
				plist = phead->next;
				while (phead && (plist != phead)) 
				{
					pstat = list_entry(plist, struct stat_info, asoc_list);
					plist = plist->next;

					/* avoid   come from STA1 and send back STA1 */
			        if (!memcmp(pstat->hwaddr, &skb->data[6], 6))
					{
						continue;
					}
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
					newskb = priv_skb_copy(skb);
#else
					newskb = skb_copy(skb, GFP_ATOMIC);
#endif
	                if (newskb) 
					{
						memcpy(newskb->data, pstat->hwaddr, 6);
				        newskb->cb[2] = (char)0xff;         // not do aggregation
				        memcpy(newskb->cb+10,newskb->data,6);
                        #if 0   // for debug
						printk("send m2u, da=");						
						MAC_PRINT(pstat->hwaddr);
						printk("\n");
                        #endif
						__rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
					}
				}
			}
	        goto free_and_stop;
		}
	}
#endif

#ifdef SUPPORT_TX_MCAST2UNI
#ifdef WDS			// when interface is WDS don't enter multi2uni path
	if (dev->base_addr && !priv->pmib->dot11WdsInfo.wdsPure)
#endif
	if (!priv->pshare->rf_ft_var.mc2u_disable) {
		if ((OPMODE & WIFI_AP_STATE) &&	IS_MCAST(skb->data))
		{
			if ((IP_MCAST_MAC(skb->data)
#if defined(__OSK__)
				// on osk platform, we transform dhcp broadcast packet into unicast packet for lease packet loss.
				|| IPV4_DHCP_BROADCAST_PACKET(skb->data)
#endif
				#ifdef	TX_SUPPORT_IPV6_MCAST2UNI
				|| IPV6_MCAST_MAC(skb->data)
				#endif
				) && (tx_flag != TX_NO_MUL2UNI))
			{
#if defined(__OSK__) && defined(CONFIG_RTL8672)
				if(enable_IGMP_SNP){		
					int i;
					for (i=0; i<RTL8192CD_NUM_VWLAN+1; i++) {	
						if ( skb->dev == wlanDev[i].dev_pointer) {
							if(check_wlan_mcast_tx(skb,i)==1){
								//xprintfk("stop tx to wlan.[%s]:[%d].\n", __FUNCTION__,__LINE__);
								goto free_and_stop;
							}
						}
					}
				}
#ifdef INET6
				if (enable_MLD_SNP) {
					int i;
						for (i=0; i<RTL8192CD_NUM_VWLAN+1; i++) {	
						if ( skb->dev == wlanDev[i].dev_pointer) {
							 if(check_wlan_ipv6_mcast_tx(skb,i)==1) {
								goto free_and_stop;
							}  		
						}
					}
				}
#endif
#endif
				if (mlcst2unicst(priv, skb)){
					return 0;
				}
			}
		}

		skb->cb[2] = 0;	// allow aggregation
	}

	skb->cb[16] = tx_flag;
#endif

#if !defined(CONFIG_RTL_8676HWNAT) && defined(CONFIG_RTL8672) && !defined(CONFIG_RTL8686) && !defined(CONFIG_RTL8685) && !defined(CONFIG_RTL8685S)
#ifdef __OSK__
//IGMP snooping
	if(enable_IGMP_SNP){
		int i;
		for (i=0; i<RTL8192CD_NUM_VWLAN+1; i++) {	
			if ( skb->dev == wlanDev[i].dev_pointer) {
				if(check_wlan_mcast_tx(skb,i)==1) {
					goto free_and_stop;
				}  
				break;
			}
		}
	}
#ifdef INET6
	if (enable_MLD_SNP) {\
		int i;
		for (i=0; i<RTL8192CD_NUM_VWLAN+1; i++) {	
			if ( skb->dev == wlanDev[i].dev_pointer) {
				if(check_wlan_ipv6_mcast_tx(skb,i)==1) {
					goto free_and_stop;
				}  
				break;
			}
		}
	}
#endif
#else
	//IGMP snooping
	if (check_wlan_mcast_tx(skb)==1) {
		goto free_and_stop;
	}
#endif //__OSK__
#endif

#if defined(__OSK__) && defined(CONFIG_MULTICAST_PORTMAPPING)
	if( enable_port_mapping )
	{
		unsigned int wlanportmap=0;
		int ipv6flag=0;
		int multflag=0;
		if((skb->data[0]==0x01) && (skb->data[1]==0x0) && ((skb->data[2]&0xfe)==0x5e)){
			multflag=1;
		}
#ifdef INET6
		if((skb->data[0]==0x33) && (skb->data[1]==0x33)){
			multflag=1;
			ipv6flag=1;
		}
#endif
		if(multflag && (skb->wan_logport != 0)){
			//bypass non-multicast pkt
			int k;
			for (k=0; k<RTL8192CD_NUM_VWLAN+1; k++) {
				if ( dev == wlanDev[k].dev_pointer) {
					wlanportmap=wlan_multicast_portmapping(skb,ipv6flag);
					if (wlanportmap<0)
					{		
					goto free_and_stop;
					} else {
						if((wlanportmap&(1<<k))!=0)
						{
							break;
						} else {	
							goto free_and_stop;
						}
					}
				}
			}
	   	}		
	}
#endif

	if(priv->pshare->rf_ft_var.drop_multicast && IS_MCAST(skb->data) && ((unsigned char)skb->data[0] != 0xff))
		goto free_and_stop;	
		
#ifdef DFS
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		GET_ROOT(priv)->pmib->dot11DFSEntry.disable_tx 
		&& (priv->site_survey)
		&& (priv->site_survey->hidden_ap_found != HIDE_AP_FOUND_DO_ACTIVE_SSAN)) {
		priv->ext_stats.tx_drops++;
		DEBUG_ERR("TX DROP: DFS probation period\n");
		goto free_and_stop;
	}
#endif

	if (!IS_DRV_OPEN(priv))
		goto free_and_stop;

#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE) {
		goto free_and_stop;
	}
#endif

#ifdef MULTI_MAC_CLONE
	if (priv->pmib->ethBrExtInfo.macclone_enable) {
		int id;
		if (!(skb->data[ETH_ALEN] & 1) && MCLONE_NUM > 0) {
			id = mclone_find_address(priv, skb->data+ETH_ALEN, skb, MAC_CLONE_SA_FIND);
			if (id > 0)
				ACTIVE_ID = id;
			else
				ACTIVE_ID = 0;
		}  else
			ACTIVE_ID = 0;
	}
	else
		ACTIVE_ID = 0;
#endif

#ifdef CONFIG_RTL867X_VLAN_MAPPING
	if (re_vlan_loaded()) {
		struct ethhdr *eth = (struct ethhdr *)skb->data;
		unsigned short vid = 0;

		if (eth->h_proto != ETH_P_PAE) {
			if (re_vlan_skb_xmit(skb, &vid)) {
				priv->ext_stats.tx_drops++;
				return 0;
			}
			if (vid && re_vlan_addtag(skb, vid)) {
				priv->ext_stats.tx_drops++;
				return 0;
			}
		}
	}
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
		vlan = &priv->pmib->vlan;

	if (vlan->global_vlan &&
			*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2)) != __constant_htons(0x888e)) {
		int get_pri = 0;
#ifdef WIFI_WMM
		if (QOS_ENABLE) {
#ifdef CLIENT_MODE
			if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE))
				pstat = get_stainfo(priv, BSSID);
			else
#endif
			{
#ifdef MCAST2UI_REFINE
                                if (pstat == NULL && !IS_MCAST(&skb->cb[10]))
                                        pstat = get_stainfo(priv, &skb->cb[10]);
#else
				if (pstat == NULL && !IS_MCAST(skb->data))
					pstat = get_stainfo(priv, skb->data);
#endif
			}

			if (pstat && pstat->QosEnabled)
				get_pri = 1;
		}
#endif

		if (!get_pri)
			skb->cb[0] = '\0';

		if (tx_vlan_process(dev, &priv->pmib->vlan, skb, get_pri)) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: by vlan!\n");
			goto free_and_stop;
		}
	}
	else
		skb->cb[0] = '\0';
#endif

#if defined(CONFIG_RTL_VLAN_PASSTHROUGH_SUPPORT)
	if (*(unsigned short *)(&skb->data[ETH_ALEN<<1]) != __constant_htons(ETH_P_8021Q))
	{	
		extern int rtl_vlan_passthrough_enable;
		if (rtl_vlan_passthrough_enable){
			if(FAILED == rtl_process_vlan_passthrough_tx(&skb))
				goto stop_proc;
		}

	}
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	do
	{
		extern int wan_mapping_check(struct sk_buff *skb);
		if(FAILED == wan_mapping_check(skb))
		{
			goto free_and_stop;
		}
	}
	while(0);
#endif


#if defined(CONFIG_RTL_819X_ECOS)&&defined(CONFIG_RTL_VLAN_SUPPORT)&&defined(CONFIG_RTL_819X_SWCORE)
	if (rtl_vlan_support_enable && ntohs(*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2))) != 0x888e) {
		int get_pri = 0;
		if (QOS_ENABLE) {
#ifdef CLIENT_MODE
			if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE))
				pstat = get_stainfo(priv, BSSID);
			else
#endif
			{
				if (pstat == NULL && !IS_MCAST(skb->data))
					pstat = get_stainfo(priv, skb->data);
			}

			if (pstat && pstat->QosEnabled)
				get_pri = 1;
		}

		if (!get_pri)
			skb->cb[0] = '\0';

		if (rtl_vlanEgressProcess(skb, priv->dev->name, get_pri) < 0) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: by vlan!\n");
			goto free_and_stop;
		}
	}
	else
		skb->cb[0] = '\0';
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
	//Ingress check for WAN->WLAN(Only NAT).
	/*#ifdef CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG
	if (*(uint16*)skb->linux_vlan_src_tag == __constant_htons(ETH_P_8021Q) &&
		*(uint16*)(skb->data+(ETH_ALEN<<1)) == __constant_htons(ETH_P_8021Q))
	{	
		uint16 tag_vid=0, src_vid=0;
		
		src_vid = ntohs(*(uint16*)(skb->linux_vlan_src_tag+2)) & 0x0fff;
		tag_vid = ntohs(*(uint16*)(skb->data+(ETH_ALEN<<1)+2)) & 0x0fff;

		if(src_vid != tag_vid)
		{	
			priv->ext_stats.tx_drops++;
			goto free_and_stop;
		}
	}
	#endif*/
	
	/*remove vlan tag if it is not need to tagged out*/
	if(linux_vlan_enable){		
		if(*((uint16*)(skb->data+(ETH_ALEN<<1))) == __constant_htons(ETH_P_8021Q)){
			vid = ntohs(*(uint16*)(skb->data+(ETH_ALEN<<1)+2)) & 0x0fff;
			
			#ifndef CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG
			//In source-tag mode, wlan tx doesn't support tagged.
			if(!(vlan_ctl_p->group[vid].tagMemberMask&(1<<dev->vlan_member_map)))
			#endif
			{
				if (skb_cloned(skb)){
					newskb = skb_copy(skb, GFP_ATOMIC);
					if (newskb == NULL) {
 					 	goto free_and_stop;
					}
					dev_kfree_skb_any(skb);
					skb = newskb;
				}
				memmove(skb->data+VLAN_HLEN, skb->data, ETH_ALEN<<1);
				skb_pull(skb,VLAN_HLEN);
			}
		}
	}
#endif

#ifdef CONFIG_PUMA_VLAN_8021Q
	if (priv->pmib->vlan.vlan_enable) {
		if(*((UINT16*)(skb->data+(ETH_ALEN<<1))) == __constant_htons(ETH_P_8021Q)){
			if (skb_cloned(skb)){
				newskb = skb_copy(skb, GFP_ATOMIC);
				if (newskb == NULL) {
			 		priv->ext_stats.tx_drops++;
			 		DEBUG_ERR("TX DROP: Can't copy the skb!\n");
					goto free_and_stop;
				}
				dev_kfree_skb_any(skb);
				skb = newskb;
			}
			memmove(skb->data+VLAN_HLEN, skb->data, ETH_ALEN<<1);
			skb_pull(skb,VLAN_HLEN);
		}
	}
#endif

#ifdef WDS
	if (dev->base_addr) {
		// normal packets
		if (priv->pmib->dot11WdsInfo.wdsPure) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: Sent normal pkt in Pure WDS mode!\n");
			goto free_and_stop;
		}
	}
	else {
		// WDS process
		if (rtl8192cd_tx_wdsDevProc(priv, skb, &dev, &wdsDev, txcfg) == TX_PROCEDURE_CTRL_STOP) {
			goto stop_proc;
		}
	}
#endif // WDS

	if (priv->pmib->miscEntry.func_off || priv->pmib->miscEntry.raku_only) {
		goto free_and_stop;
	}

	// drop packets if link status is null
#ifdef WDS
	if (!wdsDev)
#endif
	{
        temp = get_assoc_sta_num(priv, 1);        
		if (temp == 0) {
#ifdef RTK_BR_EXT
			if(!((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed)){
#endif
				priv->ext_stats.tx_drops++;
				DEBUG_WARN("TX DROP: Non asoc tx request!\n");
				goto free_and_stop;
#ifdef RTK_BR_EXT
			}
#endif
		}
	}

#ifdef CLIENT_MODE
	// nat2.5 translation, mac clone, dhcp broadcast flag add.
	if (OPMODE & (WIFI_STATION_STATE|WIFI_ADHOC_STATE)) {

#ifdef A4_STA
        pstat = get_stainfo(priv, BSSID);
        if(pstat && !(pstat->state & WIFI_A4_STA))
#endif
        {            
#ifdef RTK_BR_EXT
			if (!priv->pmib->ethBrExtInfo.nat25_disable &&
					!(skb->data[0] & 1) &&
#ifdef MULTI_MAC_CLONE
					(ACTIVE_ID == 0) &&
#endif		
#if defined(__KERNEL__) || defined(__OSK__)
					GET_BR_PORT(priv->dev) &&
#endif
					memcmp(skb->data+MACADDRLEN, priv->br_mac, MACADDRLEN) &&
//					*((unsigned short *)(skb->data+MACADDRLEN*2)) != __constant_htons(ETH_P_8021Q) &&
					*((unsigned short *)(skb->data+MACADDRLEN*2)) == __constant_htons(ETH_P_IP) &&
					!memcmp(priv->scdb_mac, skb->data+MACADDRLEN, MACADDRLEN) && priv->scdb_entry) {
                if(priv->nat25_filter) {
                    if(nat25_filter(priv, skb)== 1) {
                        priv->ext_stats.tx_drops++;
                        DEBUG_ERR("TX DROP: nat25 filter out!\n");
                        goto free_and_stop;
                    }
                }

				memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
				priv->scdb_entry->ageing_timer = jiffies;
			}
			else
#endif		
			{
#ifdef RTK_BR_EXT
			if(!(priv->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed) &&
				((OPMODE & WIFI_STATION_STATE) && memcmp(skb->data+ETH_ALEN, GET_MY_HWADDR, ETH_ALEN))) {	// ignore tx from root br to vap(client mode) if vap is not connected to remote ap successfully
#else
			if((OPMODE & WIFI_STATION_STATE) && memcmp(skb->data+ETH_ALEN, GET_MY_HWADDR, ETH_ALEN)) {	// ignore tx from root br to vap(client mode) if vap is not connected to remote ap successfully
#endif				
				struct stat_info *pstat = get_stainfo(priv,BSSID);
				if (!((NULL != pstat) && (pstat->state & (WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE))
					   && (((0 != pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen) && (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_))
						   || (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _NO_PRIVACY_)))) {
					goto free_and_stop;
				}
			}
				SMP_UNLOCK_XMIT(flags);
				if (rtl8192cd_tx_clientMode(priv, &skb) == TX_PROCEDURE_CTRL_STOP) {
					SMP_LOCK_XMIT(flags);
					goto stop_proc;
				}
				SMP_LOCK_XMIT(flags);
			}
    		
#ifdef RTK_BR_EXT //8812_client
#ifdef MULTI_MAC_CLONE
			if (ACTIVE_ID >= 0)
				mclone_dhcp_caddr(priv, skb);		
			if (ACTIVE_ID == 0)				//not a mac clone sta
#endif
			dhcp_flag_bcast(priv, skb);
#endif
        }

	}
#endif // CLIENT_MODE

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if ((OPMODE & WIFI_AP_STATE) && !wdsDev) {
			if ((*(unsigned int *)&(skb->cb[8]) == 0x86518190) &&						// come from wlan interface
				(*(unsigned int *)&(skb->cb[12]) != priv->pmib->miscEntry.groupID))		// check group ID
			{
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: not the same group!\n");
				goto free_and_stop;
			}
		}
	}
#endif

#ifdef USE_TXQUEUE
	if (BUFQ_ENABLE && priv->pshare->iot_mode_enable) {
		int pri = 0, qnum = BE_QUEUE, qlen = 0, qidx = 0;
		pri = get_skb_priority(priv, skb, NULL);
#ifdef RTL_MANUAL_EDCA
        q_num = PRI_TO_QNUM(priv, pri);
#else
		PRI_TO_QNUM(pri, qnum, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
		
		if (!priv->pshare->txq_isr)
		{
			if (txq_len(&priv->pshare->txq_list[RSVQ(qnum)]) > 0)
			{
				if (!check_rsv_txbufq(priv, qnum)) {
					append_skb_to_txq_tail(&priv->pshare->txq_list[RSVQ(qnum)], priv, skb, dev, &priv->pshare->txq_pool);
					priv->use_txq_cnt[RSVQ(qnum)]++;
					priv->pshare->txq_check = 1;
					goto stop_proc;
				} else {
					DEBUG_ERR("TX DROP: exceed the tx queue!\n");
					priv->ext_stats.tx_drops++;
					goto free_and_stop;
				}
			}
		}
	}
#endif

#ifdef WDS
	if (wdsDev)
		da = priv->pmib->dot11WdsInfo.entry[txcfg->wdsIdx].macAddr;
	else
#endif
	{

#ifdef MCAST2UI_REFINE
        da = &skb->cb[10];
#else
		da = skb->data;
#endif
	}

#ifdef CLIENT_MODE
	if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE))
		pstat = get_stainfo(priv, BSSID);
	else
#endif
	{
		pstat = get_stainfo(priv, da);
#ifdef A4_STA
		if (pstat == NULL && (OPMODE & WIFI_AP_STATE)) {
			if(priv->pshare->rf_ft_var.a4_enable == 2) {
				if(IS_MCAST(da)) {
					/*duplicate the packet and use 4-address to send to every a4 client, 
						and send one 3-address packet  to other a3 clients*/
					if(a4_tx_mcast_to_unicast(priv, skb) == 1) {
						goto free_and_stop;
					}
				} else {
					pstat = a4_sta_lookup(priv, da);	
					if(pstat == NULL) {
						/*Unknown unicast, transmit to every A4 sta*/
						a4_tx_unknown_unicast(priv, skb);                
						goto free_and_stop;
					}
				}
			} else if(priv->pshare->rf_ft_var.a4_enable == 1){
				if(!IS_MCAST(da)) {/*just send one 3-address multi/broadcast  for all clients*/
					pstat = a4_sta_lookup(priv, da);
				}
			}
        	}
#endif		

#if defined(TV_MODE) && defined(SUPPORT_TX_MCAST2UNI) /*drop (joint multicast && (ipv4 || ipv6)) packet*/
        if(priv->pmib->miscEntry.forward_streaming == 0 && (priv->tv_mode_status & BIT0) == 0 && IS_MCAST(da)) {
            unsigned short ethtype = *((unsigned short *)(skb->data+MACADDRLEN*2));
            if(ethtype == __constant_htons(ETH_P_IP) || ethtype == __constant_htons(ETH_P_IPV6)) {
                if(tv_mode_igmp_group_check(priv, skb))
                    goto free_and_stop;
            }
        }
#endif


        #if defined(RTK_ATM) || defined(SW_TX_QUEUE)
        if (pstat)
        {
            #ifdef RTK_ATM
            if(priv->pshare->rf_ft_var.atm_en)
                swq_out = priv->pshare->atm_swq_en | pstat->atm_swq.swq_empty[BK_QUEUE] | 
                                     pstat->atm_swq.swq_empty[BE_QUEUE] | 
                                     pstat->atm_swq.swq_empty[VO_QUEUE] | 
                                     pstat->atm_swq.swq_empty[VI_QUEUE];   
            else
            #endif
            {
                #ifdef SW_TX_QUEUE
                swq_out = priv->pshare->swq_en | pstat->swq.swq_empty[BK_QUEUE] | 
                                         pstat->swq.swq_empty[BE_QUEUE] | 
                                         pstat->swq.swq_empty[VO_QUEUE] | 
                                         pstat->swq.swq_empty[VI_QUEUE];
                #endif
            }

        }
        #endif //defined(RTK_ATM) || defined(SW_TX_QUEUE)

	}
#ifdef DETECT_STA_EXISTANCE
	if(pstat && pstat->leave)	{
		priv->ext_stats.tx_drops++;
		DEBUG_WARN("TX DROP: sta may leave! %02x%02x%02x%02x%02x%02x\n", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		goto free_and_stop;
	}
#endif

#ifdef TX_SCATTER
#ifdef CONFIG_IEEE80211W
		if (skb->list_num > 0 && (UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE), 0) ||
#else
		if (skb->list_num > 0 && (UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE)) ||
#endif
		(pstat && (get_sta_encrypt_algthm(priv, pstat) == _TKIP_PRIVACY_)))) {
		newskb = copy_skb(skb);
		if (newskb == NULL) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: Can't copy the skb for list buffer!\n");
			goto free_and_stop;
		}
		dev_kfree_skb_any(skb);
		skb = newskb;
	}
#endif

#if defined(CONFIG_VERIWAVE_MU_CHECK) && (MU_BEAMFORMING_SUPPORT == 1)	
	if(priv->assoc_num > 2 && priv->pshare->rf_ft_var.isExistVeriwaveSTA && pstat && !pstat->inTXBFEntry&& skb->len > 1000) {
		goto free_and_stop;
	}
#endif

#if defined(SW_TX_QUEUE) || defined(RTK_ATM) 
	if (swq_out == 0
        #ifdef GBWC
		|| priv->GBWC_consuming_Q
        #endif
		#ifdef SBWC
		|| (pstat && pstat->SBWC_consuming_q)
		#endif		
		|| (pstat && (get_sta_encrypt_algthm(priv, pstat)) && !(pstat->dot11KeyMapping.keyInCam)) //eric-11c
		) {//direct send and no use swq
        goto xmit_out;
    } 
    #ifdef RTK_ATM
    else if (priv->pshare->rf_ft_var.atm_en) {
        if(priv->pshare->atm_swq_en)
            return rtl8192cd_atm_swq_enqueue(priv, skb, pstat);
        else {
            for(temp = BK_QUEUE; temp <= VO_QUEUE; temp++)
            {
                if(pstat->atm_swq.swq_empty[temp])                                             
                    atm_skb_dequeue(priv, pstat, 0xff, temp);//send out all packet in atm_swq
            }
            goto xmit_out;
        }
    } 
    #endif
    #ifdef SW_TX_QUEUE
    else if (priv->pshare->swq_en) {//swq_en=1	
        priv->ext_stats.swq_enque_pkt++;
        return rtl8192cd_swq_enqueue(priv, skb, pstat);
    } 
    else {//swq_en=0 and swq still have packet queuing
        for(temp = BK_QUEUE; temp <= VO_QUEUE; temp++)
        {
            if(pstat->swq.swq_empty[temp]) {
                if (pstat->swq.swq_timer_id[temp])
                    rtl8192cd_swq_deltimer(priv, pstat, temp);
                rtl8192cd_swq_dequeue(priv, pstat, temp, 0, 1);
            }
        }
        goto xmit_out;
    }
    #endif	

xmit_out:
#endif //defined(SW_TX_QUEUE) || defined(RTK_ATM) 

    #ifdef CONFIG_RTK_MESH
    return __rtl8192cd_start_xmit_out(skb, pstat, NULL);
    #else
    return __rtl8192cd_start_xmit_out(skb, pstat);
    #endif	

free_and_stop:          /* Free current packet and stop TX process */
	
	rtl_kfree_skb(priv, skb, _SKB_TX_);

stop_proc:                      /* Stop process and assume the TX-ed packet is already "processed" (freed or TXed) in previous code. */

    return 0;
}

#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X

int __rtl8192cd_start_xmit_out(struct sk_buff *skb, struct stat_info *pstat
#ifdef CONFIG_RTK_MESH
, struct tx_insn *ptxcfg
#endif
) //struct net_device *dev)
{
    struct rtl8192cd_priv *priv;
    //struct stat_info        *pstat=NULL;
    struct net_device *dev = skb->dev;
    struct sk_buff *newskb = NULL;
    struct net_device *wdsDev = NULL;
    #ifdef CONFIG_RTK_MESH
    int block_mesh = 0;
    #endif
#if defined(CONFIG_RTL8672) || defined(TX_SHORTCUT)
    int k;
#endif
#ifdef TXSC_HDR
	int txsc_lv2 = 0;
#endif	
#if defined(TX_SHORTCUT)
	struct tx_sc_entry *ptxsc_entry = NULL;
	struct tx_insn		*txsc_txcfg;
#endif
    struct tx_insn tx_insn;
	int needTxBD=4;
    DECLARE_TXCFG(txcfg, tx_insn);

	priv = GET_DEV_PRIV(dev);

#if defined(CONFIG_RTK_MESH)
	if(dev == priv->mesh_dev) {
		if(ptxcfg->is_11s == 1) {
			memcpy(txcfg, ptxcfg, sizeof(*ptxcfg));
            priv = txcfg->priv;
        }    
		else { /* if ptxcfg->is_11s is RELAY_11S */
            txcfg->is_11s = RELAY_11S;                            
            memcpy(txcfg->nhop_11s, pstat->hwaddr, MACADDRLEN); 
            memcpy(&txcfg->mesh_header, skb->data + WLAN_ETHHDR_LEN, sizeof(struct lls_mesh_header));       
            priv = ptxcfg->priv;
       }    
    }    
#endif

#ifdef CONFIG_RTL8672
#ifdef SUPPORT_TX_MCAST2UNI
	if (skb->cb[16] == TX_NO_MUL2UNI)
		txcfg->isMC2UC = 1;
	else
		txcfg->isMC2UC = 0;
#endif
#endif

#ifdef WDS
	if (dev->base_addr) {
		// normal packets
		if (priv->pmib->dot11WdsInfo.wdsPure) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: Sent normal pkt in Pure WDS mode!\n");
			goto free_and_stop;
		}
	}
	else {
		// WDS process
		if (rtl8192cd_tx_wdsDevProc(priv, skb, &dev, &wdsDev, txcfg) == TX_PROCEDURE_CTRL_STOP) {
			goto stop_proc;
		}
	}
#endif // WDS

	{
		if (skb_cloned(skb)
#ifdef MCAST2UI_REFINE
                        && !memcmp(skb->data, &skb->cb[10], 6)
#endif
			)
		{
			newskb = skb_copy(skb, GFP_ATOMIC);
			if (newskb == NULL) {
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: Can't copy the skb!\n");
				goto free_and_stop;
			}
			dev_kfree_skb_any(skb);
			skb = newskb;
		}
	}

#ifdef SUPPORT_SNMP_MIB
	if (IS_MCAST(skb->data))
		SNMP_MIB_INC(dot11MulticastTransmittedFrameCount, 1);
#endif

	if ((OPMODE & WIFI_AP_STATE)
#ifdef WDS
			&& (!wdsDev)
#endif
#ifdef CONFIG_RTK_MESH
			&& (dev != priv->mesh_dev)
#endif
		) {
#ifdef MCAST2UI_REFINE
                if ((pstat && ((pstat->state & (WIFI_SLEEP_STATE | WIFI_ASOC_STATE)) ==
                                (WIFI_SLEEP_STATE | WIFI_ASOC_STATE))) ||
                        (((IS_MCAST(&skb->cb[10]) && (priv->sleep_list.next != &priv->sleep_list)) ||
                        ((priv->pshare->rf_ft_var.bcast_to_dzq) && ((unsigned char )(skb->cb[10]) == 0xff))) &&
                                (!priv->release_mcast)))
#else
		if ((pstat && ((pstat->state & (WIFI_SLEEP_STATE | WIFI_ASOC_STATE)) ==
				(WIFI_SLEEP_STATE | WIFI_ASOC_STATE))) ||
			(((IS_MCAST(skb->data) && (priv->sleep_list.next != &priv->sleep_list)) ||
			(priv->pshare->rf_ft_var.bcast_to_dzq && (*(skb->data) == 0xff))) &&
				(!priv->release_mcast)))
#endif
		{

			if (dz_queue(priv, pstat, skb) == TRUE)
			{
				DEBUG_INFO("queue up skb due to sleep mode\n");
				goto stop_proc;
			}
			else {
				if (pstat) {
					DEBUG_WARN("ucst sleep queue full!!\n");
				}
				else {
					DEBUG_WARN("mcst sleep queue full!!\n");
				}
				goto free_and_stop;
			}
		}
	}

	if (pstat)
	{
		#ifdef GBWC
		if (priv->pmib->gbwcEntry.GBWCMode)
			if (rtl8192cd_tx_gbwc(priv, pstat, skb) == TX_PROCEDURE_CTRL_STOP) 
				goto stop_proc;
		#endif

		#ifdef SBWC
		if (rtl8192cd_tx_sbwc(priv, pstat, skb) == TX_PROCEDURE_CTRL_STOP) 
			goto stop_proc;
		#endif
	}

#ifdef RTK_STA_BWC
	if (priv->pshare->rf_ft_var.sta_bwc_en && pstat && pstat->sta_bwcthrd_tx>0) {
		if (rtl8192cd_tx_sta_bwc(priv, pstat, skb) == TX_PROCEDURE_CTRL_STOP) {
			goto stop_proc;
		}
	}
#endif


#ifdef MCR_WIRELESS_EXTEND	
	if ((IS_UDP_PROTO(skb->data) && pstat) && (priv->pshare->rf_ft_var.dropUDP))
	{
		int q_num=0, pri = get_skb_priority(priv, skb, pstat);
#ifdef RTL_MANUAL_EDCA
		q_num = PRI_TO_QNUM(priv, pri);
#else
		PRI_TO_QNUM(pri, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif

		if (GET_CHIP_VER(priv)==VERSION_8814A) {
				if (priv->pshare->phw->VO_pkt_count && (priv->pshare->phw->VI_pkt_count || priv->pshare->phw->BE_pkt_count || priv->pshare->phw->BK_pkt_count)) {
					if ((q_num == VI_QUEUE) && (priv->pshare->phw->VI_droppkt_count < (priv->pshare->phw->VO_droppkt_count<<1))) {
						priv->pshare->phw->VI_droppkt_count++;
						goto free_and_stop; 	
					}
					else if ((q_num == BK_QUEUE) && (priv->pshare->phw->BK_droppkt_count < (priv->pshare->phw->VO_droppkt_count<<2))) {
						priv->pshare->phw->BK_droppkt_count++;
						goto free_and_stop;		
					}
					else if ((q_num == BE_QUEUE) && (priv->pshare->phw->BE_droppkt_count < (priv->pshare->phw->VO_droppkt_count<<2))) {
						priv->pshare->phw->BE_droppkt_count++;
						goto free_and_stop;
					}
				} else if (priv->pshare->phw->VI_pkt_count && (priv->pshare->phw->BE_pkt_count || priv->pshare->phw->BK_pkt_count)) {
					if ((q_num == BK_QUEUE) && (priv->pshare->phw->BK_droppkt_count < (priv->pshare->phw->VI_droppkt_count<<2))) {
						priv->pshare->phw->BK_droppkt_count++;
						goto free_and_stop;		
					}
					else if ((q_num == BE_QUEUE) && (priv->pshare->phw->BE_droppkt_count < (priv->pshare->phw->VI_droppkt_count<<1))) {
						priv->pshare->phw->BE_droppkt_count++;
						goto free_and_stop;
					}
				}

		}else if (GET_CHIP_VER(priv)==VERSION_8192E){

				if (priv->pshare->phw->VO_pkt_count && (priv->pshare->phw->VI_pkt_count || priv->pshare->phw->BE_pkt_count || priv->pshare->phw->BK_pkt_count)) {
					if ((q_num == VI_QUEUE) && (priv->pshare->phw->VI_droppkt_count < (priv->pshare->phw->VO_droppkt_count*3/2))) {
						priv->pshare->phw->VI_droppkt_count++;
						goto free_and_stop; 	
					}
					else if ((q_num == BK_QUEUE) && (priv->pshare->phw->BK_droppkt_count < (priv->pshare->phw->VO_droppkt_count<<1))) {
						priv->pshare->phw->BK_droppkt_count++;
						goto free_and_stop;		
					}
					else if ((q_num == BE_QUEUE) && (priv->pshare->phw->BE_droppkt_count < (priv->pshare->phw->VO_droppkt_count<<1))) {
						priv->pshare->phw->BE_droppkt_count++;
						goto free_and_stop;
					}
				} else if (priv->pshare->phw->VI_pkt_count && (priv->pshare->phw->BE_pkt_count || priv->pshare->phw->BK_pkt_count)) {
					if ((q_num == BK_QUEUE) && (priv->pshare->phw->BK_droppkt_count < (priv->pshare->phw->VI_droppkt_count<<1))) {
						priv->pshare->phw->BK_droppkt_count++;
						goto free_and_stop;		
					}
					else if ((q_num == BE_QUEUE) && (priv->pshare->phw->BE_droppkt_count < (priv->pshare->phw->VI_droppkt_count<<1))) {
						priv->pshare->phw->BE_droppkt_count++;
						goto free_and_stop;
					}
				}
			


			}

	}
#endif


#ifdef SUPPORT_TX_AMSDU
	#if defined(CONFIG_RTK_MESH) && !defined(MESH_AMSDU)
	if(dev == priv->mesh_dev){
		block_mesh = 1;
    }
	#endif
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
	if(skb->cb[47]){
//		printk("%s %d cb[47] %X\n", __func__, __LINE__, skb->cb[47]);
		// from amsdu_xmit, don't go to amsdu_check again
		txcfg->q_num = skb->cb[47] & 15;
		if(skb->cb[47]&BIT(FG_AGGRE_MSDU_FIRST))
			txcfg->aggre_en = FG_AGGRE_MSDU_FIRST;
		else if(skb->cb[47]&BIT(FG_AGGRE_MSDU_MIDDLE))
			txcfg->aggre_en = FG_AGGRE_MSDU_MIDDLE;
		else if(skb->cb[47]&BIT(FG_AGGRE_MSDU_LAST))
			txcfg->aggre_en = FG_AGGRE_MSDU_LAST;
	}else 
#endif
	if (IS_SUPPORT_TX_AMSDU(priv) 
#if defined(CONFIG_RTK_MESH)
		&& !block_mesh
#endif
		&& pstat && (pstat->aggre_mthd & AGGRE_MTHD_MSDU) && (pstat->amsdu_level > 0)
#ifdef SUPPORT_TX_MCAST2UNI
		&& (priv->pshare->rf_ft_var.mc2u_disable || (skb->cb[2] != (char)0xff))
#endif
		) {
		int ret = amsdu_check(priv, skb, pstat, txcfg);

		if (ret == RET_AGGRE_ENQUE)
			goto stop_proc;

		if (ret == RET_AGGRE_DESC_FULL)
			goto free_and_stop;
	}
#endif


#ifdef SW_TX_QUEUE

#ifdef CONFIG_RTK_MESH
    if(dev != priv->mesh_dev) /*mesh do not support sw tx queue yet, skip this*/
#endif        
    {

    	if ((priv->assoc_num > 1) && pstat)
        {
#ifdef MCAST2UI_REFINE
            if (memcmp(&skb->cb[10], priv->pshare->record_mac, 6))
#else
            if (memcmp(skb->data, priv->pshare->record_mac, 6))
#endif
        	{
            	priv->pshare->swq_txmac_chg++;
#ifdef MCAST2UI_REFINE
                memcpy(priv->pshare->record_mac, &skb->cb[10], 6);
#else
                memcpy(priv->pshare->record_mac, skb->data, 6);
#endif
           	}
            else
            {
            	int pri, q_num;

#ifdef CONFIG_RTK_MESH
                pri = get_skb_priority3(priv, skb, txcfg->is_11s, pstat);
#else
                pri = get_skb_priority(priv, skb, pstat);
#endif

#ifdef RTL_MANUAL_EDCA
                q_num = PRI_TO_QNUM(priv, pri);
#else
                PRI_TO_QNUM(pri, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
                if (priv->pshare->record_qnum != q_num)
                {
                	priv->pshare->swq_txmac_chg++;
                    priv->pshare->record_qnum = q_num;
                }
            }
        }

#ifdef SW_TX_QUEUE_SMALL_PACKET_CHECK
        // add check for small udp packet(88B) test with veriwave tool
        if ((priv->pshare->swq_en == 0) && (priv->assoc_num > 1) && (AMPDU_ENABLE))
        {
            int thd_value;

            if ((priv->swq_boost_delay > 0) && (priv->swq_boost_delay < 10))
                thd_value = priv->pshare->rf_ft_var.swq_en_highthd / 10;
            else
                thd_value = priv->pshare->rf_ft_var.swq_en_highthd;

            if (priv->pshare->swq_txmac_chg >= thd_value);
            {
                //printk("%s %d : thd_value %d\n", __FUNCTION__, __LINE__, thd_value);

                if (priv->pshare->txop_enlarge == 0)
                    priv->pshare->txop_enlarge = 2;

                priv->pshare->swq_en = 1;
                priv->pshare->swqen_keeptime = priv->up_time;

                struct stat_info	*pstat_swq;
                struct list_head	*phead, *plist;
                struct stat_info	*pstat_highest = NULL;

                phead = &priv->asoc_list;
                plist = phead->next;

                while (plist != phead)
                {
                    pstat_swq = list_entry(plist, struct stat_info, asoc_list);

                    int i;
                    for (i = BK_QUEUE; i <= VO_QUEUE; i++) 
                    {				
                        pstat_swq->swq.q_aggnum[i] = priv->pshare->rf_ft_var.swq_aggnum; // MAX_BACKOFF_CNT;
                    }

                    if (plist == plist->next)
                        break;

                    plist = plist->next;
                }
            }
        }
#endif

    }
#endif //SW_TX_QUEUE

#ifdef TX_SHORTCUT
	if (!priv->pmib->dot11OperationEntry.disable_txsc && pstat
			&& ((k = get_tx_sc_index(priv, pstat, skb->data)) >= 0))
	{
		ptxsc_entry = &pstat->tx_sc_ent[k];
		txsc_txcfg = &(ptxsc_entry->txcfg);
		if (
#if defined(NOT_RTK_BSP) && !defined(__ECOS)
			// go slow path to avoid TX drop when no enough tail room to put TKIP MIC.
			((_TKIP_PRIVACY_ != txsc_txcfg->privacy) || ((skb->tail + 8) <= skb->end)) &&
#endif
			((txsc_txcfg->privacy == 0) ||
#ifdef CONFIG_RTL_WAPI_SUPPORT
			  (txsc_txcfg->privacy == _WAPI_SMS4_) ||
#endif
#ifdef CONFIG_IEEE80211W
			!UseSwCrypto(priv, pstat, FALSE, 0) 
#else
			!UseSwCrypto(priv, pstat, FALSE)
#endif
			) &&
			(txsc_txcfg->fr_len > 0) &&
#ifdef CONFIG_RTK_MESH
			(get_skb_priority3(priv, skb, txcfg->is_11s, pstat) == ptxsc_entry->pktpri) &&
#else		
			(get_skb_priority(priv, skb, pstat) == ptxsc_entry->pktpri) &&
#endif
			(FRAGTHRSLD > 1500) 
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
			&& ((txsc_txcfg->aggre_en == FG_AGGRE_MSDU_FIRST && txcfg->aggre_en >=FG_AGGRE_MSDU_FIRST) || (txsc_txcfg->aggre_en != FG_AGGRE_MSDU_FIRST && txcfg->aggre_en < FG_AGGRE_MSDU_FIRST))
#endif
			)
		{
				int						*tx_head=NULL, *tx_tail=NULL, q_num;
				struct rtl8192cd_hw		*phw = GET_HW(priv);
				unsigned int uiCurr_tx_desc = CURRENT_NUM_TX_DESC;
#ifdef TX_SCATTER
				int reuse_txdesc = 0;
#endif
#ifdef CONFIG_WLAN_HAL
				PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
				unsigned int                halQnum;
#endif
			int bUnAvail = FALSE;

				q_num = txsc_txcfg->q_num;
#ifdef OSK_LOW_TX_DESC
				uiCurr_tx_desc = ((q_num==BE_QUEUE || q_num==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC);
#endif

#ifdef CONFIG_WLAN_HAL
				if (IS_HAL_CHIP(priv)) {
	            	ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
					halQnum=GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, q_num);
				} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
				{//not HAL
				    tx_head = get_txhead_addr(phw, q_num);
		 			tx_tail = get_txtail_addr(phw, q_num);
				}
				// check if we need active tx tasklet
	//#ifdef __KERNEL__

	            if(
#ifdef CONFIG_WLAN_HAL
					IS_HAL_CHIP(priv) ? (COMPAREAVAILABLETXBD(ptx_dma,halQnum,(uiCurr_tx_desc/2))):
#endif // CONFIG_WLAN_HAL
					(CIRC_SPACE_RTK(*tx_head, *tx_tail, uiCurr_tx_desc) < uiCurr_tx_desc/2)
	            )
	            {
#if (NUM_TX_DESC > 64)
					if(
#ifdef CONFIG_WLAN_HAL
					IS_HAL_CHIP(priv) ? (COMPAREAVAILABLETXBD(ptx_dma,halQnum,32)) : 
#endif // CONFIG_WLAN_HAL
					(CIRC_SPACE_RTK(*tx_head, *tx_tail, uiCurr_tx_desc) < 10)
	
					)
					{
						rtl8192cd_tx_queueDsr(priv, q_num);
					} else 
#endif // #if (NUM_TX_DESC > 64)
					{
						if (!priv->pshare->has_triggered_tx_tasklet) {
#ifdef __KERNEL__
							tasklet_schedule(&priv->pshare->tx_tasklet);
#endif
							priv->pshare->has_triggered_tx_tasklet = 1;
						}
					}
				}
	//#endif

				if(priv->pshare->rf_ft_var.lmtdesc && IS_UDP_PROTO(skb->data)) {
					if (GET_CHIP_VER(priv)==VERSION_8814A) {				
						if (priv->pshare->phw->VO_pkt_count >= 50) {
							if(q_num == VI_QUEUE || q_num == BE_QUEUE || q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt1;		//768		
							if(q_num == BE_QUEUE || q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt2;		//192
							if(q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt2;		// 192
						}
						else if (priv->pshare->phw->VI_pkt_count >= 50) {
							if(q_num == BE_QUEUE || q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt1;		
							if(q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt2;		
						} else if (priv->pshare->phw->BE_pkt_count >= 50) {
							if(q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt1;		
						}
					} else {
						if (priv->pshare->phw->VO_pkt_count >= 50) {
							if(q_num == VI_QUEUE || q_num == BE_QUEUE || q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt1;		//768
							if(q_num == BE_QUEUE || q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt2;		//192
#if 1							
							if(q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt2;		// 192
#endif							
						}
						else if (priv->pshare->phw->VI_pkt_count >= 50) {
							if(q_num == BE_QUEUE || q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt1;		
#if 1							
							if(q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt2;		
#endif							
						} else if (priv->pshare->phw->BE_pkt_count >= 50) {
							if(q_num == BK_QUEUE)
								needTxBD += priv->pshare->rf_ft_var.lmt3;
						}
					}
				}	
				// Check if we need to reclaim TX-ring before processing TX
#if (NUM_TX_DESC <= 64)
				if(
#ifdef CONFIG_WLAN_HAL
				IS_HAL_CHIP(priv) ? (COMPAREAVAILABLETXBD(ptx_dma,halQnum,needTxBD+28)) : 
#endif // CONFIG_WLAN_HAL
				(CIRC_SPACE_RTK(*tx_head, *tx_tail, uiCurr_tx_desc) < 10)

				)
				{
					rtl8192cd_tx_queueDsr(priv, q_num);
				}
#endif


#ifdef RESERVE_TXDESC_FOR_EACH_IF
			if ( RSVQ_ENABLE && IS_RSVQ(q_num) ) {
				int num_consumed = 
#ifdef CONFIG_WLAN_HAL
					IS_HAL_CHIP(priv) ? 1 :
#endif
					2;
				bUnAvail = check_rsv_txdesc(priv, q_num, num_consumed);
			} else
#endif
			{
				int num_rsvd = 
#ifdef TX_EARLY_MODE
					( GET_TX_EARLY_MODE && GET_EM_SWQ_ENABLE ) ? 14 :
#endif
					4;


#ifdef CONFIG_WLAN_HAL
				if (IS_HAL_CHIP(priv)) {
					bUnAvail = 
#ifdef TX_EARLY_MODE
						( GET_TX_EARLY_MODE && GET_EM_SWQ_ENABLE ) ?
						compareAvailableTXBD(priv, num_rsvd, q_num, 2) :
#endif
						COMPAREAVAILABLETXBD(ptx_dma, q_num, needTxBD) ;
				} else
#endif
				{
#ifdef TX_SCATTER
					bUnAvail = 					
						((ptxsc_entry->has_desc3 && 6 > CIRC_SPACE_RTK(*tx_head, *tx_tail, uiCurr_tx_desc)) ||
						(!ptxsc_entry->has_desc3 && 4 > CIRC_SPACE_RTK(*tx_head, *tx_tail, uiCurr_tx_desc)));
#else
					bUnAvail = (num_rsvd > CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC));
#endif
				}
			}

			if (bUnAvail) {
#ifdef USE_TXQUEUE
				if (BUFQ_ENABLE && priv->pshare->iot_mode_enable) {
					if (priv->pshare->txq_isr) {
						append_skb_to_txq_head(&priv->pshare->txq_list[RSVQ(q_num)], priv, skb, dev, &priv->pshare->txq_pool);
						priv->pshare->txq_stop = 1;
					} else {
						append_skb_to_txq_tail(&priv->pshare->txq_list[RSVQ(q_num)], priv, skb, dev, &priv->pshare->txq_pool);
					}
					priv->use_txq_cnt[RSVQ(q_num)]++;
					priv->pshare->txq_check = 1;
				}
				else
#endif
				{
#ifdef CONFIG_WLAN_HAL
					if(IS_HAL_CHIP(priv)) {
						DEBUG_ERR("%s:%d: tx drop: %d hw Queue desc not available! head=%d, tail=%d request %d\n", __FUNCTION__, __LINE__,
							q_num, ptx_dma->tx_queue[q_num].host_idx,  ptx_dma->tx_queue[q_num].hw_idx,2);
					}
					else
#endif
						DEBUG_ERR("%d hw Queue desc not available! head=%d, tail=%d request %d\n",q_num,*tx_head,*tx_tail,2);
				
					rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
				}

				goto stop_proc;
			}


#if defined(MESH_TX_SHORTCUT)
				if(txcfg->is_11s) {
	                if(!mesh_txsc_decision(txcfg, &ptxsc_entry->txcfg)) { /*compare addr 5 and 6*/
	                    goto just_skip;
	                }

	                if(memcmp(skb->data, GetAddr3Ptr(&ptxsc_entry->wlanhdr), MACADDRLEN)) {/*compare addr 3*/
	                    goto just_skip;
	                }

	                ptxsc_entry->txcfg.mesh_header.segNum = txcfg->mesh_header.segNum;
	                ptxsc_entry->txcfg.mesh_header.TTL = txcfg->mesh_header.TTL;
	                ptxsc_entry->txcfg.is_11s = txcfg->is_11s;
				}
#endif

#ifdef A4_STA
	            if(pstat->state & WIFI_A4_STA) {
	                if(memcmp(skb->data, ptxsc_entry->ethhdr.daddr, MACADDRLEN)) {/*compare addr 3*/
	                    goto just_skip;
	                }
	            }
#endif

#ifdef SUPPORT_TX_AMSDU_SHORTCUT
			if((txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE)||(txcfg->aggre_en == FG_AGGRE_MSDU_LAST))
			{
				//it doesn't need all backup txcfg for middle and last of AMSDU, set which it needed only
				txcfg->privacy = txsc_txcfg->privacy;
				txcfg->pstat = pstat;
			}
			else
#endif
			{	
#if !defined(TXSC_CFG)
			memcpy(txcfg, txsc_txcfg, sizeof(struct tx_insn));
#else
			txcfg = txsc_txcfg;
			txcfg->one_txdesc = 0;
#endif
			}
			

#ifdef CONFIG_RTL8672
#ifdef SUPPORT_TX_MCAST2UNI
				if (skb->cb[16] == TX_NO_MUL2UNI)
					txcfg->isMC2UC = 1;
				else
					txcfg->isMC2UC = 0;
#endif
#endif

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
				if (GET_CHIP_VER(priv)==VERSION_8814A) {
					txcfg->phdr   	= skb->data; 	// 802.3 hdr
					//txcfg->hdr_len	= WLAN_ETHHDR_LEN;
					txcfg->pframe 	= skb;  		// skb->data:  802.3 hdr + payload
					txcfg->fr_len 	= skb->len; 	// len(802.3 hdr + payload)
					// TODO: currently , we don't care encryption.... it's only support AES in HW
				} else
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV

				{
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
				if ((txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE) || (txcfg->aggre_en == FG_AGGRE_MSDU_LAST)){
					txcfg->fr_type = _SKB_FRAME_TYPE_;
					txcfg->phdr = NULL;
				}
				else
#endif
				{
#ifndef TXSC_HDR
				txcfg->phdr = (UINT8 *)get_wlanllchdr_from_poll(priv);
				if (txcfg->phdr == NULL) {
					DEBUG_ERR("Can't alloc wlan header!\n");
					rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
					goto stop_proc;
				}
				memcpy(txcfg->phdr, (const void *)&ptxsc_entry->wlanhdr, sizeof(struct wlanllc_hdr));
#endif
				}
				txcfg->pframe = skb;
#ifdef WDS
				/*Correct the aggre_mthd*/
				if(pstat->state & WIFI_WDS)
					if(txcfg->aggre_en != pstat->aggre_mthd) {
						/*Invalid the tx_sc entry*/
						ptxsc_entry->txcfg.fr_len=0;
						txcfg->aggre_en = pstat->aggre_mthd;
					}	
#endif

#ifdef SUPPORT_TX_AMSDU_SHORTCUT
					if(txcfg->aggre_en >= FG_AGGRE_MSDU_FIRST){
						unsigned short usLen;
						unsigned short needed_tailroom = 4;	//4 //4 bytes  needed for padding
						if (txcfg->privacy == _TKIP_PRIVACY_)
							needed_tailroom += 8;	// 8 bytes needed for TKIP MIC
						if (skb_headroom(skb) < sizeof(struct llc_snap)||skb_tailroom(skb)< needed_tailroom) {
							struct sk_buff *skb2 = dev_alloc_skb(skb->len + needed_tailroom);
#ifdef MCAST2UI_REFINE
							memcpy(skb2->cb, skb->cb, sizeof(skb->cb));
#endif
							if (skb2 == NULL) {
								printk("%s: %s, dev_alloc_skb() failed!\n", priv->dev->name, __FUNCTION__);
								rtl_kfree_skb(priv, skb, _SKB_TX_);
								if(txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
									release_wlanllchdr_to_poll(priv, txcfg->phdr);
								return 0;
							}
							memcpy(skb_put(skb2, skb->len), skb->data, skb->len);
							dev_kfree_skb_any(skb);
							skb = skb2;
							txcfg->pframe = (void *)skb;
						}
						skb_push(skb, sizeof(struct llc_snap));
						memcpy(skb->data, &ptxsc_entry->amsdullcsnaphdr, sizeof(struct wlan_amsdullcsnaphdr_t));
						usLen = (unsigned short)(skb->len - WLAN_ETHHDR_LEN);
						((struct wlan_amsdullcsnaphdr_t *)skb->data)->amsdulen = htons(usLen);
						if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST || txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE) {
							if ((usLen+14) % 4)
								skb_put(skb, 4-((usLen+14)%4));
						}
						txcfg->fr_len = skb->len;
					}else
#endif
					{

#ifdef TX_SCATTER
				if (skb->list_num > 0)
					txcfg->fr_len = skb->total_len - WLAN_ETHHDR_LEN;
				else
#endif
					txcfg->fr_len = skb->len - WLAN_ETHHDR_LEN;
				skb_pull(skb, WLAN_ETHHDR_LEN);
	            }
	            }

#ifdef SUPPORT_TX_AMSDU_SHORTCUT
			if (txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE || txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {
				if (txcfg->privacy == _TKIP_PRIVACY_) {
					if (rtl8192cd_tx_tkip(priv, skb, pstat, txcfg) == TX_PROCEDURE_CTRL_STOP) {
						goto stop_proc;
					}
				}
				// only payload modfication needed for AMSDU MIDDLE and LAST
				// log tx statistics...
				tx_sum_up(priv, pstat, txcfg);

				SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);

				// for SW LED
				if (txcfg->aggre_en > FG_AGGRE_MSDU_FIRST || GetFrameType(txcfg->phdr) == WIFI_DATA_TYPE) {
					priv->pshare->LED_tx_cnt++;
				} else {
					if (priv->pshare->LED_cnt_mgn_pkt)
						priv->pshare->LED_tx_cnt++;
				}
#ifdef CONFIG_WLAN_HAL
            			if (IS_HAL_CHIP(priv)) {
					rtl88XX_signin_txdesc_amsdu(priv, txcfg);
            			} else 
#endif
				{
					rtl8192cd_signin_txdesc_amsdu(priv, txcfg);
				}
				goto stop_proc;
			} 
#endif

#ifdef TXSC_HDR
#ifdef TX_SCATTER
				if (skb->list_num == 0) {
					reuse_txdesc = 1;
				} else {
					if (ptxsc_entry->has_desc3) {
						if (skb->list_num == 3 && (skb->list_buf[0].len == WLAN_ETHHDR_LEN) &&
							(skb->list_buf[2].len  == (get_desc(ptxsc_entry->hwdesc3.Dword7)&TX_TxBufSizeMask))){
							reuse_txdesc = 1;
						}
					} else {
						if ((skb->list_num == 1 && skb->list_buf[0].len > WLAN_ETHHDR_LEN) ||
							(skb->list_num == 2 && skb->list_buf[0].len == WLAN_ETHHDR_LEN))
							reuse_txdesc = 1;
					}
					if (skb->len == 0 && skb->list_num > 1) {
						skb->list_idx++;
						skb_assign_buf(skb, skb->list_buf[skb->list_idx].buf, skb->list_buf[skb->list_idx].len);
						skb->len = skb->list_buf[skb->list_idx].len;
					}
				}
#endif
	        if(
	                   (
                        #ifdef CONFIG_WLAN_HAL
	                    IS_HAL_CHIP(priv) ? 
                        ((GET_HAL_INTERFACE(priv)->GetShortCutTxBuffSizeHandler(priv, ptxsc_entry->hal_hw_desc)) > 0)            
                        :
                        #endif // CONFIG_WLAN_HAL
	                        ((get_desc(ptxsc_entry->hwdesc1.Dword7)&TX_TxBufSizeMask) > 0) 
	                    )
	                && (
#ifdef CONFIG_RTL_WAPI_SUPPORT
	                    // Note: for sw wapi, txcfg->mic=16; for hw wapi, txcfg->mic=0.
	                     (txcfg->privacy==_WAPI_SMS4_) ? ((skb->len+txcfg->mic)==(get_desc(ptxsc_entry->hwdesc2.Dword7)&TX_TxBufSizeMask)) :
#endif
	                    (
#ifdef CONFIG_WLAN_HAL
	                    IS_HAL_CHIP(priv) ?
	                    (skb->len == (GET_HAL_INTERFACE(priv)->GetShortCutTxBuffSizeHandler(priv, ptxsc_entry->hal_hw_desc))) 
	                    :
#endif // CONFIG_WLAN_HAL
#ifdef TXSC_SKBLEN
	                        ((GET_CHIP_VER(priv)== VERSION_8812E) ? 1 : (skb->len == (get_desc(ptxsc_entry->hwdesc2.Dword7)&TX_TxBufSizeMask)))
#else
	                        (skb->len ==  (get_desc(ptxsc_entry->hwdesc2.Dword7)&TX_TxBufSizeMask))
#endif				
	                    )
	                ) &&
#ifdef TXSC_HDR
	                        ((GET_CHIP_VER(priv)== VERSION_8812E) ? 1 : (txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate)) &&
#else
	                        (txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate) &&
#endif				
	                    (pstat->protection == priv->pmib->dot11ErpInfo.protection) &&
	                    (pstat->ht_protection == priv->ht_protection)
#if defined(WIFI_WMM) && defined(WMM_APSD)
	                    && (!(
#ifdef CLIENT_MODE
	                    (OPMODE & WIFI_AP_STATE) &&
#endif
	                    (APSD_ENABLE) && (pstat->state & WIFI_SLEEP_STATE)))
#endif
#ifdef TX_SCATTER
	                    && reuse_txdesc
#endif
	                    )
						 txsc_lv2 =1;

#ifdef CONFIG_WLAN_HAL                
		        if (IS_HAL_CHIP(priv)) {
#ifdef MERGE_HEADER_PAYLOAD
	                    memcpy((skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv)), txcfg->phdr, (txcfg->hdr_len + txcfg->llc + txcfg->iv));
	                    release_wlanllchdr_to_poll(priv, txcfg->phdr);
	                    txcfg->phdr = skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv);
	                    txcfg->one_txdesc = 1;
#else

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
	                    if (GET_CHIP_VER(priv)==VERSION_8814A) {
	                        txcfg->phdr     = skb->data;    // 802.3 hdr
	                        //txcfg->hdr_len    = WLAN_ETHHDR_LEN;
	                        txcfg->pframe   = skb;          // skb->data:  802.3 hdr + payload
	                        txcfg->fr_len   = skb->len;     // len(802.3 hdr + payload)
	                    } else
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
	                    {
	                    	txcfg->phdr = (UINT8 *)get_wlanllchdr_from_poll(priv);
	                    	if (txcfg->phdr == NULL) {
	                    	    DEBUG_ERR("Can't alloc wlan header!\n");
	                    		rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
	                    	    goto stop_proc;
	                    	}
	                    	memcpy(txcfg->phdr, (const void *)&ptxsc_entry->wlanhdr, sizeof(struct wlanllc_hdr));

	                    }
#endif	                  
		            // for TXBD mechanism, sending a packet always need one txdesc. So, txcfg->one_txdesc is useless.
		        } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
		        {//not HAL
		            if (txsc_lv2 && (skb_headroom(skb) >= (txcfg->hdr_len + txcfg->llc + txcfg->iv
#ifdef TX_EARLY_MODE
		                + (GET_TX_EARLY_MODE ? 8 : 0)
#endif
		                )) &&
		                (txcfg->privacy != _TKIP_PRIVACY_) &&
#if defined(CONFIG_RTL_WAPI_SUPPORT)
		                (txcfg->privacy != _WAPI_SMS4_) &&
#endif
		                ((((unsigned int)skb->data) % 2) == 0)
		                )
		            {
		                txcfg->phdr = skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv);
		                memcpy(txcfg->phdr, (const void *)&ptxsc_entry->wlanhdr, (txcfg->hdr_len + txcfg->llc + txcfg->iv));
		                txcfg->one_txdesc = 1;
		            } else {
						txcfg->phdr = (UINT8 *)get_wlanllchdr_from_poll(priv);
						if (txcfg->phdr == NULL) {
						    DEBUG_ERR("Can't alloc wlan header!\n");
							rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
						    goto stop_proc;
						}
						memcpy(txcfg->phdr, (const void *)&ptxsc_entry->wlanhdr, sizeof(struct wlanllc_hdr));
		            }
		        }
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
#ifdef CONFIG_RTL_HW_WAPI_SUPPORT
#ifdef CONFIG_IEEE80211W
				if((txcfg->privacy==_WAPI_SMS4_)&&(txcfg->llc>0)&&(UseSwCrypto(priv, pstat, FALSE, 0)))
#else
				if((txcfg->privacy==_WAPI_SMS4_)&&(txcfg->llc>0)&&(UseSwCrypto(priv, pstat, FALSE)))
#endif				
#else
				if((txcfg->privacy==_WAPI_SMS4_)&&(txcfg->llc>0))
#endif
				{				
					//To restore not-encrypted llc in wlan hdr
					//because llc in wlan hdr has been sms4encrypted to deliver at SecSWSMS4Encryption()
					eth_2_llc(&ptxsc_entry->ethhdr, (struct llc_snap *)(txcfg->phdr+txcfg->hdr_len + txcfg->iv));
				}
#endif

				if (txcfg->privacy == _TKIP_PRIVACY_) {
					if (rtl8192cd_tx_tkip(priv, skb, pstat, txcfg) == TX_PROCEDURE_CTRL_STOP) {
						goto stop_proc;
					}
				}

#ifdef MESH_TX_SHORTCUT
				if ( (txcfg->is_11s&1) && (GetFrameSubType(txcfg->phdr) == WIFI_11S_MESH))
	                    if( !reuse_meshhdr(priv, txcfg) ) {
						        goto stop_proc;
						}
#endif

				txcfg->tx_rate = get_tx_rate(priv, pstat);
#ifndef TX_LOWESTRATE
				txcfg->lowest_tx_rate = get_lowest_tx_rate(priv, pstat, txcfg->tx_rate);
#endif			
			// log tx statistics...
			tx_sum_up(priv, pstat, txcfg);

			SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
			if(txcfg->aggre_en >= FG_AGGRE_MSDU_FIRST)
				pstat->tx_sc_amsdu_pkts_lv1++;
			else
#endif
			pstat->tx_sc_pkts_lv1++;

#ifdef PCIE_POWER_SAVING
				PCIeWakeUp(priv, (POWER_DOWN_T0));
#endif

				// for SW LED
				priv->pshare->LED_tx_cnt++;

				if (
#ifdef RTK_AC_SUPPORT //for 11ac logo //FOR_VHT5G_PF
				( ((pstat->aggre_mthd == AGGRE_MTHD_MPDU_AMSDU) )
				&& txcfg->aggre_en ) ||
#endif
				((txcfg->aggre_en >= FG_AGGRE_MPDU) && (txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST))
				) {
					if (!pstat->ADDBA_ready[(int)skb->cb[1]]) {
						if ((pstat->ADDBA_req_num[(int)skb->cb[1]] < 5) && !pstat->ADDBA_sent[(int)skb->cb[1]]) {
							pstat->ADDBA_req_num[(int)skb->cb[1]]++;
							SMP_UNLOCK_XMIT(flags);
							issue_ADDBAreq(priv, pstat, (int)skb->cb[1]);
							SMP_LOCK_XMIT(flags);
							pstat->ADDBA_sent[(int)skb->cb[1]]++;
						}
					}
				}
#ifndef TXSC_HDR 
#ifdef TX_SCATTER
				if (skb->list_num == 0) {
					reuse_txdesc = 1;
				} else {
					if (ptxsc_entry->has_desc3) {
						if (skb->list_num == 3 && (skb->list_buf[0].len == WLAN_ETHHDR_LEN) &&
							(skb->list_buf[2].len  == (get_desc(ptxsc_entry->hwdesc3.Dword7)&TX_TxBufSizeMask))){
							reuse_txdesc = 1;
						}
					} else {
						if ((skb->list_num == 1 && skb->list_buf[0].len > WLAN_ETHHDR_LEN) ||
							(skb->list_num == 2 && skb->list_buf[0].len == WLAN_ETHHDR_LEN))
							reuse_txdesc = 1;
					}
					if (skb->len == 0 && skb->list_num > 1) {
						skb->list_idx++;
						skb_assign_buf(skb, skb->list_buf[skb->list_idx].buf, skb->list_buf[skb->list_idx].len);
						skb->len = skb->list_buf[skb->list_idx].len;
					}
				}
#endif	//TX_SCATTER
#endif

#ifdef TXSC_HDR
			if (txsc_lv2)
#elif defined(TXSC_CACHE_FRLEN)
			if (
				(ptxsc_entry->frlen > 0)
                &&
               (
                    #ifdef CONFIG_RTL_WAPI_SUPPORT
                    // Note: for sw wapi, txcfg->mic=16; for hw wapi, txcfg->mic=0.
                    (txcfg->privacy==_WAPI_SMS4_) ? ((skb->len+txcfg->mic)==(get_desc(ptxsc_entry->hwdesc2.Dword7)&TX_TxBufSizeMask)) :
                    #endif   
    				(skb->len == ptxsc_entry->frlen)                
                ) &&
                #if 1
                ((GET_CHIP_VER(priv)== VERSION_8812E) ? 1 : (txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate)) &&
                #else
				(txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate) &&
                #endif	

				(pstat->protection == priv->pmib->dot11ErpInfo.protection) &&
				(pstat->ht_protection == priv->ht_protection)
                #if defined(WIFI_WMM) && defined(WMM_APSD)
				&& (!(
                #ifdef CLIENT_MODE
				(OPMODE & WIFI_AP_STATE) &&
                #endif
				(APSD_ENABLE) && (pstat->state & WIFI_SLEEP_STATE)))
                #endif
                #ifdef TX_SCATTER
				&& reuse_txdesc
                #endif
            )
#else
			// check if we could re-use tx descriptor
			if (
               (
                    #ifdef CONFIG_WLAN_HAL
                    IS_HAL_CHIP(priv) ? 
                    ((GET_HAL_INTERFACE(priv)->GetShortCutTxBuffSizeHandler(priv, ptxsc_entry->hal_hw_desc)) > 0)
                    :
                    #endif // CONFIG_WLAN_HAL
                    ((get_desc(ptxsc_entry->hwdesc1.Dword7)&TX_TxBufSizeMask) > 0) 
                ) &&
                (
                    #ifdef CONFIG_RTL_WAPI_SUPPORT
    				// Note: for sw wapi, txcfg->mic=16; for hw wapi, txcfg->mic=0.
    				 (txcfg->privacy==_WAPI_SMS4_) ? ((skb->len+txcfg->mic)==(get_desc(ptxsc_entry->hwdesc2.Dword7)&TX_TxBufSizeMask)) :
                    #endif
                    (
                        #ifdef CONFIG_WLAN_HAL
                        IS_HAL_CHIP(priv) ?
                        ( skb->len == (GET_HAL_INTERFACE(priv)->GetShortCutTxBuffSizeHandler(priv, ptxsc_entry->hal_hw_desc))) 
                        :
                        #endif // CONFIG_WLAN_HAL
                        #ifdef TXSC_SKBLEN
                        ((GET_CHIP_VER(priv)== VERSION_8812E) ? 1 : (skb->len == (get_desc(ptxsc_entry->hwdesc2.Dword7)&TX_TxBufSizeMask)))
                        #else
                        (skb->len == (get_desc(ptxsc_entry->hwdesc2.Dword7)&TX_TxBufSizeMask))
                        #endif
                    )
                ) &&
                #if 1
                ((GET_CHIP_VER(priv)== VERSION_8812E) ? 1 : (txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate)) &&
                #else
                (txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate) &&
                #endif	
#if 1 //eric-txsc
				(txcfg->q_num == ptxsc_entry->txcfg.q_num)&&
#endif

				(pstat->protection == priv->pmib->dot11ErpInfo.protection) &&
				(pstat->ht_protection == priv->ht_protection)
                #if defined(WIFI_WMM) && defined(WMM_APSD)
				&& (!(
                #ifdef CLIENT_MODE
				(OPMODE & WIFI_AP_STATE) &&
                #endif
				(APSD_ENABLE) && (pstat->state & WIFI_SLEEP_STATE)))
                #endif
                #ifdef TX_SCATTER
				&& reuse_txdesc
                #endif
            )
#endif		
            {
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
		if(txcfg->aggre_en >= FG_AGGRE_MSDU_FIRST)
			pstat->tx_sc_amsdu_pkts_lv2++;
		else
#endif
                pstat->tx_sc_pkts_lv2++;

#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
	                if (GET_CHIP_VER(priv)==VERSION_8814A) {
	                    // Don't fill AES IV by driver in this case.....
	                } else
#endif
	                {
						if (txcfg->privacy) {
							switch (txcfg->privacy) {
							case _WEP_104_PRIVACY_:
							case _WEP_40_PRIVACY_:
								wep_fill_iv(priv, txcfg->phdr, txcfg->hdr_len, ptxsc_entry->sc_keyid);
								break;

							case _TKIP_PRIVACY_:
								tkip_fill_encheader(priv, txcfg->phdr, txcfg->hdr_len, ptxsc_entry->sc_keyid);
								break;

							case _CCMP_PRIVACY_:
								aes_fill_encheader(priv, txcfg->phdr, txcfg->hdr_len, ptxsc_entry->sc_keyid);
								break;
							}
						}
	                }
#ifndef TXSC_HDR
#ifdef CONFIG_WLAN_HAL                
					if (IS_HAL_CHIP(priv)) {
						// for TXBD mechanism, sending a packet always need one txdesc. So, txcfg->one_txdesc is useless.
#ifdef MERGE_HEADER_PAYLOAD
	                    memcpy((skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv)), txcfg->phdr, (txcfg->hdr_len + txcfg->llc + txcfg->iv));
	                    release_wlanllchdr_to_poll(priv, txcfg->phdr);
	                    txcfg->phdr = skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv);
	                    txcfg->one_txdesc = 1;
#endif
					} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
					{//not HAL
						if ((skb_headroom(skb) >= (txcfg->hdr_len + txcfg->llc + txcfg->iv
#ifdef TX_EARLY_MODE
							+ (GET_TX_EARLY_MODE ? 8 : 0)
#endif
							)) &&
							!skb_cloned(skb) &&
							(txcfg->privacy != _TKIP_PRIVACY_) &&
#if defined(CONFIG_RTL_WAPI_SUPPORT)
							(txcfg->privacy != _WAPI_SMS4_) &&
#endif
							((((unsigned int)skb->data) % 2) == 0)
							)
						{
							memcpy((skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv)), txcfg->phdr, (txcfg->hdr_len + txcfg->llc + txcfg->iv));
							release_wlanllchdr_to_poll(priv, txcfg->phdr);
							txcfg->phdr = skb->data - (txcfg->hdr_len + txcfg->llc + txcfg->iv);
							txcfg->one_txdesc = 1;
						}
					}

#endif
                    
#ifdef CONFIG_WLAN_HAL                        
				if(IS_HAL_CHIP(priv))
					rtl88XX_signin_txdesc_shortcut(priv, txcfg, k);
				else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
					rtl8192cd_signin_txdesc_shortcut(priv, txcfg, k);


#if defined(SHORTCUT_STATISTIC) //defined(__ECOS) && defined(_DEBUG_RTL8192CD_)
					priv->ext_stats.tx_cnt_sc2++;
#endif
					goto stop_proc;
				}

#ifdef CONFIG_RTK_MESH
	            if(txcfg->is_11s) {
	                memcpy(&priv->ethhdr, &(ptxsc_entry->ethhdr), sizeof(struct wlan_ethhdr_t));
	            }
#endif

#ifdef  CONFIG_WLAN_HAL
				if (IS_HAL_CHIP(priv)) {
					rtl88XX_signin_txdesc(priv, txcfg, HW_TX_SC_HEADER_CONV);
				} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
				{//not HAL
					rtl8192cd_signin_txdesc(priv, txcfg);
				}

#if defined(SHORTCUT_STATISTIC) //defined(__ECOS) && defined(_DEBUG_RTL8192CD_)
				priv->ext_stats.tx_cnt_sc1++; 
#endif
				ptxsc_entry->txcfg.tx_rate = txcfg->tx_rate;
				goto stop_proc;
		}
	}
	if (!priv->pmib->dot11OperationEntry.disable_txsc && pstat)
		pstat->tx_sc_pkts_slow++;
#endif // TX_SHORTCUT


#if defined(CONFIG_RTK_MESH) || defined(A4_STA)
just_skip:
#endif

#if defined(SHORTCUT_STATISTIC) //defined(__ECOS) && defined(_DEBUG_RTL8192CD_)
	priv->ext_stats.tx_cnt_nosc++;
#endif

	/* ==================== Slow path of packet TX process ==================== */
	SMP_UNLOCK_XMIT(flags);
	if (rtl8192cd_tx_slowPath(priv, skb, pstat, dev, wdsDev, txcfg) == TX_PROCEDURE_CTRL_STOP) {
		SMP_LOCK_XMIT(flags);
		goto stop_proc;
	}
	SMP_LOCK_XMIT(flags);

#ifdef __KERNEL__
	dev->trans_start = jiffies;
#endif

	goto stop_proc;

free_and_stop:		/* Free current packet and stop TX process */

	rtl_kfree_skb(priv, skb, _SKB_TX_);

stop_proc:			/* Stop process and assume the TX-ed packet is already "processed" (freed or TXed) in previous code. */
	
	return 0;
}
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
int __rtl8192cd_start_xmit(struct sk_buff *skb, struct net_device *dev, int tx_flag)
{
	struct rtl8192cd_priv *priv;
	struct stat_info	*pstat=NULL;
	unsigned char		*da;
#ifdef __KERNEL__
#if defined(HS2_SUPPORT) || !defined(CONFIG_PCI_HCI)
	struct sk_buff *newskb = NULL;
#endif
#endif
#ifdef TX_SCATTER
	struct sk_buff *newskb = NULL;
#endif
	struct net_device *wdsDev = NULL;
	struct tx_insn tx_insn;
    int temp;
#ifdef SMP_SYNC
	unsigned long flags;
#endif


	DECLARE_TXCFG(txcfg, tx_insn);

	priv = GET_DEV_PRIV(dev);

#ifdef ENABLE_RTL_SKB_STATS
	rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif

	if (skb->len < 15)
    {
        _DEBUG_ERR("TX DROP: SKB len small:%d\n", skb->len);
        goto free_and_stop;
    }

        skb->dev = dev;

#ifdef WDS
	if (!dev->base_addr && skb_cloned(skb)
		&& (priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_)
		) {
		struct sk_buff *mcast_skb = NULL;
		mcast_skb = skb_copy(skb, GFP_ATOMIC);
		if (mcast_skb == NULL) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: Can't copy the skb!\n");
			goto free_and_stop;
		}
		dev_kfree_skb_any(skb);
		skb = mcast_skb;
	}
#endif

#ifdef HS2_SUPPORT
	if (priv->proxy_arp)	{

		unsigned short protocol;

		if(isDHCPpkt(skb))	{
			staip_snooping_bydhcp(skb, priv);
		}

		if (IS_MCAST(skb->data))	{
			protocol = *((unsigned short *)(skb->data + 2 * ETH_ALEN));
	
			//proxy arp	
			if (protocol == __constant_htons(ETH_P_ARP))	{
				if (proxy_arp_handle(priv, skb))	{
					//reply ok and return
			        	goto stop_proc;
				}
				//drop packets
		        	goto free_and_stop;
			}
			//icmpv6 
			if (ICMPV6_MCAST_SOLI_MAC(skb->data))	{
			//if (ICMPV6_MCAST_MAC(skb->data))
				if (proxy_icmpv6_ndisc(priv,skb))	{
					goto stop_proc;
				}
				//drop packets
				goto free_and_stop;
			} 
			//drop unsolicited neighbor advertisement when proxy arp=1
			if (ICMPV6_MCAST_MAC(skb->data))	{
				if (check_nei_advt(priv,skb))	{
					 //drop packets
			             goto free_and_stop;
				}
			}
		}
	} 
	
	if(priv->dgaf_disable)
	{
		//dhcp m2u check
		if (IS_MCAST(skb->data))
		{
			if (isDHCPpkt(skb))
			{
				struct list_head *phead, *plist;
                HS2_DEBUG_INFO("DHCP multicast to unicast\n");
				phead = &priv->asoc_list;
				plist = phead->next;
				while (phead && (plist != phead)) 
				{
					pstat = list_entry(plist, struct stat_info, asoc_list);
					plist = plist->next;

					/* avoid   come from STA1 and send back STA1 */
			        if (!memcmp(pstat->hwaddr, &skb->data[6], 6))
					{
						continue;
					}
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
					newskb = priv_skb_copy(skb);
#else
					newskb = skb_copy(skb, GFP_ATOMIC);
#endif
	                if (newskb) 
					{
						memcpy(newskb->data, pstat->hwaddr, 6);
				        newskb->cb[2] = (char)0xff;         // not do aggregation
				        memcpy(newskb->cb+10,newskb->data,6);
                        #if 0   // for debug
						printk("send m2u, da=");						
						MAC_PRINT(pstat->hwaddr);
						printk("\n");
                        #endif
						__rtl8192cd_start_xmit(newskb, priv->dev, TX_NO_MUL2UNI);
					}
				}
			}
	        goto free_and_stop;
		}
	}
#endif

#ifdef SUPPORT_TX_MCAST2UNI
#ifdef WDS			// when interface is WDS don't enter multi2uni path
	if (dev->base_addr && !priv->pmib->dot11WdsInfo.wdsPure)
#endif
	if (!priv->pshare->rf_ft_var.mc2u_disable) {
		if ((OPMODE & WIFI_AP_STATE) &&	IS_MCAST(skb->data))
		{
			if ((IP_MCAST_MAC(skb->data)
				#ifdef	TX_SUPPORT_IPV6_MCAST2UNI
				|| IPV6_MCAST_MAC(skb->data)
				#endif
				) && (tx_flag != TX_NO_MUL2UNI))
			{
				if (mlcst2unicst(priv, skb)){
					return 0;
				}
			}
		}

		skb->cb[2] = 0;	// allow aggregation
	}
#endif

#ifdef SUPPORT_TX_MCAST2UNI
	skb->cb[16] = tx_flag;
#endif

#ifdef DFS
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		GET_ROOT(priv)->pmib->dot11DFSEntry.disable_tx 
		&& (priv->site_survey)
		&& (priv->site_survey->hidden_ap_found != HIDE_AP_FOUND_DO_ACTIVE_SSAN)) {
		priv->ext_stats.tx_drops++;
		DEBUG_ERR("TX DROP: DFS probation period\n");
		goto free_and_stop;
	}
#endif

	if (!IS_DRV_OPEN(priv))
		goto free_and_stop;

#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE) {
		goto free_and_stop;
	}
#endif

#ifdef MULTI_MAC_CLONE
	if (priv->pmib->ethBrExtInfo.macclone_enable) {
		int id;
		if (!(skb->data[ETH_ALEN] & 1) && MCLONE_NUM > 0) {
			id = mclone_find_address(priv, skb->data+ETH_ALEN, skb, MAC_CLONE_SA_FIND);
			if (id > 0)
				ACTIVE_ID = id;
			else
				ACTIVE_ID = 0;
		}  else
			ACTIVE_ID = 0;
	}
#endif

	// strip VLAN tag to avoid packet drop on peer which not support VLAN tag
	if (*((UINT16*)(skb->data+(ETH_ALEN<<1))) == __constant_htons(ETH_P_8021Q)) {
		if (skb_cloned(skb)) {
			newskb = skb_copy(skb, GFP_ATOMIC);
			if (newskb == NULL) {
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: Can't copy the skb!\n");
				goto free_and_stop;
			}
			dev_kfree_skb_any(skb);
			skb = newskb;
		}
		memmove(skb->data+VLAN_HLEN, skb->data, ETH_ALEN<<1);
		skb_pull(skb, VLAN_HLEN);
	}

#ifdef WDS
	if (dev->base_addr) {
		// normal packets
		if (priv->pmib->dot11WdsInfo.wdsPure) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: Sent normal pkt in Pure WDS mode!\n");
			goto free_and_stop;
		}
	}
	else {
		// WDS process
		if (rtl8192cd_tx_wdsDevProc(priv, skb, &dev, &wdsDev, txcfg) == TX_PROCEDURE_CTRL_STOP) {
			goto stop_proc;
		}
	}
#endif // WDS

	if (priv->pmib->miscEntry.func_off || priv->pmib->miscEntry.raku_only) {
		goto free_and_stop;
	}

	// drop packets if link status is null
#ifdef WDS
	if (!wdsDev)
#endif
	{
        temp = get_assoc_sta_num(priv, 1);        
		if (temp == 0) {
#ifdef RTK_BR_EXT
			if(!((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable && !priv->macclone_completed)){
#endif
				priv->ext_stats.tx_drops++;
				DEBUG_WARN("TX DROP: Non asoc tx request!\n");
				goto free_and_stop;
#ifdef RTK_BR_EXT
			}
#endif
		}
	}

#ifdef CLIENT_MODE
	// nat2.5 translation, mac clone, dhcp broadcast flag add.
	if (OPMODE & (WIFI_STATION_STATE|WIFI_ADHOC_STATE) ) {

#ifdef A4_STA
        pstat = get_stainfo(priv, BSSID);
        if(pstat && !(pstat->state & WIFI_A4_STA))
#endif
        {            
#ifdef RTK_BR_EXT
			if (!priv->pmib->ethBrExtInfo.nat25_disable &&
					!(skb->data[0] & 1) &&
#ifdef MULTI_MAC_CLONE
					(ACTIVE_ID == 0) &&
#endif		
#ifdef __KERNEL__
					GET_BR_PORT(priv->dev) &&
#endif
					memcmp(skb->data+MACADDRLEN, priv->br_mac, MACADDRLEN) &&
//					*((unsigned short *)(skb->data+MACADDRLEN*2)) != __constant_htons(ETH_P_8021Q) &&
					*((unsigned short *)(skb->data+MACADDRLEN*2)) == __constant_htons(ETH_P_IP) &&
					!memcmp(priv->scdb_mac, skb->data+MACADDRLEN, MACADDRLEN) && priv->scdb_entry) {
                if(priv->nat25_filter) {
                    if(nat25_filter(priv, skb)== 1) {
                        priv->ext_stats.tx_drops++;
                        DEBUG_ERR("TX DROP: nat25 filter out!\n");
                        goto free_and_stop;
                    }
                }

				memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
				priv->scdb_entry->ageing_timer = jiffies;
			}
			else
#endif		
			{
			if((OPMODE & WIFI_STATION_STATE) && memcmp(skb->data+ETH_ALEN, GET_MY_HWADDR, ETH_ALEN)) {	// ignore tx from root br to vap(client mode) if vap is not connected to remote ap successfully
				struct stat_info *pstat = get_stainfo(priv,BSSID);
				if (!((NULL != pstat) && (pstat->state & (WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE))
					   && (((0 != pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen) && (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm != _NO_PRIVACY_))
						   || (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _NO_PRIVACY_)))) {
					goto free_and_stop;
				}
			}
				SMP_UNLOCK_XMIT(flags);
				if (rtl8192cd_tx_clientMode(priv, &skb) == TX_PROCEDURE_CTRL_STOP) {
					SMP_LOCK_XMIT(flags);
					goto stop_proc;
				}
				SMP_LOCK_XMIT(flags);
			}
    		
#ifdef RTK_BR_EXT //8812_client
#ifdef MULTI_MAC_CLONE
			if (ACTIVE_ID >= 0)
				mclone_dhcp_caddr(priv, skb);		
			if (ACTIVE_ID == 0)				//not a mac clone sta
#endif
			dhcp_flag_bcast(priv, skb);
#endif
        }

	}
#endif // CLIENT_MODE

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
	{
		if ((OPMODE & WIFI_AP_STATE) && !wdsDev) {
			if ((*(unsigned int *)&(skb->cb[8]) == 0x86518190) &&						// come from wlan interface
				(*(unsigned int *)&(skb->cb[12]) != priv->pmib->miscEntry.groupID))		// check group ID
			{
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: not the same group!\n");
				goto free_and_stop;
			}
		}
	}
#endif

#ifdef WDS
	if (wdsDev)
		da = priv->pmib->dot11WdsInfo.entry[txcfg->wdsIdx].macAddr;
	else
#endif
	{

#ifdef MCAST2UI_REFINE
        da = &skb->cb[10];
#else
		da = skb->data;
#endif
	}

#ifdef CLIENT_MODE
	if ((OPMODE & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) == (WIFI_STATION_STATE | WIFI_ASOC_STATE))
		pstat = get_stainfo(priv, BSSID);
	else
#endif
	{
		pstat = get_stainfo(priv, da);
#ifdef A4_STA
        if (tx_flag == TX_NORMAL && pstat == NULL && (OPMODE & WIFI_AP_STATE)) {
            if(priv->pshare->rf_ft_var.a4_enable == 2) {
                if(IS_MCAST(da)) { 
                    /*duplicate the packet and use 4-address to send to every a4 client, 
                                      and send one 3-address packet  to other a3 clients*/
                    if( a4_tx_mcast_to_unicast(priv, skb) == 1) {
                        goto free_and_stop;
                    }
                }
                else {
                    pstat = a4_sta_lookup(priv, da);	
                    if(pstat == NULL) {
                        /*Unknown unicast, transmit to every A4 sta*/
                        a4_tx_unknown_unicast(priv, skb);                
                        goto free_and_stop;
                    }
                }

            }
            else if(priv->pshare->rf_ft_var.a4_enable == 1){
                if(!IS_MCAST(da)) {/*just send one 3-address multi/broadcast  for all clients*/
                    pstat = a4_sta_lookup(priv, da);
                }
            }
        }
#endif		
	}
#ifdef DETECT_STA_EXISTANCE
	if(pstat && pstat->leave)	{
		priv->ext_stats.tx_drops++;
		DEBUG_WARN("TX DROP: sta may leave! %02x%02x%02x%02x%02x%02x\n", pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
		goto free_and_stop;
	}
#endif

#ifdef WIFI_SIMPLE_CONFIG
	if (pstat && (*((u16*)(skb->data+ETH_ALEN*2)) == __constant_htons(0x888e))
			&& (skb->data[15] == 0) && (skb->data[18] == 4)) {		// EAP-Failure
		wsc_disconn_list_add(priv, da);
	}
#endif

#ifdef TX_SCATTER
#ifdef CONFIG_IEEE80211W
		if (skb->list_num > 0 && (UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE), 0) ||
#else
		if (skb->list_num > 0 && (UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE)) ||
#endif
		(pstat && (get_sta_encrypt_algthm(priv, pstat) == _TKIP_PRIVACY_)))) {
		newskb = copy_skb(skb);
		if (newskb == NULL) {
			priv->ext_stats.tx_drops++;
			DEBUG_ERR("TX DROP: Can't copy the skb for list buffer!\n");
			goto free_and_stop;
		}
		dev_kfree_skb_any(skb);
		skb = newskb;
	}
#endif

	txcfg->fr_type = _SKB_FRAME_TYPE_;
	txcfg->pframe = skb;
	txcfg->pstat = pstat;
	
	if (update_txinsn_stage1(priv, txcfg) == FALSE) {
		priv->ext_stats.tx_drops++;
		goto free_and_stop;
	}

#if !defined(SUPPORT_TX_AMSDU)
	if ((txcfg->privacy) && (
#ifdef CONFIG_IEEE80211W
		UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE), 0)	// sw enc will modify content
#else
		UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE))	// sw enc will modify content
#endif
		|| (pstat && (_TKIP_PRIVACY_ == txcfg->privacy))))		// need append MIC
#endif
	{
		if (skb_cloned(skb)
#ifdef MCAST2UI_REFINE
                        && !memcmp(skb->data, &skb->cb[10], 6)
#endif
			)
		{
			newskb = skb_copy(skb, GFP_ATOMIC);
			if (newskb == NULL) {
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: Can't copy the skb!\n");
				goto free_and_stop;
			}
			dev_kfree_skb_any(skb);
			skb = newskb;
		}
	}

	txcfg->pframe = skb;
	txcfg->next_txpath = TXPATH_HARD_START_XMIT;
	
#ifdef CONFIG_TCP_ACK_TXAGG
	if (priv->pshare->rf_ft_var.tcpack_agg && pstat && (pstat->state & WIFI_ASOC_STATE)) {
		if (*(u16 *)(skb->data + ETH_ALEN*2) == htons(ETH_P_IP)) {
			struct iphdr *iph = (struct iphdr *)(skb->data + ETH_HLEN);
			if (IPPROTO_TCP == iph->protocol) {	// 6
				struct tcphdr *tcph = (struct tcphdr *)((u8*)iph + iph->ihl*4);
				int tcp_flag;
				tcp_flag = tcp_flag_word(tcph) & __cpu_to_be32(0xff0000);
				// only handle TCP ACK(w/o TCP data) packet
				if ((TCP_FLAG_ACK == tcp_flag) && (iph->ihl*4+tcph->doff*4 == ntohs(iph->tot_len))) {
					if (rtw_xmit_enqueue_tcpack(priv, txcfg) == FALSE) {
						goto free_and_stop;
					}
					goto stop_proc;
				}
			}
		}
	}
#endif // CONFIG_TCP_ACK_TXAGG

#ifdef CONFIG_XMITBUF_TXAGG_ADV
	if (rtw_xmit_enqueue(priv, txcfg) == FALSE) {
		goto free_and_stop;
	}
	goto stop_proc;
#else // !CONFIG_XMITBUF_TXAGG_ADV

	xmit_preempt_disable(flags);
	if ((OPMODE & WIFI_AP_STATE)
#ifdef WDS
			&& (!wdsDev)
#endif
#ifdef CONFIG_RTK_MESH
			&& (dev != priv->mesh_dev)
#endif
		) {
		if ((pstat && ((pstat->state & (WIFI_SLEEP_STATE | WIFI_ASOC_STATE)) ==
			(WIFI_SLEEP_STATE | WIFI_ASOC_STATE))
#ifdef WMM_APSD
			&& (!pstat->apsd_trigger || !(pstat->apsd_bitmap & wmm_apsd_bitmask[txcfg->q_num]))
#endif
			) ||
			(((IS_MCAST(da) && !list_empty(&priv->sleep_list)) ||
			((priv->pshare->rf_ft_var.bcast_to_dzq) && (*da == 0xff)))
			&& !priv->release_mcast))
		{

			if (dz_queue(priv, pstat, txcfg) == TRUE)
			{
				DEBUG_INFO("queue up skb due to sleep mode\n");
				xmit_preempt_enable(flags);
				goto stop_proc;
			}
			else {
				if (pstat) {
					DEBUG_WARN("ucst sleep queue full!!\n");
				}
				else {
					DEBUG_WARN("mcst sleep queue full!!\n");
				}
				xmit_preempt_enable(flags);
				goto free_and_stop;
			}
		}
	}

	if ((rtw_is_tx_queue_empty(priv, txcfg) == FALSE)
#ifdef CONFIG_POWER_SAVE
			|| (RTW_PM_SUSPEND == priv->pshare->offload_function_ctrl)
#endif
			)
	{
		if (rtw_xmit_enqueue(priv, txcfg) == FALSE) {
			xmit_preempt_enable(flags);
			goto free_and_stop;
		}
		xmit_preempt_enable(flags);
		goto stop_proc;
	}
	xmit_preempt_enable(flags);
	
	if (__rtl8192cd_usb_start_xmit(priv, txcfg) == 0)
		goto stop_proc;
#endif // CONFIG_XMITBUF_TXAGG_ADV

free_and_stop:		/* Free current packet and stop TX process */

	rtl_kfree_skb(priv, skb, _SKB_TX_);

stop_proc:			/* Stop process and assume the TX-ed packet is already "processed" (freed or TXed) in previous code. */

	return 0;
}

int __rtl8192cd_usb_start_xmit(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	struct net_device *dev;
	struct net_device *wdsDev = NULL;
	struct stat_info *pstat;
	struct sk_buff *skb;

	#ifdef CONFIG_RTK_MESH
	int block_mesh = 0;
	#endif
#ifdef TX_SHORTCUT
	int k;
	struct tx_sc_entry *ptxsc_entry = NULL;
	unsigned char pktpri;
#endif
//	int priority;
#if defined(CONFIG_SDIO_HCI) && defined(TX_SCATTER)
	struct wlan_ethhdr_t	*pethhdr=NULL;
#endif
	int needTxBD=4;
	
	skb = (struct sk_buff *)txcfg->pframe;

	if (!IS_DRV_OPEN(priv))
		goto free_and_stop;

	if (priv->pmib->miscEntry.func_off || priv->pmib->miscEntry.raku_only)
		goto free_and_stop;
	
	switch( rtw_xmit_decision(priv, txcfg) ) {
	case XMIT_DECISION_ENQUEUE:
		goto stop_proc;
	case XMIT_DECISION_STOP:
		priv->ext_stats.tx_drops++;
		goto free_and_stop;
	case XMIT_DECISION_CONTINUE:
		break;
	}
	
#ifdef WDS
	if (txcfg->wdsIdx >= 0) {
		wdsDev = priv->wds_dev[txcfg->wdsIdx];
	}
#endif
	
	dev = priv->dev;
	pstat = txcfg->pstat;
	
	if (pstat)
	{
		#ifdef GBWC
		if (priv->pmib->gbwcEntry.GBWCMode)
			if (rtl8192cd_tx_gbwc(priv, pstat, skb) == TX_PROCEDURE_CTRL_STOP) 
				goto stop_proc;
		#endif

		#ifdef SBWC
		if (rtl8192cd_tx_sbwc(priv, pstat, skb) == TX_PROCEDURE_CTRL_STOP) 
			goto stop_proc;
		#endif
	}

#ifdef SUPPORT_TX_AMSDU
	#if defined(CONFIG_RTK_MESH) && !defined(MESH_AMSDU)
	if(dev == priv->mesh_dev){
		block_mesh = 1;
	}
    #endif

	if (IS_SUPPORT_TX_AMSDU(priv)
#if defined(CONFIG_RTK_MESH)
		&& !block_mesh
#endif
		&& pstat && (pstat->aggre_mthd & AGGRE_MTHD_MSDU) && (pstat->amsdu_level > 0)
#ifdef SUPPORT_TX_MCAST2UNI
		&& (priv->pshare->rf_ft_var.mc2u_disable || (skb->cb[2] != (char)0xff))
#endif
		) {
		int ret = amsdu_check(priv, skb, pstat, txcfg);

		if (ret == RET_AGGRE_ENQUE)
			goto stop_proc;

		if (ret == RET_AGGRE_DESC_FULL)
			goto free_and_stop;
	}
#endif

#ifdef TX_SHORTCUT
	if (!priv->pmib->dot11OperationEntry.disable_txsc && pstat) {
		pktpri = get_skb_priority(priv, skb, pstat);
		if ((k = get_tx_sc_index(priv, pstat, skb->data, pktpri)) >= 0)
			ptxsc_entry = &pstat->tx_sc_ent[pktpri][k];
	}
	
	if ((NULL != ptxsc_entry) &&
#if defined(NOT_RTK_BSP) && !defined(__ECOS)
		// go slow path to avoid TX drop when no enough tail room to put TKIP MIC.
		((_TKIP_PRIVACY_ != ptxsc_entry->txcfg.privacy) || ((skb->tail + 8) <= skb->end)) &&
#endif
		((ptxsc_entry->txcfg.privacy == 0) ||
#ifdef CONFIG_RTL_WAPI_SUPPORT
		  (ptxsc_entry->txcfg.privacy == _WAPI_SMS4_) ||
#endif
#ifdef CONFIG_IEEE80211W
		!UseSwCrypto(priv, pstat, FALSE, 0) ) &&
#else
		!UseSwCrypto(priv, pstat, FALSE) ) &&
#endif
		(ptxsc_entry->txcfg.fr_len > 0) &&
		(FRAGTHRSLD > 1500))
	{
#if defined(MESH_TX_SHORTCUT)
			if(txcfg->is_11s) {
                if(!mesh_txsc_decision(txcfg, &ptxsc_entry->txcfg)) { /*compare addr 5 and 6*/
                    goto just_skip;
                }

                if(memcmp(skb->data, GetAddr3Ptr(&ptxsc_entry->wlanhdr), MACADDRLEN)) {/*compare addr 3*/
                    goto just_skip;
                }

                ptxsc_entry->txcfg.mesh_header.segNum = txcfg->mesh_header.segNum;
                ptxsc_entry->txcfg.mesh_header.TTL = txcfg->mesh_header.TTL;
                ptxsc_entry->txcfg.is_11s = txcfg->is_11s;
			}
#endif

#ifdef A4_STA
            if(pstat->state & WIFI_A4_STA) {
                if(memcmp(skb->data, ptxsc_entry->ethhdr.daddr, MACADDRLEN)) {/*compare addr 3*/
                    goto just_skip;
                }
            }
#endif

			BUG_ON(txcfg->q_num != ptxsc_entry->txcfg.q_num);
			memcpy(txcfg, &ptxsc_entry->txcfg, FIELD_OFFSET(struct tx_insn, pxmitframe));

			{
			txcfg->phdr = (UINT8 *)get_wlanllchdr_from_poll(priv);
			if (txcfg->phdr == NULL) {
				DEBUG_ERR("Can't alloc wlan header!\n");
				rtl8192cd_tx_xmitSkbFail(priv, skb, dev, wdsDev, txcfg);
				goto stop_proc;
			}
			memcpy(txcfg->phdr, (const void *)&ptxsc_entry->wlanhdr, sizeof(struct wlanllc_hdr));
			txcfg->pframe = skb;
#ifdef WDS
			/*Correct the aggre_mthd*/
			if(pstat->state & WIFI_WDS)
				if(txcfg->aggre_en != pstat->aggre_mthd) {
					/*Invalid the tx_sc entry*/
					ptxsc_entry->txcfg.fr_len=0;
					txcfg->aggre_en = pstat->aggre_mthd;
				}	
#endif

				txcfg->fr_len = skb->len - WLAN_ETHHDR_LEN;
#if defined(CONFIG_SDIO_HCI) && defined(TX_SCATTER)
			// in case 1st buffer len is 14, we get ether header pointer first and then ajust the skb
			pethhdr = (struct wlan_ethhdr_t *)(skb->data);
#endif
			skb_pull(skb, WLAN_ETHHDR_LEN);
            }

#ifdef CONFIG_RTL_WAPI_SUPPORT
#ifdef CONFIG_RTL_HW_WAPI_SUPPORT
#ifdef CONFIG_IEEE80211W
			if((txcfg->privacy==_WAPI_SMS4_)&&(txcfg->llc>0)&&(UseSwCrypto(priv, pstat, FALSE, 0)))
#else
			if((txcfg->privacy==_WAPI_SMS4_)&&(txcfg->llc>0)&&(UseSwCrypto(priv, pstat, FALSE)))
#endif				
#else
			if((txcfg->privacy==_WAPI_SMS4_)&&(txcfg->llc>0))
#endif
			{				
				//To restore not-encrypted llc in wlan hdr
				//because llc in wlan hdr has been sms4encrypted to deliver at SecSWSMS4Encryption()
				eth_2_llc(&ptxsc_entry->ethhdr, (struct llc_snap *)(txcfg->phdr+txcfg->hdr_len + txcfg->iv));
			}
#endif

			if (txcfg->privacy == _TKIP_PRIVACY_) {
				if (rtl8192cd_tx_tkip(priv, skb, pstat, txcfg) == TX_PROCEDURE_CTRL_STOP) {
					goto stop_proc;
				}
			}

#ifdef MESH_TX_SHORTCUT
			if ( (txcfg->is_11s&1) && (GetFrameSubType(txcfg->phdr) == WIFI_11S_MESH))
                    if( !reuse_meshhdr(priv, txcfg) ) {
					        goto stop_proc;
					}
#endif
			if (txcfg->is_pspoll && (tx_servq_len(&pstat->tx_queue[BE_QUEUE]) > 0)) {
				SetMData(txcfg->phdr);
			}

#ifdef WMM_APSD
			if (pstat->apsd_trigger) {
				if (((pstat->apsd_bitmap & 0x01) && tx_servq_len(&pstat->tx_queue[VO_QUEUE]))
						|| ((pstat->apsd_bitmap & 0x02) && tx_servq_len(&pstat->tx_queue[VI_QUEUE]))
						|| ((pstat->apsd_bitmap & 0x04) && tx_servq_len(&pstat->tx_queue[BK_QUEUE]))
						|| ((pstat->apsd_bitmap & 0x08) && tx_servq_len(&pstat->tx_queue[BE_QUEUE]))) {
					SetMData(txcfg->phdr);
				} else {
					if ((GetFrameSubType(txcfg->phdr) & (BIT2 | BIT3 | BIT6 | BIT7)) == (WIFI_DATA_TYPE | BIT7)) {
						if (pstat->state & WIFI_SLEEP_STATE) {
							unsigned char *qosctrl = GetQosControl(txcfg->phdr);
							qosctrl[0] |= BIT4;
						}
					}
					pstat->apsd_trigger = 0;
					pstat->apsd_pkt_buffering = 0;
				}
			}
#endif

			txcfg->tx_rate = get_tx_rate(priv, pstat);
#ifndef TX_LOWESTRATE
			txcfg->lowest_tx_rate = get_lowest_tx_rate(priv, pstat, txcfg->tx_rate);
#endif			
			// log tx statistics...
			tx_sum_up(priv, pstat, txcfg);

			// for SW LED
			priv->pshare->LED_tx_cnt++;

			if (
#ifdef RTK_AC_SUPPORT //for 11ac logo //FOR_VHT5G_PF
			( ((pstat->aggre_mthd == AGGRE_MTHD_MPDU_AMSDU) )
			&& txcfg->aggre_en ) ||
#endif
			((txcfg->aggre_en >= FG_AGGRE_MPDU) && (txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST))
			) {
				if (!pstat->ADDBA_ready[(int)skb->cb[1]]) {
					if ((pstat->ADDBA_req_num[(int)skb->cb[1]] < 5) && !pstat->ADDBA_sent[(int)skb->cb[1]]) {
						pstat->ADDBA_req_num[(int)skb->cb[1]]++;
						SMP_UNLOCK_XMIT(flags);
						issue_ADDBAreq(priv, pstat, (int)skb->cb[1]);
						SMP_LOCK_XMIT(flags);
						pstat->ADDBA_sent[(int)skb->cb[1]]++;
					}
				}
			}

			// check if we could re-use tx descriptor
			if (
				((get_desc(ptxsc_entry->hwdesc1.Dword7)&TX_TxBufSizeMask) > 0) &&
				(skb->len == (get_desc(ptxsc_entry->hwdesc1.Dword7)&TX_TxBufSizeMask)) &&
#if 1
                        ((GET_CHIP_VER(priv)== VERSION_8812E) ? 1 : (txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate)) &&
#else
				(txcfg->tx_rate == ptxsc_entry->txcfg.tx_rate) &&
#endif	

				(pstat->protection == priv->pmib->dot11ErpInfo.protection) &&
				(pstat->ht_protection == priv->ht_protection)
#if defined(WIFI_WMM) && defined(WMM_APSD)
				&& (!(
#ifdef CLIENT_MODE
				(OPMODE & WIFI_AP_STATE) &&
#endif
				(APSD_ENABLE) && (pstat->state & WIFI_SLEEP_STATE)))
#endif
				) {

				pstat->tx_sc_pkts_lv2++;

				if (txcfg->privacy) {
					switch (txcfg->privacy) {
					case _WEP_104_PRIVACY_:
					case _WEP_40_PRIVACY_:
						wep_fill_iv(priv, txcfg->phdr, txcfg->hdr_len, ptxsc_entry->sc_keyid);
						break;

					case _TKIP_PRIVACY_:
						tkip_fill_encheader(priv, txcfg->phdr, txcfg->hdr_len, ptxsc_entry->sc_keyid);
						break;

					case _CCMP_PRIVACY_:
						aes_fill_encheader(priv, txcfg->phdr, txcfg->hdr_len, ptxsc_entry->sc_keyid);
						break;
					}
				}
				rtl8192cd_signin_txdesc_shortcut(priv, txcfg, ptxsc_entry);
#if defined(SHORTCUT_STATISTIC) //defined(__ECOS) && defined(_DEBUG_RTL8192CD_)
				priv->ext_stats.tx_cnt_sc2++;
#endif
				goto stop_proc;
			}

			pstat->tx_sc_pkts_lv1++;
#ifdef CONFIG_RTK_MESH
            if(txcfg->is_11s) {
                memcpy(&priv->ethhdr, &(ptxsc_entry->ethhdr), sizeof(struct wlan_ethhdr_t));
            }
#endif


#if defined(__ECOS) && defined(TX_SCATTER)
			if (skb->key)
				rtl8192cd_signin_txdesc(priv, txcfg, pethhdr); //TBD, ethhdr might be overwrite by WAPI
			else
				rtl8192cd_signin_txdesc(priv, txcfg, NULL);
#else
			rtl8192cd_signin_txdesc(priv, txcfg, NULL);
#endif
#if defined(SHORTCUT_STATISTIC) //defined(__ECOS) && defined(_DEBUG_RTL8192CD_)
			priv->ext_stats.tx_cnt_sc1++; 
#endif
			goto stop_proc;
	}
	if (!priv->pmib->dot11OperationEntry.disable_txsc && pstat)
		pstat->tx_sc_pkts_slow++;
#endif // TX_SHORTCUT
#if defined(CONFIG_RTK_MESH) || defined(A4_STA)
just_skip:
#endif

#if defined(SHORTCUT_STATISTIC) //defined(__ECOS) && defined(_DEBUG_RTL8192CD_)
	priv->ext_stats.tx_cnt_nosc++;
#endif

	/* ==================== Slow path of packet TX process ==================== */
	SMP_UNLOCK_XMIT(flags);
	if (rtl8192cd_tx_slowPath(priv, skb, pstat, dev, wdsDev, txcfg) == TX_PROCEDURE_CTRL_STOP) {
		SMP_LOCK_XMIT(flags);
		goto stop_proc;
	}
	SMP_LOCK_XMIT(flags);

#ifdef __KERNEL__
	dev->trans_start = jiffies;
#endif

	goto stop_proc;

free_and_stop:		/* Free current packet and stop TX process */

	rtl_kfree_skb(priv, skb, _SKB_TX_);

stop_proc:			/* Stop process and assume the TX-ed packet is already "processed" (freed or TXed) in previous code. */
	
	rtw_handle_xmit_fail(priv, txcfg);
	
	return 0;
}
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI

#ifndef CONFIG_RTK_MESH
int	rtl8192cd_wlantx(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	return (rtl8192cd_firetx(priv, txcfg));
}
#endif

#ifdef TX_SHORTCUT

#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_WLAN_HAL
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
void rtl88XX_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx)
{
    struct tx_desc_info *pswdescinfo, *pdescinfo;
    struct rtl8192cd_hw *phw;
    int q_num;
    u2Byte *tx_head;
    struct stat_info *pstat;
    struct sk_buff *pskb;

    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    u32                             halQNum;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_BUFFER_DESCRIPTOR           cur_txbd;
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
    PHCI_TX_AMSDU_DMA_MANAGER_88XX  ptx_dma_amsdu;
    PTX_BUFFER_DESCRIPTOR_AMSDU     cur_txbd_amsdu;
#endif

    TX_DESC_DATA_88XX               desc_data;
    BOOLEAN                         use_hwtxsc = FALSE;

    pstat = txcfg->pstat;
    pskb = (struct sk_buff *)txcfg->pframe;

    phw = GET_HW(priv);
    q_num = txcfg->q_num;

    halQNum     = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, (u32)q_num);
    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[halQNum]);
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;
    memset(&desc_data, 0, sizeof(TX_DESC_DATA_88XX));

    tx_head     = &(cur_q->host_idx);

    pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);    

    pdescinfo = pswdescinfo + *tx_head;

	// TODO: if enable HW_SEQ , it should be disabled here...
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
    if (GET_CHIP_VER(priv)==VERSION_8814A) {
        // Don't assign seq num in this case.....
    } else
#endif
    {
    assign_wlanseq(GET_HW(priv), txcfg->phdr, pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
    , txcfg->is_11s
#endif
        );
    }
#ifdef WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
    // Hw_tx_shortcut just support 1 entry for one STA
    // if enable HW TX shortcut, the following TXDESC information no need modify
    if((!IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC(priv))|| pstat->tx_sc_hw_idx != idx)
#endif        
    {

        if ((pstat->IOTPeer==HT_IOT_PEER_INTEL) && (pstat->retry_inc)) {
            if (is_MCS_rate(pstat->current_tx_rate) && !(pstat->leave)
                && priv->pshare->intel_rty_lmt) {
                desc_data.rtyLmtEn   = _TRUE;
                desc_data.dataRtyLmt = priv->pshare->intel_rty_lmt; 
            } else {
                desc_data.rtyLmtEn   = _FALSE;
                desc_data.dataRtyLmt = 1;
            }
        }
	if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
		(txcfg->pstat) && (txcfg->pstat->MIMO_ps & _HT_MIMO_PS_DYNAMIC_) &&
		(is_2T_rate(txcfg->tx_rate) || (txcfg->pstat->tx_avarage>=(3<<17))))
		{	// when HT MIMO Dynamic power save is set and rate > MCS7, RTS is needed
			desc_data.RTSEn = _TRUE;
		    desc_data.HWRTSEn = _FALSE;
		    desc_data.CTS2Self= _FALSE;	
		}
        
        #if (BEAMFORMING_SUPPORT == 1)
        if (priv->pmib->dot11nConfigEntry.dot11nSTBC && (txcfg->pstat) &&
            ((txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_RX_STBC_CAP_)) 
            #ifdef RTK_AC_SUPPORT			
            || (txcfg->pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(_VHTCAP_RX_STBC_CAP_))
            #endif
        )){
            u1Byte					Idx = 0;

            PRT_BEAMFORMING_ENTRY	pEntry; 
            pEntry = Beamforming_GetEntryByMacId(priv, txcfg->pstat->aid, &Idx);

            if(((get_rf_mimo_mode(priv) == MIMO_2T2R) || (get_rf_mimo_mode(priv) == MIMO_3T3R)) && (pEntry == NULL))
                desc_data.dataStbc = 1;
        }
        #endif
    }

	if((priv->pshare->rf_ft_var.txforce != 0xff || (txcfg->pstat && txcfg->pstat->force_rate))
        #if (BEAMFORMING_SUPPORT == 1)
				&& (!txcfg->ndpa) 
        #endif
			){
			if(txcfg->pstat && txcfg->pstat->force_rate)
				desc_data.dataRate	= txcfg->pstat->force_rate;
			else
				desc_data.dataRate	= priv->pshare->rf_ft_var.txforce;
			desc_data.disDataFB = _TRUE;
			desc_data.disRTSFB	= _TRUE;
			desc_data.useRate	= _TRUE;
	}

    desc_data.iv = txcfg->iv;
    desc_data.secType = txcfg->privacy;

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		if(txcfg->pstat)
		{
			ODM_SetTxAntByTxInfo_HAL(priv, (PVOID)&desc_data, txcfg->pstat->aid);   //HAL Set TX antenna	
		}
#endif	//#if (defined(CONFIG_HW_ANTENNA_DIVERSITY))

#ifdef CLIENT_MODE
    if (OPMODE & WIFI_STATION_STATE) {
        if (GetFrameSubType(txcfg->phdr) == WIFI_PSPOLL) {
            desc_data.navUseHdr = _TRUE;
        }

        if (priv->ps_state)
            SetPwrMgt(txcfg->phdr);
        else
            ClearPwrMgt(txcfg->phdr);
    }
#endif

    // TODO: have_hw_mic
//one_txdesc:

    //descinfo_copy(pdescinfo, &pstat->tx_sc_ent[idx].swdesc1);
    pdescinfo->type = pstat->tx_sc_ent[idx].swdesc1.type;
#ifndef TXDESC_INFO	
    pdescinfo->len = pstat->tx_sc_ent[idx].swdesc1.len;
    pdescinfo->rate = pstat->tx_sc_ent[idx].swdesc1.rate;  
#endif	

    //??? pdescinfo->buf_type[0] = pstat->tx_sc_ent[idx].swdesc1.buf_type[0];
    pdescinfo->buf_type[0] = txcfg->fr_type;
    
    pdescinfo->pframe = txcfg->phdr;
    //pdescinfo->buf_pframe[0] = txcfg->pframe;
#if defined(WIFI_WMM) && defined(WMM_APSD)
    pdescinfo->priv = priv;
#ifndef TXDESC_INFO		
    pdescinfo->pstat = pstat;
#endif	
#endif
    
    desc_data.pHdr   = txcfg->phdr;    
    desc_data.hdrLen = txcfg->hdr_len;
    desc_data.llcLen = txcfg->llc;
    desc_data.frLen  = txcfg->fr_len;   // pskb->len ??
    //if (pskb->len != txcfg->fr_len) {
    //    printk("%s(%d): pskb->len:%d, txcfg->fr_len:%d \n", __FUNCTION__, __LINE__, pskb->len, txcfg->fr_len);
    //}
    desc_data.pBuf   = pskb->data;

#ifdef WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
    if(IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC(priv))   {

        if(pstat->tx_sc_hw_idx == idx) {
            desc_data.stwEn = TRUE;
            if (txcfg->pstat->aid != MANAGEMENT_AID) {
                desc_data.macId = REMAP_AID(txcfg->pstat);
                desc_data.rateId = txcfg->pstat->ratr_idx;
                desc_data.dataBW = txcfg->pstat->tx_bw;
                desc_data.tid = pskb->cb[1];
#ifdef WMM_DSCP_C42
				{
					unsigned int	   q_num;
#ifdef RTL_MANUAL_EDCA
					q_num = PRI_TO_QNUM(priv, desc_data.tid);
#else
					PRI_TO_QNUM(desc_data.tid, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
					if(q_num ^ txcfg->q_num) 
						desc_data.tid = 0x04;
				}
#elif defined(CONFIG_WLAN_HAL_8197F)
				{
					unsigned int	   q_num;
#ifdef RTL_MANUAL_EDCA
					q_num = PRI_TO_QNUM(priv, desc_data.tid);
#else
					PRI_TO_QNUM(desc_data.tid, q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
				}
#endif
				
            }      
            use_hwtxsc = TRUE;
        }
        else {
            pstat->tx_sc_hw_idx = idx;
        }        
    }
#endif // WLAN_HAL_HW_TX_SHORTCUT_REUSE_TXDESC
#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
    if(IS_SUPPORT_WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV(priv))   {   

        desc_data.swCrypt = FALSE;
        desc_data.icv     = 0;
        desc_data.mic     = 0;

#ifdef WLAN_HAL_HW_AES_IV
        if (txcfg->privacy == _CCMP_PRIVACY_) {
            desc_data.iv = 0;
            desc_data.hwAESIv = TRUE;
        }
#endif
		desc_data.smhEn = TRUE;

		desc_data.pHdr	   = txcfg->phdr; // 802.3 hdr len + payload len;
		desc_data.hdrLen   = txcfg->fr_len; // 802.3 hdr + payload

		desc_data.llcLen   = 0;

		desc_data.frLen    = 0;
		desc_data.pBuf	   = 0; // useless....because frLen is zero
		// TODO: recycle problem for sw desc..
    }
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV



#ifdef MERGE_HEADER_PAYLOAD
//    if (txcfg->one_txdesc) {
		desc_data.pHdr	   = txcfg->phdr; // 802.3 hdr + payload
		desc_data.hdrLen   = txcfg->fr_len; // len(802.3 hdr + payload)

		desc_data.llcLen   = 0;

		desc_data.frLen    = 0;
		desc_data.pBuf	   = 0;
//    }
#endif

#ifdef SUPPORT_TX_AMSDU_SHORTCUT
    desc_data.aggreEn = txcfg->aggre_en;
    if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
        priv->amsdu_len = desc_data.hdrLen + desc_data.llcLen + desc_data.frLen;

#ifdef CONFIG_IEEE80211W
        if (txcfg->privacy && (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF)==FALSE))
#else
        if (txcfg->privacy && (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE))==FALSE))
#endif
        {
            priv->amsdu_len += (desc_data.icv + desc_data.mic + desc_data.iv);
        }
    }
//	if(txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
//		AMSDU_DBG_PRINT("%s %d desc_data.frLen %d txcfg->fr_len %d priv->amsdu_len %d desc_data.aggreEn %d\n", __func__, __LINE__, desc_data.frLen, txcfg->fr_len, priv->amsdu_len, desc_data.aggreEn);
#endif

    GET_HAL_INTERFACE(priv)->FillShortCutTxHwCtrlHandler(
        priv, halQNum, (void *)&desc_data, pstat->tx_sc_ent[idx].hal_hw_desc, 0x02, use_hwtxsc);

	// TODO: check the sw desc here for HW_TX_SHORTCUT
#ifndef TXDESC_INFO	
    pdescinfo->paddr         = get_desc(cur_txbd->TXBD_ELE[1].Dword1);//header address
#endif	
    pdescinfo->len           = get_desc(cur_txbd->TXBD_ELE[1].Dword0) & 0xffff;
    pdescinfo->buf_paddr[0]  = get_desc(cur_txbd->TXBD_ELE[2].Dword1);//payload address
    pdescinfo->buf_pframe[0] = pskb;
    pdescinfo->buf_len[0]    = txcfg->fr_len; // pskb->len;

#ifdef SUPPORT_TX_AMSDU_SHORTCUT
            if ((AMSDU_ENABLE >= 2) && IS_SUPPORT_TX_AMSDU(priv) && txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
                ptx_dma_amsdu  = (PHCI_TX_AMSDU_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMAAMSDU88XX);
                cur_txbd_amsdu = ptx_dma_amsdu->tx_amsdu_queue[halQNum-1].pTXBD_head_amsdu + cur_q->host_idx;

                pdescinfo->buf_type_amsdu[0]    = txcfg->fr_type;
//		AMSDU_DBG_PRINT("%s %d pframe %p\n", __func__, __LINE__, txcfg->pframe);
                pdescinfo->buf_pframe_amsdu[0]  = txcfg->pframe;
                pdescinfo->buf_type[0] = _RESERVED_FRAME_TYPE_;
                pdescinfo->buf_pframe[0] = NULL;
                
            } else if (txcfg->aggre_en > FG_AGGRE_MSDU_FIRST) {
                printk("%s(%d): amsdu error !!! %d \n", __FUNCTION__, __LINE__, txcfg->aggre_en);
            }
#endif // WLAN_HAL_TX_AMSDU


#ifdef WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV
    if (GET_CHIP_VER(priv)==VERSION_8814A) {
        pdescinfo->type          = _SKB_FRAME_TYPE_; // 802.3 header + payload
        pdescinfo->pframe        = txcfg->pframe; // skb
        
        pdescinfo->buf_type[0]   = 0;
        pdescinfo->buf_pframe[0] = 0; // no packet payload        
    }
#endif // WLAN_HAL_HW_TX_SHORTCUT_HDR_CONV

#ifdef MERGE_HEADER_PAYLOAD
    pdescinfo->type          = _SKB_FRAME_TYPE_; // 802.3 header + payload
    pdescinfo->pframe        = txcfg->phdr; // skb

    pdescinfo->buf_type[0]   = 0;
    pdescinfo->buf_pframe[0] = 0; // no packet payload     
#endif


#ifdef MERGE_TXDESC_HEADER_PAYLOAD
//    if (q_num == BE_QUEUE) {
    pdescinfo->type             = _SKB_FRAME_TYPE_; // txdesc + 802.3 header + payload
    pdescinfo->pframe           = txcfg->pframe;    // skb

    pdescinfo->buf_type[0]      = 0;
    pdescinfo->buf_pframe[0]    = 0;

    release_wlanllchdr_to_poll(priv, txcfg->phdr);
//        }
#endif

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	if (RSVQ_ENABLE && IS_RSVQ(q_num))
		priv->use_txdesc_cnt[RSVQ(q_num)] ++;
#endif

#ifdef SUPPORT_TX_AMSDU_SHORTCUT
    if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
        priv->amsdu_num = 1;
        return;
    }
#endif

    GET_HAL_INTERFACE(priv)->SyncSWTXBDHostIdxToHWHandler(priv, halQNum);

	return;
}
#endif // CONFIG_WLAN_HAL

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
void rtl8192cd_signin_txdesc_shortcut_8812(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx)
{
	struct tx_desc *phdesc, *pdesc, *pfrstdesc;
	struct tx_desc_info *pswdescinfo, *pdescinfo;
	struct rtl8192cd_hw	*phw;
	int *tx_head, q_num;
	struct stat_info *pstat;
	struct sk_buff *pskb;
	unsigned long pfrst_dma_desc;
	unsigned long *dma_txhead;
/*
#if defined(CONFIG_RTL_WAPI_SUPPORT)
	uint8				*wapiMic2;
	struct tx_desc		*pmicdesc;
	struct tx_desc_info	*pmicdescinfo;
#endif
*/
	pstat = txcfg->pstat;
	pskb = (struct sk_buff *)txcfg->pframe;
	pfrst_dma_desc=0;

	phw	= GET_HW(priv);
	q_num = txcfg->q_num;

	dma_txhead	= get_txdma_addr(phw, q_num);
	tx_head		= get_txhead_addr(phw, q_num);
	phdesc		= get_txdesc(phw, q_num);
	pswdescinfo	= get_txdesc_info(priv->pshare->pdesc_info, q_num);

	/*------------------------------------------------------------*/
	/*           fill descriptor of header + iv + llc             */
	/*------------------------------------------------------------*/
	pfrstdesc = pdesc = phdesc + *tx_head;
	pdescinfo = pswdescinfo + *tx_head;

    memcpy(pdesc, &pstat->tx_sc_ent[idx].hwdesc1, 40);

	assign_wlanseq(GET_HW(priv), txcfg->phdr, pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
	, txcfg->is_11s
#endif
		);

	pdesc->Dword9 = 0;
	pdesc->Dword9 |= set_desc((GetSequence(txcfg->phdr) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);	

	if((priv->pshare->rf_ft_var.txforce != 0xff)
#if (BEAMFORMING_SUPPORT == 1)
		&& (!txcfg->ndpa) 
#endif
	){
		pdesc->Dword4 &= set_desc(~(TXdesc_92E_DataRateMask << TXdesc_92E_DataRateSHIFT));
		pdesc->Dword3 |= set_desc(TXdesc_92E_DisDataFB|TXdesc_92E_DisRtsFB|TXdesc_92E_UseRate);
		pdesc->Dword4 |= set_desc((priv->pshare->rf_ft_var.txforce & TXdesc_92E_DataRateMask) << TXdesc_92E_DataRateSHIFT);
	}


	if (txcfg->one_txdesc) {
#ifdef TX_EARLY_MODE
		if (GET_TX_EARLY_MODE) {
			pdesc->Dword0 = set_desc(((get_desc(pdesc->Dword0) & 0xff00ffff) |(0x28 << TX_OffsetSHIFT)) |
									TX_LastSeg | 	(txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
			pdesc->Dword1 = set_desc(get_desc(pdesc->Dword1) | (1 << TX_PktOffsetSHIFT) );
			pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
						(txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len+8));
			memset(txcfg->phdr-8, '\0', 8);			
			if (pstat->empkt_num > 0) 				
				insert_emcontent(priv, txcfg, txcfg->phdr-8);
			pdesc->Dword10 = set_desc(get_physical_addr(priv, txcfg->phdr-8,
				(get_desc(pdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE));
					
		}
		else
#endif
		{	
			pdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & 0xffff0000) |
				TX_LastSeg | (txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
			pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
				(txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
		}
	}

#ifdef TXSC_SKBLEN
    else {
		pdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & 0xffff0000) |
						 (txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
		pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
						  (txcfg->hdr_len + txcfg->llc + txcfg->iv ));
	}
#endif

#if (BEAMFORMING_SUPPORT == 1)
	if ((priv->pmib->dot11RFEntry.txbf == 1) && (priv->pmib->dot11nConfigEntry.dot11nSTBC))	{
		if ((pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_RX_STBC_CAP_)) 
#ifdef RTK_AC_SUPPORT			
				|| (txcfg->pstat->vht_cap_buf.vht_cap_info & cpu_to_le32(_VHTCAP_RX_STBC_CAP_))
#endif
		) {		
			u1Byte					Idx = 0;
			PRT_BEAMFORMING_ENTRY	pEntry; 
			pEntry = Beamforming_GetEntryByMacId(priv, pstat->aid, &Idx);
			if(pEntry)
				pdesc->Dword5 &= set_desc(~ (BIT(TXdesc_92E_DataStbcSHIFT)));
		}
	}
#endif
#ifdef TX_EARLY_MODE
	if (GET_TX_EARLY_MODE) 
		pdesc->Dword10 = set_desc(get_physical_addr(priv, txcfg->phdr-8,
			(get_desc(pdesc->Dword7)& TX_TxBufSizeMask), PCI_DMA_TODEVICE));
	else
#endif
	pdesc->Dword10 = set_desc(get_physical_addr(priv, txcfg->phdr,
		(get_desc(pdesc->Dword7)& TX_TxBufSizeMask), PCI_DMA_TODEVICE));

	descinfo_copy(pdescinfo, &pstat->tx_sc_ent[idx].swdesc1);
#ifndef TXDESC_INFO
	pdescinfo->paddr  = get_desc(pdesc->Dword10); // buffer addr
#endif	
	if (txcfg->one_txdesc) {
		pdescinfo->type = _SKB_FRAME_TYPE_;
		pdescinfo->pframe = pskb;
		pdescinfo->priv = priv;
#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifndef TXDESC_INFO
		pdescinfo->pstat = pstat;
#endif		
#endif
	}
	else {
		pdescinfo->pframe = txcfg->phdr;
#if defined(WIFI_WMM) && defined(WMM_APSD)
		pdescinfo->priv = priv;
#ifndef TXDESC_INFO		
		pdescinfo->pstat = pstat;
#endif		
#endif
	}

#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE) {
		if (GetFrameSubType(pdescinfo->pframe) == WIFI_PSPOLL)
			pdesc->Dword1 |= set_desc(TX_NAVUSEHDR);

		if (priv->ps_state)
			SetPwrMgt(pdescinfo->pframe);
		else
			ClearPwrMgt(pdescinfo->pframe);
	}
#endif

	pfrst_dma_desc = dma_txhead[*tx_head];
/*
#ifdef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, get_desc(pdesc->Dword8), (get_desc(pdesc->Dword7)&TX_TxBufferSizeMask), PCI_DMA_TODEVICE);
#endif
*/

#ifdef OSK_LOW_TX_DESC
	if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
		txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
	else
#endif
	txdesc_rollover(pdesc, (unsigned int *)tx_head);

	if (txcfg->one_txdesc)
		goto one_txdesc;

	/*------------------------------------------------------------*/
	/*              fill descriptor of frame body                 */
	/*------------------------------------------------------------*/
	pdesc = phdesc + *tx_head;
	pdescinfo = pswdescinfo + *tx_head;
    memcpy(pdesc, &pstat->tx_sc_ent[idx].hwdesc2, 40);

#ifdef TXSC_SKBLEN
	pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
				  ( txcfg->fr_len));
#endif

	pdesc->Dword10 = set_desc(get_physical_addr(priv, pskb->data,
		(get_desc(pdesc->Dword7)&0x0fff), PCI_DMA_TODEVICE));

	descinfo_copy(pdescinfo, &pstat->tx_sc_ent[idx].swdesc2);
#ifndef TXDESC_INFO
	pdescinfo->paddr  = get_desc(pdesc->Dword10);
#endif	
	pdescinfo->pframe = pskb;
	pdescinfo->priv = priv;
/*
#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, dma_txhead[*tx_head], sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#else
	rtl_cache_sync_wback(priv, get_desc(pdesc->Dword8), (get_desc(pdesc->Dword7)&TX_TxBufferSizeMask), PCI_DMA_TODEVICE);
#endif
*/

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	if (txcfg->privacy == _WAPI_SMS4_)
	{
		SecSWSMS4Encryption(priv, txcfg);
	}
#endif

#ifndef NOT_RTK_BSP
	if ((txcfg->privacy == _TKIP_PRIVACY_) &&
		(priv->pshare->have_hw_mic) &&
		!(priv->pmib->dot11StationConfigEntry.swTkipMic))
	{
		register unsigned long int l,r;
		unsigned char *mic;
		int delay = 18;

		while ((*(volatile unsigned int *)GDMAISR & GDMA_COMPIP) == 0) {
			delay_us(delay);
			delay = delay / 2;
		}

		l = *(volatile unsigned int *)GDMAICVL;
		r = *(volatile unsigned int *)GDMAICVR;

		mic = ((struct sk_buff *)txcfg->pframe)->data + txcfg->fr_len - 8;
		mic[0] = (unsigned char)(l & 0xff);
		mic[1] = (unsigned char)((l >> 8) & 0xff);
		mic[2] = (unsigned char)((l >> 16) & 0xff);
		mic[3] = (unsigned char)((l >> 24) & 0xff);
		mic[4] = (unsigned char)(r & 0xff);
		mic[5] = (unsigned char)((r >> 8) & 0xff);
		mic[6] = (unsigned char)((r >> 16) & 0xff);
		mic[7] = (unsigned char)((r >> 24) & 0xff);

#ifdef MICERR_TEST
		if (priv->micerr_flag) {
			mic[7] ^= mic[7];
			priv->micerr_flag = 0;
		}
#endif
	}
#endif // NOT_RTK_BSP

#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(get_desc(pdesc->Dword10)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), (get_desc(pdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE);

#ifdef OSK_LOW_TX_DESC
	if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
		txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
	else
#endif
	txdesc_rollover(pdesc, (unsigned int *)tx_head);
one_txdesc:

	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(get_desc(pfrstdesc->Dword10)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), (get_desc(pfrstdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE);

#ifdef SUPPORT_SNMP_MIB
	if (txcfg->rts_thrshld <= get_mpdu_len(txcfg, txcfg->fr_len))
		SNMP_MIB_INC(dot11RTSSuccessCount, 1);
#endif

	pfrstdesc->Dword0 |= set_desc(TX_OWN);

#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(pfrst_dma_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	if (RSVQ_ENABLE && IS_RSVQ(q_num))
		priv->use_txdesc_cnt[RSVQ(q_num)] += (txcfg->one_txdesc)? 1 : 2;
#endif

	if (q_num == HIGH_QUEUE) {
		DEBUG_WARN("signin shortcut for DTIM pkt?\n");
		return 0;
	} else {
		tx_poll(priv, q_num);
	}

	return 0;
}
#endif

#if(CONFIG_WLAN_NOT_HAL_EXIST==1)
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
void rtl8192cd_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx)
{
	struct tx_desc *phdesc, *pdesc, *pfrstdesc;
	struct tx_desc_info *pswdescinfo, *pdescinfo;
	struct rtl8192cd_hw	*phw;
	int *tx_head, q_num;
	struct stat_info *pstat;
	struct sk_buff *pskb;
	unsigned long pfrst_dma_desc;
	unsigned long *dma_txhead;
/*
#if defined(CONFIG_RTL_WAPI_SUPPORT)
	uint8				*wapiMic2;
	struct tx_desc		*pmicdesc;
	struct tx_desc_info	*pmicdescinfo;
#endif
*/
#ifdef TX_SCATTER
	int	go_desc3=0;
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv)== VERSION_8723B)
		return rtl8192cd_signin_txdesc_shortcut_8812(priv, txcfg, idx);
#endif

	pstat = txcfg->pstat;
	pskb = (struct sk_buff *)txcfg->pframe;
	pfrst_dma_desc=0;

	phw	= GET_HW(priv);
	q_num = txcfg->q_num;

	dma_txhead	= get_txdma_addr(phw, q_num);
	tx_head		= get_txhead_addr(phw, q_num);
	phdesc		= get_txdesc(phw, q_num);
	pswdescinfo	= get_txdesc_info(priv->pshare->pdesc_info, q_num);

	/*------------------------------------------------------------*/
	/*           fill descriptor of header + iv + llc             */
	/*------------------------------------------------------------*/
	pfrstdesc = pdesc = phdesc + *tx_head;
	pdescinfo = pswdescinfo + *tx_head;
   	
	desc_copy(pdesc, &pstat->tx_sc_ent[idx].hwdesc1);	
	assign_wlanseq(GET_HW(priv), txcfg->phdr, pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
	, txcfg->is_11s
#endif
		);

			
	pdesc->Dword3 = 0;
	pdesc->Dword3 = set_desc((GetSequence(txcfg->phdr) & TX_SeqMask) << TX_SeqSHIFT);

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	if (CHIP_VER_92X_SERIES(priv))
#endif
	if (priv->pmib->dot11RFEntry.txbf == 1) {
		pdesc->Dword2 &= set_desc(0x03ffffff); // clear related bits
		pdesc->Dword2 |= set_desc(1 << TX_TxAntCckSHIFT);	// Set Default CCK rate with 1T
		pdesc->Dword2 |= set_desc(1 << TX_TxAntlSHIFT); 	// Set Default Legacy rate with 1T
		pdesc->Dword2 |= set_desc(1 << TX_TxAntHtSHIFT);	// Set Default Ht rate

		if (is_MCS_rate(txcfg->tx_rate)) {
			if ((txcfg->tx_rate - HT_RATE_ID) <= 6){
					pdesc->Dword2 |= set_desc(3 << TX_TxAntHtSHIFT); // Set Ht rate < MCS6 with 2T
			}
		}
	}
	if(priv->pmib->dot11RFEntry.bcn2path){
		pdesc->Dword2 &= set_desc(0x03ffffff); // clear related bits
		pdesc->Dword2 |= set_desc(1 << TX_TxAntCckSHIFT);	// Set Default CCK rate with 1T
	}

//	if (txcfg->pstat)
//		pdesc->Dword1 |= set_desc(txcfg->pstat->aid & TX_MACIDMask);

	//set Break
	if((txcfg->q_num >=1 && txcfg->q_num <=4)){
		if((pstat != priv->pshare->CurPstat[txcfg->q_num-1])) {
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E)
				pdesc->Dword2 |= set_desc(TXdesc_88E_BK);
			else
#endif
				pdesc->Dword1 |= set_desc(TX_BK);
			priv->pshare->CurPstat[txcfg->q_num-1] = pstat;
		} else
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E) {
			pdesc->Dword2 &= set_desc(~TXdesc_88E_BK);
		} else
#endif
		{
			pdesc->Dword1 &= set_desc(~TX_BK); // clear it
		}
	} else {
#ifdef CONFIG_RTL_88E_SUPPORT
		if (GET_CHIP_VER(priv)==VERSION_8188E)
			pdesc->Dword2 |= set_desc(TXdesc_88E_BK);
		else
#endif
			pdesc->Dword1 |= set_desc(TX_BK);
	}


	if ((pstat->IOTPeer==HT_IOT_PEER_INTEL) && (pstat->retry_inc)) {
		if (is_MCS_rate(pstat->current_tx_rate) && !(pstat->leave)
			&& priv->pshare->intel_rty_lmt) {
			pdesc->Dword5 |= set_desc(TX_RtyLmtEn);
			
			pdesc->Dword5 &= set_desc(~(TX_DataRtyLmtMask << TX_DataRtyLmtSHIFT));
			pdesc->Dword5 |= set_desc((priv->pshare->intel_rty_lmt & TX_DataRtyLmtMask) << TX_DataRtyLmtSHIFT);
		} else {
			pdesc->Dword5 &= set_desc(~TX_RtyLmtEn);
			pdesc->Dword5 &= set_desc(~(TX_DataRtyLmtMask << TX_DataRtyLmtSHIFT));
		}
	}

#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
	if ((GET_CHIP_VER(priv)==VERSION_8188E) && (!txcfg->fixed_rate)) {
		if (pstat->ht_current_tx_info & TX_USE_SHORT_GI)
			pdesc->Dword5 |= set_desc(TX_SGI);
		else
			pdesc->Dword5 &= set_desc(~TX_SGI);
	}
#endif

	if (txcfg->one_txdesc) {
#ifdef TX_EARLY_MODE
		if (GET_TX_EARLY_MODE) {
			pdesc->Dword0 = set_desc(((get_desc(pdesc->Dword0) & 0xff00ffff) |(0x28 << TX_OffsetSHIFT)) |
									TX_LastSeg | 	(txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
			pdesc->Dword1 = set_desc(get_desc(pdesc->Dword1) | (1 << TX_PktOffsetSHIFT) );
			pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
						(txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len+8));
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv) == VERSION_8188E) {
				pdesc->Dword6 &= set_desc(~ (0xf << TX_MaxAggNumSHIFT));
				pdesc->Dword6 |= set_desc(0xf << TX_MaxAggNumSHIFT);
			}
#endif
			memset(txcfg->phdr-8, '\0', 8);			
			if (pstat->empkt_num > 0) 				
				insert_emcontent(priv, txcfg, txcfg->phdr-8);
			pdesc->Dword8 = set_desc(get_physical_addr(priv, txcfg->phdr-8,
				(get_desc(pdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE));
					
		}
		else
#endif
		{	
#if defined(TX_EARLY_MODE) && defined(CONFIG_RTL_88E_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8188E)
				pdesc->Dword6 &= set_desc(~ (0x0f << TX_MaxAggNumSHIFT));
#endif
			pdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & 0xffff0000) |
				TX_LastSeg | (txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
			pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
				(txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
		}
	}

	if(priv->pshare->rf_ft_var.txforce != 0xff)	{
		pdesc->Dword5 &= set_desc(~(TX_DataRateMask << TX_DataRateSHIFT));
		pdesc->Dword4 |= set_desc(TX_UseRate);
		pdesc->Dword5 |= set_desc((priv->pshare->rf_ft_var.txforce & TX_DataRateMask) << TX_DataRateSHIFT);
	}

#ifdef TX_EARLY_MODE
	if (GET_TX_EARLY_MODE) 
		pdesc->Dword8 = set_desc(get_physical_addr(priv, txcfg->phdr-8,
			(get_desc(pdesc->Dword7)& TX_TxBufSizeMask), PCI_DMA_TODEVICE));
	else
#endif
	pdesc->Dword8 = set_desc(get_physical_addr(priv, txcfg->phdr,
		(get_desc(pdesc->Dword7)& TX_TxBufSizeMask), PCI_DMA_TODEVICE));

	descinfo_copy(pdescinfo, &pstat->tx_sc_ent[idx].swdesc1);
#ifndef TXDESC_INFO
	pdescinfo->paddr  = get_desc(pdesc->Dword8); // buffer addr
#endif	
	if (txcfg->one_txdesc) {
		pdescinfo->type = _SKB_FRAME_TYPE_;
		pdescinfo->pframe = pskb;
		pdescinfo->priv = priv;
#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifndef TXDESC_INFO
		pdescinfo->pstat = pstat;
#endif		
#endif
	}
	else {
		pdescinfo->pframe = txcfg->phdr;
#if defined(WIFI_WMM) && defined(WMM_APSD)
		pdescinfo->priv = priv;
#ifndef TXDESC_INFO		
		pdescinfo->pstat = pstat;
#endif		
#endif
	}

#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE) {
		if (GetFrameSubType(pdescinfo->pframe) == WIFI_PSPOLL)
			pdesc->Dword1 |= set_desc(TX_NAVUSEHDR);

		if (priv->ps_state)
			SetPwrMgt(pdescinfo->pframe);
		else
			ClearPwrMgt(pdescinfo->pframe);
	}
#endif

	pfrst_dma_desc = dma_txhead[*tx_head];
/*
#ifdef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, get_desc(pdesc->Dword8), (get_desc(pdesc->Dword7)&TX_TxBufferSizeMask), PCI_DMA_TODEVICE);
#endif
*/
#ifdef OSK_LOW_TX_DESC
	if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
		txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
	else
#endif
	txdesc_rollover(pdesc, (unsigned int *)tx_head);

	if (txcfg->one_txdesc)
		goto one_txdesc;

	/*------------------------------------------------------------*/
	/*              fill descriptor of frame body                 */
	/*------------------------------------------------------------*/
#ifdef TX_SCATTER
next_desc:
#endif	
	pdesc = phdesc + *tx_head;
	pdescinfo = pswdescinfo + *tx_head;
#ifdef TX_SCATTER
	if (go_desc3)
		desc_copy(pdesc, &pstat->tx_sc_ent[idx].hwdesc3);
	else
#endif
		desc_copy(pdesc, &pstat->tx_sc_ent[idx].hwdesc2);

#ifdef TX_SCATTER
	if (pskb->list_num > 1) {
		if (go_desc3)
			pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
				pskb->list_buf[2].len);
		else
			pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
				pskb->list_buf[1].len);
	}
#endif

	pdesc->Dword8 = set_desc(get_physical_addr(priv, pskb->data,
		(get_desc(pdesc->Dword7)&0x0fff), PCI_DMA_TODEVICE));

#ifdef TX_SCATTER
	if (go_desc3) {
		descinfo_copy(pdescinfo, &pstat->tx_sc_ent[idx].swdesc3);
		pdescinfo->type = _RESERVED_FRAME_TYPE_;
	} else
#endif
	{
		descinfo_copy(pdescinfo, &pstat->tx_sc_ent[idx].swdesc2);
	}
#ifndef TXDESC_INFO
	pdescinfo->paddr  = get_desc(pdesc->Dword8);
#endif	
	pdescinfo->pframe = pskb;
	pdescinfo->priv = priv;
/*
#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, dma_txhead[*tx_head], sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#else
	rtl_cache_sync_wback(priv, get_desc(pdesc->Dword8), (get_desc(pdesc->Dword7)&TX_TxBufferSizeMask), PCI_DMA_TODEVICE);
#endif
*/

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	if (txcfg->privacy == _WAPI_SMS4_)
	{
		SecSWSMS4Encryption(priv, txcfg);
	}
#endif

#ifndef NOT_RTK_BSP
	if ((txcfg->privacy == _TKIP_PRIVACY_) &&
		(priv->pshare->have_hw_mic) &&
		!(priv->pmib->dot11StationConfigEntry.swTkipMic))
	{
		register unsigned long int l,r;
		unsigned char *mic;
		int delay = 18;

		while ((*(volatile unsigned int *)GDMAISR & GDMA_COMPIP) == 0) {
			delay_us(delay);
			delay = delay / 2;
		}

		l = *(volatile unsigned int *)GDMAICVL;
		r = *(volatile unsigned int *)GDMAICVR;

		mic = ((struct sk_buff *)txcfg->pframe)->data + txcfg->fr_len - 8;
		mic[0] = (unsigned char)(l & 0xff);
		mic[1] = (unsigned char)((l >> 8) & 0xff);
		mic[2] = (unsigned char)((l >> 16) & 0xff);
		mic[3] = (unsigned char)((l >> 24) & 0xff);
		mic[4] = (unsigned char)(r & 0xff);
		mic[5] = (unsigned char)((r >> 8) & 0xff);
		mic[6] = (unsigned char)((r >> 16) & 0xff);
		mic[7] = (unsigned char)((r >> 24) & 0xff);

#ifdef MICERR_TEST
		if (priv->micerr_flag) {
			mic[7] ^= mic[7];
			priv->micerr_flag = 0;
		}
#endif
	}
#endif // NOT_RTK_BSP

#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(dma_txhead[*tx_head]-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(get_desc(pdesc->Dword8)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), (get_desc(pdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE);

#ifdef OSK_LOW_TX_DESC
	if (q_num!=BE_QUEUE && q_num!=HIGH_QUEUE)
		txdesc_rollover_nonbe(pdesc, (unsigned int *)tx_head);
	else
#endif
	txdesc_rollover(pdesc, (unsigned int *)tx_head);

#ifdef TX_SCATTER
	if (pstat->tx_sc_ent[idx].has_desc3 && go_desc3 == 0) {
		go_desc3 = 1;
		goto next_desc;
	}
#endif
one_txdesc:

	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(get_desc(pfrstdesc->Dword8)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), (get_desc(pfrstdesc->Dword7)&TX_TxBufSizeMask), PCI_DMA_TODEVICE);

#ifdef SUPPORT_SNMP_MIB
	if (txcfg->rts_thrshld <= get_mpdu_len(txcfg, txcfg->fr_len))
		SNMP_MIB_INC(dot11RTSSuccessCount, 1);
#endif

	pfrstdesc->Dword0 |= set_desc(TX_OWN);

#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(pfrst_dma_desc-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

#ifdef RESERVE_TXDESC_FOR_EACH_IF
	if (RSVQ_ENABLE && IS_RSVQ(q_num))
		priv->use_txdesc_cnt[RSVQ(q_num)] += (txcfg->one_txdesc)? 1 : 2;
#endif

	if (q_num == HIGH_QUEUE) {
		DEBUG_WARN("signin shortcut for DTIM pkt?\n");
		return;
	} else {
		tx_poll(priv, q_num);
	}

	return;
}
#else
#if !defined(__OSK__) && !defined(CONFIG_RTL6028)
__MIPS16
#endif
__IRAM_IN_865X
void rtl8192cd_signin_txdesc_shortcut(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, int idx)
{
	return;
}
#endif//CONFIG_WLAN_NOT_HAL_EXIST
#endif // CONFIG_PCI_HCI
#endif // TX_SHORTCUT

#ifdef CONFIG_PCI_HCI
/* This sub-routine is gonna to check how many tx desc we need */
static int check_txdesc(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	struct sk_buff 	*pskb=NULL;
	unsigned short  protocol;
	unsigned char   *da=NULL;
	struct stat_info	*pstat=NULL;
	int priority=0;
	unsigned int is_dhcp = 0;

	if (txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE || txcfg->aggre_en == FG_AGGRE_MSDU_LAST)
		return TRUE;

	txcfg->privacy = txcfg->iv = txcfg->icv = txcfg->mic = 0;
	txcfg->frg_num = 0;
	txcfg->need_ack = 1;

	if (txcfg->fr_type == _SKB_FRAME_TYPE_)
	{
		pskb = ((struct sk_buff *)txcfg->pframe);
#ifdef TX_SCATTER
		if (pskb->list_num > 0)
			txcfg->fr_len = pskb->total_len - WLAN_ETHHDR_LEN;
		else
#endif
			txcfg->fr_len = pskb->len - WLAN_ETHHDR_LEN;

#ifdef MP_TEST
		if (OPMODE & WIFI_MP_STATE) {
			txcfg->hdr_len = WLAN_HDR_A3_LEN;
			txcfg->frg_num = 1;
			if (IS_MCAST(pskb->data))
				txcfg->need_ack = 0;
			return TRUE;
		}
#endif

#ifdef WDS
		if (txcfg->wdsIdx >= 0) {
			txcfg->hdr_len = WLAN_HDR_A4_LEN;
			pstat = get_stainfo(priv, priv->pmib->dot11WdsInfo.entry[txcfg->wdsIdx].macAddr);
			if (pstat == NULL) {
				DEBUG_ERR("TX DROP: %s: get_stainfo() for wds failed [%d]!\n", (char *)__FUNCTION__, txcfg->wdsIdx);
				return FALSE;
			}

			txcfg->privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
			switch (txcfg->privacy) {
				case _WEP_40_PRIVACY_:
				case _WEP_104_PRIVACY_:
					txcfg->iv = 4;
					txcfg->icv = 4;
					break;
				case _TKIP_PRIVACY_:
					txcfg->iv = 8;
					txcfg->icv = 4;
					txcfg->mic = 0;
					txcfg->fr_len += 8; // for Michael padding
					break;
				case _CCMP_PRIVACY_:
					txcfg->iv = 8;
					txcfg->icv = 0;
					txcfg->mic = 8;
					break;
			}
			txcfg->frg_num = 1;
			if (txcfg->aggre_en < FG_AGGRE_MSDU_FIRST) {
				priority = get_skb_priority(priv, (struct sk_buff *)txcfg->pframe, pstat);
#ifdef RTL_MANUAL_EDCA
				txcfg->q_num = PRI_TO_QNUM(priv, priority);
#else
				PRI_TO_QNUM(priority, txcfg->q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
			}

			txcfg->tx_rate = get_tx_rate(priv, pstat);
#ifndef TX_LOWESTRATE
			txcfg->lowest_tx_rate = get_lowest_tx_rate(priv, pstat, txcfg->tx_rate);
#endif
			if (priv->pmib->dot11WdsInfo.entry[pstat->wds_idx].txRate)
				txcfg->fixed_rate = 1;
			txcfg->need_ack = 1;
			txcfg->pstat = pstat;
#ifdef WIFI_WMM
			if ((pstat) && (QOS_ENABLE) && (pstat->QosEnabled) && (is_qos_data(txcfg->phdr)))
				txcfg->hdr_len = WLAN_HDR_A4_QOS_LEN;
#endif

			if (txcfg->aggre_en == 0) {
				if ((pstat->aggre_mthd == AGGRE_MTHD_MPDU) && is_MCS_rate(txcfg->tx_rate))
					txcfg->aggre_en = FG_AGGRE_MPDU;
			}

			return TRUE;
		}
#endif

#ifdef WIFI_WMM
		if (OPMODE & WIFI_AP_STATE) {
#ifdef MCAST2UI_REFINE
			pstat = get_stainfo(priv, &pskb->cb[10]);
#else
			pstat = get_stainfo(priv, pskb->data);
#endif
#ifdef A4_STA
			if (pstat == NULL) {
				pstat = txcfg->pstat;
			}
#endif
		}
		else if (OPMODE & WIFI_STATION_STATE)
			pstat = get_stainfo(priv, BSSID);
		else if (OPMODE & WIFI_ADHOC_STATE)
			pstat = get_stainfo(priv, pskb->data);

		if ((pstat) && (QOS_ENABLE) && (pstat->QosEnabled) && (is_qos_data(txcfg->phdr))) {
			txcfg->hdr_len = WLAN_HDR_A3_QOS_LEN;
		}
		else
#endif
		{
			txcfg->hdr_len = WLAN_HDR_A3_LEN;
		}

#ifdef CONFIG_RTK_MESH
		if(txcfg->is_11s)
		{
			txcfg->hdr_len = WLAN_HDR_A4_QOS_LEN ;
			da = txcfg->nhop_11s;
		}
		else
#endif

#ifdef MCAST2UI_REFINE
			da = &pskb->cb[10];
#else
			da = pskb->data;
#endif

#ifdef A4_STA
        if(priv->pshare->rf_ft_var.a4_enable && get_tofr_ds(txcfg->phdr) == 3) {
            if(pstat)
                da = pstat->hwaddr;
            txcfg->hdr_len += WLAN_ADDR_LEN;
        }
#endif

		//check if da is associated, if not, just drop and return false
		if (!IS_MCAST(da)
#ifdef CLIENT_MODE
			|| (OPMODE & WIFI_STATION_STATE)
#endif
			)
		{
#ifdef CLIENT_MODE
			if (OPMODE & WIFI_STATION_STATE)
				pstat = get_stainfo(priv, BSSID);
			else
#endif
			{
				pstat = get_stainfo(priv, da);
			}

			if ((pstat == NULL) || (!(pstat->state & WIFI_ASOC_STATE)))
			{
				DEBUG_ERR("TX DROP: Non asoc tx request!\n");
				return FALSE;
			}

			protocol = ntohs(*((UINT16 *)((UINT8 *)pskb->data + ETH_ALEN*2)));


			/*--decide privacy--*/
#ifdef CONFIG_RTL_WAPI_SUPPORT
			if (protocol == ETH_P_WAPI){
				txcfg->privacy = 0;
			}else
#endif
#if (BEAMFORMING_SUPPORT == 1)
			if(txcfg->ndpa){
				txcfg->privacy = 0;
			}else
#endif			
			if (protocol == 0x888e ){	/*EAP type packets*/


				/*handle EAP protocol =1 and EAP type=0 or =1 =>all EAP exchange packets for WPS
				   -  all EAP exchange packets for WPS doesn't need encrypt				   
				   -  for WPA/WPA2  4-ways handle shake 4-4 may need encrpty,TKIP type 2 ways need encrypt*/
				#ifdef WIFI_SIMPLE_CONFIG				   
				unsigned char   *EAP_VERSION=NULL;	
				unsigned char   *EAP_TYPE=NULL;		
				   
				EAP_VERSION = (  (UINT8 *)pskb->data + ETH_ALEN*2  + 2);
				EAP_TYPE = EAP_VERSION+1 ;
				
				if( *EAP_VERSION==1 && ( *EAP_TYPE==1 ||*EAP_TYPE==0)){
					txcfg->privacy = 0; /*all EAP exchange packets for WPS no need encrypt*/
				}else
				#endif
				{
				
					if( GET_UNICAST_ENCRYP_KEYLEN == 0){
						/*for AES  4-ways handle shake 4-1~4-4 no need encrypt*/
						txcfg->privacy = 0;
					}else{
					
						/*for TKIP  4-ways handle shake 4-4 ,2-1 , 2-2 need encrpty*/
						txcfg->privacy = get_privacy(priv, pstat, &txcfg->iv, &txcfg->icv, &txcfg->mic);
					}
				}
			}
			else{
				txcfg->privacy = get_privacy(priv, pstat, &txcfg->iv, &txcfg->icv, &txcfg->mic);
			}


			/*--decide privacy--*/

			if ((OPMODE & WIFI_AP_STATE) && !IS_MCAST(da) && pskb && (isDHCPpkt(pskb) == TRUE))
				is_dhcp++;

			if ((protocol == 0x888E)
#ifdef CONFIG_RTL_WAPI_SUPPORT
				||(protocol == ETH_P_WAPI)
#endif
				|| is_dhcp) {
				// use basic rate to send EAP packet for sure
				txcfg->tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
				txcfg->lowest_tx_rate = txcfg->tx_rate;
#endif
				txcfg->fixed_rate = 1;
			} else {
				txcfg->tx_rate = get_tx_rate(priv, pstat);
#ifndef TX_LOWESTRATE
				txcfg->lowest_tx_rate = get_lowest_tx_rate(priv, pstat, txcfg->tx_rate);
#endif
				if (!is_auto_rate(priv, pstat) &&
					!(should_restrict_Nrate(priv, pstat) && is_fixedMCSTxRate(priv, pstat)))
                    {            
#ifdef RTK_AC_SUPPORT
					if(! is_fixedVHTTxRate(priv, pstat) || (pstat->vht_cap_len))
#endif
					{
						txcfg->fixed_rate = 1;
					}
				}
			}

			if (txcfg->aggre_en < FG_AGGRE_MSDU_FIRST) {
#ifdef CONFIG_RTK_MESH
				priority = get_skb_priority3(priv, (struct sk_buff *)txcfg->pframe, txcfg->is_11s, pstat);
#else
				priority = get_skb_priority(priv, (struct sk_buff *)txcfg->pframe, pstat);
#endif
#ifdef RTL_MANUAL_EDCA
				txcfg->q_num = PRI_TO_QNUM(priv, priority);
#else
				PRI_TO_QNUM(priority, txcfg->q_num, priv->pmib->dot11OperationEntry.wifi_specific);
#endif
			}

#ifdef CONFIG_RTL_WAPI_SUPPORT
			if (protocol==ETH_P_WAPI)
			{
				txcfg->q_num = MGNT_QUEUE;
			}
#endif

			if (txcfg->aggre_en == 0
#ifdef SUPPORT_TX_MCAST2UNI
				&& (priv->pshare->rf_ft_var.mc2u_disable || (pskb->cb[2] != (char)0xff))
#endif
			   ) {
				if ((pstat->aggre_mthd & AGGRE_MTHD_MPDU) &&
					/*	is_MCS_rate(txcfg->tx_rate) &&*/ (protocol != 0x888E)
#ifdef CONFIG_RTL_WAPI_SUPPORT
					&& (protocol != ETH_P_WAPI)
#endif
					&& !is_dhcp)
					txcfg->aggre_en = FG_AGGRE_MPDU;
			}

#ifdef WMM_DSCP_C42
			if((IS_HAL_CHIP(priv)) && (txcfg->q_num==BE_QUEUE) && priv->pshare->iot_mode_VO_exist) {
				txcfg->q_num++;
			}
#endif
			if(
#ifdef RTK_AC_SUPPORT //FOR_VHT5G_PF
				( txcfg->pstat &&
				  ((txcfg->pstat->aggre_mthd == AGGRE_MTHD_MPDU_AMSDU) || (txcfg->pstat->aggre_mthd == AGGRE_MTHD_MPDU))
				  && txcfg->aggre_en ) ||
#endif
				(txcfg->aggre_en >= FG_AGGRE_MPDU && txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST)
			)
			{
				//panic_printk("%s %d pstat->ADDBA_ready[priority]=%d, priority=%d\n",__func__,__LINE__,pstat->ADDBA_ready[priority],priority);
				if (!pstat->ADDBA_ready[priority]) {
					if ((pstat->ADDBA_req_num[priority] < 5) && !pstat->ADDBA_sent[priority]) {
						pstat->ADDBA_req_num[priority]++;
						SMP_UNLOCK_XMIT(flags);
						issue_ADDBAreq(priv, pstat, priority);
						SMP_LOCK_XMIT(flags);
						pstat->ADDBA_sent[priority]++;
					}
				}
			}
		}
		else
		{
			// if group key not set yet, don't let unencrypted multicast go to air
			if (priv->pmib->dot11GroupKeysTable.dot11Privacy) {
				if (GET_GROUP_ENCRYP_KEYLEN == 0) {
					DEBUG_ERR("TX DROP: group key not set yet!\n");
					return FALSE;
				}
			}

			txcfg->privacy = get_mcast_privacy(priv, &txcfg->iv, &txcfg->icv, &txcfg->mic);
#if defined(CONFIG_WLAN_HAL)
			if (IS_HAL_CHIP(priv)) {
				if (OPMODE & WIFI_AP_STATE) {
#ifdef MBSSID
					if (IS_ROOT_INTERFACE(priv))
						txcfg->q_num = HIGH_QUEUE;
					else if (priv->vap_init_seq == 1)
						txcfg->q_num = HIGH_QUEUE1;
					else if (priv->vap_init_seq == 2)
						txcfg->q_num = HIGH_QUEUE2;
					else if (priv->vap_init_seq == 3)
						txcfg->q_num = HIGH_QUEUE3;
					else if (priv->vap_init_seq == 4)
						txcfg->q_num = HIGH_QUEUE4;
					else if (priv->vap_init_seq == 5)
						txcfg->q_num = HIGH_QUEUE5;
					else if (priv->vap_init_seq == 6)
						txcfg->q_num = HIGH_QUEUE6;
					else if (priv->vap_init_seq == 7)
						txcfg->q_num = HIGH_QUEUE7;
#else
					txcfg->q_num = HIGH_QUEUE;
#endif
					SetMData(txcfg->phdr);
				} else {
					// to do: sta mode?
					txcfg->q_num = BE_QUEUE;
					pskb->cb[1] = 0;
				}
			} else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif
			{
				//not HAL
				if ((OPMODE & WIFI_AP_STATE) && priv->pkt_in_dtimQ) {
					txcfg->q_num = MCAST_QNUM;
					SetMData(txcfg->phdr);
				}
				else {
					txcfg->q_num = BE_QUEUE;
					pskb->cb[1] = 0;
				}
			}

			if ((*da) == 0xff)	// broadcast
				txcfg->tx_rate = find_rate(priv, NULL, 0, 1);
			else {				// multicast
				if (priv->pmib->dot11StationConfigEntry.lowestMlcstRate)
					txcfg->tx_rate = get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.lowestMlcstRate);
				else
					txcfg->tx_rate = find_rate(priv, NULL, 1, 1);
			}

#ifndef TX_LOWESTRATE
			txcfg->lowest_tx_rate = txcfg->tx_rate;
#endif
			txcfg->fixed_rate = 1;
		}
	}

	if (!da)
	{
		// This is non data frame, no need to frag.
#ifdef CONFIG_RTK_MESH
		if(txcfg->is_11s)
			da = GetAddr1Ptr(txcfg->phdr);
		else
#endif
			da = get_da((unsigned char *) (txcfg->phdr));

		txcfg->frg_num = 1;

		if (IS_MCAST(da))
			txcfg->need_ack = 0;
		else
			txcfg->need_ack = 1;

		if (GetPrivacy(txcfg->phdr))
		{
#ifdef CONFIG_IEEE80211W 
			if(txcfg->isPMF) {
				txcfg->privacy = _CCMP_PRIVACY_;
				txcfg->iv = 8;
				txcfg->icv = 0;
				txcfg->mic = 8;
			} else
#endif
			{
				// only auth with legacy wep...
				txcfg->iv = 4;
				txcfg->icv = 4;
				txcfg->privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
			}
		}

#ifdef DRVMAC_LB
		if (GetFrameType(txcfg->phdr) == WIFI_MGT_TYPE)
#endif
			if (txcfg->fr_len != 0)	//for mgt frame
				txcfg->hdr_len += WLAN_HDR_A3_LEN;
	}

#ifdef CLIENT_MODE
	if (OPMODE & (WIFI_AP_STATE|WIFI_ADHOC_STATE) )
#else
	if (OPMODE & WIFI_AP_STATE)
#endif
	{
		if (IS_MCAST(da))
		{
			txcfg->frg_num = 1;
			txcfg->need_ack = 0;
			txcfg->rts_thrshld = 10000;
		}
		else if(!(txcfg->phdr && (GetFrameType(txcfg->phdr) == WIFI_MGT_TYPE) && (GetFrameSubType((unsigned char *) (txcfg->phdr))>>4 == 5)))	//  exclude probe rsp
		{
			pstat = get_stainfo(priv, da);
			txcfg->pstat = pstat;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		if ((txcfg->fr_type == _SKB_FRAME_TYPE_) ||							// skb frame
			!memcmp(GetAddr1Ptr(txcfg->phdr), BSSID, MACADDRLEN)) {		// mgt frame to AP
			pstat = get_stainfo(priv, BSSID);
			txcfg->pstat = pstat;
		}
	}
#endif

#if (BEAMFORMING_SUPPORT == 1)
	if((priv->pmib->dot11RFEntry.txbf == 1) && (pstat) &&
	   ((pstat->ht_cap_len && (pstat->ht_cap_buf.txbf_cap))
#ifdef RTK_AC_SUPPORT
		||(pstat->vht_cap_len && (cpu_to_le32(pstat->vht_cap_buf.vht_cap_info) & BIT(SU_BFEE_S)))
#endif
	   ))
		Beamforming_GidPAid(priv, pstat);
#endif
	if (txcfg->privacy == _TKIP_PRIVACY_)
		txcfg->fr_len += 8;	// for Michael padding.

	txcfg->frag_thrshld -= (txcfg->mic + txcfg->iv + txcfg->icv + txcfg->hdr_len);

	if (txcfg->frg_num == 0)
	{
		if (txcfg->aggre_en > 0)
			txcfg->frg_num = 1;
		else {
			// how many mpdu we need...
			int llc;

			if ((ntohs(*((UINT16 *)((UINT8 *)pskb->data + ETH_ALEN*2))) + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN)
				llc = sizeof(struct llc_snap);
			else
				llc = 0;

			txcfg->frg_num = (txcfg->fr_len + llc) / txcfg->frag_thrshld;
			if (((txcfg->fr_len + llc) % txcfg->frag_thrshld) != 0)
				txcfg->frg_num += 1;
		}
	}

#ifdef TX_SCATTER
	if (pskb && pskb->list_num > 0 && txcfg->frg_num > 1) {
		struct sk_buff *newskb = copy_skb(pskb);
		if (newskb == NULL) {
			DEBUG_ERR("TX DROP: Can't copy the skb for list buffer in frag!\n");
			return FALSE;
		}
		dev_kfree_skb_any(pskb);
		txcfg->pframe = (void *)newskb;
	}
#endif

	return TRUE;
}
#endif // CONFIG_PCI_HCI

/*-------------------------------------------------------------------------------
tx flow:
	Please refer to design spec for detail flow

rtl8192cd_firetx: check if hw desc is available for tx
				hdr_len, iv,icv, tx_rate info are available

signin_txdesc: fillin the desc and txpoll is necessary
--------------------------------------------------------------------------------*/
int __rtl8192cd_firetx(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
	int					*tx_head=NULL, *tx_tail=NULL, q_num;
	unsigned int		val32=0, is_dhcp=0;
	struct sk_buff		*pskb=NULL;
	struct llc_snap		*pllc_snap;
	struct wlan_ethhdr_t	*pethhdr=NULL;
#ifdef SUPPORT_TX_AMSDU
	struct wlan_ethhdr_t	*pmsdu_hdr;
	struct wlan_ethhdr_t  pethhdr_data;
#endif
	struct rtl8192cd_hw	*phw = GET_HW(priv);
#ifdef CONFIG_WLAN_HAL
#ifndef TRXBD_CACHABLE_REGION
//	PHCI_TX_DMA_MANAGER_88XX	ptx_dma;
#endif	
#endif

	unsigned long		x;
    unsigned char *da;
#if defined(CONFIG_RTL_WAPI_SUPPORT)
	struct wlan_ethhdr_t	ethhdr;
	pethhdr = &ethhdr;
#endif

#ifdef PCIE_POWER_SAVING_TEST // yllin
    if((priv->pwr_state >= L2) || (priv->pwr_state == L1)) { 
        return FAIL; 
    }
#endif

#ifdef CONFIG_32K  //tingchu
           if (priv->offload_32k_flag==1){
            printk("32k rtl8192cd_fire_tx return\n");
            return FAIL;
            }
#endif


	int bUnAvail = FALSE;

#ifdef CONFIG_PCI_HCI
	/*---frag_thrshld setting---plus tune---0115*/
#ifdef	WDS
	if (txcfg->wdsIdx >= 0){
		txcfg->frag_thrshld = 2346; // if wds, disable fragment
	}else
#endif
#ifdef CONFIG_RTK_MESH
	if(txcfg->is_11s){
		txcfg->frag_thrshld = 2346; // if Mesh case, disable fragment
	}else
#endif
	{
		txcfg->frag_thrshld = FRAGTHRSLD - _CRCLNG_;
	}
	/*---frag_thrshld setting---end*/

	txcfg->rts_thrshld  = RTSTHRSLD;
	txcfg->frg_num = 0;

#ifdef DFS
	if (!priv->pmib->dot11DFSEntry.disable_DFS &&
		GET_ROOT(priv)->pmib->dot11DFSEntry.disable_tx
		&& (priv->site_survey)
		&& (priv->site_survey->hidden_ap_found != HIDE_AP_FOUND_DO_ACTIVE_SSAN)) {
		priv->ext_stats.tx_drops++;
		DEBUG_ERR("TX DROP: DFS probation period\n");

		if (txcfg->fr_type == _SKB_FRAME_TYPE_) {
			rtl_kfree_skb(priv, (struct sk_buff *)txcfg->pframe, _SKB_TX_);
			release_wlanllchdr_to_poll(priv, txcfg->phdr);
		} else if (txcfg->fr_type == _PRE_ALLOCMEM_) {
			release_mgtbuf_to_poll(priv, txcfg->pframe);
			release_wlanhdr_to_poll(priv, txcfg->phdr);
		} else if (txcfg->fr_type == _PRE_ALLOCHDR_) {
			release_wlanhdr_to_poll(priv, txcfg->phdr);
		}
#if (BEAMFORMING_SUPPORT == 1)
		else if (txcfg->ndpa) {
			release_wlanllchdr_to_poll(priv, txcfg->phdr);
		}
#endif

		return SUCCESS;
	}
#endif

	if ((check_txdesc(priv, txcfg)) == FALSE) // this will only happen in errorous forwarding
	{
		priv->ext_stats.tx_drops++;
        DEBUG_ERR("TX DROP: check_txdesc Fail \n");
		// Don't need to print "TX DROP", already print in check_txdesc()
		if (txcfg->fr_type == _SKB_FRAME_TYPE_) {
			rtl_kfree_skb(priv, (struct sk_buff *)txcfg->pframe, _SKB_TX_);
			release_wlanllchdr_to_poll(priv, txcfg->phdr);
		}
		else if (txcfg->fr_type == _PRE_ALLOCMEM_) {
			release_mgtbuf_to_poll(priv, txcfg->pframe);
			release_wlanhdr_to_poll(priv, txcfg->phdr);
		} else if (txcfg->fr_type == _PRE_ALLOCHDR_) {
			release_wlanhdr_to_poll(priv, txcfg->phdr);
		}
		return SUCCESS;
	}

	if (txcfg->aggre_en > 0)
		txcfg->frag_thrshld = 2346;

	q_num = txcfg->q_num;
	
	if (txcfg->aggre_en < FG_AGGRE_MSDU_FIRST) {
		// now we are going to calculate how many hw desc we should have before tx...
		// wlan_hdr(including iv and llc) will occupy one desc, payload will occupy one, and
		// icv/mic will occupy the third desc if swcrypto is utilized.
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        val32 = txcfg->frg_num;
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
		{//not HAL
			{
				tx_head = get_txhead_addr(phw, q_num);
				tx_tail = get_txtail_addr(phw, q_num);
			}
		

			if (txcfg->privacy)
			{
				if (
	#if defined(CONFIG_RTL_WAPI_SUPPORT)
						(_WAPI_SMS4_ != txcfg->privacy) &&
	#endif
#ifdef CONFIG_IEEE80211W
			UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE), txcfg->isPMF))
#else
			UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
#endif	

					val32 = txcfg->frg_num * 3;  //extra one is for ICV padding.
				else
					val32 = txcfg->frg_num * 2;
			}
			else {
				val32 = txcfg->frg_num * 2;
			}

#ifdef TX_SCATTER
			if (txcfg->fr_type == _SKB_FRAME_TYPE_ &&
					((struct sk_buff *)txcfg->pframe)->key > 0 &&  
						((struct sk_buff *)txcfg->pframe)->list_num > 1)
				val32 += ((struct sk_buff *)txcfg->pframe)->list_num -1;
#endif
		}

#ifdef RESERVE_TXDESC_FOR_EACH_IF
		if(RSVQ_ENABLE && IS_RSVQ(q_num)) {
			int num_consumed =
#ifdef CONFIG_WLAN_HAL
				IS_HAL_CHIP(priv) ? txcfg->frg_num :
#endif
				val32;
			bUnAvail = check_rsv_txdesc(priv, q_num, num_consumed);
		} else
#endif
		{
#ifdef CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv))
				bUnAvail = compareAvailableTXBD(priv, val32+1, q_num, 2);
			else
#endif
#ifdef OSK_LOW_TX_DESC
				bUnAvail = ((val32 + 2) > CIRC_SPACE_RTK(*tx_head, *tx_tail, ((q_num==BE_QUEUE || q_num==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC))); //per mpdu, we need 2 desc...
#else
				bUnAvail = ((val32 + 2) > CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC)); //per mpdu, we need 2 desc...
#endif
		}

		if (bUnAvail) {
#ifdef USE_TXQUEUE
			if (txcfg->fr_type == _SKB_FRAME_TYPE_ && BUFQ_ENABLE && priv->pshare->iot_mode_enable) {
				if (priv->pshare->txq_isr) {
					append_skb_to_txq_head(&priv->pshare->txq_list[RSVQ(q_num)], priv, txcfg->pframe, priv->dev, &priv->pshare->txq_pool);
					priv->pshare->txq_stop = 1;
				} else {
					append_skb_to_txq_tail(&priv->pshare->txq_list[RSVQ(q_num)], priv, txcfg->pframe, priv->dev, &priv->pshare->txq_pool);
				}
				if (txcfg->phdr)
					release_wlanllchdr_to_poll(priv, txcfg->phdr);
				priv->pshare->txq_check = 1;
				//SMP_UNLOCK_XMIT(x);
				return SUCCESS;
			}
			else
#endif
			{
				// Marked by SC, may cause crash.
				//DEBUG_ERR("%d hw Queue desc not available! head=%d, tail=%d request %d\n",q_num,*tx_head,*tx_tail,val32);
				//SMP_UNLOCK_XMIT(x);
				return CONGESTED;
			}
		}
	}

	// then we have to check if wlan-hdr is available for usage...
	// actually, the checking can be void

	/* ----------
				Actually, I believe the check below is redundant.
				Since at this moment, desc is available, hdr/icv/
				should be enough.
													--------------*/
	val32 = txcfg->frg_num;

	if (val32 >= priv->pshare->pwlan_hdr_poll->count)
	{
		DEBUG_ERR("%d hw Queue tx without enough wlan_hdr\n", q_num);
		return CONGESTED;
	}
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	switch( rtw_xmit_decision(priv, txcfg) ) {
	case XMIT_DECISION_ENQUEUE:
		return SUCCESS;
	case XMIT_DECISION_STOP:
		return CONGESTED;
	case XMIT_DECISION_CONTINUE:
		break;
	}
	
	if (update_txinsn_stage2(priv, txcfg) == FALSE) {
		return CONGESTED;
	}
	
	q_num = txcfg->q_num;
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI

	// then we have to check if wlan_snapllc_hdrQ is enough for use
	// for each msdu, we need wlan_snapllc_hdrQ for maximum

	if (txcfg->fr_type == _SKB_FRAME_TYPE_)
	{
		pskb = (struct sk_buff *)txcfg->pframe;

#ifdef CONFIG_PCI_HCI
		if ((OPMODE & WIFI_AP_STATE) && pskb && (isDHCPpkt(pskb) == TRUE))
			is_dhcp++;
#endif

#ifdef SUPPORT_TX_AMSDU
		// for AMSDU
		if (txcfg->aggre_en >= FG_AGGRE_MSDU_FIRST) {
			unsigned short usLen;
			int msdu_len;

			memcpy(&pethhdr_data, pskb->data, sizeof(struct wlan_ethhdr_t));
			pethhdr = &pethhdr_data;
			msdu_len = pskb->len - WLAN_ETHHDR_LEN;
			if ((ntohs(pethhdr->type) + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) {
				if (skb_headroom(pskb) < sizeof(struct llc_snap)) {
					struct sk_buff *skb2 = dev_alloc_skb(pskb->len);
					if (skb2 == NULL) {
						printk("%s: %s, dev_alloc_skb() failed!\n", priv->dev->name, __FUNCTION__);
						rtl_kfree_skb(priv, pskb, _SKB_TX_);
						if(txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
							release_wlanllchdr_to_poll(priv, txcfg->phdr);
						return 0;
					}
					memcpy(skb_put(skb2, pskb->len), pskb->data, pskb->len);
					dev_kfree_skb_any(pskb);
					pskb = skb2;
					txcfg->pframe = (void *)pskb;
				}
				skb_push(pskb, sizeof(struct llc_snap));
			}
			pmsdu_hdr = (struct wlan_ethhdr_t *)pskb->data;

			memcpy(pmsdu_hdr, pethhdr, 12);
			if ((ntohs(pethhdr->type) + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) {
				usLen = (unsigned short)(msdu_len + sizeof(struct llc_snap));
				pmsdu_hdr->type = htons(usLen);
				eth_2_llc(pethhdr, (struct llc_snap *)(((unsigned long)pmsdu_hdr)+sizeof(struct wlan_ethhdr_t)));
			}
			else {
				usLen = (unsigned short)msdu_len;
				pmsdu_hdr->type = htons(usLen);
			}

			if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
				eth2_2_wlanhdr(priv, pethhdr, txcfg);

			txcfg->llc = 0;
			pllc_snap = NULL;

			if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST || txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE) {
				if ((usLen+14) % 4)
					skb_put(pskb, 4-((usLen+14)%4));
			}
			txcfg->fr_len = pskb->len;
		}
		else 
#endif /* SUPPORT_TX_AMSDU */
		{	// not A-MSDU
#if defined(CONFIG_SDIO_HCI) && defined(TX_SCATTER)
			// in case 1st buffer len is 14, we get ether header pointer first and then ajust the skb
			pethhdr = (struct wlan_ethhdr_t *)(pskb->data);
			// now, we should adjust the skb ...
			skb_pull(pskb, WLAN_ETHHDR_LEN);
#else
			// now, we should adjust the skb ...
			skb_pull(pskb, WLAN_ETHHDR_LEN);
#ifdef CONFIG_RTK_MESH
            pethhdr = &priv->ethhdr;
			memcpy(pethhdr, pskb->data - sizeof(struct wlan_ethhdr_t), sizeof(struct wlan_ethhdr_t));
#else
			pethhdr = (struct wlan_ethhdr_t *)(pskb->data - sizeof(struct wlan_ethhdr_t));
#endif
#endif
			pllc_snap = (struct llc_snap *)((UINT8 *)(txcfg->phdr) + txcfg->hdr_len + txcfg->iv);

			if ((ntohs(pethhdr->type) + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) {
				eth_2_llc(pethhdr, pllc_snap);
				txcfg->llc = sizeof(struct llc_snap);
			}
			else
			{
				pllc_snap = NULL;
			}

			eth2_2_wlanhdr(priv, pethhdr, txcfg);

#ifdef CONFIG_PCI_HCI
#ifdef TX_SCATTER
			if (pskb->len == 0 && pskb->list_num > 1) {
				pskb->list_idx++;
				skb_assign_buf(pskb, pskb->list_buf[pskb->list_idx].buf, pskb->list_buf[pskb->list_idx].len);
				pskb->len = pskb->list_buf[pskb->list_idx].len;
			}
#endif
#endif

#ifdef CONFIG_RTK_MESH
			if(txcfg->is_11s&1 && GetFrameSubType(txcfg->phdr) == WIFI_11S_MESH)
			{
				const short meshhdrlen= (txcfg->mesh_header.mesh_flag & 0x01) ? 16 : 4;
				if (skb_cloned(pskb))
				{
					struct sk_buff	*newskb = skb_copy(pskb, GFP_ATOMIC);
					rtl_kfree_skb(priv, pskb, _SKB_TX_);
					if (newskb == NULL) {
						priv->ext_stats.tx_drops++;
						release_wlanllchdr_to_poll(priv, txcfg->phdr);
						DEBUG_ERR("TX DROP: Can't copy the skb!\n");
						return SUCCESS;
					}
					txcfg->pframe = pskb = newskb;
#ifdef ENABLE_RTL_SKB_STATS
					rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif
				}
				memcpy(skb_push(pskb,meshhdrlen), &(txcfg->mesh_header), meshhdrlen);
				txcfg->fr_len += meshhdrlen;
			}
#endif // CONFIG_RTK_MESH
		}

		// TODO: modify as when skb is not bigger enough, take ICV from local pool
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if (txcfg->privacy == _WAPI_SMS4_)
		{
			if ((pskb->tail + SMS4_MIC_LEN) > pskb->end)
			{
				struct sk_buff *pnewskb;
				
				pnewskb = skb_copy_expand(pskb, skb_headroom(pskb), SMS4_MIC_LEN, GFP_ATOMIC);
				if (NULL == pnewskb) {
					priv->ext_stats.tx_drops++;
					DEBUG_ERR("TX DROP: an over size skb!\n");
					rtl_kfree_skb(priv, (struct sk_buff *)txcfg->pframe, _SKB_TX_);
					release_wlanllchdr_to_poll(priv, txcfg->phdr);
					return SUCCESS;
				}
				
				dev_kfree_skb_any(pskb);
				pskb = pnewskb;
				txcfg->pframe = pskb;
#ifndef CONFIG_RTK_MESH
				if (txcfg->aggre_en < FG_AGGRE_MSDU_FIRST)
					pethhdr = (struct wlan_ethhdr_t *)(pskb->data - sizeof(struct wlan_ethhdr_t));
#endif
			}
			memcpy(&ethhdr, pethhdr, sizeof(struct wlan_ethhdr_t));
#ifdef MCAST2UI_REFINE
                        memcpy(&ethhdr.daddr, &pskb->cb[10], 6);
#endif
			pethhdr = &ethhdr;
		} else
#endif

		if (txcfg->privacy == _TKIP_PRIVACY_)
		{
			if ((pskb->tail + 8) > pskb->end)
			{
#if !defined(NOT_RTK_BSP) || defined(__ECOS) || defined(__OSK__)
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: an over size skb!\n");
				rtl_kfree_skb(priv, (struct sk_buff *)txcfg->pframe, _SKB_TX_);
				release_wlanllchdr_to_poll(priv, txcfg->phdr);
				return SUCCESS;
				
#else // NOT_RTK_BSP
				struct sk_buff *pnewskb;
				
				pnewskb = skb_copy_expand(pskb, skb_headroom(pskb), 8, GFP_ATOMIC);
				if (NULL == pnewskb) {
					priv->ext_stats.tx_drops++;
					DEBUG_ERR("TX DROP: an over size skb!\n");
					rtl_kfree_skb(priv, (struct sk_buff *)txcfg->pframe, _SKB_TX_);
					release_wlanllchdr_to_poll(priv, txcfg->phdr);
					return SUCCESS;
				}
				
				dev_kfree_skb_any(pskb);
				pskb = pnewskb;
				txcfg->pframe = pskb;
#ifndef CONFIG_RTK_MESH
				if (txcfg->aggre_en < FG_AGGRE_MSDU_FIRST)
					pethhdr = (struct wlan_ethhdr_t *)(pskb->data - sizeof(struct wlan_ethhdr_t));
#endif
#endif
			}

            da = pethhdr->daddr;
#ifdef MCAST2UI_REFINE
            memcpy(pethhdr->daddr, &pskb->cb[10], 6);
#endif

#ifdef A4_STA
            if(txcfg->pstat && (txcfg->pstat->state & WIFI_A4_STA)) {
                da = GetAddr3Ptr(txcfg->phdr);
            }
#endif

#ifdef WIFI_WMM
			if ((tkip_mic_padding(priv, da, pethhdr->saddr, ((QOS_ENABLE) && (txcfg->pstat) && (txcfg->pstat->QosEnabled))?pskb->cb[1]:0, (UINT8 *)pllc_snap,
					pskb, txcfg)) == FALSE)
#else
			if ((tkip_mic_padding(priv, da, pethhdr->saddr, 0, (UINT8 *)pllc_snap,
					pskb, txcfg)) == FALSE)
#endif
			{
				priv->ext_stats.tx_drops++;
				DEBUG_ERR("TX DROP: Tkip mic padding fail!\n");
				rtl_kfree_skb(priv, pskb, _SKB_TX_);
				release_wlanllchdr_to_poll(priv, txcfg->phdr);
				return SUCCESS;
			}
			if ((txcfg->aggre_en < FG_AGGRE_MSDU_FIRST) || (txcfg->aggre_en == FG_AGGRE_MSDU_LAST))
				skb_put((struct sk_buff *)txcfg->pframe, 8);
		}
	}

	if (txcfg->privacy && txcfg->aggre_en <= FG_AGGRE_MSDU_FIRST)
		SetPrivacy(txcfg->phdr);

#ifdef DONT_COUNT_PROBE_PACKET
	if (txcfg->pstat && GetFrameSubType(txcfg->phdr) == WIFI_PROBERSP) {
		txcfg->pstat->tx_probe_rsp_pkts++;
	}
#endif

	// log tx statistics...
	tx_sum_up(priv, txcfg->pstat, txcfg);

	SNMP_MIB_INC(dot11TransmittedFragmentCount, 1);

	// for SW LED
	if (txcfg->aggre_en > FG_AGGRE_MSDU_FIRST || GetFrameType(txcfg->phdr) == WIFI_DATA_TYPE) {
		priv->pshare->LED_tx_cnt++;
	} else {
		if (priv->pshare->LED_cnt_mgn_pkt)
			priv->pshare->LED_tx_cnt++;
	}

#ifdef PCIE_POWER_SAVING
	PCIeWakeUp(priv, (POWER_DOWN_T0));
#endif

	SAVE_INT_AND_CLI(x);
	//SMP_LOCK_XMIT(x);
	
#ifdef SUPPORT_TX_AMSDU
	if (txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE || txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {
#ifdef CONFIG_WLAN_HAL
            if (IS_HAL_CHIP(priv)) {
                rtl88XX_signin_txdesc_amsdu(priv, txcfg);
            } else 
#endif
            {
	        rtl8192cd_signin_txdesc_amsdu(priv, txcfg);
	    }
	} else
#endif
	{

#if defined(CONFIG_PCI_HCI)
#ifdef CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {
				// TODO: these if conditions are all the same as the following code, it should be optimzed.....
				if((!priv->pmib->dot11OperationEntry.disable_txsc) &&
					(txcfg->fr_type == _SKB_FRAME_TYPE_) &&
					(txcfg->pstat) &&
					(txcfg->frg_num == 1) &&
					((txcfg->privacy == 0)	
#ifdef CONFIG_IEEE80211W
						|| (!UseSwCrypto(priv, txcfg->pstat, FALSE, txcfg->isPMF))) &&
#else
						|| (!UseSwCrypto(priv, txcfg->pstat, FALSE))) &&
#endif
					(pethhdr->type != htons(0x888e)) &&
					!is_dhcp && 
					!GetMData(txcfg->phdr) &&
					#ifdef MCAST2UI_REFINE
	                pskb && !IS_MCAST(&pskb->cb[10]) &&
					#else
					!IS_MCAST((unsigned char *)pethhdr) &&
					#endif
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
					(txcfg->aggre_en <=FG_AGGRE_MSDU_FIRST)
#else
					(txcfg->aggre_en < FG_AGGRE_MSDU_FIRST)
#endif
				) {				
					rtl88XX_signin_txdesc(priv, txcfg, HW_TX_SC_BACKUP_HEADER);
				} else {
					rtl88XX_signin_txdesc(priv, txcfg, HW_TX_SC_NORMAL);
				}
				
			} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif		
			{//not HAL
				rtl8192cd_signin_txdesc(priv, txcfg);
			}
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			rtl8192cd_signin_txdesc(priv, txcfg, pethhdr);
#endif

	}

	//SMP_UNLOCK_XMIT(x);
	RESTORE_INT(x);

#ifdef CONFIG_PCI_HCI
#ifdef TX_SHORTCUT
	if ((!priv->pmib->dot11OperationEntry.disable_txsc) &&
		(txcfg->fr_type == _SKB_FRAME_TYPE_) &&
		(txcfg->pstat) &&
		(txcfg->frg_num == 1) &&
		((txcfg->privacy == 0)
#ifdef CONFIG_RTL_WAPI_SUPPORT
		|| (txcfg->privacy == _WAPI_SMS4_)
#endif
#ifdef CONFIG_IEEE80211W
		|| !UseSwCrypto(priv, txcfg->pstat, FALSE, txcfg->isPMF)) &&
#else
		|| !UseSwCrypto(priv, txcfg->pstat, FALSE)) &&
#endif
		/*(!IEEE8021X_FUN ||
			(IEEE8021X_FUN && (txcfg->pstat->ieee8021x_ctrlport == 1) &&
			(pethhdr->type != htons(0x888e)))) && */
		(pethhdr->type != htons(0x888e)) &&
		!is_dhcp && 
#ifdef CONFIG_RTL_WAPI_SUPPORT
		(pethhdr->type != htons(ETH_P_WAPI)) &&
#endif
		!GetMData(txcfg->phdr) &&
#ifdef A4_STA		
		((txcfg->pstat && txcfg->pstat->state & WIFI_A4_STA) 
        #ifdef MCAST2UI_REFINE
            ||(pskb && !IS_MCAST(&pskb->cb[10])))&& 
        #else
			||!IS_MCAST((unsigned char *)pethhdr)) &&
        #endif	
#else
#ifdef MCAST2UI_REFINE
        (
#ifdef WDS
	(txcfg->pstat->state & WIFI_WDS)? (!IS_BCAST2((unsigned char *)pethhdr)) : 
#endif
		(pskb && !IS_MCAST(&pskb->cb[10])))&&                
#else
        (
#ifdef WDS
        (txcfg->pstat->state & WIFI_WDS)? (!IS_BCAST2((unsigned char *)pethhdr)) : 
#endif  
        (!IS_MCAST((unsigned char *)pethhdr))) &&
#endif
#endif
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
		(txcfg->aggre_en <= FG_AGGRE_MSDU_FIRST)
#else
		(txcfg->aggre_en < FG_AGGRE_MSDU_FIRST)
#endif
		)
	{
		int i = get_tx_sc_index(priv, txcfg->pstat, (unsigned char *)pethhdr);
		if (i >= 0) {		
			memcpy(&txcfg->pstat->tx_sc_ent[i].txcfg, txcfg, sizeof(struct tx_insn));
			memcpy((void *)&txcfg->pstat->tx_sc_ent[i].wlanhdr, txcfg->phdr, sizeof(struct wlanllc_hdr));
		}
	}
	else {
		if (txcfg->pstat && pethhdr 
#ifdef SUPPORT_TX_AMSDU_SHORTCUT
			&& txcfg->aggre_en <= FG_AGGRE_MSDU_FIRST
#endif
			) {
			int i = get_tx_sc_index(priv, txcfg->pstat, (unsigned char *)pethhdr);
			if (i >= 0) {
				txcfg->pstat->tx_sc_ent[i].txcfg.fr_len = 0;
#ifdef TX_SCATTER
				txcfg->pstat->tx_sc_ent[i].has_desc3 = 0;
#endif
			}
		}
	}
#endif
#endif // CONFIG_PCI_HCI

	return SUCCESS;
}

#ifdef CONFIG_PCI_HCI
int rtl8192cd_firetx(struct rtl8192cd_priv *priv, struct tx_insn* txcfg)
{
#ifdef RX_TASKLET
//#ifndef SMP_SYNC
	unsigned long x;
	int locked=0;	
//#endif
	int ret;

	SAVE_INT_AND_CLI(x);
#ifdef SMP_SYNC				
	SMP_TRY_LOCK_XMIT(x,locked);
#endif

	ret = __rtl8192cd_firetx(priv, txcfg);

#ifdef SMP_SYNC				
	if(locked)
		SMP_UNLOCK_XMIT(x);
#endif
	RESTORE_INT(x);
	return ret;
#else
	return (__rtl8192cd_firetx(priv, txcfg));
#endif
}
#endif // CONFIG_PCI_HCI


#ifdef CONFIG_PCI_HCI
#ifdef CONFIG_WLAN_HAL
static int 
rtl88XX_tx_recycle(
    struct rtl8192cd_priv   *priv, 
    unsigned int            txRingIdx, 
    int                     *recycleCnt_p
)
{
	int	                        cnt = 0;
	struct tx_desc_info         *pdescinfoH, *pdescinfo;
    // TODO: int or u2Byte ?
	//volatile int	            head, tail;
    volatile u2Byte             head, tail;
	int				            needRestartQueue = 0;
	int				            recycleCnt = 0;

#ifdef PCIE_POWER_SAVING_TEST // yllin
    if((priv->pwr_state >= L2) || (priv->pwr_state == L1)) {
        return 0; 
    }
#endif

#ifdef CONFIG_32K //tingchu
            if(priv->offload_32k_flag==1){
                printk("32k rtl8192cd_tx_recycle return\n");
                return 0;
            }
               
#endif


    
    unsigned int                halQnum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, txRingIdx);
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
    head = GET_HAL_INTERFACE(priv)->GetTxQueueHWIdxHandler(priv, txRingIdx);
    tail = ptx_dma->tx_queue[halQnum].hw_idx;

	pdescinfoH	= get_txdesc_info(priv->pshare->pdesc_info, txRingIdx);

	while (CIRC_CNT_RTK(head, tail, ptx_dma->tx_queue[halQnum].total_txbd_num))
	{
		pdescinfo = pdescinfoH + (tail);

#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
		if (IS_PCIBIOS_TYPE) {
			if (_RESERVED_FRAME_TYPE_ != pdescinfo->type) {
				pci_unmap_single(priv->pshare->pdev, pdescinfo->paddr, pdescinfo->len, PCI_DMA_TODEVICE);
			}
		}
#endif

		if (pdescinfo->type == _SKB_FRAME_TYPE_)
		{
#ifdef MP_TEST 
			if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
                #if 0
                printk("skb_tail: 0x%x, skb_head: 0x%x\n", 
                                priv->pshare->skb_tail,
                                priv->pshare->skb_head
                                );
                #endif
                
				struct sk_buff *skb = (struct sk_buff *)(pdescinfo->pframe);
				skb->data = skb->head;
				skb->tail = skb->data;
				skb->len = 0;
				priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
			}
			else
#endif
			{
#if defined(__LINUX_2_6__) || defined(__OSK__)
				if (pdescinfo->pframe)
					rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				else
					priv->ext_stats.freeskb_err++;
#else
// for debug ------------
//				rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				if (pdescinfo->pframe) {
    				if (((struct sk_buff *)pdescinfo->pframe)->list) {
    					DEBUG_ERR("Free tx skb error, skip it!\n");
    					priv->ext_stats.freeskb_err++;
    				}
    				else {
    					rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
    				}
				}
#endif
				needRestartQueue = 1;
			}
		}
		else if (pdescinfo->type == _PRE_ALLOCMEM_)
		{
			release_mgtbuf_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCHDR_)
		{
			release_wlanhdr_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCLLCHDR_)
		{
			release_wlanllchdr_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCICVHDR_)
		{
			release_icv_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCMICHDR_)
		{
			release_mic_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _RESERVED_FRAME_TYPE_)
		{
			// the chained skb, no need to release memory
		}
		else
		{
			DEBUG_ERR("Unknown tx frame type %d\n", pdescinfo->type);
		}

        //Reset to default value
        pdescinfo->type     = _RESERVED_FRAME_TYPE_;
		// for skb buffer free
		pdescinfo->pframe   = NULL;

		recycleCnt++;

		for (cnt =0; cnt<TXBD_ELE_NUM-2; cnt++) {

#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
			if (IS_PCIBIOS_TYPE) {
				if (_RESERVED_FRAME_TYPE_ != pdescinfo->buf_type[cnt]) {
					//use the paddr and flen of pdesc field for icv, mic case which doesn't fill the pdescinfo
					pci_unmap_single(priv->pshare->pdev,
									 pdescinfo->buf_paddr[cnt],//payload
									 pdescinfo->buf_len[cnt],
									 PCI_DMA_TODEVICE);
				}
			}
#endif
			//if (txRingIdx == 2)
			//panic_printk("buf type[%d]=%d\n", cnt, pdescinfo->buf_type[cnt]);

			if (pdescinfo->buf_type[cnt] == _SKB_FRAME_TYPE_)
			{
#ifdef MP_TEST
				if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
					struct sk_buff *skb = (struct sk_buff *)(pdescinfo->buf_pframe[cnt]);
					skb->data = skb->head;
					skb->tail = skb->data;
					skb->len = 0;
					priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
				}
				else
#endif
				{
#if defined(__LINUX_2_6__) || defined(__OSK__)
					if (pdescinfo->buf_pframe[cnt])
						rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->buf_pframe[cnt]), _SKB_TX_IRQ_);
					else
						priv->ext_stats.freeskb_err++;
#else
// for debug ------------
//					rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
					if (pdescinfo->buf_pframe[cnt]) {
					if (((struct sk_buff *)pdescinfo->buf_pframe[cnt])->list) {
						DEBUG_ERR("Free tx skb error, skip it!\n");
						priv->ext_stats.freeskb_err++;
					}
					else
						rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->buf_pframe[cnt]), _SKB_TX_IRQ_);
					}
#endif
					needRestartQueue = 1;
				}
			}
			else if (pdescinfo->buf_type[cnt] == _PRE_ALLOCMEM_)
			{
				release_mgtbuf_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe[cnt]));
			}
			else if (pdescinfo->buf_type[cnt] == _PRE_ALLOCHDR_)
			{
				release_wlanhdr_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe[cnt]));
			}
			else if (pdescinfo->buf_type[cnt] == _PRE_ALLOCLLCHDR_)
			{
				release_wlanllchdr_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe[cnt]));
			}
			else if (pdescinfo->buf_type[cnt] == _PRE_ALLOCICVHDR_)
			{
				release_icv_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe[cnt]));
			}
			else if (pdescinfo->buf_type[cnt] == _PRE_ALLOCMICHDR_)
			{
				release_mic_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe[cnt]));
			}
			else if (pdescinfo->buf_type[cnt] == _RESERVED_FRAME_TYPE_)
			{
				// the chained skb, no need to release memory
			}
			else
			{
				DEBUG_ERR("Unknown tx frame type %d:%d\n", pdescinfo->buf_type[cnt]);
			}

            //Reset to default value
            pdescinfo->buf_type[cnt]    = _RESERVED_FRAME_TYPE_;
			// for skb buffer free
            pdescinfo->buf_pframe[cnt]  = NULL;

			recycleCnt ++;
		}

#ifdef WLAN_HAL_TX_AMSDU
		for (cnt =0; cnt<WLAN_HAL_TX_AMSDU_MAX_NUM; cnt++) {
			//if (txRingIdx == 2)
			//panic_printk("buf type[%d]=%d\n", cnt, pdescinfo->buf_type[cnt]);

			if (pdescinfo->buf_type_amsdu[cnt] == _SKB_FRAME_TYPE_)
			{
#ifdef MP_TEST
				if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
					struct sk_buff *skb = (struct sk_buff *)(pdescinfo->buf_pframe[cnt]);
					skb->data = skb->head;
					skb->tail = skb->data;
					skb->len = 0;
					priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
				}
				else
#endif
				{
#if defined(__LINUX_2_6__) || defined(__OSK__)
					if(pdescinfo->buf_pframe_amsdu[cnt])
						rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->buf_pframe_amsdu[cnt]), _SKB_TX_IRQ_);
					else
						priv->ext_stats.freeskb_err++;
#else
// for debug ------------
//					rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
					if (pdescinfo->buf_pframe_amsdu[cnt]) {
					if (((struct sk_buff *)pdescinfo->buf_pframe_amsdu[cnt])->list) {
						DEBUG_ERR("Free tx skb error, skip it!\n");
						priv->ext_stats.freeskb_err++;
					}
					else
						rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->buf_pframe_amsdu[cnt]), _SKB_TX_IRQ_);
					}
#endif
					needRestartQueue = 1;
				}
			}
#if 0
			else if (pdescinfo->buf_type_amsdu[cnt] == _PRE_ALLOCMEM_)
			{
				release_mgtbuf_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe_amsdu[cnt]));
			}
			else if (pdescinfo->buf_type_amsdu[cnt] == _PRE_ALLOCHDR_)
			{
				release_wlanhdr_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe_amsdu[cnt]));
			}
			else if (pdescinfo->buf_type_amsdu[cnt] == _PRE_ALLOCLLCHDR_)
			{
				release_wlanllchdr_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe_amsdu[cnt]));
			}
			else if (pdescinfo->buf_type_amsdu[cnt] == _PRE_ALLOCICVHDR_)
			{
				release_icv_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe_amsdu[cnt]));
			}
			else if (pdescinfo->buf_type_amsdu[cnt] == _PRE_ALLOCMICHDR_)
			{
				release_mic_to_poll(priv, (UINT8 *)(pdescinfo->buf_pframe_amsdu[cnt]));
			}
#endif
			else if (pdescinfo->buf_type_amsdu[cnt] == _RESERVED_FRAME_TYPE_)
			{
				// the chained skb, no need to release memory
			}
			else
			{
			    printk("%s(%d): Unknow tx frame type for AMSDU: %d \n", __FUNCTION__, __LINE__, pdescinfo->buf_type_amsdu[cnt]);
				DEBUG_ERR("Unknown tx frame type %d:%d\n", pdescinfo->buf_type_amsdu[cnt]);
			}

            //Reset to default value
            pdescinfo->buf_type_amsdu[cnt]    = _RESERVED_FRAME_TYPE_;
			// for skb buffer free
            pdescinfo->buf_pframe_amsdu[cnt]  = NULL;

			recycleCnt ++;
		}
#endif // WLAN_HAL_TX_AMSDU
    	tail = (tail + 1) % (ptx_dma->tx_queue[halQnum].total_txbd_num);
        ptx_dma->tx_queue[halQnum].avail_txbd_num++;

#ifdef RESERVE_TXDESC_FOR_EACH_IF
		if (RSVQ_ENABLE && IS_RSVQ(txRingIdx))
			pdescinfo->priv->use_txdesc_cnt[RSVQ(txRingIdx)]--;
#endif

		if (head == tail)
			head = GET_HAL_INTERFACE(priv)->GetTxQueueHWIdxHandler(priv, txRingIdx);
	}


    ptx_dma->tx_queue[halQnum].hw_idx = tail;

	if (recycleCnt_p)
		*recycleCnt_p = recycleCnt;

	return needRestartQueue;
}
#endif

/*
	Procedure to re-cycle TXed packet in Queue index "txRingIdx"

	=> Return value means if system need restart-TX-queue or not.

		1: Need Re-start Queue
		0: Don't Need Re-start Queue
*/

static int rtl8192cd_tx_recycle(struct rtl8192cd_priv *priv, unsigned int txRingIdx, int *recycleCnt_p)
{
	struct tx_desc	*pdescH, *pdesc;
	struct tx_desc_info *pdescinfoH, *pdescinfo;
	volatile int	head, tail;
	struct rtl8192cd_hw	*phw=GET_HW(priv);
	int				needRestartQueue=0;
	int				recycleCnt=0;
#ifdef CONFIG_WLAN_HAL
    if (IS_HAL_CHIP(priv)) {
        return rtl88XX_tx_recycle(priv, txRingIdx, recycleCnt_p);
    } else if (CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL
	{//not HAL
	head		= get_txhead(phw, txRingIdx);
	tail		= get_txtail(phw, txRingIdx);
	pdescH		= get_txdesc(phw, txRingIdx);
	pdescinfoH	= get_txdesc_info(priv->pshare->pdesc_info, txRingIdx);

#ifdef OSK_LOW_TX_DESC
	while (CIRC_CNT_RTK(head, tail, ((txRingIdx==BE_QUEUE || txRingIdx==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC)))
#else
	while (CIRC_CNT_RTK(head, tail, CURRENT_NUM_TX_DESC))
#endif
	{
		pdesc = pdescH + (tail);
		pdescinfo = pdescinfoH + (tail);

#ifdef __MIPSEB__
		pdesc = (struct tx_desc *)KSEG1ADDR(pdesc);
#endif

		if (!pdesc || (get_desc(pdesc->Dword0) & TX_OWN))
			break;
		if (pdescinfo->pframe == NULL) {
			DEBUG_ERR("Free Null Buf: head=%d tail=%d recycleCnt=%d RingIdx=%d!\n", head, tail, recycleCnt, txRingIdx);
		}

#ifdef CONFIG_PCI_HCI
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
		if (IS_PCIBIOS_TYPE) {
			//use the paddr and flen of pdesc field for icv, mic case which doesn't fill the pdescinfo
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B)	
			pci_unmap_single(priv->pshare->pdev,
							 get_desc(pdesc->Dword10),
							 (get_desc(pdesc->Dword7)&0xffff),
							 PCI_DMA_TODEVICE);
			else
#endif
			pci_unmap_single(priv->pshare->pdev,
							 get_desc(pdesc->Dword8),
							 (get_desc(pdesc->Dword7)&0xffff),
							 PCI_DMA_TODEVICE);
		}
#endif
#endif // CONFIG_PCI_HCI

		if (pdescinfo->type == _SKB_FRAME_TYPE_)
		{
#ifdef MP_TEST
			if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
				struct sk_buff *skb = (struct sk_buff *)(pdescinfo->pframe);
				skb->data = skb->head;
				skb->tail = skb->data;
				skb->len = 0;
				priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
			}
			else
#endif
			{
#if defined(__LINUX_2_6__) || defined(__OSK__)
#ifdef CONFIG_ENABLE_NCBUFF
				rtl_kfree_skb_spec(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
#else 
				if (pdescinfo->pframe)
					rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				else
					priv->ext_stats.freeskb_err++;
#endif
#else
// for debug ------------
//				rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				if (pdescinfo->pframe) {
				if (((struct sk_buff *)pdescinfo->pframe)->list) {
					DEBUG_ERR("Free tx skb error, skip it!\n");
					priv->ext_stats.freeskb_err++;
				}
				else
					rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				}
#endif
				needRestartQueue = 1;
			}
		}
		else if (pdescinfo->type == _PRE_ALLOCMEM_)
		{
			release_mgtbuf_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCHDR_)
		{
			release_wlanhdr_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCLLCHDR_)
		{
			release_wlanllchdr_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCICVHDR_)
		{
			release_icv_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _PRE_ALLOCMICHDR_)
		{
			release_mic_to_poll(priv, (UINT8 *)(pdescinfo->pframe));
		}
		else if (pdescinfo->type == _RESERVED_FRAME_TYPE_)
		{
			// the chained skb, no need to release memory
		}
		else
		{
			DEBUG_ERR("Unknown tx frame type %d\n", pdescinfo->type);
		}

		// for skb buffer free
		pdescinfo->type = _RESERVED_FRAME_TYPE_;
		pdescinfo->pframe = NULL;

		recycleCnt ++;

#ifdef OSK_LOW_TX_DESC
		tail = (tail + 1) % ((txRingIdx==BE_QUEUE || txRingIdx==HIGH_QUEUE)?BE_TXDESC:NONBE_TXDESC);
#else
		tail = (tail + 1) % CURRENT_NUM_TX_DESC;
#endif

#ifdef RESERVE_TXDESC_FOR_EACH_IF
		if (RSVQ_ENABLE && IS_RSVQ(txRingIdx))
			pdescinfo->priv->use_txdesc_cnt[RSVQ(txRingIdx)]--;
#endif
	}

	*get_txtail_addr(phw, txRingIdx) = tail;

	if (recycleCnt_p)
		*recycleCnt_p = recycleCnt;

	return needRestartQueue;
	} //not HAL
#ifdef CONFIG_WLAN_HAL
	else {
		panic_printk("Shouldn't come here %s()\n", __FUNCTION__);
		return 0;
	}
#endif
}


#ifdef  CONFIG_WLAN_HAL
static void rtl88XX_tx_dsr(unsigned long task_priv)
{
	struct rtl8192cd_priv	*priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int	        j=0;
	unsigned int	        restart_queue=0;
	struct rtl8192cd_hw	    *phw=GET_HW(priv);
	int                     needRestartQueue;
    int                     Queue_max = HIGH_QUEUE;
	unsigned long           flags;

	if (!phw)
		return;
    
#ifdef PCIE_POWER_SAVING_TEST //yllin
    if((priv->pwr_state >= L2) || (priv->pwr_state == L1)) {
        priv->pshare->has_triggered_tx_tasklet = 0;
            return;
    }
#endif

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) {
		priv->pshare->has_triggered_tx_tasklet = 0;
		return;
	}
#endif

#ifdef CONFIG_32K //tingchu
    if(priv->offload_32k_flag==1){
        priv->pshare->has_triggered_tx_tasklet = 0;
        printk("32k rtl88XX_tx_dsr return\n");
            return;
    }
#endif


#ifdef MBSSID
    if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) 
        Queue_max = HIGH_QUEUE7;      
#endif


	SAVE_INT_AND_CLI(flags);
	SMP_LOCK_XMIT(flags);

	for(j=0; j<=Queue_max; j++)
	{
		needRestartQueue = rtl8192cd_tx_recycle(priv, j, NULL);
		/* If anyone of queue report the TX queue need to be restart : we would set "restart_queue" to process ALL queues */
		if (needRestartQueue == 1)
			restart_queue = 1;
	}

	if (restart_queue)
		rtl8192cd_tx_restartQueue(priv);

#ifdef MP_TEST
#if 1//def CONFIG_RTL8672
	if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND|WIFI_MP_CTX_BACKGROUND_STOPPING))
			==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))
#else //CONFIG_RTL8672
	if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) 
#endif //CONFIG_RTL8672
	{
#ifdef CONFIG_WLAN_HAL
		PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
#endif

		RESTORE_INT(flags);

#ifdef CONFIG_WLAN_HAL
        ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);

        if (compareAvailableTXBD(priv, (CURRENT_NUM_TX_DESC/2), BE_QUEUE, 1)) {
			SMP_UNLOCK_XMIT(flags);
            mp_ctx(priv, (unsigned char *)"tx-isr");
			SMP_LOCK_XMIT(flags);
        }
#else
		int *tx_head, *tx_tail;

		tx_head = get_txhead_addr(phw, BE_QUEUE);
		tx_tail = get_txtail_addr(phw, BE_QUEUE);
		if (CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC) > (CURRENT_NUM_TX_DESC/2))
		{
			SMP_UNLOCK_XMIT(flags);
			mp_ctx(priv, (unsigned char *)"tx-isr");
			SMP_LOCK_XMIT(flags);
		}
#endif //CONFIG_WLAN_HAL

		SAVE_INT_AND_CLI(flags);
	}
#endif

	refill_skb_queue(priv);

#ifdef USE_TXQUEUE
    // TODO: HAL
	if (BUFQ_ENABLE && !priv->pshare->txq_isr && priv->pshare->txq_check)
	{
		int q_num, send_cnt = 0;
		priv->pshare->txq_isr = 1;
		
		for (q_num=VO_QUEUE; q_num>=BK_QUEUE; q_num--)
		{
			priv->pshare->txq_stop = 0;
			while ( txq_len(&priv->pshare->txq_list[RSVQ(q_num)]) > 0 )
			{
				struct sk_buff *tmp_skb = NULL;
				struct net_device *dev = NULL;
				remove_skb_from_txq(&priv->pshare->txq_list[RSVQ(q_num)], &tmp_skb, &dev, &priv->pshare->txq_pool);
				if (tmp_skb && dev) {
						((struct rtl8192cd_priv*)(dev->priv))->use_txq_cnt[RSVQ(q_num)]--;
#ifdef ENABLE_RTL_SKB_STATS
					rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif
					__rtl8192cd_start_xmit(tmp_skb, dev, TX_NORMAL);
					send_cnt++;
					if (priv->pshare->txq_stop) break;
				}
			}
		}
		
		priv->pshare->txq_isr = 0;

		if (send_cnt == 0)
			priv->pshare->txq_check = 0;
	}
#endif

	priv->pshare->has_triggered_tx_tasklet = 0;

	RESTORE_INT(flags);
	SMP_UNLOCK_XMIT(flags);
}
#endif  //CONFIG_WLAN_HAL


/*-----------------------------------------------------------------------------
Purpose of tx_dsr:

	For ALL TX queues
		1. Free Allocated Buf
		2. Update tx_tail
		3. Update tx related counters
		4. Restart tx queue if needed
------------------------------------------------------------------------------*/
#ifdef __OSK__
__IRAM_WIFI_PRI3
#endif
void rtl8192cd_tx_dsr(unsigned long task_priv)
{
	struct rtl8192cd_priv	*priv = (struct rtl8192cd_priv *)task_priv;
	unsigned int	j=0;
	unsigned int	restart_queue=0;
	struct rtl8192cd_hw	*phw=GET_HW(priv);
	int needRestartQueue;
	unsigned long flags;
#ifdef CONFIG_WLAN_HAL
    if(IS_HAL_CHIP(priv)){
        rtl88XX_tx_dsr((unsigned long)priv);
        return;
    }else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif //CONFIG_WLAN_HAL
	{//not HAL
	if (!phw)
		return;

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1)) {
		priv->pshare->has_triggered_tx_tasklet = 0;
		return;
	}
#endif
#ifdef PCIE_POWER_SAVING_TEST
    if ((priv->pwr_state >= L2) || (priv->pwr_state == L1)) {
        priv->pshare->has_triggered_tx_tasklet = 0;
        return;
    }
#endif

#ifdef CONFIG_32K //tingchu
            if (priv->offload_32k_flag==1) {
                priv->pshare->has_triggered_tx_tasklet = 0;
                printk("32k rtl8192cd_tx_dsr return\n");
                return;
            }
            
#endif


	SAVE_INT_AND_CLI(flags);
	SMP_LOCK_XMIT(flags);
	for(j=0; j<=HIGH_QUEUE; j++)
	{
		needRestartQueue = rtl8192cd_tx_recycle(priv, j, NULL);
		/* If anyone of queue report the TX queue need to be restart : we would set "restart_queue" to process ALL queues */
		if (needRestartQueue == 1)
			restart_queue = 1;
	}

	if (restart_queue)
		rtl8192cd_tx_restartQueue(priv);

#ifdef MP_TEST
#if 1//def CONFIG_RTL8672
	if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND|WIFI_MP_CTX_BACKGROUND_STOPPING))
			==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))
#else //CONFIG_RTL8672
	if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) 
#endif //CONFIG_RTL8672
	{
		int *tx_head, *tx_tail;
		RESTORE_INT(flags);
		tx_head = get_txhead_addr(phw, BE_QUEUE);
		tx_tail = get_txtail_addr(phw, BE_QUEUE);
		// OSK: because BE_Queue = CURRENT_NUM_TX_DESC, so do not need to change
		if (CIRC_SPACE_RTK(*tx_head, *tx_tail, CURRENT_NUM_TX_DESC) > (CURRENT_NUM_TX_DESC/2))
		{
			SMP_UNLOCK_XMIT(flags);
			mp_ctx(priv, (unsigned char *)"tx-isr");
		    SMP_LOCK_XMIT(flags);
	    }
		SAVE_INT_AND_CLI(flags);
	}
#endif

	refill_skb_queue(priv);

#ifdef USE_TXQUEUE
	if (BUFQ_ENABLE && !priv->pshare->txq_isr && priv->pshare->txq_check)
	{
		int q_num, send_cnt = 0, tx_flag = TX_NORMAL;
		priv->pshare->txq_isr = 1;
		
		for (q_num=VO_QUEUE; q_num>=BK_QUEUE; q_num--)
		{
			priv->pshare->txq_stop = 0;
			while ( txq_len(&priv->pshare->txq_list[RSVQ(q_num)]) > 0 )
			{
				struct sk_buff *tmp_skb = NULL;
				struct net_device *dev = NULL;
				remove_skb_from_txq(&priv->pshare->txq_list[RSVQ(q_num)], &tmp_skb, &dev, &priv->pshare->txq_pool);
				if (tmp_skb && dev) {
						((struct rtl8192cd_priv*)(dev->priv))->use_txq_cnt[RSVQ(q_num)]--;
#ifdef SUPPORT_TX_MCAST2UNI
					tx_flag = tmp_skb->cb[16];
#endif
#ifdef ENABLE_RTL_SKB_STATS
					rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif
					__rtl8192cd_start_xmit(tmp_skb, dev, tx_flag);
					send_cnt++;
					if (priv->pshare->txq_stop) break;
				}
			}
		}
		
		priv->pshare->txq_isr = 0;

		if (send_cnt == 0)
			priv->pshare->txq_check = 0;
	}
#endif

	priv->pshare->has_triggered_tx_tasklet = 0;

	RESTORE_INT(flags);
	SMP_UNLOCK_XMIT(flags);
}
}


/*
	Try to do TX-DSR for only ONE TX-queue ( rtl8192cd_tx_dsr would check for ALL TX queue )
*/
int rtl8192cd_tx_queueDsr(struct rtl8192cd_priv *priv, unsigned int txRingIdx)
{
	int recycleCnt;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	SAVE_INT_AND_CLI(flags);

	if (rtl8192cd_tx_recycle(priv, txRingIdx, &recycleCnt) == 1)
		rtl8192cd_tx_restartQueue(priv);

	RESTORE_INT(flags);
	return recycleCnt;
}


/*
	Procedure to restart TX Queue
*/
void rtl8192cd_tx_restartQueue(struct rtl8192cd_priv *priv)
{
#if defined(__KERNEL__) || defined(__OSK__)
	priv = GET_ROOT(priv);

	if (IS_DRV_OPEN(priv) && netif_queue_stopped(priv->dev)) {
		DEBUG_INFO("wake-up Root queue\n");
		netif_wake_queue(priv->dev);
	}

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)) && netif_queue_stopped(GET_VXD_PRIV(priv)->dev)) {
		DEBUG_INFO("wake-up VXD queue\n");
		netif_wake_queue(GET_VXD_PRIV(priv)->dev);
	}
#endif

#ifdef MBSSID
	if (priv->pmib->miscEntry.vap_enable) {
		int bssidIdx;
		for (bssidIdx=0; bssidIdx<RTL8192CD_NUM_VWLAN; bssidIdx++) {
			if (IS_DRV_OPEN(priv->pvap_priv[bssidIdx]) && netif_queue_stopped(priv->pvap_priv[bssidIdx]->dev)) {
				DEBUG_INFO("wake-up Vap%d queue\n", bssidIdx);
				netif_wake_queue(priv->pvap_priv[bssidIdx]->dev);
			}
		}
	}
#endif

#ifdef CONFIG_RTK_MESH
    if(priv->mesh_dev) {
		if (netif_running(priv->mesh_dev) &&
				netif_queue_stopped(priv->mesh_dev) )
		{
			netif_wake_queue(priv->mesh_dev);
		}
    }
#endif
#ifdef WDS
	if (priv->pmib->dot11WdsInfo.wdsEnabled) {
		int i;
		for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
			if (netif_running(priv->wds_dev[i]) &&
				netif_queue_stopped(priv->wds_dev[i])) {
				DEBUG_INFO("wake-up wds[%d] queue\n", i);
				netif_wake_queue(priv->wds_dev[i]);
			}
		}
	}
#endif
#endif
}
#endif // CONFIG_PCI_HCI


static int tkip_mic_padding(struct rtl8192cd_priv *priv,
				unsigned char *da, unsigned char *sa, unsigned char priority,
				unsigned char *llc, struct sk_buff *pskb, struct tx_insn* txcfg)
{
	// now check what's the mic key we should apply...

	unsigned char	*mickey = NULL;
	unsigned int	keylen = 0;
	struct stat_info	*pstat = NULL;
	unsigned char	*hdr, hdr_buf[16];
	unsigned int	num_blocks;
	unsigned char	tkipmic[8];
	unsigned char	*pbuf=pskb->data;
	unsigned int	len=pskb->len;
	unsigned int	llc_len = 0;

	// check if the mic/tkip key is valid at this moment.

	if (((txcfg->aggre_en < FG_AGGRE_MSDU_FIRST) || (txcfg->aggre_en == FG_AGGRE_MSDU_LAST)) && (pskb->tail + 8) > (pskb->end))
	{
		DEBUG_ERR("pskb have no extra room for TKIP Michael padding\n");
		return FALSE;
	}

#ifdef WDS
	if (txcfg->wdsIdx >= 0) {
		pstat = get_stainfo(priv, priv->pmib->dot11WdsInfo.entry[txcfg->wdsIdx].macAddr);
		if (pstat) {
			keylen = GET_UNICAST_MIC_KEYLEN;
			mickey = GET_UNICAST_TKIP_MIC1_KEY;
		}
	}
	else
#endif
	if (OPMODE & WIFI_AP_STATE)
	{

#ifdef CONFIG_RTK_MESH

		if(txcfg->is_11s)

			da = txcfg->nhop_11s;

#endif
		if (IS_MCAST(da))
		{
			if(priv->pmib->dot11GroupKeysTable.keyid == GKEY_ID_SECOND) {
				keylen = GET_GROUP_IDX2_MIC_KEYLEN;
				mickey = GET_GROUP_TKIP_IDX2_MIC1_KEY;
			} else {
				keylen = GET_GROUP_MIC_KEYLEN;
				mickey = GET_GROUP_TKIP_MIC1_KEY;
			}
#ifdef A4_STA			
			if (txcfg->pstat && (txcfg->pstat->state & WIFI_A4_STA)) {
				pstat = txcfg->pstat;
				keylen = GET_UNICAST_MIC_KEYLEN;
				mickey = GET_UNICAST_TKIP_MIC1_KEY;
			}
#endif			
		}
		else
		{
			pstat = get_stainfo (priv, da);
#ifdef A4_STA			
			if (priv->pshare->rf_ft_var.a4_enable && (pstat == NULL)) 
				pstat = a4_sta_lookup(priv, da);
#endif		
			if (pstat == NULL) {
				DEBUG_ERR("tx mic pstat == NULL\n");
				return FALSE;
			}

			keylen = GET_UNICAST_MIC_KEYLEN;
			mickey = GET_UNICAST_TKIP_MIC1_KEY;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		pstat = get_stainfo (priv, BSSID);
		if (pstat == NULL) {
			DEBUG_ERR("tx mic pstat == NULL\n");
			return FALSE;
		}

		keylen = GET_UNICAST_MIC_KEYLEN;
		mickey = GET_UNICAST_TKIP_MIC2_KEY;
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		keylen = GET_GROUP_MIC_KEYLEN;
		mickey = GET_GROUP_TKIP_MIC1_KEY;
	}
#endif

	if ((txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE) || (txcfg->aggre_en == FG_AGGRE_MSDU_LAST))
		mickey = pstat->tmp_mic_key;

	if (keylen == 0)
	{
		DEBUG_ERR("no mic padding for TKIP due to keylen=0\n");
		return FALSE;
	}

	if (txcfg->aggre_en <= FG_AGGRE_MSDU_FIRST) {
		hdr = hdr_buf;
		memcpy((void *)hdr, (void *)da, WLAN_ADDR_LEN);
		memcpy((void *)(hdr + WLAN_ADDR_LEN), (void *)sa, WLAN_ADDR_LEN);
		hdr[12] = priority;
		hdr[13] = hdr[14] = hdr[15] = 0;
	}
	else
		hdr = NULL;

	pbuf[len] = 0x5a;   /* Insert padding */
	pbuf[len+1] = 0x00;
	pbuf[len+2] = 0x00;
	pbuf[len+3] = 0x00;
	pbuf[len+4] = 0x00;
	pbuf[len+5] = 0x00;
	pbuf[len+6] = 0x00;
	pbuf[len+7] = 0x00;

	if (llc)
		llc_len = 8;
	num_blocks = (16 + llc_len + len + 5) / 4;
	if ((16 + llc_len + len + 5) & (4-1))
		num_blocks++;

	if (txcfg->aggre_en >= FG_AGGRE_MSDU_FIRST) {
		if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
			num_blocks = (16 + len) / 4;
			if ((16 + len) & (4-1))
				num_blocks++;
		}
		else if (txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE) {
			num_blocks = len / 4;
			if (len & (4-1))
				num_blocks++;
		}
		else if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {
			num_blocks = (len + 5) / 4;
			if ((len + 5) & (4-1))
				num_blocks++;
		}
	}

	michael(priv, mickey, hdr, llc, pbuf, (num_blocks << 2), tkipmic, 1);

	//tkip mic is MSDU-based, before filled-in descriptor, already finished.
#ifdef SUPPORT_TKIP_OSK_GDMA
	//always get tkipmic in michael function
#else
	if (!(priv->pshare->have_hw_mic) ||
		(priv->pmib->dot11StationConfigEntry.swTkipMic))
#endif
	{
#ifdef MICERR_TEST
		if (priv->micerr_flag) {
			tkipmic[7] ^= tkipmic[7];
			priv->micerr_flag = 0;
		}
#endif
		if ((txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) || (txcfg->aggre_en == FG_AGGRE_MSDU_MIDDLE)) {
			memcpy((void *)pstat->tmp_mic_key, (void *)tkipmic, 8);
		}
		else
			memcpy((void *)(pbuf + len), (void *)tkipmic, 8);
	}

	return TRUE;
}


void wep_fill_iv(struct rtl8192cd_priv *priv,
				unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid)
{
	unsigned char *iv = pwlhdr + hdrlen;
	union PN48 *ptsc48 = NULL;
	union PN48 auth_pn48;
	unsigned char *da;
	struct stat_info *pstat = NULL;

	memset(&auth_pn48, 0, sizeof(union PN48));
	da = get_da(pwlhdr);

#if defined(WDS) || defined(A4_STA)
	if (get_tofr_ds(pwlhdr) == 3)
		da = GetAddr1Ptr(pwlhdr);
#endif

	if (OPMODE & WIFI_AP_STATE)
	{
		if (IS_MCAST(da))
		{
			ptsc48 = GET_GROUP_ENCRYP_PN;
		}
		else
		{
			pstat = get_stainfo(priv, da);
			ptsc48 = GET_UNICAST_ENCRYP_PN;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		pstat = get_stainfo(priv, BSSID);
		if (pstat != NULL)
			ptsc48 = GET_UNICAST_ENCRYP_PN;
		else
			ptsc48 = &auth_pn48;
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
		ptsc48 = GET_GROUP_ENCRYP_PN;
#endif

	if (ptsc48 == NULL)
	{
		DEBUG_ERR("no TSC for WEP due to ptsc48=NULL\n");
		return;
	}

	iv[0] = ptsc48->_byte_.TSC0;
	iv[1] = ptsc48->_byte_.TSC1;
	iv[2] = ptsc48->_byte_.TSC2;
	iv[3] = 0x0 | (keyid << 6);

	if (ptsc48->val48 == 0xffffffffffffULL)
		ptsc48->val48 = 0;
	else
		ptsc48->val48++;
}


void tkip_fill_encheader(struct rtl8192cd_priv *priv,
				unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid_out)
{
	unsigned char *iv = pwlhdr + hdrlen;
	union PN48 *ptsc48 = NULL;
	unsigned int keyid = 0;
	unsigned char *da;
	struct stat_info *pstat = NULL;

	da = get_da(pwlhdr);

	if (OPMODE & WIFI_AP_STATE)
	{
#if defined(WDS) || defined(CONFIG_RTK_MESH) || defined(A4_STA)
		unsigned int to_fr_ds = (GetToDs(pwlhdr) << 1) | GetFrDs(pwlhdr);
		if (to_fr_ds == 3)
			da = GetAddr1Ptr(pwlhdr);
#endif

		if (IS_MCAST(da))
		{
			ptsc48 = GET_GROUP_ENCRYP_PN;
			keyid = priv->pmib->dot11GroupKeysTable.keyid;
		}
		else
		{
			pstat = get_stainfo(priv, da);
			ptsc48 = GET_UNICAST_ENCRYP_PN;
			keyid = 0;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		pstat = get_stainfo(priv, BSSID);
		ptsc48 = GET_UNICAST_ENCRYP_PN;
		keyid = 0;
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		ptsc48 = GET_GROUP_ENCRYP_PN;
		keyid = 0;
	}
#endif

#ifdef __DRAYTEK_OS__
	keyid = keyid_out;
#endif

	if (ptsc48 == NULL)
	{
		DEBUG_ERR("no TSC for TKIP due to ptsc48=NULL\n");
		return;
	}

	iv[0] = ptsc48->_byte_.TSC1;
	iv[1] = (iv[0] | 0x20) & 0x7f;
	iv[2] = ptsc48->_byte_.TSC0;
	iv[3] = 0x20 | (keyid << 6);
	iv[4] = ptsc48->_byte_.TSC2;
	iv[5] = ptsc48->_byte_.TSC3;
	iv[6] = ptsc48->_byte_.TSC4;
	iv[7] = ptsc48->_byte_.TSC5;

	if (ptsc48->val48 == 0xffffffffffffULL)
		ptsc48->val48 = 0;
	else
		ptsc48->val48++;
}


#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
#ifndef WIFI_MIN_IMEM_USAGE
__IRAM_IN_865X
#endif
void aes_fill_encheader(struct rtl8192cd_priv *priv,
				unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid)
{
	unsigned char *da;
	struct stat_info *pstat = NULL;
	union PN48 *pn48 = NULL;

#ifdef __DRAYTEK_OS__
	int	keyid_input = keyid;
#endif

	da = get_da(pwlhdr);

	if (OPMODE & WIFI_AP_STATE)
	{
#if defined(WDS) || defined(CONFIG_RTK_MESH) || defined(A4_STA)
		unsigned int to_fr_ds = (GetToDs(pwlhdr) << 1) | GetFrDs(pwlhdr);
		if (to_fr_ds == 3)
			da = GetAddr1Ptr(pwlhdr);
#endif


		if (IS_MCAST(da))
		{
			pn48 = GET_GROUP_ENCRYP_PN;
			keyid = priv->pmib->dot11GroupKeysTable.keyid;
		}
		else
		{
			pstat = get_stainfo(priv, da);
			if(NULL == pstat) {
				DEBUG_ERR("no such station\n");
				DEBUG_ERR("da:%02x-%02x-%02x-%02x-%02x-%02x\n",
					da[0],da[1],da[2],da[3],da[4],da[5]);
				return;
			}
			pn48 = GET_UNICAST_ENCRYP_PN;
			keyid = 0;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE)
	{
		pstat = get_stainfo(priv, BSSID);
		if(NULL == pstat) {
			DEBUG_ERR("no such station\n");
			return;
		}
		pn48 = GET_UNICAST_ENCRYP_PN;
		keyid = 0;
	}
	else if (OPMODE & WIFI_ADHOC_STATE)
	{
		pn48 = GET_GROUP_ENCRYP_PN;
		keyid = 0;
	}
#endif

	if (pn48 == NULL)
	{
		DEBUG_ERR("no TSC for AES due to pn48=NULL\n");
		return;
	}

#ifdef __DRAYTEK_OS__
	keyid = keyid_input;
#endif

	pwlhdr[hdrlen]   = pn48->_byte_.TSC0;
	pwlhdr[hdrlen+1] = pn48->_byte_.TSC1;
	pwlhdr[hdrlen+2] =  0x00;
	pwlhdr[hdrlen+3] = (0x20 | (keyid << 6));
	pwlhdr[hdrlen+4] = pn48->_byte_.TSC2;
	pwlhdr[hdrlen+5] = pn48->_byte_.TSC3;
	pwlhdr[hdrlen+6] = pn48->_byte_.TSC4;
	pwlhdr[hdrlen+7] = pn48->_byte_.TSC5;

   	if (pn48->val48 == 0xffffffffffffULL)
		pn48->val48 = 0;
	else
		pn48->val48++;
}

