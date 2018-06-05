/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : nexthop table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#ifndef	RTL865X_NEXTHOP_H
#define	RTL865X_NEXTHOP_H

#if 0//!defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define REDUCE_MEMORY_SIZE_FOR_16M
#endif


typedef struct rtl865x_nextHopEntry_s 
{
	rtl865x_netif_local_t *dstNetif;	/* dst network interface*/
	uint32	nexthop;			/* ethernet: nexthop ip address, pppoe: session Id */
	//rtl865x_ip_entry_t *srcIp_t;		/* for nexthop source ip table index, it's invalid  now*/
	uint32 srcIp;		/* for nexthop source ip table index*/

	uint16 valid:1,		/* 0: Invalid, 1: Invalid */	
		nextHopType:3,			/* IF_ETHER, IF_PPPOE */
		flag:3;			/* bit0:referenced by l3, bit2:referenced by acl */

	uint16	refCnt;			/* Reference Count */
	uint32	entryIndex;			/* Entry Index */

} rtl865x_nextHopEntry_t;

#define NEXTHOP_L3					0x01
#define NEXTHOP_DEFREDIRECT_ACL	0x02
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#define NXTHOP_ENTRY_NUM	16
#else
#if defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define NXTHOP_ENTRY_NUM	4
#else
#define NXTHOP_ENTRY_NUM	16
#endif
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
#undef NXTHOP_ENTRY_NUM
#if defined(CONFIG_RTL_8198C) ||defined(CONFIG_RTL_8197F)
#define NXTHOP_ENTRY_NUM	32
#else
#define NXTHOP_ENTRY_NUM	16
#endif
#endif

int32 rtl865x_initNxtHopTable(void);
int32 rtl865x_reinitNxtHopTable(void);
int32 rtl865x_addNxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, uint32 nexthop,uint32 srcIp);
int32 rtl865x_delNxtHop(uint32 attr, uint32 entryIdx);

#if defined(CONFIG_RTL_LOCAL_PUBLIC) || defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)  ||defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int32 rtl865x_getNxtHopIdx(uint32 attr, rtl865x_netif_local_t *netif, uint32 nexthop);
#endif

#if defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
int32 rtl865x_eventHandle_addArp_for_multiWan(void *param);
int32 rtl865x_eventHandle_delArp_for_multiWan(void *param);
#endif

#endif

