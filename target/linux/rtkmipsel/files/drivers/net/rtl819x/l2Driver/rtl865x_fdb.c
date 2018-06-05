/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */


#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/kconfig.h>
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/kernel.h>

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_fdb_api.h>
#include "AsicDriver/asicRegs.h"
//#include <common/rtl8651_aclLocal.h>
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
//#include <net/rtl/rtl_queue.h>
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#else
#include <AsicDriver/rtl865xC_tblAsicDrv.h>
#include <AsicDriver/rtl8651_tblAsicDrv.h>
#include "rtl8651_tblDrvPatch.h"
#endif

#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_eventMgr.h"
#include "common/mbuf.h"
#include "rtl865x_fdb.h"
#include "common/rtl_errno.h"

#if defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
lan_restrict_info	lan_restrict_tbl[LAN_RESTRICT_PORT_NUMBER];
int lan_restrict_tble_reset_port(int port)
{
	#if 0	//disable by jwj
	lan_restrict_tbl[port].curr_num	= 	0;
	#endif
	return SUCCESS;
}

#endif

struct rtl865x_L2Tables sw_FDB_Table;
int32    arpAgingTime = 450;
ether_addr_t cpu_mac = { {0x00, 0x00, 0x0a, 0x00, 0x00, 0x0f} };
rtl865x_tblAsicDrv_l2Param_t tmpL2buff;

#if defined(CONFIG_RTL_HARDWARE_NAT) //&&defined(REINIT_SWITCH_CORE)
#define L2_BAKUP_ENTRY_MAX 128
rtl865x_tblAsicDrv_l2Param_t l2_entry_bak[L2_BAKUP_ENTRY_MAX];
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static RTL_DECLARE_MUTEX(l2_sem);
#endif
int32 _rtl865x_fdb_alloc(void)
{
	int32 index = 0;
	memset( &sw_FDB_Table, 0, sizeof( sw_FDB_Table ) );
	#ifdef __KERNEL__
	TBL_MEM_ALLOC(sw_FDB_Table.filterDB, rtl865x_filterDbTable_t, RTL865x_FDB_NUMBER);
	#else
	sw_FDB_Table.filterDB = (rtl865x_filterDbTable_t *)malloc(RTL865x_FDB_NUMBER * sizeof(rtl865x_filterDbTable_t)); 
	#endif
	{//Initial free filter database entry
		rtl865x_filterDbTableEntry_t * tempFilterDb = NULL;
		#ifdef __KERNEL__
			TBL_MEM_ALLOC(tempFilterDb, rtl865x_filterDbTableEntry_t, RTL8651_L2TBL_ROW);
		#else
		#endif
		SLIST_INIT(&sw_FDB_Table.freefdbList.filterDBentry);
		for(index=0; index<RTL8651_L2TBL_ROW; index++)
			SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, &tempFilterDb[index], nextFDB);
	}

	return SUCCESS;
	
}

int32 rtl865x_getReserveMacAddr(ether_addr_t *macAddr)
{
	memcpy( macAddr, &cpu_mac, sizeof(ether_addr_t));
	return SUCCESS;
}

int32 _rtl865x_layer2_patch(void)
{

	int32 retval = 0;
	ether_addr_t mac = { {0xff, 0xff, 0xff, 0xff, 0xff, 0xff} };
	uint32 portmask=RTL_WANPORT_MASK|RTL_LANPORT_MASK;

	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_LAN_FID, &mac, FDB_TYPE_TRAPCPU, portmask, TRUE, FALSE);
#if defined (CONFIG_RTL_IVL_SUPPORT)
	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_WAN_FID, &mac, FDB_TYPE_TRAPCPU, portmask, TRUE, FALSE);
#endif

	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_LAN_FID, &cpu_mac, FDB_TYPE_TRAPCPU, 0, TRUE, FALSE);
#if defined (CONFIG_RTL_IVL_SUPPORT)
	retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_WAN_FID, &cpu_mac, FDB_TYPE_TRAPCPU, 0, TRUE, FALSE);
#endif
	assert(retval == SUCCESS);

	return SUCCESS;
}

int32 _rtl865x_fdb_collect(void)
{
	int32 index = 0;
	int32 index1 = 0;

	rtl865x_filterDbTable_t *filterDbPtr;
	rtl865x_filterDbTableEntry_t * tempFilterDbPtr;	
	for(index=0,filterDbPtr=&sw_FDB_Table.filterDB[0]; index<RTL865x_FDB_NUMBER; index++,filterDbPtr++) {
		for(index1=0; index1<RTL8651_L2TBL_ROW; index1++)
			while(SLIST_FIRST(&(filterDbPtr->database[index1]))) {
				tempFilterDbPtr = SLIST_FIRST(&(filterDbPtr->database[index1]));
				SLIST_REMOVE_HEAD(&(filterDbPtr->database[index1]), nextFDB);
				SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, tempFilterDbPtr, nextFDB);
			}
	}
			
	return SUCCESS;
	
}

int32 _rtl865x_fdb_init(void)
{ 
	int32 index, index1;
	/*Initial Filter database*/
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[0];
	for(index=0; index<RTL865x_FDB_NUMBER; index++, fdb_t++) {
		for(index1=0; index1<RTL8651_L2TBL_ROW; index1++)
			SLIST_INIT(&(fdb_t->database[index1]));
		fdb_t->valid = 1;
	}

	return SUCCESS;
}

int32 _rtl865x_clearHWL2Table(void)
{
	int i, j;

	for (i = 0; i < RTL8651_L2TBL_ROW; i++)
		for (j = 0; j < RTL8651_L2TBL_COLUMN; j++)
		{
			rtl8651_delAsicL2Table(i, j);	
		}

	return SUCCESS;	
}

int32 rtl865x_layer2_init(void)
{
		
	_rtl865x_fdb_alloc();

	_rtl865x_fdb_init();

	_rtl865x_layer2_patch();
	
	return SUCCESS;
}

int32 rtl865x_layer2_reinit(void)
{
	_rtl865x_clearHWL2Table();

	_rtl865x_fdb_collect();
		
	//_rtl865x_fdb_alloc();

	_rtl865x_fdb_init();

	_rtl865x_layer2_patch();
	
	return SUCCESS;
}

 uint32 rtl865x_getHWL2Index(ether_addr_t * macAddr, uint16 fid)
 {
	return (rtl8651_filterDbIndex(macAddr, fid));
 }

int32 rtl865x_setHWL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p)
{
	return (rtl8651_setAsicL2Table(row, column, l2p));
}

int32 rtl865x_getHWL2Table(uint32 row, uint32 column, rtl865x_tblAsicDrv_l2Param_t *l2p) 
{
	return rtl8651_getAsicL2Table(row, column, l2p);
}

int32 rtl865x_refleshHWL2Table(ether_addr_t * macAddr, uint32 flags,uint16 fid)
{

	rtl865x_tblAsicDrv_l2Param_t L2temp, *L2buff;
	uint32 rowIdx, col_num;
	uint32 colIdx = 0;
	uint32 found = FALSE;

	L2buff = &L2temp;
	memset(L2buff, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);

	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) {
		if ((rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff))!=SUCCESS ||
			memcmp(&(L2buff->macAddr), macAddr, 6)!= 0)
			continue;
		
		if (((flags&FDB_STATIC) && L2buff->isStatic) ||
			((flags&FDB_DYNAMIC) && !L2buff->isStatic)) {
				assert(colIdx);
				col_num = colIdx;
				found = TRUE;
				break;
		}	
	} 

	if (found == TRUE)
	{
		L2buff->ageSec = 450;
		rtl8651_setAsicL2Table(rowIdx, colIdx, L2buff);
		return SUCCESS;
	}
	else
	{
		return FAILED;
	}
}

int32 rtl_get_hw_fdb_age(uint32 fid,ether_addr_t *mac, uint32 flags)
{
        uint32 rowIdx;
        uint32 colIdx = 0;
        int32 retval = 0;
        rtl865x_tblAsicDrv_l2Param_t L2buff;

        memset(&L2buff,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));

        rowIdx = rtl8651_filterDbIndex(mac, fid);

        for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++)
        {
                retval = rtl8651_getAsicL2Table(rowIdx, colIdx, &L2buff);

                if (retval !=SUCCESS ||
                        memcmp(&(L2buff.macAddr), mac, 6)!= 0)
                        continue;
                if (((flags&FDB_DYNAMIC) && !L2buff.isStatic)||((flags&FDB_STATIC) && L2buff.isStatic))
                {
                        retval = L2buff.ageSec;
                        break;
                }

        }

        return retval;
}

EXPORT_SYMBOL(rtl_get_hw_fdb_age);

int32 rtl_get_hw_fdb_port(uint32 fid, ether_addr_t *mac, uint32 flags, uint8 *port)
{
	 int i;
        uint32 rowIdx;
        uint32 colIdx = 0;
        int32 retval = FAILED;
        rtl865x_tblAsicDrv_l2Param_t L2buff;

        memset(&L2buff,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));

        rowIdx = rtl8651_filterDbIndex(mac, fid);

        for (colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++)
        {
                retval = rtl8651_getAsicL2Table(rowIdx, colIdx, &L2buff);

                if (retval !=SUCCESS ||
                        memcmp(&(L2buff.macAddr), mac, 6)!= 0)
                        continue;
                if (((flags&FDB_DYNAMIC) && !L2buff.isStatic)||((flags&FDB_STATIC) && L2buff.isStatic))
                {
			  //retval = L2buff.memberPortMask;
			  for (i=0; i<6; i++)
			  {
				if ((1<<i) & L2buff.memberPortMask) {
					*port = i;
					return SUCCESS;
				}
			  }
                }
        }

        return retval;
}

int32 rtl865x_getPortNum(const unsigned char *addr){
	ether_addr_t *macAddr;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	int8 port_num = -1;
	//fdb->ageing_timer = 300*HZ;
	macAddr = (ether_addr_t *)(addr);
			
	if (rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS)
	{
		/*find the fdb entry*/
		port_num = rtl865x_ConvertPortMasktoPortNum(fdbEntry.memberPortMask);
	} else {
		/*can't find */
	}
	return port_num;
}

int32 rtl865x_getPortMaskByMac(const unsigned char *addr, uint32 fid)
{
	ether_addr_t *macAddr;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	//int8 port_num = -1;
	//fdb->ageing_timer = 300*HZ;
	macAddr = (ether_addr_t *)(addr);
	//fid = ff, search lan and wan fid group
	if (fid == 0xff){
		if (rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS){
			return (fdbEntry.memberPortMask);
		}
		else if (rtl865x_Lookup_fdb_entry(RTL_WAN_FID, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS){
			return (fdbEntry.memberPortMask);
		}
		else {
			return -1;
		}
	}
	else if (rtl865x_Lookup_fdb_entry(fid, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS)
	{
		/*find the fdb entry*/
		return (fdbEntry.memberPortMask);
	} else {
		/*can't find */
		return -1;
	}
	return -1;
}

#ifdef CONFIG_RTL_NIC_QUEUE
int32 rtl865x_getSwPortNum(const unsigned char *mac, uint32 fid)
{
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	rtl865x_filterDbTableEntry_t * l2entry_t ;
	uint32 rowIdx;
	
	rowIdx = rtl8651_filterDbIndex((ether_addr_t *)mac, fid);

	if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
	{	
		SLIST_FOREACH(l2entry_t, &(fdb_t->database[rowIdx]), nextFDB) 
		{
			if (memcmp(&(l2entry_t->macAddr), mac, 6)== 0)
			{
				return rtl865x_ConvertPortMasktoPortNum(l2entry_t->memberPortMask);
					
			}			
		}
	}
	
	return -1;
}
#endif

int32 rtl865x_Lookup_fdb_entry(uint32 fid, ether_addr_t *mac, uint32 flags, uint32 *col_num, rtl865x_tblAsicDrv_l2Param_t *L2buff)
{
	uint32 rowIdx;
	uint32 colIdx;
	int32 retval;

	rowIdx = rtl8651_filterDbIndex(mac, fid);
	colIdx = 0;
	
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		retval = rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff);

		if (retval !=SUCCESS ||
			memcmp(&(L2buff->macAddr), mac, 6)!= 0)
			continue;
		if (((flags&FDB_STATIC) && L2buff->isStatic) ||
			((flags&FDB_DYNAMIC) && !L2buff->isStatic)) {
			assert(colIdx);
			*col_num = colIdx;
			return SUCCESS;
		}
	
	}
	return FAILED;
}

/*
@func enum RTL_RESULT | rtl865x_addFdbEntry | Add an MAC address, said Filter Database Entry
@parm uint32 | fid | The filter database index (valid: 0~3)
@parm ether_addr_t * | mac | The MAC address to be added
@parm uint32 | portmask | The portmask of this MAC address belongs to
@parm uint32 | type | fdb entry type
@rvalue SUCCESS | Add success
@rvalue FAILED | General failure
@comm 
	Add a Filter Database Entry to L2 Table(1024-Entry)
@devnote
	(under construction)
*/
int32 rtl865x_addFilterDatabaseEntry( uint32 fid, ether_addr_t * mac, uint32 portmask, uint32 type )
{
        int32 retval;
	 unsigned long flags=0;	
        if (type != FDB_TYPE_FWD && type != FDB_TYPE_SRCBLK && type != FDB_TYPE_TRAPCPU)
                return RTL_EINVALIDINPUT; /* Invalid parameter */
 
        if (fid >= RTL865x_FDB_NUMBER)
                return RTL_EINVALIDINPUT;
        if (mac == (ether_addr_t *)NULL)
                return RTL_EINVALIDINPUT;
        if (sw_FDB_Table.filterDB[fid].valid == 0)
                return RTL_EINVALIDFID;
        /*l2 lock*/
	//rtl_down_interruptible(&l2_sem);
	SMP_LOCK_ETH(flags);	
#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
        retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, fid,  mac, type, portmask, TRUE, FALSE);
#else
        retval = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEII, fid,  mac, type, portmask, FALSE, FALSE);
#endif
        /*l2 unlock*/
	//rtl_up(&l2_sem);
 	SMP_UNLOCK_ETH(flags);
        return retval;
}

int32 rtl865x_addFilterDatabaseEntryExtension( uint16 fid, rtl865x_filterDbTableEntry_t * L2entry)
{
        int32 retval;
	 //unsigned long flags=0;
        if (L2entry->process != FDB_TYPE_FWD && L2entry->process != FDB_TYPE_SRCBLK && L2entry->process != FDB_TYPE_TRAPCPU)
                return RTL_EINVALIDINPUT; /* Invalid parameter */
 
        if (fid >= RTL865x_FDB_NUMBER)
                return RTL_EINVALIDINPUT;
        if (&(L2entry->macAddr) == (ether_addr_t *)NULL)
                return RTL_EINVALIDINPUT;
        if (sw_FDB_Table.filterDB[fid].valid == 0)
                return RTL_EINVALIDFID;
        /*l2 lock*/
	//rtl_down_interruptible(&l2_sem);	
	/*Fix jwj: disable SMP_LOCK_ETH*/
	//SMP_LOCK_ETH(flags);	
	retval = _rtl865x_addFilterDatabaseEntry(	L2entry->l2type, 
											fid,  
											&(L2entry->macAddr),
											L2entry->process,
											L2entry->memberPortMask,
											L2entry->auth,
											L2entry->SrcBlk);

        /*l2 unlock*/
	//rtl_up(&l2_sem);
 	//SMP_UNLOCK_ETH(flags);
        return retval;
}


#if 0
int32 rtl865x_addFilterDatabaseEntry( uint32 fid, ether_addr_t * mac, uint32 portmask, uint32 type , uint32 isStatic)
{
	int32 retval;
	
	if (type != FDB_TYPE_FWD && type != FDB_TYPE_SRCBLK && type != FDB_TYPE_TRAPCPU)
		return RTL_EINVALIDINPUT; /* Invalid parameter */

	if (fid >= RTL865x_FDB_NUMBER)
		return RTL_EINVALIDINPUT;
	if (mac == (ether_addr_t *)NULL)
		return RTL_EINVALIDINPUT;
	if (sw_FDB_Table.filterDB[fid].valid == 0)
		return RTL_EINVALIDFID;
	/*l2 lock*/
	
#ifdef RTL865XC_LAN_PORT_NUM_RESTRIT
	retval = _rtl865x_addFilterDatabaseEntry(isStatic == TRUE? RTL865x_L2_TYPEII:RTL865x_L2_TYPEI, fid,  mac, type, portmask, TRUE, FALSE);
#else
	retval = _rtl865x_addFilterDatabaseEntry(isStatic == TRUE? RTL865x_L2_TYPEII:RTL865x_L2_TYPEI, fid,  mac, type, portmask, FALSE, FALSE);

#endif
	/*l2 unlock*/


	return retval;
}
#endif


int32 _rtl865x_removeFilterDatabaseEntry(uint16 fid, ether_addr_t * mac, uint32 rowIdx)
{
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	rtl865x_filterDbTableEntry_t * l2entry_t ;
	/*printk("rowIdx:%dfid:%d\n",rowIdx,fid);*/
	/*printk("\n mac address is %x %x %x %x %x %x\n", mac->octet[0],
													mac->octet[1],
													mac->octet[2],
													mac->octet[3],
													mac->octet[4],
													mac->octet[5]);
													*/
	if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
	{	
		SLIST_FOREACH(l2entry_t, &(fdb_t->database[rowIdx]), nextFDB) 
		{
			 if (memcmp(&(l2entry_t->macAddr), mac, 6)== 0)
			{
				SLIST_REMOVE(
					&(fdb_t->database[rowIdx]), 
					l2entry_t, 
					 rtl865x_filterDbTableEntry_s,
					 nextFDB
				);
				SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, l2entry_t, nextFDB);
				
				rtl865x_raiseEvent(EVENT_DEL_FDB, (void *)(l2entry_t));

				return SUCCESS;
			}			
		}
	}

	return FAILED;
}

int32 rtl865x_lookup_FilterDatabaseEntry(uint16 fid, ether_addr_t * mac, rtl865x_filterDbTableEntry_t *l2_entry)
{
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	rtl865x_filterDbTableEntry_t * l2entry_t ;
	uint32 rowIdx;
	
	rowIdx = rtl8651_filterDbIndex(mac, fid);

	if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
	{	
		SLIST_FOREACH(l2entry_t, &(fdb_t->database[rowIdx]), nextFDB) 
		{
			 if (memcmp(&(l2entry_t->macAddr), mac, 6)== 0)
			{
				l2_entry->asicPos	= l2entry_t->asicPos;
				l2_entry->auth		= l2entry_t->auth;
				l2_entry->l2type		= l2entry_t->l2type;
				l2_entry->linkId		= l2entry_t->linkId;
				l2_entry->memberPortMask = l2entry_t->memberPortMask;
				l2_entry->nhFlag 	= l2entry_t->nhFlag;
				l2_entry->process	= l2entry_t->process;
				l2_entry->SrcBlk		= l2entry_t->SrcBlk;
				l2_entry->vid		= l2entry_t->vid;
				return SUCCESS;
			}			
		}
	}

	memset(l2_entry, 0, sizeof(rtl865x_filterDbTableEntry_t));
	return FAILED;
}

int32 _rtl865x_addFilterDatabaseEntry(uint16 l2Type, uint16 fid,  ether_addr_t * macAddr, uint32 type, uint32 portMask, uint8 auth, uint8 SrcBlk)
{
	uint32 rowIdx = 0;
	uint32 colIdx = 0;
	uint32 col_num = 0;
	uint32 col_tmp = 0;
	uint16 tmp_age = 0xffff;
	int32   found = FALSE;
	int32   flag = FALSE;
	int32   nexthp_flag = FALSE;
	int32  isStatic = FALSE;
	int32  toCpu = FALSE;
	rtl865x_filterDbTableEntry_t * l2entry_t = NULL;
	rtl865x_filterDbTableEntry_t *tmpL2;
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	rtl865x_tblAsicDrv_l2Param_t l2entry_tmp,*L2buff;
	int32 retval = 0;

	L2buff = &l2entry_tmp;
	memset(L2buff,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		retval = rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff);		
		if ((retval)==SUCCESS)
		{
			/*check whether mac address has been learned  to HW or not*/
			if (memcmp(&(L2buff->macAddr), macAddr, 6)== 0)
			{	
				/*the entry has been auto learned*/
				if (L2buff->memberPortMask==portMask)
				{
					/*the entry has been auto learned        */
					found = TRUE;
					col_tmp = colIdx;
				}
				else
				/*portmask is different , it should be overwrited*/	
				{
					found = FALSE;
					flag = TRUE;
				}
				break;
			}
			/*no matched entry, try get minimum aging time L2 Asic entry*/
			if (tmp_age> L2buff->ageSec)
			{
				tmp_age = L2buff->ageSec;
				col_num = colIdx;
			}
		}
		else
		{
			/*there is still empty l2 asic entry*/
			flag = TRUE;
			break;
		}
	}
	
	switch(l2Type) {	
	case RTL865x_L2_TYPEI:
		nexthp_flag = FALSE;isStatic = FALSE;
		break;
	case RTL865x_L2_TYPEII:
		nexthp_flag = TRUE; isStatic = TRUE;
		break;
	case RTL865x_L2_TYPEIII:
		nexthp_flag = FALSE;isStatic = TRUE;
		break;
	default: assert(0);	
	}

	switch(type) {	
	case FDB_TYPE_FWD:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_DSTBLK:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_SRCBLK:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_TRAPCPU:
			toCpu =  TRUE;
			break;
	default: assert(0);
	}

	if (found == FALSE)
	{
#if 0	
		/*no empty entry, overwrite the biggest aging time asic l2 entry*/
		if(flag == FALSE)
		{
			/*delete the biggest aging time software entry*/
			rtl8651_getAsicL2Table(rowIdx, col_num, L2buff);			
			_rtl865x_removeFilterDatabaseEntry(fid, &(L2buff->macAddr), rowIdx);	

			/*overwrite asic entry*/
			rtl8651_setAsicL2Table_Patch(
					rowIdx, 
					col_num, 
					macAddr, 
					toCpu, 
					SrcBlk, 
					portMask, 
					arpAgingTime, 
					isStatic, 
					nexthp_flag,
					fid,
					auth);			
			col_tmp = col_num;
		}
		else 
#endif			
		/*portmask is different , so it should overwrite the original asic entry. Or there is empty entry, set it to asic*/
		if(flag == TRUE)
		{
			rtl8651_setAsicL2Table_Patch(
					rowIdx, 
					colIdx, 
					macAddr, 
					toCpu, 
					SrcBlk, 
					portMask, 
					arpAgingTime, 
					isStatic, 
					nexthp_flag,
					fid,
					auth);
			col_tmp = colIdx;
		}
	}
	/*find the same asic entry, should update the aging time*/
	else
	{
		rtl8651_setAsicL2Table_Patch(
				rowIdx, 
				col_tmp, 
				macAddr, 
				toCpu, 
				SrcBlk, 
				portMask, 
				arpAgingTime, 
				isStatic, 
				nexthp_flag,
				fid,
				auth);		
	}

/*	
	colIdx=0;

	tmpL2 = SLIST_FIRST(&(fdb_t->database[rowIdx]));
	tmpL2->nextFDB.sle_next  = NULL;
	
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		if ((rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff))==SUCCESS)
		{
			l2entry_t->asicPos =colIdx;
			l2entry_t->auth = L2buff->auth;
			l2entry_t->configToAsic = 0;
			l2entry_t->memberPortMask = L2buff->memberPortMask;
			l2entry_t->SrcBlk = L2buff->srcBlk;
			memcpy(&l2entry_t->macAddr, &L2buff->macAddr, sizeof(ether_addr_t));

			tmpL2->nextFDB.sle_next = l2entry_t;
			l2entry_t->nextFDB.sle_next = NULL;
			tmpL2 = l2entry_t;
		}
	}
*/

	if (SLIST_FIRST(&sw_FDB_Table.freefdbList.filterDBentry) == NULL)
		return RTL_ENOFREEBUFFER;
	
	/*config the SW l2 entry */
	l2entry_t = SLIST_FIRST(&sw_FDB_Table.freefdbList.filterDBentry);
	SLIST_REMOVE_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, nextFDB);
	
	l2entry_t->asicPos =col_tmp;
	l2entry_t->auth = auth;
	l2entry_t->configToAsic = 0;
	l2entry_t->memberPortMask = portMask;
	l2entry_t->l2type = l2Type;
	l2entry_t->nhFlag = nexthp_flag;
	
	l2entry_t->SrcBlk = FALSE;
	memcpy(&l2entry_t->macAddr, macAddr, sizeof(ether_addr_t));
	switch(type) {	
	case FDB_TYPE_FWD:
	 	 l2entry_t->process = FDB_TYPE_FWD;
		 l2entry_t->memberPortMask = portMask;
		 break;
	case FDB_TYPE_DSTBLK:
		 l2entry_t->process = FDB_TYPE_DSTBLK;
		 l2entry_t->memberPortMask = 0;
		 break;
	case FDB_TYPE_SRCBLK:
		 l2entry_t->process = FDB_TYPE_SRCBLK;
		 l2entry_t->memberPortMask = 0;
		 break;
	case FDB_TYPE_TRAPCPU:
		 l2entry_t->process = FDB_TYPE_TRAPCPU;
		 l2entry_t->memberPortMask = portMask;
	 	 break;
	default: assert(0);
	}
	
	/*write the SW l2 entry */
	
	if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
	{	
		SLIST_FOREACH(tmpL2, &(fdb_t->database[rowIdx]), nextFDB) 
		{
			 if (memcmp(&(tmpL2->macAddr), macAddr, 6)== 0)
			{
				if(	(tmpL2->auth != auth) ||
					(tmpL2->process != type) || 
					(tmpL2->SrcBlk != SrcBlk) ||	
					(tmpL2->memberPortMask != portMask) ||
					(tmpL2->l2type != l2Type)	)
				{
					tmpL2->auth				= auth;
					tmpL2->process 			= type;
					tmpL2->SrcBlk 			= SrcBlk;
					tmpL2->memberPortMask	= portMask;
					tmpL2->l2type 			= l2Type;

					/*duplicate entry,avoid memory leak*/
					SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, l2entry_t, nextFDB);
					break;
/*					tmpL2 ->refCount = 1;*/
				}			
				else
				{
/*					tmpL2->refCount +=1;*/

					/*duplicate entry,avoid memory leak*/
					SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, l2entry_t, nextFDB);
					return SUCCESS;
				}				
			}			
			else if (tmpL2->nextFDB.sle_next == NULL) 
			{
/*				l2entry_t ->refCount = 1;*/
				tmpL2->nextFDB.sle_next = l2entry_t;
				l2entry_t->nextFDB.sle_next = NULL;
				break;
			}
		}
	}
	else 
	{
/*		l2entry_t ->refCount = 1;*/
		SLIST_INSERT_HEAD(&(fdb_t->database[rowIdx]), l2entry_t, nextFDB);
	}
	
	/* TypeII entry can not exceed RTL8651_L2TBL_COLUMN */
/*	if (typeII == RTL8651_L2TBL_COLUMN && l2Type == RTL8651_L2_TYPEII) 
		return RTL_NOFREEBUFFER;
*/	
	#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	if (portMask & (1<<PKTHDR_EXTPORT_P3))
		rtl865x_raiseEvent(EVENT_ADD_FDB, (void *)(l2entry_t));
	#endif
	
	return SUCCESS;
}

/*
@func int32 | rtl8651_delFilterDatabaseEntry | Remove a filter database entry.
@parm uint16 | fid | Filter database entry.
@parm ether_addr_t * | macAddr | Pointer to a MAC Address.
@rvalue RTL_EINVALIDFID | Filter database ID.
@rvalue RTL_NULLMACADDR | The specified MAC address is NULL.
@rvalue RTL_EINVALIDINPUT | Invalid input parameter.
*/
int32 rtl865x_delFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr) {
	int32 retval;
	unsigned long flags=0;
	if (fid >= RTL865x_FDB_NUMBER)
		return RTL_EINVALIDINPUT;
	if (macAddr == (ether_addr_t *)NULL)
		return RTL_EINVALIDINPUT;
	if (sw_FDB_Table.filterDB[fid].valid == 0)
		return RTL_EINVALIDFID;
	
/*	L2 lock		*/
	//rtl_down_interruptible(&l2_sem);
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, fid, macAddr);
/*	L2 unlock		*/
	//rtl_up(&l2_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return retval;
}

int32 _rtl865x_delFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr) 
{
	uint32 rowIdx = 0;
	uint32 colIdx = 0;
	rtl865x_tblAsicDrv_l2Param_t L2temp, *L2buff ;
	rtl865x_filterDbTableEntry_t * l2entry_t = NULL;
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	int32 found = FALSE;
	//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);

	L2buff = &L2temp;
	memset(L2buff, 0 , sizeof (rtl865x_tblAsicDrv_l2Param_t));
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		if ((rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff))==SUCCESS)
		{
			/*check whether mac address has been learned  to SW or not*/
			if (memcmp(&(L2buff->macAddr), macAddr, 6)== 0)
			{	
				/*the entry has been auto learned*/
				found = TRUE;
				break;
			}
		}
	}

	/*
	when a sta from eth0 to wlan0, the layer driver fdb will be deleted, but linux bridge fdb still exist,
	so delete the asic entry anyway to avoid layer connection issue.
	eg:
	first moment .sta is at eth0 
	second moment:connect sta to wlan0, the layer driver fdb will be deleted, but linux bridge fdb still there
	third moment:move sta to eth0 again, layer driver fdb won't be created due to linux bridge fdb already exist.
	*/
	if (found == TRUE)
	{
		rtl8651_delAsicL2Table(rowIdx, colIdx);
	}
	
	SLIST_FOREACH(l2entry_t, &(fdb_t->database[rowIdx]), nextFDB)
	{
		if (!memcmp(&l2entry_t->macAddr, macAddr, sizeof(ether_addr_t))) 
		{
	/*		l2entry_t->refCount -= 1;*/
	/*		if (l2entry_t->refCount == 0)*/
			{
				SLIST_REMOVE(
					&(fdb_t->database[rowIdx]), 
					l2entry_t, 
					 rtl865x_filterDbTableEntry_s,
					 nextFDB
				);
				SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, l2entry_t, nextFDB);	
				
				rtl865x_raiseEvent(EVENT_DEL_FDB, (void *)(l2entry_t));
			}
			break;
		}
	}
	return SUCCESS;	
}

#ifdef CONFIG_RTL865X_SYNC_L2
int32 rtl865x_arrangeFdbEntry(const unsigned char *timeout_addr, int32 *port)
{
	int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	rtl865x_filterDbTableEntry_t		l2temp_entry;
	int32 rowIdx;
	
	macAddr = (ether_addr_t *)(timeout_addr);
	rowIdx = rtl8651_filterDbIndex(macAddr, RTL_LAN_FID);
	found = rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
	if (found != SUCCESS)
	{
		if (rtl865x_lookup_FilterDatabaseEntry(RTL_LAN_FID, macAddr, &l2temp_entry) == SUCCESS)
		{
/*			printk("\nlook up sw fdb success\n");
			printk("\nport mask is %x\n", l2temp_entry.memberPortMask);
*/			
			*port =  rtl865x_ConvertPortMasktoPortNum(l2temp_entry.memberPortMask);
			return RTL865X_FDBENTRY_TIMEOUT;
		}

	}
	else
	{
		if(fdbEntry.ageSec == 450)
		{
			return RTL865X_FDBENTRY_450SEC;
		}
		if(fdbEntry.ageSec == 300)
		{
			return RTL865X_FDBENTRY_300SEC;
		}
		if(fdbEntry.ageSec == 150)
		{
			return RTL865X_FDBENTRY_150SEC;
		}			
	}
	return FAILED;
}

void update_hw_l2table(const char *srcName,const unsigned char *addr)
{

	//int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 ret = 0;
#if defined (CONFIG_RTL_LAYERED_DRIVER)	&& defined (CONFIG_RTL_LAYERED_DRIVER_L2)
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
#endif
	macAddr = (ether_addr_t *)(addr);
	
	if (memcmp(srcName, RTL_WLAN_NAME, 4) ==0)
	{

#if defined (CONFIG_RTL_LAYERED_DRIVER)
#if defined (CONFIG_RTL_LAYERED_DRIVER_L2)
		if (rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS)
#endif
#else
		if (rtl8651_lookupL2table(0, macAddr, FDB_DYNAMIC) == SUCCESS)				
#endif				
		{

			if((fdbEntry.memberPortMask & RTL8651_PHYSICALPORTMASK)!=0)
			{
#if defined (CONFIG_RTL_LAYERED_DRIVER)
#if defined (CONFIG_RTL_LAYERED_DRIVER_L2)
				ret = rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_LAN_FID, macAddr);
#endif
#else
				ret = rtl8651_delFilterDatabaseEntry(0, macAddr);
#endif
			}
		
		}

#if defined (CONFIG_RTL_IVL_SUPPORT)

#if defined (CONFIG_RTL_LAYERED_DRIVER)
#if defined (CONFIG_RTL_LAYERED_DRIVER_L2)
		if (rtl865x_Lookup_fdb_entry(RTL_WAN_FID, macAddr, FDB_DYNAMIC, &column,&fdbEntry) == SUCCESS)
#endif
#endif
		{

			if((fdbEntry.memberPortMask & RTL8651_PHYSICALPORTMASK)!=0)
			{
#if defined (CONFIG_RTL_LAYERED_DRIVER)
#if defined (CONFIG_RTL_LAYERED_DRIVER_L2)
				ret = rtl865x_delFilterDatabaseEntry(RTL865x_L2_TYPEII, RTL_WAN_FID, macAddr);
#endif
#endif
			}
		
		}
#endif

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8370_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
		{
		extern void del_83XX_L2(const unsigned char *addr);
		del_83XX_L2(addr);	
		}	
#endif
	}

		
}

EXPORT_SYMBOL(update_hw_l2table); 

int32 rtl865x_addFDBEntry(const unsigned char *addr)
{
	int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 ret=FAILED;
	int8 port_num = -1;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	rtl865x_filterDbTableEntry_t		l2temp_entry;
	
	if (addr == NULL)
		return RTL_EINVALIDINPUT;
	//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
	macAddr = (ether_addr_t *)(addr);
	found = rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
	if (found == SUCCESS )
	{
		port_num = rtl865x_ConvertPortMasktoPortNum(fdbEntry.memberPortMask);
		//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
		//printk("\nbefore rtl865x_lookup_FilterDatabaseEntry, port is %d\n", port_num);	
		if (rtl865x_lookup_FilterDatabaseEntry(fdbEntry.fid, macAddr, &l2temp_entry) != SUCCESS)
		{
			l2temp_entry.l2type = (fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII;
			l2temp_entry.process = FDB_TYPE_FWD;
			l2temp_entry.memberPortMask = fdbEntry.memberPortMask;
/*			l2temp_entry.auth = TRUE;*/
/*			l2temp_entry.SrcBlk = FALSE;*/
			memcpy(&(l2temp_entry.macAddr), macAddr, sizeof(ether_addr_t));
			ret =_rtl865x_addFilterDatabaseEntry(	l2temp_entry.l2type, 
		        										fdbEntry.fid,  
		        										&(l2temp_entry.macAddr),
		        										l2temp_entry.process,
		        										l2temp_entry.memberPortMask,
		        										FALSE, 
		        										FALSE);		
		}
	}
	else
	{
		//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
		/*add */
	}
	return ret;
}

#if defined (CONFIG_RTL_EXT_PORT_SUPPORT)
int32 rtl865x_addExtFDBEntry(const unsigned char *addr,int extPort)
{

	int32 ret=FAILED;
	
#if 1
	int32 found = FAILED;
	ether_addr_t *macAddr;
	//int32 column;
	//rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	rtl865x_filterDbTableEntry_t l2_entry;
	//printk("addr:%x:%x:%x:%x:%x:%x,extPort:%d,[%s][%d].\n",addr[0],addr[1],addr[2],addr[3], addr[4],addr[5],extPort,  __FUNCTION__, __LINE__);
	/*only use PKTHDR_EXTPORT_P3 now, to-do*/
	if(extPort!=PKTHDR_EXTPORT_P3)
		return RTL_EINVALIDINPUT;
	
	if (addr == NULL)
		return RTL_EINVALIDINPUT;
	macAddr = (ether_addr_t *)(addr);
	//found = rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_STATIC, &column, &fdbEntry);
	found=rtl865x_lookup_FilterDatabaseEntry(RTL_LAN_FID, macAddr, &l2_entry) ;
	if (found == SUCCESS )
	{
		#if 0
		port_num = rtl865x_ConvertPortMasktoPortNum(fdbEntry.memberPortMask);
		//printk("[%s][%d].\n", __FUNCTION__, __LINE__);
		//printk("\nbefore rtl865x_lookup_FilterDatabaseEntry, port is %d\n", port_num);	
		if (rtl865x_lookup_FilterDatabaseEntry(fdbEntry.fid, macAddr, &l2temp_entry) != SUCCESS)
		{
			l2temp_entry.l2type = RTL865x_L2_TYPEIII;
			l2temp_entry.process = FDB_TYPE_FWD;
			l2temp_entry.memberPortMask = fdbEntry.memberPortMask;

			memcpy(&(l2temp_entry.macAddr), macAddr, sizeof(ether_addr_t));
			ret =_rtl865x_addFilterDatabaseEntry(	l2temp_entry.l2type, 
		        										fdbEntry.fid,  
		        										&(l2temp_entry.macAddr),
		        										l2temp_entry.process,
		        										l2temp_entry.memberPortMask,
		        										TRUE, 
		        										FALSE);		
		}
		#endif
	}
	else
	{
		//printk("addr:%x:%x:%x:%x:%x:%x,extPort:%d,[%s][%d].\n",addr[0],addr[1],addr[2],addr[3], addr[4],addr[5],extPort,	__FUNCTION__, __LINE__);
		/*add */
		if(extPort==PKTHDR_EXTPORT_P3)
			ret = _rtl865x_addFilterDatabaseEntry(RTL865x_L2_TYPEIII, RTL_LAN_FID, macAddr, FDB_TYPE_FWD, (1<<PKTHDR_EXTPORT_P3), TRUE, FALSE);
		
	}
	#endif
	return ret;
}

int32 _rtl865x_DelFDBEntryByType(int32 l2Type)
{
	int32 index = 0;
	int32 index1 = 0;	
  	rtl865x_filterDbTable_t *filterDbPtr;
	rtl865x_filterDbTableEntry_t  *tempFilterDbPtr;
	
	for(index=0,filterDbPtr=&sw_FDB_Table.filterDB[0]; index<RTL865x_FDB_NUMBER; index++,filterDbPtr++) 
	{
		for(index1=0; index1<RTL8651_L2TBL_ROW; index1++){		
			if (SLIST_FIRST(&(filterDbPtr->database[index1]))) 
			{
				SLIST_FOREACH(tempFilterDbPtr, &(filterDbPtr->database[index1]), nextFDB) 
				{
					if(tempFilterDbPtr->l2type==l2Type)
					{
						_rtl865x_delFilterDatabaseEntry(tempFilterDbPtr->l2type,RTL_LAN_FID,&tempFilterDbPtr->macAddr);
					}
				}
			}
		}
	}
	return 0;
}

int32 rtl865x_delExtFDBEntry(void)
{
	return _rtl865x_DelFDBEntryByType(RTL865x_L2_TYPEIII);
}
	

#endif
int32 rtl865x_delLanFDBEntry(uint16 l2Type,  const unsigned char *addr)
{
	int32 ret=FAILED;
	ether_addr_t *macAddr;	

	if (addr == NULL)
	{
		return RTL_EINVALIDINPUT;
	}
	macAddr = (ether_addr_t *)(addr);
	
	ret =_rtl865x_delFilterDatabaseEntry(l2Type, RTL_LAN_FID, macAddr);		

	return ret;
}

int32 rtl865x_ConvertPortMasktoPortNum(int32 portmask)
{
	int32 i = 0;
	
	for (i = PHY0; i < EXT3; i++)
	{
		if(((portmask >> i) & 0x01) == 1)
		{
			return i;
		}		
	}
	return FAILED;
}

#if defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
extern int rtl_delFdbByMac(const unsigned char *macAddr, const char* devName);
char *rtl_getDevNameByPort(int32 port_num);
int32 rtl_check_fdb_entry_check_exist(uint32 fid, ether_addr_t *mac, uint32 flags)
{
	uint32 rowIdx;
	uint32 colIdx;
	int32 retval;
	rtl865x_tblAsicDrv_l2Param_t L2buff;

	rowIdx = rtl8651_filterDbIndex(mac, fid);
	colIdx = 0;
	
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		retval = rtl8651_getAsicL2Table(rowIdx, colIdx, &L2buff);

		if (retval !=SUCCESS ||
			memcmp(&(L2buff.macAddr), mac, 6)!= 0)
			continue;
		if (((flags&FDB_STATIC) && L2buff.isStatic) ||
			((flags&FDB_DYNAMIC) && !L2buff.isStatic)) {
			return SUCCESS;
		}
	
	}
	return FAILED;
}

int32 rtl_check_fdb_entry_check_srcBlock(uint32 fid, ether_addr_t *mac, int32 *SrcBlk)
{
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	rtl865x_filterDbTableEntry_t * l2entry_t ;
	uint32 rowIdx;
	
	rowIdx = rtl8651_filterDbIndex(mac, fid);

	if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
	{	
		SLIST_FOREACH(l2entry_t, &(fdb_t->database[rowIdx]), nextFDB) 
		{
			 if (memcmp(&(l2entry_t->macAddr), mac, 6)== 0)
			{
				*SrcBlk = l2entry_t->SrcBlk;
				return SUCCESS;
			}			
		}
	}
	return FAILED;
}

int32 lanrestrict_callbackFn_for_add_fdb(void *param)
{
	int32 port_num;
	rtl865x_filterDbTableEntry_t  *fdbEntry; 
	
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	
	fdbEntry=(rtl865x_filterDbTableEntry_t *)param;
	port_num = rtl865x_ConvertPortMasktoPortNum(fdbEntry->memberPortMask);	

	if (lan_restrict_tbl[port_num].enable == TRUE)
	{
		lan_restrict_tbl[port_num].curr_num ++;	
	}
	
	return SUCCESS;
}

int32 lanrestrict_callbackFn_for_del_fdb(void *param)
{
	int32 port_num;
	rtl865x_filterDbTableEntry_t  *fdbEntry; 
	
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	
	fdbEntry=(rtl865x_filterDbTableEntry_t *)param;
	port_num = rtl865x_ConvertPortMasktoPortNum(fdbEntry->memberPortMask);	
	if (lan_restrict_tbl[port_num].enable == TRUE)
	{
		lan_restrict_tbl[port_num].curr_num --;
		if (lan_restrict_tbl[port_num].curr_num < 0)
		{
			lan_restrict_tbl[port_num].curr_num = 0;
		}
		/*printk("\ndel authed port[%d], num:%d\n", port_num, lan_restrict_tbl[port_num].curr_num);*/
	}
	return SUCCESS;
}

static int32 lanrestrict_authadd_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_AUTHED_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=lanrestrict_callbackFn_for_add_fdb;
	rtl865x_unRegisterEvent(&eventParam);
	return SUCCESS;
}

static int32 lanrestrict_authdel_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_AUTHED_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=lanrestrict_callbackFn_for_del_fdb;
	rtl865x_unRegisterEvent(&eventParam);
	return SUCCESS;
}

int32 lanrestrict_unRegister_event(void)
{
	lanrestrict_authadd_unRegister_event();
	lanrestrict_authdel_unRegister_event();
	return SUCCESS;
}

static int32 lanrestrict_authadd_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_AUTHED_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=lanrestrict_callbackFn_for_add_fdb;
	rtl865x_registerEvent(&eventParam);
	
	#if 0
	if (rtl865x_registerEvent(&eventParam)== SUCCESS)
	{
		printk("register Event SUCCESS:%x\n",EVENT_ADD_AUTHED_FDB);
		return SUCCESS;
	}
	else if(rtl865x_registerEvent(&eventParam)== RTL_EENTRYALREADYEXIST)
	{
		printk("the Event already exist\n");
		return SUCCESS;
	}
	else
	{
		printk("register Event FAILED:%d \n",rtl865x_registerEvent(&eventParam));
		return FAILED;
	}
	#endif
	return SUCCESS;
}

static int32 lanrestrict_authdel_register_event(void)
{
	
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_AUTHED_FDB;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=lanrestrict_callbackFn_for_del_fdb;
	rtl865x_registerEvent(&eventParam);
	
	#if 0
	if (rtl865x_registerEvent(&eventParam)== SUCCESS)
	{
		printk("register Event SUCCESS:%x\n",EVENT_DEL_AUTHED_FDB);
		return SUCCESS;
	}
	else if(rtl865x_registerEvent(&eventParam)== RTL_EENTRYALREADYEXIST)
	{
		printk("the Event already exist\n");
		return SUCCESS;
	}
	else
	{
		printk("register Event FAILED:%d \n",rtl865x_registerEvent(&eventParam));
		return FAILED;
	}
	#endif
	return SUCCESS;
}

int32 lanrestrict_register_event(void)
{
	lanrestrict_authadd_register_event();
	lanrestrict_authdel_register_event();
	return SUCCESS;
}

int32 _rtl865x_addAuthFilterDatabaseEntry(uint16 l2Type, uint16 fid,  ether_addr_t * macAddr, uint32 type, uint32 portMask, uint8 auth, uint8 SrcBlk)
{
	uint32 rowIdx = 0;
	uint32 colIdx = 0;
	uint32 col_num = 0;
	uint32 col_tmp = 0;
	uint16 tmp_age = 0xffff;
	int32   found = FALSE;
	int32   flag = FALSE;
	int32   nexthp_flag = FALSE;
	int32  isStatic = FALSE;
	int32  toCpu = FALSE;
	rtl865x_filterDbTableEntry_t * l2entry_t = NULL;
	rtl865x_filterDbTableEntry_t *tmpL2;
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	rtl865x_tblAsicDrv_l2Param_t l2entry_tmp,*L2buff;
	int32 retval = 0;
	int32 overwite_blk_flag = FALSE;
	
	//printk("auth:%d,srcblk:%d\n",auth,SrcBlk);
	L2buff = &l2entry_tmp;
	memset(L2buff,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		retval = rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff);		
		if ((retval)==SUCCESS)
		{
			/*check whether mac address has been learned  to HW or not*/
			if (memcmp(&(L2buff->macAddr), macAddr, 6)== 0)
			{	
				/*the entry has been auto learned*/
				if (L2buff->memberPortMask==portMask)
				{
					/*the entry has been auto learned        */
					if ((L2buff->srcBlk == TRUE) && (L2buff->auth == FALSE))
					{
						overwite_blk_flag = TRUE;
					}
					found = TRUE;
					
					col_tmp = colIdx;
				}
				else
				/*portmask is different , it should be overwrited*/	
				{
					found = FALSE;
					flag = TRUE;
					
				}
				break;
			}
			/*no matched entry, try get minimum aging time L2 Asic entry*/
			if (tmp_age> L2buff->ageSec)
			{
				tmp_age = L2buff->ageSec;
				col_num = colIdx;
			}
		}
		else
		{
			/*there is still empty l2 asic entry*/
			
			flag = TRUE;
			break;
		}
	}
	/*printk("found:%d,overwite_blk_flag:%d",found, overwite_blk_flag);*/
	switch(l2Type) {	
	case RTL865x_L2_TYPEI:
		nexthp_flag = FALSE;isStatic = FALSE;
		break;
	case RTL865x_L2_TYPEII:
		nexthp_flag = TRUE; isStatic = TRUE;
		break;
	case RTL865x_L2_TYPEIII:
		nexthp_flag = FALSE;isStatic = TRUE;
		break;
	default: assert(0);	
	}

	switch(type) {	
	case FDB_TYPE_FWD:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_DSTBLK:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_SRCBLK:
			toCpu =  FALSE;
			break;
	case FDB_TYPE_TRAPCPU:
			toCpu =  TRUE;
			break;
	default: assert(0);
	}

	if (found == FALSE)
	{			
		/*no empty entry, overwrite the biggest aging time asic l2 entry*/
		if(flag == FALSE)
		{
			/*delete the biggest aging time software entry*/
			rtl8651_getAsicL2Table(rowIdx, col_num, L2buff);	
			
			_rtl865x_removeFilterDatabaseEntry(fid, &(L2buff->macAddr), rowIdx);	
			/*overwrite asic entry*/
			rtl8651_setAsicL2Table_Patch(
					rowIdx, 
					col_num, 
					macAddr, 
					toCpu, 
					SrcBlk, 
					portMask, 
					arpAgingTime, 
					isStatic, 
					nexthp_flag,
					fid,
					auth);			
			col_tmp = col_num;
		}
		/*portmask is different , so it should overwrite the original asic entry. Or there is empty entry, set it to asic*/
		else if(flag == TRUE)
		{
			
			rtl8651_setAsicL2Table_Patch(
					rowIdx, 
					colIdx, 
					macAddr, 
					toCpu, 
					SrcBlk, 
					portMask, 
					arpAgingTime, 
					isStatic, 
					nexthp_flag,
					fid,
					auth);
			col_tmp = colIdx;
		}
	}
	/*find the same asic entry, should update the aging time*/
	else
	{
		
		rtl8651_setAsicL2Table_Patch(
				rowIdx, 
				col_tmp, 
				macAddr, 
				toCpu, 
				SrcBlk, 
				portMask, 
				arpAgingTime, 
				isStatic, 
				nexthp_flag,
				fid,
				auth);		
	}

/*	
	colIdx=0;

	tmpL2 = SLIST_FIRST(&(fdb_t->database[rowIdx]));
	tmpL2->nextFDB.sle_next  = NULL;
	
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		if ((rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff))==SUCCESS)
		{
			l2entry_t->asicPos =colIdx;
			l2entry_t->auth = L2buff->auth;
			l2entry_t->configToAsic = 0;
			l2entry_t->memberPortMask = L2buff->memberPortMask;
			l2entry_t->SrcBlk = L2buff->srcBlk;
			memcpy(&l2entry_t->macAddr, &L2buff->macAddr, sizeof(ether_addr_t));

			tmpL2->nextFDB.sle_next = l2entry_t;
			l2entry_t->nextFDB.sle_next = NULL;
			tmpL2 = l2entry_t;
		}
	}
*/

	if (SLIST_FIRST(&sw_FDB_Table.freefdbList.filterDBentry) == NULL)
	{
		
		return RTL_ENOFREEBUFFER;
	}
	/*config the SW l2 entry */
	l2entry_t = SLIST_FIRST(&sw_FDB_Table.freefdbList.filterDBentry);
	SLIST_REMOVE_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, nextFDB);
	
	l2entry_t->asicPos =col_tmp;
	l2entry_t->auth = auth;
	l2entry_t->configToAsic = 0;
	l2entry_t->memberPortMask = portMask;
	l2entry_t->l2type = l2Type;
	l2entry_t->nhFlag = nexthp_flag;
	
	l2entry_t->SrcBlk = SrcBlk;
	memcpy(&l2entry_t->macAddr, macAddr, sizeof(ether_addr_t));
	switch(type) {	
	case FDB_TYPE_FWD:
	 	 l2entry_t->process = FDB_TYPE_FWD;
		 l2entry_t->memberPortMask = portMask;
		 break;
	case FDB_TYPE_DSTBLK:
		 l2entry_t->process = FDB_TYPE_DSTBLK;
		 l2entry_t->memberPortMask = 0;
		 break;
	case FDB_TYPE_SRCBLK:
		 l2entry_t->process = FDB_TYPE_SRCBLK;
		 l2entry_t->memberPortMask = 0;
		 break;
	case FDB_TYPE_TRAPCPU:
		 l2entry_t->process = FDB_TYPE_TRAPCPU;
		 l2entry_t->memberPortMask = portMask;
	 	 break;
	default: assert(0);
	}

	/*write the SW l2 entry */
check_swfdb:	
	if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
	{	
		SLIST_FOREACH(tmpL2, &(fdb_t->database[rowIdx]), nextFDB) 
		{
			 if (memcmp(&(tmpL2->macAddr), macAddr, 6)== 0)
			{
				if(	(tmpL2->auth != auth) ||
					(tmpL2->process != type) || 
					(tmpL2->SrcBlk != SrcBlk) ||	
					(tmpL2->memberPortMask != portMask) ||
					(tmpL2->l2type != l2Type)	)
				{
					tmpL2->auth				= auth;
					tmpL2->process 			= type;
					tmpL2->SrcBlk 			= SrcBlk;
					tmpL2->memberPortMask	= portMask;
					tmpL2->l2type 			= l2Type;
					/*tmpL2 ->refCount = 0;*/
					break;
				}
				else
				{
					return SUCCESS;
					//goto out; appear errorr:label used but not defined 
				}
			}		
			/*try to check whether there is timeout sw fdb entry*/
			else if(tmpL2->asicPos == col_tmp)
			{
			/*	if ((tmpL2->SrcBlk == TRUE) && (tmpL2->auth == FALSE))*/
				{
					_rtl865x_removeFilterDatabaseEntry(fid, &(tmpL2->macAddr), rowIdx);
					goto check_swfdb;
				}
			}
			else if (tmpL2->nextFDB.sle_next == NULL) 
			{
/*				l2entry_t->refCount = 0;*/
				tmpL2->nextFDB.sle_next = l2entry_t;
				l2entry_t->nextFDB.sle_next = NULL;
				break;
			}
		}
	}
	else 
	{
/*		l2entry_t ->refCount = 0;*/
		
		SLIST_INSERT_HEAD(&(fdb_t->database[rowIdx]), l2entry_t, nextFDB);
	}
	
	/* TypeII entry can not exceed RTL8651_L2TBL_COLUMN */
/*	if (typeII == RTL8651_L2TBL_COLUMN && l2Type == RTL8651_L2_TYPEII) 
		return RTL_NOFREEBUFFER;
*/

	if (overwite_blk_flag != TRUE)
	{
		/*Add a new entry*/
		rtl865x_raiseEvent(EVENT_ADD_AUTHED_FDB, (void *)(l2entry_t));
	}
	else
	{
			
		/*printk("\nover blk entry, no raise event\n");*/
	}
		
//out:
	return SUCCESS;
}


int32 _rtl865x_addAuthSWl2Entry(uint16 l2Type, uint16 fid,  ether_addr_t * macAddr, uint32 type, int32 port, uint8 auth, uint8 SrcBlk)
{
	uint32 rowIdx = 0;
	int32   nexthp_flag = FALSE;
	rtl865x_filterDbTableEntry_t * l2entry_t = NULL;
	rtl865x_filterDbTableEntry_t *tmpL2;
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	rtl865x_tblAsicDrv_l2Param_t l2entry_tmp,*L2buff;
	int32 overwite_blk_flag = FALSE;
	
	L2buff = &l2entry_tmp;
	memset(L2buff,0,sizeof(rtl865x_tblAsicDrv_l2Param_t));
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);
	

	if (SLIST_FIRST(&sw_FDB_Table.freefdbList.filterDBentry) == NULL)
		return RTL_ENOFREEBUFFER;
	
	/*config the SW l2 entry */
	l2entry_t = SLIST_FIRST(&sw_FDB_Table.freefdbList.filterDBentry);
	SLIST_REMOVE_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, nextFDB);
	
	l2entry_t->auth = auth;
	l2entry_t->configToAsic = 0;
	l2entry_t->memberPortMask = 1<<port;
	l2entry_t->l2type = l2Type;
	l2entry_t->nhFlag = nexthp_flag;
	
	l2entry_t->SrcBlk = SrcBlk;
	memcpy(&l2entry_t->macAddr, macAddr, sizeof(ether_addr_t));
	switch(type) {	
	case FDB_TYPE_FWD:
	 	 l2entry_t->process = FDB_TYPE_FWD;
		 l2entry_t->memberPortMask = 1<<port;
		 break;
	case FDB_TYPE_DSTBLK:
		 l2entry_t->process = FDB_TYPE_DSTBLK;
		 l2entry_t->memberPortMask = 0;
		 break;
	case FDB_TYPE_SRCBLK:
		 l2entry_t->process = FDB_TYPE_SRCBLK;
		 l2entry_t->memberPortMask = 0;
		 break;
	case FDB_TYPE_TRAPCPU:
		 l2entry_t->process = FDB_TYPE_TRAPCPU;
		 l2entry_t->memberPortMask = 1<<port;
	 	 break;
	default: assert(0);
	}

	/*write the SW l2 entry */	
	if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
	{	
		SLIST_FOREACH(tmpL2, &(fdb_t->database[rowIdx]), nextFDB) 
		{
			 if (memcmp(&(tmpL2->macAddr), macAddr, 6)== 0)
			{
				if(	(tmpL2->auth != auth) ||
					(tmpL2->process != type) || 
					(tmpL2->SrcBlk != SrcBlk) ||	
					(tmpL2->memberPortMask != 1<<port) ||
					(tmpL2->l2type != l2Type)	)
				{

					if ( ( tmpL2->auth == FALSE) && (tmpL2->SrcBlk == TRUE))
					{
						overwite_blk_flag = TRUE;
					}
					tmpL2->auth				= auth;
					tmpL2->process 			= type;
					tmpL2->SrcBlk 			= SrcBlk;
					tmpL2->memberPortMask	= 1<<port;
					tmpL2->l2type 			= l2Type;
/*					tmpL2 ->refCount = 0;*/
					break;
				}
				else
				{
					SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, l2entry_t, nextFDB);					
					//goto out;
					return SUCCESS;
				}
			}		
			
			else if (tmpL2->nextFDB.sle_next == NULL) 
			{
/*				l2entry_t->refCount = 0;*/
				tmpL2->nextFDB.sle_next = l2entry_t;
				l2entry_t->nextFDB.sle_next = NULL;
				break;
			}
		}
	}
	else 
	{
/*		l2entry_t ->refCount = 0;*/
		SLIST_INSERT_HEAD(&(fdb_t->database[rowIdx]), l2entry_t, nextFDB);
	}
	
	/* TypeII entry can not exceed RTL8651_L2TBL_COLUMN */
/*	if (typeII == RTL8651_L2TBL_COLUMN && l2Type == RTL8651_L2_TYPEII) 
		return RTL_NOFREEBUFFER;
*/

	if (overwite_blk_flag != TRUE)
	{
		//printk("\nauth:,srcblk:\n",auth,SrcBlk);
		rtl865x_raiseEvent(EVENT_ADD_AUTHED_FDB, (void *)(l2entry_t));
	}
	else
	{
		/*printk("\nNO need to add currnum,over blk entry, no raise event\n");*/
	}
	
//out:
	return SUCCESS;
}


int32 rtl865x_addAuthFilterDatabaseEntryExtension( uint16 fid, rtl865x_filterDbTableEntry_t * L2entry)
{
        int32 retval;
	unsigned long flags=0;	
        if (L2entry->process != FDB_TYPE_FWD && L2entry->process != FDB_TYPE_SRCBLK && L2entry->process != FDB_TYPE_TRAPCPU)
                return RTL_EINVALIDINPUT; /* Invalid parameter */
 
        if (fid >= RTL865x_FDB_NUMBER)
                return RTL_EINVALIDINPUT;
        if (&(L2entry->macAddr) == (ether_addr_t *)NULL)
                return RTL_EINVALIDINPUT;
        if (sw_FDB_Table.filterDB[fid].valid == 0)
                return RTL_EINVALIDFID;
	
        /*l2 lock*/
	//rtl_down_interruptible(&l2_sem);	
	SMP_LOCK_ETH(flags);	
        retval = _rtl865x_addAuthFilterDatabaseEntry(	L2entry->l2type, 
        										fid,  
        										&(L2entry->macAddr),
        										L2entry->process,
        										L2entry->memberPortMask,
        										L2entry->auth, 
        										L2entry->SrcBlk);
        /*l2 unlock*/
	//rtl_up(&l2_sem);
 	SMP_UNLOCK_ETH(flags);
        return retval;
}


/*
@func int32 | rtl8651_delFilterDatabaseEntry | Remove a filter database entry.
@parm uint16 | fid | Filter database entry.
@parm ether_addr_t * | macAddr | Pointer to a MAC Address.
@rvalue RTL_EINVALIDFID | Filter database ID.
@rvalue RTL_NULLMACADDR | The specified MAC address is NULL.
@rvalue RTL_EINVALIDINPUT | Invalid input parameter.
*/
int32 rtl865x_delAuthFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr) {
	int32 retval;
	unsigned long flags=0;	
	if (fid >= RTL865x_FDB_NUMBER)
		return RTL_EINVALIDINPUT;
	if (macAddr == (ether_addr_t *)NULL)
		return RTL_EINVALIDINPUT;
	if (sw_FDB_Table.filterDB[fid].valid == 0)
		return RTL_EINVALIDFID;
	
/*	L2 lock		*/
	//rtl_down_interruptible(&l2_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delAuthFilterDatabaseEntry(RTL865x_L2_TYPEII, fid, macAddr);
/*	L2 unlock		*/
	//rtl_up(&l2_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}


int32 _rtl865x_delAuthFilterDatabaseEntry(uint16 l2Type, uint16 fid, ether_addr_t * macAddr) 
{
	uint32 rowIdx = 0;
	uint32 colIdx = 0;
	rtl865x_tblAsicDrv_l2Param_t L2temp, *L2buff ;
	rtl865x_filterDbTableEntry_t * l2entry_t = NULL;
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[fid];
	int32 found = FALSE;
	
	rowIdx = rtl8651_filterDbIndex(macAddr, fid);
	/*print message
	printk("rowIdx%d:\n",rowIdx);
	printk("fid%d:\n",fid);
	printk("\n the mac address is %x %x %x %x %x %x\n", macAddr->octet[0],
													macAddr->octet[1],
													macAddr->octet[2],
													macAddr->octet[3],
													macAddr->octet[4],
													macAddr->octet[5]);*/
	L2buff = &L2temp;
	memset(L2buff, 0 , sizeof (rtl865x_tblAsicDrv_l2Param_t));
	for(colIdx=0; colIdx<RTL8651_L2TBL_COLUMN; colIdx++) 
	{
		if ((rtl8651_getAsicL2Table(rowIdx, colIdx, L2buff))==SUCCESS)
		{
			/*printk("\n mac address is %x %x %x %x %x %x\n", L2buff->macAddr.octet[0],
															L2buff->macAddr.octet[1],
															L2buff->macAddr.octet[2],
															L2buff->macAddr.octet[3],
															L2buff->macAddr.octet[4],
															L2buff->macAddr.octet[5]);*/

			/*check whether mac address has been learned  to SW or not*/
			if (memcmp(&(L2buff->macAddr), macAddr, 6)== 0)
			{	
				/*the entry has been auto learned*/
				found = TRUE;
				break;
			}
		}
	}
	
	SLIST_FOREACH(l2entry_t, &(fdb_t->database[rowIdx]), nextFDB)
	{
		
	if (!memcmp(&l2entry_t->macAddr, macAddr, sizeof(ether_addr_t))) 
	{
	/*		l2entry_t->refCount -= 1;*/
	/*		if (l2entry_t->refCount == 0)*/

		SLIST_REMOVE(
			&(fdb_t->database[rowIdx]), 
			l2entry_t, 
			 rtl865x_filterDbTableEntry_s,
			 nextFDB
		);
		SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, l2entry_t, nextFDB);
		if (found == TRUE)
		{
			rtl8651_delAsicL2Table(rowIdx, colIdx);
		}
		/*printk("l2entry_t->srcblk:%d,l2entry_t->auth:%d\n", l2entry_t->SrcBlk,l2entry_t->auth);*/
		if ((l2entry_t->SrcBlk == FALSE) && (l2entry_t->auth == TRUE))
		{	
			uint32 memberPortMask_t;
			memberPortMask_t=l2entry_t->memberPortMask;

			/*after del an auth fdb, consider add an auth fdb*/

			#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2)&& defined(CONFIG_RTL865X_SYNC_L2)
			{
				unsigned char swap_addr[ETHER_ADDR_LEN];
				int32 port_num;
				/*try to find blocked l2 entry, then set it to authed*/
				port_num = rtl865x_ConvertPortMasktoPortNum(memberPortMask_t);
				if (port_num != FAILED)
				{	
					/*printk("\ntime out port num is %d\n", port_num);*/
					if ((lan_restrict_getBlockAddr(port_num, swap_addr)) == SUCCESS)
					{
							//struct hlist_head *head = &br->hash[br_mac_hash(swap_addr)];
							/*									
							printk("\n arrange block entry is %x %x %x %x %x %x\n", swap_addr[0],
																	swap_addr[1],
																	swap_addr[2],
																	swap_addr[3],
																	swap_addr[4],
													 				swap_addr[5]);
																										
										*/
						rtl865x_addAuthFDBEntry(swap_addr, TRUE, port_num, FALSE);
					}
					
				}
				else
				{/*port_num==FAILED*/}
			}						
			#endif	/* #if defined (CONFIG_RTL865X_LANPORT_RESTRICTION) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2) && defined(CONFIG_RTL865X_SYNC_L2)	*/	//consider add an auth fdb entry
				
		}

		//SLIST_INSERT_HEAD(&sw_FDB_Table.freefdbList.filterDBentry, l2entry_t, nextFDB);
		rtl865x_raiseEvent(EVENT_DEL_FDB, (void *)(l2entry_t));
		//if(lanrestrict_authdel_register_event()==SUCCESS);
		rtl865x_raiseEvent(EVENT_DEL_AUTHED_FDB, (void *)(l2entry_t));

	break;
	}
	} /*SLIST_FOREACH*/
	return SUCCESS;	
}

int32 rtl865x_check_authfdbentry_Byport(int32 port_num, const unsigned char  *macAddr)
{
	uint32  rowIdx;
	int32 retval = FAILED;
	rtl865x_tblAsicDrv_l2Param_t *L2buff, l2temp;
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[0];
	rtl865x_filterDbTableEntry_t * l2entry_t ;
	
	L2buff = &l2temp;
	memset(L2buff, 0, sizeof(rtl865x_tblAsicDrv_l2Param_t));
	for(rowIdx=0; rowIdx<RTL8651_L2TBL_ROW; rowIdx++)
	{
		if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
		{
			
			SLIST_FOREACH(l2entry_t, &(fdb_t->database[rowIdx]), nextFDB) 
			{
				if ( (l2entry_t->memberPortMask&(1<<port_num)) && (l2entry_t ->auth == FALSE) && (l2entry_t->SrcBlk == TRUE))
				{
					
					memcpy((ether_addr_t *)macAddr, &(l2entry_t->macAddr), sizeof(ether_addr_t));
#if 0
					printk("\nfind block entry, rowIdx is %d, address2 is %x %x %x %x %x %x\n", rowIdx, ((ether_addr_t *)macAddr)->octet[0],
																				((ether_addr_t *)macAddr)->octet[1],
																				((ether_addr_t *)macAddr)->octet[2],
																				((ether_addr_t *)macAddr)->octet[3],
																				((ether_addr_t *)macAddr)->octet[4],
																				((ether_addr_t *)macAddr)->octet[5]);
#endif
					

					retval =  SUCCESS;
					break;
				}
			}			
		}

	}
	return retval;
}

int32 rtl865x_addAuthFDBEntry(const unsigned char *addr, int32 auth, int32  port, int32 srcblk)
{
	int32 found = FAILED;
	ether_addr_t *macAddr;
	int32 ret=FAILED;
	int8 port_num = -1;
	int32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	rtl865x_filterDbTableEntry_t		l2temp_entry;
	//int32 srcblk;
	
	if (addr == NULL)
		return RTL_EINVALIDINPUT;
	macAddr = (ether_addr_t *)(addr);
	found = rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
	/*if (auth == TRUE)
		srcblk = FALSE;
	else
		srcblk = TRUE; */
	
	if (found == SUCCESS )
	{
		port_num = rtl865x_ConvertPortMasktoPortNum(fdbEntry.memberPortMask);

		//printk("\nbefore rtl865x_lookup_FilterDatabaseEntry, port is %d, auth is %d\n", port_num, auth);	
		if (rtl865x_lookup_FilterDatabaseEntry(fdbEntry.fid, macAddr, &l2temp_entry) != SUCCESS)
		{
			
			l2temp_entry.l2type = (fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII;
			l2temp_entry.process = FDB_TYPE_FWD;
			l2temp_entry.memberPortMask = fdbEntry.memberPortMask;
/*			l2temp_entry.auth = TRUE;*/
/*			l2temp_entry.SrcBlk = FALSE;*/
			memcpy(&(l2temp_entry.macAddr), macAddr, sizeof(ether_addr_t));	
			ret =_rtl865x_addAuthFilterDatabaseEntry(	l2temp_entry.l2type, 
			        										fdbEntry.fid,  
			        										&(l2temp_entry.macAddr),
			        										l2temp_entry.process,
			        										l2temp_entry.memberPortMask,
			        										auth, 
			        										srcblk);	

		}
#if 1		
		else 
		{
			if ((l2temp_entry.auth == FALSE) && (l2temp_entry.SrcBlk == TRUE))
			{
				
				l2temp_entry.l2type = (fdbEntry.nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII;
				l2temp_entry.process = FDB_TYPE_FWD;
				l2temp_entry.memberPortMask = fdbEntry.memberPortMask;
				memcpy(&(l2temp_entry.macAddr), macAddr, sizeof(ether_addr_t));
				ret =_rtl865x_addAuthFilterDatabaseEntry(	l2temp_entry.l2type, 
				        										fdbEntry.fid,  
				        										&(l2temp_entry.macAddr),
				        										l2temp_entry.process,
				        										l2temp_entry.memberPortMask,
				        										auth, 
				        										srcblk);
			}
		}
#endif		
	}
	else
	{
		/*just add sw l2 table */
		
		l2temp_entry.l2type = RTL865x_L2_TYPEII;
		l2temp_entry.process = FDB_TYPE_FWD;
		/*			l2temp_entry.auth = TRUE;*/
		/*			l2temp_entry.SrcBlk = FALSE;*/
		memcpy(&(l2temp_entry.macAddr), macAddr, sizeof(ether_addr_t));	
		ret =_rtl865x_addAuthSWl2Entry(	l2temp_entry.l2type, 
    										0,  
    										&(l2temp_entry.macAddr),
    										l2temp_entry.process,
    										1<<port,
    										auth, 
    										srcblk);			
	}
	return ret;
}


int32 rtl865x_addAuthFDBEntry_hooks(const unsigned char *addr)
{
	int32	port_num;
	int32	ret;
	
	if ((port_num=rtl865x_getPortNum(addr))!= -1)
	{
		ret = lan_restrict_CheckStatusByport(port_num);
		
			if (ret == RTL_LAN_RESTRICT_STAT2)
		{
			//function opened, and should be authed
			rtl865x_addAuthFDBEntry(addr, TRUE, port_num, FALSE);
		} else if(ret == RTL_LAN_RESTRICT_STAT1)
		{
			//function opened, and set it to block
			rtl865x_addAuthFDBEntry(addr, FALSE, port_num, TRUE);
		
		}else  if(ret == RTL_LAN_RESTRICT_STAT0)
		{
			//function not open , no need to be authed
			rtl865x_addFDBEntry(addr);
		}
	}
	return SUCCESS;
}


int32 rtl865x_delAuthLanFDBEntry(uint16 l2Type,  const unsigned char *addr)
{
	int32 ret=FAILED;
	ether_addr_t *macAddr;	
	if (addr == NULL)
	{
		return RTL_EINVALIDINPUT;
	}
	
	macAddr = (ether_addr_t *)(addr);
	ret =_rtl865x_delAuthFilterDatabaseEntry(	l2Type, RTL_LAN_FID, macAddr);	
	return ret;
}

int32 rtl865x_BlkCheck(const unsigned char *addr)
{
	ether_addr_t *macAddr;
	rtl865x_filterDbTableEntry_t		l2temp_entry;
	
	if (addr == NULL)
		return RTL_EINVALIDINPUT;
	
	macAddr = (ether_addr_t *)(addr);
#if 0
					printk("\nin rtl865x_BlkCheck, address2 is %x %x %x %x %x %x\n", ((ether_addr_t *)macAddr)->octet[0],
																				((ether_addr_t *)macAddr)->octet[1],
																				((ether_addr_t *)macAddr)->octet[2],
																				((ether_addr_t *)macAddr)->octet[3],
																				((ether_addr_t *)macAddr)->octet[4],
																				((ether_addr_t *)macAddr)->octet[5]);
#endif

	if(rtl865x_lookup_FilterDatabaseEntry(RTL_LAN_FID, macAddr, &l2temp_entry) == SUCCESS)
	{
		if (l2temp_entry.SrcBlk == TRUE)
		{
			return TRUE;
		}
	}
	return FALSE;
}
/*
@func int32   | rtl865x_enableLanPortNumRestrict | enable max guest number restrict feature.
@parm uint8 | isEnable | enable feature or not.
@rvalue SUCCESS | enable feature.
@comm This API may change wan & lan port 802.1x mac base ablity
*/
int32 rtl865x_enableLanPortNumRestrict(uint8 isEnable)
{
	if( isEnable == TRUE)
	{
		rtl8651_setAsic802D1xMacBaseDirection(Dot1xMAC_OPDIR_IN);
		rtl8651_setAsicGuestVlanProcessControl(EN_8021X_TOCPU);	
	}
	else
	{
		rtl8651_setAsic802D1xMacBaseDirection(Dot1xMAC_OPDIR_BOTH);
		rtl8651_setAsicGuestVlanProcessControl( EN_8021X_DROP);
	}
					
	return SUCCESS;
}


/*
@func int32   	| rtl865x_setRestrictPortNum | set per port max guest number for max  host restrict feature.
@parm int32	| port 		| set port number .
@parm int8	| isEnable 	| enable max  host restrict feature or not
@parm int32 	| number 	| set max guest number 
@rvalue SUCCESS | set number for this port ok.
*/
int32 rtl865x_setRestrictPortNum(int32 port, uint8 isEnable, int32 number)
{
	int32 i = 0;
	
	if(isEnable==TRUE)
	{

		for (i = PHY0; i < EXT3; i++)
		{
			if( i == port)
			{
				rtl8651_setAsic802D1xMacBaseAbility(i, TRUE);
				break;
			}
		}
	}
	
	if(isEnable==FALSE)
	{	

		for (i = PHY0; i < EXT3; i++)
		{
			if( i == port)
			{
				rtl8651_setAsic802D1xMacBaseAbility(i, FALSE);
				break;
			}
		}
						
	}
	return SUCCESS;
}
#endif

#endif

#if defined(CONFIG_RTL_LINKCHG_PROCESS) || defined(CONFIG_RTL_IVL_SUPPORT)
int32 _rtl865x_ClearFDBEntryByPort(int32 port_num)
{
	int i, j;
	rtl865x_tblAsicDrv_l2Param_t l2entry_tmp,*L2buff;
	#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
	char *name;
	#endif


	L2buff = &l2entry_tmp;
	
	for (i = 0; i < RTL8651_L2TBL_ROW; i++)
		for (j = 0; j < RTL8651_L2TBL_COLUMN; j++)
		{
			if ((rtl8651_getAsicL2Table(i, j, L2buff))!=SUCCESS)
				continue;
			
			if ((rtl865x_ConvertPortMasktoPortNum(L2buff->memberPortMask)) != port_num)
				continue;
			
			if (L2buff->isStatic == TRUE)
				continue;

			rtl8651_delAsicL2Table(i, j);
			#ifdef CONFIG_RTL865X_LANPORT_RESTRICTION
			name = rtl_getDevNameByPort(port_num);
			if(name){
				rtl_delFdbByMac((unsigned char *)(&(L2buff->macAddr.octet)), name);
			}
			#endif
			_rtl865x_removeFilterDatabaseEntry(RTL_LAN_FID, &(L2buff->macAddr), i);
		}

	#if defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
	/*when link is down, set curr_num of port to 0 */
	lan_restrict_tble_reset_port(port_num);
	#endif

	return SUCCESS;		

}
#endif

#ifdef CONFIG_RTL_LINKCHG_PROCESS
int32 rtl865x_LinkChange_Process(void)
{
	uint32 i, status;
	
	/* Check each port. */
	for ( i = 0; i < RTL8651_MAC_NUMBER; i++ )
	{
		/* Read Port Status Register to know the port is link-up or link-down. */
		status = READ_MEM32( PSRP0 + i * 4 );
		if ( ( status & PortStatusLinkUp ) == FALSE )
		{
			/* Link is down. */
			rtl8651_setAsicEthernetLinkStatus( i, FALSE );
#if defined(CONFIG_RTL865X_LANPORT_RESTRICTION)
			_rtl865x_ClearFDBEntryByPort(i);
#endif		
			#if defined(CONFIG_RTL_HW_TX_CSUM)
			{
				extern unsigned char hw_csum_cache_mac[6];
				memset(hw_csum_cache_mac, 0, sizeof(hw_csum_cache_mac));
			}
			#endif	
		}
		else
		{
			/* Link is up. */
			rtl8651_setAsicEthernetLinkStatus( i, TRUE );
		}
	}

	return SUCCESS;

}
#endif

#ifdef CONFIG_RTL_PROC_DEBUG
#ifdef CONFIG_RTL_PROC_NEW
int32 rtl865x_sw_l2_proc_read(struct seq_file *s, void *v)
{
	int32 index = 0;
	int32 index1 = 0;	
	//int len=0;
	uint32 port, m=0;
	rtl865x_filterDbTable_t *filterDbPtr;
	rtl865x_filterDbTableEntry_t  *tempFilterDbPtr;
	
	seq_printf(s, "%s\n", "sw l2 table:");	
	
	for(index=0,filterDbPtr=&sw_FDB_Table.filterDB[0]; index<RTL865x_FDB_NUMBER; index++,filterDbPtr++) {
		for(index1=0; index1<RTL8651_L2TBL_ROW; index1++)		
			if (SLIST_FIRST(&(filterDbPtr->database[index1]))) 
			{
				SLIST_FOREACH(tempFilterDbPtr, &(filterDbPtr->database[index1]), nextFDB) 
				{
					seq_printf(s, "%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, index1, tempFilterDbPtr->asicPos, 
								tempFilterDbPtr->macAddr.octet[0], tempFilterDbPtr->macAddr.octet[1], tempFilterDbPtr->macAddr.octet[2], 
								tempFilterDbPtr->macAddr.octet[3], tempFilterDbPtr->macAddr.octet[4], tempFilterDbPtr->macAddr.octet[5], index);

					m++;

					for (port = 0 ; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ; port ++)
					{
						if (tempFilterDbPtr->memberPortMask & (1<<port))
						{
							seq_printf(s,"%d ", port);
						}
					}
					seq_printf(s,")");
					
					switch(tempFilterDbPtr->process)
					{
						case FDB_TYPE_FWD:
								seq_printf(s,"%s ", "FWD");
								break;
						case FDB_TYPE_DSTBLK:
								seq_printf(s,"%s ", "DSTBLK");
								break;
						case FDB_TYPE_SRCBLK:
								seq_printf(s,"%s ", "SRCBLK");
								break;
						case FDB_TYPE_TRAPCPU:
								seq_printf(s,"%s ", "CPU");
								break;
					}
//					seq_printf(s,"%s %s %s",tempFilterDbPtr->process?"CPU":"FWD", tempFilterDbPtr->l2type?"STA":"DYN",  tempFilterDbPtr->SrcBlk?"BLK":"");
					seq_printf(s,"%s ",  (tempFilterDbPtr->l2type != RTL865x_L2_TYPEI)?"STA":"DYN");	
					seq_printf(s,"%s ",  (tempFilterDbPtr->l2type == RTL865x_L2_TYPEII)?"NH":"");

					seq_printf(s,"%s ", tempFilterDbPtr->SrcBlk?"BLK":"");
					
					if (tempFilterDbPtr->auth)
					{
						seq_printf(s,"AUTH:%d  ",tempFilterDbPtr->auth);
					}

/*					seq_printf(s,"reference count:%d",tempFilterDbPtr->refCount);*/
					seq_printf(s,"\n");
				}
			}
		}

	return 0;
}
#else
int32 rtl865x_sw_l2_proc_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int32 index = 0;
	int32 index1 = 0;	
	int len=0;
	uint32 port, m=0;
	rtl865x_filterDbTable_t *filterDbPtr;
	rtl865x_filterDbTableEntry_t  *tempFilterDbPtr;
	
	len = sprintf(page, "%s\n", "sw l2 table:");	
	
	for(index=0,filterDbPtr=&sw_FDB_Table.filterDB[0]; index<RTL865x_FDB_NUMBER; index++,filterDbPtr++) {
		for(index1=0; index1<RTL8651_L2TBL_ROW; index1++)		
			if (SLIST_FIRST(&(filterDbPtr->database[index1]))) 
			{
				SLIST_FOREACH(tempFilterDbPtr, &(filterDbPtr->database[index1]), nextFDB) 
				{
					len += sprintf(page + len, "%4d.[%3d,%d] %02x:%02x:%02x:%02x:%02x:%02x FID:%x mbr(",m, index1, tempFilterDbPtr->asicPos, 
								tempFilterDbPtr->macAddr.octet[0], tempFilterDbPtr->macAddr.octet[1], tempFilterDbPtr->macAddr.octet[2], 
								tempFilterDbPtr->macAddr.octet[3], tempFilterDbPtr->macAddr.octet[4], tempFilterDbPtr->macAddr.octet[5], index);

					m++;

					for (port = 0 ; port < RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum ; port ++)
					{
						if (tempFilterDbPtr->memberPortMask & (1<<port))
						{
							len += sprintf(page + len,"%d ", port);
						}
					}
					len += sprintf(page + len,")");
					
					switch(tempFilterDbPtr->process)
					{
						case FDB_TYPE_FWD:
								len += sprintf(page + len,"%s ", "FWD");
								break;
						case FDB_TYPE_DSTBLK:
								len += sprintf(page + len,"%s ", "DSTBLK");
								break;
						case FDB_TYPE_SRCBLK:
								len += sprintf(page + len,"%s ", "SRCBLK");
								break;
						case FDB_TYPE_TRAPCPU:
								len += sprintf(page + len,"%s ", "CPU");
								break;
					}
//					len += sprintf(page + len,"%s %s %s",tempFilterDbPtr->process?"CPU":"FWD", tempFilterDbPtr->l2type?"STA":"DYN",  tempFilterDbPtr->SrcBlk?"BLK":"");
					len += sprintf(page + len,"%s ",  (tempFilterDbPtr->l2type != RTL865x_L2_TYPEI)?"STA":"DYN");	
					len += sprintf(page + len,"%s ",  (tempFilterDbPtr->l2type == RTL865x_L2_TYPEII)?"NH":"");

					len += sprintf(page + len,"%s ", tempFilterDbPtr->SrcBlk?"BLK":"");
					
					if (tempFilterDbPtr->auth)
					{
						len += sprintf(page + len,"AUTH:%d  ",tempFilterDbPtr->auth);
					}

/*					len += sprintf(page + len,"reference count:%d",tempFilterDbPtr->refCount);*/
					len += sprintf(page + len,"\n");
				}
			}
		}

	return len;
}
#endif

int32 rtl865x_sw_l2_proc_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	return len;
}
#endif
#if defined(CONFIG_RTL_ETH_802DOT1X_SUPPORT)
int32 rtl865x_updateAuthState(int32 auth, const unsigned char *addr)
{
    int32 found = FAILED;
    ether_addr_t *macAddr;
    int32 ret=FAILED;
    //int8 port_num = -1;
    int32 column;
    uint32 rowIdx;
    rtl865x_tblAsicDrv_l2Param_t    fdbEntry;
    //rtl865x_filterDbTableEntry_t        l2temp_entry;
	rtl865x_filterDbTable_t *fdb_t = &sw_FDB_Table.filterDB[RTL_LAN_FID];
	rtl865x_filterDbTableEntry_t *tmpL2;
    
    if (addr == NULL)
        return RTL_EINVALIDINPUT;
    //printk("[%s][%d].\n", __FUNCTION__, __LINE__);
    macAddr = (ether_addr_t *)(addr);
	rowIdx = rtl8651_filterDbIndex(macAddr, RTL_LAN_FID);
    found = rtl865x_Lookup_fdb_entry(RTL_LAN_FID, macAddr, FDB_DYNAMIC, &column, &fdbEntry);
    if (found == SUCCESS )
    {
        //port_num = rtl865x_ConvertPortMasktoPortNum(fdbEntry.memberPortMask);
        //printk("[%s][%d].\n", __FUNCTION__, __LINE__);
        //printk("\nbefore rtl865x_lookup_FilterDatabaseEntry, port is %d\n", port_num);

        if (auth != fdbEntry.auth)
        {
            //update hw l2
    		rtl8651_setAsicL2Table_Patch(
    				rowIdx, 
    				column, 
    				macAddr, 
    				fdbEntry.cpu, 
    				fdbEntry.srcBlk, 
    				fdbEntry.memberPortMask, 
    				fdbEntry.ageSec, 
    				fdbEntry.isStatic, 
    				fdbEntry.nhFlag,
    				fdbEntry.fid,
    				auth);
            //update sw l2
            if (SLIST_FIRST(&(fdb_t->database[rowIdx]))) 
            {   
                SLIST_FOREACH(tmpL2, &(fdb_t->database[rowIdx]), nextFDB) 
                {
                     if (memcmp(&(tmpL2->macAddr), macAddr, 6)== 0)
                    {
                        if((tmpL2->auth != auth))
                        {
                            tmpL2->auth             = auth;
                            break;
                        }           
                    }           
                }
            }

        }
        
    }
    else
    {
        //printk("[%s][%d].\n", __FUNCTION__, __LINE__);
        /*add */
    }
    return ret;
}
#endif

int32 rtl865x_isEthPortMask(uint32 portMask)
{
	int i;
	for(i=0; i <=RTL8651_PHY_NUMBER; i++)
	{
		
		if((1<<i) & portMask)
		{
			return TRUE;
		}
	}
	
	return FALSE;
}


int32 rtl865x_isEthMac(void  *mac)
{

	int i;
	//memset(mac, 0, sizeof(ether_addr_t));
	uint32 fdb_type[]={ FDB_STATIC, FDB_DYNAMIC };
	uint16 fid = 0;
	uint32 column;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	
	for(i=0; i<2; i++) 
	{
		 
		if(rtl865x_Lookup_fdb_entry(fid,(ether_addr_t*)mac, fdb_type[i], &column,&fdbEntry) != SUCCESS)
		{	
			continue;
		}

		if(rtl865x_isEthPortMask(fdbEntry.memberPortMask )==TRUE)
		{
			return TRUE;
		}
		
	}

	return FALSE;
}

#if defined(CONFIG_RTL_HARDWARE_NAT) //&&defined(REINIT_SWITCH_CORE)
void rtl819x_init_l2_bakup_entry(void)
{
	memset((void *)&l2_entry_bak, 0x00, sizeof(l2_entry_bak));

	return;
}

int32 rtl819x_save_inused_l2_entry(ipaddr_t ip, ether_addr_t *mac, uint32 fid)
{
	uint32 column;
	int32 ret = FAILED, i = 0;
	rtl865x_tblAsicDrv_l2Param_t	fdbEntry;
	ether_addr_t empty_mac = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

	//parameter check 
	if((mac==NULL))
	{
		return RTL_EINVALIDINPUT;
	}

	ret = rtl865x_Lookup_fdb_entry(fid, mac, FDB_DYNAMIC, &column,&fdbEntry);
	if (ret == SUCCESS){
		//found it , store it.
		for (i = 0; i < L2_BAKUP_ENTRY_MAX; i++){
			//find empty entry to store
			if (!memcmp(l2_entry_bak[i].macAddr.octet, empty_mac.octet, ETHER_ADDR_LEN)){
				memcpy((void *)&l2_entry_bak[i], (void *)&fdbEntry, sizeof(fdbEntry));
				//panic_printk("%s %d ip=0x%x mac=0x%x:%x:%x:%x:%x:%x i=%d \n", __FUNCTION__, __LINE__, ip, mac->octet[0], mac->octet[1], mac->octet[2], mac->octet[3], mac->octet[4], mac->octet[5], i);
				return SUCCESS;
			}
		}
	}

	
	return ret;
}
int rtl819x_restore_hw_l2_table(void)
{
	int32 i = 0, ret = FAILED;
	uint32 type;
	rtl865x_tblAsicDrv_l2Param_t *entry = NULL;
	ether_addr_t empty_mac = { {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} };

	for (i = 0; i < L2_BAKUP_ENTRY_MAX; i++){
		//restore non-empty entry 
		if (memcmp(l2_entry_bak[i].macAddr.octet, empty_mac.octet, ETHER_ADDR_LEN)){
			
			entry = &l2_entry_bak[i];
			if (entry->cpu)
				type = FDB_TYPE_TRAPCPU;
			else
				type = FDB_TYPE_FWD;
				
			ret = _rtl865x_addFilterDatabaseEntry((entry->nhFlag==0)?RTL865x_L2_TYPEI: RTL865x_L2_TYPEII, entry->fid, (ether_addr_t *)&entry->macAddr, type, entry->memberPortMask, entry->auth,entry->srcBlk);
			//panic_printk("%s %d ret=%d type=0x%x \n", __FUNCTION__, __LINE__, ret, type);
		}
	}

	return ret;
}

#endif

#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
int rtl_delFilterDatabaseEntryforMCast(unsigned char *mac)
{
	ether_addr_t macAddr;
	macAddr.octet[0] = mac[0];
	macAddr.octet[1] = mac[1];
	macAddr.octet[2] = mac[2];
	macAddr.octet[3] = mac[3];
	macAddr.octet[4] = mac[4];
	macAddr.octet[5] = mac[5];
 	return rtl865x_delFilterDatabaseEntry(0, 0, &macAddr);
}
int rtl_addFilterDatabaseEntryforMCast(unsigned char *mac, unsigned int fwdPortMask)
{
	ether_addr_t macAddr;
	macAddr.octet[0] = mac[0];
	macAddr.octet[1] = mac[1];
	macAddr.octet[2] = mac[2];
	macAddr.octet[3] = mac[3];
	macAddr.octet[4] = mac[4];
	macAddr.octet[5] = mac[5];
	return rtl865x_addFilterDatabaseEntry(0, &macAddr, fwdPortMask, FDB_TYPE_FWD);
}
#endif
