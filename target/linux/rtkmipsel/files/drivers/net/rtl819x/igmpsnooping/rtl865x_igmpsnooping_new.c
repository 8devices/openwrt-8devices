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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
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


#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include "rtl865x_igmpsnooping_local.h"
//#include "../common/assert.h"

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#include "../common/rtl865x_eventMgr.h"
#endif



static struct rtl_multicastModule rtl_mCastModuleArray[MAX_MCAST_MODULE_NUM];    
#if defined(__linux__) && defined(__KERNEL__)
static struct timer_list igmpSysTimer;	/*igmp timer*/
#endif
/*global system resources declaration*/
static uint32 rtl_totalMaxGroupCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxClientCnt;    /*maximum total group entry count,  default is 100*/
static uint32 rtl_totalMaxSourceCnt;   /*maximum total group entry count,  default is 3000*/

void *rtl_groupMemory=NULL;
void *rtl_clientMemory=NULL;
void *rtl_sourceMemory=NULL;
void *rtl_mcastFlowMemory=NULL;

static struct rtl_groupEntry *rtl_groupEntryPool=NULL;
static struct rtl_clientEntry *rtl_clientEntryPool=NULL;
static struct rtl_sourceEntry *rtl_sourceEntryPool=NULL;
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry *rtl_mcastFlowEntryPool=NULL;
#endif
static struct rtl_mCastTimerParameters rtl_mCastTimerParas;  /*IGMP snooping parameters */

static uint32 rtl_hashTableSize=0;
static uint32 rtl_hashMask=0;

/*the system up time*/
static uint32 rtl_startTime;
static uint32 rtl_sysUpSeconds;       

static rtl_multicastEventContext_t reportEventContext;
static rtl_multicastEventContext_t timerEventContext;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
static rtl_multicastEventContext_t linkEventContext;
#endif

#if 0
#ifndef CONFIG_RTL_IGMPSNOOPING_MAC_BASED
#define CONFIG_RTL_IGMPSNOOPING_MAC_BASED 1
#endif
#endif


#ifdef CONFIG_RTK_MESH
extern unsigned int mesh_port_no;
#endif

extern unsigned int brIgmpModuleIndex;
extern unsigned int brIgmpModuleIndex_2;
extern uint32 nicIgmpModuleIndex;


struct rtl865x_ReservedMCastRecord reservedMCastRecord[MAX_RESERVED_MULTICAST_NUM];

#if defined(CONFIG_RTL_8370_SUPPORT)
static uint32 wait_mac=0;
static uint32 saved_groupAddress;
static uint32 saved_clientAddr;
void rtl8370L2McastAddrUpdate(int mode, uint32 groupAddress, uint32 clientAddr, uint8 *mac);
#endif

/*******************************internal function declaration*****************************/


/**************************
	resource managment
**************************/
static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize);
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void);
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) ;


static  struct rtl_clientEntry* rtl_initClientEntryPool(uint32 poolSize);
static  struct rtl_clientEntry* rtl_allocateClientEntry(void);
static  void rtl_freeClientEntry(struct rtl_clientEntry* clientEntryPtr) ;

static  struct rtl_sourceEntry* rtl_initSourceEntryPool(uint32 poolSize);
static  struct rtl_sourceEntry* rtl_allocateSourceEntry(void);
static  void rtl_freeSourceEntry(struct rtl_sourceEntry* sourceEntryPtr) ;
#ifdef CONFIG_RECORD_MCAST_FLOW
static  struct rtl_mcastFlowEntry* rtl_initMcastFlowEntryPool(uint32 poolSize);
static  struct rtl_mcastFlowEntry* rtl_allocateMcastFlowEntry(void);
static  void rtl_freeMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry) ;
#endif
/**********************************Structure Maintenance*************************/

static struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr);
static void rtl_linkGroupEntry(struct rtl_groupEntry* entryNode ,  struct rtl_groupEntry ** hashTable);
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* entryNode,  struct rtl_groupEntry ** hashTable);
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntryPtr);

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)	
static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion,struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr);
#else
static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion,struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr);
#endif
static void rtl_linkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry);
static void rtl_unlinkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry);
static void rtl_clearClientEntry(struct rtl_clientEntry* clientEntryPtr);
static void rtl_deleteClientEntry(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry);

static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_clientEntry *clientEntry);
static void rtl_linkSourceEntry(struct rtl_clientEntry *clientEntry,  struct rtl_sourceEntry* entryNode);
static void rtl_unlinkSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* entryNode);
static void rtl_clearSourceEntry(struct rtl_sourceEntry* sourceEntryPtr);
static void rtl_deleteSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry);
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry* rtl_searchMcastFlowEntry(uint32 moduleIndex, uint32 ipVersion, uint32 *serverAddr,uint32 *groupAddr);
static void  rtl_linkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry ,  struct rtl_mcastFlowEntry ** hashTable);
static void rtl_unlinkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry,  struct rtl_mcastFlowEntry ** hashTable);
static void rtl_clearMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry);
static void rtl_deleteMcastFlowEntry( struct rtl_mcastFlowEntry* mcastFlowEntry, struct rtl_mcastFlowEntry ** hashTable);
#endif
static int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr);

#ifdef CONFIG_RTL_MLD_SNOOPING
static int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2);
static uint16 rtl_ipv6L3Checksum(uint8 *pktBuf, uint32 pktLen, union pseudoHeader *ipv6PseudoHdr);
#endif
static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2);
static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen);
static uint32 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry,  uint32 sysTime);
static void rtl_checkSourceTimer(struct rtl_clientEntry * clientEntry , struct rtl_sourceEntry * sourceEntry);
static uint32 rtl_getGroupSourceFwdPortMask(struct rtl_groupEntry * groupEntry, uint32 * sourceAddr, uint32 sysTime);
static uint32 rtl_getClientSourceFwdPortMask(uint32 ipVersion, struct rtl_clientEntry * clientEntry, uint32 * sourceAddr, uint32 sysTime);
  
static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_groupEntry ** hashTable);
static void rtl_checkClientEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry);

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
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)	|| defined (M2U_DELETE_CHECK)
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf);
static	uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *clientmacAddr,uint8 *pktBuf); //process leave/done report packet
static	int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf); //process MODE_IS_INCLUDE report packet 
static	int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *clientmacAddr,uint8 *pktBuf); //process MODE_IS_EXCLUDE report packet
static	int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint32 *clientAddr, uint8 *clientmacAddr,uint8 *pktBuf); //process CHANGE_TO_INCLUDE_MODE report packet
static	int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum , uint32 *clientAddr, uint8 *clientmacAddr,uint8 *pktBuf); //process CHANGE_TO_EXCLUDE_MODE report packet
static	int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *clientmacAddr,uint8 *pktBuf); //process ALLOW_NEW_SOURCES report packet 
static	int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *clientmacAddr,uint8 *pktBuf);//process BLOCK_OLD_SOURCES report packet
static	uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf);

#else

static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); // process join report packet 
static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process leave/done report packet
static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process MODE_IS_INCLUDE report packet 
static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process MODE_IS_EXCLUDE report packet
static  int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process CHANGE_TO_INCLUDE_MODE report packet
static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum , uint32 *clientAddr, uint8 *pktBuf); //process CHANGE_TO_EXCLUDE_MODE report packet
static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf); //process ALLOW_NEW_SOURCES report packet 
static  int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf);//process BLOCK_OLD_SOURCES report packet
static  uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8 *pktBuf);
#endif
/*******************different protocol process function**********************************/
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr,uint8 *clientMacAddr, uint8* pktBuf, uint32 pktLen);
#else
static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8* pktBuf, uint32 pktLen);
#endif
static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processMospf(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,  uint8* pktBuf, uint32 pktLen);
static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint8* pktBuf, uint32 pktLen);

#ifdef CONFIG_RECORD_MCAST_FLOW
static int32 rtl_recordMcastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *sourceIpAddr, uint32 *groupAddr, struct rtl_multicastFwdInfo * multicastFwdInfo);
static void rtl_invalidateMCastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *groupAddr);
static void rtl_doMcastFlowRecycle(uint32 moduleIndex,uint32 ipVersion);
#endif

#if  defined(__linux__) && defined(__KERNEL__)
static void rtl_multicastSysTimerExpired(uint32 expireDada);
static void rtl_multicastSysTimerInit(void);
static void rtl_multicastSysTimerDestroy(void);
#endif

static void rtl_deleteGroupEntry( struct rtl_groupEntry* groupEntry,struct rtl_groupEntry ** hashTable);
/************************************************
			Implementation
  ************************************************/
  
/**************************
	Initialize
**************************/

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
#ifdef CONFIG_RECORD_MCAST_FLOW		
		rtl_mCastModuleArray[i].flowHashTable=NULL;	
#endif		
		rtl_mCastModuleArray[i].enableSnooping=FALSE;
		rtl_mCastModuleArray[i].enableFastLeave=FALSE;

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
	  
	
      rtl_groupMemory=NULL;
      rtl_clientMemory=NULL;
      rtl_sourceMemory=NULL;
      rtl_mcastFlowMemory=NULL;
	
	/*initialize group entry pool*/
      if(mCastSnoopingGlobalConfig.maxGroupNum==0)
      {
	      rtl_totalMaxGroupCnt=DEFAULT_MAX_GROUP_COUNT;
      }	
      else
      {
		rtl_totalMaxGroupCnt=mCastSnoopingGlobalConfig.maxGroupNum;
      }

      rtl_groupEntryPool=rtl_initGroupEntryPool(rtl_totalMaxGroupCnt); 
      if(rtl_groupEntryPool==NULL)
      {
	      return FAILED;
      }
	  
	  /*initialize client entry pool*/
      if(mCastSnoopingGlobalConfig.maxClientNum==0)
      {
	      rtl_totalMaxClientCnt=DEFAULT_MAX_CLIENT_COUNT;
      }	
      else
      {
	      rtl_totalMaxClientCnt=mCastSnoopingGlobalConfig.maxClientNum;
      }

      rtl_clientEntryPool=rtl_initClientEntryPool(rtl_totalMaxClientCnt); 
      if(rtl_clientEntryPool==NULL)
      {
	      return FAILED;
      }
#ifdef CONFIG_RECORD_MCAST_FLOW
      rtl_mcastFlowEntryPool=rtl_initMcastFlowEntryPool(DEFAULT_MAX_FLOW_COUNT); 
      if(rtl_mcastFlowEntryPool==NULL)
      {
	      return FAILED;
      }
#endif
	/*initialize source entry pool*/
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


int32 rtl_configMulticastSnoopingFastLeave(int enableFastLeave, int ageTime)
{
	int i;
	
	for(i=0; i<MAX_MCAST_MODULE_NUM; i++)
	{	
		if(rtl_mCastModuleArray[i].enableSnooping==TRUE)
			rtl_mCastModuleArray[i].enableFastLeave= enableFastLeave;

	}
	
	  if(ageTime)
	  	  rtl_mCastTimerParas.lastMemberAgingTime= ageTime;
	  else	
	      rtl_mCastTimerParas.lastMemberAgingTime= DEFAULT_LAST_MEMBER_AGINTGTIME;
	  

	return SUCCESS;

}

int32 rtl_flushAllIgmpRecord(uint8 flush_flag)
{
	/* maintain current time */
	uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextGroupEntry=NULL;

	uint32 moduleIndex;
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
		    if(flush_flag & FLUSH_IGMP_RECORD)
            { 
			    /*maintain ipv4 group entry  timer */
			    for(i=0; i<rtl_hashTableSize; i++)
			    {
				    /*scan the hash table*/
				    if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
				    {
					    groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					    while(groupEntryPtr)              /*traverse each group list*/
					    {	
						    nextGroupEntry=groupEntryPtr->next; 
						
						    reportEventContext.moduleIndex=moduleIndex;
						    reportEventContext.ipVersion=IP_VERSION4;
						    reportEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						    reportEventContext.groupAddr[1]=0;
						    reportEventContext.groupAddr[2]=0;
						    reportEventContext.groupAddr[3]=0;
						
						    rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);

						    #ifdef CONFIG_RECORD_MCAST_FLOW
						    rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
						    #endif

						    #if defined (CONFIG_RTL_HARDWARE_MULTICAST)
						    strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
						    rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
						#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
						rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
						    #endif
						#endif

						    groupEntryPtr=nextGroupEntry;
					    }
				    }
			    }
            }
            
#ifdef CONFIG_RTL_MLD_SNOOPING
            if(flush_flag & FLUSH_MLD_RECORD)
            {
			    /*maintain ipv6 group entry  timer */
			    for(i=0; i<rtl_hashTableSize; i++)
			    {
				    /*scan the hash table*/
				    if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
				    {
					    groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
					    while(groupEntryPtr)              /*traverse each group list*/
					    {	
						    nextGroupEntry=groupEntryPtr->next; 
						
						    reportEventContext.moduleIndex=moduleIndex;
						    reportEventContext.ipVersion=IP_VERSION6;
						    reportEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						    reportEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
						    reportEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
						    reportEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
						
						    rtl_deleteGroupEntry(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);

						    #ifdef CONFIG_RECORD_MCAST_FLOW
						    rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
						    #endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
						    strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
						    rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
#endif
#endif

						    groupEntryPtr=nextGroupEntry;
					    }
				    }
			    }

            }
#endif

		}
	}
	return SUCCESS;
}

static int32 rtl_setClientMacAddr(struct rtl_macFrameInfo *macFrameInfo)
{
	/* maintain current time */
	uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextGroupEntry=NULL;
	struct rtl_clientEntry* clientEntryPtr=NULL;

	uint32 moduleIndex;
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{

			/*maintain ipv4 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				  /*scan the hash table*/
				 if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
				 {
				 	
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						nextGroupEntry=groupEntryPtr->next; 
						clientEntryPtr=groupEntryPtr->clientList;
						while(clientEntryPtr!=NULL)
						{
					
							if(memcmp(clientEntryPtr->clientAddr, macFrameInfo->srcIpAddr,4)==0)
							{
								memcpy(clientEntryPtr->clientMacAddr,macFrameInfo->srcMacAddr,6);	
#if defined(CONFIG_RTL_8370_SUPPORT)
								if ((wait_mac == 1) &&
									(groupEntryPtr->groupAddr[0] == saved_groupAddress) &&
									(clientEntryPtr->clientAddr[0] == saved_clientAddr)
									) {
									wait_mac = 0;
									//  add/update 8370 L2 multicast entry here
									rtl8370L2McastAddrUpdate(1, saved_groupAddress, saved_clientAddr, 
										clientEntryPtr->clientMacAddr);								
								}
#endif
							}
							clientEntryPtr=clientEntryPtr->next;
						}
						groupEntryPtr=nextGroupEntry;
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
						nextGroupEntry=groupEntryPtr->next; 
						clientEntryPtr=groupEntryPtr->clientList;
						while(clientEntryPtr!=NULL)
						{
							if(memcmp(clientEntryPtr->clientAddr, macFrameInfo->srcIpAddr,16)==0)
							{
								memcpy(clientEntryPtr->clientMacAddr,macFrameInfo->srcMacAddr,6);	
							}
							clientEntryPtr=clientEntryPtr->next;
						}
						groupEntryPtr=nextGroupEntry;
					}
				}
			}
#endif

		}
	}
	return SUCCESS;
}


int32 rtl_delIgmpRecordByMacAddr(uint8 *macAddr)
{
	/* maintain current time */
	uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextGroupEntry=NULL;
	struct rtl_clientEntry* clientEntryPtr=NULL;
	struct rtl_clientEntry* nextClientEntryPtr=NULL;
	
	uint32 moduleIndex;
	uint32 deleteFlag=FALSE;
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{

			/*maintain ipv4 group entry  timer */
			for(i=0; i<rtl_hashTableSize; i++)
			{
				  /*scan the hash table*/
				 if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
				 {
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						deleteFlag=FALSE;
						reportEventContext.moduleIndex=moduleIndex;
						reportEventContext.ipVersion=IP_VERSION4;
						reportEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						reportEventContext.groupAddr[1]=0;
						reportEventContext.groupAddr[2]=0;
						reportEventContext.groupAddr[3]=0;
						
						nextGroupEntry=groupEntryPtr->next; 
						clientEntryPtr=groupEntryPtr->clientList;
						while(clientEntryPtr!=NULL)
						{
							nextClientEntryPtr=clientEntryPtr->next;
							if(memcmp(clientEntryPtr->clientMacAddr,macAddr,6)==0)
							{
								rtl_deleteClientEntry(groupEntryPtr, clientEntryPtr);
								deleteFlag=TRUE;
							}
							clientEntryPtr=nextClientEntryPtr;
						}

						if(deleteFlag==TRUE)
						{
							#ifdef CONFIG_RECORD_MCAST_FLOW
							rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
							#endif

							#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
							strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
							rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
							rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
							#endif
						}
						groupEntryPtr=nextGroupEntry;
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
						deleteFlag=FALSE;
						reportEventContext.moduleIndex=moduleIndex;
						reportEventContext.ipVersion=IP_VERSION6;
						reportEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						reportEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
						reportEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
						reportEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
						
						nextGroupEntry=groupEntryPtr->next; 
						clientEntryPtr=groupEntryPtr->clientList;
						while(clientEntryPtr!=NULL)
						{
							nextClientEntryPtr=clientEntryPtr->next;
							if(memcmp(clientEntryPtr->clientMacAddr,macAddr, 6)==0)
							{
								rtl_deleteClientEntry(groupEntryPtr, clientEntryPtr);
								deleteFlag=TRUE;
							}
							clientEntryPtr=nextClientEntryPtr;
						}
						
						if(deleteFlag==TRUE)
						{
							#ifdef CONFIG_RECORD_MCAST_FLOW
							rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
							#endif
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
							strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
							rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
#endif
#endif
						}

						groupEntryPtr=nextGroupEntry;
					}
				}
			}
#endif

		}
	}
	return SUCCESS;
}

static inline uint32 rtl_igmpHashAlgorithm(uint32 ipVersion,uint32 *groupAddr)
{
	uint32 hashIndex=0;
	
	if(ipVersion==IP_VERSION4)
	{
		/*to do:change hash algorithm*/
		hashIndex=rtl_hashMask&groupAddr[0];
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		hashIndex=rtl_hashMask&groupAddr[3];
	}
#endif

	return hashIndex;
}

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
	
	if(rtl_clientMemory!=NULL)
	{
		rtl_glueFree(rtl_clientMemory);	
	}
	
	rtl_totalMaxClientCnt=0;
	rtl_clientMemory=NULL;
	rtl_clientEntryPool=NULL;

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

/*group entry memory management*/
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

// allocate a group entry from the group entry pool
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

/*client entry memory management*/
static  struct rtl_clientEntry* rtl_initClientEntryPool(uint32 poolSize)
{
	
	uint32 idx=0;
	struct rtl_clientEntry *poolHead=NULL;
	struct rtl_clientEntry *entryPtr=NULL;
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_clientEntry *)rtl_glueMalloc(sizeof(struct rtl_clientEntry) * poolSize);
	rtl_clientMemory=(void *)poolHead;
	
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_clientEntry)));
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


// allocate a client entry from the client entry pool
static  struct rtl_clientEntry* rtl_allocateClientEntry(void)
{
	struct rtl_clientEntry *ret = NULL;

	rtl_glueMutexLock();	
	if (rtl_clientEntryPool!=NULL)
	{
		ret = rtl_clientEntryPool;
		if(rtl_clientEntryPool->next!=NULL)
		{
			rtl_clientEntryPool->next->previous=NULL;
		}
		rtl_clientEntryPool = rtl_clientEntryPool->next;
		memset(ret, 0, sizeof(struct rtl_clientEntry));
	}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a client entry and link it back to the client entry pool, default is link to the pool head
static  void rtl_freeClientEntry(struct rtl_clientEntry* clientEntryPtr) 
{
	if (!clientEntryPtr)
	{
		return;
	}
		
	rtl_glueMutexLock();	
	clientEntryPtr->next = rtl_clientEntryPool;
	if(rtl_clientEntryPool!=NULL)
	{
		rtl_clientEntryPool->previous=clientEntryPtr;
	}
	rtl_clientEntryPool=clientEntryPtr;	
	rtl_glueMutexUnlock();	
}

/*source entry memory management*/
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
	poolHead = (struct rtl_sourceEntry *)rtl_glueMalloc(sizeof(struct rtl_sourceEntry) * poolSize);
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


// allocate a source entry from the source entry pool
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

// free a source entry and link it back to the source entry pool, default is link to the pool head
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
#ifdef CONFIG_RECORD_MCAST_FLOW
/*multicast flow entry memory management*/
static  struct rtl_mcastFlowEntry* rtl_initMcastFlowEntryPool(uint32 poolSize)
{
	
	uint32 idx=0;
	struct rtl_mcastFlowEntry *poolHead=NULL;
	struct rtl_mcastFlowEntry *entryPtr=NULL;
	
	rtl_glueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_mcastFlowEntry *)rtl_glueMalloc(sizeof(struct rtl_mcastFlowEntry) * poolSize);
	rtl_mcastFlowMemory=(void *)poolHead;
	
	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_mcastFlowEntry)));
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


// allocate a multicast flow entry  from the multicast flow pool
static  struct rtl_mcastFlowEntry* rtl_allocateMcastFlowEntry(void)
{
	struct rtl_mcastFlowEntry *ret = NULL;

	rtl_glueMutexLock();	
		if (rtl_mcastFlowEntryPool!=NULL)
		{
			ret = rtl_mcastFlowEntryPool;
			if(rtl_mcastFlowEntryPool->next!=NULL)
			{
				rtl_mcastFlowEntryPool->next->previous=NULL;
			}
			rtl_mcastFlowEntryPool = rtl_mcastFlowEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_mcastFlowEntry));
		}
		
	rtl_glueMutexUnlock();	
	
	return ret;
}

// free a multicast flow entry and link it back to the multicast flow entry pool, default is link to the pool head
static  void rtl_freeMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry) 
{
	if (NULL==mcastFlowEntry)
	{
		return;
	}
		
	rtl_glueMutexLock();	
	mcastFlowEntry->next = rtl_mcastFlowEntryPool;
	if(rtl_mcastFlowEntryPool!=NULL)
	{
		rtl_mcastFlowEntryPool->previous=mcastFlowEntry;
	}
	rtl_mcastFlowEntryPool=mcastFlowEntry;	
	rtl_glueMutexUnlock();	
}

#endif

/*********************************************
			Group list operation
 *********************************************/

/*       find a group address in a group list    */
struct rtl_groupEntry* rtl_searchGroupEntry(uint32 moduleIndex, uint32 ipVersion,uint32 *multicastAddr)
{
	struct rtl_groupEntry* groupPtr = NULL;
	int32 hashIndex=0;

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, multicastAddr);
	
	if(ipVersion==IP_VERSION4)
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
			groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
			groupPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
	}
#endif	

	while (groupPtr!=NULL)
	{	
		if(ipVersion==IP_VERSION4)
		{
			if(multicastAddr[0]==groupPtr->groupAddr[0])
			{
				return groupPtr;
			}
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
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
#endif	
		groupPtr = groupPtr->next;

	}

	return NULL;
}

int32 rtl_getGroupNum(uint32 ipVersion)
{
	int32 moduleIndex;
	int32 hashIndex;
	int32 groupCnt=0;
	struct rtl_groupEntry *groupEntryPtr;
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
	
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{		
			if(ipVersion ==IP_VERSION4)
			{
				groupCnt=0; 

				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
				{
					for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
					{
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
						while(groupEntryPtr!=NULL)
						{
							if(groupEntryPtr->attribute!=STATIC_RESERVED_MULTICAST)
								groupCnt++;
						
						
						
							groupEntryPtr=groupEntryPtr->next;	
						}
					
				   	}
				}
				
			}
#if defined (CONFIG_RTL_MLD_SNOOPING)			
			else if(ipVersion ==IP_VERSION6)
			{
				groupCnt=0; 

				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
				{
					for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
					{
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
						while(groupEntryPtr!=NULL)
						{
							if(groupEntryPtr->attribute!=STATIC_RESERVED_MULTICAST)
								groupCnt++; 
							
								
							groupEntryPtr=groupEntryPtr->next;	
						}
						
					}
				}
			}
			
#endif			
		}
	}

	return groupCnt;
}

#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
int32 rtl_getGroupNumbyBr(uint32 ipVersion, char *brName)
{
	int32 moduleIndex;
	int32 hashIndex;
	int32 groupCnt=0;
	struct rtl_groupEntry *groupEntryPtr;
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
	
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE && 
		   strcmp(rtl_mCastModuleArray[moduleIndex].deviceInfo.devName, brName)==0)
		{		
			if(ipVersion ==IP_VERSION4)
			{
				groupCnt=0; 

				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
				{
					for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
					{
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
						while(groupEntryPtr!=NULL)
						{
							if(groupEntryPtr->attribute!=STATIC_RESERVED_MULTICAST)
								groupCnt++;
						
						
						
							groupEntryPtr=groupEntryPtr->next;	
						}
					
				   	}
				}
				
			}
#if defined (CONFIG_RTL_MLD_SNOOPING)			
			else if(ipVersion ==IP_VERSION6)
			{
				groupCnt=0; 

				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
				{
					for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
					{
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
						while(groupEntryPtr!=NULL)
						{
							if(groupEntryPtr->attribute!=STATIC_RESERVED_MULTICAST)
								groupCnt++; 
							
								
							groupEntryPtr=groupEntryPtr->next;	
						}
						
					}
				}
			}
			
#endif			
		}
	}

	return groupCnt;
}
#endif


/* link group Entry in the front of a group list */
static void  rtl_linkGroupEntry(struct rtl_groupEntry* groupEntry ,  struct rtl_groupEntry ** hashTable)
{
	uint32 hashIndex=0;
	
	rtl_glueMutexLock();//Lock resource
	hashIndex=rtl_igmpHashAlgorithm(groupEntry->ipVersion, groupEntry->groupAddr);
	if(NULL==groupEntry || hashTable == NULL)
	{
		rtl_glueMutexUnlock();
		return;
	}
	
	if(hashTable[hashIndex]!=NULL)
	{
		hashTable[hashIndex]->previous=groupEntry;
	}
	groupEntry->next = hashTable[hashIndex];
	hashTable[hashIndex]=groupEntry;
	hashTable[hashIndex]->previous=NULL;
		
	rtl_glueMutexUnlock();//UnLock resource

}


/* unlink a group entry from group list */
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* groupEntry,  struct rtl_groupEntry ** hashTable)
{	
	uint32 hashIndex=0;
	
	if(NULL==groupEntry || hashTable==NULL)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	

	hashIndex=rtl_igmpHashAlgorithm(groupEntry->ipVersion, groupEntry->groupAddr);
	/* unlink entry node*/
	if(groupEntry==hashTable[hashIndex]) /*unlink group list head*/
	{
		hashTable[hashIndex]=groupEntry->next;
		if(hashTable[hashIndex]!=NULL)
		{
			hashTable[hashIndex]->previous=NULL;
		}
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
	}
	
	groupEntry->previous=NULL;
	groupEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource
	
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

/*********************************************
			Client list operation
 *********************************************/
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)
static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion, struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr)	
#else
static struct rtl_clientEntry* rtl_searchClientEntry(uint32 ipVersion, struct rtl_groupEntry* groupEntry, uint32 portNum, uint32 *clientAddr)
#endif
{
	struct rtl_clientEntry* clientPtr = groupEntry->clientList;

	if(clientAddr==NULL)
	{
		return NULL;
	}
	while (clientPtr!=NULL)
	{	
		if(ipVersion==IP_VERSION4)
		{
			if((clientPtr->clientAddr[0]==clientAddr[0])
			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)
			||((clientmacAddr[0]==clientPtr->clientMacAddr[0])
			&&(clientmacAddr[1]==clientPtr->clientMacAddr[1])
			&&(clientmacAddr[2]==clientPtr->clientMacAddr[2])
			&&(clientmacAddr[3]==clientPtr->clientMacAddr[3])
			&&(clientmacAddr[4]==clientPtr->clientMacAddr[4])
			&&(clientmacAddr[5]==clientPtr->clientMacAddr[5])
			)
			#endif
			)
			{
				if(portNum<MAX_SUPPORT_PORT_NUMBER) 
				{
					/*update port number,in case of client change port*/
					clientPtr->portNum=portNum;
				}
				return clientPtr;
			}
			
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			if(	((clientPtr->clientAddr[0]==clientAddr[0])
				&&(clientPtr->clientAddr[1]==clientAddr[1])
				&&(clientPtr->clientAddr[2]==clientAddr[2])
				&&(clientPtr->clientAddr[3]==clientAddr[3]))		
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)
			||((clientmacAddr[0]==clientPtr->clientMacAddr[0])
			&&(clientmacAddr[1]==clientPtr->clientMacAddr[1])
			&&(clientmacAddr[2]==clientPtr->clientMacAddr[2])
			&&(clientmacAddr[3]==clientPtr->clientMacAddr[3])
			&&(clientmacAddr[4]==clientPtr->clientMacAddr[4])
			&&(clientmacAddr[5]==clientPtr->clientMacAddr[5])
			)
#endif
			)
			{
		
				if(portNum<MAX_SUPPORT_PORT_NUMBER) 
				{
					/*update port number,in case of client change port*/
					clientPtr->portNum=portNum;
				}
				return clientPtr;
			}
		}
#endif	
		clientPtr = clientPtr->next;

	}

	return NULL;
}

uint32 rtl_getAllClientPortMask(struct rtl_groupEntry* groupEntry)
{
	uint32 clientPortMask=0;

	struct rtl_clientEntry* clientPtr = groupEntry->clientList; 
	
	while (clientPtr!=NULL)
	{	

		clientPortMask |=1<<(clientPtr->portNum);
		clientPtr = clientPtr->next;
	}

	return clientPortMask;
}
/* link client Entry in the front of group client list */
static void  rtl_linkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry )
{
	rtl_glueMutexLock();//Lock resource
	if(NULL==clientEntry)
	{
		return;
	}
	
	if(NULL==groupEntry)
	{
		return;
	}


	if(groupEntry->clientList!=NULL)
	{
		groupEntry->clientList->previous=clientEntry;
	}
	clientEntry->next = groupEntry->clientList;
	
	groupEntry->clientList=clientEntry;
	groupEntry->clientList->previous=NULL;
		
	rtl_glueMutexUnlock();//UnLock resource

}


/* unlink a client entry from group client list */
static void rtl_unlinkClientEntry(struct rtl_groupEntry *groupEntry, struct rtl_clientEntry* clientEntry)
{	
	if(NULL==clientEntry)
	{
		return;
	}
	
	if(NULL==groupEntry)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	
	
	/* unlink entry node*/
	if(clientEntry==groupEntry->clientList) /*unlink group list head*/
	{
		groupEntry->clientList=groupEntry->clientList->next;
		if(groupEntry->clientList!=NULL)
		{
			groupEntry->clientList->previous=NULL;
		}

	}
	else
	{
		if(clientEntry->previous!=NULL)
		{
			clientEntry->previous->next=clientEntry->next;
		}
		 
		if(clientEntry->next!=NULL)
		{
			clientEntry->next->previous=clientEntry->previous;
		}
	}
	
	clientEntry->previous=NULL;
	clientEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource
	
}


/* clear the content of client entry */
static void rtl_clearClientEntry(struct rtl_clientEntry* clientEntry)
{
	rtl_glueMutexLock();
	if (NULL!=clientEntry)
	{
		memset(clientEntry, 0, sizeof(struct rtl_clientEntry));
	}
	rtl_glueMutexUnlock();
}

static uint32 rtl_getClientNum(uint32 moduleIndex,uint32 ipVersion,struct rtl_groupEntry* groupEntry,struct rtl_clientEntry *clientEntry)
{
	uint32 ClientNum=0;
	
	struct rtl_clientEntry* clientPtr=NULL;
	
	if(groupEntry==NULL)
		return 0;
	/*printk("[%s]:[%d]moduleIndex:%d,group:%x,client:%x\n",__FUNCTION__,__LINE__,moduleIndex,
		groupEntry->groupAddr[0],clientEntry->clientAddr[0]);*/
	clientPtr = groupEntry->clientList; 
	
	while (clientPtr!=NULL)
	{	
		if(ipVersion==IP_VERSION4)
		{
			if(clientEntry&&(clientPtr->clientAddr[0]==clientEntry->clientAddr[0]))
			{
				clientPtr = clientPtr->next;
				continue;
			}
			
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			if(clientEntry&&
				((clientPtr->clientAddr[0]==clientEntry->clientAddr[0])
				&&(clientPtr->clientAddr[1]==clientEntry->clientAddr[1])
				&&(clientPtr->clientAddr[2]==clientEntry->clientAddr[2])
				&&(clientPtr->clientAddr[3]==clientEntry->clientAddr[3])))
			{
				clientPtr = clientPtr->next;
				continue;
			}
		}
#endif	
		ClientNum++;
		clientPtr = clientPtr->next;
	}
	
	return ClientNum;
}

/*********************************************
			source list operation
 *********************************************/
static struct rtl_sourceEntry* rtl_searchSourceEntry(uint32 ipVersion, uint32 *sourceAddr, struct rtl_clientEntry *clientEntry)
{
	struct rtl_sourceEntry *sourcePtr=clientEntry->sourceList;
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
		else
		{
			if(	(sourceAddr[0]==sourcePtr->sourceAddr[0])&&
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

#if 0
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
#endif

static void rtl_linkSourceEntry(struct rtl_clientEntry *clientEntry,  struct rtl_sourceEntry* entryNode)
{
	if(NULL==entryNode)
	{
		return;
	}
	
	if(NULL==clientEntry)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	

	if(clientEntry->sourceList!=NULL)
	{
		clientEntry->sourceList->previous=entryNode;
	}
	entryNode->next=clientEntry->sourceList;
	clientEntry->sourceList=entryNode;
	clientEntry->sourceList->previous=NULL;
	
	rtl_glueMutexUnlock();  /* lock resource*/	
}

static void rtl_unlinkSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry)
{
	if(NULL==sourceEntry)
	{
		return;
	}
	
	if(NULL==clientEntry)
	{
		return;
	}
	
	rtl_glueMutexLock();  /* lock resource*/	
	/* unlink entry node*/ 
	if(sourceEntry==clientEntry->sourceList) /*unlink group list head*/
	{

		clientEntry->sourceList=sourceEntry->next;
		if(clientEntry->sourceList!=NULL)
		{
			clientEntry->sourceList ->previous=NULL;
		}
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
	}
	
	sourceEntry->previous=NULL;
	sourceEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource

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

/*********************************************
			multicast flow list operation
 *********************************************/
 
#ifdef CONFIG_RECORD_MCAST_FLOW
static struct rtl_mcastFlowEntry* rtl_searchMcastFlowEntry(uint32 moduleIndex, uint32 ipVersion, uint32 *serverAddr,uint32 *groupAddr)
{
	struct rtl_mcastFlowEntry* mcastFlowPtr = NULL;
	uint32 hashIndex=0;

	if(NULL==serverAddr)
	{
		return NULL;
	}

	if(NULL==groupAddr)
	{
		return NULL;
	}
	
	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddr);

	if(rtl_mCastModuleArray[moduleIndex].flowHashTable != NULL)
		mcastFlowPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

	while (mcastFlowPtr!=NULL)
	{	

		if(mcastFlowPtr->ipVersion!=ipVersion)
		{
			goto nextFlow;
		}
	
		if(ipVersion==IP_VERSION4)
		{
			if( (serverAddr[0]==mcastFlowPtr->serverAddr[0]) && (groupAddr[0]==mcastFlowPtr->groupAddr[0]) )
			{
				mcastFlowPtr->refreshTime=rtl_sysUpSeconds;
				return mcastFlowPtr;
			}
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			
			if(	(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(serverAddr[0]==mcastFlowPtr->serverAddr[0])
				&&(groupAddr[0]==mcastFlowPtr->groupAddr[0])
				&&(groupAddr[1]==mcastFlowPtr->groupAddr[1])
				&&(groupAddr[2]==mcastFlowPtr->groupAddr[2])
				&&(groupAddr[3]==mcastFlowPtr->groupAddr[3]))
			{
				mcastFlowPtr->refreshTime=rtl_sysUpSeconds;
				return mcastFlowPtr;
			}
		}

nextFlow:
#endif	
		mcastFlowPtr = mcastFlowPtr->next;

	}

	return NULL;
}

/* link multicast flow entry in the front of a forwarding flow list */
static void  rtl_linkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry ,  struct rtl_mcastFlowEntry ** hashTable)
{
	uint32 hashIndex=0;
	rtl_glueMutexLock();//Lock resource
	if(NULL==mcastFlowEntry)
	{
		rtl_glueMutexUnlock();
		return;
	}

	if(NULL==hashTable)
	{
		rtl_glueMutexUnlock();
		return;
	}

	hashIndex=rtl_igmpHashAlgorithm(mcastFlowEntry->ipVersion, mcastFlowEntry->groupAddr);

	if(hashTable[hashIndex]!=NULL)
	{
		hashTable[hashIndex]->previous=mcastFlowEntry;
	}
	mcastFlowEntry->next = hashTable[hashIndex];
	hashTable[hashIndex]=mcastFlowEntry;
	hashTable[hashIndex]->previous=NULL;

	rtl_glueMutexUnlock();//UnLock resource
	return;

}

/* unlink a multicast flow entry*/
static void rtl_unlinkMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry,  struct rtl_mcastFlowEntry ** hashTable)
{	 
	uint32 hashIndex=0;
	if(NULL==mcastFlowEntry)
	{
		return;
	}

	hashIndex=rtl_igmpHashAlgorithm(mcastFlowEntry->ipVersion, mcastFlowEntry->groupAddr);

	rtl_glueMutexLock();  /* lock resource*/	
	/* unlink entry node*/
	if(mcastFlowEntry==hashTable[hashIndex]) /*unlink flow list head*/
	{
		hashTable[hashIndex]=mcastFlowEntry->next;
		if(hashTable[hashIndex]!=NULL)
		{
			hashTable[hashIndex]->previous=NULL;
		}
	}
	else
	{
		if(mcastFlowEntry->previous!=NULL)
		{
			mcastFlowEntry->previous->next=mcastFlowEntry->next;
		}
		 
		if(mcastFlowEntry->next!=NULL)
		{
			mcastFlowEntry->next->previous=mcastFlowEntry->previous;
		}
	}
	
	mcastFlowEntry->previous=NULL;
	mcastFlowEntry->next=NULL;
	
	rtl_glueMutexUnlock();//UnLock resource
	
}


/* clear the content of multicast flow entry */
static void rtl_clearMcastFlowEntry(struct rtl_mcastFlowEntry* mcastFlowEntry)
{
	rtl_glueMutexLock();
	if (NULL!=mcastFlowEntry)
	{
		memset(mcastFlowEntry, 0, sizeof(struct rtl_mcastFlowEntry));
	}
	rtl_glueMutexUnlock();
}


static void rtl_deleteMcastFlowEntry( struct rtl_mcastFlowEntry* mcastFlowEntry, struct rtl_mcastFlowEntry ** hashTable)
{	
	if(mcastFlowEntry!=NULL)
	{

		rtl_unlinkMcastFlowEntry(mcastFlowEntry, hashTable);
		rtl_clearMcastFlowEntry(mcastFlowEntry);
		rtl_freeMcastFlowEntry(mcastFlowEntry);
	}

	return;	
}
#endif
	
/*****source entry/client entry/group entry/flow entry operation*****/

static void rtl_deleteSourceEntry(struct rtl_clientEntry *clientEntry, struct rtl_sourceEntry* sourceEntry)
{
	if(clientEntry==NULL)
	{
		return;
	}
	
	if(sourceEntry!=NULL)
	{
		rtl_unlinkSourceEntry(clientEntry,sourceEntry);
		rtl_clearSourceEntry(sourceEntry);
		rtl_freeSourceEntry(sourceEntry);
	}
}
	
static void rtl_deleteSourceList(struct rtl_clientEntry* clientEntry)
{
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	
	sourceEntry=clientEntry->sourceList;
	while(sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		rtl_deleteSourceEntry(clientEntry,sourceEntry);
		sourceEntry=nextSourceEntry;
	}
}

static void rtl_deleteClientEntry(struct rtl_groupEntry* groupEntry,struct rtl_clientEntry *clientEntry)
{	
	if(NULL==clientEntry)
	{
		return;
	}
	
	if(NULL==groupEntry)
	{
		return;
	}

#if defined(CONFIG_RTL_8370_SUPPORT)
	// delete/update 8370 L2 multicast entry here
	if (clientEntry->portNum == 0) {
		rtl8370L2McastAddrUpdate(2, groupEntry->groupAddr[0], clientEntry->clientAddr[0], 
			clientEntry->clientMacAddr);
	}
#endif

	rtl_deleteSourceList(clientEntry);
	rtl_unlinkClientEntry(groupEntry,clientEntry);
	rtl_clearClientEntry(clientEntry);
	rtl_freeClientEntry(clientEntry);
	return;	
		
}

static void rtl_deleteClientList(struct rtl_groupEntry* groupEntry)
{

	struct rtl_clientEntry *clientEntry=NULL;
	struct rtl_clientEntry *nextClientEntry=NULL;

	if(NULL==groupEntry)
	{
		return;
	}
	
	clientEntry=groupEntry->clientList;
	while(clientEntry!=NULL)
	{
		nextClientEntry=clientEntry->next;
		rtl_deleteClientEntry(groupEntry,clientEntry);
		clientEntry=nextClientEntry;
	}
}


static void rtl_deleteGroupEntry( struct rtl_groupEntry* groupEntry,struct rtl_groupEntry ** hashTable)
{	
	if(groupEntry!=NULL)
	{
	
		rtl_deleteClientList(groupEntry);
		rtl_unlinkGroupEntry(groupEntry, hashTable);
		rtl_clearGroupEntry(groupEntry);
		rtl_freeGroupEntry(groupEntry);
		
	}
		
}


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
	else
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


static uint32 rtl_getClientFwdPortMask(struct rtl_clientEntry * clientEntry, uint32 sysTime)
{

	uint32 portMask=(1<<clientEntry->portNum);
	uint32 fwdPortMask=0;
	
	struct rtl_sourceEntry * sourcePtr=NULL;;
	
	if(clientEntry->groupFilterTimer>sysTime) /*exclude mode never expired*/
	{
		fwdPortMask|=portMask;
	}
	else/*include mode*/
	{
		sourcePtr=clientEntry->sourceList;
		while(sourcePtr!=NULL)
		{
			if(sourcePtr->portTimer>sysTime)
			{
				fwdPortMask|=portMask;
				break;
			}
			sourcePtr=sourcePtr->next;
		
		}
		
	}

	return fwdPortMask;
}

static void rtl_checkSourceTimer(struct rtl_clientEntry * clientEntry , struct rtl_sourceEntry * sourceEntry)
{
	uint8 deleteFlag=FALSE;
	uint8 oldFwdState,newFwdState;

	oldFwdState=sourceEntry->fwdState;
	
	if(sourceEntry->portTimer<=rtl_sysUpSeconds) /*means time out*/
	{
		if(clientEntry->groupFilterTimer<=rtl_sysUpSeconds) /* include mode*/
		{
			deleteFlag=TRUE;
		}
	
		sourceEntry->fwdState=FALSE;
	}
	else
	{
		deleteFlag=FALSE;
		sourceEntry->fwdState=TRUE;
	}
	
	newFwdState=sourceEntry->fwdState;
	
	if(deleteFlag==TRUE) /*means INCLUDE mode and expired*/
	{
		rtl_deleteSourceEntry(clientEntry,sourceEntry);
	}

	if((deleteFlag==TRUE) || (newFwdState!=oldFwdState))
	{
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(timerEventContext.moduleIndex, timerEventContext.ipVersion, timerEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(timerEventContext.ipVersion==IP_VERSION4)
		{
#ifdef CONFIG_PROC_FS
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &timerEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
			rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &timerEventContext);
#endif

		}
		#endif
	}

}

static uint32 rtl_getClientSourceFwdPortMask(uint32 ipVersion, struct rtl_clientEntry * clientEntry,uint32 *sourceAddr, uint32 sysTime)
{
	uint32 portMask=(1<<clientEntry->portNum);
	uint32 fwdPortMask=0;
	struct rtl_sourceEntry * sourceEntry=NULL;
	if(clientEntry==NULL)
	{
		return 0xFFFFFFFF; /*broadcast*/
	}
	else
	{
		sourceEntry=rtl_searchSourceEntry(ipVersion,sourceAddr, clientEntry);
	
		if(clientEntry->groupFilterTimer<=sysTime)	/*include mode*/
		{	
			if(sourceEntry!=NULL)
			{
				if( sourceEntry->portTimer>sysTime)
				{
					fwdPortMask|=portMask;
				}
			}
		}
		else/*exclude mode*/
		{	
			if(sourceEntry==NULL)
			{
				fwdPortMask|=portMask;
			}
			else
			{
				if(sourceEntry->portTimer>sysTime)
				{
					fwdPortMask|=portMask;
				}
			}
		}
		
		return fwdPortMask;
		
	}
}

static uint32 rtl_getGroupSourceFwdPortMask(struct rtl_groupEntry * groupEntry,uint32 *sourceAddr, uint32 sysTime)
{
	uint32 fwdPortMask=0;
	struct rtl_clientEntry * clientEntry=NULL;
#if defined(CONFIG_SMP)
	unsigned long flags=0;
#endif

	if(groupEntry==NULL)
	{
		return 0xFFFFFFFF; /*broadcast*/
	}

	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if(groupEntry->attribute==STATIC_RESERVED_MULTICAST)
	{
		return groupEntry->staticFwdPortMask;
	}
	#endif

	SMP_LOCK_IGMP(flags);

	clientEntry=groupEntry->clientList;
	while(clientEntry!=NULL)
	{
		fwdPortMask|= rtl_getClientSourceFwdPortMask(groupEntry->ipVersion, clientEntry, sourceAddr, sysTime);
		clientEntry=clientEntry->next;
	}

	SMP_UNLOCK_IGMP(flags);
		
	return fwdPortMask;
}


static void rtl_checkClientEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_clientEntry * clientEntry)
{
	uint32 oldFwdPortMask=0;
	uint32 newFwdPortMask=0;
	struct rtl_sourceEntry *sourceEntry=clientEntry->sourceList;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	
	oldFwdPortMask=rtl_getClientFwdPortMask(clientEntry, rtl_sysUpSeconds);
	
	while(sourceEntry!=NULL)
	{
		nextSourceEntry=sourceEntry->next;
		rtl_checkSourceTimer(clientEntry, sourceEntry);
		sourceEntry=nextSourceEntry;
	}
	
	newFwdPortMask=rtl_getClientFwdPortMask(clientEntry, rtl_sysUpSeconds);


	if(newFwdPortMask==0) /*none active port*/
	{
		rtl_deleteClientEntry(groupEntry,clientEntry);	
	}

	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return;
	}
	#endif	

	if((oldFwdPortMask!=newFwdPortMask) || (newFwdPortMask==0))
	{	
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(timerEventContext.moduleIndex, timerEventContext.ipVersion, timerEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(timerEventContext.ipVersion==IP_VERSION4)
		{
#ifdef CONFIG_PROC_FS
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &timerEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
			rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &timerEventContext);
#endif

		}
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		if(timerEventContext.ipVersion==IP_VERSION6)
		{
#ifdef CONFIG_PROC_FS
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST6,&timerEventContext);
		}
		#endif
		#endif
		
	}

}

static void rtl_checkGroupEntryTimer(struct rtl_groupEntry * groupEntry, struct rtl_groupEntry ** hashTable)
{
	uint32 deleteFlag=FALSE;
	struct rtl_clientEntry *clientEntry=groupEntry->clientList;
	struct rtl_clientEntry *nextClientEntry=NULL;
	
		
	while(clientEntry!=NULL)
	{
		nextClientEntry=clientEntry->next;
		timerEventContext.portMask=1<<(clientEntry->portNum);
		rtl_checkClientEntryTimer(groupEntry, clientEntry);
		clientEntry=nextClientEntry;
	}

	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return;
	}
	#endif	

	if(groupEntry->clientList==NULL) /*none active client*/
	{
		deleteFlag=TRUE;
		rtl_deleteGroupEntry(groupEntry,hashTable);	
	}

	if(deleteFlag==TRUE)
	{	
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(timerEventContext.moduleIndex, timerEventContext.ipVersion, timerEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(timerEventContext.ipVersion==IP_VERSION4)
		{
#ifdef CONFIG_PROC_FS		
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &timerEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
			rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &timerEventContext);
#endif

		}
		#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		if(timerEventContext.ipVersion==IP_VERSION6)
		{
#ifdef CONFIG_PROC_FS		
			rtl_mCastModuleArray[timerEventContext.moduleIndex].expireEventCnt++;
#endif
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &timerEventContext);
		}
		#endif
		#endif
		
	}

}



static int32 rtl_initHashTable(uint32 moduleIndex, uint32 hashTableSize)
{
	uint32 i=0;
	
	/* Allocate memory */
	rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable = (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);

	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!= NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	
			rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i]=NULL;
		}

	}
	else
	{
		return FAILED;
	}
	
#ifdef CONFIG_RTL_MLD_SNOOPING	
	rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable=  (struct rtl_groupEntry **)rtl_glueMalloc(4 * hashTableSize);
	if (rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	

			rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i]=NULL;
		}
	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		
		return FAILED;

	}
#endif

	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_mCastModuleArray[moduleIndex].flowHashTable= (struct rtl_mcastFlowEntry **)rtl_glueMalloc(4 * hashTableSize);
	
	if (rtl_mCastModuleArray[moduleIndex].flowHashTable!=NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	

			rtl_mCastModuleArray[moduleIndex].flowHashTable[i]=NULL;
		}
	}
	else
	{
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}

#ifdef CONFIG_RTL_MLD_SNOOPING	
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable!=NULL)
		{
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif		
		return FAILED;


	}
	#endif
return SUCCESS;

}



/**************************
	Utility
**************************/
static void  rtl_parseMacFrame(uint32 moduleIndex, uint8* macFrame, uint32 verifyCheckSum, struct rtl_macFrameInfo* macInfo) 
{
	
//MAC Frame :DA(6 bytes)+SA(6 bytes)+ CPU tag(4 bytes) + VlAN tag(Optional, 4 bytes)
//                   +Type(IPv4:0x0800, IPV6:0x86DD, PPPOE:0x8864, 2 bytes )+Data(46~1500 bytes)+CRC(4 bytes)

	uint8 *ptr=macFrame;

#ifdef CONFIG_RTL_MLD_SNOOPING	
	int i=0;
	uint8 nextHeader=0;
	uint16 extensionHdrLen=0;
	uint8 routingHead=FALSE;
	
	uint8 optionDataLen=0;
	uint8 optionType=0;
	uint32 ipv6RAO=0;
#endif

#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
	int is_pppoe = 0;	
#endif

	uint32 ipAddr[4]={0,0,0,0};
	union pseudoHeader pHeader;
	
	memset(macInfo,0,sizeof(struct rtl_macFrameInfo));
	memset(&pHeader, 0, sizeof(union pseudoHeader));

	memcpy(macInfo->srcMacAddr, ptr+6,6);
	
	ptr=ptr+12;


	/*check the presence of VLAN tag*/	
	if(*(int16 *)(ptr)==(int16)htons(VLAN_PROTOCOL_ID))
	{
		ptr=ptr+4;
	}

	/*ignore packet with PPPOE header*/	
	if(*(int16 *)(ptr)==(int16)htons(PPPOE_ETHER_TYPE))
	{
	#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
		ptr = ptr+2+6;
		is_pppoe = 1;
	#else
		return;	
	#endif
	}

	
	/*check the presence of ipv4 type*/
	#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
	if((*(int16 *)(ptr)==(int16)htons(IPV4_ETHER_TYPE)) ||
		((is_pppoe == 1) && (*(int16 *)(ptr)==(int16)htons(PPP_IPV4_PROTOCOL))))
	#else
	if(*(int16 *)(ptr)==(int16)htons(IPV4_ETHER_TYPE))
	#endif
	{
		ptr=ptr+2;
		macInfo->ipBuf=ptr;
		macInfo->ipVersion=IP_VERSION4;
	}
	else
	{
		/*check the presence of ipv4 type*/
	#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
		if((*(int16 *)(ptr)==(int16)htons(IPV6_ETHER_TYPE))||
		  ((is_pppoe == 1) && (*(int16 *)(ptr)==(int16)htons(PPP_IPV6_PROTOCOL))))

	#else
		if(*(int16 *)(ptr)==(int16)htons(IPV6_ETHER_TYPE))
	#endif
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
		macInfo->srcIpAddr[0]=ntohl(((struct ipv4Pkt *)(macInfo->ipBuf))->sourceIp);
		macInfo->dstIpAddr[0]=ntohl(((struct ipv4Pkt *)(macInfo->ipBuf))->destinationIp);
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
				#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
						else if(is_pppoe == 1)
						{
							macInfo->l3Protocol=IGMP_PROTOCOL;
						}
				#endif
			      									
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
		macInfo->srcIpAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[0]);
		macInfo->srcIpAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[1]);
		macInfo->srcIpAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[2]);
		macInfo->srcIpAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[3]);
		
		macInfo->dstIpAddr[0]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[0]);
		macInfo->dstIpAddr[1]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[1]);
		macInfo->dstIpAddr[2]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[2]);
		macInfo->dstIpAddr[3]=ntohl(((struct ipv6Pkt *)(macInfo->ipBuf))->destinationAddr[3]);
		
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
					     routingHead=TRUE;
					     
					     
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

								#if defined(CONFIG_RTL_PROCESS_PPPOE_IGMP_FOR_BRIDGE_FORWARD)
								else if(is_pppoe == 1)
								{
									macInfo->l3Protocol=ICMP_PROTOCOL;
								}
								#endif
							
						}

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

				break;
				
				default:		
					goto out;
				break;
			}
		
		}

out:
		
		
       	if(verifyCheckSum==TRUE)
       	{
       		if(macInfo->l3PktBuf==NULL)
       		{
				return;	
			}
			
       		/*generate pseudo header*/
		       for(i=0; i<4; i++)
		      {
			     pHeader.ipv6_pHdr.sourceAddr[i]=((struct ipv6Pkt *)(macInfo->ipBuf))->sourceAddr[i];
			     
		       }
           
		      if(routingHead==FALSE)
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

			/*verify checksum*/
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
			macInfo->checksumFlag=SUCCESS;
		}
       }	
#endif
	return;
}


static uint32  rtl_getMulticastRouterPortMask(uint32 moduleIndex, uint32 ipVersion, uint32 sysTime)
{
	uint32 portIndex=0;
	uint8 portMaskn=PORT0_MASK;
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
	else
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
	#if 0
	struct rtl_groupEntry *groupEntry=NULL;
	struct rtl_clientEntry * clientEntry=NULL;
	struct rtl_sourceEntry*sourceEntry=NULL;
	uint32 i=0;
	#endif
	uint32 groupAddress[4]={0,0,0,0};
	uint32 suppressFlag=0;
	uint32 *sourceAddr=NULL;
	uint32 numOfSrc=0;
	
	
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
		//	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);
		}
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
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
		
		if(	(groupAddress[0]==0)&& 
			(groupAddress[1]==0)&&
			(groupAddress[2]==0)&&
			(groupAddress[3]==0)	)/*means general query*/
		{
			goto out;
		}
		else
		{
		//	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);
		}
	}
#endif	
	if(suppressFlag==0)
	{
		/*ignore group specific query handling*/
		#if 0
		groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
		if((groupEntry!=NULL))
		{	
			
			if(numOfSrc==0) /*means group specific query*/
			{
				clientEntry=groupEntry->clientList;
				while(clientEntry!=NULL)
				{
					if(clientEntry->groupFilterTimer>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
					{
						clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}	
					clientEntry=clientEntry->next;
				}
				
			}
			else /*means group and source specific query*/
			{
				clientEntry=groupEntry->clientList;
				while(clientEntry!=NULL)
				{
					for(i=0; i<numOfSrc; i++)
					{	
						
						sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr, clientEntry);
						
						if(sourceEntry!=NULL)
						{
							if(sourceEntry->portTimer>(rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime))
							{
								sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
							}
							
						}

						if(ipVersion==IP_VERSION4)
						{
							sourceAddr++;
						}
#ifdef CONFIG_RTL_MLD_SNOOPING
						else
						{
							sourceAddr=sourceAddr+4;
						}
#endif
					}
					
					clientEntry=clientEntry->next;
				}
				
			}
		}
		#endif
		
	}
	
	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif

	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
	}
	#endif
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
	else
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.querier.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.querierPresentInterval;/*update timer value*/
	}
#endif	
	return;
}
int rtl_is_reserve_multicastAddr(uint32 *groupAddress,uint32 ipVersion)
{
	int ret=FAILED;
	if(ipVersion==IP_VERSION4)
	{	
		/*upnp:239.255.255.250
		mdns:224.0.0.251
		iapp:224.0.1.178*/
		if(((groupAddress[0] & 0xFFFFFFFF)==0xEFFFFFFA)
			||((groupAddress[0] & 0xFFFFFFFF)==0xE00001B2)
			||((groupAddress[0] & 0xFFFFFFFF)==0xE00000FB))
		{
			ret=SUCCESS;
		}
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else if (ipVersion==IP_VERSION6)
	{
		/*ff02::1:FFxx:xxxx	
		MDNS:ff02::fb
		UPNP: ff02::c */
		if(
			(((groupAddress[0] & 0xFFFFFFFF)==0xFF020000)
			&&((groupAddress[1] & 0xFFFFFFFF)==0x0)
			&&((groupAddress[2] & 0xFFFFFFFF)==0x1)
			&&((groupAddress[3] & 0xFF000000)==0xFF000000))
			||(((groupAddress[0] & 0xFFFFFFFF)==0xFF020000)
			&&((groupAddress[1] & 0xFFFFFFFF)==0x00000000)
			&&((groupAddress[2] & 0xFFFFFFFF)==0x00000000)
			&&((groupAddress[3] & 0xFFFFFFFF)==0x000000FB))
			||(((groupAddress[0] & 0xFFFFFFFF)==0xFF020000)
			&&((groupAddress[1] & 0xFFFFFFFF)==0x00000000)
			&&((groupAddress[2] & 0xFFFFFFFF)==0x00000000)
			&&((groupAddress[3] & 0xFFFFFFFF)==0x0000000C))
		)
		{
			//diag_printf("reverve mulitcast:%x:%x:%x:%x\n",groupAddress[0],groupAddress[1],groupAddress[2],groupAddress[3]);
			ret=SUCCESS;
		}
	}
#endif
	return ret;
}


/*Process Report Packet*/
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)	|| defined (M2U_DELETE_CHECK)
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else
static  uint32 rtl_processJoin(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	
	
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;

	uint32 hashIndex=0;
	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	uint32 allClientPortMask=0;
#if defined(CONFIG_SMP)
	unsigned long flags = 0;
#endif	
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
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		
		groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);
	}
#endif	
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		goto out;
	
	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			goto out;
		}

		assert(newGroupEntry->clientList==NULL);
#ifdef CONFIG_RTL_MLD_SNOOPING
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

		newGroupEntry->ipVersion=ipVersion;
	
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif			
		groupEntry=newGroupEntry;
		
	}
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
#else	
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
#endif

#ifdef CONFIG_RTL_8370_SUPPORT
	// www.iana.org/assignments/multicast-addresses/multicast-addresses.xhtml
	#define RESERVED_MC_IP1		0xE00000FC	// Link-local Multicast Name Resolution	[RFC4795]
	if ((portNum == 0)) {
		if (groupAddress[0] != RESERVED_MC_IP1) {
			wait_mac = 1;
			saved_groupAddress = groupAddress[0];
			saved_clientAddr = *clientAddr;
		}
	}
#endif

	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			goto out;
		}
		
		assert(newClientEntry->sourceList==NULL);
		newClientEntry->portNum=portNum;
		newClientEntry->igmpVersion=IGMP_V2;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif
	#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
		newClientEntry->clientMacAddr[0]=clientmacAddr[0];
		newClientEntry->clientMacAddr[1]=clientmacAddr[1];
		newClientEntry->clientMacAddr[2]=clientmacAddr[2];
		newClientEntry->clientMacAddr[3]=clientmacAddr[3];
		newClientEntry->clientMacAddr[4]=clientmacAddr[4];
		newClientEntry->clientMacAddr[5]=clientmacAddr[5];
	#endif
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}
	

	//rtl_deleteSourceList(clientEntry);
	clientEntry->igmpVersion=IGMP_V2;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;	

	SMP_LOCK_IGMP(flags);
	rtl_deleteSourceList(clientEntry);
	SMP_UNLOCK_IGMP(flags);

	allClientPortMask=rtl_getAllClientPortMask( groupEntry);
#ifdef CONFIG_RTK_MESH   
#if defined(CONFIG_RTL_MESH_SINGLE_IFACE)
	if(mesh_port_no != 0xFFFFFFFF)
		allClientPortMask &= (~(1<<mesh_port_no)); /*always forward to wlan-msh*/
#else	//!CONFIG_RTL_MESH_SINGLE_IFACE
	if(mesh_port_no != 0x0)
		allClientPortMask |= mesh_port_no; /*always forward to wlan-msh*/
#endif	//CONFIG_RTL_MESH_SINGLE_IFACE
#endif	//CONFIG_RTK_MESH
		
	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		goto out;
	}
	#endif

	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
	}
	#endif
	#endif

out:
		
	return (((~allClientPortMask)| multicastRouterPortMask) & (~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	//return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else
static  uint32 rtl_processLeave(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_clientEntry *clientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;

	uint32 hashIndex=0;
	uint32 ClientNum=0;
//	uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct igmpv2Pkt *)pktBuf)->groupAddr);
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		groupAddress[0]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mldv1Pkt *)pktBuf)->mCastAddr[3]);
	}
#endif	
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		goto out;
	
	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	
	if(groupEntry!=NULL)
	{   
	#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
		clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
	#else
		clientEntry=rtl_searchClientEntry( ipVersion, groupEntry, portNum, clientAddr);
	#endif
		if(clientEntry!=NULL) 
		{

			if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
			{
				rtl_deleteClientEntry(groupEntry, clientEntry);
			}
			else
			{
				while(sourceEntry!=NULL)
				{
					nextSourceEntry=sourceEntry->next;
					if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
					{
						sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
					sourceEntry=nextSourceEntry;
				}
				
				if(clientEntry->groupFilterTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
				{
					clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
				}
			
			}

		}
		
	}	

	ClientNum=rtl_getClientNum(moduleIndex,ipVersion,groupEntry,clientEntry);
	
	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		goto out;
	}
	#endif
	
	reportEventContext.ipVersion=ipVersion;

#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
#else
	reportEventContext.groupAddr[0]=groupAddress[0];
#endif
	
	if((groupEntry!=NULL) && (groupEntry->clientList==NULL))
	{	
		if(ipVersion==IP_VERSION4)
			rtl_deleteGroupEntry(groupEntry,rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);	
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else if(ipVersion==IP_VERSION6)	
			rtl_deleteGroupEntry(groupEntry,rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);	
		#endif
	}
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
			rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		}

	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
		}

	}
	#endif
	#endif

#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
out:	
#endif
    #if 0
	/*no client exist, send leave*/
	if (ClientNum==0){
		//printk("no client exist.\n");
		return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	}
	else{
		//printk("exist clinetNum :%d\n",ClientNum);
		return 0;
	}	
	//return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
    #endif
    return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)	|| defined (M2U_DELETE_CHECK)
static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else
static  int32 rtl_processIsInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
#endif
{

	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	uint32 srcAddress[4] = {0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	
	uint32 hashIndex=0;

	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;
	uint32 *sourceAddrHead = NULL;
#if defined(CONFIG_SMP)
	unsigned long flags=0;
#endif
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddrHead=&(((struct groupRecord *)pktBuf)->srcList);

	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
	{
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddrHead=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif	
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		return FAILED;
	
	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);
	
	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

		assert(newGroupEntry->clientList==NULL);
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
		
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif
		groupEntry=newGroupEntry;
	}
	
	/*from here groupEntry is the same as newGroupEntry*/
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
#else
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
#endif
	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}
		
		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif
		
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)  
		newClientEntry->clientMacAddr[0]=clientmacAddr[0];
		newClientEntry->clientMacAddr[1]=clientmacAddr[1];
		newClientEntry->clientMacAddr[2]=clientmacAddr[2];
		newClientEntry->clientMacAddr[3]=clientmacAddr[3];
		newClientEntry->clientMacAddr[4]=clientmacAddr[4];
		newClientEntry->clientMacAddr[5]=clientmacAddr[5];
#endif
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}

	/*from here client entry is the same as the newClientEntry*/
	//rtl_deleteSourceList(clientEntry);
	clientEntry->igmpVersion=IGMP_V3;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds;//include mode
	//assert(clientEntry->sourceList==NULL);

	SMP_LOCK_IGMP(flags);	
	/*here to handle the source list*/
	/*add src in igmp/mld to sourcelist*/
	
	sourceAddr = sourceAddrHead;
	for(j=0; j<numOfSrc; j++)
	{
		if(ipVersion==IP_VERSION4)
		{	
			srcAddress[0] = ntohl(sourceAddr[0]);
		}
	#ifdef CONFIG_RTL_MLD_SNOOPING
		else if(ipVersion==IP_VERSION6)
		{	
			srcAddress[0] = ntohl(sourceAddr[0]);
			srcAddress[1] = ntohl(sourceAddr[1]);
			srcAddress[2] = ntohl(sourceAddr[2]);
			srcAddress[3] = ntohl(sourceAddr[3]);
		}
	#endif
		
		sourceEntry=rtl_searchSourceEntry(ipVersion, srcAddress,clientEntry);
		if(sourceEntry==NULL)
		{
			newSourceEntry=rtl_allocateSourceEntry();
			if(newSourceEntry==NULL)
			{
				rtl_gluePrintf("run out of source entry!\n");
				SMP_UNLOCK_IGMP(flags);
				return FAILED;
			}
	
			if(ipVersion==IP_VERSION4)
			{	
				newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
				//sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{	
				newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
				newSourceEntry->sourceAddr[1]= ntohl(sourceAddr[1]);
				newSourceEntry->sourceAddr[2]= ntohl(sourceAddr[2]);
				newSourceEntry->sourceAddr[3]= ntohl(sourceAddr[3]);
				//sourceAddr=sourceAddr+4;
			}
#endif	
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			rtl_linkSourceEntry(clientEntry,newSourceEntry);

		}
		else
		{
			
			/*just update source timer*/
			sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;		
		}

		if(ipVersion==IP_VERSION4)
		{	
			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			sourceAddr=sourceAddr+4;
		}
#endif	
	}

	/*handle source not in igmp/mld*/
	sourceEntry = clientEntry->sourceList;
	while(sourceEntry!=NULL)
	{
		sourceAddr = sourceAddrHead;
		nextSourceEntry = sourceEntry->next;
		for(j=0; j<numOfSrc; j++)
		{
			if(ipVersion == IP_VERSION4)
			{
				if(ntohl(sourceAddr[0])==sourceEntry->sourceAddr[0])
					break;
				sourceAddr++;
			}
			else
			{
				if(	(ntohl(sourceAddr[0])==sourceEntry->sourceAddr[0])&&
					(ntohl(sourceAddr[1])==sourceEntry->sourceAddr[1])&&
					(ntohl(sourceAddr[2])==sourceEntry->sourceAddr[2])&&
					(ntohl(sourceAddr[3])==sourceEntry->sourceAddr[3]))
					break;
				sourceAddr += 4;
			}
				
		}
		if(j==numOfSrc)
		{
			/*change portTimer of src not in igmp/mld to lastMemberAging Time*/
			if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				rtl_deleteSourceEntry(clientEntry,sourceEntry);
			else
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
			
		}			
		sourceEntry=nextSourceEntry;
	}
	SMP_UNLOCK_IGMP(flags);


	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return SUCCESS;
	}
	#endif

	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif

	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
	}

	#endif
	#endif
	


	return SUCCESS;
}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else
static  int32 rtl_processIsExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceArray=NULL;
	uint32 *sourceAddr=NULL;

#if defined (CONFIG_SMP)
	unsigned long flags = 0;
#endif

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct groupRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		return FAILED;
	
	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}
	
		assert(newGroupEntry->clientList==NULL);
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
		
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif
		groupEntry=newGroupEntry;
	}
	
	/*from here groupEntry is the same as  newGroupEntry*/
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
#else	
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
#endif
	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}

		assert(newClientEntry->sourceList==NULL);
		
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif	
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
		newClientEntry->clientMacAddr[0]=clientmacAddr[0];
		newClientEntry->clientMacAddr[1]=clientmacAddr[1];
		newClientEntry->clientMacAddr[2]=clientmacAddr[2];
		newClientEntry->clientMacAddr[3]=clientmacAddr[3];
		newClientEntry->clientMacAddr[4]=clientmacAddr[4];
		newClientEntry->clientMacAddr[5]=clientmacAddr[5];
#endif
		
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;

	}
	
	/*from here clientEntry  is the same as newClientEntry*/
	
	/*set exclude mode first*/
	//rtl_deleteSourceList( clientEntry);
	clientEntry->igmpVersion=IGMP_V3;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
	//assert(clientEntry->sourceList==NULL);

	/*flush the old source list*/
	SMP_LOCK_IGMP(flags);
	rtl_deleteSourceList( clientEntry);
	
	/*link the new source list*/
	for(j=0; j<numOfSrc; j++)
	{
		newSourceEntry=rtl_allocateSourceEntry();
		if(newSourceEntry==NULL)
		{
			rtl_gluePrintf("run out of source entry!\n");
			SMP_UNLOCK_IGMP(flags);
			return FAILED;
		}

		if(ipVersion==IP_VERSION4)
		{	
			newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);

			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{	
			newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
			newSourceEntry->sourceAddr[1]= ntohl(sourceAddr[1]);
			newSourceEntry->sourceAddr[2]= ntohl(sourceAddr[2]);
			newSourceEntry->sourceAddr[3]= ntohl(sourceAddr[3]);

			sourceAddr=sourceAddr+4;
		}
#endif						
		/*time out the sources included in the MODE_IS_EXCLUDE report*/
		newSourceEntry->portTimer=rtl_sysUpSeconds;
		rtl_linkSourceEntry(clientEntry,newSourceEntry);
		
	}
	SMP_UNLOCK_IGMP(flags);

	
	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return SUCCESS;
	}
	#endif

	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		
	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
		
	}

	#endif
	#endif
	
	return SUCCESS;

}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static  int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else
static int32 rtl_processToInclude(uint32 moduleIndex, uint32 ipVersion,  uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *nextSourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;
	uint32 *sourceAddrHead=NULL;
	uint32 ClientNum=0;

	uint32 sourceAddr_host[4] = {0,0,0,0};
#if defined (CONFIG_SMP)
	unsigned long flags = 0;
#endif

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddrHead=&(((struct groupRecord *)pktBuf)->srcList);
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddrHead=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	
	}
#endif
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		goto OUT;
	
	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{	
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

#ifdef CONFIG_RTL_MLD_SNOOPING	
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif

		newGroupEntry->ipVersion=ipVersion;
	
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif			
		groupEntry=newGroupEntry;
	}

	/*from here groupEntry is the same as newGroupEntry*/
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
#else	
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
#endif	
	if(clientEntry==NULL)
	{
		
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}
		
		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif
		
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
		newClientEntry->clientMacAddr[0]=clientmacAddr[0];
		newClientEntry->clientMacAddr[1]=clientmacAddr[1];
		newClientEntry->clientMacAddr[2]=clientmacAddr[2];
		newClientEntry->clientMacAddr[3]=clientmacAddr[3];
		newClientEntry->clientMacAddr[4]=clientmacAddr[4];
		newClientEntry->clientMacAddr[5]=clientmacAddr[5];
#endif
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}
	
	/*here to handle the source list*/
	
	clientEntry->igmpVersion=IGMP_V3;
	
	if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
	{
		clientEntry->groupFilterTimer=rtl_sysUpSeconds;
	#if 0
		rtl_deleteSourceList(clientEntry);
		/*link the new source list*/
		for(j=0; j<numOfSrc; j++)
		{

			newSourceEntry=rtl_allocateSourceEntry();
			if(newSourceEntry==NULL)
			{
				rtl_gluePrintf("run out of source entry!\n");
				return FAILED;
			}

			if(ipVersion==IP_VERSION4)
			{	
				newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{	
				newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
				newSourceEntry->sourceAddr[1]= ntohl(sourceAddr[1]);
				newSourceEntry->sourceAddr[2]= ntohl(sourceAddr[2]);
				newSourceEntry->sourceAddr[3]= ntohl(sourceAddr[3]);

				sourceAddr=sourceAddr+4;
			}
#endif
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			rtl_linkSourceEntry(clientEntry,newSourceEntry);
		}
#else
		/*add/modify source address in igmp/mld to source list*/
		SMP_LOCK_IGMP(flags);
		sourceAddr = sourceAddrHead;
		for(j=0; j<numOfSrc; j++)
		{
			if(ipVersion==IP_VERSION4)
			{	
				sourceAddr_host[0] = ntohl(sourceAddr[0]);
			}
		#ifdef CONFIG_RTL_MLD_SNOOPING
			else if(ipVersion==IP_VERSION6)
			{	
				sourceAddr_host[0] = ntohl(sourceAddr[0]);
				sourceAddr_host[1] = ntohl(sourceAddr[1]);
				sourceAddr_host[2] = ntohl(sourceAddr[2]);
				sourceAddr_host[3] = ntohl(sourceAddr[3]);
			}
		#endif
			sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr_host, clientEntry);
			if(sourceEntry!=NULL)
			{
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
			else
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if(newSourceEntry==NULL)
				{
					rtl_gluePrintf("run out of source entry!\n");
					SMP_UNLOCK_IGMP(flags);
					return FAILED;
				}
					
				if(ipVersion==IP_VERSION4)
				{	
					newSourceEntry->sourceAddr[0]=sourceAddr_host[0];
					
				}
#ifdef CONFIG_RTL_MLD_SNOOPING
				else
				{	
					newSourceEntry->sourceAddr[0]=sourceAddr_host[0];
					newSourceEntry->sourceAddr[1]=sourceAddr_host[1];
					newSourceEntry->sourceAddr[2]=sourceAddr_host[2];
					newSourceEntry->sourceAddr[3]=sourceAddr_host[3];
				}
#endif
				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}
			
			if(ipVersion==IP_VERSION4)
			{	
				
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{	
				sourceAddr=sourceAddr+4;
			}
#endif
		}
		//delete source not in igmp/mld but in old source list
		sourceEntry = clientEntry->sourceList;
		while(sourceEntry!=NULL)
		{
			nextSourceEntry=sourceEntry->next;
			sourceAddr = sourceAddrHead;
			for(j=0; j<numOfSrc; j++)
			{
				if(ipVersion == IP_VERSION4)
				{
					if(ntohl(sourceAddr[0])==sourceEntry->sourceAddr[0])
						break;
					sourceAddr++;
				}
				else
				{
					if( (ntohl(sourceAddr[0])==sourceEntry->sourceAddr[0])&&
						(ntohl(sourceAddr[1])==sourceEntry->sourceAddr[1])&&
						(ntohl(sourceAddr[2])==sourceEntry->sourceAddr[2])&&
						(ntohl(sourceAddr[3])==sourceEntry->sourceAddr[3]))
						break;
					sourceAddr += 4;
				}
					
			}
			if(j==numOfSrc)//not found
				rtl_deleteSourceEntry(clientEntry,sourceEntry);
		
			sourceEntry=nextSourceEntry;
		}
		SMP_UNLOCK_IGMP(flags);

#endif
			
	}
	else
	{
		//add new source list first, then modify old source list time out
		#if 0
		while(sourceEntry!=NULL)
		{
			nextSourceEntry=sourceEntry->next;
			if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
			{
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
			}
			sourceEntry=nextSourceEntry;
		}
		#endif
		
		/*add  new source list*/
		sourceAddr = sourceAddrHead;
		for(j=0; j<numOfSrc; j++)
		{
			if(ipVersion==IP_VERSION4)
			{	
				sourceAddr_host[0] = ntohl(sourceAddr[0]);
			}
		#ifdef CONFIG_RTL_MLD_SNOOPING
			else if(ipVersion==IP_VERSION6)
			{	
				sourceAddr_host[0] = ntohl(sourceAddr[0]);
				sourceAddr_host[1] = ntohl(sourceAddr[1]);
				sourceAddr_host[2] = ntohl(sourceAddr[2]);
				sourceAddr_host[3] = ntohl(sourceAddr[3]);
			}
		#endif
			sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr_host, clientEntry);
			if(sourceEntry!=NULL)
			{
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			}
			else
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if(newSourceEntry==NULL)
				{
					rtl_gluePrintf("run out of source entry!\n");
					return FAILED;
				}
					
				if(ipVersion==IP_VERSION4)
				{					
					newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
					
				}
#ifdef CONFIG_RTL_MLD_SNOOPING
				else
				{	
					newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
					newSourceEntry->sourceAddr[1]= ntohl(sourceAddr[1]);
					newSourceEntry->sourceAddr[2]= ntohl(sourceAddr[2]);
					newSourceEntry->sourceAddr[3]= ntohl(sourceAddr[3]);
				}
#endif
				newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}
			
			if(ipVersion==IP_VERSION4)
			{	
				
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{	

				sourceAddr=sourceAddr+4;
			}
#endif
		}

		//modify old sourcelist time out
		sourceEntry = clientEntry->sourceList;
		while(sourceEntry!=NULL)
		{
			nextSourceEntry=sourceEntry->next;
			sourceAddr = sourceAddrHead;
			for(j=0; j<numOfSrc; j++)
			{
				if(ipVersion == IP_VERSION4)
				{
					if(ntohl(sourceAddr[0])==sourceEntry->sourceAddr[0])
						break;
					sourceAddr++;
				}
				else
				{
					if( (ntohl(sourceAddr[0])==sourceEntry->sourceAddr[0])&&
						(ntohl(sourceAddr[1])==sourceEntry->sourceAddr[1])&&
						(ntohl(sourceAddr[2])==sourceEntry->sourceAddr[2])&&
						(ntohl(sourceAddr[3])==sourceEntry->sourceAddr[3]))
						break;
					sourceAddr += 4;
				}
					
			}
			if(j==numOfSrc)//not found
				sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
		
			sourceEntry=nextSourceEntry;
		}	
		if(clientEntry->groupFilterTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
		{
			clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
		}
		
	}
    //to_in(empty) == leave
	if(numOfSrc ==0)
		ClientNum=rtl_getClientNum(moduleIndex,ipVersion,groupEntry,clientEntry);
	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return SUCCESS;
	}
	#endif

	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		}
		
	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
		}
		
	}
	#endif
	#endif
	
OUT:	
	/*no client exist, send leave*/
    #if 0
	if (ClientNum == 0){
		//printk("no client exist.\n");
		return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	}
	else{
		//printk("exist clinetNum :%d\n",ClientNum);
		return 0;
	}
	//return SUCCESS;
    #endif
        return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	
}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else
static  int32 rtl_processToExclude(uint32 moduleIndex, uint32 ipVersion,uint32 portNum , uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceArray=NULL;
	uint32 *sourceAddr=NULL;
#if defined (CONFIG_SMP)
	unsigned long flags = 0;
#endif

	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct groupRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceArray=&(((struct mCastAddrRecord *)pktBuf)->srcList);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		return FAILED;
	
	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddress);
	
	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

#ifdef CONFIG_RTL_MLD_SNOOPING
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif


		newGroupEntry->ipVersion=ipVersion;
		assert(newGroupEntry->clientList==NULL);
			
		
		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif
		groupEntry=newGroupEntry;
	}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
#else	
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
#endif

	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->portNum=portNum;
		
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif	
		
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
		newClientEntry->clientMacAddr[0]=clientmacAddr[0];
		newClientEntry->clientMacAddr[1]=clientmacAddr[1];
		newClientEntry->clientMacAddr[2]=clientmacAddr[2];
		newClientEntry->clientMacAddr[3]=clientmacAddr[3];
		newClientEntry->clientMacAddr[4]=clientmacAddr[4];
		newClientEntry->clientMacAddr[5]=clientmacAddr[5];
#endif
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}

	/*flush the old source list*/
	//rtl_deleteSourceList( clientEntry);
	clientEntry->igmpVersion=IGMP_V3;
	clientEntry->groupFilterTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;

	SMP_LOCK_IGMP(flags);
	
	rtl_deleteSourceList( clientEntry);
	
	/*link the new source list*/
	for(j=0; j<numOfSrc; j++)
	{
		newSourceEntry=rtl_allocateSourceEntry();
		if(newSourceEntry==NULL)
		{
			rtl_gluePrintf("run out of source entry!\n");
			SMP_UNLOCK_IGMP(flags);
			return FAILED;
		}

		if(ipVersion==IP_VERSION4)
		{	
			newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);

			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{			
			newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
			newSourceEntry->sourceAddr[1]= ntohl(sourceAddr[1]);
			newSourceEntry->sourceAddr[2]= ntohl(sourceAddr[2]);
			newSourceEntry->sourceAddr[3]= ntohl(sourceAddr[3]);
			sourceAddr=sourceAddr+4;
		}
#endif						
		/*time out the sources included in the MODE_IS_EXCLUDE report*/
		newSourceEntry->portTimer=rtl_sysUpSeconds;
		rtl_linkSourceEntry(clientEntry,newSourceEntry);
		
	}

	SMP_UNLOCK_IGMP(flags);
	
	#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return SUCCESS;
	}
	#endif

	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif

	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		
	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
	}
	
	#endif
	#endif
	
	
		
	return SUCCESS;
}
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else

static  int32 rtl_processAllow(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;
	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;

	uint32 sourceAddr_host[4] = {0,0,0,0};
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif	
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		return FAILED;
	
	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)   /*means new group address, create new group entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

#ifdef CONFIG_RTL_MLD_SNOOPING	
		newGroupEntry->groupAddr[0]=groupAddress[0];
		newGroupEntry->groupAddr[1]=groupAddress[1];
		newGroupEntry->groupAddr[2]=groupAddress[2];
		newGroupEntry->groupAddr[3]=groupAddress[3];
#else
		newGroupEntry->groupAddr[0]=groupAddress[0];
#endif


		newGroupEntry->ipVersion=ipVersion;

		if(ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
#endif

		groupEntry=newGroupEntry;
	}
	
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
#else
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);
#endif

	if(clientEntry==NULL)
	{
		newClientEntry=rtl_allocateClientEntry();
		if(newClientEntry==NULL)
		{
			rtl_gluePrintf("run out of client entry!\n");
			return FAILED;
		}
		

		assert(newClientEntry->sourceList==NULL);
		newClientEntry->sourceList=NULL;
		newClientEntry->portNum=portNum;
		newClientEntry->igmpVersion=IGMP_V3;
		newClientEntry->groupFilterTimer=rtl_sysUpSeconds;
		if(ipVersion==IP_VERSION4)
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			newClientEntry->clientAddr[0]=clientAddr[0];
			newClientEntry->clientAddr[1]=clientAddr[1];
			newClientEntry->clientAddr[2]=clientAddr[2];
			newClientEntry->clientAddr[3]=clientAddr[3];

		}
		#endif
	
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
		newClientEntry->clientMacAddr[0]=clientmacAddr[0];
		newClientEntry->clientMacAddr[1]=clientmacAddr[1];
		newClientEntry->clientMacAddr[2]=clientmacAddr[2];
		newClientEntry->clientMacAddr[3]=clientmacAddr[3];
		newClientEntry->clientMacAddr[4]=clientmacAddr[4];
		newClientEntry->clientMacAddr[5]=clientmacAddr[5];
#endif
		rtl_linkClientEntry(groupEntry, newClientEntry);
		clientEntry=newClientEntry;
	}
	
	clientEntry->igmpVersion=IGMP_V3;
	
	/*here to handle the source list*/
	for(j=0; j<numOfSrc; j++)
	{
		if(ipVersion==IP_VERSION4)
		{	
			sourceAddr_host[0] = ntohl(sourceAddr[0]);
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else if(ipVersion==IP_VERSION6)
		{	
			sourceAddr_host[0] = ntohl(sourceAddr[0]);
			sourceAddr_host[1] = ntohl(sourceAddr[1]);
			sourceAddr_host[2] = ntohl(sourceAddr[2]);
			sourceAddr_host[3] = ntohl(sourceAddr[3]);
		}
#endif
		sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr_host, clientEntry);
	
		if(sourceEntry==NULL)
		{
			newSourceEntry=rtl_allocateSourceEntry();
			if(newSourceEntry==NULL)
			{
				rtl_gluePrintf("run out of source entry!\n");
				return FAILED;
			}

		
			if(ipVersion==IP_VERSION4)
			{	
				newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{
				newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
				newSourceEntry->sourceAddr[1]= ntohl(sourceAddr[1]);
				newSourceEntry->sourceAddr[2]= ntohl(sourceAddr[2]);
				newSourceEntry->sourceAddr[3]= ntohl(sourceAddr[3]);
			}
#endif						
			newSourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
			rtl_linkSourceEntry(clientEntry,newSourceEntry);
		
		}
		else
		{		
			/*just update source timer*/
			sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;		
		}
			
		if(ipVersion==IP_VERSION4)
		{	
			sourceAddr++;
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			sourceAddr=sourceAddr+4;
		}
#endif				
	}


#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return SUCCESS;
	}
#endif	

	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif

	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		
	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
	}
	#endif
	#endif

	return SUCCESS;
}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static  int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)
#else

static int32 rtl_processBlock(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	uint32 j=0;
	uint32 groupAddress[4]={0, 0, 0, 0};

	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	//struct rtl_clientEntry* newClientEntry=NULL;
	struct rtl_sourceEntry *sourceEntry=NULL;
	struct rtl_sourceEntry *newSourceEntry=NULL;

	
	uint32 hashIndex=0;
	uint16 numOfSrc=0;
	uint32 *sourceAddr=NULL;

	uint32 sourceAddr_host[4] = {0,0,0,0};
	
	if(ipVersion==IP_VERSION4)
	{
		groupAddress[0]=ntohl(((struct groupRecord *)pktBuf)->groupAddr);
		numOfSrc=ntohs(((struct groupRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct groupRecord *)pktBuf)->srcList);
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING		
	else
	{
		
		groupAddress[0]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[0]);
		groupAddress[1]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[1]);
		groupAddress[2]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[2]);
		groupAddress[3]=ntohl(((struct mCastAddrRecord *)pktBuf)->mCastAddr[3]);
		
		numOfSrc=ntohs(((struct mCastAddrRecord *)pktBuf)->numOfSrc);
		sourceAddr=&(((struct mCastAddrRecord *)pktBuf)->srcList);
	}
#endif
	if(rtl_is_reserve_multicastAddr(groupAddress,ipVersion)==SUCCESS)
		return FAILED;
	
	hashIndex=rtl_igmpHashAlgorithm( ipVersion, groupAddress);

	groupEntry=rtl_searchGroupEntry(moduleIndex, ipVersion, groupAddress);

	if(groupEntry==NULL)
	{
		goto out;
	}

#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED)		
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr,clientmacAddr);
#else
	clientEntry=rtl_searchClientEntry(ipVersion, groupEntry, portNum, clientAddr);	
#endif

	if(clientEntry==NULL)
	{
		goto out;
	}

	clientEntry->igmpVersion=IGMP_V3;
	
	if(clientEntry->groupFilterTimer>rtl_sysUpSeconds) /*means exclude mode*/
	{

		for(j=0; j<numOfSrc; j++)
		{
			
			if(ipVersion==IP_VERSION4)
			{	
				sourceAddr_host[0] = ntohl(sourceAddr[0]);
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else if(ipVersion==IP_VERSION6)
			{	
				sourceAddr_host[0] = ntohl(sourceAddr[0]);
				sourceAddr_host[1] = ntohl(sourceAddr[1]);
				sourceAddr_host[2] = ntohl(sourceAddr[2]);
				sourceAddr_host[3] = ntohl(sourceAddr[3]);
			}
#endif
			sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr_host, clientEntry);
		
			if(sourceEntry==NULL)
			{
				newSourceEntry=rtl_allocateSourceEntry();
				if(newSourceEntry==NULL)
				{
					rtl_gluePrintf("run out of source entry!\n");
					return FAILED;
				}

			
				if(ipVersion==IP_VERSION4)
				{	
					newSourceEntry->sourceAddr[0]= ntohl(sourceAddr[0]);
				
				}
#ifdef CONFIG_RTL_MLD_SNOOPING
				else
				{		
					newSourceEntry->sourceAddr[0]=ntohl(sourceAddr[0]);
					newSourceEntry->sourceAddr[1]=ntohl(sourceAddr[1]);
					newSourceEntry->sourceAddr[2]=ntohl(sourceAddr[2]);
					newSourceEntry->sourceAddr[3]=ntohl(sourceAddr[3]);
				}
#endif                          
				newSourceEntry->portTimer=rtl_sysUpSeconds;
				rtl_linkSourceEntry(clientEntry,newSourceEntry);
			}
			else
			{
				if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				{
					sourceEntry->portTimer=rtl_sysUpSeconds;	
				}
				else
				{
					if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
					{
						sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
				}	
			}

		
			if(ipVersion==IP_VERSION4)
			{	
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{
				sourceAddr=sourceAddr+4;
			}
#endif					
		}
                    
	}
	else           /*means include mode*/
	{

		for(j=0; j<numOfSrc; j++)
        {       
			if(ipVersion==IP_VERSION4)
			{
				sourceAddr_host[0]=ntohl(sourceAddr[0]);
			}
			#ifdef CONFIG_RTL_MLD_SNOOPING
			else if(ipVersion==IP_VERSION6)
			{
				sourceAddr_host[0]=ntohl(sourceAddr[0]);
				sourceAddr_host[1]=ntohl(sourceAddr[1]);
				sourceAddr_host[2]=ntohl(sourceAddr[2]);
				sourceAddr_host[3]=ntohl(sourceAddr[3]);
			}
			#endif
			sourceEntry=rtl_searchSourceEntry(ipVersion, sourceAddr_host,clientEntry);
			if(sourceEntry!=NULL)
			{
				if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
				{
					sourceEntry->portTimer=rtl_sysUpSeconds;
				}
				else
				{
					if(sourceEntry->portTimer>rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime)
					{
						sourceEntry->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.lastMemberAgingTime;
					}
							
				}
				
			}	

			if(ipVersion==IP_VERSION4)
			{	
				sourceAddr++;
			}
#ifdef CONFIG_RTL_MLD_SNOOPING
			else
			{
				sourceAddr=sourceAddr+4;
			}
#endif					
 		}

	}

#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
	if((groupEntry!=NULL) && (groupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		return SUCCESS;
	}
#endif	

out:
	
	reportEventContext.ipVersion=ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupAddress[0];
	reportEventContext.groupAddr[1]=groupAddress[1];
	reportEventContext.groupAddr[2]=groupAddress[2];
	reportEventContext.groupAddr[3]=groupAddress[3];
	#else
	reportEventContext.groupAddr[0]=groupAddress[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
			rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		}
		
	}
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	if(ipVersion==IP_VERSION6)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		if(rtl_mCastModuleArray[moduleIndex].enableFastLeave==TRUE)
		{
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &reportEventContext);
		}
	}
	#endif
	#endif
	
	return SUCCESS;

}


#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint32 *clientAddr,uint8 *clientmacAddr, uint8 *pktBuf)

#else

static uint32 rtl_processIgmpv3Mldv2Reports(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8 *pktBuf)
#endif
{
	uint32 i=0;
	uint16 numOfRecords=0;
	uint8 *groupRecords=NULL;
	uint8 recordType=0xff;
	uint16 numOfSrc=0;
	int32 returnVal=0;
	//uint32 multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, ipVersion, rtl_sysUpSeconds);
	
	if(ipVersion==IP_VERSION4)
	{
		numOfRecords=ntohs(((struct igmpv3Report *)pktBuf)->numOfRecords);
		if(numOfRecords!=0)
		{
			groupRecords=(uint8 *)(&(((struct igmpv3Report *)pktBuf)->recordList));
		}
	}
#ifdef CONFIG_RTL_MLD_SNOOPING
	else
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
		else
		{
			recordType=((struct mCastAddrRecord *)groupRecords)->type;
		}
#endif		
	
		switch(recordType)
		{
			case MODE_IS_INCLUDE:
			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
				returnVal=rtl_processIsInclude(moduleIndex, ipVersion, portNum, clientAddr,clientmacAddr, groupRecords);
			#else	
				returnVal=rtl_processIsInclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			#endif
			break;
			
			case MODE_IS_EXCLUDE:
			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
				returnVal=rtl_processIsExclude(moduleIndex, ipVersion, portNum, clientAddr,clientmacAddr, groupRecords);
			#else	
				returnVal=rtl_processIsExclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			#endif
			break;
			
			case CHANGE_TO_INCLUDE_MODE:
			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
				returnVal=rtl_processToInclude(moduleIndex, ipVersion, portNum, clientAddr,clientmacAddr, groupRecords);
			#else	
				returnVal=rtl_processToInclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			#endif
			break;
			
			case CHANGE_TO_EXCLUDE_MODE:
			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
				returnVal=rtl_processToExclude(moduleIndex, ipVersion, portNum, clientAddr,clientmacAddr, groupRecords);
			#else	
				returnVal=rtl_processToExclude(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			#endif
			break;
			
			case ALLOW_NEW_SOURCES:
			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
				returnVal=rtl_processAllow(moduleIndex, ipVersion, portNum, clientAddr,clientmacAddr, groupRecords);
			#else	
				returnVal=rtl_processAllow(moduleIndex, ipVersion, portNum, clientAddr, groupRecords);
			#endif
			break;
			
			case BLOCK_OLD_SOURCES:
			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
				returnVal=rtl_processBlock(moduleIndex, ipVersion, portNum, clientAddr,clientmacAddr, groupRecords);
			#else	
				returnVal=rtl_processBlock(moduleIndex, ipVersion, portNum, clientAddr ,groupRecords);
			#endif
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
		else
		{
			numOfSrc=ntohs(((struct mCastAddrRecord *)groupRecords)->numOfSrc);
			/*shift pointer to another group record*/
			groupRecords=groupRecords+20+numOfSrc*16+(((struct mCastAddrRecord *)(groupRecords))->auxLen)*4;
		}
#endif		
	}
	
	if((numOfRecords==1) && (recordType==CHANGE_TO_INCLUDE_MODE))
  	{
  		return returnVal;
  	}	
	else
	{
		/*no report supress, due to multiple group record in igmpv3 report*/
		return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	}
	//return (multicastRouterPortMask&(~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	
}
#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr,uint8 *clientMacAddr, uint8* pktBuf, uint32 pktLen)
#else
static uint32 rtl_processIgmpMld(uint32 moduleIndex, uint32 ipVersion, uint32 portNum,uint32 *clientAddr, uint8* pktBuf, uint32 pktLen)
#endif
{	
	uint32 fwdPortMask=0;

	reportEventContext.moduleIndex=moduleIndex;

	switch(pktBuf[0])
	{
		case IGMP_QUERY:
			fwdPortMask=rtl_processQueries(moduleIndex, ipVersion, portNum, pktBuf, pktLen);
		break;
			
		case IGMPV1_REPORT:
		#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)	
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum,clientAddr,clientMacAddr,pktBuf);
		#else
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum,clientAddr,pktBuf);
		#endif
		break;
			
		case IGMPV2_REPORT:	
		#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum,clientAddr,clientMacAddr,pktBuf);
		#else
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum,clientAddr, pktBuf);
		#endif
		break;
			
		case IGMPV2_LEAVE:
		#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum,clientAddr,clientMacAddr,pktBuf);
		#else	
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, clientAddr,pktBuf);
		#endif
		break;

		case IGMPV3_REPORT:
		#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum,clientAddr,clientMacAddr,pktBuf);
		#else
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, clientAddr,pktBuf);
		#endif
		break;

		case MLD_QUERY:
			fwdPortMask=rtl_processQueries(moduleIndex, ipVersion, portNum, pktBuf, pktLen);
		break;
			
		case MLDV1_REPORT:
		#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum,clientAddr,clientMacAddr,pktBuf);
		#else		
			 fwdPortMask=rtl_processJoin(moduleIndex, ipVersion, portNum, clientAddr, pktBuf);
		#endif
		break;
			
		case MLDV1_DONE:	
		#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK) 
			
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, clientAddr,clientMacAddr, pktBuf);
		#else
			 fwdPortMask=rtl_processLeave(moduleIndex, ipVersion, portNum, clientAddr, pktBuf);
		#endif
		break;
			
		case MLDV2_REPORT:
		#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum,clientAddr,clientMacAddr,pktBuf);
		#else
			 fwdPortMask=rtl_processIgmpv3Mldv2Reports(moduleIndex, ipVersion, portNum, clientAddr, pktBuf);
		#endif
		break;

		default:
			fwdPortMask=((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
		break;
	}						
	
	return fwdPortMask;
			
}



static uint32 rtl_processDvmrp(uint32 moduleIndex, uint32 ipVersion,uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	
	if(ipVersion==IP_VERSION4)
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.dvmrpRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.dvmrpRouterAgingTime; /*update timer*/
	}

	if(ipVersion==IP_VERSION4)
	{

		reportEventContext.ipVersion=ipVersion;
		reportEventContext.groupAddr[0]=0;
		reportEventContext.groupAddr[1]=0;
		reportEventContext.groupAddr[2]=0;
		reportEventContext.groupAddr[3]=0;

		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
		#endif
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		/*we only support ipv4 hardware multicast*/ 
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		#endif
	}

	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));

}

static uint32 rtl_processMospf(uint32 moduleIndex,uint32 ipVersion,uint32 portNum, uint8* pktBuf, uint32 pktLen)
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
	else
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

	

	reportEventContext.ipVersion=ipVersion;
	reportEventContext.groupAddr[0]=0;
	reportEventContext.groupAddr[1]=0;
	reportEventContext.groupAddr[2]=0;
	reportEventContext.groupAddr[3]=0;
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
	}
	#endif
	
	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
	
}

static uint32 rtl_processPim(uint32 moduleIndex, uint32 ipVersion, uint32 portNum, uint8* pktBuf, uint32 pktLen)
{
	if(ipVersion==IP_VERSION4)
	{	
		rtl_mCastModuleArray[moduleIndex].rtl_ipv4MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
		
	}
#ifdef CONFIG_RTL_MLD_SNOOPING	
	else
	{
		rtl_mCastModuleArray[moduleIndex].rtl_ipv6MulticastRouters.pimRouter.portTimer[portNum]=rtl_sysUpSeconds+rtl_mCastTimerParas.pimRouterAgingTime; /*update timer*/
	}
#endif



	reportEventContext.ipVersion=ipVersion;
	reportEventContext.groupAddr[0]=0;
	reportEventContext.groupAddr[1]=0;
	reportEventContext.groupAddr[2]=0;
	reportEventContext.groupAddr[3]=0;
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(ipVersion==IP_VERSION4)
	{
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
	}
	#endif

	return ((~(1<<portNum))&((1<<MAX_SUPPORT_PORT_NUMBER)-1));
}



/*********************************************
				External Function
  *********************************************/


//External called function by high level program

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
		rtl_mCastModuleArray[index].enableFastLeave=FALSE;
		rtl_mCastModuleArray[index].enableSnooping=TRUE;
		rtl_mCastModuleArray[index].ipv4UnknownMCastFloodMap=DEFAULT_IPV4_UNKNOWN_MCAST_FLOOD_MAP;
#ifdef CONFIG_RTL_MLD_SNOOPING
		rtl_mCastModuleArray[index].ipv6UnknownMCastFloodMap=DEFAULT_IPV6_UNKNOWN_MCAST_FLOOD_MAP;
#endif
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



int32 rtl_unregisterIgmpSnoopingModule(uint32 moduleIndex)
{
	uint32 i=0;
	struct rtl_groupEntry *groupEntryPtr=NULL;
	#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry *mcastFlowEntryPtr=NULL;
	#endif
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
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
		{
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
		}
#ifdef CONFIG_RTL_MLD_SNOOPING		
		/*delete ipv6 multicast entry*/
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
		{
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
		}
#endif

#ifdef CONFIG_RECORD_MCAST_FLOW
		/*delete multicast flow entry*/
		if(rtl_mCastModuleArray[moduleIndex].flowHashTable != NULL)
		{
        	for(i=0;i<rtl_hashTableSize;i++)
	     	{
				mcastFlowEntryPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
				
				while(mcastFlowEntryPtr!=NULL)
				{
					rtl_deleteMcastFlowEntry(mcastFlowEntryPtr, rtl_mCastModuleArray[moduleIndex].flowHashTable);
					mcastFlowEntryPtr=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
				}
	       	}
			rtl_glueFree(rtl_mCastModuleArray[moduleIndex].flowHashTable);
			rtl_mCastModuleArray[moduleIndex].flowHashTable=NULL;
		}
#endif
		rtl_mCastModuleArray[moduleIndex].enableSnooping=FALSE;
		rtl_mCastModuleArray[moduleIndex].enableFastLeave=FALSE;
		rtl_mCastModuleArray[moduleIndex].ipv4UnknownMCastFloodMap=0;
#ifdef CONFIG_RTL_MLD_SNOOPING
		rtl_mCastModuleArray[moduleIndex].ipv6UnknownMCastFloodMap=0;
#endif
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
int rtl_getMulticastParameters(struct rtl_mCastTimerParameters *mCastTimerParameters)
{
	if(mCastTimerParameters == NULL)
		return FAILED;

	mCastTimerParameters->disableExpire= rtl_mCastTimerParas.disableExpire;
	mCastTimerParameters->groupMemberAgingTime= rtl_mCastTimerParas.groupMemberAgingTime;
	mCastTimerParameters->lastMemberAgingTime= rtl_mCastTimerParas.lastMemberAgingTime;
	mCastTimerParameters->querierPresentInterval=rtl_mCastTimerParas.querierPresentInterval;
	mCastTimerParameters->dvmrpRouterAgingTime=rtl_mCastTimerParas.dvmrpRouterAgingTime;
	mCastTimerParameters->mospfRouterAgingTime=rtl_mCastTimerParas.mospfRouterAgingTime;	
	mCastTimerParameters->pimRouterAgingTime=rtl_mCastTimerParas.pimRouterAgingTime;

	return SUCCESS;
}


void rtl_modifyGroupTimer(int groupAgeTime)
{
	int32 moduleIndex;
	int32 hashIndex;
	struct rtl_groupEntry *groupEntryPtr=NULL;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_sourceEntry *sourceEntryPtr=NULL;

	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
			if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
			{
				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
				{
					for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
					{
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
						while(groupEntryPtr!=NULL)
						{
							clientEntry=groupEntryPtr->clientList;
						
							while (clientEntry!=NULL)
							{					
							
								if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
								{
									if((clientEntry->groupFilterTimer-rtl_sysUpSeconds)>rtl_mCastTimerParas.groupMemberAgingTime)
										clientEntry->groupFilterTimer = rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
								}

								sourceEntryPtr=clientEntry->sourceList;
	
								while(sourceEntryPtr!=NULL)
								{
									if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
									{
										if((sourceEntryPtr->portTimer-rtl_sysUpSeconds)>rtl_mCastTimerParas.groupMemberAgingTime)
											sourceEntryPtr->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
									}
									
									sourceEntryPtr=sourceEntryPtr->next;
								}
								clientEntry = clientEntry->next;
							}
							groupEntryPtr=groupEntryPtr->next;	
						}
					
					}
				}
				if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
				{
					for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
					{
						groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
						while(groupEntryPtr!=NULL)
						{
							clientEntry=groupEntryPtr->clientList;
						
							while (clientEntry!=NULL)
							{					
							
								if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
								{
									if((clientEntry->groupFilterTimer-rtl_sysUpSeconds)>rtl_mCastTimerParas.groupMemberAgingTime)
										clientEntry->groupFilterTimer = rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
								}

								sourceEntryPtr=clientEntry->sourceList;
	
								while(sourceEntryPtr!=NULL)
								{
									if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
									{
										if((sourceEntryPtr->portTimer-rtl_sysUpSeconds)>rtl_mCastTimerParas.groupMemberAgingTime)
											sourceEntryPtr->portTimer=rtl_sysUpSeconds+rtl_mCastTimerParas.groupMemberAgingTime;
									}
									
									sourceEntryPtr=sourceEntryPtr->next;
								}
								clientEntry = clientEntry->next;
							}
							groupEntryPtr=groupEntryPtr->next;	
						}
					
					}
				}	
			}
		}

}

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
	
	rtl_mCastModuleArray[moduleIndex].enableFastLeave=mCastSnoopingLocalConfig->enableFastLeave;
	rtl_mCastModuleArray[moduleIndex].ipv4UnknownMCastFloodMap=mCastSnoopingLocalConfig->ipv4UnknownMcastFloodMap;
#ifdef CONFIG_RTL_MLD_SNOOPING
	rtl_mCastModuleArray[moduleIndex].ipv6UnknownMCastFloodMap=mCastSnoopingLocalConfig->ipv6UnknownMcastFloodMap;
#endif
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
@func int32	| rtl_maintainMulticastSnoopingTimerList	|   Multicast snooping timer list maintenance function.
@parm  uint32	| currentSystemTime	|The current system time (unit: seconds).
@rvalue SUCCESS	|Always return SUCCESS.
@comm 
 This function should be called once a second to maintain multicast timer list.
*/
int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime)
{
	/* maintain current time */
	uint32 i=0;
	uint32 maxTime=0xffffffff;

	struct rtl_groupEntry* groupEntryPtr=NULL;
	struct rtl_groupEntry* nextEntry=NULL;

	uint32 moduleIndex;
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM; moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			if((currentSystemTime%DEFAULT_MCAST_FLOW_EXPIRE_TIME)==0)
			{
				rtl_doMcastFlowRecycle(moduleIndex, BOTH_IPV4_IPV6);
			}
			
		}
	}
	#endif	
	
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
				 	timerEventContext.ipVersion=IP_VERSION4;
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						nextEntry=groupEntryPtr->next; 
						timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						timerEventContext.groupAddr[1]=0;
						timerEventContext.groupAddr[2]=0;
						timerEventContext.groupAddr[3]=0;
						rtl_checkGroupEntryTimer(groupEntryPtr,  rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
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
					timerEventContext.ipVersion=IP_VERSION6;
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[i];
					while(groupEntryPtr)              /*traverse each group list*/
					{	
						nextEntry=groupEntryPtr->next; 
						timerEventContext.groupAddr[0]=groupEntryPtr->groupAddr[0];
						timerEventContext.groupAddr[1]=groupEntryPtr->groupAddr[1];
						timerEventContext.groupAddr[2]=groupEntryPtr->groupAddr[2];
						timerEventContext.groupAddr[3]=groupEntryPtr->groupAddr[3];
						rtl_checkGroupEntryTimer(groupEntryPtr, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
						groupEntryPtr=nextEntry;/*because expired group entry  will be cleared*/
					}
				}
			}
#endif

		}
	}
	return SUCCESS;
}



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

		if(macFrameInfo.checksumFlag!=SUCCESS)
		{
			return FAILED;
		}
		
		switch(macFrameInfo.l3Protocol)
		{

			case IGMP_PROTOCOL:
			//	printk("%x-%x-%x-%x-%x-%x[%s]:[%d].\n",macFrameInfo.srcMacAddr[0],macFrameInfo.srcMacAddr[1],macFrameInfo.srcMacAddr[2],macFrameInfo.srcMacAddr[3],macFrameInfo.srcMacAddr[4],macFrameInfo.srcMacAddr[5],__FUNCTION__,__LINE__);
    			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.srcIpAddr,macFrameInfo.srcMacAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			    #else
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.srcIpAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
                #endif
			break;

			case ICMP_PROTOCOL:
    			#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion),portNum, macFrameInfo.srcIpAddr,macFrameInfo.srcMacAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			    #else
				*fwdPortMask=rtl_processIgmpMld(moduleIndex, (uint32)(macFrameInfo.ipVersion),portNum, macFrameInfo.srcIpAddr, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
                #endif
			break;


			case DVMRP_PROTOCOL:
				*fwdPortMask=rtl_processDvmrp(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			case MOSPF_PROTOCOL:
				*fwdPortMask=rtl_processMospf(moduleIndex, (uint32)(macFrameInfo.ipVersion), portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;
				
			case PIM_PROTOCOL:
				*fwdPortMask=rtl_processPim(moduleIndex, (uint32)(macFrameInfo.ipVersion),portNum, macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen);
			break;

			default: break;
		}
		
		rtl_setClientMacAddr(&macFrameInfo);
	}
	
	return SUCCESS;
}

#ifdef CONFIG_RECORD_MCAST_FLOW
static int32 rtl_recordMcastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *sourceIpAddr, uint32 *groupAddr, struct rtl_multicastFwdInfo * multicastFwdInfo)
{
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL; 

	if(multicastFwdInfo==NULL)
	{
		return FAILED;
	}
	
	mcastFlowEntry=rtl_searchMcastFlowEntry(moduleIndex, ipVersion, sourceIpAddr, groupAddr);

	if(mcastFlowEntry==NULL)
	{

		mcastFlowEntry=rtl_allocateMcastFlowEntry();
		if(mcastFlowEntry==NULL)
		{
			rtl_doMcastFlowRecycle(moduleIndex, ipVersion);
			
			mcastFlowEntry=rtl_allocateMcastFlowEntry();
			if(mcastFlowEntry==NULL)
			{
				rtl_gluePrintf("run out of multicast flow entry!\n");
				return FAILED;
			}
		}
	
		if(ipVersion==IP_VERSION4)
		{
			mcastFlowEntry->serverAddr[0]=sourceIpAddr[0];
			mcastFlowEntry->groupAddr[0]=groupAddr[0];
		
		}
#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			mcastFlowEntry->serverAddr[0]=sourceIpAddr[0];
			mcastFlowEntry->serverAddr[1]=sourceIpAddr[1];
			mcastFlowEntry->serverAddr[2]=sourceIpAddr[2];
			mcastFlowEntry->serverAddr[3]=sourceIpAddr[3];
			
			mcastFlowEntry->groupAddr[0]=groupAddr[0];
			mcastFlowEntry->groupAddr[1]=groupAddr[1];
			mcastFlowEntry->groupAddr[2]=groupAddr[2];
			mcastFlowEntry->groupAddr[3]=groupAddr[3];
		}
#endif		

		mcastFlowEntry->ipVersion=ipVersion;

		memcpy(&mcastFlowEntry->multicastFwdInfo, multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo ));

		mcastFlowEntry->refreshTime=rtl_sysUpSeconds;
		
		rtl_linkMcastFlowEntry(mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
		
		return SUCCESS;
			
	}
	else
	{
		/*update forward port mask information */
		memcpy(&mcastFlowEntry->multicastFwdInfo, multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo ));
		mcastFlowEntry->refreshTime=rtl_sysUpSeconds;
		return SUCCESS;
	}

	return SUCCESS;
}

static void rtl_invalidateMCastFlow(uint32 moduleIndex,uint32 ipVersion, uint32 *groupAddr)
{
	uint32 hashIndex;
	struct rtl_mcastFlowEntry* mcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* nextMcastFlowEntry = NULL;
	
	if(NULL==groupAddr)
	{
		return ;
	}
	

	hashIndex=rtl_igmpHashAlgorithm(ipVersion, groupAddr);

	if(rtl_mCastModuleArray[moduleIndex].flowHashTable != NULL)
		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
	
	while (mcastFlowEntry!=NULL)
	{	
		nextMcastFlowEntry=mcastFlowEntry->next;

		if(ipVersion==mcastFlowEntry->ipVersion)		
		{
		
				
#ifdef CONFIG_RTL_MLD_SNOOPING	
			if((groupAddr[0]==0)&&(groupAddr[1]==0)&&(groupAddr[2]==0)&&(groupAddr[3]==0))	
#else
			if(groupAddr[0]==0)
#endif
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
			}
			else
			{
#ifdef CONFIG_RTL_MLD_SNOOPING	
				if (	(mcastFlowEntry->groupAddr[0]==groupAddr[0])&&(mcastFlowEntry->groupAddr[1]==groupAddr[1])&&
					(mcastFlowEntry->groupAddr[2]==groupAddr[2])&&(mcastFlowEntry->groupAddr[3]==groupAddr[3])	)	
#else
				if(mcastFlowEntry->groupAddr[0] == groupAddr[0])
#endif	
				{
					rtl_deleteMcastFlowEntry(mcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
				}
			}


					
		}
		
		mcastFlowEntry = nextMcastFlowEntry;
	}
	
	return ;

}

static void rtl_doMcastFlowRecycle(uint32 moduleIndex, uint32 ipVersion)
{
	uint32 i;
	uint32 freeCnt=0;
	struct rtl_mcastFlowEntry* mcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* nextMcastFlowEntry = NULL;
	struct rtl_mcastFlowEntry* oldestMcastFlowEntry = NULL;

	if(rtl_mCastModuleArray[moduleIndex].flowHashTable == NULL)
		return;

	for (i = 0 ; i < rtl_hashTableSize ; i++)
	{
		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
		
		if(oldestMcastFlowEntry==NULL)
		{
			oldestMcastFlowEntry=mcastFlowEntry;
		}
		
		while (mcastFlowEntry!=NULL)
		{	
			nextMcastFlowEntry=mcastFlowEntry->next;
			/*keep the most recently used entry*/
			if((mcastFlowEntry->refreshTime+DEFAULT_MCAST_FLOW_EXPIRE_TIME) < rtl_sysUpSeconds)
			{
				rtl_deleteMcastFlowEntry(mcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);
				freeCnt++;
			}
			mcastFlowEntry=nextMcastFlowEntry;
			
		}
	}

	if(freeCnt>0)
	{
		return;
	}

	/*if too many concurrent flow,we have to do LRU*/
	for (i = 0 ; i < rtl_hashTableSize ; i++)
	{
		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[i];
		
		if(oldestMcastFlowEntry==NULL)
		{
			oldestMcastFlowEntry=mcastFlowEntry;
		}
		
		while (mcastFlowEntry!=NULL)
		{	
			nextMcastFlowEntry=mcastFlowEntry->next;
			if(mcastFlowEntry->refreshTime < oldestMcastFlowEntry->refreshTime)
			{
				oldestMcastFlowEntry=mcastFlowEntry;
			}
			
			mcastFlowEntry=nextMcastFlowEntry;
			
		}
	}

	if(oldestMcastFlowEntry!=NULL)
	{
		rtl_deleteMcastFlowEntry(oldestMcastFlowEntry,  rtl_mCastModuleArray[moduleIndex].flowHashTable);

	}
					
	return ;

}

#endif
#if 0
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

	if(retVal==SUCCESS)
	{
		if(multicastFwdInfo.unknownMCast==TRUE)
		{
			return FAILED;
		}
		else
		{
			return SUCCESS;
		}
	}
	
	return FAILED;
	
}
#endif

int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo *multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo)
{
	#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL; 
	#endif
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
	
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	mcastFlowEntry=rtl_searchMcastFlowEntry( moduleIndex, multicastDataInfo->ipVersion, multicastDataInfo->sourceIp, multicastDataInfo->groupAddr);
	if(mcastFlowEntry!=NULL)
	{
		memcpy(multicastFwdInfo, &mcastFlowEntry->multicastFwdInfo, sizeof(struct rtl_multicastFwdInfo));
		return SUCCESS;
	}
	#endif

	//added for MDNS packets
	if ((IS_RESERVE_MULTICAST_MDNS_ADDR_V4(multicastDataInfo->groupAddr[0])&&multicastDataInfo->ipVersion==IP_VERSION4)
		||(IS_RESERVE_MULTICAST_MDNS_ADDR_V6(multicastDataInfo->groupAddr)&&multicastDataInfo->ipVersion==IP_VERSION6))
	{
		multicastFwdInfo->reservedMCast=TRUE;
		multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
				
		return FAILED;
	}
	
	groupEntry=rtl_searchGroupEntry(moduleIndex,multicastDataInfo->ipVersion, multicastDataInfo->groupAddr); 

	if(groupEntry==NULL)
	{
		if(	(multicastDataInfo->groupAddr[0]==RESERVE_MULTICAST_ADDR1) ||
			(multicastDataInfo->groupAddr[0]==RESERVE_MULTICAST_ADDR2)	)
		{
			multicastFwdInfo->unknownMCast=FALSE;
			multicastFwdInfo->reservedMCast=TRUE;
			multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
			multicastFwdInfo->cpuFlag=TRUE;
			
			return FAILED;
		}
		#if 0
		else if(IN_MULTICAST_RESV1(multicastDataInfo->groupAddr[0]) )
		{
			multicastFwdInfo->unknownMCast=FALSE;
			multicastFwdInfo->reservedMCast=TRUE;
			multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
			multicastFwdInfo->cpuFlag=TRUE;
			
			return FAILED;
		}
		#endif

	
		multicastFwdInfo->unknownMCast=TRUE;
		if(multicastDataInfo->ipVersion==IP_VERSION4)
		{
			multicastFwdInfo->fwdPortMask= rtl_mCastModuleArray[moduleIndex].ipv4UnknownMCastFloodMap;
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else if (multicastDataInfo->ipVersion==IP_VERSION6)
		{
			multicastFwdInfo->fwdPortMask= rtl_mCastModuleArray[moduleIndex].ipv6UnknownMCastFloodMap;
		}
		#endif
		else
		{
			multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
		}
		
		
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		return FAILED;
	}
	else
	{
		/*step1: handle user specified static reserved multicast entry*/
		#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
		if(groupEntry->attribute==STATIC_RESERVED_MULTICAST)
		{
			multicastFwdInfo->unknownMCast=FALSE;
			multicastFwdInfo->reservedMCast=TRUE;
			multicastFwdInfo->cpuFlag=TRUE;
			multicastFwdInfo->fwdPortMask=groupEntry->staticFwdPortMask;
			multicastFwdInfo->fwdPortMask=(multicastFwdInfo->fwdPortMask|multicastRouterPortMask);
			return FAILED;
		}
		#endif

		/*step 2: handle hard-coded reserved multicast address*/
		if(	(multicastDataInfo->groupAddr[0]==RESERVE_MULTICAST_ADDR1) ||
			(multicastDataInfo->groupAddr[0]==RESERVE_MULTICAST_ADDR2)	)
		{
			multicastFwdInfo->unknownMCast=FALSE;
			multicastFwdInfo->reservedMCast=TRUE;
			multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
			multicastFwdInfo->cpuFlag=TRUE;
			return FAILED;
		}
		#if 0
		else if(IN_MULTICAST_RESV1(multicastDataInfo->groupAddr[0]) )
		{
			multicastFwdInfo->unknownMCast=FALSE;
			multicastFwdInfo->reservedMCast=TRUE;
			multicastFwdInfo->fwdPortMask=0xFFFFFFFF;
			multicastFwdInfo->cpuFlag=TRUE;
			
			return FAILED;
		}
		#endif

		/*step3:handle normal multicast address */
			
		/*here to get multicast router port mask and forward port mask*/
		//multicastRouterPortMask=rtl_getMulticastRouterPortMask(moduleIndex, multicastDataInfo->ipVersion, rtl_sysUpSeconds);
		multicastFwdInfo->fwdPortMask=rtl_getGroupSourceFwdPortMask(groupEntry, multicastDataInfo->sourceIp, rtl_sysUpSeconds);
		multicastFwdInfo->fwdPortMask=(multicastFwdInfo->fwdPortMask|multicastRouterPortMask);

		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if((multicastFwdInfo->fwdPortMask & rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask)!=0)
		{
			multicastFwdInfo->cpuFlag=TRUE;
		}
		#endif

		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_recordMcastFlow(moduleIndex,multicastDataInfo->ipVersion, multicastDataInfo->sourceIp, multicastDataInfo->groupAddr, multicastFwdInfo);
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


static int32 rtl_syncAllIpv4UnknownMCastFlow(uint32 moduleIndex)
{

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{
		/*maintain ipv4 group entry  timer */
		reportEventContext.moduleIndex=moduleIndex;
		reportEventContext.ipVersion=IP_VERSION4;
		reportEventContext.groupAddr[0]=0;
		reportEventContext.groupAddr[1]=0;
		reportEventContext.groupAddr[2]=0;
		reportEventContext.groupAddr[3]=0;
		
	
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
		#endif

		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		#endif

	}
	return SUCCESS;
}


#ifdef CONFIG_RTL_MLD_SNOOPING	
static int32 rtl_syncAllIpv6UnknownMCastFlow(uint32 moduleIndex)
{
	reportEventContext.moduleIndex=moduleIndex;
	reportEventContext.ipVersion=IP_VERSION6;
	reportEventContext.groupAddr[0]=0;
	reportEventContext.groupAddr[1]=0;
	reportEventContext.groupAddr[2]=0;
	reportEventContext.groupAddr[3]=0;
	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{

		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
		#endif
	}
	
	return SUCCESS;
}
#endif

int32 rtl_setIpv4UnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	rtl_mCastModuleArray[moduleIndex].ipv4UnknownMCastFloodMap=unknownMCastFloodMap;

	rtl_syncAllIpv4UnknownMCastFlow(moduleIndex);
	return SUCCESS;
}

int32 rtl_getIpv4UnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap)
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

	*unknownMCastFloodMap=rtl_mCastModuleArray[moduleIndex].ipv4UnknownMCastFloodMap;
	
	return SUCCESS;
}

#ifdef CONFIG_RTL_MLD_SNOOPING	
int32 rtl_setIpv6UnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap)
{
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}

	rtl_mCastModuleArray[moduleIndex].ipv6UnknownMCastFloodMap=unknownMCastFloodMap;
	
	rtl_syncAllIpv6UnknownMCastFlow(moduleIndex);

	return SUCCESS;
}

int32 rtl_getIpv6UnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap)
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

	*unknownMCastFloodMap=rtl_mCastModuleArray[moduleIndex].ipv6UnknownMCastFloodMap;
	
	return SUCCESS;
}
#endif

int32 rtl_setIgmpSnoopingModuleUnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap)
{
	 rtl_setIpv4UnknownMCastFloodMap( moduleIndex, unknownMCastFloodMap);
	 #ifdef CONFIG_RTL_MLD_SNOOPING	
	 rtl_setIpv6UnknownMCastFloodMap( moduleIndex, unknownMCastFloodMap);
	 #endif
	 return SUCCESS;
}

#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
static int32 rtl_addStaticGroupEntry(uint32 moduleIndex, struct rtl_groupEntry* groupEntry)
{
	struct rtl_groupEntry* existGroupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;

	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	if(groupEntry==NULL)
	{
		return FAILED;
	}

	if((groupEntry->ipVersion!=IP_VERSION4) && (groupEntry->ipVersion!=IP_VERSION6))
	{
		return FAILED;
	}

	if(groupEntry->ipVersion==IP_VERSION4)
	{
		if((groupEntry->groupAddr[0]!=0) && (!IS_CLASSD_ADDR(groupEntry->groupAddr[0])))
		{
			return FAILED;
		}
	}
	else if(groupEntry->ipVersion==IP_VERSION6)
	{
		/*need to complete ipv6 address range */
	}

	/*set unknown multicast default forwarding  configuration*/
	if(groupEntry->ipVersion==IP_VERSION4)
	{
		if(groupEntry->groupAddr[0]==0)
		{
			rtl_setIpv4UnknownMCastFloodMap(moduleIndex,groupEntry->staticFwdPortMask);
			return SUCCESS;
		}
	}
	else if(groupEntry->ipVersion==IP_VERSION6)
	{
#ifdef CONFIG_RTL_MLD_SNOOPING
		if(	(groupEntry->groupAddr[0]==0) &&
			(groupEntry->groupAddr[0]==0) &&
			(groupEntry->groupAddr[0]==0) &&
			(groupEntry->groupAddr[0]==0)	)
		{
			rtl_setIpv6UnknownMCastFloodMap(moduleIndex,groupEntry->staticFwdPortMask);
			return SUCCESS;
		}
#else
			return FAILED;
#endif
	}
		
	
	existGroupEntry=rtl_searchGroupEntry(moduleIndex, groupEntry->ipVersion, groupEntry->groupAddr);
	if(existGroupEntry!=NULL)
	{
		existGroupEntry->attribute=STATIC_RESERVED_MULTICAST;
		existGroupEntry->staticFwdPortMask=groupEntry->staticFwdPortMask;
	}
	else
	{

		newGroupEntry=rtl_allocateGroupEntry();
		
		if(newGroupEntry==NULL)
		{
			rtl_gluePrintf("run out of group entry!\n");
			return FAILED;
		}

		assert(newGroupEntry->clientList==NULL);
		#ifdef CONFIG_RTL_MLD_SNOOPING
		newGroupEntry->groupAddr[0]=groupEntry->groupAddr[0];
		newGroupEntry->groupAddr[1]=groupEntry->groupAddr[1];
		newGroupEntry->groupAddr[2]=groupEntry->groupAddr[2];
		newGroupEntry->groupAddr[3]=groupEntry->groupAddr[3];
		#else
		newGroupEntry->groupAddr[0]=groupEntry->groupAddr[0];
		#endif

		newGroupEntry->ipVersion=groupEntry->ipVersion;
		newGroupEntry->attribute=STATIC_RESERVED_MULTICAST;
		newGroupEntry->staticFwdPortMask=groupEntry->staticFwdPortMask;
		
		if(groupEntry->ipVersion==IP_VERSION4)
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable);
		}
		#ifdef CONFIG_RTL_MLD_SNOOPING
		else
		{
			rtl_linkGroupEntry(newGroupEntry, rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable);
		}
		#endif			
	}

	reportEventContext.moduleIndex=moduleIndex;
	reportEventContext.ipVersion=groupEntry->ipVersion;
	#ifdef CONFIG_RTL_MLD_SNOOPING
	reportEventContext.groupAddr[0]=groupEntry->groupAddr[0];
	reportEventContext.groupAddr[1]=groupEntry->groupAddr[1];
	reportEventContext.groupAddr[2]=groupEntry->groupAddr[2];
	reportEventContext.groupAddr[3]=groupEntry->groupAddr[3];
	#else
	reportEventContext.groupAddr[0]=groupEntry->groupAddr[0];
	#endif
	
	#ifdef CONFIG_RECORD_MCAST_FLOW
	rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
	#endif
		
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
	if(groupEntry->ipVersion==IP_VERSION4)
	{
		/*we only support ipv4 hardware multicast*/
		strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
		rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
		rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
	}
	#endif
	
	return SUCCESS;
}

static int32 rtl_delStaticGroupEntry(uint32 moduleIndex,struct rtl_groupEntry* groupEntry)
{
	struct rtl_groupEntry* existGroupEntry=NULL;
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	if(groupEntry==NULL)
	{
		return FAILED;
	}

	if((groupEntry->ipVersion!=IP_VERSION4) && (groupEntry->ipVersion!=IP_VERSION6))
	{
		return FAILED;
	}

	if(groupEntry->ipVersion==IP_VERSION4)
	{
		if((groupEntry->groupAddr[0]!=0) && (!IS_CLASSD_ADDR(groupEntry->groupAddr[0])))
		{
			return FAILED;
		}
	}
	else if(groupEntry->ipVersion==IP_VERSION6)
	{
		/*to-do:check ipv6 address range */
	}

	if(groupEntry->ipVersion==IP_VERSION4)
	{
		if(groupEntry->groupAddr[0]==0)
		{
			rtl_mCastModuleArray[moduleIndex].ipv4UnknownMCastFloodMap=DEFAULT_IPV4_UNKNOWN_MCAST_FLOOD_MAP;
			return SUCCESS;
		}
	}
	else if(groupEntry->ipVersion==IP_VERSION6)
	{
		if(	(groupEntry->groupAddr[0]==0) &&
			(groupEntry->groupAddr[0]==0) &&
			(groupEntry->groupAddr[0]==0) &&
			(groupEntry->groupAddr[0]==0)	)
		{
#ifdef CONFIG_RTL_MLD_SNOOPING
			rtl_mCastModuleArray[moduleIndex].ipv6UnknownMCastFloodMap=DEFAULT_IPV6_UNKNOWN_MCAST_FLOOD_MAP;
#endif
			return SUCCESS;
		}
	}

	existGroupEntry=rtl_searchGroupEntry(moduleIndex, groupEntry->ipVersion, groupEntry->groupAddr);
	if((existGroupEntry!=NULL) && (existGroupEntry->attribute==STATIC_RESERVED_MULTICAST))
	{
		existGroupEntry->attribute=0;
		existGroupEntry->staticFwdPortMask=0;
		
		reportEventContext.moduleIndex=moduleIndex;
		reportEventContext.ipVersion=groupEntry->ipVersion;
		#ifdef CONFIG_RTL_MLD_SNOOPING
		reportEventContext.groupAddr[0]=groupEntry->groupAddr[0];
		reportEventContext.groupAddr[1]=groupEntry->groupAddr[1];
		reportEventContext.groupAddr[2]=groupEntry->groupAddr[2];
		reportEventContext.groupAddr[3]=groupEntry->groupAddr[3];
		#else
		reportEventContext.groupAddr[0]=groupEntry->groupAddr[0];
		#endif
		
		#ifdef CONFIG_RECORD_MCAST_FLOW
		rtl_invalidateMCastFlow(reportEventContext.moduleIndex, reportEventContext.ipVersion, reportEventContext.groupAddr);
		#endif
			
		#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
		if(groupEntry->ipVersion==IP_VERSION4)
		{
			/*we only support ipv4 hardware multicast*/
			strcpy(reportEventContext.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.devName);
			rtl865x_raiseEvent(EVENT_UPDATE_MCAST, &reportEventContext);
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
			rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &reportEventContext);
#endif
		}
		#endif	
	}
		
	return SUCCESS;
}

int32 rtl_addSpecialMCast(uint32 moduleIndex, uint32 ipVersion, uint32 *groupAddr, uint32 fwdPortMask)
{

	struct rtl_groupEntry groupEntry;
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	if(groupAddr==NULL)
	{
		return FAILED;
	}

	if((ipVersion!=IP_VERSION4) && (ipVersion!=IP_VERSION6))
	{
		return FAILED;
	}
	
	memset(&groupEntry, 0 ,sizeof(struct rtl_groupEntry));
	groupEntry.ipVersion=ipVersion;
	
	if(ipVersion ==IP_VERSION4)
	{
		groupEntry.groupAddr[0]=groupAddr[0];
	}
	else
	{
		groupEntry.groupAddr[0]=groupAddr[0];
		groupEntry.groupAddr[1]=groupAddr[1];
		groupEntry.groupAddr[2]=groupAddr[2];
		groupEntry.groupAddr[3]=groupAddr[3];
	}

	groupEntry.staticFwdPortMask=fwdPortMask;
	
	return rtl_addStaticGroupEntry(moduleIndex, &groupEntry);
}

int32 rtl_delSpecialMCast(uint32 moduleIndex, uint32 ipVersion, uint32 *groupAddr)
{

	struct rtl_groupEntry groupEntry;
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return FAILED;
	}

	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==FALSE)
	{
		return FAILED;
	}
	
	
	if((ipVersion!=IP_VERSION4) && (ipVersion!=IP_VERSION6))
	{
		return FAILED;
	}
	
	memset(&groupEntry, 0 ,sizeof(struct rtl_groupEntry));
	groupEntry.ipVersion=ipVersion;
	
	if(ipVersion ==IP_VERSION4)
	{
		groupEntry.groupAddr[0]=groupAddr[0];
	}
	else
	{
		groupEntry.groupAddr[0]=groupAddr[0];
		groupEntry.groupAddr[1]=groupAddr[1];
		groupEntry.groupAddr[2]=groupAddr[2];
		groupEntry.groupAddr[3]=groupAddr[3];
	}

	return rtl_delStaticGroupEntry(moduleIndex, &groupEntry);
}
#endif


int rtl_add_ReservedMCastAddr(uint32 groupAddr, int flag)
{
	int ret =FAILED;
	int i=0;
	if((groupAddr & 0xf0000000) != 0xe0000000)
		goto OUT;
	for(i=0; i<MAX_RESERVED_MULTICAST_NUM; i++)
	{
		if((reservedMCastRecord[i].valid==0)&&(flag==ADD_RESERVED_MULTICAST_FLAG))
		{
			reservedMCastRecord[i].valid =1;
			reservedMCastRecord[i].groupAddr=groupAddr;
			ret =SUCCESS;
			goto OUT;
		}
		else if((reservedMCastRecord[i].valid==1)
			&&(flag==DEL_RESERVED_MULTICAST_FLAG)
			&&(reservedMCastRecord[i].groupAddr== groupAddr)
			)
		{		
			reservedMCastRecord[i].valid =0;			
			ret =SUCCESS;			
			goto OUT;
		}
	}
	
OUT:
	return ret;
}

int rtl_check_ReservedMCastAddr(uint32 groupAddr)
{
	int ret =FAILED;
	int i=0;

	if((groupAddr & 0xf0000000) != 0xe0000000)
		goto OUT;
	for(i=0; i<MAX_RESERVED_MULTICAST_NUM; i++)
	{
		if((reservedMCastRecord[i].valid==1)
		&&(reservedMCastRecord[i].groupAddr== groupAddr)
		)
		{		
			ret =SUCCESS;			
			goto OUT;
		}
	}
	
OUT:
	return ret;
}

#ifdef CONFIG_PROC_FS
int igmp_show(struct seq_file *s, void *v)
{
	int32 moduleIndex;
	int32 hashIndex,groupCnt,clientCnt;
	struct rtl_groupEntry *groupEntryPtr;
	struct rtl_clientEntry* clientEntry=NULL;
	struct rtl_sourceEntry *sourceEntryPtr;
	#ifdef CONFIG_RECORD_MCAST_FLOW	
	int32 flowCnt;
	struct rtl_mcastFlowEntry *mcastFlowEntry=NULL; 
	#endif
	#if defined (CONFIG_RTL_MLD_SNOOPING)	
	int mldVersion;
	#endif
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			seq_printf(s, "-------------------------------------------------------------------------\n");
			seq_printf(s, "module index:%d, ",moduleIndex);
			#ifdef CONFIG_RTL_HARDWARE_MULTICAST
			seq_printf(s, "device:%s, portMask:0x%x,swPortMask:0x%x ",rtl_mCastModuleArray[moduleIndex].deviceInfo.devName,rtl_mCastModuleArray[moduleIndex].deviceInfo.portMask,rtl_mCastModuleArray[moduleIndex].deviceInfo.swPortMask);
			#endif
			seq_printf(s, "fastleave:%d ipv4[0x%x]",rtl_mCastModuleArray[moduleIndex].enableFastLeave,rtl_mCastModuleArray[moduleIndex].ipv4UnknownMCastFloodMap);
			#if defined (CONFIG_RTL_MLD_SNOOPING)	
			seq_printf(s, "ipv6[0x%x]\n",rtl_mCastModuleArray[moduleIndex].ipv6UnknownMCastFloodMap);
			#endif
			seq_printf(s, "\n");
			seq_printf(s,"igmp list:\n");
			groupCnt=0;	
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
			{
				for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
					while(groupEntryPtr!=NULL)
					{
						groupCnt++;
						seq_printf(s, "    [%d] Group address:%d.%d.%d.%d",groupCnt,//hashIndex
						groupEntryPtr->groupAddr[0]>>24, (groupEntryPtr->groupAddr[0]&0x00ff0000)>>16,
						(groupEntryPtr->groupAddr[0]&0x0000ff00)>>8, (groupEntryPtr->groupAddr[0]&0xff));
					
						#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
						if(groupEntryPtr->attribute==STATIC_RESERVED_MULTICAST)
						{
							seq_printf(s, " <static> portMask:0x%x",groupEntryPtr->staticFwdPortMask);
						}
						#endif
						seq_printf(s, "\n");
					
						clientEntry=groupEntryPtr->clientList;
					
						clientCnt=0;
						while (clientEntry!=NULL)
						{	
						
							clientCnt++;
							seq_printf(s, "        <%d>%d.%d.%d.%d",clientCnt,clientEntry->clientAddr[0]>>24, (clientEntry->clientAddr[0]&0x00ff0000)>>16,
								(clientEntry->clientAddr[0]&0x0000ff00)>>8, clientEntry->clientAddr[0]&0xff);
							#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)
							seq_printf(s, "(%02X-%02X-%02X-%02X-%02X-%02X)",clientEntry->clientMacAddr[0],clientEntry->clientMacAddr[1],clientEntry->clientMacAddr[2],
								clientEntry->clientMacAddr[3],clientEntry->clientMacAddr[4],clientEntry->clientMacAddr[5]);
							#endif
							seq_printf(s, "\\port %d\\IGMPv%d\\",clientEntry->portNum, clientEntry->igmpVersion);
						
							seq_printf(s, "%s",(clientEntry->groupFilterTimer>rtl_sysUpSeconds)?"EXCLUDE":"INCLUDE");
							if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
							{
								seq_printf(s, ":%ds",clientEntry->groupFilterTimer-rtl_sysUpSeconds);
							}
							else
							{
								seq_printf(s, ":0s");
							}
						
							sourceEntryPtr=clientEntry->sourceList;
							if(sourceEntryPtr!=NULL)
							{
								seq_printf(s, "\\source list:");
							}

							while(sourceEntryPtr!=NULL)
							{
								seq_printf(s, "%d.%d.%d.%d:",
										sourceEntryPtr->sourceAddr[0]>>24, (sourceEntryPtr->sourceAddr[0]&0x00ff0000)>>16,
										(sourceEntryPtr->sourceAddr[0]&0x0000ff00)>>8, (sourceEntryPtr->sourceAddr[0]&0xff));
						
								if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
								{
									seq_printf(s, "%ds",sourceEntryPtr->portTimer-rtl_sysUpSeconds);
								}
								else
								{
									seq_printf(s, "0s");
								}

								if(sourceEntryPtr->next!=NULL)
								{
									seq_printf(s, ", ");
								}
								
								sourceEntryPtr=sourceEntryPtr->next;
							}

						
							seq_printf(s, "\n");
							clientEntry = clientEntry->next;
						}
						
						seq_printf(s, "\n");	
						groupEntryPtr=groupEntryPtr->next;	
					}
				
		       	}
			}
			if(groupCnt==0)
			{
				seq_printf(s,"    <empty>\n");
			}
		
#if defined (CONFIG_RTL_MLD_SNOOPING)			
			seq_printf(s, "\n\n");
			seq_printf(s, "mld list:\n");
			groupCnt=0;	
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
			{
				for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{
					groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
					while(groupEntryPtr!=NULL)
					{
						groupCnt++;	
						seq_printf(s, "    [%d] Group address:%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x",groupCnt,
						(groupEntryPtr->groupAddr[0])>>28,(groupEntryPtr->groupAddr[0]<<4)>>28, (groupEntryPtr->groupAddr[0]<<8)>>28,(groupEntryPtr->groupAddr[0]<<12)>>28, 
						(groupEntryPtr->groupAddr[0]<<16)>>28,(groupEntryPtr->groupAddr[0]<<20)>>28,(groupEntryPtr->groupAddr[0]<<24)>>28, (groupEntryPtr->groupAddr[0]<<28)>>28, 
						(groupEntryPtr->groupAddr[1])>>28,(groupEntryPtr->groupAddr[1]<<4)>>28, (groupEntryPtr->groupAddr[1]<<8)>>28,(groupEntryPtr->groupAddr[1]<<12)>>28, 
						(groupEntryPtr->groupAddr[1]<<16)>>28,(groupEntryPtr->groupAddr[1]<<20)>>28,(groupEntryPtr->groupAddr[1]<<24)>>28, (groupEntryPtr->groupAddr[1]<<28)>>28, 
						(groupEntryPtr->groupAddr[2])>>28,(groupEntryPtr->groupAddr[2]<<4)>>28, (groupEntryPtr->groupAddr[2]<<8)>>28,(groupEntryPtr->groupAddr[2]<<12)>>28, 
						(groupEntryPtr->groupAddr[2]<<16)>>28,(groupEntryPtr->groupAddr[2]<<20)>>28,(groupEntryPtr->groupAddr[2]<<24)>>28, (groupEntryPtr->groupAddr[2]<<28)>>28, 
						(groupEntryPtr->groupAddr[3])>>28,(groupEntryPtr->groupAddr[3]<<4)>>28, (groupEntryPtr->groupAddr[3]<<8)>>28,(groupEntryPtr->groupAddr[3]<<12)>>28, 
						(groupEntryPtr->groupAddr[3]<<16)>>28,(groupEntryPtr->groupAddr[3]<<20)>>28,(groupEntryPtr->groupAddr[3]<<24)>>28, (groupEntryPtr->groupAddr[3]<<28)>>28);
						#if defined (CONFIG_STATIC_RESERVED_MULTICAST)
						if(groupEntryPtr->attribute==STATIC_RESERVED_MULTICAST)
						{
							seq_printf(s, " <static> portMask:0x%x",groupEntryPtr->staticFwdPortMask);
						}
						#endif
						seq_printf(s, "\n");
						clientEntry=groupEntryPtr->clientList;
						
						clientCnt=0;
						while (clientEntry!=NULL)
						{	
							if(clientEntry->igmpVersion==IGMP_V3)
							{
								mldVersion = MLD_V2;
							}
							else
							{
								mldVersion = MLD_V1;
							}
							clientCnt++;
							seq_printf(s, "        <%d>%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x",clientCnt,
								(clientEntry->clientAddr[0])>>28,(clientEntry->clientAddr[0]<<4)>>28, (clientEntry->clientAddr[0]<<8)>>28,(clientEntry->clientAddr[0]<<12)>>28, 
								(clientEntry->clientAddr[0]<<16)>>28,(clientEntry->clientAddr[0]<<20)>>28,(clientEntry->clientAddr[0]<<24)>>28, (clientEntry->clientAddr[0]<<28)>>28, 
								(clientEntry->clientAddr[1])>>28,(clientEntry->clientAddr[1]<<4)>>28, (clientEntry->clientAddr[1]<<8)>>28,(clientEntry->clientAddr[1]<<12)>>28, 
								(clientEntry->clientAddr[1]<<16)>>28,(clientEntry->clientAddr[1]<<20)>>28,(clientEntry->clientAddr[1]<<24)>>28, (clientEntry->clientAddr[1]<<28)>>28, 
								(clientEntry->clientAddr[2])>>28,(clientEntry->clientAddr[2]<<4)>>28, (clientEntry->clientAddr[2]<<8)>>28,(clientEntry->clientAddr[2]<<12)>>28, 
								(clientEntry->clientAddr[2]<<16)>>28,(clientEntry->clientAddr[2]<<20)>>28,(clientEntry->clientAddr[2]<<24)>>28, (clientEntry->clientAddr[2]<<28)>>28, 
								(clientEntry->clientAddr[3])>>28,(clientEntry->clientAddr[3]<<4)>>28, (clientEntry->clientAddr[3]<<8)>>28,(clientEntry->clientAddr[3]<<12)>>28, 
								(clientEntry->clientAddr[3]<<16)>>28,(clientEntry->clientAddr[3]<<20)>>28,(clientEntry->clientAddr[3]<<24)>>28, (clientEntry->clientAddr[3]<<28)>>28); 
						#if defined (CONFIG_RTL_IGMPSNOOPING_MAC_BASED) || defined (M2U_DELETE_CHECK)
							seq_printf(s, "(%02X-%02X-%02X-%02X-%02X-%02X)",clientEntry->clientMacAddr[0],clientEntry->clientMacAddr[1],clientEntry->clientMacAddr[2],
									clientEntry->clientMacAddr[3],clientEntry->clientMacAddr[4],clientEntry->clientMacAddr[5]);
							#endif
							seq_printf(s, "\\port %d\\MLDv%d\\",clientEntry->portNum, mldVersion);
							seq_printf(s, "%s",(clientEntry->groupFilterTimer>rtl_sysUpSeconds)?"EXCLUDE":"INCLUDE");
							if(clientEntry->groupFilterTimer>rtl_sysUpSeconds)
							{
								seq_printf(s, ":%ds",clientEntry->groupFilterTimer-rtl_sysUpSeconds);
							}
							else
							{
								seq_printf(s, ":0s");
							}
							
							sourceEntryPtr=clientEntry->sourceList;
							if(sourceEntryPtr!=NULL)
							{
								seq_printf(s, "\\source list:");
							}

							while(sourceEntryPtr!=NULL)
							{
								seq_printf(s, "%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x-%x%x%x%x%x%x%x%x:",
									(sourceEntryPtr->sourceAddr[0])>>28,(sourceEntryPtr->sourceAddr[0]<<4)>>28, (sourceEntryPtr->sourceAddr[0]<<8)>>28,(sourceEntryPtr->sourceAddr[0]<<12)>>28, 
									(sourceEntryPtr->sourceAddr[0]<<16)>>28,(sourceEntryPtr->sourceAddr[0]<<20)>>28,(sourceEntryPtr->sourceAddr[0]<<24)>>28, (sourceEntryPtr->sourceAddr[0]<<28)>>28, 
									(sourceEntryPtr->sourceAddr[1])>>28,(sourceEntryPtr->sourceAddr[1]<<4)>>28, (sourceEntryPtr->sourceAddr[1]<<8)>>28,(sourceEntryPtr->sourceAddr[1]<<12)>>28, 
									(sourceEntryPtr->sourceAddr[1]<<16)>>28,(sourceEntryPtr->sourceAddr[1]<<20)>>28,(sourceEntryPtr->sourceAddr[1]<<24)>>28, (sourceEntryPtr->sourceAddr[1]<<28)>>28, 
									(sourceEntryPtr->sourceAddr[2])>>28,(sourceEntryPtr->sourceAddr[2]<<4)>>28, (sourceEntryPtr->sourceAddr[2]<<8)>>28,(sourceEntryPtr->sourceAddr[2]<<12)>>28, 
									(sourceEntryPtr->sourceAddr[2]<<16)>>28,(sourceEntryPtr->sourceAddr[2]<<20)>>28,(sourceEntryPtr->sourceAddr[2]<<24)>>28, (sourceEntryPtr->sourceAddr[2]<<28)>>28, 
									(sourceEntryPtr->sourceAddr[3])>>28,(sourceEntryPtr->sourceAddr[3]<<4)>>28, (sourceEntryPtr->sourceAddr[3]<<8)>>28,(sourceEntryPtr->sourceAddr[3]<<12)>>28, 
									(sourceEntryPtr->sourceAddr[3]<<16)>>28,(sourceEntryPtr->sourceAddr[3]<<20)>>28,(sourceEntryPtr->sourceAddr[3]<<24)>>28, (sourceEntryPtr->sourceAddr[3]<<28)>>28);
						
								if(sourceEntryPtr->portTimer>rtl_sysUpSeconds)
								{
									seq_printf(s, "%ds",sourceEntryPtr->portTimer-rtl_sysUpSeconds);
								}
								else
								{
									seq_printf(s, "0s");
								}

								if(sourceEntryPtr->next!=NULL)
								{
									seq_printf(s, ", ");
								}
								
								sourceEntryPtr=sourceEntryPtr->next;
							}

							seq_printf(s, "\n");
							clientEntry = clientEntry->next;
						}
						
						seq_printf(s, "\n");	
						groupEntryPtr=groupEntryPtr->next;	
					}
				
		       	}
			}
			if(groupCnt==0)
			{
				seq_printf(s,"    <empty>\n");
			}
#endif			
#ifdef CONFIG_RECORD_MCAST_FLOW	

			if(rtl_mCastModuleArray[moduleIndex].flowHashTable != NULL)
			{
				seq_printf(s,"ipv4 flow list:\n");
				flowCnt=1;
				for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{

					/*to dump multicast flow information*/
		     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
				
					while(mcastFlowEntry!=NULL)
					{
						if(mcastFlowEntry->ipVersion==IP_VERSION4)
						{
							seq_printf(s, "    [%d] %d.%d.%d.%d-->",flowCnt,
							mcastFlowEntry->serverAddr[0]>>24, (mcastFlowEntry->serverAddr[0]&0x00ff0000)>>16,
							(mcastFlowEntry->serverAddr[0]&0x0000ff00)>>8, (mcastFlowEntry->serverAddr[0]&0xff));
						
							seq_printf(s, "%d.%d.%d.%d-->",
							mcastFlowEntry->groupAddr[0]>>24, (mcastFlowEntry->groupAddr[0]&0x00ff0000)>>16,
							(mcastFlowEntry->groupAddr[0]&0x0000ff00)>>8, (mcastFlowEntry->groupAddr[0]&0xff));
			
							seq_printf(s, "port mask:0x%x\n",mcastFlowEntry->multicastFwdInfo.fwdPortMask);
						}

						flowCnt++;
						mcastFlowEntry=mcastFlowEntry->next;
					}

				}
				seq_printf(s, "ipv6 flow list:\n");
				flowCnt=1;
				for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
		     	{

					/*to dump multicast flow information*/
		     		mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];
				
					while(mcastFlowEntry!=NULL)
					{
						if(mcastFlowEntry->ipVersion==IP_VERSION6)
						{
							seq_printf(s, "    [%d] %x-%x-%x-%x-->",flowCnt,
							mcastFlowEntry->serverAddr[0], mcastFlowEntry->serverAddr[1],
							mcastFlowEntry->serverAddr[2], (mcastFlowEntry->serverAddr[3]);
							
							seq_printf(s, "%x-%x-%x-%x-->",
							mcastFlowEntry->groupAddr[1], mcastFlowEntry->groupAddr[1],
							mcastFlowEntry->groupAddr[2], mcastFlowEntry->groupAddr[3];
				
							seq_printf(s, "port mask:0x%x\n",mcastFlowEntry->multicastFwdInfo.fwdPortMask);
						}

						flowCnt++;
						mcastFlowEntry=mcastFlowEntry->next;
					}

				}
			}
#endif
		}
	}

	seq_printf(s, "------------------------------------------------------------------\n");
	return SUCCESS;
}


int igmp_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
	
{
#if defined (CONFIG_STATIC_RESERVED_MULTICAST)	
	char tmp[256];
	
	char		*strptr, *cmd_addr;
	char		*tokptr;
	uint32	allModuleFlag=FALSE;
	uint32	moduleIndex=0xFFFFFFFF;
	uint32 	ipAddr[4];
	int cnt;
	
	struct rtl_groupEntry groupEntry;

	
	if (count < 5)
		return -EFAULT;


	if (buffer && !copy_from_user(tmp, buffer, count)) {

		tmp[count] = '\0';
		strptr=tmp;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}
	
		if (	(!memcmp(cmd_addr, "add", 3)) || 
			(!memcmp(cmd_addr, "Add", 3)) ||
			(!memcmp(cmd_addr, "ADD", 3))	)
		{

			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}

			if(	(!memcmp(tokptr, "all", 3)) || 
				(!memcmp(tokptr, "All", 3)) ||
				(!memcmp(tokptr, "ALL", 3))	)	
			{
				allModuleFlag=TRUE;
			}
			else
			{
				allModuleFlag=FALSE;
				for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
				{
					if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
					{
						if(strcmp(rtl_mCastModuleArray[moduleIndex].deviceInfo.devName,tokptr)==0)
						{
							break;
						}
					}
				}

				if(moduleIndex>=MAX_MCAST_MODULE_NUM)
				{
					goto errout;
				}
			}
			
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			
			if(	(!memcmp(tokptr, "ipv4", 4)) || 
				(!memcmp(tokptr, "Ipv4", 4)) ||
				(!memcmp(tokptr, "IPV4", 4))	)	
			{
				groupEntry.ipVersion=IP_VERSION4;
			}
			#if defined(CONFIG_RTL_MLD_SNOOPING)
			else if (	(!memcmp(tokptr, "ipv6", 4)) || 
					(!memcmp(tokptr, "Ipv6", 4)) ||
					(!memcmp(tokptr, "IPV6", 4))	)
			{
				groupEntry.ipVersion=IP_VERSION6;

			}
			#endif
			else 
			{
				goto errout;
			}
			
			if(groupEntry.ipVersion==IP_VERSION4)
			{
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				
				cnt = sscanf(tokptr, "%d.%d.%d.%d", &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]);
		
				groupEntry.groupAddr[0]=(ipAddr[0]<<24)|(ipAddr[1]<<16)|(ipAddr[2]<<8)|(ipAddr[3]);
				groupEntry.groupAddr[1]=0;
				groupEntry.groupAddr[2]=0;
				groupEntry.groupAddr[3]=0;

				
			
			}
			#if defined(CONFIG_RTL_MLD_SNOOPING)
			else if (groupEntry.ipVersion==IP_VERSION6)
			{
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				
				cnt = sscanf(tokptr, "0x%x-%x-%x-%x", &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]);

				groupEntry.groupAddr[0]=ipAddr[0];
				groupEntry.groupAddr[1]=ipAddr[1];
				groupEntry.groupAddr[2]=ipAddr[2];
				groupEntry.groupAddr[3]=ipAddr[3];
			}
			#endif

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			
			groupEntry.staticFwdPortMask=simple_strtol(tokptr, NULL, 0);
			if(allModuleFlag==TRUE)
			{
				for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
				{
					if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
					{
						rtl_addStaticGroupEntry(moduleIndex, &groupEntry);
					}
				}
				
			}
			else
			{
				rtl_addStaticGroupEntry(moduleIndex, &groupEntry);
			}
			
		}
		else if (	(!memcmp(cmd_addr, "del", 3)) || 
			(!memcmp(cmd_addr, "Del", 3)) ||
			(!memcmp(cmd_addr, "DEL", 3))	)
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}

			if(	(!memcmp(tokptr, "all", 3)) || 
				(!memcmp(tokptr, "All", 3)) ||
				(!memcmp(tokptr, "ALL", 3))	)	
			{
				allModuleFlag=TRUE;
			}
			else
			{
				allModuleFlag=FALSE;
				for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
				{
					if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
					{
						if(strcmp(rtl_mCastModuleArray[moduleIndex].deviceInfo.devName,tokptr)==0)
						{
							break;
						}
					}
				}

				if(moduleIndex>=MAX_MCAST_MODULE_NUM)
				{
					goto errout;
				}
			}

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			
			if(	(!memcmp(tokptr, "ipv4", 4)) || 
				(!memcmp(tokptr, "Ipv4", 4)) ||
				(!memcmp(tokptr, "IPV4", 4))	)	
			{
				groupEntry.ipVersion=IP_VERSION4;
			}
			#if defined(CONFIG_RTL_MLD_SNOOPING)
			else if (	(!memcmp(tokptr, "ipv6", 4)) || 
					(!memcmp(tokptr, "Ipv6", 4)) ||
					(!memcmp(tokptr, "IPV6", 4))	)
			{
				groupEntry.ipVersion=IP_VERSION6;

			}
			#endif
			else 
			{
				goto errout;
			}
			
			if(groupEntry.ipVersion==IP_VERSION4)
			{
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				cnt = sscanf(tokptr, "%d.%d.%d.%d", &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]);
				
				groupEntry.groupAddr[0]=(ipAddr[0]<<24)|(ipAddr[1]<<16)|(ipAddr[2]<<8)|(ipAddr[3]);
				groupEntry.groupAddr[1]=0;
				groupEntry.groupAddr[2]=0;
				groupEntry.groupAddr[3]=0;

				
			
			}
			
			#if defined(CONFIG_RTL_MLD_SNOOPING)
			else if (groupEntry.ipVersion==IP_VERSION6)
			{
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				cnt = sscanf(tokptr, "0x%x-%x-%x-%x", &ipAddr[0], &ipAddr[1], &ipAddr[2], &ipAddr[3]);
				
				groupEntry.groupAddr[0]=ipAddr[0];
				groupEntry.groupAddr[1]=ipAddr[1];
				groupEntry.groupAddr[2]=ipAddr[2];
				groupEntry.groupAddr[3]=ipAddr[3];
			}
			#endif
			
			if(allModuleFlag==TRUE)
			{
				for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
				{
					if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
					{
						rtl_delStaticGroupEntry(moduleIndex, &groupEntry);
					}
				}
				
			}
			else
			{
				rtl_delStaticGroupEntry(moduleIndex, &groupEntry);
			}
			
		}
		else if (	(!memcmp(cmd_addr, "flush", 4)) || 
			(!memcmp(cmd_addr, "Flush", 4)) ||
			(!memcmp(cmd_addr, "FLUSH", 4))	)
		{
			#if defined(CONFIG_RTL_MLD_SNOOPING)
			rtl_flushAllIgmpRecord(FLUSH_IGMP_RECORD|FLUSH_MLD_RECORD);
			#else
			rtl_flushAllIgmpRecord(FLUSH_IGMP_RECORD);
			#endif
		}
		else
		{
errout:
			printk("error input!\n");
		}

	
	}
#endif
	return count;	
}
#endif

#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
int rtl_get_brIgmpModuleIndexbyId(int idx,char *name);
#endif

void rtl865x_igmpLinkStatusChangeCallback(uint32 moduleIndex, rtl_igmpPortInfo_t * portInfo)
{
	int32 hashIndex;
	int32 clearFlag=FALSE;
	struct rtl_groupEntry *groupEntryPtr, *brGroupEntryPtr = NULL;
	struct rtl_clientEntry* clientEntry=NULL, *brClientEntryPtr = NULL;
	struct rtl_clientEntry* nextClientEntry=NULL;
	#ifdef CONFIG_RECORD_MCAST_FLOW
	struct rtl_mcastFlowEntry *mcastFlowEntry, *nextMcastFlowEntry;
	#endif

	uint32 groupAddress[4] = {0};
	uint32 clientAddress[4] = {0};
	
	#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	struct rtl_groupEntry *brGroupEntryPtr_2 = NULL;
	struct rtl_clientEntry *brClientEntryPtr_2 = NULL;
	#endif

	#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
	int br_index = 0;
	unsigned char br_name[16]={0};
	unsigned int igmpModuleIndex=0xFFFFFFFF;
	#endif
	
	if(portInfo==NULL)
	{
		return ;
	}
	
	if(moduleIndex>=MAX_MCAST_MODULE_NUM)
	{
		return ;
	}
	
#ifdef CONFIG_RECORD_MCAST_FLOW	
	if(rtl_mCastModuleArray[moduleIndex].flowHashTable != NULL)
	{
		for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
	    {

	     	mcastFlowEntry=rtl_mCastModuleArray[moduleIndex].flowHashTable[hashIndex];

			while(mcastFlowEntry!=NULL)
			{
				nextMcastFlowEntry=mcastFlowEntry->next;
				
				/*clear multicast forward flow cache*/
				rtl_deleteMcastFlowEntry( mcastFlowEntry, rtl_mCastModuleArray[moduleIndex].flowHashTable);
				
				mcastFlowEntry=nextMcastFlowEntry;
			}

		}
	}
#endif


	if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
	{

		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
		{
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
	     	{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
				while(groupEntryPtr!=NULL)
				{				
					clientEntry=groupEntryPtr->clientList;
					while (clientEntry!=NULL)
					{	
						/*save next client entry first*/
						nextClientEntry=clientEntry->next;
						if(((1<<clientEntry->portNum) & portInfo->linkPortMask)==0)
						{
							groupAddress[0] = groupEntryPtr->groupAddr[0];
							clientAddress[0] = clientEntry->clientAddr[0];
							//this client will be deleted, the portNum is meanlingless
						#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
							for(br_index=0; br_index<RTL_IMGP_MAX_BRMODULE; br_index++)
							{
								igmpModuleIndex = rtl_get_brIgmpModuleIndexbyId(br_index,br_name);
								if(igmpModuleIndex != 0xFFFFFFFF)
								{
									brGroupEntryPtr = rtl_searchGroupEntry(igmpModuleIndex, 4, groupAddress);
									if(brGroupEntryPtr != NULL)
									{
										brClientEntryPtr = rtl_searchClientEntry(4,brGroupEntryPtr,MAX_SUPPORT_PORT_NUMBER,clientAddress);
										if(brClientEntryPtr != NULL)
											rtl_deleteClientEntry(brGroupEntryPtr,brClientEntryPtr);
									}
								}
							}
						#else
							brGroupEntryPtr = rtl_searchGroupEntry(brIgmpModuleIndex, 4, groupAddress);
					
							#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
							brGroupEntryPtr_2 = rtl_searchGroupEntry(brIgmpModuleIndex_2, 4, groupAddress);;
							#endif
							if(brGroupEntryPtr != NULL)
							{
								brClientEntryPtr = rtl_searchClientEntry(4,brGroupEntryPtr,MAX_SUPPORT_PORT_NUMBER,clientAddress);
								if(brClientEntryPtr != NULL)
									rtl_deleteClientEntry(brGroupEntryPtr,brClientEntryPtr);
							}
							#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
							if(brGroupEntryPtr_2 != NULL)
							{
								brClientEntryPtr_2 = rtl_searchClientEntry(4,brGroupEntryPtr_2,MAX_SUPPORT_PORT_NUMBER,clientAddress);
								if(brClientEntryPtr_2 != NULL)
									rtl_deleteClientEntry(brGroupEntryPtr,brClientEntryPtr);
							}
							#endif
						#endif
							rtl_deleteClientEntry(groupEntryPtr,clientEntry);
							clearFlag=TRUE;
						}
						
			
						clientEntry = nextClientEntry;
					}
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
					if(clearFlag==TRUE)
					{
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
					linkEventContext.ipVersion = 4;
#endif
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
#if defined (CONFIG_RTL_L2_HW_MULTICAST_SUPPORT)
					rtl865x_raiseEvent(EVENT_UPDATE_L2_MCAST, &linkEventContext);
#endif
					}
#endif
					groupEntryPtr=groupEntryPtr->next;	
				}
			
	       	}
		}
#if defined (CONFIG_RTL_MLD_SNOOPING)
		if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
		{
			for(hashIndex=0;hashIndex<rtl_hashTableSize;hashIndex++)
	     	{
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
				while(groupEntryPtr!=NULL)
				{
					clientEntry=groupEntryPtr->clientList;
					while (clientEntry!=NULL)
					{	
						/*save next client entry first*/
						nextClientEntry=clientEntry->next;
						if(((1<<clientEntry->portNum) & portInfo->linkPortMask)==0)
						{
							//this client will be deleted, the portNum is meanlingless
								groupAddress[0] = groupEntryPtr->groupAddr[0];
								groupAddress[1] = groupEntryPtr->groupAddr[1];
								groupAddress[2] = groupEntryPtr->groupAddr[2];
								groupAddress[3] = groupEntryPtr->groupAddr[3];
								clientAddress[0] = clientEntry->clientAddr[0];
								clientAddress[1] = clientEntry->clientAddr[1];
								clientAddress[2] = clientEntry->clientAddr[2];
								clientAddress[3] = clientEntry->clientAddr[3];
								//this client will be deleted, the portNum is meanlingless
						#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
								for(br_index=0; br_index<RTL_IMGP_MAX_BRMODULE; br_index++)
								{
									igmpModuleIndex = rtl_get_brIgmpModuleIndexbyId(br_index,br_name);
									if(igmpModuleIndex != 0xFFFFFFFF)
									{
										brGroupEntryPtr = rtl_searchGroupEntry(igmpModuleIndex, 6, groupAddress);
										if(brGroupEntryPtr != NULL)
										{
											brClientEntryPtr = rtl_searchClientEntry(6,brGroupEntryPtr,MAX_SUPPORT_PORT_NUMBER,clientAddress);
											if(brClientEntryPtr != NULL)
												rtl_deleteClientEntry(brGroupEntryPtr,brClientEntryPtr);
										}
									}
								}
						#else
								brGroupEntryPtr = rtl_searchGroupEntry(brIgmpModuleIndex, 6, groupAddress);
							
								if(brGroupEntryPtr != NULL)
								{
									brClientEntryPtr = rtl_searchClientEntry(6,brGroupEntryPtr,MAX_SUPPORT_PORT_NUMBER,clientAddress);
									if(brClientEntryPtr != NULL)
										rtl_deleteClientEntry(brGroupEntryPtr,brClientEntryPtr);
								}
						#endif

			
							rtl_deleteClientEntry(groupEntryPtr,clientEntry);
							#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
							clearFlag = TRUE;
							#endif
						}
					
						clientEntry = nextClientEntry;
					}
					
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
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
									
						rtl865x_raiseEvent(EVENT_UPDATE_MCAST6, &linkEventContext);
					}
#endif
#endif
					groupEntryPtr=groupEntryPtr->next;	
				}
			
	       	}
		}
#endif		

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
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable != NULL)
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv4HashTable[hashIndex];
			else 
				groupEntryPtr = NULL;
				
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
#if defined(CONFIG_RTL_MLD_SNOOPING)
int32 rtl_getGroupInfov6(uint32 * groupAddr,struct rtl_groupInfo * groupInfo)
{
	int32 moduleIndex;
	int32 hashIndex;
	struct rtl_groupEntry *groupEntryPtr;
	
	if(groupAddr==NULL||groupInfo==NULL)
	{
		return FAILED;
	}

	memset(groupInfo, 0 , sizeof(struct rtl_groupInfo));
	
	for(moduleIndex=0; moduleIndex<MAX_MCAST_MODULE_NUM ;moduleIndex++)
	{
		if(rtl_mCastModuleArray[moduleIndex].enableSnooping==TRUE)
		{
			hashIndex=rtl_hashMask&groupAddr[3];
			if(rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable != NULL)
				groupEntryPtr=rtl_mCastModuleArray[moduleIndex].rtl_ipv6HashTable[hashIndex];
			else
				groupEntryPtr = NULL;
			
			while(groupEntryPtr!=NULL)
			{
				if(groupEntryPtr->groupAddr[0]==groupAddr[0]&&
				   groupEntryPtr->groupAddr[1]==groupAddr[1]&&
				   groupEntryPtr->groupAddr[2]==groupAddr[2]&&
				   groupEntryPtr->groupAddr[3]==groupAddr[3])
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

#endif

#ifdef M2U_DELETE_CHECK
int FindClientforM2U(unsigned int moduleindex, unsigned char *dMac, unsigned char *sMac)
{
	int32 hashIndex;
	struct rtl_groupEntry *groupEntryPtr = NULL;
	struct rtl_clientEntry* clientEntryPtr = NULL;
	unsigned int groupip[4] = {0,0,0,0};
    int ret = 0;

    #if 0
    printk("dst = %X:%X:%X:%X:%X:%X, src = %X:%X:%X:%X:%X:%X, [%s:%d]\n", 
        dMac[0], dMac[1], dMac[2], dMac[3], dMac[4], dMac[5],
        sMac[0], sMac[1], sMac[2], sMac[3], sMac[4], sMac[5],
        __FUNCTION__, __LINE__);
    #endif
    
    if(dMac[0]==0x01 && dMac[1]==0x00 && dMac[2]==0x5e)
    {
        if(rtl_mCastModuleArray[moduleindex].enableSnooping == TRUE)
        {
        	if(rtl_mCastModuleArray[moduleindex].rtl_ipv4HashTable != NULL)
        	{
            	for(hashIndex = 0; hashIndex < rtl_hashTableSize; ++hashIndex)
	            {
	                groupEntryPtr = rtl_mCastModuleArray[moduleindex].rtl_ipv4HashTable[hashIndex];
	                while(groupEntryPtr != NULL)
	                {
	                    memcpy(groupip, groupEntryPtr->groupAddr, 4);

	                    #if 0
	                    printk("groupip = %X:%X:%X:%X, [%s:%d]\n", 
	                        groupip[0], groupip[1], groupip[2], groupip[3], __FUNCTION__, __LINE__);

	                    #endif
	                    if((dMac[3]==(unsigned char)((groupip[0]>>16)&0xFF))&&
	                       (dMac[4]==(unsigned char)((groupip[0]>>8)&0xFF))&&
	                       (dMac[5]==(unsigned char)(groupip[0]&0xFF)))
	                    {

	                        clientEntryPtr = groupEntryPtr->clientList;
	                        while(clientEntryPtr != NULL)
	                        {
	                            if((sMac[0] == clientEntryPtr->clientMacAddr[0]) &&
	                               (sMac[1] == clientEntryPtr->clientMacAddr[1]) &&
	                               (sMac[2] == clientEntryPtr->clientMacAddr[2]) &&
	                               (sMac[3] == clientEntryPtr->clientMacAddr[3]) &&
	                               (sMac[4] == clientEntryPtr->clientMacAddr[4]) &&
	                               (sMac[5] == clientEntryPtr->clientMacAddr[5]))
	                            {
	                                ret = 1;
	                                break;
	                            }
	                            clientEntryPtr = clientEntryPtr->next;
	                        }

	                        /*client found*/
	                        if(clientEntryPtr)
	                        {
	                            break;
	                        }
	                    }
	                    groupEntryPtr = groupEntryPtr->next;
	                }
	                if(groupEntryPtr)
	                {
	                    break;
	                }
	            }
        	}
        }
    }
#ifdef CONFIG_RTL_MLD_SNOOPING
    else if(dMac[0]==0x33 && dMac[1]==0x33 && dMac[2]!=0x0f)
    {
        if(rtl_mCastModuleArray[moduleindex].enableSnooping == TRUE)
        {
        	if(rtl_mCastModuleArray[moduleindex].rtl_ipv6HashTable != NULL)
        	{
            	for(hashIndex = 0; hashIndex < rtl_hashTableSize; ++hashIndex)
	            {
	                groupEntryPtr = rtl_mCastModuleArray[moduleindex].rtl_ipv6HashTable[hashIndex];
	                while(groupEntryPtr != NULL)
	                {
	                    memcpy(groupip, groupEntryPtr->groupAddr, 16);
						if((dMac[2] == (unsigned char)((groupip[3]>>24)&0xFF)) &&
   						   (dMac[3] == (unsigned char)((groupip[3]>>16)&0xFF)) &&
   						   (dMac[4] == (unsigned char)((groupip[3]>>8)&0xFF)) &&
  						   (dMac[5] == (unsigned char)(groupip[3]&0xFF)))

	                    {
	                        clientEntryPtr = groupEntryPtr->clientList;
	                        while(clientEntryPtr != NULL)
	                        {
	                            if((sMac[0] == clientEntryPtr->clientMacAddr[0]) &&
	                               (sMac[1] == clientEntryPtr->clientMacAddr[1]) &&
	                               (sMac[2] == clientEntryPtr->clientMacAddr[2]) &&
	                               (sMac[3] == clientEntryPtr->clientMacAddr[3]) &&
	                               (sMac[4] == clientEntryPtr->clientMacAddr[4]) &&
	                               (sMac[5] == clientEntryPtr->clientMacAddr[5]))
	                            {
	                                ret = 1;
	                                break;
	                            }
	                            clientEntryPtr = clientEntryPtr->next;
	                        }

	                        /*client found*/
	                        if(clientEntryPtr)
	                            break;
	                    }
	                    groupEntryPtr = groupEntryPtr->next;
	                }
	                if(groupEntryPtr)
	                    break;
	            }
        	}
        }
    }
#endif
return ret;
}

int rtl_M2UDeletecheck(unsigned char *dMac, unsigned char *sMac)
{
#if defined CONFIG_RT_MULTIPLE_BR_SUPPORT	
	int i = 0;
	unsigned char br_name[16]={0};
	unsigned int igmpModuleIndex=0xFFFFFFFF;

	for(i=0; i<RTL_IMGP_MAX_BRMODULE; i++)
	{
		igmpModuleIndex = rtl_get_brIgmpModuleIndexbyId(i,br_name);
		if(igmpModuleIndex!=0xFFFFFFFF && FindClientforM2U(igmpModuleIndex, dMac, sMac))
			return 0;
	}
#else
    if(brIgmpModuleIndex!=0xFFFFFFFF && FindClientforM2U(brIgmpModuleIndex, dMac, sMac))
        return 0;
#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
    if(brIgmpModuleIndex_2!=0xFFFFFFFF && FindClientforM2U(brIgmpModuleIndex_2, dMac, sMac))
        return 0;
#endif
#endif
    return 1;
}

#endif

