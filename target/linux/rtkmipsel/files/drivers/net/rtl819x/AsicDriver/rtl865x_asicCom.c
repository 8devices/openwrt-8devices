/*
* Program : Switch table basic operation driver
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*
*  Copyright (c) 2011 Realtek Semiconductor Corp.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*/

#include <linux/delay.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
//#include "common/rtl_utils.h"
#include "rtl865x_hwPatch.h"
#include "asicRegs.h"
//#include "assert.h"
#include "rtl865x_asicBasic.h"
#include "rtl865x_asicCom.h"



#define tick_Delay10ms(x) { int i=x; while(i--) __delay(5000); }

static int32 _rtl865xC_lockTLUCounter	=	0;
#if 1
static int32 _rtl865xC_lockTLUPHYREG[RTL8651_PORT_NUMBER+1]	=	{0};
static int32 _rtl_inputBandWidth[RTL8651_PORT_NUMBER+1]	=	{0};
#if defined(CONFIG_RTL_8197F)
static int32 _rtl_inputBandWidth3 = 0;
#endif
static int32 _rtl_FCRegister = 0;
#else
static int32 _rtl865xC_lockTLUPHYREG[RTL8651_PORT_NUMBER+1]	=	{0};
#endif
int32		rtl8651_totalExtPortNum=0; //this replaces all RTL8651_EXTPORT_NUMBER defines
int32		rtl8651_allExtPortMask=0; //this replaces all RTL8651_EXTPORTMASK defines
rtl8651_tblAsic_InitPara_t rtl8651_tblAsicDrvPara;

#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E))  && !defined(CONFIG_RTL_8198C)
int rtl8651_findAsicVlanIndexByVid(uint16 *vid)
{
	int i;
	rtl865x_tblAsicDrv_vlanParam_t vlan;

	for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
	{
		if (rtl8651_getAsicVlan(i, &vlan) == SUCCESS){
			if(*vid == vlan.vid){
				*vid = i;
				return SUCCESS;
			}
		}
	}

	return FAILED;
}

static int rtl8651_getAsicVlanIndex(rtl865xc_tblAsic_vlanTable_t *entry, uint16 *vid)
{
	int i;
	int ret = FAILED;
	rtl865x_tblAsicDrv_vlanParam_t vlan;

	for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
	{
		if ((rtl8651_getAsicVlan(i, &vlan) == SUCCESS) && (entry->vid == vlan.vid)){
			if((entry->memberPort != vlan.memberPortMask) || (entry->egressUntag != vlan.untagPortMask) ||(entry->fid != vlan.fid)){
				*vid = i;
				return SUCCESS;
			}else{
				return FAILED;
			}
		}
	}

	for ( i = 0; i < RTL865XC_VLANTBL_SIZE; i++ )
	{
		if ( rtl8651_getAsicVlan( i, &vlan ) == FAILED )
			break;
	}

	if(i == RTL865XC_VLANTBL_SIZE){
		ret = FAILED;	//vlan table is full
	}else{
		*vid = i;
		ret = SUCCESS;
	}

	return ret;
}
#endif

/*=========================================
  * ASIC DRIVER API: VLAN TABLE
  *=========================================*/
int32 rtl8651_setAsicVlan(uint16 vid, rtl865x_tblAsicDrv_vlanParam_t *vlanp)
{
	rtl865xc_tblAsic_vlanTable_t entry;
#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E))  && !defined(CONFIG_RTL_8198C)
	int flag = FAILED;
#endif

	memset(&entry,0,sizeof(entry));
	if(vlanp == NULL)
		return FAILED;        
	if (vid>=4096)
		return FAILED;
	if(vlanp->memberPortMask > RTL8651_PHYSICALPORTMASK )
		entry.extMemberPort = vlanp->memberPortMask >> RTL8651_PORT_NUMBER;
	if(vlanp->untagPortMask > RTL8651_PHYSICALPORTMASK )
		entry.extEgressUntag = vlanp->untagPortMask >> RTL8651_PORT_NUMBER;	
	entry.memberPort = vlanp->memberPortMask & RTL8651_PHYSICALPORTMASK;
	entry.egressUntag = vlanp->untagPortMask & RTL8651_PHYSICALPORTMASK;
	entry.fid=vlanp->fid;

#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E))  && !defined(CONFIG_RTL_8198C)
	entry.vid=vid;
	flag = rtl8651_getAsicVlanIndex(&entry, &vid);
	if(flag == FAILED)
		return FAILED;
#endif

#if defined(CONFIG_RTL_8197F)
	entry.hp=vlanp->hp;
#endif

	_rtl8651_forceAddAsicEntry(TYPE_VLAN_TABLE, vid, &entry);
	
	return SUCCESS;
}


int32 rtl8651_delAsicVlan(uint16 vid) {
	rtl865xc_tblAsic_vlanTable_t entry;
#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E))  && !defined(CONFIG_RTL_8198C)
	int flag = FAILED;
	flag = rtl8651_findAsicVlanIndexByVid(&vid);
	if(flag == FAILED)
		return FAILED;
#endif
	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_VLAN_TABLE, vid, &entry);
}


int32 rtl8651_getAsicVlan(uint16 vid, rtl865x_tblAsicDrv_vlanParam_t *vlanp) {	
	rtl865xc_tblAsic_vlanTable_t entry;
	if(vlanp == NULL||vid>=4096)
		return FAILED;        

	_rtl8651_readAsicEntry(TYPE_VLAN_TABLE, vid, &entry);	
	if((entry.extMemberPort | entry.memberPort) == 0)
	{
		return FAILED;
	}
	vlanp->memberPortMask = (entry.extMemberPort<<RTL8651_PORT_NUMBER) | entry.memberPort;
	vlanp->untagPortMask = (entry.extEgressUntag<<RTL8651_PORT_NUMBER) |entry.egressUntag;
	vlanp->fid=entry.fid;

#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E))  && !defined(CONFIG_RTL_8198C)
	vlanp->vid=entry.vid;
#endif	

#if defined(CONFIG_RTL_8197F)
	vlanp->hp=entry.hp;
#endif

	return SUCCESS;
}


int32 rtl8651_setAsicPvid(uint32 port, uint32 pvid)
{
	uint32 regValue,offset;
	
	if(port>=RTL8651_AGGREGATOR_NUMBER || pvid>=RTL865XC_VLAN_NUMBER)
		return FAILED;;
	offset=(port*2)&(~0x3);
	regValue=READ_MEM32(PVCR0+offset);
	if((port&0x1))
	{
		regValue=  ((pvid &0xfff) <<16) | (regValue&~0xFFF0000);
	}
	else
	{	
		regValue =  (pvid &0xfff) | (regValue &~0xFFF);
	}
	
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	rtl819x_setSwEthPvid(port, pvid);
#endif

	WRITE_MEM32(PVCR0+offset,regValue);
	return SUCCESS;
}

int32 rtl8651_getAsicPVlanId(uint32 port, uint32 *pvid) 
{
	uint16 offset;
	offset=(port*2)&(~0x3);
	if(port>=RTL8651_AGGREGATOR_NUMBER || pvid == NULL)
		return FAILED;
	if((port&0x1))
	{
		*pvid=(((READ_MEM32(PVCR0+offset)>>16)&0xFFF));		
	}
	else
	{
		*pvid=((READ_MEM32(PVCR0+offset)&0xFFF));
	}
	return SUCCESS;
}

int32 rtl8651_setPortToNetif(uint32 port, uint32 netifidx)
{
	uint16 offset;

	if(port>= RTL8651_AGGREGATOR_NUMBER || netifidx > 8)
		return FAILED;
	offset = (port * 3);
	WRITE_MEM32(PLITIMR,((READ_MEM32(PLITIMR) & (~(0x7 << offset))) | ((netifidx & 0x7)<< offset)));
	return SUCCESS;
}


/*=========================================
 * ASIC DRIVER API: Protocol-based VLAN
 *=========================================*/

/*
@func int32 | rtl8651_defineProtocolBasedVLAN | configure user-definable protocol-based VLAN
@parm uint32 | ruleNo        | Valid values: RTL8651_PBV_RULE_USR1 and RTL8651_PBV_RULE_USR2
@parm uint8  | ProtocolType  | 00:ethernetII, 01:RFC-1042, 10: LLC-Other, 11:reserved
@parm uint16 | ProtocolValue | ethernetII:ether type, RFC-1042:ether type, LLC-Other:PDSAP(8),SSAP(8)}
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_defineProtocolBasedVLAN( uint32 ruleNo, uint8 ProtocolType, uint16 ProtocolValue )
{
	assert( ruleNo == RTL8651_PBV_RULE_USR1 ||
	        ruleNo == RTL8651_PBV_RULE_USR2 );

	if ( ruleNo == RTL8651_PBV_RULE_USR1 )
	{
		WRITE_MEM32( PBVCR0, ( ProtocolType << PBVCR_PROTO_TYPE_OFFSET ) |
		                    ( ProtocolValue << PBVCR_PROTO_VALUE_OFFSET ) );
	}
	else if ( ruleNo == RTL8651_PBV_RULE_USR2 )
	{
		WRITE_MEM32( PBVCR1, ( ProtocolType << PBVCR_PROTO_TYPE_OFFSET ) |
		                     ( ProtocolValue << PBVCR_PROTO_VALUE_OFFSET ) );
	}

	return SUCCESS;
}

/*
@func int32 | rtl8651_setProtocolBasedVLAN | set corresponding table index of protocol-based VLAN
@parm uint32 | ruleNo  | rule Number (1~6)
@parm uint8  | port    | 0~4:PHY  5:MII  6~8:ExtPort
@parm uint8  | vlanIdx | VLAN Table index (0~7)
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_setProtocolBasedVLAN( uint32 ruleNo, uint32 port, uint8 valid, uint16 vlanId )
{
	uint32 addr;
	uint32 value;

	assert( ruleNo > 0 && ruleNo < RTL8651_PBV_RULE_MAX );
//	assert( vlanId < RTL865XC_VLAN_NUMBER );
//	assert( port < RTL8651_AGGREGATOR_NUMBER );
	ruleNo = ruleNo-1;
	valid = valid ? TRUE : FALSE;
	if ( valid == FALSE )
	{
		vlanId = 0; // clear it for looking pretty.
	}

	if ( port < RTL8651_MAC_NUMBER )
	{
		// Port0 ~ Port9
		addr=PBVR0_0 +(ruleNo*5*4) + ((port/2)*4) ;
		value = ( vlanId<<1 | valid );
		if (port&0x1)
			value =(value <<16) | (0x0000FFFF& READ_MEM32(addr));
		else
			value =value|( 0xFFFF0000& READ_MEM32(addr));		
		WRITE_MEM32(addr,value);
	}
	return SUCCESS;
}

/*
@func int32 | rtl8651_getProtocolBasedVLAN | get corresponding table index of protocol-based VLAN
@parm uint32 | ruleNo  | rule Number (1~6)
@parm uint8* | port    | (output) 0~4:PHY  5:MII  6~8:ExtPort
@parm uint8* | vlanIdx | (output) VLAN Table index (0~7)
@rvalue SUCCESS | 
@comm
 */
int32 rtl8651_getProtocolBasedVLAN( uint32 ruleNo, uint32 port, uint8* valid, uint32* vlanId )
{
	uint32 value;

	assert( ruleNo > 0 && ruleNo < RTL8651_PBV_RULE_MAX );
	assert( port < RTL8651_AGGREGATOR_NUMBER );
	ruleNo=ruleNo-1;
	if ( port < RTL865XC_PORT_NUMBER )
	{
		// Port0 ~ Port9		
		value =  READ_MEM32(PBVR0_0+(ruleNo*5*4) +(port/2)*4 );
		if (port&0x1)
			value =(value & 0xffff0000)>>16;
		else
			value &= 0x0000ffff ;
		if ( valid ) *valid = value & 1;
		if ( vlanId ) *vlanId = value >> 1;
	}
	
	assert( *vlanId < RTL865XC_VLAN_NUMBER );

	return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: INTERFACE TABLE
  *=========================================*/
  
/*
@func int32		| rtl865xC_setNetDecisionPolicy	| Set Interface Multilayer-Decision-Base Control 
@parm uint32 | policy | Possible values: NETIF_VLAN_BASED / NETIF_PORT_BASED / NETIF_MAC_BASED
@rvalue SUCCESS	| 	Success
@comm
RTL865xC supports Multilayer-Decision-Base for interface lookup.
 */
int32 rtl865xC_setNetDecisionPolicy( enum ENUM_NETDEC_POLICY policy )
{
	if ( policy == NETIF_PORT_BASED )
		WRITE_MEM32( SWTCR0, ( READ_MEM32( SWTCR0 ) & ~LIMDBC_MASK ) | LIMDBC_PORT );
	else if ( policy == NETIF_MAC_BASED )
		WRITE_MEM32( SWTCR0, ( READ_MEM32( SWTCR0 ) & ~LIMDBC_MASK ) | LIMDBC_MAC );
	else
		WRITE_MEM32( SWTCR0, ( READ_MEM32( SWTCR0 ) & ~LIMDBC_MASK ) | LIMDBC_VLAN );

	return SUCCESS;
}

/*
@func int32		| rtl865x_setDefACLForNetDecisionMiss	| ACL action when netif decision miss match
@parm uint8 | start_ingressAclIdx |acl index
@parm uint8 | end_ingressAclIdx |acl index
@parm uint8 | start_egressAclIdx |acl index
@parm uint8 | end_egressAclIdx |acl index
@rvalue SUCCESS	| 	Success
@comm
RTL865xC supports Multilayer-Decision-Base for interface lookup.
 */
int32 rtl865x_setDefACLForNetDecisionMiss(uint8 start_ingressAclIdx, uint8 end_ingressAclIdx,uint8 start_egressAclIdx,uint8 end_egressAclIdx)
{
	if(start_ingressAclIdx >= RTL8651_ACLHWTBL_SIZE || end_ingressAclIdx >= RTL8651_ACLHWTBL_SIZE ||
		start_egressAclIdx >= RTL8651_ACLHWTBL_SIZE ||end_egressAclIdx >= RTL8651_ACLHWTBL_SIZE )
		return FAILED;
	#if 0
	WRITE_MEM32(DACLRCR,start_ingressAclIdx|end_ingressAclIdx<<7|start_egressAclIdx<<14|end_egressAclIdx<<21);
	#else
	WRITE_MEM32(DACLRCR,start_ingressAclIdx|end_ingressAclIdx<<ACLI_EDA_OFFSET|start_egressAclIdx<<ACLO_STA_OFFSET|end_egressAclIdx<<ACLO_EDA_OFFSET);
	#endif
	return SUCCESS;
}

#if 1 //defined (CONFIG_RTL_LOCAL_PUBLIC)
int32 rtl865xC_getNetDecisionPolicy( enum ENUM_NETDEC_POLICY *policy )
{
	unsigned int tmp=0;
	tmp=READ_MEM32(SWTCR0);
	*policy=NETIF_VLAN_BASED;
	if(tmp & LIMDBC_VLAN)
	{
		*policy=NETIF_VLAN_BASED;
	}
	else if (tmp & LIMDBC_PORT)
	{
		*policy=NETIF_PORT_BASED;
	}
	else if (tmp &  LIMDBC_MAC)
	{
		*policy=NETIF_MAC_BASED;
	}
	
	return SUCCESS;
}

int32 rtl865x_getDefACLForNetDecisionMiss(uint8 *start_ingressAclIdx, uint8 *end_ingressAclIdx,uint8 *start_egressAclIdx,uint8 *end_egressAclIdx)
{
	int defAclRule=0;
	
	defAclRule=READ_MEM32(DACLRCR);
	#if 0
	*start_ingressAclIdx = defAclRule & 0x7f;
	*end_ingressAclIdx = (defAclRule>>7) & 0x7f;
	*start_egressAclIdx = (defAclRule>>14) & 0x7f;
	*end_egressAclIdx =	 (defAclRule>>21) & 0x7f;
	#else
	*start_ingressAclIdx = (defAclRule & ACLI_STA_MASK);
	*end_ingressAclIdx = ((defAclRule & ACLI_EDA_MASK) >> ACLI_EDA_OFFSET);
	*start_egressAclIdx = ((defAclRule & ACLO_STA_MASK) >> ACLO_STA_OFFSET);
	*end_egressAclIdx =	 ((defAclRule & ACLO_EDA_MASK) >> ACLO_EDA_OFFSET);
	#endif

	return SUCCESS;
}
#endif

/*
@func int32		| rtl865x_delNetInterfaceByVid	| Delete ASIC Interface Table according to Vlan ID
@parm uint16 | vid | vlan id .
@rvalue SUCCESS	| 	Success
@rvalue FAILED	| 	Failed
@comm
 */
int32 rtl865x_delNetInterfaceByVid(uint16 vid)
{
	rtl865xc_tblAsic_netifTable_t entry;
	uint32 i,netIfIdx;
	int32 retVal = FAILED;

	netIfIdx = RTL865XC_NETIFTBL_SIZE;

	if(vid < 1 || vid > 4095)
		return FAILED;

	/*search...*/
	for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
	{
		_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
	 	if ( entry.valid && entry.vid==vid )
	 	{
	 		netIfIdx = i;
			break;
		}
	}

	if(netIfIdx < RTL865XC_NETIFTBL_SIZE)
	{
		memset(&entry,0,sizeof(entry));
		retVal = _rtl8651_forceAddAsicEntry(TYPE_NETINTERFACE_TABLE, netIfIdx, &entry);		
	}

	return retVal;
	
}

/*
@func int32		| rtl8651_setAsicNetInterface	| Set ASIC Interface Table 
@parm uint32 | idx | Table index. Specific RTL865XC_NETIFTBL_SIZE to auto-search.
@parm rtl865x_tblAsicDrv_intfParam_t* | intfp | pointer to interface structure to add
@rvalue SUCCESS	| 	Success
@rvalue FAILED	| 	Failed
@comm
To read an interface entry, we provide two ways:
1. given the index which we want to force set
2. leave the index with RTL865XC_NETIFTBL_SIZE, we will search the whole table to find out existed entry or empty entry.
 */

int32 rtl8651_setAsicNetInterface( uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp )
{
	rtl865xc_tblAsic_netifTable_t entry;
	uint32 i;
 
	if(intfp == NULL)
		return FAILED;

	if ( idx==RTL865XC_NETIFTBL_SIZE )
	{
		/* User does not specific idx, we shall find out idx first. */
#ifdef FULL_CAP
		/* search Interface table to see if exists */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{
			_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
	 		if ( entry.valid && entry.vid==intfp->vid )
	 		{
	 			idx = i;
				goto exist;
			}
		}	
		/* Not existed, find an empty entry */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{
			_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if ( !entry.valid )
				break;
		}
		if ( i>=RTL865XC_NETIFTBL_SIZE )
			return FAILED; /* no empty entry */
		idx = i;
#else
		/* search Interface table to see if exists */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{	
			/* Since FPGA only has entry 0,1,6,7, we ignore null entry. */
			if (i>1&&(i<(RTL865XC_NETIFTBL_SIZE-2))) continue;
			
			_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if (entry.valid)
				if (entry.vid==intfp->vid)
		 		{
		 			idx = i;
					goto exist;
				}
		}	
		/* Not existed, find an empty entry */
		for (i=0;i<RTL865XC_NETIFTBL_SIZE;i++)
		{	
			/* Since FPGA only has entry 0,1,6,7, we ignore null entry. */
			if (i>1&&(i<(RTL865XC_NETIFTBL_SIZE-2))) continue;

			_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if (!entry.valid)
			{ 
				break;
			}
		}
		if ( i>=RTL865XC_NETIFTBL_SIZE )
			return FAILED; /* no empty entry */
		idx = i;
#endif
	}
	
exist:
	assert( idx < RTL865XC_NETIFTBL_SIZE );

#ifdef CONFIG_HARDWARE_NAT_DEBUG
/*2007-12-19*/
	rtlglue_printf("%s:%d,idx is %d ,intfp->vid is %d\n",__FUNCTION__,__LINE__,idx, intfp->vid);
#endif

	memset(&entry,0,sizeof(entry));
	entry.valid = intfp->valid;
	entry.vid = intfp->vid;
	entry.mac47_19 = (intfp->macAddr.octet[0]<<21) | (intfp->macAddr.octet[1]<<13) | (intfp->macAddr.octet[2]<<5) |
					 (intfp->macAddr.octet[3]>>3);
	entry.mac18_0 = (intfp->macAddr.octet[3]<<16) | (intfp->macAddr.octet[4]<<8) | (intfp->macAddr.octet[5]);


#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	entry.inACLStartH = (intfp->inAclStart >>1)&0x7f;
	entry.inACLStartL = intfp->inAclStart&0x1;
	entry.inACLEnd    = intfp->inAclEnd;
	entry.outACLStart = intfp->outAclStart;
	entry.outACLEnd   = intfp->outAclEnd;

#ifdef CONFIG_RTL_8198C_FPGA_TEST
	entry.enHWRouteV6 = intfp->enableRouteV6;
#else
	entry.enHWRouteV6 = (rtl8651_getAsicOperationLayer()>2)?	(intfp->enableRouteV6==TRUE? 1: 0):0;
#endif

	entry.enHWRoute   = (rtl8651_getAsicOperationLayer()>2)?	(intfp->enableRoute==TRUE? 1: 0):0;

	switch(intfp->macAddrNumber) {
		case 0:
		case 1:
		    entry.macMaskL = 1;
		    entry.macMaskH = 3;
		break;
		case 2:
		    entry.macMaskL = 0;
		    entry.macMaskH = 3;
		break;
		case 4:
		    entry.macMaskL = 0;
		    entry.macMaskH = 2;
		break;
		case 8:
		    entry.macMaskL = 0;
		    entry.macMaskH = 0;
			break;
		default:
		    return FAILED;//Not permitted macNumber value
	}
	entry.mtu   = intfp->mtu;
	entry.mtuV6 = intfp->mtuV6;
#else

	entry.inACLStartH = (intfp->inAclStart >>2)&0x1f;
	entry.inACLStartL = intfp->inAclStart&0x3;
	entry.inACLEnd = intfp->inAclEnd;
	entry.outACLStart= intfp->outAclStart;
	entry.outACLEnd = intfp->outAclEnd;
	
	entry.enHWRoute = (rtl8651_getAsicOperationLayer()>2)?	(intfp->enableRoute==TRUE? 1: 0):0;


	switch(intfp->macAddrNumber) {
		case 0:
		case 1:
		    entry.macMask = 7;
		break;
		case 2:
		    entry.macMask = 6;
		break;
		case 4:
		    entry.macMask = 4;
		break;
		case 8:
		    entry.macMask = 0;
			break;
		default:
		    return FAILED;//Not permitted macNumber value
	}
	entry.mtuH = intfp->mtu >>3;
	entry.mtuL = intfp->mtu & 0x7;
#endif
	return _rtl8651_forceAddAsicEntry(TYPE_NETINTERFACE_TABLE, idx, &entry);
}


/*
@func int32		| rtl8651_getAsicNetInterface	| Get ASIC Interface Table 
@parm uint32 | idx | Table index. 
@parm rtl865x_tblAsicDrv_intfParam_t* | intfp | pointer to store interface structure
@rvalue SUCCESS	| 	Success
@rvalue FAILED	| 	Failed. Possible reason: idx error, or invalid entry.
@comm
To read an interface entry, we provide two ways:
1. given the index which we want to read
2. leave the index with RTL865XC_NETIFTBL_SIZE, we will search the whole table according the given intfp->vid
 */
int32 rtl8651_getAsicNetInterface( uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp )
{
	rtl865xc_tblAsic_netifTable_t entry;
	uint32 i;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
     uint32   macMask;
#endif
	if(intfp == NULL)
		return FAILED;

	intfp->valid=0;

	if ( idx == RTL865XC_NETIFTBL_SIZE )
	{
		/* idx is not specified, we search whole interface table. */
		for( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
		{
#ifdef FULL_CAP
#else
			/* Since FPGA only has entry 0,1,6,7, we ignore null entry. */
			if (i>1&&(i<(RTL865XC_NETIFTBL_SIZE-2))) continue;
#endif

			_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, i, &entry);
			if ( entry.valid && entry.vid==intfp->vid ){
				goto found;
			}
		}

		/* intfp.vid is not found. */
		return FAILED;
	}
	else
	{
		/* idx is specified, read from ASIC directly. */
		_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, idx, &entry);
	}

found:
	intfp->valid=entry.valid;
	intfp->vid=entry.vid;
	intfp->macAddr.octet[0] = entry.mac47_19>>21;
	intfp->macAddr.octet[1] = (entry.mac47_19 >>13)&0xff;
	intfp->macAddr.octet[2] = (entry.mac47_19 >>5)&0xff;
	intfp->macAddr.octet[3] = ((entry.mac47_19 &0x3f) <<3) | (entry.mac18_0 >>16);
	intfp->macAddr.octet[4] = (entry.mac18_0 >> 8)&0xff;
	intfp->macAddr.octet[5] = entry.mac18_0 & 0xff;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	intfp->inAclEnd     = entry.inACLEnd;
	intfp->inAclStart   = (entry.inACLStartH<<1)|entry.inACLStartL;  
	intfp->outAclStart  = entry.outACLStart;
	intfp->outAclEnd    = entry.outACLEnd;
	intfp->enableRoute  = entry.enHWRoute==1? TRUE: FALSE;
	intfp->enableRouteV6= entry.enHWRouteV6==1? TRUE: FALSE;

    macMask = (entry.macMaskH<<1)|entry.macMaskL;
	switch(macMask)
	{
		case 0:
			intfp->macAddrNumber =8;
			break;
		case 6:
			intfp->macAddrNumber =2;
			break;
		case 4:
			intfp->macAddrNumber =4;
			break;
		case 7:
			intfp->macAddrNumber =1;
			break;			
	}  
	intfp->mtu = entry.mtu;
	intfp->mtuV6 = entry.mtuV6;
#else
	intfp->inAclEnd = entry.inACLEnd;
	intfp->inAclStart= (entry.inACLStartH<<2)|entry.inACLStartL;
	intfp->outAclStart = entry.outACLStart;
	intfp->outAclEnd = entry.outACLEnd;
	intfp->enableRoute = entry.enHWRoute==1? TRUE: FALSE;

	switch(entry.macMask)
	{
		case 0:
			intfp->macAddrNumber =8;
			break;
		case 6:
			intfp->macAddrNumber =2;
			break;
		case 4:
			intfp->macAddrNumber =4;
			break;
		case 7:
			intfp->macAddrNumber =1;
			break;

			
	}
	intfp->mtu = (entry.mtuH <<3)|entry.mtuL;
#endif
	return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: ACL Table
  *=========================================*/

/*
@func int32	| rtl865xC_setDefaultACLReg	| This function sets default ACL Rule Control Register.
@parm uint32	| isIngress	| TRUE if you want to set default ingress ACL register. FLASE if egress ACL register.
@parm uint32	| start	| The starting address in the ACL table.
@parm uint32	| end	| The ending address in the ACL table.
@rvalue SUCCESS	| Done
@comm 
This function sets the ACL range (starting & ending address) of default ACL Rule Control Register.
*/
int32 rtl865xC_setDefaultACLReg(uint32 isIngress, uint32 start, uint32 end)
{
	/* set default ingress ACL reg's start & end index of ASIC ACL table */
	if ( isIngress == TRUE )
	{
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLI_STA_MASK ) ) | start );
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLI_EDA_MASK ) ) | ( end << ACLI_EDA_OFFSET) );
	}
	else		/* egress */
	{
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLO_STA_MASK ) ) | ( start << ACLO_STA_OFFSET ) );
		WRITE_MEM32( DACLRCR, ( READ_MEM32( DACLRCR ) & ( ~ACLO_EDA_MASK ) ) | ( end << ACLO_EDA_OFFSET ) );
	}
	return SUCCESS;
}

/*
@func int32	| rtl865xC_getDefaultACLReg	| This function gets default ACL Rule Control Register.
@parm uint32	| isIngress	| TRUE if you want to set default ingress ACL register. FLASE if egress ACL register.
@parm uint32 *	| start	| Memory to store the starting address in the ACL table.
@parm uint32 *	| end	| Memory to store the ending address in the ACL table.
@rvalue SUCCESS	| Done
@comm 
This function gets the ACL range (starting & ending address) of default ACL Rule Control Register.
*/
int32 rtl865xC_getDefaultACLReg(uint32 isIngress, uint32 *start, uint32 *end)
{
	/* set default ingress ACL reg's start & end index of ASIC ACL table */
	if ( isIngress == TRUE )
	{
		*start = READ_MEM32( DACLRCR ) & ACLI_STA_MASK;
		*end = ( READ_MEM32( DACLRCR ) & ACLI_EDA_MASK ) >> ACLI_EDA_OFFSET;
	}
	else		/* egress */
	{
		*start = ( READ_MEM32( DACLRCR ) & ACLO_STA_MASK ) >> ACLO_STA_OFFSET;
		*end = ( READ_MEM32( DACLRCR ) & ACLO_EDA_MASK ) >> ACLO_EDA_OFFSET;
	}
	return SUCCESS;
}

int32 rtl8651_getAsicAclRule(uint32 index, rtl865xc_tblAsic_aclTable_t *rule)
{
	if(index >= RTL8651_ACLTBL_SIZE+RTL8651_ACLTBL_RESERV_SIZE || rule == NULL)
		return FAILED;
	
	return _rtl8651_readAsicEntry(TYPE_ACL_RULE_TABLE, index, rule);
}

/*
@func int32 | rtl865xC_lockSWCore | stop TLU operation
@rvalue SUCCESS | 
@comm
	When TLU operation stopped, all the received pkt will be queue in rx buffer.
 */
int32 rtl865xC_lockSWCore(void)
{
	if (_rtl865xC_lockTLUCounter == 0)
	{
	#if 1
		/*	record the original value	*/
		_rtl865xC_lockTLUPHYREG[0] = READ_MEM32(PCRP0);
		_rtl865xC_lockTLUPHYREG[1] = READ_MEM32(PCRP1);
		_rtl865xC_lockTLUPHYREG[2] = READ_MEM32(PCRP2);
		_rtl865xC_lockTLUPHYREG[3] = READ_MEM32(PCRP3);
		_rtl865xC_lockTLUPHYREG[4] = READ_MEM32(PCRP4);
		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
			_rtl865xC_lockTLUPHYREG[5] = READ_MEM32(PCRP5);

		/*	disable phy */
		WRITE_MEM32(PCRP0, ((READ_MEM32(PCRP0))&(~PauseFlowControlNway)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);
		WRITE_MEM32(PCRP1, ((READ_MEM32(PCRP1))&(~PauseFlowControlNway)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
		WRITE_MEM32(PCRP2, ((READ_MEM32(PCRP2))&(~PauseFlowControlNway)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
		WRITE_MEM32(PCRP3, ((READ_MEM32(PCRP3))&(~PauseFlowControlNway)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
		WRITE_MEM32(PCRP4, ((READ_MEM32(PCRP4))&(~PauseFlowControlNway)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);

		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			WRITE_MEM32(PCRP5, ((READ_MEM32(PCRP5))&(~PauseFlowControlNway)) ); /* Jumbo Frame */
		}

		//_rtl_inputThrehold = REG32(IQFCTCR);
		_rtl_FCRegister = REG32(FCREN);
		_rtl_inputBandWidth[0] = REG32(IBCR0);
		_rtl_inputBandWidth[1] = REG32(IBCR1);
		_rtl_inputBandWidth[2] = REG32(IBCR2);
		#if defined(CONFIG_RTL_8197F)
		_rtl_inputBandWidth3 = REG32(IBCR3);
		#endif
		//REG32(IQFCTCR) = (_rtl_inputThrehold&0xffffff00);
		REG32(FCREN) = (_rtl_FCRegister&0x3ff);
		REG32(IBCR0) = 0x10001;
		REG32(IBCR1) = 0x10001;
		REG32(IBCR2) = 0x10001;
		#if defined(CONFIG_RTL_8197F)
		REG32(IBCR3) = 0;
		#endif
//		REG32(QNUMCR) = 0;
	#else
		/*	record the original value	*/
		_rtl865xC_lockTLUPHYREG[0] = READ_MEM32(PCRP0);
		_rtl865xC_lockTLUPHYREG[1] = READ_MEM32(PCRP1);
		_rtl865xC_lockTLUPHYREG[2] = READ_MEM32(PCRP2);
		_rtl865xC_lockTLUPHYREG[3] = READ_MEM32(PCRP3);
		_rtl865xC_lockTLUPHYREG[4] = READ_MEM32(PCRP4);
		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
			_rtl865xC_lockTLUPHYREG[5] = READ_MEM32(PCRP5);

		/*	disable phy */
		WRITE_MEM32(PCRP0, ((READ_MEM32(PCRP0))&(~EnablePHYIf)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);
		WRITE_MEM32(PCRP1, ((READ_MEM32(PCRP1))&(~EnablePHYIf)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
		WRITE_MEM32(PCRP2, ((READ_MEM32(PCRP2))&(~EnablePHYIf)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
		WRITE_MEM32(PCRP3, ((READ_MEM32(PCRP3))&(~EnablePHYIf)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
		WRITE_MEM32(PCRP4, ((READ_MEM32(PCRP4))&(~EnablePHYIf)) ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);

		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			WRITE_MEM32(PCRP5, ((READ_MEM32(PCRP5))&(~EnablePHYIf)) ); /* Jumbo Frame */
		}
	#endif
	}

	_rtl865xC_lockTLUCounter++;
	return SUCCESS;
}

/*
@func int32 | rtl865xC_unLockSWCore | restart TLU operation
@rvalue SUCCESS | 
@comm
	restore the system operation.
 */
 int32 rtl865xC_unLockSWCore(void)
 {
 	if (_rtl865xC_lockTLUCounter == 1)
 	{
	#if 1
		//REG32(IQFCTCR) = _rtl_inputThrehold;
		REG32(FCREN) = _rtl_FCRegister;
		//REG32(QNUMCR) = _rtl_QNumRegister;
		REG32(IBCR0) = _rtl_inputBandWidth[0];
		REG32(IBCR1) = _rtl_inputBandWidth[1];
		REG32(IBCR2) = _rtl_inputBandWidth[2];
		#if defined(CONFIG_RTL_8197F)
		REG32(IBCR3) = _rtl_inputBandWidth3;
		#endif

		WRITE_MEM32(PCRP0, _rtl865xC_lockTLUPHYREG[0]); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);
		WRITE_MEM32(PCRP1, _rtl865xC_lockTLUPHYREG[1] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
		WRITE_MEM32(PCRP2, _rtl865xC_lockTLUPHYREG[2] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
		WRITE_MEM32(PCRP3, _rtl865xC_lockTLUPHYREG[3] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
		WRITE_MEM32(PCRP4, _rtl865xC_lockTLUPHYREG[4] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);

		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			WRITE_MEM32(PCRP5, _rtl865xC_lockTLUPHYREG[5] ); /* Jumbo Frame */
		}

	#else
		/*	restore phy */
#if 0
		WRITE_MEM32(PCRP0, ((READ_MEM32(PCRP0))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP1, ((READ_MEM32(PCRP1))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP2, ((READ_MEM32(PCRP2))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP3, ((READ_MEM32(PCRP3))|(EnablePHYIf)) ); /* Jumbo Frame */
		WRITE_MEM32(PCRP4, ((READ_MEM32(PCRP4))|(EnablePHYIf)) ); /* Jumbo Frame */

		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			WRITE_MEM32(PCRP5, ((READ_MEM32(PCRP5))|(EnablePHYIf)) ); /* Jumbo Frame */
		}

#else
		WRITE_MEM32(PCRP0, _rtl865xC_lockTLUPHYREG[0]); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);
		WRITE_MEM32(PCRP1, _rtl865xC_lockTLUPHYREG[1] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
		WRITE_MEM32(PCRP2, _rtl865xC_lockTLUPHYREG[2] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
		WRITE_MEM32(PCRP3, _rtl865xC_lockTLUPHYREG[3] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
		WRITE_MEM32(PCRP4, _rtl865xC_lockTLUPHYREG[4] ); /* Jumbo Frame */
		TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);

		if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B)
		{
			WRITE_MEM32(PCRP5, _rtl865xC_lockTLUPHYREG[5] ); /* Jumbo Frame */
		}
#endif
	#endif
 	}

	_rtl865xC_lockTLUCounter--;
	return SUCCESS;
 }


/*define for version control --Mark*/
#define RLRevID_OFFSET  12
#define RLRevID_MASK    0x0f
#define A_DIFF_B_ADDR   (PCI_CTRL_BASE+0x08) /*B800-3408*/
/* get CHIP version */
int32 rtl8651_getChipVersion(int8 *name,uint32 size, int32 *rev)
{
	int32 revID;
	uint32 val;
	
	revID = ((READ_MEM32(CRMR)) >> RLRevID_OFFSET) & RLRevID_MASK ;
#ifdef CONFIG_RTL_819X	
	strncpy(name,"8196C",size);
#else
	strncpy(name,"865xC",size);
#endif
	if(rev == NULL)
		return SUCCESS;
		
	/*modified by Mark*/
	/*if RLRevID >= 1 V.B  *rev = RLRevID*/
	/*RLRevID == 0 ,then need to check [B800-3408]*/	

	if(revID >= RTL865X_CHIP_REV_B )
	   *rev = revID ;
	else /*A-CUT or B-CUT*/
	{
		val = READ_MEM32(A_DIFF_B_ADDR);
		if(val == 0)			
			*rev = RTL865X_CHIP_REV_A; /* RTL865X_CHIP_REV_A*/
		else
			*rev = RTL865X_CHIP_REV_B; /* RTL865X_CHIP_REV_B*/
	}	
	return SUCCESS;
}

int32 rtl8651_getChipNameID(int32 *id)
{
	*id = RTL865X_CHIP_VER_RTL8196C;
	return  SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: SYSTEM INIT
  *=========================================*/
#define RTL865X_ASIC_DRIVER_SYSTEM_INIT_API

/*
@func void | rtl8651_clearRegister | Clear ALL registers in ASIC
@comm
	Clear ALL registers in ASIC.
	for RTL865xC only
*/

void rtl8651_clearRegister(void)
{
	int i = 0;

	WRITE_MEM32(MACCR,LONG_TXE);	
	WRITE_MEM32(MGFCR_E0R0,0);
	WRITE_MEM32(MGFCR_E0R1,0);
	WRITE_MEM32(MGFCR_E0R2,0);
	WRITE_MEM32(MGFCR_E1R0,0);
	WRITE_MEM32(MGFCR_E1R1,0);
	WRITE_MEM32(MGFCR_E1R2,0);	
	WRITE_MEM32(MGFCR_E2R0,0);
	WRITE_MEM32(MGFCR_E2R1,0);
	WRITE_MEM32(MGFCR_E2R2,0);
	WRITE_MEM32(MGFCR_E3R0,0);
	WRITE_MEM32(MGFCR_E3R1,0);
	WRITE_MEM32(MGFCR_E3R2,0);	
	WRITE_MEM32(VCR0,0);		
	WRITE_MEM32(VCR1,0);	
	WRITE_MEM32(PVCR0,0);	
	WRITE_MEM32(PVCR1,0);	
	WRITE_MEM32(PVCR2,0);	
	WRITE_MEM32(PVCR3,0);	
	WRITE_MEM32(PVCR4,0);	
	WRITE_MEM32(TEACR,0);
	WRITE_MEM32(FFCR,0);
	WRITE_MEM32(DOT1XPORTCR,0);
	WRITE_MEM32(DOT1XMACCR,0);
	WRITE_MEM32(GVGCR,0);
	WRITE_MEM32(SWTCR0,0);
	WRITE_MEM32(SWTCR1,0);
	WRITE_MEM32(PLITIMR,0);
	/* Set all Protocol-Based Reg. to 0 */
	WRITE_MEM32(PBVCR0,  0x00000000);
	WRITE_MEM32(PBVCR1, 0x00000000);
	WRITE_MEM32(PBVR0_0, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_1, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_2, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_3, 0x00000000);	/* IPX */
	WRITE_MEM32(PBVR0_4, 0x00000000);	/* IPX */		
	WRITE_MEM32(PBVR1_0,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_1,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_2,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_3,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR1_4,  0x00000000);	/* NetBIOS */
	WRITE_MEM32(PBVR2_0,  0x00000000);	/* PPPoE Control */
	WRITE_MEM32(PBVR2_1,  0x00000000);	
	WRITE_MEM32(PBVR2_2,  0x00000000);	
	WRITE_MEM32(PBVR2_3,  0x00000000);	
	WRITE_MEM32(PBVR2_4,  0x00000000);	
	WRITE_MEM32(PBVR3_0,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_1,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_2,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_3,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR3_4,  0x00000000);	/* PPPoE Session */
	WRITE_MEM32(PBVR4_0,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_1,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_2,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_3,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR4_4,  0x00000000);	/* User-defined 1 */
	WRITE_MEM32(PBVR5_0,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_1,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_2,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_3,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(PBVR5_4,  0x00000000);	/* User-defined 2 */
	WRITE_MEM32(MSCR,0);   
	
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	WRITE_MEM32(PCRP0, (1 | MacSwReset));
	TOGGLE_BIT_IN_REG_TWICE(PCRP0,EnForceMode);
	WRITE_MEM32(PCRP1, (1 | MacSwReset));  
	TOGGLE_BIT_IN_REG_TWICE(PCRP1,EnForceMode);
	WRITE_MEM32(PCRP2, (1 | MacSwReset));   
	TOGGLE_BIT_IN_REG_TWICE(PCRP2,EnForceMode);
	WRITE_MEM32(PCRP3, (1 | MacSwReset));   
	TOGGLE_BIT_IN_REG_TWICE(PCRP3,EnForceMode);
	WRITE_MEM32(PCRP4, (1 | MacSwReset));   
	TOGGLE_BIT_IN_REG_TWICE(PCRP4,EnForceMode);
#else
	WRITE_MEM32(PCRP0,1);   
	WRITE_MEM32(PCRP1,1);   
	WRITE_MEM32(PCRP2,1);   
	WRITE_MEM32(PCRP3,1);   
	WRITE_MEM32(PCRP4,1);   
#endif	
	WRITE_MEM32(PCRP5,1);   
	WRITE_MEM32(PCRP6,1);   
	WRITE_MEM32(PCRP7,1);   
	WRITE_MEM32(PCRP8,1);   
	WRITE_MEM32(PPMAR,1);
	WRITE_MEM32(SIRR, TRXRDY);
	/* WRITE_MEM32(TMCR,0xFCFC0000); */
	WRITE_MEM32(QIDDPCR,(0x1<<NAPT_PRI_OFFSET)|(0x1<<ACL_PRI_OFFSET)|(0x1<<DSCP_PRI_OFFSET)|(0x1<<BP8021Q_PRI_OFFSET)|(0x1<<PBP_PRI_OFFSET));
	WRITE_MEM32(LPTM8021Q,0);
	WRITE_MEM32(DSCPCR0,0);
	WRITE_MEM32(DSCPCR1,0);
	WRITE_MEM32(DSCPCR2,0);
	WRITE_MEM32(DSCPCR3,0);
	WRITE_MEM32(DSCPCR4,0);
	WRITE_MEM32(DSCPCR5,0);
	WRITE_MEM32(DSCPCR6,0);	
	WRITE_MEM32(RMACR,0);
	WRITE_MEM32(ALECR,0);

	/* System Based Flow Control Threshold Register */
	WRITE_MEM32( SBFCR0, (0x1E0<<S_DSC_RUNOUT_OFFSET) );
	WRITE_MEM32( SBFCR1, (0x0190<<S_DSC_FCOFF_OFFSET)|(0x01CC<<S_DSC_FCON_OFFSET) );
	WRITE_MEM32( SBFCR2, (0x0050<<S_Max_SBuf_FCOFF_OFFSET)|(0x006C<<S_Max_SBuf_FCON_OFFSET) );
	/* Port Based Flow Control Threshold Register */
	WRITE_MEM32( PBFCR0, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR1, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR2, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR3, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR4, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );
	WRITE_MEM32( PBFCR5, (0x003C<<P_MaxDSC_FCOFF_OFFSET)|(0x005A<<P_MaxDSC_FCON_OFFSET) );

	/* Packet Schecduling Control Register */
	WRITE_MEM32(ELBPCR, (51<<Token_OFFSET)|(38<<Tick_OFFSET) );
	WRITE_MEM32(ELBTTCR, /*0x99*/0x400<<L2_OFFSET );
	WRITE_MEM32(ILBPCR1, (0<<UpperBound_OFFSET)|(0<<LowerBound_OFFSET) );
	WRITE_MEM32(ILBPCR2, (0x33<<ILB_feedToken_OFFSET)|(0x26<<ILB_Tick_OFFSET) );
	for( i = 0; i<42; i++ )
		WRITE_MEM32(P0Q0RGCR+i*4, (7<<PPR_OFFSET)|(0xff<<L1_OFFSET)|(0x3FFF<<APR_OFFSET) );
	for( i = 0; i<7; i++ )
	{
		#if defined(CONFIG_RTL_8197F)
		WRITE_MEM32(WFQRCRP0+i*4*3+ 0, 0xfffff<<APR_OFFSET ); /*WFQRCRP0*/
		#else
		WRITE_MEM32(WFQRCRP0+i*4*3+ 0, 0x3fff<<APR_OFFSET ); /*WFQRCRP0*/
		#endif
		WRITE_MEM32(WFQRCRP0+i*4*3+ 4, 0 ); 				 /* WFQWCR0P0 */
		WRITE_MEM32(WFQRCRP0+i*4*3+ 8, 0 ); 				 /* WFQWCR1P0 */
	}

	for (i=0;i<48;i+=4)
			WRITE_MEM32(PATP0+i,0xfe12);	

	rtl8651_totalExtPortNum=0;

	/* Set chip's mode as NORMAL mode */
	WRITE_MEM32(TMCR,~ENHSBTESTMODE&READ_MEM32(TMCR));

	/* CPU NIC */
	WRITE_MEM32(CPUICR,READ_MEM32(CPUICR)&~(LBMODE|LB10MHZ|MITIGATION|EXCLUDE_CRC)); 
	WRITE_MEM32(CPUIISR,READ_MEM32(CPUIISR));  /* clear all interrupt */
	WRITE_MEM16(CPUQDM0,0); 
	WRITE_MEM16(CPUQDM1,0); 
	WRITE_MEM16(CPUQDM2,0); 
	WRITE_MEM16(CPUQDM3,0); 
	WRITE_MEM16(CPUQDM4,0); 
	WRITE_MEM16(CPUQDM5,0); 
	WRITE_MEM32(RMCR1P,0); 
	WRITE_MEM32(DSCPRM0,0); 
	WRITE_MEM32(DSCPRM1,0); 
	WRITE_MEM32(RLRC,0); 
	
#if 0 /* Since swNic only init once when starting model code, we don't clear the following registers. */
	WRITE_MEM32(CPURPDCR0,0); 
	WRITE_MEM32(CPURPDCR1,0); 
	WRITE_MEM32(CPURPDCR2,0); 
	WRITE_MEM32(CPURPDCR3,0); 
	WRITE_MEM32(CPURPDCR4,0); 
	WRITE_MEM32(CPURPDCR5,0); 
	WRITE_MEM32(CPURMDCR0,0); 
	WRITE_MEM32(CPUTPDCR0,0); 
	WRITE_MEM32(CPUTPDCR1,0); 
	WRITE_MEM32(CPUIIMR,0); 
#endif

}

void rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count) 
{
	rtl865xc_tblAsic_aclTable_t entry;
	uint32 idx;
	
	memset(&entry,0,sizeof(entry));
	for (idx=0; idx<count; idx++)// Write into hardware
		_rtl8651_forceAddAsicEntry(type, idx, &entry);

}

int32 rtl8651_clearAsicCommTable(void)
{
	//rtl8651_clearSpecifiedAsicTable(TYPE_L2_SWITCH_TABLE, RTL8651_L2TBL_ROW*RTL8651_L2TBL_COLUMN);
	//rtl8651_clearSpecifiedAsicTable(TYPE_RATE_LIMIT_TABLE, RTL8651_RATELIMITTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_NETINTERFACE_TABLE, RTL865XC_NETINTERFACE_NUMBER);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	REG32(MEMCR) = 0;
	REG32(MEMCR) = ((1<<2) | (1<<5)); // VLAN and ACL

	// wait for ACL table clear done due to the code below will set ACL table.
	//while ( (REG32(MEMCR) & (1<< (5+8))) == 0 ) {} ;
	while ( (REG32(MEMCR) & ((1<<10) | (1<<13))) != ((1<<10) | (1<<13)) ) {} ;
#else
	rtl8651_clearSpecifiedAsicTable(TYPE_VLAN_TABLE, RTL865XC_VLANTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_ACL_RULE_TABLE, RTL8651_ACLTBL_SIZE);
#endif

	/*
	_rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOE_NUMBER);
	_rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_ALG_TABLE, RTL865XC_ALGTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_SERVER_PORT_TABLE, RTL8651_SERVERPORTTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, RTL8651_TCPUDPTBL_SIZE);
	_rtl8651_clearSpecifiedAsicTable(TYPE_L4_ICMP_TABLE, RTL8651_ICMPTBL_SIZE);	
	
	*/	

	{
		rtl865xc_tblAsic_aclTable_t rule;
		int32 aclIdx;
		
		memset(&rule,0,sizeof(rtl865xc_tblAsic_aclTable_t));
		rule.actionType = 0x0;
		rule.ruleType = 0x0;
		for(aclIdx=0; aclIdx<=RTL8651_ACLHWTBL_SIZE; aclIdx++)
			_rtl8651_forceAddAsicEntry(TYPE_ACL_RULE_TABLE,aclIdx, &rule);
		
	}
	
	return SUCCESS;
}

#if defined(CONFIG_OPENWRT_SDK)
unsigned int GetSysClockRateForAsicDrv(void)
{
	unsigned int SysClkRate;
	
    REG32(MACMR) = REG32(MACMR) & ~SYS_CLK_MASK;
    switch ( REG32(MACMR) & SYS_CLK_MASK )
    {
        case SYS_CLK_100M:
            SysClkRate = 100000000;
            break;
        case SYS_CLK_90M:
            SysClkRate = 90000000;
            break;
        case SYS_CLK_85M:
            SysClkRate = 85000000;
            break;
        case SYS_CLK_96M:
            SysClkRate = 96000000;
            break;
        case SYS_CLK_80M:
            SysClkRate = 80000000;
            break;
        case SYS_CLK_75M:
            SysClkRate = 75000000;
            break;
        case SYS_CLK_70M:
            SysClkRate = 70000000;
            break;
        case SYS_CLK_50M:
            SysClkRate = 50000000;
            break;
        default:
            while(1);
    }

    return SysClkRate;
}
#endif

/*=========================================
  * ASIC DRIVER API: SWITCH MODE
  *=========================================*/
static int32 rtl8651_operationlayer=0;
int32 rtl8651_setAsicOperationLayer(uint32 layer) 
{
	if(layer<1 || layer>4)
		return FAILED;
	/*   for bridge mode ip multicast patch
		When  in bridge mode,
		only one vlan(8) is available,
		if rtl8651_operationlayer is set less than 3,
		(pelase refer to rtl8651_setAsicVlan() )
		the "enable routing bit" of VLAN table will be set to 0 according to rtl8651_operationlayer.
		On the one hand, multicast data is flooded in vlan(8) by hardware, 
		on the other hand,it will be also trapped to cpu.
		In romedriver process,
		it will do _rtl8651_l2PhysicalPortRelay() in _rtl8651_l2switch(),
		and results in same multicast packet being flooded twice: 
		one is by hareware, the other is by romedriver.
		so the minimum  rtl8651_operationlayer will be set 3.
	*/
	#if 0//defined (CONFIG_RTL_HARDWARE_MULTICAST)
	/*if hardware multicast is enable the minimium asic operation layer must be above layer 3  */
	if(layer<4)
	{
		layer=4;
	}
	#endif
	if(layer == 1) {
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_L2|EN_L3|EN_L4));
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_IN_ACL));
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_OUT_ACL));
	}else{
		/*
		 * Egress acl check should never enable, becuase of some hardware bug 
		 * reported by alpha    2007/12/05
		 */
		#ifndef CONFIG_RTL_NFJROM_MP 
		WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_IN_ACL));
		#endif
		/*WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_OUT_ACL));*/

		if(layer == 2) {
			WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_L2));
			WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_L3|EN_L4));
		}
		else
		{	// options for L3/L4 enable
			//WRITE_MEM32(MISCCR,READ_MEM32(MISCCR)|FRAG2CPU); //IP fragment packet need to send to CPU when multilayer is enabled
			/*
			  * Only for RTL8650B:
			  * It is no need to trap fragment packets to CPU in pure rouitng mode while ACL is enabled, hence we should 
			  * turn this bit (FRAG2CPU) off.
			  * NOTE: if we do this, we should also turn ENFRAG2ACLPT on.     
			  *														-chhuang, 7/30/2004
			  */
			   /*
			  *    FRAG2CPU bit should be in ALECR register, not MSCR.
			  */
			//WRITE_MEM32(MSCR,READ_MEM32(MSCR) & ~FRAG2CPU);
			#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
			WRITE_MEM32(ALECR,READ_MEM32(ALECR) | FRAG2CPU);
			#else
			WRITE_MEM32(ALECR,READ_MEM32(ALECR) & ~FRAG2CPU);
			#endif
		
			if(layer == 3) {
				WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_L2|EN_L3));
				WRITE_MEM32(MSCR,READ_MEM32(MSCR)&~(EN_L4));
			}
			else {	// layer 4
				WRITE_MEM32(MSCR,READ_MEM32(MSCR)|(EN_L2|EN_L3|EN_L4));
			}
		}
	}
	if(layer == 1)
		rtl8651_setAsicAgingFunction(FALSE, FALSE);
	else if (layer == 2 || layer == 3)
		rtl8651_setAsicAgingFunction(TRUE, FALSE);
	else
		rtl8651_setAsicAgingFunction(TRUE, TRUE);
	rtl8651_operationlayer	=layer;
	return SUCCESS;
}



int32 rtl8651_getAsicOperationLayer(void) 
{
	return  rtl8651_operationlayer;
}

int32 rtl8651_setAsicAgingFunction(int8 l2Enable, int8 l4Enable)
{
	WRITE_MEM32( TEACR, (READ_MEM32(TEACR) & ~0x3) |(l2Enable == TRUE? 0x0: 0x1) | (l4Enable == TRUE? 0x0 : 0x2));
	return SUCCESS;
}

int32 rtl8651_getAsicAgingFunction(int8 * l2Enable, int8 * l4Enable) {
	if(l2Enable == NULL || l4Enable == NULL)
		return FAILED;

	*l2Enable = (READ_MEM32(TEACR) & 0x1)? FALSE: TRUE;
	*l4Enable = (READ_MEM32(TEACR) & 0x2)? FALSE: TRUE;
	return SUCCESS;
}

   #define BSP_SW_IE           (1 << 15)
#if defined(EN_STROM_CTRL_BEFORE_SYS_SETTLED)
extern uint32 _1s_count;
#endif
void rtl865x_start(void)
{
	/* set "Lexra Bus Burst Size" to 128
		Prevents DMA occupying the Lexra bus for too long.
		00: 32 Bytes,  01: 64 (alignment issue must be fixed) 10:128
	*/
	//REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_32WORDS | MBUF_2048BYTES;
	REG32(CPUICR) = TXCMD | RXCMD | BUSBURST_128WORDS | MBUF_2048BYTES;

#if defined(CONFIG_RTL_8197F)
	// the HiFifoMark value will be reset to default value (0x57) after updated the burst size field of CPUICR
	REG32(DMA_CR0) = (REG32(DMA_CR0) & ~(LowFifoMark_MASK|HiFifoMark_MASK)) | ((LOW_FIFO_MARK_VLAUE<<LowFifoMark_OFFSET)|HIGH_FIFO_MARK_VLAUE);
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	REG32(0xbb0c0000) = REG32(0xbb0c0000); // read entry 0 of ACL table to fix driver can't receive packet when lan un-plug
#endif

	REG32(CPUIISR) = REG32(CPUIISR);
#if defined(CONFIG_RTL_8197F) && defined(EN_RTL_INTERRUPT_MIGRATION)
	REG32(CPUIMCR) |=  (BIT(0) | BIT(8));
	REG32(CPUIMTTR0) |= 0x3FF; 
	REG32(CPUIMTTR2) |= 0x3FF; 	
	REG32(CPUIMPNTR0) |= 0x3F; 
	REG32(CPUIMPNTR2) |= 0x3F; 
#endif
#if 1
#if defined(EN_RTL_INTERRUPT_MIGRATION)
	REG32(CPUIIMR) = RX_DONE_IE_ALL | TX_DONE_IE_ALL | LINK_CHANGE_IE | PKTHDR_DESC_RUNOUT_IE_ALL;
#else
	REG32(CPUIIMR) = RX_DONE_IE_ALL | TX_ALL_DONE_IE_ALL | LINK_CHANGE_IE | PKTHDR_DESC_RUNOUT_IE_ALL;
#endif
#else
	//REG32(CPUIIMR) = RX_DONE_IE_ALL | LINK_CHANGE_IE | PKTHDR_DESC_RUNOUT_IE_ALL | MBUF_DESC_RUNOUT_IE_ALL | TX_ALL_DONE_IE_ALL;
	REG32(CPUIIMR) = RX_DONE_IE_ALL | LINK_CHANGE_IE | PKTHDR_DESC_RUNOUT_IE_ALL | MBUF_DESC_RUNOUT_IE_ALL;
#endif
	REG32(SIRR) = TRXRDY;
	REG32(GIMR) |= (BSP_SW_IE);
#if defined(EN_STROM_CTRL_BEFORE_SYS_SETTLED)
	_1s_count = 1; // for reinit case, like "init.sh gw all"
#endif
}

void rtl865x_down(void)
{
	REG32(CPUIIMR) = 0;
	REG32(CPUIISR) = REG32(CPUIISR); 
	REG32(GIMR) &= ~(BSP_SW_IE);
	REG32(CPUICR) = 0; 
	REG32(SIRR) = 0;
}

unsigned int rtl865x_probeSdramSize(void)
{
	unsigned int memsize;
	unsigned int MCRsdram;
#if defined(CONFIG_RTL_819X)
	unsigned int colcnt, rowcnt;
	unsigned int colsize, rowsize;
	MCRsdram = READ_MEM32(MCR);
	{
		/*	96c & 98	*/
		colcnt = (READ_MEM32(DCR)&COLCNT_MASK)>>COLCNT_OFFSET;
		rowcnt = (READ_MEM32(DCR)&ROWCNT_MASK)>>ROWCNT_OFFSET;
		switch (colcnt)
		{
			case 0:	colsize = 256; break;
			case 1:	colsize = 512; break;
			case 2:	colsize = 1024; break;
			case 3:	colsize = 2048; break;
			case 4:	colsize = 4096; break;
			default:
			printk( "DDR SDRAM unknown(0x%08X):column cnt(0x%x)\n", MCRsdram, colcnt); 
			memsize = 0;
			goto out;
		}
		switch(rowcnt)
		{
			case 0:	rowsize = 2048; break;
			case 1:	rowsize = 4096; break;
			case 2:	rowsize = 8192; break;
			case 3:	rowsize = 16384; break;
			default:
			printk( "DDR SDRAM unknown(0x%08X):row cnt(0x%x)\n", MCRsdram, rowcnt); 
			memsize = 0;
			goto out;
		}
		memsize = (colsize*rowsize)<<3;
	}
out:
#else
	switch ( MCRsdram = (READ_MEM32( MCR ) & 0x1C100010 ) )
	{
		/* SDRAM 16-bit mode */
		case 0x00000000: memsize =  2<<20; break;
		case 0x04000000: memsize =  4<<20; break;
		case 0x08000000: memsize =  8<<20; break;
		case 0x0C000000: memsize = 16<<20; break;
		case 0x10000000: memsize = 32<<20; break;
		case 0x14000000: memsize = 64<<20; break;

		/* SDRAM 16-bit mode - 2 chip select */
		case 0x00000010: memsize =  4<<20; break;
		case 0x04000010: memsize =  8<<20; break;
		case 0x08000010: memsize = 16<<20; break;
		case 0x0C000010: memsize = 32<<20; break;
		case 0x10000010: memsize = 64<<20; break;
		case 0x14000010: memsize = 128<<20; break;

		/* SDRAM 32-bit mode */
		case 0x00100000: memsize =  4<<20; break;
		case 0x04100000: memsize =  8<<20; break;
		case 0x08100000: memsize = 16<<20; break;
		case 0x0C100000: memsize = 32<<20; break;
		case 0x10100000: memsize = 64<<20; break;
		case 0x14100000: memsize =128<<20; break;

		/* SDRAM 32-bit mode - 2 chip select */
		case 0x00100010: memsize =  8<<20; break;
		case 0x04100010: memsize = 16<<20; break;
		case 0x08100010: memsize = 32<<20; break;
		case 0x0C100010: memsize = 64<<20; break;
		case 0x10100010: memsize =128<<20; break;
		/*
		case 0x14100010: memsize =256<<20; break;
		*/

		default:
			printk( "SDRAM unknown(0x%08X)\n", MCRsdram ); 
			memsize = 0;
			break;
	}
#endif
	return memsize;
}

/*=========================================
  * ASIC DRIVER API: ASIC Counter
  *=========================================*/
static void _rtl8651_initialRead(void) {//RTL8651 read counter for the first time will get value -1 and this is meaningless
	uint32 i;
	for(i=0; i<=0xac; i+=0x4)
	{
		rtl8651_returnAsicCounter(i);
	}
}

uint32 rtl8651_returnAsicCounter(uint32 offset) 
{
	if(offset & 0x3)
		return 0;
	return  READ_MEM32(MIB_COUNTER_BASE + offset);
}

uint64 rtl865xC_returnAsicCounter64(uint32 offset)
{
	if ( offset & 0x3 )
		return 0;

	return ( READ_MEM32( MIB_COUNTER_BASE + offset ) + ( ( uint64 ) READ_MEM32( MIB_COUNTER_BASE + offset + 4 ) << 22 ) );
}

int32 rtl8651_clearAsicCounter(void) 
{
	WRITE_MEM32(MIB_CONTROL, ALL_COUNTER_RESTART_MASK);
#if 0	/* We don't want to read once first. */
	_rtl8651_initialRead();
#endif
	return SUCCESS;
}

int32 rtl8651_resetAsicMIBCounter(uint32 port, uint32 inCounterReset, uint32 outCounterReset ) 
{
	if(port==0xffffffff)
	{
		WRITE_MEM32(MIB_CONTROL, SYS_COUNTER_RESTART);
		return SUCCESS;
	}
	else
	{
		switch(port)
		{
			case 0:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P0);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P0);
				}
				break;
				
			case 1:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P1);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P1);
				}
				break;
			case 2:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P2);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P2);
				}
				break;
			case 3:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P3);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P3);
				}
				break;
			case 4:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P4);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P4);
				}
				break;
			case 5:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P5);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P5);
				}
				break;
			case 6:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P6);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P6);
				}
				break;
			case 7:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P7);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P7);
				}
				break;
				
			case 8:
				if(inCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, IN_COUNTER_RESTART_P8);
				}
				
				if(outCounterReset==TRUE)
				{
					WRITE_MEM32(MIB_CONTROL, OUT_COUNTER_RESTART_P8);
				}
				break;
		
			default:
				rtlglue_printf("wrong port number\n"); 
				return FAILED;
		}
		
		return SUCCESS;
		
	}
	
}

int32 rtl8651_resetAllAsicMIBCounter(void) 
{
	WRITE_MEM32(MIB_CONTROL, ALL_COUNTER_RESTART_MASK);
	return SUCCESS;
}

int32 rtl8651_getSimpleAsicMIBCounter(uint32 port, rtl865x_tblAsicDrv_simpleCounterParam_t * simpleCounter) 
{
	uint32 addrOffset_fromP0 =0;
	
	if((port>6)||(simpleCounter==NULL) )
	{
		return FAILED;
	}
	
	addrOffset_fromP0= port* MIB_ADDROFFSETBYPORT;
	
//	simpleCounter->rxBytes = rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 );
	simpleCounter->rxPkts = 	rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 );
//	simpleCounter->rxPausePkts=rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ) ;

//	simpleCounter->drops = rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 );
//	simpleCounter->txBytes =rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 );				
	simpleCounter->txPkts =rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 );
//	simpleCounter->txPausePkts=rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 );
	return SUCCESS;
}

int32 rtl8651_getAdvancedMIBCounter(uint32 port, uint32 asicCounterType, rtl865x_tblAsicDrv_advancedCounterParam_t * advancedCounter)
{
	uint32 addrOffset_fromP0 =0;
	
	if((port>6)||(advancedCounter==NULL) )
	{
		return FAILED;
	}
	
	addrOffset_fromP0= port* MIB_ADDROFFSETBYPORT;
	
	memset(advancedCounter,0,sizeof(rtl865x_tblAsicDrv_advancedCounterParam_t));

	if((asicCounterType & ASIC_IN_COUNTERS)!=0)
	{
		advancedCounter->ifInOctets=rtl865xC_returnAsicCounter64(OFFSET_IFINOCTETS_P0 + addrOffset_fromP0);
		advancedCounter->ifInUcastPkts=rtl8651_returnAsicCounter(OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0) ;
		advancedCounter->etherStatsOctets=rtl865xC_returnAsicCounter64(OFFSET_ETHERSTATSOCTETS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsUndersizePkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsFraments=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsPkts64Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsPkts65to127Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsPkts128to255Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsPkts256to511Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsPkts512to1023Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsPkts1024to1518Octets=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsOversizePkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsJabbers=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsMulticastPkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsBroadcastPkts=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0);
		advancedCounter->dot1dTpPortInDiscards=rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatusDropEvents=rtl8651_returnAsicCounter( OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0);
		advancedCounter->dot3FCSErrors=rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0);
		advancedCounter->dot3StatsSymbolErrors=rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0);
		advancedCounter->dot3ControlInUnknownOpcodes=rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0);
		advancedCounter->dot3InPauseFrames=rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0);
	}
		
	if((asicCounterType & ASIC_OUT_COUNTERS)!=0)
	{
		advancedCounter->ifOutOctets=rtl865xC_returnAsicCounter64(OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0);
		advancedCounter->ifOutUcastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0);
		advancedCounter->ifOutMulticastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0);
		advancedCounter->ifOutBroadcastPkts=rtl8651_returnAsicCounter(OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0);
		advancedCounter->ifOutDiscards=rtl8651_returnAsicCounter(OFFSET_IFOUTDISCARDS + addrOffset_fromP0);
		advancedCounter->dot3StatsSingleCollisionFrames=rtl8651_returnAsicCounter(OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0+ addrOffset_fromP0);
		advancedCounter->dot3StatsMultipleCollisionFrames=rtl8651_returnAsicCounter(OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0);
		advancedCounter->dot3StatsDefferedTransmissions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0);
		advancedCounter->dot3StatsLateCollisions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0);
		advancedCounter->dot3StatsExcessiveCollisions=rtl8651_returnAsicCounter(OFFSET_DOT3STATSEXCESSIVECOLLISIONS_P0 + addrOffset_fromP0);
		advancedCounter->dot3OutPauseFrames=rtl8651_returnAsicCounter(OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0);
		advancedCounter->dot1dBasePortDelayExceededDiscards=rtl8651_returnAsicCounter(OFFSET_DOT1DBASEPORTDELAYEXCEEDEDDISCARDS_P0 + addrOffset_fromP0);
		advancedCounter->etherStatsCollisions=rtl8651_returnAsicCounter(OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0);

	}

	if((asicCounterType & ASIC_WHOLE_SYSTEM_COUNTERS)!=0)
	{
		advancedCounter->dot1dTpLearnedEntryDiscards=rtl8651_returnAsicCounter(MIB_ADDROFFSETBYPORT);
		advancedCounter->etherStatsCpuEventPkts=rtl8651_returnAsicCounter(MIB_ADDROFFSETBYPORT);
	}
	
	return SUCCESS;
}

/*
@func int32 | rtl865xC_dumpAsicCounter | Dump common counters of all ports (CPU port included).
@rvalue SUCCESS | Finish showing the counters.
@comm
Dump common counters of all ports. Includes Rx/Tx Bytes, Rx/Tx pkts, Rx/Tx Pause frames, Rx Drops.
*/
int32 rtl865xC_dumpAsicCounter(void)
{
	uint32 i;

	for ( i = 0; i <= RTL8651_PORT_NUMBER; i++ )
	{
		uint32 addrOffset_fromP0 = i * MIB_ADDROFFSETBYPORT;

		if ( i == RTL8651_PORT_NUMBER )
			rtlglue_printf("<CPU port>\n");
		else
			rtlglue_printf("<Port: %d>\n", i);
		rtlglue_printf("Rx %llu Bytes, Rx %u Pkts, Drop %u pkts, Rx Pause %u pkts\n", 
			rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ) );
		rtlglue_printf("Tx %llu Bytes, Tx %u Pkts Tx Pause %u pkts\n", 
			rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ) + rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ) );
	}

	return SUCCESS;
}

/*
@func int32 | rtl865xC_dumpAsicDiagCounter | Dump complex counters of all ports (CPU port included).
@rvalue SUCCESS | Finish showing the counters.
@comm
Dump complex counters of all ports.
*/
#ifdef CONFIG_RTL_PROC_NEW
int32 rtl865xC_dumpAsicDiagCounter(struct seq_file *s)
{
	uint32 i;

	for ( i = 0; i <= RTL8651_PORT_NUMBER; i++ )
	{
		uint32 addrOffset_fromP0 = i * MIB_ADDROFFSETBYPORT;
		
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8197F)
		if (i == 5) continue;
#endif			

		if ( i == RTL8651_PORT_NUMBER )
			seq_printf(s,"<CPU port (extension port included)>\n");
		else
			seq_printf(s,"<Port: %d>\n", i);

		seq_printf(s,"Rx counters\n");
		seq_printf(s,"   Rcv %llu bytes, TpPortInDiscard %u, DropEvent %u", 
			rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter(OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ));
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || \
	defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		seq_printf(s,", QmDiscard  %u", 
			rtl8651_returnAsicCounter( OFFSET_QMDISCARDCNT_P0 + addrOffset_fromP0 ));
#endif			
		seq_printf(s,"\n   CRCAlignErr %u, SymbolErr %u, FragErr %u, JabberErr %u\n",
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
		seq_printf(s,"   Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
		seq_printf(s,"   < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
		seq_printf(s,"   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
		seq_printf(s,"   oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));
		
		seq_printf(s,"Output counters\n");
		seq_printf(s,"   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
			rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
		seq_printf(s,"   Broadcast %u pkts, Late collision %u, Deferred transmission %u\n",
			rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
		seq_printf(s,"   Collisions %u, Single collision %u, Multiple collision %u, pause %u\n",
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));
		
	}

	seq_printf(s,"<Whole system counters>\n");
	seq_printf(s,"   CpuEvent %u pkts\n", rtl8651_returnAsicCounter(OFFSET_ETHERSTATSCPUEVENTPKT));

	return SUCCESS;
}
#else
int32 rtl865xC_dumpAsicDiagCounter(char *page, int *ret_len)
{
	uint32 i;
    int len = *ret_len;

	for ( i = 0; i <= RTL8651_PORT_NUMBER; i++ )
	{
		uint32 addrOffset_fromP0 = i * MIB_ADDROFFSETBYPORT;
		
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8197F)
		if (i == 5) continue;
#endif			

		if ( i == RTL8651_PORT_NUMBER )
			len += sprintf(page+len,"<CPU port (extension port included)>\n");
		else
			len += sprintf(page+len,"<Port: %d>\n", i);

		len += sprintf(page+len,"Rx counters\n");
		len += sprintf(page+len,"   Rcv %llu bytes, TpPortInDiscard %u, DropEvent %u", 
			rtl865xC_returnAsicCounter64( OFFSET_IFINOCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT1DTPPORTINDISCARDS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter(OFFSET_ETHERSTATSDROPEVENTS_P0 + addrOffset_fromP0 ));
#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || \
	defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		len += sprintf(page+len,", QmDiscard  %u", 
			rtl8651_returnAsicCounter( OFFSET_QMDISCARDCNT_P0 + addrOffset_fromP0 ));
#endif			
		len += sprintf(page+len,"\n   CRCAlignErr %u, SymbolErr %u, FragErr %u, JabberErr %u\n",
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSFCSERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSSYMBOLERRORS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSFRAGMEMTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSJABBERS_P0 + addrOffset_fromP0 ));
		len += sprintf(page+len,"   Unicast %u pkts, Multicast %u pkts, Broadcast %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_IFINUCASTPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSMULTICASTPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSBROADCASTPKTS_P0 + addrOffset_fromP0 ));
		len += sprintf(page+len,"   < 64: %u pkts, 64: %u pkts, 65 -127: %u pkts, 128 -255: %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSUNDERSIZEPKTS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS64OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS65TO127OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS128TO255OCTETS_P0 + addrOffset_fromP0 ));
		len += sprintf(page+len,"   256 - 511: %u pkts, 512 - 1023: %u pkts, 1024 - 1518: %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS256TO511OCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS512TO1023OCTETS_P0 + addrOffset_fromP0), 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSPKTS1024TO1518OCTETS_P0 + addrOffset_fromP0 ) );
		len += sprintf(page+len,"   oversize: %u pkts, Control unknown %u pkts, Pause %u pkts\n", 
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSOVERSIZEPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3CONTROLINUNKNOWNOPCODES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3INPAUSEFRAMES_P0 + addrOffset_fromP0 ));
		
		len += sprintf(page+len,"Output counters\n");
		len += sprintf(page+len,"   Snd %llu bytes, Unicast %u pkts, Multicast %u pkts\n",
			rtl865xC_returnAsicCounter64( OFFSET_IFOUTOCTETS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_IFOUTUCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_IFOUTMULTICASTPKTS_P0 + addrOffset_fromP0 ));
		len += sprintf(page+len,"   Broadcast %u pkts, Late collision %u, Deferred transmission %u\n",
			rtl8651_returnAsicCounter( OFFSET_IFOUTBROADCASTPKTS_P0 + addrOffset_fromP0 ),
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSLATECOLLISIONS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSDEFERREDTRANSMISSIONS_P0 + addrOffset_fromP0 ));
		len += sprintf(page+len,"   Collisions %u, Single collision %u, Multiple collision %u, pause %u\n",
			rtl8651_returnAsicCounter( OFFSET_ETHERSTATSCOLLISIONS_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSSINGLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3STATSMULTIPLECOLLISIONFRAMES_P0 + addrOffset_fromP0 ), 
			rtl8651_returnAsicCounter( OFFSET_DOT3OUTPAUSEFRAMES_P0 + addrOffset_fromP0 ));
		
	}

	len += sprintf(page+len,"<Whole system counters>\n");
	len += sprintf(page+len,"   CpuEvent %u pkts\n", rtl8651_returnAsicCounter(OFFSET_ETHERSTATSCPUEVENTPKT));
    *ret_len = len;
	return SUCCESS;
}

#endif

/*
@func int32 | rtl8651_clearAsicCounter | Clear specified ASIC counter to zero
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the corresponding counter will be reset to zero and read the counter once
	for guarantee following read can get correct value. 
*/
int32 rtl8651_clearAsicSpecifiedCounter(uint32 counterIdx) {

	uint32 reg;
	rtlglue_printf("attention!this function is obsolete, please use new api:rtl8651_resetAsicMIBCounter()  or rtl8651_clearAsicCounter()\n");
	return FAILED;
	switch(counterIdx) {
		case 0:
		reg = READ_MEM32(MIB_CONTROL);
		WRITE_MEM32(MIB_CONTROL, SYS_COUNTER_RESTART);
		WRITE_MEM32(MIB_CONTROL, reg);
		break;
	default:	
		rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range

	}
	_rtl8651_initialRead();
	return SUCCESS;
}

/*
@func int32 | rtl8651_resetAsicCounterMemberPort | Clear the specified counter value and its member port
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the member port of the specified counter will be cleared to null set. 
*/
int32 rtl8651_resetAsicCounterMemberPort(uint32 counterIdx){

	rtlglue_printf("attention!this function is obsolete, please use new api:rtl8651_resetAsicMIBCounter()  or rtl8651_clearAsicCounter()\n");
	return FAILED;
	switch(counterIdx) {
		case 0:
		WRITE_MEM32(MIB_CONTROL, 0x0);
		break;
		default:	
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range

	}
	_rtl8651_initialRead();
	return SUCCESS;
}

/*
@func int32 | rtl8651_addAsicCounterMemberPort | The specified counter value add the specified port port into counter monitor member
@parm	uint32	|	counterIdx |  Specify the counter to add member port
@parm	uint32	|	port |  The added member port
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter and a valid port number, the specified port will be added to the counter coverage. 
*/
int32 rtl8651_addAsicCounterMemberPort(uint32 counterIdx, uint32 port) {
	uint32 reg, portMask;
	rtlglue_printf("attention!this function is obsolete, it shouldn't be used any more\n");
	return FAILED;

	
	if(port <RTL8651_PORT_NUMBER)
		portMask = 1<<(port + PORT_FOR_COUNTING_OFFSET);
	else if (port < RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
		portMask = 1<<(port - RTL8651_PORT_NUMBER + EXT_PORT_FOR_COUNTING_OFFSET);
	else
		return FAILED;//Port number out of range
	switch(counterIdx) {
		case 0:
		reg = READ_MEM32(MIB_CONTROL) & 0x3FE00000;
		WRITE_MEM32(MIB_CONTROL, reg|portMask);
		break;
		default:	
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range
	}
	return SUCCESS;
}


/*
@func int32 | rtl8651_resetAsicCounterMemberPort | Clear the specified counter value and its member port
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the corresponding counter will be reset to zero, its member port will be cleared to null set and read the counter once
	for guarantee following read can get correct value. 
*/
int32 rtl8651_delAsicCounterMemberPort(uint32 counterIdx, uint32 port) {
	uint32 reg, portMask;
	rtlglue_printf("attention!this function is obsolete, it shouldn't be used any more\n");
	return FAILED;
	if(port <RTL8651_PORT_NUMBER)
		portMask = 1<<(port + PORT_FOR_COUNTING_OFFSET);
	else if (port < RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
		portMask = 1<<(port - RTL8651_PORT_NUMBER + EXT_PORT_FOR_COUNTING_OFFSET);
	else
		return FAILED;//Port number out of range
	switch(counterIdx) {
		case 0:
		reg = READ_MEM32(MIB_CONTROL) & 0x3FE00000;
		WRITE_MEM32(MIB_CONTROL, reg&~portMask);
		break;
		default:	
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range
	}
	return SUCCESS;
}


/*
@func int32 | rtl8651_resetAsicCounterMemberPort | Clear the specified counter value and its member port
@parm	uint32	|	counterIdx |  Specify the counter to clear
@rvalue SUCCESS | When counter index is valid, return SUCCESS
@rvalue FAILED | When counter index is invalid, return FAILED
@comm
	When specify a vlid counter, the corresponding counter will be reset to zero, its member port will be cleared to null set and read the counter once
	for guarantee following read can get correct value. 
*/
int32 rtl8651_getAsicCounter(uint32 counterIdx, rtl865x_tblAsicDrv_basicCounterParam_t * basicCounter) {

	rtlglue_printf("attention!this function is obsolete, please use new api:rtl8651_getSimpleAsicMIBCounter()  or rtl8651_getAdvancedMIBCounter()\n");
	return FAILED;
	_rtl8651_initialRead();
	switch(counterIdx) {
		case 0:
				basicCounter->rxBytes = READ_MEM32(MIB_COUNTER_BASE);
				basicCounter->rxPackets = READ_MEM32(MIB_COUNTER_BASE+0x14) +	//Unicast
						READ_MEM32(MIB_COUNTER_BASE+0x18) + 	//Multicast
						READ_MEM32(MIB_COUNTER_BASE+0x1c);	//Broadcast
				basicCounter->rxErrors = READ_MEM32(MIB_COUNTER_BASE+0x8) +	//CRC error and Alignment error
						READ_MEM32(MIB_COUNTER_BASE+0xc) +	//Fragment error
						READ_MEM32(MIB_COUNTER_BASE+010);	//Jabber error
				basicCounter->drops = READ_MEM32(MIB_COUNTER_BASE+0x4);
				basicCounter->cpus = READ_MEM32(MIB_COUNTER_BASE+0x74);
				basicCounter->txBytes = READ_MEM32(MIB_COUNTER_BASE+0x48);
				basicCounter->txPackets = READ_MEM32(MIB_COUNTER_BASE+0x4c) +	//Unicast
						READ_MEM32(MIB_COUNTER_BASE+0x50) +	//Multicast
						READ_MEM32(MIB_COUNTER_BASE+0x54);	//Broadcast
				/*
				basicCounter->mbr = (READ_MEM32(MIB_CONTROL)&PORT_FOR_COUNTING_MASK)>>PORT_FOR_COUNTING_OFFSET | 
								((READ_MEM32(MIB_CONTROL)&EXT_PORT_FOR_COUNTING_MASK)>>EXT_PORT_FOR_COUNTING_OFFSET)<<6;
				*/
		break;
		default:
			rtlglue_printf("Not Comptable Counter Index  %d\n",counterIdx);
			return FAILED;//counter index out of range

	}
	return SUCCESS;
}

//sync from rtl865x kernel 2.4
void FullAndSemiReset( void )
{

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	REG32(SIRR) |= FULL_RST;
	mdelay(300);

	REG32(SYS_CLK_MAG) &= ~CM_ACTIVE_SWCORE;
	mdelay(300);
	
	REG32(SYS_CLK_MAG) |= CM_ACTIVE_SWCORE;
	mdelay(50);

#elif defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
	REG32(SIRR) |= FULL_RST;
	mdelay(300);

	REG32(SYS_CLK_MAG) |= CM_PROTECT;
	REG32(SYS_CLK_MAG) &= ~CM_ACTIVE_SWCORE;
	mdelay(300);
	
	REG32(SYS_CLK_MAG) |= CM_ACTIVE_SWCORE;
	REG32(SYS_CLK_MAG) &= ~CM_PROTECT;
	mdelay(50);
	
#elif defined(CONFIG_RTL8198_REVISION_B) 
 	if (REG32(BSP_REVR) >= BSP_RTL8198_REVISION_B)
	{
	  	REG32(SYS_CLK_MAG)&=(~(SYS_SW_RESET));
		mdelay(300);
		REG32(SYS_CLK_MAG)|=(SYS_SW_RESET);
		mdelay(50);
	}
	else {
		REG32(SIRR) |= FULL_RST;
		tick_Delay10ms(50);
	}
#else
	/* Perform full-reset for sw-core. */ 
	REG32(SIRR) |= FULL_RST;
	tick_Delay10ms(50);
#endif

	// 08-15-2012, set TRXRDY bit in rtl865x_start() in rtl865x_asicCom.c
	/* Enable TRXRDY */
	//REG32(SIRR) |= TRXRDY;
}

