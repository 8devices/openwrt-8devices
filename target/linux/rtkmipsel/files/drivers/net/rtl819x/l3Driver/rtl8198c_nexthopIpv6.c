/*
* Copyright c                  Realtek Semiconductor Corporation, 2013  
* All rights reserved.
* 
* Program : ipv6 nexthop table driver
* Abstract : 
* Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl8196c_nexthopIpv6.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl_errno.h"
#include "common/rtl865x_netif_local.h" 
#include "rtl865x_ip.h"
#include "rtl8198c_nexthopIpv6.h"
//#include "rtl865x_ppp.h"
#include "rtl865x_ppp_local.h"
#include "rtl8198c_routeIpv6.h"
#include "rtl8198c_arpIpv6.h"

#include "common/rtl865x_vlan.h"
#include "common/rtl865x_eventMgr.h" /*call back function....*/

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif
#include "l2Driver/rtl865x_fdb.h"
#include <net/rtl/rtl865x_fdb_api.h>

#include <linux/version.h>

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#include "rtl865x_route.h"
#include "rtl865x_arp.h"
#include "rtl865x_nexthop.h"
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#ifdef CONFIG_RTL_HW_6RD_SUPPORT
#include <linux/netdevice.h>
#include <net/ip_tunnels.h>
#include "rtl865x_6rd.h"
extern ipaddr_t br_ip_addr;
#endif
#endif


static rtl8198c_ipv6_nextHopEntry_t *rtl865x_ipv6_nxtHopTable;

#define IPV6_NEXTHOP_TABLE_INDEX(entry)	(entry - rtl865x_ipv6_nxtHopTable)
static int32 _rtl8198c_ipv6_nextHop_register_event(void);
static int32 _rtl8198c_ipv6_nextHop_unRegister_event(void);
static int32 _rtl8198c_delIpv6NxtHop(uint32 attr, uint32 entryIdx);

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
extern int32 _rtl865x_referPpp(uint32 sessionId);
extern int32 _rtl865x_deReferPpp(uint32 sessionId);
#endif

#if 0
static void _print_softNxtHop(void)
{
	int32 i = 0;
	rtl8198c_ipv6_nextHopEntry_t *entry = rtl865x_ipv6_nxtHopTable;
	printk("software nextHop table:\n");
	for(i = 0; i < IPV6_NXTHOP_ENTRY_NUM; i++,entry++)
	{
		printk("idx(%d): valid(%d),dst(%s),nexthop(0x%x%x%x%x),nexthopType(%d),refCnt(%d)\n",i,entry->valid,entry->dstNetif->name,
			entry->un.nexthop.v6_addr32[0], entry->un.nexthop.v6_addr32[1],entry->un.nexthop.v6_addr32[2],
			entry->un.nexthop.v6_addr32[3],entry->nextHopType,entry->refCnt);
	}
}
#endif

/*
@func int32 | rtl8198c_initIpv6NxtHopTable |initialize the nexthop table
@rvalue SUCCESS | success.
@rvalue FAILED | failed. system should reboot.
@comm	
*/
int32 rtl8198c_initIpv6NxtHopTable(void)
{
	TBL_MEM_ALLOC(rtl865x_ipv6_nxtHopTable, rtl8198c_ipv6_nextHopEntry_t, IPV6_NXTHOP_ENTRY_NUM);	
	memset(rtl865x_ipv6_nxtHopTable,0,sizeof(rtl8198c_ipv6_nextHopEntry_t)*IPV6_NXTHOP_ENTRY_NUM);
	_rtl8198c_ipv6_nextHop_register_event();
	return SUCCESS;	
}

/*
@func int32 | rtl8198c_reinitIpv6NxtHopTable |reinitialize the nexthop table
@rvalue SUCCESS | success.
@comm	
*/
int32 rtl8198c_reinitIpv6NxtHopTable(void)
{
	int32 i;
	_rtl8198c_ipv6_nextHop_unRegister_event();

	for(i = 0; i < IPV6_NXTHOP_ENTRY_NUM; i++)
	{
		if(rtl865x_ipv6_nxtHopTable[i].valid)
			_rtl8198c_delIpv6NxtHop(IPV6_NEXTHOP_L3,i);
	}
	
	_rtl8198c_ipv6_nextHop_register_event();
	return SUCCESS;
}

static int32 _rtl8198c_synIpv6NxtHopToAsic(rtl8198c_ipv6_nextHopEntry_t *entry_t)
{
	rtl8198C_tblAsicDrv_nextHopV6Param_t asic;
	ether_addr_t reservedMac;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif;
	uint32 fid = 0;
	uint32 columIdx = 0;
	int32 retval = 0;

	retval = rtl865x_getReserveMacAddr(&reservedMac);
	
	bzero(&asic, sizeof(rtl8198C_tblAsicDrv_nextHopV6Param_t));

	if (entry_t == NULL)
		return RTL_EINVALIDINPUT;
	
	if (entry_t->nextHopType == IF_ETHER) {
		rtl8198c_ipv6_arpMapping_entry_t arp_t;
		int32 ret_arpFound = FAILED;

		/*if the arp info of nexthop is not found, reserved to cpu Mac is used for trap packet to CPU*/
		if ((entry_t->un.nexthop.v6_addr32[0]!=0)||(entry_t->un.nexthop.v6_addr32[1]!=0)||
		     (entry_t->un.nexthop.v6_addr32[2]!=0)||(entry_t->un.nexthop.v6_addr32[3]!=0))
			ret_arpFound = rtl8198c_getIpv6ArpMapping(entry_t->un.nexthop, &arp_t);
		
		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid, &fid);
		retval = rtl865x_Lookup_fdb_entry(fid, (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
		
		asic.nextHopRow = rtl8651_filterDbIndex( (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, fid );
		asic.nextHopColumn = (retval == SUCCESS)? columIdx: 0;

		asic.isPppoe = FALSE;
		asic.pppoeIdx = 0;
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#ifdef CONFIG_RTL_HW_6RD_SUPPORT
	else if(entry_t->nextHopType == IF_6RD)
	{
		rtl865x_arpMapping_entry_t arp_t;
		int32 ret_arpFound = FAILED;
		/*if the arp info of nexthop is not found, reserved to cpu Mac is used for trap packet to CPU*/

		ret_arpFound = rtl8198c_6rd_Ipv6ArpMapping(br_ip_addr, &arp_t);
		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		retval = rtl865x_Lookup_fdb_entry(fid, (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);

		asic.nextHopRow = rtl8651_filterDbIndex( (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, fid );
		asic.nextHopColumn = (retval == SUCCESS)? columIdx: 0;
		asic.isPppoe = FALSE;
		asic.pppoeIdx = 0;
		
		//printk("%d,%d,[%s]:[%d].\n",asic.nextHopRow,asic.nextHopColumn,__FUNCTION__,__LINE__);
	}
#endif
#endif
	else 
	{	
		/*session based interface type*/
		rtl865x_ppp_t pppoe;		
		int32 pppidx = 0;

		memset(&pppoe,0,sizeof(rtl865x_ppp_t));
		
		if(entry_t->un.pppSessionId)
			retval = rtl865x_getPppBySessionId(entry_t->un.pppSessionId, &pppoe);

		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);

		retval =rtl865x_Lookup_fdb_entry(fid, (pppoe.valid)? &pppoe.server_mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
			
		asic.nextHopRow = rtl8651_filterDbIndex( (pppoe.valid)? &pppoe.server_mac : &reservedMac, fid);
		asic.nextHopColumn = (pppoe.valid)? columIdx: 0;
		asic.isPppoe = (pppoe.type == IF_PPPOE)? TRUE: FALSE;

		retval = rtl865x_getPppIdx(&pppoe, &pppidx);

		//printk("%s(%d): pppoeIdx(%d), pppoeType(%d), pppoevalid(%d),pppoeSid(%d)\n",__FUNCTION__,__LINE__,pppidx,pppoe.type,pppoe.valid,pppoe.sessionId);
		asic.pppoeIdx	= (pppoe.type == IF_PPPOE)? pppidx: 0;
	}
	
	if (entry_t->dstNetif->is_slave == 1) {
		dstNetif = entry_t->dstNetif->master;

		if(dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	} else {
		dstNetif = entry_t->dstNetif;
	}

	if(dstNetif == NULL)
		printk("_%s(%d), BUG!!!!!!",__FUNCTION__,__LINE__);
	
	asic.dvid 			= dstNetif->asicIdx;

	//printk("%s(%d), entryIdx(%d),asic.isPPPoe(%d),asic.pppoeIdx(%d),asic.dvid(%d)\n", __FUNCTION__,__LINE__,entry_t->entryIndex,asic.isPppoe,asic.pppoeIdx,asic.dvid);
	rtl8198C_setAsicNextHopTableV6(entry_t->entryIndex,  &asic);
	
	return SUCCESS;
}


static int32 _rtl8198c_arrangeIpv6NxtHop(uint32 start, uint32 num)
{
	int32 idx;
	rtl8198c_ipv6_nextHopEntry_t *entry = NULL;

	if(start < 0 || start + num >= IPV6_NXTHOP_ENTRY_NUM)
		return RTL_EINVALIDINPUT;
	
	for(idx=start; idx<start+num; idx++)
	{
		entry = &rtl865x_ipv6_nxtHopTable[idx];

		if(entry->valid)
			_rtl8198c_synIpv6NxtHopToAsic(entry);
	}
	
	return SUCCESS;
}

static int32 _rtl8198c_addIpv6NxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, inv6_addr_t nexthop)
{
	int entryIdx;
	rtl8198c_ipv6_nextHopEntry_t *entry = NULL, *entry1 = NULL;
	rtl8198c_ipv6_route_t *rt_t = NULL;

	/*
	  * NOTE:
	  * parameter description:
	  * (1) attr: why need to add the nexthop entry? NEXTHOP_L3 or NEXTHOP_DEFREDIRECT_ACL?
	  * (2) ref_ptr: when attr = NEXTHOP_L3, ref_ptr point to a route structure,
	  *				   attr = NEXTHOP_DEFREDIRECT_ACL, ref_ptr = NULL,
	  *				   attr = others, ref_ptr = NULL
	  * (3) netif: destination network interface
	  * (4) nexthop: 
	  *		a) netif->if_type == IF_ETHER, nexthop = nexthop ip address,
	  *		b) netif->if_type == session based type, nexthop = session Id,
	  *
	  * following case should be NOTED now:
	  * (1) ETHERNET type network interface:
	  *	 a) If nexthop != NULL , it means the entry is added for:
	  *		nexthop ip&mac information, nextHop = arp entry of nexthop ip address.
	  *	  b) If nexthop == 0, use default route's nexthop or nexthop TOCPU
	  *
	  * (2) PPPoE/PPTP/L2TP type network interface:
	  *	  The "nexthop" will explicitly specify the PPPoE session (PPTP/L2TP session).
	  */

	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	
	/* Allocate an empty entry for new one */
	/*Note: all nexthop entry referenced by L3 must be 2 entries aligned(reference l3 Datasheet)*/
	for (entryIdx = 0; entryIdx < IPV6_NXTHOP_ENTRY_NUM; entryIdx++)
	{
		if (rtl865x_ipv6_nxtHopTable[entryIdx].valid == 0) {
			switch(attr)
			{
				case IPV6_NEXTHOP_L3:
					if ((entryIdx%2==0)&&((entryIdx + 1)<IPV6_NXTHOP_ENTRY_NUM)&&
					     (rtl865x_ipv6_nxtHopTable[entryIdx+1].valid==0)) {
						entry = &rtl865x_ipv6_nxtHopTable[entryIdx];
						goto found;
					}
					break;
					
				case IPV6_NEXTHOP_DEFREDIRECT_ACL:
					entry = &rtl865x_ipv6_nxtHopTable[entryIdx];
					goto found;
					break;
					
				default:
					printk("ipv6: attr(%d) is not support.....\n",attr);
					break;
			}
		}
	}

	/*if not found proper nextHop entry, return*/
	entry = NULL;

found:
	if(entry == NULL)
		return RTL_ENOFREEBUFFER;

	entry->valid = 1;
	entry->dstNetif = netif;
	entry->entryIndex = entryIdx;
	entry->nextHopType = netif->if_type;
	entry->refCnt = 1;
	entry->flag = attr;

	switch(netif->if_type)
	{
		case IF_ETHER:
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
		case IF_6RD:
#endif
#endif
			memcpy(&entry->un.nexthop, &nexthop, sizeof(inv6_addr_t));
			break;
		case IF_PPPOE:
		case IF_PPTP:
		case IF_L2TP:
			/*nexthop is sessionId*/
			entry->un.pppSessionId = nexthop.v6_addr32[0];
			break;
		
	}

	if (attr == IPV6_NEXTHOP_L3) {
		entry1 = &rtl865x_ipv6_nxtHopTable[entryIdx+1];
		memcpy(entry1, entry, sizeof(rtl8198c_ipv6_nextHopEntry_t));
		entry1->entryIndex = entryIdx + 1;

		_rtl8198c_arrangeIpv6NxtHop(entryIdx, 2);

		/*entry1 used netif,update reference netif*/
		rtl865x_referNetif(netif->name);
		/*entry1 used pppoe!, update reference pppoe*/
		if((entry1->nextHopType==IF_PPPOE) || (entry1->nextHopType==IF_PPTP) || (entry1->nextHopType==IF_L2TP)) {
			#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
			_rtl865x_referPpp(nexthop.v6_addr32[0]);
			#else
			rtl865x_referPpp(nexthop.v6_addr32[0]);
			#endif
		}
		
		/*FIXME_hyking:lazy, update the route information right here....*/
		rt_t = (rtl8198c_ipv6_route_t *)ref_ptr;
		rt_t ->un.nxthop.nxtHopSta = entryIdx;
		rt_t ->un.nxthop.nxtHopEnd = entryIdx + 1;
	}
	else {
		_rtl8198c_arrangeIpv6NxtHop(entryIdx, 1);
	}
	
	/*update reference dstnetif&pppoe arp?*/
	rtl865x_referNetif(netif->name);
	if((entry1->nextHopType==IF_PPPOE) || (entry1->nextHopType==IF_PPTP) || (entry1->nextHopType==IF_L2TP)){
		#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
		_rtl865x_referPpp(nexthop.v6_addr32[0]);
		#else
		rtl865x_referPpp(nexthop.v6_addr32[0]);
		#endif
	}
	
	return SUCCESS;
}

static int32 _rtl8198c_delIpv6NxtHop(uint32 attr, uint32 entryIdx)
{
	rtl8198c_ipv6_nextHopEntry_t *entry;

	if (entryIdx >= IPV6_NXTHOP_ENTRY_NUM)
		return RTL_EINVALIDINPUT;

	entry = &rtl865x_ipv6_nxtHopTable[entryIdx];

	if(entry->valid == 0)
		return RTL_EENTRYNOTFOUND;

	if (entry->refCnt > 1) {
		printk("%s(%d),refcnt(%d)\n", __FUNCTION__, __LINE__, entry->refCnt);
		return SUCCESS;
	}

	if((entry->nextHopType==IF_PPPOE) || (entry->nextHopType==IF_PPTP) || (entry->nextHopType==IF_L2TP)){
		#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
		_rtl865x_deReferPpp(entry->un.pppSessionId);
		#else
		rtl865x_deReferPpp(entry->un.pppSessionId);
		#endif
	}

	rtl865x_deReferNetif(entry->dstNetif->name);	
	memset(entry, 0, sizeof(rtl8198c_ipv6_nextHopEntry_t));

	/*jwj: Here not call _rtl8198c_arrangeIpv6NxtHop,because nexthop entry
	is used for default router, and default router's nexthop will always exist.*/
	/*update asic nextHop table*/
	//_rtl8198c_arrangeIpv6NxtHop(entryIdx,1);

	return SUCCESS;
	
}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
int32 _rtl865x_eventHandle_6rd_addArp(rtl865x_arpMapping_entry_t * arp)
{
	rtl8198c_ipv6_nextHopEntry_t *entry;
	int32 i;
	int32 sixrdIndex=FAILED;
	rtl865x_6rd_s * sixrdEntry=NULL;
	if(arp == NULL)
		return EVENT_CONTINUE_EXECUTE;
	
	entry = rtl865x_ipv6_nxtHopTable;
	for(i = 0; i < IPV6_NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_6RD)
		{
			/*update nexthop*/
			if(entry->un.nexthop.v6_addr32[0] != 0 ||
			   entry->un.nexthop.v6_addr32[1] != 0 ||
			   entry->un.nexthop.v6_addr32[2] != 0 ||
			   entry->un.nexthop.v6_addr32[3] != 0)
			{
				sixrdIndex=_rtl865x_getIpv66RDEntryByName(entry->dstNetif->name);
				if(sixrdIndex!=FAILED)
					sixrdEntry=_rtl865x_getIpv66RDEntryByIndex(sixrdIndex);
		
				if(sixrdEntry)
				{
					if(arp->ip == br_ip_addr)
				    {
						_rtl8198c_synIpv6NxtHopToAsic(entry);
					}
				}
			}
		}
	}
	return EVENT_CONTINUE_EXECUTE;
}

int32 _rtl865x_eventHandle_6rd_delArp(rtl865x_arpMapping_entry_t * arp)
{
	rtl8198c_ipv6_nextHopEntry_t *entry;
	int32 i;
	int32 sixrdIndex=FAILED;
	rtl865x_6rd_s * sixrdEntry=NULL;
	
	if(arp == NULL)
		return EVENT_CONTINUE_EXECUTE;
	
	entry = rtl865x_ipv6_nxtHopTable;
	for(i = 0; i < IPV6_NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_6RD)
		{
			/*update nexthop*/
			sixrdIndex=_rtl865x_getIpv66RDEntryByName(entry->dstNetif->name);
			if(sixrdIndex!=FAILED)
				sixrdEntry=_rtl865x_getIpv66RDEntryByIndex(sixrdIndex);
			
			if(sixrdEntry)
			{
				if(arp->ip == br_ip_addr)
				{
					/*reset the gw ipv6 address to reset the default gw L3 nexthop info*/
					br_ip_addr = 0;
					_rtl8198c_synIpv6NxtHopToAsic(entry);
				}
			}
		}
	}
	return EVENT_CONTINUE_EXECUTE;
}
#endif
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
extern int32 _rtl865x_eventHandle_addIpv6Arp(rtl8198c_ipv6_arpMapping_entry_t * arp);
extern int32 _rtl865x_eventHandle_delIpv6Arp(rtl8198c_ipv6_arpMapping_entry_t * arp);
#endif
#endif

static int32 _rtl8198c_eventHandle_addIpv6Arp(void *param)
{
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	extern int dslite_hw_fw ;
#endif
#endif

	int32 i;
	rtl8198c_ipv6_arpMapping_entry_t *arp;
	rtl8198c_ipv6_nextHopEntry_t *entry;
	
	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl8198c_ipv6_arpMapping_entry_t *)param;
	entry = rtl865x_ipv6_nxtHopTable;
	for (i = 0; i < IPV6_NXTHOP_ENTRY_NUM; i++, entry++)
	{
		/*skip ipv6 nexthop update to let ipv6 routing packet trap to cpu!!!!*/
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
		if(!dslite_hw_fw)
		#endif
		#endif
		if (entry->valid && (entry->nextHopType == IF_ETHER)) {
			/*update nexthop*/
			if (memcmp(&entry->un.nexthop, &arp->ip, sizeof(inv6_addr_t))==0)
				_rtl8198c_synIpv6NxtHopToAsic(entry);
		}	
		
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	/*for dslite tunnel to update nexthop info*/
	_rtl865x_eventHandle_addIpv6Arp(arp);
#endif
#endif
	
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl8198c_eventHandle_delIpv6Arp(void *param)
{
	int32 i;
	rtl8198c_ipv6_arpMapping_entry_t *arp;
	rtl8198c_ipv6_nextHopEntry_t *entry;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl8198c_ipv6_arpMapping_entry_t *)param;
	
	entry = rtl865x_ipv6_nxtHopTable;
	for (i = 0; i < IPV6_NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if (entry->valid && (entry->nextHopType == IF_ETHER)) {
			/*update nexthop*/
			if (memcmp(&entry->un.nexthop, &arp->ip, sizeof(inv6_addr_t))==0)
				_rtl8198c_synIpv6NxtHopToAsic(entry);
		}		
	}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	/*for dslite tunnel to update nexthop info*/
	_rtl865x_eventHandle_delIpv6Arp(arp);
#endif
#endif
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl8198c_eventHandle_delIpv6Ppp(void *param)
{
	int32 i;
	rtl865x_ppp_t *pppoe;
	rtl8198c_ipv6_nextHopEntry_t *entry;
	
	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	pppoe = (rtl865x_ppp_t *)param;

	entry = rtl865x_ipv6_nxtHopTable;
	for (i = 0; i < IPV6_NXTHOP_ENTRY_NUM; i++, entry++)
	{
		if((entry->valid) && ((entry->nextHopType == IF_PPPOE) ||
		    (entry->nextHopType == IF_PPTP)||(entry->nextHopType == IF_L2TP))) {
				/*update nexthop*/
				if (entry->un.pppSessionId == pppoe->sessionId) {
					/*update the action(TOCPU)*/
					entry->un.pppSessionId  = 0;
					_rtl8198c_synIpv6NxtHopToAsic(entry);
				}
			}		
	}

	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl8198c_ipv6_nextHop_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_DEL_IPV6_ARP;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = _rtl8198c_eventHandle_delIpv6Arp;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam, 0, sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_ADD_IPV6_ARP;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = _rtl8198c_eventHandle_addIpv6Arp;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam, 0, sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_DEL_PPP;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = _rtl8198c_eventHandle_delIpv6Ppp;
	rtl865x_registerEvent(&eventParam);
	
	return SUCCESS;

}

static int32 _rtl8198c_ipv6_nextHop_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_DEL_IPV6_ARP;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = _rtl8198c_eventHandle_delIpv6Arp;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam, 0, sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_ADD_IPV6_ARP;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = _rtl8198c_eventHandle_addIpv6Arp;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam, 0, sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_DEL_PPP;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = _rtl8198c_eventHandle_delIpv6Ppp;
	rtl865x_unRegisterEvent(&eventParam);
	
	return SUCCESS;

}


/*
@func int32 | rtl8198c_addIpv6NxtHop |add a nexthop entry
@parm uint32 | attr | attribute. IPV6_NEXTHOP_L3/IPV6_NEXTHOP_REDIRECT.
@parm void* | ref_ptr | entry pointer who refer this nexthop entry.
@parm rtl865x_netif_local_t* | netif | network interface.
@parm inv6_addr_t | nexthop | nexthop. ipv6 ip address when linktype is ethernet, session id when linktype is ppp session based.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl8198c_addIpv6NxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, inv6_addr_t nexthop)
{
	int32 ret = FAILED;
	unsigned long flags=0;	
	
	SMP_LOCK_ETH_HW(flags);
	ret = _rtl8198c_addIpv6NxtHop(attr, ref_ptr, netif, nexthop);	
	SMP_UNLOCK_ETH_HW(flags);
	
	return ret;
}

/*
@func int32 | rtl8198c_delIpv6NxtHop |delete nexthop entry
@parm uint32 | attr | attribute. IPV6_NEXTHOP_L3/IPV6_NEXTHOP_REDIRECT.
@parm uint32 | entryIdx | entry index.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl8198c_delIpv6NxtHop(uint32 attr, uint32 entryIdx)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl8198c_delIpv6NxtHop(attr, entryIdx);
	SMP_UNLOCK_ETH_HW(flags);
	
	return retval;	
}
