/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : Vlan driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "rtl_errno.h"
//#include "rtl_utils.h"
//#include "rtl_glue.h"
#include "rtl865x_vlan.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif
#include "rtl865x_eventMgr.h"


static rtl865x_vlan_entry_t *vlanTbl = NULL;

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static RTL_DECLARE_MUTEX(vlan_sem);
#endif
static int32 _rtl865x_delVlan(uint16 vid);

static int32 _rtl865x_setAsicVlan(uint16 vid,rtl865x_vlan_entry_t *vlanEntry)
{
	int32 retval = FAILED;
	rtl865x_tblAsicDrv_vlanParam_t asicEntry;
	/*add this entry to asic table*/
	asicEntry.fid = vlanEntry->fid;
	asicEntry.memberPortMask = vlanEntry->memberPortMask;
	asicEntry.untagPortMask = vlanEntry->untagPortMask;

#if defined(CONFIG_RTL_8197F)
	asicEntry.hp = vlanEntry->hp;
#endif	

	retval = rtl8651_setAsicVlan(vid,&asicEntry);
	return retval;
}

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
static rtl865x_vlan_entry_t *rtl_getVlanEntryByVid(uint16 vid)
{
	int i;
	rtl865x_vlan_entry_t *entry = NULL;

	for(i=0; i<VLANTBL_SIZE; i++)
	{
		if(vlanTbl[i].vid == vid){
			entry = &vlanTbl[i];
			break;
		}
	}

	return entry;
}

static rtl865x_vlan_entry_t *rtl_findEmptyVlanEntry(void)
{
	int i;
	rtl865x_vlan_entry_t *entry = NULL;

	for(i=0; i<VLANTBL_SIZE; i++)
	{
		if(vlanTbl[i].valid == 0){
			entry = &vlanTbl[i];
			break;
		}
	}

	return entry;
}
#endif

static int32 _rtl865x_referVlan(uint16 vid)
{
	rtl865x_vlan_entry_t *entry;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER-1)
		return RTL_EINVALIDVLANID;
	
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
	
	entry = rtl_getVlanEntryByVid(vid);
	if(entry == NULL)
		return RTL_EINVALIDVLANID;
#else

	entry = &vlanTbl[vid];
#endif
	
	if(entry->valid != 1)
		return RTL_EINVALIDVLANID;

	entry->refCnt++;
	return SUCCESS;
}

static int32 _rtl865x_deReferVlan(uint16 vid)
{
	rtl865x_vlan_entry_t *entry;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER-1)
		return RTL_EINVALIDVLANID;
	
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
	
	entry = rtl_getVlanEntryByVid(vid);
	if(entry == NULL)
		return RTL_EINVALIDVLANID;
#else

	entry = &vlanTbl[vid];
#endif
	
	if(entry->valid != 1)
		return RTL_EINVALIDVLANID;

	entry->refCnt--;
	return SUCCESS;
}

int32 _rtl865x_addVlan(uint16 vid)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *entry;

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
	entry = rtl_findEmptyVlanEntry();
	if(entry == NULL)
		return RTL_EINVALIDVLANID;
#else
	entry=&vlanTbl[vid];
	if(1 == entry->valid)
		return RTL_EVLANALREADYEXISTS;
#endif

	/*add new vlan entry*/
	memset(entry,0,sizeof(rtl865x_vlan_entry_t));
	entry->vid = vid;
	entry->valid = 1;
	entry->refCnt = 1;

	/*add this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,entry);	
	
	return retval;
}


static int32 _rtl865x_delVlan(uint16 vid)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry,org;

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)		
	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return RTL_EINVALIDVLANID;
#else
	vlanEntry = &vlanTbl[vid];
#endif
	if(0 == vlanEntry->valid)
		return RTL_EINVALIDVLANID;
	/*
	if(vlanEntry->refCnt > 1)
	{
		printk("vid(%d),reference(%d)\n",vid,vlanEntry->refCnt);
		return RTL_EREFERENCEDBYOTHER;
	}
	*/

	memcpy(&org,vlanEntry,sizeof(rtl865x_vlan_entry_t));
	
	/*delete vlan entry*/
	vlanEntry->valid =  0;
	/*ignor other member...*/

	retval = rtl8651_delAsicVlan(vid);

	if(SUCCESS == retval)
	{
		/*if vlan entry is deleted, this information should be noticed by uplayer module*/
		#if 0
		do_eventAction(EV_DEL_VLAN, (void *)&org);
		#else
		rtl865x_raiseEvent(EVENT_DEL_VLAN, (void *)&org);
		#endif
	}
	
	return retval;
}

static int32 _rtl865x_addVlanPortMember(uint16 vid, uint32 portMask)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return RTL_EINVALIDVLANID;
#else
	vlanEntry = &vlanTbl[vid];
#endif
	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	/*add member port*/
	vlanEntry->memberPortMask |= portMask;
	vlanEntry->untagPortMask |= portMask;

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
	
	
}

static int32 _rtl865x_addVlanPortMember2(uint16 vid, uint32 portMask, uint32 untagPortMask)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry = NULL;

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
	vlanEntry = rtl_getVlanEntryByVid(vid);

	if(vlanEntry == NULL)
		return RTL_EENTRYNOTFOUND;
#else
	vlanEntry = &vlanTbl[vid];
#endif

	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	/*add member port*/
	vlanEntry->memberPortMask = portMask;
	vlanEntry->untagPortMask = untagPortMask;

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;		
}

int32 rtl865x_addVlanPortMember2(uint16 vid, uint32 portMask, uint32 untagPortMask)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;

	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_addVlanPortMember2(vid,portMask,untagPortMask);	
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}

static int32 _rtl865x_delVlanPortMember(uint16 vid, uint32 portMask)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
	//rtl865x_tblAsicDrv_vlanParam_t asicEntry;
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return RTL_EINVALIDVLANID;
#else
	vlanEntry = &vlanTbl[vid];
#endif

	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	/*add member port*/
	vlanEntry->memberPortMask &= ~portMask;
	vlanEntry->untagPortMask &=~portMask;

	if(vlanEntry->memberPortMask == 0)
		vlanEntry->valid = 0;

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
	
	
}


static int32 _rtl865x_setVlanPortTag(uint16 vid, uint32 portMask, uint8 tag)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return RTL_EINVALIDVLANID;
#else
	vlanEntry = &vlanTbl[vid];
#endif

	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	if(tag == 0)
		vlanEntry->untagPortMask |= vlanEntry->memberPortMask & portMask;
	else
		vlanEntry->untagPortMask &=~(vlanEntry->memberPortMask & portMask);

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
	
}

static int32 _rtl865x_setVlanFID(uint16 vid, uint32 fid)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;

	if(fid >= RTL865X_FDB_NUMBER)
		return RTL_EINVALIDFID;
	
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return RTL_EINVALIDVLANID;
#else
	vlanEntry = &vlanTbl[vid];
#endif

	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	vlanEntry->fid = fid;		

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
}


static int32 _rtl865x_getVlanFilterDatabaseId(uint16 vid, uint32 *fid)
{
	int32 retval = 0;
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
	rtl865x_vlan_entry_t *vlanEntry;
#endif

	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)

	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return RTL_EINVALIDVLANID;
	
	if(vlanEntry->valid == 1)
#else

	if(vlanTbl[vid].valid == 1)		
#endif
	{
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
		*fid = vlanEntry->fid;
#else
		*fid = vlanTbl[vid].fid;
#endif
		retval = SUCCESS;
	}
	else
	{
		/*jwj: Because I may change eth1 netif vid to 1 when CONFIG_RTL_VLAN_BASED_NETIF
		enabled in rtl_set_wanport_vlanconfig()*/
		#if !defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
		#ifndef CONFIG_RTL_IVL_SUPPORT
		extern int rtl865x_curOpMode;
		if (rtl865x_curOpMode == 0) // GATEWAY_MODE
			printk("%s(%d):the vlan[%d] is invalid!\n",__FUNCTION__,__LINE__, vid);
		#endif
		#endif
		retval = FAILED;
	}

	return retval;
	
}
rtl865x_vlan_entry_t *_rtl8651_getVlanTableEntry(uint16 vid)
{
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
	rtl865x_vlan_entry_t *vlanEntry;

	if(vid < 1 || vid > VLAN_NUMBER -1)
		return NULL;

	vlanEntry = rtl_getVlanEntryByVid(vid);
	return vlanEntry;
#else
	if(vlanTbl[vid].valid == 1)
		return &vlanTbl[vid];
	return NULL;
#endif
}

/*
@func int32 | rtl865x_referVlan | reference a VLAN entry.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
if a vlan entry is referenced, please call this API.
*/
int32 rtl865x_referVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_referVlan(vid);
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return retval;
}

/*
@func int32 | rtl865x_deReferVlan | dereference a VLAN entry.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
if a vlan entry is dereferenced, please call this API.
NOTE: rtl865x_deReferVlan should be called after rtl865x_referVlan.
*/
int32 rtl865x_deReferVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_deReferVlan(vid);
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return retval;
}

/*
@func int32 | rtl865x_addVlan | Add a VLAN.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@rvalue RTL_EVLANALREADYEXISTS | Vlan already exists.
*/
int32 rtl865x_addVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER-1)
		return RTL_EINVALIDVLANID;

	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_addVlan(vid);
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
		
}

/*
@func int32 | rtl865x_delVlan | Delete a VLAN.
@parm uint16 | vid | VLAN ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@rvalue RTL_EREFERENCEDBYOTHER | the Vlan is referenced by other,please delete releated table entry first.
*/
int32 rtl865x_delVlan(uint16 vid)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delVlan(vid);	
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}

/*
@func int32 | rtl865x_addVlanPortMember | configure vlan member port
@parm uint16 | vid | VLAN ID.
@parm uint32 | portMask | Port mask.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
the parm portMask is the MASK of port. bit0 mapping to physical port 0,bit1 mapping to physical port 1.
*/
int32 rtl865x_addVlanPortMember(uint16 vid, uint32 portMask)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;

	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_addVlanPortMember(vid,portMask);	
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}

/*
@func int32 | rtl865x_delVlanPortMember | delete vlan's member port
@parm uint16 | vid | VLAN ID.
@parm uint32 | portMask | Port mask.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
the parm portMask is the MASK of port. bit0 mapping to physical port 0,bit1 mapping to physical port 1.
*/
int32 rtl865x_delVlanPortMember(uint16 vid,uint32 portMask)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delVlanPortMember(vid,portMask);	
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);

	return retval;
	
}


/*
@func uint32 | rtl865x_getVlanPortMask | get the member portMask of a vlan
@parm uint16 | vid | VLAN ID.
@comm
if the retrun value is zero, it means vlan entry is invalid or no member port in this vlan.
*/
uint32 rtl865x_getVlanPortMask(uint32 vid)
{
	rtl865x_vlan_entry_t *vlanEntry;
	
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return 0;
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	

	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return 0;
#else
	
	vlanEntry = &vlanTbl[vid];
#endif
	if(vlanEntry->valid == 0)
		return 0;	
	
	return vlanEntry->memberPortMask;
}


/*
@func int32 | rtl865x_setVlanPortTag | configure member port vlan tag attribute
@parm uint16 | vid | VLAN ID.
@parm uint32 | portMask | Port mask.
@parm uint8 | portMask | vlantag or untag.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
the parm portMask is the MASK of port. bit0 mapping to physical port 0,bit1 mapping to physical port 1.
parm tag is used to indicated physical port is vlantag or untag. value 1 means vlan tagged, and vlan 0 means vlan untagged.
*/
int32 rtl865x_setVlanPortTag(uint16 vid,uint32 portMask,uint8 tag)
{
	int32 retval = FAILED;
	unsigned long flags=0;	
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_setVlanPortTag(vid,portMask,tag);	
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}

/*
@func int32 | rtl865x_setVlanFilterDatabase | configure the filter database for a vlan.
@parm uint16 | vid | VLAN ID.
@parm uint32 | fid | filter data base ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@rvalue RTL_EINVALIDFID | Invalid filter database ID.
@comm
in realtek 865x, 4 filter databases are support.
if you want to configure SVL for all vlan, please set the fid of vlan is same.
default configure is SVL.
*/
int32 rtl865x_setVlanFilterDatabase(uint16 vid, uint32 fid)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;
	
	//rtl_down_interruptible(&vlan_sem);//Lock resource
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_setVlanFID(vid,fid);	
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);

	return retval;	
}

/*
@func int32 | rtl865x_getVlanFilterDatabaseId | get the vlan's filter database ID.
@parm uint16 | vid | VLAN ID.
@parm uint32 | fid | filter data base ID.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDVLANID | Invalid VLAN ID.
@comm
*/
int32 rtl865x_getVlanFilterDatabaseId(uint16 vid, uint32 *fid)
{
	int32 retval = FAILED;
	retval = _rtl865x_getVlanFilterDatabaseId(vid, fid);

	return retval;
}

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
static int32 _rtl865x_modVlanPortMember(uint16 vid, uint32 portMask, uint32 untagset)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
	vlanEntry = rtl_getVlanEntryByVid(vid);
	if(vlanEntry == NULL)
		return RTL_EINVALIDVLANID;
#else
	vlanEntry = &vlanTbl[vid];
#endif
	if(vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	/*add member port*/
	vlanEntry->memberPortMask = portMask;
	vlanEntry->untagPortMask = untagset;

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
}

int32 rtl865x_modVlanPortMember(uint16 vid, uint32 portMask, uint32 untagset)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	/* vid should be legal vlan ID */
	if(vid < 1 || vid > VLAN_NUMBER -1)
		return RTL_EINVALIDVLANID;

	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_modVlanPortMember(vid, portMask, untagset);	
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}
#endif /* CONFIG_RTL_ISP_MULTI_WAN_SUPPORT */

/*
@func int32 | rtl865x_initVlanTable | initialize vlan table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int32 rtl865x_initVlanTable(void)
{	
	TBL_MEM_ALLOC(vlanTbl, rtl865x_vlan_entry_t, VLANTBL_SIZE);	
	memset(vlanTbl,0,sizeof(rtl865x_vlan_entry_t)*VLANTBL_SIZE);

	return SUCCESS;	
}

/*
@func int32 | rtl865x_reinitVlantable | initialize vlan table.
@rvalue SUCCESS | Success.
*/
int32 rtl865x_reinitVlantable(void)
{
	uint16 i;
	for(i = 0; i < VLAN_NUMBER; i++)
	{
		if(vlanTbl[i].valid)
		{
			#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)	
			_rtl865x_delVlan(vlanTbl[i].vid);
			#else
			_rtl865x_delVlan(i);
			#endif
		}
	}
	return SUCCESS;
}

#if defined(CONFIG_RTL_8197F)
static int32 _rtl865x_setVlanHp(uint16 vid, uint8 hp)
{
	int32 retval = FAILED;
	rtl865x_vlan_entry_t *vlanEntry;
	
	vlanEntry = &vlanTbl[vid];
	if (vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	/*add vid priority*/
	vlanEntry->hp = hp;

	/*update this entry to asic table*/
	retval = _rtl865x_setAsicVlan(vid,vlanEntry);

	return retval;
}

int32 rtl865x_setVlanHp(uint16 vid, uint8 hp)
{
	int32 retval = FAILED;
	unsigned long flags = 0;
	/* vid should be legal vlan ID */
	if ((vid < 1) ||(vid > VLAN_NUMBER -1) ||(hp < 0) ||(hp > 7))
		return RTL_EINVALIDVLANID;

	//rtl_down_interruptible(&vlan_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_setVlanHp(vid, hp);
	//rtl_up(&vlan_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}

int32 rtl865x_getVlanHp(uint16 vid, uint8 *hp)
{
	rtl865x_vlan_entry_t *vlanEntry;
	
	/* vid should be legal vlan ID */
	if ((vid < 1) ||(vid > VLAN_NUMBER -1) ||(hp == NULL))
		return RTL_EINVALIDVLANID;

	vlanEntry = &vlanTbl[vid];
	if (vlanEntry->valid == 0)
		return RTL_EINVALIDVLANID;

	*hp = vlanEntry->hp;

	return SUCCESS;
}
#endif /* CONFIG_RTL_8197F */

