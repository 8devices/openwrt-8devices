/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __linux__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/kconfig.h>
#else
#include <linux/config.h>
#endif
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#endif



#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include "common/rtl_utils.h"
//#include "common/assert.h"

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL3.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h" 
#include "common/rtl865x_tblDrvPatch.h"
#endif

#include "AsicDriver/asicRegs.h"
#include "AsicDriver/asicTabs.h"

#include "common/rtl8651_tblDrvProto.h"

#include "common/rtl865x_eventMgr.h"
#include "common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>

#include "l3Driver/rtl865x_ip.h"

#ifdef RTL865X_TEST
#include <string.h>
#endif

#include <net/rtl/rtl865x_multicast.h>
#include <net/rtl/rtl865x_igmpsnooping.h>

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
int32 rtl8198C_initMulticastv6(void);
int32 rtl8198C_reinitMulticastv6(void);
#endif

/********************************************************/
/*			Multicast Related Global Variable			*/
/********************************************************/

static rtl865x_mcast_fwd_descriptor_t *rtl865x_mcastFwdDescPool=NULL;
static mcast_fwd_descriptor_head_t  free_mcast_fwd_descriptor_head;
static struct rtl865x_multicastTable mCastTbl;
static uint32 rtl865x_externalMulticastPortMask = 0;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static rtl8198c_mcast_fwd_descriptor6_t *rtl8198c_mcastFwdDescPool6=NULL;
static mcast_fwd_descriptor_head6_t free_mcast_fwd_descriptor_head6;
static struct rtl8198c_multicastv6Table mCastTbl6;
static uint32 rtl8198c_externalMulticastPortMask6 = 0;
#endif
#if defined(__linux__) && defined(__KERNEL__)
static struct timer_list rtl865x_mCastSysTimer;	/*igmp timer*/
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static struct timer_list rtl8198c_mCast6SysTimer;/*MLD timer*/
#endif
#endif

#if defined (CONFIG_RTL_HW_MCAST_WIFI)
extern int hwwifiEnable;
#endif


#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
extern int rtl_get_brIgmpModuleIndexbyName(char *name, int * index);
extern struct net_bridge * rtl_get_brByIndex(int index, char *name);
#endif

static int32 _rtl865x_initMCastFwdDescPool(void)
{
	int32 i;


	MC_LIST_INIT(&free_mcast_fwd_descriptor_head);

	TBL_MEM_ALLOC(rtl865x_mcastFwdDescPool, rtl865x_mcast_fwd_descriptor_t,MAX_MCAST_FWD_DESCRIPTOR_CNT);
	
	if(rtl865x_mcastFwdDescPool!=NULL)
	{
	
		memset( rtl865x_mcastFwdDescPool, 0, MAX_MCAST_FWD_DESCRIPTOR_CNT * sizeof(rtl865x_mcast_fwd_descriptor_t));	
	}
	else
	{
		return FAILED;
	}
	

	for(i = 0; i<MAX_MCAST_FWD_DESCRIPTOR_CNT;i++)
	{
		MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head, &rtl865x_mcastFwdDescPool[i], next);
	}
	
	return SUCCESS;
}

static rtl865x_mcast_fwd_descriptor_t *_rtl865x_allocMCastFwdDesc(void)
{
	rtl865x_mcast_fwd_descriptor_t *retDesc=NULL;
	retDesc = MC_LIST_FIRST(&free_mcast_fwd_descriptor_head);
	if(retDesc!=NULL)
	{
		MC_LIST_REMOVE(retDesc, next);
		memset(retDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	}
	return retDesc;
}

static int32 _rtl865x_freeMCastFwdDesc(rtl865x_mcast_fwd_descriptor_t *descPtr)
{
	if(descPtr==NULL)
	{
		return SUCCESS;
	}
	memset(descPtr,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head, descPtr, next);
	
	return SUCCESS;
}

static int32 _rtl865x_flushMCastFwdDescChain(mcast_fwd_descriptor_head_t * descChainHead)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc,*nextDesc;
	
	if(descChainHead==NULL)
	{
		return SUCCESS;
	}
	
	curDesc=MC_LIST_FIRST(descChainHead);
	while(curDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		/*remove from the old descriptor chain*/
		MC_LIST_REMOVE(curDesc, next);
		/*return to the free descriptor chain*/
		_rtl865x_freeMCastFwdDesc(curDesc);
		curDesc = nextDesc;
	}

	return SUCCESS;
}



static int32 _rtl865x_mCastFwdDescEnqueue(mcast_fwd_descriptor_head_t * queueHead,
												rtl865x_mcast_fwd_descriptor_t * enqueueDesc)
{

	rtl865x_mcast_fwd_descriptor_t *newDesc;
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(enqueueDesc==NULL)
	{
		return SUCCESS;
	}
	
	/*multicast forward descriptor is internal maintained,always alloc new one*/
	newDesc=_rtl865x_allocMCastFwdDesc();
	
	if(newDesc!=NULL)
	{
		memcpy(newDesc, enqueueDesc,sizeof(rtl865x_mcast_fwd_descriptor_t ));
		//memset(&(newDesc->next), 0, sizeof(MC_LIST_ENTRY(rtl865x_mcast_fwd_descriptor_s)));
		newDesc->next.le_next=NULL;
		newDesc->next.le_prev=NULL;
	}
	else
	{
		/*no enough memory*/
		return FAILED;
	}
	

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{

		nextDesc=MC_LIST_NEXT(curDesc, next);
		
		/*merge two descriptor*/
	//	if((strcmp(curDesc->netifName,newDesc->netifName)==0) && (curDesc->vid==newDesc->vid))
		if(strcmp(curDesc->netifName,newDesc->netifName)==0)
		{	
			
			if(newDesc->descPortMask==0)
			{
				newDesc->descPortMask=curDesc->descPortMask;
			}
			MC_LIST_REMOVE(curDesc, next);
			_rtl865x_freeMCastFwdDesc(curDesc);
			
		}
	}

	/*not matched descriptor is found*/
	MC_LIST_INSERT_HEAD(queueHead, newDesc, next);

	return SUCCESS;
	
}


static int32 _rtl865x_mergeMCastFwdDescChain(mcast_fwd_descriptor_head_t * targetChainHead ,
													rtl865x_mcast_fwd_descriptor_t *srcChain)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc;

	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		
		_rtl865x_mCastFwdDescEnqueue(targetChainHead, curDesc);
		
	}
	
	return SUCCESS;
}




static int32 _rtl865x_initMCastEntryPool(void)
{
	int32 index;
	rtl865x_tblDrv_mCast_t *multiCast_t;
	struct MCast_hash_head *mCast_hash_head;
	
	TBL_MEM_ALLOC(multiCast_t, rtl865x_tblDrv_mCast_t ,MAX_MCAST_TABLE_ENTRY_CNT);
	TAILQ_INIT(&mCastTbl.freeList.freeMultiCast);
	for(index=0; index<MAX_MCAST_TABLE_ENTRY_CNT; index++)
	{
		memset( &multiCast_t[index], 0, sizeof(rtl865x_tblDrv_mCast_t));
		TAILQ_INSERT_HEAD(&mCastTbl.freeList.freeMultiCast, &multiCast_t[index], nextMCast);
	}

	//TBL_MEM_ALLOC(multiCast_t, rtl865x_tblDrv_mCast_t, RTL8651_MULTICASTTBL_SIZE);
	TBL_MEM_ALLOC(mCast_hash_head, struct MCast_hash_head, RTL8651_MULTICASTTBL_SIZE);
	memset(mCast_hash_head, 0,RTL8651_MULTICASTTBL_SIZE* sizeof(struct MCast_hash_head));
	mCastTbl.inuseList.mCastTbl = (void *)mCast_hash_head;

	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++)
	{
		TAILQ_INIT(&mCastTbl.inuseList.mCastTbl[index]);
	}

	return SUCCESS;
}

static rtl865x_tblDrv_mCast_t * _rtl865x_allocMCastEntry(uint32 hashIndex)
{
	rtl865x_tblDrv_mCast_t *newEntry;
	newEntry=TAILQ_FIRST(&mCastTbl.freeList.freeMultiCast);
	if (newEntry == NULL)
	{
		return NULL;
	}		
	
	TAILQ_REMOVE(&mCastTbl.freeList.freeMultiCast, newEntry, nextMCast);

	
	/*initialize it*/
	if(MC_LIST_FIRST(&newEntry->fwdDescChain)!=NULL)
	{
		_rtl865x_flushMCastFwdDescChain(&newEntry->fwdDescChain);
	}
	MC_LIST_INIT(&newEntry->fwdDescChain);
	
	memset(newEntry, 0, sizeof(rtl865x_tblDrv_mCast_t));

	TAILQ_INSERT_TAIL(&mCastTbl.inuseList.mCastTbl[hashIndex], newEntry, nextMCast);
	
	return newEntry;
}

static int32 _rtl865x_flushMCastEntry(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	_rtl865x_flushMCastFwdDescChain(&mCastEntry->fwdDescChain);
	
	memset(mCastEntry, 0, sizeof(rtl865x_tblDrv_mCast_t));
	return SUCCESS;
}

static int32 _rtl865x_freeMCastEntry(rtl865x_tblDrv_mCast_t * mCastEntry, uint32 hashIndex)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[hashIndex], mCastEntry, nextMCast);
	_rtl865x_flushMCastEntry(mCastEntry);
	TAILQ_INSERT_HEAD(&mCastTbl.freeList.freeMultiCast, mCastEntry, nextMCast);
	return SUCCESS;
}


static uint32 _rtl865x_doMCastEntrySrcVlanPortFilter(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc,*nextDesc;
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	for(curDesc=MC_LIST_FIRST(&mCastEntry->fwdDescChain);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next);
		{
			curDesc->fwdPortMask=curDesc->fwdPortMask & (~(1<<mCastEntry->port));
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl865x_freeMCastFwdDesc(curDesc);

			}
		}
		
	}

	return SUCCESS;
}
#if defined(CONFIG_RTL_MULTI_LAN_DEV) && defined(CONFIG_RTL_HW_MCAST_WIFI)
static uint32 rtl865x_genMCastEntryAsicFwdMaskIgnoreCpuFlag(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint32 asicFwdPortMask=0;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		asicFwdPortMask |= curDesc->fwdPortMask;
	}
	asicFwdPortMask = asicFwdPortMask & (~(1<<mCastEntry->port)); 
	return asicFwdPortMask;
}
#endif

 
static uint32 rtl865x_genMCastEntryAsicFwdMask(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint32 asicFwdPortMask=0;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(curDesc->toCpu==0)
		{
			asicFwdPortMask|=(curDesc->fwdPortMask & ((1<<RTL8651_MAC_NUMBER)-1));
		}
		else
		{
			asicFwdPortMask|=( 0x01<<RTL8651_MAC_NUMBER);
		}
	}
	asicFwdPortMask = asicFwdPortMask & (~(1<<mCastEntry->port)); 
	return asicFwdPortMask;
}

static uint16 rtl865x_genMCastEntryCpuFlag(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint16 cpuFlag=FALSE;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}

	if(mCastEntry->cpuHold==TRUE)
	{
		cpuFlag=TRUE;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(	(curDesc->toCpu==TRUE)	||
			(memcmp(curDesc->netifName, RTL_WLAN_NAME,4)==0)	)
		{
			cpuFlag=TRUE;
		}
	}
	
	return cpuFlag;
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)
/*for linux bridge level igmp snooping usage*/
#if !defined(CONFIG_RTL_MULTI_LAN_DEV)
static uint32 rtl865x_getMCastEntryDescPortMask(rtl865x_tblDrv_mCast_t *mCastEntry)
{
	uint32 descPortMask=0;
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		descPortMask=descPortMask | curDesc->descPortMask;
	}
	
	return descPortMask;
}
#endif
#endif
/*=======================================
  * Multicast Table APIs
  *=======================================*/
#define RTL865X_MULTICASE_TABLE_APIs

static void  _rtl865x_setASICMulticastPortStatus(void) {
	uint32 index;

	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++) {
		rtl8651_setAsicMulticastPortInternal(index, (rtl865x_externalMulticastPortMask&(1<<index))?FALSE:TRUE);
	}
}

void rtl865x_arrangeMulticastPortStatus(void) {

	rtl865x_externalMulticastPortMask=rtl865x_getExternalPortMask();
	_rtl865x_setASICMulticastPortStatus();
}

/*
@func int32	| rtl865x_addMulticastExternalPort	| API to add a hardware multicast external port.
@parm  uint32 | extPort	| External port number to be added. 
@rvalue SUCCESS	|Add hardware multicast external port successfully.
@rvalue FAILED	|Add hardware multicast external port failed.
*/
int32 rtl865x_addMulticastExternalPort(uint32 extPort)
{
	rtl865x_externalMulticastPortMask |= (1<<extPort);
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_delMulticastExternalPort	| API to delete a hardware multicast external port.
@parm  uint32 | extPort	| External port number to be deleted.
@rvalue SUCCESS	|Delete external port successfully.
@rvalue FAILED	|Delete external port failed.
*/
int32 rtl865x_delMulticastExternalPort(uint32 extPort)
{
	rtl865x_externalMulticastPortMask &= ~(1<<extPort);
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_setMulticastExternalPortMask	| API to set hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be set.
@rvalue SUCCESS	|Set external port mask successfully.
@rvalue FAILED	|Set external port mask failed.
*/
int32 rtl865x_setMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask =extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_addMulticastExternalPortMask	| API to add hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be added.
@rvalue SUCCESS	|Add external port mask successfully.
@rvalue FAILED	|Add external port mask failed.
*/
int32 rtl865x_addMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask|= extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

/*
@func int32	| rtl865x_delMulticastExternalPortMask	|  API to delete hardware multicast external port mask.
@parm  uint32 | extPortMask	| External port mask to be deleted.
@rvalue SUCCESS	|Delete external port mask successfully.
@rvalue FAILED	|Delete external port mask failed.
*/
int32 rtl865x_delMulticastExternalPortMask(uint32 extPortMask)
{
	rtl865x_externalMulticastPortMask &= ~extPortMask;
	_rtl865x_setASICMulticastPortStatus();
	return SUCCESS;
}

int32 rtl865x_getMulticastExternalPortMask(void)
{
	return rtl865x_externalMulticastPortMask ;
}

static inline void _rtl865x_patchPppoeWeak(rtl865x_tblDrv_mCast_t *mCast_t)
{
	rtl865x_mcast_fwd_descriptor_t * curDesc;
	uint32 netifType;
	/* patch: keep cache in software if one vlan's interface is pppoe */
	MC_LIST_FOREACH(curDesc, &(mCast_t->fwdDescChain), next)
	{
		if(rtl865x_getNetifType(curDesc->netifName, &netifType)==SUCCESS)
		{
			/*how about pptp,l2tp?*/
			if(netifType==IF_PPPOE)
			{
				mCast_t->flag |= RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
				return;
			}
		}
		
	}

	mCast_t->flag &= ~RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
}
#if 0
static int _rtl865x_checkMulticastEntryEqual(rtl865x_tblDrv_mCast_t * mCastEntry1, rtl865x_tblDrv_mCast_t * mCastEntry2)
{
	if((mCastEntry1==NULL) && (mCastEntry2==NULL))
	{
		return TRUE;
	}
	
	if((mCastEntry1==NULL) && (mCastEntry2!=NULL))
	{
		return FALSE;
	}

	if((mCastEntry1!=NULL) && (mCastEntry2==NULL))
	{
		return FALSE;
	}
	
	if(mCastEntry1->sip!=mCastEntry2->sip)
	{
		return FALSE;
	}

	if(mCastEntry1->dip!=mCastEntry2->dip)
	{
		return FALSE;
	}

	if(mCastEntry1->svid!=mCastEntry2->svid)
	{
		return FALSE;
	}
	
	if(mCastEntry1->port!=mCastEntry2->port)
	{
		return FALSE;
	}

	if(mCastEntry1->mbr!=mCastEntry2->mbr)
	{
		return FALSE;
	}
	
	if(mCastEntry1->cpu!=mCastEntry2->cpu)
	{
		return FALSE;
	}
	
	if(mCastEntry1->extIp!=mCastEntry2->extIp)
	{
		return FALSE;
	}

	if(mCastEntry1->flag!=mCastEntry2->flag)
	{
		return FALSE;
	}

	
	if(mCastEntry1->inAsic!=mCastEntry2->inAsic)
	{
		return FALSE;
	}			

	return TRUE;
}
#endif
#ifdef CONFIG_PROC_FS
static unsigned int mcastAddOpCnt=0;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static unsigned int mcastAddOpCnt6=0;
#endif
unsigned int _rtl865x_getAddMcastOpCnt(void)
{
	return mcastAddOpCnt;
}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
unsigned int _rtl8198C_getAddMcastv6OpCnt(void)
{
	return mcastAddOpCnt6;
}
#endif

static unsigned int mcastDelOpCnt=0;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static unsigned int mcastDelOpCnt6=0;
#endif
unsigned int _rtl865x_getDelMcastOpCnt(void)
{
	return mcastDelOpCnt;
}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
unsigned int _rtl8198C_getDelMcastv6OpCnt(void)
{
	return mcastDelOpCnt6;
}

#endif
#endif
/* re-select Multicast entry to ASIC for the index ""entryIndex */
static void _rtl865x_arrangeMulticast(uint32 entryIndex)
{
	rtl865x_tblAsicDrv_multiCastParam_t asic_mcast;
	rtl865x_tblDrv_mCast_t *mCast_t=NULL;
	rtl865x_tblDrv_mCast_t *select_t=NULL;
	rtl865x_tblDrv_mCast_t *swapOutEntry=NULL;
	int32 retval;
	
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entryIndex], nextMCast) 
	{
	
		#ifdef CONFIG_RTL_HW_MCAST_WIFI
		if(((hwwifiEnable)||((hwwifiEnable==0)&&(mCast_t->cpu == 0)))
			&& !(mCast_t->flag & RTL865X_MULTICAST_PPPOEPATCH_CPUBIT))
		#else
		if ((mCast_t->cpu == 0) && !(mCast_t->flag & RTL865X_MULTICAST_PPPOEPATCH_CPUBIT)) 
		#endif
		{ /* Ignore cpu=1 */

			if(mCast_t->inAsic==TRUE)
			{
				if(swapOutEntry==NULL)
				{
					swapOutEntry=mCast_t;
				}
				else
				{
					/*impossible, two flow in one asic entry*/
					swapOutEntry->inAsic=FALSE;
					mCast_t->inAsic = FALSE;
				}
			}
			
			if (select_t) 
			{


				if ((mCast_t->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
				{
					/*replace unknown multicast*/
					select_t = mCast_t;
				}
				else
				{
					/*select the heavy load*/
					if ((mCast_t->count) > (select_t->count))
					{
						select_t = mCast_t;
					}
				}
				
			}
			else 
			{
				select_t = mCast_t;
			}
			
			
		}
		else
		{
			mCast_t->inAsic = FALSE;	/* reset "inAsic" bit */
		} 


	}
	
	if(select_t && swapOutEntry)
	{
		if ((swapOutEntry->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
		{
			/*replace unknown multicast*/
			select_t = swapOutEntry;
		}
		else
		{
			if((select_t->count <= (swapOutEntry->count+RTL865X_HW_MCAST_SWAP_GAP)))
				select_t = swapOutEntry;
		}
	}	
	

	
	/*
	if(swapOutEntry)
	{
		printk("%s:%d,swapOutEntry->count:%d,swapOutEntry->dip is 0x%x,swapOutEntry->mbr is 0x%x\n",__FUNCTION__,__LINE__,swapOutEntry->count,swapOutEntry->dip,swapOutEntry->mbr);

	}
	
	if (select_t) 
	{
		printk("%s:%d,select_t->count:%d,select_t->dip is 0x%x,select_t->mbr is 0x%x\n",__FUNCTION__,__LINE__,select_t->count,select_t->dip,select_t->mbr);
	}
	*/
	if (select_t) 
	{
		if((swapOutEntry==NULL) ||(select_t==swapOutEntry))
		{
			select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
			bzero(&asic_mcast, sizeof(rtl865x_tblAsicDrv_multiCastParam_t));
			memcpy(&asic_mcast, select_t, (uint32)&(((rtl865x_tblDrv_mCast_t *)0)->extIp));
			if (select_t->extIp)
			{
			
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
				int32 ipIdx;
				if(rtl865x_getIpIdxByExtIp(select_t->extIp, &ipIdx)==SUCCESS)
				{
					asic_mcast.extIdx=(uint16)ipIdx;
				}
#else
				asic_mcast.extIdx=0;
#endif
		
			}
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
			asic_mcast.idx=(uint16)entryIndex;
	#endif
			retval = rtl8651_setAsicIpMulticastTable(&asic_mcast);
			
#ifdef CONFIG_PROC_FS
			mcastAddOpCnt++;
#endif
			assert(retval == SUCCESS);
			if(retval==SUCCESS)
			{
				select_t->inAsic = TRUE;
			}
			else
			{
				select_t->inAsic = FALSE;
				rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
				mcastDelOpCnt++;
#endif
			}
				
			assert(retval == SUCCESS);
			TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
			TAILQ_INSERT_HEAD(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
		}
		else/*(swapOutEntry!=NULL) && (select_t!=swapOutEntry)*/
		{
			/*disable swap and only explicit joined mulicast flow can replace unknown multicast flow*/
			if(1)
			{
				/*don't forget to set swapOutEntry's inAsic flag*/
				swapOutEntry->inAsic=FALSE;
				
				select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
				bzero(&asic_mcast, sizeof(rtl865x_tblAsicDrv_multiCastParam_t));
				memcpy(&asic_mcast, select_t, (uint32)&(((rtl865x_tblDrv_mCast_t *)0)->extIp));

				if (select_t->extIp)
				{
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3		
					int32 ipIdx;
					if(rtl865x_getIpIdxByExtIp(select_t->extIp, &ipIdx)==SUCCESS)
					{
						asic_mcast.extIdx=(uint16)ipIdx;
					}
#else
					asic_mcast.extIdx=0;
#endif
				}

				retval = rtl8651_setAsicIpMulticastTable(&asic_mcast);
#ifdef CONFIG_PROC_FS
				mcastAddOpCnt++;
#endif
				assert(retval == SUCCESS);
				if(retval==SUCCESS)
				{
					select_t->inAsic = TRUE;
				}
				else
				{
					select_t->inAsic = FALSE;
					rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt++;
#endif
				}
				
				TAILQ_REMOVE(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
				TAILQ_INSERT_HEAD(&mCastTbl.inuseList.mCastTbl[entryIndex], select_t, nextMCast);

			}
			#if 0
			else
			{			
				if(swapOutEntry->inAsic == FALSE)
				{
					/*maybe something is wrong, we remove the asic entry*/
					rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt++;
#endif
				}
				
			}	
			#endif
			
		}
		
	}
	else 	
	{
		if(swapOutEntry!=NULL)
		{
			swapOutEntry->inAsic=FALSE;
		}
		rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
		mcastDelOpCnt++;
#endif
	}
}


static void _rtl865x_mCastEntryReclaim(void)
{
	uint32 index;
	uint32 freeCnt=0;
	uint32 asicFwdPortMask=0;
	uint32 needReArrange=FALSE;
	rtl865x_tblDrv_mCast_t *curMCastEntry, *nextMCastEntry;

	/*free unused software forward entry*/
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]);
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if((curMCastEntry->inAsic==FALSE)  && (curMCastEntry->count==0))
			{
				_rtl865x_freeMCastEntry(curMCastEntry, index);
				freeCnt++;
			}
			curMCastEntry = nextMCastEntry;
		}
		
	}

	if(freeCnt>0)
	{
		return;
	}
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]);
		needReArrange=FALSE;
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if(curMCastEntry->inAsic)
			{
				asicFwdPortMask=rtl865x_genMCastEntryAsicFwdMask(curMCastEntry);
				if(asicFwdPortMask==0) 
				{
					_rtl865x_freeMCastEntry(curMCastEntry, index);
					needReArrange=TRUE;
				}
			}
			curMCastEntry = nextMCastEntry;
		}
		
		if(needReArrange==TRUE)
		{
			_rtl865x_arrangeMulticast(index);
		}
	}

	return;
}
/*
@func rtl865x_tblDrv_mCast_t *	| rtl865x_findMCastEntry	|  API to find a hardware multicast entry.
@parm  ipaddr_t 	| mAddr	| Multicast stream destination group address. 
@parm  ipaddr_t	|  sip	| Multicast stream source ip address.
@parm  uint16		| svid	| Multicast stream input vlan index.
@parm  uint16 	| sport	| Multicast stream input port number.
*/
rtl865x_tblDrv_mCast_t *rtl865x_findMCastEntry(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport)
{
	rtl865x_tblDrv_mCast_t *mCast_t;
	uint32 entry = rtl8651_ipMulticastTableIndex(sip, mAddr);
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast) {
		if (mCast_t->dip==mAddr && mCast_t->sip==sip && mCast_t->svid==svid && mCast_t->port==sport)
		{
			if (mCast_t->inAsic == FALSE) 
			{
				mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
				mCast_t->count ++;
			}

			return mCast_t;
		}
	}
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	for(entry=RTL8651_IPMULTICASTTBL_SIZE; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast) {
			if (mCast_t->dip==mAddr && mCast_t->sip==sip && mCast_t->svid==svid && mCast_t->port==sport)
			{
				if (mCast_t->inAsic == FALSE) 
				{
					mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					mCast_t->count ++;
				}

				return mCast_t;
			}
					
		}
	}
#endif	
	return (rtl865x_tblDrv_mCast_t *)NULL;	
}


#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)

int rtl865x_findEmptyCamEntry(void)
{
	int index=-1;

	for(index=RTL8651_IPMULTICASTTBL_SIZE; index<RTL8651_MULTICASTTBL_SIZE; index++)
	{

		if(TAILQ_EMPTY(&mCastTbl.inuseList.mCastTbl[index]))
		{
			return index;
		}
	}
	
	return -1;
}
#endif

/*
@func int32	| rtl865x_addMulticastEntry	|  API to add a hardwawre multicast forwarding entry.
@parm  ipaddr_t 	| mAddr	| Multicast flow Destination group address. 
@parm  ipaddr_t 	| sip	| Multicast flow source ip address. 
@parm  uint16 	| svid	| Multicast flow input vlan index. 
@parm  uint16		| sport	| Multicast flow input port number. 
@parm  rtl865x_mcast_fwd_descriptor_t *	| newFwdDescChain	| Multicast flow forwarding descriptor chain to be added. 
@parm  int32 	| flushOldChain	| Flag to indicate to flush old mulicast forwarding descriptor chain or not. 1 is to flush old chain, and 0 is not to. 
@parm  ipaddr_t 	| extIp	| External source ip address used when forward multicast data from lan to wan. 
@parm  int8	| toCpu	| Cpu forwarding flag, 1 is to forward multicast data by cpu,and  0 is not.
@parm  int8	| flag	| For future usage, set to 0 at present.
@rvalue SUCCESS	|Add hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Add hardware multicast forwarding entry failed.
*/
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int32 rtl865x_addMulticastEntry(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport, 
									rtl865x_mcast_fwd_descriptor_t * newFwdDescChain, 
									int32 flushOldChain, ipaddr_t extIp, char cpuHold, uint8 flag, unsigned int mapPortMask)
#else
int32 rtl865x_addMulticastEntry(ipaddr_t mAddr, ipaddr_t sip, uint16 svid, uint16 sport, 
									rtl865x_mcast_fwd_descriptor_t * newFwdDescChain, 
									int32 flushOldChain, ipaddr_t extIp, char cpuHold, uint8 flag)
#endif
{

	rtl865x_tblDrv_mCast_t *mCast_t;
	uint32 hashIndex = rtl8651_ipMulticastTableIndex(sip, mAddr);
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	uint32 emptyCamIndex=-1; 
	#endif
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	struct rtl_groupInfo groupInfo;
	#endif
	/*windows xp upnp:239.255.255.0*/
	if(mAddr==0xEFFFFFFA)
	{
		return FAILED;
	}
#if 0
	/*reserved multicast address 224.0.0.x*/
	if((mAddr & 0xFFFFFF00) == 0xE0000000)
	{
		return FAILED;
	}
#endif	

#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t=rtl865x_findMCastEntry(mAddr, sip, svid, sport);
	if(mCast_t==NULL)
	{
		/*table entry collided*/
		if(!TAILQ_EMPTY(&mCastTbl.inuseList.mCastTbl[hashIndex]))
		{
			emptyCamIndex=rtl865x_findEmptyCamEntry();
			if(emptyCamIndex!=-1)
			{
				hashIndex=emptyCamIndex;
			}
		}
	}
	else
	{
		hashIndex=mCast_t->hashIndex;
	}
#else
	/*try to match hash line*/
	TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[hashIndex], nextMCast) 
	{
		if (mCast_t->sip==sip && mCast_t->dip==mAddr && mCast_t->svid==svid && mCast_t->port==sport)
			break;
	}
#endif	
	
	if (mCast_t == NULL) 
	{
		mCast_t=_rtl865x_allocMCastEntry(hashIndex);
		if (mCast_t == NULL)
		{
			_rtl865x_mCastEntryReclaim();
			mCast_t=_rtl865x_allocMCastEntry(hashIndex);
			if(mCast_t == NULL)
			{
				return FAILED;
			}
		}
		mCast_t->sip			= sip;
		mCast_t->dip			= mAddr;
		mCast_t->svid		= svid;
		mCast_t->port		= sport;
		mCast_t->mbr		= 0;
		mCast_t->count		= 0;
		//mCast_t->maxPPS		= 0;
		
		#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
		mCast_t->mapPortMbr = mapPortMask;
		#endif
		
		mCast_t->inAsic		= FALSE;
	}
	
	if(flushOldChain)
	{
		_rtl865x_flushMCastFwdDescChain(&mCast_t->fwdDescChain);
		
	}
	
	_rtl865x_mergeMCastFwdDescChain(&mCast_t->fwdDescChain,newFwdDescChain);
	_rtl865x_doMCastEntrySrcVlanPortFilter(mCast_t);
	
	mCast_t->mbr			= rtl865x_genMCastEntryAsicFwdMask(mCast_t);
	mCast_t->extIp			= extIp;

	mCast_t->age			= RTL865X_MULTICAST_TABLE_AGE;
#if 0
	mCast_t->cpu			= (toCpu==TRUE? 1: 0);
#else
	mCast_t->cpuHold			= cpuHold;
	mCast_t->cpu 			= rtl865x_genMCastEntryCpuFlag(mCast_t);
#endif	
	mCast_t->flag			= flag;
	
	if (extIp)
		mCast_t->flag |= RTL865X_MULTICAST_EXTIP_SET;
	else
		mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
	#if defined (CONFIG_RTL_IGMP_SNOOPING)
	rtl_getGroupInfo(mAddr, &groupInfo);
	if(groupInfo.ownerMask==0)
	{
		mCast_t->unKnownMCast=TRUE;
	}
	else
	{
		mCast_t->unKnownMCast=FALSE;
	}
	#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t->hashIndex=hashIndex;
#endif
	_rtl865x_patchPppoeWeak(mCast_t);
	_rtl865x_arrangeMulticast(hashIndex);
	return SUCCESS;	
}


/*
@func int32	| rtl865x_delMulticastEntry	|  API to delete multicast forwarding entry related with a certain group address.
@parm  ipaddr_t 	| mcast_addr	| Group address to be mached in deleting hardware multicast forwarding entry. 
@rvalue SUCCESS	|Delete hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Delete hardware multicast forwarding entry failed.
*/
int32 rtl865x_delMulticastEntry(ipaddr_t mcast_addr)
{

	rtl865x_tblDrv_mCast_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	uint32 deleteFlag=FALSE;

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
		deleteFlag=FALSE;
		mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if (!mcast_addr || mCastEntry->dip == mcast_addr) 
			{
				deleteFlag=TRUE;
				_rtl865x_freeMCastEntry(mCastEntry, entry);
			}
			
			mCastEntry = nextMCastEntry;
		}
		
		if(deleteFlag==TRUE)
		{
			_rtl865x_arrangeMulticast(entry);
		}
	}

	return SUCCESS;
}

#if 0
/*the following function maybe used in future*/

int32 rtl865x_addMulticastFwdDesc(ipaddr_t mcast_addr, rtl865x_mcast_fwd_descriptor_t * newFwdDesc)
{

	rtl865x_tblDrv_mCast_t *mCast_t;
	uint32 entry, matchedIdx = 0;
	uint32 oldFwdPortMask,newFwdPortMask;
	if(newFwdDesc==NULL)
	{
		return SUCCESS;
	}

	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++)
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[entry], nextMCast)
		{
			if (mCast_t->dip != mcast_addr)
				continue;

			oldFwdPortMask=mCast_t->mbr;

			_rtl865x_mergeMCastFwdDescChain(&mCast_t->fwdDescChain,newFwdDesc);
			_rtl865x_doMCastEntrySrcVlanPortFilter(mCast_t);
			
			mCast_t->mbr 		= rtl865x_genMCastEntryFwdMask(mCast_t);
			newFwdPortMask		= mCast_t->mbr ;
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
			if (mCast_t->flag & RTL865X_MULTICAST_UPLOADONLY)
			{	/* remove upload term*/
				if(oldFwdPortMask!=newFwdPortMask)
				{
					mCast_t->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
					/* we assume multicast member will NEVER in External interface, so we remove
					     external ip now */
					mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
					mCast_t->extIp= 0;
				}
			}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */

			_rtl865x_patchPppoeWeak(mCast_t);
			_rtl865x_arrangeMulticast(entry);
			matchedIdx = entry;
		}
	}

	if (matchedIdx) 
	{
		return SUCCESS;
	}
	return FAILED;
}

int32 rtl865x_delMulticastFwdDesc(ipaddr_t mcast_addr,  rtl865x_mcast_fwd_descriptor_t * deadFwdDesc)
{

	uint32 index;
	rtl865x_tblDrv_mCast_t  *mCastEntry, *nextMCastEntry;
	uint32 oldFwdPortMask,newFwdPortMask;
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{

		for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
		{
			nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
			
			if ((mcast_addr) && (mCastEntry->dip != mcast_addr))
			{
				continue;
			}
			
			oldFwdPortMask=mCastEntry->mbr;
		
			_rtl865x_subMCastFwdDescChain(&mCastEntry->fwdDescChain, deadFwdDesc);
			
			mCastEntry->mbr=rtl865x_genMCastEntryFwdMask(mCastEntry);
			newFwdPortMask=mCastEntry->mbr; 	
			if (mCastEntry->mbr == 0)
			{
				/*to-do:unknown multicast hardware blocking*/
				_rtl865x_freeMCastEntry(mCastEntry, index);
				mCastEntry=NULL;
				_rtl865x_arrangeMulticast(index);
			}
			else
			{
			
				_rtl865x_patchPppoeWeak(mCastEntry);
			}
			
		}
			
		_rtl865x_arrangeMulticast(index);
	}

	return SUCCESS;
}

int32 rtl865x_delMulticastUpStream(ipaddr_t mcast_addr, ipaddr_t sip, uint16 svid, uint16 sport)
{
	uint32 index;
	rtl865x_tblDrv_mCast_t *mCast_t;
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[index], nextMCast) 
		{
			if ((!mcast_addr || mCast_t->dip == mcast_addr) && 
				(!sip || mCast_t->sip==sip) && 
				(!svid || mCast_t->svid==svid) && 
				mCast_t->port==sport)
			{
				_rtl865x_freeMCastEntry(mCast_t, index);
				_rtl865x_arrangeMulticast(index);
				return SUCCESS;
			}
		}
	}
	return FAILED;
}

int32 rtl865x_delMulticastByVid(uint32 vid)
{
	uint16 sport;
	uint32 sportMask;
	rtl865x_mcast_fwd_descriptor_t vlanFwdDesc;
	memset(&vlanFwdDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	
	/* delete all upstream related to vid */
	sport = 0;
	sportMask=rtl865x_getVlanPortMask(vid);
	while (sportMask) 
	{
		if (sportMask & 1)
		{
			rtl865x_delMulticastUpStream(0, 0, vid, sport);
		}
		
		sportMask = sportMask >> 1;
		sport ++;
	}
	
	/* delete all downstream related to vid*/
	vlanFwdDesc.vid=vid;
	vlanFwdDesc.fwdPortMask=rtl865x_getVlanPortMask(vid);
	rtl865x_delMulticastFwdDesc(0, &vlanFwdDesc);

	return FAILED;
}

int32 rtl865x_delMulticastByPort(uint32 port)
{

	rtl865x_mcast_fwd_descriptor_t portFwdDesc;
	memset(&portFwdDesc,0,sizeof(rtl865x_mcast_fwd_descriptor_t));
	
	/* delete all upstream related to this port */
	rtl865x_delMulticastUpStream(0, 0, 0, port);

	/* delete all downstream related to this port*/
	portFwdDesc.vid=0;
	portFwdDesc.fwdPortMask=1<<port;
	rtl865x_delMulticastFwdDesc(0, &portFwdDesc);

	return SUCCESS;
}

int32 rtl865x_setMGroupAttribute(ipaddr_t groupIp, int8 toCpu)
{
	uint32 index;
	rtl865x_tblDrv_mCast_t *mCast_t;

	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl.inuseList.mCastTbl[index], nextMCast) 
		{
			if (mCast_t->dip == groupIp)
			{
				mCast_t->cpu = (toCpu==TRUE? 1: 0);
			}
		}
		_rtl865x_arrangeMulticast(index);
	}
	return SUCCESS;
}


static int32 _rtl865x_subMCastFwdDescChain(mcast_fwd_descriptor_head_t * targetChainHead,rtl865x_mcast_fwd_descriptor_t *srcChain)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc;
	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		_rtl865x_mCastFwdDescDequeue(targetChainHead, curDesc);
	}

	return SUCCESS;
}

static int32 _rtl865x_mCastFwdDescDequeue(mcast_fwd_descriptor_head_t * queueHead,rtl865x_mcast_fwd_descriptor_t * dequeueDesc)
{
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
	
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(dequeueDesc==NULL)
	{
		return FAILED;
	}

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		if((strcmp(curDesc->netifName,dequeueDesc->netifName)==0) ||
			((dequeueDesc->vid==0 ) ||(curDesc->vid==dequeueDesc->vid)))
		{
			curDesc->fwdPortMask &= (~dequeueDesc->fwdPortMask);
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old descriptor chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl865x_freeMCastFwdDesc(curDesc);

			}

			return SUCCESS;
		}
	}

	/*never reach here*/
	return SUCCESS;
}

#endif

#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)
int32 rtl865x_multicastUpdate(rtl865x_mcast_fwd_descriptor_t* desc)
{
	uint32 index;
	//uint32 oldDescPortMask,newDescPortMask;/*for device decriptor forwarding usage*/
	uint32 oldAsicFwdPortMask,newAsicFwdPortMask;/*for physical port forwarding usage*/
	uint32 oldCpuFlag,newCpuFlag;
	
	rtl865x_mcast_fwd_descriptor_t newFwdDesc;
	rtl865x_tblDrv_mCast_t	*mCastEntry,*nextMCastEntry;
	//printk("[%s:%d]\n",__FUNCTION__,__LINE__);
	if(desc==NULL)
	{
		//printk("[%s:%d]failed\n",__FUNCTION__,__LINE__);
		return FAILED;
	}
	/*check device name's validity*/
	if(strlen(desc->netifName)==0)
	{
		//printk("[%s:%d]failed\n",__FUNCTION__,__LINE__);
		return FAILED;
	}
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(memcmp(desc->netifName,RTL_BR_NAME,3)==0 || memcmp(desc->devName,RTL_BR1_NAME,3)==0)
#else
	if(memcmp(desc->netifName,RTL_BR_NAME,3)==0)
#endif
	{
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				//printk("[%s:%d]hw dip is:0x%x,cur dip is 0x%x\n",__FUNCTION__,__LINE__,mCastEntry->dip,desc->dip);	
				if (mCastEntry->dip != desc->dip)
				{
					continue;
				}
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,desc->netifName);
	
				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;
	
				/*sync with control plane*/
				newFwdDesc.fwdPortMask=desc->fwdPortMask & (~(1<<mCastEntry->port));
				newFwdDesc.toCpu=desc->toCpu;
				
				/*update/replace old forward descriptor*/
					
				_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
				mCastEntry->mbr 	= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
				mCastEntry->cpu 	= rtl865x_genMCastEntryCpuFlag(mCastEntry);
					
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
				/*printk("[%s:%d]old fwPortMask is %d,newfwPortMask is %d\n",
					   __FUNCTION__,
					   __LINE__,
					   oldAsicFwdPortMask,
					   newAsicFwdPortMask);*/	
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
						/* we assume multicast member will NEVER in External interface, so we remove
								 external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
						mCastEntry->extIp= 0;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */
	
				mCastEntry->unKnownMCast=FALSE;
					
				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl865x_patchPppoeWeak(mCastEntry);
						
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{
						_rtl865x_freeMCastEntry(mCastEntry, index);
					}
						
					_rtl865x_arrangeMulticast(index);
				}
			}
				
		}
	}
	return SUCCESS;
}
#endif
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
extern struct net_bridge* bridge0;
extern unsigned int rtl865x_getPhyFwdPortMask(struct net_bridge *br,unsigned int brFwdPortMask);
#endif
#if defined(CONFIG_RT_MULTIPLE_BR_SUPPORT)
extern int rtl_get_brIgmpModuleIndexbyId(int idx,char *name);
#endif

static int32 rtl865x_multicastCallbackFn(void *param)
{
#if defined (CONFIG_RTL_IGMP_SNOOPING)
	uint32 index;
#if !defined(CONFIG_RTL_MULTI_LAN_DEV)
	uint32 oldDescPortMask=0,newDescPortMask=0;/*for device decriptor forwarding usage*/
#endif	
	uint32 oldAsicFwdPortMask=0,newAsicFwdPortMask=0;/*for physical port forwarding usage*/
	uint32 oldCpuFlag=0,newCpuFlag=0;
	
	rtl_multicastEventContext_t mcastEventContext;

	rtl865x_mcast_fwd_descriptor_t newFwdDesc;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	rtl865x_tblDrv_mCast_t  *mCastEntry,*nextMCastEntry;
#if !defined(CONFIG_RTL_MULTI_LAN_DEV) && !defined(CONFIG_RT_MULTIPLE_BR_SUPPORT)
	struct rtl_multicastDeviceInfo_s bridgeMCastDev;
#endif

#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
unsigned char br_name[16]={0};
int br_index;

#if defined (CONFIG_RTL_MULTI_LAN_DEV)
	struct net_bridge* br_dev;
#else
	unsigned int br_moduleIndex = 0xFFFFFFFF;
#if defined (CONFIG_RTL_HW_MCAST_WIFI)
	unsigned int currentCpuFlag = 0;
#endif
#endif
#endif


#if 0//defined(CONFIG_RTL_ISP_MULTIPLE_BR_SUPPORT) && defined(CONFIG_RTL_MULTI_LAN_DEV)
//need to check, some are used only for CONFIG_RTL_MULTI_LAN_DEV
	struct net_bridge* br_dev;
	int  br_indextmp;
	int br_moduleIndex = 0xFFFFFFFF;
	unsigned char br_name[16]={0};
	unsigned int fwdPortMask_tmp = 0xFFFFFFFF;
	unsigned int cpuTrueNum = 0, bridgeNum = 0;
	unsigned int joinBridgeNum = 0;
#endif
	struct rtl_groupInfo groupInfo;
	int32 retVal=FAILED;

	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));
	/*check device name's validity*/
	if(strlen(mcastEventContext.devName)==0)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
	printk("%s:%d,mcastEventContext.devName is %s,moduleindex:%x, mcastEventContext.groupAddr is 0x%x,mcastEventContext.sourceAdd is 0x%x,mcastEventContext.portMask is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName, mcastEventContext.moduleIndex,mcastEventContext.groupAddr[0], mcastEventContext.sourceAddr[0], mcastEventContext.portMask);
	#endif

#if defined (CONFIG_RTL_MULTI_LAN_DEV)
#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,2)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0)
#endif
	{
		#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
		printk("%s:%d,multicast event from %s,mcastEventContext.groupAddr[0] is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName,mcastEventContext.groupAddr[0]);
		#endif
		
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				
				if ((mcastEventContext.groupAddr!=0) && (mCastEntry->dip != mcastEventContext.groupAddr[0]))
				{
					continue;
				}
				
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);

				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;

				/*sync with control plane*/
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  mCastEntry->sip;
				multicastDataInfo.groupAddr[0]= mCastEntry->dip;

				#if 0
				//#if defined (CONFIG_RTL_ISP_MULTIPLE_BR_SUPPORT)
				for(br_index = 0; br_index<RTL_IMGP_MAX_BRMODULE; br_index++)
				{
					br_dev = rtl_get_brByIndex(br_index, br_name);
					if(br_dev)
					{
						bridgeNum ++;
						br_moduleIndex = rtl_get_brIgmpModuleIndexbyName(br_name,&br_indextmp);//br_indextmep should be equal to br_index
						retVal= rtl_getMulticastDataFwdInfo(br_moduleIndex, &multicastDataInfo, &multicastFwdInfo);
						if (multicastFwdInfo.unknownMCast == TRUE)
							cpuTrueNum ++;
						fwdPortMask_tmp = rtl865x_getPhyFwdPortMask(br_dev,multicastFwdInfo.fwdPortMask);
						newFwdDesc.fwdPortMask |= fwdPortMask_tmp;
						#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
						printk("dev:%s, multicastFwdInfo.fwdPortMask:%x, fwdPortMask_tmp:%x, in_port:%x, newFwdPortMask:%x, [%s:%d]\n",
							br_name, multicastFwdInfo.fwdPortMask, fwdPortMask_tmp, mCastEntry->port, newFwdDesc.fwdPortMask, __FUNCTION__, __LINE__);
						#endif
					}
					newFwdDesc.fwdPortMask &= (~(1<<mCastEntry->port));
				}
				if(bridgeNum == cpuTrueNum)
					multicastFwdInfo.cpuFlag = TRUE;
					
				#else
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);

				/*fix jwj*/
				if (multicastFwdInfo.unknownMCast == TRUE)
					multicastFwdInfo.cpuFlag = TRUE;

				if(retVal !=0)
				{
					multicastFwdInfo.fwdPortMask=0;
					multicastFwdInfo.cpuFlag = TRUE;
				}
				#endif

				#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT
				br_dev = NULL;
				for(br_index = 0; br_index<RTL_IMGP_MAX_BRMODULE; br_index++)
				{
					br_dev = rtl_get_brByIndex(br_index, br_name);
					if(strcmp(br_name, mcastEventContext.devName)==0)
						break;
				}
				if(br_dev==NULL)
					continue;

				#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
					newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(br_dev,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
					if(retVal == SUCCESS)
					{
						if(multicastFwdInfo.cpuFlag == TRUE)
						{
							//wifi entry exist
							newFwdDesc.fwdPortMask |= (0x01<<6);
						}					
					}
				}
				else
				{
					newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(br_dev,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
				}
				#else
				newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(br_dev,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
				#endif
				#else
				
				#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{

					newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(bridge0,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
					if(retVal == SUCCESS)
					{
						if(multicastFwdInfo.cpuFlag == TRUE)
						{
							//wifi entry exist
							newFwdDesc.fwdPortMask |= (0x01<<6);
						}					
					}
						
				}
				else
				{
					newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(bridge0,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
				}
				#else
				newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(bridge0,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
				#endif
				#endif
				
				newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
		#if defined	CONFIG_RTL865X_MUTLICAST_DEBUG
				panic_printk("newFwdDesc.tocpu %d, newFwdDesc.fwdPortMask:%x,[%s:%d]\n", multicastFwdInfo.cpuFlag, newFwdDesc.fwdPortMask, __FUNCTION__, __LINE__);
		#endif	
				/*update/replace old forward descriptor*/
				_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);

				/*tocpu flag=1, DescFwdMask will be ignored, to cpu flag*/
			#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
					mCastEntry->mbr  	= rtl865x_genMCastEntryAsicFwdMaskIgnoreCpuFlag(mCastEntry);
				}
				else
				{
					mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
				}
			#else
				mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
			#endif
				mCastEntry->cpu		= rtl865x_genMCastEntryCpuFlag(mCastEntry);
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
				
				#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
				printk("%s:%d,oldAsicFwdPortMask is %d,newAsicFwdPortMask is %d\n",__FUNCTION__,__LINE__,oldAsicFwdPortMask,newAsicFwdPortMask);
				#endif
				
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
						/* we assume multicast member will NEVER in External interface, so we remove
						     external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
						mCastEntry->extIp= 0;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */

				rtl_getGroupInfo(mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
#if defined CONFIG_RTL865X_MUTLICAST_DEBUG				
				printk("old FwdPortMask:%x, newAsicFwdPortMask:%x, [%s:%d]\n", oldAsicFwdPortMask, newAsicFwdPortMask, __FUNCTION__, __LINE__);
#endif

#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
				newAsicFwdPortMask &= (mCastEntry->mapPortMbr |(0x01<<6));
#endif

#if defined(CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
					if(((newAsicFwdPortMask)&(~(1<<6)))==0)
						newAsicFwdPortMask =0;
				}
#endif

#if defined CONFIG_RTL865X_MUTLICAST_DEBUG
				printk("old FwdPortMask:%x, newAsicFwdPortMask:%x, [%s:%d]\n", oldAsicFwdPortMask, newAsicFwdPortMask, __FUNCTION__, __LINE__);
#endif
				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl865x_patchPppoeWeak(mCastEntry);
					
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{
						_rtl865x_freeMCastEntry(mCastEntry, index);
					}

					_rtl865x_arrangeMulticast(index);
				}
			}

				
			
		}
	}

#else  //CONFIG_RTL_MULTI_LAN_DEV
    /*case 1:this is multicast event from bridge */
	/*sync wlan and ethernet*/
	//hyking:[Fix me] the RTL_BR_NAME...
#if defined (CONFIG_OPENWRT_SDK)
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
    if(memcmp(mcastEventContext.devName,RTL_BR_NAME,6)==0 || memcmp(mcastEventContext.devName,RTL_BR1_NAME,3)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,6)==0)
#endif
#else
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
    if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0 || memcmp(mcastEventContext.devName,RTL_BR1_NAME,3)==0)
#elif defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,2)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0)
#endif
#endif
	{

		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				
				if ((mcastEventContext.groupAddr!=0) && (mCastEntry->dip != mcastEventContext.groupAddr[0]))
				{
					continue;
				}

				rtl_getGroupInfo(mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}

				oldDescPortMask=rtl865x_getMCastEntryDescPortMask( mCastEntry);	

		#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
					oldAsicFwdPortMask=mCastEntry->mbr;
					oldCpuFlag = mCastEntry->cpu;
				}
		#endif
		
		#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT
				newDescPortMask = 0;
				newCpuFlag = 0;
		#if defined CONFIG_RTL_HW_MCAST_WIFI
				currentCpuFlag = 0;
		#endif
				for(br_index = 0; br_index<RTL_IMGP_MAX_BRMODULE; br_index++)
				{	
					br_moduleIndex=rtl_get_brIgmpModuleIndexbyId(br_index,br_name);
					if(br_moduleIndex==0xFFFFFFFF)
						continue;

					/*sync with control plane*/
					memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
					strcpy(newFwdDesc.netifName,br_name);
				
					multicastDataInfo.ipVersion=4;
					multicastDataInfo.sourceIp[0]=	mCastEntry->sip;
					multicastDataInfo.groupAddr[0]= mCastEntry->dip;
				
				
					retVal= rtl_getMulticastDataFwdInfo(br_moduleIndex, &multicastDataInfo, &multicastFwdInfo);
					
				#if defined (CONFIG_RTL_HW_MCAST_WIFI)
					if(hwwifiEnable)
					{
						if(retVal != SUCCESS)
						{
							if(multicastFwdInfo.unknownMCast == 1)
								multicastFwdInfo.cpuFlag = 0;
							else
								multicastFwdInfo.cpuFlag = 1;
						}

						currentCpuFlag = multicastFwdInfo.cpuFlag;

						if(currentCpuFlag)
							newAsicFwdPortMask = (1<<RTL8651_MAC_NUMBER); //cpu port
						else
							newAsicFwdPortMask = 0;

						newFwdDesc.fwdPortMask = newAsicFwdPortMask;
						newFwdDesc.toCpu = currentCpuFlag;
					#if defined CONFIG_RTL865X_MUTLICAST_DEBUG
						printk("newFwdDesc.fwdPortMask = %x, newFwdDesc.toCpu = %d, [%s:%d]\n", newFwdDesc.fwdPortMask, newFwdDesc.toCpu, __FUNCTION__, __LINE__);
					#endif
						_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
					}
				#endif

					if(retVal == SUCCESS)
					{
						newCpuFlag |= multicastFwdInfo.cpuFlag;
					}
					
				}
				//printk("br:%s :oldAsicPortMask:%x,newAsicPortMask:%x,oldCpuFlag:%d,newCpuFlag:%d,[%s]:[%d].\n",devName,oldAsicPortMask,newAsicPortMask,oldCpuFlag,newCpuFlag,__FUNCTION__,__LINE__);
			
				#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
				
					mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
					mCastEntry->cpu 	= rtl865x_genMCastEntryCpuFlag(mCastEntry);
					newAsicFwdPortMask =mCastEntry->mbr;
					newCpuFlag=mCastEntry->cpu;
					if(mCastEntry->unKnownMCast==TRUE)
					{
						newAsicFwdPortMask = 0;
						newCpuFlag = 0;
					}
						
					if(((newAsicFwdPortMask) & (~(1<<6)))==0)
					{
						//only cpu port
						newAsicFwdPortMask = 0;
					}
							
					if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						if(newAsicFwdPortMask == 0){
							//printk("[%s]:[%d].\n",__FUNCTION__,__LINE__);
							_rtl865x_freeMCastEntry(mCastEntry, index);
						}	
						
						_rtl865x_arrangeMulticast(index);
					}
				}
				else
				{
					if(mCastEntry->unKnownMCast==TRUE)
						newCpuFlag = 1;	

					if(oldCpuFlag != newCpuFlag)
					{
						_rtl865x_freeMCastEntry(mCastEntry, index);
						_rtl865x_arrangeMulticast(index);
					}
				}
				#else
				if(mCastEntry->unKnownMCast==TRUE)
					newCpuFlag = 1;	
				if(oldCpuFlag != newCpuFlag)
				{
					_rtl865x_freeMCastEntry(mCastEntry, index);
					_rtl865x_arrangeMulticast(index);
				}

				#endif
		#else  //CONFIG_RT_MULTIPLE_BR_SUPPORT
				
				/*sync with control plane*/
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  mCastEntry->sip;
				multicastDataInfo.groupAddr[0]= mCastEntry->dip;
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				
			#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
					if(retVal==SUCCESS)
					{
						newCpuFlag = multicastFwdInfo.cpuFlag;
						
						if(newCpuFlag)
							newAsicFwdPortMask = (1<<RTL8651_MAC_NUMBER);
						else
							newAsicFwdPortMask = 0;
						
					}	
					else
					{
						newAsicFwdPortMask=0;
						newCpuFlag =1;
					}
					//this fwdDesc is used to set cpu port, here to update br fwdDesc
					newFwdDesc.fwdPortMask = newAsicFwdPortMask;
					newFwdDesc.toCpu = newCpuFlag;
					
					//printk("br:%s :oldAsicPortMask:%x,newAsicPortMask:%x,oldCpuFlag:%d,newCpuFlag:%d,[%s]:[%d].\n",mcastEventContext.devName,oldAsicPortMask,newAsicPortMask,oldCpuFlag,newCpuFlag,__FUNCTION__,__LINE__);
					_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
					mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
					mCastEntry->cpu		= rtl865x_genMCastEntryCpuFlag(mCastEntry);
					
					newAsicFwdPortMask =mCastEntry->mbr;
					newCpuFlag =mCastEntry->cpu;
					//printk("br:%s :oldAsicPortMask:%x,newAsicPortMask:%x,oldCpuFlag:%d,newCpuFlag:%d,[%s]:[%d].\n",mcastEventContext.devName,oldAsicPortMask,newAsicPortMask,oldCpuFlag,newCpuFlag,__FUNCTION__,__LINE__);
					//just cpu port not add hw entry
					if((newAsicFwdPortMask&(~(1<<6)))==0){
						newAsicFwdPortMask =0;
					}
					if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						if(newAsicFwdPortMask == 0)
							_rtl865x_freeMCastEntry(mCastEntry, index);
						_rtl865x_arrangeMulticast(index);
					}
				}
				else
				{
					if(retVal!=SUCCESS)
					{
						continue;
					}
					
					retVal= rtl_getIgmpSnoopingModuleDevInfo(mcastEventContext.moduleIndex, &bridgeMCastDev);
					if(retVal!=SUCCESS)
					{
						continue;
					}
					newDescPortMask=multicastFwdInfo.fwdPortMask;
					//shirley note:oldDescPortMask is always 0, the judge condision is equal to (newDescPortMask & bridgeMCastDev.swPortMask)!=0
					if(	(oldDescPortMask != newDescPortMask) &&
						(	((newDescPortMask & bridgeMCastDev.swPortMask)!=0) ||
							(((oldDescPortMask & bridgeMCastDev.swPortMask) !=0) && ((newDescPortMask & bridgeMCastDev.swPortMask)==0)))	)
					{
						/*this multicast entry should be re-generated at linux protocol stack bridge level*/
						_rtl865x_freeMCastEntry(mCastEntry, index);
						_rtl865x_arrangeMulticast(index);
					}
				}
			#else

				if(retVal!=SUCCESS)
				{
					continue;
				}
				
				retVal= rtl_getIgmpSnoopingModuleDevInfo(mcastEventContext.moduleIndex, &bridgeMCastDev);
				if(retVal!=SUCCESS)
				{
					continue;
				}
				newDescPortMask=multicastFwdInfo.fwdPortMask;
				//shirley note:oldDescPortMask is always 0, the judge condision is equal to (newDescPortMask & bridgeMCastDev.swPortMask)!=0
				if(	(oldDescPortMask != newDescPortMask) &&
					(	((newDescPortMask & bridgeMCastDev.swPortMask)!=0) ||
						(((oldDescPortMask & bridgeMCastDev.swPortMask) !=0) && ((newDescPortMask & bridgeMCastDev.swPortMask)==0)))	)
				{
					/*this multicast entry should be re-generated at linux protocol stack bridge level*/
					_rtl865x_freeMCastEntry(mCastEntry, index);
					_rtl865x_arrangeMulticast(index);
				}
			#endif	
		#endif
			}
		}
		
		return EVENT_CONTINUE_EXECUTE;
	}		

/*update ethernet forwarding port mask*/

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
    if(memcmp(mcastEventContext.devName,"eth*",4)==0 || memcmp(mcastEventContext.devName,RTL_PS_ETH_NAME_ETH2,4)==0)
#else
	if(memcmp(mcastEventContext.devName,"eth*",4)==0)
#endif
	{
		#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
		printk("%s:%d,multicast event from %s,mcastEventContext.groupAddr[0] is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName,mcastEventContext.groupAddr[0]);
		#endif
		
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				
				if ((mcastEventContext.groupAddr!=0) && (mCastEntry->dip != mcastEventContext.groupAddr[0]))
				{
					continue;
				}
				
				memset(&newFwdDesc, 0 ,sizeof(rtl865x_mcast_fwd_descriptor_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);

				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;

				/*sync with control plane*/
				multicastDataInfo.ipVersion=4;
				multicastDataInfo.sourceIp[0]=  mCastEntry->sip;
				multicastDataInfo.groupAddr[0]= mCastEntry->dip;

				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				
#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
					if(retVal != SUCCESS)
					{
						newFwdDesc.fwdPortMask=0;
						newFwdDesc.toCpu =1;
					}
					else
					{
						newFwdDesc.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<mCastEntry->port));
						newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
					}
				}
				else
#endif
				{
					/*fix jwj*/
					if (multicastFwdInfo.unknownMCast == TRUE)
						multicastFwdInfo.cpuFlag = TRUE;
					newFwdDesc.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<mCastEntry->port));
					newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
				}
				#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
					newFwdDesc.fwdPortMask &= mCastEntry->mapPortMbr;
				#endif

			
				/*update/replace old forward descriptor*/
				
				_rtl865x_mergeMCastFwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
				mCastEntry->mbr 		= rtl865x_genMCastEntryAsicFwdMask(mCastEntry);
				mCastEntry->cpu		= rtl865x_genMCastEntryCpuFlag(mCastEntry);
				
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
				
#if defined (CONFIG_RTL_HW_MCAST_WIFI)
				if(hwwifiEnable)
				{
					if((newAsicFwdPortMask&(~(1<<6)))==0)
						newAsicFwdPortMask =0;
				}
#endif
				#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
				printk("%s:%d,oldAsicFwdPortMask is %d,newAsicFwdPortMask is %d\n",__FUNCTION__,__LINE__,oldAsicFwdPortMask,newAsicFwdPortMask);
				#endif
				
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
						/* we assume multicast member will NEVER in External interface, so we remove
						     external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
						mCastEntry->extIp= 0;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */

				rtl_getGroupInfo(mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
	
				
				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl865x_patchPppoeWeak(mCastEntry);
					
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{
						_rtl865x_freeMCastEntry(mCastEntry, index);
					}

					_rtl865x_arrangeMulticast(index);
				}
			}

				
			
		}
	}
#endif
#endif
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_multicastUnRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_multicastCallbackFn;
	rtl865x_unRegisterEvent(&eventParam);

	return SUCCESS;
}

static int32 _rtl865x_multicastRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_multicastCallbackFn;
	rtl865x_registerEvent(&eventParam);

	return SUCCESS;
}

#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
extern void ConvertMulticatIPtoMacAddr(__u32 group, unsigned char *gmac);
extern int rtl_delFilterDatabaseEntryforMCast(unsigned char *mac);
extern int rtl_addFilterDatabaseEntryforMCast(unsigned char *mac, unsigned int fwdPortMask);
extern int nicIgmpModuleIndex;

static int32 rtl865x_L2multicastCallbackFn(void *param)
{
	rtl_multicastEventContext_t mcastEventContext;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	int32 retVal=FAILED, ret;
	unsigned char mac[6]={0};

	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));

	if(strlen(mcastEventContext.devName)==0)
	{
		return EVENT_CONTINUE_EXECUTE;
	}

	if(mcastEventContext.moduleIndex != nicIgmpModuleIndex)
	{
		//only deal with eth event.
		return EVENT_CONTINUE_EXECUTE;
	}

	if(mcastEventContext.ipVersion != 4)
	{
		//only deal with eth event.
		return EVENT_CONTINUE_EXECUTE;
	}

	multicastDataInfo.ipVersion=4;
	//l2 multicast only support igmpv1 and igmpv2, sourceip is not meaningful
	multicastDataInfo.sourceIp[0]=	mcastEventContext.sourceAddr[0];
	multicastDataInfo.groupAddr[0]= mcastEventContext.groupAddr[0];
	ConvertMulticatIPtoMacAddr(mcastEventContext.groupAddr[0], mac);
	retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);

	if(retVal!=SUCCESS)
	{
		//need to delete l2 table entry
		ret=rtl_delFilterDatabaseEntryforMCast(mac);
		//printk("delFilter return:%d\n", ret);
	}
	else
	{
		//update l2 table entry
		ret=rtl_addFilterDatabaseEntryforMCast(mac, multicastFwdInfo.fwdPortMask);
		//printk("addFilter return:%d\n", ret);
	}
	return 0;
}
static int32 _rtl865x_L2multicastUnRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_L2_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_L2multicastCallbackFn;
	rtl865x_unRegisterEvent(&eventParam);

	return SUCCESS;
}

static int32 _rtl865x_L2multicastRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;

	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_L2_MCAST;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_L2multicastCallbackFn;
	rtl865x_registerEvent(&eventParam);

	return SUCCESS;
}

#endif
void _rtl865x_timeUpdateMulticast(uint32 secPassed)
{

	rtl865x_tblDrv_mCast_t *mCast_t, *nextMCast_t;
	uint32 entry;
	uint32 needReArrange=FALSE;
	uint32 hashLineCnt=0;
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		hashLineCnt=0;
		needReArrange=FALSE;
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == TRUE)
			{
				/* Entry is in the ASIC */
				if (mCast_t->age <= secPassed) 
				{
					if(mCast_t->mbr==0)
					{
						_rtl865x_freeMCastEntry(mCast_t, entry);
						needReArrange=TRUE;
					}
					else
					{
						mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					}
				}
				else
				{
					mCast_t->age -= secPassed;
				}
			}
			else 
			{
				/* Entry is not in the ASIC */
				if (mCast_t->age <= secPassed)
				{ /* aging out */
					_rtl865x_freeMCastEntry(mCast_t, entry);
				}
				else 
				{
					mCast_t->age -= secPassed;
				}
			}
			
			/*won't count multicast entry forwarded by cpu*/
			if(mCast_t->cpu==0)
			{
				
				hashLineCnt++;
				//printk("------------hashLineCnt:%d,[%s]:[%d].\n",hashLineCnt,__FUNCTION__,__LINE__);
				if(hashLineCnt>=2)
				{
					needReArrange=TRUE;
				}
			}
		
			//mCast_t->count = 0;
			mCast_t = nextMCast_t;
		}

		if(needReArrange==TRUE)
		{
			//printk("------------entry:%d,hashLineCnt:%d,[%s]:[%d].\n",entry,hashLineCnt,__FUNCTION__,__LINE__);
			_rtl865x_arrangeMulticast(entry);
		}
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == FALSE)
				mCast_t->count=0;
			
			mCast_t = nextMCast_t;
		}
		
	}
}

#if defined(__linux__) && defined(__KERNEL__)
static void _rtl865x_mCastSysTimerExpired(uint32 expireDada)
{

	_rtl865x_timeUpdateMulticast(1);
	mod_timer(&rtl865x_mCastSysTimer, jiffies+HZ);
	
}

static void _rtl865x_initMCastSysTimer(void)
{

	init_timer(&rtl865x_mCastSysTimer);
	rtl865x_mCastSysTimer.data=rtl865x_mCastSysTimer.expires;
	rtl865x_mCastSysTimer.expires=jiffies+HZ;
	rtl865x_mCastSysTimer.function=(void*)_rtl865x_mCastSysTimerExpired;
	add_timer(&rtl865x_mCastSysTimer);
}

static void _rtl865x_destroyMCastSysTimer(void)
{
	del_timer(&rtl865x_mCastSysTimer);
}

#endif

/*
@func int32	| rtl865x_initMulticast	|  Init hardware ip multicast module.
@parm  rtl865x_mCastConfig_t *	| mCastConfigPtr	| Pointer of hardware multicast configuration. 
@rvalue SUCCESS	|Initialize successfully.
@rvalue FAILED	|Initialize failed.
*/
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static void _rtl8198C_initMCastv6SysTimer(void);
#endif
int32 rtl865x_initMulticast(rtl865x_mCastConfig_t * mCastConfigPtr)
{
	_rtl865x_multicastUnRegisterEvent();
#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
	_rtl865x_L2multicastUnRegisterEvent();
#endif
	_rtl865x_initMCastEntryPool();
	_rtl865x_initMCastFwdDescPool();
	rtl865x_setMulticastExternalPortMask(0);
	if(mCastConfigPtr!=NULL)
	{
		rtl865x_setMulticastExternalPortMask(mCastConfigPtr->externalPortMask);
	}
	#if defined(__linux__) && defined(__KERNEL__)
	_rtl865x_initMCastSysTimer();
	#endif
#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
	rtl8651_setAsicOperationLayer(2);
#else
	rtl8651_setAsicOperationLayer(3);
#endif
	rtl8651_setAsicMulticastMTU(1522); 
#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
	rtl8651_setAsicMulticastEnable(FALSE);
#else
	rtl8651_setAsicMulticastEnable(TRUE);
#endif
	rtl865x_setAsicMulticastAging(TRUE);
	_rtl865x_multicastRegisterEvent();
#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
	_rtl865x_L2multicastRegisterEvent();
#endif
#if (defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)) && defined(CONFIG_RTL_MLD_SNOOPING)
	rtl8198C_initMulticastv6();
#endif
	return SUCCESS;
}

/*
@func int32	| rtl865x_reinitMulticast	|  Re-init hardware ip multicast module.
@rvalue SUCCESS	|Re-initialize successfully.
@rvalue FAILED	|Re-initialize failed.
*/
int32 rtl865x_reinitMulticast(void)
{
	_rtl865x_multicastUnRegisterEvent();
#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
	_rtl865x_L2multicastUnRegisterEvent();
#endif
	/*delete all multicast entry*/
	rtl8651_setAsicMulticastEnable(FALSE);
	rtl865x_delMulticastEntry(0);
	
	#if defined(__linux__) && defined(__KERNEL__)
	_rtl865x_destroyMCastSysTimer();
	_rtl865x_initMCastSysTimer();
	#endif
	
	/*regfster twice won't cause any side-effect, 
	because event management module will handle duplicate event issue*/
	rtl8651_setAsicMulticastMTU(1522); 
	
#if !defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
	rtl8651_setAsicMulticastEnable(TRUE);
#endif
	rtl865x_setAsicMulticastAging(TRUE);
	_rtl865x_multicastRegisterEvent();
#if defined CONFIG_RTL_L2_HW_MULTICAST_SUPPORT
	_rtl865x_L2multicastRegisterEvent();
#endif
#if 0//defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	rtl8198C_reinitMulticastv6();
#endif
	return SUCCESS;
}	



#ifdef CONFIG_PROC_FS
extern int32 rtl8651_getAsicMulticastSpanningTreePortState(uint32 port, uint32 *portState);
#ifdef CONFIG_RTL_PROC_NEW
int32 rtl_dumpSwMulticastInfo(struct seq_file *s)
{
	uint32 mCastMtu=0;
	uint32 mCastEnable=FALSE;
	uint32 index;
	int8 isInternal;
	uint32 portStatus;
	uint32 internalPortMask=0;
	uint32 externalPortMask=0;
	int32 ret=FAILED;
	
	rtl865x_tblDrv_mCast_t *mCast_t, *nextMCast_t;
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    rtl8198c_tblDrv_mCastv6_t *mCastv6_t, *nextMCastv6_t;
    rtl8198c_mcast_fwd_descriptor6_t *curDescv6, *nextDescv6;
#endif
	uint32 entry;
	uint32 cnt;
	seq_printf(s,"----------------------------------------------------\n");
	seq_printf(s,"Asic Operation Layer :%d\n", rtl8651_getAsicOperationLayer());
	
	ret=rtl8651_getAsicMulticastEnable(&mCastEnable);
	if(ret==SUCCESS)
	{
		seq_printf(s,"Asic Multicast Table:%s\n", (mCastEnable==TRUE)?"Enable":"Disable");
	}
	else
	{
		seq_printf(s,"Read Asic Multicast Table Enable Bit Error\n");
	}
	ret=rtl8651_getAsicMulticastMTU(&mCastMtu); 
	if(ret==SUCCESS)
	{
		seq_printf(s,"Asic Multicast MTU:%d\n", mCastMtu);
	}
	else
	{
		seq_printf(s,"Read Asic Multicast MTU Error\n");
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	ret=rtl8198C_getAsicMulticastv6Enable(&mCastEnable);
	if(ret==SUCCESS)
	{
		seq_printf(s,"Asic IPV6 Multicast Table:%s\n", (mCastEnable==TRUE)?"Enable":"Disable");
	}
	else
	{
		seq_printf(s,"Read Asic IPV6 Multicast Table Enable Bit Error\n");
	}
	ret=rtl8198C_getAsicMulticastv6MTU(&mCastMtu); 
	if(ret==SUCCESS)
	{
		seq_printf(s,"Asic IPV6 Multicast MTU:%d\n", mCastMtu);
	}
	else
	{
		seq_printf(s,"Read Asic IPV6 Multicast MTU Error\n");
	}
#endif
	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++)
	{
		ret=rtl8651_getAsicMulticastPortInternal(index, &isInternal);
		if(ret==SUCCESS)
		{
			if(isInternal==TRUE)
			{
				internalPortMask |= 1<<index;
			}
			else
			{
				externalPortMask |= 1<<index;
			}
		}
	
	}

	seq_printf(s,"Internal Port Mask:0x%x\nExternal Port Mask:0x%x\n", internalPortMask,externalPortMask);
	seq_printf(s,"----------------------------------------------------\n");
	seq_printf(s,"Multicast STP State:\n");
	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++)
	{
		ret= rtl8651_getAsicMulticastSpanningTreePortState(index, &portStatus);
		if(ret==SUCCESS)
		{
			seq_printf(s,"port[%d]:",index);
			if(portStatus==RTL8651_PORTSTA_DISABLED)
			{
				seq_printf(s,"disabled\n");
			}
			else if(portStatus==RTL8651_PORTSTA_BLOCKING)
			{
				seq_printf(s,"blocking\n");
			}
			else if(portStatus==RTL8651_PORTSTA_LEARNING)
			{
				seq_printf(s,"learning\n");
			}
			else if(portStatus==RTL8651_PORTSTA_FORWARDING)
			{
				seq_printf(s,"forwarding\n");
			}
		}
		
	}
	seq_printf(s,"----------------------------------------------------\n");
	seq_printf(s,"Software Multicast Table:\n");
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		#if defined(CONFIG_RTL_WTDOG)
		rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
		#endif
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			seq_printf(s,"\t[%2d]  dip:%d.%d.%d.%d, sip:%d.%d.%d.%d, mbr:0x%x, svid:%d, spa:%d, \n", entry,
				mCast_t->dip>>24, (mCast_t->dip&0x00ff0000)>>16, (mCast_t->dip&0x0000ff00)>>8, (mCast_t->dip&0xff), 
				mCast_t->sip>>24, (mCast_t->sip&0x00ff0000)>>16, (mCast_t->sip&0x0000ff00)>>8, (mCast_t->sip&0xff),
				mCast_t->mbr,mCast_t->svid, mCast_t->port);
		#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
			seq_printf(s,"\t      extIP:0x%x,age:%d, cpu:%d, count:%d, inAsic:%d, (%s), portMappingMask:0x%x\n", 
				mCast_t->extIp,mCast_t->age, mCast_t->cpu,mCast_t->count,mCast_t->inAsic,mCast_t->unKnownMCast?"UnknownMCast":"KnownMCast", mCast_t->mapPortMbr);
		#else
			seq_printf(s,"\t      extIP:0x%x,age:%d, cpu:%d, count:%d, inAsic:%d, (%s)\n", 
				mCast_t->extIp,mCast_t->age, mCast_t->cpu,mCast_t->count,mCast_t->inAsic,mCast_t->unKnownMCast?"UnknownMCast":"KnownMCast");
		#endif
			cnt=0;
			curDesc=MC_LIST_FIRST(&mCast_t->fwdDescChain);
			while(curDesc)
			{
				nextDesc=MC_LIST_NEXT(curDesc, next );
				seq_printf(s,"\t      netif(%s),descPortMask(0x%x),toCpu(%d),fwdPortMask(0x%x)\n",curDesc->netifName,curDesc->descPortMask,curDesc->toCpu,curDesc->fwdPortMask);
				curDesc = nextDesc;
			}
			
			seq_printf(s,"\n");
			mCast_t = nextMCast_t;
		}
		
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	seq_printf(s,"----------------------------------------------------\n");
	seq_printf(s,"IPV6 Software Multicast Table:\n");
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		#if defined(CONFIG_RTL_WTDOG)
		rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
		#endif
		mCastv6_t = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCastv6_t) {
			/*save the next entry first*/
			nextMCastv6_t=TAILQ_NEXT(mCastv6_t, nextMCast); 		
			seq_printf(s,"\t[%2d]  dip:%08x:%08x%08x:%08x\n\t      sip:%08x:%08x:%08x:%08x\n\t      mbr:0x%x, svid:%d, spa:%d, ",
				entry,
				mCastv6_t->dip.v6_addr32[0],mCastv6_t->dip.v6_addr32[1],
				mCastv6_t->dip.v6_addr32[2],mCastv6_t->dip.v6_addr32[3],
				mCastv6_t->sip.v6_addr32[0],mCastv6_t->sip.v6_addr32[1],
				mCastv6_t->sip.v6_addr32[2],mCastv6_t->sip.v6_addr32[3],

				mCastv6_t->mbr,
				mCastv6_t->svid,
				mCastv6_t->port);
			
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
			seq_printf(s,"age:%d, cpu:%d, count:%d, inAsic:%d, (%s), portMappingMask:0x%x\n", 
				mCastv6_t->age, mCastv6_t->cpu,mCastv6_t->count,mCastv6_t->inAsic,mCastv6_t->unKnownMCast?"UnknownMCast":"KnownMCast", mCastv6_t->mapPortMbr);
#else
			seq_printf(s,"age:%d, cpu:%d, count:%d, inAsic:%d, (%s)\n", 
				mCastv6_t->age, mCastv6_t->cpu,mCastv6_t->count,mCastv6_t->inAsic,mCastv6_t->unKnownMCast?"UnknownMCast":"KnownMCast");
#endif
			cnt=0;
			curDescv6=MC_LIST_FIRST(&mCastv6_t->fwdDescChain);
			while(curDescv6)
			{
				nextDescv6=MC_LIST_NEXT(curDescv6, next );
				seq_printf(s,"\t      netif(%s),descPortMask(0x%x),toCpu(%d),fwdPortMask(0x%x)\n",curDescv6->netifName,curDescv6->descPortMask,curDescv6->toCpu,curDescv6->fwdPortMask);
				curDescv6 = nextDescv6;
			}
				
			seq_printf(s,"\n");
			mCastv6_t = nextMCastv6_t;
		}
			
	}
#endif

	return SUCCESS;
}
#else
int32 rtl_dumpSwMulticastInfo(char *page, int *ret_len)
{
    int len = *ret_len;
	uint32 mCastMtu=0;
	uint32 mCastEnable=FALSE;
	uint32 index;
	int8 isInternal;
	uint32 portStatus;
	uint32 internalPortMask=0;
	uint32 externalPortMask=0;
	int32 ret=FAILED;
	
	rtl865x_tblDrv_mCast_t *mCast_t, *nextMCast_t;
	rtl865x_mcast_fwd_descriptor_t *curDesc,*nextDesc;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    rtl8198c_tblDrv_mCastv6_t *mCastv6_t, *nextMCastv6_t;
    rtl8198c_mcast_fwd_descriptor6_t *curDescv6, *nextDescv6;
#endif
	uint32 entry;
	uint32 cnt;
	len += sprintf(page+len, "----------------------------------------------------\n");
	len += sprintf(page+len,"Asic Operation Layer :%d\n", rtl8651_getAsicOperationLayer());
	
	ret=rtl8651_getAsicMulticastEnable(&mCastEnable);
	if(ret==SUCCESS)
	{
		len += sprintf(page+len,"Asic Multicast Table:%s\n", (mCastEnable==TRUE)?"Enable":"Disable");
	}
	else
	{
		len += sprintf(page+len,"Read Asic Multicast Table Enable Bit Error\n");
	}
	ret=rtl8651_getAsicMulticastMTU(&mCastMtu); 
	if(ret==SUCCESS)
	{
		len += sprintf(page+len,"Asic Multicast MTU:%d\n", mCastMtu);
	}
	else
	{
		len += sprintf(page+len,"Read Asic Multicast MTU Error\n");
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	ret=rtl8198C_getAsicMulticastv6Enable(&mCastEnable);
	if(ret==SUCCESS)
	{
		len += sprintf(page+len,"Asic IPV6 Multicast Table:%s\n", (mCastEnable==TRUE)?"Enable":"Disable");
	}
	else
	{
		len += sprintf(page+len,"Read Asic IPV6 Multicast Table Enable Bit Error\n");
	}
	ret=rtl8198C_getAsicMulticastv6MTU(&mCastMtu); 
	if(ret==SUCCESS)
	{
		len += sprintf(page+len,"Asic IPV6 Multicast MTU:%d\n", mCastMtu);
	}
	else
	{
		len += sprintf(page+len,"Read Asic IPV6 Multicast MTU Error\n");
	}
#endif
	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++)
	{
		ret=rtl8651_getAsicMulticastPortInternal(index, &isInternal);
		if(ret==SUCCESS)
		{
			if(isInternal==TRUE)
			{
				internalPortMask |= 1<<index;
			}
			else
			{
				externalPortMask |= 1<<index;
			}
		}
	
	}

	len += sprintf(page+len,"Internal Port Mask:0x%x\nExternal Port Mask:0x%x\n", internalPortMask,externalPortMask);
	len += sprintf(page+len,"----------------------------------------------------\n");
	len += sprintf(page+len,"Multicast STP State:\n");
	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++)
	{
		ret= rtl8651_getAsicMulticastSpanningTreePortState(index, &portStatus);
		if(ret==SUCCESS)
		{
			len += sprintf(page+len,"port[%d]:",index);
			if(portStatus==RTL8651_PORTSTA_DISABLED)
			{
				len += sprintf(page+len,"disabled\n");
			}
			else if(portStatus==RTL8651_PORTSTA_BLOCKING)
			{
				len += sprintf(page+len,"blocking\n");
			}
			else if(portStatus==RTL8651_PORTSTA_LEARNING)
			{
				len += sprintf(page+len,"learning\n");
			}
			else if(portStatus==RTL8651_PORTSTA_FORWARDING)
			{
				len += sprintf(page+len,"forwarding\n");
			}
		}
		
	}
	len += sprintf(page+len,"----------------------------------------------------\n");
	len += sprintf(page+len,"Software Multicast Table:\n");
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		#if defined(CONFIG_RTL_WTDOG)
		rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
		#endif
		mCast_t = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			len += sprintf(page+len,"\t[%2d]  dip:%d.%d.%d.%d, sip:%d.%d.%d.%d, mbr:0x%x, svid:%d, spa:%d, \n", entry,
				mCast_t->dip>>24, (mCast_t->dip&0x00ff0000)>>16, (mCast_t->dip&0x0000ff00)>>8, (mCast_t->dip&0xff), 
				mCast_t->sip>>24, (mCast_t->sip&0x00ff0000)>>16, (mCast_t->sip&0x0000ff00)>>8, (mCast_t->sip&0xff),
				mCast_t->mbr,mCast_t->svid, mCast_t->port);
			
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
			len += sprintf(page+len,"\t	  extIP:0x%x,age:%d, cpu:%d, count:%d, inAsic:%d, (%s), portMappingMask:0x%x\n", 
				mCast_t->extIp,mCast_t->age, mCast_t->cpu,mCast_t->count,mCast_t->inAsic,mCast_t->unKnownMCast?"UnknownMCast":"KnownMCast", mCast_t->mapPortMbr);
#else
			len += sprintf(page+len,"\t      extIP:0x%x,age:%d, cpu:%d, count:%d, inAsic:%d, (%s)\n", 
				mCast_t->extIp,mCast_t->age, mCast_t->cpu,mCast_t->count,mCast_t->inAsic,mCast_t->unKnownMCast?"UnknownMCast":"KnownMCast");
#endif			
			cnt=0;
			curDesc=MC_LIST_FIRST(&mCast_t->fwdDescChain);
			while(curDesc)
			{
				nextDesc=MC_LIST_NEXT(curDesc, next );
				len += sprintf(page+len,"\t      netif(%s),descPortMask(0x%x),toCpu(%d),fwdPortMask(0x%x)\n",curDesc->netifName,curDesc->descPortMask,curDesc->toCpu,curDesc->fwdPortMask);
				curDesc = nextDesc;
			}
			
			len += sprintf(page+len,"\n");
			mCast_t = nextMCast_t;
		}
		
	}
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	len += sprintf(page+len,"----------------------------------------------------\n");
	len += sprintf(page+len,"IPV6 Software Multicast Table:\n");
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		#if defined(CONFIG_RTL_WTDOG)
		rtl_periodic_watchdog_kick(entry, WATCHDOG_NUM_OF_TIMES);
		#endif
		mCastv6_t = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCastv6_t) {
			/*save the next entry first*/
			nextMCastv6_t=TAILQ_NEXT(mCastv6_t, nextMCast); 		
			len += sprintf(page+len,"\t[%2d]  dip:%08x:%08x:%08x:%08x\n\t      sip:%08x:%08x:%08x:%08x\n\t      mbr:0x%x, svid:%d, spa:%d, ",
				entry,
				mCastv6_t->dip.v6_addr32[0],mCastv6_t->dip.v6_addr32[1],
				mCastv6_t->dip.v6_addr32[2],mCastv6_t->dip.v6_addr32[3],
				mCastv6_t->sip.v6_addr32[0],mCastv6_t->sip.v6_addr32[1],
				mCastv6_t->sip.v6_addr32[2],mCastv6_t->sip.v6_addr32[3],
				mCastv6_t->mbr,
				mCastv6_t->svid,
				mCastv6_t->port);
		#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
			len += sprintf(page+len,"age:%d, cpu:%d, count:%d, inAsic:%d, (%s), portMappingMask:0x%x\n", 
				mCastv6_t->age, mCastv6_t->cpu,mCastv6_t->count,mCastv6_t->inAsic,mCastv6_t->unKnownMCast?"UnknownMCast":"KnownMCast", mCastv6_t->mapPortMbr);
		#else
			len += sprintf(page+len,"age:%d, cpu:%d, count:%d, inAsic:%d, (%s)\n", 
				mCastv6_t->age, mCastv6_t->cpu,mCastv6_t->count,mCastv6_t->inAsic,mCastv6_t->unKnownMCast?"UnknownMCast":"KnownMCast");
		#endif
			cnt=0;
			curDescv6=MC_LIST_FIRST(&mCastv6_t->fwdDescChain);
			while(curDescv6)
			{
				nextDescv6=MC_LIST_NEXT(curDescv6, next );
				len += sprintf(page+len,"\t      netif(%s),descPortMask(0x%x),toCpu(%d),fwdPortMask(0x%x)\n",curDescv6->netifName,curDescv6->descPortMask,curDescv6->toCpu,curDescv6->fwdPortMask);
				curDescv6 = nextDescv6;
			}
				
			len += sprintf(page+len,"\n");
			mCastv6_t = nextMCastv6_t;
		}
			
	}
#endif
    *ret_len = len;
	return SUCCESS;
}
#endif
#endif

int rtl865x_genVirtualMCastFwdDescriptor(unsigned int forceToCpu, uint32 fwdPortMask, rtl865x_mcast_fwd_descriptor_t *fwdDescriptor)
{
	
	if(fwdDescriptor==NULL)
	{
		return FAILED;
	}
	
	memset(fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
	fwdDescriptor->toCpu=forceToCpu;
	fwdDescriptor->fwdPortMask=fwdPortMask;
	return SUCCESS;

}
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr, unsigned int mapPortMask)
#else
int rtl865x_blockMulticastFlow(unsigned int srcVlanId, unsigned int srcPort,unsigned int srcIpAddr, unsigned int destIpAddr)
#endif
{
	rtl865x_mcast_fwd_descriptor_t fwdDescriptor;
	rtl865x_tblDrv_mCast_t * existMCastEntry=NULL;
	existMCastEntry=rtl865x_findMCastEntry(destIpAddr, srcIpAddr, (uint16)srcVlanId, (uint16)srcPort);
	if(existMCastEntry!=NULL)
	{
		if(existMCastEntry->mbr==0)
		{
			return SUCCESS;
		}
	}
	memset(&fwdDescriptor, 0, sizeof(rtl865x_mcast_fwd_descriptor_t ));
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
	rtl865x_addMulticastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort, &fwdDescriptor, TRUE, 0, 0, 0, mapPortMask);
#else
	rtl865x_addMulticastEntry(destIpAddr, srcIpAddr, (unsigned short)srcVlanId, (unsigned short)srcPort, &fwdDescriptor, TRUE, 0, 0, 0);
#endif
	return SUCCESS;
}

/*
@func int32	| rtl865x_flushHWMulticastEntry	|  API to delete all multicast 
forwarding entry
@rvalue SUCCESS	|Delete hardware multicast forwarding entry successfully. 
@rvalue FAILED	|Delete hardware multicast forwarding entry failed.
*/

int32 rtl865x_flushHWMulticastEntry(void)
{

	rtl865x_tblDrv_mCast_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	

	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
		
		mCastEntry = TAILQ_FIRST(&mCastTbl.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if(mCastEntry->dip)
			{
				
				_rtl865x_freeMCastEntry(mCastEntry, entry);
				_rtl865x_arrangeMulticast(entry);
			}
			
			mCastEntry = nextMCastEntry;
		}
		


		
	}

	return SUCCESS;
}

int rtl865x_getMCastHashMethod(unsigned int *hashMethod)
{
	return rtl865x_getAsicMCastHashMethod(hashMethod);
}

int rtl865x_setMCastHashMethod(unsigned int hashMethod)
{
	uint32 oldHashMethod = 0;
	rtl865x_getAsicMCastHashMethod(&oldHashMethod);
	hashMethod&=0x3;
	if(hashMethod > 3)
	{
		return -1;
	}
	if(oldHashMethod != hashMethod)
	{
		rtl865x_setAsicMCastHashMethod(hashMethod);		
        /* exclude 0->1 and 1->0 */
        if(!((hashMethod == HASH_METHOD_SIP_DIP0 && oldHashMethod == HASH_METHOD_SIP_DIP1) || 
            (hashMethod == HASH_METHOD_SIP_DIP1 && oldHashMethod == HASH_METHOD_SIP_DIP0)))
        {

            rtl865x_flushHWMulticastEntry();                 
        }
	}
	return 0;
}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
static int is_ip6_addr_equal(inv6_addr_t addr1,inv6_addr_t addr2)
{
	if(addr1.v6_addr32[0]==addr2.v6_addr32[0]&&
	   addr1.v6_addr32[1]==addr2.v6_addr32[1]&&
	   addr1.v6_addr32[2]==addr2.v6_addr32[2]&&
	   addr1.v6_addr32[3]==addr2.v6_addr32[3])
	   return 1;
	else
	   return 0;
}
static int32 _rtl8198C_initMCastv6EntryPool(void)
{
	int32 index;
	rtl8198c_tblDrv_mCastv6_t *multiCast_t;
	struct MCast6_hash_head *mCast6_hash_head;
	
	TBL_MEM_ALLOC(multiCast_t, rtl8198c_tblDrv_mCastv6_t ,MAX_MCASTV6_TABLE_ENTRY_CNT);
	TAILQ_INIT(&mCastTbl6.freeList.freeMultiCast);
	for(index=0; index<MAX_MCASTV6_TABLE_ENTRY_CNT; index++)
	{
		memset( &multiCast_t[index], 0, sizeof(rtl8198c_tblDrv_mCastv6_t));
		TAILQ_INSERT_HEAD(&mCastTbl6.freeList.freeMultiCast, &multiCast_t[index], nextMCast);
	}

	TBL_MEM_ALLOC(mCast6_hash_head, struct MCast6_hash_head, RTL8651_MULTICASTTBL_SIZE);
	memset(mCast6_hash_head, 0,RTL8651_MULTICASTTBL_SIZE* sizeof(struct MCast6_hash_head));
	mCastTbl6.inuseList.mCastTbl = (void *)mCast6_hash_head;

	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++)
	{
		TAILQ_INIT(&mCastTbl6.inuseList.mCastTbl[index]);
	}
	return SUCCESS;
}

static int32 _rtl8198C_freeMCastv6FwdDesc(rtl8198c_mcast_fwd_descriptor6_t *descPtr)
{
	if(descPtr==NULL)
	{
		return SUCCESS;
	}
	memset(descPtr,0,sizeof(rtl8198c_mcast_fwd_descriptor6_t));
	MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head6, descPtr, next);
	
	return SUCCESS;
}


static rtl8198c_mcast_fwd_descriptor6_t *_rtl8198C_allocMCastv6FwdDesc(void)
{
	rtl8198c_mcast_fwd_descriptor6_t *retDesc=NULL;
	retDesc = MC_LIST_FIRST(&free_mcast_fwd_descriptor_head6);
	if(retDesc!=NULL)
	{
		MC_LIST_REMOVE(retDesc, next);
		memset(retDesc,0,sizeof(rtl8198c_mcast_fwd_descriptor6_t));
	}
	return retDesc;
}

static int32 _rtl8198C_mCastv6FwdDescEnqueue(mcast_fwd_descriptor_head6_t * queueHead,
												rtl8198c_mcast_fwd_descriptor6_t * enqueueDesc)
{
	rtl8198c_mcast_fwd_descriptor6_t *newDesc;
	rtl8198c_mcast_fwd_descriptor6_t *curDesc,*nextDesc;
	if(queueHead==NULL)
	{
		return FAILED;
	}
	
	if(enqueueDesc==NULL)
	{
		return SUCCESS;
	}
	
	/*multicast forward descriptor is internal maintained,always alloc new one*/
	newDesc=_rtl8198C_allocMCastv6FwdDesc();
	
	if(newDesc!=NULL)
	{
		memcpy(newDesc, enqueueDesc,sizeof(rtl8198c_mcast_fwd_descriptor6_t));
		memset(&(newDesc->next), 0, sizeof(MC_LIST_ENTRY(rtl8198c_mcast_fwd_descriptor6_t)));
		newDesc->next.le_next=NULL;
		newDesc->next.le_prev=NULL;
	}
	else
	{
		/*no enough memory*/
		return FAILED;
	}
	

	for(curDesc=MC_LIST_FIRST(queueHead);curDesc!=NULL;curDesc=nextDesc)
	{

		nextDesc=MC_LIST_NEXT(curDesc, next);
		
		/*merge two descriptor*/
		if((strcmp(curDesc->netifName,newDesc->netifName)==0) && (curDesc->vid==newDesc->vid))
		if(strcmp(curDesc->netifName,newDesc->netifName)==0)
		{	
			if(newDesc->descPortMask==0)
			{
				newDesc->descPortMask=curDesc->descPortMask;
			}
			MC_LIST_REMOVE(curDesc, next);
			_rtl8198C_freeMCastv6FwdDesc(curDesc);
			
		}
	}

	/*not matched descriptor is found*/
	MC_LIST_INSERT_HEAD(queueHead, newDesc, next);

	return SUCCESS;
	
}

static int32 _rtl8198C_flushMCastv6FwdDescChain(mcast_fwd_descriptor_head6_t * descChainHead)
{
	rtl8198c_mcast_fwd_descriptor6_t * curDesc,*nextDesc;
	
	if(descChainHead==NULL)
	{
		return SUCCESS;
	}
	
	curDesc=MC_LIST_FIRST(descChainHead);
	while(curDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next );
		/*remove from the old descriptor chain*/
		MC_LIST_REMOVE(curDesc, next);
		/*return to the free descriptor chain*/
		_rtl8198C_freeMCastv6FwdDesc(curDesc);
		curDesc = nextDesc;
	}

	return SUCCESS;
}


static int32 _rtl8198C_flushMCastv6Entry(rtl8198c_tblDrv_mCastv6_t *mCastEntry)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	_rtl8198C_flushMCastv6FwdDescChain(&mCastEntry->fwdDescChain);
	
	memset(mCastEntry, 0, sizeof(rtl8198c_tblDrv_mCastv6_t));
	return SUCCESS;
}


static int32 _rtl8198C_freeMCastv6Entry(rtl8198c_tblDrv_mCastv6_t * mCastEntry, uint32 hashIndex)
{
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	TAILQ_REMOVE(&mCastTbl6.inuseList.mCastTbl[hashIndex], mCastEntry, nextMCast);
	_rtl8198C_flushMCastv6Entry(mCastEntry);
	TAILQ_INSERT_HEAD(&mCastTbl6.freeList.freeMultiCast, mCastEntry, nextMCast);
	return SUCCESS;
}


static void _rtl8198C_arrangeMulticastv6(uint32 entryIndex)
{
	rtl8198C_tblAsicDrv_multiCastv6Param_t asic_mcast;
	rtl8198c_tblDrv_mCastv6_t *mCast_t=NULL;
	rtl8198c_tblDrv_mCastv6_t *select_t=NULL;
	rtl8198c_tblDrv_mCastv6_t *swapOutEntry=NULL;
	int32 retval;
	int32 hashMethod=0;
	rtl8198C_getMCastv6HashMethod(&hashMethod);
	TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[entryIndex], nextMCast) 
	{
		if ((mCast_t->cpu == 0) && !(mCast_t->flag & RTL865X_MULTICAST_PPPOEPATCH_CPUBIT)) 
		{ /* Ignore cpu=1 */

			if(mCast_t->inAsic==TRUE)
			{
				if(swapOutEntry==NULL)
				{
					swapOutEntry=mCast_t;
				}
				else
				{
					/*impossible, two flow in one asic entry*/
					swapOutEntry->inAsic=FALSE;
					mCast_t->inAsic = FALSE;
				}
			}
		
			if (select_t) 
			{
				if ((mCast_t->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
				{
					/*replace unknown multicast*/
					select_t = mCast_t;
				}
				else
				{
					/*select the heavy load*/
					if ((mCast_t->count) > (select_t->count))
					{
						select_t = mCast_t;
					}
				}
				
			}
			else 
			{
				select_t = mCast_t;
			}
			
			
		}
		else
		{
			mCast_t->inAsic = FALSE;	/* reset "inAsic" bit */
		} 
	}
	
	if(select_t && swapOutEntry)
	{
		if ((swapOutEntry->unKnownMCast==FALSE) && (select_t->unKnownMCast==TRUE))
		{
			/*replace unknown multicast*/
			select_t = swapOutEntry;
		}
		else
		{
			if((select_t->count <= (swapOutEntry->count+RTL865X_HW_MCAST_SWAP_GAP)))
				select_t = swapOutEntry;
		}
	}	
	
	if (select_t) 
	{
		if((swapOutEntry==NULL) ||(select_t==swapOutEntry))
		{
			select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
			bzero(&asic_mcast, sizeof(rtl8198C_tblAsicDrv_multiCastv6Param_t));
//			memcpy(&asic_mcast, select_t, (uint32)&(((rtl8198c_tblDrv_mCastv6_t *)0)->six_rd_idx));
			memcpy(&asic_mcast, select_t, sizeof(rtl8198C_tblAsicDrv_multiCastv6Param_t));
			
			retval = rtl8198C_setAsicIpMulticastv6Table(hashMethod,&asic_mcast);
			
#ifdef CONFIG_PROC_FS
			mcastAddOpCnt6++;
#endif
			assert(retval == SUCCESS);
			if(retval==SUCCESS)
			{
				select_t->inAsic = TRUE;
			}
			else
			{
				select_t->inAsic = FALSE;
				rtl8198C_delAsicIpMulticastv6Table(entryIndex);
#ifdef CONFIG_PROC_FS
				mcastDelOpCnt6++;
#endif
			}
				
			assert(retval == SUCCESS);
			TAILQ_REMOVE(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
			TAILQ_INSERT_HEAD(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
		}
		else/*(swapOutEntry!=NULL) && (select_t!=swapOutEntry)*/
		{
			
			/*disable swap and only explicit joined mulicast flow can replace unknown multicast flow*/
			if(1)
			{
				/*don't forget to set swapOutEntry's inAsic flag*/
				swapOutEntry->inAsic=FALSE;
				
				select_t->age = RTL865X_MULTICAST_TABLE_ASIC_AGE;
				bzero(&asic_mcast, sizeof(rtl8198C_tblAsicDrv_multiCastv6Param_t));
				memcpy(&asic_mcast, select_t, (uint32)&(((rtl8198C_tblAsicDrv_multiCastv6Param_t *)0)->six_rd_idx));
				retval = rtl8198C_setAsicIpMulticastv6Table(hashMethod,&asic_mcast);
				//printk("asic_mcast:dip:%x,[%s]:[%d].\n",asic_mcast.dip,__FUNCTION__,__LINE__);
#ifdef CONFIG_PROC_FS
				mcastAddOpCnt6++;
				
#endif
				assert(retval == SUCCESS);
				if(retval==SUCCESS)
				{
					select_t->inAsic = TRUE;
				}
				else
				{
					select_t->inAsic = FALSE;
					rtl8198C_delAsicIpMulticastv6Table(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt6++;
#endif
				}
				
				TAILQ_REMOVE(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);
				TAILQ_INSERT_HEAD(&mCastTbl6.inuseList.mCastTbl[entryIndex], select_t, nextMCast);

			}
			#if 0
			else
			{	
				//printk("swapOutEntry:%d,select:%d,[%s]:[%d].\n",swapOutEntry->unKnownMCast,select_t->unKnownMCast,__FUNCTION__,__LINE__);
				if(swapOutEntry->inAsic == FALSE)
				{
					/*maybe something is wrong, we remove the asic entry*/
					rtl8651_delAsicIpMulticastTable(entryIndex);
#ifdef CONFIG_PROC_FS
					mcastDelOpCnt6++;
#endif
				}
				
			}	
			#endif
			
		}
		
	}
	else 	
	{
		if(swapOutEntry!=NULL)
		{
			swapOutEntry->inAsic=FALSE;
		}
		rtl8198C_delAsicIpMulticastv6Table(entryIndex);
#ifdef CONFIG_PROC_FS
		mcastDelOpCnt6++;
#endif
	}
}

void _rtl8198C_timeUpdateMulticastv6(uint32 secPassed)
{
	rtl8198c_tblDrv_mCastv6_t *mCast_t, *nextMCast_t;
	uint32 entry;
	uint32 needReArrange=FALSE;
	uint32 hashLineCnt=0;
	//printk("[%s:%d]\n",__FUNCTION__,__LINE__);
	/* check to Aging and HW swapping */
	for (entry=0; entry< RTL8651_MULTICASTTBL_SIZE; entry++) {
		hashLineCnt=0;
		needReArrange=FALSE;
		mCast_t = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == TRUE)
			{
				/* Entry is in the ASIC */
				if (mCast_t->age <= secPassed) 
				{
					if(mCast_t->mbr==0)
					{
						_rtl8198C_freeMCastv6Entry(mCast_t,entry);
						needReArrange=TRUE;
					}
					else
					{
						mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					}
				}
				else
				{
					mCast_t->age -= secPassed;
				}
			}
			else 
			{
				//printk("------------mCast_t->count:%d,[%s]:[%d].\n",mCast_t->count,__FUNCTION__,__LINE__);
			
				//mCast_t->count=0;
			
				/* Entry is not in the ASIC */
				if (mCast_t->age <= secPassed)
				{ /* aging out */
					_rtl8198C_freeMCastv6Entry(mCast_t, entry);
				}
				else 
				{
					mCast_t->age -= secPassed;
				}
			}
			
			/*won't count multicast entry forwarded by cpu*/
			if(mCast_t->cpu==0)
			{
				
				hashLineCnt++;
				//printk("------------hashLineCnt:%d,[%s]:[%d].\n",hashLineCnt,__FUNCTION__,__LINE__);
				if(hashLineCnt>=2)
				{
					needReArrange=TRUE;
				}
			}
		
			//mCast_t->count = 0;
			mCast_t = nextMCast_t;
		}
		
		if(needReArrange==TRUE)
		{
			//printk("------------entry:%d,hashLineCnt:%d,[%s]:[%d].\n",entry,hashLineCnt,__FUNCTION__,__LINE__);
			_rtl8198C_arrangeMulticastv6(entry);
		}
		mCast_t = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		
		while (mCast_t) {
			/*save the next entry first*/
			nextMCast_t=TAILQ_NEXT(mCast_t, nextMCast);
			
			if (mCast_t->inAsic == FALSE)
				mCast_t->count=0;
			
			mCast_t = nextMCast_t;
		}
		
	}
}

#if defined(__linux__) && defined(__KERNEL__)
static void _rtl8198C_mCastv6SysTimerExpired(uint32 expireDada)
{

	_rtl8198C_timeUpdateMulticastv6(1);
	mod_timer(&rtl8198c_mCast6SysTimer, jiffies+HZ);
	
}

static void _rtl8198C_initMCastv6SysTimer(void)
{
	init_timer(&rtl8198c_mCast6SysTimer);
	rtl8198c_mCast6SysTimer.data=rtl8198c_mCast6SysTimer.expires;
	rtl8198c_mCast6SysTimer.expires=jiffies+HZ;
	rtl8198c_mCast6SysTimer.function=(void*)_rtl8198C_mCastv6SysTimerExpired;
	add_timer(&rtl8198c_mCast6SysTimer);
}
static void _rtl8198C_destroyMCastv6SysTimer(void)
{
	del_timer(&rtl8198c_mCast6SysTimer);
}
#endif


#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)

int rtl8198C_findEmptyCamEntryv6(void)
{
	int index=-1;

	for(index=RTL8651_IPMULTICASTTBL_SIZE; index<RTL8651_MULTICASTTBL_SIZE; index++)
	{

		if(TAILQ_EMPTY(&mCastTbl6.inuseList.mCastTbl[index]))
		{
			return index;
		}
	}
	
	return -1;
}
#endif


static int32 _rtl8198C_initMCastv6FwdDescPool(void)
{
	int32 i;
	MC_LIST_INIT(&free_mcast_fwd_descriptor_head6);
	TBL_MEM_ALLOC(rtl8198c_mcastFwdDescPool6, rtl8198c_mcast_fwd_descriptor6_t,MAX_MCASTV6_FWD_DESCRIPTOR_CNT);
	
	if(rtl8198c_mcastFwdDescPool6!=NULL)
	{
		memset( rtl8198c_mcastFwdDescPool6, 0, MAX_MCASTV6_FWD_DESCRIPTOR_CNT * sizeof(rtl8198c_mcast_fwd_descriptor6_t));	
	}
	else
	{
		return FAILED;
	}
	
	for(i = 0; i<MAX_MCASTV6_FWD_DESCRIPTOR_CNT;i++)
	{
		MC_LIST_INSERT_HEAD(&free_mcast_fwd_descriptor_head6, &rtl8198c_mcastFwdDescPool6[i], next);
	}
	
	return SUCCESS;
}

static rtl8198c_tblDrv_mCastv6_t * _rtl8198C_allocMCastv6Entry(uint32 hashIndex)
{
	rtl8198c_tblDrv_mCastv6_t *newEntry;
	newEntry=TAILQ_FIRST(&mCastTbl6.freeList.freeMultiCast);
	if (newEntry == NULL)
	{
		return NULL;
	}		
	
	TAILQ_REMOVE(&mCastTbl6.freeList.freeMultiCast, newEntry, nextMCast);

	
	/*initialize it*/
	if(MC_LIST_FIRST(&newEntry->fwdDescChain)!=NULL)
	{
		_rtl8198C_flushMCastv6FwdDescChain(&newEntry->fwdDescChain);
	}
	MC_LIST_INIT(&newEntry->fwdDescChain);
	
	memset(newEntry, 0, sizeof(rtl8198c_tblDrv_mCastv6_t));

	TAILQ_INSERT_TAIL(&mCastTbl6.inuseList.mCastTbl[hashIndex], newEntry, nextMCast);
	
	return newEntry;
}

static int32 _rtl8198C_mergeMCastv6FwdDescChain(mcast_fwd_descriptor_head6_t * targetChainHead ,
													rtl8198c_mcast_fwd_descriptor6_t *srcChain)
{
	rtl8198c_mcast_fwd_descriptor6_t *curDesc;

	if(targetChainHead==NULL)
	{
		return FAILED;
	}
	
	for(curDesc=srcChain; curDesc!=NULL; curDesc=MC_LIST_NEXT(curDesc,next))
	{
		
		_rtl8198C_mCastv6FwdDescEnqueue(targetChainHead, curDesc);
		
	}
	
	return SUCCESS;
}

static uint32 _rtl8198C_doMCastv6EntrySrcVlanPortFilter(rtl8198c_tblDrv_mCastv6_t *mCastEntry)
{
	rtl8198c_mcast_fwd_descriptor6_t * curDesc,*nextDesc;
	if(mCastEntry==NULL)
	{
		return SUCCESS;
	}
	
	for(curDesc=MC_LIST_FIRST(&mCastEntry->fwdDescChain);curDesc!=NULL;curDesc=nextDesc)
	{
		nextDesc=MC_LIST_NEXT(curDesc, next);
		{
			curDesc->fwdPortMask=curDesc->fwdPortMask & (~(1<<mCastEntry->port));
			if(curDesc->fwdPortMask==0)
			{
				/*remove from the old chain*/
				MC_LIST_REMOVE(curDesc, next);
				/*return to the free descriptor chain*/
				_rtl8198C_freeMCastv6FwdDesc(curDesc);

			}
		}
		
	}

	return SUCCESS;
}


static uint32 rtl8198C_genMCastv6EntryAsicFwdMask(rtl8198c_tblDrv_mCastv6_t *mCastEntry)
{
	uint32 asicFwdPortMask=0;
	rtl8198c_mcast_fwd_descriptor6_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(curDesc->toCpu==0)
		{
			asicFwdPortMask|=(curDesc->fwdPortMask & ((1<<RTL8651_MAC_NUMBER)-1));
		}
		else
		{
			asicFwdPortMask|=( 0x01<<RTL8651_MAC_NUMBER);
		}
	}
	asicFwdPortMask = asicFwdPortMask & (~(1<<mCastEntry->port)); 
	return asicFwdPortMask;
}

static uint16 rtl8198C_genMCastv6EntryCpuFlag(rtl8198c_tblDrv_mCastv6_t *mCastEntry)
{
	uint16 cpuFlag=FALSE;
	rtl8198c_mcast_fwd_descriptor6_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}

	if(mCastEntry->cpuHold==TRUE)
	{
		cpuFlag=TRUE;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		if(	(curDesc->toCpu==TRUE)	||
			(memcmp(curDesc->netifName, RTL_WLAN_NAME,4)==0)	)
		{
			cpuFlag=TRUE;
		}
	}
	
	return cpuFlag;
}

static void  _rtl8198C_setASICMulticastv6PortStatus(void) {
	uint32 index;
	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++) {
		rtl8651_setAsicMulticastPortInternal(index, (rtl865x_externalMulticastPortMask&(1<<index))?FALSE:TRUE);
	}
}


static void _rtl8198C_mCastv6EntryReclaim(void)
{
	uint32 index;
	uint32 freeCnt=0;
	uint32 asicFwdPortMask=0;
	uint32 needReArrange=FALSE;
	rtl8198c_tblDrv_mCastv6_t *curMCastEntry, *nextMCastEntry;

	/*free unused software forward entry*/
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]);
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if((curMCastEntry->inAsic==FALSE)  && (curMCastEntry->count==0))
			{
				_rtl8198C_freeMCastv6Entry(curMCastEntry, index);
				freeCnt++;
			}
			curMCastEntry = nextMCastEntry;
		}
		
	}

	if(freeCnt>0)
	{
		return;
	}
	
	for(index=0; index<RTL8651_MULTICASTTBL_SIZE; index++) 
	{
		curMCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]);
		needReArrange=FALSE;
		while (curMCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(curMCastEntry, nextMCast);
			if(curMCastEntry->inAsic)
			{
				asicFwdPortMask=rtl8198C_genMCastv6EntryAsicFwdMask(curMCastEntry);
				if(asicFwdPortMask==0) 
				{
					_rtl8198C_freeMCastv6Entry(curMCastEntry,index);
					needReArrange=TRUE;
				}
			}
			curMCastEntry = nextMCastEntry;
		}
		
		if(needReArrange==TRUE)
		{
			_rtl8198C_arrangeMulticastv6(index);
		}
	}

	return;
}

static inline void _rtl8198C_patchPppoeWeakv6(rtl8198c_tblDrv_mCastv6_t *mCast_t)
{
	rtl8198c_mcast_fwd_descriptor6_t * curDesc;
	uint32 netifType;
	/* patch: keep cache in software if one vlan's interface is pppoe */
	MC_LIST_FOREACH(curDesc, &(mCast_t->fwdDescChain), next)
	{
		if(rtl865x_getNetifType(curDesc->netifName, &netifType)==SUCCESS)
		{
			/*how about pptp,l2tp?*/
			if(netifType==IF_PPPOE)
			{
				mCast_t->flag |= RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
				return;
			}
		}
		
	}

	mCast_t->flag &= ~RTL865X_MULTICAST_PPPOEPATCH_CPUBIT;
}
//static 
uint32 rtl8198C_getMCastv6EntryDescPortMask(rtl8198c_tblDrv_mCastv6_t *mCastEntry)
{
	uint32 descPortMask=0;
	rtl8198c_mcast_fwd_descriptor6_t * curDesc;
	if(mCastEntry==NULL)
	{
		return 0;
	}
	
	MC_LIST_FOREACH(curDesc, &(mCastEntry->fwdDescChain), next)
	{
		descPortMask=descPortMask | curDesc->descPortMask;
	}
	
	return descPortMask;
}

static int32 rtl8198C_multicastv6CallbackFn(void *param)
{
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	inv6_addr_t groupAddr;
	uint32 index;
#if !defined(CONFIG_RTL_MULTI_LAN_DEV)
	uint32 oldDescPortMask;
#if !defined(CONFIG_RT_MULTIPLE_BR_SUPPOR)
	uint32 newDescPortMask;/*for device decriptor forwarding usage*/
#endif
#endif	
	uint32 oldAsicFwdPortMask,newAsicFwdPortMask;/*for physical port forwarding usage*/
	uint32 oldCpuFlag = 0,newCpuFlag = 0;
		
	rtl_multicastEventContext_t mcastEventContext;
	
	rtl8198c_mcast_fwd_descriptor6_t newFwdDesc;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	rtl8198c_tblDrv_mCastv6_t *mCastEntry,*nextMCastEntry;
#if !defined(CONFIG_RTL_MULTI_LAN_DEV) && !defined(CONFIG_RT_MULTIPLE_BR_SUPPORT)
	struct rtl_multicastDeviceInfo_s bridgeMCastDev;
#endif
#ifdef CONFIG_RTL_MLD_SNOOPING
	struct rtl_groupInfo groupInfo;
#endif

#if defined(CONFIG_RT_MULTIPLE_BR_SUPPORT)
	unsigned char br_name[16]={0};
	int br_index;
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
	struct net_bridge* br_dev = NULL;
#else
	unsigned int br_moduleIndex = 0xFFFFFFFF;
#endif
#endif
	int32 retVal=FAILED;
	
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	memcpy(&mcastEventContext,param,sizeof(rtl_multicastEventContext_t));
	/*check device name's validity*/
	if(strlen(mcastEventContext.devName)==0)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
	printk("%s:%d,mcastEventContext.devName is %s, mcastEventContext.groupAddr is 0x%x,mcastEventContext.sourceIP is 0x%x,mcastEventContext.portMask is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName, mcastEventContext.groupAddr[0], mcastEventContext.sourceAddr[0], mcastEventContext.portMask);
	#endif
	bzero(&groupAddr,sizeof(inv6_addr_t));
	memcpy(&groupAddr,mcastEventContext.groupAddr,sizeof(inv6_addr_t));
	/*case 1:this is multicast event from bridge(br0) */
	/*sync wlan and ethernet*/
	//hyking:[Fix me] the RTL_BR_NAME...
#if !defined(CONFIG_RTL_MULTI_LAN_DEV)
#if defined(CONFIG_OPENWRT_SDK)
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
    if(memcmp(mcastEventContext.devName,RTL_BR_NAME,6)==0 || memcmp(mcastEventContext.devName,RTL_BR1_NAME,3)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,6)==0)
#endif
#else
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0 || memcmp(mcastEventContext.devName,RTL_BR1_NAME,3)==0)
#elif defined(CONFIG_RT_MULTIPLE_BR_SUPPORT)
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,2)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0)
#endif
#endif
	{
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
				if (!is_ip6_addr_equal(mCastEntry->dip,groupAddr))
				{
					continue;
				}
	
				rtl_getGroupInfov6((uint32*)&mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
	
				oldDescPortMask=rtl8198C_getMCastv6EntryDescPortMask( mCastEntry); 
					
				/*sync with control plane*/
				#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT
				oldCpuFlag=mCastEntry->cpu;
				newDescPortMask = 0;
				newCpuFlag = 0;
				
				for(br_index = 0; br_index<RTL_IMGP_MAX_BRMODULE; br_index++)
				{	
					br_moduleIndex=rtl_get_brIgmpModuleIndexbyId(br_index,br_name);
					if(br_moduleIndex==0xFFFFFFFF)
						continue;

					/*sync with control plane*/
					memset(&newFwdDesc, 0 ,sizeof(rtl8198c_mcast_fwd_descriptor6_t));
					strcpy(newFwdDesc.netifName,br_name);
				
					multicastDataInfo.ipVersion=6;
					memcpy(multicastDataInfo.sourceIp, &mCastEntry->sip, sizeof(struct inv6_addr_s));
					memcpy(multicastDataInfo.groupAddr, &mCastEntry->dip, sizeof(struct inv6_addr_s));
				
				
					retVal= rtl_getMulticastDataFwdInfo(br_moduleIndex, &multicastDataInfo, &multicastFwdInfo);
					

					if(retVal == SUCCESS)
					{
						newCpuFlag |= multicastFwdInfo.cpuFlag;
					}
					
				}

				if(mCastEntry->unKnownMCast==TRUE)
					newCpuFlag = 1;	

				if(oldCpuFlag != newCpuFlag)
				{
					_rtl8198C_freeMCastv6Entry(mCastEntry,index);
					_rtl8198C_arrangeMulticastv6(index);
				}
				//printk("br:%s :oldAsicPortMask:%x,newAsicPortMask:%x,oldCpuFlag:%d,newCpuFlag:%d,[%s]:[%d].\n",devName,oldAsicPortMask,newAsicPortMask,oldCpuFlag,newCpuFlag,__FUNCTION__,__LINE__);
				#else
				memset(&newFwdDesc, 0 ,sizeof(rtl8198c_mcast_fwd_descriptor6_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
				multicastDataInfo.ipVersion=6;
				memcpy(multicastDataInfo.sourceIp,&mCastEntry->sip,sizeof(inv6_addr_t));
				memcpy(multicastDataInfo.groupAddr,&mCastEntry->dip,sizeof(inv6_addr_t));
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
					
	
				if(retVal!=SUCCESS)
				{
					continue;
				}
					
				retVal= rtl_getIgmpSnoopingModuleDevInfo(mcastEventContext.moduleIndex, &bridgeMCastDev);
				if(retVal!=SUCCESS)
				{
					continue;
				}
				newDescPortMask=multicastFwdInfo.fwdPortMask;
				if( (oldDescPortMask != newDescPortMask) &&
					(	((newDescPortMask & bridgeMCastDev.swPortMask)!=0) ||
						(((oldDescPortMask & bridgeMCastDev.swPortMask) !=0) && ((newDescPortMask & bridgeMCastDev.swPortMask)==0)))	)
				{
					/*this multicast entry should be re-generated at linux protocol stack bridge level*/
					_rtl8198C_freeMCastv6Entry(mCastEntry,index);
					_rtl8198C_arrangeMulticastv6(index);
				}
				#endif
					
			}
		}
			
		return EVENT_CONTINUE_EXECUTE;
	}

	/*case 2:this is multicast event from ethernet (eth0)*/
	/*update ethernet forwarding port mask*/

	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(memcmp(mcastEventContext.devName,"eth*",4)==0 || memcmp(mcastEventContext.devName,RTL_PS_ETH_NAME_ETH2,4)==0)
	#else
	if(memcmp(mcastEventContext.devName,"eth*",4)==0)
	#endif

	{
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
		printk("%s:%d,multicast event from ethernet (%s),mcastEventContext.groupAddr[0] is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName,mcastEventContext.groupAddr[0]);
	#endif
			
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
#if	0				
				printk("mCastEntry->dip:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
					mCastEntry->dip.v6_addr16[0],mCastEntry->dip.v6_addr16[1],
					mCastEntry->dip.v6_addr16[2],mCastEntry->dip.v6_addr16[3],
					mCastEntry->dip.v6_addr16[4],mCastEntry->dip.v6_addr16[5],
					mCastEntry->dip.v6_addr16[6],mCastEntry->dip.v6_addr16[7]);
				printk("groupAddr:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
					groupAddr.v6_addr16[0],groupAddr.v6_addr16[1],
					groupAddr.v6_addr16[2],groupAddr.v6_addr16[3],
					groupAddr.v6_addr16[4],groupAddr.v6_addr16[5],
					groupAddr.v6_addr16[6],groupAddr.v6_addr16[7]);
#endif
				if (!is_ip6_addr_equal(mCastEntry->dip,groupAddr))
				{
					continue;
				}
					
				memset(&newFwdDesc, 0 ,sizeof(rtl8198c_mcast_fwd_descriptor6_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
	
				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;
	
				/*sync with control plane*/
				multicastDataInfo.ipVersion=6;
				memcpy(multicastDataInfo.sourceIp,&mCastEntry->sip,sizeof(inv6_addr_t));
				memcpy(multicastDataInfo.groupAddr,&mCastEntry->dip,sizeof(inv6_addr_t));
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				if (multicastFwdInfo.unknownMCast == TRUE)
					multicastFwdInfo.cpuFlag = TRUE;

				newFwdDesc.fwdPortMask=multicastFwdInfo.fwdPortMask & (~(1<<mCastEntry->port));
				newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
				
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
				newFwdDesc.fwdPortMask &= mCastEntry->mapPortMbr;
#endif
				
				/*update/replace old forward descriptor*/

				_rtl8198C_mergeMCastv6FwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
				mCastEntry->mbr 		= rtl8198C_genMCastv6EntryAsicFwdMask(mCastEntry);
				mCastEntry->cpu 	= rtl8198C_genMCastv6EntryCpuFlag(mCastEntry);
					
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
					
			#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
				printk("%s:%d,oldAsicFwdPortMask is %d,newAsicFwdPortMask is %d\n",__FUNCTION__,__LINE__,oldAsicFwdPortMask,newAsicFwdPortMask);
			#endif
					
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
						/* we assume multicast member will NEVER in External interface, so we remove
								external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */
#ifdef CONFIG_RTL_MLD_SNOOPING	
				rtl_getGroupInfov6((uint32*)&mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
#endif		
				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl8198C_patchPppoeWeakv6(mCastEntry);
						
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{
						_rtl8198C_freeMCastv6Entry(mCastEntry,index);
					}
					_rtl8198C_arrangeMulticastv6(index);
				}
			}	
				
		}
	}

#else /*CONFIG_RTL_MULTI_LAN_DEV*/
#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,2)==0)
#else
	if(memcmp(mcastEventContext.devName,RTL_BR_NAME,3)==0)
#endif

	{
	#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
		printk("%s:%d,multicast event from ethernet (%s),mcastEventContext.groupAddr[0] is 0x%x\n",__FUNCTION__,__LINE__,mcastEventContext.devName,mcastEventContext.groupAddr[0]);
	#endif
			
		for (index=0; index< RTL8651_MULTICASTTBL_SIZE; index++)
		{
			for (mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[index]); mCastEntry; mCastEntry = nextMCastEntry)
			{
				nextMCastEntry=TAILQ_NEXT(mCastEntry, nextMCast);
#if	0				
				printk("mCastEntry->dip:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
					mCastEntry->dip.v6_addr16[0],mCastEntry->dip.v6_addr16[1],
					mCastEntry->dip.v6_addr16[2],mCastEntry->dip.v6_addr16[3],
					mCastEntry->dip.v6_addr16[4],mCastEntry->dip.v6_addr16[5],
					mCastEntry->dip.v6_addr16[6],mCastEntry->dip.v6_addr16[7]);
				printk("groupAddr:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
					groupAddr.v6_addr16[0],groupAddr.v6_addr16[1],
					groupAddr.v6_addr16[2],groupAddr.v6_addr16[3],
					groupAddr.v6_addr16[4],groupAddr.v6_addr16[5],
					groupAddr.v6_addr16[6],groupAddr.v6_addr16[7]);
#endif
				if (!is_ip6_addr_equal(mCastEntry->dip,groupAddr))
				{
					continue;
				}
					
				memset(&newFwdDesc, 0 ,sizeof(rtl8198c_mcast_fwd_descriptor6_t));
				strcpy(newFwdDesc.netifName,mcastEventContext.devName);
	
				/*save old multicast entry forward port mask*/
				oldAsicFwdPortMask=mCastEntry->mbr;
				oldCpuFlag=mCastEntry->cpu;
	
				/*sync with control plane*/
				multicastDataInfo.ipVersion=6;
				memcpy(multicastDataInfo.sourceIp,&mCastEntry->sip,sizeof(inv6_addr_t));
				memcpy(multicastDataInfo.groupAddr,&mCastEntry->dip,sizeof(inv6_addr_t));
				retVal= rtl_getMulticastDataFwdInfo(mcastEventContext.moduleIndex, &multicastDataInfo, &multicastFwdInfo);
				if (multicastFwdInfo.unknownMCast == TRUE)
					multicastFwdInfo.cpuFlag = TRUE;

				#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT
				br_dev = NULL;
				for(br_index = 0; br_index<RTL_IMGP_MAX_BRMODULE; br_index++)
				{
					br_dev = rtl_get_brByIndex(br_index, br_name);
					if(strcmp(br_name, mcastEventContext.devName)==0)
						break;
				}
				if(br_dev==NULL)
					continue;
				newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(br_dev,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
				#else
				newFwdDesc.fwdPortMask=rtl865x_getPhyFwdPortMask(bridge0,multicastFwdInfo.fwdPortMask) & (~(1<<mCastEntry->port));
				#endif 
				newFwdDesc.toCpu=multicastFwdInfo.cpuFlag;
				
				/*update/replace old forward descriptor*/

				_rtl8198C_mergeMCastv6FwdDescChain(&mCastEntry->fwdDescChain,&newFwdDesc);
				mCastEntry->mbr 		= rtl8198C_genMCastv6EntryAsicFwdMask(mCastEntry);
				mCastEntry->cpu 	= rtl8198C_genMCastv6EntryCpuFlag(mCastEntry);
					
				newAsicFwdPortMask	= mCastEntry->mbr ;
				newCpuFlag			=mCastEntry->cpu;
					
			#ifdef CONFIG_RTL865X_MUTLICAST_DEBUG
				printk("%s:%d,oldAsicFwdPortMask is %d,newAsicFwdPortMask is %d\n",__FUNCTION__,__LINE__,oldAsicFwdPortMask,newAsicFwdPortMask);
			#endif
					
#ifndef RTL8651_MCAST_ALWAYS2UPSTREAM
				if (mCastEntry->flag & RTL865X_MULTICAST_UPLOADONLY)
				{	/* remove upload term*/
					if((newAsicFwdPortMask!=0) && (newAsicFwdPortMask!=oldAsicFwdPortMask))
					{
						mCastEntry->flag &= ~RTL865X_MULTICAST_UPLOADONLY;
							/* we assume multicast member will NEVER in External interface, so we remove
								 external ip now */
						mCastEntry->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
					}
				}
#endif /* RTL8651_MCAST_ALWAYS2UPSTREAM */
#ifdef CONFIG_RTL_MLD_SNOOPING	
				rtl_getGroupInfov6((uint32*)&mCastEntry->dip, &groupInfo);
				if(groupInfo.ownerMask==0)
				{
					mCastEntry->unKnownMCast=TRUE;
				}
				else
				{
					mCastEntry->unKnownMCast=FALSE;
				}
#endif	

#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
				newAsicFwdPortMask &= (mCastEntry->mapPortMbr |(0x01<<6));
#endif

				if((oldCpuFlag != newCpuFlag)||(newAsicFwdPortMask!=oldAsicFwdPortMask)) 
				{
					_rtl8198C_patchPppoeWeakv6(mCastEntry);
						
					/*reset inAsic flag to re-select or re-write this hardware asic entry*/
					if(newAsicFwdPortMask==0)
					{
						_rtl8198C_freeMCastv6Entry(mCastEntry,index);
					}
					_rtl8198C_arrangeMulticastv6(index);
				}
			}	
				
		}
	}
#endif
#endif
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl8198C_multicastv6UnRegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST6;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl8198C_multicastv6CallbackFn;
	rtl865x_unRegisterEvent(&eventParam);
	return SUCCESS;
}
static int32 _rtl8198C_multicastv6RegisterEvent(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_UPDATE_MCAST6;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl8198C_multicastv6CallbackFn;
	rtl865x_registerEvent(&eventParam);
	return SUCCESS;
}


int32 rtl8198C_initMulticastv6(void)
{
	_rtl8198C_multicastv6UnRegisterEvent();
	_rtl8198C_initMCastv6EntryPool();
	_rtl8198C_initMCastv6FwdDescPool();
	#if defined(__linux__) && defined(__KERNEL__)
	_rtl8198C_initMCastv6SysTimer();
	#endif
	_rtl8198C_multicastv6RegisterEvent();
	rtl8198C_setAsicMulticastv6MTU(1522);
	rtl8198C_setAsicMulticastv6Enable(TRUE);
	rtl8198C_setAsicMulticastv6Aging(TRUE);
	return SUCCESS;
}

int32 rtl8198C_reinitMulticastv6(void)
{
	inv6_addr_t ip6addr0;
	_rtl8198C_multicastv6UnRegisterEvent();
	/*delete all multicast entry*/
	rtl8198C_setAsicMulticastv6Enable(FALSE);
	memset(&ip6addr0,0,sizeof(inv6_addr_t));
	rtl8198C_delMulticastv6Entry(ip6addr0);
	#if defined(__linux__) && defined(__KERNEL__)
	_rtl8198C_destroyMCastv6SysTimer();
	_rtl8198C_initMCastv6SysTimer();
	#endif
	
	/*regfster twice won't cause any side-effect, 
	because event management module will handle duplicate event issue*/

	rtl8198C_setAsicMulticastv6Enable(TRUE);
	rtl8198C_setAsicMulticastv6Aging(TRUE);

	_rtl8198C_multicastv6RegisterEvent();
	return SUCCESS;
}	

int32 rtl8198C_addMulticastv6ExternalPort(uint32 extPort)
{
	rtl8198c_externalMulticastPortMask6 |= (1<<extPort);
	_rtl8198C_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl8198C_delMulticastv6ExternalPort(uint32 extPort)
{
	rtl8198c_externalMulticastPortMask6 &= ~(1<<extPort);
	_rtl8198C_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl8198C_setMulticastv6ExternalPortMask(uint32 extPortMask)
{
	rtl8198c_externalMulticastPortMask6 =extPortMask;
	_rtl8198C_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl8198C_addMulticastv6ExternalPortMask(uint32 extPortMask)
{
	rtl8198c_externalMulticastPortMask6|= extPortMask;
	_rtl8198C_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl8198C_delMulticastv6ExternalPortMask(uint32 extPortMask)
{
	rtl8198c_externalMulticastPortMask6 &= ~extPortMask;
	_rtl8198C_setASICMulticastv6PortStatus();
	return SUCCESS;
}

int32 rtl8198C_getMulticastv6ExternalPortMask(void)
{
	return rtl8198c_externalMulticastPortMask6 ;
}

rtl8198c_tblDrv_mCastv6_t *rtl8198C_findMCastv6Entry(inv6_addr_t dip,inv6_addr_t sip, uint16 svid, uint16 sport)
{
	rtl8198c_tblDrv_mCastv6_t *mCast_t;
	uint32 entry=0;
	uint32 hashMethod=0;
	rtl8198C_getMCastv6HashMethod(&hashMethod);
	entry = rtl8198C_ipMulticastv6TableIndex(hashMethod,sip,dip);
	TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[entry], nextMCast) {
		if (is_ip6_addr_equal(mCast_t->dip,dip) && is_ip6_addr_equal(mCast_t->sip,sip) && mCast_t->port==sport)
		{
			if (mCast_t->inAsic == FALSE) 
			{
				mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
				mCast_t->count ++;
			}
			return mCast_t;
		}
	}
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	for(entry=RTL8651_IPMULTICASTTBL_SIZE; entry<RTL8651_MULTICASTTBL_SIZE; entry++)
	{
		TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[entry], nextMCast) {
			if (is_ip6_addr_equal(mCast_t->dip,dip) && is_ip6_addr_equal(mCast_t->sip,sip) && mCast_t->port==sport)
			{
				if (mCast_t->inAsic == FALSE) 
				{
					mCast_t->age = RTL865X_MULTICAST_TABLE_AGE;
					mCast_t->count ++;
				}
				return mCast_t;
			}
					
		}
	}
#endif	
	return (rtl8198c_tblDrv_mCastv6_t *)NULL;	
}

#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl8198C_addMulticastv6Entry(inv6_addr_t dip,inv6_addr_t sip, unsigned short svid, unsigned short sport, 
									rtl8198c_mcast_fwd_descriptor6_t * newFwdDescChain, 
									int flushOldChain, unsigned int extIp, char cpuHold, unsigned char flag, unsigned int mapPortMask)

#else
int rtl8198C_addMulticastv6Entry(inv6_addr_t dip,inv6_addr_t sip, unsigned short svid, unsigned short sport, 
									rtl8198c_mcast_fwd_descriptor6_t * newFwdDescChain, 
									int flushOldChain, unsigned int extIp, char cpuHold, unsigned char flag)
#endif								
{
	rtl8198c_tblDrv_mCastv6_t *mCast_t;
	uint32 hashIndex;
#ifdef CONFIG_RTL_MLD_SNOOPING
	uint32 groupIpAddr[4];
#endif
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	uint32 emptyCamIndex=-1; 
	#endif
	uint32 hashMethod=0;
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	struct rtl_groupInfo groupInfo;
	#endif
	rtl8198C_getMCastv6HashMethod(&hashMethod);
	hashIndex = rtl8198C_ipMulticastv6TableIndex(hashMethod,sip,dip);
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t=rtl8198C_findMCastv6Entry(dip, sip, svid, sport);
	if(mCast_t==NULL)
	{
		/*table entry collided*/
		if(!TAILQ_EMPTY(&mCastTbl6.inuseList.mCastTbl[hashIndex]))
		{
			emptyCamIndex=rtl8198C_findEmptyCamEntryv6();
			if(emptyCamIndex!=-1)
			{
				hashIndex=emptyCamIndex;
			}
		}
	}
	else
	{
		hashIndex=mCast_t->hashIndex;
	}
#else
	/*try to match hash line*/
	TAILQ_FOREACH(mCast_t, &mCastTbl6.inuseList.mCastTbl[hashIndex], nextMCast) 
	{
		if (is_ip6_addr_equal(mCast_t->sip,sip) && is_ip6_addr_equal(mCast_t->dip,dip) && mCast_t->port==sport)
			break;
	}
#endif	
	
	if (mCast_t == NULL) 
	{
		mCast_t=_rtl8198C_allocMCastv6Entry(hashIndex);
		if (mCast_t == NULL)
		{
			_rtl8198C_mCastv6EntryReclaim();
			mCast_t=_rtl8198C_allocMCastv6Entry(hashIndex);
			if(mCast_t == NULL)
			{
				return FAILED;
			}
		}
		mCast_t->sip		= sip;
		mCast_t->dip		= dip;
		mCast_t->port		= sport;
		mCast_t->mbr		= 0;
		mCast_t->count		= 0;
		mCast_t->inAsic		= FALSE;
		mCast_t->six_rd_eg  = 0;
		mCast_t->six_rd_idx = 0;
		mCast_t->svid		= svid;	
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
		mCast_t->mapPortMbr = mapPortMask;
#endif
	}
	
	if(flushOldChain)
	{
		_rtl8198C_flushMCastv6FwdDescChain(&mCast_t->fwdDescChain);
		
	}
	
	_rtl8198C_mergeMCastv6FwdDescChain(&mCast_t->fwdDescChain,newFwdDescChain);
	_rtl8198C_doMCastv6EntrySrcVlanPortFilter(mCast_t);
	
	mCast_t->mbr			= rtl8198C_genMCastv6EntryAsicFwdMask(mCast_t);

	mCast_t->age			= RTL865X_MULTICAST_TABLE_AGE;
#if 0
	mCast_t->cpu			= (toCpu==TRUE? 1: 0);
#else
	mCast_t->cpuHold			= cpuHold;
	mCast_t->cpu 			= rtl8198C_genMCastv6EntryCpuFlag(mCast_t);
#endif	
	mCast_t->flag			= flag;

	
	if (extIp)
		mCast_t->flag |= RTL865X_MULTICAST_EXTIP_SET;
	else
		mCast_t->flag &= ~RTL865X_MULTICAST_EXTIP_SET;
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	memcpy(groupIpAddr,&dip,sizeof(inv6_addr_t));
	rtl_getGroupInfov6(groupIpAddr, &groupInfo);
	if(groupInfo.ownerMask==0)
	{
		mCast_t->unKnownMCast=TRUE;
	}
	else
	{
		mCast_t->unKnownMCast=FALSE;
	}
	#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	mCast_t->hashIndex=hashIndex;
#endif
	_rtl8198C_patchPppoeWeakv6(mCast_t);
	_rtl8198C_arrangeMulticastv6(hashIndex);
	return SUCCESS;	
}
int rtl8198C_delMulticastv6Entry(inv6_addr_t groupAddr)
{
	rtl8198c_tblDrv_mCastv6_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	uint32 deleteFlag=FALSE;
	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
		deleteFlag=FALSE;
		mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if (is_ip6_addr_equal(groupAddr,mCastEntry->dip)) 
			{
				deleteFlag=TRUE;
				_rtl8198C_freeMCastv6Entry(mCastEntry, entry);
			}
			
			mCastEntry = nextMCastEntry;
		}
		
		if(deleteFlag==TRUE)
		{
			_rtl8198C_arrangeMulticastv6(entry);
		}
	}
	return SUCCESS;
}
int rtl8198C_genVirtualMCastv6FwdDescriptor(unsigned int forceToCpu, unsigned int  fwdPortMask, rtl8198c_mcast_fwd_descriptor6_t *fwdDescriptor)
{
	if(fwdDescriptor==NULL)
	{
		return FAILED;
	}
	memset(fwdDescriptor, 0, sizeof(rtl8198c_mcast_fwd_descriptor6_t ));
	fwdDescriptor->toCpu=forceToCpu;
	fwdDescriptor->fwdPortMask=fwdPortMask;
	return SUCCESS;
}
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
int rtl8198C_blockMulticastv6Flow(unsigned int srcVlanId,unsigned int srcPort,inv6_addr_t srcIpAddr,inv6_addr_t destIpAddr, unsigned int mapPortMask)
#else
int rtl8198C_blockMulticastv6Flow(unsigned int srcVlanId,unsigned int srcPort,inv6_addr_t srcIpAddr,inv6_addr_t destIpAddr)
#endif
{
	rtl8198c_mcast_fwd_descriptor6_t fwdDescriptor;
	rtl8198c_tblDrv_mCastv6_t * existMCastEntry=NULL;
	existMCastEntry=rtl8198C_findMCastv6Entry(destIpAddr, srcIpAddr, (uint16)srcVlanId, (uint16)srcPort);
	if(existMCastEntry!=NULL)
	{
		if(existMCastEntry->mbr==0)
		{
			return SUCCESS;
		}
	}
	memset(&fwdDescriptor, 0, sizeof(rtl8198c_mcast_fwd_descriptor6_t ));
#if defined CONFIG_RTL_MULTICAST_PORT_MAPPING
	rtl8198C_addMulticastv6Entry(destIpAddr,srcIpAddr,(unsigned short)srcVlanId,(unsigned short)srcPort,&fwdDescriptor,TRUE,0,0,0, mapPortMask);
#else
	rtl8198C_addMulticastv6Entry(destIpAddr,srcIpAddr,(unsigned short)srcVlanId,(unsigned short)srcPort,&fwdDescriptor,TRUE,0,0,0);
#endif
	return SUCCESS;
}

int32 rtl8198C_flushHWMulticastv6Entry(void)
{
	rtl8198c_tblDrv_mCastv6_t *mCastEntry, *nextMCastEntry;
	uint32 entry;
	inv6_addr_t ip0;
	memset(&ip0,0,sizeof(inv6_addr_t));
	for(entry=0; entry<RTL8651_MULTICASTTBL_SIZE; entry++) 
	{
		mCastEntry = TAILQ_FIRST(&mCastTbl6.inuseList.mCastTbl[entry]);
		while (mCastEntry)
		{
			nextMCastEntry = TAILQ_NEXT(mCastEntry, nextMCast);
			if(!is_ip6_addr_equal(ip0,mCastEntry->dip))
			{
				_rtl8198C_freeMCastv6Entry(mCastEntry, entry);
				_rtl8198C_arrangeMulticastv6(entry);
			}
			mCastEntry = nextMCastEntry;
		}	
	}
	return SUCCESS;
}
int rtl8198C_getMCastv6HashMethod(unsigned int *hashMethod)
{
	return rtl8198C_getAsicMCastv6HashMethod(hashMethod);
}
int rtl8198C_setMCastv6HashMethod(unsigned int hashMethod)
{
	uint32	  oldHashMethod = 0;
	rtl8198C_getAsicMCastv6HashMethod(&oldHashMethod);
	if(hashMethod >3)
	{
		return -1;
	}
	hashMethod &= 3;
					
	if (oldHashMethod != hashMethod) /* set IPV6CR Register bit 17~18 and flush multicastv6 table */
	{
		rtl8198C_setAsicMCastv6HashMethod(hashMethod);
		/* exclude 0->1 and 1->0 */
		if(!((hashMethod == HASH_METHOD_SIP_DIP0 && oldHashMethod == HASH_METHOD_SIP_DIP1) || 
			(hashMethod == HASH_METHOD_SIP_DIP1 && oldHashMethod == HASH_METHOD_SIP_DIP0)))
		{
			rtl8198C_flushHWMulticastv6Entry();   	
		}
				
	}
	return 0;
}

#endif

