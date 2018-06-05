/*
* Copyright c                  Realsil Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program :  igmp snooping function
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/

#ifndef RTL865X_IGMP_SNOOPING_H
#define RTL865X_IGMP_SNOOPING_H

/* multicast configuration*/
struct rtl_mCastSnoopingGlobalConfig
{
	uint32 maxGroupNum;
	uint32 maxClientNum;
	uint32 maxSourceNum;
	uint32 hashTableSize;
	
	uint32 groupMemberAgingTime;                
	uint32 lastMemberAgingTime;			
	uint32 querierPresentInterval;                   
	
	uint32 dvmrpRouterAgingTime;			
	uint32 mospfRouterAgingTime;                     
	uint32 pimRouterAgingTime;     
};

struct rtl_mCastSnoopingLocalConfig
{
	uint8 enableSourceList;
	uint8 enableFastLeave;
	uint8   gatewayMac[6];
	uint32 gatewayIpv4Addr;
	uint32 gatewayIpv6Addr[4];
	//uint32 unknownMcastFloodMap;
	uint32 ipv4UnknownMcastFloodMap;
	uint32 ipv6UnknownMcastFloodMap;
	uint32 staticRouterPortMask;
};

 struct rtl_mCastTimerParameters
 {
	uint32 disableExpire;
	uint32 groupMemberAgingTime;              
	uint32 lastMemberAgingTime;			   
	uint32 querierPresentInterval;                   
	
	uint32 dvmrpRouterAgingTime;				  /*DVMRP multicast router aging time*/
	uint32 mospfRouterAgingTime;                           /*MOSPF multicast router aging time*/
	uint32 pimRouterAgingTime;                          /*PIM-DM multicast router aging time*/
	
};

#define IP_VERSION4 4
#define IP_VERSION6 6

struct rtl_multicastDataInfo
{
	uint32 ipVersion;
	uint32 sourceIp[4];
	uint32 groupAddr[4];

};

struct rtl_multicastFwdInfo
{
	uint8 unknownMCast;
	uint8 reservedMCast;
	uint16 cpuFlag;
	uint32 fwdPortMask;
	
};

struct rtl_groupInfo
{
	uint32 ownerMask;
};

typedef struct rtl_multicastDeviceInfo_s
{
	char devName[32];
	uint32 vlanId;
	uint32 portMask;
	uint32 swPortMask;
}rtl_multicastDeviceInfo_t;


typedef struct rtl_multicastEventContext_s
{
	char devName[16];
	uint32 moduleIndex;
	uint32 ipVersion;
	uint32 groupAddr[4];
	uint32 sourceAddr[4];
	uint32 portMask;
}rtl_multicastEventContext_t;


typedef struct rtl_igmpPortInfo_s
{
	uint32 linkPortMask;
}rtl_igmpPortInfo_t;

#define DEL_RESERVED_MULTICAST_FLAG	0
#define ADD_RESERVED_MULTICAST_FLAG	1
#define MAX_RESERVED_MULTICAST_NUM 16
#if defined (CONFIG_RT_MULTIPLE_BR_SUPPORT)
#define RTL_IMGP_MAX_BRMODULE	8
#endif
struct rtl865x_ReservedMCastRecord
{
	unsigned int groupAddr;
	unsigned int valid;
};

int rtl_add_ReservedMCastAddr(uint32 groupAddr, int flag);
int rtl_check_ReservedMCastAddr(uint32 groupAddr);

/******************************************************
	Function called in the system initialization 
******************************************************/

int32 rtl_initMulticastSnooping(struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig);
int32 rtl_exitMulticastSnooping(void);
void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters);

int32 rtl_registerIgmpSnoopingModule(uint32 *moduleIndex);

int32 rtl_setIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo);
int32 rtl_getIgmpSnoopingModuleDevInfo(uint32 moduleIndex,rtl_multicastDeviceInfo_t *devInfo);

int32 rtl_getDeviceIgmpSnoopingModuleIndex(rtl_multicastDeviceInfo_t *devInfo,uint32 *moduleIndex);

int32 rtl_unregisterIgmpSnoopingModule(uint32 moduleIndex);

int32 rtl_configIgmpSnoopingModule(uint32 moduleIndex, struct rtl_mCastSnoopingLocalConfig *multicastSnoopingConfig);

int32 rtl_igmpMldProcess(uint32 moduleIndex, uint8 * macFrame,  uint32 portNum, uint32 *fwdPortMask);

int32 rtl_getMulticastDataFwdInfo(uint32 moduleIndex, struct rtl_multicastDataInfo * multicastDataInfo, struct rtl_multicastFwdInfo *multicastFwdInfo);

int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime);
int32 rtl_getGroupNum(uint32 ipVersion);

#ifdef CONFIG_PROC_FS
int igmp_show(struct seq_file *s, void *v);
int igmp_write(struct file *file, const char __user *buffer, size_t count, loff_t *data);
#endif

int32 rtl_getGroupInfo(uint32 groupAddr, struct rtl_groupInfo *groupInfo);
#if defined(CONFIG_RTL_MLD_SNOOPING)
int32 rtl_getGroupInfov6(uint32* groupAddr, struct rtl_groupInfo *groupInfo);
#endif
int32 rtl_flushAllIgmpRecord(uint8 flush_flag);
int32 rtl_delIgmpRecordByMacAddr(uint8 *macAddr);

int32 rtl_addSpecialMCast(uint32 moduleIndex, uint32 ipVersion, uint32 *groupAddr, uint32 fwdPortMask);
int32 rtl_delSpecialMCast(uint32 moduleIndex, uint32 ipVersion, uint32 *groupAddr);

int32 rtl_setIpv4UnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap);
int32 rtl_getIpv4UnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap);
int32 rtl_setIpv6UnknownMCastFloodMap(uint32 moduleIndex,uint32 unknownMCastFloodMap);
int32 rtl_getIpv6UnknownMCastFloodMap(uint32 moduleIndex,uint32 *unknownMCastFloodMap);
#define	Any_0_hop_protocl		114
#ifndef M2U_DELETE_CHECK
#define M2U_DELETE_CHECK
#endif

#define FLUSH_IGMP_RECORD 0x01
#ifdef CONFIG_RTL_MLD_SNOOPING
#define FLUSH_MLD_RECORD  0x10
#endif

#if 0//!defined(IMPROVE_MCAST_PERFORMANCE_WITH_RTL8367) && defined(CONFIG_RTL_IGMP_SNOOPING) && defined(CONFIG_RTL_8367R_SUPPORT)
#define IMPROVE_MCAST_PERFORMANCE_WITH_RTL8367
#endif
#endif 

