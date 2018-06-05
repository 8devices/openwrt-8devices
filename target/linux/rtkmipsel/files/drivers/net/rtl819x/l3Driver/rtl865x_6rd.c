/*
* Copyright c                  Realtek Semiconductor Corporation, 2014  
* All rights reserved.
* 
* Program : 6rd table driver
* Abstract : 
* Author : hualai_gu
*/

/*      @doc RTL_LAYEREDDRV_API

        Copyright <cp>2014 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.
*/

#include <linux/netdevice.h>

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/ip_tunnels.h>
#include "common/rtl_errno.h"
#include "common/rtl865x_netif_local.h" 
#include "rtl865x_ip.h"
#include "rtl865x_nexthop.h"
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

#if defined(CONFIG_RTL_8198C) ||defined(CONFIG_RTL_8197F)
#include "rtl8198c_arpIpv6.h"
#include "rtl8198c_nexthopIpv6.h"
#endif

#include "rtl865x_6rd.h"

static int32 _rtl865x_updateIpv66RDEntry(rtl865x_6rd_s *tunnel_param, int entryIdx);
static int32 _rtl865x_updateIpv66RDEntryByBR(int entryIdx);

/*ipv6 6rd*/
#define IPV6_6RD_TABLE_INDEX(entry)	(entry - rtl865x_ipv6_6RDTable)
#define IPV6_6RD_ENTRY_NUM	8

static rtl865x_6rd_s *rtl865x_ipv6_6RDTable = NULL;
ipaddr_t br_ip_addr;

int32 rtl8198c_initIpv66RDTable(void)
{
	TBL_MEM_ALLOC(rtl865x_ipv6_6RDTable, rtl865x_6rd_s, IPV6_6RD_ENTRY_NUM);
	memset(rtl865x_ipv6_6RDTable,0,sizeof(rtl865x_6rd_s)*IPV6_6RD_ENTRY_NUM);
	br_ip_addr = 0;
	return SUCCESS;
}

int32 rtl8198c_reinitIpv66RDTable(void)
{
	int32 i;
	
	for(i = 0; i < IPV6_6RD_ENTRY_NUM; i++)
	{
		if(rtl865x_ipv6_6RDTable[i].valid)
			_rtl865x_delIpv66RDEntry(i);
	}
	br_ip_addr = 0;

	return SUCCESS;
}

rtl865x_6rd_s * rtl865x_get6RDEntry(void)
{
	int32 entryIdx;	
	rtl865x_6rd_s *entry = NULL;	
	
	for (entryIdx = 0; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)
	{
		entry=&rtl865x_ipv6_6RDTable[entryIdx];
		if(entry)
		{
			if (entry->valid == 1)
			{
				return entry;
			}
		}
	}
	return NULL;
}

rtl865x_6rd_s * _rtl865x_getIpv66RDEntryByIndex(int32 entryIdx)
{
	rtl865x_6rd_s *entry = NULL;

	entry=&rtl865x_ipv6_6RDTable[entryIdx];
	if(entry)
	{
		if (entry->valid == 1)		
		{
			return entry;
		}
	}
	
	return NULL;
}

int32 _rtl865x_getIpv66RDEntryByName(char * devName)
{
	int32 entryIdx;
	rtl865x_6rd_s *entry = NULL;
	int32 ret=FAILED;
	int32 retVal=FAILED;
	rtl8198C_tblAsicDrv_6rdParam_t sixrd_entry;
	rtl8198C_tblAsicDrv_6rdParam_t *asic_entry =&sixrd_entry;
	
	if(devName==NULL)
		goto out;
	
	for (entryIdx = 0; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)
	{
		entry=&rtl865x_ipv6_6RDTable[entryIdx];
		if(entry)
		{
			if ((entry->valid == 1)
				&& (strcmp(devName,entry->name)==0)
				)
			{
				goto found;
			}
		}
	}
	entry =NULL;
	
	if(entry ==NULL)
		goto out;
	
found:
	ret=rtl8198C_getAsic6rdTable(entryIdx, asic_entry);
	//printk("%s %d: found! entryIdx: %d, ret: %d, valid: %d\n", __FUNCTION__, __LINE__, entryIdx, ret, entry->valid);
	if(ret == SUCCESS)
		retVal = entryIdx;

out:
	return retVal;
}

int32 _rtl865x_getIpv66RDEntrybyIndex(uint32 entryIdx,rtl865x_6rd_s *entry)
{
	int32 ret=FAILED;
	entry=&rtl865x_ipv6_6RDTable[entryIdx];
	
	if (entry->valid == 1 )
	{
		ret =SUCCESS;
	}
	
	return ret;
}

static int32 _rtl865x_synIpv66RDToAsic(rtl865x_6rd_s *entry_t)
{
	rtl8198C_tblAsicDrv_6rdParam_t asic_entry;
	uint32 index;
	int ret = FAILED;
	if(entry_t == NULL)
		return FAILED;

	asic_entry.ce_ip_addr					= entry_t->ce_ip_addr;
	asic_entry.ce_ip_mask_len				= entry_t->ce_ip_mask_len;
	asic_entry.six_rd_prefix.v6_addr32[0]	= entry_t->six_rd_prefix.v6_addr32[0];
	asic_entry.six_rd_prefix.v6_addr32[1]	= entry_t->six_rd_prefix.v6_addr32[1];
	asic_entry.six_rd_prefix.v6_addr32[2]	= entry_t->six_rd_prefix.v6_addr32[2];
	asic_entry.six_rd_prefix.v6_addr32[3]	= entry_t->six_rd_prefix.v6_addr32[3];
	asic_entry.six_rd_prefix_len			= entry_t->six_rd_prefix_len;
	asic_entry.br_ip_addr					= entry_t->br_ip_addr;
	asic_entry.br_ip_mask_len				= entry_t->br_ip_mask_len;
	asic_entry.mtu							= entry_t->mtu;
	asic_entry.valid						= entry_t->valid;
	index									= entry_t->index;
	
	ret = rtl8198C_setAsic6rdTable(index, &asic_entry);
	return ret;
}

static void _rtl865x_tunnelTo6RDparam(struct ip_tunnel *t, rtl865x_6rd_s *tunnel_param)
{
	tunnel_param->ce_ip_addr					= htonl(t->parms.iph.saddr);
	tunnel_param->ce_ip_mask_len				= 0;
	tunnel_param->six_rd_prefix.v6_addr32[0]	= htonl(t->ip6rd.prefix.s6_addr32[0]);
	tunnel_param->six_rd_prefix.v6_addr32[1]	= htonl(t->ip6rd.prefix.s6_addr32[1]);
	tunnel_param->six_rd_prefix.v6_addr32[2]	= htonl(t->ip6rd.prefix.s6_addr32[2]);
	tunnel_param->six_rd_prefix.v6_addr32[3]	= htonl(t->ip6rd.prefix.s6_addr32[3]);
	tunnel_param->six_rd_prefix_len				= t->ip6rd.prefixlen;
	tunnel_param->mtu							= t->dev->mtu;
	tunnel_param->br_ip_mask_len				= 24;
	memcpy(tunnel_param->name, t->dev->name, MAX_IFNAMESIZE);	
}

int32 _rtl865x_addIpv66RDEntry(struct ip_tunnel *t)
{
	int retVal;
	int entryIdx=FAILED;
	rtl865x_6rd_s param;
	rtl865x_6rd_s *entry = NULL, *tunnel_param = &param;
	
	//entry = _rtl865x_getIpv6DsLiteEntry(host_ipv6_addr,host_ipv6_mask,aftr_ipv6_addr,aftr_ipv6_mask,devName);
	memset(tunnel_param, 0, sizeof(rtl865x_6rd_s));
	_rtl865x_tunnelTo6RDparam(t, tunnel_param);
	entryIdx = _rtl865x_getIpv66RDEntryByName(tunnel_param->name);
	if(entryIdx!=FAILED)
	{
		_rtl865x_updateIpv66RDEntry(tunnel_param, entryIdx);
		entry = &rtl865x_ipv6_6RDTable[entryIdx];
		goto sync;
	}
	else
	{
		/* Allocate an empty entry for new one */
		/*
				pppoe ----> index 0
				direct ---> index 1
				nexthop ---> index 2
				arp ------> index 3
		*/
		//now test just for direct ,so use index 1 to use.
		//for (entryIdx = 0; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)			
		//for (entryIdx = 1; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)
		for (entryIdx = 2; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)
		{
			if (rtl865x_ipv6_6RDTable[entryIdx].valid == 0) {		
				entry = &rtl865x_ipv6_6RDTable[entryIdx];
				goto found;
				break;			
			}
		}
	}

	/*if not found proper nextHop entry, return*/
	entry = NULL;

found:
	if(entry == NULL)
		return RTL_ENOFREEBUFFER;
			
	/*common information*/
	memcpy(entry, tunnel_param, sizeof(rtl865x_6rd_s));
	entry->valid = 1;
	entry->index = entryIdx;

sync:	
	retVal = _rtl865x_synIpv66RDToAsic(entry);
#if 0
	printk("[%s]:[%d].retVal:%d, index:%d, ce: %x/%d, 6rd prefix: %x-%x-%x-%x/%d, br: %x/%d, if: %s, mtu: %d\n",
		__FUNCTION__,__LINE__,retVal, entryIdx, entry->ce_ip_addr, entry->ce_ip_mask_len,
		entry->six_rd_prefix.v6_addr32[0], entry->six_rd_prefix.v6_addr32[1],
		entry->six_rd_prefix.v6_addr32[2], entry->six_rd_prefix.v6_addr32[3],
		entry->six_rd_prefix_len, entry->br_ip_addr, entry->br_ip_mask_len,
		entry->name, entry->mtu);
#endif
	return retVal;
}

int32 _rtl865x_addIpv66RDEntryByBR(char *devname)
{
	int retVal;
	int entryIdx=FAILED;
	rtl865x_6rd_s *entry = NULL;

	entryIdx = _rtl865x_getIpv66RDEntryByName(devname);
	if(entryIdx!=FAILED)
	{
		_rtl865x_updateIpv66RDEntryByBR(entryIdx);
		entry = &rtl865x_ipv6_6RDTable[entryIdx];
		goto sync;
	}
	else
	{
		/* Allocate an empty entry for new one */
		/*
				pppoe ----> index 0
				direct ---> index 1
				nexthop ---> index 2
				arp ------> index 3
		*/
		//now test just for direct ,so use index 1 to use.
		//for (entryIdx = 0; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)			
		//for (entryIdx = 1; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)
		for (entryIdx = 2; entryIdx < IPV6_6RD_ENTRY_NUM; entryIdx++)
		{
			if (rtl865x_ipv6_6RDTable[entryIdx].valid == 0) {		
				entry = &rtl865x_ipv6_6RDTable[entryIdx];
				goto found;
				break;			
			}
		}
	}

	/*if not found proper nextHop entry, return*/
	entry = NULL;

found:
	if(entry == NULL)
		return RTL_ENOFREEBUFFER;
			
	/*common information*/
	memcpy(entry->name, devname, MAX_IFNAMESIZE);
	entry->br_ip_addr = br_ip_addr;
	entry->br_ip_mask_len = 24;
	entry->valid = 1;
	entry->index = entryIdx;

sync:
	retVal = _rtl865x_synIpv66RDToAsic(entry);
#if 0
	printk("[%s]:[%d].retVal:%d, index: %d, ce: %x/%d, 6rd prefix: %x-%x-%x-%x/%d, br: %x/%d, if: %s, mtu: %d\n",
		__FUNCTION__,__LINE__,retVal, entryIdx, entry->ce_ip_addr, entry->ce_ip_mask_len,
		entry->six_rd_prefix.v6_addr32[0], entry->six_rd_prefix.v6_addr32[1],
		entry->six_rd_prefix.v6_addr32[2], entry->six_rd_prefix.v6_addr32[3],
		entry->six_rd_prefix_len, entry->br_ip_addr, entry->br_ip_mask_len,
		entry->name, entry->mtu);
#endif
	return retVal;
}

static int32 _rtl865x_updateIpv66RDEntry(rtl865x_6rd_s *tunnel_param, int entryIdx)
{
	rtl865x_6rd_s *entry = NULL;

	if(tunnel_param == NULL)
		return FAILED;

	if(entryIdx < 0 || entryIdx >= IPV6_6RD_ENTRY_NUM)
		return FAILED;

	entry = &rtl865x_ipv6_6RDTable[entryIdx];

	if(tunnel_param->ce_ip_addr != 0)
		entry->ce_ip_addr = tunnel_param->ce_ip_addr;
	if(tunnel_param->ce_ip_mask_len != 0)
		entry->ce_ip_mask_len = tunnel_param->ce_ip_mask_len;
	if(tunnel_param->six_rd_prefix.v6_addr32[0] != 0 ||
		tunnel_param->six_rd_prefix.v6_addr32[1] != 0 ||
		tunnel_param->six_rd_prefix.v6_addr32[2] != 0 ||
		tunnel_param->six_rd_prefix.v6_addr32[3] != 0)
	{
		entry->six_rd_prefix.v6_addr32[0] = tunnel_param->six_rd_prefix.v6_addr32[0];
		entry->six_rd_prefix.v6_addr32[1] = tunnel_param->six_rd_prefix.v6_addr32[1];
		entry->six_rd_prefix.v6_addr32[2] = tunnel_param->six_rd_prefix.v6_addr32[2];
		entry->six_rd_prefix.v6_addr32[3] = tunnel_param->six_rd_prefix.v6_addr32[3];
	}
	if(tunnel_param->six_rd_prefix_len != 0)
		entry->six_rd_prefix_len = tunnel_param->six_rd_prefix_len;
	if(tunnel_param->br_ip_addr != 0)
		entry->br_ip_addr = tunnel_param->br_ip_addr;
	if(tunnel_param->br_ip_mask_len != 0)
		entry->br_ip_mask_len = tunnel_param->br_ip_mask_len;
	if(tunnel_param->mtu != 0)
		entry->mtu = tunnel_param->mtu;
	
	return SUCCESS;
}

static int32 _rtl865x_updateIpv66RDEntryByBR(int entryIdx)
{
	rtl865x_6rd_s *entry = NULL;

	if(entryIdx < 0 || entryIdx >= IPV6_6RD_ENTRY_NUM)
		return FAILED;

	entry = &rtl865x_ipv6_6RDTable[entryIdx];

	entry->br_ip_addr = br_ip_addr;
	entry->br_ip_mask_len = 24;

	return SUCCESS;
}

int32 _rtl865x_delIpv66RDEntry(uint32 entryIdx)
{
	return 0;
}
