/*
* Program :  igmp snooping function
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*  Copyright (c) 2011 Realtek Semiconductor Corp.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/

#ifdef __linux__
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
#include <linux/kconfig.h>
#else
#include <linux/config.h>
#endif
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#ifdef CONFIG_PROC_FS
#include <linux/seq_file.h>
#endif
#endif

#include "rtl865x_igmpsnooping_glue.h"
#include "rtl865x_igmpsnooping.h"
#include "rtl865x_igmpsnooping_local.h"

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include "../common/rtl865x_eventMgr.h"
#endif

static struct rtl_multicastModule rtl_mCastModuleArray[MAX_MCAST_MODULE_NUM];    
#if defined(__linux__) && defined(__KERNEL__)
static struct timer_list igmpSysTimer;	/*igmp timer*/
#endif
/*global system resources declaration*/
static uint32 rtl_totalMaxGroupCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxSourceCnt;   /*maximum total group entry count,  default is 3000*/

void *rtl_groupMemory=NULL;
void *rtl_sourceMemory=NULL;

static struct rtl_groupEntry *rtl_groupEntryPool=NULL;
static struct rtl_sourceEntry *rtl_sourceEntryPool=NULL;

static struct rtl_mCastTimerParameters rtl_mCastTimerParas;  /*IGMP snooping parameters */

static uint32 rtl_hashTableSize=0;
static uint32 rtl_hashMask=0;

/*the system up time*/
static uint32 rtl_startTime;
static uint32 rtl_sysUpSeconds;       
     
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
static rtl_multicastEventContext_t reportEventContext;
static rtl_multicastEventContext_t timerEventContext;
static rtl_multicastEventContext_t linkEventContext;
#endif

/*******************************internal function declaration*****************************/



/**************************
	resource managment
**************************/
static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize);
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void);
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) ;


static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize);
static  struct rtl_sourceEntry* rtl_allocateSourceEntry(void);
static  void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) ;


/**********************************Structure Maintenance*************************/

static struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr);
static void rtl_linkGroupEntry(struct rtl_groupEntry* entryNode ,  struct rtl_groupEntry ** hashTable, uint32 hashIndex);
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* entryNode,  struct rtl_groupEntry ** hashTable, uint32 hashIndex);
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntryPtr);

static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_groupEntry *groupEntry);
static void rtl_linkSourceEntry(struct rtl_groupEntry *groupEntry,  struct rtl_sourceEntry* entryNode);
static void rtl_unlinkSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* entryNode);
static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr);
static void rtl_deleteSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* sourceEntry);
#if 0
static int32 rtl_checkPortMask(uint8 pktPortMask);
static uint32 rtl_mapPortMaskToPortNum(uint8 pormask);
#endif


//static int32 rtl_mapMCastIPToMAC(uint32 ipVersion, uint32 *ipAddr, uint8 *macAddr );
static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr);

#ifdef CONFIG_RTL_MLD_SNOOPING
static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2);
static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr);
#endif
static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2);
static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen);

static uint32 rtl_getGroupFwdPortMask(struct rtl_groupEntry * groupEntry, uint32 enableSourceList, uint32 sysTime);
static void rtl_checkSourceTimer(struct rtl_groupEntry * groupEntry , struct rtl_sourceEntry * sourceEntry);
static uint32 rtl_getSourceFwdPortMask(struct rtl_groupEntry * groupEntry,uint32 *sourceAddr, uint32 sysTime);
//static void rtl_checkGroupFilterTimer(struct rtl_groupEntry *groupEntry);             
static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry,uint32 enableSourceList, struct rtl_groupEntry ** hashTable);

static uint32  rtl_getMulticastRouterPortMask(uint32 moduleIndex, uint32 ipVersion, uint32 sysTime);


/*hash table operation*/
static int32 rtl_initHashTable(uint32 moduleIndex, uint32 hashTableSize);


/************************************Pkt Process**********************************/
/*MAC frame analyze function*/
static void  rtl_parseMacFrame(uint32 moduleIndex, uint8* MacFrame, uint32 verifyCheckSum, struct rtl_macFrameInfo* macInfo);

/*Process Query Packet*/
static void rtl_snoopQuerier(uint32 moduleIndex, uint32 ipVersion, uint32 portNum);
static uint32 rtl_processQueries(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen);
/*Process Report Packet*/
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf); // process join report packet 
static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf); //process leave/done report packet
static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf); //process MODE_IS_INCLUDE report packet 
static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint8 *pktBuf); //process MODE_IS_EXCLUDE report packet
static  int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf); //process CHANGE_TO_INCLUDE_MODE report packet
static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint8 *pktBuf); //process CHANGE_TO_EXCLUDE_MODE report packet
static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf); //process ALLOW_NEW_SOURCES report packet 
static  int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint8 *pktBuf); //process BLOCK_OLD_SOURCES report packet
static  uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf);

/*******************different protocol process function**********************************/
static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum);
static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum);
static uint32 rtl_processMospf(uint32 moduleIndex, uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum);
static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum);

#if  defined(__linux__) && defined(__KERNEL__)
static void rtl_multicastSysTimerExpired(uint32 expireDada);
static void rtl_multicastSysTimerInit(void);
static void rtl_multicastSysTimerDestroy(void);
#endif


/************************************************
			Implementation
  ************************************************/
  
/**************************
	Initialize
**************************/
/*
@func int32		| rtl_initMulticastSnooping	|Initialize  igmp snooping system.
@parm 		struct rtl_mCastSnoopingGlobalConfig  | mCastSnoopingGlobalConfig | Global parameters to config igmp snooping system.
@rvalue SUCCESS	| 	Initialize successfully.
@rvalue FAILED	| 	Initialize failed.
*/
int32 rtl_initMulticastSnooping(struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig)
{
	int i,j;
	uint32 maxHashTableSize=MAX_HASH_TABLE_SIZE;
	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		memset(&(rtl_mCastModuleArray[i]), 0,sizeof(struct rtl_multicastModule));	
		
		for(j=0; j<6; j++)
		{
			rtl_mCastModuleArray[i].rtl_gatewayMac[j]=0;
		}
		
		rtl_mCastModuleArray[i].rtl_gatewayIpv4Addr=0;
		rtl_mCastModuleArray[i].rtl_ipv4HashTable=NULL;	
		
		#ifdef CONFIG_RTL_MLD_SNOOPING	
		for(j=0; j<4; j++)
		{
			rtl_mCastModuleArray[i].rtl_gatewayIpv6Addr[j]=0;
		}
		rtl_mCastModuleArray[i].rtl_ipv6HashTable=NULL;
		#endif

		rtl_mCastModuleArray[i].enableSourceList=TRUE;
		rtl_mCastModuleArray[i].enableSnooping=FALSE;
		rtl_mCastModuleArray[i].enableFastLeave=TRUE;
	}


       /*set multicast snooping parameters, use default value*/
      if(mCastSnoopingGlobalConfig.groupMemberAgingTime==0)
      {
	      rtl_mCastTimerParas.groupMemberAgingTime= DEFAULT_GROUP_MEMBER_INTERVAL;
      }
      else
      {
	      rtl_mCastTimerParas.groupMemberAgingTime= mCastSnoopingGlobalConfig.groupMemberAgingTime;
      }

      if(mCastSnoopingGlobalConfig.lastMemberAgingTime==0)
      {
             rtl_mCastTimerParas.lastMemberAgingTime= 0;
      }
      else
      {
	      rtl_mCastTimerParas.lastMemberAgingTime= mCastSnoopingGlobalConfig.lastMemberAgingTime;
      }

      if(mCastSnoopingGlobalConfig.querierPresentInterval==0)
      {
	      rtl_mCastTimerParas.querierPresentInterval= DEFAULT_QUERIER_PRESENT_TIMEOUT;
      }
	else
      {
	      rtl_mCastTimerParas.querierPresentInterval=mCastSnoopingGlobalConfig.querierPresentInterval;
      }


      if(mCastSnoopingGlobalConfig.dvmrpRouterAgingTime==0)
      {
	      rtl_mCastTimerParas.dvmrpRouterAgingTime=DEFAULT_DVMRP_AGING_TIME;
      }
      else
      {
	      rtl_mCastTimerParas.dvmrpRouterAgingTime=mCastSnoopingGlobalConfig.dvmrpRouterAgingTime;
      }

      if(mCastSnoopingGlobalConfig.mospfRouterAgingTime==0)
      {
	      rtl_mCastTimerParas.mospfRouterAgingTime=DEFAULT_MOSPF_AGING_TIME;
      }
  	else
      {
	      rtl_mCastTimerParas.mospfRouterAgingTime=mCastSnoopingGlobalConfig.mospfRouterAgingTime;
      }

      if(mCastSnoopingGlobalConfig.pimRouterAgingTime==0)
      {
	      rtl_mCastTimerParas.pimRouterAgingTime=DEFAULT_PIM_AGING_TIME;
      }
      else
      {
	      rtl_mCastTimerParas.pimRouterAgingTime=mCastSnoopingGlobalConfig.pimRouterAgingTime;
      }

	 /* set hash table size and hash mask*/
       if(mCastSnoopingGlobalConfig.hashTableSize==0)
        {
	      rtl_hashTableSize=DEFAULT_HASH_TABLE_SIZE;   /*default hash table size*/
        }
        else
        {
		for(i=0;i<11;i++)
	        {
		      if(mCastSnoopingGlobalConfig.hashTableSize>=maxHashTableSize)
		      {
			      rtl_hashTableSize=maxHashTableSize;
		
			      break;
		      }
	 	      maxHashTableSize=maxHashTableSize>>1;
		
	        }
        }

      rtl_hashMask=rtl_hashTableSize-1;
	  
	  
      if(mCastSnoopingGlobalConfig.maxGroupNum==0)
      {
	      rtl_totalMaxGroupCnt=DEFAULT_MAX_GROUP_COUNT;
      }	
      else
      {
		rtl_totalMaxGroupCnt=mCastSnoopingGlobalConfig.maxGroupNum;
      }

	/*initialize group entry pool*/
      rtl_groupMemory=NULL;
      rtl_sourceMemory=NULL;

      rtl_groupEntryPool=rtl_initGroupEntryPool(rtl_totalMaxGroupCnt); 
      if(rtl_groupEntryPool==NULL)
      {
	      return FAILED;
      }
	  

	if(mCastSnoopingGlobalConfig.maxSourceNum==0)
      {
	      rtl_totalMaxSourceCnt=DEFAULT_MAX_SOURCE_COUNT;
      }	
      else
      {
            	rtl_totalMaxSourceCnt=mCastSnoopingGlobalConfig.maxSourceNum;
      }
	  
	rtl_sourceEntryPool=rtl_initSourceEntryPool(rtl_totalMaxSourceCnt); 
      if(rtl_sourceEntryPool==NULL)
      {
	      rtl_totalMaxSourceCnt=0;
	      return FAILED;
      }
	  
#if defined(__linux__) && defined(__KERNEL__)
	rtl_multicastSysTimerInit();
#endif

	return SUCCESS;

}

/*
@func int32	| rtl_exitMulticastSnooping	| Exit igmp snooping system.
@rvalue SUCCESS	| 	Exit successfully.
@rvalue FAILED	| 	Exit failed.
*/
int32 rtl_exitMulticastSnooping(void)
{

	uint32 moduleIndex;
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		rtl_unregisterIgmpSnoopingModule(moduleIndex);
	}

	rtl_hashTableSize=0;
	rtl_hashMask=0;
	memset(&rtl_mCastTimerParas,0,sizeof(struct rtl_mCastTimerParameters));
	
	if(rtl_groupMemory!=NULL)
	{
		rtl_glueFree(rtl_groupMemory);	
	}
	
	rtl_totalMaxGroupCnt=0;
	rtl_groupMemory=NULL;
	rtl_groupEntryPool=NULL;
	
	
	if(rtl_sourceMemory!=NULL)
	{
		rtl_glueFree(rtl_sourceMemory);
	}	  

	rtl_totalMaxSourceCnt=0;
	rtl_sourceMemory=NULL;
	rtl_sourceEntryPool=NULL;

#if defined(__linux__) && defined(__KERNEL__)
	rtl_multicastSysTimerDestroy();
#endif

	 return SUCCESS;
	
}

static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize)
{
	
	uint32 idx=0;
	struct rtl_groupEntry *poolHead=NULL;
	struct rtl_groupEntry *entryPtr=NULL;
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_groupEntry *)rtl_glueMalloc(sizeof(struct rtl_groupEntry) * poolSize);
	rtl_groupMemory=(void *)poolHead;
	
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_groupEntry)));
		entryPtr = poolHead;

		/* link the whole group entry pool */
		for (idx = 0 ; idx < poolSize ; idx++, entryPtr++)
		{	
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(idx == (poolSize - 1))
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (poolSize - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
		}
	}
	
out:

	rtl_glueMutexUnlock();	/* UnLock resource */
	return poolHead;
	
}



/**************************
	Resource Managment
**************************/

// allocate a group entry pool from the group entry pool
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void)
{
	struct rtl_groupEntry *ret = NULL;

	rtl_glueMutexLock();	
		if (rtl_groupEntryPool!=NULL)
		{
			ret = rtl_groupEntryPool;
			if(rtl_groupEntryPool->next!=NULL)
			{
				rtl_groupEntryPool->next->previous=NULL;
			}
			rtl_groupEntryPool = rtl_groupEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_groupEntry));
		}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a group entry and link it back to the group entry pool, default is link to the pool head
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) 
{
	if (!groupEntryPtr)
	{
		return;
	}
		
	rtl_glueMutexLock();	
		groupEntryPtr->next = rtl_groupEntryPool;
		if(rtl_groupEntryPool!=NULL)
		{
			rtl_groupEntryPool->previous=groupEntryPtr;
		}
		rtl_groupEntryPool=groupEntryPtr;	
	rtl_glueMutexUnlock();	
}



static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize)
{

	uint32 idx=0;
	struct rtl_sourceEntry *poolHead=NULL;
	struct rtl_sourceEntry *entryPtr=NULL;
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_sourceEntry *)rtl_glueMalloc(sizeof(struct rtl_sourceEntry) * rtl_totalMaxSourceCnt);
	rtl_sourceMemory=(void *)poolHead;
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_sourceEntry)));
		entryPtr = poolHead;

		/* link the whole source entry pool */
		for (idx = 0 ; idx < poolSize ; idx++, entryPtr++)
		{	
			if(idx==0)
			{
				entryPtr->previous=NULL;
				if(idx == (poolSize - 1))
				{
					entryPtr->next=NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			else
			{
				entryPtr->previous=entryPtr-1;
				if (idx == (poolSize - 1))
				{
					entryPtr->next = NULL;
				}
				else
				{
					entryPtr->next = entryPtr + 1;
				}
			}
			
		}
	}
	
out:
	rtl_glueMutexUnlock();	/* UnLock resource */
	return poolHead;

}



/**************************
	Resource Managment
**************************/

// allocate a group entry pool from the group entry pool
static  struct rtl_sourceEntry* rtl_allocateSourceEntry(void)
{
	struct rtl_sourceEntry *ret = NULL;

	rtl_glueMutexLock();	
		if (rtl_sourceEntryPool!=NULL)
		{	
			ret = rtl_sourceEntryPool;
			if(rtl_sourceEntryPool->next!=NULL)
			{
				rtl_sourceEntryPool->next->previous=NULL;
			}
			rtl_sourceEntryPool = rtl_sourceEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_sourceEntry));
		}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a group entry and link it back to the group entry pool, default is link to the pool head
static  void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) 
{
	if (!sourceEntryPtr)
	{
		return;
	}
		
	rtl_glueMutexLock();	
		sourceEntryPtr->next = rtl_sourceEntryPool;
		if(rtl_sourceEntryPool!=NULL)
		{
			rtl_sourceEntryPool->previous=sourceEntryPtr;
		}

		rtl_sourceEntryPool=sourceEntryPtr;	

	rtl_glueMutexUnlock();	
}



/*********************************************
			Group list operation
 *********************************************/

/*       find a group address in a group list    */

struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr)
{
	struct rtl_groupEntry* groupPtr = NULL;
	int32 hashIndex;
	if(ipVersion==IP_VERSION4)
	{
		hashIndex=rtl_hashMask&multicastAddr[0];
		groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];

	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING	
	if(ipVersion==IP_VERSION6)
	{
		hashIndex=rtl_hashMask&multicastAddr[3];
		groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
	}
#endif	

	while (groupPtr!=NULL)
	{	

#ifdef CONFIG_RTL_MLD_SNOOPING
		if(ipVersion==IP_VERSION4)
		{
			if((multicastAddr[0]==groupPtr->groupAddr[0]) && (groupPtr->ipVersion==IP_VERSION4))
			{
				return groupPtr;
			}
		}
		
		if(ipVersion==IP_VERSION6)
		{
			if(	(multicastAddr[0]==groupPtr->groupAddr[0])&&
				(multicastAddr[1]==groupPtr->groupAddr[1])&&
				(multicastAddr[2]==groupPtr->groupAddr[2])&&
				(multicastAddr[3]==groupPtr->groupAddr[3])
			)	
			{
				return groupPtr;
				
			}
		}
#else
	if(ipVersion==IP_VERSION4)
	{
		if(multicastAddr[0]==groupPtr->groupAddr[0])
		{
			return groupPtr;
		}
	}
#endif
		groupPtr = groupPtr->next;

	}

	return NULL;
}


/* link group Entry in the front of a group list */
static void  rtl_linkGroupEntry(struct rtl_groupEntry* groupEntry ,  struct rtl_groupEntry ** hashTable, uint32 hashIndex)
{
	rtl_glueMutexLock();//Lock resource
	if(NULL==groupEntry)
	{
		return;
	}
	else
	{
		if(hashTable[hashIndex]!=NULL)
		{
			hashTable[hashIndex]->previous=groupEntry;
		}
		groupEntry->next = hashTable[hashIndex];
		hashTable[hashIndex]=groupEntry;
		hashTable[hashIndex]->previous=NULL;
		
	}
	rtl_glueMutexUnlock();//UnLock resource

}


/* unlink a group entry from group list */
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* groupEntry,  struct rtl_groupEntry ** hashTable, uint32 hashIndex)
{	
	if(NULL==groupEntry)
	{
		return;
	}
	else
	{
		rtl_glueMutexLock();  /* lock resource*/	
		/* unlink entry node*/
		if(groupEntry==hashTable[hashIndex]) /*unlink group list head*/
		{
			hashTable[hashIndex]=groupEntry->next;
			if(hashTable[hashIndex]!=NULL)
			{
				hashTable[hashIndex]->previous=NULL;
			}

			groupEntry->previous=NULL;
			groupEntry->next=NULL;
		}
		else
		{
			if(groupEntry->previous!=NULL)
			{
				groupEntry->previous->next=groupEntry->next;
			}
			 
			if(groupEntry->next!=NULL)
			{
				groupEntry->next->previous=groupEntry->previous;
			}
			groupEntry->previous=NULL;
			groupEntry->next=NULL;
		}
		
		rtl_glueMutexUnlock();//UnLock resource
	}
}


/* clear the content of group entry */
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntry)
{
	rtl_glueMutexLock();
	if (NULL!=groupEntry)
	{
		memset(groupEntry, 0, sizeof(struct rtl_groupEntry));
	}
	rtl_glueMutexUnlock();
}

static void rtl_deleteSourceList(struct rtl_groupEntry* groupEntry)
{
	struct rtl_sourceEntry *sourceEntry=groupEntry->sourceList;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	while(sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		rtl_deleteSourceEntry(groupEntry,sourceEntry);
		sourceEntry=nextSourceEntry;
	}
}

static void rtl_deleteGroupEntry( struct rtl_groupEntry* groupEntry,struct rtl_groupEntry ** hashTable)
{	
	if(groupEntry!=NULL)
	{
		
		if(groupEntry->ipVersion==IP_VERSION4)
		{	
			rtl_deleteSourceList(groupEntry);
			rtl_unlinkGroupEntry(groupEntry, hashTable,(groupEntry->groupAddr[0]&rtl_hashMask));
			rtl_clearGroupEntry(groupEntry);
			rtl_freeGroupEntry(groupEntry);
		}
		else
		{
			rtl_deleteSourceList(groupEntry);
			rtl_unlinkGroupEntry(groupEntry, hashTable,(groupEntry->groupAddr[3]&rtl_hashMask));
			rtl_clearGroupEntry(groupEntry);
			rtl_freeGroupEntry(groupEntry);
		}
	}
		
}

static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_groupEntry *groupEntry)
{
	struct rtl_sourceEntry *sourcePtr=groupEntry->sourceList;
	while(sourcePtr!=NULL)
	{
		if(ipVersion==IP_VERSION4)
		{
			if(sourceAddr[0]==sourcePtr->sourceAddr[0])
			{
				return sourcePtr;
			}
		}
		
#ifdef CONFIG_RTL_MLD_SNOOPING		
		if(ipVersion==IP_VERSION6)
		{
			if(	(sourceAddr[0]==sourcePtr->sourceAddr[0]) &&
				(sourceAddr[1]==sourcePtr->sourceAddr[1])&&
				(sourceAddr[2]==sourcePtr->sourceAddr[2])&&
				(sourceAddr[3]==sourcePtr->sourceAddr[3])
			)
			{
				return sourcePtr;
					
			}
		}
#endif
		sourcePtr=sourcePtr->next;
	}

	return NULL;
}

static int32 rtl_searchSourceAddr(uint32 ipVersion, uint32 *sourceAddr, uint32 *sourceArray, uint32 elementCount)
{
	uint32 i=0;
	uint32 *srcPtr=sourceArray;
	
	for(i=0; i<elementCount; i++)
	{
		if(ipVersion==IP_VERSION4)
		{
			if(sourceAddr[0]==srcPtr[0])
			{
				return TRUE;
			}
			srcPtr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		if(ipVersion==IP_VERSION6)
		{
			if(	(sourceAddr[0]==srcPtr[0])&&\
				(sourceAddr[1]==srcPtr[1])&&\
				(sourceAddr[2]==srcPtr[2])&&\
				(sourceAddr[3]==srcPtr[3]))
			{
			
				return TRUE;
			}
			
			srcPtr=srcPtr+4;
		}
#endif
	}
	
	return FALSE;
}

static void rtl_linkSourceEntry(struct rtl_groupEntry *groupEntry,  struct rtl_sourceEntry* entryNode)
{
	if((NULL==entryNode) || (NULL==groupEntry))
	{
		return;
	}
	else
	{
		rtl_glueMutexLock();  /* lock resource*/	

		if(groupEntry->sourceList!=NULL)
		{
			groupEntry->sourceList->previous=entryNode;
		}
		entryNode->next=groupEntry->sourceList;
		groupEntry->sourceList=entryNode;
		groupEntry->sourceList->previous=NULL;
		
		rtl_glueMutexUnlock();  /* lock resource*/	
	}
}

static void rtl_unlinkSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* sourceEntry)
{

	if((NULL==sourceEntry) ||(NULL==groupEntry))
	{
		return;
	}
	else
	{
		rtl_glueMutexLock();  /* lock resource*/	
		/* unlink entry node*/ 
		if(sourceEntry==groupEntry->sourceList) /*unlink group list head*/
		{
	
			groupEntry->sourceList=sourceEntry->next;
			if(groupEntry->sourceList!=NULL)
			{
				groupEntry->sourceList ->previous=NULL;
			}
			
			sourceEntry->previous=NULL;
			sourceEntry->next=NULL;
		}
		else
		{	
			if(sourceEntry->previous!=NULL)
			{
				sourceEntry->previous->next=sourceEntry->next;
			}

			if(sourceEntry->next!=NULL)
			{
				sourceEntry->next->previous=sourceEntry->previous;
			}
			sourceEntry->previous=NULL;
			sourceEntry->next=NULL;
		}
		
		rtl_glueMutexUnlock();//UnLock resource
	}
	

}

static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr)
{
	rtl_glueMutexLock();
	if (NULL!=sourceEntryPtr)
	{
		memset(sourceEntryPtr, 0, sizeof(struct rtl_sourceEntry));
	}
	rtl_glueMutexUnlock();
}


static void rtl_deleteSourceEntry(struct rtl_groupEntry *groupEntry, struct rtl_sourceEntry* sourceEntry)
{
	if(sourceEntry!=NULL)
	{
		rtl_unlinkSourceEntry(groupEntry,sourceEntry);
		rtl_clearSourceEntry(sourceEntry);
		rtl_freeSourceEntry(sourceEntry);
	}
}
	
#if 0	
static int32 rtl_checkPortMask(uint8 pktPortMask)
{
	int32 i=0;
	uint8 portMaskn=PORT0_MASK;
	uint8 count=0;
	for(i=0; i<MAX_SUPPORT_PORT_NUMBER; i++)
	{
		if(portMaskn&pktPortMask)
		{
			count++;
		}
		portMaskn=portMaskn<<1;
	}
	
	if(count==1)
	{
		return SUCCESS;
	}
	else
	{
		return FAILED;
	}
}

static uint32 rtl_mapPortMaskToPortNum(uint8 portMask)
{	
	int i;
	for(i=0; i<MAX_SUPPORT_PORT_NUMBER; i++)
	{
		if((portMask&(1<<i))!=0)
		{
			return i;
		}
	}

	return 0xFFFFFFFF;
}


static int32 rtl_mapMCastIPToMAC(uint32 ipVersion, uint32 *ipAddr, uint8 *macAddr )
{
	if(ipVersion==IP_VERSION6)
	{
		if(IS_IPV6_MULTICAST_ADDRESS(ipAddr))
		{
			macAddr[0]=0x33;
			macAddr[1]=0x33;
			macAddr[2]=(ipAddr[3]&0xff000000)>>24;
			macAddr[3]=(ipAddr[3]&0x00ff0000)>>16;
			macAddr[4]=(ipAddr[3]&0x0000ff00)>>8;
			macAddr[5]= ipAddr[3]&0x000000ff;
			return SUCCESS;
		}
		else
		{
			return FAILED;
		}
	}

	if(ipVersion==IP_VERSION4)
	{
		if(IS_IPV4_MULTICAST_ADDRESS(ipAddr))
		{
			macAddr[0]=0x01;
			macAddr[1]=0x00;
			macAddr[2]=0x5e;
			macAddr[3]=(ipAddr[0]&0x007f0000)>>16;
			macAddr[4]=(ipAddr[0]&0x0000ff00)>>8;
			macAddr[5]= ipAddr[0]&0x000000ff;
			return SUCCESS;
		}
		else
		{
			return FAILED;
		}
	}

	return FAILED;
}
#endif

static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr)
{
	if(ipVersion==IP_VERSION4)
	{
		if(macAddr[0]!=0x01)
		{
			return FALSE;
		}

		if((macAddr[3]&0x7f)!=(uint8)((ipAddr[0]&0x007f0000)>>16))
		{
			return FALSE;
		}
		
		if(macAddr[4]!=(uint8)((ipAddr[0]&0x0000ff00)>>8))
		{
			return FALSE;
		}

		if(macAddr[5]!=(uint8)(ipAddr[0]&0x000000ff))
		{
			return FALSE;
		}

		return TRUE;
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	if(ipVersion==IP_VERSION6)
	{
		if(macAddr[0]!=0x33)
		{
			return FALSE;
		}

		if(macAddr[1]!=0x33)
		{
			return FALSE;
		}

		if(macAddr[2]!=(uint8)((ipAddr[3]&0xff000000)>>24))
		{
			return FALSE;
		}
		
		if(macAddr[3]!=(uint8)((ipAddr[3]&0x00ff0000)>>16))
		{
			return FALSE;
		}

		if(macAddr[4]!=(uint8)((ipAddr[3]&0x0000ff00)>>8))
		{
			return FALSE;
		}
		
		if(macAddr[5]!=(uint8)(ipAddr[3]&0x000000ff))
		{
			return FALSE;
		}
		
		return TRUE;
	}
#endif
	return FALSE;
}

#ifdef CONFIG_RTL_MLD_SNOOPING	
static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2)
{
	int i;
	for(i=0; i<4; i++)
	{
		if(ipv6Addr1[i]!=ipv6Addr2[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}
#endif

static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2)
{
	int i;
	for(i=0; i<6; i++)
	{
		if(macAddr1[i]!=macAddr2[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen)
{
	/*note: the first bytes of  packetBuf should be two bytes aligned*/
	uint32  checksum=0;
	uint32 count=packetLen;
	uint16   *ptr= (uint16 *) (packetBuf);	
	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(packetBuf+packetLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);

}

#ifdef CONFIG_RTL_MLD_SNOOPING
static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr)
{
	uint32  checksum=0;
	uint32 count=pktLen;
	uint16   *ptr;

	/*compute ipv6 pseudo-header checksum*/
	ptr= (uint16 *) (ipv6PseudoHdr);	
	for(count=0; count<20; count++) /*the pseudo header is 40 bytes long*/
	{
		  checksum+= ntohs(*ptr);
		  ptr++;
	}
	
	/*compute the checksum of mld buffer*/
	 count=pktLen;
	 ptr=(uint16 *) (pktBuf);	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);
	
}
#endif

static uint32 rtl_getGroupFwdPortMask(struct rtl_groupEntry * groupEntry, uint32 enableSourceList, uint32 sysTime)
{
	int i;
	uint32 portMaskn=PORT0_MASK;
	uint32 fwdPortMask=0;
	struct rtl_sourceEntry * sourcePtr=NULL;;
	
	for(i=0; i<MAX_SUPPORT_PORT_NUMBER; i++)
	{
		if(enableSourceList==TRUE)
		{
			if(groupEntry->groupFilterTimer[i]>sysTime) /*exclude mode never expired*/
			{
				fwdPortMask|=portMaskn;
			}
			else/*include mode*/
			{
				sourcePtr=groupEntry->sourceList;
				while(sourcePtr!=NULL)
				{
					if(sourcePtr->portTimer[i]>sysTime)
					{
						fwdPortMask|=portMaskn;
						break;
					}
				 	else
					{
						sourcePtr=sourcePtr->next;
				 	}
				
				}
				
			}
		}
		else
		{
			if(groupEntry->groupFilterTimer[i]>sysTime) /*exclude mode never expired*/
			{
				fwdPortMask|=portMaskn;
			}
		}
		portMaskn=portMaskn<<1;
	}

	
	return fwdPortMask;
}



static void rtl_checkSourceTimer(struct rtl_groupEntry * groupEntry , struct rtl_sourceEntry * sourceEntry)
{
	int i=0;

	uint32 portMaskn=PORT0_MASK;
	uint32 deletePortMask=0;
	for(i=0; i<MAX_SUPPORT_PORT_NUMBER; i++)
	{	
	
		if(sourceEntry->portTimer[i]==0)/*means not exist*/
		{
			deletePortMask|=portMaskn;
		}
		else
		{
			if(sourceEntry->portTimer[i]<=rtl_sysUpSeconds) /*means time out*/
			{
				if(groupEntry->groupFilterTimer[i]<=rtl_sysUpSeconds) /*means include mode*/
				{
					sourceEntry->portTimer[i]=0;
					deletePortMask|=portMaskn;
				}
				
			}
		}
		
		portMaskn=portMaskn<<1;
	}

	if(deletePortMask==((1<<MAX_SUPPORT_PORT_NUMBER)-1)) /*means all port  are INCLUDE mode and expired*/
	{
		rtl_deleteSourceEntry(groupEntry,sourceEntry);
	}

	
}

static uint32 rtl_getSourceFwdPortMask(struct rtl_groupEntry * groupEntry,uint32 *sourceAddr, uint32 sysTime)
{
	int i;
	uint32 portMaskn=PORT0_MASK;
	uint32 fwdPortMask=0;
	struct rtl_sourceEntry * sourceEntry=NULL;
	if(groupEntry==NULL)
	{
		return 0xFFFFFFFF; /*broadcast*/
	}
	else
	{
		sourceEntry=rtl_searchSourceEntry((uint32)(groupEntry->ipVersion),sourceAddr, groupEntry);
		for(i=0; i<MAX_SUPPORT_PORT_NUMBER; i++)
		{
			
			if(groupEntry->groupFilterTimer[i]<=sysTime)	/*include mode*/
			{	
				if(sourceEntry!=NULL)
				{
					if( sourceEntry->portTimer[i]>sysTime)
					{
						fwdPortMask|=portMaskn;
					}
				}
			}
			else/*exclude mode*/
			{	
				if(sourceEntry==NULL)
				{
					fwdPortMask|=portMaskn;
				}
				else
				{
					if((sourceEntry->portTimer[i]>sysTime) || (sourceEntry->portTimer[i]==0))
					{
						fwdPortMask|=portMaskn;
					}
				}
			}
			
			portMaskn=portMaskn<<1;
		}
		return fwdPortMask;
		
	}
}




static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry, uint32 enableSourceList, struct rtl_groupEntry ** hashTable)
{
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	uint32 oldFwdPortMask=0;
	uint32 tmpFwdPortMask=0;
	#endif
	
	uint32 newFwdPortMask=0;
	struct rtl_sourceEntry *sourceEntry=groupEntry->sourceList;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	oldFwdPortMask=rtl_getGroupFwdPortMask(groupEntry, enableSourceList, rtl_sysUpSeconds?(rtl_sysUpSeconds-1):0);
	#endif
	
	while(sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		timerEventContext.sourceAddr[0]=sourceEntry->sourceAddr[0];
		#endif
		rtl_checkSourceTimer(groupEntry, sourceEntry);
		sourceEntry=nextSourceEntry;
	}
	
	newFwdPortMask=rtl_getGroupFwdPortMask(groupEntry, enableSourceList, rtl_sysUpSeconds);


	if(newFwdPortMask==0) /*none active port*/
	{
		rtl_deleteGroupEntry(groupEntry,hashTable);	
	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if((oldFwdPortMask!=newFwdPortMask) ||(newFwdPortMask==0))
	{	
		timerEventContext.sourceAddr[0]=0;

		tmpFwdPortMask=oldFwdPortMask&(~newFwdPortMask);
		if(tmpFwdPortMask==0)
		{
			tmpFwdPortMask=newFwdPortMask & (~oldFwdPortMask);
		}
		
#ifdef CONFIG_PROC_FS
		rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
		timerEventContext.portMask=(uint32)tmpFwdPortMask;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &timerEventContext);
		
	}
	#endif

}



static int32 rtl_initHashTable(uint32 moduleIndex, uint32 hashTableSize)
{
	uint32 i=0;
	
	/* Allocate memory */
	rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable=NULL;
	rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable = (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);
	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!= NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	
			rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i]=NULL;
		}

#ifndef CONFIG_RTL_MLD_SNOOPING	
		return SUCCESS;
#endif

	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		
#ifndef CONFIG_RTL_MLD_SNOOPING	
		return FAILED;
#endif

	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING	
	rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=NULL;
	rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=  (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);
	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	

			rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i]=NULL;
		}
		return SUCCESS;
	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}

		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		
		return FAILED;

	}
#endif
	

}



/**************************
	Utility
**************************/
static void  rtl_parseMacFrame(uint32 moduleIndex, uint8* macFrame, uint32 verifyCheckSum, struct rtl_macFrameInfo* macInfo) 
{
	
//MAC Frame :DA(6 bytes)+SA(6 bytes)+ CPU tag(4 bytes) + VlAN tag(Optional, 4 bytes)
//                   +Type(IPv4:0x0800, IPV6:0x86DD, PPPOE:0x8864, 2 bytes )+Data(46~1500 bytes)+CRC(4 bytes)
//CPU tag: Realtek Ethertype==0x8899(2 bytes)+protocol==0x9(4 MSB)+priority(2 bits)+reserved(4 bits)+portmask(6 LSB)
	uint8 *ptr=macFrame;

#ifdef CONFIG_RTL_MLD_SNOOPING	
	int i=0;
	uint8 nextHeader=0;
	uint16 extensionHdrLen=0;
	uint8 routerhead=FALSE;
	uint8 needchecksum=FALSE;
	
	uint8 optionDataLen=0;
	uint8 optionType=0;
	uint32 ipv6RAO=0;
#endif

	uint32 ipAddr[4]={0,0,0,0};
	union pseudoHeader pHeader;
	
	memset(macInfo,0,sizeof(struct rtl_macFrameInfo));
	memset(&pHeader, 0, sizeof(union pseudoHeader));

	ptr=ptr+12;


	/*check the presence of VLAN tag*/	
	if(*(int16 *)(ptr)==(int16)htons(VLAN_PROTOCOL_ID))
	{
		ptr=ptr+4;
	}

	/*ignore packet with PPPOE header*/	
	if(*(int16 *)(ptr)==(int16)htons(PPPOE_ETHER_TYPE))
	{
		return;	
	}

	
	/*check the presence of ipv4 type*/
	if(*(int16 *)(ptr)==(int16)htons(IPV4_ETHER_TYPE))
	{
		ptr=ptr+2;
		macInfo->ipBuf=ptr;
		macInfo->ipVersion=IP_VERSION4;
	}
	else
	{
		/*check the presence of ipv4 type*/
		if(*(int16 *)(ptr)==(int16)htons(IPV6_ETHER_TYPE))
		{
			ptr=ptr+2;
			macInfo->ipBuf=ptr;
			macInfo->ipVersion=IP_VERSION6;
		}
	}

	if((macInfo->ipVersion!=IP_VERSION4) && (macInfo->ipVersion!=IP_VERSION6))
	{
		return;
	}
	macInfo->checksumFlag=FAILED;
	
	if(macInfo->ipVersion==IP_VERSION4)
	{
		macInfo->ipHdrLen=(uint16)((((struct ipv4Pkt *)(macInfo->ipBuf))->vhl&0x0f)<<2);
		macInfo->l3PktLen=ntohs(((struct ipv4Pkt *)(macInfo->ipBuf))->length)-macInfo->ipHdrLen;
		ptr=ptr+macInfo->ipHdrLen;
		macInfo->l3PktBuf=ptr;
		macInfo->macFrameLen=(uint16)((ptr-macFrame)+macInfo->l3PktLen);
	
/*distinguish different IGMP packet:
                                                    ip_header_length      destination_ip      igmp_packet_length   igmp_type   group_address         	
IGMPv1_general_query:                            20                   224.0.0.1                       8                    0x11                 0
IGMPv2_general_query:                            24                   224.0.0.1                       8                    0x11                 0                     
IGMPv2_group_specific_query:			24                   224.0.0.1                       8                    0x11               !=0  
IGMPv3 _query:                                        24                   224.0.0.1                   >=12                  0x11        according_to_different_situation 

IGMPv1_join:                                            20          actual_multicast_address         8                    0x12           actual_multicast_address
IGMPv2_join:                                            24          actual_multicast_address         8                    0x16           actual_multicast_address
IGMPv2_leave:                                          24          actual_multicast_address         8                    0x17           actual_multicast_address
IGMPv3_report:                                         24          actual_multicast_address       >=12                0x22           actual_multicast_address*/

	/* parse IGMP type and version*/	
		if(((struct ipv4Pkt *)(macInfo->ipBuf))->protocol==IGMP_PROTOCOL)
		{	
			/*check DVMRP*/
			if((macInfo->l3PktBuf[0]==DVMRP_TYPE) && (((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(DVMRP_ADDR)) )
			{
				macInfo->l3Protocol=DVMRP_PROTOCOL;
			}
			else
			{
				/*means unicast*/
				if((macFrame[0]&0x01)==0)
				{	
						if(rtl_compareMacAddr(macFrame, rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac)==TRUE) 
				       	{
								if(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr))
								{
									macInfo->l3Protocol=IGMP_PROTOCOL;
               							goto otherpro;
								}
						}
			      									
				}
				else /*means multicast*/
				{	
					ipAddr[0]=ntohl(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp);
					if(rtl_checkMCastAddrMapping(IP_VERSION4,ipAddr,macFrame)==TRUE)
					{
						macInfo->l3Protocol=IGMP_PROTOCOL;
					}
					else
					{
						return;
					}
				}
			}
			
		}

otherpro:	
			if(((struct ipv4Pkt *)(macInfo->ipBuf))->protocol==MOSPF_PROTOCOL &&\
			((((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(IPV4_MOSPF_ADDR1)) ||\
			(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(IPV4_MOSPF_ADDR2))))
		{
			macInfo->l3Protocol=MOSPF_PROTOCOL;
		}

		if(((struct ipv4Pkt *)(macInfo->ipBuf))->protocol==PIM_PROTOCOL && (((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp==htonl(IPV4_PIM_ADDR)))
		{
			macInfo->l3Protocol=PIM_PROTOCOL;
		}

		if(verifyCheckSum==TRUE)
		{
			if(rtl_checksum(macInfo->l3PktBuf, macInfo->l3PktLen)!=0)
			{
				macInfo->checksumFlag=FAILED;
			}
			else
			{
				macInfo->checksumFlag=SUCCESS;
			}
		}
		else
		{
			macInfo->checksumFlag=SUCCESS;
		}
	}

#ifdef CONFIG_RTL_MLD_SNOOPING
	if(macInfo->ipVersion==IP_VERSION6)
	{
		macInfo->macFrameLen=(uint16)(ptr-macFrame+IPV6_HEADER_LENGTH+ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth));
		macInfo->ipHdrLen=IPV6_HEADER_LENGTH;
		
		nextHeader=((struct ipv6Pkt *)(macInfo->ipBuf))->nextHeader;
		ptr=ptr+IPV6_HEADER_LENGTH;
		while((ptr-macInfo->ipBuf)<(ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)+IPV6_HEADER_LENGTH))
		{
			switch(nextHeader) 
			{
				case HOP_BY_HOP_OPTIONS_HEADER:
					/*parse hop-by-hop option*/
					nextHeader=ptr[0];
					extensionHdrLen=((uint16)(ptr[1])+1)*8;
					ptr=ptr+2;
					
					while((ptr-macInfo->ipBuf-40)<extensionHdrLen)
					{
						optionType=ptr[0];
						/*pad1 option*/
						if(optionType==0)
						{
							ptr=ptr+1;
							continue;
						}

						/*padN option*/
						if(optionType==1)
						{
							optionDataLen=ptr[1];
							ptr=ptr+optionDataLen+2;
							continue;
						}

						/*router alter option*/
						if(ntohl(*(uint32 *)(ptr))==IPV6_ROUTER_ALTER_OPTION)
						{
							ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
							ptr=ptr+4;	
							continue;
						}

						/*other TLV option*/
						if((optionType!=0) && (optionType!=1))
						{
							optionDataLen=ptr[1];
							ptr=ptr+optionDataLen+2;
							continue;
						}
					

					}
					/*
					if((ptr-macInfo->ipBuf-40)!=extensionHdrLen)
					{
						rtl_gluePrintf("ipv6 packet parse error\n");
					}*/
					
				break;
				
				case ROUTING_HEADER:
					nextHeader=ptr[0];
					extensionHdrLen=((uint16)(ptr[1])+1)*8;
					
                                  
					 if (ptr[3]>0)
				   	{
                                          ptr=ptr+extensionHdrLen;
						for(i=0; i<4; i++)
						{
						      pHeader.ipv6_pHdr.destinationAddr[i]=*((uint32 *)(ptr)-4+i);

						}
					     routerhead=TRUE;
					     
					     
					}
					else
					{
                                          ptr=ptr+extensionHdrLen;
					}
					
					
					
				break;
				
				case FRAGMENT_HEADER:
					nextHeader=ptr[0];
					ptr=ptr+8;
				break;
				
				case DESTINATION_OPTION_HEADER:
					nextHeader=ptr[0];
					extensionHdrLen=((uint16)(ptr[1])+1)*8;
					ptr=ptr+extensionHdrLen;
				break;
				
				case ICMP_PROTOCOL:
					nextHeader=NO_NEXT_HEADER;
					macInfo->l3PktLen=ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)-(uint16)(ptr-macInfo->ipBuf-IPV6_HEADER_LENGTH);
					macInfo->l3PktBuf=ptr;
					if((ptr[0]==MLD_QUERY) ||(ptr[0]==MLDV1_REPORT) ||(ptr[0]==MLDV1_DONE) ||(ptr[0]==MLDV2_REPORT))
					{
						/*means multicast*/
						if(	(macFrame[0]==0x33)&&\
							(macFrame[1]==0x33))
						{
							ipAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
							ipAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
							ipAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
							ipAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
							
							if(rtl_checkMCastAddrMapping(IP_VERSION6, ipAddr, macFrame)==TRUE)
							{
								macInfo->l3Protocol=ICMP_PROTOCOL;
							}
							
						}
						else /*means multicast*/
						{	
							
								ipAddr[0]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[0]);
								ipAddr[1]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[1]);
								ipAddr[2]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[2]);
								ipAddr[3]=htonl(rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[3]);
								if(	(rtl_compareMacAddr(macFrame, rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac)==TRUE) &&\
								(rtl_compareIpv6Addr(((struct ipv6Pkt *)macInfo->ipBuf)->destinationAddr, ipAddr) == TRUE))
								{
									macInfo->l3Protocol=ICMP_PROTOCOL;
								}		

							
						}

						needchecksum=TRUE;
						/*
						if(ipv6RAO!=IPV6_ROUTER_ALTER_OPTION)
						{
							rtl_gluePrintf("router alter option error\n");
						}*/
					}
				
					
				break;
				
				case PIM_PROTOCOL:
					nextHeader=NO_NEXT_HEADER;
					macInfo->l3PktLen=ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)-(uint16)(ptr-macInfo->ipBuf-IPV6_HEADER_LENGTH);
					macInfo->l3PktBuf=ptr;
					
					ipAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
					ipAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
					ipAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
					ipAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
					if(IS_IPV6_PIM_ADDR(ipAddr))
					{
						macInfo->l3Protocol=PIM_PROTOCOL;
					}
					needchecksum=TRUE;
				
				break;
				
				case MOSPF_PROTOCOL:
					nextHeader=NO_NEXT_HEADER;
					macInfo->l3PktLen=ntohs(((struct ipv6Pkt *)(macInfo->ipBuf))->payloadLenth)-(uint16)(ptr-macInfo->ipBuf-IPV6_HEADER_LENGTH);
					macInfo->l3PktBuf=ptr;
					
					ipAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
					ipAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
					ipAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
					ipAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
					
					if(IS_IPV6_MOSPF_ADDR1(ipAddr) || IS_IPV6_MOSPF_ADDR2(ipAddr))
					{
						macInfo->l3Protocol=MOSPF_PROTOCOL;
					}
					needchecksum=TRUE;
				break;
				
				default:		
					goto out;
				break;
			}
		
		}

out:
		/*compute pseudo header*/
             if(needchecksum==TRUE)
             	{
		       for(i=0; i<4; i++)
		      {
			     pHeader.ipv6_pHdr.sourceAddr[i]=((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[i];
			     
		       }
           
		      if(routerhead==FALSE)
		      {
		             for(i=0;i<4;i++)
		      	      {
			            pHeader.ipv6_pHdr.destinationAddr[i]=((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[i];
		      	      }
		       }
	      
		
		       pHeader.ipv6_pHdr.nextHeader=macInfo->l3Protocol;
		       pHeader.ipv6_pHdr.upperLayerPacketLength=htonl((uint32)(macInfo->l3PktLen));
		       pHeader.ipv6_pHdr.zeroData[0]=0;
		       pHeader.ipv6_pHdr.zeroData[1]=0;
		       pHeader.ipv6_pHdr.zeroData[2]=0;
		
		       if(macInfo->l3PktBuf!=NULL)
		       {
		       	if(verifyCheckSum==TRUE)
		       	{
				      if(rtl_ipv6L3Checksum(macInfo->l3PktBuf, macInfo->l3PktLen,&pHeader)!=0)
				      {
					      macInfo->checksumFlag=FAILED;
				       }
				       else
				       {
					      macInfo->checksumFlag=SUCCESS;
				       }
		       	}
				else
				{
					macInfo->checksumFlag=FAILED;
				}
		       }
             	}
	}
#endif
	return;
}


static uint32  rtl_getMulticastRouterPortMask(uint32 moduleIndex, uint32 ipVersion, uint32 sysTime)
{
	uint32 portIndex=0;
	uint32 portMaskn=PORT0_MASK;
	uint32 routerPortmask=0;
	
	if(ipVersion==IP_VERSION4)
	{
		for(portIndex=0; portIndex<MAX_SUPPORT_PORT_NUMBER; portIndex++)
		{
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.querier.portTimer[portIndex]>sysTime)
			{
				routerPortmask=routerPortmask|portMaskn;
			}
			
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	

			
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.mospfRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}		


			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.pimRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	
		
			portMaskn=portMaskn<<1;  /*shift to next port mask*/
			
		}
	
	}

#ifdef CONFIG_RTL_MLD_SNOOPING
	if(ipVersion==IP_VERSION6)
	{
		for(portIndex=0; portIndex<MAX_SUPPORT_PORT_NUMBER; portIndex++)
		{
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.querier.portTimer[portIndex]>sysTime)
			{	

				routerPortmask=routerPortmask|portMaskn;
			}		

			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.mospfRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	
			
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.pimRouter.portTimer[portIndex]>sysTime)
			{	
				routerPortmask=routerPortmask|portMaskn;
			}	
			
			portMaskn=portMaskn<<1;  /*shift to next port mask*/
			
		}

	}
#endif	

	routerPortmask= routerPortmask |rtl_mCastModuleArray[moduleIndex].staticRouterPortMask;

	return routerPortmask;
}

static uint32 rtl_processQueries(uint32 moduleIndex,uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	struct rtl_groupEntry *groupEntry=NULL;
	struct rtl_sourceEntry*sourceEntry=NULL;
	uint32 timerIndex=0;
	uint32 hashIndex=0; 
	uint32 groupAddress[4]={0,0,0,0};
	uint32 suppressFlag=0;
	uint32 *sourceAddr=NULL;
	uint32 numOfSrc=0;
	uint32 i=0;
	
	/*querier timer update and election process*/
	rtl_snoopQuerier(moduleIndex, ipVersion, portNum);

	if(ipVersion==IP_VERSION4)
	{	
		if(pktLen>=12) /*means igmpv3 query*/
		{
			groupAddress[0]=ntohl(((struct igmpv3Query*)pktBuf)->groupAddr);
			suppressFlag=((struct igmpv3Query*)pktBuf)->rsq & S_FLAG_MASK;
			sourceAddr=&(((struct igmpv3Query*)pktBuf)->srcList);
			numOfSrc=(uint32)ntohs(((struct igmpv3Query*)pktBuf)->numOfSrc);

		}
		else
		{
			groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
		}
		
		if(groupAddress[0]==0) /*means general query*/
		{
			goto out;
		}
		else
		{
			hashIndex=groupAddress[0]&rtl_hashMask;
		}
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	if(ipVersion==IP_VERSION6)
	{
		if(pktLen>=28) /*means mldv2 query*/
		{
			groupAddress[0]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[0]);
			groupAddress[1]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[1]);
			groupAddress[2]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[2]);
			groupAddress[3]=ntohl(((struct mldv2Query*)pktBuf)->mCastAddr[3]);

			suppressFlag=((struct mldv2Query*)pktBuf)->rsq & S_FLAG_MASK;
			sourceAddr=&(((struct mldv2Query*)pktBuf)->srcList);
			numOfSrc=(uint32)ntohs(((struct mldv2Query*)pktBuf)->numOfSrc);

		}
		else /*means mldv1 query*/
		{
			groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
			groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
			groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
			groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);
			
		}
		
		if(	(groupAddress[0]==0) &&
			(groupAddress[1]==0) &&
			(groupAddress[2]==0) &&
			(groupAddress[3]==0) )/*means general query*/
		{
			goto out;
		}
		else
		{
			hashIndex=groupAddress[3]&rtl_hashMask;
		}
	}
#endif	
	if(suppressFlag==0)
	{
	
		groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
		if((groupEntry!=NULL))
		{	
			if(numOfSrc==0) /*means group specific query*/
			{
				for(timerIndex=0; timerIndex<MAX_SUPPORT_PORT_NUMBER; timerIndex++)
				{
					if(groupEntry->groupFilterTimer[timerIndex]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
					{
						groupEntry->groupFilterTimer[timerIndex]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}	
			
				}
			}
			else /*means group and source specific query*/
			{
				if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
				{
					for(i=0; i<numOfSrc; i++)
					{	
						
						sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr, groupEntry);
						
						if(sourceEntry!=NULL)
						{
							for(timerIndex=0; timerIndex<MAX_SUPPORT_PORT_NUMBER; timerIndex++)
							{
								if(sourceEntry->portTimer[timerIndex]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
								{
									sourceEntry->portTimer[timerIndex]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
								}
							}
						}

						if(ipVersion==IP_VERSION4)
						{
							sourceAddr++;
						}
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{
							sourceAddr=sourceAddr+4;
						}
#endif
					}
				}
			}
		}
		
		
	}
	
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

out:	
	return (~(1<<portNum) & ((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}


static void rtl_snoopQuerier(uint32 moduleIndex, uint32 ipVersion, uint32 portNum)
{
	
	if(ipVersion==IP_VERSION4)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.querier.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.querierPresentInterval;/*update timer value*/
	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING	
	if(ipVersion==IP_VERSION6)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.querier.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.querierPresentInterval;/*update timer value*/
	}
#endif	
	return;
}


/*Process Report Packet*/
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf)
{
	
	
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;

	uint32 hashIndex=0;

	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);

	if(ipVersion==IP_VERSION4)
	{
		if(pktBuf[0]==0x12)
		{ 
			groupAddress[0]=ntohl(((struct igmpv1Pkt *)pktBuf)->groupAddr);
		}

		if(pktBuf[0]==0x16)
		{
			groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
		}
		
		hashIndex=groupAddress[0]&rtl_hashMask;

	}


#ifdef CONFIG_RTL_MLD_SNOOPING
	if(ipVersion==IP_VERSION6)
	{
		
		groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);

		hashIndex=groupAddress[3]&rtl_hashMask;
	}
#endif	

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			goto out;
		}
		else
		{
			/*set new multicast entry*/	
			#ifdef CONFIG_RTL_MLD_SNOOPING
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
			#else
			newGroupEntry->groupAddr[0]=groupAddress[0];
			#endif
			
			newGroupEntry->sourceList=NULL;

			newGroupEntry->ipVersion=ipVersion;

			newGroupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			
			/*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
			if(ipVersion==IP_VERSION4)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable, hashIndex);
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			if(ipVersion==IP_VERSION6)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable, hashIndex);
			}
#endif			

		}
		
	}
	else
	{  

		sourceEntry=groupEntry->sourceList;
		/*delete all the source list*/
		while(sourceEntry!=NULL)
		{
			sourceEntry->portTimer[portNum]=0;
			sourceEntry=sourceEntry->next;
		}
		
		groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
	
	}

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
#endif

out:
	return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}

static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf)
{
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;

	uint32 hashIndex=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
		hashIndex=groupAddress[0]&rtl_hashMask;
	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING
	if(ipVersion==IP_VERSION6)
	{
		groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);

		hashIndex=groupAddress[3]&rtl_hashMask;
	}
#endif	

	      groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	      /*lower the timer of the group*/
	      if(groupEntry!=NULL)
	      {   

		     if(groupEntry->groupFilterTimer[portNum]>rtl_sysUpSeconds)
		     {
                  
				if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				{
					groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds;
				}
				else
				{
					if(groupEntry->groupFilterTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
					{
						groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
				}
				
                   }

		     sourceEntry=groupEntry->sourceList;
		     while(sourceEntry)
		     {
				if(sourceEntry->portTimer[portNum]>rtl_sysUpSeconds)
				{
					if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
			        	{
						sourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
				  	}
					else
					{
						if(sourceEntry->portTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
						}
					}
					
				}

				sourceEntry=sourceEntry->next;
		      }

		
	      }	
	
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif
	
	return (multicastRouterPortMask&(~(1<<portNum))&0x3f);
}

static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf)
{

	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;

	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;


	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[0]&rtl_hashMask;
	}

#ifdef CONFIG_RTL_MLD_SNOOPING
	if(ipVersion==IP_VERSION6)
	{
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[3]&rtl_hashMask;
	
	}
#endif	

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}
		else
		{	
			/*set new multicast entry*/
#ifdef CONFIG_RTL_MLD_SNOOPING
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
#else
			newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

			newGroupEntry->ipVersion=ipVersion;

			newGroupEntry->sourceList=NULL;
			
			/*end of set group entry*/
			
			/*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
			if(ipVersion==IP_VERSION4)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable, hashIndex);
			}
			
#ifdef CONFIG_RTL_MLD_SNOOPING
			if(ipVersion==IP_VERSION6)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable, hashIndex);
			}
#endif
			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				newGroupEntry->groupFilterTimer[portNum]=0;
				/*link the new source list*/
				for(j=0; j<numOfSrc; j++)
				{

					
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
						}
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
						rtl_linkSourceEntry(newGroupEntry,newSourceEntry);
					}

					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
				}
			
			}
			else
			{	
				newGroupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
		
			
		}

		
	}
	else /*means it can be found in the forward hash table*/
	{  

		if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
		{
			/*here to handle the source list*/
			for(j=0; j<numOfSrc; j++)
			{
				
				sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
				if(sourceEntry==NULL)
				{
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
						
						}
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif						
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
						rtl_linkSourceEntry(groupEntry,newSourceEntry);
					}

				}
				else
				{		
					/*just update source timer*/
					sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				}
					
				if(ipVersion==IP_VERSION4)
				{	
					sourceAddr++;
				}
#ifdef CONFIG_RTL_MLD_SNOOPING
				if(ipVersion==IP_VERSION6)
				{
					sourceAddr=sourceAddr+4;
				}
#endif				
			}

		}
		else
		{
			groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
		}
		
	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return SUCCESS;


}

static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceArray=NULL;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct groupRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[0]&rtl_hashMask;
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	if(ipVersion==IP_VERSION6)
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[3]&rtl_hashMask;
	
	}
#endif
	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}
		else
		{	
			/*set new multicast entry*/		
#ifdef CONFIG_RTL_MLD_SNOOPING	
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
#else
			newGroupEntry->groupAddr[0]=groupAddress[0];
#endif


			newGroupEntry->ipVersion=ipVersion;

			newGroupEntry->sourceList=NULL;
			/*means the filter mode is exclude*/
			newGroupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;

			/*end of set group entry*/
			
			/*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
			if(ipVersion==IP_VERSION4)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable, hashIndex);
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			if(ipVersion==IP_VERSION6)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable, hashIndex);
			}
#endif
			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				/*link the new source list*/
				for(j=0; j<numOfSrc; j++)
				{
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
						}
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif						
						/*time out the sources included in the MODE_IS_EXCLUDE report*/
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
						rtl_linkSourceEntry(newGroupEntry,newSourceEntry);
					}

					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif								
				}

			}
				
		}

	}
	else /*means group address can be found in the forward hash table*/
	{  

		if(groupEntry->groupFilterTimer[portNum]<=rtl_sysUpSeconds) /*means include mode*/
		{
			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				/*here to handle the source list*/

				/*delete (A-B)*/
				sourceEntry=groupEntry->sourceList;
				while(sourceEntry)
				{
					if(rtl_searchSourceAddr(ipVersion,sourceEntry->sourceAddr,sourceArray, numOfSrc)== FALSE)
					{
						sourceEntry->portTimer[portNum]=0;
					}
					sourceEntry=sourceEntry->next;
				}

				/*(B-A) time out*/
				for(j=0; j<numOfSrc; j++)
				{
					
					sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
					
					if(sourceEntry==NULL)
					{
						newSourceEntry=rtl_allocateSourceEntry();
						if(newSourceEntry==NULL)
						{
							rtl_gluePrintf("run out of source entry!\n");
							return FAILED;
						}
						else
						{	
						
							if(ipVersion==IP_VERSION4)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
							
							}
#ifdef CONFIG_RTL_MLD_SNOOPING
							if(ipVersion==IP_VERSION6)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
								newSourceEntry->sourceAddr[1]=sourceAddr[1];
								newSourceEntry->sourceAddr[2]=sourceAddr[2];
								newSourceEntry->sourceAddr[3]=sourceAddr[3];
							}
#endif							
							newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
							rtl_linkSourceEntry(groupEntry,newSourceEntry);
						}

					}
					else
					{
						if(sourceEntry->portTimer[portNum]==0)
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
						}
					}

				
					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
				}
			}
			
		}
		else/*means exclude mode*/
		{
			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				/*here to handle the source list*/
				/*delete (X-A) and delete (Y-A)*/
				sourceEntry=groupEntry->sourceList;
				while(sourceEntry)
				{
					if(rtl_searchSourceAddr(ipVersion,sourceEntry->sourceAddr,sourceArray, numOfSrc)== FALSE)
					{
						sourceEntry->portTimer[portNum]=0;
					}
					sourceEntry=sourceEntry->next;
				}

				/*A-X-Y=GMI*/
				for(j=0; j<numOfSrc; j++)
				{
					sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
					
					if(sourceEntry==NULL)
					{
						newSourceEntry=rtl_allocateSourceEntry();
						if(newSourceEntry==NULL)
						{
							rtl_gluePrintf("run out of source entry!\n");
							return FAILED;
						}
						else
						{	
						
							if(ipVersion==IP_VERSION4)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
							
							}
#ifdef CONFIG_RTL_MLD_SNOOPING
							if(ipVersion==IP_VERSION6)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
								newSourceEntry->sourceAddr[1]=sourceAddr[1];
								newSourceEntry->sourceAddr[2]=sourceAddr[2];
								newSourceEntry->sourceAddr[3]=sourceAddr[3];
							}
#endif							
						
							newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
							rtl_linkSourceEntry(groupEntry,newSourceEntry);
						}

					}
					else
					{
						if(sourceEntry->portTimer[portNum]==0)
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
						}
					}
					
					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
				}
			}
		
			
		}

		groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return SUCCESS;

}

static int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[0]&rtl_hashMask;
	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING		
	if(ipVersion==IP_VERSION6)
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[3]&rtl_hashMask;
	
	}
#endif

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}
		else
		{	
			/*set new multicast entry*/
	#ifdef CONFIG_RTL_MLD_SNOOPING	
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
	#else
			newGroupEntry->groupAddr[0]=groupAddress[0];
	#endif


			newGroupEntry->ipVersion=ipVersion;

			newGroupEntry->sourceList=NULL;
			
			/*end of set group entry*/
			
			/*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
			if(ipVersion==IP_VERSION4)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable, hashIndex);
			}
			
#ifdef CONFIG_RTL_MLD_SNOOPING
			if(ipVersion==IP_VERSION6)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable, hashIndex);
			}
#endif			

			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				newGroupEntry->groupFilterTimer[portNum]=0;
				/*link the new source list*/
				for(j=0; j<numOfSrc; j++)
				{

					
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							
						}
						
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif						
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
						rtl_linkSourceEntry(newGroupEntry,newSourceEntry);
					}
						
					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
					
				}
			}
			else
			{	
				newGroupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
			
			
		}

	}
	else /*means it can be found in the forward hash table*/
	{  

		if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
		{
			/*here to handle the source list*/
			
			for(j=0; j<numOfSrc; j++)
			{

				sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
		
				if(sourceEntry==NULL)
				{
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
						
						}
						
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif						
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
						rtl_linkSourceEntry(groupEntry,newSourceEntry);
					}

				}
				else
				{		
					/*just update source timer*/
					sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;		
				}
					
				if(ipVersion==IP_VERSION4)
				{	
					sourceAddr++;
				}
				
#ifdef CONFIG_RTL_MLD_SNOOPING
				if(ipVersion==IP_VERSION6)
				{
					sourceAddr=sourceAddr+4;
				}
#endif				
				
			}
                      
	//		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)         /*lower the timer of (A-B) or (X-A)*/
		//	{
			    sourceEntry=groupEntry->sourceList;
				  
	                  if(ipVersion==IP_VERSION4)
	                  {
	                             sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);		
	                  }
					  
#ifdef CONFIG_RTL_MLD_SNOOPING		
	                  if(ipVersion==IP_VERSION6)
	                  {			
		                      sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	                  }
#endif	  
				while(sourceEntry)
				{				  
				         if((rtl_searchSourceAddr(ipVersion, sourceEntry->sourceAddr, sourceAddr, numOfSrc)==FALSE)&&(sourceEntry->portTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)))
					 {      
						if(sourceEntry->portTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;						
						}

					 }
				         sourceEntry=sourceEntry->next;
				}
				
                            if(groupEntry->groupFilterTimer[portNum]>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)   /*lower the group timer if in exclude mode*/
                            {
					groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
			
				}

				if((numOfSrc==0)&&(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE))
				{
		     			if(groupEntry->groupFilterTimer[portNum]>rtl_sysUpSeconds)
		     			{
						groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds;
			 		}
						
					sourceEntry=groupEntry->sourceList;
					
					while(sourceEntry)
					{
						if(sourceEntry->portTimer[portNum]>rtl_sysUpSeconds)
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
						}
		 				sourceEntry=sourceEntry->next;
					}
					
				}
		}
		else
		{	
			if((numOfSrc==0)&&(groupEntry->groupFilterTimer[portNum]>rtl_sysUpSeconds))
			{
		     	 		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		     			{
						groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds;
					}
					else
					{
						groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}			
			}
			else
			{
				groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}

		}
	

	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

return SUCCESS;
	
}

static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum , uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceArray=NULL;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct groupRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[0]&rtl_hashMask;
	}

#ifdef CONFIG_RTL_MLD_SNOOPING		
	if(ipVersion==IP_VERSION6)
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[3]&rtl_hashMask;
	
	}
#endif

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}
		else
		{	
			/*set new multicast entry*/
#ifdef CONFIG_RTL_MLD_SNOOPING	
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
#else
			newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

			newGroupEntry->ipVersion=ipVersion;

			newGroupEntry->sourceList=NULL;
			
			/*means the filter mode is exclude*/
			newGroupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			
			/*end of set group entry*/
			
			/*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
			if(ipVersion==IP_VERSION4)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable, hashIndex);
			}
			
#ifdef CONFIG_RTL_MLD_SNOOPING
			if(ipVersion==IP_VERSION6)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable, hashIndex);
			}
#endif
			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				/*link the new source list*/
				for(j=0; j<numOfSrc; j++)
				{
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
						}
						
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif						
						/*time out the sources included  in the TO_EXCLUDE report */
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
						rtl_linkSourceEntry(newGroupEntry,newSourceEntry);
					}
						
					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}

#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
					
				}

			}
			
			
		}
		
	}
	else /*means group address can be found in the forward hash table*/
	{  
	
		if(groupEntry->groupFilterTimer[portNum]<=rtl_sysUpSeconds) /*means include mode*/
		{

			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				/*here to handle the source list*/
				/*delete (A-B)*/
				sourceEntry=groupEntry->sourceList;
				while(sourceEntry)
				{
					if(rtl_searchSourceAddr(ipVersion,sourceEntry->sourceAddr,sourceArray, numOfSrc)== FALSE)
					{
						sourceEntry->portTimer[portNum]=0;
					}
					sourceEntry=sourceEntry->next;
				}

				/*(B-A) time out*/
				for(j=0; j<numOfSrc; j++)
				{
					sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
					
					if(sourceEntry==NULL)
					{
						newSourceEntry=rtl_allocateSourceEntry();
						if(newSourceEntry==NULL)
						{
							rtl_gluePrintf("run out of source entry!\n");
							return FAILED;
						}
						else
						{	
						
							if(ipVersion==IP_VERSION4)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
							
							}
							
#ifdef CONFIG_RTL_MLD_SNOOPING
							if(ipVersion==IP_VERSION6)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
								newSourceEntry->sourceAddr[1]=sourceAddr[1];
								newSourceEntry->sourceAddr[2]=sourceAddr[2];
								newSourceEntry->sourceAddr[3]=sourceAddr[3];
							}
#endif							
						
							/*B-A time out*/
							newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
							rtl_linkSourceEntry(groupEntry,newSourceEntry);
						}

					}
					else/*maybe include redundant sources*/
					{
						/*B-A time out*/
						if(sourceEntry->portTimer[portNum]==0)
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
						}

						if(sourceEntry->portTimer[portNum]>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)            /*lower the A*B timer if the cpu is router*/
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
						}
					}

					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
				}

			}	
			
		}
		else/*means exclude mode*/
		{

			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				/*here to handle the source list*/
				/*delete (X-A) and delete (Y-A)*/
				sourceEntry=groupEntry->sourceList;
				while(sourceEntry)
				{
					if(rtl_searchSourceAddr(ipVersion,sourceEntry->sourceAddr,sourceArray, numOfSrc)== FALSE)
					{
						sourceEntry->portTimer[portNum]=0;
					}
					sourceEntry=sourceEntry->next;
				}

				/*A-X-Y=filter timer*/
				for(j=0; j<numOfSrc; j++)
				{
					sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
					
					if(sourceEntry==NULL)
					{
						newSourceEntry=rtl_allocateSourceEntry();
						if(newSourceEntry==NULL)
						{
							rtl_gluePrintf("run out of source entry!\n");
							return FAILED;
						}
						else
						{	
						
							if(ipVersion==IP_VERSION4)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
							}
							
#ifdef CONFIG_RTL_MLD_SNOOPING
							if(ipVersion==IP_VERSION6)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
								newSourceEntry->sourceAddr[1]=sourceAddr[1];
								newSourceEntry->sourceAddr[2]=sourceAddr[2];
								newSourceEntry->sourceAddr[3]=sourceAddr[3];	
							}
#endif							
                                                 newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
                                                 rtl_linkSourceEntry(groupEntry,newSourceEntry);													
							
						}

					}
					else
					{	
						if((sourceEntry->portTimer[portNum]==0)||(sourceEntry->portTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)))                                                          
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
						}
					}
				
					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif
					
				}
			}
		
			
		}

		/*switch to exclude mode or update exclude mode filter timer*/
		groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
		
	

	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return SUCCESS;
}

static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;

	

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[0]&rtl_hashMask;
	}

#ifdef CONFIG_RTL_MLD_SNOOPING		
	if(ipVersion==IP_VERSION6)
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[3]&rtl_hashMask;
	
	}
#endif	

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}
		else
		{	
			/*set new multicast entry*/
#ifdef CONFIG_RTL_MLD_SNOOPING	
			newGroupEntry->groupAddr[0]=groupAddress[0];
			newGroupEntry->groupAddr[1]=groupAddress[1];
			newGroupEntry->groupAddr[2]=groupAddress[2];
			newGroupEntry->groupAddr[3]=groupAddress[3];
#else
			newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

		
			newGroupEntry->ipVersion=ipVersion;

			newGroupEntry->sourceList=NULL;
			
			/*end of set group entry*/
			
			/*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
			if(ipVersion==IP_VERSION4)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable, hashIndex);
			}

#ifdef CONFIG_RTL_MLD_SNOOPING
			if(ipVersion==IP_VERSION6)
			{
				rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable, hashIndex);
			}
#endif
			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				newGroupEntry->groupFilterTimer[portNum]=0;
				/*link the new source list*/
				for(j=0; j<numOfSrc; j++)
				{

					
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
						}
						
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif						
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
						rtl_linkSourceEntry(newGroupEntry,newSourceEntry);
					}
					
					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
				}
			}
			else
			{
				newGroupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;;
			}
	
		}

	
		
	}
	else /*means it can be found in the forward hash table*/
	{  
		if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
		{
			/*here to handle the source list*/
			for(j=0; j<numOfSrc; j++)
			{
			
				sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
			
				if(sourceEntry==NULL)
				{
					newSourceEntry=rtl_allocateSourceEntry();
					if(newSourceEntry==NULL)
					{
						rtl_gluePrintf("run out of source entry!\n");
						return FAILED;
					}
					else
					{	
					
						if(ipVersion==IP_VERSION4)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
						}
						
#ifdef CONFIG_RTL_MLD_SNOOPING
						if(ipVersion==IP_VERSION6)
						{	
							newSourceEntry->sourceAddr[0]=sourceAddr[0];
							newSourceEntry->sourceAddr[1]=sourceAddr[1];
							newSourceEntry->sourceAddr[2]=sourceAddr[2];
							newSourceEntry->sourceAddr[3]=sourceAddr[3];
						}
#endif						
						newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
						rtl_linkSourceEntry(groupEntry,newSourceEntry);
					}

				}
				else
				{		
					/*just update source timer*/
					sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;		
				}
					
				if(ipVersion==IP_VERSION4)
				{	
					sourceAddr++;
				}
				
#ifdef CONFIG_RTL_MLD_SNOOPING
				if(ipVersion==IP_VERSION6)
				{
					sourceAddr=sourceAddr+4;
				}
#endif				
			}
		}
		else
		{
			groupEntry->groupFilterTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;;
		}

	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return SUCCESS;
}

static int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint8 *pktBuf)
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};

	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
//	struct rtl_sourceEntry *nextSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[0]&rtl_hashMask;
	}

#ifdef CONFIG_RTL_MLD_SNOOPING		
	if(ipVersion==IP_VERSION6)
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		hashIndex=groupAddress[3]&rtl_hashMask;
	}
#endif

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry!=NULL)
	{  
				
		if(groupEntry->groupFilterTimer[portNum]>rtl_sysUpSeconds) /*means exclude mode*/
		{
			if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
			{
				/*here to handle the source list*/
				/*A-X-Y=filter timer*/
				for(j=0; j<numOfSrc; j++)
				{
					
					sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
				
					if(sourceEntry==NULL)
					{
						newSourceEntry=rtl_allocateSourceEntry();
						if(newSourceEntry==NULL)
						{
							rtl_gluePrintf("run out of source entry!\n");
							return FAILED;
						}
						else
						{	
						
							if(ipVersion==IP_VERSION4)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
							
							}
							
#ifdef CONFIG_RTL_MLD_SNOOPING
							if(ipVersion==IP_VERSION6)
							{	
								newSourceEntry->sourceAddr[0]=sourceAddr[0];
								newSourceEntry->sourceAddr[1]=sourceAddr[1];
								newSourceEntry->sourceAddr[2]=sourceAddr[2];
								newSourceEntry->sourceAddr[3]=sourceAddr[3];
							}
#endif                           
                                                
                                                 if(groupEntry->groupFilterTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
                                                 {
                                                 	newSourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
							}
							else
							{
								newSourceEntry->portTimer[portNum]=groupEntry->groupFilterTimer[portNum];
							}

                                                 rtl_linkSourceEntry(groupEntry,newSourceEntry);
							
						}

					}
					else
					{
						    
                                       	if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
						{
						   	sourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
						}
						else
						{
							if((sourceEntry->portTimer[portNum]==0)||(sourceEntry->portTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)))
							{
								sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
							}
						}

					}

				
					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
					
				}
                            
			}
			#if 0
			else
			{
					groupEntry->groupFilterTimer[portIndex]=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;;
			}
			#endif
		}
              else           /*means include mode*/
              {
                     if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)             /*process the special-group query if the cpu is router*/
                     {
                      	for(j=0; j<numOfSrc; j++)
                          	{

				        sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr,groupEntry);
					if(sourceEntry!=NULL)
					{
						
						if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
						{
							sourceEntry->portTimer[portNum]=rtl_sysUpSeconds;
						}
						else
						{
							if((sourceEntry->portTimer[portNum]==0)||(sourceEntry->portTimer[portNum]>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)))
							{
								sourceEntry->portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
							}
						}
					}	

					if(ipVersion==IP_VERSION4)
					{	
						sourceAddr++;
					}
					
#ifdef CONFIG_RTL_MLD_SNOOPING
					if(ipVersion==IP_VERSION6)
					{
						sourceAddr=sourceAddr+4;
					}
#endif					
			     	}

			}

	        }
	
	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=groupAddress[0];
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return SUCCESS;

}


static uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8 *pktBuf)
{
	uint32 i=0;
	uint16 numOfRecords=0;
	uint8 *groupRecords=NULL;
	uint8 recordType=0xff;
	uint16 numOfSrc=0;
	int32 returnVal=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	
	if(ipVersion==IP_VERSION4)
	{
		numOfRecords=ntohs(((struct igmpv3Report *)pktBuf)->numOfRecords);
		if(numOfRecords!=0)
		{
			groupRecords=(uint8 *)(&(((struct igmpv3Report *)pktBuf)->recordList));
		}
	}

#ifdef CONFIG_RTL_MLD_SNOOPING
	if(ipVersion==IP_VERSION6)
	{	
		numOfRecords=ntohs(((struct mldv2Report *)pktBuf)->numOfRecords);
		if(numOfRecords!=0)
		{
			groupRecords=(uint8 *)(&(((struct mldv2Report *)pktBuf)->recordList));
		}
	}
#endif
	
	for(i=0; i<numOfRecords; i++)
	{
		if(ipVersion==IP_VERSION4)
		{
			recordType=((struct groupRecord *)groupRecords)->type;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING		
		if(ipVersion==IP_VERSION6)
		{
			recordType=((struct mCastAddrRecord *)groupRecords)->type;
		}
#endif		
	
		switch(recordType)
		{
			case MODE_IS_INCLUDE:
				returnVal=rtl_processIsInclude(moduleIndex, ipVersion, portNum, groupRecords);
			break;
			
			case MODE_IS_EXCLUDE:
				returnVal=rtl_processIsExclude(moduleIndex, ipVersion, portNum, groupRecords);
			break;
			
			case CHANGE_TO_INCLUDE_MODE:
				returnVal=rtl_processToInclude(moduleIndex, ipVersion, portNum, groupRecords);
			break;
			
			case CHANGE_TO_EXCLUDE_MODE:
				returnVal=rtl_processToExclude(moduleIndex, ipVersion, portNum, groupRecords);
			break;
			
			case ALLOW_NEW_SOURCES:
				returnVal=rtl_processAllow(moduleIndex, ipVersion, portNum, groupRecords);
			break;
			
			case BLOCK_OLD_SOURCES:
				returnVal=rtl_processBlock(moduleIndex, ipVersion, portNum, groupRecords);
			break;
			
			default:break;
			
		}

		if(ipVersion==IP_VERSION4)
		{
			numOfSrc=ntohs(((struct groupRecord *)groupRecords)->numOfSrc);
			/*shift pointer to another group record*/
			groupRecords=groupRecords+8+numOfSrc*4+(((struct groupRecord *)(groupRecords))->auxLen)*4;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING		
		if(ipVersion==IP_VERSION6)
		{
			numOfSrc=ntohs(((struct mCastAddrRecord *)groupRecords)->numOfSrc);
			/*shift pointer to another group record*/
			groupRecords=groupRecords+20+numOfSrc*16+(((struct mCastAddrRecord *)(groupRecords))->auxLen)*4;
		}
#endif		
	}

	return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	
}

static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum)
{	
	uint32 fwdPortMask=0;

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	reportEventContext.moduleIndex=moduleIndex;
	#endif
	
	switch(pktBuf[0])
	{
		case IGMP_QUERY:
			fwdPortMask=rtl_processQueries(moduleIndex, ipVersion, portNum, pktBuf, pktLen);
		break;
			
		case IGMPV1_REPORT:
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, pktBuf);
		break;
			
		case IGMPV2_REPORT:	
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, pktBuf);
		break;
			
		case IGMPV2_LEAVE:
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, pktBuf);
		break;

		case IGMPV3_REPORT:
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, pktBuf);
		break;

		case MLD_QUERY:
			fwdPortMask=rtl_processQueries(moduleIndex, ipVersion, portNum, pktBuf, pktLen);
		break;
			
		case MLDV1_REPORT:
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, pktBuf);
		break;
			
		case MLDV1_DONE:	
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, pktBuf);
		break;
			
		case MLDV2_REPORT:
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, pktBuf);
		break;

		default:
			fwdPortMask=((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
		break;
	}						
	
	return fwdPortMask;
			
}



static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum)
{
	
	if(ipVersion==IP_VERSION4)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.dvmrpRouterAgingTime; /*update timer*/
	}

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=0;
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));

}

static uint32 rtl_processMospf(uint32 moduleIndex,uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum)
{ 
	struct ipv4MospfHdr *ipv4MospfHeader=(struct ipv4MospfHdr*)pktBuf;
	struct ipv4MospfHello *ipv4HelloPkt=(struct ipv4MospfHello*)pktBuf;
	
#ifdef CONFIG_RTL_MLD_SNOOPING		
	struct ipv6MospfHdr *ipv6MospfHeader=(struct ipv6MospfHdr*)pktBuf;
	struct ipv6MospfHello *ipv6HelloPkt=(struct ipv6MospfHello*)pktBuf;
#endif


	if(ipVersion==IP_VERSION4)
	{	
		/*mospf is built based on ospfv2*/
		if((ipv4MospfHeader->version==2) && (ipv4MospfHeader->type==MOSPF_HELLO_TYPE))
		{
			if((ipv4HelloPkt->options & 0x04)!=0)
			{
				rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.mospfRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/
			}
		}
	}

#ifdef CONFIG_RTL_MLD_SNOOPING	
	if(ipVersion==IP_VERSION6)
	{	
		if((ipv6MospfHeader->version==3) && (ipv6MospfHeader->type==MOSPF_HELLO_TYPE))
		{
			if((ipv6HelloPkt->options[2] & 0x04)!=0)
			{
				rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.mospfRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.mospfRouterAgingTime; /*update timer*/
			
			}
		}
	}
#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=0;
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	
}

static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion, uint8* pktBuf, uint32 pktLen, uint32 portNum)
{
	if(ipVersion==IP_VERSION4)
	{	
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
		
	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING	
	if(ipVersion==IP_VERSION6)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
	}
#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		reportEventContext.groupAddr[0]=0;
		reportEventContext.sourceAddr[0]=0;
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
	}
	#endif

	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}



/*********************************************
				External Function
  *********************************************/


//External called function by high level program
/*
@func int32	| rtl_registerIgmpSnoopingModule	|   API to register an igmp snooping module.
@parm  uint32 *	| moduleIndex	| Output parameter to return the igmp snooping module index.
@rvalue SUCCESS	| Register igmp snooping module successfully.
@rvalue FAILED	| Register igmp snooping module failed.
*/
int32 rtl_registerIgmpSnoopingModule(uint32 *moduleIndex)
{
	int32 i=0;
	uint32 index=0xFFFFFFFF;
	
	*moduleIndex=0xFFFFFFFF;

	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		if(rtl_mCastModuleArray[i].enableSnooping==FALSE)
		{
			index=i;
			break;
		}
	}

	if(i>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	

	if(rtl_mCastModuleArray[index].enableSnooping==FALSE)
	{
	      /*initialize multicast Routers information*/
	      for(i=0; i<MAX_SUPPORT_PORT_NUMBER; i++)
	      {
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.querier.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.pimRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv4MulticastRouters.mospfRouter.portTimer[i]=0;
			
#ifdef CONFIG_RTL_MLD_SNOOPING		
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.querier.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.dvmrpRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.pimRouter.portTimer[i]=0;
			rtl_mCastModuleArray[index].rtl_ipv6MulticastRouters.mospfRouter.portTimer[i]=0;
#endif			
	      }
	
	      /*initialize hash table*/
	      rtl_initHashTable(index, rtl_hashTableSize);
	
	      if((rtl_mCastModuleArray[index].rtl_ipv4HashTable==NULL) )
	      {
		      return FAILED;
	      }
		  
#ifdef CONFIG_RTL_MLD_SNOOPING
		if(rtl_mCastModuleArray[index].rtl_ipv6HashTable==NULL)
		{
			return FAILED;
		}
#endif

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		memset(&rtl_mCastModuleArray[index].deviceInfo,0, sizeof(rtl_multicastDeviceInfo_t));
#endif
      		for(i=0; i<6; i++)
      		{
	      		rtl_mCastModuleArray[index].rtl_gatewayMac[i]=0;
      		}
	
		rtl_mCastModuleArray[index]. rtl_gatewayIpv4Addr=0;

#ifdef CONFIG_RTL_MLD_SNOOPING		
		 for(i=0; i<4; i++)
      		{
	      		rtl_mCastModuleArray[index].rtl_gatewayIpv6Addr[i]=0;
     		 }
#endif
		rtl_mCastModuleArray[index].enableSourceList=TRUE;
	       rtl_mCastModuleArray[index].enableFastLeave=TRUE;
	       rtl_mCastModuleArray[index].enableSnooping=TRUE;
		rtl_mCastModuleArray[index].unknownMCastFloodMap=0;
		rtl_mCastModuleArray[index].staticRouterPortMask=0;

#ifdef CONFIG_PROC_FS
		rtl_mCastModuleArray[index].expireEventCnt=0;
#endif
		*moduleIndex=index;
		return SUCCESS;
	}
	else
	{
	       return FAILED;
	}

	*moduleIndex=index;
	*moduleIndex=index;
	return SUCCESS;
}


/*
@func int32	| rtl_unregisterIgmpSnoopingModule	|  Unregister an igmp snooping module.
@parm  uint32	| moduleIndex	| Igmp snooping module index. 
@rvalue SUCCESS	|Unregister igmp snooping module successfully.
@rvalue FAILED	|Unregister igmp snooping module failed.
@comm 
 This function should be called once a second to maintain multicast timer list.
*/
int32 rtl_unregisterIgmpSnoopingModule(uint32 moduleIndex)
{
	uint32 i=0;
	struct rtl_groupEntry *groupEntryPtr=NULL;
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
       if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
       {
	
		 rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr=0;
		 
          	 for(i=0; i<6; i++)
     		{
	   		 rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[i]=0;
      		}
			
#ifdef CONFIG_RTL_MLD_SNOOPING					 
		for(i=0;i<4;i++)
		{
			rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[i]=0;
		}
#endif	

	 	 /*delete ipv4 multicast entry*/
        	for(i=0;i<rtl_hashTableSize;i++)
	     	{
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
				
			while(groupEntryPtr!=NULL)
			{
				rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
			}
	       }
		rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable=NULL;
		memset(&(rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters), 0, sizeof(struct rtl_multicastRouters));
		
#ifdef CONFIG_RTL_MLD_SNOOPING		
		/*delete ipv6 multicast entry*/
		for(i=0; i<rtl_hashTableSize; i++)
		{
		
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
			while(groupEntryPtr!=NULL)
			{
				rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
			}
		}
		rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=NULL;
		memset(&(rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters), 0, sizeof(struct rtl_multicastRouters));
#endif

		rtl_mCastModuleArray[moduleIndex].enableSnooping=FALSE;
		rtl_mCastModuleArray[moduleIndex].enableSourceList=TRUE;
		rtl_mCastModuleArray[moduleIndex].enableFastLeave=TRUE;

		rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=0;
		rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=0;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		memset(&rtl_mCastModuleArray[moduleIndex].deviceInfo,0,sizeof(rtl_multicastDeviceInfo_t));
#endif
	       return SUCCESS;
       }
	   
	 return SUCCESS;
	
}

static void _rtl865x_configIgmpSnoopingExpire(int32 disableExpire)
{
	uint32 maxTime=0xffffffff;
	
	if((rtl_mCastTimerParas.disableExpire==FALSE) && (disableExpire==TRUE))
	{
		rtl_mCastTimerParas.disableExpire=TRUE;
	}
		
	if((rtl_mCastTimerParas.disableExpire==TRUE) && (disableExpire==FALSE) )
	{
#if defined(__linux__) && defined(__KERNEL__)
		struct timeval currentTimeVector; 
		do_gettimeofday(&currentTimeVector);
		/*reset start time*/
		if(currentTimeVector.tv_sec>=rtl_sysUpSeconds)
		{
			rtl_startTime=(uint32)(currentTimeVector.tv_sec)-rtl_sysUpSeconds;
		}
		else
		{
			/*avoid timer wrap back*/
			rtl_startTime=maxTime-rtl_sysUpSeconds+(uint32)(currentTimeVector.tv_sec)+1;
		}
#endif
		rtl_mCastTimerParas.disableExpire=FALSE;
	}
	

	return;
}

//External called function by high level program
/*
@func  void	| rtl_setMulticastParameters	|   API to config igmp snooping time parameters.
@parm  struct rtl_mCastTimerParameters	| mCastTimerParameters	|IGMP snooping time parameters to be set.
*/
void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters)
{
	_rtl865x_configIgmpSnoopingExpire(mCastTimerParameters.disableExpire);

	if(mCastTimerParameters.groupMemberAgingTime!=0)
	{
		rtl_mCastTimerParas.groupMemberAgingTime= mCastTimerParameters.groupMemberAgingTime;
	}
	
	if(mCastTimerParameters.lastMemberAgingTime!=0)
	{
		rtl_mCastTimerParas.lastMemberAgingTime= mCastTimerParameters.lastMemberAgingTime;
	}

	if(mCastTimerParameters.querierPresentInterval!=0)
	{
	
		rtl_mCastTimerParas.querierPresentInterval=mCastTimerParameters.querierPresentInterval;
	}


	if(mCastTimerParameters.dvmrpRouterAgingTime!=0)
	{
	
		rtl_mCastTimerParas.dvmrpRouterAgingTime=mCastTimerParameters.dvmrpRouterAgingTime;
	}

	if(mCastTimerParameters.mospfRouterAgingTime!=0)
	{
	
		rtl_mCastTimerParas.mospfRouterAgingTime=mCastTimerParameters.mospfRouterAgingTime;
	}

	if(mCastTimerParameters.pimRouterAgingTime!=0)
	{
	
		rtl_mCastTimerParas.pimRouterAgingTime=mCastTimerParameters.pimRouterAgingTime;
	}
	
	return;
}

/*
@func int32	| rtl_configIgmpSnoopingModule	|   API to config local parameters of an igmp snooping module.
@parm  uint32	| moduleIndex	| Input parameter to specify an igmp snooping module.
@parm  struct rtl_mCastSnoopingLocalConfig *	| mCastSnoopingLocalConfig	| Local parameters to be set to an igmp snooping module
@rvalue SUCCESS	| Configuration  success.
@rvalue FAILED	| Configuration failed.
*/
int32 rtl_configIgmpSnoopingModule(uint32 moduleIndex, struct rtl_mCastSnoopingLocalConfig *mCastSnoopingLocalConfig)
{

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
	if(mCastSnoopingLocalConfig==NULL)
	{
		return FAILED;
	}
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	rtl_mCastModuleArray[moduleIndex].enableSourceList=mCastSnoopingLocalConfig->enableSourceList;
	rtl_mCastModuleArray[moduleIndex].enableFastLeave=mCastSnoopingLocalConfig->enableFastLeave;
	rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=mCastSnoopingLocalConfig->unknownMcastFloodMap;
	rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=mCastSnoopingLocalConfig->staticRouterPortMask;
		
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[0]=mCastSnoopingLocalConfig->gatewayMac[0];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[1]=mCastSnoopingLocalConfig->gatewayMac[1];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[2]=mCastSnoopingLocalConfig->gatewayMac[2];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[3]=mCastSnoopingLocalConfig->gatewayMac[3];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[4]=mCastSnoopingLocalConfig->gatewayMac[4];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayMac[5]=mCastSnoopingLocalConfig->gatewayMac[5];



	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv4Addr=mCastSnoopingLocalConfig->gatewayIpv4Addr;
	
#ifdef CONFIG_RTL_MLD_SNOOPING		
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[0]=mCastSnoopingLocalConfig->gatewayIpv6Addr[0];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[1]=mCastSnoopingLocalConfig->gatewayIpv6Addr[1];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[2]=mCastSnoopingLocalConfig->gatewayIpv6Addr[2];
	rtl_mCastModuleArray[moduleIndex].rtl_gatewayIpv6Addr[3]=mCastSnoopingLocalConfig->gatewayIpv6Addr[3];
#endif
	return SUCCESS;
}

/*
@func int32	| rtl_maintainMulticastSnoopingTimerList	|   Maintain igmp snooping internal timer list.
@parm  uint32	| currentSystemTime	|The current system time (unit: seconds).
@rvalue SUCCESS	|Always return SUCCESS.
@comm 
 This function should be called every second
*/
int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime)
{
	/* maintain current time */
	uint32 i=0;
	uint32 maxTime=0xffffffff;

	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextEntry=NULL;

	uint32 moduleIndex;

	if(rtl_mCastTimerParas.disableExpire==TRUE)
	{
		return SUCCESS;
	}
	
	/*handle timer conter overflow*/
	if(currentSystemTime>rtl_startTime)
	{
		rtl_sysUpSeconds=currentSystemTime-rtl_startTime;
	}
	else
	{
		rtl_sysUpSeconds=(maxTime-rtl_startTime)+currentSystemTime+1;
	}

	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
			strcpy(timerEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
			timerEventContext.moduleIndex=moduleIndex;
			#endif

			/*maintain ipv4 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				  /*scan the hash table*/
				 if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
				 {
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						nextEntry=groupEntryPtr->next; 
						#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
						timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						#endif
						rtl_checkGroupEntryTimer(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].enableSourceList, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
						groupEntryPtr=nextEntry;/*because expired group entry  will be cleared*/
					}
				 }
			}
			
#ifdef CONFIG_RTL_MLD_SNOOPING		
			/*maintain ipv6 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				  /*scan the hash table*/
				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
				{
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						nextEntry=groupEntryPtr->next; 
						rtl_checkGroupEntryTimer(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].enableSourceList, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
						groupEntryPtr=nextEntry;/*because expired group entry  will be cleared*/
					}
				}
			}
#endif

		}
	}
	return SUCCESS;
}


/*
@func int32	| rtl_igmpMldProcess	|  API to process an incoming igmp packet.
@parm  uint32	| moduleIndex	| Igmp snooping module index.
@parm  uint8 *	| macFrame	| Data pointer of igmp mac frame.
@parm  uint32	| portNum	| Input parameter to specify at which port this igmp packet was recieved 
@parm  uint32 *	| fwdPortMask	| Output parameter to tell where to relay this igmp packet.
@rvalue SUCCESS	|Doing igmp snooping successfully.
@rvalue FAILED	|Doing igmp snooping failed.
*/
int32 rtl_igmpMldProcess(uint32 moduleIndex, uint8 * macFrame,  uint32 portNum, uint32 *fwdPortMask)
{

	struct rtl_macFrameInfo macFrameInfo;

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	reportEventContext.portMask=1<<portNum;
	#endif

	*fwdPortMask=(~(1<<portNum)) & 0xFFFFFFFF;

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
	//rtl_parseMacFrame(moduleIndex, macFrame, TRUE, &macFrameInfo);
	rtl_parseMacFrame(moduleIndex, macFrame, FALSE, &macFrameInfo);
	if(  rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		if(macFrameInfo.ipBuf==NULL)
		{
			return FAILED;
		}
		
		if((macFrameInfo.ipVersion!=IP_VERSION4) && (macFrameInfo.ipVersion!=IP_VERSION6))
		{
		
			return FAILED;
		}
		
#ifndef CONFIG_RTL_MLD_SNOOPING	
		if (macFrameInfo.ipVersion==IP_VERSION6)
		{
			return FAILED;
		}
#endif
		/*port num starts from 0*/
		if(portNum>=MAX_SUPPORT_PORT_NUMBER)
		{
			return FAILED;
		}
		#if 0
		if(rtl_checkPortMask(pktPortMask)==FAILED)
		{
			return FAILED;
		}
		#endif
		if(macFrameInfo.checksumFlag!=SUCCESS)
		{
			return FAILED;
		}

		switch(macFrameInfo.l3Protocol)
		{

			case IGMP_PROTOCOL:
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, portNum);
			break;

			case ICMP_PROTOCOL:
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, portNum);
			break;


			case DVMRP_PROTOCOL:
				*fwdPortMask=rtl_processDvmrp(moduleIndex, (uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, portNum);
			break;

			case MOSPF_PROTOCOL:
				*fwdPortMask=rtl_processMospf(moduleIndex, (uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen,portNum);
			break;
				
			case PIM_PROTOCOL:
				*fwdPortMask=rtl_processPim(moduleIndex, (uint32)(macFrameInfo.ipVersion), macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, portNum);
			break;

			default: break;
		}
		
	}
	
	return SUCCESS;
}


/*
@func int32	| rtl_getMulticastDataFwdPortMask	| API to get multicast data forward port mask.
@parm  uint32	| moduleIndex	| Igmp snooping module index.
@parm  struct rtl_multicastDataInfo *	| multicastDataInfo	| Multicast data related information.
@parm  uint32 *	| fwdPortMask	|Output parameter to tell where to forward this multicast data.
@rvalue SUCCESS	| Get multicast data forward port mask successfully.
@rvalue FAILED	| Get multicast data forward port mask failed.
*/
int32 rtl_getMulticastDataFwdPortMask(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, uint32 *fwdPortMask)
{
	int32 retVal=FAILED;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
		
	if(multicastDataInfo==NULL)
	{
		return FAILED;
	}

	if(fwdPortMask==NULL)
	{
		return FAILED;
	}
	
	retVal=rtl_getMulticastDataFwdInfo( moduleIndex, multicastDataInfo, &multicastFwdInfo);

	*fwdPortMask=multicastFwdInfo.fwdPortMask;

	return retVal;
	
}



int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo)
{

	struct rtl_groupEntry * groupEntry=NULL;
	uint32 multicastRouterPortMask=0;

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
		
	if(multicastDataInfo==NULL)
	{
		return FAILED;
	}

	if(multicastFwdInfo==NULL)
	{
		return FAILED;
	}

	memset(multicastFwdInfo, 0, sizeof(struct rtl_multicastFwdInfo));

	if(multicastDataInfo->groupAddr[0]==RESERVE_MULTICAST_ADDR1)
	{
		multicastFwdInfo->reservedMCast=TRUE;
		multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
		multicastFwdInfo->cpuFlag=TRUE;
		
		return FAILED;
	}
	
	if(IN_MULTICAST_RESV1(multicastDataInfo->groupAddr[0]) )
	{
		multicastFwdInfo->reservedMCast=TRUE;
		multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
		multicastFwdInfo->cpuFlag=TRUE;
		
		return FAILED;
	}
	
	groupEntry=rtl_searchGroupEntry(moduleIndex,multicastDataInfo->ipVersion, multicastDataInfo->groupAddr); 
	
	if(groupEntry==NULL)
	{
		multicastFwdInfo->unknownMCast=TRUE;
		multicastFwdInfo->fwdPortMask= rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap;

		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		return SUCCESS;
	}
	else
	{
		/*here to get multicast router port mask and forward port mask*/
		//multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, multicastDataInfo->ipVersion, rtl_sysUpSeconds);
		if(rtl_mCastModuleArray[moduleIndex].enableSourceList==TRUE)
		{
			multicastFwdInfo->fwdPortMask=rtl_getSourceFwdPortMask(groupEntry, multicastDataInfo->sourceIp, rtl_sysUpSeconds);
		}
		else
		{
			if(groupEntry!=NULL)
			{
				multicastFwdInfo->fwdPortMask=(uint32)rtl_getGroupFwdPortMask(groupEntry,0, rtl_sysUpSeconds);	
			}

		}
		
		multicastFwdInfo->fwdPortMask=(multicastFwdInfo->fwdPortMask|multicastRouterPortMask);

		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		return SUCCESS;
	
	}
	return FAILED;
	
}

#if defined(__linux__) && defined(__KERNEL__)

static void rtl_multicastSysTimerExpired(uint32 expireDada)
{
	struct timeval currentTimeVector; 
	
	do_gettimeofday(&currentTimeVector);
	rtl_maintainMulticastSnoopingTimerList((uint32)(currentTimeVector.tv_sec));
	mod_timer(&igmpSysTimer, jiffies+HZ);
	
}

static void rtl_multicastSysTimerInit(void)
{
	struct timeval startTimeVector; 
	do_gettimeofday(&startTimeVector);
	rtl_startTime=(uint32)(startTimeVector.tv_sec);
	rtl_sysUpSeconds=0;  
	
	init_timer(&igmpSysTimer);
	igmpSysTimer.data=igmpSysTimer.expires;
	igmpSysTimer.expires=jiffies+HZ;
	igmpSysTimer.function=(void*)rtl_multicastSysTimerExpired;
	add_timer(&igmpSysTimer);
}

static void rtl_multicastSysTimerDestroy(void)
{
	del_timer(&igmpSysTimer);
}

#endif

int32 rtl_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex)
{
	int i;
	*moduleIndex=0xFFFFFFFF;
	if(devInfo==NULL)
	{
		return FAILED;
	}
	
	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{
		if(rtl_mCastModuleArray[i].enableSnooping==TRUE)
		{
			if(strcmp(rtl_mCastModuleArray[i].deviceInfo.devName, devInfo->devName)==0)
			{
				*moduleIndex=i;
				return SUCCESS;
			}
		}
	}
	
	return FAILED;
}

int32 rtl865x_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex)
{
	return rtl_getDeviceIgmpSnoopingModuleIndex(devInfo,moduleIndex);
}


/*
@func int32	| rtl_setIgmpSnoopingModuleDevInfo	|   API to set igmp snooping module related device information.
@parm  uint32	| moduleIndex	| Input parameter to specify igmp snooping module.
@parm  rtl_multicastDeviceInfo_t	*| devInfo	| Device information to be set.
@rvalue SUCCESS	|Setting success.
@rvalue FAILED	|Setting failed.
*/
int32 rtl_setIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(devInfo==NULL)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	memcpy(&rtl_mCastModuleArray[moduleIndex].deviceInfo,devInfo, sizeof(rtl_multicastDeviceInfo_t));
	
	return SUCCESS;
}

int32 rtl_setIgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 staticRouterPortMask)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	
	rtl_mCastModuleArray[moduleIndex].staticRouterPortMask=staticRouterPortMask;
	
	return SUCCESS;
}

int32 rtl_getgmpSnoopingModuleStaticRouterPortMask(uint32 moduleIndex,uint32 *staticRouterPortMask)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
	if(staticRouterPortMask==NULL)
	{
		return FAILED;
	}
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	*staticRouterPortMask=rtl_mCastModuleArray[moduleIndex].staticRouterPortMask;
	
	return SUCCESS;
}


int32 rtl_setIgmpSnoopingModuleUnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap=unknownMCastFloodMap;
	
	return SUCCESS;
}

int32 rtl_getIgmpSnoopingModuleUnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}
	
	if(unknownMCastFloodMap==NULL)
	{
		return FAILED;
	}
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	*unknownMCastFloodMap=rtl_mCastModuleArray[moduleIndex].unknownMCastFloodMap;
	
	return SUCCESS;
}

/*
@func int32	| rtl_setIgmpSnoopingModuleDevInfo	| API to get igmp snooping module related device information.
@parm  uint32	| moduleIndex	| Input parameter to specify igmp snooping module
@parm  rtl_multicastDeviceInfo_t	*| devInfo	| Output parameter to store the device information of specified igmp snooping module.
@rvalue SUCCESS	|Geting success.
@rvalue FAILED	|Getting failed.
*/
int32 rtl_getIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(devInfo==NULL)
	{
		return FAILED;
	}
	memset(devInfo,0,sizeof(rtl_multicastDeviceInfo_t));
	
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	memcpy(devInfo,&rtl_mCastModuleArray[moduleIndex].deviceInfo, sizeof(rtl_multicastDeviceInfo_t));
	return SUCCESS;
}



#ifdef CONFIG_PROC_FS
int igmp_show(struct seq_file *s, void *v)
{
	int32 moduleIndex;
	int i=0,j=0;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_sourceEntry *sourceEntryPtr;
	int groupCnt=0;
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{	
			groupCnt=0;
			seq_printf(s, "---------------------------------------------\n");
			seq_printf(s, "module index:%d,",moduleIndex);
			#ifdef CONFIG_RTL_HARDWARE_MULTICAST
			seq_printf(s, "device name:%s\n",rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
			#endif
			for(i=0;i<rtl_hashTableSize;i++)
		     	{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					
				while(groupEntryPtr!=NULL)
				{
					groupCnt++;
					seq_printf(s, "[%d]group address:%d.%d.%d.%d\n",groupCnt,
						groupEntryPtr->groupAddr[0]>>24, (groupEntryPtr->groupAddr[0]&0x00ff0000)>>16,
						(groupEntryPtr->groupAddr[0]&0x0000ff00)>>8, (groupEntryPtr->groupAddr[0]&0xff));
				
					for(j=0;j<MAX_SUPPORT_PORT_NUMBER;j++)
					{
						if(groupEntryPtr->groupFilterTimer[j]>=rtl_sysUpSeconds)
						{
							seq_printf(s, "\tgroup timer[port%d]:%d seconds\n",j,groupEntryPtr->groupFilterTimer[j]-rtl_sysUpSeconds);
						}
						else
						{
							//printk("\tgroup timer[port%d]:0 seconds\n",j);

						}
					}
					
					sourceEntryPtr=groupEntryPtr->sourceList;
					while(sourceEntryPtr!=NULL)
					{
						seq_printf(s, "\t\tsource address:%d.%d.%d.%d\n",
						sourceEntryPtr->sourceAddr[0]>>24, (sourceEntryPtr->sourceAddr[0]&0x00ff0000)>>16,
						(sourceEntryPtr->sourceAddr[0]&0x0000ff00)>>8, (sourceEntryPtr->sourceAddr[0]&0xff));
						for(j=0;j<MAX_SUPPORT_PORT_NUMBER;j++)
						{
							if(sourceEntryPtr->portTimer[j]>=rtl_sysUpSeconds)
							{
								seq_printf(s, "\t\tsource  timer[port%d]:%d seconds\n",j,sourceEntryPtr->portTimer[j]-rtl_sysUpSeconds);
							}
							else
							{
								//printk("\t\tsource  timer[port%d]:0 seconds\n",j);
							}
						}
						sourceEntryPtr=sourceEntryPtr->next;
					}
					seq_printf(s, "\n");
					groupEntryPtr=groupEntryPtr->next;
				}
		       }
			seq_printf(s, "expire event cnt is %d\n",rtl_mCastModuleArray[moduleIndex].expireEventCnt);
			seq_printf(s, "\n\n");
		}
	}

	return SUCCESS;
}
#endif



void rtl865x_igmpLinkStatusChangeCallback(uint32 moduleIndex, rtl_igmpPortInfo_t * portInfo)
{
	int i=0,j=0;
	int32 clearFlag=FALSE;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_sourceEntry *sourceEntryPtr;
	
	
	if(portInfo==NULL)
	{
		return ;
	}

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return ;
	}
	

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		
		for(i=0;i<rtl_hashTableSize;i++)
	     	{
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
				
			while(groupEntryPtr!=NULL)
			{
				clearFlag=FALSE;
					
				for(j=0;j<MAX_SUPPORT_PORT_NUMBER;j++)
				{
					if(( (1<<j) & (portInfo->linkPortMask)) ==0)
					{
						if(groupEntryPtr->groupFilterTimer[j]!=0)
						{
							groupEntryPtr->groupFilterTimer[j]=0;
							clearFlag=TRUE;
						}
					}
				
				}
				
				sourceEntryPtr=groupEntryPtr->sourceList;
				while(sourceEntryPtr!=NULL)
				{
					

					for(j=0;j<MAX_SUPPORT_PORT_NUMBER;j++)
					{
						if(( (1<<j) & (portInfo->linkPortMask)) ==0)
						{
							if(sourceEntryPtr->portTimer[j]!=0)
							{
								sourceEntryPtr->portTimer[j]=0;
								clearFlag=TRUE;
							}
						}
						
					}
					sourceEntryPtr=sourceEntryPtr->next;
				}
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
				if(clearFlag==TRUE)
				{
					strcpy(linkEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
					linkEventContext.moduleIndex=moduleIndex;
					
					linkEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
					linkEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
					linkEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
					linkEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
					
					linkEventContext.sourceAddr[0]=0;
					linkEventContext.sourceAddr[1]=0;
					linkEventContext.sourceAddr[2]=0;
					linkEventContext.sourceAddr[3]=0;
					
					rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &linkEventContext);
				}
#endif
				
				groupEntryPtr=groupEntryPtr->next;
			}
	       }
	
	}

	return ;
}

int32 rtl_getGroupInfo(uint32 groupAddr, struct rtl_groupInfo * groupInfo)
{
	int32 moduleIndex;
	int32 hashIndex;
	struct rtl_groupEntry *groupEntryPtr;
	
	if(groupInfo==NULL)
	{
		return FAILED;
	}

	memset(groupInfo, 0 , sizeof(struct rtl_groupInfo));
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			hashIndex=rtl_hashMask&groupAddr;
			groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
				
			while(groupEntryPtr!=NULL)
			{
				if(groupEntryPtr->groupAddr[0]==groupAddr)
				{
					groupInfo->ownerMask |= (1<<moduleIndex);
					break;
				}
				groupEntryPtr=groupEntryPtr->next;
			}
		      
		}
	}

	return SUCCESS;
}

