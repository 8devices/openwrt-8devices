/*
* ----------------------------------------------------------------
* $Header: /home/cvsroot/linux-2.6.19/linux-2.6.x/drivers/net/re865x/rtl865xc_swNic.c,v 1.11 2008/04/11 10:49:14 bo_zhao Exp $
*
* Abstract: Switch core polling mode NIC driver source code.
*
* $Author: bo_zhao $
*
*  Copyright (c) 2011 Realtek Semiconductor Corp.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
* ---------------------------------------------------------------
*/
#include <linux/skbuff.h>
#include <linux/netdevice.h> 
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "common/rtl_errno.h"
#include "AsicDriver/asicRegs.h"
#include "rtl865xc_swNic.h"
#include <net/rtl/rtl865x_netif.h>
#include "common/mbuf.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865xC_hs.h"
#ifdef	CONFIG_RTL865X_ROMEPERF
#include "romeperf.h"
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT)
#include <linux/if_ether.h>
#endif

#ifdef CONFIG_RTL_HW_TX_CSUM
#include <linux/etherdevice.h>
#include <net/rtl/rtk_vlan.h>
#include <linux/ip.h>
#endif

#if defined(CONFIG_RTL_DNS_TRAP)
extern int dns_filter_enable;
#endif

extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);

/* RX Ring */
static uint32*  rxPkthdrRing[RTL865X_SWNIC_RXRING_HW_PKTDESC];                 /* Point to the starting address of RX pkt Hdr Ring */
__DRAM_FWD static uint32   rxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC];              /* Total pkt count for each Rx descriptor Ring */
#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
__DRAM_FWD static uint32   rxPkthdrRefillThreshold[RTL865X_SWNIC_RXRING_HW_PKTDESC];              /* Ether refill threshold for each Rx descriptor Ring */
#endif

/* TX Ring */
static uint32*  txPkthdrRing[RTL865X_SWNIC_TXRING_HW_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define CONFIG_RTL_ENHANCE_RELIABILITY		1
#endif

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
__DRAM_FWD static uint32 txPkthdrRing_base[RTL865X_SWNIC_TXRING_HW_PKTDESC];
__DRAM_FWD static uint32 rxPkthdrRing_base[RTL865X_SWNIC_RXRING_HW_PKTDESC];
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
__DRAM_FWD uint32 rxMbufRing_base;
#endif
#endif

#if defined(CONFIG_RTL8196C_REVISION_B)
__DRAM_FWD static uint32	rtl_chip_version;
static uint32*  txPkthdrRing_backup[RTL865X_SWNIC_TXRING_HW_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */
#endif

__DRAM_FWD static uint32   txPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC];          /* Total pkt count for each Tx descriptor Ring */

#define txPktHdrRingFull(idx)   (((txPkthdrRingFreeIndex[idx] + 1) & (txPkthdrRingMaxIndex[idx])) == (txPkthdrRingDoneIndex[idx]))

/* Mbuf */
uint32 *rxMbufRing=NULL;                                                     /* Point to the starting address of MBUF Ring */
__DRAM_FWD uint32  rxMbufRingCnt;                                                  /* Total MBUF count */

__DRAM_FWD uint32  size_of_cluster;

/* descriptor ring tracing pointers */
__DRAM_FWD static int32   currRxPkthdrDescIndex[RTL865X_SWNIC_RXRING_HW_PKTDESC];      /* Rx pkthdr descriptor to be handled by CPU */
__DRAM_FWD static int32   currRxMbufDescIndex;        /* Rx mbuf descriptor to be handled by CPU */

__DRAM_FWD static int32   currTxPkthdrDescIndex[RTL865X_SWNIC_TXRING_HW_PKTDESC];      /* Tx pkthdr descriptor to be handled by CPU */
__DRAM_FWD static int32 txPktDoneDescIndex[RTL865X_SWNIC_TXRING_HW_PKTDESC];

/* debug counters */
//__DRAM_FWD static int32   rxPktCounter;
//__DRAM_FWD static int32   txPktCounter;

#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
__DRAM_FWD static int32   rxDescReadyForHwIndex[RTL865X_SWNIC_RXRING_HW_PKTDESC];
__DRAM_FWD static int32   rxDescCrossBoundFlag[RTL865X_SWNIC_RXRING_HW_PKTDESC];
#endif

__DRAM_FWD static uint8 extPortMaskToPortNum[_RTL865XB_EXTPORTMASKS+1] =
{
	5, 6, 7, 5, 8, 5, 5, 5
};
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
extern int extPortEnabled;
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
int set_portmask_tag;
int32 swNic_setVlanPortTag(int portmask){
        set_portmask_tag = portmask &0x3f;
        return 0;
}
#endif

#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
__DRAM_FWD unsigned int rx_noBuffer_cnt;
__DRAM_FWD unsigned int tx_ringFull_cnt;
__DRAM_FWD unsigned int tx_drop_cnt;
#endif

#if defined(CONFIG_RTL_HW_VLAN_SUPPORT)
int32 auto_set_tag_portmask;
int32 swNic_setVlanPortTag(int portmask)
{
        auto_set_tag_portmask = portmask &0x3f;
        return 0;
}
#ifdef CONFIG_RTL_HW_VLAN_SUPPORT_HW_NAT
extern uint32 rtk_getTagPortmaskByVid(uint16 vid , void *data_p);  //rtl_nic.c
extern void rtk_get_real_nicTxVid(struct sk_buff *skb , unsigned short *nictx_vid);
#endif
#endif
#if defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_SWCONFIG)
#include <linux/if_ether.h>
extern uint32 rtk_get_vlan_tagmask(uint16 vid);
#endif


#ifdef CONFIG_RTL_ETH_TX_SG
#define SG_MBUF_NUM 	(NUM_TX_PKTHDR_DESC+200)
uint32 sg_mbuf[SG_MBUF_NUM];
int sg_mbuf_idx=0;
#endif
#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
extern int gHwNatEnabled;
#endif

/*************************************************************************
*   FUNCTION
*       swNic_intHandler
*
*   DESCRIPTION
*       This function is the handler of NIC interrupts
*
*   INPUTS
*       intPending      Pending interrupt sources.
*
*   OUTPUTS
*       None
*************************************************************************/
void swNic_intHandler(uint32 intPending) {return;}
__MIPS16
__IRAM_FWD
inline int32 rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid)
{
	struct rtl_pktHdr *m_pkthdr = (struct rtl_pktHdr *)pkt;
	uint32 srcPortNum;
	
	srcPortNum = m_pkthdr->ph_portlist;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	srcPortNum &=0x7;
#endif
	*vid = m_pkthdr->ph_vlanId;


	if (srcPortNum < RTL8651_CPU_PORT)
	{
		#if defined (CONFIG_RTL_EXT_PORT_SUPPORT) 
		/* rx from physical port */
		if(extPortEnabled&&((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0))
		{	
			/* No CPU bit, finished nat translation, only dest ext mbr port... */
			/*
				if dest ph_extPortList 0x1 => to dst ext port 1 => to dst port 1+5=6
				if dest ph_extPortList 0x2 => to dst ext port 2 => to dst port 2+5=7
				if dest ph_extPortList 0x4 => to dst ext port 3 => to dst port 3+5=8
			*/
			/* No CPU bit, only dest ext mbr port... */
			if(	m_pkthdr->ph_extPortList&&
				(5!=extPortMaskToPortNum[m_pkthdr->ph_extPortList]))
			{
				#if defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
				
				
				//printk("ph_extPortList:%d,srcPortNum:%d,ph_isOriginal:%d,ph_l2Trans:%d,hwfwd:%d,[%s]:[%d].\n",
				//m_pkthdr->ph_extPortList,srcPortNum,m_pkthdr->ph_isOriginal,m_pkthdr->ph_l2Trans,m_pkthdr->ph_hwFwd,__FUNCTION__,__LINE__);
				if(m_pkthdr->ph_l2Trans!=0) 
				{
					/*wan-->ext port, finished napt translation*/
					*vid = PKTHDR_EXTPORT_MAGIC;
				}
				#endif
			}
			else
			{
				/*no dst ext port*/
				/*drop it*/
				return FAILED;
			}
		}
		/*to-do:mark it*/
		else
		{
			/* has CPU bit, pkt is original pkt from port 0~5 */
			/*original packet, let prototocol stack to handle it*/
			
		}
		
		#endif
	}	
	else 
	{
		/*rx from extension port*/
			
		#if defined (CONFIG_RTL_EXT_PORT_SUPPORT)
		/*to-do:*/
			if(extPortEnabled)
			{
				if ((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)
				{
					
					/* No CPU bit, finished nat translation, only dest ext mbr port... */
					/*
						if dest ph_extPortList 0x1 => to dst ext port 1 => to dst port 1+5=6
						if dest ph_extPortList 0x2 => to dst ext port 2 => to dst port 2+5=7
						if dest ph_extPortList 0x4 => to dst ext port 3 => to dst port 3+5=8
					*/
					if(	m_pkthdr->ph_extPortList&&
						(5!=extPortMaskToPortNum[m_pkthdr->ph_extPortList]))
					{
						#if defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
						/*WISP mode ext port-->ext port, finished napt translation*/
						//*vid = PKTHDR_EXTPORT_MAGIC2;
						/*drop it, may fix me in future*/
						return FAILED;
						#endif
					}
					else
					{
						/*no dst ext port*/
						/*drop it*/
						return FAILED;
					}
				
				
				}
				else
				{
					/* has CPU bit, pkt is original pkt from port 6~8 */
					/*case 1: not finished napt translation, need roll back interface to wlan ext interface*/
					
					#if defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
					*vid = PKTHDR_EXTPORT_MAGIC4;
					srcPortNum=extPortMaskToPortNum[m_pkthdr->ph_extPortList];
					m_pkthdr->ph_portlist =srcPortNum;
					/*drop it, may fix me in future*/
					return SUCCESS;
					#endif
				}
			}else
			{
				return FAILED;
			}
		   
#else
		return FAILED;
#endif
	
	}

	return SUCCESS;
}

#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
inline int return_to_rxing_check(int ringIdx)
{
	int ret;
	//unsigned long flags=0;
	//local_irq_save(flags);
	ret = ((rxDescReadyForHwIndex[ringIdx]!=currRxPkthdrDescIndex[ringIdx]) && (rxPkthdrRingCnt[ringIdx]!=0))? 1:0;
	//local_irq_restore(flags);
	return ret;
}
static inline int buffer_reuse(int ringIdx)
{
	int index1,index2,gap;
	#if !defined(CONFIG_SMP)
	unsigned long flags=0;
	SMP_LOCK_ETH_RECV(flags);
	#endif
	index1 = rxDescReadyForHwIndex[ringIdx];
	index2 = currRxPkthdrDescIndex[ringIdx]+1;
	gap = (index2 > index1) ? (index2 - index1) : (index2 + rxPkthdrRingCnt[ringIdx] - index1);

	if ((rxPkthdrRingCnt[ringIdx] - gap) < (rxPkthdrRefillThreshold[ringIdx]))
	{
		#if !defined(CONFIG_SMP)
		SMP_UNLOCK_ETH_RECV(flags);
		#endif
		return 1;
	}
	else
	{
		#if !defined(CONFIG_SMP)
		SMP_UNLOCK_ETH_RECV(flags);
		#endif
		return 0;
	}
}

static inline void set_RxPkthdrRing_OwnBit(uint32 rxRingIdx)
{
	rxPkthdrRing[rxRingIdx][rxDescReadyForHwIndex[rxRingIdx]] |= DESC_SWCORE_OWNED;

	if ( ++rxDescReadyForHwIndex[rxRingIdx] == rxPkthdrRingCnt[rxRingIdx] ) {
		rxDescReadyForHwIndex[rxRingIdx] = 0;
		#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		rxDescCrossBoundFlag[rxRingIdx]--;
		#endif
	}
}

__MIPS16
__IRAM_FWD
static void release_pkthdr(struct sk_buff  *skb, int idx)
{
	struct rtl_pktHdr *pReadyForHw;
	uint32 mbufIndex;
	//unsigned long flags=0;

	_dma_cache_inv((unsigned long)skb->data, 1536);

	//local_irq_save(flags);
	pReadyForHw = (struct rtl_pktHdr *)(rxPkthdrRing[idx][rxDescReadyForHwIndex[idx]] &
						~(DESC_OWNED_BIT | DESC_WRAP));
	mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct rtl_mBuf));

	pReadyForHw->ph_mbuf->m_data = skb->data;
	pReadyForHw->ph_mbuf->m_extbuf = skb->data;
	pReadyForHw->ph_mbuf->skb = skb;

#ifdef _PKTHDR_CACHEABLE
	//_dma_cache_wback_inv((unsigned long)pReadyForHw, sizeof(struct rtl_pktHdr));
	_dma_cache_wback_inv((unsigned long)(pReadyForHw->ph_mbuf), sizeof(struct rtl_mBuf));
#endif

	rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;
	set_RxPkthdrRing_OwnBit(idx);
	//local_irq_restore(flags);
}

__IRAM_FWD
int check_rx_pkthdr_ring(int idx, int *return_idx)
{
	int i;
	unsigned long flags=0;

	SMP_LOCK_ETH_RECV(flags);
	for(i = idx; i >= 0; i--) {
		if (return_to_rxing_check(i)) {
			*return_idx = i;
			SMP_UNLOCK_ETH_RECV(flags);
			return SUCCESS;
		}
	}

	*return_idx = -1;
	SMP_UNLOCK_ETH_RECV(flags);
	return FAILED;
}

__IRAM_FWD
int check_and_return_to_rx_pkthdr_ring(void *skb, int idx)
{
	unsigned long flags=0;

	SMP_LOCK_ETH_RECV(flags);
	if (return_to_rxing_check(idx)) {
		release_pkthdr(skb, idx);
		SMP_UNLOCK_ETH_RECV(flags);
		return SUCCESS;
	} else {
		SMP_UNLOCK_ETH_RECV(flags);
		return FAILED;
	}
}

/*
	return value: 1 ==> success, returned to rx pkt hdr desc
	return value: 0 ==> failed, no return ==> release to priv skb buf pool
 */
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);

__IRAM_FWD
int return_to_rx_pkthdr_ring(unsigned char *head)
{
	struct sk_buff *skb;
	int ret, i;
	unsigned long flags=0;

	ret=FAILED;
	SMP_LOCK_ETH_RECV(flags);
	for(i = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1; i >= 0; i--)
	{
		if (return_to_rxing_check(i)) {
			skb = dev_alloc_8190_skb(head, CROSS_LAN_MBUF_LEN);
			if (skb == NULL) {
				goto _ret1;
			}

			skb_reserve(skb, RX_OFFSET);
			release_pkthdr(skb, i);
			ret = SUCCESS;
			REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
			break;
		}
	}

_ret1:
	SMP_UNLOCK_ETH_RECV(flags);
	return ret;
}
#endif //DELAY_REFILL_ETH_RX_BUF

__IRAM_FWD
static void increase_rx_idx_release_pkthdr(struct sk_buff  *skb, int idx)
{
	struct rtl_pktHdr *pReadyForHw;
	uint32 mbufIndex;
#if !defined(CONFIG_SMP)
	unsigned long flags=0;

	SMP_LOCK_ETH_RECV(flags);
#endif

	_dma_cache_inv((unsigned long)skb->data, 1536);

#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
	pReadyForHw = (struct rtl_pktHdr *)(rxPkthdrRing[idx][rxDescReadyForHwIndex[idx]] &
						~(DESC_OWNED_BIT | DESC_WRAP));
#else

	#if defined(CONFIG_RTL_ENHANCE_RELIABILITY) && (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F))
	pReadyForHw = (struct rtl_pktHdr *) (rxPkthdrRing_base[idx] + (sizeof(struct rtl_pktHdr) * currRxPkthdrDescIndex[idx]));								
	#else		
	pReadyForHw = (struct rtl_pktHdr *)(rxPkthdrRing[idx][currRxPkthdrDescIndex[idx]] &
						~(DESC_OWNED_BIT | DESC_WRAP));
	#endif

#endif


#if defined(CONFIG_RTL_ENHANCE_RELIABILITY) && (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F))
	mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - rxMbufRing_base) /(sizeof(struct rtl_mBuf));
#else
	mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct rtl_mBuf));
#endif

	pReadyForHw->ph_mbuf->m_data = skb->data;
	pReadyForHw->ph_mbuf->m_extbuf = skb->data;
	pReadyForHw->ph_mbuf->skb = skb;

#ifdef _PKTHDR_CACHEABLE
	//_dma_cache_wback_inv((unsigned long)pReadyForHw, sizeof(struct rtl_pktHdr));
	_dma_cache_wback_inv((unsigned long)(pReadyForHw->ph_mbuf), sizeof(struct rtl_mBuf));
#endif

	rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;

#if !defined(DELAY_REFILL_ETH_RX_BUF) && !defined(ALLOW_RX_RING_PARTIAL_EMPTY)
	//set own bit to siwtch in the final step
	rxPkthdrRing[idx][currRxPkthdrDescIndex[idx]] |= DESC_SWCORE_OWNED; 
#endif	
	if ( ++currRxPkthdrDescIndex[idx] == rxPkthdrRingCnt[idx] ) {
		currRxPkthdrDescIndex[idx] = 0;
		#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		rxDescCrossBoundFlag[idx]++;
		#endif
	}

#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
	set_RxPkthdrRing_OwnBit(idx);
#endif
#if !defined(CONFIG_SMP)
	SMP_UNLOCK_ETH_RECV(flags);
#endif

}

__IRAM_FWD
static int __swNic_geRxRingIdx(uint32 rxRingIdx, uint32 *currRxPktDescIdx)
{
	#if !defined(CONFIG_SMP)
	unsigned long flags=0;
	#endif

	if(rxPkthdrRingCnt[rxRingIdx] == 0) {
		return FAILED;
	}

	#if !defined(CONFIG_SMP)
	SMP_LOCK_ETH_RECV(flags);
	#endif
	#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
	if ( (rxDescCrossBoundFlag[rxRingIdx]==0&&(currRxPkthdrDescIndex[rxRingIdx]>=rxDescReadyForHwIndex[rxRingIdx]))
		|| (rxDescCrossBoundFlag[rxRingIdx]==1&&(currRxPkthdrDescIndex[rxRingIdx]<rxDescReadyForHwIndex[rxRingIdx])) )
	#endif
	{
		if((rxPkthdrRing[rxRingIdx][currRxPkthdrDescIndex[rxRingIdx]] & DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			//SMP_UNLOCK_ETH_RECV(flags);
			*currRxPktDescIdx = currRxPkthdrDescIndex[rxRingIdx];
			#if !defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return SUCCESS;
		}
	}

	#if !defined(CONFIG_SMP)
	SMP_UNLOCK_ETH_RECV(flags);
	#endif
	return FAILED;
}

#if defined(RTL_MULTIPLE_RX_TX_RING)
/*	It's the caller's responsibility to make sure "rxRingIdx" and
*	"currRxPktDescIdx" NOT NULL, since the callee never check
*	sanity of the parameters, in order to speed up.
*/
/*
	Should protected by Caller
*/
__MIPS16
__IRAM_FWD
static inline int32 swNic_getRxringIdx(uint32 *rxRingIdx, uint32 *currRxPktDescIdx,uint32 policy)
{
	int32	i;
	int32	priority;


	priority = policy;

	for(i = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1; i >= priority; i--)
	{
		if (__swNic_geRxRingIdx(i, currRxPktDescIdx)==SUCCESS) {
			*rxRingIdx = i;
			return SUCCESS;
		}
	}
	return FAILED;
}
#endif

__IRAM_FWD
int swNic_increaseRxIdx(int rxRingIdx)
{
#if !defined(CONFIG_SMP)
	unsigned long	flags=0;
	//int32		nextIdx;

	SMP_LOCK_ETH_RECV(flags);
#endif
	if ( ++currRxPkthdrDescIndex[rxRingIdx] == rxPkthdrRingCnt[rxRingIdx] ) {
		currRxPkthdrDescIndex[rxRingIdx] = 0;
		#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		rxDescCrossBoundFlag[rxRingIdx]++;
		#endif
	}
#if !defined(CONFIG_SMP)
	SMP_UNLOCK_ETH_RECV(flags);
#endif

	return SUCCESS;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
int mbuf_pending_times = 0;
int get_mbuf_pending_times(void)
{
	return mbuf_pending_times;
}
int get_nic_txRing_buf(void)
{
	int txCnt = 0;
	int i,j;
	struct rtl_pktHdr *pPkthdr;
	for(i = RTL865X_SWNIC_TXRING_HW_PKTDESC -1; i >= 0; i--)
	{
		if(txPkthdrRingCnt[i] == 0)
			continue;

		for(j = 0; j <txPkthdrRingCnt[i]; j++)
		{

			pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[i][j]& ~(DESC_OWNED_BIT | DESC_WRAP));

			if(pPkthdr->ph_mbuf->skb)
			{
				if(is_rtl865x_eth_priv_buf(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->head))
					txCnt++;
			}
		}
	}

	return txCnt;
}

int get_nic_rxRing_buf(void)
{
	int rxCnt = 0;
	int i,j;
	struct rtl_pktHdr *pPkthdr;
	for(i = RTL865X_SWNIC_RXRING_HW_PKTDESC -1; i >= 0; i--)
	{
		if(rxPkthdrRingCnt[i] == 0)
			continue;

		for(j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			{
				pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][j] & ~(DESC_OWNED_BIT | DESC_WRAP));
				if(pPkthdr->ph_mbuf->skb)
				{
					if(is_rtl865x_eth_priv_buf(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->head))
						rxCnt++;
				}
			}
		}
	}

	return rxCnt;
}
#endif

int32 swNic_flushRxRingByPriority(int priority)
{
	int32	i;
	struct rtl_pktHdr * pPkthdr;
	void *skb;
	unsigned long flags=0;

	#if defined(CONFIG_RTL865X_WTDOG)
	REG32(WDTCNR) |=  WDTCLR; /* reset watchdog timer */
	#endif
	SMP_LOCK_ETH_RECV(flags);
	for(i = priority -1; i >= 0; i--)
	{
		if(rxPkthdrRingCnt[i] == 0)
			continue;

		while((rxPkthdrRing[i][currRxPkthdrDescIndex[i]] & DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][currRxPkthdrDescIndex[i]] & ~(DESC_OWNED_BIT | DESC_WRAP));
			skb = pPkthdr->ph_mbuf->skb;
			increase_rx_idx_release_pkthdr((void*)skb, i);
		}
	}
	SMP_UNLOCK_ETH_RECV(flags);
	REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
	return SUCCESS;
}


/*************************************************************************
*   FUNCTION
*       swNic_receive
*
*   DESCRIPTION
*       This function reads one packet from rx descriptors, and return the
*       previous read one to the switch core. This mechanism is based on
*       the assumption that packets are read only when the handling
*       previous read one is done.
*
*   INPUTS
*       None
*
*   OUTPUTS
*       None
*************************************************************************/
#define	RTL_NIC_RX_RETRY_MAX		(256)

//#if	defined(DELAY_REFILL_ETH_RX_BUF)
#define	RTL_ETH_NIC_DROP_RX_PKT_RESTART		\
	do {\
		increase_rx_idx_release_pkthdr(pPkthdr->ph_mbuf->skb, rxRingIdx); \
		REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL); \
	} while(0)
//#else
//#define	RTL_ETH_NIC_DROP_RX_PKT_RESTART		#error	"check here"
//#endif

__MIPS16
__IRAM_FWD
int32 swNic_receive(rtl_nicRx_info *info, int retryCount)
{
	struct rtl_pktHdr * pPkthdr;
	unsigned char *buf;
	void *skb;
	uint32 rxRingIdx;
	uint32 currRxPktDescIdx;
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	uint32	vid;
	#endif
#ifdef CONFIG_RTL_JUMBO_FRAME
	struct sk_buff	*pskb=NULL;
	struct rtl_mBuf *m_next=NULL;
	uint32 mbufIndex_tmp = 0;
	unsigned char *buf_tmp=NULL;
	unsigned int offset;
	struct rtl_mBuf *mbuf=NULL;
	void *skb_tmp=NULL;
#endif
	#if defined(CONFIG_SMP)
	//unsigned long flags = 0;
	//SMP_LOCK_ETH_RECV(flags);
	#endif
	
get_next:
	 /* Check OWN bit of descriptors */
	#if defined(RTL_MULTIPLE_RX_TX_RING)
	if (swNic_getRxringIdx(&rxRingIdx,&currRxPktDescIdx,info->priority) == SUCCESS )
	#else
	rxRingIdx = 0;
	if (__swNic_geRxRingIdx(rxRingIdx,&currRxPktDescIdx)==SUCCESS)
	#endif
	{
		/* Fetch pkthdr */
#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
		pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing_base[rxRingIdx] + (sizeof(struct rtl_pktHdr) * currRxPktDescIdx));								
#else
		pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[rxRingIdx][currRxPktDescIdx] & ~(DESC_OWNED_BIT | DESC_WRAP));
#endif
		/* Increment counter */
		//rxPktCounter++;

#if defined(_PKTHDR_CACHEABLE)
		_dma_cache_inv((unsigned long)pPkthdr, sizeof(struct rtl_pktHdr));
		_dma_cache_inv((unsigned long)(pPkthdr->ph_mbuf), sizeof(struct rtl_mBuf));
#endif
		//_dma_cache_inv((unsigned long)(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->data), pPkthdr->ph_len-4);
		/*	checksum error drop it	*/
		if ((pPkthdr->ph_flags & (CSUM_TCPUDP_OK | CSUM_IP_OK)) != (CSUM_TCPUDP_OK | CSUM_IP_OK))
		{
#ifdef CONFIG_RTL_IPV6READYLOGO
             		if (!(((pPkthdr->ph_flags&(CSUM_TCPUDP_OK))!=CSUM_TCPUDP_OK) && 
             			((pPkthdr->ph_portlist&0x38)==0x10) && ((pPkthdr->ph_reason==0x0060) || (pPkthdr->ph_reason==0x0008))))
#endif	
			{
				RTL_ETH_NIC_DROP_RX_PKT_RESTART;
				goto get_next;
			}
		}
#ifdef CONFIG_RTL_JUMBO_FRAME	
		mbuf=pPkthdr->ph_mbuf;
		if(mbuf && mbuf->m_next)	/*we will process jumbo frame here*/
		{
			//allocate a big space to hold jumbo frame	
			m_next = mbuf->m_next;

			buf_tmp = alloc_rx_buf(&skb_tmp, pPkthdr->ph_len - 4);			
			offset=0;
			/*cp data from frag mbuf to continious big space*/
			while(buf_tmp!=NULL && mbuf!=NULL){
				memcpy(buf_tmp+offset,mbuf->m_data,mbuf->m_len);
				offset +=mbuf->m_len;				
				mbuf = mbuf->m_next;
			}			
			/*release frag mbuf and pkt header*/
			increase_rx_idx_release_pkthdr(pPkthdr->ph_mbuf->skb, rxRingIdx);
		
			while(m_next)
			{
				pskb = (struct sk_buff *)m_next->skb;		
				mbufIndex_tmp = ((uint32)(m_next) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
							(sizeof(struct rtl_mBuf));
				/*m_data must be the same as skb->data*/
				m_next->m_data = pskb->data;
				m_next->m_extbuf = pskb->data;
				rxMbufRing[mbufIndex_tmp] |= DESC_SWCORE_OWNED;
				//panic_printk("mbufIndex_tmp:%d\n",mbufIndex_tmp);
				m_next = m_next->m_next;
			}

			REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
			if(buf_tmp == NULL)
				goto get_next;
			
#if defined(CONFIG_RTL_HARDWARE_NAT)
			if (rtl8651_rxPktPreprocess(pPkthdr, &vid) != 0) {
				RTL_ETH_NIC_DROP_RX_PKT_RESTART;
				goto get_next;
			} 
			info->vid = vid;
#else
			/*
			 * vid is assigned in rtl8651_rxPktPreprocess()
			 * do not update it when CONFIG_RTL_HARDWARE_NAT is defined
			 */
			info->vid=pPkthdr->ph_vlanId;			
#endif
			info->pid=pPkthdr->ph_portlist;
			info->input = skb_tmp;
			info->len = pPkthdr->ph_len - 4;
		#if defined(RTL_MULTIPLE_RX_TX_RING)
			info->priority = rxRingIdx;
		#endif
			#if defined(CONFIG_SMP)
			//SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_OK;
		}
#endif	


#if defined(CONFIG_RTL_HARDWARE_NAT)
		if (rtl8651_rxPktPreprocess(pPkthdr, &vid) != 0) {
			RTL_ETH_NIC_DROP_RX_PKT_RESTART;
			goto get_next;
		} else {
			#if defined(SKIP_ALLOC_RX_BUFF)
			buf = NULL;
			#else
			buf = alloc_rx_buf(&skb, size_of_cluster);
			#endif
		}
		info->vid = vid;
#else
		/*
		 * vid is assigned in rtl8651_rxPktPreprocess()
		 * do not update it when CONFIG_RTL_HARDWARE_NAT is defined
		 */
		info->vid=pPkthdr->ph_vlanId;
		#if defined(SKIP_ALLOC_RX_BUFF)
		buf = NULL;
		#else
		buf = alloc_rx_buf(&skb, size_of_cluster);
		#endif
#endif
#ifdef CONFIG_RTL_VLAN_8021Q
        //info->srcvid=pPkthdr->ph_svlanId;
        info->srcvid=pPkthdr->ph_vlanId;
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		info->pid=(pPkthdr->ph_portlist&0x7);
#else
		info->pid=pPkthdr->ph_portlist;
#endif
		if (buf)
		{
			info->input = pPkthdr->ph_mbuf->skb;
			info->len = pPkthdr->ph_len - 4;
			/* Increment index */
			increase_rx_idx_release_pkthdr(skb, rxRingIdx);

			REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
		}
#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		else if (!buffer_reuse(rxRingIdx)) 
		{
			info->input = pPkthdr->ph_mbuf->skb;
			info->len = pPkthdr->ph_len - 4;
			//printk("====CPU is pending====\n");
			#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
			mbuf_pending_times ++;
			#endif
			/* we do not free this skbuff in swNic_freeRxBuf() */
			pPkthdr->ph_mbuf->skb = NULL;

			/* Increment index */
			swNic_increaseRxIdx(rxRingIdx);
		}
#endif
		else {
			#if defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
				rx_noBuffer_cnt++;
			#endif
			#if 0
			if (retryCount>RTL_NIC_RX_RETRY_MAX) {
				/* drop pkt */
				increase_rx_idx_release_pkthdr((void*)pPkthdr->ph_mbuf->skb, rxRingIdx);
			}
			#if defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_REPEAT;
			#else
			#if defined(CONFIG_SMP)
			//SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_NULL;
			#endif
		}

		#if defined(RTL_MULTIPLE_RX_TX_RING)
		info->priority = rxRingIdx;
		#endif
		
		#if defined(CONFIG_SMP)
		//SMP_UNLOCK_ETH_RECV(flags);
		#endif

		return RTL_NICRX_OK;
	} else {
		#if defined(CONFIG_SMP)
		//SMP_UNLOCK_ETH_RECV(flags);
		#endif

		return RTL_NICRX_NULL;
	}
}

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
int32 swNic_freeRxRing(void)
{
	struct rtl_pktHdr * pPkthdr;
	uint32 rxRingIdx;
	uint32 currRxPktDescIdx;
	printk("[%s]:[%d].\n",__FUNCTION__,__LINE__);
	 /* Check OWN bit of descriptors */
	#if defined(RTL_MULTIPLE_RX_TX_RING)
	/*to-do:free each rx ring one by one*/
	#else
	rxRingIdx = 0;
	while (__swNic_geRxRingIdx(rxRingIdx,&currRxPktDescIdx)==SUCCESS)
	#endif
	{
	
		/* Fetch pkthdr */
#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
		pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing_base[rxRingIdx] + (sizeof(struct rtl_pktHdr) * currRxPktDescIdx));								
#else
		pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[rxRingIdx][currRxPktDescIdx] & ~(DESC_OWNED_BIT | DESC_WRAP));
#endif
		/* Increment counter */
		//rxPktCounter++;

#if defined(_PKTHDR_CACHEABLE)
		_dma_cache_inv((unsigned long)pPkthdr, sizeof(struct rtl_pktHdr));
		_dma_cache_inv((unsigned long)(pPkthdr->ph_mbuf), sizeof(struct rtl_mBuf));
#endif
		_dma_cache_inv((unsigned long)(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->data), pPkthdr->ph_len-4);
		RTL_ETH_NIC_DROP_RX_PKT_RESTART;

	}

	return 0;
}
#endif

#undef	RTL_ETH_NIC_DROP_RX_PKT_RESTART


#ifdef CONFIG_RTL_HW_TX_CSUM
static inline struct iphdr * getIpv4Header(uint8 *macFrame)
{
	uint8 *ptr;
	struct iphdr *iph=NULL;
	
	ptr=macFrame+12;	

	/*it's not ipv4 packet*/
	if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IP))
	{
		return NULL;
	}
	
	iph=(struct iphdr *)(ptr+2);

	return iph;
}

static inline int get_protocol_type2(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct udphdr *udph;
	   
	iph = getIpv4Header(skb->data);
	if(iph == NULL)
		return PKTHDR_ETHERNET;
	udph = (void *) iph + iph->ihl*4;
	if (iph->protocol == IPPROTO_UDP )
		return PKTHDR_UDP;
	else if(iph->protocol == IPPROTO_TCP )
		return PKTHDR_TCP;
	else if(iph->protocol == IPPROTO_ICMP )
		return PKTHDR_ICMP;
	else if(iph->protocol == IPPROTO_IGMP )
		return PKTHDR_IGMP;
	else
		return PKTHDR_IP;
}

/*
  for 8198C: only can use direct tx mode when use tx checksum feature
  software need to parse pkt and fill: type/Vi/Li/Pi/ipv6/ipv4/ipv4_1st/ipv6_hdrlen of pkthdr
  Vi: pkt has vlan tag
  Li: pkt has llc snap header
  Pi: pkt has pppoe header
  ipv6: pkt has ipv6 hdr
  ipv4: pkt has ipv4 hdr
  ipv4_1st: the first layer is ipv4
  ipv6_hdrlen: ipv6=1 must assign ipv6_hdrlen, it include base+extension, minimum is 40 bytes.

  for example:  
    pure ipv6 pkt: ipv6=1, ipv4=0, ipv4_1st=0
    pure ipv4 pkt: ipv6=0, ipv4=1, ipv4_1st=1
    DS-Lite pkt (ipv6+ipv4): ipv6=1, ipv4=1, ipv4_1st=0
    6-RD pkt (ipv4+ipv6): ipv6=1, ipv4=1, ipv4_1st=1

  type: no matter ipv4 or ipv6, parse to L4 (if have)
	ICMP	: L4 is icmp
	IGMP	: L4 is igmp
	TCP		: L4 is tcp
	UDP		: L4 is udp
	PPTP	: L4 is gre
	IP		: L3 ipv4 (no L4)
	IPV6	: L3 ipv6 (no L4)
	ETHERNET: L2 pkt   

  above setting are complicated, however for 8197F, HW Tx checksum support HW_lookup mode,
  so if want to use HW Tx checksum feature, please ALWAYS use HW_lookup mode,
  even broadcast/multicast pkt, even MULTIPLE_LAN defined,
  even RTK_VLAN defined and vlan is enabled, even ...
 */
static inline int get_protocol_type(struct sk_buff *skb, struct rtl_pktHdr *pPkthdr)
{
	struct iphdr *iph=NULL;
	#ifdef CONFIG_IPV6
	uint8 proto_type=PKTHDR_ETHERNET;
	uint8 next_header;
	#endif

	pPkthdr->ph_vlanTagged = 0;
	pPkthdr->ph_pppeTagged = 0;
	if(*(int16 *)(skb->data+12)==(int16)htons(ETH_P_IP)) {
		iph=(struct iphdr *)(skb->data+14);
	}
	else if ((*(int16 *)(skb->data+12)==(int16)htons(ETH_P_PPP_SES)) ||
			 (*(int16 *)(skb->data+12)==(int16)htons(ETH_P_PPP_DISC)) ) {
		pPkthdr->ph_pppeTagged = 1;
		if(*(int16 *)(skb->data+12+8)==(int16)htons(0x0021)) {
			iph=(struct iphdr *)(skb->data+12+10);
		}
		#ifdef CONFIG_IPV6
		else if(*(int16 *)(skb->data+12+8)==(int16)htons(0x0057)) {
			proto_type = PKTHDR_IPV6;
			next_header = *(uint8 *)(skb->data+12+8+2+6);
		}
		#endif

	}
	else if(*(int16 *)(skb->data+12)==(int16)htons(ETH_P_8021Q)) {
		pPkthdr->ph_vlanTagged = 1;
		if(*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_IP)) {
			iph=(struct iphdr *)(skb->data+12+4+2);
		} 
		#ifdef CONFIG_IPV6
		else if(*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_IPV6)) {
			proto_type = PKTHDR_IPV6;
			next_header = *(uint8 *)(skb->data+12+4+2+6);
		}
		#endif
		else if ((*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_PPP_SES)) ||
			 (*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_PPP_DISC)) ) {
			pPkthdr->ph_pppeTagged = 1;
			if(*(int16 *)(skb->data+12+4+8)==(int16)htons(0x0021)) {
				iph=(struct iphdr *)(skb->data+12+4+10);
			}
			#ifdef CONFIG_IPV6
			else if(*(int16 *)(skb->data+12+4+8)==(int16)htons(0x0057)) {
				proto_type = PKTHDR_IPV6;
				next_header = *(uint8 *)(skb->data+12+4+8+2+6);
			}
			#endif
		}
	}
	#ifdef CONFIG_IPV6
	else if(*(int16 *)(skb->data+12)==(int16)htons(ETH_P_IPV6)) {
		proto_type = PKTHDR_IPV6;
		next_header = *(uint8 *)(skb->data+12+2+6);
	}
	#endif

	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	#ifdef CONFIG_IPV6
	if (proto_type == PKTHDR_IPV6) {
			pPkthdr->ph_ipIpv6 = 1;
			pPkthdr->ph_ipIpv6HdrLen = 40;
			//IPv4 fileds should be 0
			pPkthdr->ph_ipIpv4 = 0;
			pPkthdr->ph_ipIpv4_1st = 0;
			
		if (next_header == IPPROTO_TCP )
			return PKTHDR_TCP;
		else if(next_header == IPPROTO_UDP )
			return PKTHDR_UDP;
		else if(next_header == IPPROTO_ICMP )
			return PKTHDR_ICMP;
		else if(next_header == IPPROTO_IGMP )
			return PKTHDR_IGMP;
		else if(next_header == IPPROTO_GRE )
			return PKTHDR_PPTP;	
		else
			return PKTHDR_IPV6;
	} else 
	#endif
	{
			pPkthdr->ph_ipIpv4 = 1;
			pPkthdr->ph_ipIpv4_1st = 1;
			//IPv6 fileds should be 0
			pPkthdr->ph_ipIpv6 = 0;
			pPkthdr->ph_ipIpv6HdrLen = 0;
	}
	#endif
	
        if (iph == NULL)
                return PKTHDR_ETHERNET;

	if (iph->protocol == IPPROTO_TCP )
		return PKTHDR_TCP;
	else if(iph->protocol == IPPROTO_UDP )
		return PKTHDR_UDP;
	else if(iph->protocol == IPPROTO_ICMP )
		return PKTHDR_ICMP;
	else if(iph->protocol == IPPROTO_IGMP )
		return PKTHDR_IGMP;
	else if(iph->protocol == IPPROTO_GRE )
		return PKTHDR_PPTP;	
	else
		return PKTHDR_IP;
}
#endif

/*************************************************************************
*   FUNCTION
*       swNic_send
*
*   DESCRIPTION
*       This function writes one packet to tx descriptors, and waits until
*       the packet is successfully sent.
*
*   INPUTS
*       None
*
*   OUTPUTS
*       None
*************************************************************************/
__MIPS16
__IRAM_FWD  inline int32 _swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx)
{
	struct rtl_pktHdr * pPkthdr;
	int next_index, ret;

#ifdef CONFIG_RTL_ETH_TX_SG
	struct rtl_mBuf *pMbuf2, *pMbuf3=NULL;
	struct sk_buff *pskb = (struct sk_buff *)skb;
	int i;
#endif

	if ((currTxPkthdrDescIndex[nicTx->txIdx]+1)==txPkthdrRingCnt[nicTx->txIdx])
		next_index = 0;
	else
		next_index = currTxPkthdrDescIndex[nicTx->txIdx]+1;

	if (next_index == txPktDoneDescIndex[nicTx->txIdx])	{
		/*	TX ring full	*/
			return -1;
	}

#if defined(CONFIG_RTL_ENHANCE_RELIABILITY) && (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F))
	pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing_base[nicTx->txIdx] + 
		(sizeof(struct rtl_pktHdr) * currTxPkthdrDescIndex[nicTx->txIdx]));

#else
	/* Fetch packet header from Tx ring */
	pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[nicTx->txIdx][currTxPkthdrDescIndex[nicTx->txIdx]]
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));

#endif

	/* Pad small packets and add CRC */
	if ( len < 60 )
		len = 64;
	else
		len += 4;

#ifdef CONFIG_RTL_ETH_TX_SG
	if (skb_shinfo(pskb)->nr_frags>0) {
		pPkthdr->ph_mbuf->m_len  = pskb->len - pskb->data_len;
		pPkthdr->ph_mbuf->m_extsize = pskb->len - pskb->data_len;	
		pPkthdr->ph_mbuf->m_flags  = MBUF_USED | MBUF_EXT | MBUF_PKTHDR;
	}
	else {
		pPkthdr->ph_mbuf->m_len  = len;
		pPkthdr->ph_mbuf->m_extsize = len;
		pPkthdr->ph_mbuf->m_flags  = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
		pPkthdr->ph_mbuf->m_next  = NULL;
	}
#else
	pPkthdr->ph_mbuf->m_len  = len;
	pPkthdr->ph_mbuf->m_extsize = len;
#endif

	pPkthdr->ph_mbuf->skb = skb;
	pPkthdr->ph_len = len;

	pPkthdr->ph_vlanId = nicTx->vid;
	
	#if defined(CONFIG_8198_PORT5_GMII) || defined(CONFIG_8198_PORT5_RGMII) || defined(CONFIG_8198C_8211FS) || defined(CONFIG_RTL_8198C_8211F) || defined(CONFIG_RTL_8198C_8367RB)
	pPkthdr->ph_portlist = nicTx->portlist&0x3f;
	#else
	pPkthdr->ph_portlist = nicTx->portlist&0x1f;
	#endif
	pPkthdr->ph_srcExtPortNum = nicTx->srcExtPort;
	pPkthdr->ph_flags = nicTx->flags;
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS)
	pPkthdr->ph_txPriority = nicTx->priority;
#endif
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if (*((unsigned short *)((unsigned char*)output+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q))
		pPkthdr->ph_txCVlanTagAutoAdd = nicTx->tagport;
	else
		pPkthdr->ph_txCVlanTagAutoAdd = 0;
#endif

#if ((defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_SWCONFIG)) && defined(CONFIG_RTL_QOS_8021P_SUPPORT)) || defined(CONFIG_RTL_HW_TX_CSUM)
	pPkthdr->ph_vlanTagged = 0;
#endif

#if defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_SWCONFIG)
	if (*((unsigned short *)((unsigned char*)output+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q))
	{
		#if defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG)
		extern int rtl865x_curOpMode;
		if(rtl865x_curOpMode == 0)
			pPkthdr->ph_txCVlanTagAutoAdd = RTL_WANPORT_MASK & rtk_get_vlan_tagmask(pPkthdr->ph_vlanId);
		else
			pPkthdr->ph_txCVlanTagAutoAdd = 0x3f & rtk_get_vlan_tagmask(pPkthdr->ph_vlanId);
		#else
		pPkthdr->ph_txCVlanTagAutoAdd = 0x3f & rtk_get_vlan_tagmask(pPkthdr->ph_vlanId);
		#endif
	}
	else{
		#if defined(CONFIG_RTL_DNS_TRAP)
		if (dns_filter_enable && ((struct sk_buff *)skb)->is_dns_pkt){
			//packets tagged here
			pPkthdr->ph_vlanTagged = 1;
			pPkthdr->ph_txCVlanTagAutoAdd = 0x3f & rtk_get_vlan_tagmask(pPkthdr->ph_vlanId);
			//printk("%s %d pPkthdr->ph_vlanTagged=0x%x pPkthdr->ph_txCVlanTagAutoAdd=0x%x pPkthdr->ph_vlanId=0x%x \n", __func__, __LINE__, pPkthdr->ph_vlanTagged, pPkthdr->ph_txCVlanTagAutoAdd, pPkthdr->ph_vlanId);
		}else
		#endif
			pPkthdr->ph_txCVlanTagAutoAdd = 0;
	}

	#if defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	if(gHwNatEnabled==0)
	#endif
	{
	if(pPkthdr->ph_portlist == RTL_WANPORT_MASK)
	{
		if(((struct sk_buff *)skb)->srcVlanPriority>0 && ((struct sk_buff *)skb)->srcVlanPriority<=7)
		{
			if (*(unsigned short *)((unsigned char*)output+ETH_ALEN*2) == __constant_htons(ETH_P_8021Q))
			{
				pPkthdr->ph_vlanTagged = 1;
				pPkthdr->ph_txCVlanTagAutoAdd = RTL_WANPORT_MASK;
				pPkthdr->ph_txPriority = ((struct sk_buff *)skb)->srcVlanPriority;
			}		
		}
	}
	}
  	#endif
	
    #if 0
    panic_printk("%s %d pPkthdr->ph_txCVlanTagAutoAdd=0x%x pPkthdr->ph_portlist=0x%x pPkthdr->ph_vlanId=%d\n",
        __FUNCTION__, __LINE__, pPkthdr->ph_txCVlanTagAutoAdd, pPkthdr->ph_portlist, pPkthdr->ph_vlanId);
    #endif
#elif defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if (*((unsigned short *)((unsigned char*)output+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q))
	{
#ifdef  CONFIG_RTL_HW_VLAN_SUPPORT_HW_NAT
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT) //mark_8367		
	 		rtk_get_real_nicTxVid((struct sk_buff *)skb , &nicTx->vid);
			pPkthdr->ph_vlanId = nicTx->vid;
#endif
			pPkthdr->ph_txCVlanTagAutoAdd = (0x3f) & rtk_getTagPortmaskByVid(pPkthdr->ph_vlanId,output); 
#else
			pPkthdr->ph_txCVlanTagAutoAdd = auto_set_tag_portmask;
#endif
	}
	else
			pPkthdr->ph_txCVlanTagAutoAdd = 0;
#endif

#ifdef CONFIG_RTL_HW_TX_CSUM
	if (((struct sk_buff *)skb)->ip_summed == CHECKSUM_PARTIAL)
	{
		pPkthdr->ph_proto = get_protocol_type(skb, pPkthdr);
		pPkthdr->ph_flags= (PKTHDR_USED|PKT_OUTGOING|CSUM_TCPUDP|CSUM_IP);		
	}	
	else {
		// set these fields to 0 which we may used before.
		pPkthdr->ph_proto = 0;
		pPkthdr->ph_pppeTagged = 0;

		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		pPkthdr->ph_ipIpv4 = 0;
		pPkthdr->ph_ipIpv4_1st = 0;
		pPkthdr->ph_ipIpv6 = 0;
		pPkthdr->ph_ipIpv6HdrLen = 0;
		#endif
	}
#endif

	/* Set cluster pointer to buffer */
	pPkthdr->ph_mbuf->m_data    = (output);
	pPkthdr->ph_mbuf->m_extbuf = (output);

#ifdef CONFIG_RTL_ETH_TX_SG
	for (i = 0; i < skb_shinfo(pskb)->nr_frags; i++) 
	{
		skb_frag_t *frag = &skb_shinfo(pskb)->frags[i];
		void *addr;

		addr = ((void *) page_address(frag->page.p)) + frag->page_offset;
		_dma_cache_wback((unsigned long) addr, frag->size);

		if (i==0) { // first one
			pMbuf2 = (struct rtl_mBuf *)sg_mbuf[sg_mbuf_idx++];
			if (sg_mbuf_idx >= SG_MBUF_NUM) sg_mbuf_idx=0;
			pPkthdr->ph_mbuf->m_next  = pMbuf2;
		}
		else {
			pMbuf2  = pMbuf3;
		}
		pMbuf2->m_data  = addr;
		pMbuf2->m_extbuf  = addr;
		pMbuf2->m_pkthdr = pPkthdr;
		if (i == (skb_shinfo(pskb)->nr_frags-1)) { // last one
			pMbuf2->m_len  = frag->size+4;
			pMbuf2->m_extsize = frag->size+4;
			pMbuf2->m_flags = MBUF_USED | MBUF_EXT | MBUF_EOR;
			pMbuf2->m_next = NULL;
		}
		else {
			pMbuf2->m_len  = frag->size;
			pMbuf2->m_extsize = frag->size;
			pMbuf2->m_flags = MBUF_USED | MBUF_EXT;
			pMbuf3 = (struct rtl_mBuf *)sg_mbuf[sg_mbuf_idx++];
			if (sg_mbuf_idx >= SG_MBUF_NUM) sg_mbuf_idx=0;		
			pMbuf2->m_next = pMbuf3;
		}		
		_dma_cache_wback((unsigned long)pMbuf2, sizeof(struct rtl_mBuf));		
	}
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	pPkthdr->ph_ptpPkt = 0;
#endif

#ifdef _PKTHDR_CACHEABLE
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	_dma_cache_wback((unsigned long)pPkthdr, sizeof(struct rtl_pktHdr));
	_dma_cache_wback((unsigned long)(pPkthdr->ph_mbuf), sizeof(struct rtl_mBuf));
	#else
	_dma_cache_wback_inv((unsigned long)pPkthdr, sizeof(struct rtl_pktHdr));
	_dma_cache_wback_inv((unsigned long)(pPkthdr->ph_mbuf), sizeof(struct rtl_mBuf));
	#endif
#endif

	ret = currTxPkthdrDescIndex[nicTx->txIdx];
	currTxPkthdrDescIndex[nicTx->txIdx] = next_index;
	/* Give descriptor to switch core */
	txPkthdrRing[nicTx->txIdx][ret] |= DESC_SWCORE_OWNED;

#if defined(CONFIG_RTL_ENHANCE_RELIABILITY) && !defined(CONFIG_RTL_8198C) &&  !defined(CONFIG_RTL_8197F)
	{
	uint32 pkthdr2 = (uint32)txPkthdrRing[nicTx->txIdx][ret];
	if ((pkthdr2 & DESC_OWNED_BIT) == 0)
		rtlglue_printf("_swNic_send: idx= %d, read back pkthdr= 0x%x.\n", ret, pkthdr2);
	}
#endif

#if 0
	memDump((void*)output, 64, "TX");
	printk("index %d address 0x%p, 0x%x 0x%p.\n", ret, &txPkthdrRing[nicTx->txIdx][ret], (*(volatile uint32 *)&txPkthdrRing[nicTx->txIdx][ret]), pPkthdr);
	printk("Flags 0x%x proto 0x%x portlist 0x%x vid %d extPort %d srcExtPort %d len %d.\n",
		pPkthdr->ph_flags, pPkthdr->ph_proto, pPkthdr->ph_portlist, pPkthdr->ph_vlanId,
		pPkthdr->ph_extPortList, pPkthdr->ph_srcExtPortNum, pPkthdr->ph_len);
#endif

	/* Set TXFD bit to start send */
	#if 0 //defined(CONFIG_RTL_8197F)
        REG32(CPUICR)=REG32(CPUICR)&(~(0x1<<29));
	#endif
	REG32(CPUICR) |= TXFD;

	return ret;
}

__IRAM_FWD
int32 swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx)
{
	int	ret;
	unsigned long flags=0;

	SMP_LOCK_ETH_XMIT(flags);
	ret = _swNic_send(skb, output, len, nicTx);
	SMP_UNLOCK_ETH_XMIT(flags);
	return ret;
}

__IRAM_FWD
int32 swNic_txDone(int idx)
{
	struct rtl_pktHdr	*pPkthdr;
	//int				free_num;
	unsigned long flags=0;

	SMP_LOCK_ETH_XMIT(flags);
	//free_num = 0;
	{
		while (txPktDoneDescIndex[idx] != currTxPkthdrDescIndex[idx]) {
		if ( (*(volatile uint32 *)&txPkthdrRing[idx][txPktDoneDescIndex[idx]]
			& DESC_OWNED_BIT) == DESC_RISC_OWNED )
		{
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing[idx][txPktDoneDescIndex[idx]] =txPkthdrRing_backup[idx][txPktDoneDescIndex[idx]] ;
			#endif

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing_base[idx] + (sizeof(struct rtl_pktHdr) * txPktDoneDescIndex[idx]));								
#else
			pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[idx][txPktDoneDescIndex[idx]]
				& ~(DESC_OWNED_BIT | DESC_WRAP));
#endif			

			if (pPkthdr->ph_mbuf->skb)
			{
                #if !(defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)) || !defined(CONFIG_RPS)
				SMP_UNLOCK_ETH_XMIT(flags);
                #endif
				#if defined(CONFIG_RTL_FAST_BRIDGE)
				tx_done_callback(pPkthdr->ph_mbuf->skb);
				#else
				dev_kfree_skb_any((struct sk_buff *)pPkthdr->ph_mbuf->skb);
				#endif
                #if !(defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)) || !defined(CONFIG_RPS)
				SMP_LOCK_ETH_XMIT(flags);
                #endif
				pPkthdr->ph_mbuf->skb = NULL;
			}


			if (++txPktDoneDescIndex[idx] == txPkthdrRingCnt[idx])
				txPktDoneDescIndex[idx] = 0;

			//free_num++;
		}
		else
			break;
		}
	}

	SMP_UNLOCK_ETH_XMIT(flags);
	return 0; //free_num;
}

#ifdef  CONFIG_RTL865X_MODEL_TEST_FT2
int32 swNic_send_portmbr(void * output, uint32 len, uint32 portmbr)
{
    struct rtl_pktHdr * pPkthdr;
    uint8 pktbuf[2048];
    uint8* pktbuf_alligned = (uint8*) (( (uint32) pktbuf & 0xfffffffc) | 0xa0000000);

    /* Copy Packet Content */
    memcpy(pktbuf_alligned, output, len);

    ASSERT_CSP( ((int32) txPkthdrRing[0][currTxPkthdrDescIndex] & DESC_OWNED_BIT) == DESC_RISC_OWNED );

    /* Fetch packet header from Tx ring */
    pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[0][currTxPkthdrDescIndex]
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));

    /* Pad small packets and add CRC */
    if ( len < 60 )
        pPkthdr->ph_len = 64;
    else
        pPkthdr->ph_len = len + 4;

    pPkthdr->ph_mbuf->m_len = pPkthdr->ph_len;
    pPkthdr->ph_mbuf->m_extsize = pPkthdr->ph_len;

    /* Set cluster pointer to buffer */
    pPkthdr->ph_mbuf->m_data = pktbuf_alligned;
    pPkthdr->ph_mbuf->m_extbuf = pktbuf_alligned;

    /* Set destination port */
    pPkthdr->ph_portlist = portmbr;

    /* Give descriptor to switch core */
    txPkthdrRing[0][currTxPkthdrDescIndex] |= DESC_SWCORE_OWNED;

    /* Set TXFD bit to start send */
    REG32(CPUICR) |= TXFD;

    /* Wait until packet is successfully sent */
#if 1
    while ( (*(volatile uint32 *)&txPkthdrRing[0][currTxPkthdrDescIndex]
                    & DESC_OWNED_BIT) == DESC_SWCORE_OWNED );
#endif
    //txPktCounter++;

    if ( ++currTxPkthdrDescIndex == txPkthdrRingCnt[0] )
        currTxPkthdrDescIndex = 0;

    return 0;
}
#endif


void swNic_freeRxBuf(void)
{
	int i;
	//int idx;
	//struct rtl_pktHdr * pPkthdr;
	#if defined(CONFIG_SMP)
	unsigned long flags = 0;
	#endif

	/* Initialize index of Tx pkthdr descriptor */
	for (i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++)
	{
		currTxPkthdrDescIndex[i] = 0;
		txPktDoneDescIndex[i]=0;
	}

	for(i=RTL865X_SWNIC_RXRING_HW_PKTDESC-1; i >= 0 ; i--)
	{
		/* Initialize index of current Rx pkthdr descriptor */
		currRxPkthdrDescIndex[i] = 0;
		/* Initialize index of current Rx Mbuf descriptor */
		currRxMbufDescIndex = 0;
		#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		rxDescReadyForHwIndex[i] = 0;
		rxDescCrossBoundFlag[i] = 0;
		#endif
#if 0		
		for (idx=0; idx<rxPkthdrRingCnt[i]; idx++)
		{
			if (!((rxPkthdrRing[i][idx] & DESC_OWNED_BIT) == DESC_RISC_OWNED)) {
				pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][idx] &
					~(DESC_OWNED_BIT | DESC_WRAP));

				/*if(pPkthdr == NULL || pPkthdr->ph_mbuf == NULL)
				*	continue;
				*/
				if (pPkthdr->ph_mbuf->skb)
				{
					free_rx_buf(pPkthdr->ph_mbuf->skb);
					pPkthdr->ph_mbuf->skb = NULL;
				}

				pPkthdr->ph_mbuf->m_data = NULL;
				pPkthdr->ph_mbuf->m_extbuf = NULL;
		    	}
	    	}
#endif
	}
	if (rxMbufRing) {
		struct rtl_mBuf *pMbuf;

		for (i=0;i<rxMbufRingCnt;i++)
		{
			#if defined(CONFIG_SMP)
			SMP_LOCK_ETH_RECV(flags);
			#endif
			pMbuf = (struct rtl_mBuf *)(rxMbufRing[i] & ~(DESC_OWNED_BIT | DESC_WRAP));

			if (pMbuf->skb)
			{
				#if defined(CONFIG_SMP)
				SMP_UNLOCK_ETH_RECV(flags);
				#endif
				free_rx_buf(pMbuf->skb);
				#if defined(CONFIG_SMP)
				SMP_LOCK_ETH_RECV(flags);
				#endif
				pMbuf->skb = NULL;
			}
			#if defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif

#ifdef INIT_RX_RING_ERR_HANDLE
			if ((rxMbufRing[i] & DESC_WRAP) != 0)
				break;
#endif			
		}	
	}
}

void swNic_freeTxRing(void)
{
	struct rtl_pktHdr	*pPkthdr;
	uint32 idx;
	unsigned long flags=0;

	SMP_LOCK_ETH_XMIT(flags);
	//free_num = 0;

	/* Initialize index of Tx pkthdr descriptor */
	for (idx=0;idx<RTL865X_SWNIC_TXRING_HW_PKTDESC;idx++)
	{
			while (txPktDoneDescIndex[idx] != currTxPkthdrDescIndex[idx]) {

			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing[idx][txPktDoneDescIndex[idx]] =txPkthdrRing_backup[idx][txPktDoneDescIndex[idx]] ;
			#endif

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing_base[idx] + (sizeof(struct rtl_pktHdr) * txPktDoneDescIndex[idx]));								
#else
			pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[idx][txPktDoneDescIndex[idx]]
				& ~(DESC_OWNED_BIT | DESC_WRAP));
#endif			

			if (pPkthdr->ph_mbuf->skb)
			{

				#if defined(CONFIG_RTL_FAST_BRIDGE)
				tx_done_callback(pPkthdr->ph_mbuf->skb);
				#else
				dev_kfree_skb_any((struct sk_buff *)pPkthdr->ph_mbuf->skb);
				#endif

				pPkthdr->ph_mbuf->skb = NULL;
			}

			txPkthdrRing[idx][txPktDoneDescIndex[idx]] &= ~DESC_SWCORE_OWNED;

			if (++txPktDoneDescIndex[idx] == txPkthdrRingCnt[idx])
				txPktDoneDescIndex[idx] = 0;

		}


	}


	SMP_UNLOCK_ETH_XMIT(flags);
	return ; //free_num;
}

#if defined(REINIT_SWITCH_CORE)

#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
int swNic_refillRxRing(void)
{
	unsigned long flags=0;
	unsigned int i;
	void *skb;
	unsigned char *buf;

	SMP_LOCK_ETH_RECV(flags);
	for(i =  0; i <RTL865X_SWNIC_RXRING_MAX_PKTDESC; i++)
	{
		while (return_to_rxing_check(i)) {
			skb=NULL;
			buf = alloc_rx_buf(&skb, size_of_cluster);

			if ((buf == NULL) ||(skb==NULL) ) {
				SMP_UNLOCK_ETH_RECV(flags);
				return -1;
			}

			release_pkthdr(skb, i);
		}
		REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
	}

	SMP_UNLOCK_ETH_RECV(flags);
	return 0;
}
#endif

int32 swNic_reConfigRxTxRing(void)
{
	uint32 i,j,k;
	//struct rtl_pktHdr	*pPkthdr;
	unsigned long flags=0;

	SMP_LOCK_ETH(flags);

	k = 0;

	for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++)
	{
		for (j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			/* Setup descriptors */
			rxPkthdrRing[i][j] = rxPkthdrRing[i][j] | DESC_SWCORE_OWNED;
			rxMbufRing[k] = rxMbufRing[k]  | DESC_SWCORE_OWNED;
			k++;
		}

		/* Initialize index of current Rx pkthdr descriptor */
		currRxPkthdrDescIndex[i] = 0;

		/* Initialize index of current Rx Mbuf descriptor */
		currRxMbufDescIndex = 0;

		/* Set wrap bit of the last descriptor */
		if(rxPkthdrRingCnt[i] > 0)
			rxPkthdrRing[i][rxPkthdrRingCnt[i] - 1] |= DESC_WRAP;

		#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		rxDescReadyForHwIndex[i] = 0;
		rxDescCrossBoundFlag[i] = 0;
		#endif
	}

	rxMbufRing[rxMbufRingCnt - 1] |= DESC_WRAP;


	for (i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++)
	{
		for (j = 0; j < txPkthdrRingCnt[i]; j++)
		{
			txPkthdrRing[i][j] &= ~DESC_SWCORE_OWNED;
		}

		if(txPkthdrRingCnt[i] > 0)
		{
			txPkthdrRing[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
		}	
		currTxPkthdrDescIndex[i] = 0;
		txPktDoneDescIndex[i]=0;
	}

	/* Fill Tx packet header FDP */
	REG32(CPUTPDCR0) = (uint32) txPkthdrRing[0];
	REG32(CPUTPDCR1) = (uint32) txPkthdrRing[1];
	
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	REG32(CPUTPDCR2) = (uint32) txPkthdrRing[2];
	REG32(CPUTPDCR3) = (uint32) txPkthdrRing[3];
#endif


	/* Fill Rx packet header FDP */
	REG32(CPURPDCR0) = (uint32) rxPkthdrRing[0];
	REG32(CPURPDCR1) = (uint32) rxPkthdrRing[1];
	REG32(CPURPDCR2) = (uint32) rxPkthdrRing[2];
	REG32(CPURPDCR3) = (uint32) rxPkthdrRing[3];
	REG32(CPURPDCR4) = (uint32) rxPkthdrRing[4];
	REG32(CPURPDCR5) = (uint32) rxPkthdrRing[5];

	REG32(CPURMDCR0) = (uint32) rxMbufRing;

	SMP_UNLOCK_ETH(flags);

	return 0;
}
int32 swNic_reInit(void)
{
	swNic_freeTxRing();
#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
	swNic_refillRxRing();
#endif
	swNic_reConfigRxTxRing();
	return SUCCESS;
}

#endif
//#pragma ghs section text=default
/*************************************************************************
*   FUNCTION
*       swNic_init
*
*   DESCRIPTION
*       This function initializes descriptors and data structures.
*
*   INPUTS
*       userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC] :
*          Number of Rx pkthdr descriptors of each ring.
*       userNeedRxMbufRingCnt :
*          Number of Tx mbuf descriptors.
*       userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC] :
*          Number of Tx pkthdr descriptors of each ring.
*       clusterSize :
*          Size of cluster.
*
*   OUTPUTS
*       Status.
*************************************************************************/

#if (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)) && defined(_PKTHDR_CACHEABLE)
#include <asm/cpu-features.h>
#endif

int32 swNic_init(uint32 userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC],
                 uint32 userNeedRxMbufRingCnt,
                 uint32 userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC],
                 uint32 clusterSize)
{
	uint32 i, j, k;
	static uint32 totalRxPkthdrRingCnt = 0, totalTxPkthdrRingCnt = 0;
	static struct rtl_pktHdr *pPkthdrList_start;
	static struct rtl_mBuf *pMbufList_start;
	struct rtl_pktHdr *pPkthdrList;
	struct rtl_mBuf *pMbufList;
	struct rtl_pktHdr * pPkthdr;
	struct rtl_mBuf * pMbuf;
	unsigned long flags=0;
	int	ret;
#ifdef ALLOW_RX_RING_PARTIAL_EMPTY
	int	mem_not_enough, ready_for_hw_idx;
#endif

#if defined(_PKTHDR_CACHEABLE)
	int _cpu_dcache_line; 
#if (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)) 
	_cpu_dcache_line = cpu_dcache_line_size(); // in \arch\mips\include\asm\cpu-features.h
#else
	_cpu_dcache_line = cpu_dcache_line;
#endif
#endif
	/* init const array for rx pre-process	*/
	extPortMaskToPortNum[0] = 5;
	extPortMaskToPortNum[1] = 6;
	extPortMaskToPortNum[2] = 7;
	extPortMaskToPortNum[3] = 5;
	extPortMaskToPortNum[4] = 8;
	extPortMaskToPortNum[5] = 5;
	extPortMaskToPortNum[6] = 5;
	extPortMaskToPortNum[7] = 5;

#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
	rxPkthdrRefillThreshold[0] = ETH_REFILL_THRESHOLD;
	rxPkthdrRefillThreshold[1] = ETH_REFILL_THRESHOLD1;
	rxPkthdrRefillThreshold[2] = ETH_REFILL_THRESHOLD2;
	rxPkthdrRefillThreshold[3] = ETH_REFILL_THRESHOLD3;
	rxPkthdrRefillThreshold[4] = ETH_REFILL_THRESHOLD4;
	rxPkthdrRefillThreshold[5] = ETH_REFILL_THRESHOLD5;
#endif

	#if defined(CONFIG_RTL8196C_REVISION_B)
	rtl_chip_version = REG32(REVR);
	#endif

	//rtlglue_printf("\n#######################################################\n");
	//rtlglue_printf("  NUM_RX_PKTHDR_DESC= %d, eth_skb_free_num= %d\n",
	//	NUM_RX_PKTHDR_DESC, get_buf_in_poll());
	//rtlglue_printf("#######################################################\n");

	ret = SUCCESS;
	SMP_LOCK_ETH_RECV(flags);
	if (rxMbufRing == NULL)
	{
		size_of_cluster = clusterSize;

		/* Allocate Rx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++) {
			rxPkthdrRingCnt[i] = userNeedRxPkthdrRingCnt[i];
			if (rxPkthdrRingCnt[i] == 0)
			{
				rxPkthdrRing[i] = NULL;
				continue;
			}

			rxPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(rxPkthdrRingCnt[i] * sizeof(uint32*));
			ASSERT_CSP( (uint32) rxPkthdrRing[i] & 0x0fffffff );

			totalRxPkthdrRingCnt += rxPkthdrRingCnt[i];
		}

		if (totalRxPkthdrRingCnt == 0) {
			ret = EINVAL;
			goto out;
		}

		/* Allocate Tx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_TXRING_HW_PKTDESC; i++) {
			txPkthdrRingCnt[i] = userNeedTxPkthdrRingCnt[i];

			if (txPkthdrRingCnt[i] == 0)
			{
				txPkthdrRing[i] = NULL;
				continue;
			}

			txPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32*));
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i]=(uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32));
			#endif

			ASSERT_CSP( (uint32) txPkthdrRing[i] & 0x0fffffff );

			totalTxPkthdrRingCnt += txPkthdrRingCnt[i];
		}

		if (totalTxPkthdrRingCnt == 0) {
			ret = EINVAL;
			goto out;
		}

		/* Allocate MBuf descriptors of rings */
		rxMbufRingCnt = userNeedRxMbufRingCnt;

		if (userNeedRxMbufRingCnt == 0) {
			ret = EINVAL;
			goto out;
		}

		rxMbufRing = (uint32 *) UNCACHED_MALLOC((rxMbufRingCnt+RESERVERD_MBUF_RING_NUM) * sizeof(uint32*));
		ASSERT_CSP( (uint32) rxMbufRing & 0x0fffffff );

		/* Allocate pkthdr */
#ifdef _PKTHDR_CACHEABLE

#if 0 //defined(CONFIG_RTL_8198C)
		pPkthdrList_start = (struct rtl_pktHdr *) kmalloc(
		(totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct rtl_pktHdr), GFP_ATOMIC);
		ASSERT_CSP( (uint32) pPkthdrList_start & 0x0fffffff );

		/* Allocate mbufs */
		pMbufList_start = (struct rtl_mBuf *) kmalloc(
		(rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+ totalTxPkthdrRingCnt) * sizeof(struct rtl_mBuf), GFP_ATOMIC);
		ASSERT_CSP( (uint32) pMbufList_start & 0x0fffffff );

#else
		pPkthdrList_start = (struct rtl_pktHdr *) kmalloc(
		(totalRxPkthdrRingCnt+totalTxPkthdrRingCnt+1) * sizeof(struct rtl_pktHdr), GFP_ATOMIC);
		ASSERT_CSP( (uint32) pPkthdrList_start & 0x0fffffff );

		pPkthdrList_start = (struct rtl_pktHdr *)(((uint32) pPkthdrList_start + (_cpu_dcache_line - 1))& ~(_cpu_dcache_line - 1));
		memset(pPkthdrList_start, 0, (totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct rtl_pktHdr));

		/* Allocate mbufs */
#ifdef CONFIG_RTL_ETH_TX_SG
		pMbufList_start = (struct rtl_mBuf *) kmalloc(
		(rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+(totalTxPkthdrRingCnt)+1+SG_MBUF_NUM+2) * sizeof(struct rtl_mBuf), GFP_ATOMIC);
#else
		pMbufList_start = (struct rtl_mBuf *) kmalloc(
		(rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+totalTxPkthdrRingCnt+1) * sizeof(struct rtl_mBuf), GFP_ATOMIC);
#endif
		ASSERT_CSP( (uint32) pMbufList_start & 0x0fffffff );
		
		pMbufList_start = (struct rtl_mBuf *)(((uint32) pMbufList_start + (_cpu_dcache_line - 1))& ~(_cpu_dcache_line - 1));
#ifdef CONFIG_RTL_ETH_TX_SG
		memset(pMbufList_start, 0, (rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+(totalTxPkthdrRingCnt)+SG_MBUF_NUM+2) * sizeof(struct rtl_mBuf));
#else
		memset(pMbufList_start, 0, (rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+ totalTxPkthdrRingCnt) * sizeof(struct rtl_mBuf));
#endif
#endif

#else
		pPkthdrList_start = (struct rtl_pktHdr *) UNCACHED_MALLOC(
		(totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct rtl_pktHdr));
		ASSERT_CSP( (uint32) pPkthdrList_start & 0x0fffffff );

		/* Allocate mbufs */
		pMbufList_start = (struct rtl_mBuf *) UNCACHED_MALLOC(
		(rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+ totalTxPkthdrRingCnt) * sizeof(struct rtl_mBuf));
		ASSERT_CSP( (uint32) pMbufList_start & 0x0fffffff );
#endif

#if defined( CONFIG_RTL_8198C)
		SMP_UNLOCK_ETH_RECV(flags);
		return ret;
#endif
	}

	/* Initialize interrupt statistics counter */
	//rxPktCounter = txPktCounter = 0;

	/* Initialize index of Tx pkthdr descriptor */
	for (i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++)
	{
		currTxPkthdrDescIndex[i] = 0;
		txPktDoneDescIndex[i]=0;
	}

	pPkthdrList = pPkthdrList_start;
	pMbufList = pMbufList_start;

	/* Initialize Tx packet header descriptors */
	for (i = 0; i < RTL865X_SWNIC_TXRING_HW_PKTDESC; i++)
	{
		for (j = 0; j < txPkthdrRingCnt[i]; j++)
		{
			/* Dequeue pkthdr and mbuf */
			pPkthdr = pPkthdrList++;
			pMbuf = pMbufList++;

			bzero((void *) pPkthdr, sizeof(struct rtl_pktHdr));
			bzero((void *) pMbuf, sizeof(struct rtl_mBuf));

			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len = 0;
			pPkthdr->ph_flags = PKTHDR_USED | PKT_OUTGOING;
			pPkthdr->ph_type = PKTHDR_ETHERNET;
			pPkthdr->ph_portlist = 0;

			pMbuf->m_next = NULL;
			pMbuf->m_pkthdr = pPkthdr;
			pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
			pMbuf->m_data = NULL;
			pMbuf->m_extbuf = NULL;
			pMbuf->m_extsize = 0;

			txPkthdrRing[i][j] = (int32) pPkthdr | DESC_RISC_OWNED;
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i][j]=(int32) pPkthdr | DESC_RISC_OWNED;
			#endif
		}

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
		txPkthdrRing_base[i] = txPkthdrRing[i][0];
#endif

		if(txPkthdrRingCnt[i] > 0)
		{
			/* Set wrap bit of the last descriptor */
			txPkthdrRing[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
			#endif
		}

	}

#ifdef CONFIG_RTL_ETH_TX_SG
	for (i = 0; i < SG_MBUF_NUM; i++) {
		pMbuf = pMbufList++;
		sg_mbuf[i] = (uint32)pMbuf;
	}
#endif

	/* Fill Tx packet header FDP */
	REG32(CPUTPDCR0) = (uint32) txPkthdrRing[0];
	REG32(CPUTPDCR1) = (uint32) txPkthdrRing[1];

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	REG32(CPUTPDCR2) = (uint32) txPkthdrRing[2];
	REG32(CPUTPDCR3) = (uint32) txPkthdrRing[3];
#endif

	/* Initialize Rx packet header descriptors */
	k = 0;

	for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++)
	{
#if defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		mem_not_enough = 0;
		ready_for_hw_idx = 0;
#endif

		for (j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			/* Dequeue pkthdr and mbuf */
			pPkthdr = pPkthdrList++;
			pMbuf = pMbufList++;

			bzero((void *) pPkthdr, sizeof(struct rtl_pktHdr));
			bzero((void *) pMbuf, sizeof(struct rtl_mBuf));

			/* Setup pkthdr and mbuf */
			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len = 0;
			pPkthdr->ph_flags = PKTHDR_USED | PKT_INCOMING;
			pPkthdr->ph_type = PKTHDR_ETHERNET;
			pPkthdr->ph_portlist = 0;
			pMbuf->m_next = NULL;
			pMbuf->m_pkthdr = pPkthdr;
			pMbuf->m_len = 0;
			pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
			pMbuf->m_extsize = size_of_cluster;

#if defined(ALLOW_RX_RING_PARTIAL_EMPTY)
			if (mem_not_enough == 0) {

				#ifdef PRIV_BUF_CAN_USE_KERNEL_BUF			
				pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf_init(&pPkthdr->ph_mbuf->skb, size_of_cluster);
				#else
				pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pPkthdr->ph_mbuf->skb, size_of_cluster);
				#endif

				if (pMbuf->m_data == NULL) {
					mem_not_enough = 1;
					ready_for_hw_idx = j;
					rxPkthdrRing[i][j] = (int32) pPkthdr;
					rxMbufRing[k++] = (int32) pMbuf;
				}
				else {
					/* Setup descriptors */
					rxPkthdrRing[i][j] = (int32) pPkthdr | DESC_SWCORE_OWNED;
					rxMbufRing[k++] = (int32) pMbuf | DESC_SWCORE_OWNED;
				}

			}
			else {
				pMbuf->m_data = pMbuf->m_extbuf = pPkthdr->ph_mbuf->skb = NULL;
				rxPkthdrRing[i][j] = (int32) pPkthdr;
				rxMbufRing[k++] = (int32) pMbuf;
			}
#else
			#ifdef PRIV_BUF_CAN_USE_KERNEL_BUF			
			pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf_init(&pPkthdr->ph_mbuf->skb, size_of_cluster);
			#else
			pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pPkthdr->ph_mbuf->skb, size_of_cluster);
			#endif
			
			#ifdef INIT_RX_RING_ERR_HANDLE
			if (pMbuf->m_data == NULL) { 
				rxPkthdrRingCnt[i] = j;
				rxMbufRingCnt = k;
				break;
			}
			else 
			#endif
			{
				/* Setup descriptors */
				rxPkthdrRing[i][j] = (int32) pPkthdr | DESC_SWCORE_OWNED;
				rxMbufRing[k++] = (int32) pMbuf | DESC_SWCORE_OWNED;
			}
#endif
		}

#ifdef CONFIG_RTL_ENHANCE_RELIABILITY
		rxPkthdrRing_base[i] = rxPkthdrRing[i][0] & ~DESC_OWNED_BIT;
#endif

		/* Initialize index of current Rx pkthdr descriptor */
		currRxPkthdrDescIndex[i] = 0;

		/* Initialize index of current Rx Mbuf descriptor */
		currRxMbufDescIndex = 0;

		/* Set wrap bit of the last descriptor */
		if(rxPkthdrRingCnt[i] > 0)
			rxPkthdrRing[i][rxPkthdrRingCnt[i] - 1] |= DESC_WRAP;

		#if	defined(DELAY_REFILL_ETH_RX_BUF)
		rxDescReadyForHwIndex[i] = 0;
		rxDescCrossBoundFlag[i] = 0;
		#endif

#if defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		if (mem_not_enough == 1) {
			rxDescReadyForHwIndex[i] = ready_for_hw_idx;
			rxDescCrossBoundFlag[i] = 1;
		}
		else {
			rxDescReadyForHwIndex[i] = 0;
			rxDescCrossBoundFlag[i] = 0;
		}
#endif
	}

#if defined(CONFIG_RTL_ENHANCE_RELIABILITY) && (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F))
	rxMbufRing_base = rxMbufRing[0] & ~DESC_OWNED_BIT;
#endif

	rxMbufRing[rxMbufRingCnt - 1] |= DESC_WRAP;

	/* Fill Rx packet header FDP */
	REG32(CPURPDCR0) = (uint32) rxPkthdrRing[0];
	REG32(CPURPDCR1) = (uint32) rxPkthdrRing[1];
	REG32(CPURPDCR2) = (uint32) rxPkthdrRing[2];
	REG32(CPURPDCR3) = (uint32) rxPkthdrRing[3];
	REG32(CPURPDCR4) = (uint32) rxPkthdrRing[4];
	REG32(CPURPDCR5) = (uint32) rxPkthdrRing[5];

	REG32(CPURMDCR0) = (uint32) rxMbufRing;

out:

#ifdef _PKTHDR_CACHEABLE
	_dma_cache_wback_inv((unsigned long)pPkthdrList_start, (totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct rtl_pktHdr));
#ifdef CONFIG_RTL_ETH_TX_SG
	_dma_cache_wback_inv((unsigned long)pMbufList_start, (rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+(totalTxPkthdrRingCnt)+SG_MBUF_NUM+2) * sizeof(struct rtl_mBuf));
#else
	_dma_cache_wback_inv((unsigned long)pMbufList_start, (rxMbufRingCnt+RESERVERD_MBUF_RING_NUM+ totalTxPkthdrRingCnt) * sizeof(struct rtl_mBuf));
#endif
#endif

	SMP_UNLOCK_ETH_RECV(flags);
	return ret;
}


#ifdef FAT_CODE
/*************************************************************************
*   FUNCTION
*       swNic_resetDescriptors
*
*   DESCRIPTION
*       This function resets descriptors.
*
*   INPUTS
*       None.
*
*   OUTPUTS
*       None.
*************************************************************************/
void swNic_resetDescriptors(void)
{
    /* Disable Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    return;
}
#endif//FAT_CODE

#if 	defined(CONFIG_RTL_PROC_DEBUG)
/*	dump the rx ring info	*/
int32	rtl_dumpRxRing(struct seq_file *s)
{
	int	idx, cnt;
	struct rtl_pktHdr * pPkthdr;

	for(idx=0;idx<RTL865X_SWNIC_RXRING_HW_PKTDESC;idx++)
	{
#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
		seq_printf(s,"**********************************************\nRxRing%d: cnt %d crossFlags[%d]\n",
			idx, rxPkthdrRingCnt[idx], rxDescCrossBoundFlag[idx]);
#else
		seq_printf(s,"**********************************************\nRxRing%d: cnt %d\n",
			idx, rxPkthdrRingCnt[idx]);
#endif
		/*	skip the null rx ring */
		if (rxPkthdrRingCnt[idx]==0)
			continue;

		/*	dump all the pkt header	*/
		for(cnt=0;cnt<rxPkthdrRingCnt[idx];cnt++)
		{
			pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));

#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
			seq_printf(s,"  idx[%03d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb,
				(rxPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(rxPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currRxPkthdrDescIndex[idx]?"  <===currIdx":"",
				cnt ==rxDescReadyForHwIndex[idx]?" <===readyForHw":"");
#else
			seq_printf(s,"  idx[%03d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb,
				(rxPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(rxPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currRxPkthdrDescIndex[idx]?"  <===currIdx":"");
#endif
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(cnt, WATCHDOG_NUM_OF_TIMES);
			#endif

		}
	}
	return SUCCESS;
}

/*	dump the tx ring info	*/
int32	rtl_dumpTxRing(struct seq_file *s)
{
	int	idx, cnt;
	struct rtl_pktHdr * pPkthdr = NULL;

	for(idx=0;idx<RTL865X_SWNIC_TXRING_HW_PKTDESC;idx++)
	{
		seq_printf(s,"**********************************************\nTxRing%d: cnt %d\n",
			idx, txPkthdrRingCnt[idx]);

		/*	skip the null rx ring */
		if (txPkthdrRingCnt[idx]==0)
			continue;

		/*	dump all the pkt header	*/
		for(cnt=0;cnt<txPkthdrRingCnt[idx];cnt++)
		{
 #ifdef CONFIG_RTL8196C_REVISION_B
		  if (rtl_chip_version == RTL8196C_REVISION_A)
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing_backup[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));
		  else
#endif
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));

			seq_printf(s,"  idx[%03d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb,
				(txPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(txPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currTxPkthdrDescIndex[idx]?"  <===currIdx":"",
				cnt==txPktDoneDescIndex[idx]?"  <===txDoneIdx":"");
			
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(cnt, WATCHDOG_NUM_OF_TIMES);
			#endif

		}
	}
	return SUCCESS;
}

/*	dump the tx ring info	*/
int32	rtl_dumpMbufRing(struct seq_file *s)
{
	int	idx;
	struct rtl_mBuf *mbuf;

	idx = 0;
	seq_printf(s,"**********************************************\nMbufRing:\n");
	while(1)
	{
		mbuf = (struct rtl_mBuf *)(rxMbufRing[idx] & ~(DESC_OWNED_BIT | DESC_WRAP));
		seq_printf(s,"mbuf[%03d]: 0x%p: ==> pkthdr[0x%p] ==> skb[0x%p]%s%s%s\n", idx, mbuf, mbuf->m_pkthdr,
				mbuf->skb,
				(rxMbufRing[idx]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE",
				(rxMbufRing[idx]&DESC_WRAP)==DESC_ENG_OWNED?" :WRAP":"",
				idx==currRxMbufDescIndex?"  <===currIdx":"");
			if ((rxMbufRing[idx]&DESC_WRAP)!=0)
				break;
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(idx, WATCHDOG_NUM_OF_TIMES);
			#endif
			idx++;
	}
	return SUCCESS;
}

/* dump brief info */
int32 rtl_dumpIndexs(void)
{
	int	i;

	rtlglue_printf("Dump RX infos:\n");
#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
	for(i=0;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++) {
		rtlglue_printf("	TotalCnt: %d, currPkthdrIdx: %d currMbufIdx: %d readyForHwIdx: %d crossBoundFlag: %d.\n",
			rxPkthdrRingCnt[i], currRxPkthdrDescIndex[i], currRxMbufDescIndex, rxDescReadyForHwIndex[i], rxDescCrossBoundFlag[i]);
	}
#else
	for(i=0;i<RTL865X_SWNIC_RXRING_HW_PKTDESC;i++) {
		rtlglue_printf("	TotalCnt: %d, currPkthdrIdx: %d currMbufIdx: %d\n",
			rxPkthdrRingCnt[i], currRxPkthdrDescIndex[i], currRxMbufDescIndex);
	}
#endif

	rtlglue_printf("\nDump TX infos:\n");
	for(i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++) {
		rtlglue_printf("	TotalCnt: %d, currPkthdrIdx: %d pktDoneIdx: %d.\n",
			txPkthdrRingCnt[i], currTxPkthdrDescIndex[i], txPktDoneDescIndex[i]);
	}

	return SUCCESS;
}
#endif

#if defined(REINIT_SWITCH_CORE)
int32 rtl_check_tx_done_desc_swCore_own(int32 *tx_done_inx)
{
	int ret = FAILED;
	int tx_ring_idx = 0;   //default use tx ring0

	if ((*(volatile uint32 *)&txPkthdrRing[tx_ring_idx][txPktDoneDescIndex[tx_ring_idx]]&DESC_OWNED_BIT) == DESC_SWCORE_OWNED) {
		*tx_done_inx = txPktDoneDescIndex[tx_ring_idx];
		ret = SUCCESS;
	}

	return ret;
}
#endif


