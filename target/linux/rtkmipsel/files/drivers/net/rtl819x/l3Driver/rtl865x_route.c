/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : route table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_route.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

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
#include "rtl865x_route.h"
#include "rtl865x_ip.h"
#include "rtl865x_nexthop.h"
#include "rtl865x_arp.h"

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
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#include "rtl8198c_arpIpv6.h"
#include "rtl8198c_nexthopIpv6.h"
#endif

static rtl865x_route_t *rtl865x_route_freeHead;
static rtl865x_route_t *rtl865x_route_inusedHead;
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static RTL_DECLARE_MUTEX(route_sem);
#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
extern int32  _rtl865x_getIpv6DsLiteEntryByName(char * devName);
extern int32 _rtl865x_delIpv6DsLiteEntry(uint32 entryIdx);
#endif
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT)//&&defined(REINIT_SWITCH_CORE)
static rtl865x_route_t rtl865x_route_inused_bak[RT_DRV_ENTRY_NUM];
#endif
#if 0
void _rtl865x_route_print(void)
{
	int32 i;
	rtl865x_route_t *rt = NULL;
	rt = rtl865x_route_inusedHead;

	printk("=============inused list:\n");
	while(rt)
	{		
		printk("asicIdx(%d),ip(0x%x),mask(0x%x),nextHop(0x%x),process(0x%x),dst(%s) valid(%d)\n",rt->asicIdx, rt->ipAddr,rt->ipMask,rt->nextHop,rt->process,rt->dstNetif->name,rt->valid);
		rt = rt->next;
	}

	i = 0;
	rt = rtl865x_route_freeHead;
	while(rt)
	{
		i++;
		rt = rt->next;
	}
	
	printk("=======free list count(%d)\n",i);
}
#endif


static rtl865x_route_t* _rtl865x_getDefaultRoute(void)
{
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1)&&(rt->asicIdx == RT_ASIC_ENTRY_NUM -1))
			return rt;
		rt = rt->next;
	}

	return NULL;
}

static rtl865x_route_t* _rtl865x_getRouteEntry(ipaddr_t ipAddr, ipaddr_t ipMask)
{
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{
		if((rt->valid==1)&&((rt->ipAddr&rt->ipMask)==(ipAddr&ipMask))&&(rt->ipMask==ipMask))
			return rt;
		rt = rt->next;
	}	
	return NULL;
}


static int32 _rtl865x_synRouteToAsic(rtl865x_route_t *rt_t)
{
	int32 ret = FAILED;
	rtl865x_tblAsicDrv_routingParam_t asic_t;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif = NULL;
	uint32 columIdx,fid=0;
	int32 pppIdx = 0;

	bzero(&asic_t, sizeof(rtl865x_tblAsicDrv_routingParam_t));

	if(rt_t == NULL)
	{
		printk("%s(%d):NULL!!!!!!!!!!!!!!!!\n",__FUNCTION__,__LINE__);
		return FAILED;
	}
	/*common information*/
	asic_t.ipAddr	= rt_t->ipAddr;
	asic_t.ipMask	= rt_t->ipMask;
	asic_t.ipAddr = rt_t->ipAddr;
	asic_t.ipMask = rt_t->ipMask;
	/*if the dstNetif is attach on another interface, the netifIdx should the master interface's index*/
	if(rt_t->dstNetif->is_slave == 1)
	{
		//printk("========%s(%d), ip(0x%x),mask(0x%x),netif(%s)\n",__FUNCTION__,__LINE__,rt_t->ipAddr,rt_t->ipMask,rt_t->dstNetif->name);
		dstNetif = _rtl865x_getNetifByName(rt_t->dstNetif->name);
		if(dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	}
	else
		dstNetif = rt_t->dstNetif;

	if(dstNetif == NULL){
		printk("%s(%d) BUG!!!!!!\n",__FUNCTION__,__LINE__);
		return FAILED;
	}
	
	asic_t.vidx = dstNetif->asicIdx;
	asic_t.internal = rt_t->dstNetif->is_wan? 0 : 1;
	asic_t.DMZFlag = rt_t->dstNetif->dmz? 1 : 0;
	asic_t.process = rt_t->process;
	
	switch (rt_t->process)
	{
	case RT_PPPOE:
		ret = rtl865x_getPppIdx(rt_t->un.pppoe.pppInfo, &pppIdx);
		
		asic_t.pppoeIdx = pppIdx;
		/*
		*if process==RT_PPPOE, the mac address of pppoe server is add in pppoe module,
		*so, we read the FDB information directly....
		*/
		ret = rtl865x_getVlanFilterDatabaseId(rt_t->dstNetif->vid,&fid);		
		ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt_t->un.pppoe.macInfo, FDB_DYNAMIC, &columIdx,&asic_l2);

		if(ret != SUCCESS)
		{
			printk("can't get l2 entry by mac.....\n");
			
			#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
			/*pppoe passthru will set pppoe pkt vid=100, then pppoe server mac
 * 			will be learnt on fid=0.Here we need to add it to fid=1 too.*/
			if (rtl865x_Lookup_fdb_entry(RTL_LAN_FID, (ether_addr_t *)rt_t->un.pppoe.macInfo, FDB_DYNAMIC, &columIdx,&asic_l2) == SUCCESS) {
				//ret = rtl865x_addFilterDatabaseEntry(fid, (ether_addr_t *)rt_t->un.pppoe.macInfo, asic_l2.memberPortMask, FDB_TYPE_FWD);
				if (rt_t->un.pppoe.macInfo != NULL)
				{
        			ret = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, fid,  (ether_addr_t *)rt_t->un.pppoe.macInfo, FDB_TYPE_FWD, asic_l2.memberPortMask, FALSE, FALSE);
				}
				
				ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt_t->un.pppoe.macInfo, FDB_STATIC, &columIdx,&asic_l2);
				//panic_printk("rtl865x_Lookup_fdb_entry ret is %d.....\n", ret);
			}
			#endif			
		}
		
		/*FIXME_hyking: update mac/fdb table reference count*/
		asic_t.nextHopRow = rtl8651_filterDbIndex( rt_t->un.pppoe.macInfo,fid);
		asic_t.nextHopColumn = columIdx;
		break;

	case RT_L2:
		/*
		* NOTE:this type not used now...
		* if we want to use it, please add FDB entry to sure this L2 entry in both software FDB table and Asic L2 table.
		*/
		ret = rtl865x_getVlanFilterDatabaseId(rt_t->dstNetif->vid,&fid);		
		ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt_t->un.direct.macInfo, FDB_STATIC, &columIdx,&asic_l2);

		if(ret != SUCCESS)
			printk("can't get l2 entry by mac.....***************\n");
		
		/*FIXME_hyking: update mac/fdb table reference count*/
		asic_t.nextHopRow = rtl8651_filterDbIndex(rt_t->un.direct.macInfo,fid);
		asic_t.nextHopColumn = columIdx;
		break;

	case RT_ARP:
		/*FIXME_hyking: update arp table reference count??*/
		asic_t.arpStart	= rt_t->un.arp.arpsta;
		asic_t.arpEnd	= rt_t->un.arp.arpend;	
		asic_t.arpIpIdx	= rt_t->un.arp.arpIpIdx;		
		break;
		
	case RT_CPU:
	case RT_DROP:
		/*do nothing*/
		break;
		
	case RT_NEXTHOP:
		
 		asic_t.nhStart		 = rt_t->un.nxthop.nxtHopSta;
		asic_t.nhNum	 	 = rt_t->un.nxthop.nxtHopEnd - rt_t->un.nxthop.nxtHopSta + 1;
		asic_t.nhNxt	 	 = asic_t.nhStart;
		asic_t.ipDomain		 = rt_t->un.nxthop.ipDomain;
		asic_t.nhAlgo	 	 = rt_t->un.nxthop.nhalog;	

		break;
		
	default:
		printk("Process_Type(%d) is not support!\n",rt_t->process);
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)

	/*for dislite */
	asic_t.DSLEG =rt_t->DSLEG;
	asic_t.DSL_IDX =rt_t->DSL_IDX;
#endif
#endif	
	if(rt_t->asicIdx > RT_ASIC_ENTRY_NUM-1)
	{
		printk("BUG!! %s(%d)....", __FUNCTION__,__LINE__);
		return FAILED;
	}
	
	ret = rtl8651_setAsicRouting(rt_t->asicIdx, &asic_t);

	return ret;
	
}

static int32 _rtl865x_updateDefaultRoute(rtl865x_route_t *rt, int32 action)
{
	int32 i;
	rtl865x_route_t *entry;
	int32 retval = FAILED;

	entry = rt;
	if(entry == NULL)
		return RTL_EINVALIDINPUT;
	
	/*delete nexthop which is add by default route*/
	if(rt->process == RT_NEXTHOP)
		for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd && entry->un.nxthop.nxtHopEnd != 0; i++)
		{
			retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
		}

	entry->un.nxthop.nxtHopSta = 0;
	entry->un.nxthop.nxtHopEnd = 0;
	switch(action)
	{
		case RT_DEFAULT_RT_NEXTHOP_CPU:
			retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)entry, entry->dstNetif, 0,entry->srcIp);
			break;

		case RT_DEFAULT_RT_NEXTHOP_NORMAL:
			{
				rt->process = RT_NEXTHOP;	
				switch(rt->dstNetif->if_type)
				{
					case IF_ETHER:
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, rt->nextHop,entry->srcIp);
						break;
					case IF_PPPOE:
						{
							rtl865x_ppp_t *pppoe;
							
							pppoe = rtl865x_getPppByNetifName(rt->dstNetif->name);

							if(pppoe != NULL)
							{
								/*got pppoe session*/
								retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, pppoe->sessionId,entry->srcIp);
							}
							else
								/*nexthop's action is to CPU*/
								retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, rt->dstNetif, 0,entry->srcIp);
						}
						break;
				}
			}
			break;
	}
	retval = _rtl865x_synRouteToAsic(entry);

	return retval;
}

static int32 _rtl865x_arrangeRoute(rtl865x_route_t *start_rt, int32 start_idx)
{
	int32 count;
	rtl865x_route_t *rt = NULL;

	rt = start_rt;
	count = 0;
	while(rt)
	{		
		if(rt->valid)
		{
			/*if the rule is default route...*/
			if(rt->ipMask == 0)
				rt->asicIdx = RT_ASIC_ENTRY_NUM-1;
			else
			{
				/* entry number more than asic table's capacity*/
				/* entry index=RT_ASIC_ENTRY_NUM-1 is reserved for default route*/
				if((start_idx + count > RT_ASIC_ENTRY_NUM-2))
					break;
				
				/*delete old asic entry firstly...*/
				if(start_idx+count < rt->asicIdx && rt->asicIdx < RT_ASIC_ENTRY_NUM-1)
					rtl8651_delAsicRouting(rt->asicIdx);
				
				rt->asicIdx = start_idx+count;
				_rtl865x_synRouteToAsic(rt);
			}			
		}

		/*next entry*/
		rt= rt->next;
		count++;
	}

	
	/*more route entry need to add?*/
	if(rt)
	{
		/*not enough asic table entry! have to update default route's action TOCPU*/
		rt = _rtl865x_getDefaultRoute();
		_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_CPU);		
	}
	else
	{
		rt = _rtl865x_getDefaultRoute();
		_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_NORMAL);
	}
	
	return SUCCESS;
}

static int32 _rtl865x_addRouteToInusedList(rtl865x_route_t *rt)
{
	int32 retval = FAILED;
	int32 start_idx = 0;
	rtl865x_route_t *entry,*fore_rt,*start_rt;

	fore_rt = NULL;
	entry = rtl865x_route_inusedHead;

	/*always set 0x0f when init..., this value would be reset in arrange route*/
	rt->asicIdx = 0x0f;
	rt->next = NULL;
		
	/*find the right position...*/
	while(entry)
	{
		if(entry->valid == 1)
		{
			if(entry->ipMask < rt->ipMask)
			{
				break;
			}
		}
		fore_rt = entry;
		entry = entry->next;
	}

	/*insert this rule after insert_entry*/
	if(fore_rt)
	{
		rt->next = fore_rt->next;
		fore_rt->next = rt;
		start_idx = fore_rt->asicIdx+1;
		start_rt = rt;
	}
	else		
	{
		/*insert head...*/
		rt->next = rtl865x_route_inusedHead;		
		rtl865x_route_inusedHead = rt;
		
		start_idx = 0;
		start_rt = rtl865x_route_inusedHead;
	}	

	retval = _rtl865x_arrangeRoute(start_rt, start_idx);
	
	//_rtl865x_route_print();
	return retval;
	
}

static int32 _rtl865x_delRouteFromInusedList(rtl865x_route_t * rt)
{
	int32 retval,start_idx;
	rtl865x_route_t *fore_rt = NULL,*entry = NULL,*start_rt = NULL;

	entry = rtl865x_route_inusedHead;
	while(entry)
	{
		if(entry == rt)
			break;

		fore_rt = entry;		
		entry = entry->next;
	}

	/*fore_rt == NULL means delete list head*/
	if(fore_rt == NULL)
	{
		rtl865x_route_inusedHead = rtl865x_route_inusedHead->next;
		start_rt = rtl865x_route_inusedHead;
		start_idx = 0;
	}
	else
	{
		fore_rt->next = rt->next;
		start_rt = fore_rt->next;
		start_idx = fore_rt->asicIdx + 1;
	}

	/*delete route from asic*/
	if(rt->asicIdx < RT_ASIC_ENTRY_NUM)
		rtl8651_delAsicRouting(rt->asicIdx);
	
	retval = _rtl865x_arrangeRoute(start_rt, start_idx);
	rt->asicIdx = 0x0f;
	
	//_rtl865x_route_print();
	
	return retval;	
	
}


static int32 _rtl865x_usedNetifInRoute(int8 *ifname)
{

	rtl865x_route_t *rt = NULL;
	rt = rtl865x_route_inusedHead;

	while(rt)
	{	
		if(memcmp(rt->dstNetif->name,ifname,strlen(ifname)) == 0)
			return SUCCESS;
		rt = rt->next;
	}	
	return FAILED;
}



static int32 _rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop, int8 * ifName,ipaddr_t srcIp)
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_route_t *rt = NULL,*tmp_rt = NULL;
	int32 idx;
	int32 netSize = 0, usedArpCnt = 0;
	int32 retval = FAILED;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	int32 dsltIndex = FAILED;
#endif
#endif

	if ((ifName==NULL) && (nextHop!=0)) {
		extern int rtl865x_curOpMode;
		if ((rtl865x_curOpMode==1) ||(rtl865x_curOpMode==2)) // BRIDGE_MODE or WISP_MODE
			return RTL_EINVNETIFNAME;
	}


	/*para check*/
	if(ifName == NULL)
		netif = _rtl865x_getDefaultWanNetif();
	else	
		netif = _rtl865x_getSWNetifByName(ifName);

#if 0
	if ((ipAddr&ipMask) == 0xc0a80200)
		return RTL_EINVALIDINPUT;
#endif
	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	
	if(netif->if_type == IF_NONE)
		return RTL_ENOLLTYPESPECIFY;

	idx = 0;
	for(idx = 0; idx < 32; idx++)
		if((1<<idx) & ipMask)
			break;

	netSize = 1<<idx;

	if(netSize > RT_MAX_ARP_SIZE)
		return RTL_EINVALIDINPUT;

#if defined(CONFIG_SWCONFIG_SUPPORT_HW_NAT)
	if (swconfig_vlan_enable) {
		if (strncmp(netif->name, hw_nat_wan_dev_name, MAX_IFNAMESIZE) &&
		     strncmp(netif->name, RTL_DRV_LAN_NETIF_NAME, MAX_IFNAMESIZE) &&
		     strncmp(netif->name, RTL_SWCONFIG_DRV_PPP_NETIF_NAME, MAX_IFNAMESIZE))
			return RTL_EINVALIDINPUT;
	}
#endif
	
	/*
	*duplicate entry check:
	*	in Driver system, default route is always exist.
	*	so if ipMask == 0, it's means that default route should be update...
	*/
	if(ipMask != 0 && (rt = _rtl865x_getRouteEntry(ipAddr, ipMask)) != NULL)
	{
		//rt->ref_count++;
		#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
		rt->exist_in_ps = 1;
		#endif
		return RTL_EENTRYALREADYEXIST;
	}

	/*add default route: just update the default route becase the default route always exist!*/
	if(ipMask == 0)
	{
		rt = _rtl865x_getDefaultRoute();
		/*deference rt's orginal netif*/
		if (rt && rt->dstNetif) {
			#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
			if (rt->nextHop == nextHop) {
				if (ifName) {
					if (strncmp(rt->dstNetif->name, ifName, MAX_IFNAMESIZE) == 0) {
						rt->exist_in_ps = 1;
						return RTL_EENTRYALREADYEXIST;
					}
				} else {
					rt->exist_in_ps = 1;
					return RTL_EENTRYALREADYEXIST;
				}
			}
			#endif
			rtl865x_deReferNetif(rt->dstNetif->name);
		}
	}

	/*allocate a new buffer for adding entry*/
	if(rt == NULL)
	{
		rt = rtl865x_route_freeHead;

		if(rt)
		{
			rtl865x_route_freeHead = rt->next;
			
		}
	}
	
	if(rt == NULL)
	{
		/*no buffer, default route should be TOCPU?*/
		return RTL_ENOFREEBUFFER;
	}

		
	/*common information*/
	rt->ipAddr 	= ipAddr & ipMask;
	rt->ipMask 	= ipMask;
	rt->nextHop	= nextHop;
	rt->srcIp		= srcIp;
	rt->dstNetif 	= netif;	
	#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
	rt->exist_in_ps = 1;
	#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	if(netif->if_type == IF_DSLT)
	{
		dsltIndex=_rtl865x_getIpv6DsLiteEntryByName(netif->name);

		/*hook default gw for dslist interface */
		if(ipAddr ==0 && ipMask ==0 && nextHop!=0 && srcIp!=0)
		{		
			if(dsltIndex == FAILED){
				rt->DSLEG	= 0;
				rt->DSL_IDX = 0;			
			}
			else
			{
				rt->DSLEG	= 1;
				rt->DSL_IDX = dsltIndex;
			}
		}
	}
#endif
#endif	
	
	/*don't specify the nexthop ip address, it's means that:
	* all packet match this route entry should be forward by network interface with arp
	*/

	if(nextHop == 0 && ipMask != 0)
	{
		switch(netif->if_type)
		{
			case IF_ETHER:
				
				rt->process = RT_ARP;
				tmp_rt = rtl865x_route_inusedHead;
				while(tmp_rt)
				{
					if(tmp_rt->valid && tmp_rt->process == RT_ARP && tmp_rt->dstNetif == netif)
						usedArpCnt += tmp_rt->un.arp.arpend - tmp_rt->un.arp.arpend + 1;
					
					tmp_rt = tmp_rt->next;
				}
				
				if((usedArpCnt + netSize) > RT_MAX_ARP_SIZE)
				{
					printk("!!!!ERROR!!!usedArp(%d),netsize(%d)\n",usedArpCnt,netSize);
					goto addFailed;
				}

				/*allocate arp entry for this route rule...*/
				
				retval = rtl865x_arp_tbl_alloc(rt);
				if( retval != SUCCESS)
				{
					printk("error!!can't allocate arp for this route entry....retval(%d)\n",retval);
					goto addFailed;
				}

				//rt->un.arp.arpIpIdx = 0; /*FIXME_hyking[this field is invalid, right?]*/
				rtl865x_getIpIdxByIpRange(ipAddr, ipMask, &rt->un.arp.arpIpIdx);
				//printk("%s %d ipAddr=0x%x ifName=%s rt->un.arp.arpIpIdx=%d\n", __FUNCTION__, __LINE__, ipAddr, ifName, rt->un.arp.arpIpIdx);
				break;

			case IF_PPPOE:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = RT_PPPOE;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if(ppp == NULL)
					{
						printk("error!!can't get pppoe session information by interface name(%s)\n",netif->name);
						goto addFailed;
					}

					rt->un.pppoe.macInfo = &ppp->server_mac;
					rt->un.pppoe.pppInfo = ppp;
					
					/*update reference...*/
					rtl865x_referPpp(ppp->sessionId);
#if 0//defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
					retval = rtl865x_getNxtHopIdx(NEXTHOP_DEFREDIRECT_ACL, netif, ppp->sessionId);
					if (retval == -1) {
						retval = rtl865x_addNxtHop(NEXTHOP_DEFREDIRECT_ACL, (void*)rt, netif, ppp->sessionId, srcIp);
						if (retval != SUCCESS) {
							printk("Warning!!add pppoe nexthop for redirect acl fail! retval (%d)\n", retval);
							goto addFailed;
						}
					}
#endif
				}
				break;
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *ppp = NULL;

					rt->process = RT_L2;
					ppp = rtl865x_getPppByNetifName(netif->name);					

					if(ppp == NULL)
					{
						/*printk("Warning!!CAn't get pptp/l2tp session information by interface name(%s)\n",netif->name);*/
						goto addFailed;
					}

					rt->un.direct.macInfo = &ppp->server_mac;
				}
				break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
			case IF_DSLT:
				if(dsltIndex!=FAILED)
				{
					/*
						set L3 table
						All packet dip equal to tunnel local ip,trap to cpu.
					*/
					extern int  get_P2P_local_ip(unsigned char *ifname, unsigned int *ipAddr);

					/*get p2p device local ip *******************/
					if(!get_P2P_local_ip(netif->name,&(rt->ipAddr)))
					{
						printk("%s,%d.get local ip fail\n",__FUNCTION__,__LINE__);
					}
					rt->process = RT_CPU;	
				}
				break;
#endif
#endif			
			default:
				printk("lltype(%d) is not support now....\n",netif->if_type);
				goto addFailed;
		}
		
	}
	else
	{
		/*if default is valid, delete nexthop firstly...*/
		if(rt->valid == 1 && rt->process == RT_NEXTHOP)
              {
                        int i;  
                        for ( i = rt->un.nxthop.nxtHopSta; i <= rt->un.nxthop.nxtHopEnd; i++)
                        {       
                                retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
                        }       
              }
		
		/*use nexthop type*/
		rt->process = RT_NEXTHOP;			
		switch(netif->if_type)
		{
			case IF_ETHER:
				retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif, nextHop,srcIp);
				break;
			case IF_PPPOE:
			case IF_PPTP:
			case IF_L2TP:
				{
					rtl865x_ppp_t *pppoe;
					
					pppoe = rtl865x_getPppByNetifName(netif->name);

					if(pppoe != NULL)
					{
						/*got pppoe session*/
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif, pppoe->sessionId,srcIp);
					}
					else
						/*nexthop's action is to CPU*/
						retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif, 0,srcIp);
				}
				break;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)

			case IF_DSLT:	
				{
					retval = rtl865x_addNxtHop(NEXTHOP_L3, (void*)rt, netif, nextHop,srcIp);					
				}
				break;
#endif
#endif		

			default:
				retval = FAILED;
				break;
				
		}
		
		if(retval != SUCCESS)
		{
			printk("error!!add nexthop error! retval (%d)\n",retval);
			goto addFailed;	
		}		
		rt->un.nxthop.nhalog = RT_ALOG_SIP; /* use per-source IP */
		rt->un.nxthop.ipDomain = RT_DOMAIN_16_1;		
	}

	rt->valid		= 1;
	rt->ref_count	= 1;
	/*update reference....*/
	rtl865x_referNetif(netif->name);
	if(ipMask == 0)
		_rtl865x_setDefaultWanNetif(netif->name);

	/**/
	if(rt->asicIdx == RT_ASIC_ENTRY_NUM-1)
	{
		retval = _rtl865x_synRouteToAsic(rt);
	}
	else
	{		
		/*insert the adding route to inused list*/
		retval = _rtl865x_addRouteToInusedList(rt);		
	}

	/*if route is add, please enable Routing for the releated netif*/
	retval = rtl865x_enableNetifRouting(netif);
	return retval;
	
addFailed:
	if(rt->asicIdx == RT_ASIC_ENTRY_NUM -1)
	{
		_rtl865x_updateDefaultRoute(rt, RT_DEFAULT_RT_NEXTHOP_CPU);
	}
	else
	{
		/*free this route entry and return error code...*/	
		memset(rt,0,sizeof(rtl865x_route_t));
		rt->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = rt;
	}
	return retval;
	
}

static int32 _rtl865x_delRoute( ipaddr_t ipAddr, ipaddr_t ipMask )
{
	rtl865x_route_t *entry;
	int32 i;
	int32 retval = 0;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	int32 dsltIndex = FAILED;
#endif
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
	int32 fid = 0;
#endif

	entry = _rtl865x_getRouteEntry(ipAddr, ipMask);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	dsltIndex=_rtl865x_getIpv6DsLiteEntryByName(entry->dstNetif->name);
	if(dsltIndex !=FAILED)
	{
		_rtl865x_delIpv6DsLiteEntry(dsltIndex);
	}
#endif	
#endif
	if(entry->asicIdx == RT_ASIC_ENTRY_NUM-1)
	{
		/*if default route
		* 1. reset default route
		* 2. reset entry->netif...
		*/
		rtl865x_netif_local_t *netif = NULL;
		_rtl865x_clearDefaultWanNetif(entry->dstNetif->name);

		netif = _rtl865x_getDefaultWanNetif();
		if(netif==NULL)
		{
			return RTL_EINVNETIFNAME;
		}
		
		if(netif != entry->dstNetif)
		{
			rtl865x_deReferNetif(entry->dstNetif->name);
			entry->dstNetif = netif;
			rtl865x_referNetif(netif->name);
		}
		
		retval = _rtl865x_updateDefaultRoute(entry, RT_DEFAULT_RT_NEXTHOP_CPU);
	}
	else		
	{
		/*not default route*/
		switch(entry->process)
		{
			case RT_PPPOE:
				{
					rtl865x_ppp_t *ppp = entry->un.pppoe.pppInfo;
					if(ppp)
						rtl865x_deReferPpp(ppp->sessionId);
				}
			
				#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
				rtl865x_getVlanFilterDatabaseId(entry->dstNetif->vid, &fid);
				//rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, fid, (ether_addr_t *)entry->un.pppoe.macInfo);
				if (entry->un.pppoe.macInfo != NULL)
					_rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, fid, (ether_addr_t *)entry->un.pppoe.macInfo);
				#endif
				
				break;
			case RT_L2:
				/*
				* NOTE:this type not used now...
				* if we want to use it, please DELETE FDB entry to sure this L2 entry is deleted both software FDB table and Asic L2 table.
				*/
				break;
			case RT_ARP:
				/*free arp*/
				retval = rtl865x_arp_tbl_free(entry);
				if( retval != SUCCESS)
				{
					printk("======error!!can't FREE arp entry for this route entry....retval(%d)\n",retval);					
				}
				break;				

			case RT_CPU:
			case RT_DROP:
				/*do nothing*/
				
				break;

			case RT_NEXTHOP:
				/*delete nexthop which is add by l3*/
				for ( i = entry->un.nxthop.nxtHopSta; i <= entry->un.nxthop.nxtHopEnd; i++)
				{
					retval = rtl865x_delNxtHop(NEXTHOP_L3, i);
				}				
				break;
		}

		/*FIXME_hyking: update netif reference count*/
		rtl865x_deReferNetif(entry->dstNetif->name);

		
		/*remove from inused list...*/		
		_rtl865x_delRouteFromInusedList(entry);

		if(_rtl865x_usedNetifInRoute(entry->dstNetif->name) == FAILED)
			rtl865x_disableNetifRouting(entry->dstNetif);

		/*add to free list*/
		memset(entry,0,sizeof(rtl865x_route_t));
		entry->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = entry;

		retval = SUCCESS;		
	}	
	
	//_rtl865x_route_print();
	return retval;
	
}

rtl865x_route_t* _rtl85x_getRouteEntry(ipaddr_t dst)
{
	rtl865x_route_t *tmpRtEntry = NULL;
	rtl865x_route_t *rt=rtl865x_route_inusedHead;
	uint32 mask;
	
	mask = 0;
	while(rt)
	{
		if (rt->valid == 1 && rt->ipAddr == (rt->ipMask & dst) && mask <= rt->ipMask) {
			mask = rt->ipMask;
			tmpRtEntry = rt;
		}
		rt = rt->next;
	}
	return tmpRtEntry;
}

/*
@func int32 | rtl865x_addRoute |add a route entry.
@parm ipaddr_t | ipAddr | ip address.
@parm ipaddr_t | ipMask | ip mask.
@parm ipaddr_t | nextHop | the route's next hop.
@parm int8* | ifName | destination network interface. 
@parm ipaddr_t | srcIp |source IP
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EINVALIDINPUT | invalid input.
@rvalue RTL_ENOLLTYPESPECIFY | network interface's link type is not specified.
@rvalue RTL_EENTRYALREADYEXIST | route entry is already exist.
@rvalue RTL_ENOFREEBUFFER | not enough memory in system.
@comm
	if ifName=NULL, it means the destionation network interface of route entry with ip/ipMask/nextHop is default wan.
*/
int32 rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop,int8 * ifName,ipaddr_t srcIp)
{
	int32 retval = 0;
	unsigned long flags=0;	
	//printk("========%s(%d), ip(0x%x),mask(0x%x),ifname(%s),nxthop(0x%x)\n",__FUNCTION__,__LINE__,ipAddr,ipMask,ifName,nextHop);
	//rtl_down_interruptible(&route_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_addRoute(ipAddr,ipMask,nextHop,ifName,srcIp);		
	//rtl_up(&route_sem);
	SMP_UNLOCK_ETH(flags);	
	//printk("========%s(%d), ip(0x%x),mask(0x%x),ifname(%s),nxthop(0x%x),retval(%d)\n",__FUNCTION__,__LINE__,ipAddr,ipMask,ifName,nextHop,retval);
	//_rtl865x_route_print();
	return retval;
}
/*
@func int32 | rtl865x_delRoute |delete a route entry.
@parm ipaddr_t | ipAddr | ipAddress.
@parm ipaddr_t | ipMask | ipMask.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EENTRYNOTFOUND | not found the entry.
@comm	
*/
int32 rtl865x_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask)
{

	int32 retval = 0;
	unsigned long flags=0;	
	//printk("========%s(%d), ip(0x%x),mask(0x%x)\n",__FUNCTION__,__LINE__,ipAddr,ipMask);
	//rtl_down_interruptible(&route_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delRoute(ipAddr,ipMask);
	//rtl_up(&route_sem);	
	SMP_UNLOCK_ETH(flags);
	//printk("==================================retval(%d)\n",retval);
	return retval;

}

/*
@func int32 | rtl865x_getRouteEntry |according the destination ip address, get the matched route entry.
@parm ipaddr_t | dst | destionation ip address.
@parm rtl865x_route_t* | rt | retrun value: route entry pointer
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_getRouteEntry(ipaddr_t dst,rtl865x_route_t *rt)
{
	int32 retval = FAILED;
	rtl865x_route_t *ret_entry = NULL;

	ret_entry = _rtl85x_getRouteEntry(dst);
	if(ret_entry && rt)
	{
		memcpy(rt,ret_entry,sizeof(rtl865x_route_t));		
		retval = SUCCESS;
	}
	return retval;
}

/*
@func int32 | rtl865x_initRouteTable |initialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_initRouteTable(void)
{
	int32 i;
	rtl865x_route_t *rt;
	rtl865x_route_freeHead = NULL;
	rtl865x_route_inusedHead = NULL;

	/*malloc buffer*/
	TBL_MEM_ALLOC(rt, rtl865x_route_t, RT_DRV_ENTRY_NUM);	
	memset(rt,0,sizeof(rtl865x_route_t)*RT_DRV_ENTRY_NUM);
	for(i = 0; i < RT_DRV_ENTRY_NUM; i++)
	{
		rt[i].next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = &rt[i];		
	}	

	#if defined(CONFIG_RTL_HARDWARE_NAT)//&&defined(REINIT_SWITCH_CORE)
	memset(rtl865x_route_inused_bak, 0, sizeof(rtl865x_route_t)*RT_DRV_ENTRY_NUM);
	#endif
	
	return SUCCESS;	
}

/*
@func int32 | rtl865x_reinitRouteTable |reinitialize route tabel.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_reinitRouteTable(void)
{
	rtl865x_route_t *rt;

	rt = rtl865x_route_inusedHead;
	while(rt && rt->asicIdx != RT_ASIC_ENTRY_NUM -1)
	{
		_rtl865x_delRoute(rt->ipAddr,rt->ipMask);
		rt = rtl865x_route_inusedHead;
	}

	/*delete the last route*/
	rt = rtl865x_route_inusedHead;
	if(rt)
	{
		/*FIXME_hyking: update netif reference count*/
		rtl865x_deReferNetif(rt->dstNetif->name);
		
		/*remove from inused list...*/		
		_rtl865x_delRouteFromInusedList(rt);

		/*add to free list*/
		memset(rt,0,sizeof(rtl865x_route_t));
		rt->next = rtl865x_route_freeHead;
		rtl865x_route_freeHead = rt;
	}
	return SUCCESS;
}

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
int rtl865x_getLanRoute(rtl865x_route_t routeTbl[], int tblSize )
{
	int cnt=0;
	rtl865x_route_t *rt = NULL;	
	rt = rtl865x_route_inusedHead;

	while(rt)
	{	
		
		if((rt->valid==1) && (rt->dstNetif->is_wan==0))
		{			
			memcpy(&routeTbl[cnt], rt, sizeof(rtl865x_route_t) );
			cnt++;
		}
		rt = rt->next;
	}	
	return cnt;
}
#endif
#ifdef CONFIG_RTL_PROC_NEW
int32 sw_l3_read(struct seq_file *s, void *v)
{
	rtl865x_route_t * entry=NULL;
	
	uint32 idx=0;

	seq_printf(s, "%s\n", "SW L3 Routing Table:\n");

	entry = rtl865x_route_inusedHead;
	while(entry)
	{
		if(entry->valid == 0)
		
			continue;
		
		seq_printf(s,"\t[%d]  ip:%x,%x,	%x,nexthop:%x  process(%d)  asicIdx:%d, netif%s \n", 
			idx, entry->ipAddr,entry->ipMask,entry->srcIp,
			entry->nextHop,entry->process,entry->asicIdx,entry->dstNetif->name);
		idx++;
		entry=entry->next;
	}

	return 0;
}
#else
int32 sw_l3_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_route_t * entry=NULL;
	uint32 idx = 0;
	
	len = sprintf(page, "%s\n", "SW L3 Routing Table:\n");
	entry = rtl865x_route_inusedHead;
	while(entry)
	{
		if(entry->valid == 0)
		
			continue;
		
		len += sprintf(page + len,"\t[%d]  ip:%x,%x,	%x,nexthop:%x  process(%d)  asicIdx:%d, netif:%s \n", 
			idx, entry->ipAddr,entry->ipMask,entry->srcIp,
			entry->nextHop,entry->process,entry->asicIdx,entry->dstNetif->name);
		idx++;
		entry=entry->next;
	}
	

	return len;
}
#endif

int32 sw_l3_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int rtl865x_checkL3L2Valid(ipaddr_t ipAddr )
{

	int ret=0;
	rtl865x_route_t *rt = NULL; 
	
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	
	uint32 columIdx,fid;
	
	rt = rtl865x_route_inusedHead;
	
	if(ipAddr==0)
		return ret;
	
	while(rt)
	{	
		
		if((rt->valid==1) && (rt->dstNetif->is_wan==1))
		{		
			if((rt->ipAddr&rt->ipMask)==(ipAddr&rt->ipMask))
			{
				switch (rt->process)
				{
				case RT_PPPOE:
					/*fix me: to real check pppoe valid*/
					return 1;
					break;

				case RT_L2:
					/*
					* NOTE:this type not used now...
					* if we want to use it, please add FDB entry to sure this L2 entry in both software FDB table and Asic L2 table.
					*/
					ret = rtl865x_getVlanFilterDatabaseId(rt->dstNetif->vid,&fid);		
					ret = rtl865x_Lookup_fdb_entry(fid, (ether_addr_t *)rt->un.direct.macInfo, FDB_STATIC, &columIdx,&asic_l2);
					if(ret == SUCCESS)
					{
						return 1;
					}
					else
					{
						return 0;
					}

					break;

				case RT_ARP:
					if(rtl865x_isArpExisted( ipAddr))
					{
						return 1;
					}
					else
					{
						return 0;
					}
					break;
					
				case RT_CPU:
				case RT_DROP:
					return 0;
					break;
					
				case RT_NEXTHOP:
				
					switch(rt->dstNetif->if_type)
					{
						case IF_ETHER:
							if( rtl865x_getNxtHopIdx(NEXTHOP_L3,rt->dstNetif,rt->nextHop)!=-1)
							{
								return 1;
							}
							else
							{
								return 0;
							}
							
							break;
						case IF_PPPOE:
							/*fix me, to really check pppoe nexthop valid or not*/
							return 1;
							
						case IF_PPTP:
						case IF_L2TP:
							return 0;
							break;

						default:
							return 0;
							break;
							
					}

					break;
					
				default:
					return 0;
					break;
				}
		
			}
			
		}
		rt = rt->next;
	}	
	
	return 0;
}

#endif 

int rtl865x_delPppoeRouter(char *name)
{
	int i;
	rtl865x_route_t *rt;

	for (i=0; i<RT_ASIC_ENTRY_NUM; i++)
	{
		rt = rtl865x_route_inusedHead;
		while (rt)
		{
			if ((rt->valid==1) && (strncmp(rt->dstNetif->name, name, 4)==0)) {
				_rtl865x_delRoute(rt->ipAddr,rt->ipMask);
				break;
			}
			rt = rt->next;
		}
	}
		
	return SUCCESS;
}

#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
int rtl_clear_router_exist_in_ps_flag(void)
{
	unsigned long flags=0;	
	rtl865x_route_t *rt = NULL;	
	
	SMP_LOCK_ETH(flags);
	rt = rtl865x_route_inusedHead;
	while(rt)
	{	
		if (rt->valid == 1)
			rt->exist_in_ps = 0;
		
		rt = rt->next;
	}	
	SMP_UNLOCK_ETH(flags);
	return SUCCESS;
}

int rtl_del_router_not_exist_in_ps(void)
{
	int i, count = 0;
	unsigned long flags=0;	
	rtl865x_route_t *rt = NULL;	
	struct ip_mask_tuple tmp[RT_DRV_ENTRY_NUM];
	
	SMP_LOCK_ETH(flags);
	
	memset(tmp, 0, sizeof(struct ip_mask_tuple)*RT_DRV_ENTRY_NUM);
	rt = rtl865x_route_inusedHead;
	while(rt)
	{	
		/*find the router entry who is not in linux ps*/
		if ((rt->valid==1) && (rt->exist_in_ps==0)) {
			tmp[count].ip 		= rt->ipAddr;
			tmp[count].mask	= rt->ipMask;
			count ++;
		}

		rt = rt->next;
	}	

	for (i=0; i<count; i++)
	{
		_rtl865x_delRoute(tmp[i].ip, tmp[i].mask);
	}
	
	SMP_UNLOCK_ETH(flags);
	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_HARDWARE_NAT)//&&defined(REINIT_SWITCH_CORE)
int rtl819x_save_inused_route(void)
{
	int i = 0;
	rtl865x_route_t *rt = rtl865x_route_inusedHead;

	memset(rtl865x_route_inused_bak, 0, sizeof(rtl865x_route_t)*RT_DRV_ENTRY_NUM);
	while(rt)
	{
		if (rt->valid == 1) {
			memcpy(&rtl865x_route_inused_bak[i], rt, sizeof(rtl865x_route_t));
			i++;
			if (i >= RT_DRV_ENTRY_NUM)
				break;
		}
		rt = rt->next;
	}

	return SUCCESS;
}

int rtl819x_restore_inused_route(void)
{
	int i = 0;
	rtl865x_route_t *rt = NULL;

	for (i=0; i<RT_DRV_ENTRY_NUM; i++)
	{
		if (rtl865x_route_inused_bak[i].valid == 1) {
			rt = &rtl865x_route_inused_bak[i];
			rtl865x_addRoute(rt->ipAddr, rt->ipMask, rt->nextHop, rt->dstNetif->name, rt->srcIp);
		}
	}

	return SUCCESS;
}
#endif

#if defined(CONFIG_SWCONFIG_SUPPORT_HW_NAT)
int rtl_check_default_router_valid(void)
{
	int ret = FALSE;
	rtl865x_route_t *rt = NULL;

	rt = _rtl865x_getDefaultRoute();
	if (rt && rt->dstNetif) {
		if ((strncmp(rt->dstNetif->name, hw_nat_wan_dev_name, MAX_IFNAMESIZE) == 0) ||
		    (strncmp(rt->dstNetif->name, RTL_SWCONFIG_DRV_PPP_NETIF_NAME, MAX_IFNAMESIZE) == 0))
			ret = TRUE;
	}

	return ret;
}
#endif

