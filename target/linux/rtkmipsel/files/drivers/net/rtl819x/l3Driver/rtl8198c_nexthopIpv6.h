/*
* Copyright c                  Realtek Semiconductor Corporation, 2013  
* All rights reserved.
* 
* Program : ipv6 nexthop table driver
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/
#ifndef	RTL8198C_NEXTHOP_IPV6_H
#define	RTL8198C_NEXTHOP_IPV6_H

typedef struct rtl8198c_ipv6_nextHopEntry_s 
{
	rtl865x_netif_local_t *dstNetif;	/* dst network interface*/
	union {
		inv6_addr_t	nexthop;
		uint32		pppSessionId;
	} un; 	/* ethernet: nexthop ip address, pppoe: session Id */

	uint16 valid:1,		/* 0: Invalid, 1: Invalid */	
		nextHopType:3,			/* IF_ETHER, IF_PPPOE */
		flag:3;			/* bit0:referenced by l3, bit2:referenced by acl */

	uint16	refCnt;			/* Reference Count */
	uint32	entryIndex;			/* Entry Index */
	
} rtl8198c_ipv6_nextHopEntry_t;

#define IPV6_NEXTHOP_L3					0x01
#define IPV6_NEXTHOP_DEFREDIRECT_ACL	0x02

#define IPV6_NXTHOP_ENTRY_NUM	32

int32 rtl8198c_initIpv6NxtHopTable(void);
int32 rtl8198c_reinitIpv6NxtHopTable(void);
int32 rtl8198c_addIpv6NxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, inv6_addr_t nexthop);
int32 rtl8198c_delIpv6NxtHop(uint32 attr, uint32 entryIdx);

#endif
