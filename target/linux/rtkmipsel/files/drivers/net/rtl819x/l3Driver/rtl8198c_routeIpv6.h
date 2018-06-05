/*
* Copyright c                  Realtek Semiconductor Corporation, 2013  
* All rights reserved.
* 
* Program : ipv6 route table driver
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/
#ifndef	RTL198C_ROUTE_IPV6_H
#define	RTL198C_ROUTE_IPV6_H
#include <net/rtl/rtl8198c_route_ipv6_api.h>

typedef struct rtl198c_ipv6_route_s
{
	struct rtl198c_ipv6_route_s *next;
	inv6_addr_t 	ipAddr;						/* Destination IP Address */
	//ipaddr_t	ipMask;						/* Network mask */
	int fc_dst_len;								/*prefix len*/
	inv6_addr_t 	nextHop;					/* next hop IP address */
	//ipaddr_t	srcIp;						/* source IP address,only for multiple wan now*/
	uint32	valid:1, 		
			process:4,					/* 000: PPPoE, 001: L2, 010: ARP, 100: CPU, 101: NextHop, 110: Drop*/
			asicIdx:4;
				
	uint32            ref_count;                       /*referrence count*/
	rtl865x_netif_local_t *dstNetif;			/*destination network interface*/

	struct
	{
		uint32 subnetIdx;
	} arp;
	
	/* nexthop informaiton */
	union {
		struct 
		{
			void *macInfo;	/*direct nexthop's mac information*/
		}direct;

		struct
		{
			void *macInfo; /*pppoe server's mac information*/
			rtl865x_ppp_t *pppInfo;			
		}pppoe;
		
		struct
		{
			
			uint32 nxtHopSta;	/* pointer to Nexthop table: starting range */
			uint32 nxtHopEnd;		/* pointer to Nexthop table: ending range */				
			uint8 nhalog;							/* algo. for load balance */
			//uint8 ipDomain;						/* IP domain */
		} nxthop;
		
	} un;

	uint32 sixRd_eg;
 	uint32 sixRd_idx;
} rtl8198c_ipv6_route_t;
#define IPV6_RT_DEFAULT_RT_NEXTHOP_CPU 		0x00
#define IPV6_RT_DEFAULT_RT_NEXTHOP_NORMAL 	0x01

/* process: */
#define IPV6_RT_PPPOE				0x00
#define IPV6_RT_L2					0x01
#define IPV6_RT_ARP					0x02
#define IPV6_RT_CPU					0x04
#define IPV6_RT_NEXTHOP				0x05
#define IPV6_RT_DROP					0x06

/* nhalog: */
#define IPV6_RT_ALOG_PACKET			0x00
#define IPV6_RT_ALOG_SESSION		0x01
#define IPV6_RT_ALOG_SIP				0x02

int32 rtl8198c_initIpv6RouteTable(void);
int32 rtl8198c_reinitIpv6RouteTable(void);
int32 rtl8198c_getIpv6RouteEntryByIp(inv6_addr_t dst, rtl8198c_ipv6_route_t *rt);

#endif


