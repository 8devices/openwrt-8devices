/*
 *  Header files defines some RX inline routines
 *
 *  $Id: 8192cd_rx.h,v 1.4.4.5 2010/12/10 06:11:55 button Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_RX_H_
#define _8192CD_RX_H_

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_util.h"
#ifdef CONFIG_DUAL_CPU
#include "dual_cpu.h"
#endif

#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
int fwdEngine_rx_skb (struct re_private *cp, struct sk_buff *skb,struct rx_info *pRxInfo);
#endif
#ifdef RX_BUFFER_GATHER_REORDER
void flush_rx_list_inq(struct rtl8192cd_priv *priv,struct rx_frinfo *pfrinfo);
#endif

#define SN_NEXT(n)		((n + 1) & 0xfff)
#define SN_LESS(a, b)	(((a - b) & 0x800) != 0)
#define SN_DIFF(a, b)	((a >= b)? (a - b):(0xfff - b + a + 1))


#define init_frinfo(pinfo) \
	do	{	\
			pinfo->pskb = pskb;		\
			pinfo->rssi = 0;		\
			INIT_LIST_HEAD(&(pinfo->mpdu_list)); \
			INIT_LIST_HEAD(&(pinfo->rx_list)); \
	} while(0)


#ifdef CONFIG_PCI_HCI
#ifdef  CONFIG_WLAN_HAL
#define ALIGN_OFFSET_SKB_DATA       32  //It is necessary to be power of 2
#define	GetOffsetStartToRXDESC(priv, pskb)		(ALIGN_OFFSET_SKB_DATA - ((((unsigned long)pskb->data) + sizeof(struct rx_frinfo)) & (ALIGN_OFFSET_SKB_DATA-1)))


static __inline__ 
#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
void init_rxdesc_88XX(
    struct rtl8192cd_priv   *priv, 
    struct sk_buff          *pskb, 
    u2Byte                  i, 
    pu4Byte                 pBufAddr,   // output
    pu4Byte                 pBufLen      // output
    )
{
    struct rtl8192cd_hw *phw;
    struct rx_frinfo    *pfrinfo;
    u4Byte              offset;

    phw = GET_HW(priv);

    offset = GetOffsetStartToRXDESC(priv, pskb);
    skb_reserve(pskb, sizeof(struct rx_frinfo) + offset);
    pfrinfo = get_pfrinfo(pskb);
    init_frinfo(pfrinfo);    

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
    pfrinfo->is_br_mgnt = 0;
#endif
#ifdef RX_BUFFER_GATHER
    pfrinfo->gather_flag = 0;
#ifdef RX_BUFFER_GATHER_REORDER
	INIT_LIST_HEAD(&pfrinfo->gather_list_head);
#endif
#endif

#ifdef __ECOS
#ifdef DELAY_REFILL_RX_BUF
		pskb->priv = priv;
#endif
#endif

    phw->rx_infoL[i].pbuf   = (void *)pskb;
#ifdef CONFIG_DUAL_CPU_SLAVE
	phw->rx_infoL[i].paddr	= get_physical_addr(priv, pskb->data, (RX_BUF_LEN - sizeof(struct rx_frinfo) - offset), PCI_DMA_FROMDEVICE) + CONFIG_LUNA_SLAVE_PHYMEM_OFFSET_HAL;
#else
    phw->rx_infoL[i].paddr  = get_physical_addr(priv, pskb->data, (RX_BUF_LEN - sizeof(struct rx_frinfo) - offset), PCI_DMA_FROMDEVICE);
#endif

    *pBufAddr   = (u4Byte)pskb->data;
#if 0    
    *pBufLen    = RX_BUF_LEN - sizeof(struct rx_frinfo) - offset;
#else

    //3 this buf len must be 32 byte alignment in 8881A !!!!, If not, the fs/ls mechanism will be fail
    *pBufLen    = RX_BUF_LEN - sizeof(struct rx_frinfo) - 64;
#ifdef CONFIG_WLAN_HAL_8881A
    if(GET_CHIP_VER(priv) == VERSION_8881A) {
        if ((*pBufLen & 0x1f) != 0) {
            *pBufLen = (((*pBufLen) >> 5) << 5); // *pBufLen = *pBufLen / 32 * 32;
            //printk("%s(%d): RX_BUF_LEN(%d) must be 32 byte alignment !!! \n", __FUNCTION__, __LINE__, *pBufLen);
        }
    }
#endif

#endif

#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
    // Remove it because pci_map_single() in get_physical_addr() already performed memory sync.
    //rtl_cache_sync_wback(priv, bus_to_virt(phw->rx_infoL[i].paddr), RX_BUF_LEN - sizeof(struct rx_frinfo) - offset, PCI_DMA_FROMDEVICE);     
#else
#ifdef CONFIG_ENABLE_NCBUFF
    if(skb_ncbuff_tag(pskb)->tag != NUBUFF_CACHE_TAG)
#endif
    rtl_cache_sync_wback(priv, (unsigned long)(bus_to_virt(phw->rx_infoL[i].paddr)-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), 
        RX_BUF_LEN - sizeof(struct rx_frinfo) - offset, PCI_DMA_FROMDEVICE);
#endif 
}
#endif  //CONFIG_WLAN_HAL

static __inline__ void init_rxdesc(struct sk_buff *pskb, int i, struct rtl8192cd_priv *priv)
{
	struct rtl8192cd_hw	*phw;
	struct rx_frinfo	*pfrinfo;
	int offset;

	phw = GET_HW(priv);

	offset = 0x20 - ((((unsigned long)pskb->data) + sizeof(struct rx_frinfo)) & 0x1f);	// need 32 byte aligned
	skb_reserve(pskb, sizeof(struct rx_frinfo) + offset);
	pfrinfo = get_pfrinfo(pskb);

	init_frinfo(pfrinfo);
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	pfrinfo->is_br_mgnt = 0;
#endif
#ifdef RX_BUFFER_GATHER
	pfrinfo->gather_flag = 0;
#endif

#ifdef __ECOS
#ifdef DELAY_REFILL_RX_BUF
	pskb->priv = priv;
#endif
#endif

	phw->rx_infoL[i].pbuf  = (void *)pskb;
	phw->rx_infoL[i].paddr = get_physical_addr(priv, pskb->data, (RX_BUF_LEN - sizeof(struct rx_frinfo) - 64), PCI_DMA_FROMDEVICE);
	phw->rx_descL[i].Dword6 = set_desc(phw->rx_infoL[i].paddr);
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
	// Remove it because pci_map_single() in get_physical_addr() already performed memory sync.
	//rtl_cache_sync_wback(priv, (unsigned long)bus_to_virt(phw->rx_infoL[i].paddr), RX_BUF_LEN - sizeof(struct rx_frinfo)-64, PCI_DMA_FROMDEVICE);
#else
#ifdef CONFIG_ENABLE_NCBUFF
        if(skb_ncbuff_tag(pskb)->tag != NUBUFF_CACHE_TAG)
#endif
	rtl_cache_sync_wback(priv, (unsigned long)(phw->rx_infoL[i].paddr), RX_BUF_LEN - sizeof(struct rx_frinfo)-64, PCI_DMA_FROMDEVICE);
#endif
	phw->rx_descL[i].Dword0 = set_desc((i == (NUM_RX_DESC_IF(priv) - 1)? RX_EOR : 0) | RX_OWN |((RX_BUF_LEN - sizeof(struct rx_frinfo)-64) & RX_PktLenMask));	//32 for alignment, 32 for TKIP MIC

}
#endif // CONFIG_PCI_HCI

static __inline__ unsigned char cal_rssi_avg(unsigned int agv, unsigned int pkt_rssi)
{
	unsigned int rssi;

	rssi = ((agv * 19) + pkt_rssi) / 20;
	if (pkt_rssi > agv)
		rssi++;

	return (unsigned char)rssi;
}

static __inline__ unsigned char cal_rssi_avg_mp(struct rtl8192cd_priv *priv, unsigned int agv, unsigned int pkt_rssi)
{
	unsigned int rssi;

	rssi = ((agv * (100 - priv->pshare->mp_rssi_weight)) + (pkt_rssi*priv->pshare->mp_rssi_weight)) / 100;
	if (pkt_rssi > agv)
		rssi++;

	return (unsigned char)rssi;
}

static __inline__ void update_sta_rssi(struct rtl8192cd_priv *priv,
				struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
	int i;
#if defined(MULTI_STA_REFINE) && defined(TX_SHORTCUT) && defined(SW_TX_QUEUE)
	int rssi_old =0;
#endif
#ifdef SW_ANT_SWITCH
	if(priv->pshare->RSSI_test == TRUE)
		return;
#endif

#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE) {
		//if (priv->pshare->rf_ft_var.rssi_dump) {
		if (pfrinfo->physt && pfrinfo->rssi) {
			priv->pshare->mp_rssi = cal_rssi_avg_mp(priv, priv->pshare->mp_rssi, pfrinfo->rssi);
			priv->pshare->mp_sq   = pfrinfo->sq;
			priv->pshare->mp_rx_rate = pfrinfo->rx_rate;
#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
			if(IS_OUTSRC_CHIP(priv))
#endif
			{
				for (i=0; i<4; i++)
					priv->pshare->mp_rf_info.mimorssi[i] = cal_rssi_avg_mp(priv, priv->pshare->mp_rf_info.mimorssi[i], pfrinfo->rf_info.mimorssi[i]);
				memcpy(&priv->pshare->mp_rf_info.mimosq[0], &pfrinfo->rf_info.mimosq[0], 2);
			}
#endif
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
			if(!IS_OUTSRC_CHIP(priv))
#endif
			{
			for (i=0; i<4; i++)
				priv->pshare->mp_rf_info.mimorssi[i] = cal_rssi_avg_mp(priv, priv->pshare->mp_rf_info.mimorssi[i], pfrinfo->rf_info.mimorssi[i]);
			memcpy(&priv->pshare->mp_rf_info.mimosq[0], &pfrinfo->rf_info.mimosq[0], sizeof(struct rf_misc_info) - 2);
		}
#endif
		}
		return;
	}
#endif
#if defined(MULTI_STA_REFINE) && defined(TX_SHORTCUT) && defined(SW_TX_QUEUE)
	rssi_old = pstat->rssi;
#endif

	if (pfrinfo->physt) {
			if (pfrinfo->rssi) {

#if defined(HW_ANT_SWITCH)&& (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
			if (pfrinfo->driver_info_size > 0) {
				unsigned char *phystatus =	(unsigned char*)pfrinfo->driver_info;			 
				int i = 1&(phystatus[27]>>7);
				if (is_CCK_rate(pfrinfo->rx_rate)) 
					++(pstat->cckPktCount[i]);				
				else 
					++(pstat->hwRxAntSel[i]);			
				if(!pstat->AntRSSI[i])
					pstat->AntRSSI[i] = pfrinfo->rssi;
				else
					pstat->AntRSSI[i] = cal_rssi_avg(pstat->AntRSSI[i], pfrinfo->rssi);
	
				if(priv->pshare->rf_ft_var.ant_dump&1)  {
					panic_printk("pkt--> cck:%d, B7=%d, B6=%d, R:(%d) Len:%d\n", is_CCK_rate(pfrinfo->rx_rate), 
						i,  (1&(phystatus[27]>>6))
						, pfrinfo->rssi, pfrinfo->pktlen);	
				}
	
			}	
#endif	
			
			for (i=0;i<4;i++)
			pstat->rx_snr[i]		= pfrinfo->rx_snr[i];

			pstat->rssi             = cal_rssi_avg(pstat->rssi, pfrinfo->rssi);
			pstat->sq               = pfrinfo->sq;
			pstat->rx_rate          = pfrinfo->rx_rate;
			pstat->rx_bw            = pfrinfo->rx_bw;
			pstat->rx_splcp         = pfrinfo->rx_splcp;

#ifdef RSSI_MONITOR_NCR
			if(priv->pshare->rf_ft_var.rssi_monitor_enable ) {
				unsigned char rssi = pstat->rssi;
#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
				if(IS_OUTSRC_CHIP(priv))
#endif
				{
					rssi = pstat->rssi_stat.UndecoratedSmoothedPWDB;
				}
#endif
				rssi_monitor(priv, pstat, rssi);
			}
#endif


			{
				if (pfrinfo->rf_info.mimorssi[0] != 0 || pfrinfo->rf_info.mimorssi[1] != 0)
					for (i=0; i<4; i++)
						pstat->rf_info.mimorssi[i] = cal_rssi_avg(pstat->rf_info.mimorssi[i], pfrinfo->rf_info.mimorssi[i]);

				if (priv->pshare->rf_ft_var.rssi_dump)
					memcpy(&pstat->rf_info.mimosq[0], &pfrinfo->rf_info.mimosq[0], sizeof(struct rf_misc_info) - 2);
			}

#if defined(MULTI_STA_REFINE) && defined(TX_SHORTCUT) && defined(SW_TX_QUEUE)
			if(((pstat->rssi <30 && rssi_old >=30)||(pstat->rssi >=30 && rssi_old <30)) && (priv->pshare->swq_numActiveSTA > LOWAGGRESTA))
				clearTxShortCutBufSize(priv, pstat);
#endif


			if (pstat->highest_rx_rate < pstat->rx_rate)
				pstat->highest_rx_rate = pstat->rx_rate;
		}
	}
}
#endif // _8192CD_RX_H_

