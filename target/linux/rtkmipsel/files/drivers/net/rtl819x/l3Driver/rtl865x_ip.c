/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : ip table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_ip.c - RTL865x Home gateway controller Layered driver API documentation       |
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
#include "common/rtl_errno.h"
//#include "rtl_utils.h"
//#include <net/rtl/rtl865x_ip_api.h>
#include "rtl865x_ip.h"
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#include <net/rtl/rtl865x_localPublic.h>
#endif
#include "common/rtl865x_eventMgr.h" /*call back function....*/

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL3.h"	
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif

static rtl865x_ip_entry_t *rtl865x_ipTable;

#define IP_TABLE_INDEX(entry)	(entry - rtl865x_ipTable)
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static RTL_DECLARE_MUTEX(ip_sem);
#endif
static int32 _rtl865x_delIp(ipaddr_t extIp);

/*
@func int32 | rtl865x_initIpTable | initialize ip table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int32 rtl865x_initIpTable(void)
{
	TBL_MEM_ALLOC(rtl865x_ipTable, rtl865x_ip_entry_t, IP_NUMBER);	
	memset(rtl865x_ipTable,0,sizeof(rtl865x_ip_entry_t)*IP_NUMBER);
	return SUCCESS;	
}

/*
@func int32 | rtl865x_initIpTable | reinitialize ip table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed.
*/
int32 rtl865x_reinitIpTable(void)
{
	int32 i ;
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid)
			_rtl865x_delIp(rtl865x_ipTable[i].extIp);
	}
	return SUCCESS;
}


static int32 _rtl865x_addIp(ipaddr_t intIp, ipaddr_t extIp, uint32 ip_type)
{
	int i;
	rtl865x_ip_entry_t *entry = NULL;
	rtl865x_tblAsicDrv_extIntIpParam_t asicIp;
	int32 retval = FAILED;
#if defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
	int empty_entry = -1;
#endif

	if(ip_type < IP_TYPE_NAPT || ip_type > IP_TYPE_LOCALSERVER)
		return RTL_EINVALIDINPUT;

	/*duplicate check*/
	/*found a valid entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 0)
		{
#if defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
			if (empty_entry == -1)
				empty_entry = i;
#else
			entry = &rtl865x_ipTable[i];
			break;
#endif
		}
		else
		{				
			if(rtl865x_ipTable[i].extIp == extIp)
				return RTL_EENTRYALREADYEXIST;

			if(ip_type == IP_TYPE_NAPT && rtl865x_ipTable[i].type == IP_TYPE_NAPT && rtl865x_ipTable[i].defNaptIp == 1)
				return RTL_EENTRYALREADYEXIST;
		}		
	}

#if defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
	if (empty_entry != -1)
		entry = &rtl865x_ipTable[empty_entry];
#endif

	if(entry == NULL)
		return RTL_ENOFREEBUFFER;

		
	/*update releated information*/
	entry->valid = 1;
	entry->intIp = intIp;
	entry->extIp = extIp;
	entry->type = ip_type;
		
	/*from 865xC, this field is invalid...*/
	entry->nexthop = NULL;


	/*add this ip entry to asic*/
	/* Set asic */
	bzero(&asicIp, sizeof(rtl865x_tblAsicDrv_extIntIpParam_t));
	asicIp.extIpAddr	= extIp;
	asicIp.intIpAddr	= intIp;
	asicIp.localPublic	= (ip_type == IP_TYPE_LOCALSERVER)? TRUE: FALSE;
	asicIp.nat			= (ip_type == IP_TYPE_NAT)? TRUE: FALSE;
	asicIp.nhIndex		= 0;
	
	retval = rtl8651_setAsicExtIntIpTable(IP_TABLE_INDEX(entry), &asicIp);

	if(ip_type == IP_TYPE_NAPT)
		rtl8651_setAsicOperationLayer(4);

	return SUCCESS;
	
}

static int32 _rtl865x_ipTableIsNull(void)
{
	int i;

	/*found the entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 1)
		{
			return FAILED;
		}
	}

	return SUCCESS;
}

static int32 _rtl865x_delIp(ipaddr_t extIp)
{
	int i;
	rtl865x_ip_entry_t *entry = NULL;

	/*found the entry*/
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 1 && rtl865x_ipTable[i].extIp == extIp)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	}

	/*update asic ip table*/
	if(entry!=NULL)
	{
		rtl8651_delAsicExtIntIpTable(IP_TABLE_INDEX(entry));	
		memset(entry,0,sizeof(rtl865x_ip_entry_t));
	}
	else
		return RTL_EENTRYNOTFOUND;
	
	//no entry in ip table, set operation to layer 2
	if(_rtl865x_ipTableIsNull() == SUCCESS)
	{
		#if defined(CONFIG_RTL_HARDWARE_MULTICAST)
			rtl8651_setAsicOperationLayer(3);
		#else
			rtl8651_setAsicOperationLayer(2);
		#endif
	}
	
	return SUCCESS;
	
}

static rtl865x_ip_entry_t* _rtl865x_getIpEntryByExtIp(ipaddr_t extIp)
{
	int32 i;
	rtl865x_ip_entry_t *entry = NULL;
	
	for(i = 0; i < IP_NUMBER; i++)
		if(rtl865x_ipTable[i].valid == 1 && rtl865x_ipTable[i].extIp == extIp)
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	return entry;
	
}

/*
@func rtl865x_ip_entry_t* | rtl865x_getIpEntryByIp | get ip entry .
@parm ipaddr_t | extIp | ip address
@rvalue ip_entry | Success.
@rvalue NULL | Failed
*/
rtl865x_ip_entry_t* rtl865x_getIpEntryByIp(ipaddr_t extIp)
{
	return _rtl865x_getIpEntryByExtIp(extIp);
}


/*
@func int32 | rtl865x_getIpIdxByExtIp | get asic idex .
@parm ipaddr_t | extIp | ip address
@parm int32* | idx | index
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed
*/
int32 rtl865x_getIpIdxByExtIp(ipaddr_t extIp, int32 *idx)
{
	int32 retval = FAILED;
	rtl865x_ip_entry_t *entry = NULL;

	entry = _rtl865x_getIpEntryByExtIp(extIp);
	if(entry)
	{
		if(idx)
			*idx = IP_TABLE_INDEX(entry);
		retval = SUCCESS;
	}

	return retval;
}

/*
@func int32 | rtl865x_addIp | add ip table entry.
@parm ipaddr_t | intIp | internal ip address
@parm ipaddr_t | extIp | external ip address
@parm uint32 | ip_type | entry type. support IP_TYPE_NAPT/IP_TYPE_NAT/IP_TYPE_LOCALSERVER
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | Invalid input.
@rvalue RTL_EENTRYALREADYEXIST | entry already exist.
@rvalue RTL_ENOFREEBUFFER | not enough buffer in System.
@rvalue FAILED | Failed.
@comm
the extIp is the primary key of the ip table.
*/
int32 rtl865x_addIp(ipaddr_t intIp, ipaddr_t extIp, uint32 ip_type)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	//rtl_down_interruptible(&ip_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_addIp(intIp, extIp, ip_type);
	//rtl_up(&ip_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;

}

/*
@func int32 | rtl865x_delIp | delete ip table entry.
@parm ipaddr_t | extIp | external ip address
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | Invalid input.
@rvalue RTL_EENTRYNOTFOUND | not found.
@rvalue FAILED | Failed.
@comm
the extIp is the primary key of the ip table.
*/
int32 rtl865x_delIp(ipaddr_t extIp)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&ip_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delIp(extIp);
	//rtl_up(&ip_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
	
}

int32 rtl865x_getIPIdx(rtl865x_ip_entry_t *entry, int32 *idx)
{
	if(idx)
	{
		*idx =IP_TABLE_INDEX(entry);
		return SUCCESS;
	}
	return FAILED;
}

#if defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
extern int rtl_check_extIp_exist_in_ps(uint32 extIp);
int32 rtl_del_extIp_not_exist_in_ps(void)
{
	int i, found = 0;
	
	for (i=0; i<IP_NUMBER; i++)
	{
		found = 0;
		if (rtl865x_ipTable[i].valid == 1) {
			if (rtl_check_extIp_exist_in_ps(rtl865x_ipTable[i].extIp) == SUCCESS)
				found = 1;

			/*Delete the ip not exist in protocol stack*/
			if (found == 0) {
				rtl865x_delIp(rtl865x_ipTable[i].extIp);
			}
		}
	}

	return SUCCESS;
}
#endif
/*
@func int32 | rtl865x_getIpIdxByIpRange | get asic idex .
@parm ipaddr_t | ip | ip address
@parm ipaddr_t | mask | ip address
@parm int32* | idx | index
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed
*/
int32 rtl865x_getIpIdxByIpRange(ipaddr_t ip, ipaddr_t mask, int32 *idx)
{
	int32 retval = FAILED;
	int32 i;
	rtl865x_ip_entry_t *entry = NULL;

	*idx = 0; //default point to index 0
	for(i = 0; i < IP_NUMBER; i++)
	{
		if(rtl865x_ipTable[i].valid == 1 && ((rtl865x_ipTable[i].extIp & mask) == (ip&mask)))
		{
			entry = &rtl865x_ipTable[i];
			break;
		}
	}

	if(entry)
	{
		if(idx)
			*idx = IP_TABLE_INDEX(entry);
		retval = SUCCESS;
	}

	return retval;
}


