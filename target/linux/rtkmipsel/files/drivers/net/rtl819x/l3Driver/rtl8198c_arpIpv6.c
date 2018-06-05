/*      @doc RTL_LAYEREDDRV_API

        @module rtl8198c_arpIpv6.c - RTL8198c Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl8198c prefix
        are external functions.
        @normal Jia Wenjian(wenjain_jai@realsil.com.cn) <date>

        Copyright <cp>2013 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/

#include <net/rtl/rtl_types.h>
#include "common/rtl_errno.h"
#include "common/mbuf.h"

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
#include "rtl8198c_routeIpv6.h"
#include "rtl8198c_arpIpv6.h"
#include <net/rtl/rtl865x_fdb_api.h>

static uint32 rtl8198c_ipv6ArpHash(inv6_addr_t ip, uint32 subnetIdx);
static int32 rtl8198c_ipv6_arp_callbackFn_for_del_fdb(void *param);
static int32 rtl8198c_ipv6_arp_register_event(void);
static struct rtl8198c_ipv6_arp_table ipv6ArpTables;
#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
static struct rtl8198c_ipv6_arp_table ipv6ArpTablesBack;
#endif
static int32 rtl8198c_ipv6_arp_callbackFn_for_del_fdb(void *param)
{
	int i;
	rtl865x_filterDbTableEntry_t  *fdbEntry; 
	
	if (param == NULL) {
		return EVENT_CONTINUE_EXECUTE;
	}
	
	fdbEntry = (rtl865x_filterDbTableEntry_t *)param;
	for(i=0; i<RTL8198C_IPV6_ARPTBL_SIZE; i++)
	{
		/*be careful of dead loop, the delete fdb event maybe caused by arp time out*/
		if ((ipv6ArpTables.mappings[i].valid==1)&&memcmp(&(fdbEntry->macAddr), &(ipv6ArpTables.mappings[i].mac), 6)==0) 
			rtl8198c_delIpv6Arp(ipv6ArpTables.mappings[i].ip);
	}
	
	return EVENT_CONTINUE_EXECUTE;
}

static int32 rtl8198c_ipv6_arp_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_DEL_FDB;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = rtl8198c_ipv6_arp_callbackFn_for_del_fdb;
	rtl865x_unRegisterEvent(&eventParam);
	return SUCCESS;
}
static int32 rtl8198c_ipv6_arp_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId = DEFAULT_IPV6_LAYER3_EVENT_LIST_ID;
	eventParam.eventId = EVENT_DEL_FDB;
	eventParam.eventPriority = 0;
	eventParam.event_action_fn = rtl8198c_ipv6_arp_callbackFn_for_del_fdb;
	rtl865x_registerEvent(&eventParam);
	return SUCCESS;
}

int32 rtl8198c_ipv6_arp_init(void)
{
	int i;
	
	rtl8198c_ipv6_arp_unRegister_event();
	
	memset(ipv6ArpTables.mappings, 0, RTL8198C_IPV6_ARPTBL_SIZE*sizeof(rtl8198c_ipv6_arpMapping_entry_t));

	for(i=0; i<RTL8198C_IPV6_ARPTBL_SIZE; i++)
	{
		rtl8198C_delAsicArpV6(i);
	}
	
	rtl8198c_ipv6_arp_register_event();
	
	return SUCCESS;
}

int32 rtl8198c_ipv6_arp_reinit(void)
{
	return rtl8198c_ipv6_arp_init();
}

static uint32 rtl8198c_ipv6ArpHash(inv6_addr_t ip, uint32 subnetIdx)
{
	uint32 id[66];
	uint32 arpv6_hash_idx[6], arpv6_hash[6];
	uint32 i, src;
	uint32 offset= 0;
	uint32 hash_idx_arpv6 = 0;

	memset(arpv6_hash_idx, 0, 6*4);
	memset(arpv6_hash, 0, 6*4);

    	src = ip.v6_addr32[3];
	for (i=0; i<32; i++) 
	{
		if ((src&(1<<i)) != 0) {
		    id[i+offset]=1;
		} else {
		    id[i+offset]=0;
		}		
	}
    
    	offset = 32;
    	src = ip.v6_addr32[2];
	for (i=0; i<32; i++)	
	{
		if ((src&(1<<i)) != 0) {
		    id[i+offset]=1;
		} else {
		    id[i+offset]=0;
		}		
	}
 
    	offset = 64;
	src = subnetIdx;
	for (i=0; i<3; i++)	
	{
		if ((src&(1<<i)) != 0) {
		    id[i+offset]=1;
		} else {
		    id[i+offset]=0;
		}		
	} 

	arpv6_hash_idx[0] = id[0] ^ id[6]  ^ id[12] ^ id[18] ^ id[24] ^ id[30] ^ id[36] ^ id[42] ^ id[48] ^ id[54] ^ id[60] ^ id[66];
	arpv6_hash_idx[1] = id[1] ^ id[7]  ^ id[13] ^ id[19] ^ id[25] ^ id[31] ^ id[37] ^ id[43] ^ id[49] ^ id[55] ^ id[61] ;
	arpv6_hash_idx[2] = id[2] ^ id[8]  ^ id[14] ^ id[20] ^ id[26] ^ id[32] ^ id[38] ^ id[44] ^ id[50] ^ id[56] ^ id[62] ;
	arpv6_hash_idx[3] = id[3] ^ id[9]  ^ id[15] ^ id[21] ^ id[27] ^ id[33] ^ id[39] ^ id[45] ^ id[51] ^ id[57] ^ id[63] ;
	arpv6_hash_idx[4] = id[4] ^ id[10] ^ id[16] ^ id[22] ^ id[28] ^ id[34] ^ id[40] ^ id[46] ^ id[52] ^ id[58] ^ id[64] ;
	arpv6_hash_idx[5] = id[5] ^ id[11] ^ id[17] ^ id[23] ^ id[29] ^ id[35] ^ id[41] ^ id[47] ^ id[53] ^ id[59] ^ id[65] ;

	for (i=0; i<6; i++) 
	{
		arpv6_hash[i] = arpv6_hash_idx[i] & (0x01);
	}

	for (i=0; i<6; i++) 
	{
		hash_idx_arpv6 = hash_idx_arpv6+(arpv6_hash[i]<<i);
	}
 
   //	printk("hash_idx_arpv6:0x%x @ rtl8651_Arpv6TableIndexn\n",hash_idx_arpv6);
	return hash_idx_arpv6&0x3f;
}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)

int32 rtl8198c_dslite_Ipv6ArpMapping(inv6_addr_t ip, rtl8198c_ipv6_arpMapping_entry_t * arp_mapping)
{
	int i;
	rtl8198c_ipv6_arpMapping_entry_t * arpEntry=NULL;
	
	if (arp_mapping == NULL)
	{
		printk("%s,%d. arp_mapping NULL\n",__FUNCTION__,__LINE__);
		return FAILED;
	}	
	memset(arp_mapping, 0, sizeof(rtl8198c_ipv6_arpMapping_entry_t));

	for(i=0;i<RTL8651_ARPTBL_SIZE;i++)
	{
		arpEntry = &ipv6ArpTables.mappings[i];
		
		if(arpEntry->valid==0)
			continue;	
		
		if(memcmp(&ip,&(arpEntry->ip),sizeof(inv6_addr_t))==0)
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

int32 rtl8198c_getIpv6ArpMapping(inv6_addr_t ip, rtl8198c_ipv6_arpMapping_entry_t * arp_mapping)
{
	uint32 hash_low, hash_high, hash;
	rtl8198c_ipv6_route_t *route,rt_entry;
	int32 retval = FAILED;

	if (arp_mapping == NULL)
		return FAILED;
	
	memset(arp_mapping, 0, sizeof(rtl8198c_ipv6_arpMapping_entry_t));
	
	route = &rt_entry;
	memset(route, 0, sizeof(rtl8198c_ipv6_route_t));
	retval = rtl8198c_getIpv6RouteEntryByIp(ip, route);
	if (retval != SUCCESS)
		return retval;

	if ((route->valid!=1) ||(route->process!=2) ||(route->dstNetif==NULL))
		return FAILED;

	hash_high = rtl8198c_ipv6ArpHash(ip, route->arp.subnetIdx);
	/*ipv6 arp table use 4-way hash*/
	for (hash_low=0; hash_low<4; hash_low++)
	{
		hash = hash_high<<2 |hash_low;		
		if ((memcmp(&ipv6ArpTables.mappings[hash].ip, &ip, sizeof(inv6_addr_t))==0)&&
			(ipv6ArpTables.mappings[hash].subnetIdx==route->arp.subnetIdx)) {
			memcpy(arp_mapping, &(ipv6ArpTables.mappings[hash]), sizeof(rtl8198c_ipv6_arpMapping_entry_t));
			return SUCCESS;
		}
	}
	return FAILED;
}

static int32 rtl8198c_getIpv6ArpFid(inv6_addr_t ip, uint16 *fid)
{
	rtl8198c_ipv6_route_t *route,rt_entry;
	rtl865x_vlan_entry_t *vlan;
	int32 retval = FAILED;

	if ((ip.v6_addr32[0]==0)&&(ip.v6_addr32[1]==0)&&(ip.v6_addr32[2]==0)&&(ip.v6_addr32[3]==0))
		return FAILED;

	route = &rt_entry;
	memset(route, 0, sizeof(rtl8198c_ipv6_route_t));
	retval = rtl8198c_getIpv6RouteEntryByIp(ip, route);
	if(retval != SUCCESS)
		return retval;
	
	if((route->valid!=1) ||(route->process!=2) ||(route->dstNetif==NULL))
		return FAILED;

	vlan = _rtl8651_getVlanTableEntry(route->dstNetif->vid);
	*fid = vlan->fid;
	
	return SUCCESS;
}

int32 rtl8198c_isIpv6EthArp(inv6_addr_t ip)
{
	uint32 hash_low, hash_high, hash, retval;
	rtl8198c_ipv6_route_t *route, rt_entry;

	route = &rt_entry;
	memset(route, 0, sizeof(rtl8198c_ipv6_route_t));
	retval = rtl8198c_getIpv6RouteEntryByIp(ip, route);
	if (retval != SUCCESS)
		return FALSE;
	if ((route->valid!=1) ||(route->process!=2) ||(route->dstNetif==NULL))
		return FALSE;

	hash_high = rtl8198c_ipv6ArpHash(ip, route->arp.subnetIdx);
	for (hash_low=0; hash_low<4; hash_low++)
	{
		hash = hash_high<<2 |hash_low;
		if ((ipv6ArpTables.mappings[hash].valid==1) && (ipv6ArpTables.mappings[hash].subnetIdx==route->arp.subnetIdx) &&
		     (memcmp(&ipv6ArpTables.mappings[hash].ip, &ip, sizeof(inv6_addr_t))==0)) {
			return TRUE;
		}
	}

	return FALSE;
}

int32 rtl8198c_isEthPortMask(uint32 portMask)
{
	int i;
	
	for (i=0; i <=RTL8651_PHY_NUMBER; i++)
	{
		if ((1<<i) & portMask)
			return TRUE;
	}
	
	return FALSE;
}

int32 rtl8198c_addIpv6Arp(inv6_addr_t ip, ether_addr_t *mac)
{
	uint16 fid = 0;
	int32   hash_found = -1;
	uint32 i, hash_high, hash_low, hash, column, retval;
	
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	
	rtl8198c_ipv6_arpMapping_entry_t oldArpMapping;
	rtl8198c_ipv6_arpMapping_entry_t newArpMapping;
	
	rtl8198c_ipv6_route_t *route, rt_entry;
	rtl8198C_tblAsicDrv_arpV6Param_t asicArpEntry;
	
	uint32 fdb_type[]={ FDB_STATIC, FDB_DYNAMIC };


	#if 0
	printk("%s.%d.ipAddr(0x%4x%4x%4x%4x%4x%4x%4x%4x),mac(%02x:%02x:%02x:%02x:%02x:%02x)****\n",
			__FUNCTION__,__LINE__,
			ip.v6_addr16[0],ip.v6_addr16[1],ip.v6_addr16[2],ip.v6_addr16[3],
			ip.v6_addr16[4],ip.v6_addr16[5],ip.v6_addr16[6],ip.v6_addr16[7],
			mac->octet[0],mac->octet[1],
			mac->octet[2],mac->octet[3],mac->octet[4],mac->octet[5]);
	#endif
	
	if (((ip.v6_addr32[0]==0)&&(ip.v6_addr32[1]==0)&&(ip.v6_addr32[2]==0)&&(ip.v6_addr32[3]==0)) ||(mac==NULL))
		return RTL_EINVALIDINPUT;

	route = &rt_entry;
	memset(route, 0, sizeof(rtl8198c_ipv6_route_t));
	retval = rtl8198c_getIpv6RouteEntryByIp(ip, route);
	if (retval != SUCCESS)
		return FAILED;
	if ((route->valid!=1) ||(route->process!=2) ||(route->dstNetif==NULL))
		return FAILED;
		
	hash_high = rtl8198c_ipv6ArpHash(ip, route->arp.subnetIdx);
	for (hash_low=0; hash_low<4; hash_low++)
	{
		hash = hash_high<<2 |hash_low;
		if ((ipv6ArpTables.mappings[hash].valid==1) && (ipv6ArpTables.mappings[hash].subnetIdx==route->arp.subnetIdx) &&
		     (memcmp(&ipv6ArpTables.mappings[hash].ip, &ip, sizeof(inv6_addr_t))==0)) {
			if (memcmp(&ipv6ArpTables.mappings[hash].mac, mac, 6) != 0) {
				/*should clear old arp mapping before delete fdb entry and raise arp event*/
				memcpy(&oldArpMapping, &(ipv6ArpTables.mappings[hash]), sizeof(rtl8198c_ipv6_arpMapping_entry_t));
				rtl8198C_delAsicArpV6(hash);
				memset(&(ipv6ArpTables.mappings[hash]), 0, sizeof(rtl8198c_ipv6_arpMapping_entry_t));

				if (rtl8198c_getIpv6ArpFid(oldArpMapping.ip, &fid) == SUCCESS)
					rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEI|RTL865x_L2_TYPEII, fid, &oldArpMapping.mac);

				rtl865x_raiseEvent(EVENT_DEL_IPV6_ARP, (void*)(&oldArpMapping));	
			} else {
				hash_found = hash;
			}
		}
	}

	if (hash_found == -1) {
		for (hash_low=0; hash_low<4; hash_low++)
		{
			hash = hash_high<<2 |hash_low;
			if (ipv6ArpTables.mappings[hash].valid==0)
				break;
		}
	} else {
		/*use the exist entry*/
		hash = hash_found;
	}

	/*here to handle the new arp mapping*/
	memcpy(&newArpMapping.ip, &ip, sizeof(inv6_addr_t));
	memcpy(&newArpMapping.mac,mac,sizeof(ether_addr_t));
	newArpMapping.subnetIdx = route->arp.subnetIdx;
	if (rtl8198c_getIpv6ArpFid(newArpMapping.ip, &fid) != SUCCESS)
		return FAILED;
	
	for (i=0; i<2; i++) 
	{

		#if 0
		printk("%s:%d\n,fid(%d),mac(%02x:%02x:%02x:%02x:%02x:%02x)\n",__FUNCTION__,__LINE__,fid,mac->octet[0],mac->octet[1],
			mac->octet[2],mac->octet[3],mac->octet[4],mac->octet[5]);
		#endif
		
		if (rtl865x_Lookup_fdb_entry(fid, mac, fdb_type[i], &column, &fdbEntry) != SUCCESS)
			continue;

		#if 1//#if defined(CONFIG_RTL_AVOID_ADDING_WLAN_PKT_TO_HW_NAT)
		if (rtl8198c_isEthPortMask(fdbEntry.memberPortMask) == FALSE)
			continue;
		#endif
		
		/*update or reflesh arp mapping*/
		asicArpEntry.valid = 1;
		asicArpEntry.aging    =    300;
		asicArpEntry.subnet_id = route->arp.subnetIdx;
		asicArpEntry.nextHopColumn = column;			
	 	asicArpEntry.nextHopRow=rtl865x_getHWL2Index(mac, fid);
		memcpy(&asicArpEntry.hostid, &ip, sizeof(inv6_addr_t));
		rtl8198C_setAsicArpV6(hash, &asicArpEntry);
		
		rtl865x_refleshHWL2Table(mac, FDB_DYNAMIC|FDB_STATIC, fid);
		/*update mapping table*/
		memcpy(&(ipv6ArpTables.mappings[hash]), &newArpMapping, sizeof(rtl8198c_ipv6_arpMapping_entry_t));
		ipv6ArpTables.mappings[hash].valid = 1;
		rtl865x_raiseEvent(EVENT_ADD_IPV6_ARP,(void*)(&newArpMapping));	
		
		return SUCCESS;
	}	
	
	return FAILED;
}

int32 rtl8198c_delIpv6Arp(inv6_addr_t ip)
{
	uint16 fid = 0;
	uint32 hash_high, hash_low, hash, retval;
	rtl8198c_ipv6_route_t *route, rt_entry;
	rtl8198c_ipv6_arpMapping_entry_t arpMapping;	

	if ((ip.v6_addr32[0]==0)&&(ip.v6_addr32[1]==0)&&(ip.v6_addr32[2]==0)&&(ip.v6_addr32[3]==0))
		return FAILED;

	route = &rt_entry;
	memset(route, 0, sizeof(rtl8198c_ipv6_route_t));
	retval = rtl8198c_getIpv6RouteEntryByIp(ip, route);
	if (retval != SUCCESS)
		return FAILED;
	if ((route->valid!=1) ||(route->process!=2) ||(route->dstNetif==NULL))
		return FAILED;
		
	hash_high = rtl8198c_ipv6ArpHash(ip, route->arp.subnetIdx);
	for (hash_low=0; hash_low<4; hash_low++)
	{
		hash = hash_high<<2 |hash_low;
		if ((ipv6ArpTables.mappings[hash].valid==1) && (ipv6ArpTables.mappings[hash].subnetIdx==route->arp.subnetIdx) &&
		     (memcmp(&ipv6ArpTables.mappings[hash].ip, &ip, sizeof(inv6_addr_t))==0)) {
				memcpy(&arpMapping, &ipv6ArpTables.mappings[hash], sizeof(rtl8198c_ipv6_arpMapping_entry_t));
				if (rtl8198c_getIpv6ArpFid(ip, &fid) != SUCCESS)
					return FAILED;

				/*should clear old arp mapping before delete fdb entry and raise arp event*/
				rtl8198C_delAsicArpV6(hash);
				memset(&ipv6ArpTables.mappings[hash], 0, sizeof(rtl8198c_ipv6_arpMapping_entry_t));

				rtl865x_raiseEvent(EVENT_DEL_IPV6_ARP, (void *)(&arpMapping));
				return SUCCESS;
		}
	}
	
	return FAILED;
}

int32 rtl8198c_delIpv6ArpBySubnetIdx(uint32 subnetIdx)
{
	int32 i;
	uint16 fid = 0;

	for (i=0; i<RTL8198C_IPV6_ARPTBL_SIZE; i++)
	{
		if ((ipv6ArpTables.mappings[i].valid==1) && (ipv6ArpTables.mappings[i].subnetIdx==subnetIdx)) {
			if (rtl8198c_getIpv6ArpFid(ipv6ArpTables.mappings[i].ip, &fid) != SUCCESS)
				continue;

			rtl8198C_delAsicArpV6(i);
			rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, fid, &(ipv6ArpTables.mappings[i].mac));
			memset(&ipv6ArpTables.mappings[i], 0, sizeof(rtl8198c_ipv6_arpMapping_entry_t));
		}
	}
	
	return SUCCESS;
}

uint32 rtl8198c_ipv6ArpSync(inv6_addr_t ip, uint32 refresh )
{
	uint16 fid;
	uint32 age=0;
	int32   found = -1;
	uint32 hash_high, hash_low, hash, retval;
	rtl8198c_ipv6_route_t *route, rt_entry;
	rtl8198c_ipv6_arpMapping_entry_t arpMapping;
	rtl8198C_tblAsicDrv_arpV6Param_t asicArpEntry;
	rtl865x_tblAsicDrv_l2Param_t l2entry;
	
	if ((ip.v6_addr32[0]==0)&&(ip.v6_addr32[1]==0)&&(ip.v6_addr32[2]==0)&&(ip.v6_addr32[3]==0))
		return 0;

	route = &rt_entry;
	memset(route, 0, sizeof(rtl8198c_ipv6_route_t));
	retval = rtl8198c_getIpv6RouteEntryByIp(ip, route);
	if (retval != SUCCESS)
		return 0;
	if ((route->valid!=1) ||(route->process!=2) ||(route->dstNetif==NULL))
		return 0;
		
	hash_high = rtl8198c_ipv6ArpHash(ip, route->arp.subnetIdx);
	for (hash_low=0; hash_low<4; hash_low++)
	{
		hash = hash_high<<2 |hash_low;
		if ((ipv6ArpTables.mappings[hash].valid==1) && (ipv6ArpTables.mappings[hash].subnetIdx==route->arp.subnetIdx) &&
		     (memcmp(&ipv6ArpTables.mappings[hash].ip, &ip, sizeof(inv6_addr_t))==0)) {
				memcpy(&arpMapping, &(ipv6ArpTables.mappings[hash]), sizeof(rtl8198c_ipv6_arpMapping_entry_t));
				found = 1;
				break;
		}
	}

	if (found == -1)
		return 0;

	/*asic arp entry is invalid*/
	if (rtl8198C_getAsicArpV6(hash, &asicArpEntry)!=SUCCESS)
		goto delete_and_out;	
		
	if (rtl865x_getHWL2Table(asicArpEntry.nextHopRow, asicArpEntry.nextHopColumn, &l2entry) != SUCCESS)
		/*the old fdb entry has timed out*/
		goto delete_and_out;	

	if (memcmp(&(l2entry.macAddr), &(arpMapping.mac), 6) != 0)
		/*this layer 2 entry has different mac address,
		also indicates the old fdb entry has timed out*/
		goto delete_and_out;	

	age = l2entry.ageSec;
	if (refresh) {
		 rtl865x_refleshHWL2Table(&(l2entry.macAddr), FDB_DYNAMIC|FDB_STATIC, l2entry.fid);
		 age=150;
	} else {
		if (age>=300) {
			age=age-150;
		}
		else {
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
	
	if (rtl8198c_getIpv6ArpFid(ip, &fid) == SUCCESS) {
		/*should clear old arp mapping before delete fdb entry and raise arp event*/
		rtl8198C_delAsicArpV6(hash);
		memset(&ipv6ArpTables.mappings[hash], 0, sizeof(rtl8198c_ipv6_arpMapping_entry_t));
		
		rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEI|RTL865x_L2_TYPEII, fid, &arpMapping.mac);
		rtl865x_raiseEvent(EVENT_DEL_IPV6_ARP, (void *)(&arpMapping));
	}
	
	return 0;
	
}

int32 rtl8198c_changeIpv6ArpSubnetIdx(uint32 old_subnetIdx, uint32 new_subnetIdx)
{
	int i;
	rtl8198C_tblAsicDrv_arpV6Param_t asicArpEntry;

	for (i=0; i<RTL8198C_IPV6_ARPTBL_SIZE; i++)
	{
		if ((ipv6ArpTables.mappings[i].valid==1) && (ipv6ArpTables.mappings[i].subnetIdx==old_subnetIdx)) {	
			if (rtl8198C_getAsicArpV6(i, &asicArpEntry) != SUCCESS) {
				printk("%s[%d], Waring ipv6 arp sw tables is not the same with hw table!\n", __FUNCTION__, __LINE__);
				continue;
			}
			
			ipv6ArpTables.mappings[i].subnetIdx = new_subnetIdx;
			asicArpEntry.subnet_id = new_subnetIdx;
			rtl8198C_setAsicArpV6(i, &asicArpEntry);
		}
	}

	return SUCCESS;
}
	

#ifdef CONFIG_RTL_PROC_NEW
int32 sw_arp6_read(struct seq_file *s, void *v)
{


	uint32 i;
	rtl8198c_ipv6_arpMapping_entry_t * arpEntry=NULL;


	seq_printf(s, "%s\n", "SW arp6 Table:\n");
	
	for(i=0;i<RTL8651_ARPTBL_SIZE;i++)
	{
		arpEntry = &ipv6ArpTables.mappings[i];
		if(arpEntry->valid==0)
			continue;	
		seq_printf(s,"  [%d]  %d,ip:%x:%x:%x:%x,mac:%x-%x-%x-%x-%x-%x \n", i,arpEntry->subnetIdx,
			arpEntry->ip.v6_addr32[0],arpEntry->ip.v6_addr32[1],arpEntry->ip.v6_addr32[2],arpEntry->ip.v6_addr32[3],
			arpEntry->mac.octet[0],arpEntry->mac.octet[1],arpEntry->mac.octet[2],
			arpEntry->mac.octet[3],arpEntry->mac.octet[4],arpEntry->mac.octet[5]);	
	}

	return 0;
}
#else
int32 sw_arp6_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	rtl8198c_ipv6_arpMapping_entry_t * arpEntry=NULL;

	uint32 i;

	len = sprintf(page, "%s\n", "SW arp6 Table:\n");
	for(i=0;i<RTL8651_ARPTBL_SIZE;i++)
	{
		arpEntry = &ipv6ArpTables.mappings[i];
		if(arpEntry->valid==0)
			continue;	
		len += sprintf(page + len,"  [%d]  %d,ip:%x:%x:%x:%x,mac:%x-%x-%x-%x-%x-%x \n", i,arpEntry->subnetIdx,
			arpEntry->ip.v6_addr32[0],arpEntry->ip.v6_addr32[1],arpEntry->ip.v6_addr32[2],arpEntry->ip.v6_addr32[3],
			arpEntry->mac.octet[0],arpEntry->mac.octet[1],arpEntry->mac.octet[2],
			arpEntry->mac.octet[3],arpEntry->mac.octet[4],arpEntry->mac.octet[5]);	
	}

	return len;
}
#endif

int32 sw_arp6_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}

#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
void rtl819x_save_hw_ipv6_arp_table(void)
{
	memcpy(&ipv6ArpTablesBack, &ipv6ArpTables, sizeof(struct rtl8198c_ipv6_arp_table));
	
	return;
}

int rtl819x_restore_hw_ipv6_arp_table(void)
{
	int i;
	int ret = FAILED;
	ether_addr_t empty_mac = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

	
	for (i = 0; i < RTL8198C_IPV6_ARPTBL_SIZE; i++)
	{
		if ((ipv6ArpTablesBack.mappings[i].ip.v6_addr32[0]&& ipv6ArpTablesBack.mappings[i].ip.v6_addr32[1]
			&&ipv6ArpTablesBack.mappings[i].ip.v6_addr32[2]&& ipv6ArpTablesBack.mappings[i].ip.v6_addr32[3]) 
			|| memcmp(ipv6ArpTablesBack.mappings[i].mac.octet, empty_mac.octet, ETHER_ADDR_LEN)!=0){
			ret = rtl8198c_addIpv6Arp(ipv6ArpTablesBack.mappings[i].ip, &ipv6ArpTablesBack.mappings[i].mac);
		}
	}

	return SUCCESS;
}

#endif
