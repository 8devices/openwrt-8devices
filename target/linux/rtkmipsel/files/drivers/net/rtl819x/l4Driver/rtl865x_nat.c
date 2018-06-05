/*
* Program : napt table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*
*  Copyright (c) 2011 Realtek Semiconductor Corp.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_netif.h>
#include "common/mbuf.h"
//#include "assert.h"

//#include "rtl865xc_swNic.h"
//#include <common/types.h>
#include "AsicDriver/rtl865x_hwPatch.h"		/* define for chip related spec */
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER

#include "AsicDriver/rtl865x_asicL4.h"
#else
#include "common/rtl8651_aclLocal.h"
#include "rtl865xC_tblAsicDrv.h"
#endif

#include "common/rtl_errno.h"
//#include <net/rtl/rtl_queue.h>
#include "AsicDriver/rtl865xc_asicregs.h"
#include "common/rtl865x_eventMgr.h"
#include "l3Driver/rtl865x_ip.h"

#include <net/rtl/rtl865x_nat.h>
#include "rtl865x_nat_local.h"

//#include "rtl865x_ppp.h"
#include "common/rtl865x_netif_local.h"
#include "l3Driver/rtl865x_ppp_local.h"
//#include "l3Driver/rtl865x_route.h"
#include <net/rtl/rtl865x_outputQueue.h>
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
#include <net/rtl/rtl865x_arp_api.h>
#include "l3Driver/rtl865x_route.h"
#endif

#ifdef CONFIG_RTL_PROC_DEBUG
#include <linux/seq_file.h>
#endif

#include <linux/jiffies.h>

#if defined (CONFIG_RTL865X_NAT_ADD_FAIL_CHECK)
static uint32 failedCount=0;
static unsigned long	 lastJiffies;
#define ADD_FAILURE_TRHREHOLD 100
#endif

static struct nat_table nat_tbl;
static int32 rtl865x_enableNaptFourWay=FALSE;
static int32 _rtl865x_naptIdleCheck(int32 index);
#if 0
static int32 rtl865x_nat_callbackFn_for_del_ip(void *param);

static int32 rtl865x_nat_register_event(void);

static int32 rtl865x_nat_callbackFn_for_del_ip(void *param)
{
	int i;
	rtl865x_tblAsicDrv_extIntIpParam_t *natip;
	struct nat_entry *nat_out, *nat_in;
	struct nat_tuple nat_tuple;
	natip=(rtl865x_tblAsicDrv_extIntIpParam_t *)param;

	for(i=0; i<RTL8651_TCPUDPTBL_SIZE; i++)
	{
		if((nat_tbl.nat_bucket[i].flags & NAT_OUTBOUND) && (nat_tbl.nat_bucket[i].ext_ip_==natip->extIpAddr))
		{
			memcpy(&nat_tuple, &nat_tbl.nat_bucket[i].tuple_info, sizeof(struct nat_tuple));
			nat_out = &nat_tbl.nat_bucket[i];
			nat_in = &nat_tbl.nat_bucket[nat_out->in];
			rtl8651_delAsicNaptTcpUdpTable(nat_out->in, nat_out->in);
			rtl8651_delAsicNaptTcpUdpTable(nat_out->out, nat_out->out);
			memset(nat_in, 0, sizeof(*nat_in));
			memset(nat_out, 0, sizeof(*nat_out));

			if(nat_tbl.connNum > 0)
			{
				nat_tbl.connNum--;
			}	
		}

	}
	return EVENT_CONTINUE_EXECUTE;
}

static int32 _rtl865x_nat_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_IP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_nat_callbackFn_for_del_ip;
	rtl865x_unRegisterEvent(&eventParam);

	return SUCCESS;
}

static int32 _rtl865x_nat_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_IP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_nat_callbackFn_for_del_ip;
	rtl865x_registerEvent(&eventParam);

	return SUCCESS;
}
#endif

#define	RTL_NAT_AVG_INTERVAL	3
#define	RTL_NAT_MIN_INTERVAL	1

int32 rtl_nat_expire_interval_update(int proto, int32 interval)
{
	int32	asic_interval;

	asic_interval = interval>RTL_NAT_AVG_INTERVAL?interval-RTL_NAT_AVG_INTERVAL:interval-RTL_NAT_MIN_INTERVAL;
	asic_interval = asic_interval<RTL_NAT_MIN_INTERVAL?RTL_NAT_MIN_INTERVAL:asic_interval;
	
	if (proto==RTL865X_PROTOCOL_UDP) {
		nat_tbl.tcp_timeout = asic_interval;
	}else if (proto==RTL865X_PROTOCOL_UDP) {
		nat_tbl.udp_timeout = asic_interval;
	} else {
		return FAILED;
	}

	/* Set ASIC timeout value */
	rtl8651_setAsicNaptTcpLongTimeout(nat_tbl.tcp_timeout);
	rtl8651_setAsicNaptTcpMediumTimeout(nat_tbl.tcp_timeout);
	rtl8651_setAsicNaptTcpFastTimeout(nat_tbl.tcp_timeout);
	rtl8651_setAsicNaptUdpTimeout(nat_tbl.udp_timeout);

	return SUCCESS;
}

static int32 _rtl865x_nat_init(void)
{
	rtl865x_tblAsicDrv_naptTcpUdpParam_t naptTcpUdp;
	uint32 flowTblIdx;
	
	memset(nat_tbl.nat_bucket, 0, 
		sizeof(struct nat_entry)*RTL8651_TCPUDPTBL_SIZE);

	nat_tbl.connNum = 0;
	nat_tbl.tcp_timeout = TCP_TIMEOUT; //24*60*60;
	nat_tbl.udp_timeout = UDP_TIMEOUT; //60*15;
	nat_tbl.freeEntryNum=RTL8651_TCPUDPTBL_SIZE;

	/* Set ASIC timeout value */
	rtl8651_setAsicNaptTcpLongTimeout(TCP_TIMEOUT);
	rtl8651_setAsicNaptTcpMediumTimeout(TCP_TIMEOUT);
	rtl8651_setAsicNaptTcpFastTimeout(TCP_TIMEOUT);
	rtl8651_setAsicNaptUdpTimeout(UDP_TIMEOUT);

	/*enable 865xC enhanced hash1*/
	_rtl8651_enableEnhancedHash1();
	
	/* Initial ASIC NAT Table */
	memset( &naptTcpUdp, 0, sizeof(naptTcpUdp) );
	naptTcpUdp.isCollision = 1;
	naptTcpUdp.isCollision2 = 1;
	for(flowTblIdx=0; flowTblIdx<RTL8651_TCPUDPTBL_SIZE; flowTblIdx++)
		rtl8651_setAsicNaptTcpUdpTable(TRUE, flowTblIdx, &naptTcpUdp );

	//rtl865x_nat_register_event();
		
	return SUCCESS;
}



static struct nat_entry * _rtl865x_nat_outbound_lookup(struct nat_tuple *nat_tuple)
{
	struct nat_entry *nat_out;
	uint32 i,hash;

	hash = rtl8651_naptTcpUdpTableIndex((uint8)nat_tuple->proto, nat_tuple->int_host.ip, nat_tuple->int_host.port, 
											nat_tuple->rem_host.ip, nat_tuple->rem_host.port);
	if(rtl865x_enableNaptFourWay==TRUE)
	{
		for(i=0; i<4; i++)
		{
			nat_out = &nat_tbl.nat_bucket[hash];
			if (!memcmp(nat_out, nat_tuple, sizeof(*nat_tuple)) &&
				(nat_out->flags&NAT_OUTBOUND))
			{
				return nat_out;
			}
			hash=(hash&0xFFFFFFFC)+(hash+1)%4;
			assert(hash<=RTL8651_TCPUDPTBL_SIZE);
		}
	}
	else
	{
		nat_out = &nat_tbl.nat_bucket[hash];
		if (!memcmp(nat_out, nat_tuple, sizeof(*nat_tuple)) &&
			(nat_out->flags&NAT_OUTBOUND))
			return nat_out;
	}
	return (struct nat_entry *)0;
}

static struct nat_entry * _rtl865x_nat_inbound_lookup(struct nat_tuple *nat_tuple)
{
	struct nat_entry *nat_in;
	uint32 hash;

	hash = rtl8651_naptTcpUdpTableIndex((uint8)nat_tuple->proto, nat_tuple->rem_host.ip, nat_tuple->rem_host.port, 
											nat_tuple->ext_host.ip, nat_tuple->ext_host.port);
	
	
	nat_in = &nat_tbl.nat_bucket[hash];
	if (!memcmp(nat_in, nat_tuple, sizeof(*nat_tuple)) && 
		(nat_in->flags&NAT_INBOUND))
	{
			return nat_in;
	}
	
	return (struct nat_entry *)0;
}

#if defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
static int _rtl865x_isEntryPreReserved(uint32 index)
{
	struct nat_entry *natEntry;
	if(index>=RTL8651_TCPUDPTBL_SIZE)
	{
		return FALSE;
	}
	
	natEntry= &nat_tbl.nat_bucket[index];

	if((natEntry->flags & NAT_PRE_RESERVED))
	{
		if(jiffies>=natEntry->reserveTime)
		{
			if(jiffies>(natEntry->reserveTime+RESERVE_EXPIRE_TIME*HZ))
			{
				/*pre-reserve become invalid now*/
				natEntry->flags &= (~(NAT_PRE_RESERVED));
				natEntry->reserveTime=0;
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
		else
		{
			/*timer overflow*/
			if(((0xFFFFFFFF-natEntry->reserveTime)+jiffies+1)>(RESERVE_EXPIRE_TIME*HZ))
			{
				natEntry->flags &= (~(NAT_PRE_RESERVED));
				natEntry->reserveTime=0;
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
		return TRUE;
	}

	return FALSE;
}

static int _rtl865x_PreReserveEntry(uint32 index)
{
	struct nat_entry *natEntry;
	if(index>=RTL8651_TCPUDPTBL_SIZE)
	{
		return FAILED;
	}
	natEntry= &nat_tbl.nat_bucket[index];
	
	if(NAT_INUSE(natEntry))
	{
		/*already used by other napt connection, can not reserve it*/
		natEntry->flags &= (~(NAT_PRE_RESERVED));
		natEntry->reserveTime=0;
	}
	else
	{
		natEntry->flags|=NAT_PRE_RESERVED;
		natEntry->reserveTime=jiffies;
	}
	return SUCCESS;
}

static int _rtl865x_getNaptHashInfo( rtl865x_napt_entry *naptEntry, 
                        rtl865x_naptHashInfo_t *naptHashInfo)
{
	
	uint32 in, out;
	uint32  i,index;
	struct nat_entry *nat_in, *nat_out;
	struct nat_entry *natEntry;
	
	if(naptHashInfo==NULL)
	{
		return FAILED;
	}
	
	memset(naptHashInfo, 0, sizeof(rtl865x_naptHashInfo_t));
	
	in = rtl8651_naptTcpUdpTableIndex((uint8)(naptEntry->protocol), naptEntry->remIp, naptEntry->remPort, naptEntry->extIp, naptEntry->extPort);
	out = rtl8651_naptTcpUdpTableIndex((uint8)(naptEntry->protocol), naptEntry->intIp, naptEntry->intPort, naptEntry->remIp, naptEntry->remPort);

	if(rtl865x_enableNaptFourWay==TRUE)
	{
		uint32 outAvailIdx=0xFFFFFFFF;
		index=out;
		for(i=0;i<4;i++)
		{
			natEntry = &nat_tbl.nat_bucket[index];
			if (NAT_INUSE(natEntry) || _rtl865x_isEntryPreReserved(index))
			{
			
			}
			else
			{
				if(index==in)
				{
					/*collide with inbound*/
				}
				else
				{
					out=index;
					break;
				}
			}
			index=(index&0xFFFFFFFC)+(index+1)%4;
			assert(index<=RTL8651_TCPUDPTBL_SIZE);
				
		}

		if(i>=4)
		{
			/*only one empty entry, but collide with its own inbound*/
			if(outAvailIdx!=0xFFFFFFFF)
			{
				out=outAvailIdx;
			}
		}
		else
		{
			/*proper empty entry has been found*/
		}
	}
	
	naptHashInfo->outIndex=out;
	naptHashInfo->inIndex=in;

	if((in&0xFFFFFFFC)==(out&0xFFFFFFFC))
	{
		naptHashInfo->sameFourWay=1;
	}	

	if(in==out)
	{
		naptHashInfo->sameLocation=1;
		
		nat_out = &nat_tbl.nat_bucket[out];
		if(NAT_INUSE(nat_out)|| _rtl865x_isEntryPreReserved(out))
		{
			naptHashInfo->outCollision=1;
		}

		naptHashInfo->inCollision=1;
	}
	else
	{
		nat_out = &nat_tbl.nat_bucket[out];
		nat_in = &nat_tbl.nat_bucket[in];
		
		if(NAT_INUSE(nat_out) || _rtl865x_isEntryPreReserved(out))
		{
			naptHashInfo->outCollision=1;
		}
		
		if (NAT_INUSE(nat_in) ||  _rtl865x_isEntryPreReserved(in))
		{
			naptHashInfo->inCollision=1;
		}
	}
	

	index=in;
	naptHashInfo->inFreeCnt=0;
	for(i=0;i<4;i++)
	{
		natEntry = &nat_tbl.nat_bucket[index];
		if (NAT_INUSE(natEntry) || _rtl865x_isEntryPreReserved(index))
		{
		
		}
		else
		{
			naptHashInfo->inFreeCnt++;
		}
		index=(index&0xFFFFFFFC)+(index+1)%4;
		assert(index<=RTL8651_TCPUDPTBL_SIZE);
	}
	#if 0
	printk("%s:%d:%s (%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u) ,out is %d,in is %d\n",
			__FUNCTION__,__LINE__,protocol?"tcp":"udp", 
			NIPQUAD(intIp), intPort, NIPQUAD(extIp), extPort, NIPQUAD(remIp), remPort, out, in);	
	#endif
	return SUCCESS;
}

int rtl865x_optimizeExtPort(unsigned short origDelta, unsigned int rangeSize, unsigned short *newDelta)
{
	int i;
	int msb;
	unsigned int bitShift;

	msb=0;
	for(i=0;i<16;i++)
	{
		if((1<<i) & rangeSize)
		{
			msb=i;
		}
	}

	if(((1<<msb)+1)>rangeSize)
	{
		if(msb>1)
		{
			msb--;
		}
	}
		
 	*newDelta=0;
	if(msb<10)
	{
		bitShift=0x01;
		for(i=0;i<=msb;i++)
		{
			if(i==0)/*bit0 keep the same*/
			{
				if(origDelta&bitShift)
				{
					 *newDelta|=bitShift;
				}
			}
			else /*original bit1~ bit_maxPower mapped to bit_maxPower~bit1*/
			{
				if(origDelta&bitShift) 
				{
					 *newDelta |=(0x1<<(msb+1-i));
				}
			}

			bitShift=bitShift<<1;
		}
	}
	else
	{
		bitShift=0x01;
		
		for(i=0;i<=msb;i++)
		{
			if(i==0)	/*bit0 keep the same*/
			{
				if(origDelta&bitShift) 
				{
					*newDelta |=bitShift;
				}
			}
			else if (i<10) /*bit1~ bit9 mapped to bit 9~bit1*/
			{
				if(origDelta&bitShift) 
				{
					*newDelta  |=(0x1<<(10-i));
				}
			}
			else/*other bits keep the same*/
			{
				if(origDelta&bitShift) 
				{
					*newDelta  |=bitShift;
				}
			}

			bitShift=bitShift<<1;
		}


	}
	return SUCCESS;
}

int rtl865x_getAsicNaptHashScore(rtl865x_napt_entry *naptEntry, 
					                        uint32 *naptHashScore)
{
	 rtl865x_naptHashInfo_t naptHashInfo;
	_rtl865x_getNaptHashInfo(naptEntry, &naptHashInfo);

	/*initialize napt hash score*/
	*naptHashScore=100;

	/*note:we can not change outbound index*/
	
	if(naptHashInfo.inCollision==FALSE)
	{
		if(naptHashInfo.inFreeCnt==4)
		{
			if(!naptHashInfo.sameFourWay)
			{
				*naptHashScore=100;
			}
			else
			{
				if(!naptHashInfo.sameLocation)
				{
					*naptHashScore=80;
				}
				else
				{
					*naptHashScore=0;
				}
			}
		}
		else if (naptHashInfo.inFreeCnt==3)
		{
			if(!naptHashInfo.sameFourWay)
			{
				*naptHashScore=80;
			}
			else
			{
				if(!naptHashInfo.sameLocation)
				{
					*naptHashScore=70;
				}
				else
				{
					*naptHashScore=0;
				}
			}
		}
		else if (naptHashInfo.inFreeCnt==2)
		{
			if(!naptHashInfo.sameFourWay)
			{
				*naptHashScore=70;
			}
			else
			{
				if(!naptHashInfo.sameLocation==FALSE)
				{
					*naptHashScore=60;
				}
				else
				{
					*naptHashScore=0;
				}
			}
		}
		else if (naptHashInfo.inFreeCnt==1)
		{
			if(naptHashInfo.sameFourWay==FALSE)
			{
				*naptHashScore=60;
			}
			else
			{
				*naptHashScore=0;
					
			}
		}
		else
		{
			*naptHashScore=0;
		}
		

	}
	else
	{
		/*worst case:inbound is collision*/
		*naptHashScore=0;
	}
	
	return SUCCESS;

}

int32 rtl865x_preReserveConn(rtl865x_napt_entry *naptEntry)
{

	 rtl865x_naptHashInfo_t naptHashInfo;
	_rtl865x_getNaptHashInfo(naptEntry, &naptHashInfo);

	if(naptHashInfo.outCollision==FALSE)
	{
		_rtl865x_PreReserveEntry(naptHashInfo.outIndex);
	}

	if(naptHashInfo.inCollision==FALSE)
	{
		_rtl865x_PreReserveEntry(naptHashInfo.inIndex);
	}
	
	return SUCCESS;
}	

#endif
#if defined (CONFIG_RTL865X_NAT_ADD_FAIL_CHECK)
static int32 _rtl865x_addNaptConnectionFailCheck(void)
{

	if(time_before_eq((lastJiffies+HZ),jiffies))
	{
		lastJiffies=jiffies;
		failedCount=0;
	}

	if(failedCount>ADD_FAILURE_TRHREHOLD)
	{
		return RTL_ETOOMANYCOLLISION;
	}
	return SUCCESS;
}
#endif


static int32 _rtl865x_addNaptConnection(rtl865x_napt_entry *naptEntry, rtl865x_priority *prio)
{
	int32 retval;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat;
	struct nat_entry *nat_in, *nat_out;
	struct nat_tuple nat_tuple;
	uint32 in, out, offset, ipidx, i;
	uint16 very,selEidx_out;
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) 
	#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	rtl865x_route_t		rt;
	rtl865x_arpMapping_entry_t	arpInfo;
	ipaddr_t		sip, dip;
	uint16		sport, dport;
	rtl865x_AclRule_t		rule4[2], rule2[2];
	int32		defPriority[2];
	int32 		upDown[2], defUpDown[2];//0: uplink; 1: downlink
	#endif
	int32		priority[2];
#endif
	int32		elasped, del_conn_flags;
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	uint32 isExtNaptEntry=0;
#endif
	uint32 outCollision=FALSE;
	uint32 inCollision=FALSE;
	uint16 extPort;

	/* Make sure natip */
	retval = rtl865x_getIpIdxByExtIp(htonl(naptEntry->extIp), &ipidx);
	if(retval != SUCCESS)
		return RTL_EINVALIDINPUT;
#if defined (CONFIG_RTL865X_NAT_ADD_FAIL_CHECK)
	if(_rtl865x_addNaptConnectionFailCheck()==RTL_ETOOMANYCOLLISION)
	{
		return RTL_ETOOMANYCOLLISION;
	}
#endif	
	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip		= htonl(naptEntry->intIp);
	nat_tuple.int_host.port		= htons((uint16)(naptEntry->intPort));
	nat_tuple.ext_host.ip		= htonl(naptEntry->extIp);
	nat_tuple.ext_host.port		= htons((uint16)(naptEntry->extPort));
	nat_tuple.rem_host.ip		= htonl(naptEntry->remIp);
	nat_tuple.rem_host.port		= htons((uint16)(naptEntry->remPort));
	nat_tuple.proto				= naptEntry->protocol;
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	isExtNaptEntry =naptEntry->isExtNaptEntry;
#endif	
	nat_out = _rtl865x_nat_outbound_lookup(&nat_tuple);
	if (nat_out)
	{
		return RTL_EENTRYALREADYEXIST;
	}
	
	nat_in = _rtl865x_nat_inbound_lookup(&nat_tuple);
	if(nat_in)
	{
		return RTL_EENTRYALREADYEXIST;
	}

	extPort = htons((uint16)(naptEntry->extPort));
	offset = (extPort&0x0000ffff)>>10;
	selEidx_out = extPort&0x3ff;

	very = rtl8651_naptTcpUdpTableIndex(((uint8)naptEntry->protocol) |HASH_FOR_VERI , htonl(naptEntry->remIp), htons((uint16)(naptEntry->remPort)), 0, 0);
	in = rtl8651_naptTcpUdpTableIndex((uint8)naptEntry->protocol, htonl(naptEntry->remIp), htons((uint16)(naptEntry->remPort)), htonl(naptEntry->extIp), extPort );
	out = rtl8651_naptTcpUdpTableIndex((uint8)naptEntry->protocol, htonl(naptEntry->intIp), htons((uint16)(naptEntry->intPort)), htonl(naptEntry->remIp), htons((uint16)(naptEntry->remPort)) );
	/*support outbound 4-way*/
	if(rtl865x_enableNaptFourWay==TRUE)
	{

		uint32 hash=out;
		uint32 outAvailIdx=0xFFFFFFFF;

		for(i=0;i<4;i++)
		{
			nat_out = &nat_tbl.nat_bucket[hash];
			if (NAT_INUSE(nat_out))
			{
				/* collision with outbound */
			}
			else
			{
				outAvailIdx=hash;
				if(hash==in)
				{
					/*collision with inbound*/
				}
				else
				{
					out=hash;
					break;
				}
			}
			hash=(hash&0xFFFFFFFC)+(hash+1)%4;
			assert(hash<=RTL8651_TCPUDPTBL_SIZE);
				
		}


		if(i>=4)
		{
			/*only one empty entry,but collide with its own inbound*/
			if(outAvailIdx!=0xFFFFFFFF)
			{
				out=outAvailIdx;
			}
		}
	}

	
	nat_out = &nat_tbl.nat_bucket[out];
	nat_in = &nat_tbl.nat_bucket[in];

	del_conn_flags = 0;
	if (NAT_INUSE(nat_out))
	{
		elasped = _rtl865x_naptIdleCheck(out);
		if ( ((nat_out->tuple_info.proto==RTL865X_PROTOCOL_UDP)&&(elasped>UDP_OVERRIDE_ELASPED_THRESHOLD)) 
			|| ((nat_out->tuple_info.proto==RTL865X_PROTOCOL_TCP)&&(elasped>TCP_OVERRIDE_ELASPED_THRESHOLD)))
		{
			NAT_INVALID(nat_out);		/*	invalide	*/
			nat_tbl.freeEntryNum++;
			del_conn_flags++;
		}
	}

	if (NAT_INUSE(nat_in))
	{
		elasped = _rtl865x_naptIdleCheck(in);
		if ( ((nat_out->tuple_info.proto==RTL865X_PROTOCOL_UDP)&&(elasped>UDP_OVERRIDE_ELASPED_THRESHOLD)) 
			|| ((nat_out->tuple_info.proto==RTL865X_PROTOCOL_TCP)&&(elasped>TCP_OVERRIDE_ELASPED_THRESHOLD)))
		{
			NAT_INVALID(nat_in);		/*	invalide	*/
			nat_tbl.freeEntryNum++;
			del_conn_flags++;
		}
	}

	if (del_conn_flags==2) {
		nat_tbl.connNum--;
	}

	if ( NAT_INUSE(nat_out) && NAT_INUSE(nat_in))
	{
		/*both outbound and inbound has been occupied*/
		#if defined (CONFIG_RTL865X_NAT_ADD_FAIL_CHECK)
		failedCount++;
		#endif
		return RTL_EINVALIDINPUT;
	}
	
	if(out==in)
	{
		outCollision=FALSE;
		inCollision=TRUE;
		/*we don't support this case at present, otherwise, when delete napt connection must be very very careful*/
		//return RTL_EENTRYALREADYEXIST;
	}
	else
	{
		if (NAT_INUSE(nat_out))
		{
			outCollision=TRUE;
		}

		if(NAT_INUSE(nat_in))
		{
			inCollision=TRUE;
		}
	}
#ifdef CONFIG_HARDWARE_NAT_DEBUG
	rtlglue_printf("LR(%s):  %s (%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u) g:(%u.%u.%u.%u:%u)\n",
			("add_nat"), ((naptEntry->protocol)? "tcp": "udp"), 
			NIPQUAD(naptEntry->intIp), naptEntry->intPort, NIPQUAD(naptEntry->remIp), naptEntry->remPort, NIPQUAD(naptEntry->extIp), naptEntry->extPort);
#endif

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	if(isExtNaptEntry)
	{
		#if 0
		if(outCollision==TRUE)
		#else
		if((outCollision==TRUE) || (inCollision==TRUE))
		#endif	
		{
		#if defined (CONFIG_RTL865X_NAT_ADD_FAIL_CHECK)
			failedCount++;
		#endif	
			/*we must make sure both direction can be written into asic*/
			return RTL_EINVALIDINPUT;
		}
	}else
#endif
	{
#if defined (CONFIG_RTL_HALF_NAPT_ACCELERATION)

#else
	if((outCollision==TRUE) || (inCollision==TRUE))
	{
			#if defined (CONFIG_RTL865X_NAT_ADD_FAIL_CHECK)
			failedCount++;
			#endif
		/*we must make sure both direction can be written into asic*/
		return RTL_EINVALIDINPUT;
	}
#endif
	}

	if ( outCollision==FALSE)
	{
		memset(nat_out, 0, sizeof(struct nat_entry));
		*((struct nat_tuple *)nat_out)=nat_tuple;
		nat_out->out=out;
		if( inCollision==FALSE)
		{
			nat_out->in=in;
		}
		else
		{
			nat_out->in=0xFFFFFFFF;
		}
		
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
		nat_out->isExtNaptEntry =isExtNaptEntry;
#endif
		nat_out->natip_idx	=ipidx;	
		SET_NAT_FLAGS(nat_out, NAT_OUTBOUND);
	}
	else
	{
		nat_out=NULL;
	}

	if( inCollision==FALSE)
	{
		memset(nat_in, 0, sizeof(struct nat_entry));
		*((struct nat_tuple *)nat_in) = nat_tuple;
		nat_in->in = in;
		if ( outCollision==FALSE)
		{
			 nat_in->out = out;
		}
		else
		{
			 nat_in->out=0xFFFFFFFF;
		}
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
		nat_in->isExtNaptEntry =isExtNaptEntry;
#endif
		nat_in->natip_idx = ipidx;
		SET_NAT_FLAGS(nat_in, NAT_INBOUND);
	}
	else
	{
		nat_in=NULL;
	}

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
#if	defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	sip = naptEntry->intIp;
	dip = naptEntry->remIp;
	sport = naptEntry->intPort;
	dport = naptEntry->remPort;
	
	//Initial
	for(i=0;i<2;i++)
	{
		priority[i] = 0;
		defPriority[i]=-1;
		upDown[i]=-1;
		defUpDown[i]=-1;
	}
	
	for(i=0;i<2;i++)
	{
		if (rtl865x_getRouteEntry(sip, &rt)==SUCCESS)
		{
			/*	check ip base rule firstly	*/

			memset(&rule4[i], 0, sizeof(rtl865x_AclRule_t));
			rule4[i].ruleType_ = (naptEntry->protocol==RTL865X_PROTOCOL_TCP?RTL865X_ACL_TCP:RTL865X_ACL_UDP);
			rule4[i].srcIpAddr_ = sip;
			rule4[i].dstIpAddr_ = dip;
			rule4[i].tcpSrcPortLB_ = sport;
			rule4[i].tcpDstPortLB_ = dport;
			rule4[i].netifIdx_ = _rtl865x_getNetifIdxByNameExt(rt.dstNetif->name);
			
			if (rtl865x_qosCheckNaptPriority(&rule4[i])==SUCCESS)
			{
				priority[i] = rule4[i].priority_;		/* matched priority	*/
				upDown[i]=rule4[i].upDown_;
				//break;

				if (i==0)
				{
					sip = naptEntry->remIp;
					dip = naptEntry->intIp;
					sport = naptEntry->remPort;
					dport = naptEntry->intPort;
					continue;
				}
				 
			}
			else if (i==0)
			{
				sip = naptEntry->remIp;
				dip = naptEntry->intIp;
				sport = naptEntry->remPort;
				dport = naptEntry->intPort;
				continue;
			}
			else
			{
				defPriority[i] = rule4[i].priority_;
				defUpDown[i]=rule4[i].upDown_;
			}
		}
		else
		{
			sip = naptEntry->remIp;
			dip = naptEntry->intIp;
			sport = naptEntry->remPort;
			dport = naptEntry->intPort;
		}
	}

	{
		sip = naptEntry->intIp;
		for(i=0; i<2; i++)
		{
			memset(&rule2[i], 0, sizeof(rtl865x_AclRule_t));
		}
		
		for(i=0;i<2;i++)
		{
			if (rtl865x_getArpMapping(sip, &arpInfo)==SUCCESS && rtl865x_getRouteEntry(sip, &rt)==SUCCESS)
			{
				/*	check mac base rule secondly	*/
				//memset(&rule2[i], 0, sizeof(rtl865x_AclRule_t));
				rule2[i].ruleType_ = RTL865X_ACL_MAC;
				memcpy(rule2[i].srcMac_.octet, arpInfo.mac.octet, ETHER_ADDR_LEN);
				memset(rule2[i].srcMacMask_.octet, 0xff, ETHER_ADDR_LEN);
				rule2[i].netifIdx_ = _rtl865x_getNetifIdxByNameExt(rt.dstNetif->name);

				//printk("----%s[%d], rt.dstNetif->name is %s, rule2[i].srcMac_.octet is 0x%x%x%x%x%x%x\n", __FUNCTION__, __LINE__, rt.dstNetif->name, 
					//rule2[i].srcMac_.octet[0], rule2[i].srcMac_.octet[1],rule2[i].srcMac_.octet[2],rule2[i].srcMac_.octet[3],rule2[i].srcMac_.octet[4],rule2[i].srcMac_.octet[5]);

				if (rtl865x_qosCheckNaptPriority(&rule2[i])==SUCCESS)
				{
					priority[i] = rule2[i].priority_;		/* matched priority	*/
					upDown[i]=rule2[i].upDown_;
					//break;

					if(i==0)
					{
						sip = naptEntry->remIp;
						continue;
					}
				}
				else if(i==0)
				{
					sip = naptEntry->remIp;
					continue;
				}
			}else{
				sip = naptEntry->remIp;
			}
		}

		dip = naptEntry->remIp;
		for(i=0; i<2; i++)
		{
			if((rule2[i].aclIdx==0)&&rtl865x_getArpMapping(dip, &arpInfo)==SUCCESS && rtl865x_getRouteEntry(naptEntry->remIp, &rt)==SUCCESS)		
			{
				/*	check mac base rule secondly  */
				//memset(&rule2[i], 0, sizeof(rtl865x_AclRule_t));
				rule2[i].ruleType_ = RTL865X_ACL_MAC;
				memcpy(rule2[i].dstMac_.octet, arpInfo.mac.octet, ETHER_ADDR_LEN);
				memset(rule2[i].dstMacMask_.octet, 0xff, ETHER_ADDR_LEN);
				rule2[i].netifIdx_ = _rtl865x_getNetifIdxByNameExt(rt.dstNetif->name);

				//printk("----%s[%d], rt.dstNetif->name is %s, rule2[i].dstMac_.octet is 0x%x%x%x%x%x%x\n", __FUNCTION__, __LINE__, rt.dstNetif->name, 
					//rule2[i].dstMac_.octet[0], rule2[i].dstMac_.octet[1],rule2[i].dstMac_.octet[2],rule2[i].dstMac_.octet[3],rule2[i].dstMac_.octet[4],rule2[i].dstMac_.octet[5]);

				if (rtl865x_qosCheckNaptPriority(&rule2[i])==SUCCESS)
				{
					priority[i] = rule2[i].priority_;		/* matched priority	*/
					upDown[i]=rule2[i].upDown_;
					//break;
				
					if(i==0)
					{
						dip = naptEntry->intIp;
						continue;
					}
					
				}
				else if(i==0)
				{
					dip = naptEntry->intIp;
					continue;
				}
				else
				{
					defPriority[i] = rule2[i].priority_;
					defUpDown[i]=rule2[i].upDown_;
				}
			}
			else
			{
				dip = naptEntry->intIp;
			}
		}
	}


	for(i=0;i<2;i++)
	{
		if (rule4[i].aclIdx&&rule2[i].aclIdx)
		{
			priority[i] = (rule4[i].aclIdx<rule2[i].aclIdx)?rule4[i].priority_:rule2[i].priority_;
			upDown[i]=(rule4[i].aclIdx<rule2[i].aclIdx)?rule4[i].upDown_:rule2[i].upDown_;
		}
		else if (rule4[i].aclIdx)
		{
			priority[i] = rule4[i].priority_;
			upDown[i]=rule4[i].upDown_;
		}
		else if (rule2[i].aclIdx)
		{
			priority[i] = rule2[i].priority_;
			upDown[i]=rule2[i].upDown_;
		}
		else if (defPriority[i]>-1)
		{
			priority[i] = defPriority[i];
			upDown[i]=defUpDown[i];
		}
	}
#else	/*	defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)	*/
	priority[0]=prio->uplinkPrio;
	priority[1]=prio->downlinkPrio;
#endif	/*	defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)	*/
#endif	/*	defined(CONFIG_RTL_HW_QOS_SUPPORT)		*/

	for(i=0; i<2; i++) {
		
		if(i==0)
		{
			/*check writing outbound connection into asic*/
			if((outCollision==TRUE) || (nat_out==NULL))
			{
				/*shouldn't be written into asic*/
				continue;
			}
		}
		else if(i==1)
		{
			/*check writing inbound connection into asic*/
			if((inCollision==TRUE)||(nat_in==NULL))
			{
				/*shouldn't be written into asic*/
				continue;
			}
		}
		else
		{
			break;
		}
		//If qos enabled, not add inbound napt
		//That is all downlink pkt will be trapped to CPU for software QoS
		#if 0
		/* drop pkt in the queue will resolve the wan->lan issue */
		if((flags==FLAG_QOS_ENABLE)&&(i==1))
			break;
		#endif
		
		memset(&asic_nat, 0, sizeof(asic_nat));
		asic_nat.insideLocalIpAddr	= htonl(naptEntry->intIp);
		asic_nat.insideLocalPort	= htons((uint16)(naptEntry->intPort));
		asic_nat.isCollision			= 0;
		asic_nat.isCollision2		= 0;
		asic_nat.isDedicated		= 0;
		asic_nat.isStatic			= 1;
		asic_nat.isTcp			= (naptEntry->protocol==RTL865X_PROTOCOL_TCP)? 1: 0;
		asic_nat.isValid			= 1;
		asic_nat.offset			= ((i==0)?offset : (extPort & 0x3f));
		asic_nat.selEIdx			= ((i==0)?selEidx_out: very &0x3ff);
		asic_nat.selExtIPIdx		= ((i==0)?ipidx:((extPort & 0x3ff) >> 6));
		//asic_nat.tcpFlag			= (((in!=out)? 0x2:0x0) | ((i==0)? 1: 0));
		/*enhanced hash1 doesn't support outbound/inbound share one connection*/
		asic_nat.tcpFlag			= (0x2 | ((i==0)? 1: 0));
		asic_nat.ageSec			= (naptEntry->protocol==RTL865X_PROTOCOL_TCP)? nat_tbl.tcp_timeout:nat_tbl.udp_timeout;
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
		asic_nat.priValid			=	FALSE;
		#if defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
		if(i==0&&upDown[i]==0)//Because ((i==0)?out: in)
		{
			//out: uplink
			asic_nat.priority			=	priority[i];
		}
		if (i==1&&upDown[i]==1)
		{
			//in: downlink
			asic_nat.priority			=	priority[i];
		}
		#else	/*	defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)	*/
		asic_nat.priority			=	priority[i];
		#endif	/*	defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)	*/
		asic_nat.priValid			=	TRUE;
#else
		asic_nat.priValid			=	FALSE;
#endif
		
		rtl8651_setAsicNaptTcpUdpTable(1, ((i==0)?out: in), &asic_nat);
	}


	nat_tbl.connNum++;
	
	if((outCollision==FALSE) && (nat_out!=NULL))
	{
		if(nat_tbl.freeEntryNum>0)
		{
			nat_tbl.freeEntryNum--;
		}
	}
		
	if((inCollision==FALSE) && (nat_in!=NULL))
	{
		if(nat_tbl.freeEntryNum>0)
		{
			nat_tbl.freeEntryNum--;
		}
	}
	
	return SUCCESS;
}


static int32 _rtl865x_delNaptConnection(rtl865x_napt_entry *naptEntry)
{
	struct nat_entry *nat_out, *nat_in;
	struct nat_tuple nat_tuple;

	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip		= htonl(naptEntry->intIp);
	nat_tuple.int_host.port		= htons((uint16)(naptEntry->intPort));
	nat_tuple.ext_host.ip		= htonl(naptEntry->extIp);
	nat_tuple.ext_host.port		= htons((uint16)(naptEntry->extPort));
	nat_tuple.rem_host.ip		= htonl(naptEntry->remIp);
	nat_tuple.rem_host.port		= htons((uint16)(naptEntry->remPort));
	nat_tuple.proto				= naptEntry->protocol;

	nat_out = _rtl865x_nat_outbound_lookup(&nat_tuple);
	nat_in =  _rtl865x_nat_inbound_lookup(&nat_tuple);

	if ((nat_out==NULL) && (nat_in==NULL))
	{
		return RTL_EENTRYNOTFOUND;
	}

	if(nat_out==nat_in)	
	{
		rtl8651_delAsicNaptTcpUdpTable(nat_out->out, nat_out->out);
		memset(nat_out, 0, sizeof(*nat_out));
		nat_tbl.freeEntryNum++;
		
	}
	else
	{
		if(nat_out)
		{
			rtl8651_delAsicNaptTcpUdpTable(nat_out->out, nat_out->out);
			memset(nat_out, 0, sizeof(*nat_out));
			nat_tbl.freeEntryNum++;
		}	

		if(nat_in)
		{
			rtl8651_delAsicNaptTcpUdpTable(nat_in->in, nat_in->in);
			memset(nat_in, 0, sizeof(*nat_in));
			nat_tbl.freeEntryNum++;
		}
	}
	#ifdef CONFIG_HARDWARE_NAT_DEBUG
	/*2007-12-19*/
	rtlglue_printf("LR(%s):  %s (%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u) g:(%u.%u.%u.%u:%u)\n",
			("del_nat"), ((naptEntry->protocol)? "tcp": "udp"), 
			NIPQUAD(naptEntry->intIp), naptEntry->intPort, NIPQUAD(naptEntry->remIp), naptEntry->remPort, NIPQUAD(naptEntry->extIp), naptEntry->extPort);
	#endif

	if(nat_tbl.connNum>0)
	{
		nat_tbl.connNum--;
	}
	
	return SUCCESS;
}

static int32 _rtl865x_naptIdleCheck(int32 index)
{
	rtl865x_tblAsicDrv_naptTcpUdpParam_t	entry;
	int32 	hw_now;
	
	rtl8651_getAsicNaptTcpUdpTable(index, &entry);

	hw_now = entry.ageSec;
	
	if (entry.isTcp) {
		return (nat_tbl.tcp_timeout>hw_now?nat_tbl.tcp_timeout-hw_now:0);
	} else {
		return (nat_tbl.udp_timeout>hw_now?nat_tbl.udp_timeout-hw_now:0);
	}
}

static int32 _rtl865x_naptSync(rtl865x_napt_entry *naptEntry, uint32 refresh)
{
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat_out;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat_in;
	struct nat_entry *nat_out,*nat_in;
	struct nat_tuple nat_tuple;
	int32 rc;
	int32 hw_now;

	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip		= htonl(naptEntry->intIp);
	nat_tuple.int_host.port		= htons((uint16)(naptEntry->intPort));
	nat_tuple.ext_host.ip		= htonl(naptEntry->extIp);
	nat_tuple.ext_host.port		= htons((uint16)(naptEntry->extPort));
	nat_tuple.rem_host.ip		= htonl(naptEntry->remIp);
	nat_tuple.rem_host.port		= htons((uint16)(naptEntry->remPort));
	nat_tuple.proto				= naptEntry->protocol;
	
	nat_out = _rtl865x_nat_outbound_lookup(&nat_tuple);
	nat_in = _rtl865x_nat_inbound_lookup(&nat_tuple);

	if( (!nat_out) && (!nat_in))
	{
		return -1;
	}

	memset(&asic_nat_out ,0 ,sizeof(rtl865x_tblAsicDrv_naptTcpUdpParam_t));
	memset(&asic_nat_in,0 ,sizeof(rtl865x_tblAsicDrv_naptTcpUdpParam_t));
		
	if((nat_out!=NULL) && (nat_in!=NULL))
	{
		rc = rtl8651_getAsicNaptTcpUdpTable(nat_out->out, &asic_nat_out);
		assert(rc==SUCCESS);
		
		rc = rtl8651_getAsicNaptTcpUdpTable(nat_in->in, &asic_nat_in);
		assert(rc==SUCCESS);
		hw_now = (asic_nat_out.ageSec>asic_nat_in.ageSec)? asic_nat_out.ageSec: asic_nat_in.ageSec;
	}
	else if((nat_out!=NULL) && (nat_in==NULL))
	{
		rc = rtl8651_getAsicNaptTcpUdpTable(nat_out->out, &asic_nat_out);
		assert(rc==SUCCESS);
		hw_now = asic_nat_out.ageSec;
	}
	else if((nat_out==NULL) && (nat_in!=NULL))
	{
		rc = rtl8651_getAsicNaptTcpUdpTable(nat_in->in, &asic_nat_in);
		assert(rc==SUCCESS);
		hw_now = asic_nat_in.ageSec;
		
	}
	else
	{
		hw_now = 0;
	}

	if (hw_now==0) {
		return -1;
	} else {
		if (naptEntry->protocol==RTL865X_PROTOCOL_UDP) {
			return (nat_tbl.udp_timeout>hw_now?nat_tbl.udp_timeout-hw_now:0);
		} else if (naptEntry->protocol==RTL865X_PROTOCOL_TCP) {
			return (nat_tbl.tcp_timeout>hw_now?nat_tbl.tcp_timeout-hw_now:0);
		}
	}

	return -1;	
}


/*
@func int32 | rtl865x_addNaptConnection |add a napt entry.
@parm uint32 | protocol | protocol.
@parm ipaddr_t | intIp | internal ip address.
@parm uint32 | intPort | internal port.
@parm ipaddr_t | extIp | external ip address.
@parm uint32 | extPort | external port.
@parm ipaddr_t | remIp | remote ip address.
@parm uint32 | remPort | remote port.
@parm int32 | flags | flags.
@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EINVALIDINPUT | invalid input.
@rvalue RTL_EENTRYALREADYEXIST | route entry is already exist.
@rvalue RTL_ENOFREEBUFFER | not enough memory in system.
@comm
	value of protocol should be RTL865X_PROTOCOL_TCP/RTL865X_PROTOCOL_UDP
*/
int32 rtl865x_addNaptConnection(rtl865x_napt_entry *naptEntry, rtl865x_priority *prio)
{
	int32 retval = FAILED;
	unsigned long flags = 0;

	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_addNaptConnection(naptEntry, prio);
	SMP_UNLOCK_ETH_HW(flags);

	return retval;
}

#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
static int32 rtl865x_nat_callbackFn_for_del_arp(void *param)
{
	uint32 i=0;
	//int ret=0;
	struct nat_entry *natEntryPtr=NULL;
	rtl865x_arpMapping_entry_t *arpEntry=NULL;	
	rtl865x_napt_entry naptEntry;
	if(param==NULL)
	{
		return EVENT_CONTINUE_EXECUTE;
	}
	arpEntry=(rtl865x_arpMapping_entry_t *)param;
	memset(&naptEntry,0,sizeof(rtl865x_napt_entry));
	for (i=0; i<RTL8651_TCPUDPTBL_SIZE; i++)
	{
		natEntryPtr= &nat_tbl.nat_bucket[i];
		if (NAT_INUSE(natEntryPtr)) {
			
			if ((arpEntry->ip==natEntryPtr->int_ip_)|| (arpEntry->ip==natEntryPtr->rem_ip_))
			{
				#if 0
				printk("intIp is 0x%x, intPort is %d, remIp is 0x%x, remPort is %d, extIp is 0x%x, extPort is %d,[%s]:[%d]\n", 
				natEntryPtr->int_ip_, natEntryPtr->int_port_,natEntryPtr->rem_ip_, natEntryPtr->rem_port_, 
				natEntryPtr->ext_ip_, natEntryPtr->ext_port_,__FUNCTION__,__LINE__);
				#endif
				naptEntry.protocol=natEntryPtr->proto_;
				naptEntry.intIp=natEntryPtr->int_ip_;
				naptEntry.intPort=natEntryPtr->int_port_;
				naptEntry.remIp=natEntryPtr->rem_ip_;
				naptEntry.remPort=natEntryPtr->rem_port_;
				naptEntry.extIp=natEntryPtr->ext_ip_;
				naptEntry.extPort=natEntryPtr->ext_port_;
				//ret=1;
				rtl865x_delNaptConnection(&naptEntry);
			}
		}
	
	}
	#if 0
	if(ret)
		rtl865x_delNaptConnection(&naptEntry);
	#endif
	return EVENT_CONTINUE_EXECUTE;

}

static int32 rtl865x_nat_unRegister_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_nat_callbackFn_for_del_arp;
	rtl865x_unRegisterEvent(&eventParam);
	return SUCCESS;

}
static int32 rtl865x_nat_register_event(void)
{
	rtl865x_event_Param_t eventParam;
	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_DEL_ARP;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_nat_callbackFn_for_del_arp;
	rtl865x_registerEvent(&eventParam);
	return SUCCESS;

}

#endif
/*
@func int32 | rtl865x_delNaptConnection |delete a napt entry.
@parm uint32 | protocol | protocol.
@parm ipaddr_t | intIp | internal ip address.
@parm uint32 | intPort | internal port.
@parm ipaddr_t | extIp | external ip address.
@parm uint32 | extPort | external port.
@parm ipaddr_t | remIp | remote ip address.
@parm uint32 | remPort | remote port.

@rvalue SUCCESS | success.
@rvalue FAILED | failed.
@rvalue RTL_EENTRYNOTFOUND | not found this entry in system.
@comm
*/
int32 rtl865x_delNaptConnection(rtl865x_napt_entry *naptEntry)
{
	int32 retval = FAILED;
	unsigned long flags = 0;

	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_delNaptConnection(naptEntry);
	SMP_UNLOCK_ETH_HW(flags);

	return retval;
}

int32 rtl865x_naptSync(rtl865x_napt_entry *naptEntry, uint32 refresh)
{
	return _rtl865x_naptSync(naptEntry,refresh);
}

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
inline static int32 rtl865x_naptSetAsicWithPriority(struct nat_entry *entry, int32 priority)
{
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat;
	int32		idx;

	//If qos enabled, not add inbound napt
	//That is all downlink pkt will be trapped to CPU for software QoS
	if(entry->flags&NAT_INBOUND)
		return SUCCESS;
	
	//printk("--%s(%d),entry(%p)\n",__FUNCTION__,__LINE__,entry);
	idx = (entry->flags&NAT_INBOUND)?entry->in:entry->out;
	
	rtl8651_getAsicNaptTcpUdpTable(idx, &asic_nat);
	asic_nat.priority = priority;
	asic_nat.priValid = TRUE;
	rtl8651_setAsicNaptTcpUdpTable(1, idx, &asic_nat);

	return SUCCESS;
}

static int32 rtl865x_naptCallbackFn_for_qosChange(void *param)
{
	int num, i;
	struct nat_entry *nat_this, *nat_that;
	ipaddr_t		sip, dip;
	uint16		sport, dport;
	rtl865x_route_t		rt;
	rtl865x_arpMapping_entry_t	arpInfo;
	rtl865x_AclRule_t		rule4, rule2;
	int32		priority=-1, defPriority=-1;

	num = i = 0;

	while(num < nat_tbl.connNum && i < RTL8651_TCPUDPTBL_SIZE)
	{
		if(NAT_INUSE(&nat_tbl.nat_bucket[i]))
		{
			nat_this = &nat_tbl.nat_bucket[i];
			if (nat_this->flags&NAT_INBOUND)
			{
				if(nat_this->out!=0xFFFFFFFF)
				{
					nat_that = &nat_tbl.nat_bucket[nat_this->out];
				}
				else
				{
					/*no outbound*/
					nat_that = NULL;
				}
				
				sip = nat_this->rem_ip_;
				dip = nat_this->ext_ip_;
				sport = nat_this->rem_port_;
				dport = nat_this->ext_port_;
			}
			else
			{
				if(nat_this->in!=0xFFFFFFFF)
				{
					nat_that = &nat_tbl.nat_bucket[nat_this->in];
				}
				else
				{
					/*no inbound*/
					nat_that=NULL;
				}
				
				sip = nat_this->int_ip_;
				dip = nat_this->rem_ip_;
				sport = nat_this->int_port_;
				dport = nat_this->rem_port_;
			}
			
			if (nat_this->flags&NAT_PRI_PROCESSED)
			{
				CLR_NAT_FLAGS(nat_this, NAT_PRI_PROCESSED);
				num++;
			}
			else 
			{
				if (rtl865x_getRouteEntry(sip, &rt)==SUCCESS)
				{
					memset(&rule4, 0, sizeof(rtl865x_AclRule_t));
					rule4.ruleType_ = (nat_this->proto_==RTL865X_PROTOCOL_TCP?RTL865X_ACL_TCP:RTL865X_ACL_UDP);
					rule4.srcIpAddr_ = sip;
					rule4.dstIpAddr_ = dip;
					rule4.tcpSrcPortLB_ = sport;
					rule4.tcpDstPortLB_ = dport;
					rule4.netifIdx_ = _rtl865x_getNetifIdxByNameExt(rt.dstNetif->name);
					if(rule4.netifIdx_ < 0 || rule4.netifIdx_ >= NETIF_NUMBER)
					{
						printk("===%s %s(%d) Can't get netif(%s)\n",__FILE__,__FUNCTION__,__LINE__,rt.dstNetif->name);						
					}

					if(rule4.netifIdx_ >=0 && rule4.netifIdx_ < NETIF_NUMBER)
						if (rtl865x_qosCheckNaptPriority(&rule4)!=SUCCESS)
						{
							if (nat_this->flags&NAT_INBOUND)
							{
								rule4.dstIpAddr_ = nat_this->int_ip_;
								rule4.tcpDstPortLB_ = nat_this->int_port_;
								if (rtl865x_qosCheckNaptPriority(&rule4)!=SUCCESS)
									defPriority = rule4.priority_;
							}
							else
								defPriority = rule4.priority_;
						}
						
					if (rtl865x_getArpMapping(sip, &arpInfo)==SUCCESS)
					{
						memset(&rule2, 0, sizeof(rtl865x_AclRule_t));
						rule2.ruleType_ = RTL865X_ACL_MAC;
						memcpy(rule2.srcMac_.octet, arpInfo.mac.octet, ETHER_ADDR_LEN);
						memset(rule2.srcMacMask_.octet, 0xff, ETHER_ADDR_LEN);
						rule2.netifIdx_ = _rtl865x_getNetifIdxByNameExt(rt.dstNetif->name);
						if(rule4.netifIdx_ < 0 || rule4.netifIdx_ >= NETIF_NUMBER)
						{
							printk("===%s %s(%d) Can't get netif(%s)\n",__FILE__,__FUNCTION__,__LINE__,rt.dstNetif->name);						
						}
						
						if(rule4.netifIdx_ >=0 && rule4.netifIdx_ < NETIF_NUMBER)
							if (rtl865x_qosCheckNaptPriority(&rule2)!=SUCCESS)
							{
								if (nat_this->flags&NAT_INBOUND)
								{
									if(rtl865x_getArpMapping(nat_this->int_ip_, &arpInfo)==SUCCESS)
									{
										memset(&rule2, 0, sizeof(rtl865x_AclRule_t));
										rule2.ruleType_ = RTL865X_ACL_MAC;
										memcpy(rule2.dstMac_.octet, arpInfo.mac.octet, ETHER_ADDR_LEN);
										memset(rule2.dstMacMask_.octet, 0xff, ETHER_ADDR_LEN);
										rule2.netifIdx_ = _rtl865x_getNetifIdxByNameExt(rt.dstNetif->name);
										if (rtl865x_qosCheckNaptPriority(&rule2)!=SUCCESS)
											defPriority = rule2.priority_;
									}
								}
								else
									defPriority = rule2.priority_;
							}
					}

					if (rule4.aclIdx&& rule2.aclIdx)
					{
						priority = (rule4.aclIdx<rule2.aclIdx)?rule4.priority_:rule2.priority_;
					}
					else if (rule4.aclIdx)
					{
						priority = rule4.priority_;
					}
					else if (rule2.aclIdx)
					{
						priority = rule2.priority_;
					}

					if (priority>-1)
					{
						
						rtl865x_naptSetAsicWithPriority(nat_this, priority);
						
						if(nat_that!=NULL)
						{
							rtl865x_naptSetAsicWithPriority(nat_that, priority);
						}
						
						if (nat_this->flags&NAT_PRI_HALF_PROCESSED)
						{
							CLR_NAT_FLAGS(nat_this, NAT_PRI_HALF_PROCESSED);
							num++;
						}
						else
						{
							if(nat_that!=NULL)
							{
								SET_NAT_FLAGS(nat_that, NAT_PRI_PROCESSED);
							}
							else
							{
								/*only half accelerated*/
								num++;
							}
						}
					}
					else
					{
						if (nat_this->flags&NAT_PRI_HALF_PROCESSED)
						{
							rtl865x_naptSetAsicWithPriority(nat_this, defPriority>-1?defPriority:0);
							assert(nat_that!=NULL);
							if(nat_that!=NULL)
							{
								rtl865x_naptSetAsicWithPriority(nat_that, defPriority>-1?defPriority:0);
							}
							CLR_NAT_FLAGS(nat_this, NAT_PRI_HALF_PROCESSED);
							num++;
						}
						else
						{
							if(nat_that!=NULL)
							{
								SET_NAT_FLAGS(nat_that, NAT_PRI_HALF_PROCESSED);
							}
							else
							{
								/*only half accelerated*/
								num++;
							}
						}
					}
				}
				else
				{
					if (nat_this->flags&NAT_PRI_HALF_PROCESSED)
					{
						CLR_NAT_FLAGS(nat_this, NAT_PRI_HALF_PROCESSED);
						num++;
					}
					else
					{
						if(nat_that!=NULL)
						{
							SET_NAT_FLAGS(nat_that, NAT_PRI_HALF_PROCESSED);
						}
						else
						{
							/*only half accelerated*/
							num++;
						}
					}
				}
			}
		}

		i++;
	}
	return EVENT_CONTINUE_EXECUTE;
}


static int32 rtl865x_napt_register_qosEvent(void)
{
	rtl865x_event_Param_t eventParam;
	
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_CHANGE_QOSRULE;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_naptCallbackFn_for_qosChange;
	rtl865x_registerEvent(&eventParam);

	eventParam.eventId=EVENT_FLUSH_QOSRULE;
	rtl865x_registerEvent(&eventParam);

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_ARP;
	rtl865x_registerEvent(&eventParam);

	return SUCCESS;
}

static int32 rtl865x_napt_unRegister_qosEvent(void)
{
	rtl865x_event_Param_t eventParam;
	
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_CHANGE_QOSRULE;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_naptCallbackFn_for_qosChange;
	rtl865x_unRegisterEvent(&eventParam);

	eventParam.eventId=EVENT_FLUSH_QOSRULE;
	rtl865x_unRegisterEvent(&eventParam);

	eventParam.eventLayerId=DEFAULT_LAYER3_EVENT_LIST_ID;
	eventParam.eventId=EVENT_ADD_ARP;
	rtl865x_unRegisterEvent(&eventParam);

	return SUCCESS;
}
#endif

/*
@func int32 | rtl865x_setNatFourWay |enable 4way hash algorithm.
@parm int32 | enable | enable or disable.
@rvalue SUCCESS | success.
@comm
	default is enable in system.
*/
int32 rtl865x_setNatFourWay(int32 enable)
{
	 _set4WayHash(enable);
	rtl865x_enableNaptFourWay=enable;
	return SUCCESS;
}

/*
@func int32 | rtl865x_nat_init |initialize napt table.
@rvalue SUCCESS | success.
@comm	
*/
int32 rtl865x_nat_init(void)
{
	int32 retval = FAILED;
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	rtl865x_napt_unRegister_qosEvent();
#endif
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	rtl865x_nat_unRegister_event();
#endif
	retval = _rtl865x_nat_init();
	rtl865x_setNatFourWay(TRUE);

#if	defined(CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_RTL_IPTABLES_RULE_2_ACL)
	rtl865x_napt_register_qosEvent();
#endif
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	rtl865x_nat_register_event();
#endif
	return retval;
}

int32 rtl865x_nat_reinit(void)
{
	return rtl865x_nat_init();

}

#ifdef CONFIG_RTL_PROC_DEBUG
int32 rtl865x_flushAllNaptConnection(void)
{
	uint32 i,outIndex,inIndex;
	struct nat_entry *nat_out=NULL, *nat_in=NULL, *tmp=NULL;
	for(i=0;i<RTL8651_TCPUDPTBL_SIZE;i++)
	{
		tmp = &nat_tbl.nat_bucket[i];

		if(NAT_INUSE(tmp))
		{
			outIndex=tmp->out;
			inIndex=tmp->in;
			if(outIndex!=0xFFFFFFFF)
			{
				nat_out=&nat_tbl.nat_bucket[outIndex];
			}
			else
			{
				nat_out=NULL;
			}

			if(inIndex!=0xFFFFFFFF)
			{
				nat_in=&nat_tbl.nat_bucket[inIndex];	
			}
			else
			{
				nat_in=NULL;	
			}

			if((nat_out==NULL) &&(nat_in==NULL))
			{
				rtl8651_delAsicNaptTcpUdpTable(i, i);
				continue;
			}
			
			if(nat_out==nat_in)
			{
				if(nat_out->flags&NAT_OUTBOUND)
				{
					rtl8651_delAsicNaptTcpUdpTable(outIndex, outIndex);
					memset(nat_out, 0, sizeof(*nat_out));
					nat_tbl.freeEntryNum++;
				}
				else if(nat_in->flags&NAT_INBOUND)
				{
					rtl8651_delAsicNaptTcpUdpTable(inIndex, inIndex);
					memset(nat_in, 0, sizeof(*nat_in));
					nat_tbl.freeEntryNum++;
				}
				else
				{
					/*fatal error*/
					return RTL_EENTRYNOTFOUND;
				}
			}
			else
			{
				if((nat_out!=NULL) && (nat_out->flags&NAT_OUTBOUND))
				{
					rtl8651_delAsicNaptTcpUdpTable(outIndex, outIndex);
					memset(nat_out, 0, sizeof(*nat_out));
					nat_tbl.freeEntryNum++;
				}
				
				if((nat_in!=NULL) && (nat_in->flags&NAT_INBOUND))		
				{
					rtl8651_delAsicNaptTcpUdpTable(inIndex,inIndex);
					memset(nat_in, 0, sizeof(*nat_in));
					nat_tbl.freeEntryNum++;
				}
			}
			
			if(nat_tbl.connNum>0)
			{
				nat_tbl.connNum--;
			}
		
		}
	}
	
	return SUCCESS;
}

int32 rtl865x_sw_napt_seq_read(struct seq_file *s, void *v)
{

	int i;
	struct nat_entry *natEntryPtr;
	int len=0;
	
	len = seq_printf(s, "%s\n", "sw napt table:");
	
	for(i=0; i<RTL8651_TCPUDPTBL_SIZE; i++)
	{
		natEntryPtr= &nat_tbl.nat_bucket[i];
		if(NAT_INUSE(natEntryPtr))
		{
			if(natEntryPtr->flags&NAT_OUTBOUND)
			{
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
				len += seq_printf(s, "[%4d]%s:%d.%d.%d.%d:%d---->%d.%d.%d.%d:%d---->%d.%d.%d.%d:%d\n	  flags:0x%x,outbound:(%d),inbound:(%d) ,isExtNaptEntry(%d)\n",
#else
				len += seq_printf(s, "[%4d]%s:%d.%d.%d.%d:%d---->%d.%d.%d.%d:%d---->%d.%d.%d.%d:%d\n      flags:0x%x,outbound:(%d),inbound:(%d)\n",
#endif
				i,natEntryPtr->proto_==1?"tcp":"udp" ,NIPQUAD(natEntryPtr->int_ip_),natEntryPtr->int_port_,
				NIPQUAD(natEntryPtr->ext_ip_),natEntryPtr->ext_port_,NIPQUAD(natEntryPtr->rem_ip_),natEntryPtr->rem_port_,natEntryPtr->flags,natEntryPtr->out, natEntryPtr->in
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
				,natEntryPtr->isExtNaptEntry
#endif
				);
			}

			if(natEntryPtr->flags&NAT_INBOUND)
			{
			
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
				len += seq_printf(s, "[%4d]%s:%d.%d.%d.%d:%d<----%d.%d.%d.%d:%d<----%d.%d.%d.%d:%d\n	  flags:0x%x, outbound:(%d), inbound:(%d),isExtNaptEntry(%d)\n",
#else
				len += seq_printf(s, "[%4d]%s:%d.%d.%d.%d:%d<----%d.%d.%d.%d:%d<----%d.%d.%d.%d:%d\n      flags:0x%x, outbound:(%d), inbound:(%d)\n",
#endif
				i,natEntryPtr->proto_==1?"tcp":"udp" ,NIPQUAD(natEntryPtr->int_ip_),natEntryPtr->int_port_,
				NIPQUAD(natEntryPtr->ext_ip_),natEntryPtr->ext_port_,NIPQUAD(natEntryPtr->rem_ip_),natEntryPtr->rem_port_,natEntryPtr->flags,natEntryPtr->out,natEntryPtr->in
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
				,natEntryPtr->isExtNaptEntry
#endif

				);
			}
		}
		#if defined(CONFIG_RTL_WTDOG)
		rtl_periodic_watchdog_kick(i, WATCHDOG_NUM_OF_TIMES);
		#endif

	}
	
	len += seq_printf(s, "total napt connection number is %d\n",nat_tbl.connNum);
	len += seq_printf(s, "total free entry number is %d\n",nat_tbl.freeEntryNum);
	return 0;
}

int32  rtl865x_sw_napt_seq_write( struct file *filp, const char *buff,unsigned long len, loff_t *off )
{
	char 	tmpbuf[64];
	uint32	delIndex,inIndex=0,outIndex=0;
	char		*strptr, *cmd_addr;
	char		*tokptr;
	struct nat_entry *nat_out=NULL, *nat_in=NULL, *tmp=NULL;
	
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "flush", 5))
		{
			rtl865x_flushAllNaptConnection();
		}
		else if (!memcmp(cmd_addr, "del", 3))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}

			delIndex=simple_strtol(tokptr, NULL, 0);
			if(delIndex>RTL8651_TCPUDPTBL_SIZE)
			{
				printk("error input!\n");
				return len;
			}
			tmp = &nat_tbl.nat_bucket[delIndex];

			if(NAT_INUSE(tmp))
			{
				outIndex=tmp->out;
				inIndex=tmp->in;
				if(outIndex!=0xFFFFFFFF)
				{
					nat_out=&nat_tbl.nat_bucket[outIndex];
				}
				else
				{
					nat_out=NULL;
				}

				if(inIndex!=0xFFFFFFFF)
				{
					nat_in=&nat_tbl.nat_bucket[inIndex];	
				}
				else
				{
					nat_in=NULL;	
				}
			
				if ((nat_out==NULL) && (nat_in==NULL))
				{
					rtl8651_delAsicNaptTcpUdpTable(delIndex, delIndex);
					goto errout;
				}

				if(nat_out==nat_in)	
				{
					rtl8651_delAsicNaptTcpUdpTable(outIndex, outIndex);
					memset(nat_out, 0, sizeof(*nat_out));
					nat_tbl.freeEntryNum++;
				}
				else
				{
					if((nat_out!=NULL) && (nat_out->flags&NAT_OUTBOUND))
					{
						rtl8651_delAsicNaptTcpUdpTable(outIndex, outIndex);
						memset(nat_out, 0, sizeof(*nat_out));
						nat_tbl.freeEntryNum++;
					}
					
					if((nat_in!=NULL) && ( nat_in->flags&NAT_INBOUND))		
					{
						rtl8651_delAsicNaptTcpUdpTable(inIndex,inIndex);
						memset(nat_in, 0, sizeof(*nat_in));
						nat_tbl.freeEntryNum++;
					}
				}
				
				if(nat_tbl.connNum>0)
				{
					nat_tbl.connNum--;
				}
				printk("del napt flow,outbound:%d,inbound:%d\n", outIndex, inIndex);
			
			}
			
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		return len;
	}

	return len;
}

#endif


#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int rtl_check_ext_port_napt_entry(uint32 sip, uint16 sport, uint32 dip, uint16 dport, uint8 protocol)
{
	
	struct nat_tuple temp;
	struct nat_tuple *nat_tuple;
	uint32 i,hash;
	struct nat_entry *nat_out;

	nat_tuple=&temp;
	nat_tuple->proto=( (protocol==6) ?RTL865X_PROTOCOL_TCP:RTL865X_PROTOCOL_UDP);
	nat_tuple->int_host.ip=sip;
	nat_tuple->int_host.port= sport;
	nat_tuple->rem_host.ip=dip;
	nat_tuple->rem_host.port=dport;

	//printk("sip is 0x%x, sport is %d, dip is 0x%x, dport is %d\n", sip, sport, dip, dport);
	//outbound
	hash = rtl8651_naptTcpUdpTableIndex((uint8)nat_tuple->proto, htonl(nat_tuple->int_host.ip), htons(nat_tuple->int_host.port), 
											htonl(nat_tuple->rem_host.ip), htons(nat_tuple->rem_host.port));
	//panic_printk("hash:%d,[%s]:[%d].\n",hash,__FUNCTION__,__LINE__);
	if(rtl865x_enableNaptFourWay==TRUE)
	{
		for(i=0; i<4; i++)
		{
			nat_out = &nat_tbl.nat_bucket[hash];
			if (((htonl(sip)==nat_out->int_ip_)&&(htons(sport)==nat_out->int_port_)
				&&(htonl(dip)==nat_out->rem_ip_)&&(htons(dport)==nat_out->rem_port_)
				&&(nat_tuple->proto==nat_out->proto_)&&(nat_out->flags&NAT_OUTBOUND)) 
			 )
			{
				return 1;
			}
			hash=(hash&0xFFFFFFFC)+(hash+1)%4;
			assert(hash<=RTL8651_TCPUDPTBL_SIZE);
		}
	}
	else
	{
		nat_out = &nat_tbl.nat_bucket[hash];
		if(
			((htonl(sip)==nat_out->int_ip_)&&(htons(sport)==nat_out->int_port_)
			&&(htonl(dip)==nat_out->rem_ip_)&&(htons(dport)==nat_out->rem_port_)
			&&(nat_tuple->proto==nat_out->proto_)&&(nat_out->flags&NAT_OUTBOUND)) 
		 )
		 {
			return 1;
		}
	}
	//panic_printk("pro:%d,%d,%x,%d,%x,%d[%s]:[%d].\n",protocol,nat_tuple->proto,
	//	sip,sport,dip,dport,__FUNCTION__,__LINE__);
	
	return 0;
}
int32 rtl865x_flush_ext_port_entry(void)
{
	uint32 i,outIndex,inIndex;
	struct nat_entry *nat_out=NULL, *nat_in=NULL, *tmp=NULL;
	for(i=0;i<RTL8651_TCPUDPTBL_SIZE;i++)
	{
		tmp = &nat_tbl.nat_bucket[i];

		if(NAT_INUSE(tmp))
		{
			outIndex=tmp->out;
			inIndex=tmp->in;
			if(outIndex!=0xFFFFFFFF)
			{
				nat_out=&nat_tbl.nat_bucket[outIndex];
			}
			else
			{
				nat_out=NULL;
			}

			if(inIndex!=0xFFFFFFFF)
			{
				nat_in=&nat_tbl.nat_bucket[inIndex];	
			}
			else
			{
				nat_in=NULL;	
			}

			if((nat_out->isExtNaptEntry)&&(nat_out==NULL) &&(nat_in->isExtNaptEntry)&&(nat_in==NULL))
			{
				rtl8651_delAsicNaptTcpUdpTable(outIndex, inIndex);
				continue;
			}
			
			if((nat_out->isExtNaptEntry)&&(nat_out==nat_in))
			{
				if(nat_out->flags&NAT_OUTBOUND)
				{
					rtl8651_delAsicNaptTcpUdpTable(outIndex, outIndex);
					memset(nat_out, 0, sizeof(*nat_out));
					nat_tbl.freeEntryNum++;
				}
				else if(nat_in->flags&NAT_INBOUND)
				{
					rtl8651_delAsicNaptTcpUdpTable(inIndex, inIndex);
					memset(nat_in, 0, sizeof(*nat_in));
					nat_tbl.freeEntryNum++;
				}
				else
				{
					/*fatal error*/
					return RTL_EENTRYNOTFOUND;
				}
			}
			else
			{
				if((nat_out->isExtNaptEntry)&&(nat_out!=NULL) && (nat_out->flags&NAT_OUTBOUND))
				{
					rtl8651_delAsicNaptTcpUdpTable(outIndex, outIndex);
					memset(nat_out, 0, sizeof(*nat_out));
					nat_tbl.freeEntryNum++;
				}
				
				if((nat_in->isExtNaptEntry)&&(nat_in!=NULL) && (nat_in->flags&NAT_INBOUND))		
				{
					rtl8651_delAsicNaptTcpUdpTable(inIndex,inIndex);
					memset(nat_in, 0, sizeof(*nat_in));
					nat_tbl.freeEntryNum++;
				}
			}
			
			if(nat_tbl.connNum>0)
			{
				nat_tbl.connNum--;
			}
		
		}
	}
	
	return SUCCESS;
}

#endif

#if defined(RTL_REFRESH_HW_L2_ENTRY_DECIDE_BY_HW_NAT)
int rtl865x_check_hw_nat_by_ip(ipaddr_t ip)
{
	int i;
	int ret = FAILED;
	struct nat_entry *natEntryPtr;

	for (i=0; i<RTL8651_TCPUDPTBL_SIZE; i++)
	{
		natEntryPtr = &nat_tbl.nat_bucket[i];
		if (NAT_INUSE(natEntryPtr)) {
			if ((ip == natEntryPtr->int_ip_) ||(ip == natEntryPtr->rem_ip_)) {
				ret = SUCCESS;
				break;
			}
		}
	}

	return ret;
}
#endif

