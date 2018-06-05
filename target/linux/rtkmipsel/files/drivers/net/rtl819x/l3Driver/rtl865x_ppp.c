/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : ppp table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_ppp.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include <net/rtl/rtl865x_ppp.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl865x_fdb_api.h>

#include "common/rtl_errno.h"
#include "common/rtl865x_netif_local.h"
#include "rtl865x_ppp_local.h"

#include "common/rtl865x_eventMgr.h" /*call back function....*/

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif

#include "common/rtl865x_vlan.h"
#include "l2Driver/rtl865x_fdb.h"

static rtl865x_ppp_t *rtl865x_pppTable;
#if defined(CONFIG_RTL_HARDWARE_NAT)
static rtl865x_ppp_t rtl865x_pppTable_bak[PPP_NUMBER];
#endif

#define PPP_TABLE_INDEX(entry)	(entry - rtl865x_pppTable)
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static RTL_DECLARE_MUTEX(ppp_sem);
#endif

static int32 _rtl865x_ppp_register_event(void);
static int32 _rtl865x_ppp_unRegister_event(void);
static int32 _rtl865x_delPpp(uint32 sessionId);

/*
@func int32 | rtl865x_initPppTable | initialize ppp table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed.
*/
int32 rtl865x_initPppTable(void)
{
	TBL_MEM_ALLOC(rtl865x_pppTable, rtl865x_ppp_t, PPP_NUMBER);	
	memset(rtl865x_pppTable,0,sizeof(rtl865x_ppp_t)*PPP_NUMBER);

	/*event regist....*/
	_rtl865x_ppp_register_event();
	return SUCCESS;		
}

/*
@func int32 | rtl865x_reinitPppTable | reinitialize ppp table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed.
*/
int32 rtl865x_reinitPppTable(void)
{
	int32 i;
	_rtl865x_ppp_unRegister_event();

	for(i = 0; i < PPP_NUMBER; i++)
	{
		if(rtl865x_pppTable[i].valid == 1)
			_rtl865x_delPpp(rtl865x_pppTable[i].sessionId);
	}
	
	_rtl865x_ppp_register_event();
	return SUCCESS;
}



static int32 _rtl865x_delPpp(uint32 sessionId)
{
	int i;
	rtl865x_ppp_t *entry = NULL;
	rtl865x_tblAsicDrv_pppoeParam_t asicppp;
	int32 retval = FAILED;
	uint32 fid;

	/*found the entry*/
	for(i = 0; i < PPP_NUMBER; i++)
	{
		if(rtl865x_pppTable[i].valid == 1 && rtl865x_pppTable[i].sessionId== sessionId)
		{
			entry = &rtl865x_pppTable[i];
			break;
		}
	}

	if (entry==NULL)
		return SUCCESS;
	/*check reference count*/
	if(entry->refCnt > 1)
	{
		printk("Please del other table entry which referenced me...ppp sessionId(%d)\n",sessionId);
		return RTL_EREFERENCEDBYOTHER;
	}

	
	/*del asic ppp table*/
	asicppp.age = 0;
	asicppp.sessionId = 0;
	rtl8651_setAsicPppoe(PPP_TABLE_INDEX(entry), &asicppp);

	/*FIXME_hyking:dereference netif & mac*/
	retval = rtl865x_deReferNetif(entry->netif->name);

	retval = rtl865x_getVlanFilterDatabaseId(entry->netif->vid, &fid);
	
	retval = rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, fid, &entry->server_mac);


	/*raise event??*/
	rtl865x_raiseEvent(EVENT_DEL_PPP,(void *)entry);

	memset(entry,0,sizeof(rtl865x_ppp_t));
	

	return SUCCESS;
	
}
static int32 _rtl865x_addPpp(uint8 *ifname, ether_addr_t *mac, uint32 sessionId, int32 type)
{
	int i;
	rtl865x_netif_local_t *netif;
	rtl865x_ppp_t *entry = NULL;
	rtl865x_tblAsicDrv_pppoeParam_t asicPpp;
	rtl865x_tblAsicDrv_l2Param_t fdbEntry;
	rtl865x_filterDbTableEntry_t	 l2temp_entry;	
	uint32 fid,column;
	int32 retval;
	uint32 fdb_type[]={ FDB_STATIC, FDB_DYNAMIC };
	
/*printk("%s(%d): ifname(%s),mac(%02x:%02x:%02x:%02x:%02x:%02x),sid(%d),type(%d)",__FUNCTION__,__LINE__,ifname,
	mac->octet[0],mac->octet[1],mac->octet[2],mac->octet[3],mac->octet[4],mac->octet[5], sessionId,type);*/

#if 1	//Do not add hw ppp info in wisp or bridge mode
	{
		extern int rtl865x_curOpMode;
		if ((rtl865x_curOpMode==1) ||(rtl865x_curOpMode==2)) // BRIDGE_MODE or WISP_MODE
			return RTL_ENETIFINVALID;
	}
#endif
	/*duplicate check*/
	for(i = 0; i < PPP_NUMBER; i++)
	{
		if(rtl865x_pppTable[i].valid && rtl865x_pppTable[i].sessionId == sessionId)
			return RTL_EENTRYALREADYEXIST;
	}	

	netif = _rtl865x_getSWNetifByName(ifname);
	if(netif == NULL)
		return RTL_ENETIFINVALID;

	if(netif->if_type != IF_PPPOE)
		return RTL_ELINKTYPESHOULDBERESET;

	/*found a valid entry*/
	for(i = 0; i < PPP_NUMBER; i++)
		if(rtl865x_pppTable[i].valid == 0)
		{
			entry = &rtl865x_pppTable[i];
			break;
		}		
	
	if(entry == NULL)
		return RTL_ENOFREEBUFFER;
	

		
	/*update releated information*/
	entry->valid = 1;
	entry->netif = netif;
	memcpy(&entry->server_mac,mac,sizeof(ether_addr_t));
	entry->sessionId = sessionId;
	entry->type = type;
	entry->refCnt = 1;
		
	/*add this ip entry to asic*/
	/* Set asic */
	bzero(&asicPpp, sizeof(rtl865x_tblAsicDrv_pppoeParam_t));
	asicPpp.sessionId = sessionId;
	asicPpp.age = 300;
	
	rtl8651_setAsicPppoe(PPP_TABLE_INDEX(entry), &asicPpp);


	/*FIXME_hyking:reference netif & mac*/
	retval = rtl865x_referNetif(netif->name);

	/*add fdb entry...*/
	fid = 0;
	column = 0;
	retval = rtl865x_getVlanFilterDatabaseId(netif->vid,&fid);

	for(i = 0; i < 2; i++)
	{
		/*
		printk("%s:%d\n,fid(%d),mac(%02x:%02x:%02x:%02x:%02x:%02x)\n",__FUNCTION__,__LINE__,fid,mac->octet[0],mac->octet[1],
			mac->octet[2],mac->octet[3],mac->octet[4],mac->octet[5]);
		 
		printk("%s:%d\n",__FUNCTION__,__LINE__);
		*/
		if(rtl865x_Lookup_fdb_entry(fid, mac, fdb_type[i], &column,&fdbEntry) != SUCCESS)
		{	
			continue;
		}

		
		/*in case of layer2 auto learn, add hardware entry to layer 2 software table*/
		l2temp_entry.l2type = (fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII;
		l2temp_entry.process = FDB_TYPE_FWD;
		l2temp_entry.memberPortMask = fdbEntry.memberPortMask;
		l2temp_entry.auth = fdbEntry.auth;
		l2temp_entry.SrcBlk = fdbEntry.srcBlk;
		memcpy(&(l2temp_entry.macAddr), mac, sizeof(ether_addr_t));
		rtl865x_addFilterDatabaseEntryExtension(fid, &l2temp_entry);		
//		retval = _rtl865x_addFilterDatabaseEntry((fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII, fid, mac, FDB_TYPE_FWD, fdbEntry.memberPortMask, fdbEntry.auth,fdbEntry.srcBlk);
		rtl865x_refleshHWL2Table(mac, FDB_DYNAMIC|FDB_STATIC,fid);
	}

	/*raise event??*/
	rtl865x_raiseEvent(EVENT_ADD_PPP, (void*)entry);

	return SUCCESS;
	
}

static int32 _rtl865x_eventHandle_delNetif(void *para)
{
	rtl865x_netif_local_t *netif;
	rtl865x_ppp_t *entry = NULL;
	int32 i = 0;
	if(para == NULL)
		return EVENT_CONTINUE_EXECUTE;
	
	netif = (rtl865x_netif_local_t *)para;

	entry = rtl865x_pppTable;
	for(i = 0 ; i < PPP_NUMBER;i++,entry++)
	{
		#if !defined(CONFIG_OPENWRT_SDK)
		if((entry->valid) && entry->netif == netif)
			printk("%s(%d): BUG....deleted netif is referenced by ppp table!\n",__FUNCTION__,__LINE__);
		#endif
	}
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_eventHandle_delL2Fdb(void * param)
{
	rtl865x_filterDbTableEntry_t  *fdbEntry;
	rtl865x_ppp_t *entry = NULL;
	int32 i;

	if(param == NULL)
		return EVENT_CONTINUE_EXECUTE;

	fdbEntry = (rtl865x_filterDbTableEntry_t *)param;

	entry = rtl865x_pppTable;
	for(i = 0; i < PPP_NUMBER; i++,entry++)
	{
		//if((entry->valid) && memcmp(&entry->server_mac,&fdbEntry->macAddr,sizeof(ether_addr_t)) == 0)
			//printk("%s(%d): BUG....deleted l2 fdb is referenced by ppp table!\n",__FUNCTION__,__LINE__);
	}
	
	return EVENT_CONTINUE_EXECUTE;
}

static int32 rtl865x_ppp_eventHandle_delNetif(void *para)
{
	int32 retval = EVENT_CONTINUE_EXECUTE;
	//rtl_down_interruptible(&ppp_sem);
	retval = _rtl865x_eventHandle_delNetif(para);
	//rtl_up(&ppp_sem);
	return retval;
}

static int32 rtl865x_ppp_eventHandle_delL2Fdb(void *para)
{
	int32 retval = EVENT_CONTINUE_EXECUTE;
	//rtl_down_interruptible(&ppp_sem);
	retval = _rtl865x_eventHandle_delL2Fdb(para);	
	//rtl_up(&ppp_sem);
	return retval;
}


static int32 _rtl865x_ppp_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_ppp_eventHandle_delL2Fdb;
	rtl865x_registerEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_ppp_eventHandle_delNetif;
	rtl865x_registerEvent(&eventParam);
	
	return SUCCESS;

}

static int32 _rtl865x_ppp_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_ppp_eventHandle_delL2Fdb;
	rtl865x_unRegisterEvent(&eventParam);

	memset(&eventParam,0,sizeof(rtl865x_event_Param_t));	
	eventParam.eventLayerId=DEFAULT_COMMON_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_NETIF;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_ppp_eventHandle_delNetif;
	rtl865x_unRegisterEvent(&eventParam);
	
	return SUCCESS;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
static 
#endif
int32 _rtl865x_referPpp(uint32 sessionId)
{
	int i;
	rtl865x_ppp_t *entry = NULL;

	entry = rtl865x_pppTable;
	for(i = 0; i < PPP_NUMBER; i++,entry++)
		if(entry->valid && (entry->sessionId == sessionId))
		{
			entry->refCnt++;
			return SUCCESS;
		}
	return FAILED;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
static 
#endif 
int32 _rtl865x_deReferPpp(uint32 sessionId)
{
	int i;
	rtl865x_ppp_t *entry = NULL;

	entry = rtl865x_pppTable;
	for(i = 0; i < PPP_NUMBER; i++,entry++)
		if(entry->valid && (entry->sessionId == sessionId))
		{
			entry->refCnt--;
			return SUCCESS;
		}
	return FAILED;
}

/*
@func int32 | rtl865x_referPpp |refer ppp table entry.
@parm uint32 | sessionId | ppp session ID
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_referPpp(uint32 sessionId)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	//rtl_down_interruptible(&ppp_sem);
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_referPpp(sessionId);
	//rtl_up(&ppp_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return retval;
}
/*
@func int32 | rtl865x_deReferPpp |dereference ppp table entry.
@parm uint32 | sessionId | ppp session ID
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_deReferPpp(uint32 sessionId)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	//rtl_down_interruptible(&ppp_sem);
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_deReferPpp(sessionId);
	//rtl_up(&ppp_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return retval;
}
	
/*
@func int32 | rtl865x_addPpp |add ppp session information.
@parm uint8* | ifname | network interface name
@parm ether_addr_t* | mac | MAC Address of ppp session server
@parm uint32 | sessionId | ppp session ID
@parm int32 | type | connect type. should be IF_PPPOE/IF_PPTP/IF_L2TP
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYALREADYEXIST | entry already exist.
@rvalue RTL_ELINKTYPESHOULDBERESET | link type is error.
@rvalue RTL_ENOFREEBUFFER | no enough memory buffer in system.
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_addPpp(uint8 *ifname, ether_addr_t *mac, uint32 sessionId, int32 type)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	//printk("====================%s(%d): sessionId(%d) ifname(%s) retval(%d)\n",__FUNCTION__,__LINE__,sessionId,ifname,retval);
	//rtl_down_interruptible(&ppp_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_addPpp(ifname, mac, sessionId, type);
	//rtl_up(&ppp_sem);
	SMP_UNLOCK_ETH(flags);
	//printk("====================%s(%d): sessionId(%d) retval(%d)\n",__FUNCTION__,__LINE__,sessionId,retval);
	return retval;

}

/*
@func int32 | rtl865x_delPpp |delete ppp session information.
@parm uint32 | sessionId | ppp session ID
@rvalue SUCCESS | Success.
@rvalue RTL_EREFERENCEDBYOTHER | entry is refered by other table entry.
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_delPpp(uint32 sessionId)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	//rtl_down_interruptible(&ppp_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delPpp(sessionId);
	//rtl_up(&ppp_sem);
	SMP_UNLOCK_ETH(flags);
	//printk("====================%s(%d): sessionId(%d) retval(%d)\n",__FUNCTION__,__LINE__,sessionId,retval);
	return retval;
	
}

/*
@func int32 | rtl865x_delPppbyIfName |delete ppp session by network interface name.
@parm char* | name | network interface name.
@rvalue SUCCESS | Success.
@rvalue RTL_EREFERENCEDBYOTHER | entry is refered by other table entry.
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_delPppbyIfName(char *name)
{
	int i;
	rtl865x_ppp_t *entry = NULL;
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&ppp_sem);
	SMP_LOCK_ETH(flags);
	for(i = 0; i < PPP_NUMBER; i++)
		if(rtl865x_pppTable[i].valid && (memcmp(rtl865x_pppTable[i].netif->name,name,sizeof(name)) == 0))
		{
			entry = &rtl865x_pppTable[i];
			break;
		}

	if(entry)
		retval = _rtl865x_delPpp(entry->sessionId);
	//rtl_up(&ppp_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}

/*
@func int32 | rtl865x_getPppBySessionId |get ppp session information by session ID.
@parm uint32 | sessionId | ppp session ID
@parm rtl865x_ppp_t* | entry | ppp entry.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed
@comm	
*/
int32 rtl865x_getPppBySessionId(uint32 sessionId,rtl865x_ppp_t *entry)
{
	rtl865x_ppp_t *ppp = NULL;	
	int32 retval = FAILED;
	int i;
	
	for(i = 0; i < PPP_NUMBER; i++)
	{
		if(rtl865x_pppTable[i].valid && rtl865x_pppTable[i].sessionId== sessionId)
			ppp = &rtl865x_pppTable[i];
	}

	if(ppp && entry)
	{
		memcpy(entry,ppp,sizeof(rtl865x_ppp_t));
		retval = SUCCESS;
	}

	return retval;
		
}

/*
@func rtl865x_ppp_t* | rtl865x_getPppByNetifName |get ppp session information by network interface name.
@parm char* | name | network interface name.
@rvalue NULL | Failed
@comm	
*/
rtl865x_ppp_t* rtl865x_getPppByNetifName(char *name)
{
	rtl865x_ppp_t *ppp = NULL;	
	int i;
	
	for(i = 0; i < PPP_NUMBER; i++)
	{
		if(rtl865x_pppTable[i].valid && (memcmp(rtl865x_pppTable[i].netif->name,name,sizeof(name)) == 0))
		{
			ppp = &rtl865x_pppTable[i];
			break;
		}
	}

	return ppp;
	
}

/*
@func int32 | rtl865x_getPppIdx |get ppp session entry index.
@parm rtl865x_ppp_t* | ppp | ppp entry.
@parm int32* | idx | return value for index.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@comm	
*/
int32 rtl865x_getPppIdx(rtl865x_ppp_t *ppp, int32 *idx)
{
	int32 i = 0;
	rtl865x_ppp_t *entry;
	entry = rtl865x_pppTable;
	for(i = 0; i < PPP_NUMBER; i++,entry++)
	{
		if(entry->valid && entry->sessionId == ppp->sessionId)
		{
			if(idx)
				*idx = i;
			return SUCCESS;
		}		
	}
	
	return FAILED;
}

/*
int32 rtl865x_printPpp(void)
{
	int i;
	for(i = 0; i < PPP_NUMBER; i++)
	{
		printk("====idx(%d),rtl865x_pppTable.valid(%d),refcnt(%d),session(%d),type(%d)\n", i,rtl865x_pppTable[i].valid,rtl865x_pppTable[i].refCnt,rtl865x_pppTable[i].sessionId,rtl865x_pppTable[i].type);
	}
	return SUCCESS;
}
*/

#if defined(CONFIG_RTL_HARDWARE_NAT)
int rtl819x_save_hw_ppp_table(void)
{
	int i = 0;
	uint32 fid = RTL_WAN_FID;
	
	memset((void *)&rtl865x_pppTable_bak,0,sizeof(rtl865x_pppTable_bak));
	
	for(i = 0; i < PPP_NUMBER; i++)
	{
		if(rtl865x_pppTable[i].valid == 1)
		{
			rtl865x_pppTable_bak[i].netif = rtl865x_pppTable[i].netif;
			rtl865x_pppTable_bak[i].sessionId = rtl865x_pppTable[i].sessionId;
			rtl865x_pppTable_bak[i].valid = rtl865x_pppTable[i].valid;
			rtl865x_pppTable_bak[i].type = rtl865x_pppTable[i].type;
			rtl865x_pppTable_bak[i].refCnt = rtl865x_pppTable[i].refCnt;
			memcpy(rtl865x_pppTable_bak[i].server_mac.octet, rtl865x_pppTable[i].server_mac.octet, ETHER_ADDR_LEN);
			//save ppp server related l2 entry.
			rtl865x_getVlanFilterDatabaseId(rtl865x_pppTable[i].netif->vid,&fid);
			rtl819x_save_inused_l2_entry(0, (ether_addr_t *)&rtl865x_pppTable[i].server_mac, fid);
		}
	}

	return SUCCESS;
}
int rtl819x_restore_hw_ppp_table(void)
{
	int i = 0;
	rtl865x_ppp_t *entry = NULL;
	
	for(i = 0; i < PPP_NUMBER; i++)
	{
		if (rtl865x_pppTable_bak[i].valid == 1){
			entry = &rtl865x_pppTable_bak[i];
			rtl865x_addPpp(entry->netif->name, (ether_addr_t *)&entry->server_mac, entry->sessionId, entry->type);
		}
	}

	return SUCCESS;
}
#endif

