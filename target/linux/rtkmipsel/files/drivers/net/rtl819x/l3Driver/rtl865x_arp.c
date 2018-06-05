
/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_arp.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/

#include <net/rtl/rtl_types.h>
#include "common/rtl_errno.h"

#include "common/mbuf.h"
//#include "assert.h"

//#include "rtl865xc_swNic.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "rtl865xC_tblAsicDrv.h"
#include "common/rtl8651_aclLocal.h"
#endif

#include "AsicDriver/rtl865x_hwPatch.h"		/* define for chip related spec */

#include "common/rtl865x_eventMgr.h"

#include "common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"

#include "l2Driver/rtl865x_fdb.h"

#include "rtl865x_ppp_local.h"
#include "rtl865x_route.h"
#include "rtl865x_arp.h"

#if defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
#include <net/rtl/rtl865x_multipleWan_api.h>
#include "rtl865x_multipleWan.h"
#include "rtl865x_nexthop.h"
#endif
#include <net/rtl/rtl865x_fdb_api.h>

static int32 rtl865x_arp_hash(ipaddr_t ip, uint32 *index);

static int32 rtl865x_arp_callbackFn_for_del_fdb(void *param);
static int32 rtl865x_arp_register_event(void);

static struct rtl865x_arp_table arpTables;
#if defined(CONFIG_RTL_HARDWARE_NAT) //&&defined(REINIT_SWITCH_CORE)
static struct rtl865x_arp_table arpTables_bak;
#endif
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
static int32 rtl865x_arp_callbackFn_for_add_fdb(void *param);
static struct rtl865x_arp_table arpTables_all;
extern int  extPortEnabled;
#endif
#if defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
static int (*rtl_callback_for_ps_arp)(ipaddr_t ip,rtl865x_arpMapping_entry_t *arp);
#endif

static int32 rtl865x_arp_callbackFn_for_del_fdb(void *param)
{
	int i;
	rtl865x_filterDbTableEntry_t  *fdbEntry; 
	
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	
	fdbEntry=(rtl865x_filterDbTableEntry_t *)param;

	for(i=0;i<RTL8651_ARPTBL_SIZE;i++)
	{
		/*be careful of dead loop, the delete fdb event maybe caused by arp time out*/
		if(memcmp(&(fdbEntry->macAddr),&(arpTables.mappings[i].mac), 6)==0)
		{
			rtl865x_delArp(arpTables.mappings[i].ip);
		}
		
	}
	
	return EVENT_CONTINUE_EXECUTE;

}

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
static int32 rtl865x_arp_callbackFn_for_add_fdb(void *param)
{
	int i;
	rtl865x_filterDbTableEntry_t  *fdbEntry; 
	
	if (param == NULL)
		return EVENT_CONTINUE_EXECUTE;
	if(extPortEnabled==0)
		return EVENT_CONTINUE_EXECUTE;
	
	fdbEntry=(rtl865x_filterDbTableEntry_t *)param;

	for (i=0;i<RTL8651_ARPTBL_SIZE;i++)
	{
		/*be careful of dead loop, the delete fdb event maybe caused by arp time out*/
		if (memcmp(&(fdbEntry->macAddr), &(arpTables.mappings[i].mac), 6) == 0) {
			return EVENT_CONTINUE_EXECUTE;
		}
	}
#if 1
	for (i=0;i<RTL8651_ARPTBL_SIZE;i++)
	{
		/*be careful of dead loop, the delete fdb event maybe caused by arp time out*/
		if (memcmp(&(fdbEntry->macAddr),&(arpTables_all.mappings[i].mac), 6) == 0){
			#if 0
			printk("%s[%d], ip is 0x%x, mac is 0x%2x%2x%2x%2x%2x%2x\n", __FUNCTION__, __LINE__,
				arpTables_all.mappings[i].ip, arpTables_all.mappings[i].mac.octet[0],  arpTables_all.mappings[i].mac.octet[1], 
				 arpTables_all.mappings[i].mac.octet[2],  arpTables_all.mappings[i].mac.octet[3],  arpTables_all.mappings[i].mac.octet[4], 
				  arpTables_all.mappings[i].mac.octet[5]);
			#endif
			rtl865x_addArp(arpTables_all.mappings[i].ip, &(arpTables_all.mappings[i].mac));
		}
	}
#endif	
	return EVENT_CONTINUE_EXECUTE;


}

#endif


static int32 rtl865x_arp_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_arp_callbackFn_for_del_fdb;
	rtl865x_unRegisterEvent(&eventParam);

	#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	memset(&eventParam, 0, sizeof(rtl865x_event_Param_t));
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_arp_callbackFn_for_add_fdb;
	rtl865x_unRegisterEvent(&eventParam);
	#endif
	
	return SUCCESS;

}
static int32 rtl865x_arp_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_arp_callbackFn_for_del_fdb;
	rtl865x_registerEvent(&eventParam);

	#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	memset(&eventParam, 0, sizeof(rtl865x_event_Param_t));
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_arp_callbackFn_for_add_fdb;
	rtl865x_registerEvent(&eventParam);
	#endif
	
	return SUCCESS;

}


int32 rtl865x_arp_init(void)
{
	int i;
	rtl865x_arp_unRegister_event();
	memset(arpTables.allocBitmap, 0, 64);
	memset(arpTables.mappings, 0, RTL8651_ARPTBL_SIZE * sizeof(rtl865x_arpMapping_entry_t));
	for(i=0; i<RTL8651_ARPTBL_SIZE; i++)
	{
		rtl8651_delAsicArp(i);
	}
	rtl865x_arp_register_event();
	#if defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
	rtl_callback_for_ps_arp = NULL;
	#endif
	#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	memset(arpTables_all.allocBitmap, 0, 64);
	memset(arpTables_all.mappings, 0, RTL8651_ARPTBL_SIZE * sizeof(rtl865x_arpMapping_entry_t));
	#endif
	return SUCCESS;
}

int32 rtl865x_arp_reinit(void)
{
	return rtl865x_arp_init();
}

int32 rtl865x_arp_tbl_alloc(rtl865x_route_t *route)
{
	uint32 netSize, entry, bestSize=0, bestStartPos=0xffffffff;
	uint32 curSize, curStartPos;
	
	if(route==NULL)
	{
		return FAILED;
	}
	
  	/* process definition(000: PPPoE, 001: L2, 010: ARP, 100: CPU, 101: NextHop, 110: Drop)*/
	if((route->process!=2))
	{
		return FAILED;
	}
	
	for(entry=0; entry<32; entry++)
	{
		if((route->ipMask) & (1<<entry))
			break;
	}
	
	if ((netSize = (1<<entry)) > 1) 
	{
		curStartPos = bestSize = curSize = 0;
		for(entry = 0; entry <= 64; entry++) 
		{
			if((entry == 64) || arpTables.allocBitmap[entry]) 
			{
				if(curSize > bestSize) 
				{
					bestStartPos = curStartPos;
					bestSize = curSize;
				}
				curStartPos = entry+1;
				curSize = 0;
			} else curSize++;
		}
	} 

	if (netSize>1 && ((bestSize<<3) >= netSize)) 
	{
		route->un.arp.arpsta= bestStartPos<<3;
		route->un.arp.arpend = (bestStartPos + (netSize>>3) - ((netSize&0x7)==0? 1: 0))<<3;
		for(entry=route->un.arp.arpsta>>3; entry<=route->un.arp.arpend>>3; entry++)
		{
			arpTables.allocBitmap[entry] = 1;
		}
		return SUCCESS;
	}
	return FAILED;

}

int32 rtl865x_arp_tbl_free(rtl865x_route_t *route)
{
	rtl865x_vlan_entry_t *vlan;
	uint32 index;
	int32 i, j;
	
	if(route==NULL)
	{
		return FAILED;
	}
	if((route->valid!=1) || (route->process!=2) || (route->dstNetif==NULL) )
	{
		return FAILED;
	}

	for(i=route->un.arp.arpsta>>3; i<=route->un.arp.arpend>>3; i++) 
	{
		arpTables.allocBitmap[i] = 0;
		for(j=0; j<8; j++) 
		{
			index = (i<<3)+j;
			if(arpTables.mappings[index].ip!=0)
			{
				rtl8651_delAsicArp(index);
				vlan = _rtl8651_getVlanTableEntry(route->dstNetif->vid);
				rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, vlan->fid, &(arpTables.mappings[index].mac));
				memset(&(arpTables.mappings[index]),0,sizeof(rtl865x_arpMapping_entry_t));
			}
			else
			{
				memset(&(arpTables.mappings[index]),0,sizeof(rtl865x_arpMapping_entry_t));
			}
		}
	}
	
	return SUCCESS;
}

static int32 rtl865x_arp_hash(ipaddr_t ip, uint32 *index)
{
	rtl865x_route_t rt_entry,*route;
	uint32 arpIndex;
	int32 retval = FAILED;

	route = &rt_entry;	
	memset(route,0,sizeof(rtl865x_route_t));
	retval =rtl865x_getRouteEntry(ip, route);

	if (retval != SUCCESS)
		return retval;
	if((route->valid!=1) || (route->process!=2) ||(route->dstNetif==NULL))
	{
		return FAILED;
	}

	arpIndex=((route->un.arp.arpsta)+( (ip) & ~(route->ipMask)));
	if(arpIndex>=RTL8651_ARPTBL_SIZE)
	{
		return FAILED;
	}
	*index=arpIndex;
	return SUCCESS;
}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
int32 rtl8198c_6rd_Ipv6ArpMapping(ipaddr_t ip, rtl865x_arpMapping_entry_t * arp_mapping)
{
	int i;
	rtl865x_arpMapping_entry_t * arpEntry=NULL;
	
	if (arp_mapping == NULL)
	{
		printk("%s,%d. arp_mapping NULL\n",__FUNCTION__,__LINE__);
		return FAILED;
	}	
	memset(arp_mapping, 0, sizeof(rtl865x_arpMapping_entry_t));

	for(i=0;i<512;i++)
	{
		arpEntry = &arpTables.mappings[i];
		
		if(arpEntry->ip == ip)
		{
			memcpy(&(arp_mapping->mac),&(arpEntry->mac),6);
			//printk("%s,%d. get mac success ******************\n",__FUNCTION__,__LINE__);
			return SUCCESS;
		}
	}	
	return FAILED;
}
#endif
#endif


int32 rtl865x_getArpMapping(ipaddr_t ip, rtl865x_arpMapping_entry_t * arp_mapping)
{

	rtl865x_route_t *route,rt_entry;
	uint32 hash;
	int32 retval = FAILED;

	if(arp_mapping==NULL)
	{
		return FAILED;
	}
	memset(arp_mapping, 0, sizeof(rtl865x_arpMapping_entry_t));
	route= &rt_entry;
	memset(route,0,sizeof(rtl865x_route_t));
	retval=rtl865x_getRouteEntry(ip,route);

	if(retval != SUCCESS)
		return retval;
	

	if((route->valid!=1) || (route->process!=2) ||(route->dstNetif==NULL))
	{
		return FAILED;
	}

	if(rtl865x_arp_hash(ip,&hash)==FAILED)
	{
		return FAILED;
	}

	if(arpTables.mappings[hash].ip==ip)
	{
		memcpy(arp_mapping,&(arpTables.mappings[hash]),sizeof(rtl865x_arpMapping_entry_t));
	}
	else
		return FAILED;

	return SUCCESS;
}


int32 rtl865x_getAsicArpEntry(ipaddr_t ip,rtl865x_tblAsicDrv_arpParam_t *asicArpEntry)
{

	rtl865x_route_t *route,rt_entry;
	uint32 hash;
	int32 retval;
	
	if(asicArpEntry==NULL)
	{
		return FAILED;
	}

	route = &rt_entry;
	memset(route,0,sizeof(rtl865x_route_t));
	memset(asicArpEntry, 0, sizeof(rtl865x_tblAsicDrv_arpParam_t));
	
	retval = rtl865x_getRouteEntry(ip,route);
	if(retval != SUCCESS)
		return retval;
	
	if((route==NULL)||(route->valid!=1) || (route->process!=2) ||(route->dstNetif==NULL))
	{
		return FAILED;
	}

	if(rtl865x_arp_hash(ip,&hash)==FAILED)
	{
		return FAILED;
	}

	rtl8651_getAsicArp(hash, asicArpEntry);

	return SUCCESS;

}

int32 rtl865x_mapIpToMac(ipaddr_t ip,ether_addr_t * mac)
{
	uint32 hash;
	
	if(mac==NULL)
	{
		return FAILED;
	}
	
	memset(mac, 0, sizeof(ether_addr_t));
	
	if(rtl865x_arp_hash(ip, &hash)!=SUCCESS)
	{
		return FAILED;
	}

	if(arpTables.mappings[hash].ip==ip)
	{
		memcpy(mac,&(arpTables.mappings[hash].mac),sizeof(ether_addr_t));
		return SUCCESS;
	}
	
	return FAILED;
}

static int32 rtl865x_getArpFid(ipaddr_t ip, uint16 *fid)
{
	rtl865x_route_t *route,rt_entry;
	rtl865x_vlan_entry_t *vlan;
	int32 retval = FAILED;
	

	if(ip==0)
	{
		return FAILED;
	}

	route = &rt_entry;
	memset(route,0,sizeof(rtl865x_route_t));
	retval = rtl865x_getRouteEntry(ip,route);
	if(retval != SUCCESS)
		return retval;
	
	if((route->valid!=1) || (route->process!=2) ||(route->dstNetif==NULL))
	{
		return FAILED;
	}


	vlan = _rtl8651_getVlanTableEntry(route->dstNetif->vid);
	*fid = vlan->fid;
	return SUCCESS;
}

#if 1//#if defined(CONFIG_RTL_AVOID_ADDING_WLAN_PKT_TO_HW_NAT)
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int32 rtl865x_isArpExisted(ipaddr_t ip)
{
	uint32 hash;
	
	if(rtl865x_arp_hash(ip, &hash)!=SUCCESS)
	{
		return FALSE;
	}

	if(arpTables.mappings[hash].ip!=ip)
	{
		return FALSE;
	}

	return TRUE;
}

#endif


#if 0//defined(CONFIG_RTL_EXT_PORT_SUPPORT)

int32 rtl865x_isEthArp(ipaddr_t ip)
{
	uint32 hash;
	int i;
	//memset(mac, 0, sizeof(ether_addr_t));
	uint32 fdb_type[]={ FDB_STATIC, FDB_DYNAMIC };
	uint16 fid = 0;
	uint32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	
	if(rtl865x_arp_hash(ip, &hash)!=SUCCESS)
	{
		return FALSE;
	}

	if(arpTables.mappings[hash].ip!=ip)
	{
		return FALSE;
	}
	
	if(rtl865x_getArpFid(arpTables.mappings[hash].ip, &fid)!=SUCCESS)
	{
		return FALSE;
	}


	for(i=0; i<2; i++) 
	{
		 
		if(rtl865x_Lookup_fdb_entry(fid, &(arpTables.mappings[hash].mac), fdb_type[i], &column,&fdbEntry) != SUCCESS)
		{	
			continue;
		}

		if(rtl865x_isEthPortMask(fdbEntry.memberPortMask )==TRUE)
		{
			return SUCCESS;
		}
		
	}

	return FALSE;
}
#else
int32 rtl865x_isEthArp(ipaddr_t ip)
{
	uint32 hash;
	
	//memset(mac, 0, sizeof(ether_addr_t));
	
	if(rtl865x_arp_hash(ip, &hash)!=SUCCESS)
	{
		return FALSE;
	}

	if(arpTables.mappings[hash].ip!=ip)
	{
		return FALSE;
	}

	return TRUE;
}

#endif

#endif

#if defined(CONFIG_RTL_MULTIPLE_WAN) || defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
int32 rtl_set_callback_for_ps_arp(int (*call_back_fn)(u32 ip,rtl865x_arpMapping_entry_t *entry))
{
	if(NULL == call_back_fn)
	{
		return FAILED;
	}

	rtl_callback_for_ps_arp = call_back_fn;
	return SUCCESS;
}

int32 rtl865x_get_ps_arpMapping(ipaddr_t ip,rtl865x_arpMapping_entry_t *entry)
{
	if(rtl_callback_for_ps_arp)
		return rtl_callback_for_ps_arp(ip,entry);
	
	return FAILED;
}
#endif
int32 rtl865x_addArp(ipaddr_t ip, ether_addr_t * mac)
{
	uint32 i;
	uint32 hash; 
	uint16 fid = 0;

	rtl865x_arpMapping_entry_t oldArpMapping;
	rtl865x_arpMapping_entry_t newArpMapping;
	
	rtl865x_tblAsicDrv_arpParam_t asicArpEntry;
	
	uint32 fdb_type[]={ FDB_STATIC, FDB_DYNAMIC };
	
	uint32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	rtl865x_filterDbTableEntry_t		l2temp_entry;

	if((ip==0) ||(mac==NULL))
	{
		return RTL_EINVALIDINPUT;
	}
		
	#if defined(CONFIG_RTL_MULTIPLE_WAN) || defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
	if(rtl_get_advRt_entry_by_nexthop(ip))
	{
		newArpMapping.ip=ip;
		memcpy(&newArpMapping.mac,mac,sizeof(ether_addr_t));
		rtl865x_eventHandle_addArp_for_multiWan(&newArpMapping);
	}
	#endif
	
	if(rtl865x_arp_hash(ip, &hash)==FAILED)
	{
		return FAILED;
	}
	
    if ((mac->octet[0]&mac->octet[1]&mac->octet[2]&mac->octet[3]&mac->octet[4]&mac->octet[5]) == 0xff)
		return FAILED;

	//printk("%s:%d+++++++++++++++++++++++++++++\n",__FUNCTION__,__LINE__);
	
	/*check the old arp mapping first*/
	memcpy(&oldArpMapping,&(arpTables.mappings[hash] ),sizeof(rtl865x_arpMapping_entry_t));
	if((oldArpMapping.ip!=ip) ||(memcmp(&(oldArpMapping.mac),mac, 6)!=0))
	{	
		/*delete old arp mapping*/
		if(oldArpMapping.ip!=0)
		{
			/*should clear old arp mapping before delete fdb entry and raise arp event*/
			rtl8651_delAsicArp(hash);
			memset(&(arpTables.mappings[hash] ),0,sizeof(rtl865x_arpMapping_entry_t));

			if(rtl865x_getArpFid(oldArpMapping.ip,&fid)==SUCCESS)
			{
				rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEI|RTL865x_L2_TYPEII, fid, &oldArpMapping.mac);
			}
			rtl865x_raiseEvent(EVENT_DEL_ARP,(void*)(&oldArpMapping));		
		}
	}
	
#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
	else 
	{
		/*ip and mac of the new one is the same with the old one*/
		arpTables.mappings[hash].exist_in_ps = 1;
		return RTL_EENTRYALREADYEXIST;
	}
#endif

	/*here to handle the new arp mapping*/
	newArpMapping.ip=ip;
	memcpy(&newArpMapping.mac,mac,sizeof(ether_addr_t));
	if(rtl865x_getArpFid(newArpMapping.ip, &fid)!=SUCCESS)
	{
		return FAILED;
	}
	
	for(i=0; i<2; i++) 
	{
		/*
		printk("%s:%d\n,fid(%d),mac(%02x:%02x:%02x:%02x:%02x:%02x)\n",__FUNCTION__,__LINE__,fid,mac->octet[0],mac->octet[1],
			mac->octet[2],mac->octet[3],mac->octet[4],mac->octet[5]);
		 
		printk("%s:%d\n",__FUNCTION__,__LINE__);
		*/
		#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
		if (i == 0)
			memcpy(&(arpTables_all.mappings[hash]), &newArpMapping,sizeof(rtl865x_arpMapping_entry_t));
		#endif
		if(rtl865x_Lookup_fdb_entry(fid, mac, fdb_type[i], &column,&fdbEntry) != SUCCESS)
		{	
			continue;
		}

		#ifdef CONFIG_RTL_EXT_PORT_SUPPORT
		if(extPortEnabled==0)
		#endif
		{
		if(rtl865x_isEthPortMask(fdbEntry.memberPortMask)==FALSE)
		{
			continue;
		}
		}	

		/*indicate new arp mapping*/
		if((oldArpMapping.ip!=ip) ||(memcmp(&(oldArpMapping.mac),mac, 6)!=0))
		{
			/*in case of layer2 auto learn, add hardware entry to layer 2 software table*/
			l2temp_entry.l2type = (fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII;
			l2temp_entry.process = FDB_TYPE_FWD;
			l2temp_entry.memberPortMask = fdbEntry.memberPortMask;
			l2temp_entry.auth = fdbEntry.auth;
			l2temp_entry.SrcBlk = fdbEntry.srcBlk;
			memcpy(&(l2temp_entry.macAddr), mac, sizeof(ether_addr_t));
#ifdef CONFIG_RTL865X_SYNC_L2
#else
//			rtl865x_addFilterDatabaseEntryExtension(fid, &l2temp_entry);
#endif
//			_rtl865x_addFilterDatabaseEntry((fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII, fid, mac, FDB_TYPE_FWD, fdbEntry.memberPortMask, fdbEntry.auth,fdbEntry.srcBlk);

		}

		/*update or reflesh arp mapping*/
		asicArpEntry.nextHopColumn = column;			
		asicArpEntry.aging    =    300;
	 	asicArpEntry.nextHopRow=rtl865x_getHWL2Index(mac, fid);
		rtl8651_setAsicArp(hash, &asicArpEntry);
		rtl865x_refleshHWL2Table(mac, FDB_DYNAMIC|FDB_STATIC,fid);
		/*update mapping table*/
		memcpy(&(arpTables.mappings[hash]),&newArpMapping,sizeof(rtl865x_arpMapping_entry_t));
#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
		arpTables.mappings[hash].exist_in_ps 	= 1;
		arpTables.mappings[hash].valid 			= 1;
#endif
		rtl865x_raiseEvent(EVENT_ADD_ARP,(void*)(&newArpMapping));	
		return SUCCESS;
		
	}
	
	return FAILED;
}



int32 rtl865x_delArp(ipaddr_t ip)
{
	uint32 hash;
	uint16 fid = 0;
	
	rtl865x_arpMapping_entry_t arpMapping;	
	
	if(ip==0)
	{
		return FAILED;
	}

	#if defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
	if(rtl_get_advRt_entry_by_nexthop(ip))
	{
		arpMapping.ip = ip;
		memset(arpMapping.mac.octet,0,ETHER_ADDR_LEN);
		rtl865x_eventHandle_delArp_for_multiWan(&arpMapping);
	}
	#endif
	
	if(rtl865x_arp_hash(ip,&hash)==FAILED)
	{
		return FAILED;
	}

	//printk("%s:%d***************************************\n",__FUNCTION__,__LINE__);
	memcpy(&arpMapping, &arpTables.mappings[hash], sizeof(rtl865x_arpMapping_entry_t));

	if(arpMapping.ip!=ip)
	{
		return FAILED;
	}
		
	if(rtl865x_getArpFid(ip, &fid)!=SUCCESS)
	{
		return FAILED;
	}

	/*should clear old arp mapping before delete fdb entry and raise arp event*/
	rtl8651_delAsicArp(hash);
	memset(&arpTables.mappings[hash], 0, sizeof(rtl865x_arpMapping_entry_t));
	#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	memset(&arpTables_all.mappings[hash], 0, sizeof(rtl865x_arpMapping_entry_t));
	#endif

#ifdef CONFIG_RTL865X_SYNC_L2
#else
//	rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEI|RTL865x_L2_TYPEII, fid, &arpMapping.mac);
#endif
	rtl865x_raiseEvent(EVENT_DEL_ARP,(void *)(&arpMapping));
	
	return SUCCESS;
}

uint32 rtl865x_arpSync(ipaddr_t ip, uint32 refresh )
{
	uint32 hash;
	rtl865x_arpMapping_entry_t arpMapping;
	rtl865x_tblAsicDrv_arpParam_t asicArpEntry;
	
	uint16 fid;
	rtl865x_tblAsicDrv_l2Param_t l2entry;
	uint32 age=0;
	
	if(ip==0)
	{
		return 0;
	}

	if(rtl865x_arp_hash(ip,&hash)==FAILED)
	{
		return 0;
	}

	memcpy(&arpMapping, &(arpTables.mappings[hash]), sizeof(rtl865x_arpMapping_entry_t));
	
	if(arpMapping.ip!=ip)
	{
		return 0;
	}	

	/*asic arp entry is invalid*/
	if (rtl8651_getAsicArp(hash, &asicArpEntry)!=SUCCESS)
	{
		goto delete_and_out;	
		
	}

	if(rtl865x_getHWL2Table(asicArpEntry.nextHopRow, asicArpEntry.nextHopColumn, &l2entry)!=SUCCESS)
	{
		/*the old fdb entry has timed out*/
		goto delete_and_out;	
	}

	if(memcmp(&(l2entry.macAddr), &(arpMapping.mac), 6)!= 0)
	{
		/*this layer 2 entry has different mac address,
		also indicates the old fdb entry has timed out*/
		goto delete_and_out;	
	}

	age = l2entry.ageSec;
	if (refresh) 
	{
		 rtl865x_refleshHWL2Table(&(l2entry.macAddr), FDB_DYNAMIC|FDB_STATIC,l2entry.fid);
		 age=150;
	}
	else
	{
		if(age>=300)
		{
			age=age-150;
		}
		else
		{
			age=0;
			
			/*to make sure linux arp entry time out before fdb entry*/
			/*asic fdb entry's age is 150 seconds*/
			/*since linux protocol stack arp entry has timed out and l2 entry's precision is also 150 second,
			we should delete both arp and fdb  to sync between linux protocol stack and asic*/
			//goto delete_and_out;
		}
	}
	return age;


delete_and_out:	
	
	if(rtl865x_getArpFid(ip, &fid)==SUCCESS)
	{
		/*should clear old arp mapping before delete fdb entry and raise arp event*/
		rtl8651_delAsicArp(hash);
		memset(&arpTables.mappings[hash], 0, sizeof(rtl865x_arpMapping_entry_t));
		
		rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEI|RTL865x_L2_TYPEII, fid, &arpMapping.mac);
		rtl865x_raiseEvent(EVENT_DEL_ARP,(void *)(&arpMapping));
	}
		
	return 0;
	
}


#if defined(CONFIG_RTL_HARDWARE_NAT) //&&defined(REINIT_SWITCH_CORE)
void rtl819x_save_hw_arp_table(void)
{
	int i = 0;
	uint16 fid = 0;
	ether_addr_t empty_mac = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

	memcpy(&arpTables_bak, &arpTables, sizeof(struct rtl865x_arp_table));

	//store arp related l2 entry	
	rtl819x_init_l2_bakup_entry();

	for (i = 0; i < RTL8651_ARPTBL_SIZE; i++){
		if ((arpTables_bak.mappings[i].ip != 0) || 
			memcmp(arpTables_bak.mappings[i].mac.octet, empty_mac.octet, ETHER_ADDR_LEN)!=0){
			//panic_printk("%s %d ip=0x%x \n", __FUNCTION__, __LINE__, arpTables_bak.mappings[i].ip);
			if(rtl865x_getArpFid(arpTables_bak.mappings[i].ip, &fid)!=SUCCESS)
			{
				printk("%s %d ip=0x%x \n", __FUNCTION__, __LINE__, arpTables_bak.mappings[i].ip);
			}
			rtl819x_save_inused_l2_entry(arpTables_bak.mappings[i].ip, (ether_addr_t *)&arpTables_bak.mappings[i].mac, fid);
		}
	}
	return;
}

int rtl819x_restore_hw_arp_table(void)
{
	int i;
	int ret = FAILED;
	ether_addr_t empty_mac = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

	//restore hw l2 table first.	
	//rtl819x_restore_hw_l2_table();
	
	for (i = 0; i < RTL865X_ARPTBL_SIZE; i++)
	{
		if ((arpTables_bak.mappings[i].ip != 0) || 
			memcmp(arpTables_bak.mappings[i].mac.octet, empty_mac.octet, ETHER_ADDR_LEN)!=0){
			ret = rtl865x_addArp(arpTables_bak.mappings[i].ip, &arpTables_bak.mappings[i].mac);
			//panic_printk("%s %d ip=0x%x ret = %d \n", __FUNCTION__, __LINE__, arpTables_bak.mappings[i].ip, ret);
		}
	}

	return SUCCESS;
}
#endif

#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
static ipaddr_t arp_ip_entry[RTL8651_ARPTBL_SIZE];

int rtl_clear_arp_exist_in_ps_flag(void)
{
	int i;
	unsigned long flags=0;	
	
	SMP_LOCK_ETH(flags);
	for (i=0; i<RTL8651_ARPTBL_SIZE; i++)
	{
		if (arpTables.mappings[i].valid) 
			arpTables.mappings[i].exist_in_ps = 0;
	}
	SMP_UNLOCK_ETH(flags);
	return SUCCESS;
}

int rtl_del_arp_not_exist_in_ps(void)
{
	int i, count = 0;
	unsigned long flags=0;	
	ipaddr_t *tmp = arp_ip_entry;
	
	SMP_LOCK_ETH(flags);
	
	memset(arp_ip_entry, 0, sizeof(ipaddr_t)*RTL8651_ARPTBL_SIZE);

	for (i=0; i<RTL8651_ARPTBL_SIZE; i++)
	{
		if ((arpTables.mappings[i].valid==1) && (arpTables.mappings[i].exist_in_ps==0)) {
			tmp[count] = arpTables.mappings[i].ip;
			count++;
		}
	}

	for (i=0; i<count; i++)
	{
		rtl865x_delArp(tmp[i]);
	}
	
	SMP_UNLOCK_ETH(flags);
	return SUCCESS;
}
#endif



