/*
 *	Abstract: provide NIC initialization, reception	and	transmission
 *			  functions	for	new	descriptor architecture.
 *
 *	Author:
 *		Joey Lin, <joey.lin@realtek.com>
 *
 *	Copyright (c) 2015 Realtek Semiconductor Corp.
 *
 *	This program is	free software; you can redistribute	it and/or
 *	modify it under	the	terms of the GNU General Public	License
 *	as published by	the	Free Software Foundation; either version
 *	2 of the License, or (at your option) any later	version.
 */

#if	0
TODO: refer	to /linux-3.10/drivers/net/ethernet/broadcom/bcm63xx_enet.c
			   /linux-3.10/drivers/net/ethernet/realtek/r8169.c
	  and follow them to use those functions

rtl_open()
{
	/*
	 * Rx and Tx descriptors needs 256 bytes alignment.
	 * dma_alloc_coherent provides more.
	 */
	tp->RxDescArray	= dma_alloc_coherent(&pdev->dev, R8169_RX_RING_BYTES,
						 &tp->RxPhyAddr, GFP_KERNEL);
	.....
	smp_mb();
	.....
	dma_free_coherent(&pdev->dev, R8169_RX_RING_BYTES, tp->RxDescArray,
			  tp->RxPhyAddr);
}

static struct sk_buff *rtl8169_alloc_rx_data(struct	rtl8169_private	*tp,
						 struct	RxDesc *desc)
{
	.....
	mapping	= dma_map_single(d,	rtl8169_align(data), rx_buf_sz,
				 DMA_FROM_DEVICE);
	.....
}

static netdev_tx_t rtl8169_start_xmit(struct sk_buff *skb,
					  struct net_device	*dev)
{
	.....
	mapping	= dma_map_single(d,	skb->data, len,	DMA_TO_DEVICE);
	.....
	wmb();

	wmb();

	mmiowb();

		smp_wmb();

		smp_mb();

	.....
}
#endif

#include <linux/version.h>
#include <linux/kconfig.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0))
#include <linux/if_ether.h>
#include <linux/in.h>
#else
#include <uapi/linux/if_ether.h>
#include <uapi/linux/in.h>
#endif
#if 1//defined(CONFIG_RTL_97F_HW_TX_CSUM) && defined(CONFIG_RTL_EXT_PORT_SUPPORT)
#include <linux/if_vlan.h>
#endif
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <asm/cpu-features.h>
#include "common/rtl_errno.h"
#include "AsicDriver/asicRegs.h"
#include <net/rtl/rtl865x_netif.h>
#include "rtl865xc_swNic.h"
#include "common/mbuf.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "rtl819x_swNic.h"

//#include "m24kctrl.h" //added for romperf testing	
#if defined(CONFIG_RTL_DNS_TRAP)
extern int dns_filter_enable;
#endif

#if	defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
extern unsigned	int	rx_noBuffer_cnt;
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
extern int gHwNatEnabled;
#endif

#if defined (CONFIG_RTL_EXT_PORT_SUPPORT)
__DRAM_FWD static uint8 extPortMaskToPortNum[_RTL865XB_EXTPORTMASKS+1] =
{
	5, 6, 7, 5, 8, 5, 5, 5
};
#endif

struct ring_info {
	unsigned int		skb;
//	unsigned int		frag;	// no used now
};

static struct ring_info	*rx_skb[NEW_NIC_MAX_RX_DESC_RING];
static struct ring_info	*tx_skb[NEW_NIC_MAX_TX_DESC_RING];

__DRAM_FWD static int32		 New_rxDescRingCnt[NEW_NIC_MAX_RX_DESC_RING];
static DMA_RX_DESC			*New_rxDescRing[NEW_NIC_MAX_RX_DESC_RING] =	{NULL};
__DRAM_FWD static int32		 New_currRxPkthdrDescIndex[NEW_NIC_MAX_RX_DESC_RING];

__DRAM_FWD static uint32	 New_txDescRingCnt[NEW_NIC_MAX_TX_DESC_RING];
static DMA_TX_DESC			*New_txDescRing[NEW_NIC_MAX_TX_DESC_RING];

__DRAM_FWD static uint32 New_currTxPkthdrDescIndex[NEW_NIC_MAX_TX_DESC_RING];
__DRAM_FWD static uint32 New_txPktDoneDescIndex[NEW_NIC_MAX_TX_DESC_RING];

__DRAM_FWD static uint32 New_txDescRing_base[RTL865X_SWNIC_TXRING_HW_PKTDESC];
__DRAM_FWD static uint32 New_rxDescRing_base[RTL865X_SWNIC_RXRING_HW_PKTDESC];

static uint32 TxCDP_reg[RTL865X_SWNIC_TXRING_HW_PKTDESC] =
	{ CPUTPDCR0, CPUTPDCR1,	CPUTPDCR2, CPUTPDCR3};

#define	NEXT_IDX(N,RING_SIZE)	( ((N+1) ==	RING_SIZE) ? 0 : (N+1) )
#define CIRC_SPACE_2(curr,done,size) ((done > curr) ? (done-curr) : (done+size-curr))

#define _DESC_CACHE_ACCESS_RX			1

// must	define USE_SWITCH_TX_CDP when use _DESC_CACHE_ACCESS_TX
/*
  when _DESC_CACHE_ACCESS_TX and USE_SWITCH_TX_CDP are both defined,
  in New_swNic_txDone(), for example,
  it will happen: 
    New_currTxPkthdrDescIndex[0] = 16
    New_txPktDoneDescIndex[0] = 12
    hw_cdp_idx = 17
  after New_swNic_txDone() done, will be:
    New_currTxPkthdrDescIndex[0] = 16
    New_txPktDoneDescIndex[0] = 17
    hw_cdp_idx = 17
  ==> Tx will STOP.
 */
//#define	_DESC_CACHE_ACCESS_TX		1

/* since _DESC_CACHE_ACCESS_TX can not used, we use a local TxDesc to store
   the setting, then copy the local version to physical desc memory once.
 */
#define _LOCAL_TX_DESC	1
static struct tx_desc local_txd;

#define	USE_SWITCH_TX_CDP	1 // use Switch	Tx Current Descriptor Pointer, instead of OWN bit.

/*
  in my	test in	FPGA, RxRing0 desc number is only 32,
  I	have connect 1 LAN PC and 1	WAN	PC,	then boot up kernel,
  after	the	ethernet interface up, and New_swNic_receive() be executed first time,
  the sw_curr_idx is 0 and hw_desc_idx is 0,
  but HW is	received 32	pkt	and	desc ran out.

  MUST figure out a	solution to	fix	above situation,
  then USE_SWITCH_RX_CDP flag can be used.
 */
#define USE_SWITCH_RX_CDP	1 // use Switch	Rx CDP

void New_dumpTxRing(struct seq_file	*s)
{
	uint32 i,j;
	struct sk_buff *skb;
	uint32 *p;

	seq_printf(s,"=========================	dump tx	ring =========================\n");

	for	(i = 0;	i <	NEW_NIC_MAX_TX_DESC_RING; i++)
	{
		seq_printf(s,"====== ring %d ======\n",i);
		seq_printf(s,"txDoneIdx[%d]: %d, txCurrIdx[%d]:	%d\n",i,
			New_txPktDoneDescIndex[i],i,New_currTxPkthdrDescIndex[i]);
		seq_printf(s,"HW CDP: 0x%x,	HW idx (calculated): %d\n",
			REG32(TxCDP_reg[i]), (REG32(TxCDP_reg[i]) -	New_txDescRing_base[i])	/ sizeof(struct	dma_tx_desc));

		for	(j = 0;	j <	New_txDescRingCnt[i]; j++)
		{
			seq_printf(s,"tx_desc[%d][%d]: 0x%x,",i,j,(uint32)(&New_txDescRing[i][j]) );

			p =	(uint32	*)(&New_txDescRing[i][j]);
			p =	(uint32	*)(((uint32)p) | UNCACHE_MASK);
			seq_printf(s,"	%s owned\n", ((*p) & DESC_OWNED_BIT) ? "SWCORE"	: "CPU"	);

			seq_printf(s,"	 6DW: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
					(uint32)(*p), (uint32)*(p+1),
					(uint32)*(p+2),	(uint32)*(p+3),
					(uint32)*(p+4),	(uint32)*(p+5) );
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(j, WATCHDOG_NUM_OF_TIMES);
			#endif

		}
		seq_printf(s,"\n");
	}

	seq_printf(s,"=========================	dump tx	skb	=========================\n");
	for	(i = 0;	i <	NEW_NIC_MAX_TX_DESC_RING; i++)
	{
		seq_printf(s,"====== for ring %d ======\n",i);
		for	(j = 0;	j <	New_txDescRingCnt[i]; j++)
		{
			skb	= (struct sk_buff *)tx_skb[i][j].skb;
			if (skb	== NULL)
				seq_printf(s,"[%d][%d]: tx_skb: %x, skb: NULL\n",i,j,
					(uint32)(&tx_skb[i][j])	);
			else
				seq_printf(s,"[%d][%d]: tx_skb: %x, skb: %x, skb->data: %x\n",i,j,
					(uint32)(&tx_skb[i][j]), (uint32)tx_skb[i][j].skb,
					(uint32)skb->data);
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(j, WATCHDOG_NUM_OF_TIMES);
			#endif

		}
		seq_printf(s,"\n");
	}
}

void New_dumpRxRing(struct seq_file	*s)
{
	uint32 i,j;
	struct sk_buff *skb;
	uint32 *p;

	seq_printf(s,"=========================	dump rx	ring =========================\n");

	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		seq_printf(s,"====== ring %d ======\n",i);
		seq_printf(s,"currRxIdx[%d]: %d\n",i,New_currRxPkthdrDescIndex[i]);

		seq_printf(s,"HW CDP: 0x%x,	HW idx (calculated): %d\n",
			REG32(CPURPDCR(i)),	(REG32(CPURPDCR(i))	- New_rxDescRing_base[i]) /	sizeof(struct dma_rx_desc));

		for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
		{
			seq_printf(s,"rx_desc[%d][%d]: 0x%x,",i,j,(uint32)(&New_rxDescRing[i][j]) );

			p =	(uint32	*)(&New_rxDescRing[i][j]);
			p =	(uint32	*)(((uint32)p) | UNCACHE_MASK);
			seq_printf(s,"	%s owned\n", ((*p)&DESC_OWNED_BIT) ? "SWCORE" :	"CPU" );

			seq_printf(s,"	 6DW: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
					(uint32)(*p), (uint32)*(p+1),
					(uint32)*(p+2),	(uint32)*(p+3),
					(uint32)*(p+4),	(uint32)*(p+5) );
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(j, WATCHDOG_NUM_OF_TIMES);
			#endif

		}
		seq_printf(s,"\n");
	}

	seq_printf(s,"=========================	dump rx	skb	=========================\n");
	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		seq_printf(s,"====== for ring %d ======\n",i);
		for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
		{
			skb	= (struct sk_buff *)rx_skb[i][j].skb;
			if (skb	== NULL)
				seq_printf(s,"[%d][%d]: rx_skb: %x, skb: NULL\n",i,j,
					(uint32)(&rx_skb[i][j])	);
			else
				seq_printf(s,"[%d][%d]: rx_skb: %x, skb: %x, skb->data: %x\n",i,j,
						(uint32)(&rx_skb[i][j]), (uint32)rx_skb[i][j].skb,
						(uint32)skb->data);
			#if defined(CONFIG_RTL_WTDOG)
			rtl_periodic_watchdog_kick(j, WATCHDOG_NUM_OF_TIMES);
			#endif

		}
		seq_printf(s,"\n");
	}
}

int32 New_swNic_init(uint32	userNeedRxPkthdrRingCnt[NEW_NIC_MAX_RX_DESC_RING],
				 uint32	userNeedTxPkthdrRingCnt[NEW_NIC_MAX_TX_DESC_RING],
				 uint32	clusterSize)
{
	uint32 i, j;
	static uint32 totalRxPkthdrRingCnt = 0,	totalTxPkthdrRingCnt = 0;
	unsigned long flags=0;
	int	ret	= SUCCESS;
	int	_cpu_dcache_line = cpu_dcache_line_size(); // in \arch\mips\include\asm\cpu-features.h

	SMP_LOCK_ETH_RECV(flags);

	/* 1. Allocate Rx descriptor rings */
	if (New_rxDescRing[0] == NULL) {

	size_of_cluster = clusterSize;
	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++) {
		New_rxDescRingCnt[i] = userNeedRxPkthdrRingCnt[i];
		if (New_rxDescRingCnt[i] ==	0)
		{
			New_rxDescRing[i] =	NULL;
			continue;
		}

		New_rxDescRing[i] =	(struct	dma_rx_desc	*) CACHED_MALLOC((New_rxDescRingCnt[i]) * sizeof(struct dma_rx_desc)+(2*_cpu_dcache_line));
		ASSERT_CSP(	(uint32) New_rxDescRing[i] & 0x0fffffff	);
		New_rxDescRing[i] =	(struct	dma_rx_desc	*)(((uint32) New_rxDescRing[i] + (_cpu_dcache_line - 1))& ~(_cpu_dcache_line - 1));
		memset(New_rxDescRing[i],0,New_rxDescRingCnt[i]	* sizeof(struct	dma_rx_desc));
		totalRxPkthdrRingCnt +=	New_rxDescRingCnt[i];

		if(New_rxDescRingCnt[i]){

			//rx_skb[i]=(struct	ring_info *)UNCACHED_MALLOC(sizeof(struct ring_info)*New_rxDescRingCnt[i]);
			rx_skb[i]=(struct ring_info	*)CACHED_MALLOC(sizeof(struct ring_info)*New_rxDescRingCnt[i]);

			if (!rx_skb[i])
			{
				goto err_out;
			}
			memset(rx_skb[i],0,New_rxDescRingCnt[i]	* sizeof(struct	ring_info));
		}
	}

	if (totalRxPkthdrRingCnt ==	0) {
		goto err_out;
	}

	/* 2. Allocate Tx descriptor rings */
	for	(i = 0;	i <	NEW_NIC_MAX_TX_DESC_RING; i++) {
		New_txDescRingCnt[i] = userNeedTxPkthdrRingCnt[i];

		if (New_txDescRingCnt[i] ==	0)
		{
			New_txDescRing[i] =	NULL;
			continue;
		}

		New_txDescRing[i] =	(struct	dma_tx_desc	*)CACHED_MALLOC((New_txDescRingCnt[i] ) * sizeof(struct dma_tx_desc)+ (_cpu_dcache_line*2));
		ASSERT_CSP(	(uint32) New_txDescRing[i] & 0x0fffffff	);
		New_txDescRing[i] =	(struct	dma_tx_desc	*)(((uint32) New_txDescRing[i] + (_cpu_dcache_line - 1))& ~(_cpu_dcache_line - 1));
		memset(New_txDescRing[i],0,New_txDescRingCnt[i]	* sizeof(struct	dma_tx_desc));
		totalTxPkthdrRingCnt +=	New_txDescRingCnt[i];

		if(New_txDescRingCnt[i]){

			tx_skb[i]=(struct ring_info	*)CACHED_MALLOC(sizeof(struct ring_info)*New_txDescRingCnt[i]);
			if (!tx_skb[i])
			{
				goto err_out;
			}
			memset(tx_skb[i],0,New_txDescRingCnt[i]	* sizeof(struct	ring_info));
		}
	}

	if (totalTxPkthdrRingCnt ==	0) {
		goto err_out;
	}
	}

	/* Initialize index	of Tx pkthdr descriptor	*/
	for	(i=0;i<NEW_NIC_MAX_TX_DESC_RING;i++)
	{
		for (j = 0; j < New_txDescRingCnt[i]; j++)
			New_txDescRing[i][j].opts1 &= ~DESC_SWCORE_OWNED; // Set all the tx desc to RISC owned

		New_txDescRing[i][New_txDescRingCnt[i] - 1].opts1 |= DESC_WRAP;
		New_currTxPkthdrDescIndex[i] = 0;
		New_txPktDoneDescIndex[i]=0;
		New_txDescRing_base[i] = ((uint32) New_txDescRing[i]) |	UNCACHE_MASK;
		REG32(TxCDP_reg[i])	= New_txDescRing_base[i];

		if (New_txDescRing[i] != NULL)
			_dma_cache_wback_inv((unsigned long)New_txDescRing[i],(unsigned	long)(New_txDescRingCnt[i] * sizeof(struct dma_tx_desc)));
	}

	/* Maximum TX packet header 0 descriptor entries.
	   the register only provide 16-bit for each tx ring,
	   this way (set TX_RING0_TAIL_AWARE bit) CAN NOT used when tx desc number is larger than 2730.
	 */
	REG32(DMA_CR1) = (New_txDescRingCnt[0]-1) * (sizeof(struct tx_desc)); 
	REG32(DMA_CR4) = TX_RING0_TAIL_AWARE;
	
	/* Initialize Rx packet	header descriptors */
	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
		{
			void *skb;

			New_rxDescRing[i][j].addr =
				(uint32)alloc_rx_buf((void **)&skb,	clusterSize);

			// todo: error check, if addr/skb =	NULL, ....
			rx_skb[i][j].skb = (uint32)skb;

			if (j == (New_rxDescRingCnt[i] - 1))
				New_rxDescRing[i][j].opts1 = (DESC_SWCORE_OWNED	| DESC_WRAP	|(clusterSize<<RD_M_EXTSIZE_OFFSET));
			else
				New_rxDescRing[i][j].opts1 = (DESC_SWCORE_OWNED	|(clusterSize<<RD_M_EXTSIZE_OFFSET)	);
		}
		/* Initialize index	of current Rx pkthdr descriptor	*/
		New_currRxPkthdrDescIndex[i] = 0;
		New_rxDescRing_base[i] = ((uint32) New_rxDescRing[i]) |	UNCACHE_MASK;
		REG32(CPURPDCR(i)) = New_rxDescRing_base[i];

		if (New_rxDescRing[i] != NULL)
			_dma_cache_wback_inv((unsigned long)New_rxDescRing[i],(unsigned	long)(New_rxDescRingCnt[i] * sizeof(struct dma_rx_desc)));
	}

	SMP_UNLOCK_ETH_RECV(flags);
	return ret;

err_out:
	SMP_UNLOCK_ETH_RECV(flags);
	return (-EINVAL);
}

#define	SET_OWN_BIT(desc) \
	desc->rx_own = DESC_SWCORE_OWNED;

#define	INC_IDX(ring, idx) \
	idx	= NEXT_IDX(idx,New_rxDescRingCnt[ring]);	\
	New_currRxPkthdrDescIndex[ring]	= idx;

#define	REUSE_DESC() \
	SET_OWN_BIT(desc); \
	INC_IDX(ring_idx, rx_idx); 


#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_EXT_PORT_SUPPORT)
extern int extPortEnabled;

__MIPS16
__IRAM_FWD
inline int32 New_rtl8651_rxPktPreprocess(void *pkt, uint16 *vid)
{
	struct rx_desc *desc = (struct rx_desc *)pkt;
	uint32 srcPortNum = 0;
	
	srcPortNum = desc->rx_spa;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	srcPortNum &=0x7;
#endif
	*vid = desc->rx_dvlanid;

	if (!extPortEnabled)
		return SUCCESS;
	
	if (srcPortNum < RTL8651_CPU_PORT)
	{
		#if defined (CONFIG_RTL_EXT_PORT_SUPPORT) 
		/* rx from physical port */
		if(extPortEnabled&&((desc->rx_dp_ext & PKTHDR_EXTPORTMASK_CPU) == 0))
		{	
			/* No CPU bit, finished nat translation, only dest ext mbr port... */
			/*
				if dest ph_extPortList 0x1 => to dst ext port 1 => to dst port 1+5=6
				if dest ph_extPortList 0x2 => to dst ext port 2 => to dst port 2+5=7
				if dest ph_extPortList 0x4 => to dst ext port 3 => to dst port 3+5=8
			*/
			/* No CPU bit, only dest ext mbr port... */
			if(	desc->rx_dp_ext &&
				(5!=extPortMaskToPortNum[desc->rx_dp_ext]))
			{
				#if defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
				
				
				//panic_printk("extPortList:%d,srcPortNum:%d,isOriginal:%d,l2Trans:%d,fwd:%d,*vid=%d [%s]:[%d].\n",
				//desc->rx_dp_ext,srcPortNum,desc->rx_org,desc->rx_l2act,desc->rx_fwd, *vid, __FUNCTION__,__LINE__);
				if(desc->rx_l2act!=0) 
				{
					/*wan-->ext port, finished napt translation*/
					*vid = PKTHDR_EXTPORT_MAGIC;
					#if 0
					if (net_ratelimit())
						panic_printk("extPortList:%d,srcPortNum:%d,isOriginal:%d,l2Trans:%d,fwd:%d,*vid=%d [%s]:[%d].\n",
						desc->rx_dp_ext,srcPortNum,desc->rx_org,desc->rx_l2act,desc->rx_fwd, *vid, __FUNCTION__,__LINE__);
					#endif
				}
				#endif
			}
			else
			{
				/*no dst ext port*/
				/*drop it*/
				#if 0
				if (net_ratelimit())
					panic_printk("extPortList:%d,srcPortNum:%d,isOriginal:%d,l2Trans:%d,fwd:%d,*vid=%d [%s]:[%d].\n",
					desc->rx_dp_ext,srcPortNum,desc->rx_org,desc->rx_l2act,desc->rx_fwd, *vid, __FUNCTION__,__LINE__);
				#endif
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
				if ((desc->rx_dp_ext & PKTHDR_EXTPORTMASK_CPU) == 0)
				{
					
					/* No CPU bit, finished nat translation, only dest ext mbr port... */
					/*
						if dest ph_extPortList 0x1 => to dst ext port 1 => to dst port 1+5=6
						if dest ph_extPortList 0x2 => to dst ext port 2 => to dst port 2+5=7
						if dest ph_extPortList 0x4 => to dst ext port 3 => to dst port 3+5=8
					*/
					if(	desc->rx_dp_ext &&
						(5!=extPortMaskToPortNum[desc->rx_dp_ext]))
					{
						#if defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
						/*WISP mode ext port-->ext port, finished napt translation*/
						//*vid = PKTHDR_EXTPORT_MAGIC2;
						#if 0
						if (net_ratelimit())
							panic_printk("extPortList:%d,srcPortNum:%d,isOriginal:%d,l2Trans:%d,fwd:%d,*vid=%d [%s]:[%d].\n",
							desc->rx_dp_ext,srcPortNum,desc->rx_org,desc->rx_l2act,desc->rx_fwd, *vid, __FUNCTION__,__LINE__);
						#endif
						/*drop it, may fix me in future*/
						return FAILED;
						#endif
					}
					else
					{
						/*no dst ext port*/
						/*drop it*/
						#if 0
						if (net_ratelimit())
							panic_printk("extPortList:%d,srcPortNum:%d,isOriginal:%d,l2Trans:%d,fwd:%d,*vid=%d [%s]:[%d].\n",
							desc->rx_dp_ext,srcPortNum,desc->rx_org,desc->rx_l2act,desc->rx_fwd, *vid, __FUNCTION__,__LINE__);
						#endif
						return FAILED;
					}
				
				
				}
				else
				{
					/* has CPU bit, pkt is original pkt from port 6~8 */
					/*case 1: not finished napt translation, need roll back interface to wlan ext interface*/
					
					#if defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
					*vid = PKTHDR_EXTPORT_MAGIC4;
					//srcPortNum = extPortMaskToPortNum[desc->rx_extspa];
					srcPortNum = desc->rx_extspa + 5; 	
#ifdef _DESC_CACHE_ACCESS_RX
					desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
#endif
					desc->rx_spa =srcPortNum;
					#if 0
					if (net_ratelimit())
						panic_printk("extPortList:%d,srcPortNum:%d,isOriginal:%d,l2Trans:%d,fwd:%d,*vid=%d desc->rx_extspa=%d [%s]:[%d].\n",
						desc->rx_dp_ext,srcPortNum,desc->rx_org,desc->rx_l2act,desc->rx_fwd, *vid,desc->rx_extspa,  __FUNCTION__,__LINE__);
					#endif
					/*drop it, may fix me in future*/
					return SUCCESS;
					#endif
				}
			}else
			{
				#if 0
				if (net_ratelimit())
					panic_printk("extPortList:%d,desc->rx_extspa:%d, srcPortNum:%d,isOriginal:%d,l2Trans:%d,fwd:%d,*vid=%d [%s]:[%d].\n",
					desc->rx_dp_ext,desc->rx_extspa, srcPortNum,desc->rx_org,desc->rx_l2act,desc->rx_fwd, *vid, __FUNCTION__,__LINE__);
				#endif
				return FAILED;
			}
		   
#else
		return FAILED;
#endif
	
	}

	return SUCCESS;
}
#endif

int32 New_swNic_receive(rtl_nicRx_info *info, int retryCount)
{
	uint32 rx_idx, ring_idx;
	struct sk_buff *r_skb;
	struct rx_desc *desc;
	unsigned char *buf;
	void *skb;
	#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	uint16	vid;
	#endif

	#ifdef USE_SWITCH_RX_CDP
	uint32 cdp_value;
	int	hw_idx;
	#endif

	#if	!defined(CONFIG_SMP)
	unsigned long flags	= 0;
	SMP_LOCK_ETH_RECV(flags);
	#endif

	#if	defined(RTL_MULTIPLE_RX_TX_RING)
	for(ring_idx = (NEW_NIC_MAX_RX_DESC_RING-1);ring_idx >=	(info->priority);ring_idx--)
	#else
	for(ring_idx=0;ring_idx<1;ring_idx++)
	#endif
	{
get_next:
		rx_idx = New_currRxPkthdrDescIndex[ring_idx];

		#ifdef USE_SWITCH_RX_CDP
		cdp_value =	REG32(CPURPDCR(ring_idx));
		hw_idx = (cdp_value	- New_rxDescRing_base[ring_idx]) / sizeof(struct dma_rx_desc);
		if (rx_idx == hw_idx) { // no incoming pkt or Rx desc runout
			// check runout case
			desc = (struct rx_desc *)&New_rxDescRing[ring_idx][rx_idx];
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			if (desc->rx_own)	// SWCORE owned
			// end of "check runout case"
				
				continue; // go for next rx ring
		}
		#endif

		desc = (struct rx_desc *)&New_rxDescRing[ring_idx][rx_idx];

		// NEEDED, do not removed. when	previous interrupt_dsr_rx()	is finished,
		//		the	New_rxDescRing[ring_idx][rx_idx] has been read,	and	found "no more packet",
		//		so its address is cached.
		//		if _dma_cache_inv()	removed, and one packet	come in, read the desc and it own bit
		//		is still SWCORE	owned.
		// TODO: need to test, remove this one,	and	add	_dma_cache_inv to the next IF (own bit is SWCORE owned)

		// todo: in	FPGA, when rx_desc use cacheable and do	_dma_cache_inv,	the	rx ring	will abnormal
		#ifdef _DESC_CACHE_ACCESS_RX
		_dma_cache_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
		#else
		desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
		#endif

		#ifndef	USE_SWITCH_RX_CDP
		//1.check desc's own bit
		if (desc->rx_own) //1: SWCORE owned
		{
			continue; // go	for	next rx	ring
		}
		#endif

		// if checksum failed, reuse this desc, except the ipv6 ready logo case.
		if (!(desc->rx_l3csok) || !(desc->rx_l4csok))
		{
			#ifdef CONFIG_RTL_IPV6READYLOGO
			if (!( !(desc->rx_l4csok) && 
				   (desc->rx_ipv6) && (desc->rx_reason==0x60)))
			#endif	
			{
				#ifdef _DESC_CACHE_ACCESS_RX
				desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
				#endif
				REUSE_DESC();
				goto get_next;
			}
		}

		// porting from swNic_receive(), need or not ??
		#if 0 //defined(CONFIG_RTL_HARDWARE_NAT)
		if (desc->rx_spa >= RTL8651_CPU_PORT) {
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}
		#endif
		
		//2.rx skb
		r_skb =	(struct	sk_buff	*)(rx_skb[ring_idx][rx_idx].skb);
		if (r_skb == NULL) {	// should not happen
			// todo: may call alloc_rx_buf() to	get	a new skb,....
			//printk("ETH_ERR: skb_array[%d][%d] is NULL\n",ring_idx,rx_idx);
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}

		//3.alloc new rx skb
		buf	= alloc_rx_buf(&skb, size_of_cluster);
		if (buf) {
			//update to	info struct
			//info->priority = ring_idx; //	todo
			info->vid =	desc->rx_dvlanid;
			info->pid =	desc->rx_spa;
			info->rxPri	= desc->rx_dpri;
			info->input	= (void *)r_skb;
			info->len =	desc->rx_len - 4;
			// todo, update	the	other field: priority, ...
			#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_EXT_PORT_SUPPORT)
			if (New_rtl8651_rxPktPreprocess((void *)desc, &vid) != SUCCESS){
				//panic_printk("%s %d info->vid=0x%x \n", __FUNCTION__, __LINE__, info->vid);
				#ifdef _DESC_CACHE_ACCESS_RX
				desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
				#endif
				REUSE_DESC();
				goto get_next;
			}
			info->vid = vid;
			#endif
			
			/* after enable	Rx scatter gather feature (0xb80100a4 bit5 cf_rx_gather	= 1),
			 * the mdata of	second/third/... pkthdr	will be	updated	to 4-byte alignment	by hardware.
			 */
			r_skb->data	= (unsigned	char *)(desc->mdata);

			// should remove this note ??
			/* 2015-11-13, since 8198C has a WAN to WLAN throughput low issue (branch 3.4 rev. 34320).
			 * we do the _dma_cache_inv after allocate a new skb, instead of after packet received.
			 * David's suggestion: in 8197F and New descriptor architecture, also apply this change.
			 * so _dma_cache_inv is move down.
			 */
			//_dma_cache_inv((unsigned long)r_skb->data, r_skb->len);

			//4.hook
			rx_skb[ring_idx][rx_idx].skb = (uint32)skb;

			#ifdef _DESC_CACHE_ACCESS_RX
			//desc->addr = (uint32)buf;
			//_dma_cache_wback_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
			desc = (struct rx_desc *)(((uint32)desc) |	UNCACHE_MASK);
			desc->mdata = (uint32)buf;
			#else
			desc->mdata = (uint32)buf;
			#endif

			// 2015-11-13, see the comment above.
			_dma_cache_inv((unsigned long)((struct sk_buff *)skb)->data, 1536);

			//5.set	own	bit, move down
			SET_OWN_BIT(desc);

			//6. increase rx index
			INC_IDX(ring_idx, rx_idx);

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_OK;
		}
		else {
			// just	leave this packet in rx	desc
			// do not change own_bit and rx_index
			#if	defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
			rx_noBuffer_cnt++;
			#endif

			#if	defined(CONFIG_RTL_ETH_PRIV_SKB)
			//printk("ETH_ERR: no private skb buffer\n");
			#else
			//printk("ETH_ERR: kernel allocate skb failed\n");
			#endif

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_NULL;
		}
	}

	#if	!defined(CONFIG_SMP)
	SMP_UNLOCK_ETH_RECV(flags);
	#endif
	return (RTL_NICRX_NULL);
}

static inline void fill_txd_misc(struct tx_desc *txd, rtl_nicTx_info *nicTx, struct sk_buff *skb)
{
	// extract from	_swNic_send, purpose: keep a function shortly and easy to read
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS)
	txd->tx_dpri = nicTx->priority;
	#endif

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
		txd->tx_vlantag = nicTx->tagport;
	else
		txd->tx_vlantag = 0;
	#endif

	#if defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_SWCONFIG)
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
	{
		#if defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_SRC_TAG)
		extern int rtl865x_curOpMode;
		if(rtl865x_curOpMode == 0)
			txd->tx_vlantag = RTL_WANPORT_MASK & rtk_get_vlan_tagmask(txd->tx_dvlanid);
		else
			txd->tx_vlantag = 0x3f & rtk_get_vlan_tagmask(txd->tx_dvlanid);
		#else
		txd->tx_vlantag = 0x3f & rtk_get_vlan_tagmask(txd->tx_dvlanid);
		#endif
	}
	else{		
		#if defined(CONFIG_RTL_DNS_TRAP)
		if (dns_filter_enable && ((struct sk_buff *)skb)->is_dns_pkt){
			//packets tagged here
			txd->tx_vi = 1;
			txd->tx_vlantag = 0x3f & rtk_get_vlan_tagmask(txd->tx_dvlanid);
		}else
		#endif
		txd->tx_vlantag = 0;
	}

	#if defined(CONFIG_RTL_QOS_8021P_SUPPORT)
	#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
	if(gHwNatEnabled==0)
	#endif
	{
	if(txd->tx_dp == RTL_WANPORT_MASK && (txd->tx_vlantag & RTL_WANPORT_MASK))
	{
		if(skb->srcVlanPriority>=0 && skb->srcVlanPriority<=7)
			txd->tx_dpri = skb->srcVlanPriority;
	}
	}
  	#endif

	#elif defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
	{
		#ifdef	CONFIG_RTL_HW_VLAN_SUPPORT_HW_NAT
			#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
			rtk_get_real_nicTxVid(skb, &nicTx->vid);
			txd->tx_dvlanid = nicTx->vid;
			#endif
			txd->tx_vlantag = (0x3f	& rtk_getTagPortmaskByVid(nicTx->vid,(void *)skb->data));
		#else
			txd->tx_vlantag = auto_set_tag_portmask;
		#endif
	}
	else
		txd->tx_vlantag = 0;
	#endif
}

static inline void fill_txd_tso(struct tx_desc	*txd, char *data)
{
	/* TODO:
		check dev->features's NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT
		bits(set in	re865x_probe), if true then	set	these 2	bits of	the	Tx pktHdr,
		NOT	always set.

		due	to we enable NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT bits
		of dev->features in	re865x_probe(),	so we must enable hardware L3/L4 checksum offload feature.
	 */
	uint16 type	= htons( *(uint16 *)(data+12) );

	txd->tx_l4cs = 1;
	txd->tx_l3cs = 1;

	// TODO: the type is listed	?? vi/li/pi/...	need to	be checked
	if (type ==	ETH_P_IPV6)	{
		txd->tx_type = PKTHDR_IPV6;
		txd->tx_ipv6 = 1;
	}
	else if	(type == ETH_P_IP) {
		struct iphdr *iph;

		iph	= (struct iphdr	*)(data+14);

		if (iph->protocol == IPPROTO_TCP)
			txd->tx_type = PKTHDR_TCP;
		else if	(iph->protocol == IPPROTO_UDP)
			txd->tx_type = PKTHDR_UDP;
		else if	(iph->protocol == IPPROTO_ICMP)
			txd->tx_type = PKTHDR_ICMP;
		else if	(iph->protocol == IPPROTO_IGMP)
			txd->tx_type = PKTHDR_IGMP;
		else
			txd->tx_type = PKTHDR_IP;
		txd->tx_ipv4 = 1;
		txd->tx_ipv4_1st = 1;
	}
	else {
		txd->tx_type = PKTHDR_ETHERNET;
	}
}

static inline uint32 *dw_copy(uint32 *dest, uint32 *src, int cnt)
{
	int i;
	uint32 *org_dest;
	dest = (uint32 *)(((uint32)dest)| UNCACHE_MASK);
	org_dest = dest;
	for (i=0; i<cnt; i++)
		*dest++ = *src++;
	return (org_dest);
}

#if 1 //defined(CONFIG_RTL_97F_HW_TX_CSUM) //&& defined(CONFIG_RTL_EXT_PORT_SUPPORT)
/*
*  details please refer rtl865x_swNic.c function get_protocol_type
*  currently, parse packets incomplete.......please fixed me ?  
*/
static inline int parse_pkt(struct tx_desc *txd, rtl_nicTx_info *nicTx, struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	#ifdef CONFIG_IPV6
	uint8 cur_proto = 0;
	uint8 next_header=0;
	#endif
	
	if((*(int16 *)(skb->data+(ETH_ALEN<<1)))==(int16)htons(ETH_P_IP)){
		iph = (struct iphdr *)(skb->data+(ETH_ALEN<<1)+2);
	}
	else if (((*(int16 *)(skb->data+(ETH_ALEN<<1)))==(int16)htons(ETH_P_PPP_SES)) ||
			 ((*(int16 *)(skb->data+(ETH_ALEN<<1)))==(int16)htons(ETH_P_PPP_DISC)) ) {
		txd->tx_pi = 1;
		if(*(int16 *)(skb->data+(ETH_ALEN<<1)+8)==(int16)htons(0x0021)) {
			iph=(struct iphdr *)(skb->data+(ETH_ALEN<<1)+10);
		}
		#ifdef CONFIG_IPV6
		else if(*(int16 *)(skb->data+(ETH_ALEN<<1)+8)==(int16)htons(0x0057)) {
			next_header = *(uint8 *)(skb->data+(ETH_ALEN<<1)+8+2+6);
			cur_proto = PKTHDR_IPV6;
		}
		#endif

	}
	else if((*(int16 *)(skb->data+(ETH_ALEN<<1)))==(int16)htons(ETH_P_8021Q)) {
		txd->tx_vi = 1;
		if((*(int16 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN))==(int16)htons(ETH_P_IP)) {
			iph = (struct iphdr *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN+2);
		}
		#ifdef CONFIG_IPV6
		else if(*(int16 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN)==(int16)htons(ETH_P_IPV6)) {
			next_header = *(uint8 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN+2+6);
			cur_proto = PKTHDR_IPV6;
		}
		#endif		
		else if (((*(int16 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN))==(int16)htons(ETH_P_PPP_SES)) ||
			 ((*(int16 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN))==(int16)htons(ETH_P_PPP_DISC)) ) {
			txd->tx_pi = 1;
			if((*(int16 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN+8))==(int16)htons(0x0021)) {
				iph=(struct iphdr *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN+8+2);
			}
			#ifdef CONFIG_IPV6
			else if((*(int16 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN+8))==(int16)htons(0x0057)) {
				next_header = *(uint8 *)(skb->data+(ETH_ALEN<<1)+VLAN_HLEN+8+2+6);
				cur_proto = PKTHDR_IPV6;
			}
			#endif
		}

	}
	#ifdef CONFIG_IPV6
	else if(*(int16 *)(skb->data+(ETH_ALEN<<1))==(int16)htons(ETH_P_IPV6)) {
		next_header = *(uint8 *)(skb->data+(ETH_ALEN<<1)+2+6);
		cur_proto = PKTHDR_IPV6;
	}

	if (cur_proto) {
		txd->tx_ipv6 = 1;

		if (next_header == IPPROTO_TCP )
			txd->tx_type = PKTHDR_TCP;
		else if(next_header == IPPROTO_UDP )
			txd->tx_type = PKTHDR_UDP;
		else if(next_header == IPPROTO_ICMP )
			txd->tx_type = PKTHDR_ICMP;
		else if(next_header == IPPROTO_IGMP )
			txd->tx_type = PKTHDR_IGMP;
		else if(next_header == IPPROTO_GRE )
			txd->tx_type = PKTHDR_PPTP;	
		else
			txd->tx_type = PKTHDR_IPV6;		
		
		return 0;
	}
	#endif

	if (iph){		
		txd->tx_ipv4 = 1;
		txd->tx_ipv4_1st = 1;
		
		if (iph->protocol == IPPROTO_TCP ){
			txd->tx_type = PKTHDR_TCP;
		}
		else if(iph->protocol == IPPROTO_UDP ){
			txd->tx_type = PKTHDR_UDP; 
		}
		else if(iph->protocol == IPPROTO_ICMP ){
			txd->tx_type = PKTHDR_ICMP;
		}
		else if(iph->protocol == IPPROTO_IGMP ){
			txd->tx_type = PKTHDR_IGMP;
		}
		else if(iph->protocol == IPPROTO_GRE ){
			txd->tx_type = PKTHDR_PPTP;
		}
		else{
			txd->tx_type = PKTHDR_IP;
		}
	}
	else
		txd->tx_type = PKTHDR_ETHERNET;

	return 0;
}
#endif

static inline int get_protocol_type_new_desc(struct sk_buff *skb, struct tx_desc *txd)
{

	if(*(int16 *)(skb->data+12)==(int16)htons(ETH_P_IP)) {
		return PKTHDR_TCP;
	}
	else if ((*(int16 *)(skb->data+12)==(int16)htons(ETH_P_PPP_SES)) ||
			 (*(int16 *)(skb->data+12)==(int16)htons(ETH_P_PPP_DISC)) ) {
		txd->tx_pi = 1;
		if(*(int16 *)(skb->data+12+8)==(int16)htons(0x0021)) {
			return PKTHDR_TCP;
		}
		else if(*(int16 *)(skb->data+12+8)==(int16)htons(0x0057)) {
			txd->tx_ipv6 = 1;
			return PKTHDR_IPV6;
		}
		else{
			return PKTHDR_ETHERNET;
		}
	}
	else if(*(int16 *)(skb->data+12)==(int16)htons(ETH_P_8021Q)) {
		txd->tx_vi = 1;
		if(*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_IP)) {
			return PKTHDR_TCP;
		}
		else if(*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_IPV6)) {
			txd->tx_ipv6 = 1;
			return PKTHDR_IPV6;
		}
		else if ((*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_PPP_SES)) ||
			 (*(int16 *)(skb->data+12+4)==(int16)htons(ETH_P_PPP_DISC)) ) {
			txd->tx_pi = 1; 
			if(*(int16 *)(skb->data+12+4+8)==(int16)htons(0x0021)) {
				return PKTHDR_TCP;
			} else if(*(int16 *)(skb->data+12+4+8)==(int16)htons(0x0057)) {
				txd->tx_ipv6 = 1;
				return PKTHDR_IPV6;
			}
			else
				return PKTHDR_ETHERNET;
		}

		else {
			return PKTHDR_ETHERNET;
		}
	}
	else if(*(int16 *)(skb->data+12)==(int16)htons(ETH_P_IPV6)) {
		txd->tx_ipv6 = 1;
		return PKTHDR_IPV6;
	}
	else
		return PKTHDR_ETHERNET;
}

/*************************************************************************
*	FUNCTION
*		_New_swNic_send
*
*	DESCRIPTION
*		This function writes one packet	to tx descriptors, and waits until
*		the	packet is successfully sent.
*
*	INPUTS
*		None
*
*	OUTPUTS
*		None
*************************************************************************/

int32 _New_swNic_send(void *skb, void *output, uint32 len, rtl_nicTx_info *nicTx)
{

	uint32 tx_idx, ret = SUCCESS;
	struct tx_desc *txd;
	uint32 next_index;
//	startCP3Ctrl(PERF_EVENT_CYCLE);	//added for romperf testing	
	tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];

	next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);

	if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
		/* TX ring full	*/
//		stopCP3Ctrl(1, 0);	//added for romperf testing	
		return(FAILED);
	}

	#ifdef _LOCAL_TX_DESC
	txd	= &local_txd;
	#else
	txd	= (struct tx_desc *)&New_txDescRing[nicTx->txIdx][tx_idx];
	// txd should NOT be NULL, otherwise ....
	
	#ifndef	_DESC_CACHE_ACCESS_TX
	txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
	#endif
	#endif

	memset((void *)txd, 0, sizeof(struct tx_desc));
	// store the skb pointer for New_swNic_txDone later
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;

	/* Pad small packets and hardware will add CRC */
	if ( len < 60 )
		len	= 64;
	else
		len	+= 4;

	txd->mdata  = ((uint32)output);

	// do not need to set tail bit after we use DMA_CR1/DMA_CR4 register
	//if (tx_idx == (New_txDescRingCnt[nicTx->txIdx] - 1))
	//	txd->tx_eor = 1;
	txd->tx_ls = 1;
	txd->tx_fs = 1;
	txd->tx_ph_len = len;
	txd->tx_mlen = len;
	txd->tx_dvlanid = nicTx->vid;
	txd->tx_dp = nicTx->portlist;

	#ifdef CONFIG_RTL_97F_HW_TX_CSUM
	/* due to we enable NETIF_F_HW_CSUM in dev->features,
	   we need to enable HW Tx CSUM if ip_summed = CHECKSUM_PARTIAL.
	   TODO: if this pkt need to do HW Tx CSUM and it is sent to wan interface,
	   		 how can it work?
	 */
	if (((struct sk_buff *)skb)->ip_summed == CHECKSUM_PARTIAL) {

		/* direct tx packets need fill some field for hw tx csum
		*/
		parse_pkt(txd, nicTx, (struct sk_buff *)skb);

#if (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)) && !defined(CONFIG_RTL_CPU_TAG)
		nicTx->flags &= ~(PKTHDR_HWLOOKUP);
#endif
		
		txd->tx_l4cs = 1;
		txd->tx_l3cs = 1;		
	}
	#endif

	if ((nicTx->flags &	PKTHDR_HWLOOKUP) !=	0) {
		txd->tx_hwlkup = 1;
		#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
		if (nicTx->flags &	PKTHDR_BRIDGING)
			txd->tx_bridge = 1;
		else
			txd->tx_bridge = 0;
		#else
		txd->tx_bridge = 1;
		#endif
		txd->tx_extspa = PKTHDR_EXTPORT_LIST_CPU;	
	}
	
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) ||	defined(CONFIG_RTL_HW_VLAN_SUPPORT) || defined(CONFIG_SWCONFIG)
	fill_txd_misc(txd, nicTx, (struct sk_buff *)skb);
	#endif

	#ifdef _LOCAL_TX_DESC
	txd	= (struct tx_desc *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
			(uint32 *)txd, sizeof(struct tx_desc) / 4);
	#else
	/* note: cache_wback of	output data	has	been done in caller	*/
	#ifdef _DESC_CACHE_ACCESS_TX
	_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct tx_desc));
	txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
	#endif
	#endif
	
	txd->tx_own = 1; //	set	own	bit	after all done.

	New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;

	/* Set TXFD	bit	to start transmission */
	REG32(CPUICR) |= TXFD;
//	stopCP3Ctrl(1, 0);	//added for romperf testing	
	return ret;
}

#if (defined(CONFIG_RTL_TSO) || defined(CONFIG_RTL_GSO))
inline uint8 find_L3L4_hdr_len(struct sk_buff *skb, uint8 *L4_hdr_len)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	uint8 *ptr, L3_hdr_len=5;

	ptr = skb->data + 12;
	
	if(*(int16 *)(ptr)==(int16)htons(ETH_P_8021Q))
		ptr = skb->data + 16;

	if(*(int16 *)(ptr)==(int16)htons(ETH_P_IP))
	{	
		//ipv4	
		iph=(struct iphdr *)(ptr+2);
		L3_hdr_len = iph->ihl;

		if (iph->protocol == IPPROTO_TCP) {
			tcph = (void *)iph + iph->ihl*4;
			*L4_hdr_len = tcph->doff;
		}
	}
	#ifdef CONFIG_IPV6
	else if(*(int16 *)(ptr)==(int16)htons(ETH_P_IPV6))
	{
		if (*(ptr+2+6) == IPPROTO_TCP) {
			tcph = (struct tcphdr *)(ptr+2+40);
			*L4_hdr_len = tcph->doff;
		}
	}
	#endif
	return (L3_hdr_len);
}

int32 _New_swNic_send_tso_sg(struct	sk_buff	*skb, void * output, uint32	len,rtl_nicTx_info *nicTx)
{
	uint32 tx_idx =	0, eor =	0;
	DMA_TX_DESC	*txd = NULL, *first_txd=NULL;
	uint32 next_index;
	uint32 first_len, frag_size=0;
	uint32 skb_gso_size	= skb_shinfo(skb)->gso_size;
	uint32 ring_cnt=New_txDescRingCnt[nicTx->txIdx];
	uint8 L3_hdr_len=5, L4_hdr_len=5, L3_ip6_hdr_len = 40;
	uint8 proto_type=0, cur_frag;
	bool cur_pi=0, cur_vi=0;
	uint8 nr_frags=(skb_shinfo(skb)->nr_frags)+1;

	first_len =	skb->len - skb->data_len;
	for	(cur_frag =	0; cur_frag	< skb_shinfo(skb)->nr_frags; cur_frag++)
	{	
		frag_size += (&skb_shinfo(skb)->frags[cur_frag])->size;
	}
	if(!first_len || (frag_size != skb->data_len)) {	/* should not happen */
		dev_kfree_skb_any(skb);
		return(SUCCESS);
	}	
	
	/* check cpu_owned desc num is enough or not */
	if (CIRC_SPACE_2(New_currTxPkthdrDescIndex[nicTx->txIdx],New_txPktDoneDescIndex[nicTx->txIdx],ring_cnt) <= nr_frags) /* keep the same rule with _New_swNic_send(), means "equal" is not enough */
		return(FAILED);		

	//L3_hdr_len = find_L3L4_hdr_len(skb, &L4_hdr_len);

	if(first_len)
	{
		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,ring_cnt);

		#ifdef _LOCAL_TX_DESC
		txd	= (DMA_TX_DESC *)&local_txd;
		memset((void *)txd, 0, sizeof(DMA_TX_DESC));
		#else
		// todo: error check, if txd = NULL....
		txd	= &New_txDescRing[nicTx->txIdx][tx_idx];
		first_txd =	(DMA_TX_DESC *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif
		
		//skb @	the	latest tx_skb
		tx_skb[nicTx->txIdx][tx_idx].skb = 0x0;
		/* Pad small packets*/
		if ( skb->len < 60 )
			skb->len = 60;
		
		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;
		txd->addr  = (uint32)output;
		txd->opts1 = (eor	|(((skb->len+4)	& TD_PHLEN_MASK) <<	TD_PHLEN_OFFSET));
		txd->opts1 |= FirstFrag;

		if ((skb_shinfo(skb)->nr_frags) == 0)
			first_len += 4;
		
		txd->opts2	= ((first_len &	TD_M_LEN_MASK) << TD_M_LEN_OFFSET);

		txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);	// direct tx now

		proto_type = get_protocol_type_new_desc(skb, (struct tx_desc *)txd); //Check Vi, Pi, and type
		cur_vi = ((struct tx_desc *)txd)->tx_vi;
		cur_pi = ((struct tx_desc *)txd)->tx_pi;
		
		if (skb_gso_size) {
			/* refer to RTL8198E_LSO_spec_v1_150311.doc
		   		no matter direct_tx or hw_lookup,
	 		    need to check payload more and set txd more.
			*/
			L3_hdr_len = find_L3L4_hdr_len(skb, &L4_hdr_len);
			txd->opts4 |= TD_LSO_MASK;
			txd->opts1 = (txd->opts1 & ~(TD_TYPE_MASK << TD_TYPE_OFFSET)) | (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5

			if (proto_type == PKTHDR_IPV6) { //IPv6
					txd->opts3 |= TD_L4CS_MASK;
					txd->opts4 |= ((L3_ip6_hdr_len<<TD_IPV6_HDRLEN_OFFSET));
					txd->opts5 |= ((skb_gso_size<<TD_MSS_OFFSET)| (L4_hdr_len));
			} else { //IPv4
					txd->opts3 |= (TD_L3CS_MASK | TD_L4CS_MASK);
					txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
					txd->opts5 |= ((skb_gso_size<<TD_MSS_OFFSET)| (L3_hdr_len<<TD_IPV4_HLEN_OFFSET) | (L4_hdr_len));
			}
		} else { //fragment only
				txd->opts3 |= (TD_L3CS_MASK	| TD_L4CS_MASK);
		}

		if (!(nicTx->flags & PKTHDR_HWLOOKUP)){
			parse_pkt((struct tx_desc *)txd, nicTx, (struct sk_buff *)skb);
			txd->opts1 &= ~(TD_BRIDGE_MASK | TD_HWLKUP_MASK);
			txd->opts5 = (txd->opts5 & ~(TD_EXTSPA_MASK<<TD_EXTSPA_OFFSET))|(0 << TD_EXTSPA_OFFSET);			
		}
		else{
			txd->opts1 |= (TD_BRIDGE_MASK | TD_HWLKUP_MASK);
			txd->opts5 |= (PKTHDR_EXTPORT_LIST_CPU << TD_EXTSPA_OFFSET);
		}

#if (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)) && !defined(CONFIG_RTL_CPU_TAG)
		txd->opts1 &= ~(TD_BRIDGE_MASK |	TD_HWLKUP_MASK);
		txd->opts5 |= (0 << TD_EXTSPA_OFFSET);
#endif

		((struct tx_desc *)txd)->tx_dvlanid = nicTx->vid;
		#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) ||	defined(CONFIG_RTL_HW_VLAN_SUPPORT) || defined(CONFIG_SWCONFIG)
		fill_txd_misc( (struct tx_desc *)txd, nicTx, (struct sk_buff *)skb);
		#endif
		
		#ifdef _LOCAL_TX_DESC
		first_txd = (DMA_TX_DESC *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
				(uint32 *)txd, sizeof(struct tx_desc) / 4);
		#else
		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct dma_tx_desc));
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}

	for	(cur_frag =	0; cur_frag	< skb_shinfo(skb)->nr_frags; cur_frag++)
	{
		skb_frag_t *frag = &skb_shinfo(skb)->frags[cur_frag];
		void *addr;

		addr = ((void *) page_address(frag->page.p)) + frag->page_offset;
		_dma_cache_wback((unsigned long)addr,frag->size);

		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,ring_cnt);

		#ifdef _LOCAL_TX_DESC
		txd	= (DMA_TX_DESC *)&local_txd;
		memset((void *)txd, 0, sizeof(DMA_TX_DESC));
		#else
		// todo: error check, if txd = NULL....
		txd	= &New_txDescRing[nicTx->txIdx][tx_idx];
		if (first_txd == NULL)
			first_txd =	(DMA_TX_DESC *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		tx_skb[nicTx->txIdx][tx_idx].skb = 0;

		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;
		txd->addr  = (uint32)addr;
		txd->opts1 = (eor |(((skb->len+4) &	TD_PHLEN_MASK) << TD_PHLEN_OFFSET));

		if (cur_frag == ((skb_shinfo(skb)->nr_frags) - 1))
			txd->opts2 = (((frag->size + 4) & TD_M_LEN_MASK) << TD_M_LEN_OFFSET);
		else
			txd->opts2 = ((frag->size &	TD_M_LEN_MASK) << TD_M_LEN_OFFSET);

		txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);	// direct tx now

		//Vi, Pi from first_len
		((struct tx_desc *)txd)->tx_vi = cur_vi;
		((struct tx_desc *)txd)->tx_pi = cur_pi;
		
		if (skb_gso_size) {
			/* refer to RTL8198E_LSO_spec_v1_150311.doc
		   		no matter direct_tx or hw_lookup,
	 		    need to check payload more and set txd more.
			*/
			txd->opts4 |= TD_LSO_MASK;
			txd->opts1 = (txd->opts1 & ~(TD_TYPE_MASK << TD_TYPE_OFFSET)) | (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5

			if (proto_type == PKTHDR_IPV6) { //IPv6
					txd->opts3 |= (TD_L4CS_MASK | TD_IPV6_MASK);
					txd->opts4 |= ((L3_ip6_hdr_len<<TD_IPV6_HDRLEN_OFFSET));
					txd->opts5 |= ((skb_gso_size<<TD_MSS_OFFSET)| (L4_hdr_len));
			} else { //IPv4
					txd->opts3 |= (TD_L3CS_MASK	| TD_L4CS_MASK);
					txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
					txd->opts5 |= ((skb_gso_size<<TD_MSS_OFFSET)| (L3_hdr_len<<TD_IPV4_HLEN_OFFSET) |	(L4_hdr_len));
			}
			
		} else { //fragment only
				txd->opts3 |= (TD_L3CS_MASK	| TD_L4CS_MASK);
		}

		if (!(nicTx->flags & PKTHDR_HWLOOKUP)){
			parse_pkt((struct tx_desc *)txd, nicTx, (struct sk_buff *)skb);
			txd->opts1 &= ~(TD_BRIDGE_MASK | TD_HWLKUP_MASK);
			txd->opts5 = (txd->opts5 & ~(TD_EXTSPA_MASK<<TD_EXTSPA_OFFSET))|(0 << TD_EXTSPA_OFFSET);			
		}
		else{
			txd->opts1 |= (TD_BRIDGE_MASK | TD_HWLKUP_MASK);
			txd->opts5 |= (PKTHDR_EXTPORT_LIST_CPU << TD_EXTSPA_OFFSET);
		}
		
#if (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)) && !defined(CONFIG_RTL_CPU_TAG)
		txd->opts1 &= ~(TD_BRIDGE_MASK |	TD_HWLKUP_MASK);
		txd->opts5 |= (0 << TD_EXTSPA_OFFSET);
#endif

		((struct tx_desc *)txd)->tx_dvlanid = nicTx->vid;
		#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) ||	defined(CONFIG_RTL_HW_VLAN_SUPPORT) || defined(CONFIG_SWCONFIG)
		fill_txd_misc( (struct tx_desc *)txd, nicTx, (struct sk_buff *)skb);
		#endif

		#ifdef _LOCAL_TX_DESC
		txd = (DMA_TX_DESC *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
				(uint32 *)txd, sizeof(DMA_TX_DESC) / 4);
		#else
		/* note: cache_wback of	output data	has	been done in caller	*/
		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct dma_tx_desc));
		txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		txd->opts1 |= DESC_SWCORE_OWNED; //	set	own	bit	after all done.

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}

	#ifdef _LOCAL_TX_DESC
	if ((skb_shinfo(skb)->nr_frags) == 0)
		first_txd->opts1 |=	LastFrag;
	else
	#endif		
		txd->opts1 |= LastFrag;
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;

	// set own bit of first	tx_pkthdr after	all	done.
	first_txd->opts1 |=	DESC_SWCORE_OWNED;
	
	/* Set TXFD	bit	to start send */
	REG32(CPUICR) |= TXFD;

	return(SUCCESS);
}
#endif

__IRAM_FWD
int32 New_swNic_send(void *skb,	void * output, uint32 len, rtl_nicTx_info *nicTx)
{		
	int	ret;
	unsigned long flags	= 0;

	SMP_LOCK_ETH_XMIT(flags);
	ret	= _New_swNic_send(skb, output, len,	nicTx);
	SMP_UNLOCK_ETH_XMIT(flags);
	return ret;
}

#if (defined(CONFIG_RTL_TSO) || defined(CONFIG_RTL_GSO))
__IRAM_FWD
int32 New_swNic_send_tso_sg(void *skb, void	* output, uint32 len, rtl_nicTx_info *nicTx)
{
	int	ret;
	unsigned long flags	= 0;

	SMP_LOCK_ETH_XMIT(flags);
	ret	= _New_swNic_send_tso_sg(skb, output, len, nicTx);
	SMP_UNLOCK_ETH_XMIT(flags);
	return ret;
}
#endif

/*
  Why to use HW	CDP	instead	of own bit (i.e. use USE_SWITCH_TX_CDP flag)?
  Reason:
	the	cache line of 8197F	is 32-byte.
	the	descriptor size	is 24-byte and can not pad to 32-byte because
		HW CDP will	move to	next 24-byte after it Rx/Tx	a pkt.
	when _DESC_CACHE_ACCESS_TX is defined, for example:
		current	Tx index is	5,
		previous Tx	index is 4,	in cacheable address the own bit is	1(SWCORE ownd),
			IN CASE	HW is TX done this pkt,	it will	set	own	bit	to 0 (CPU ownd)	in physical	memory,
		for	current	Tx pkt (idx:5),	it will	do cache writeback the desc	(24-byte) to physical memory,
			it will	ALSO writeback previous	desc's own bit from	cacheable address (own bit=1) to
			physical memory	(own bit=0).
		when New_swNic_txDone()	run	to the "checking of	idx	4",	the	own	bit	will NEVER equal to	"CPU ownd".
			so the Tx path is failed.
  i.e. MUST	define USE_SWITCH_TX_CDP when use _DESC_CACHE_ACCESS_TX
 */
__IRAM_FWD
int32 New_swNic_txDone(int idx)
{
	unsigned long flags=0;
	struct sk_buff *skb;
	#ifdef USE_SWITCH_TX_CDP
	uint32 hw_cdp_data;
	int	hw_cdp_idx;
	#else
	DMA_TX_DESC	*tx_desc;
	#endif

	SMP_LOCK_ETH_XMIT(flags);

	#ifdef USE_SWITCH_TX_CDP
	hw_cdp_data	= REG32(TxCDP_reg[idx]);
	hw_cdp_idx = (hw_cdp_data -	New_txDescRing_base[idx]) /	sizeof(struct dma_tx_desc);
	
	while (New_txPktDoneDescIndex[idx] != hw_cdp_idx) {

		skb	= (struct sk_buff *)tx_skb[idx][New_txPktDoneDescIndex[idx]].skb;

		if (skb)
		{
			SMP_UNLOCK_ETH_XMIT(flags);
			#if defined(CONFIG_RTL_FAST_BRIDGE)
			tx_done_callback(skb);
			#else
			dev_kfree_skb_any(skb);
			#endif
			SMP_LOCK_ETH_XMIT(flags);			
			tx_skb[idx][New_txPktDoneDescIndex[idx]].skb = 0;
		}

		if (++New_txPktDoneDescIndex[idx] == New_txDescRingCnt[idx])
			New_txPktDoneDescIndex[idx]	= 0;
	}

	#else
	while (New_txPktDoneDescIndex[idx] != New_currTxPkthdrDescIndex[idx]) {

		tx_desc	= &New_txDescRing[idx][New_txPktDoneDescIndex[idx]];
		// or, use _dma_cache_inv((unsigned	long)tx_desc,(unsigned long)sizeof(struct dma_tx_desc));
		tx_desc	= (DMA_TX_DESC *)(((uint32)tx_desc)	| UNCACHE_MASK);

		if ((tx_desc->opts1	& DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			skb	= (struct sk_buff *)tx_skb[idx][New_txPktDoneDescIndex[idx]].skb;

			if (skb)
			{
				SMP_UNLOCK_ETH_XMIT(flags);
				#if defined(CONFIG_RTL_FAST_BRIDGE)
				tx_done_callback(skb);
				#else
				dev_kfree_skb_any(skb);
				#endif
				SMP_LOCK_ETH_XMIT(flags);				
				tx_skb[idx][New_txPktDoneDescIndex[idx]].skb = 0;
			}

			if (++New_txPktDoneDescIndex[idx] == New_txDescRingCnt[idx])
				New_txPktDoneDescIndex[idx]	= 0;
		}
		else
			break;
	}
	#endif

	SMP_UNLOCK_ETH_XMIT(flags);
	return 0;
}

void New_swNic_freeRxBuf(void)
{
	int	i,j;

	/* Initialize index	of Tx pkthdr descriptor	*/
	for	(i=0;i<NEW_NIC_MAX_TX_DESC_RING;i++)
	{
		New_currTxPkthdrDescIndex[i] = 0;
		New_txPktDoneDescIndex[i]=0;
	}

	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		if (rx_skb[i]){
			for	(j = 0;	j <	New_rxDescRingCnt[i]; j++)
			{
				void *skb;

				skb	= (void	*)rx_skb[i][j].skb;
				if (skb)
				{
					free_rx_buf(skb);
					rx_skb[i][j].skb = 0;
				}
			}
		}
		/* Initialize index	of current Rx pkthdr descriptor	*/
		New_currRxPkthdrDescIndex[i] = 0;
	}
}

__IRAM_FWD
void New_swNic_freeTxRing(void)
{
	unsigned long flags=0;
	struct sk_buff *skb;
	int i,j;

	SMP_LOCK_ETH_XMIT(flags);
	
	for	(i=0;i<NEW_NIC_MAX_TX_DESC_RING;i++)
	{
    	for	(j=0;j< New_txDescRingCnt[i] ;j++) {

			skb	= (struct sk_buff *)tx_skb[i][j].skb;    
			if (skb) {
				SMP_UNLOCK_ETH_XMIT(flags);
				#if defined(CONFIG_RTL_FAST_BRIDGE)
				tx_done_callback(skb);
				#else
				dev_kfree_skb_any(skb);
				#endif
				SMP_LOCK_ETH_XMIT(flags);
				tx_skb[i][j].skb = 0;
			}
		}
	}
	SMP_UNLOCK_ETH_XMIT(flags);
	return;
}

#if defined(REINIT_SWITCH_CORE)
void New_swNic_reConfigRxTxRing(void)
{
	int i,j;
	unsigned long flags=0;
	struct tx_desc *txd=0;	
	struct rx_desc *rxd=0;	

	SMP_LOCK_ETH(flags);

	/* Initialize index	of Tx pkthdr descriptor	*/
	for	(i=0;i<NEW_NIC_MAX_TX_DESC_RING;i++)
	{
		for	(j = 0;	j <	New_txDescRingCnt[i]; j++) {
			txd	= (struct tx_desc *)(((uint32)(&New_txDescRing[i][j])) | UNCACHE_MASK);
			txd->tx_own = DESC_RISC_OWNED;
		}
		New_currTxPkthdrDescIndex[i] = 0;
		New_txPktDoneDescIndex[i]=0;
		New_txDescRing_base[i] = ((uint32) New_txDescRing[i]) |	UNCACHE_MASK;
		REG32(TxCDP_reg[i])	= New_txDescRing_base[i];
	}

	REG32(DMA_CR1) = (New_txDescRingCnt[0]-1) * (sizeof(struct tx_desc)); 
	REG32(DMA_CR4) = TX_RING0_TAIL_AWARE;
	
	/* Initialize Rx packet	header descriptors */
	for	(i = 0;	i <	NEW_NIC_MAX_RX_DESC_RING; i++)
	{
		for	(j = 0;	j <	New_rxDescRingCnt[i]; j++) {
			rxd	= (struct rx_desc *)(((uint32)(&New_rxDescRing[i][j])) | UNCACHE_MASK);
			rxd->rx_own = DESC_SWCORE_OWNED;
		}
		rxd->rx_eor = 1;
		/* Initialize index	of current Rx pkthdr descriptor	*/
		New_currRxPkthdrDescIndex[i] = 0;
		New_rxDescRing_base[i] = ((uint32) New_rxDescRing[i]) |	UNCACHE_MASK;
		REG32(CPURPDCR(i)) = New_rxDescRing_base[i];
	}

	SMP_UNLOCK_ETH(flags);
	return;
}

int32 New_swNic_reInit(void)
{
	New_swNic_freeTxRing();
	New_swNic_reConfigRxTxRing();
	return SUCCESS;
}

int32 New_check_tx_done_desc_swCore_own(int32 *tx_done_inx)
{
	struct tx_desc *txd;	

	txd	= (struct tx_desc *)(((uint32)(&New_txDescRing[0][New_txPktDoneDescIndex[0]])) | UNCACHE_MASK);

	if (txd->tx_own == DESC_SWCORE_OWNED) {
		*tx_done_inx = New_txPktDoneDescIndex[0];
		return SUCCESS;
	}
	else {
		/* 	normal code will not happened, 
			it's for the case: tx tso pkt, just send first fragment to tx desc (and does not set own bit),
				then return and skip to send 2nd(or 3rd) fragment.
				(!!! this software bug has been fixed)
			failure scenario: 
				CPUIISR = 6, cnt_swcore_tx: 76532412 (increased very soon)

				txDoneIdx(Tx Ring0) own bit= cpu owned
				HW_idx(Tx Ring0) own bit= cpu owned
				txDoneIdx and HW_idx will NOT changed anymore
				eventually txCurrIdx(Tx Ring0) will be (txDoneIdx - 1), and Tx Ring full
		 */
		if ((New_currTxPkthdrDescIndex[0] != New_txPktDoneDescIndex[0])) {
			*tx_done_inx = New_txPktDoneDescIndex[0];
			return SUCCESS;
		}
		else
			return FAILED;	
	}
}
#endif

#ifdef UDP_FRAGMENT_PKT_QUEUEING

uint8 uf_enabled = 1;
uint8 uf_start = 0;
uint16 uf_tx_desc_low = UF_START_QUEUEING;
uf_t udp_frag[2];

/*
	similar to _New_swNic_send(), but no "free tx desc check", no "TXFD set"
 */
int32 _New_swNic_send2(void *skb, void *output, uint32 len, rtl_nicTx_info *nicTx)
{
	uint32 tx_idx, ret = SUCCESS;
	struct tx_desc *txd;
	uint32 next_index;
	tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];

	next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);

	#ifdef _LOCAL_TX_DESC
	txd	= &local_txd;
	#else
	txd	= (struct tx_desc *)&New_txDescRing[nicTx->txIdx][tx_idx];
	// txd should NOT be NULL, otherwise ....
	
	#ifndef	_DESC_CACHE_ACCESS_TX
	txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
	#endif
	#endif

	memset((void *)txd, 0, sizeof(struct tx_desc));
	// store the skb pointer for New_swNic_txDone later
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;

	/* Pad small packets and hardware will add CRC */
	if ( len < 60 )
		len	= 64;
	else
		len	+= 4;

	txd->mdata  = ((uint32)output);

	// do not need to set tail bit after we use DMA_CR1/DMA_CR4 register
	//if (tx_idx == (New_txDescRingCnt[nicTx->txIdx] - 1))
	//	txd->tx_eor = 1;
	txd->tx_ls = 1;
	txd->tx_fs = 1;
	txd->tx_ph_len = len;
	txd->tx_mlen = len;
	txd->tx_dvlanid = nicTx->vid;
	txd->tx_dp = nicTx->portlist;

	#ifdef CONFIG_RTL_97F_HW_TX_CSUM
	/* due to we enable NETIF_F_HW_CSUM in dev->features,
	   we need to enable HW Tx CSUM if ip_summed = CHECKSUM_PARTIAL.
	   TODO: if this pkt need to do HW Tx CSUM and it is sent to wan interface,
	   		 how can it work?
	 */
	if (((struct sk_buff *)skb)->ip_summed == CHECKSUM_PARTIAL) {

		/* direct tx packets need fill some field for hw tx csum
		*/
		parse_pkt(txd, nicTx, (struct sk_buff *)skb);

#if (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)) && !defined(CONFIG_RTL_CPU_TAG)
		nicTx->flags &= ~(PKTHDR_HWLOOKUP);
#endif
		
		txd->tx_l4cs = 1;
		txd->tx_l3cs = 1;		
	}
	#endif

	if ((nicTx->flags &	PKTHDR_HWLOOKUP) !=	0) {
		txd->tx_hwlkup = 1;
		#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
		if (nicTx->flags &	PKTHDR_BRIDGING)
			txd->tx_bridge = 1;
		else
			txd->tx_bridge = 0;
		#else
		txd->tx_bridge = 1;
		#endif
		txd->tx_extspa = PKTHDR_EXTPORT_LIST_CPU;	
	}
	
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) ||	defined(CONFIG_RTL_HW_VLAN_SUPPORT) || defined(CONFIG_SWCONFIG)
	fill_txd_misc(txd, nicTx, (struct sk_buff *)skb);
	#endif

	#ifdef _LOCAL_TX_DESC
	txd	= (struct tx_desc *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
			(uint32 *)txd, sizeof(struct tx_desc) / 4);
	#else
	/* note: cache_wback of	output data	has	been done in caller	*/
	#ifdef _DESC_CACHE_ACCESS_TX
	_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct tx_desc));
	txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
	#endif
	#endif
	
	txd->tx_own = 1; //	set	own	bit	after all done.

	New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;

	return ret;
}

void send_queued_pkt(uf_t *uf)
{
	rtl_nicTx_info *nicTx;
	struct sk_buff *skb;
	int i;

	nicTx = &(uf->uf_tx_info);
	for (i=0;i<uf->que_num;i++)
	{
		skb = uf->uf_skb_list[i];
		RTL_CACHE_WBACK((unsigned long) skb->data, skb->len);
		_New_swNic_send2(skb, skb->data, skb->len, nicTx);
	}
	REG32(CPUICR) |= TXFD;

	uf->que_num = 0;
	return;
}

#ifdef _UF_DEBUG
int _uf_cntr_tx_group_pkt = 0; // may be called from process_udp_fragment_pkt or uf_timeout_check
int _uf_cntr_free_group_pkt = 0; // may be called from process_udp_fragment_pkt or uf_timeout_check
int _uf_cntr_timer_timeout = 0; // either udp_frag[0].uf_expired_timer or udp_frag[1].uf_expired_timer
int _uf_cntr_tx_unfinished_group_pkt = 0; // due to uf_start=0 in one_sec_timer
int _uf_cntr_queue_end = 0;
#endif

void check_free_desc_and_send(uf_t *uf)
{
	rtl_nicTx_info *nicTx;
	struct sk_buff *skb;
	int i;
	
	nicTx = &(uf->uf_tx_info);

	/* check cpu_owned desc num is enough or not */
	if (CIRC_SPACE_2(New_currTxPkthdrDescIndex[nicTx->txIdx],New_txPktDoneDescIndex[nicTx->txIdx],New_txDescRingCnt[nicTx->txIdx]) 
		> uf->que_num) /* keep the same rule with _New_swNic_send(), means "equal" is not enough */
	{
		// send all of the queued skb
		for (i=0;i<uf->que_num;i++)
		{
			skb = uf->uf_skb_list[i];
			RTL_CACHE_WBACK((unsigned long) skb->data, skb->len);
			_New_swNic_send2(skb, skb->data, skb->len, nicTx);
		}
		/* Set TXFD	bit	to start transmission */
		REG32(CPUICR) |= TXFD;
		#ifdef _UF_DEBUG
		_uf_cntr_tx_group_pkt++;
		#endif		
	}
	else
	{
		// free all of the queued skb
		for (i=0;i<uf->que_num;i++)
			dev_kfree_skb_any(uf->uf_skb_list[i]);			

		#ifdef _UF_DEBUG
		_uf_cntr_free_group_pkt++;
		#endif		
	}
	uf->que_num = 0;
	return;
}

void uf_timeout_check(void)
{
	if (CIRC_SPACE_2(New_currTxPkthdrDescIndex[0],New_txPktDoneDescIndex[0],New_txDescRingCnt[0]) 
		< uf_tx_desc_low)
		uf_start = 1;
	else 	// if free tx desc num >= UF_START_QUEUEING, stop queueing
		uf_start = 0;

	if ((udp_frag[0].que_num == 0) && (udp_frag[1].que_num == 0))
		// no any queued pkt.
		return;

	if (uf_start == 0) // since tx_desc# >= UF_START_QUEUEING, clean up queued packet
	{		
		if (udp_frag[0].que_num != 0) {
			send_queued_pkt(&udp_frag[0]);
			#ifdef _UF_DEBUG
			_uf_cntr_tx_unfinished_group_pkt++;
			#endif
		}
		if (udp_frag[1].que_num != 0) {
			send_queued_pkt(&udp_frag[1]);		
			#ifdef _UF_DEBUG
			_uf_cntr_tx_unfinished_group_pkt++;
			#endif
		}
	}		
	else // decrease timeout timer, and handle timeout case
	{
		if (udp_frag[0].que_num != 0)
			if (udp_frag[0].uf_expired_timer-- == 0) {
				check_free_desc_and_send(&udp_frag[0]);
				#ifdef _UF_DEBUG
				_uf_cntr_timer_timeout++;
				#endif
			}
		if (udp_frag[1].que_num != 0)
			if (udp_frag[1].uf_expired_timer-- == 0) {
				check_free_desc_and_send(&udp_frag[1]);
				#ifdef _UF_DEBUG
				_uf_cntr_timer_timeout++;
				#endif
			}
	}
	return;
}

/*
 * return:
 *		0: need to do New_swNic_send() in caller re865x_start_xmit
 *		1: has been queued or sent, just return in caller re865x_start_xmit
 */
int32 process_udp_fragment_pkt(struct sk_buff *skb, rtl_nicTx_info *nicTx)
{		
	uint8 *p;
	unsigned long flags	= 0;
	uint16 ether_type, ip_id;
	uint8 protocol, more_frag, index, ret;

	if (0 == uf_start)
		return(0);

	p = skb->data;
	ether_type = htons(*(uint16 *)(p+12));
	protocol = *(p+23);
	more_frag = ((*(p+20)) & BIT(5)) >> 5;
	ret = 0;

	if ((0 == udp_frag[0].que_num) && (0 == udp_frag[1].que_num))	// only check more_fragments==1
	{	
		if ((ether_type == 0x0800) && (protocol==17) && (more_frag==1))
		{
			index = 0;
			
			memcpy(&udp_frag[index].uf_tx_info, nicTx, sizeof(rtl_nicTx_info));
			udp_frag[index].uf_skb_list[udp_frag[index].que_num] = skb;
			udp_frag[index].uf_ip_id = htons(*(uint16 *)(p+18));
			udp_frag[index].uf_expired_timer = UF_TIMEOUT_VALUE;
			udp_frag[index].que_num++;
			ret = 1;
		}
	}
	else // need to check more_fragments==1 and more_fragments==0
	{
	
		if ((ether_type == 0x0800) && (protocol==17) && (more_frag==1))
		{
			ip_id = htons(*(uint16 *)(p+18));
		
			if (ip_id == udp_frag[0].uf_ip_id) index = 0;
			else if (ip_id == udp_frag[1].uf_ip_id) index = 1;
			else if (0 == udp_frag[0].que_num) index = 0;
			else if (0 == udp_frag[1].que_num) index = 1;
			else // only can queue 2 group of udp fragment pkt
				return(0);
			
			memcpy(&udp_frag[index].uf_tx_info, nicTx, sizeof(rtl_nicTx_info));
			udp_frag[index].uf_skb_list[udp_frag[index].que_num] = skb;
			udp_frag[index].uf_ip_id = htons(*(uint16 *)(p+18));
			udp_frag[index].uf_expired_timer = UF_TIMEOUT_VALUE;
			udp_frag[index].que_num++;

			// if udp_frag[].uf_skb_list change to Linux link list struct,
			//    the check code below is no need.
			if (udp_frag[index].que_num == (UF_QUE_SKB_NUM-1))		
			{
				SMP_LOCK_ETH_XMIT(flags);		
				check_free_desc_and_send(&udp_frag[index]);
				SMP_UNLOCK_ETH_XMIT(flags);
				#ifdef _UF_DEBUG
				_uf_cntr_queue_end++;
				#endif
			}
			ret = 1;
		}
		else if ((ether_type == 0x0800) && (protocol==17) && (more_frag==0)) // last one if id match
		{
			ip_id = htons(*(uint16 *)(p+18));

			if (ip_id == udp_frag[0].uf_ip_id)
				index = 0;
			else if (ip_id == udp_frag[1].uf_ip_id)
				index = 1;
			else 
				return(0);

			udp_frag[index].uf_skb_list[udp_frag[index].que_num] = skb;
			udp_frag[index].que_num++;

			SMP_LOCK_ETH_XMIT(flags);		
			check_free_desc_and_send(&udp_frag[index]);
			SMP_UNLOCK_ETH_XMIT(flags);
			ret = 1;			
		}	
	}
	return((uint32)ret);
}
#endif

/* ===========================================================================
 *
 *			Backup code
 *
 * ===========================================================================
 */
// previous version, not easy to read
// use this version of New_swNic_receive(), and enable _DESC_CACHE_ACCESS_TX and USE_SWITCH_TX_CDP,
//		it seems okay, still do not know why.
#if 0
#define	SET_OWN_BIT(desc) \
	desc->opts1	|= DESC_SWCORE_OWNED;

#define	INC_IDX(ring, idx) \
	idx	= NEXT_IDX(idx,New_rxDescRingCnt[ring]);	\
	New_currRxPkthdrDescIndex[ring]	= idx;

#define	REUSE_DESC() \
	SET_OWN_BIT(desc); \
	INC_IDX(ring_idx, rx_idx); 

int32 New_swNic_receive(rtl_nicRx_info *info, int retryCount)
{
	uint32 rx_idx, ring_idx;
	struct sk_buff *r_skb;
	DMA_RX_DESC	*desc;
	unsigned char *buf;
	void *skb;

	#ifdef USE_SWITCH_RX_CDP
	uint32 cdp_value;
	int	hw_idx;
	#endif

	#if	!defined(CONFIG_SMP)
	unsigned long flags	= 0;
	SMP_LOCK_ETH_RECV(flags);
	#endif

	#if	defined(RTL_MULTIPLE_RX_TX_RING)
	for(ring_idx = (NEW_NIC_MAX_RX_DESC_RING-1);ring_idx >=	(info->priority);ring_idx--)
	#else
	for(ring_idx=0;ring_idx<1;ring_idx++)
	#endif
	{
get_next:
		rx_idx = New_currRxPkthdrDescIndex[ring_idx];

		#ifdef USE_SWITCH_RX_CDP
		cdp_value =	REG32(CPURPDCR(ring_idx));
		hw_idx = (cdp_value	- New_rxDescRing_base[ring_idx]) / sizeof(struct dma_rx_desc);
		if (rx_idx == hw_idx) {
			continue; // go	for	next rx	ring
		}
		#endif

		desc = &New_rxDescRing[ring_idx][rx_idx];

		// NEEDED, do not removed. when	previous interrupt_dsr_rx()	is finished,
		//		the	New_rxDescRing[ring_idx][rx_idx] has been read,	and	found "no more packet",
		//		so its address is cached.
		//		if _dma_cache_inv()	removed, and one packet	come in, read the desc and it own bit
		//		is still SWCORE	owned.
		// TODO: need to test, remove this one,	and	add	_dma_cache_inv to the next IF (own bit is SWCORE owned)

		// todo: in	FPGA, when rx_desc use cacheable and do	_dma_cache_inv,	the	rx ring	will abnormal
		#ifdef _DESC_CACHE_ACCESS_RX
		_dma_cache_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
		#else
		desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
		#endif

		#ifndef	USE_SWITCH_RX_CDP
		//1.check desc's own bit
		if (desc->opts1	& DESC_OWNED_BIT)//1: SWCORE owned
		{
			continue; // go	for	next rx	ring
		}
		#endif

		// if checksum failed, reuse this desc, except the ipv6 ready logo case.
		if ((desc->opts5 & (RD_L3CSOK_MASK | RD_L4CSOK_MASK)) != (RD_L3CSOK_MASK | RD_L4CSOK_MASK))
		{
			#ifdef CONFIG_RTL_IPV6READYLOGO
			if (!(((desc->opts5&(RD_L4CSOK_MASK))!=RD_L4CSOK_MASK) && 
				((desc->opts4&RD_IPV6_MASK)==RD_IPV6_MASK) && ((desc->opts3&RD_REASON_MASK)==0x60)))
			#endif	
			{
				#ifdef _DESC_CACHE_ACCESS_RX
				desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
				#endif
				REUSE_DESC();
				goto get_next;
			}
		}

		// porting from swNic_receive(), need or not ??
		#if 0 //defined(CONFIG_RTL_HARDWARE_NAT)
		if (((desc->opts4 & RD_SPA_MASK) >> RD_SPA_OFFSET) >= RTL8651_CPU_PORT) {
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}
		#endif
		
		//2.rx skb
		r_skb =	(struct	sk_buff	*)(rx_skb[ring_idx][rx_idx].skb);
		if (r_skb == NULL) {	// should not happen
			// todo: may call alloc_rx_buf() to	get	a new skb,....
			printk("ETH_ERR: skb_array[%d][%d] is NULL\n",ring_idx,rx_idx);
			#ifdef _DESC_CACHE_ACCESS_RX
			desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
			#endif
			REUSE_DESC();
			goto get_next;
		}

		//3.alloc new rx skb
		buf	= alloc_rx_buf(&skb, size_of_cluster);
		if (buf) {
			//update to	info struct
			//info->priority = ring_idx; //	todo
			info->vid =	(desc->opts4 & RD_DVLANID_MASK)	>>	RD_DVLANID_OFFSET;
			info->pid =	(desc->opts4 & RD_SPA_MASK)	>> RD_SPA_OFFSET;
			info->rxPri	= (desc->opts5 & RD_DPRI_MASK) >> RD_DPRI_OFFSET;
			r_skb->len = (desc->opts2 &	RD_LEN_MASK) - 4;
			info->input	= r_skb;
			info->len =	r_skb->len;
			// todo, update	the	other field: priority, ...

			/* after enable	Rx scatter gather feature (0xb80100a4 bit5 cf_rx_gather	= 1),
			 * the mdata of	second/third/... pkthdr	will be	updated	to 4-byte alignment	by hardware.
			 */
			r_skb->data	= (unsigned	char *)(desc->addr);

			_dma_cache_inv((unsigned long)r_skb->data, r_skb->len);

			//4.hook
			rx_skb[ring_idx][rx_idx].skb = (uint32)skb;

			#ifdef _DESC_CACHE_ACCESS_RX
			//desc->addr = (uint32)buf;
			//_dma_cache_wback_inv((unsigned long)desc,sizeof(struct dma_rx_desc));
			desc = (DMA_RX_DESC	*)(((uint32)desc) |	UNCACHE_MASK);
			desc->addr = (uint32)buf;
			#else
			desc->addr = (uint32)buf;
			#endif

			//5.set	own	bit, move down
			SET_OWN_BIT(desc);

			//6. increase rx index
			INC_IDX(ring_idx, rx_idx);

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_OK;
		}
		else {
			// just	leave this packet in rx	desc
			// do not change own_bit and rx_index
			#if	defined(CONFIG_RTL_PROC_DEBUG)||defined(CONFIG_RTL_DEBUG_TOOL)
			rx_noBuffer_cnt++;
			#endif

			#if	defined(CONFIG_RTL_ETH_PRIV_SKB)
			printk("ETH_ERR: no	private	skb	buffer\n");
			#else
			printk("ETH_ERR: kernel	allocate skb failed\n");
			#endif

			REG32(CPUIISR) = (PKTHDR_DESC_RUNOUT_IP_ALL);
			#if	!defined(CONFIG_SMP)
			SMP_UNLOCK_ETH_RECV(flags);
			#endif
			return RTL_NICRX_NULL;
		}
	}

	#if	!defined(CONFIG_SMP)
	SMP_UNLOCK_ETH_RECV(flags);
	#endif
	return (RTL_NICRX_NULL);
}

static inline void fill_txd_misc(DMA_TX_DESC *txd, rtl_nicTx_info *nicTx, struct sk_buff *skb)
{
	// extract from	_swNic_send, purpose: keep a function shortly and easy to read
	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS)
	txd->opts3 = (txd->opts3 & ~TD_DPRI_MASK) |	(nicTx->priority<<TD_DPRI_OFFSET);
	#endif

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| nicTx->tagport;
	else
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK);
	#endif

	#ifdef CONFIG_RTL_VLAN_8021Q
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| (0x3f	& rtk_get_vlan_tagmask(nicTx->vid));
	else
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK);

	#elif defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	if (*((unsigned	short *)(skb->data+ETH_ALEN*2))	!= htons(ETH_P_8021Q))
	{
		#ifdef	CONFIG_RTL_HW_VLAN_SUPPORT_HW_NAT
			#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
			rtk_get_real_nicTxVid(skb, &nicTx->vid);
			txd->opts3 = (txd->opts3 & ~TD_DVLANID_MASK) | nicTx->vid;
			#endif
			txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| (0x3f	& rtk_getTagPortmaskByVid(nicTx->vid,(void *)skb->data));
		#else
			txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK)	| auto_set_tag_portmask;
		#endif
	}
	else
		txd->opts2 = (txd->opts2 & ~TD_VLANTAGSET_MASK);
	#endif
}

static inline void fill_txd_tso(DMA_TX_DESC	*txd, char *data)
{
	/* TODO:
		check dev->features's NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT
		bits(set in	re865x_probe), if true then	set	these 2	bits of	the	Tx pktHdr,
		NOT	always set.

		due	to we enable NETIF_F_IP_CSUM_BIT/NETIF_F_IPV6_CSUM_BIT/NETIF_F_HW_CSUM_BIT bits
		of dev->features in	re865x_probe(),	so we must enable hardware L3/L4 checksum offload feature.
	 */
	uint16 type	= htons( *(uint16 *)(data+12) );

	txd->opts3 |= (TD_L3CS_MASK	| TD_L4CS_MASK);

	// TODO: the type is listed	?? vi/li/pi/...	need to	be checked
	if (type ==	ETH_P_IPV6)	{
		txd->opts1 |= (PKTHDR_IPV6 << TD_TYPE_OFFSET);
		txd->opts3 |= TD_IPV6_MASK;
	}
	else if	(type == ETH_P_IP) {
		struct iphdr *iph;

		iph	= (struct iphdr	*)(data+14);

		if (iph->protocol == IPPROTO_TCP)
			txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET);
		else if	(iph->protocol == IPPROTO_UDP)
			txd->opts1 |= (PKTHDR_UDP << TD_TYPE_OFFSET);
		else if	(iph->protocol == IPPROTO_ICMP)
			txd->opts1 |= (PKTHDR_ICMP << TD_TYPE_OFFSET);
		else if	(iph->protocol == IPPROTO_IGMP)
			txd->opts1 |= (PKTHDR_IGMP << TD_TYPE_OFFSET);
		else
			txd->opts1 |= (PKTHDR_IP <<	TD_TYPE_OFFSET);
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
	}
	else {
		txd->opts1 |= (PKTHDR_ETHERNET << TD_TYPE_OFFSET);
	}
}

/*************************************************************************
*	FUNCTION
*		_New_swNic_send
*
*	DESCRIPTION
*		This function writes one packet	to tx descriptors, and waits until
*		the	packet is successfully sent.
*
*	INPUTS
*		None
*
*	OUTPUTS
*		None
*************************************************************************/

int32 _New_swNic_send(void *skb, void *output, uint32 len, rtl_nicTx_info *nicTx)
{
	uint32 tx_idx, eor,	ret	= SUCCESS;
	DMA_TX_DESC	*txd;
	uint32 next_index;

	tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];

	next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);

	if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
		/* TX ring full	*/
		return(FAILED);
	}

	txd	= &New_txDescRing[nicTx->txIdx][tx_idx];
	// txd should NOT be NULL, otherwise ....

	#ifndef	_DESC_CACHE_ACCESS_TX
	txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
	#endif

	// store the skb pointer for New_swNic_txDone later
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;
	eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;

	/* Pad small packets and hardware will add CRC */
	if ( len < 60 )
		len	= 64;
	else
		len	+= 4;

	txd->addr  = ((uint32)output);
	txd->opts1 = (eor  |FirstFrag |LastFrag	|((len & TD_PHLEN_MASK)	<< TD_PHLEN_OFFSET));
	txd->opts2 = ((len & TD_M_LEN_MASK)	<< TD_M_LEN_OFFSET);
	txd->opts3 = nicTx->vid;
	txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);

	if ((nicTx->flags &	PKTHDR_HWLOOKUP) !=	0) {
		txd->opts1 |= ((1<<TD_BRIDGE_OFFSET) | (1<<TD_HWLKUP_OFFSET));
		txd->opts5 = (PKTHDR_EXTPORT_LIST_CPU << TD_EXTSPA_OFFSET);
	}

	#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTK_VOIP_QOS) || defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) ||	defined(CONFIG_RTL_HW_VLAN_SUPPORT)
	fill_txd_misc(txd, nicTx, (struct sk_buff *)skb);
	#endif

	#ifdef CONFIG_RTL_TSO
	fill_txd_tso(txd, (char	*)output);
	#endif

	/* note: cache_wback of	output data	has	been done in caller	*/
	#ifdef _DESC_CACHE_ACCESS_TX
	_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct dma_tx_desc));
	txd	= (DMA_TX_DESC *)(((uint32)txd)	| UNCACHE_MASK);
	#endif

	txd->opts1 |= DESC_SWCORE_OWNED; //	set	own	bit	after all done.

	New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;

	/* Set TXFD	bit	to start transmission */
	REG32(CPUICR) |= TXFD;

	return ret;
}

int32 _New_swNic_send_tso_sg(struct	sk_buff	*skb, void * output, uint32	len,rtl_nicTx_info *nicTx)
{
	uint32 tx_idx =	0, ret = SUCCESS; //, eor =	0;
	struct tx_desc	*txd=NULL, *first_txd=NULL;
	uint32 next_index;
	uint32 cur_frag;
	uint32 first_len;
	uint32 skb_gso_size	= skb_shinfo(skb)->gso_size;

	first_len =	skb->len - skb->data_len;

	if(first_len)
	{
		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);

		if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
			/*	TX ring	full	*/
			return(FAILED);
		}

		#ifdef _LOCAL_TX_DESC
		txd	= &local_txd;
		#else
		txd	= (struct tx_desc *)&New_txDescRing[nicTx->txIdx][tx_idx];

		first_txd =	(struct tx_desc *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		memset((void *)txd, 0, sizeof(struct tx_desc));

		//skb @	the	latest tx_skb
		tx_skb[nicTx->txIdx][tx_idx].skb = 0x0;

		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;
		txd->mdata  = (uint32)output;
		txd->tx_ph_len = skb->len+4;
		txd->tx_fs = 1;
		if ((skb_shinfo(skb)->nr_frags) == 0)
			first_len += 4;
		txd->tx_mlen = first_len;

		// due to enable HW Tx CSUM, always use hw lookup, no need to assign tx_dp 
		//txd->tx_dp = nicTx->portlist;	// direct tx now
		txd->tx_l3cs = 1;
		txd->tx_l4cs = 1;

		if(skb_gso_size)
			txd->tx_lso = 1;

		// use hw lookup, no need to assign tx_dp 
		/***
		txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
		***/
		txd->tx_type = PKTHDR_TCP;
		txd->tx_ipv4 = 1;
		txd->tx_ipv4_1st = 1;		
		txd->tx_mss = skb_gso_size;

		/* TODO: if this pkt need to do HW Tx CSUM and it is sent to wan interface,
		   		 how can it work?
		 */
		txd->tx_hwlkup = 1;
		txd->tx_bridge = 1;
		txd->tx_extspa = PKTHDR_EXTPORT_LIST_CPU;

		#ifdef _LOCAL_TX_DESC
		// we will set own bit in first_txd later
		first_txd = (struct tx_desc *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
					(uint32 *)txd, sizeof(struct tx_desc) / 4);
		#else

		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct tx_desc));
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}
	else
	{
		printk("\n !!! TODO: need to support this kind of skb !!!\n");
	}

	for	(cur_frag =	0; cur_frag	< skb_shinfo(skb)->nr_frags; cur_frag++)
	{
		skb_frag_t *frag = &skb_shinfo(skb)->frags[cur_frag];
		void *addr;

		addr = ((void *) page_address(frag->page.p)) + frag->page_offset;
		_dma_cache_wback((unsigned long)addr,frag->size);

		//1.get	desc index
		tx_idx = New_currTxPkthdrDescIndex[nicTx->txIdx];
		//2.get	next desc index
		next_index = NEXT_IDX(tx_idx,New_txDescRingCnt[nicTx->txIdx]);
		if (next_index == New_txPktDoneDescIndex[nicTx->txIdx])	{
			/*	TX ring	full	*/
			return(FAILED);
		}

		#ifdef _LOCAL_TX_DESC
		txd	= &local_txd;
		#else
		txd	= (struct tx_desc *)&New_txDescRing[nicTx->txIdx][tx_idx];
		//if (first_txd == NULL)
		//	first_txd =	(struct tx_desc *)(((uint32)txd) |	UNCACHE_MASK);

		// todo: in	FPGA, when tx_desc use cacheable and do	_dma_cache_wback, the tx ring will abnormal
		#ifndef	_DESC_CACHE_ACCESS_TX
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		memset((void *)txd, 0, sizeof(struct tx_desc));

		tx_skb[nicTx->txIdx][tx_idx].skb = 0;

		//eor	= (tx_idx == (New_txDescRingCnt[nicTx->txIdx] -	1))	? DESC_WRAP	: 0;

		txd->mdata  = (uint32)addr;
		txd->tx_ph_len = skb->len+4;
		if (cur_frag == ((skb_shinfo(skb)->nr_frags) - 1))
			txd->tx_mlen = frag->size + 4;
		else
			txd->tx_mlen = frag->size;

		//txd->opts4 = ((nicTx->portlist & TD_DP_MASK) <<	TD_DP_OFFSET);	// direct tx now
		txd->tx_l3cs = 1;
		txd->tx_l4cs = 1;

		if(skb_gso_size)
			txd->tx_lso = 1;

		/***
		txd->opts1 |= (PKTHDR_TCP << TD_TYPE_OFFSET); // type=5
		txd->opts3 |= (TD_IPV4_MASK	| TD_IPV4_1ST_MASK);
		***/
		txd->tx_type = PKTHDR_TCP;
		txd->tx_ipv4 = 1;
		txd->tx_ipv4_1st = 1;
		txd->tx_mss = skb_gso_size;

		/* TODO: if this pkt need to do HW Tx CSUM and it is sent to wan interface,
		   		 how can it work?
		 */
		txd->tx_hwlkup = 1;
		txd->tx_bridge = 1;
		txd->tx_extspa = PKTHDR_EXTPORT_LIST_CPU;

		#ifdef _LOCAL_TX_DESC
		txd = (struct tx_desc *)dw_copy((uint32 *)&New_txDescRing[nicTx->txIdx][tx_idx],
					(uint32 *)txd, sizeof(struct tx_desc) / 4);
		#else
		#ifdef _DESC_CACHE_ACCESS_TX
		_dma_cache_wback((unsigned long)txd,(unsigned long)sizeof(struct tx_desc));
		txd	= (struct tx_desc *)(((uint32)txd)	| UNCACHE_MASK);
		#endif
		#endif

		txd->tx_own = 1; //	set	own	bit	after all done.

		New_currTxPkthdrDescIndex[nicTx->txIdx]	= next_index;
	}

	//if (first_txd == NULL) // should not happen
	//	return(FAILED);

	#ifdef _LOCAL_TX_DESC
	if ((skb_shinfo(skb)->nr_frags) == 0)
		first_txd->tx_ls = 1;
	else
	#endif		
		txd->tx_ls = 1;
	tx_skb[nicTx->txIdx][tx_idx].skb = (uint32)skb;

	// set own bit of first	tx_pkthdr after	all	done.
	first_txd->tx_own = 1;
	
	/* Set TXFD	bit	to start send */
	REG32(CPUICR) |= TXFD;

	return ret;
}
#endif

