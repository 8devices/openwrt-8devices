/*
* Copyright c                  Realtek Semiconductor Corporation, 2013  
* All rights reserved.
* 
* Program : ipv6 route table driver
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/
/*      @doc RTL_LAYEREDDRV_API

        @module rtl8198c_routeIpv6.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl8198c prefix
        are external functions.
        @normal Jia Wenjian (wenjain_jai@realsil.com.cn) <date>

        Copyright <cp>2013 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include <net/rtl/rtl865x_ppp.h>
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl_errno.h"
//#include "rtl_utils.h"
#include "common/rtl865x_netif_local.h"
#include "rtl865x_ppp_local.h"
#include "rtl8198c_routeIpv6.h"
//#include "rtl865x_ip.h"
#include "rtl8198c_nexthopIpv6.h"
#include "rtl8198c_arpIpv6.h"

//#include "common/rtl_glue.h"
#include "common/rtl865x_eventMgr.h" /*call back function....*/
#include "common/rtl865x_vlan.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif
#include "l2Driver/rtl865x_fdb.h"
#include "AsicDriver/rtl865xc_asicregs.h"
#include <net/rtl/rtl865x_fdb_api.h>
#include <net/ipv6.h>
#include <linux/in6.h>
#include <net/addrconf.h>

#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
static rtl8198c_ipv6_route_t rtl8198c_ipv6_route_inused_back[IPV6_RT_DRV_ENTRY_NUM];
#endif

static rtl8198c_ipv6_route_t *rtl8198c_ipv6_route_freeHead;
static rtl8198c_ipv6_route_t *rtl8198c_ipv6_route_inusedHead;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
extern ipaddr_t br_ip_addr;
extern int32 _rtl865x_addIpv66RDEntryByBR(char *devname);
extern int32 _rtl865x_getIpv66RDEntryByName(char * devName);
extern int32 _rtl865x_delIpv66RDEntry(uint32 entryIdx);
#endif
#endif

static rtl8198c_ipv6_route_t* _rtl8198c_getDefaultIpv6Route(void)
{
	rtl8198c_ipv6_route_t *rt = NULL;	
	rt = rtl8198c_ipv6_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1) && (rt->asicIdx==(IPV6_RT_ASIC_ENTRY_NUM -1)))
			return rt;
		
		rt = rt->next;
	}

	return NULL;
}

static rtl8198c_ipv6_route_t* _rtl8198c_getIpv6RouteEntry(inv6_addr_t ipAddr, int fc_len)
{
	rtl8198c_ipv6_route_t *rt = NULL;	
	rt = rtl8198c_ipv6_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1) && (rt->fc_dst_len==fc_len) && (ipv6_prefix_equal((struct in6_addr *)(&rt->ipAddr), (struct in6_addr *)(&ipAddr), rt->fc_dst_len)))
			return rt;
		
		rt = rt->next;
	}	
	
	return NULL;
}

//get globle route count by interface name.
//For pppoe, only one globle route should be added to hw route table, and add more will fail
static int32 _rtl8198c_getIpv6GloRouteCountByIfname(char* ifname,int fc_len)
{
	rtl8198c_ipv6_route_t *rt = NULL;
	int32 count=0;
	
	if(!ifname) return 0;
	rt = rtl8198c_ipv6_route_inusedHead;
	//printk("%s:%d ifName=%s count=%d\n",__FUNCTION__,__LINE__,ifname,count);

	while(rt)
	{
		if((rt->valid==1) 
			&& (fc_len==0 || rt->fc_dst_len==fc_len)
			&&((rt->ipAddr.v6_addr32[0]>>16)!=0xfe80 && (rt->ipAddr.v6_addr32[0]>>16)!=0x0)
			&&(rt->dstNetif->name && (strcmp(rt->dstNetif->name,ifname)==0))
			)
			count++;
		
		rt = rt->next;
	}
	
	//printk("%s:%d ifName=%s count=%d\n",__FUNCTION__,__LINE__,ifname,count);
	return count;
}

static int32 _rtl8198c_synIpv6RouteToAsic(rtl8198c_ipv6_route_t *rt_t)
{
	int32 ret = FAILED;
	rtl8198C_tblAsicDrv_routingv6Param_t asic_t;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif = NULL;
	uint32 columIdx, fid;
	int32 pppIdx = 0;

	bzero(&asic_t, sizeof(rtl8198C_tblAsicDrv_routingv6Param_t));

	if (rt_t == NULL) {
		printk("%s(%d):NULL!!!!!!!!!!!!!!!!\n",__FUNCTION__,__LINE__);
		return FAILED;
	}
	/*common information*/

	memcpy(&asic_t.ipAddr, &rt_t->ipAddr, sizeof(inv6_addr_t));
	asic_t.ipMask = rt_t->fc_dst_len;

	/*if the dstNetif is attach on another interface, the netifIdx should the master interface's index*/
	if (rt_t->dstNetif->is_slave == 1) {
		//printk("========%s(%d), ip(0x%x),mask(0x%x),netif(%s)\n",__FUNCTION__,__LINE__,rt_t->ipAddr,rt_t->ipMask,rt_t->dstNetif->name);
		dstNetif = _rtl865x_getNetifByName(rt_t->dstNetif->name);
		if (dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	} else {
		dstNetif = rt_t->dstNetif;
	}

	if (dstNetif == NULL) {
		printk("%s(%d) BUG!!!!!!\n",__FUNCTION__,__LINE__);
		return FAILED;
	}
	
	asic_t.vidx = dstNetif->asicIdx;
	asic_t.process = rt_t->process;

	switch (rt_t->process)
	{
		case IPV6_RT_PPPOE:
			
			ret = rtl865x_getPppIdx(rt_t->un.pppoe.pppInfo, &pppIdx);
			asic_t.pppoeIdx = pppIdx;
			/*
			*if process==RT_PPPOE, the mac address of pppoe server is add in pppoe module,
			*so, we read the FDB information directly....
			*/
			ret = rtl865x_getVlanFilterDatabaseId(rt_t->dstNetif->vid, &fid);		
			ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt_t->un.pppoe.macInfo, FDB_DYNAMIC, &columIdx, &asic_l2);

			if (ret != SUCCESS)
				printk("Ipv6 can't get l2 entry by mac.....\n");
			
			/*update mac/fdb table reference count*/
			asic_t.nextHopRow = rtl8651_filterDbIndex(rt_t->un.pppoe.macInfo, fid);
			asic_t.nextHopColumn = columIdx;
			
			break;

		case IPV6_RT_L2:
			/*
			* NOTE:this type not used now...
			* if we want to use it, please add FDB entry to sure this L2 entry in both software FDB table and Asic L2 table.
			*/
			ret = rtl865x_getVlanFilterDatabaseId(rt_t->dstNetif->vid, &fid);		
			ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt_t->un.direct.macInfo, FDB_STATIC, &columIdx,&asic_l2);
			if(ret != SUCCESS)
				printk("Ipv6 can't get l2 entry by mac.....\n");

			/*update mac/fdb table reference count*/
			asic_t.nextHopRow = rtl8651_filterDbIndex(rt_t->un.direct.macInfo, fid);
			asic_t.nextHopColumn = columIdx;

			break;

		case IPV6_RT_ARP:
			asic_t.subnet_idx = rt_t->arp.subnetIdx;
			break;
			
		case IPV6_RT_CPU:
		case IPV6_RT_DROP:
			/*do nothing*/
			break;
			
		case IPV6_RT_NEXTHOP:
	 		asic_t.nhStart		 = rt_t->un.nxthop.nxtHopSta;
			asic_t.nhNum	 	 = rt_t->un.nxthop.nxtHopEnd - rt_t->un.nxthop.nxtHopSta + 1;
			asic_t.nhNxt	 	 = asic_t.nhStart;
			asic_t.nhAlgo	 	 = rt_t->un.nxthop.nhalog;	
			break;
			
		default:
			printk("Ipv6 Process_Type(%d) is not support!\n",rt_t->process);
	}

	if (rt_t->asicIdx > IPV6_RT_ASIC_ENTRY_NUM-1) {
		printk("BUG!! %s(%d)....", __FUNCTION__,__LINE__);
		return FAILED;
	}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	asic_t.six_rd_eg = rt_t->sixRd_eg; 
	asic_t.six_rd_idx = rt_t->sixRd_idx;
#endif
#endif

	ret = rtl8198C_setAsicRoutingv6(rt_t->asicIdx, &asic_t);

	return ret;
	
}

static int32 _rtl8198c_updateDefaultIpv6Route(rtl8198c_ipv6_route_t *rt, int32 action)
{
	int32 i;
	inv6_addr_t tmp_nexthop;
	rtl8198c_ipv6_route_t *entry;
	int32 retval = FAILED;

	entry = rt;
	if(entry == NULL)
		return RTL_EINVALIDINPUT;
	
	/*delete nexthop which is add by default route*/
	if(rt->process == IPV6_RT_NEXTHOP) {
		for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd && entry->un.nxthop.nxtHopEnd != 0; i++)
		{
			retval = rtl8198c_delIpv6NxtHop(IPV6_NEXTHOP_L3, i);
		}
	}

	entry->un.nxthop.nxtHopSta = 0;
	entry->un.nxthop.nxtHopEnd = 0;
	switch(action)
	{
		case IPV6_RT_DEFAULT_RT_NEXTHOP_CPU:
			memset(&tmp_nexthop, 0, sizeof(inv6_addr_t));
			retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)entry, entry->dstNetif, tmp_nexthop);
			break;

		case IPV6_RT_DEFAULT_RT_NEXTHOP_NORMAL:
			{
				rt->process = IPV6_RT_NEXTHOP;	
				switch(rt->dstNetif->if_type)
				{
					case IF_ETHER:
						retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, rt->dstNetif, rt->nextHop);
						break;
					case IF_PPPOE:
						{
							rtl865x_ppp_t *pppoe;
							memset(&tmp_nexthop, 0, sizeof(inv6_addr_t));
							
							pppoe = rtl865x_getPppByNetifName(rt->dstNetif->name);
							if (pppoe != NULL) {
								/*got pppoe session*/
								tmp_nexthop.v6_addr32[0] = pppoe->sessionId;
								retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, rt->dstNetif, tmp_nexthop);
							} else {
								/*nexthop's action is to CPU*/
								retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, rt->dstNetif, tmp_nexthop);
							}
						}
					
						break;
				}
			}
			break;
	}
	
	retval = _rtl8198c_synIpv6RouteToAsic(entry);

	return retval;
}

static int32 _rtl8198c_arrangeIpv6Route(rtl8198c_ipv6_route_t *start_rt, int32 start_idx)
{
	int32 count;
	rtl8198c_ipv6_route_t *rt = NULL;

	rt = start_rt;
	count = 0;
	while(rt)
	{		
		if (rt->valid) {
			/*if the rule is default route...*/
			if(rt->fc_dst_len == 0) {
				rt->asicIdx = IPV6_RT_ASIC_ENTRY_NUM-1;
			} else {
				/* entry number more than asic table's capacity*/
				/* entry index=RT_ASIC_ENTRY_NUM-1 is reserved for default route*/
				if((start_idx + count > IPV6_RT_ASIC_ENTRY_NUM-2))
					break;
				
				/*delete old asic entry firstly...*/
				if(start_idx+count < rt->asicIdx && rt->asicIdx < IPV6_RT_ASIC_ENTRY_NUM-1)
					rtl8198C_delAsicRoutingv6(rt->asicIdx);
				
				rt->asicIdx = start_idx+count;
				/*jwj: arp subnetIdx need to keep the same with rt->asicIdx.*/
				if (rt->process == IPV6_RT_ARP) {
					if (rt->arp.subnetIdx != rt->asicIdx) {
						/*rt->asicIdx change, so its arp->subnetIdx need to change
						also with the new rt->asicIdx.*/
						if (rt->arp.subnetIdx != 0xf)	/*The new add router no need call rtl8198c_changeIpv6ArpSubnetIdx*/
							rtl8198c_changeIpv6ArpSubnetIdx(rt->arp.subnetIdx, rt->asicIdx);
						rt->arp.subnetIdx = rt->asicIdx;	
					}
				}
				
				_rtl8198c_synIpv6RouteToAsic(rt);
			}			
		}

		/*next entry*/
		rt= rt->next;
		count++;
	}

	
	/*more route entry need to add?*/
	if (rt) {
		/*not enough asic table entry! have to update default route's action TOCPU*/
		rt = _rtl8198c_getDefaultIpv6Route();
		_rtl8198c_updateDefaultIpv6Route(rt, IPV6_RT_DEFAULT_RT_NEXTHOP_CPU);		
	} else {
		rt = _rtl8198c_getDefaultIpv6Route();
		_rtl8198c_updateDefaultIpv6Route(rt, IPV6_RT_DEFAULT_RT_NEXTHOP_NORMAL);
	}
	
	return SUCCESS;
}

static int32 _rtl8198c_addIpv6RouteToInusedList(rtl8198c_ipv6_route_t *rt)
{
	int32 retval = FAILED;
	int32 start_idx = 0;
	rtl8198c_ipv6_route_t *entry,*fore_rt,*start_rt;

	fore_rt = NULL;
	entry = rtl8198c_ipv6_route_inusedHead;

	/*always set 0x0f when init..., this value would be reset in arrange route*/
	rt->asicIdx = 0x0f;
	rt->next = NULL;
		
	/*find the right position...*/
	while(entry)
	{
		if (entry->valid == 1) {
			if (entry->fc_dst_len < rt->fc_dst_len) {
				break;
			}
		}
		fore_rt = entry;
		entry = entry->next;
	}

	/*insert this rule after insert_entry*/
	if (fore_rt) {
		rt->next = fore_rt->next;
		fore_rt->next = rt;
		start_idx = fore_rt->asicIdx+1;
		start_rt = rt;
	} else {
		/*insert head...*/
		rt->next = rtl8198c_ipv6_route_inusedHead;		
		rtl8198c_ipv6_route_inusedHead = rt;
		
		start_idx = 0;
		start_rt = rtl8198c_ipv6_route_inusedHead;
	}	

	retval = _rtl8198c_arrangeIpv6Route(start_rt, start_idx);
	
	return retval;
}

static int32 _rtl8198c_delIpv6RouteFromInusedList(rtl8198c_ipv6_route_t * rt)
{
	int32 retval, start_idx;
	rtl8198c_ipv6_route_t *fore_rt = NULL, *entry = NULL, *start_rt = NULL;

	entry = rtl8198c_ipv6_route_inusedHead;
	while(entry)
	{
		if(entry == rt)
			break;

		fore_rt = entry;		
		entry = entry->next;
	}

	/*fore_rt == NULL means delete list head*/
	if (fore_rt == NULL) {
		rtl8198c_ipv6_route_inusedHead = rtl8198c_ipv6_route_inusedHead->next;
		start_rt = rtl8198c_ipv6_route_inusedHead;
		start_idx = 0;
	} else {
		fore_rt->next = rt->next;
		start_rt = fore_rt->next;
		start_idx = fore_rt->asicIdx + 1;
	}

	/*delete route from asic*/
	if(rt->asicIdx < IPV6_RT_ASIC_ENTRY_NUM)
		rtl8198C_delAsicRoutingv6(rt->asicIdx);
	
	retval = _rtl8198c_arrangeIpv6Route(start_rt, start_idx);
	rt->asicIdx = 0x0f;
		
	return retval;	
	
}


static int32 _rtl8198c_usedNetifInIpv6Route(int8 *ifname)
{
	rtl8198c_ipv6_route_t *rt = NULL;
	rt = rtl8198c_ipv6_route_inusedHead;

	while(rt)
	{	
		if (memcmp(rt->dstNetif->name, ifname, strlen(ifname)) == 0)
			return SUCCESS;
		
		rt = rt->next;
	}	
	
	return FAILED;
}
#if 1
static int32 _rtl8198c_Ipv6ToCpu(rtl8198c_ipv6_route_t *rt, 	rtl865x_netif_local_t *netif)
{
		inv6_addr_t tmp_nexthop;
		int32 retval = FAILED;
		/*link local ipv6 route entry always to cpu*/
		rt->process = IPV6_RT_NEXTHOP;
		memset(&rt->nextHop, 0, sizeof(inv6_addr_t));
		memset(&tmp_nexthop, 0, sizeof(inv6_addr_t));
		retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, netif, tmp_nexthop);
		if (retval != SUCCESS) {
			return retval;
		}		
			/*update reference....*/
		rtl865x_referNetif(netif->name);
		rt->valid		= 1;
		rt->ref_count	= 1;
		retval = _rtl8198c_addIpv6RouteToInusedList(rt);
		
		if (retval != SUCCESS) {
			rtl865x_deReferNetif(netif->name);
		}	
		return retval;
	
}

#endif

#if 0
static void print_ipv6_addr(void *addr,int prefix,const char* function,int line)
{
	inv6_addr_t ipAddr=*((inv6_addr_t *)addr);
	printk("%s:%d ipAddr(0x%8x%8x%8x%8x),prefix len(%d)\n",function,line,
		ipAddr.v6_addr32[0],ipAddr.v6_addr32[1],ipAddr.v6_addr32[2],ipAddr.v6_addr32[3],prefix);
	return;
}
#endif
static void htonl_array_hwv6(u32 *org_buf, u32 *dst_buf, unsigned int words)
{
	int i = 0;
	
	if (!org_buf || !dst_buf)
		return;
	while (words--) {
		dst_buf[i] = htonl(org_buf[i]);
		i++;
	}
	
	return;
}

static rtl8198c_ipv6_route_t * _rtl8198c_getNewRouteEntry(inv6_addr_t ipAddr,int fc_dst_len,inv6_addr_t nextHop,rtl865x_netif_local_t *netif)
{
	rtl8198c_ipv6_route_t *rt=NULL;
	/*allocate a new buffer for adding entry*/
	rt = rtl8198c_ipv6_route_freeHead;

	if (rt) {
		rtl8198c_ipv6_route_freeHead = rt->next;
	}else{
		return NULL;
	}

	/*common information*/
	ipv6_addr_prefix((struct in6_addr *)(&rt->ipAddr), (struct in6_addr *)(&ipAddr), fc_dst_len);
	rt->fc_dst_len = fc_dst_len;
	memcpy(&rt->nextHop, &nextHop, sizeof(inv6_addr_t));
	rt->dstNetif 	= netif;
	
	return rt;
	
}
static int32 _rtl8198c_addIpv6Route(inv6_addr_t ipAddr, int fc_dst_len, inv6_addr_t nextHop, int8 * ifName)
{
	inv6_addr_t tmp_nexthop;
	rtl865x_netif_local_t *netif = NULL;
	rtl8198c_ipv6_route_t *rt = NULL;
	int32 retval = FAILED;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	int32 sixrdIndex = FAILED;
#endif
#endif
	
	/*para check*/
	if (ifName == NULL)
		netif = _rtl865x_getDefaultWanNetif();
	else	
		netif = _rtl865x_getSWNetifByName(ifName);
	
	if (netif == NULL)
		return RTL_EINVALIDINPUT;
	
	if (netif->if_type == IF_NONE)
		return RTL_ENOLLTYPESPECIFY;
#if 0
	printk("%s.%d.ipAddr(0x%8x%8x%8x%8x),fc_dst_len(%d)nextHop(0x%8x%8x%8x%8x),ifName(%s)\n",
		__FUNCTION__,__LINE__,
		ipAddr.v6_addr32[0],ipAddr.v6_addr32[1],ipAddr.v6_addr32[2],ipAddr.v6_addr32[3],
		fc_dst_len,
		nextHop.v6_addr32[0],nextHop.v6_addr32[1],nextHop.v6_addr32[2],nextHop.v6_addr32[3],
		ifName);
#endif
	/*
	*duplicate entry check:
	*	in Driver system, default route is always exist.
	*	so if ipMask == 0, it's means that default route should be update...
	*/
	if (fc_dst_len != 0 && (rt = _rtl8198c_getIpv6RouteEntry(ipAddr, fc_dst_len)) != NULL) {
		//rt->ref_count++;
		return RTL_EENTRYALREADYEXIST;
	}

	/*add default route: just update the default route becase the default route always exist!*/
#ifdef CONFIG_RTL_IPV6READYLOGO
	if ((fc_dst_len==0) || !((nextHop.v6_addr32[0]==0)&&(nextHop.v6_addr32[1]==0)&&(nextHop.v6_addr32[2]==0)&&(nextHop.v6_addr32[3]==0))) 
#else
	if (fc_dst_len==0)
#endif
	{
		rt = _rtl8198c_getDefaultIpv6Route();
		/*deference rt's orginal netif*/
		if (rt && rt->dstNetif)
			rtl865x_deReferNetif(rt->dstNetif->name);
	}

	/*allocate a new buffer for adding entry*/
	if (rt == NULL) {
		rt=_rtl8198c_getNewRouteEntry(ipAddr,fc_dst_len,nextHop,netif);
	}
	
	if (rt == NULL) {
		/*no buffer, default route should be TOCPU?*/
		return RTL_ENOFREEBUFFER;
	}

		

#if 1
	if((ipAddr.v6_addr32[0]>>16)==0xFE80 )
	{
	//	printk("%s:%d ifName=%s ipAddr.v6_addr16[0]=%x\n",__FUNCTION__,__LINE__,ifName,(ipAddr.v6_addr32[0]>>16));
		
		retval=_rtl8198c_Ipv6ToCpu(rt,netif);
		if (retval != SUCCESS) {
			goto addFailed;	
		}		
		return retval;
	}

	//when pppoe add its ipv6 route to cpu
	
	if(ifName && (strncmp(ifName,"ppp",3)==0)   //ppp0
		&&((ipAddr.v6_addr32[0]>>16)!=0x0 && (ipAddr.v6_addr32[0]>>16)!=0xFE80)) //global ip
		{
			struct net_device *dev = NULL;
			struct inet6_dev *idev;
			struct inet6_ifaddr *ifa;
			struct in6_addr ifaAddr;

			
			
			
			dev = __dev_get_by_name(&init_net, ifName);
			idev= __in6_dev_get(dev);
			//printk("%s:%d ifName=%s\n",__FUNCTION__,__LINE__,ifName);

			if (idev != NULL) {
				list_for_each_entry(ifa, &idev->addr_list, if_list) {
					
					htonl_array_hwv6((u32 *)&ifa->addr.s6_addr32[0], (u32 *)&ifaAddr.s6_addr32[0], 4);
					//print_ipv6_addr(&ifaAddr,ifa->prefix_len,__FUNCTION__,__LINE__);
					if(ipv6_prefix_equal(&ifaAddr,(struct in6_addr *)&rt->ipAddr,rt->fc_dst_len)){
						rtl8198c_ipv6_route_t *rt_pppGlo=NULL;
						rt_pppGlo = _rtl8198c_getNewRouteEntry(*((inv6_addr_t*)(&ifaAddr)),128,nextHop,netif);
						if (rt_pppGlo == NULL) {
							/*no buffer, default route should be TOCPU?*/
							return RTL_ENOFREEBUFFER;
						}			
						//print_ipv6_addr(&rt_pppGlo->ipAddr,rt_pppGlo->fc_dst_len,__FUNCTION__,__LINE__);
						
						if(_rtl8198c_getIpv6GloRouteCountByIfname(ifName,128)!=0){
							goto addFailed;
						}
						//printk("%s:%d ifName=%s\n",__FUNCTION__,__LINE__,ifName);

						retval=_rtl8198c_Ipv6ToCpu(rt_pppGlo,netif);
						if (retval != SUCCESS) {
							goto addFailed;	
						}
					}
					
					//printk("%s:%d ifName=%s\n",__FUNCTION__,__LINE__,ifName);
				}
				
				//printk("%s:%d ifName=%s\n",__FUNCTION__,__LINE__,ifName);
			}					
		}
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
		if(((ipAddr.v6_addr32[0]==0) && (ipAddr.v6_addr32[1]==0) && 
			(ipAddr.v6_addr32[2]==0) && (ipAddr.v6_addr32[3]==0)) 
			 &&
			((nextHop.v6_addr32[0]!=0) || (nextHop.v6_addr32[1]!=0) ||
			 (nextHop.v6_addr32[2]!=0) || (nextHop.v6_addr32[3]!=0))
		)
		{
			extern inv6_addr_t gw_ipv6_addr;
			memcpy(&gw_ipv6_addr,&nextHop,sizeof(inv6_addr_t));
		}
#endif		
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	if(netif->if_type == IF_6RD)
	{	
		if(((ipAddr.v6_addr32[0]==0) && (ipAddr.v6_addr32[1]==0) && 
			(ipAddr.v6_addr32[2]==0) && (ipAddr.v6_addr32[3]==0)) 
			&&
			((nextHop.v6_addr32[0]==0) && (nextHop.v6_addr32[1]==0) &&
			(nextHop.v6_addr32[2]==0) && (nextHop.v6_addr32[3]!=0))
			)
		{
			br_ip_addr = nextHop.v6_addr32[3];
			_rtl865x_addIpv66RDEntryByBR(ifName);
		}
	}
#endif
#endif

	/*don't specify the nexthop ip address, it's means that:
	* all packet match this route entry should be forward by network interface with arp
	*/
	
	if (((nextHop.v6_addr32[0]==0)&&(nextHop.v6_addr32[1]==0)&&(nextHop.v6_addr32[2]==0)&&(nextHop.v6_addr32[3]==0)) && (fc_dst_len != 0)) {
		
		switch(netif->if_type)
		{
			case IF_ETHER:
				//printk("%s,%d.add arp info to asic ...........\n",__FUNCTION__,__LINE__);
				rt->process = IPV6_RT_ARP;
				/*rt->arp.subnetIdx will be updated as rt->asicIdx later in _rtl865x_arrangeRoute*/ 
				break;

			case IF_PPPOE:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = IPV6_RT_PPPOE;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if (ppp == NULL) {
						printk("Ipv6 error!!can't get pppoe session information by interface name(%s)\n",netif->name);
						goto addFailed;
					}

					rt->un.pppoe.macInfo = &ppp->server_mac;
					rt->un.pppoe.pppInfo = ppp;
					
					/*update reference...*/
					rtl865x_referPpp(ppp->sessionId);
				}
				break;
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = IPV6_RT_L2;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if (ppp == NULL) {
						/*printk("Warning!!CAn't get pptp/l2tp session information by interface name(%s)\n",netif->name);*/
						goto addFailed;
					}

					rt->un.direct.macInfo = &ppp->server_mac;
				}
				break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
			case IF_6RD:
				//do nothing
				return SUCCESS;
#endif
#endif
			default:
				printk("Ipv6 lltype(%d) is not support now....\n", netif->if_type);
				goto addFailed;
		}
		
	}
	else {
		/*if default is valid, delete nexthop firstly...*/
		if ((rt->valid==1) && (rt->process==IPV6_RT_NEXTHOP))  {
                        int i;  
                        for ( i = rt->un.nxthop.nxtHopSta; i <= rt->un.nxthop.nxtHopEnd; i++)
                        {       
                                retval = rtl8198c_delIpv6NxtHop(IPV6_NEXTHOP_L3, i);
                        }       
              }

#ifdef CONFIG_RTL_IPV6READYLOGO
		if (strncmp(netif->name, RTL_DRV_LAN_NETIF_NAME, sizeof(RTL_DRV_LAN_NETIF_NAME))==0)
		{
			rt->process = IPV6_RT_NEXTHOP;
			memset(&rt->nextHop, 0, sizeof(inv6_addr_t));
			 _rtl8198c_updateDefaultIpv6Route(rt, IPV6_RT_DEFAULT_RT_NEXTHOP_CPU);
		}
		else
#endif
		{
		
		/*use nexthop type*/
		rt->process = IPV6_RT_NEXTHOP;
		//for restore nexthop table, when hw l3v6 cleared.
		memcpy(&rt->nextHop, &nextHop, sizeof(inv6_addr_t));
		switch(netif->if_type)
		{
			case IF_ETHER:
				retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, netif, nextHop);
				break;
			case IF_PPPOE:
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *pppoe;
					memset(&tmp_nexthop, 0, sizeof(inv6_addr_t));
					
					pppoe = rtl865x_getPppByNetifName(netif->name);

					if (pppoe != NULL) {
						/*got pppoe session*/
						tmp_nexthop.v6_addr32[0] = pppoe->sessionId;
						retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, netif, tmp_nexthop);
					}
					else {
						/*nexthop's action is to CPU*/
						retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, netif, tmp_nexthop);
					}
				}
				break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
			case IF_6RD:
				retval = rtl8198c_addIpv6NxtHop(IPV6_NEXTHOP_L3, (void*)rt, netif, nextHop);
				break;
#endif
#endif

			default:
				retval = FAILED;
				break;				
		}
		
		if (retval != SUCCESS) {
			printk("error!!add ipv6 nexthop error! retval (%d)\n",retval);
			goto addFailed;	
		}		
		rt->un.nxthop.nhalog = IPV6_RT_ALOG_SIP; /* use per-source IP */
	}
	}

	rt->valid		= 1;
	rt->ref_count	= 1;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	if(netif->if_type == IF_6RD)
	{
		sixrdIndex=_rtl865x_getIpv66RDEntryByName(netif->name);

		/*hook default gw for 6rd interface */
		if(((ipAddr.v6_addr32[0]==0) && (ipAddr.v6_addr32[1]==0) && 
			(ipAddr.v6_addr32[2]==0) && (ipAddr.v6_addr32[3]==0)) 
			&& fc_dst_len == 0 &&
			((nextHop.v6_addr32[0]==0) && (nextHop.v6_addr32[1]==0) &&
			(nextHop.v6_addr32[2]==0) && (nextHop.v6_addr32[3]!=0)))
		{		
			if(sixrdIndex == FAILED)
			{
				rt->sixRd_eg = 0;
				rt->sixRd_idx = 0;			
			}
			else
			{
				rt->sixRd_eg	= 1;
				rt->sixRd_idx 	= sixrdIndex;
			}
		}
	}
#endif
#endif
	
	/*update reference....*/
	rtl865x_referNetif(netif->name);
	if (fc_dst_len == 0)
		_rtl865x_setDefaultWanNetif(netif->name);

	if (rt->asicIdx == IPV6_RT_ASIC_ENTRY_NUM-1) {
		retval = _rtl8198c_synIpv6RouteToAsic(rt);
	}
	else {		
		/*insert the adding route to inused list*/	
		retval = _rtl8198c_addIpv6RouteToInusedList(rt);		
	}

	/*if route is add, please enable Routing for the releated netif*/
	retval = rtl865x_enableNetifRoutingV6(netif);
	return retval;
	
addFailed:
	if (rt->asicIdx == IPV6_RT_ASIC_ENTRY_NUM -1) {
#ifdef CONFIG_RTL_IPV6READYLOGO
		rt->process = IPV6_RT_NEXTHOP;
#endif
		_rtl8198c_updateDefaultIpv6Route(rt, IPV6_RT_DEFAULT_RT_NEXTHOP_CPU);
	}
	else {
		/*free this route entry and return error code...*/	
		memset(rt, 0, sizeof(rtl8198c_ipv6_route_t));
		rt->arp.subnetIdx = 0xf;
		rt->next = rtl8198c_ipv6_route_freeHead;
		rtl8198c_ipv6_route_freeHead = rt;
	}
	return retval;
}

static int32 _rtl8198c_delIpv6Route(inv6_addr_t ipAddr, int fc_dst_len )
{
	rtl8198c_ipv6_route_t *entry;
	int32 i;
	int32 retval = 0;

	entry = _rtl8198c_getIpv6RouteEntry(ipAddr, fc_dst_len);

	if (entry == NULL)
		return RTL_EENTRYNOTFOUND;

	if (entry->asicIdx == (IPV6_RT_ASIC_ENTRY_NUM-1)) {
		/*if default route
		* 1. reset default route
		* 2. reset entry->netif...
		*/
		rtl865x_netif_local_t *netif = NULL;
		_rtl865x_clearDefaultWanNetif(entry->dstNetif->name);

		netif = _rtl865x_getDefaultWanNetif();
		if (netif == NULL) {
			return RTL_EINVNETIFNAME;
		}
		
		if (netif != entry->dstNetif) {
			rtl865x_deReferNetif(entry->dstNetif->name);
			entry->dstNetif = netif;
			rtl865x_referNetif(netif->name);
		}
		
		retval = _rtl8198c_updateDefaultIpv6Route(entry, IPV6_RT_DEFAULT_RT_NEXTHOP_CPU);
	} else {
		/*not default route*/
		switch(entry->process)
		{
			case IPV6_RT_PPPOE:
				{
					rtl865x_ppp_t *ppp = entry->un.pppoe.pppInfo;
					if (ppp)
						rtl865x_deReferPpp(ppp->sessionId);
				}
				break;
			case IPV6_RT_L2:
				/*
				* NOTE:this type not used now...
				* if we want to use it, please DELETE FDB entry to sure this L2 entry is deleted both software FDB table and Asic L2 table.
				*/
				break;
			case IPV6_RT_ARP:
				/*free arp*/
				retval = rtl8198c_delIpv6ArpBySubnetIdx(entry->arp.subnetIdx);
				if ( retval != SUCCESS) {
					printk("======Ipv6 error!!can't FREE arp entry for this route entry....retval(%d)\n",retval);					
				}
				break;				

			case IPV6_RT_CPU:
			case IPV6_RT_DROP:
				/*do nothing*/
				
				break;

			case IPV6_RT_NEXTHOP:
				/*delete nexthop which is add by l3*/
				for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd; i++)
				{
					retval = rtl8198c_delIpv6NxtHop(IPV6_NEXTHOP_L3, i);
				}				
				break;
		}

		/*update netif reference count*/
		rtl865x_deReferNetif(entry->dstNetif->name);
		
		/*remove from inused list...*/		
		_rtl8198c_delIpv6RouteFromInusedList(entry);

		if(_rtl8198c_usedNetifInIpv6Route(entry->dstNetif->name) == FAILED)
			rtl865x_disableNetifRoutingV6(entry->dstNetif);

		/*add to free list*/
		memset(entry, 0, sizeof(rtl8198c_ipv6_route_t));
		entry->arp.subnetIdx = 0xf;
		entry->next = rtl8198c_ipv6_route_freeHead;
		rtl8198c_ipv6_route_freeHead = entry;

		retval = SUCCESS;		
	}	
	
	return retval;
	
}

rtl8198c_ipv6_route_t* _rtl8198c_getIpv6RouteEntryByIp(inv6_addr_t dst)
{
	rtl8198c_ipv6_route_t *tmpRtEntry = NULL;
	rtl8198c_ipv6_route_t *rt=rtl8198c_ipv6_route_inusedHead;
	int fc_len;

	fc_len = 0;
	while(rt)
	{
		if (rt->valid == 1 && (ipv6_prefix_equal((struct in6_addr *)(&rt->ipAddr), (struct in6_addr *)(&dst), rt->fc_dst_len)) && fc_len <= rt->fc_dst_len) {
			fc_len = rt->fc_dst_len;
			tmpRtEntry = rt;
		}
		rt = rt->next;
	}
	return tmpRtEntry;
}

/*
@func int32 | rtl8198c_addIpv6Route |add a route entry.
@parm inv6_addr_t | ipAddr | ip address.
@parm int | fc_dst_len | ip prefix len.
@parm inv6_addr_t | nextHop | the route's next hop.
@parm int8* | ifName | destination network interface. 
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EINVALIDINPUT | invalid input.
@rvalue RTL_ENOLLTYPESPECIFY | network interface's link type is not specified.
@rvalue RTL_EENTRYALREADYEXIST | route entry is already exist.
@rvalue RTL_ENOFREEBUFFER | not enough memory in system.
@comm
	if ifName=NULL, it means the destionation network interface of route entry with ip/ipMask/nextHop is default wan.
*/
int32 rtl8198c_addIpv6Route(inv6_addr_t ipAddr, int fc_dst_len, inv6_addr_t nextHop, int8 * ifName)
{
	int32 retval = 0;
	unsigned long flags=0;	
	//printk("========%s(%d), ip(0x%x%x%x%x),fc_dst_len(%d),ifname(%s),nxthop(0x%x)\n",__FUNCTION__,__LINE__,
		//ipAddr.s6_addr32[0], ipAddr.s6_addr32[1],ipAddr.s6_addr32[2],ipAddr.s6_addr32[3],fc_dst_len,ifName,nextHop);
	SMP_LOCK_ETH(flags);
	retval = _rtl8198c_addIpv6Route(ipAddr,fc_dst_len,nextHop,ifName);		
	SMP_UNLOCK_ETH(flags);	
	//printk("========%s(%d), ip(0x%x%x%x%x),fc_dst_len(%d),ifname(%s),nxthop(0x%x),retval(%d)\n",__FUNCTION__,__LINE__,
		//ipAddr.s6_addr32[0], ipAddr.s6_addr32[1],ipAddr.s6_addr32[2],ipAddr.s6_addr32[3],,fc_dst_len,ifName,nextHop,retval);
	return retval;
}
/*
@func int32 | rtl8198c_delIpv6Route |delete a route entry.
@parm inv6_addr_t | ipAddr | ipAddress.
@parm int | fc_dst_len | ip prefix len.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EENTRYNOTFOUND | not found the entry.
@comm	
*/
int32 rtl8198c_delIpv6Route(inv6_addr_t ipAddr, int fc_dst_len)
{

	int32 retval = 0;
	unsigned long flags=0;	
	//printk("========%s(%d), ip(0x%x%x%x%x),fc_dst_len(%d)\n",__FUNCTION__,__LINE__,ipAddr.s6_addr32[0],
		//ipAddr.s6_addr32[1],ipAddr.s6_addr32[2],ipAddr.s6_addr32[3],fc_dst_len);
	SMP_LOCK_ETH(flags);
	retval = _rtl8198c_delIpv6Route(ipAddr,fc_dst_len);
	SMP_UNLOCK_ETH(flags);
	//printk("==================================retval(%d)\n",retval);
	return retval;

}

/*
@func int32 | rtl8198c_getIpv6RouteEntryByIp |according the destination ip address, get the matched route entry.
@parm inv6_addr_t | dst | destionation ip address.
@parm rtl8198c_ipv6_route_t* | rt | retrun value: route entry pointer
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl8198c_getIpv6RouteEntryByIp(inv6_addr_t dst, rtl8198c_ipv6_route_t *rt)
{
	int32 retval = FAILED;
	rtl8198c_ipv6_route_t *ret_entry = NULL;

	ret_entry = _rtl8198c_getIpv6RouteEntryByIp(dst);
	if (ret_entry&&rt) {
		memcpy(rt,ret_entry,sizeof(rtl8198c_ipv6_route_t));		
		retval = SUCCESS;
	}
	
	return retval;
}

/*
@func int32 | rtl198c_initIpv6RouteTable |initialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl8198c_initIpv6RouteTable(void)
{
	int32 i;
	rtl8198c_ipv6_route_t *rt;
	rtl8198c_ipv6_route_freeHead = NULL;
	rtl8198c_ipv6_route_inusedHead = NULL;

	/*malloc buffer*/
	TBL_MEM_ALLOC(rt, rtl8198c_ipv6_route_t, IPV6_RT_DRV_ENTRY_NUM);	
	memset(rt, 0, sizeof(rtl8198c_ipv6_route_t)*IPV6_RT_DRV_ENTRY_NUM);
	for(i = 0; i < IPV6_RT_DRV_ENTRY_NUM; i++)
	{
		rt[i].arp.subnetIdx = 0xf;
		rt[i].next = rtl8198c_ipv6_route_freeHead;
		rtl8198c_ipv6_route_freeHead = &rt[i];		
	}	

	#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
	memset((void *)&rtl8198c_ipv6_route_inused_back, 0, sizeof(rtl8198c_ipv6_route_inused_back));
	#endif
	
	return SUCCESS;	
}

/*
@func int32 | rtl198c_reinitIpv6RouteTable |reinitialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl8198c_reinitIpv6RouteTable(void)
{
	rtl8198c_ipv6_route_t *rt;

	rt = rtl8198c_ipv6_route_inusedHead;
	while(rt && rt->asicIdx != IPV6_RT_ASIC_ENTRY_NUM -1)
	{
		_rtl8198c_delIpv6Route(rt->ipAddr,rt->fc_dst_len);
		rt = rtl8198c_ipv6_route_inusedHead;
	}

	/*delete the last route*/
	rt = rtl8198c_ipv6_route_inusedHead;
	if(rt)
	{
		/*update netif reference count*/
		rtl865x_deReferNetif(rt->dstNetif->name);
		
		/*remove from inused list...*/		
		_rtl8198c_delIpv6RouteFromInusedList(rt);

		/*add to free list*/
		memset(rt, 0, sizeof(rtl8198c_ipv6_route_t));
		rt->arp.subnetIdx = 0xf;
		rt->next = rtl8198c_ipv6_route_freeHead;
		rtl8198c_ipv6_route_freeHead = rt;
	}
	return SUCCESS;
}

#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
int rtl819x_save_inused_ipv6_route(void)
{
	int i = 0;
	rtl8198c_ipv6_route_t *rt = rtl8198c_ipv6_route_inusedHead;

	memset(rtl8198c_ipv6_route_inused_back, 0, sizeof(rtl8198c_ipv6_route_t)*IPV6_RT_DRV_ENTRY_NUM);
	while(rt)
	{
		if (rt->valid == 1) {
			memcpy(&rtl8198c_ipv6_route_inused_back[i], rt, sizeof(rtl8198c_ipv6_route_t));
			i++;
			if (i >= IPV6_RT_DRV_ENTRY_NUM)
				break;
		}
		rt = rt->next;
	}

	return SUCCESS;
}

int rtl819x_restore_inused_ipv6_route(void)
{
	int i = 0;
	int ret = 0;
	rtl8198c_ipv6_route_t *rt = NULL;

	for (i=0; i<IPV6_RT_DRV_ENTRY_NUM; i++)
	{
		if (rtl8198c_ipv6_route_inused_back[i].valid == 1) {
			rt = &rtl8198c_ipv6_route_inused_back[i];
			ret = rtl8198c_addIpv6Route(rt->ipAddr, rt->fc_dst_len, rt->nextHop, rt->dstNetif->name);
			#if 0
			panic_printk("%s %d ret=%d ip=0x%x:%x:%x:%x rt->fc_dst_len=%d nexthop=0x%x:%x:%x:%x rt->dstNetif->name=%s \n", __FUNCTION__, __LINE__, ret, rt->ipAddr.v6_addr32[0], 
				rt->ipAddr.v6_addr32[1], rt->ipAddr.v6_addr32[2], rt->ipAddr.v6_addr32[3], rt->fc_dst_len, 
				rt->nextHop.v6_addr32[0], rt->nextHop.v6_addr32[1], rt->nextHop.v6_addr32[2], rt->nextHop.v6_addr32[3], rt->dstNetif->name);
			#endif
		}
	}

	return SUCCESS;
}	
#endif


