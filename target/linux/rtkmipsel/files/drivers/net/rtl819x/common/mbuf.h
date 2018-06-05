
/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : The mbuf module header file
* Abstract :
* Author : David Chun-Feng Liu (cfliu@realtek.com.tw)
*
*/
#ifndef _MBUF_H_
#define	_MBUF_H_

/*********************************************************************************
	SECTION 1:		mbuf module default settings
**********************************************************************************/
#define	PKT_MBUF_CLUSTER_LEN		1650

#define PKT_BUF_SZ		PKT_MBUF_CLUSTER_LEN /* Size of each temporary Rx buffer.*/

/*********************************************************************************
	SECTION 2:		mbuf module data structure definitions
**********************************************************************************/
#define 	BUF_FREE			0x00   /* Buffer is Free  */
#define 	BUF_USED			0x80   /* Buffer is occupied */
#define 	BUF_ASICHOLD		0x80   /* Buffer is hold by ASIC */
#define 	BUF_DRIVERHOLD 	0xc0   /* Buffer is hold by driver */

#define 	MBUFTYPE_DATA		0x01	//not in use now. Keep for backward compatablity.

#define _PKTHDR_CACHEABLE		1

/*@struct m_buf | The mbuf header associated with each cluster */

struct rtl_mBuf
{
    struct rtl_mBuf *m_next;
    struct rtl_pktHdr *m_pkthdr;            /* Points to the pkthdr structure */

#ifdef _LITTLE_ENDIAN
    int8      m_reserved2;              /* padding */
    int8      m_flags;                  /* mbuf flags; see below */
    uint16    m_len;                    /* data bytes used in this cluster */
#else	
    uint16    m_len;                    /* data bytes used in this cluster */
    int8      m_flags;                  /* mbuf flags; see below */
    int8      m_reserved2;              /* padding */
#endif

#define MBUF_FREE            BUF_FREE   /* Free. Not occupied. should be on free list   */
#define MBUF_USED            BUF_USED   /* Buffer is occupied */
#define MBUF_EXT             0x10       /* has associated with an external cluster, this is always set. */
#define MBUF_PKTHDR          0x08       /* is the 1st mbuf of this packet */
#define MBUF_EOR             0x04       /* is the last mbuf of this packet. Set only by ASIC*/
    uint8     *m_data;                  /*  location of data in the cluster */
    uint8     *m_extbuf;                /* start of buffer*/

#ifdef _LITTLE_ENDIAN
    int8      m_reserved[2];            /* padding */
    uint16    m_extsize;                /* sizeof the cluster */
#else	
    uint16    m_extsize;                /* sizeof the cluster */
    int8      m_reserved[2];            /* padding */
#endif

    void			*skb;
#if defined(_PKTHDR_CACHEABLE)
    uint32	 pending0;	// for cache line alignment
#endif
};

/*@struct rtl_pktHdr |  pkthdr records packet specific information. Each pkthdr is exactly 32 bytes.
 first 20 bytes are for ASIC, the rest 12 bytes are for driver and software usage.
 */
struct rtl_pktHdr
{
    union
    {
        struct rtl_pktHdr *pkthdr_next;     /*  next pkthdr in free list */
        struct rtl_mBuf *mbuf_first;        /*  1st mbuf of this pkt */
    }PKTHDRNXT;
#define ph_nextfree         PKTHDRNXT.pkthdr_next
#define ph_mbuf             PKTHDRNXT.mbuf_first

#ifdef _LITTLE_ENDIAN

    uint16    ph_srcExtPortNum: 2;      /* Both in RX & TX. Source extension port number. */
    uint16    ph_l2Trans: 1;            /* l2Trans - copy from HSA bit 129 */
    uint16    ph_isOriginal: 1;         /* isOriginal - DP included cpu port or more than one ext port */
    uint16    ph_hwFwd: 1;              /* hwFwd - copy from HSA bit 200 */
    uint16    ph_reserved2: 3;          /* reserved */
    uint16    ph_extPortList: 4;        /* dest extension port list. must be 0 for TX */
    uint16    ph_queueId: 3;            /* bit 2~0: Queue ID */
    uint16    ph_reserved1: 1;           /* reserved */
    uint16    ph_len;                   /*   total packet length */

#else
    uint16    ph_len;                   /*   total packet length */
    uint16    ph_reserved1: 1;           /* reserved */
    uint16    ph_queueId: 3;            /* bit 2~0: Queue ID */
    uint16    ph_extPortList: 4;        /* dest extension port list. must be 0 for TX */
    uint16    ph_reserved2: 3;          /* reserved */
    uint16    ph_hwFwd: 1;              /* hwFwd - copy from HSA bit 200 */
    uint16    ph_isOriginal: 1;         /* isOriginal - DP included cpu port or more than one ext port */
    uint16    ph_l2Trans: 1;            /* l2Trans - copy from HSA bit 129 */
    uint16    ph_srcExtPortNum: 2;      /* Both in RX & TX. Source extension port number. */
#endif

		/* for ph_extPortList */
		#define	PKTHDR_EXTPORT_MAGIC		0xA530
		#define	PKTHDR_EXTPORT_MAGIC2		0xA531
		#define	PKTHDR_EXTPORT_MAGIC3		0xA532
		#define	PKTHDR_EXTPORT_MAGIC4		0xA533
		#define	PKTHDR_EXTPORT_P1               6
		#define	PKTHDR_EXTPORT_P2               7
		#define	PKTHDR_EXTPORT_P3               8
		
		#define PKTHDR_EXTPORT_LIST_P0		0
		#define PKTHDR_EXTPORT_LIST_P1		1
		#define PKTHDR_EXTPORT_LIST_P2		2
		#define PKTHDR_EXTPORT_LIST_CPU		3
		#define PKTHDR_EXTPORTMASK_P0		(0x1 << (PKTHDR_EXTPORT_LIST_P0))
		#define PKTHDR_EXTPORTMASK_P1		(0x1 << (PKTHDR_EXTPORT_LIST_P1))
		#define PKTHDR_EXTPORTMASK_P2		(0x1 << (PKTHDR_EXTPORT_LIST_P2))
		#define PKTHDR_EXTPORTMASK_CPU		(0x1 << (PKTHDR_EXTPORT_LIST_CPU))
		#define PKTHDR_EXTPORTMASK_ALL		(	PKTHDR_EXTPORTMASK_P0 |\
												PKTHDR_EXTPORTMASK_P1 |\
												PKTHDR_EXTPORTMASK_P2 |\
												PKTHDR_EXTPORTMASK_CPU \
											)

#ifdef _LITTLE_ENDIAN
    uint16    ph_reason;                /* indicates wht the packet is received by CPU */

    uint16    ph_linkID: 7;             /* for WLAN WDS multiple tunnel */
    uint16    ph_pppoeIdx: 3;
    uint16    ph_pppeTagged: 1;         /* the tag status after ALE */
    uint16    ph_LLCTagged: 1;          /* the tag status after ALE */
    uint16    ph_vlanTagged: 1;         /* the tag status after ALE */
    uint16    ph_type: 3;
#else
    uint16    ph_type: 3;
    uint16    ph_vlanTagged: 1;         /* the tag status after ALE */
    uint16    ph_LLCTagged: 1;          /* the tag status after ALE */
    uint16    ph_pppeTagged: 1;         /* the tag status after ALE */
    uint16    ph_pppoeIdx: 3;
    uint16    ph_linkID: 7;             /* for WLAN WDS multiple tunnel */
    uint16    ph_reason;                /* indicates wht the packet is received by CPU */
#endif

#define	ph_proto				ph_type
#define PKTHDR_ETHERNET      0
#define PKTHDR_PPTP          1
#define PKTHDR_IP            2
#define PKTHDR_ICMP          3
#define PKTHDR_IGMP          4
#define PKTHDR_TCP           5
#define PKTHDR_UDP           6
#define PKTHDR_IPV6          7

#ifdef _LITTLE_ENDIAN
    uint8     ph_portlist;              /* RX: source port number, TX: destination portmask */
    uint8     ph_orgtos;                /* RX: original TOS of IP header's value before remarking, TX: undefined */
    uint16    ph_flags;                 /*  NEW:Packet header status bits */
#else
    uint16    ph_flags;                 /*  NEW:Packet header status bits */
    uint8     ph_orgtos;                /* RX: original TOS of IP header's value before remarking, TX: undefined */
    uint8     ph_portlist;              /* RX: source port number, TX: destination portmask */
#endif

#define PKTHDR_FREE          (BUF_FREE << 8)        /* Free. Not occupied. should be on free list   */
#define PKTHDR_USED          (BUF_USED << 8)
#define PKTHDR_ASICHOLD      (BUF_ASICHOLD<<8)      /* Hold by ASIC */
#define PKTHDR_DRIVERHOLD    (BUF_DRIVERHOLD<<8)    /* Hold by driver */
#define PKTHDR_CPU_OWNED     0x4000
#define PKT_INCOMING         0x1000     /* Incoming: packet is incoming */
#define PKT_OUTGOING         0x0800     /*  Outgoing: packet is outgoing */
#define PKT_BCAST            0x0100     /*send/received as link-level broadcast  */
#define PKT_MCAST            0x0080     /*send/received as link-level multicast   */
#define PKTHDR_BRIDGING      0x0040     /* when PKTHDR_HWLOOKUP is on. 1: Hardware assist to do L2 bridging only, 0:hardware assist to do NAPT*/
#define PKTHDR_HWLOOKUP      0x0020	/* valid when ph_extPortList!=0. 1:Hardware table lookup assistance*/
#define PKTHDR_PPPOE_AUTOADD    0x0004  /* PPPoE header auto-add */
#define CSUM_TCPUDP_OK       0x0001     /*Incoming:TCP or UDP cksum checked */
#define CSUM_IP_OK           0x0002     /* Incoming: IP header cksum has checked */
#define CSUM_TCPUDP          0x0001     /*Outgoing:TCP or UDP cksum offload to ASIC*/
#define CSUM_IP              0x0002     /* Outgoing: IP header cksum offload to ASIC*/


#ifdef _LITTLE_ENDIAN
	union
	{
		uint16	_flags2;			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
								/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
		struct
		{
			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
			uint16 _svlanId:12;			/* Source (Original) VLAN ID */
			uint16 _rxPktPriority:3;		/* Rx packet's original priority */
			uint16 _reserved:1;
		} _rx;

		struct
		{
			/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
			
			uint16 _txCVlanTagAutoAdd:6;	/* BitMask to indicate the port which would need to add VLAN tag */
			#if defined(CONFIG_RTL_8197F)
            uint16 _reserved:2;
			uint16 _q_priority_tx:3;            
			uint16 _reserved_2:5;

			#define ph_q_priority_tx	_flags2._tx._q_priority_tx
			#else
			uint16 _reserved:10;
			#endif            
		} _tx;
	} _flags2;

    uint16     ph_vlanId: 12;
    uint16     ph_txPriority: 3;
    uint16     ph_vlanId_resv: 1;
#else
   uint16     ph_vlanId_resv: 1;
   uint16     ph_txPriority: 3;
   uint16     ph_vlanId: 12;
	union
	{
		uint16	_flags2;			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
								/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
		struct
		{
			/* RX: bit 15: Reserved, bit14~12: Original Priority, bit 11~0: Original VLAN ID */
			uint16 _reserved:1;
			uint16 _rxPktPriority:3;		/* Rx packet's original priority */
			uint16 _svlanId:12;			/* Source (Original) VLAN ID */
		} _rx;

		struct
		{
			/* TX: bit 15~6: Reserved, bit 5~0: Per Port Tag mask setting for TX(bit 5:MII, bit 4~0: Physical Port) */
			uint16 _reserved:10;
			uint16 _txCVlanTagAutoAdd:6;	/* BitMask to indicate the port which would need to add VLAN tag */
		} _tx;
	} _flags2;
#endif

	#define ph_dvlanId		ph_vlanId
	#define ph_rxPriority		ph_txPriority

	#define PKTHDR_TXPRIORITY_MIN			0
	#define PKTHDR_TXPRIORITY_MAX			7

	#define ph_flags2				_flags2._flags2
	#define ph_svlanId				_flags2._rx._svlanId
	#define ph_rxPktPriority			_flags2._rx._rxPktPriority
	#define ph_txCVlanTagAutoAdd	_flags2._tx._txCVlanTagAutoAdd

	#define PKTHDR_TXCVID(vid)					(vid & 0xfff)
	#define PKTHDR_VLAN_P0_AUTOADD			(0x0001<<0)
	#define PKTHDR_VLAN_P1_AUTOADD			(0x0001<<1)
	#define PKTHDR_VLAN_P2_AUTOADD			(0x0001<<2)
	#define PKTHDR_VLAN_P3_AUTOADD			(0x0001<<3)
	#define PKTHDR_VLAN_P4_AUTOADD			(0x0001<<4)
	#define PKTHDR_VLAN_P5_AUTOADD			(0x0001<<5)
	#define PKTHDR_VLAN_AUTOADD				(	(PKTHDR_VLAN_P0_AUTOADD)|	\
												(PKTHDR_VLAN_P1_AUTOADD)|	\
												(PKTHDR_VLAN_P2_AUTOADD)|	\
												(PKTHDR_VLAN_P3_AUTOADD)|	\
												(PKTHDR_VLAN_P4_AUTOADD)|	\
												(PKTHDR_VLAN_P5_AUTOADD)	)

#ifdef _LITTLE_ENDIAN
	#if defined(CONFIG_RTL_8197F)
	uint16	ph_ipIpv6HdrLen;	/* IPv6 header length */
	uint8	ph_ipIpv4_1st:1;	/* IPv4 first header */
	uint8	ph_ipIpv4:1;		/* IPv4 Header flag */
	uint8	ph_ipIpv6:1;		/* IPv6 Header flag */
	uint8	ph_ipResv:5;

	uint8	ph_ptpPkt:1;		/* 1: PTP */
	uint8	ph_ptpVer:2;		/* PTP version, 0: 1588v1; 1: 1588v2 or 802.1as; others: reserved */
	uint8	ph_ptpMsgType:4;	/* message type */
	uint8	ph_ptpResv:1;
	#else
	uint8	ph_reserved[4];
	#endif	
#else
	#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8198C)
	uint8	ph_ptpResv:1;
	uint8	ph_ptpMsgType:4;	/* message type */
	uint8	ph_ptpVer:2;		/* PTP version, 0: 1588v1; 1: 1588v2 or 802.1as; others: reserved */
	uint8	ph_ptpPkt:1;			/* 1: PTP */
	#else
	uint8	ph_ptpreserved;
	#endif

	#if defined(CONFIG_RTL_8198C)
	uint8	ph_ipResv:5;
	uint8	ph_ipIpv6:1;		/* IPv6 Header flag */
	uint8	ph_ipIpv4:1;		/* IPv4 Header flag */
	uint8	ph_ipIpv4_1st:1;	/* IPv4 first header */
	uint16	ph_ipIpv6HdrLen;	/* IPv6 header length */
	#else
	uint8	ph_reserved[3];		/* padding */
	#endif
#endif

#if defined(_PKTHDR_CACHEABLE)
	uint32 	pending0;	//for cache line alianment
	uint32	pending1;
#endif
};

	//property for ph_unnumber		: cw_du 
	#define PHUNNUMBER_SRCGLOBEIP			0x01
	#define PHUNNUMBER_DIRECTION			0x02		// 1 : WAN->LAN; 	0: LAN->WAN
	#define PHUNNUMBER_UNNUMBEREDSRCIP	0x04
	#define PHUNNUMBER_ADVRTMATCHED		0x08
	#define PHUNNUMBER_ALLBITS				0xff

	
	#define PKTHDR_PHUNNUMBER_SET(pkthdrPtr, property) \
		do{\
			assert(pkthdrPtr);\
			pkthdrPtr->ph_unnumber |=property;\
		}while(0)

	#define PKTHDR_PHUNNUMBER_CLR(pkthdrPtr, property) \
		do{\
			assert(pkthdrPtr);\
			pkthdrPtr->ph_unnumber &= ~property;\
		}while(0)

	#define PKTHDR_PHUNNUMBER_TEST(pkthdrPtr, property) 	((pkthdrPtr->ph_unnumber & property) ? 1: 0)
	
struct rtl_mBufStatus
{
	uint32 	m_totalmbufs;	/* Total mbufs allocated during initialization */
	uint32 	m_totalclusters;/* Total clusters allocated during initialization */
	uint32 	m_totalpkthdrs;	/* Total pkthdrs allocated during initialization */
	uint32 	m_freembufs;	/* free mbufs in pool now*/
	uint32 	m_freeclusters;	/* free clusters in  pool now*/
	uint32 	m_freepkthdrs;	/* free pkthdrs in pool now*/
	uint32 	m_msize;		/* length of an mbuf */
	uint32 	m_mclbytes;		/* length of an mbuf cluster */
	uint32 	m_pkthdrsize;	/* length of an pkthdr */
	uint32 	m_wait;			/* times waited for space, includes mbuf, pkthdr and cluster */
};


/*********************************************************************************
	SECTION 3:		mbuf module exported variables, symbols and macros
**********************************************************************************/
#define	MBUF_COPYALL		1000000000	   /* length for m_copy to copy all */
#define	MBUF_WAITOK		0x01
#define	MBUF_DONTWAIT	0x02	   /* Don't wait if there is no buffer available */
#define	MBUF_ONLY	0x04	   /* Don't allocate a cluster in mBuf_get */
#define	MBUF_ALLOCPKTHDR	0x08	   /* Allocate a packet header with mbuf chain in mBuf_getm, mBuf_cloneMbufChain, mBuf_dupMbufChain*/
#define	MBUF_GETNEWMBUF 0x10		   /* In mBuf_prepend, alloate new mbufs directly */
#define MBUF_CHECKPKTHDR(m)	((m)&&(ISSET((m)->m_flags, MBUF_USED) && ((m)->m_pkthdr))?1:0)
#define MBUF_GETPKTHDRFIELD16(field)  (*((uint16 *)(field)))
#define MBUF_SETPKTHDRFIELD16(field, value)	*((uint16 *)(field)) = (value)

//The size of each cluster.
extern int32 m_clusterSize;

/*********************************************************************************
	SECTION 4:		mbuf module exported API prototype
**********************************************************************************/

/* mbuf module exported APIs */

/*	@doc MBUF_API

	@module mbuf.h - Mbuf module API documentation	|
	This document illustrates the API interface of the mbuf module.
	@normal Chun-Feng Liu (cfliu@realtek.com.tw) <date>

	Copyright <cp>2001 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | MBUF_API
*/
extern int32 mBuf_init(uint32, uint32, uint32, uint32, uint32);			   //was tunable_mbinit()
#if defined(RTL865X_TEST)||defined(RTL865X_MODEL_USER)
extern int32 mBuf_Reinit(void);
#endif /* RTL865X_TEST */
/*
@func void	| mBuf_init	| mbuf module initialization
@parm uint32 	| mbufs		| Number of mbufs
@parm uint32 	| clusters	| Number of clusters
@parm uint32 	| pkthdrs		| Number of packet headers
@parm uint32		| clusterSize	| Size of each cluster. must be power of 2
@parm uint32		| msgLogId	| Debuging message log list id
@rdesc None
@rvalue SUCCESS		| 	The mbuf module and its memory pool is initiated as requested
@rvalue FAILED	| 	Failed to initialize the mbuf module.
@comm mbuf module initialization. Allocate mbuf and related structure pools. Value for <p clusterSize> must be power of 2. 
If <p clusters> is 0, clusters would be allocated externally through registered OS buffer allocation glue function. If clusters
are allocated this way, MBUF_WAITOK flag can't be used for mBuf_get, mBuf_getm, mBuf_dupMbufChain, mBuf_dupPacket since
mbuf module knows nothing about external cluter pool managed by OS hence doesn't know when to wakeup waiting threads.
 */


extern int32 mBuf_getBufStat(struct rtl_mBufStatus *mbs);
/*
@func  int32	| mBuf_getBufStat	| Returns current status of the mbuf module
@parm struct rtl_mBufStatus *	| mbs	| A pointer to the mbstat structure for mBuf_getBufStat() to fill in
@rdesc Returns current status of the mbuf module
@rvalue <p FAILED>	| 	Failed to get mbuf module status. Maybe mbs is NULL or mbuf module not yet initiated.
@rvalue <p SUCCESS>	| 	Mbuf module status is returned with the mbstat structure given.
 */



extern int32 mBuf_leadingSpace(struct rtl_mBuf *m);

/*
@func int32	| mBuf_leadingSpace	| Calculate the number of leading free data bytes.
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns the number of free leading space
@rvalue n		| 	The number of free leading data bytes in cluster.
@comm
Calculate the number of leading free data bytes.
@xref  <c mBuf_trailingSpace>

 */


extern int32 mBuf_trailingSpace(struct rtl_mBuf *m);

/*
@func int32	| mBuf_trailingSpace	| Calculate the number of trailing free data bytes.
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns the number of free trailing space
@rvalue n		| 	The number of free trailing data bytes in cluster.
@comm
Calculate the number of trailing free data bytes.
@xref <c mBuf_leadingSpace>

 */


extern int32 mBuf_clusterIsWritable(struct rtl_mBuf *m);

/*
@func int32	| mBuf_clusterIsWritable	| Determine whether <p m>'s cluster is writable or not.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf.
@rdesc Returns TRUE or FALSE
@rvalue TRUE		| 	The cluster is writable.
@rvalue FALSE	| 	The cluster is not writable.
@comm
Determine whether <p m>'s cluster is writable or not. New mbufs allocated due to mBuf_clonePacket, mBuf_split, mBuf_cloneMbufChain
should not modify its cluster becoz it is not the owner of these clusters. This function always return TRUE if the 'clusters' parameter 
during mBuf_init() was initialized to 0.

 */

extern   uint32 mBuf_getPktlen(struct rtl_mBuf *m);
/*
@func uint32	| mBuf_getPktlen	| Get total number of data bytes in the packet which <p m> belongs to
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns data length of the packet
@rvalue -1		| 	Failed.
@rvalue <p length>| 	The actual length of  packet
@comm
Get total number of data bytes in the packet which <p m> belongs to
@xref <c MBUF_GETLEN>
*/

extern struct rtl_mBuf *mBuf_data2Mbuf(int8 * x);  //was dtom

/*
@func struct rtl_mBuf *	| mBuf_data2Mbuf	| Given data address <p x>, return mbuf address <p m>
@parm int8  * 	|x		| Data address
@rdesc Returns mbuf address <p m> which owns the cluster block where <p x> resides.
@rvalue <p NULL>		| 	Can't find the address. The data address <p x> given might be within a zombie cluster whose owning mbuf has already been freed.
@rvalue <p m>		| 	The address of owning mbuf
@comm
Finds the mbuf address of given data address <p x>
This function can't be called if the 'clusters' parameter during mBuf_init() was initialized to 0.
@devnote  Original BSD code define this as a dtom macro. In our implmentation, we make it a function.
@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_data2Mbuf(x);
	if (n != NULL) {
		printf("There are %d bytes in the cluster", n->m_len);
	}
  @xref  <c MBUF2DATAPTR>
*/



extern struct rtl_mBuf *mBuf_get(int32 how, int32 unused, uint32 Nbuf);

/*
@func struct rtl_mBuf *	| mBuf_get	| mbuf space allocation routines.
@parm int32			| how	| <p MBUF_WAITOK>, <p MBUF_DONTWAIT>, and/or <p MBUF_ONLY>
@parm int32			| unused	| unused now. Keep for backward compatibility.
@parm uint32 			| Nbuf	| The number of mbufs requesting.

@rdesc Returns the address of allocated mbuf head
@rvalue <p NULL>		| 	Can't allocate all  <p Nbuf>s at once.
@rvalue <p n>		| 	The address of first mbuf allocated. All <p Nbuf>s have been linked together.
@comm
Get <p Nbuf> mbufs and clusters. All mbufs are chained via the <p m_next> pointer inside each mbuf. Note that memory content in all allocated clusters are NOT initialized.

If <p how> has MBUF_DONTWAIT bit set, and we can't get all <p Nbuf>s immediately, return NULL.

If <p how> has MBUF_WAITOK bit set, and we can't get all <p Nbuf>s right away, block waiting until our request is satisfied

If <p how> has MBUF_ONLY bit set, then no clusters would be allocated. Only mbufs are allocated. Can be used with MBUF_DONTWAIT or MBUF_WAITOK

@devnote
1.Side effect: <p mBuf_get> wakes up any sleeping threads if they are block waiting for free mbufs or clusters.
2. If the 'clusters' parameter during mBuf_init() was initialized to 0, mBuf_get doesn't accept MBUF_ONLY flag

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *m;
	m= mBuf_get( MBUF_DONTWAIT, 0, 5);  //allocate 5 mbufs and clusters without blocking.
	if (!m)
		return NULL;

  @xref
  <c mBuf_getCleared> ,  <c mBuf_getm>, <c mBuf_driverGet>
 */


extern struct rtl_mBuf *mBuf_getCleared(int32 how, int32 unused, uint32 Nbuf);

/*
@func struct rtl_mBuf *	| mBuf_getCleared| Same as mBuf_get, but initialize all clusters to zero.
@parm int32			|how	| <p MBUF_WAIT>, <p MBUF_DONTWAIT> and/or <p MBUF_ONLY>
@parm int32			|unused	| unused now. Keep for backward compatibility.
@parm uint32 			| Nbuf	| The number of mbufs requesting.

@rdesc Returns the address of allocated mbuf head
@rvalue <p NULL>		| 	Can't allocate all  <p Nbuf>s at once.
@rvalue <p n>		| 	The address of first mbuf allocated. All <p Nbuf>s have been linked together and cleared to 0.
@comm
Same as <p mget>(). However, content in all mbufs and clusters have been cleared to 0.
@ex The following example demonstrates how to use this function |
	register struct rtl_mBuf *m;
	m= mBuf_getCleared(MBUF_DONTWAIT | MBUF_ONLY, 0, 5);  //allocate 5  mbufs without blocking.
	if (!m)
		return NULL;

  @xref
  <c mBuf_get>,  <c mBuf_getm>, <c mBuf_driverGet>
 */


extern uint32 mBuf_driverGet(uint32 Nmbuf,struct rtl_mBuf **ppFirstMbuf, struct rtl_mBuf **ppLastMbuf);
/*
@func uint32	| mBuf_driverGet| Driver specific. Get multiple mbufs without blocking
@parm uint32	|Nmbuf	| Number of mbufs requesting
@parm struct rtl_mBuf **	|ppFirstMbuf	| Returns a pointer which points to  the first mbuf allocated.
@parm struct rtl_mBuf **	| ppLastMbuf	| Returns a pointer which points to  the last mbuf allocated.
@rdesc Returns number of mbufs successfully allocated.
@comm
An optimized mBuf_get() for driver. <p ppFirstMbuf> and <p ppLastMbuf> should not be NULL!!
mbuf and cluster not initialized to zero. Only required fields in mBuf_driverGet have been properly filled.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf * localTempMbufHead,  * localTempMbufTail;
	int32 allocated;
	requested = 5;
	allocated= mBuf_driverGet(requested, &localTempMbufHead, &localTempMbufTail);
	if (allocated!=requested)
		printf("Can't allocate all 5 mbufs in driver\n")

  @xref
  <c mBuf_get>,  <c mBuf_getm>, <c mBuf_getCleared>
 */

extern struct rtl_mBuf *mBuf_getm(struct rtl_mBuf *m, uint32 len, int32 how, int32 unused);

/*
@func struct rtl_mBuf *	| mBuf_getm| allocate <p len>-worth of  mbuf clusters and return a pointer to the top
of the allocated chain.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. Could be NULL.
@parm uint32			|len		| Number of data bytes requesting.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT> and/or <p M_ALLOCPKTHDR>.
@parm int32			|unused	| unused now. Keep for backward compatibility.
@rdesc Returns return a pointer to the head of the allocated chain.
@rvalue <p NULL>		| 	Allocation failed.
@rvalue <p n>		| 	The memory address of first mbuf allocated.
@comm
If <p m> is non-null, then it is an mbuf chain to which we want <p len> bytes worth of clusters and their associating mbufs be attached, and so
if allocation is successful, a pointer to m is returned. i.e <p n> = <p m>)

If <p m> is null,  <p len> bytes worth of clusters and their associating mbufs would be allocated.

If <p how> has M_ALLOCPKTHDR bit set, <p mBuf_getm> also allocates a pkthdr with the mbuf chain.
If <p how> has MBUF_DONTWAIT bit set, and we can't get a pkthdr immediately, return NULL.
If <p how> has MBUF_WAITOK bit set, and we can't get a pkthdr right away, block waiting until our request is satisfied.

M_BUFONLY can't be used with mBuf_getm, in this case, you should use mBuf_get() and calculate how many mbufs you need on your own.

@ex The following example demonstrates how to use this function |
	m = mBuf_getm(m, size, MBUF_WAITOK, 0);
	if (m == NULL)
		return ENOBUFS;

  @xref
  <c mBuf_get> , <c mBuf_driverGet>, <c mBuf_getCleared>
 */


extern struct rtl_mBuf *mBuf_getPkthdr(struct rtl_mBuf *m, int32 how);
/*
@func struct rtl_mBuf *	| mBuf_getPkthdr| Allocate a packet header for mbuf chain <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>

@rdesc Returns the address of mbuf <p m> that holds a packet header.
@rvalue <p NULL>		| 	Can't allocate a  pkthdr for mbuf <p m>.
@rvalue <p m>		| 	A pkthdr is allocated to mbuf <p m>.
@comm

Given an mbuf <p m>, allocate a packet header (pkthdr) structure for <p m> and makes <p m> the owner of this pkthdr.
If there is already a pkthdr owned by some mbuf after m on the same mbuf chain, m becomes the owner of that pkthdr without getting a new one. Corresponding pkthdr fields are modified accordingly.

If <p how> has MBUF_DONTWAIT bit set, and we can't get a pkthdr immediately, return NULL.
If <p how> has MBUF_WAITOK bit set, and we can't get a pkthdr right away, block waiting until our request is satisfied.


 mBuf_getPkthdr may wake up other sleeping threads waiting for pkthdrs if any

@ex The following example demonstrates how to use this function |
	if (!mBuf_getPkthdr(pNewMbuf, MBUF_DONTWAIT))	//get a pkthdr
		goto nospace;
  @xref
  <c mBuf_driverGetPkthdr>
 */

extern uint32 mBuf_driverGetPkthdr(uint32 Npkthdr,struct rtl_pktHdr **ppHeadPkthdr, struct rtl_pktHdr **ppTailPkthdr);
/*
@func uint32	| mBuf_driverGetPkthdr| Driver specific. Get multiple pkthdrs without blocking
@parm uint32	|Npkthdr	| Number of pkthdrs requesting
@parm struct rtl_pktHdr **	|ppHeadPkthdr	| Returns a pointer which points to  the first pkthdr allocated.
@parm struct rtl_pktHdr **	| ppTailPkthdr	| Returns a pointer which points to  the last pkthdr allocated.
@rdesc Returns number of pkthdrs successfully allocated.
@comm
This function is dedicated for driver.  <p ppHeadPkthdr> and <p ppTailPkthdr> should not be NULL!!

@ex The following example demonstrates how to use this function |
	struct rtl_pktHdr * localTempPkthdrHead,  * localTempPkthdrTail;
	int32 allocated;
	requested = 5;
	allocated= mBuf_driverGetPkthdr(requested, &localTempPkthdrHead, &localTempPkthdrTail);
	if (allocated!=requested)
		printf("Can't allocate all 5 mbufs in driver\n")

  @xref
  <c mBuf_getPkthdr>
 */


extern void mBuf_freeMbuf(struct rtl_mBuf *m);
/*
@func struct rtl_mBuf *	| mBuf_freeMbuf	| Free a single mbuf <p m>..
@parm struct rtl_mBuf * 	| m		|  Pointer to an mbuf chain.
@rdesc None
@comm
 Free a single mbuf. Leave assocated pkthdr or cluster, if any, intact.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	mBuf_freeMbuf(m);
@xref
  <c mBuf_freeMbufChain> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>
 */


extern int32 mBuf_attachCluster(struct rtl_mBuf *m, void *buffer, uint32 id, uint32 size, uint16 datalen, uint16 align);
/*
@func struct rtl_mBuf *	| mBuf_attachCluster	| Attach a cluster to mbuf <p m>
@parm struct rtl_mBuf * 	| m		|  Pointer to an mbuf chain.
@parm void * 	| buffer	|  Address of cluster to be associated with mbuf <p m>.
@parm uint32 	| id		| Identifier of cluster to be associated with mbuf <p m>.
@parm uint32  	| size	|  Size of cluster to be associated with mbuf <p m>.
@parm uint32  	| datalen	|  Byte count of data already in packet.
@parm uint16  	| align	|  byte offset of first data byte in cluster <p buffer>,if any,from which <p m> attaches to. Default is 0.
@rvalue <p SUCCESS>		| 	cluster has been successfully attached with mbuf <p m>.
@rvalue <p FAILED>		| 	Failed to attach cluster with mbuf <p m>
@comm
Attach a cluster to mbuf <p m>. If there is already a cluster attached with mbuf <p m>, return FAILED.
If <p align> is non-zero, mbuf <p m> sets it's data pointer from which <p m>'s data pointer attaches.
@xref
  <c mBuf_get> , <c mBuf_driverGetMbufChain>, <c mBuf_GetPkthdr>, <c mBuf_getm>
 */


extern int32 mBuf_freeOneMbufPkthdr(struct rtl_mBuf *m, void **buffer, uint32 *id, uint16 *size);
/*
@func struct int32	| mBuf_freeOneMbufPkthdr	| Free a single mbuf <p m> and associated pkthdr, if any. Returns information of cluster associated.
@parm struct rtl_mBuf * 	| m		|  Pointer to an mbuf chain.
@parm void * 	| buffer	|  placeholder for address of cluster associated on return.
@parm void * 	| id		|  placeholder of identifier of cluster associated on return.
@parm uint16 * 	| size	|  placeholder of size of cluster associated on return.
@rdesc Returns number of mbufs being freed
@rvalue <p n>		| 	number of mbufs being freed
@comm
 Free a single mbuf and associated pkthdr, if any. Place the successor, if any, in <p n>.
 Note that this function DOES NOT free the associated cluster. 

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	void *buffer ;
	uint32 size, id;
	n = mBuf_freeOne(m,&buffer,&id, &size);

@xref
  <c mBuf_freeMbufChain> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>, <c mBuf_freeOne>
 */


extern struct rtl_mBuf *mBuf_freeOne(struct rtl_mBuf *m);

/*
@func struct rtl_mBuf *	| mBuf_freeOne	| Free a single mbuf <p m> and associated cluster storage.
@parm struct rtl_mBuf * 	| m		|  Pointer to an mbuf chain.
@rdesc Returns address of next mbuf if any
@rvalue <p NULL>		| 	<p m> has been freed and there isn't any successing mbufs.
@rvalue <p n>		| 	The address of next mbuf after <p m>.
@comm
 Free a single mbuf and associated external storage. Place the successor, if any, in <p n>.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	n = mBuf_freeOne(m);

@xref
  <c mBuf_freeMbufChain> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>
 */

extern uint32 mBuf_freeMbufChain(register struct rtl_mBuf *m);

/*
@func struct rtl_mBuf *	| mBuf_freeMbufChain| Free the whole mbuf chain started from <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns number of mbufs being freed in mbuf chain <p m>.
@rvalue <p n>		| 	The number of mbufs being freed. If <p n> is 0, <p m> is not freed.
@comm
 Free the whole mbuf chain starting from <p m>.

@devnote  Return type changed from void to int32 to provide more information.
@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *m;
	int32 n;
	m = mBuf_get(MBUF_DONTWAIT, 0, 5);
	n = mBuf_freeMbufChain(m);
	if (n!=5){
		//Do error handling...
	}
@xref
  <c mBuf_freeOne> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>
 */

extern uint32 mBuf_driverFreeMbufChain(struct rtl_mBuf *pFirstMbuf);
/*
@func uint32	| mBuf_driverFreeMbufChain| Free the whole mbuf chain started from <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to the mbuf chain.
@rdesc Returns number of mbufs being freed in mbuf chain <p m>.
@rvalue <p n>		| 	The number of mbufs being freed. If <p n> is 0, <p m> is not freed.
@comm
 Free the whole mbuf chain starting from <p m>.  To be used only in driver.
@ex The following example demonstrates how to use this function |
	struct rtl_mBuf * localTempMbufHead,  * localTempMbufTail;
	int32 allocated;
	requested = 5;
	allocated= mBuf_driverGet(requested, &localTempMbufHead, &localTempMbufTail);
	if (allocated==requested){
		n = mBuf_driverFreeMbufChain(localTempMbufHead);
		if(n==allocated)
			printf("All mbufs successfully freed in driver\n");
	}
@xref
  <c mBuf_freeOne> , <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>, <c mBuf_driverFreePkthdr>
 */



extern void mBuf_freePkthdr(struct rtl_pktHdr *ph);
/*
@func struct rtl_pktHdr *	| mBuf_freePkthdr| Free a pkthdr alone.
@parm struct rtl_pktHdr *	|ph	| The pkthdr to be freed.
@rdesc No return value
@comm  Free a pkthdr alone. Callers should be aware that this is NOT the normal way to free a pkthdr with mbufs attached.
You should use <p mBuf_freeOne> or <p mBuf_freeMbufChain> to free pkthdrs attached with mbuf chains. This function is usd once only in TCP module.
@devnote
Caller of this function should remove the links between mbufs and pkthdrs on their own before <p mBuf_freePkthdr> is called.
@xref  <c mBuf_freeOne> , <c mBuf_freeMbufChain>, <c mBuf_driverFreeMbufChain>, <c mBuf_driverFreePkthdr>
 */

uint32 mBuf_driverFreePkthdr(struct rtl_pktHdr *ph, uint32 Npkthdr, struct rtl_pktHdr **ppHeadPkthdr);
/*
@func uint32	| mBuf_driverFreePkthdr| Driver specific. Free multiple pkthdrs without blocking
@parm struct rtl_pktHdr *	|ph	| The first pkthdr to be freed
@parm uint32	|Npkthdr	| Number of pkthdrs to be freed
@parm struct rtl_pktHdr **	|ppHeadPkthdr	| Returns next un-freed pkthdr, if any.
@rdesc Returns number of pkthdrs successfully freed.
@comm
This function is dedicated for driver.  <p ph> should not be NULL!!

 @xref
   <c mBuf_freeOne> , <c mBuf_freeMbufChain>, <c mBuf_driverFreeMbufChain>, <c mBuf_freePkthdr>
 */

extern struct rtl_mBuf *mBuf_adjHead(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_adjHead	| Remove <p req_len> bytes of data from head the mbuf chain pointed to by <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be removed
@rdesc m_adj returns the address of first mbuf after trimming data.
@rvalue <p NULL>	| 	Can't adjust length of mbuf. <p m> might 1)be NULL 2)have no clusters.
@rvalue <p m>		| 	When success, the first mbuf <p m> where user can read/write data is returned.
@comm
Remove <p req_len> bytes of data from the head of mbuf chain  <p m>. If user removed all
data bytes in the whole packet, <p m> is returned.

@devnote  If any clusters, after m_adj, is totally unused (ie. m_len=0),
the m_data pointer would be reset to m_extbuf and the mbuf would NOT be freed.

@xref
  <c mBuf_adjTail>,   <c mBuf_trimHead>,  <c mBuf_trimTail>
 */

extern struct rtl_mBuf *mBuf_adjTail(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_adjTail	| Remove <p req_len> bytes of data from tail of the mbuf chain pointed to by <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be removed
@rdesc m_adj returns the address of mbuf which contains the last data byte.
@rvalue <p NULL>	| 	Can't adjust length of mbuf. <p m> might 1)be NULL 2)have no clusters or <p req_len> is 0
@rvalue <p m>		| 	When success, the address of the mbuf which hold the last data byte is returned.
@comm
Remove <p req_len> bytes of data from the tail of mbuf chain  <p m>. If user removed all
data bytes in the whole packet, <p m> is returned.

@devnote  If any clusters, after m_adj, is totally unused (ie. m_len=0),
the m_data pointer would be reset to m_extbuf and the mbuf would NOT be freed.

@xref
  <c mBuf_adjHead>,   <c mBuf_trimHead>,  <c mBuf_trimTail>
 */

extern struct rtl_mBuf *mBuf_trimHead(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_trimHead	| Same as mBuf_adjHead, but also frees unused mbufs and clusters
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be trimmed from head.
@rdesc mBuf_trimHead returns the address of first mbuf after trimming.
@rvalue <p NULL>	| 	Can't adjust length of mbuf or the whole mbuf chain is trimmed and freed.
@rvalue <p m>		| 	When success, the first mbuf <p m> where user can read/write data is returned.
@comm
Same as mBuf_adjHead, but also frees unused mbufs and clusters

@devnote  mBuf_trimHead is implemented with mBuf_split and mBuf_freeMbufChain. It first splits the mbuf
to two mbuf chains from indicated position and frees the first one.
There is a possible risk that mBuf_trimHead may fail when the mbuf chain <p m> consumes all mbufs and no free mbufs
is available for mBuf_split to work correctly.

@xref
  <c mBuf_adjHead>,   <c mBuf_adjTail>,  <c mBuf_trimTail>
 */

extern struct rtl_mBuf *mBuf_trimTail(struct rtl_mBuf *, uint32 req_len);
/*
@func struct rtl_mBuf *	| mBuf_trimTail	| Same as mBuf_adjTail, but also frees unused mbufs and clusters
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain. <p m> doesn't need to be the first mbuf in chain.
@parm int32			| req_len	| Number of data bytes to be trimmed from tail.
@rdesc mBuf_trimTail returns the address <p m>.
@rvalue <p NULL>	| 	Can't adjust length of mbuf or the whole mbuf chain is trimmed and freed.
@rvalue <p m>		| 	When success, returns the first mbuf <p m> with data.
@comm
Same as mBuf_adjTail, but also frees unused mbufs and clusters

@devnote  mBuf_trimTail is implemented with mBuf_split and mBuf_freeMbufChain. It first splits the mbuf
to two mbuf chains from indicated position and frees the latter one.
There is a possible risk that mBuf_trimTail may fail when the mbuf chain <p m> consumes all mbufs and no free mbufs
is available for mBuf_split to work correctly.

@xref
  <c mBuf_adjHead>,   <c mBuf_adjTail>,  <c mBuf_trimTail>
*/



extern int32 mBuf_copyToMbuf(struct rtl_mBuf *, uint32 offset, uint32 len, int8 *cp);

/*
@func int32			| mBuf_copyToMbuf	| Copy <p len> bytes of data from user's buffer <p cp> to mbuf chain <p m>, started form offset <p offset>.
@parm struct rtl_mBuf * 	| m		| Address of the mbuf chain.
@parm uint32			|offset	| Starting byte to be copied in mbuf chain <p m>. Start from 0
@parm uint32			|len		| Number of bytes to be copied from <p cp>
@parm int8 * 		|cp		| Address of user's buffer.
@rdesc Returns number of bytes successfully copied
@rvalue <p -1>		| 	Failed.
@rvalue <p n>		| 	<p n> bytes have been copied from <p cp> into indicated mbuf chain <p m>
@comm
Copy <p len> bytes from user's buffer <p cp>, to the indicated mbuf chain
<p m> beginning from the <p offset>-th data byte in mbuf chain.
If there aren't at least 'offset' bytes in mbuf chain, -1 is returned.
mBuf_copyToMbuf() extends mbuf chain if neccessary.
@comm 
Be careful, when clusters are allocated externally by OS, mbuf module doesn't know which mbuf
is the first referee and owns the write priviledge to cluster. Therefore, write priviledge
is granted to ALL cluster referees.

@ex The following example demonstrates how to use this function |
	#define SIZE	100
	int32 i;
	int8 my_buffer[SIZE];
	for(i=0; i<SIZE; i++)
		my_buffer[i] = i;
	if (100!=mBuf_copyToMbuf(m, 10, 100, my_buffer))	//copy 100 bytes from my_buffer to mbuf m, started from the 10th bytes in mbuf
		printf("Can't copy all data!");
  @xref
  <c mBuf_copyToUserBuffer>
 */



extern int32 mBuf_copyToUserBuffer(struct rtl_mBuf *m, uint32 off, uint32 len, int8 * cp);

/*
@func int32			| mBuf_copyToUserBuffer	| Copy some data from an mbuf chain <p m> to user's buffer <p cp>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm uint32			|offset	| The number of starting data byte in mbuf chain <p m>. Start from 0.
@parm uint32			|len		| The number of data bytes to be copied. If <p len>=M_COPYALL, then copy all data till the end of mbuf chain.
@parm int8 *			|cp		| User specified buffer address.
@rdesc Returns number of bytes successfully received and copied to user's buffer
@rvalue <p -1>		| 	Failed.
@rvalue <p n>		| 	<p n> bytes have been copied from <p m> into indicated buffer <p cp> successfully.
@comm
Copy data from an mbuf chain starting from the <p offset>-th data byte,
continuing for <p len> bytes, into the indicated buffer <p cp>. User should be sure that there is enough free space in
the specified buffer <p cp>.

@ex The following example demonstrates how to use this function |
	int8 my_buffer[100];
	mBuf_copyToUserBuffer(m, 10, 100, my_buffer); //copy 100 bytes from the 10-th byte in mbuf to user's cp buffer.

  @xref
  <c mBuf_copyToMbuf>
 */

extern struct rtl_mBuf *mBuf_cloneMbufChain(struct rtl_mBuf *pThisMbuf, int32 iOffset,
							int32 iLength, int32 iWait);

/*
@func struct rtl_mBuf *	| mBuf_cloneMbufChain	| Clone a part of mbuf chain <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|offset	| The number of starting data byte in mbuf chain <p m>. Start from 0
@parm int32			|len		| The number of data bytes to be cloned. If <p len>=M_COPYALL, then clone all data till the end of mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT> and/or <p M_ALLOCPKTHDR>
@rdesc Returns address of the new, cloned mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't clone mbuf chain <p m>.
@rvalue <p n>		| 	The address of cloned mbuf chain <p n>. Data in <p n> is read only.
@comm
Make a clone of an mbuf chain starting from <p offset>-th byte from the beginning,
continuing for <p len> bytes.  If <p len> is M_COPYALL, then clone to end of the whole mbuf chain. (You may choose to use the optimized mBuf_clonePacket() in this case)
The <p how> parameter is a choice of MBUF_WAIT or MBUF_DONTWAIT and/or M_ALLOCPKTHDR by the caller.
If M_ALLOCPKTHDR flag is given, a new pkthdr would be allocated for duplicated packet. 

Note that the clone is read-only, The new mbuf chain <p n> only shares cluster data with <p m>.
<p n> can only read cluster data, but not write. <p m> still owns write priviledge.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_cloneMbufChain(m, 20, M_COPYALL, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_clonePacket>, <c mBuf_dupPacket>, <c mBuf_cloneMbufChain>
 */

extern struct rtl_mBuf *mBuf_dupMbufChain(struct rtl_mBuf *pMbufChain, int32 iOffset, int32 iLength,  int32 flag);

/*
@func struct rtl_mBuf *	| mBuf_dupMbufChain	| Duplicate a part of mbuf chain <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|offset	| The number of starting data byte in mbuf chain <p m>. Start from 0
@parm int32			|len		| The number of data bytes to be cloned. If <p len>=M_COPYALL, then duplicate all data till the end of mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT> and/or <p M_ALLOCPKTHDR>
@rdesc Returns address of the new, duplicated mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't duplicate mbuf chain <p m>.
@rvalue <p n>		| 	The address of duplicated mbuf chain <p n>. Data in <p n> is writable.
@comm
Duplicate an mbuf chain starting from <p offset>-th byte from the beginning,
continuing for <p len> bytes.  If <p len> is M_COPYALL, then duplicate to end of the whole mbuf chain. (You may choose to use the optimized mBuf_dupPacket() in this case)
The <p how> parameter is a choice of MBUF_WAIT or MBUF_DONTWAIT and/or M_ALLOCPKTHDR by the caller.
If M_ALLOCPKTHDR flag is given, a new pkthdr would be allocated for duplicated packet. 

Note that the duplicated mbuf is writable..

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_dupMbufChain(m, 20, M_COPYALL, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_clonePacket>, <c mBuf_dupPacket>, <c mBuf_cloneMbufChain>
 */



extern struct rtl_mBuf *mBuf_clonePacket(struct rtl_mBuf *pMbuf, int32 iHow);

/*
@func struct rtl_mBuf *	| mBuf_clonePacket	| Clone the entire packet <p m>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>
@rdesc Returns address of the new mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't clone mbuf chain <p m>.
@rvalue <p n>		| 	The address of copied mbuf chain <p n>. Data in <p n> is read only.
@comm
Clone an entire mbuf chain <p m>, including the packet header (which must be present).
An optimization of the common case `mBuf_cloneMbufChain(m, 0, M_COPYALL, how)'.
The new cloned packet always allocates a new pkthdr.
Note that the copy is read-only, because clusters are not copied, only their reference counts are incremented.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	n = mBuf_clonePacket(m, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_dupPacket>, <c mBuf_cloneMbufChain>, <c mBuf_dupMbufChain>
 */

extern struct rtl_mBuf *mBuf_dupPacket(struct rtl_mBuf *pMbuf, int32 iHow);

/*
@func struct rtl_mBuf *	| mBuf_dupPacket	| Duplicate an mbuf chain <p m>, including its data,  into a completely new chain <p n>
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>
@rdesc Returns address of the new mbuf chain <p n>.
@rvalue <p NULL>		| 	Can't duplicate mbuf chain <p m>.
@rvalue <p n>		| 	The address of copied mbuf chain <p n>. Data in <p n> is writable.
@comm
Duplicate an mbuf chain <p m> into a completely new chain <p n>, including
copying data in any <p m>'s mbuf clusters. 
The new duplicated packet always allocates a new pkthdr.
Use this instead of mBuf_clonePacket() when you need a writable copy of an mbuf chain.

@ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;
	n = mBuf_dupPacket(m, MBUF_WAIT);
	if (n != NULL) {
		// do following processing
	}
  @xref
  <c mBuf_clonePacket> , <c mBuf_copyToUserBuffer>, <c mBuf_cloneMbufChain>
 */





extern struct rtl_mBuf *mBuf_prepend(struct rtl_mBuf *m, uint32 plen, int32 how);

/*
@func struct rtl_mBuf *	| mBuf_prepend	| Arrange to prepend space of size <p plen> to mbuf <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm uint32			|plen	| Number of bytes to be prepended before <p m>
@parm int32			|how	| <p MBUF_WAIT>, <p MBUF_DONTWAIT>, <p M_GETNEWMBUF>
@rdesc Returns address of the first mbuf.
@rvalue <p NULL>		| 	Can't allocate memory or parameter problem.
@rvalue <p n>		| 	The address of first mbuf after prepending.
@comm
Arrange to prepend space of size <p plen> to mbuf <p m>.
 If new mbufs must be allocated, <p how> specifies whether to wait or not. When user calls mBuf_prepend, the
 data length recorded in mbufs and pkthdr is incremented accordingly.

By default, mBuf_prepend uses any leading free space before allocating any new clusters.
But if <p M_GETNEWMBUF> is set, mBuf_prepend allocates new mbufs directly and leaves any leading free buffer space as is.

If <p how> has MBUF_DONTWAIT bit set, and we can't get all buffers immediately, return NULL.
If <p how> has MBUF_WAITOK bit set, and we can't get all buffers right away, block waiting until our request is satisfied.

M_BUFONLY can't be used with mBuf_prepend.

@ex The following example demonstrates how to use this function |
	m0 = mBuf_prepend(m0, PPP_HDRLEN, MBUF_DONTWAIT | M_GETNEWMBUF);
	if (m0 == 0) {
	    error = ENOBUFS;
	    goto bad;
	}
  @xref <c mBuf_cat>, <c mBuf_padding>

 */

extern struct rtl_mBuf *mBuf_padding(struct rtl_mBuf *m, uint32 plen, int32 how);
/*
@func struct rtl_mBuf *	| mBuf_padding	| Arrange to append space of size <p plen> to mbuf <p m>.
@parm struct rtl_mBuf * 	| m		| Pointer to an mbuf chain.
@parm uint32			|plen	| Number of bytes to append at <p m>
@parm int32			|how	| <p MBUF_WAIT> or <p MBUF_DONTWAIT>
@rdesc Returns address of the last mbuf padded.
@rvalue <p NULL>		| 	Can't allocate memory or parameter problem.
@rvalue <p n>		| 	The address of last mbuf after padding
@comm
Arrange to append space of size <p plen> to mbuf <p m>.
 If new mbufs must be allocated, <p how> specifies whether to wait or not. When user calls mBuf_padding, the
 data length recorded in mbufs and pkthdr are incremented accordingly.

 M_BUFONLY can't be used with mBuf_padding.
@devnote  API CHANGED:  If <p how> is MBUF_DONTWAIT and allocation fails, NULL is returned.
Original mbuf <p m> is intact.

@ex The following example demonstrates how to use this function |
	m0 = mBuf_padding(m, (ETHER_MIN_LEN-ETHER_CRC_LEN-len), MBUF_DONTWAIT);
	if (m0 == 0) {
	    error = ENOBUFS;
	    goto bad;
	}
@xref <c mBuf_split>, <c mBuf_cat>, <c mBuf_getm>, <c mBuf_prepend>, <c mBuf_split>


 */


extern struct rtl_mBuf *mBuf_cat(struct rtl_mBuf *m, struct rtl_mBuf *n);

/*
@func struct rtl_mBuf *	| mBuf_cat	| Concatenate mbuf chain <p n> to <p m>.
@parm struct rtl_mBuf * 	| m		| mbuf chain to be appended.
@parm struct rtl_mBuf * 	| n		| mbuf chain to be appended after <p m>
@rdesc Returns the address of <p m> or NULL if failed.
@rvalue <p NULL>		| 	Can't concatenate two mbuf chain
@rvalue <p m>		| 	When success, the address of <p m> is returned..
@comm
Concatenate mbuf chain <p n> to <p m>.
Total packet length of <p m> would be adjusted accordingly. However, <p n>'s control header, if any, would be freed
@xref   <c mBuf_split>, <c mBuf_padding>, <p mBuf_getm>

 */


extern int32 mBuf_pullup(struct rtl_mBuf *, int32);

/*
@func int32		| mBuf_pullup| 	Rearange an mbuf chain so that <p len> bytes are contiguous
 								 in <p m>'s first cluster.
@parm struct rtl_mBuf * 	| m	| Pointer to an mbuf chain.
@parm 	int32			|len	| The number of databytes requested to put in <p m>'s first cluster. <p len> should not exceed the size of a cluster buffer (256 bytes).
@rdesc	Returns total data bytes in the first mbuf cluster.
@rvalue <p n>		| 	The number of data bytes in <p m>'s first cluster. If <p n> is less than <p len>,
						that means <p mBuf_pullup> can't pull up all <p len> bytes requested because there isn't
						so much data in mbuf chain.
@comm  Original BSD4.4 note: Rearange an mbuf chain so that <p len> bytes are contiguous and in <p m>'s cluster.
Successing mbufs are adjusted accordingly. If the return value <p n> is smaller than requested <p len>,
maybe there isn't enough data bytes in the mbuf chain <p m>, or maybe the requested value <p len> exceeds
the maximum capacity for a single cluster.

Our implementation note:
(cfliu 2002.02.19)This function is important in traditional BSD networking stack because
original mbuf can't do dtom() if data is saved in cluster. This is not the
case here since we offer a back pointer from cluster to mbuf via cltag_mbuf field.
However, this function is still useful if we want to collect continous databytes from later clusters
to the specified mbuf's cluster. The maximum number of  <p len> should be less than a cluster
can hold (ie. len less than or equal to m_clusterSIze)

(2002.08.11) If 1) requested length <p len> is smaller than m_clusterSize, and 2) there are enough data in this mbuf chain. However, the
trailing space in <p m> is not large enough, <p mBuf_pullup> will move m's data upward first and then copy requested data from latter clusters

@devnote  This function is different from traditional BSD mbuf code. When pull up fails, the mbuf chain will not be freed,
and the return value has been changed to an integer instead of a pointer to mbuf structure.

@ex The following example demonstrates how to use this function |
	int32 n;

	n = mBuf_pullup(m, 128);
	if (n < 128) {
		//There isn't so much data in mbuf chain.
	}
  @xref
  <c m_pulldown>
 */


struct rtl_mBuf *mBuf_split(register struct rtl_mBuf *m0, uint32 len0, int32 wait);

/*@func struct rtl_mBuf * | mBuf_split | Partition an mbuf chain in two pieces. Data is cloned
  @parm struct rtl_mBuf * 	| m    	| Pointer to an mbuf chain
  @parm 	int32			| len    	| The number of last data byte to remain in original mbuf chain <p m>
  @parm  int32 			| wait   	| <p MBUF_DONTWAIT> or <p MBUF_WAIT>
  @rdesc	Returns a pointer to the splited mbuf chain <p n>
  @rvalue NULL		| Can't split the mbuf chain
  @rvalue <p n>		| Success. <p n> is the address of second mbuf chain. It holds all data bytes after the <p len>-th byte in <p m>.
  @comm  Partition an mbuf chain <p m> into two pieces, returning the tail mbuf chain <p n>. In case of failure, it returns NULL and
 attempts to restore the chain to its original state. Data in clusters  are NOT copied, but cloned only.

  @ex The following example demonstrates how to use this function |
	struct rtl_mBuf *n;

	n = mBuf_split(m, off, MBUF_DONTWAIT);
	if (n == NULL) {
		goto bad;
	}
  @xref
  <c mBuf_cat>, <c mBuf_padding>, <c mBuf_getm>

 */
void mBuf_getBMjmpTable(uint8 *pat,  uint16 *jump_tbl,uint16 patLen, uint8 caseSensitive);
int32 mBuf_BMpatternMatch(struct rtl_mBuf *m, uint32 len, uint8 *delimiter, uint32 delimitLen, uint16 *jmp_tbl, uint8 caseSensitive);

#define MBUF_GETLEN(m)	 ((m)? ((m)->m_len : -1)
/*
@func MACRO	| MBUF_GETLEN	| Get total number of data bytes in the mbuf <p m>
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns data length of the mbuf
@rvalue -1		| 	Failed.
@rvalue <p length>| 	The actual length of data in <p m>
@comm
 Get total number of data bytes in the mbuf <p m>
@xref  <c mBuf_getPktlen>
*/


 struct rtl_mBuf *mBuf_attachHeader(void *buffer, uint32 id, uint32 bufsize,uint32 datalen, uint16 align);
struct rtl_mBuf *mBuf_attachHeaderJumbo(void *buffer, uint32 id, uint32 bufsize,uint32 datalen);

int32 mBuf_setNICRxRingSize(uint32 size);
extern int32 mBuf_reserve(struct rtl_mBuf * m, uint16 headroom);


#define MBUF_SET_PKTHDRFLAGS(m, Flags) do{\
	assert((m));\
	assert(ISSET((m)->m_flags, MBUF_USED));\
	assert((m)->m_pkthdr);\
	MBUF_SETPKTHDRFIELD16(((memaddr *)&(m)->m_pkthdr->ph_flags), (Flags));\
 }while(0)


/*
@func MACRO	| MBUF_SET_PKTHDRFLAGS	| Set packet specific flags for ASIC processing
@parm struct rtl_mBuf *	|m		| Indicate the packet
@parm uint32	|Flags		| flag to be set
@rdesc None
@comm
Set packet specific flags for ASIC processing. This overwrites original flag value. If you are adding flag bits rather than reseting it,
remember to save its old value first

@xref  <c MBUF_GET_PKTHDRFLAGS>
*/


#define MBUF_GET_PKTHDRFLAGS(m)	(MBUF_CHECKPKTHDR((m))? MBUF_GETPKTHDRFIELD16((memaddr *)(&m->m_pkthdr->ph_flags)):-1)
/*
@func MACRO	| MBUF_GET_PKTHDRFLAGS	| Get packet specific flags set by ASIC
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns the result of execution
@rvalue -1		| 	Failed. <p m> might have no pkthdrs.
@rvalue FLAGS	| 	Returns the flags.
@comm
Get packet specific flags set by ASIC

@xref  <c MBUF_SET_PKTHDRFLAGS>
*/

#define MBUF_SET_PORTLIST(m,Portlist)  do{\
	assert((m));\
	assert(ISSET((m)->m_flags, MBUF_USED));\
	assert((m)->m_pkthdr);\
	(m)->m_pkthdr->ph_portlist = (Portlist);\
}while(0)

/*
@func MACRO	| MBUF_SET_PORTLIST	| Set outgoing port list
@parm struct rtl_mBuf *	|m		| Indicate the packet
@parm uint32	|Portlist		| port list
@rdesc None
@comm  Set outgoing port list
@xref  <c MBUF_GET_PORTLIST>
*/

#define MBUF_GET_PORTLIST(m) (MBUF_CHECKPKTHDR((m))? ((m)->m_pkthdr->ph_portlist):-1)
/*
@func MACRO	| MBUF_GET_PORTLIST	| get incoming port list
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns the result of execution
@rvalue 0		| 	Failed. <p m> might have no pkthdrs.
@rvalue Portlist	| 	Returns incoming portlist.
@comm
 get incoming port list

@xref  <c MBUF_SET_PORTLIST>
*/


#define MBUF_SET_TRAPREASON(m, Reason)	do{\
	assert((m));\
	assert(ISSET((m)->m_flags, MBUF_USED));\
	assert((m)->m_pkthdr);\
	MBUF_SETPKTHDRFIELD16(((memaddr *)&(m)->m_pkthdr->ph_reason), (Reason));\
}while(0)

/*
@func MACRO	| MBUF_SET_TRAPREASON	| Set packet trapping reason for ASIC processing
@parm struct rtl_mBuf *	|m		| Indicate the packet
@parm uint16	|Reason		| Trapping reason
@rdesc None
@comm
Set packet trapping reason for ASIC processing

@xref  <c MBUF_GET_TRAPREASON>
*/

#define MBUF_GET_TRAPREASON(m)	(MBUF_CHECKPKTHDR((m))? MBUF_GETPKTHDRFIELD16((memaddr *)(&m->m_pkthdr->ph_reason)):-1)
/*
@func MACRO	| MBUF_GET_TRAPREASON	| Get packet trapping reason set by ASIC
@parm struct rtl_mBuf *	|m		| Indicate the packet
@rdesc Returns the result of execution
@rvalue -1		| 	Failed. <p m> might have no pkthdrs.
@rvalue Reason	| 	Returns trap reason
@comm
Get packet trapping reason set by ASIC

@xref  <c MBUF_SET_TRAPREASON>
*/

#define	MBUF2DATAPTR(m, t)	((t)((m)->m_data))
/*
@func MACRO	| MBUF2DATAPTR	| Given mbuf pointer 'm', returns address of m->m_data, and convert it to type 't'
@parm struct rtl_mBuf *	|m		| Pointer to an mbuf
@parm TYPE	|t		| A type to be casted
@rdesc Address of m->m_data
@comm
Given mbuf pointer 'm', returns address of m->m_data, and convert it to type 't'
@xref  <c mBuf_data2Mbuf>
*/


#define	MBUF_ALIGN(m, len) do {	\
	assert((m)->m_len==0);\
	assert(len>0);\
	assert(((m)->m_extsize - (len))>0);\
	(m)->m_data += ( (m)->m_extsize - (len)) & ~(sizeof(memaddr) - 1);	\
} while (0)

/*
@func MACRO	| MBUF_ALIGN	| Align the m->m_data pointer from end of the cluster, for 'len' bytes.
@parm struct rtl_mBuf *	|m		| Pointer to an mbuf
@parm uint32	|len		| Size to be aligned.
@rdesc None
@comm
Set the m_data pointer of a NEWLY-allocated cluster  to place
an object of the specified size at the end of the mbuf, longword aligned.
@xref  <c MBUF_RESERVE>
*/


#define	MBUF_RESERVE(m, len) do {	\
	assert(len>0);\
	(m)->m_data += ( ((len) > (m)->m_extsize)? (m)->m_extsize : len);	\
} while (0)
/*
@func MACRO	| MBUF_RESERVE	| Reserve 'len' bytes from the beginning of the newly allocated cluster
@parm struct rtl_mBuf *	|m		| Pointer to an mbuf
@parm uint32	|len		| Size to be aligned.
@rdesc None
@comm
Forward move the m_data pointer of a NEWLY-allocated cluster  for 'len' bytes.
m_data won't move beyond m_extbuf + m_extsize, make sure 'len' you give is a reasonable value
@xref  <c MBUF_ALIGN>
*/
#endif			   /* !_MBUF_H_ */

