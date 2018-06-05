/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* $Header: /home/cvsroot/linux-2.6.19/linux-2.6.x/drivers/net/re865x/rtl865xc_swNic.h,v 1.3 2008/04/11 10:12:38 bo_zhao Exp $
*
* Abstract: Switch core polling mode NIC header file.
*
* $Author: bo_zhao $
*
* $Log: rtl865xc_swNic.h,v $
* Revision 1.3  2008/04/11 10:12:38  bo_zhao
* *: swap nic drive to 8186 style
*
* Revision 1.2  2007/11/11 02:51:27  davidhsu
* Fix the bug that do not fre rx skb in rx descriptor when driver is shutdown
*
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.4  2006/09/15 03:53:39  ghhuang
* +: Add TFTP download support for RTL8652 FPGA
*
* Revision 1.3  2005/09/22 05:22:31  bo_zhao
* *** empty log message ***
*
* Revision 1.1.1.1  2005/09/05 12:38:24  alva
* initial import for add TFTP server
*
* Revision 1.2  2004/03/31 01:49:20  yjlou
* *: all text files are converted to UNIX format.
*
* Revision 1.1  2004/03/16 06:36:13  yjlou
* *** empty log message ***
*
* Revision 1.1.1.1  2003/09/25 08:16:56  tony
*  initial loader tree
*
* Revision 1.1.1.1  2003/05/07 08:16:07  danwu
* no message
*
* ---------------------------------------------------------------
*/


#ifndef RTL865XC_SWNIC_H
#define	RTL865XC_SWNIC_H

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
//#define	RTL_MULTIPLE_RX_TX_RING		1	/*enable multiple input queue(multiple rx ring)*/
#endif

#if defined(DELAY_REFILL_ETH_RX_BUF) && !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F)
#define SKIP_ALLOC_RX_BUFF 1
#endif

#define RTL865X_SWNIC_RXRING_HW_PKTDESC	6

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL865X_SWNIC_TXRING_HW_PKTDESC	4
#else
#define RTL865X_SWNIC_TXRING_HW_PKTDESC	2
#endif
#define RESERVERD_MBUF_RING_NUM			8

#if defined(CONFIG_RTL_NFJROM_MP)
	#define MAX_PRE_ALLOC_RX_SKB		64
	#define NUM_RX_PKTHDR_DESC		8
	#define NUM_TX_PKTHDR_DESC		64
	#define	ETH_REFILL_THRESHOLD		4	// must < NUM_RX_PKTHDR_DESC
#elif defined(CONFIG_RTL_8198) && !defined(CONFIG_RTL_8198_AP_ROOT)
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB		0
  	#ifdef CONFIG_RTL_ULINKER
	#define NUM_RX_PKTHDR_DESC			1022	//512+510
  	#else
	#define NUM_RX_PKTHDR_DESC			1024	//512+512
  	#endif
	#else
	#define MAX_PRE_ALLOC_RX_SKB		512
  	#ifdef CONFIG_RTL_ULINKER
	#define NUM_RX_PKTHDR_DESC			510
  	#else
	#define NUM_RX_PKTHDR_DESC			512
  	#endif
  	#endif
	#define NUM_TX_PKTHDR_DESC			1024
	#define	ETH_REFILL_THRESHOLD		8	// must < NUM_RX_PKTHDR_DESC
#elif defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) ||defined(CONFIG_RTL_83XX_SUPPORT)
	#ifndef CONFIG_RTL_SHRINK_MEMORY_SIZE	
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB			0
	#define NUM_RX_PKTHDR_DESC			1412
	#else
	#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	#define MAX_PRE_ALLOC_RX_SKB		2048 
	#define NUM_RX_PKTHDR_DESC			900 
	#else
	#define MAX_PRE_ALLOC_RX_SKB		512
	#define NUM_RX_PKTHDR_DESC			900
	#endif
	#endif
	#define	ETH_REFILL_THRESHOLD		8	// must < NUM_RX_PKTHDR_DESC	
	#define NUM_TX_PKTHDR_DESC			768	
	#else	
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB			0
	#define NUM_RX_PKTHDR_DESC			1412
	#else
	#define MAX_PRE_ALLOC_RX_SKB		256
	#define NUM_RX_PKTHDR_DESC			450
	#endif
	#define	ETH_REFILL_THRESHOLD		8	// must < NUM_RX_PKTHDR_DESC	
	#define NUM_TX_PKTHDR_DESC			300
	#endif
#elif defined(CONFIG_RTL_819XD)
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB		0 
	#define NUM_RX_PKTHDR_DESC			1024	//512+512
	#else
	#define MAX_PRE_ALLOC_RX_SKB		512 
	#define NUM_RX_PKTHDR_DESC			512
	#endif
	#define ETH_REFILL_THRESHOLD			8	// must < NUM_RX_PKTHDR_DESC	
	#define NUM_TX_PKTHDR_DESC			512 
#elif defined(CONFIG_RTL_8198C)

	#if defined(SKIP_ALLOC_RX_BUFF)
	/*
		Note: Rx=448, Tx=448: Chariot LAN<->WAN 3D3U test for 1 minute (throughput is about 588Mbps for software NAT), 
			  the result is no Rx_drop, Tx_retry, Tx_drop.
			  for LAN<->WLAN or LAN<->Samba application, please enlarge them if need.
	 */
	#define MAX_PRE_ALLOC_RX_SKB			0
	#define NUM_RX_PKTHDR_DESC			448
	#define NUM_TX_PKTHDR_DESC			448
	#else

	#if defined(DELAY_REFILL_ETH_RX_BUF)
	#define MAX_PRE_ALLOC_RX_SKB			512
	#else
	#if defined(CONFIG_WLAN_HAL_8814AE) || defined(CONFIG_WLAN_HAL_8822BE)
	#define MAX_PRE_ALLOC_RX_SKB			2048
	#else
	#define MAX_PRE_ALLOC_RX_SKB			1024
	#endif
	#endif
	
	#if defined(CONFIG_WLAN_HAL_8814AE)
	#define NUM_RX_PKTHDR_DESC			2048
	#define NUM_TX_PKTHDR_DESC			2048
	#else
	#define NUM_RX_PKTHDR_DESC			512
	#define NUM_TX_PKTHDR_DESC			1024
	#endif
	#endif
	#define	ETH_REFILL_THRESHOLD			8	// must < NUM_RX_PKTHDR_DESC	

#elif defined(CONFIG_RTL_8197F)
	#ifndef CONFIG_RTL_SHRINK_MEMORY_SIZE		
	#if defined(SKIP_ALLOC_RX_BUFF)
	/*
		Note: Rx=448, Tx=448: Chariot LAN<->WAN 3D3U test for 1 minute (throughput is about 588Mbps for software NAT), 
			  the result is no Rx_drop, Tx_retry, Tx_drop.
			  for LAN<->WLAN or LAN<->Samba application, please enlarge them if need.
	 */
	#define MAX_PRE_ALLOC_RX_SKB			0
	#define NUM_RX_PKTHDR_DESC			448
	#define NUM_TX_PKTHDR_DESC			448
	#else

	#if defined(DELAY_REFILL_ETH_RX_BUF)
	#define MAX_PRE_ALLOC_RX_SKB			512
	#else
	#if defined(CONFIG_RTL_8211F_SUPPORT) && defined(CONFIG_WLAN_HAL_8822BE)
	#define MAX_PRE_ALLOC_RX_SKB			2048
	#else
	#define MAX_PRE_ALLOC_RX_SKB			512
	#endif
	#endif
	
	#define NUM_RX_PKTHDR_DESC			900
	#define NUM_TX_PKTHDR_DESC			768
	#endif 
	#else	
	#if defined(SKIP_ALLOC_RX_BUFF)
    #define MAX_PRE_ALLOC_RX_SKB            0 
    #define NUM_RX_PKTHDR_DESC                      292
    #define NUM_TX_PKTHDR_DESC                      284
    #else
    #define MAX_PRE_ALLOC_RX_SKB            192 
    #define NUM_RX_PKTHDR_DESC                      256
    #define NUM_TX_PKTHDR_DESC                      300     
    #endif
	#endif  
	#define	ETH_REFILL_THRESHOLD			8	// must < NUM_RX_PKTHDR_DESC	

#elif defined(CONFIG_RTL_8881A)
	#define ETH_REFILL_THRESHOLD			8	// must < NUM_RX_PKTHDR_DESC	
	#if defined(CONFIG_USE_PCIE_SLOT_0)
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB		0	
	#define NUM_RX_PKTHDR_DESC			912	//400+512
	#else
	#define MAX_PRE_ALLOC_RX_SKB		400//512	
	#define NUM_RX_PKTHDR_DESC			512
	#endif
	#define NUM_TX_PKTHDR_DESC			1024 //768
	#else	
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB		0			
	#define NUM_RX_PKTHDR_DESC			556		//256+300
	#else
	#define MAX_PRE_ALLOC_RX_SKB		256	//512	
	#define NUM_RX_PKTHDR_DESC			300	//512
	#endif
	#define NUM_TX_PKTHDR_DESC			512
	#endif
#elif defined(CONFIG_RTL_8196E)
	#define ETH_REFILL_THRESHOLD			8	// must < NUM_RX_PKTHDR_DESC	
	#ifdef CONFIG_RTL_SDRAM_GE_32M
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB		0
	#if defined(CONFIG_RTL_DISABLE_ETH_MIPS16)
	#define NUM_RX_PKTHDR_DESC			518	
	#else
	#define NUM_RX_PKTHDR_DESC			428	//128+300
	#endif
	#else
	#define MAX_PRE_ALLOC_RX_SKB		128
	#define NUM_RX_PKTHDR_DESC			300//160
	#endif
	#define NUM_TX_PKTHDR_DESC			400	
	#else
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB		0 
	#define NUM_RX_PKTHDR_DESC			292
	#define NUM_TX_PKTHDR_DESC			284
	#else
	#define MAX_PRE_ALLOC_RX_SKB		192 
	#define NUM_RX_PKTHDR_DESC			256
	#define NUM_TX_PKTHDR_DESC			300	
	#endif
	#endif
#else
	#if defined(SKIP_ALLOC_RX_BUFF)
	#define MAX_PRE_ALLOC_RX_SKB		0 
	#define NUM_RX_PKTHDR_DESC			1024	//512+512
	#else
	#define MAX_PRE_ALLOC_RX_SKB		512 
	#define NUM_RX_PKTHDR_DESC			512
	#endif
	#define ETH_REFILL_THRESHOLD			8	// must < NUM_RX_PKTHDR_DESC	
	#define NUM_TX_PKTHDR_DESC			256 	
#endif

#if defined(RTL_MULTIPLE_RX_TX_RING)

#define	RTL_CPU_QOS_ENABLED		1

#define	RTL865X_SWNIC_RXRING_MAX_PKTDESC    6
#define	RTL865X_SWNIC_TXRING_MAX_PKTDESC    2
#define	RTL_CPU_RX_RING_NUM			4
/*	By default, only using rxring 0 and rxring 5
*	in order to make different between low/high
*	priority
*/
#define	NUM_RX_PKTHDR_DESC1			2
#define	NUM_RX_PKTHDR_DESC2			2
#define	NUM_RX_PKTHDR_DESC3			2
#define	NUM_RX_PKTHDR_DESC4			2
#define	NUM_RX_PKTHDR_DESC5			NUM_RX_PKTHDR_DESC
#define	NUM_TX_PKTHDR_DESC1			NUM_TX_PKTHDR_DESC

#define	ETH_REFILL_THRESHOLD1	0	// must < NUM_RX_PKTHDR_DESC1
#define	ETH_REFILL_THRESHOLD2	0	// must < NUM_RX_PKTHDR_DESC2
#define	ETH_REFILL_THRESHOLD3	0	// must < NUM_RX_PKTHDR_DESC3
#define	ETH_REFILL_THRESHOLD4	0	// must < NUM_RX_PKTHDR_DESC4
#define	ETH_REFILL_THRESHOLD5	ETH_REFILL_THRESHOLD	// must < NUM_RX_PKTHDR_DESC5

#define	QUEUEID0_RXRING_MAPPING		0x0000
#define	QUEUEID1_RXRING_MAPPING		0x0000
#define	QUEUEID2_RXRING_MAPPING		0x5555
#define	QUEUEID3_RXRING_MAPPING		0x5555
#define	QUEUEID4_RXRING_MAPPING		0x5555
#define	QUEUEID5_RXRING_MAPPING		0x5555
#else
#define	RTL865X_SWNIC_RXRING_MAX_PKTDESC    1
#define	RTL865X_SWNIC_TXRING_MAX_PKTDESC    1
#define	RTL_CPU_RX_RING_NUM			1
#define	NUM_RX_PKTHDR_DESC1		2
#define	NUM_RX_PKTHDR_DESC2		2
#define	NUM_RX_PKTHDR_DESC3		2
#define	NUM_RX_PKTHDR_DESC4		2
#define	NUM_RX_PKTHDR_DESC5		2
#define	NUM_TX_PKTHDR_DESC1		2

#define	ETH_REFILL_THRESHOLD1	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD2	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD3	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD4	0	// must < NUM_RX_PKTHDR_DESC
#define	ETH_REFILL_THRESHOLD5	0	// must < NUM_RX_PKTHDR_DESC

#define	QUEUEID0_RXRING_MAPPING		0
#define	QUEUEID1_RXRING_MAPPING		0
#define	QUEUEID2_RXRING_MAPPING		0
#define	QUEUEID3_RXRING_MAPPING		0
#define	QUEUEID4_RXRING_MAPPING		0
#define	QUEUEID5_RXRING_MAPPING		0
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define	NUM_TX_PKTHDR_DESC2		2
#define	NUM_TX_PKTHDR_DESC3		2
#endif

/* refer to rtl865xc_swNic.c & rtl865xc_swNic.h
 */
#define	UNCACHE_MASK   0x20000000

/* rxPreProcess */
#define	RTL8651_CPU_PORT                0x07 /* in rtl8651_tblDrv.h */
#define	_RTL865XB_EXTPORTMASKS   7
#define CONFIG_RTL_CUSTOM_PASSTHRU

typedef struct {
	uint16			vid;
	uint16			pid;
	uint16			len;
	uint16			priority:3;
	uint16			rxPri:3;
	void* 			input;
	struct dev_priv*	priv;
	uint32			isPdev;
#if defined(CONFIG_RTL_STP) && !defined(CONFIG_RTL_MULTI_LAN_DEV)
	int8				isStpVirtualDev;
#endif

#ifdef CONFIG_RTL_VLAN_8021Q
	uint16			srcvid;
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)&&defined(CONFIG_IPV6)
	struct dev_priv*	oriPriv;
#endif

}	rtl_nicRx_info;

typedef struct {
	uint16		vid;
	uint16		portlist;
	uint16		srcExtPort;
	uint16		flags;
	void 		*out_skb;

	// total: 16 bits
	uint16		txIdx:2;
#if defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTL_QOS_PATCH)|| defined(CONFIG_RTK_VOIP_QOS)
	uint16		priority:3;
	uint16		queueId:3;
#else
	uint16		resv2:6;
#endif
#if defined(CONFIG_RTL_8197F)
	uint16		extPortList:4;
	uint16		tx_qid:3;
#else
	uint16		resv3:7;
#endif
	uint16		resv4:1;

#if defined(CONFIG_RTK_VLAN_WAN_TAG_SUPPORT)
	uint16		tagport;
#endif
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	uint16 		wFlags;
#endif

	// todo: check they are needed or not !!
#if defined(CONFIG_RTL_8197F)

	/* defined for new Tx Shortcut */
	/* opts1 */
	uint32		type:3;			// [31:29]
	uint32		vi:1;			// [28]
	uint32		li:1;			// [27]
	uint32		pi:1;			// [26]
	uint32		pppidx:3;		// [25:23]
	uint32		len:17;			// [22:6]
	uint32		bridge:1;		// [5]
	uint32		hwlkup:1;		// [4]
	uint32		fs:1;			// [3]
	uint32		ls:1;			// [2]
	uint32		eor:1;			// [1]
	uint32		own:1;			// [0]

	/* opts2 */
	uint32		m_len:17;		// [31:15]
	uint32		qid:3;			// [14:12]
	uint32		pqid:3;			// [11:9]
	uint32		vlantagset:9;	// [8:0]

	/* opts3 */
	uint32		ptp_pkt:1;		// [31]
	uint32		ptp_typ:4;		// [30:27]
	uint32		ptp_ver:2;		// [26:25]
	uint32		dpri:3;			// [24:22]
	uint32		po:1;			// [21]
	uint32		l3cs:1;			// [20]
	uint32		l4cs:1;			// [19]
	uint32		ipv6:1;			// [18]
	uint32		ipv4:1;			// [17]
	uint32		ipv4_1st:1;		// [16]
								// [15] Reserved
	uint32		dp_ext:3;		// [14:12]
	uint32		dvlanid:12;		// [11:0]

	/* opts4 */
	uint32		lso:1;			// [31]
	uint32		dp:7;			// [30:24]
								// [23] Reserved
	uint32		linked:7;		// [22:16]
	uint32		ipv6_hdrlen:16;	// [15:0]

	/* opts5 */
	uint32		extspa:2;		// [31:30]
	uint32		mss:14;			// [29:16]
//	uint32		flags:8;		// [15:8]	// duplicated
	uint32		ipv4_hllen:4;	// [7:4]
	uint32		tcp_hlen:4;		// [3:0]	
#endif
}	rtl_nicTx_info;


#if defined(RTL_CPU_QOS_ENABLED)
#define	RTL_NIC_QUEUE_LEN					(32)
#define	RTL_CPUQOS_PKTCNT_THRESHOLD	(1000)
#define	RTL_ASSIGN_RX_PRIORITY			((highestPriority<cpuQosHoldLow)?((totalLowQueueCnt<RTL_CPUQOS_PKTCNT_THRESHOLD)?highestPriority:cpuQosHoldLow):highestPriority)

typedef struct {
	int	cnt;
	int	start;
	int	end;
	rtl_nicRx_info	entry[RTL_NIC_QUEUE_LEN];
}	rtl_queue_entry;
#else
#define	RTL_ASSIGN_RX_PRIORITY			0
#endif
/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_init
 * --------------------------------------------------------------------
 * FUNCTION: This service initializes the switch NIC.
 * INPUT   :
        userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_MAX_PKTDESC]: Number of Rx pkthdr descriptors. of each ring
        userNeedRxMbufRingCnt: Number of Rx mbuf descriptors.
        userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_MAX_PKTDESC]: Number of Tx pkthdr descriptors. of each ring
        clusterSize: Size of a mbuf cluster.
 * OUTPUT  : None.
 * RETURN  : Upon successful completion, the function returns ENOERR.
        Otherwise,
		EINVAL: Invalid argument.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
int32 swNic_init(uint32 userNeedRxPkthdrRingCnt[],
                 uint32 userNeedRxMbufRingCnt,
                 uint32 userNeedTxPkthdrRingCnt[],
                 uint32 clusterSize);



/* --------------------------------------------------------------------
 * ROUTINE NAME - swNic_intHandler
 * --------------------------------------------------------------------
 * FUNCTION: This function is the NIC interrupt handler.
 * INPUT   :
		intPending: Pending interrupts.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 * -------------------------------------------------------------------*/
void swNic_intHandler(uint32 intPending);
int32 swNic_flushRxRingByPriority(int priority);
__MIPS16 __IRAM_FWD int32 swNic_receive(rtl_nicRx_info *info, int retryCount);
int32 swNic_send(void *skb, void * output, uint32 len, rtl_nicTx_info *nicTx);
//__MIPS16
int32 swNic_txDone(int idx);
void swNic_freeRxBuf(void);
void swNic_freeTxRing(void);
int32	swNic_txRunout(void);
#if defined(DELAY_REFILL_ETH_RX_BUF) || defined(ALLOW_RX_RING_PARTIAL_EMPTY)
extern int check_rx_pkthdr_ring(int idx, int *return_idx);
extern int check_and_return_to_rx_pkthdr_ring(void *skb, int idx);
extern int return_to_rx_pkthdr_ring(unsigned char *head);
#endif
extern	uint32* rxMbufRing;
extern unsigned char *alloc_rx_buf(void **skb, int buflen);
extern unsigned char *alloc_rx_buf_init(void **skb, int buflen);
extern void free_rx_buf(void *skb);
#if defined(CONFIG_RTL_FAST_BRIDGE)
extern void tx_done_callback(void *skb);
#endif
extern void eth_save_and_cli(unsigned long *flags);
extern void eth_restore_flags(unsigned long flags);

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
int32 swNic_setVlanPortTag(int portmask);
#endif

#define	RTL8651_IOCTL_GETWANLINKSTATUS			2000
#define	RTL8651_IOCTL_GETLANLINKSTATUS			2001
#define	RTL8651_IOCTL_GETWANTHROUGHPUT			2002
#define	RTL8651_IOCTL_GETLANPORTLINKSTATUS		2003
#define	RTL8651_IOCTL_GETWANPORTLINKSTATUS		2004
#define 	RTL8651_IOCTL_GETWANLINKSPEED 			2100
//#define 	RTL8651_IOCTL_SETWANLINKSPEED 			2101
#if defined(CONFIG_AUTO_DHCP_CHECK) || defined(CONFIG_RTK_WLAN_EVENT_INDICATE)
#define RTL8651_IOCTL_SET_GETLINKSTATUS_PID			2102
#endif
#define RTL8651_IOCTL_GETLANLINKSTATUSALL		2105

#define	RTL8651_IOCTL_SETWANLINKSTATUS			2200

#define	RTL8651_IOCTL_CLEARBRSHORTCUTENTRY		2210

#define	RTL8651_IOCTL_GETPORTIDBYCLIENTMAC		2013

#if defined(CONFIG_RTL_ETH_802DOT1X_SUPPORT)
#define	RTL8651_IOCTL_DOT1X_SETPID		           2300
#define RTL8651_IOCTL_DOT1X_GET_INFO              2301
#define RTL8651_IOCTL_DOT1X_SET_AUTH_RESULT   	   2302
#endif

#define	RTL_NICRX_OK	0
#define	RTL_NICRX_REPEAT	-2
#define	RTL_NICRX_NULL	-1
#if defined(REINIT_SWITCH_CORE)
int32 swNic_reInit(void);
#endif
#if 	defined(CONFIG_RTL_PROC_DEBUG)
int32	rtl_dumpRxRing(struct seq_file *s);
int32	rtl_dumpTxRing(struct seq_file *s);
int32	rtl_dumpMbufRing(struct seq_file *s);
int32	rtl_dumpIndexs(void);
#endif

struct ring_que {
	int qlen;
	int qmax;
	int head;
	int tail;
	struct sk_buff **ring;
};

#if defined(CONFIG_RTL_NIC_QUEUE)
#define TX_QUEUE_NUM 6
#define TX_DEV_NUM 5

struct rtk_tx_tbf {
	unsigned int token; /*in bytes*/
	unsigned int buffer;
};

struct rtk_tx_queue {
	unsigned char used;	
	struct net_device *dev;
	struct sk_buff_head list;
	struct rtk_tx_tbf tbf;
	int phyPort;
};

struct rtk_tx_dev {
	unsigned int hit_count;
	unsigned int hit_flag;
#ifdef CONFIG_RTL_MULTI_LAN_DEV
	struct net_device *dev;
#else
	int dport;
#endif
};
#endif

static inline void *UNCACHED_MALLOC(int size)
{
	return ((void *)(((uint32)kmalloc(size, GFP_ATOMIC)) | UNCACHE_MASK));
}

static inline void *CACHED_MALLOC(int size)
{
	return ((void *)(((uint32)kmalloc(size, GFP_ATOMIC))));
}

#ifdef CONFIG_RTL_SWITCH_NEW_DESCRIPTOR
//#define UDP_FRAGMENT_PKT_QUEUEING			1 /* for wifi 802.11AC LOGO (sigma) test */
#endif

#ifdef UDP_FRAGMENT_PKT_QUEUEING

#define UF_QUE_SKB_NUM		20
#define UF_TIMEOUT_VALUE	3
#define UF_START_QUEUEING	80
//#define _UF_DEBUG			1

typedef struct uf_s
{
	rtl_nicTx_info uf_tx_info;
	struct sk_buff *uf_skb_list[UF_QUE_SKB_NUM]; // use Linux link list later
	//uint32 uf_next_offset;
	uint16 uf_ip_id;
	uint8 uf_expired_timer;
	uint8 que_num;
} uf_t;

extern uint8 uf_enabled;
extern uint8 uf_start;
extern uint16 uf_tx_desc_low;
extern uf_t udp_frag[];

#ifdef _UF_DEBUG
extern int _uf_cntr_tx_group_pkt, _uf_cntr_free_group_pkt, _uf_cntr_timer_timeout;
extern int _uf_cntr_tx_unfinished_group_pkt, _uf_cntr_queue_end;
#endif
				
extern int32 process_udp_fragment_pkt(struct sk_buff *skb, rtl_nicTx_info *nicTx);
extern void uf_timeout_check(void);
#endif

#endif /* _SWNIC_H */
