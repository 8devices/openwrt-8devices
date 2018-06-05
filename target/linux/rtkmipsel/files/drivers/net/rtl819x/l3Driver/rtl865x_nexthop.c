/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : nexthop table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_nexthop.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl_errno.h"
#include "common/rtl865x_netif_local.h" 
#include "rtl865x_ip.h"
#include "rtl865x_nexthop.h"
//#include "rtl865x_ppp.h"
#include "rtl865x_ppp_local.h"
#include "rtl865x_route.h"
#include "rtl865x_arp.h"

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
#include "rtl8198c_arpIpv6.h"
#include "rtl8198c_nexthopIpv6.h"
#endif

#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#ifdef CONFIG_RTL_HW_DSLITE_SUPPORT
#include "l3Driver/rtl865x_dslite.h"
extern inv6_addr_t gw_ipv6_addr;
#endif
#endif

static rtl865x_nextHopEntry_t *rtl865x_nxtHopTable;

#define NEXTHOP_TABLE_INDEX(entry)	(entry - rtl865x_nxtHopTable)
static int32 _rtl865x_nextHop_register_event(void);
static int32 _rtl865x_nextHop_unRegister_event(void);
static int32 _rtl865x_delNxtHop(uint32 attr, uint32 entryIdx);

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static RTL_DECLARE_MUTEX(nxthop_sem);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
extern int32 _rtl865x_referPpp(uint32 sessionId);
extern int32 _rtl865x_deReferPpp(uint32 sessionId);
#endif

#if 0
static void _print_softNxtHop(void)
{
	int32 i = 0;
	rtl865x_nextHopEntry_t *entry = rtl865x_nxtHopTable;
	printk("software nextHop table:\n");
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		printk("idx(%d): valid(%d),dst(%s),nexthop(%d),nexthopType(%d),refCnt(%d)\n",i,entry->valid,entry->dstNetif->name,
			entry->nexthop,entry->nextHopType,entry->refCnt);
	}
}
#endif

/*
@func int32 | rtl865x_initNxtHopTable |initialize the nexthop table
@rvalue SUCCESS | success.
@rvalue FAILED | failed. system should reboot.
@comm	
*/
int32 rtl865x_initNxtHopTable(void)
{
	TBL_MEM_ALLOC(rtl865x_nxtHopTable, rtl865x_nextHopEntry_t, NXTHOP_ENTRY_NUM);	
	memset(rtl865x_nxtHopTable,0,sizeof(rtl865x_nextHopEntry_t)*NXTHOP_ENTRY_NUM);
	_rtl865x_nextHop_register_event();
	return SUCCESS;	
}

/*
@func int32 | rtl865x_reinitNxtHopTable |reinitialize the nexthop table
@rvalue SUCCESS | success.
@comm	
*/
int32 rtl865x_reinitNxtHopTable(void)
{
	int32 i;
	_rtl865x_nextHop_unRegister_event();

	for(i = 0; i < NXTHOP_ENTRY_NUM; i++)
	{
		if(rtl865x_nxtHopTable[i].valid)
			_rtl865x_delNxtHop(NEXTHOP_L3,i);
	}
	
	_rtl865x_nextHop_register_event();
	return SUCCESS;
}
#if defined CONFIG_RTL_LOCAL_PUBLIC
extern  int rtl865x_getLocalPublicArpMapping(unsigned int ip, rtl865x_arpMapping_entry_t * arp_mapping);
#endif
static int32 _rtl865x_synNxtHopToAsic(rtl865x_nextHopEntry_t *entry_t)
{
	rtl865x_tblAsicDrv_nextHopParam_t asic;
	ether_addr_t reservedMac;
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif;
	uint32 fid = 0;
	uint32 columIdx = 0;
	int32 retval = 0;
	int32 ipIdx = 0;

	retval = rtl865x_getReserveMacAddr(&reservedMac);
	
	bzero(&asic, sizeof(rtl865x_tblAsicDrv_nextHopParam_t));

	if(entry_t == NULL)
	{
		return RTL_EINVALIDINPUT;
	}
	
	if (entry_t->nextHopType == IF_ETHER)
	{
		rtl865x_arpMapping_entry_t arp_t;
		int32 ret_arpFound = FAILED;

		/*if the arp info of nexthop is not found, reserved to cpu Mac is used for trap packet to CPU*/
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
		if(entry_t->nexthop)
		{
			ret_arpFound = rtl865x_getArpMapping(entry_t->nexthop,&arp_t);
			if(ret_arpFound!=SUCCESS)
			{
				/*try to match local pulbic ip*/
				ret_arpFound = rtl865x_getLocalPublicArpMapping(entry_t->nexthop, &arp_t);
				
			}
		}
#else
		if(entry_t->nexthop)
			ret_arpFound = rtl865x_getArpMapping(entry_t->nexthop,&arp_t);
#endif		
		
		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		retval = rtl865x_Lookup_fdb_entry(fid, (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
		
		asic.nextHopRow = rtl8651_filterDbIndex( (ret_arpFound == SUCCESS)? &arp_t.mac : &reservedMac, fid );
		asic.nextHopColumn = (retval == SUCCESS)? columIdx: 0;
		asic.isPppoe = FALSE;
		asic.pppoeIdx = 0;
		
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)

	else if (entry_t->nextHopType == IF_DSLT)
	{
		rtl8198c_ipv6_arpMapping_entry_t arp_t;
		int32 ret_arpFound = FAILED;
		/*if the arp info of nexthop is not found, reserved to cpu Mac is used for trap packet to CPU*/

		ret_arpFound = rtl8198c_dslite_Ipv6ArpMapping(gw_ipv6_addr, &arp_t);
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
		
		if(entry_t->nexthop)
			retval = rtl865x_getPppBySessionId(entry_t->nexthop,&pppoe);

		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		

		retval =rtl865x_Lookup_fdb_entry(fid, (pppoe.valid)? &pppoe.server_mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);

		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if (retval == FAILED) {
			retval =rtl865x_Lookup_fdb_entry(fid, (pppoe.valid)? &pppoe.server_mac : &reservedMac, FDB_STATIC, &columIdx, &asic_l2);
			//panic_printk("1: rtl865x_Lookup_fdb_entry ret is %d.....\n", retval);
		}
		#endif
		
		asic.nextHopRow = rtl8651_filterDbIndex( (pppoe.valid)? &pppoe.server_mac : &reservedMac, fid);
		asic.nextHopColumn = (pppoe.valid)? columIdx: 0;
		asic.isPppoe = (pppoe.type == IF_PPPOE)? TRUE: FALSE;

		retval = rtl865x_getPppIdx(&pppoe, &pppidx);

		//printk("%s(%d): pppoeIdx(%d), pppoeType(%d), pppoevalid(%d),pppoeSid(%d)\n",__FUNCTION__,__LINE__,pppidx,pppoe.type,pppoe.valid,pppoe.sessionId);
		asic.pppoeIdx	= (pppoe.type == IF_PPPOE)? pppidx: 0;
	}
	
	if(entry_t->dstNetif->is_slave == 1)
	{
		dstNetif = entry_t->dstNetif->master;

		if(dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	}
	else
		dstNetif = entry_t->dstNetif;

	if(dstNetif == NULL)
		printk("_%s(%d), BUG!!!!!!",__FUNCTION__,__LINE__);
	
	asic.dvid 			= dstNetif->asicIdx;

	if(entry_t->srcIp)
		retval = rtl865x_getIpIdxByExtIp(entry_t->srcIp, &ipIdx);
	
	asic.extIntIpIdx	= entry_t->srcIp? ipIdx: 0;
	rtl8651_setAsicNextHopTable(entry_t->entryIndex,  &asic);
	
	return SUCCESS;
}


static int32 _rtl865x_arrangeNxtHop(uint32 start, uint32 num)
{
	int32 idx;
	rtl865x_nextHopEntry_t *entry = NULL;

	if(start < 0 || start + num >= NXTHOP_ENTRY_NUM)
		return RTL_EINVALIDINPUT;
	for(idx = start; idx < start + num; idx++)
	{
		entry = &rtl865x_nxtHopTable[idx];

		if(entry->valid)
			_rtl865x_synNxtHopToAsic(entry);
	}
	
	return SUCCESS;
}

static int32 _rtl865x_addNxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, uint32 nexthop,uint32 srcIp)
{
	int entryIdx;
	rtl865x_nextHopEntry_t *entry = NULL, *entry1 = NULL;
	rtl865x_route_t *rt_t = NULL;

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
	for(entryIdx = 0; entryIdx < NXTHOP_ENTRY_NUM; entryIdx++)
	{
		if(rtl865x_nxtHopTable[entryIdx].valid == 0)
		{
			switch(attr)
			{
				case NEXTHOP_L3:
					if( entryIdx%2 == 0 && (entryIdx + 1) < NXTHOP_ENTRY_NUM &&  rtl865x_nxtHopTable[entryIdx+1].valid == 0)
					{
						entry = &rtl865x_nxtHopTable[entryIdx];
						goto found;
					}
					break;
					
				case NEXTHOP_DEFREDIRECT_ACL:
					entry = &rtl865x_nxtHopTable[entryIdx];
					goto found;
					break;
					
				default:
					printk("attr(%d) is not support.....\n",attr);
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
	entry->srcIp = srcIp;
	entry->refCnt = 1;
	entry->flag = attr;

	switch(netif->if_type)
	{
		case IF_ETHER:
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
		case IF_DSLT:
	#endif
	#endif
			entry->nexthop = nexthop;
			break;
		case IF_PPPOE:
		case IF_PPTP:
		case IF_L2TP:
			/*nexthop is sessionId*/
			entry->nexthop = nexthop;
			break;
		
	}
	
	if(attr == NEXTHOP_L3)
	{
		entry1 = &rtl865x_nxtHopTable[entryIdx+1];
		memcpy(entry1,entry,sizeof(rtl865x_nextHopEntry_t));
		entry1->entryIndex = entryIdx + 1;

		_rtl865x_arrangeNxtHop(entryIdx, 2);

		/*entry1 used netif,update reference netif*/
		rtl865x_referNetif(netif->name);
		/*entry1 used pppoe!, update reference pppoe*/
		if((entry1->nextHopType == IF_PPPOE)
			|| (entry1->nextHopType == IF_PPTP)
			|| (entry1->nextHopType == IF_L2TP)
			){
			#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
			_rtl865x_referPpp(nexthop);
			#else
			rtl865x_referPpp(nexthop);
			#endif
		}
		
		/*FIXME_hyking:lazy, update the route information right here....*/
		rt_t = (rtl865x_route_t *)ref_ptr;
		rt_t ->un.nxthop.nxtHopSta = entryIdx;
		rt_t ->un.nxthop.nxtHopEnd = entryIdx + 1;
	}
	else
		_rtl865x_arrangeNxtHop(entryIdx, 1);
	
	/*update reference dstnetif&pppoe arp?*/
	rtl865x_referNetif(netif->name);
	if((entry->nextHopType == IF_PPPOE)
			|| (entry->nextHopType == IF_PPTP)
			|| (entry->nextHopType == IF_L2TP)
			){
			#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
			_rtl865x_referPpp(nexthop);
			#else
			rtl865x_referPpp(nexthop);
			#endif
	}
	
	return SUCCESS;
	
}

static int32 _rtl865x_delNxtHop(uint32 attr, uint32 entryIdx)
{
	rtl865x_nextHopEntry_t *entry;
	//int32 retval = 0;

	if(entryIdx >= NXTHOP_ENTRY_NUM)
		return RTL_EINVALIDINPUT;

	entry = &rtl865x_nxtHopTable[entryIdx];

	if(entry->valid == 0)
		return RTL_EENTRYNOTFOUND;

	if(entry->refCnt > 1)
	{
		printk("%s(%d),refcnt(%d)\n",__FUNCTION__,__LINE__,entry->refCnt);
		return SUCCESS;
	}

	/*now delete the entry*/
	//if(entry->srcIp_t)
		//retval = rtl865x_delIp(entry->srcIp_t->extIp);

	if((entry->nextHopType == IF_PPPOE)
			|| (entry->nextHopType == IF_PPTP)
			|| (entry->nextHopType == IF_L2TP)
			)
	{
		
		#if LINUX_VERSION_CODE > KERNEL_VERSION(3,4,0)
		_rtl865x_deReferPpp(entry->nexthop);
		#else
		rtl865x_deReferPpp(entry->nexthop);
		#endif
		
	}

	rtl865x_deReferNetif(entry->dstNetif->name);	
	memset(entry,0,sizeof(rtl865x_nextHopEntry_t));
	
	/*update asic nextHop table*/
	//_rtl865x_arrangeNxtHop(entryIdx,1);

	return SUCCESS;
	
}

#if defined(CONFIG_RTL_MULTIPLE_WAN)  ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
static int32 _rtl865x_synNxtHop_by_arp_entry(rtl865x_nextHopEntry_t *entry_t,rtl865x_arpMapping_entry_t *arp)
{
	rtl865x_tblAsicDrv_nextHopParam_t asic;
	ether_addr_t reservedMac;
	ether_addr_t zero_mac ={{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
	rtl865x_tblAsicDrv_l2Param_t asic_l2;
	rtl865x_netif_local_t *dstNetif;
	uint32 fid = 0;
	uint32 columIdx = 0;
	int32 retval = 0;
	int32 ipIdx = 0;

	retval = rtl865x_getReserveMacAddr(&reservedMac);
	
	bzero(&asic, sizeof(rtl865x_tblAsicDrv_nextHopParam_t));

	if(entry_t == NULL || arp == NULL)
	{
		return RTL_EINVALIDINPUT;
	}
	
	if (entry_t->nextHopType == IF_ETHER)
	{
		int mac_valid = 0;
		mac_valid = memcmp(arp->mac.octet,zero_mac.octet,ETHER_ADDR_LEN);
		
		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		retval = rtl865x_Lookup_fdb_entry(fid, (mac_valid)? &arp->mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
		
		asic.nextHopRow = rtl8651_filterDbIndex( (mac_valid)? &arp->mac : &reservedMac, fid );
		asic.nextHopColumn = (retval == SUCCESS)? columIdx: 0;

		asic.isPppoe = FALSE;
		asic.pppoeIdx = 0;
	}
	else
	{	
		/*session based interface type*/
		rtl865x_ppp_t pppoe;		
		int32 pppidx = 0;

		memset(&pppoe,0,sizeof(rtl865x_ppp_t));
		
		if(entry_t->nexthop)
			retval = rtl865x_getPppBySessionId(entry_t->nexthop,&pppoe);

		rtl865x_getVlanFilterDatabaseId(entry_t->dstNetif->vid,&fid);
		

		retval =rtl865x_Lookup_fdb_entry(fid, (pppoe.valid)? &pppoe.server_mac : &reservedMac, FDB_DYNAMIC, &columIdx,&asic_l2);
			
		asic.nextHopRow = rtl8651_filterDbIndex( (pppoe.valid)? &pppoe.server_mac : &reservedMac, fid);
		asic.nextHopColumn = (pppoe.valid)? columIdx: 0;
		asic.isPppoe = (pppoe.type == IF_PPPOE)? TRUE: FALSE;

		retval = rtl865x_getPppIdx(&pppoe, &pppidx);

		//printk("%s(%d): pppoeIdx(%d), pppoeType(%d), pppoevalid(%d),pppoeSid(%d)\n",__FUNCTION__,__LINE__,pppidx,pppoe.type,pppoe.valid,pppoe.sessionId);
		asic.pppoeIdx	= (pppoe.type == IF_PPPOE)? pppidx: 0;
	}
	
	if(entry_t->dstNetif->is_slave == 1)
	{
		dstNetif = entry_t->dstNetif->master;

		if(dstNetif == NULL)
			dstNetif = _rtl865x_getDefaultWanNetif();
	}
	else
		dstNetif = entry_t->dstNetif;

	if(dstNetif == NULL)
		printk("_%s(%d), BUG!!!!!!",__FUNCTION__,__LINE__);
	
	asic.dvid			= dstNetif->asicIdx;

	if(entry_t->srcIp)
		retval = rtl865x_getIpIdxByExtIp(entry_t->srcIp, &ipIdx);
	
	asic.extIntIpIdx	= entry_t->srcIp? ipIdx: 0;

	//printk("%s(%d), entryIdx(%d),asic.isPPPoe(%d),asic.pppoeIdx(%d),asic.dvid(%d)\n", __FUNCTION__,__LINE__,entry_t->entryIndex,asic.isPppoe,asic.pppoeIdx,asic.dvid);
	rtl8651_setAsicNextHopTable(entry_t->entryIndex,  &asic);
	
	return SUCCESS;
}

int32 rtl865x_eventHandle_addArp_for_multiWan(void *param)
{
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;
	
	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;
	entry = rtl865x_nxtHopTable;
		
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_ETHER)
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
			{				
				_rtl865x_synNxtHop_by_arp_entry(entry,arp);
			}
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}

int32 rtl865x_eventHandle_delArp_for_multiWan(void *param)
{
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;	
	entry = rtl865x_nxtHopTable;
	
	//set mac to 0 for sync nexthop
	memset(arp->mac.octet,0,ETHER_ADDR_LEN);
	
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_ETHER)
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
				_rtl865x_synNxtHop_by_arp_entry(entry,arp);
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}

#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
extern int32 _rtl865x_eventHandle_6rd_addArp(rtl865x_arpMapping_entry_t * arp);
extern int32 _rtl865x_eventHandle_6rd_delArp(rtl865x_arpMapping_entry_t * arp);
#endif
#endif

static int32 _rtl865x_eventHandle_addArp(void *param)
{
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	extern int sixrd_hw_fw ;
#endif
#endif
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;

	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		/*skip ipv6 nexthop update to let ipv6 routing packet trap to cpu!!!!*/
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
		if(!sixrd_hw_fw)
		{
#endif
#endif
		if(entry->valid && entry->nextHopType == IF_ETHER)
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
				_rtl865x_synNxtHopToAsic(entry);
		}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
		}
#endif
#endif
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	_rtl865x_eventHandle_6rd_addArp(arp);
#endif
#endif
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_eventHandle_delArp(void *param)
{
	rtl865x_arpMapping_entry_t *arp;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	arp = (rtl865x_arpMapping_entry_t *)param;
	
	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if((entry->valid && entry->nextHopType == IF_ETHER))
		{
			/*update nexthop*/
			if(entry->nexthop == arp->ip)
				_rtl865x_synNxtHopToAsic(entry);
		}		
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	_rtl865x_eventHandle_6rd_delArp(arp);
#endif
#endif
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_eventHandle_delPpp(void *param)
{
	rtl865x_ppp_t *pppoe;
	rtl865x_nextHopEntry_t *entry;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	pppoe = (rtl865x_ppp_t *)param;

	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid )
			if((entry->nextHopType == IF_PPPOE)
			|| (entry->nextHopType == IF_PPTP)
			|| (entry->nextHopType == IF_L2TP)
			)
			{
				/*update nexthop*/
				if(entry->nexthop == pppoe->sessionId)
				{
					/*update the action(TOCPU)*/
					entry->nexthop  = 0;
					_rtl865x_synNxtHopToAsic(entry);
				}
			}		
	}

	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_nextHop_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delArp;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_addArp;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_PPP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delPpp;
	rtl865x_registerEvent(&eventParam);
	
	return SUCCESS;

}

static int32 _rtl865x_nextHop_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delArp;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_addArp;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_PPP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=_rtl865x_eventHandle_delPpp;
	rtl865x_unRegisterEvent(&eventParam);
	
	return SUCCESS;

}


/*
@func int32 | rtl865x_addNxtHop |add a nexthop entry
@parm uint32 | attr | attribute. NEXTHOP_L3/NEXTHOP_REDIRECT.
@parm void* | ref_ptr | entry pointer who refer this nexthop entry.
@parm rtl865x_netif_local_t* | netif | network interface.
@parm uint32 | nexthop | nexthop. ip address when linktype is ethernet, session id when linktype is ppp session based.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_addNxtHop(uint32 attr, void *ref_ptr, rtl865x_netif_local_t *netif, uint32 nexthop,uint32 srcIp)
{
	int32 ret = FAILED;
	unsigned long flags=0;	
	//rtl_down_interruptible(&nxthop_sem);	
	SMP_LOCK_ETH_HW(flags);
	ret = _rtl865x_addNxtHop(attr,ref_ptr,netif,nexthop,srcIp);	
	//rtl_up(&nxthop_sem);	
	SMP_UNLOCK_ETH_HW(flags);
	return ret;
}

/*
@func int32 | rtl865x_delNxtHop |delete nexthop entry
@parm uint32 | attr | attribute. NEXTHOP_L3/NEXTHOP_REDIRECT.
@parm uint32 | entryIdx | entry index.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_delNxtHop(uint32 attr, uint32 entryIdx)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	//rtl_down_interruptible(&nxthop_sem);
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_delNxtHop(attr,entryIdx);
	//rtl_up(&nxthop_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return retval;	
}

#if defined (CONFIG_RTL_LOCAL_PUBLIC) || defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)||defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int32 rtl865x_getNxtHopIdx(uint32 attr, rtl865x_netif_local_t *netif, uint32 nexthop)
{
	rtl865x_nextHopEntry_t *entry;
	int32 i;
	
	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(	(entry->valid) &&
			(entry->flag == attr)&&
			(entry->dstNetif == netif) &&
			(entry->nexthop== nexthop))
		{
			return i;
		}		
	}
	return -1;
}

#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
int32 _rtl865x_eventHandle_addIpv6Arp(rtl8198c_ipv6_arpMapping_entry_t * arp)
{
	rtl865x_nextHopEntry_t *entry;
	int32 i;
	int32 dsltIndex=FAILED;
	rtl865x_dslite_s * dsltEntry=NULL;
	if(arp == NULL)
		return EVENT_CONTINUE_EXECUTE;
	
	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{	
		if(entry->valid && entry->nextHopType == IF_DSLT)
		{
			/*update nexthop*/
			if(entry->nexthop )
			{
				dsltIndex=_rtl865x_getIpv6DsLiteEntryByName(entry->dstNetif->name);
				if(dsltIndex!=FAILED)
					dsltEntry=_rtl865x_getIpv6DsLiteEntryByIndex(dsltIndex);
		
				if(dsltEntry)
				{						
					if(memcmp(&gw_ipv6_addr, &arp->ip, sizeof(inv6_addr_t))==0)
				    {						
						_rtl865x_synNxtHopToAsic(entry);
					}	
				}
			}
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}

int32 _rtl865x_eventHandle_delIpv6Arp(rtl8198c_ipv6_arpMapping_entry_t * arp)
{
	rtl865x_nextHopEntry_t *entry;
	int32 i;
	int32 dsltIndex=FAILED;
	rtl865x_dslite_s * dsltEntry=NULL;
	
	if(arp == NULL)
		return EVENT_CONTINUE_EXECUTE;
	
	entry = rtl865x_nxtHopTable;
	for(i = 0; i < NXTHOP_ENTRY_NUM; i++,entry++)
	{
		if(entry->valid && entry->nextHopType == IF_DSLT)
		{
			/*update nexthop*/
			dsltIndex=_rtl865x_getIpv6DsLiteEntryByName(entry->dstNetif->name);
			if(dsltIndex!=FAILED)
				dsltEntry=_rtl865x_getIpv6DsLiteEntryByIndex(dsltIndex);
			
			if(dsltEntry)
			{
				if(memcmp(&gw_ipv6_addr, &arp->ip, sizeof(inv6_addr_t))==0)
				{
					/*reset the gw ipv6 address to reset the default gw L3 nexthop info*/
					memset(&gw_ipv6_addr,0,sizeof(inv6_addr_t));
					_rtl865x_synNxtHopToAsic(entry);
				}
			}
		}		
	}
	return EVENT_CONTINUE_EXECUTE;
}
#endif
#endif

#ifdef CONFIG_RTL_PROC_NEW
int32 sw_nexthop_read(struct seq_file *s, void *v)
{


	uint32 idx;
	rtl865x_nextHopEntry_t *sw_nxthop = NULL;

	seq_printf(s, "%s\n", "SW Next Hop Table:\n");
	for(idx = 0; idx < NXTHOP_ENTRY_NUM; idx++)
	{
		if(rtl865x_nxtHopTable[idx].valid == 0)
			continue;
		#if 0
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

}
#endif		
		sw_nxthop =&rtl865x_nxtHopTable[idx];
		seq_printf(s,"  [%d]  entryIndex(%d)  type(%d) netif(%s) srcIp(%x)  nextHop(%x) \n", idx,
			sw_nxthop->entryIndex,sw_nxthop->nextHopType,sw_nxthop->dstNetif->name,
			sw_nxthop->srcIp, sw_nxthop->nexthop);
	}

	return 0;
}
#else
int32 sw_nexthop_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl865x_nextHopEntry_t *sw_nxthop=NULL;

	uint32 idx;

	len = sprintf(page, "%s\n", "SW Next Hop Table:\n");
	for(idx = 0; idx < NXTHOP_ENTRY_NUM; idx++)
	{
		if(rtl865x_nxtHopTable[idx].valid == 0)
			continue;
		sw_nxthop =&rtl865x_nxtHopTable[idx];
		len += sprintf(page + len,"  [%d]  entryIndex(%d)  type(%d) netif(%s) srcIp(%d)  nextHop(%d) n", idx,
			sw_nxthop->nextHopType,sw_nxthop->entryIndex,sw_nxthop->dstNetif->name,
			sw_nxthop->srcIp, sw_nxthop->nexthop);
	}

	return len;
}
#endif

int32 sw_nexthop_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
