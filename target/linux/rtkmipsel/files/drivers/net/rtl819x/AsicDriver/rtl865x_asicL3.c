/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table Layer3 route driver,following features are included:
*	Route/Multicast
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#include <net/rtl/rtl_types.h>
#include "asicRegs.h"
#include "rtl865x_asicCom.h"
#include "rtl865x_asicBasic.h"
#include "rtl865x_asicL3.h"
//#include "rtl_utils.h"
#include "rtl865x_hwPatch.h"



int32 rtl8651_setAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp) 
{
	rtl8651_tblAsic_extIpTable_t   entry;
	
	if((index >= RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL) || 
	((extIntIpp->localPublic == TRUE) && (extIntIpp->nat == TRUE))) //Local public IP and NAT property cannot co-exist
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.externalIP = extIntIpp->extIpAddr;
	entry.internalIP = extIntIpp->intIpAddr;
	entry.isLocalPublic = extIntIpp->localPublic==TRUE? 1: 0;
	entry.isOne2One = extIntIpp->nat==TRUE? 1: 0;
    	entry.nextHop = extIntIpp->nhIndex;
	entry.valid = 1;
	return _rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);

}

int32 rtl8651_delAsicExtIntIpTable(uint32 index) 
{
    	rtl8651_tblAsic_extIpTable_t   entry;

	if(index >= RTL8651_IPTABLE_SIZE)
		return FAILED;
        
	memset(&entry,0,sizeof(entry));
	entry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
}

int32 rtl8651_getAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp)
{
    	rtl8651_tblAsic_extIpTable_t   entry;
    
	if((index>=RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL))
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;//Entry not found
	extIntIpp->extIpAddr = ntohl(entry.externalIP);
	extIntIpp->intIpAddr = ntohl(entry.internalIP);
	extIntIpp->localPublic = entry.isLocalPublic==1? TRUE: FALSE;
	extIntIpp->nat = entry.isOne2One==1? TRUE: FALSE;
    	extIntIpp->nhIndex = entry.nextHop;
    	return SUCCESS;
}

int32 rtl8651_setAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) 
{
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL) || (pppoep->sessionId == 0xffff))
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.sessionID = pppoep->sessionId;
	entry.ageTime = pppoep->age;
	
	return _rtl8651_forceAddAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
}

int32 rtl8651_getAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) {
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
	pppoep->sessionId = entry.sessionID;
	pppoep->age = entry.ageTime;

	return SUCCESS;
}



int32 rtl8651_setAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp)
{
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index >= RTL8651_NEXTHOPTBL_SIZE ) || (nextHopp == NULL))
        return FAILED;

	/* for debug
	rtlglue_printf("[%s:%d] rtl8651_setAsicNextHopTable(idx=%d,Row=%d,Col=%d,PPPIdx=%d,dvid=%d,IPIdx=%d,type=%d)\n",
		__FILE__,__LINE__,index, nextHopp->nextHopRow,nextHopp->nextHopColumn,nextHopp->pppoeIdx,
		nextHopp->dvid,nextHopp->extIntIpIdx,nextHopp->isPppoe);
	*/
    memset(&entry,0,sizeof(entry));
    entry.nextHop = (nextHopp->nextHopRow <<2) | nextHopp->nextHopColumn;
    entry.PPPoEIndex = nextHopp->pppoeIdx;
    entry.dstVid = nextHopp->dvid;
    entry.IPIndex = nextHopp->extIntIpIdx;
    entry.type = nextHopp->isPppoe==TRUE? 1: 0;
    return _rtl8651_forceAddAsicEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
}

int32 rtl8651_getAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp) 
{
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index>=RTL8651_NEXTHOPTBL_SIZE) || (nextHopp == NULL))
        return FAILED;

    _rtl8651_readAsicEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
    nextHopp->nextHopRow = entry.nextHop>>2;
    nextHopp->nextHopColumn = entry.nextHop&0x3;
    nextHopp->pppoeIdx = entry.PPPoEIndex;
    nextHopp->dvid = entry.dstVid;
    nextHopp->extIntIpIdx = entry.IPIndex;
    nextHopp->isPppoe = entry.type==1? TRUE: FALSE;
    return SUCCESS;
}


int32 rtl8651_setAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) 
{
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
		extern int sixrd_hw_fw ;
#endif
#endif
	uint32 i, asicMask;
	rtl865xc_tblAsic_l3RouteTable_t entry;
#ifdef FPGA
	if (index==2) index=6;
	if (index==3) index=7;
	if (index>=4 && index <=5) 
		rtlglue_printf("Out of range\n");
#endif	
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	if (routingp->ipMask) {
		routingp->ipMask = routingp->ipMask;
		for(i=0; i<32; i++)
			if(routingp->ipMask & (1<<i)) break;
		asicMask = 31 - i;
	} else asicMask = 0;
    
	memset(&entry,0,sizeof(entry));
	entry.IPAddr = routingp->ipAddr;
	switch(routingp->process) {
	case 0://PPPoE
		entry.linkTo.PPPoEEntry.PPPoEIndex = routingp->pppoeIdx;
		entry.linkTo.PPPoEEntry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.PPPoEEntry.IPMask = asicMask;
		entry.linkTo.PPPoEEntry.netif = routingp->vidx;
		entry.linkTo.PPPoEEntry.internal=routingp->internal;
		entry.linkTo.PPPoEEntry.isDMZ=routingp->DMZFlag;
		entry.linkTo.PPPoEEntry.process = routingp->process;
		entry.linkTo.PPPoEEntry.valid = 1;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        entry.linkTo.PPPoEEntry.DSLEG      = routingp->DSLEG;
        entry.linkTo.PPPoEEntry.DSL_IDX1_0 = routingp->DSL_IDX&0x3;
#endif 
		break;
	case 1://Direct
		entry.linkTo.L2Entry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.L2Entry.IPMask = asicMask;
		entry.linkTo.L2Entry.netif = routingp->vidx;
		entry.linkTo.L2Entry.internal=routingp->internal;
		entry.linkTo.L2Entry.isDMZ=routingp->DMZFlag;
		entry.linkTo.L2Entry.process = routingp->process;
		entry.linkTo.L2Entry.valid = 1;		
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        entry.linkTo.L2Entry.DSLEG      = routingp->DSLEG;
        entry.linkTo.L2Entry.DSL_IDX1_0 = routingp->DSL_IDX&0x3;
#endif        
		break;
	case 2://arp
		entry.linkTo.ARPEntry.ARPEnd = routingp->arpEnd >> 3;
		entry.linkTo.ARPEntry.ARPStart = routingp->arpStart >> 3;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		entry.linkTo.ARPEntry.isDMZ = routingp->DMZFlag;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;

		entry.linkTo.ARPEntry.ARPIpIdx = routingp->arpIpIdx; /* for RTL8650B C Version Only */

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        entry.linkTo.ARPEntry.DSLEG      = routingp->DSLEG;
        entry.linkTo.ARPEntry.DSL_IDX1_0 = routingp->DSL_IDX&0x3;
#endif
		break;
	case 4://CPU
	case 6://DROP
		/*
		  *   Note:  although the process of this routing entry is CPU/DROP,
		  *             we still have to assign "vid" field for packet decision process use.
		  *                                                                                          - 2005.3.23 -
		  */
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		break;
	case 5://NAPT NextHop
		entry.linkTo.NxtHopEntry.nhStart = routingp->nhStart >> 1;
		switch (routingp->nhNum)
		{
		    case 2: entry.linkTo.NxtHopEntry.nhNum = 0; break;
		    case 4: entry.linkTo.NxtHopEntry.nhNum = 1; break;
		    case 8: entry.linkTo.NxtHopEntry.nhNum = 2; break;
		    case 16: entry.linkTo.NxtHopEntry.nhNum = 3; break;
		    case 32: entry.linkTo.NxtHopEntry.nhNum = 4; break;
		    default: return FAILED;
		}
		entry.linkTo.NxtHopEntry.nhNxt = routingp->nhNxt;
		entry.linkTo.NxtHopEntry.nhAlgo = routingp->nhAlgo;
		entry.linkTo.NxtHopEntry.IPMask = asicMask;
		entry.linkTo.NxtHopEntry.process = routingp->process;
		entry.linkTo.NxtHopEntry.valid = 1;
		entry.linkTo.NxtHopEntry.IPDomain = routingp->ipDomain;
		entry.linkTo.NxtHopEntry.internal = routingp->internal;
		entry.linkTo.NxtHopEntry.isDMZ = routingp->DMZFlag;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        entry.linkTo.NxtHopEntry.DSLEG      = routingp->DSLEG;
        entry.linkTo.NxtHopEntry.DSL_IDX1_0 = routingp->DSL_IDX&0x3;
#endif
		break;
		
	default: 
		return FAILED;
	}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        entry.DSL_IDX2   = routingp->DSL_IDX>>2;
#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_6RD_SUPPORT)
	if(sixrd_hw_fw)
	{
		if(entry.linkTo.ARPEntry.process  ==2){
			
			return FAILED;	
		}		
	}
#endif
#endif

    	return _rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}
int32 rtl8651_delAsicRouting(uint32 index) 
{
	rtl865xc_tblAsic_l3RouteTable_t entry;

	if(index >= RTL8651_ROUTINGTBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(entry));
	entry.linkTo.ARPEntry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}

int32 rtl8651_getAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) 
{
	uint32 i;
	rtl865xc_tblAsic_l3RouteTable_t entry;
    
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
	if(entry.linkTo.ARPEntry.valid == 0)
		return FAILED;

	routingp->ipAddr = ntohl(entry.IPAddr);
	routingp->process = entry.linkTo.ARPEntry.process;
	for(i=0, routingp->ipMask = 0; i<=entry.linkTo.ARPEntry.IPMask; i++)
		routingp->ipMask |= 1<<(31-i);
    
	routingp->ipMask = ntohl(routingp->ipMask);
    
	routingp->vidx = entry.linkTo.ARPEntry.netif;
	routingp->internal= entry.linkTo.PPPoEEntry.internal;
	switch(routingp->process) {
	case 0://PPPoE
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = entry.linkTo.PPPoEEntry.PPPoEIndex;
		routingp->nextHopRow = entry.linkTo.PPPoEEntry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.PPPoEEntry.nextHop & 0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        routingp->DSLEG      = entry.linkTo.PPPoEEntry.DSLEG;
        routingp->DSL_IDX   = entry.linkTo.PPPoEEntry.DSL_IDX1_0;
#endif
		break;
	case 1://Direct
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = entry.linkTo.L2Entry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.L2Entry.nextHop&0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        routingp->DSLEG      = entry.linkTo.L2Entry.DSLEG;
        routingp->DSL_IDX   = entry.linkTo.L2Entry.DSL_IDX1_0;
#endif
		break;
	case 2://Indirect
		routingp->arpEnd = entry.linkTo.ARPEntry.ARPEnd;
		routingp->arpStart = entry.linkTo.ARPEntry.ARPStart;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->arpIpIdx = entry.linkTo.ARPEntry.ARPIpIdx;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        routingp->DSLEG      = entry.linkTo.ARPEntry.DSLEG;
        routingp->DSL_IDX   = entry.linkTo.ARPEntry.DSL_IDX1_0;
#endif
		break;
	case 4: /* CPU */
	case 6: /* Drop */
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        routingp->DSLEG      = 0;
        routingp->DSL_IDX   = 0;
#endif
		break;

	case 5:
		routingp->nhStart = (entry.linkTo.NxtHopEntry.nhStart) << 1;
		switch (entry.linkTo.NxtHopEntry.nhNum)
		{
		case 0: routingp->nhNum = 2; break;
		case 1: routingp->nhNum = 4; break;
		case 2: routingp->nhNum = 8; break;
		case 3: routingp->nhNum = 16; break;
		case 4: routingp->nhNum = 32; break;
		default: return FAILED;
		}
		routingp->nhNxt = entry.linkTo.NxtHopEntry.nhNxt;
		routingp->nhAlgo = entry.linkTo.NxtHopEntry.nhAlgo;
		routingp->ipDomain = entry.linkTo.NxtHopEntry.IPDomain;
		routingp->internal= entry.linkTo.NxtHopEntry.internal;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        routingp->DSLEG      = entry.linkTo.NxtHopEntry.DSLEG;
        routingp->DSL_IDX   = entry.linkTo.NxtHopEntry.DSL_IDX1_0;
#endif		 
		break;

	default: return FAILED;
	}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        routingp->DSL_IDX    = routingp->DSL_IDX |(entry.DSL_IDX2<<0x2);
#endif

    return SUCCESS;
}


int32 rtl8651_setAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.nextHop = (arpp->nextHopRow<<2) | (arpp->nextHopColumn&0x3);
	entry.valid = 1;
	entry.aging=arpp->aging;
	return _rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_delAsicArp(uint32 index) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if(index >= RTL8651_ARPTBL_SIZE)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_getAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;
	//_rtl8651_readAsicEntry(TYPE_ARP_TABLE, index, &entry);
	/*for 8196B patch,read arp table and ip multicast table should stop TLU*/
	_rtl8651_readAsicEntryStopTLU(TYPE_ARP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;
	arpp->nextHopRow = entry.nextHop>>2;
	arpp->nextHopColumn = entry.nextHop&0x3;
	arpp->aging=entry.aging&0x1f;
	return SUCCESS;
}

inline unsigned int bitReverse(unsigned int x)
{
  unsigned int y = 0x55555555;
  x = (((x >> 1) & y) | ((x & y) << 1));
  y = 0x33333333;
  x = (((x >> 2) & y) | ((x & y) << 2));
  y = 0x0f0f0f0f;
  x = (((x >> 4) & y) | ((x & y) << 4));
  y = 0x00ff00ff;
  x = (((x >> 8) & y) | ((x & y) << 8));
  return((x >> 16) | (x << 16));
}

/*=========================================
  * ASIC DRIVER API: Multicast Table
  *=========================================*/
static uint32 currHashMethod = 0; /* init hash method as 0 */

uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr)
{
	uint32 idx;

#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8198C)	|| defined(CONFIG_RTL_8197F)
    currHashMethod = ( REG32(FFCR) & 0x00000060) >> 5; 
    
    if (currHashMethod == HASH_METHOD_SIP) 
    {
        idx = srcAddr & 0xff;
    }
    else if (currHashMethod == HASH_METHOD_DIP) 
    {
        idx = dstAddr & 0xff;
    }
    else
    {
        #if 0
    	uint32 sip[32],dip[32];
    	uint32 hash[8];
    	uint32 i;

    	for(i=0; i<8; i++) {
    		hash[i]=0;
    	}

    	for(i=0; i<32; i++)	{
    		if((srcAddr & (1<<i))!=0) {
    			sip[i]=1;
    		}
    		else 	{
    			sip[i]=0;
    		}

    		if((dstAddr & (1<<i))!=0) {
    			dip[i]=1;
    		}
    		else {
    			dip[i]=0;
    		}			
    	}

    	hash[7] = sip[0] ^ sip[8]   ^ sip[16] ^ sip[24] ^ dip[7] ^ dip[15] ^ dip[23] ^ dip[31];
    	hash[6] = sip[1] ^ sip[9]   ^ sip[17] ^ sip[25] ^ dip[6] ^ dip[14] ^ dip[22] ^ dip[30];
    	hash[5] = sip[2] ^ sip[10] ^ sip[18] ^ sip[26] ^ dip[5] ^ dip[13] ^ dip[21] ^ dip[29];
    	hash[4] = sip[3] ^ sip[11] ^ sip[19] ^ sip[27] ^ dip[4] ^ dip[12] ^ dip[20] ^ dip[28];
    	hash[3] = sip[4] ^ sip[12] ^ sip[20] ^ sip[28] ^ dip[3] ^ dip[11] ^ dip[19] ^ dip[27];
    	hash[2] = sip[5] ^ sip[13] ^ sip[21] ^ sip[29] ^ dip[2] ^ dip[10] ^ dip[18] ^ dip[26];
    	hash[1] = sip[6] ^ sip[14] ^ sip[22] ^ sip[30] ^ dip[1] ^ dip[9]  ^ dip[17] ^ dip[25];
    	hash[0] = sip[7] ^ sip[15] ^ sip[23] ^ sip[31] ^ dip[0] ^ dip[8]  ^ dip[16] ^ dip[24];

    	for(i=0; i<8; i++) {
    		hash[i]=hash[i] & (0x01);
    	}

    	idx=0;
    	for(i=0; i<8; i++) {
    		idx=idx+(hash[i]<<i);
    	}
        #else
        uint32 temp;
        uint8 *tempPtr;
        uint32 newSrcAddr = bitReverse(srcAddr);
        
        temp = newSrcAddr^dstAddr;
        tempPtr = (uint8 *)(&temp);
        idx = (uint32)(tempPtr[0]^tempPtr[1]^tempPtr[2]^tempPtr[3]);
        #endif
        
    }
	
#elif defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD)
    currHashMethod = ( REG32(FFCR) & 0x00000060) >> 5; /* Frame Forwarding Configuration Register bit:5~6 */
    
    if (currHashMethod == HASH_METHOD_SIP) /* hash method:  SIP[6:0] */
    {
        idx = srcAddr & 0x7f;
    }
    else if (currHashMethod == HASH_METHOD_DIP) /* hash method:  DIP[6:0] */
    {
        idx = dstAddr & 0x7f;
    }
    else
    {
        #if 0
        uint32 sip[32],dip[32];
    	uint32 hash[7];
    	uint32 i;

    	for(i=0; i<7; i++) {
    		hash[i]=0;
    	}

    	for(i=0; i<32; i++)	{
    		if((srcAddr & (1<<i))!=0) {
    			sip[i]=1;
    		}
    		else 	{
    			sip[i]=0;
    		}

    		if((dstAddr & (1<<i))!=0) {
    			dip[i]=1;
    		}
    		else {
    			dip[i]=0;
    		}			
    	}

    	hash[0] = sip[0] ^ sip[7]   ^ sip[14] ^ sip[21] ^ sip[28] ^ dip[1] ^ dip[8]   ^ dip[15] ^ dip[22] ^ dip[29];
    	hash[1] = sip[1] ^ sip[8]   ^ sip[15] ^ sip[22] ^ sip[29] ^ dip[2] ^ dip[9]   ^ dip[16] ^ dip[23] ^ dip[30];
    	hash[2] = sip[2] ^ sip[9]   ^ sip[16] ^ sip[23] ^ sip[30] ^ dip[3] ^ dip[10] ^ dip[17] ^ dip[24] ^ dip[31];
    	hash[3] = sip[3] ^ sip[10] ^ sip[17] ^ sip[24] ^ sip[31] ^ dip[4] ^ dip[11] ^ dip[18] ^ dip[25];
    	hash[4] = sip[4] ^ sip[11] ^ sip[18] ^ sip[25]               ^ dip[5] ^ dip[12] ^ dip[19] ^ dip[26];
    	hash[5] = sip[5] ^ sip[12] ^ sip[19] ^ sip[26]               ^ dip[6] ^ dip[13] ^ dip[20] ^ dip[27];
    	hash[6] = sip[6] ^ sip[13] ^ sip[20] ^ sip[27]   ^ dip[0] ^ dip[7] ^ dip[14] ^ dip[21] ^ dip[28];

    	for(i=0; i<7; i++) {
    		hash[i]=hash[i] & (0x01);
    	}

    	idx=0;
    	for(i=0; i<7; i++) {
    		idx=idx+(hash[i]<<i);
    	}
        #else
        uint32 hashSrcAddr = 0;
        uint32 hashDstAddr = 0;
        
        hashSrcAddr = srcAddr ^ (srcAddr>>7) ^ (srcAddr>>14) ^ (srcAddr>>21) ^ ((srcAddr>>28)&0xF);
        hashDstAddr = ((dstAddr>>1) ^ (dstAddr>>8) ^ (dstAddr>>15) ^ (dstAddr>>22)) ^ ((dstAddr>>29)&0x7)^((dstAddr&0x1)<<6);
        idx= (hashSrcAddr ^ hashDstAddr) & 0x7F;
        #endif
    }
    
	return idx;
#else
	#error "wrong compile flag, not supported IC reversion"	
#endif

	return idx;
}

#ifdef CONFIG_RTL_SWITCH_FPGA
void set_mc_entry_for_test(void)
{
	rtl865x_tblAsicDrv_multiCastParam_t mCast;

	mCast.sip = htonl(0xAC010163);	// VLC server is in WAN (port 0), IP: 172.1.1.99, MC IP: 224.1.2.3
	mCast.dip = htonl(0xE0010203);	// VLC client is in LAN (port 1), join 224.1.2.3
	mCast.svid = 0;
	mCast.port = 0;	// WAN (port 0)
	mCast.mbr = 0x2; // LAN (port 1)
	mCast.extIdx = 0;	
	rtl8651_setAsicIpMulticastTable(&mCast);
}
#endif

int32 rtl8651_setAsicIpMulticastTable(rtl865x_tblAsicDrv_multiCastParam_t *mCast_t) {
	uint32 idx;
 	rtl865xc_tblAsic_ipMulticastTable_t entry;
	//int16 age;

	if(mCast_t->dip >>28 != 0xe || mCast_t->port >= RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
		return FAILED;//Non-IP multicast destination address
	memset(&entry,0,sizeof(entry));
	entry.srcIPAddr 		= mCast_t->sip;
	entry.destIPAddrLsbs 	= mCast_t->dip & 0xfffffff;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	idx = mCast_t->idx;
#else
	idx = rtl8651_ipMulticastTableIndex(mCast_t->sip, mCast_t->dip);
#endif

	entry.srcPort 			= mCast_t->port;
	entry.portList 			= mCast_t->mbr;
#ifdef CONFIG_RTL_HW_MCAST_WIFI
 	entry.toCPU      = (entry.portList & ( 0x01<<RTL8651_MAC_NUMBER)) ? 1 : 0;
 	entry.portList     = entry.portList &(~( 0x01<<RTL8651_MAC_NUMBER));
#else
	entry.toCPU 			= 0;
#endif
	entry.valid 			= 1;
	entry.extIPIndex 		= mCast_t->extIdx;	
#if 1
	entry.ageTime			= 7;	
#else
	entry.ageTime			= 0;	
	age = (int16)mCast_t->age;
	while ( age > 0 ) {
		if ( (++entry.ageTime) == 7)
			break;
		age -= 5;
	}
#endif
#if defined (CONFIG_RTL_8197F)
	entry.destInterface = 0;
#endif
	
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
}

int32 rtl8651_delAsicIpMulticastTable(uint32 index) {
	rtl865xc_tblAsic_ipMulticastTable_t entry;
	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);
}

int32 rtl8651_getAsicIpMulticastTable(uint32 index, rtl865x_tblAsicDrv_multiCastParam_t *mCast_t) 
{	
	rtl865xc_tblAsic_ipMulticastTable_t entry;
	
	if (mCast_t == NULL)
		return FAILED;
   	//_rtl8651_readAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);
	_rtl8651_readAsicEntryStopTLU(TYPE_MULTICAST_TABLE, index, &entry);

	mCast_t->sip = entry.srcIPAddr;
	if(entry.valid)
	{
 		mCast_t->dip	= entry.destIPAddrLsbs | 0xe0000000;
	}
	else
	{
		if(entry.destIPAddrLsbs==0)
		{
			mCast_t->dip	= entry.destIPAddrLsbs;
		}
		else
		{
			mCast_t->dip	= entry.destIPAddrLsbs | 0xe0000000;
		}
	}
	mCast_t->dip = mCast_t->dip;
	mCast_t->svid = 0;
	mCast_t->port = entry.srcPort;
	mCast_t->mbr = entry.portList;
	mCast_t->extIdx = entry.extIPIndex ;
	mCast_t->age	= entry.ageTime * 5;
	mCast_t->cpu = entry.toCPU;
	
	if(entry.valid == 0)
	{
		return FAILED;
	}
	return SUCCESS;	
}


/*
@func int32		|	rtl8651_setAsicMulticastEnable 	| Enable / disable ASIC IP multicast support.
@parm uint32		|	enable	| TRUE to indicate ASIC IP multicast process is enabled; FALSE to indicate ASIC IP multicast process is disabled.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
We would use this API to enable/disable ASIC IP multicast process.
If it's disabled here, Hardware IP multicast table would be ignored.
If it's enabled here, IP multicast table is used to forwarding IP multicast packets.
 */
int32 rtl8651_setAsicMulticastEnable(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(FFCR, READ_MEM32(FFCR)|EN_MCAST);
	} else
	{
		WRITE_MEM32(FFCR, READ_MEM32(FFCR) & ~EN_MCAST);
	}

	return SUCCESS;
}
/*
@func int32		|	rtl8651_getAsicMulticastEnable 	| Get the state about ASIC IP multicast support.
@parm uint32*		|	enable	| Pointer to store the state about ASIC IP multicast support.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
We would use this API to get the status of ASIC IP multicast process.
TRUE to indicate ASIC IP multicast process is enabled; FALSE to indicate ASIC IP multicast process is disabled.
*/
int32 rtl8651_getAsicMulticastEnable(uint32 *enable)
{
	if (enable == NULL)
	{
		return FAILED;
	}

	*enable = (READ_MEM32(FFCR) & EN_MCAST) ? TRUE : FALSE;

	return SUCCESS;
}

int32 rtl865x_setAsicMulticastVlanRemark(int8 port, uint8 mode, int32 vid)
{
	uint32 regValue, reg;
	if ((port < 0) || (port > 4) || (vid < 0) || (vid > 4096))
		return FAILED;

	reg = V4VLDSCPCR0+4*port;
	
	regValue = READ_MEM32(reg)&(~((VidRemarkModeMask<<VidRemarkModeOffset)|VidRemarkValueMask));
	switch(mode)
	{
		case VID_REMARK:
		case VID_ADDTAG:
			regValue |= vid;
		case VID_REMOVE:
		case VID_BYPASS:
			regValue |= (mode<<VidRemarkModeOffset);
			break;
		default:
			break;
	}
	WRITE_MEM32(reg,regValue);

	return SUCCESS;
}
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)

int32 rtl865x_set_mcastMacClone(uint32 enabled, unsigned char * macAddr)
{
	uint32 value0_31=0,value32_47=0;
	
	if(enabled)
	{
		WRITE_MEM32(FFCR,((READ_MEM32(FFCR)&(~CF_IPMMAC_CLONE_EN))|CF_IPMMAC_CLONE_EN));
		if(macAddr)
		{
			value32_47 = (macAddr[0]<<8)|(macAddr[1]);
			value0_31 = (macAddr[2]<<24)|(macAddr[3]<<16)|(macAddr[4]<<8)|(macAddr[5]);
			WRITE_MEM32(IPMCMCR0,value0_31);
			WRITE_MEM32(IPMCMCR1,value32_47);
		}
	}
	else
	{
		WRITE_MEM32(FFCR,((READ_MEM32(FFCR)&(~CF_IPMMAC_CLONE_EN))));
	}
	return SUCCESS;
}

#endif
/*
@func int32		|	rtl8651_setAsicMulticastPortInternal 	| Configure internal/external state for each port
@parm uint32		|	port		| Port to set its state.
@parm int8		|	isInternal	| set TRUE to indicate <p port> is internal port; set FALSE to indicate <p port> is external port
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
In RTL865x platform,
user would need to config the Internal/External state for each port to support HW multicast NAPT.
If packet is sent from internal port to external port, and source VLAN member port checking indicates that L34 is needed.
Source IP modification would be applied.
 */
int32 rtl8651_setAsicMulticastPortInternal(uint32 port, int8 isInternal)
{
	if (port >= RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum)
	{	/* Invalid port */
		return FAILED;
	}

	/*
		RTL865XC : All multicast mode are stored in [SWTCR0 / Switch Table Control Register 0]
	*/
	if (isInternal == TRUE)
	{
		WRITE_MEM32(SWTCR0, READ_MEM32(SWTCR0) | (((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET));
	} else
	{
		WRITE_MEM32(SWTCR0, READ_MEM32(SWTCR0) & ~(((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET));
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_getAsicMulticastPortInternal 	| Get internal/external state for each port
@parm uint32		|	port		| Port to set its state.
@parm int8*		|	isInternal	| Pointer to get port state of <p port>.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To get the port internal / external state for <p port>:
TRUE to indicate <p port> is internal port; FALSE to indicate <p port> is external port
 */
int32 rtl8651_getAsicMulticastPortInternal(uint32 port, int8 *isInternal)
{
	if (isInternal == NULL)
	{
		return FAILED;
	}

	if (port >= RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum)
	{	/* Invalid port */
		return FAILED;
	}

	if (READ_MEM32(SWTCR0) & (((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET))
	{
		*isInternal = TRUE;
	} else
	{
		*isInternal = FALSE;
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastMTU 	| Set MTU for ASIC IP multicast forwarding
@parm uint32		|	mcastMTU	| MTU used by HW IP multicast forwarding.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To set the MTU for ASIC IP multicast forwarding.
Its independent from other packet forwarding because IP multicast would include L2/L3/L4 at one time.
*/
int32 rtl8651_setAsicMulticastMTU(uint32 mcastMTU)
{
	if (mcastMTU & ~(MultiCastMTU_MASK) )
	{	/* multicast MTU overflow */
		return FAILED;
	}

	UPDATE_MEM32(ALECR, mcastMTU, MultiCastMTU_MASK, MultiCastMTU_OFFSET);

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastMTU 	| Get MTU for ASIC IP multicast forwarding
@parm uint32*	|	mcastMTU	| Pointer to get MTU used by HW IP multicast forwarding.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To get the MTU value for ASIC IP multicast forwarding.
Its independent from other packet forwarding because IP multicast would include L2/L3/L4 at one time.
*/
int32 rtl8651_getAsicMulticastMTU(uint32 *mcastMTU)
{
	if (mcastMTU == NULL)
	{
		return FAILED;
	}

	*mcastMTU = GET_MEM32_VAL(ALECR, MultiCastMTU_MASK, MultiCastMTU_OFFSET);

	return SUCCESS;
}

int32 rtl865x_setAsicMulticastAging(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR) & ~IPMcastAgingDisable);
		
	} else
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR)|IPMcastAgingDisable);
	}

	return SUCCESS;
}
int32 rtl865x_getAsicMCastHashMethod(unsigned int *hashMethod)
{
	if(hashMethod==NULL)
	{
		return FAILED;
	}
	*hashMethod = (READ_MEM32(FFCR) & 0x60)>>5;

	return SUCCESS;
}

int32 rtl865x_setAsicMCastHashMethod(unsigned int hashMethod)
{
    uint32 	  oldHashMethod = 0;
	uint32 	  currHashMethod = 0;
 	oldHashMethod = (READ_MEM32(FFCR) & 0x60)>>5;
    currHashMethod = hashMethod; 
	if(currHashMethod >3)
	{
		return FAILED;
	}
	currHashMethod=currHashMethod&0x3;				
    if (oldHashMethod != currHashMethod) /* set FFCR Register bit 5~6 */
    {
    	WRITE_MEM32(FFCR,(READ_MEM32(FFCR) & 0xFFFFFF9F)|(currHashMethod << 5));
    }
    return SUCCESS;
}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) //add for 8198C ipv6 (+)

int32 rtl8198C_setAsicMulticastv6Enable(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(IPV6CR0, READ_MEM32(IPV6CR0)|EN_MCASTv6);
	} 
	else
	{
		WRITE_MEM32(IPV6CR0, READ_MEM32(IPV6CR0) & ~EN_MCASTv6);
	}
	return SUCCESS;
}
int32 rtl8198C_getAsicMulticastv6Enable(uint32 *enable)
{
	if (enable == NULL)
	{
		return FAILED;
	}

	*enable = (READ_MEM32(IPV6CR0) & EN_MCASTv6) ? TRUE : FALSE;

	return SUCCESS;
}
int32 rtl8198C_setAsicMulticastv6MTU(uint32 mcastMTU)
{
	if (mcastMTU & ~(MultiCastv6MTU_MASK) )
	{	/* multicast MTU overflow */
		return FAILED;
	}

	UPDATE_MEM32(IPV6CR0, mcastMTU, MultiCastv6MTU_MASK, MultiCastv6MTU_OFFSET);

	return SUCCESS;
}
int32 rtl8198C_getAsicMulticastv6MTU(uint32 *mcastMTU)
{
	if (mcastMTU == NULL)
	{
		return FAILED;
	}

	*mcastMTU = GET_MEM32_VAL(IPV6CR0, MultiCastv6MTU_MASK, MultiCastv6MTU_OFFSET);

	return SUCCESS;
}
int32 rtl8198C_getAsicMCastv6HashMethod(unsigned int *hashMethod)
{
	if(hashMethod==NULL)
	{
		return FAILED;
	}
	*hashMethod = (READ_MEM32(IPV6CR0) & 0x60000)>>17;
	return SUCCESS;
}
int32 rtl8198C_setAsicMCastv6HashMethod(unsigned int hashMethod)//to be checked
{
	uint32	  oldHashMethod = 0;
	oldHashMethod = (READ_MEM32(IPV6CR0) & 0x60000)>>17;
	currHashMethod = hashMethod; 
	if(hashMethod >3)
	{
		return FAILED;
	}
			
	hashMethod=hashMethod&0x3;
					
	if (oldHashMethod != hashMethod) /* set IPV6CR Register bit 17~18 and flush multicastv6 table */
	{
		WRITE_MEM32(IPV6CR0,(READ_MEM32(IPV6CR0) & 0xFFF9FFFF)|(currHashMethod << 17));
	}
	return SUCCESS;
}
int32 rtl8198C_setAsicMulticastv6Aging(uint32 enable)
{
	if (enable == TRUE)
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR) & ~IPv6McastAgingDisable);
		
	} else
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR)|IPv6McastAgingDisable);
	}

	return SUCCESS;
}


///////////////
//6rd
///////////////
int32 rtl8198C_setAsic6rdTable(uint32 index, rtl8198C_tblAsicDrv_6rdParam_t *six_rd_entry)
{
	uint32 asicMask=0;
    rtl8198C_tblAsic_6rdTable_t  entry;

	if((index >= RTL8198C_6RDTBL_SIZE ) || (six_rd_entry == NULL))
        return FAILED;

    if((six_rd_entry->six_rd_prefix_len==0)||(six_rd_entry->br_ip_mask_len==0))
    {
        return FAILED;        
    }

	/* for debug
	rtlglue_printf("[%s:%d] rtl8198C_setAsic6rdTable(idx=%d,Row=%d,Col=%d,PPPIdx=%d,dvid=%d,IPIdx=%d,type=%d)\n",
		__FILE__,__LINE__,index, nextHopp->nextHopRow,nextHopp->nextHopColumn,nextHopp->pppoeIdx,
		nextHopp->dvid,nextHopp->extIntIpIdx,nextHopp->isPppoe);
	*/
    memset(&entry,0,sizeof(entry));
	entry.ce_ip_addr = six_rd_entry->ce_ip_addr;

#if 0   
    for(i=0; i<32; i++)
        if(six_rd_entry->ce_ip_mask & (1<<i)) break;
    asicMask = i;
    printk("six_rd_entry->ce_ip_mask:0x%x i:%d asicMask:%d\n",six_rd_entry->ce_ip_mask,i,asicMask);//leroy	



	if (1/*six_rd_entry->ce_ip_mask*/) {
		for(i=0; i<32; i++)
			if(six_rd_entry->ce_ip_mask & (1<<i)) break;
		///leroy////asicMask = 31 - i;
		if(i!=0)
		    asicMask = i-1;
        else
            asicMask = 0;//31
    	printk("six_rd_entry->ce_ip_mask:0x%x i:%d asicMask:%d\n",six_rd_entry->ce_ip_mask,i,asicMask);//leroy	
	} else asicMask = 0;
#endif
	entry.ce_ip_mask      = 32-six_rd_entry->ce_ip_mask_len;
	entry.six_rd_pfx25_0  = six_rd_entry->six_rd_prefix.v6_addr32[1]
						    &0x3FFFFFF;
	entry.six_rd_pfx57_26 =(  (six_rd_entry->six_rd_prefix.v6_addr32[0]<<6)| 
		                      (six_rd_entry->six_rd_prefix.v6_addr32[1]>>26) );
	entry.six_rd_pfx63_58 = (six_rd_entry->six_rd_prefix.v6_addr32[0]>>26);

	if (six_rd_entry->six_rd_prefix_len){
		asicMask=six_rd_entry->six_rd_prefix_len-1;
	}else{
		asicMask=0;
	}
	entry.pfx_mask        = asicMask ;
	entry.br_ip_addr19_0  = six_rd_entry->br_ip_addr&0xFFFFF;
	entry.br_ip_addr31_20 = six_rd_entry->br_ip_addr>>20;

#if 0 
	if (1/*six_rd_entry->br_ip_mask*/) {
		for(i=0; i<32; i++)
			if(six_rd_entry->br_ip_mask & (1<<i)) break;
		///leroy////asicMask = 31 - i;
		if(i!=0)
		    asicMask = i-1;
        else
            asicMask = 0;//31
       	printk("six_rd_entry->br_ip_mask:0x%x i:%d asicMask:%d\n",six_rd_entry->br_ip_mask,i,asicMask);//leroy	  
	} else asicMask = 0;
	entry.br_ip_mask = asicMask ;
#endif

	if (six_rd_entry->br_ip_mask_len){
		asicMask=six_rd_entry->br_ip_mask_len-1;
	}else{
		asicMask=0;
	}
    
	entry.br_ip_mask = asicMask ;
	entry.six_rd_mtu = six_rd_entry->mtu;
	entry.valid      = six_rd_entry->valid;
	return _rtl8651_forceAddAsicEntry(TYPE_6RD_TABLE, index, &entry);
}

int32 rtl8198C_delAsic6rdTable(uint32 index) 
{
	rtl8198C_tblAsic_6rdTable_t   entry;
	if(index >= RTL8198C_6RDTBL_SIZE)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_6RD_TABLE, index, &entry);
}

int32 rtl8198C_getAsic6rdTable(uint32 index, rtl8198C_tblAsicDrv_6rdParam_t *six_rd_entry) 
{
    rtl8198C_tblAsic_6rdTable_t  entry;
    if((index>=RTL8198C_6RDTBL_SIZE) || (six_rd_entry == NULL))
        return FAILED;

	memset(&entry,0,sizeof(entry));   //leroy todo remove this
    _rtl8651_readAsicEntry(TYPE_6RD_TABLE, index, &entry);

	if(entry.valid == 0)
        return FAILED;
	

    six_rd_entry->ce_ip_addr = entry.ce_ip_addr;

    ////six_rd_entry->ce_ip_mask_len = entry.ce_ip_mask+1;
    six_rd_entry->ce_ip_mask_len = 32-entry.ce_ip_mask;
	////for(i=0, six_rd_entry->ce_ip_mask = 0; i<entry.ce_ip_mask; i++)
	////	six_rd_entry->ce_ip_mask |= 1<<(i);

    six_rd_entry->six_rd_prefix.v6_addr32[1]= entry.six_rd_pfx25_0 
		                                    |((entry.six_rd_pfx57_26 &0x3F)<<26) ;
    six_rd_entry->six_rd_prefix.v6_addr32[0]= ((entry.six_rd_pfx57_26 &(~0x3F))>>6) 
                                            |((entry.six_rd_pfx63_58 &0x3F)<<26) ;   
    six_rd_entry->six_rd_prefix_len=entry.pfx_mask+1;
    six_rd_entry->br_ip_addr = entry.br_ip_addr19_0 | (entry.br_ip_addr31_20<<20);	
    six_rd_entry->mtu        = entry.six_rd_mtu;
    six_rd_entry->valid      = entry.valid;

	//for(i=0, six_rd_entry->br_ip_mask = 0; i<=entry.br_ip_mask; i++)
	//	six_rd_entry->br_ip_mask |= 1<<(i);
    six_rd_entry->br_ip_mask_len = entry.br_ip_mask+1;

    return SUCCESS;
}

///////////////
//dslite
///////////////
int32 rtl8198C_setAsicDsliteTable(uint32 index, rtl8198C_tblAsicDrv_dsliteParam_t *dslite_entry)
{
	uint32 asicMask;
    rtl8198C_tblAsic_dsliteTable_t  entry;

	if((index >= RTL8198C_DSLITETBL_SIZE ) || (dslite_entry == NULL))
	{
        return FAILED;
	}
	/* for debug
	rtlglue_printf("[%s:%d] rtl8651_setAsic6rdTable(idx=%d,Row=%d,Col=%d,PPPIdx=%d,dvid=%d,IPIdx=%d,type=%d)\n",
		__FILE__,__LINE__,index, nextHopp->nextHopRow,nextHopp->nextHopColumn,nextHopp->pppoeIdx,
		nextHopp->dvid,nextHopp->extIntIpIdx,nextHopp->isPppoe);
	*/
    memset(&entry,0,sizeof(entry));
	entry.host_ipv6_addr31_0   = dslite_entry->host_ipv6_addr.v6_addr32[3];
	entry.host_ipv6_addr63_32  = dslite_entry->host_ipv6_addr.v6_addr32[2];
	entry.host_ipv6_addr95_64  = dslite_entry->host_ipv6_addr.v6_addr32[1];
	entry.host_ipv6_addr127_96 = dslite_entry->host_ipv6_addr.v6_addr32[0];

	if (dslite_entry->host_ipv6_mask) {
		asicMask = dslite_entry->host_ipv6_mask-1;
	} else{
		asicMask = 0;
	}	
	entry.host_ipv6_mask = asicMask;

	entry.aftr_ipv6_addr24_0    =     dslite_entry->aftr_ipv6_addr.v6_addr32[3]&0x1FFFFFF;	
	entry.aftr_ipv6_addr56_25   = (  (dslite_entry->aftr_ipv6_addr.v6_addr32[2]<<7)| 
		                             (dslite_entry->aftr_ipv6_addr.v6_addr32[3]>>25) );	
	entry.aftr_ipv6_addr88_57   = (  (dslite_entry->aftr_ipv6_addr.v6_addr32[1]<<7)|
		                             (dslite_entry->aftr_ipv6_addr.v6_addr32[2]>>25) );	
	entry.aftr_ipv6_addr120_89  = (  (dslite_entry->aftr_ipv6_addr.v6_addr32[0]<<7)|
		                             (dslite_entry->aftr_ipv6_addr.v6_addr32[1]>>25) );	
	entry.aftr_ipv6_addr127_121 = (   dslite_entry->aftr_ipv6_addr.v6_addr32[0]>>25);

	if (dslite_entry->aftr_ipv6_mask){
		asicMask = dslite_entry->aftr_ipv6_mask-1;
	}else{
		asicMask = 0;
	}
	entry.aftr_ipv6_mask = asicMask ;
	entry.dslite_mtu     = dslite_entry->mtu;
	entry.valid          = dslite_entry->valid;
	return _rtl8651_forceAddAsicEntry(TYPE_DS_LITE_TABLE, index, &entry);	
}

int32 rtl8198C_delAsicDsliteTable(uint32 index) 
{
	rtl8198C_tblAsic_dsliteTable_t   entry;
	if(index >= RTL8198C_DSLITETBL_SIZE)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_DS_LITE_TABLE, index, &entry);
}

int32 rtl8198C_getAsicDsliteTable(uint32 index, rtl8198C_tblAsicDrv_dsliteParam_t *dslite_entry) 
{
    rtl8198C_tblAsic_dsliteTable_t  entry;
    if((index>=RTL8198C_DSLITETBL_SIZE) || (dslite_entry == NULL))
        return FAILED;
	
	memset(&entry,0,sizeof(entry));

    _rtl8651_readAsicEntry(TYPE_DS_LITE_TABLE, index, &entry);
	if(entry.valid == 0)
        return FAILED;
	
    dslite_entry->host_ipv6_addr.v6_addr32[3]= entry.host_ipv6_addr31_0;
    dslite_entry->host_ipv6_addr.v6_addr32[2]= entry.host_ipv6_addr63_32;
    dslite_entry->host_ipv6_addr.v6_addr32[1]= entry.host_ipv6_addr95_64;
    dslite_entry->host_ipv6_addr.v6_addr32[0]= entry.host_ipv6_addr127_96;

    dslite_entry->host_ipv6_mask=entry.host_ipv6_mask+1;
    dslite_entry->aftr_ipv6_addr.v6_addr32[3]= entry.aftr_ipv6_addr24_0 |
	 											(entry.aftr_ipv6_addr56_25<<25)	;
    	
    dslite_entry->aftr_ipv6_addr.v6_addr32[2]= (entry.aftr_ipv6_addr88_57<<25)|
												(entry.aftr_ipv6_addr56_25>>7)	;
	 
    dslite_entry->aftr_ipv6_addr.v6_addr32[1]= (entry.aftr_ipv6_addr120_89<<25)|
												(entry.aftr_ipv6_addr88_57>>7)	;

    dslite_entry->aftr_ipv6_addr.v6_addr32[0]= (entry.aftr_ipv6_addr127_121<<25)|
												(entry.aftr_ipv6_addr120_89>>7)	;

    dslite_entry->aftr_ipv6_mask=entry.aftr_ipv6_mask+1;
    dslite_entry->mtu   = entry.dslite_mtu;
    dslite_entry->valid = entry.valid;
    return SUCCESS;
}

int32 rtl8198C_setAsicRoutingv6(uint32 index, rtl8198C_tblAsicDrv_routingv6Param_t *routingp) 
{
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	extern int dslite_hw_fw ;
#endif

	uint32 asicMask;
	rtl8198C_tblAsic_l3v6RouteTable_t entry;

/* 
#ifdef FPGA
	if (index==2) index=6;
	if (index==3) index=7;
	if (index>=4 && index <=5) 
		rtlglue_printf("Out of range\n");
#endif	
*/
	if((index >= RTL8198C_ROUTINGV6TBL_SIZE) || (routingp == NULL))
		return FAILED;

    memset(&entry,0,sizeof(entry));
	entry.ipv6_addr31_0   = routingp->ipAddr.v6_addr32[3];
	entry.ipv6_addr63_32  = routingp->ipAddr.v6_addr32[2];
	entry.ipv6_addr95_64  = routingp->ipAddr.v6_addr32[1];
	entry.ipv6_addr127_96 = routingp->ipAddr.v6_addr32[0];

	if (routingp->ipMask) {
		asicMask=routingp->ipMask-1;
	} else{
		asicMask = 0;
	}	
	switch(routingp->process) {
    	case 0://PPPoE
    		entry.linkTo.PPPoEEntry.PPPoEIndex = routingp->pppoeIdx;
    		entry.linkTo.PPPoEEntry.nextHop    = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
    		entry.linkTo.PPPoEEntry.IPMask     = asicMask;
    		entry.linkTo.PPPoEEntry.netif      = routingp->vidx;
    		entry.linkTo.PPPoEEntry.process    = routingp->process;
    		entry.linkTo.PPPoEEntry.valid      = 1;
    		break;
    	case 1://Direct
    		entry.linkTo.L2Entry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
    		entry.linkTo.L2Entry.IPMask  = asicMask;
    		entry.linkTo.L2Entry.netif   = routingp->vidx;
    		entry.linkTo.L2Entry.process = routingp->process;
    		entry.linkTo.L2Entry.valid   = 1;		
    		break;

    	case 2://arp
    		entry.linkTo.ARPEntry.IPMask   = asicMask;
    		entry.linkTo.ARPEntry.netif    = routingp->vidx;
    		entry.linkTo.ARPEntry.process  = routingp->process;
    		entry.linkTo.ARPEntry.subnet_idx = routingp->subnet_idx;
    		entry.linkTo.ARPEntry.valid    = 1;
#if 0            
    		printk("rtl8651_setAsicRoutingv6 arp\n");
    		printk("ARPEntry.ARPEnd   :0x%x\n",entry.linkTo.ARPEntry.ARPEnd);
    		printk("ARPEntry.ARPStart :0x%x\n",entry.linkTo.ARPEntry.ARPStart);
    		printk("ARPEntry.IPMask   :0x%x\n",entry.linkTo.ARPEntry.IPMask);
    		printk("ARPEntry.netif    :0x%x\n",entry.linkTo.ARPEntry.netif);
    		printk("ARPEntry.process  :0x%x\n",entry.linkTo.ARPEntry.process);
#endif
    		break;
    	case 4://CPU
    	case 6://DROP
    		/*
    		  *   Note:  although the process of this routing entry is CPU/DROP,
    		  *             we still have to assign "vid" field for packet decision process use.
    		  *                                                                                          - 2005.3.23 -
    		  */
    		entry.linkTo.ARPEntry.netif   = routingp->vidx;
    		entry.linkTo.ARPEntry.IPMask  = asicMask;
    		entry.linkTo.ARPEntry.process = routingp->process;
    		entry.linkTo.ARPEntry.valid   = 1;
    		break;
    	case 5://NAPT NextHop
    		entry.linkTo.NxtHopEntry.nhStart = routingp->nhStart >> 1;

    		switch (routingp->nhNum)
    		{
    		    case 2: entry.linkTo.NxtHopEntry.nhNum  = 0; break;
    		    case 4: entry.linkTo.NxtHopEntry.nhNum  = 1; break;
    		    case 8: entry.linkTo.NxtHopEntry.nhNum  = 2; break;
    		    case 16: entry.linkTo.NxtHopEntry.nhNum = 3; break;
    		    case 32: entry.linkTo.NxtHopEntry.nhNum = 4; break;
    		    default: return FAILED;
    		}
    		entry.linkTo.NxtHopEntry.nhNxt   = routingp->nhNxt;
    		entry.linkTo.NxtHopEntry.nhAlgo  = routingp->nhAlgo;

    		entry.linkTo.NxtHopEntry.IPMask  = asicMask;
    		entry.linkTo.NxtHopEntry.process = routingp->process;
    		entry.linkTo.NxtHopEntry.valid   = 1;

            break;
    		
    	default: 
    		return FAILED;
	}

    entry.six_rd_eg  = routingp->six_rd_eg;
    entry.six_rd_idx = routingp->six_rd_idx;

#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
	if(dslite_hw_fw)
	{
		if(entry.linkTo.ARPEntry.process  ==2){	
			return FAILED;
		}		
	}
#endif
	return _rtl8651_forceAddAsicEntry(TYPE_L3_V6_ROUTING_TABLE, index, &entry);

}

int32 rtl8198C_delAsicRoutingv6(uint32 index) 
{
	rtl8198C_tblAsic_l3v6RouteTable_t entry;

	if(index >= RTL8198C_ROUTINGV6TBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(entry));
	entry.linkTo.ARPEntry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_L3_V6_ROUTING_TABLE, index, &entry);
}

int32 rtl8198C_getAsicRoutingv6(uint32 index, rtl8198C_tblAsicDrv_routingv6Param_t *routingp) 
{
	rtl8198C_tblAsic_l3v6RouteTable_t entry;
  
	if((index >= RTL8198C_ROUTINGV6TBL_SIZE) || (routingp == NULL))
		return FAILED;
    
	memset(&entry,0,sizeof(entry));
	_rtl8651_readAsicEntry(TYPE_L3_V6_ROUTING_TABLE, index, &entry);
	
	if(entry.linkTo.ARPEntry.valid == 0)
		return FAILED;

    routingp->ipAddr.v6_addr32[3]= entry.ipv6_addr31_0;
    routingp->ipAddr.v6_addr32[2]= entry.ipv6_addr63_32;
    routingp->ipAddr.v6_addr32[1]= entry.ipv6_addr95_64;
    routingp->ipAddr.v6_addr32[0]= entry.ipv6_addr127_96;

	routingp->process = entry.linkTo.ARPEntry.process;

    routingp->vidx = entry.linkTo.ARPEntry.netif;

	switch(routingp->process) {
    	case 0://PPPoE
    		routingp->arpStart = 0;
    		routingp->arpEnd = 0;
    		routingp->pppoeIdx = entry.linkTo.PPPoEEntry.PPPoEIndex;
    		routingp->nextHopRow = entry.linkTo.PPPoEEntry.nextHop>>2;
    		routingp->nextHopColumn = entry.linkTo.PPPoEEntry.nextHop & 0x3;
            routingp->ipMask=entry.linkTo.PPPoEEntry.IPMask+1;
    		break;
    	case 1://Direct
    		routingp->arpStart = 0;
    		routingp->arpEnd = 0;
    		routingp->pppoeIdx = 0;
    		routingp->nextHopRow = entry.linkTo.L2Entry.nextHop>>2;
    		routingp->nextHopColumn = entry.linkTo.L2Entry.nextHop&0x3;
            routingp->ipMask=entry.linkTo.L2Entry.IPMask+1;
    		break;
    	case 2://arp
    		routingp->arpStart = 0;
    		routingp->arpEnd = 0;
    		routingp->pppoeIdx   = 0;
            routingp->nextHopRow = 0;
    		routingp->nextHopColumn = 0;
    		routingp->subnet_idx= entry.linkTo.ARPEntry.subnet_idx;
            routingp->ipMask=entry.linkTo.ARPEntry.IPMask+1;
    		break;
    	case 4: /* CPU */
    	case 6: /* Drop */
    		routingp->arpStart      = 0;
    		routingp->arpEnd        = 0;
    		routingp->pppoeIdx      = 0;
    		routingp->nextHopRow    = 0;
    		routingp->nextHopColumn = 0;
            routingp->ipMask        = 0;
    		break;

    	case 5:
    		routingp->nhStart = (entry.linkTo.NxtHopEntry.nhStart) << 1;
    		switch (entry.linkTo.NxtHopEntry.nhNum)
    		{
        		case 0: routingp->nhNum = 2; break;
        		case 1: routingp->nhNum = 4; break;
        		case 2: routingp->nhNum = 8; break;
        		case 3: routingp->nhNum = 16; break;
        		case 4: routingp->nhNum = 32; break;
        		default: return FAILED;
    		}
    		routingp->nhNxt = entry.linkTo.NxtHopEntry.nhNxt;
    		routingp->nhAlgo = entry.linkTo.NxtHopEntry.nhAlgo;
            routingp->ipMask=entry.linkTo.NxtHopEntry.IPMask+1;	 
    		break;

    	default:
    	    return FAILED;
	}


    routingp->six_rd_eg  = entry.six_rd_eg;
    routingp->six_rd_idx = entry.six_rd_idx;

#if 0
	printk("ipAddr:%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n"
		,routingp->ipAddr.v6_addr16[0],routingp->ipAddr.v6_addr16[1]
		,routingp->ipAddr.v6_addr16[2],routingp->ipAddr.v6_addr16[3]
		,routingp->ipAddr.v6_addr16[4],routingp->ipAddr.v6_addr16[5]
		,routingp->ipAddr.v6_addr16[6],routingp->ipAddr.v6_addr16[7]);
	printk("ipMask     :0x%x\n",routingp->ipMask);
	printk("process    :0x%x\n",routingp->process);
	printk("vidx       :0x%x\n",routingp->vidx);   
	printk("arpStart   :0x%x\n",routingp->arpStart);
	printk("arpEnd     :0x%x\n",routingp->arpEnd);
	printk("nextHopRow   :0x%x\n",routingp->nextHopRow);
	printk("nextHopColumn:0x%x\n",routingp->nextHopColumn);
	printk("pppoeIdx     :0x%x\n",routingp->pppoeIdx);   
	printk("nhStart   :0x%x\n",routingp->nhStart);
	printk("nhNum     :0x%x\n",routingp->nhNum);
	printk("nhNxt     :0x%x\n",routingp->nhNxt);
	printk("nhAlgo    :0x%x\n",routingp->nhAlgo);
    printk("six_rd_eg :0x%x\n",routingp->six_rd_eg);
	printk("six_rd_idx:0x%x\n",routingp->six_rd_idx);
#endif   
    return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: IPv6 ARP Table
  *=========================================*/
int32 rtl8198C_Arpv6TableIndex(rtl8198C_tblAsicDrv_arpV6Param_t *arpp)
{
	uint32 hash_idx_arpv6;
	uint32 id[66];
	uint32 arpv6_hash_idx[6],arpv6_hash[6];
	uint32 i,offset,src;

	for(i=0; i<6; i++) {
		arpv6_hash_idx[i]=0;      
	}

    offset = 0;
    src = arpp->hostid.v6_addr32[3];
    for(i=0; i<32; i++)	{
        if((src& (1<<i))!=0) {
            id[i+offset]=1;
        }
        else 	{
            id[i+offset]=0;
        }		
    }
    
    offset = 32;
    src = arpp->hostid.v6_addr32[2];
    for(i=0; i<32; i++)	{
        if((src& (1<<i))!=0) {
            id[i+offset]=1;
        }
        else 	{
            id[i+offset]=0;
        }		
    }
 
    offset = 64;
    src = arpp->subnet_id;
     for(i=0; i<3; i++)	{
        if((src& (1<<i))!=0) {
            id[i+offset]=1;
        }
        else 	{
            id[i+offset]=0;
        }		
    } 

	arpv6_hash_idx[0] = id[0] ^ id[6]  ^ id[12] ^ id[18] ^ id[24] ^ id[30] ^ id[36] ^ id[42] ^ id[48] ^ id[54] ^ id[60] ^ id[66];
	arpv6_hash_idx[1] = id[1] ^ id[7]  ^ id[13] ^ id[19] ^ id[25] ^ id[31] ^ id[37] ^ id[43] ^ id[49] ^ id[55] ^ id[61] ;
	arpv6_hash_idx[2] = id[2] ^ id[8]  ^ id[14] ^ id[20] ^ id[26] ^ id[32] ^ id[38] ^ id[44] ^ id[50] ^ id[56] ^ id[62] ;
	arpv6_hash_idx[3] = id[3] ^ id[9]  ^ id[15] ^ id[21] ^ id[27] ^ id[33] ^ id[39] ^ id[45] ^ id[51] ^ id[57] ^ id[63] ;
	arpv6_hash_idx[4] = id[4] ^ id[10] ^ id[16] ^ id[22] ^ id[28] ^ id[34] ^ id[40] ^ id[46] ^ id[52] ^ id[58] ^ id[64] ;
	arpv6_hash_idx[5] = id[5] ^ id[11] ^ id[17] ^ id[23] ^ id[29] ^ id[35] ^ id[41] ^ id[47] ^ id[53] ^ id[59] ^ id[65] ;

	for(i=0; i<6; i++) {
		arpv6_hash[i]=arpv6_hash_idx[i] & (0x01);
	}

	hash_idx_arpv6=0;
	for(i=0; i<6; i++) {
		hash_idx_arpv6=hash_idx_arpv6+(arpv6_hash[i]<<i);
	}
 
    ////printk("hash_idx_arpv6:0x%x @ rtl8651_Arpv6TableIndexn\n",hash_idx_arpv6);
	return hash_idx_arpv6;
}

int32 rtl8198C_addAsicArpV6(uint32 index, rtl8198C_tblAsicDrv_arpV6Param_t *arpp) 
{
    uint32 found=0;
	uint32 rowIdx=0,colIdx=0;
	rtl8198C_tblAsicDrv_arpV6Param_t   tbl_entry;

    rowIdx = rtl8198C_Arpv6TableIndex(arpp);
	memset(&tbl_entry,0,sizeof(tbl_entry));

    for(colIdx=0;colIdx<4;colIdx++)
    {          
        tbl_entry.nextHopRow=1;////for rtl8198C_getAsicArpV6 response real get failed or valid==0
		if ((rtl8198C_getAsicArpV6(rowIdx<<2|colIdx,&tbl_entry))!=SUCCESS)
		{
            if(tbl_entry.nextHopRow==1) /*real get failed*/
            {
                goto rtl8198C_addAsicArpV6_err;
            }
            else/*tbl_entry.nextHopRow==0 ==> an empty entry */
            {
                found = TRUE;  
            }
		}
        else        /*get one non empty entry==> compare.*/
        {
            if((tbl_entry.subnet_id == arpp->subnet_id) && 
                (memcmp(&tbl_entry.hostid,&arpp->hostid,sizeof(inv6_addr_t))==0)    )
            {
                found = TRUE;
            }
        }
        
        if (found == TRUE)
            break;
    }
    
    if(found == TRUE )
    {
        if(rtl8198C_setAsicArpV6(rowIdx<<2|colIdx,arpp)==SUCCESS);
            return SUCCESS;
    }

    /*all 4-way full*/
rtl8198C_addAsicArpV6_err:
    return FAILED;
}

int32 rtl8198C_setAsicArpV6(uint32 index, rtl8198C_tblAsicDrv_arpV6Param_t *arpp) 
{
	rtl8198C_tblAsic_arpV6Table_t   entry;

	if((index >= RTL8198C_ARPV6TBL_SIZE) || (arpp == NULL))
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.nextHop    = (arpp->nextHopRow<<2) | (arpp->nextHopColumn&0x3);
	entry.valid      = 1;
	entry.aging      = arpp->aging;
    entry.subnet_idx = arpp->subnet_id;

    entry.host_id_17_0  = arpp->hostid.v6_addr32[3]&0x3FFFF; 
    entry.host_id_49_18 = (arpp->hostid.v6_addr32[2]<<14)|(arpp->hostid.v6_addr32[3]>>18);
    entry.host_id_63_50 = arpp->hostid.v6_addr32[2]>>18;  

    return _rtl8651_forceAddAsicEntry(TYPE_ARP_V6_TABLE, index, &entry);
}

int32 rtl8198C_delAsicArpV6(uint32 index) 
{
	rtl8198C_tblAsic_arpV6Table_t   entry;
	if(index >= RTL8198C_ARPV6TBL_SIZE)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_ARP_V6_TABLE, index, &entry);
}

int32 rtl8198C_getAsicArpV6(uint32 index, rtl8198C_tblAsicDrv_arpV6Param_t *arpp) 
{
	rtl8198C_tblAsic_arpV6Table_t   entry;

	if( (index >= RTL8198C_ARPV6TBL_SIZE) || (arpp == NULL))
		return FAILED;
	//_rtl8651_readAsicEntry(TYPE_ARP_TABLE, index, &entry);
	/*for 8196B patch,read arp table and ip multicast table should stop TLU*/

    memset(&entry,0,sizeof(entry));
	_rtl8651_readAsicEntryStopTLU(TYPE_ARP_V6_TABLE, index, &entry);

	if(entry.valid == 0)
	{
        arpp->nextHopRow=0;//for upper layer more information.
        return FAILED;
	}
	arpp->nextHopRow    = entry.nextHop>>2;
	arpp->nextHopColumn = entry.nextHop&0x3;
	arpp->aging         = entry.aging&0x1f;
    arpp->subnet_id     = entry.subnet_idx;
    arpp->valid         = entry.valid;

    arpp->hostid.v6_addr32[3] =  (entry.host_id_17_0) |
                                 ((entry.host_id_49_18&0x3FFF)<<18);
    arpp->hostid.v6_addr32[2] =  (entry.host_id_49_18>>14) |
                                 (entry.host_id_63_50<<18);
	return SUCCESS;
}

int32 rtl8198C_setAsicNextHopTableV6(uint32 index, rtl8198C_tblAsicDrv_nextHopV6Param_t *nextHopp)
{
    rtl8198C_tblAsic_nextHopV6Table_t  entry;

	if((index >= RTL8198C_NEXTHOPV6TBL_SIZE ) || (nextHopp == NULL))
        return FAILED;

	/* for debug
	rtlglue_printf("[%s:%d] rtl8651_setAsicNextHopTable(idx=%d,Row=%d,Col=%d,PPPIdx=%d,dvid=%d,IPIdx=%d,type=%d)\n",
		__FILE__,__LINE__,index, nextHopp->nextHopRow,nextHopp->nextHopColumn,nextHopp->pppoeIdx,
		nextHopp->dvid,nextHopp->extIntIpIdx,nextHopp->isPppoe);
	*/
    memset(&entry,0,sizeof(entry));
    entry.nextHop    = (nextHopp->nextHopRow <<2) | nextHopp->nextHopColumn;
    entry.PPPoEIndex = nextHopp->pppoeIdx;
    entry.dstVid     = nextHopp->dvid;
    entry.type       = nextHopp->isPppoe==TRUE? 1: 0;
	return _rtl8651_forceAddAsicEntry(TYPE_NEXT_HOP_V6_TABLE, index, &entry);
}

int32 rtl8198C_delAsicNextHopTableV6(uint32 index) 
{
	rtl8198C_tblAsic_nextHopV6Table_t   entry;
    if(index>=RTL8198C_NEXTHOPV6TBL_SIZE)
        return FAILED;
	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_NEXT_HOP_V6_TABLE, index, &entry);
}

int32 rtl8198C_getAsicNextHopTableV6(uint32 index, rtl8198C_tblAsicDrv_nextHopV6Param_t *nextHopp) 
{
    rtl8198C_tblAsic_nextHopV6Table_t  entry;
    if((index>=RTL8198C_NEXTHOPV6TBL_SIZE) || (nextHopp == NULL))
        return FAILED;

    _rtl8651_readAsicEntry(TYPE_NEXT_HOP_V6_TABLE, index, &entry);
    nextHopp->nextHopRow    = entry.nextHop>>2;
    nextHopp->nextHopColumn = entry.nextHop&0x3;
    nextHopp->pppoeIdx      = entry.PPPoEIndex;
    nextHopp->dvid          = entry.dstVid;
    nextHopp->isPppoe       = entry.type==1? TRUE: FALSE;
    return SUCCESS;
}


/*=========================================
  * ASIC DRIVER API: IPv6 Multicast Table
  *=========================================*/
uint32 rtl8198C_ipMulticastv6TableIndex(uint32 hash_type,inv6_addr_t srcAddr, inv6_addr_t dstAddr)
{
	uint32 idx=0,hash_idx_sip,hash_idx_dip;
	static uint32 sip[128],dip[128];
	uint32 sip_hash[8],dip_hash[8];
	uint32 i,j,offset,src,dst;

    if(hash_type==2)
    {
        idx = srcAddr.v6_addr32[3]&0xFF;
        return idx;
    }
    else if(hash_type==3)
    {
        idx = dstAddr.v6_addr32[3]&0xFF;
        return idx;
    }

	for(i=0; i<8; i++) {
		sip_hash[i]=0;
		dip_hash[i]=0;        
	}

    for(j=0;j<4;j++)
    {
        offset = j*32;
        src = srcAddr.v6_addr32[j];
        dst = dstAddr.v6_addr32[j];
    	for(i=0; i<32; i++)	{
    		if((src& (1<<i))!=0) {
    			sip[i+offset]=1;
    		}
    		else 	{
    			sip[i+offset]=0;
    		}

    		if((dst & (1<<i))!=0) {
    			dip[i+offset]=1;
    		}
    		else {
    			dip[i+offset]=0;
    		}			
    	}
    }

	sip_hash[0] = sip[7] ^ sip[15] ^ sip[23] ^ sip[31] ^ sip[39] ^ sip[47] ^ sip[55] ^ sip[63] ^ sip[71] ^ sip[79] ^ sip[87] ^ sip[95] ^ sip[103]^ sip[111] ^ sip[119] ^ sip[127];
	sip_hash[1] = sip[6] ^ sip[14] ^ sip[22] ^ sip[30] ^ sip[38] ^ sip[46] ^ sip[54] ^ sip[62] ^ sip[70] ^ sip[78] ^ sip[86] ^ sip[94] ^ sip[102]^ sip[110] ^ sip[118] ^ sip[126];
	sip_hash[2] = sip[5] ^ sip[13] ^ sip[21] ^ sip[29] ^ sip[37] ^ sip[45] ^ sip[53] ^ sip[61] ^ sip[69] ^ sip[77] ^ sip[85] ^ sip[93] ^ sip[101]^ sip[109] ^ sip[117] ^ sip[125];
	sip_hash[3] = sip[4] ^ sip[12] ^ sip[20] ^ sip[28] ^ sip[36] ^ sip[44] ^ sip[52] ^ sip[60] ^ sip[68] ^ sip[76] ^ sip[84] ^ sip[92] ^ sip[100]^ sip[108] ^ sip[116] ^ sip[124];
	sip_hash[4] = sip[3] ^ sip[11] ^ sip[19] ^ sip[27] ^ sip[35] ^ sip[43] ^ sip[51] ^ sip[59] ^ sip[67] ^ sip[75] ^ sip[83] ^ sip[91] ^ sip[99] ^ sip[107] ^ sip[115] ^ sip[123];
	sip_hash[5] = sip[2] ^ sip[10] ^ sip[18] ^ sip[26] ^ sip[34] ^ sip[42] ^ sip[50] ^ sip[58] ^ sip[66] ^ sip[74] ^ sip[82] ^ sip[90] ^ sip[98] ^ sip[106] ^ sip[114] ^ sip[122];
	sip_hash[6] = sip[1] ^ sip[9]  ^ sip[17] ^ sip[25] ^ sip[33] ^ sip[41] ^ sip[49] ^ sip[57] ^ sip[65] ^ sip[73] ^ sip[81] ^ sip[89] ^ sip[97] ^ sip[105] ^ sip[113] ^ sip[121];
	sip_hash[7] = sip[0] ^ sip[8]  ^ sip[16] ^ sip[24] ^ sip[32] ^ sip[40] ^ sip[48] ^ sip[56] ^ sip[64] ^ sip[72] ^ sip[80] ^ sip[88] ^ sip[96] ^ sip[104] ^ sip[112] ^ sip[120];

	dip_hash[7] = dip[7] ^ dip[15] ^ dip[23] ^ dip[31] ^ dip[39] ^ dip[47] ^ dip[55] ^ dip[63] ^ dip[71] ^ dip[79] ^ dip[87] ^ dip[95] ^ dip[103]^ dip[111] ^ dip[119] ^ dip[127];
	dip_hash[6] = dip[6] ^ dip[14] ^ dip[22] ^ dip[30] ^ dip[38] ^ dip[46] ^ dip[54] ^ dip[62] ^ dip[70] ^ dip[78] ^ dip[86] ^ dip[94] ^ dip[102]^ dip[110] ^ dip[118] ^ dip[126];
	dip_hash[5] = dip[5] ^ dip[13] ^ dip[21] ^ dip[29] ^ dip[37] ^ dip[45] ^ dip[53] ^ dip[61] ^ dip[69] ^ dip[77] ^ dip[85] ^ dip[93] ^ dip[101]^ dip[109] ^ dip[117] ^ dip[125];
	dip_hash[4] = dip[4] ^ dip[12] ^ dip[20] ^ dip[28] ^ dip[36] ^ dip[44] ^ dip[52] ^ dip[60] ^ dip[68] ^ dip[76] ^ dip[84] ^ dip[92] ^ dip[100]^ dip[108] ^ dip[116] ^ dip[124];
	dip_hash[3] = dip[3] ^ dip[11] ^ dip[19] ^ dip[27] ^ dip[35] ^ dip[43] ^ dip[51] ^ dip[59] ^ dip[67] ^ dip[75] ^ dip[83] ^ dip[91] ^ dip[99] ^ dip[107] ^ dip[115] ^ dip[123];
	dip_hash[2] = dip[2] ^ dip[10] ^ dip[18] ^ dip[26] ^ dip[34] ^ dip[42] ^ dip[50] ^ dip[58] ^ dip[66] ^ dip[74] ^ dip[82] ^ dip[90] ^ dip[98] ^ dip[106] ^ dip[114] ^ dip[122];
	dip_hash[1] = dip[1] ^ dip[9]  ^ dip[17] ^ dip[25] ^ dip[33] ^ dip[41] ^ dip[49] ^ dip[57] ^ dip[65] ^ dip[73] ^ dip[81] ^ dip[89] ^ dip[97] ^ dip[105] ^ dip[113] ^ dip[121];
	dip_hash[0] = dip[0] ^ dip[8]  ^ dip[16] ^ dip[24] ^ dip[32] ^ dip[40] ^ dip[48] ^ dip[56] ^ dip[64] ^ dip[72] ^ dip[80] ^ dip[88] ^ dip[96] ^ dip[104] ^ dip[112] ^ dip[120];

	for(i=0; i<8; i++) {
		sip_hash[i]=sip_hash[i] & (0x01);
		dip_hash[i]=dip_hash[i] & (0x01);
	}

	hash_idx_sip=0;
	for(i=0; i<8; i++) {
		hash_idx_sip=hash_idx_sip+(sip_hash[i]<<i);
	}
	hash_idx_dip=0;
	for(i=0; i<8; i++) {
		hash_idx_dip=hash_idx_dip+(dip_hash[i]<<i);
	}

 	idx=0;
    idx = hash_idx_dip ^ hash_idx_sip;
    
	return idx;
}

int32 rtl8198C_setAsicIpMulticastv6Table(uint32 hash_type,rtl8198C_tblAsicDrv_multiCastv6Param_t *mCast_t)
{
    uint32 idx;
 	rtl8198C_tblAsic_ipMulticastv6Table_t entry;
	
	if(mCast_t == NULL)
		return FAILED;

	if(mCast_t->dip.v6_addr32[0] >>24 != 0xFF || mCast_t->port >= RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
	{
        return FAILED;//Non-IP multicast destination address
	}
       
	memset(&entry,0,sizeof(entry));
	entry.sip_addr31_0 		= mCast_t->sip.v6_addr32[3];
	entry.sip_addr63_32 	= mCast_t->sip.v6_addr32[2];
	entry.sip_addr95_64 	= mCast_t->sip.v6_addr32[1];
	entry.sip_addr127_96 	= mCast_t->sip.v6_addr32[0];

	entry.dip_addr31_0 		= mCast_t->dip.v6_addr32[3];
	entry.dip_addr63_32 	= mCast_t->dip.v6_addr32[2];
	entry.dip_addr95_64 	= mCast_t->dip.v6_addr32[1];
	entry.dip_addr123_96 	= mCast_t->dip.v6_addr32[0]&0xFFFFFFF;

    idx=rtl8198C_ipMulticastv6TableIndex(hash_type,mCast_t->sip, mCast_t->dip);

	entry.srcPort 	    = mCast_t->port;
	entry.extmbr 		= mCast_t->mbr >> RTL8651_PORT_NUMBER;
	entry.mbr 			= mCast_t->mbr & (RTL8651_PHYSICALPORTMASK);	
	entry.toCPU 		= mCast_t->cpu;
	entry.valid 		= 1;
    entry.six_rd_eg     = mCast_t->six_rd_eg;
    entry.six_rd_idx    = mCast_t->six_rd_idx;
	entry.ageTime		= mCast_t->age;
	////age = (int16)mCast_t->age;
#if 0
	while ( age > 0 ) {
		if ( (++entry.ageTime) == 7)
			break;
		age -= 5;
	}
#endif
	////entry.ageTime = 7;
    /* todo add compare already has data in this entry?*/
#if defined (CONFIG_RTL_8197F)
	entry.destInterface = 0;
#endif
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_V6_TABLE, idx, &entry);
}

int32 rtl8198C_delAsicIpMulticastv6Table(uint32 index) 
{
	rtl8198C_tblAsic_ipMulticastv6Table_t entry;
	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_V6_TABLE, index, &entry);
}

int32 rtl8198C_getAsicIpMulticastv6Table(uint32 index, rtl8198C_tblAsicDrv_multiCastv6Param_t *mCast_t) 
{	
	rtl8198C_tblAsic_ipMulticastv6Table_t entry;
	
	if (mCast_t == NULL)
	{
        return FAILED;
	}
    
	memset(&entry,0,sizeof(entry));
	_rtl8651_readAsicEntryStopTLU(TYPE_MULTICAST_V6_TABLE, index, &entry);

    mCast_t->sip.v6_addr32[3] = entry.sip_addr31_0;
	mCast_t->sip.v6_addr32[2] = entry.sip_addr63_32;
	mCast_t->sip.v6_addr32[1] = entry.sip_addr95_64;
	mCast_t->sip.v6_addr32[0] = entry.sip_addr127_96;

	mCast_t->dip.v6_addr32[3] = entry.dip_addr31_0;
	mCast_t->dip.v6_addr32[2] = entry.dip_addr63_32;
	mCast_t->dip.v6_addr32[1] = entry.dip_addr95_64;
	mCast_t->dip.v6_addr32[0] = entry.dip_addr123_96;

	mCast_t->port = entry.srcPort;
	mCast_t->mbr  = (entry.extmbr<<RTL8651_PORT_NUMBER) |entry.mbr;	
	mCast_t->age  = entry.ageTime;////*5
	mCast_t->cpu  = entry.toCPU;
    mCast_t->six_rd_eg  = entry.six_rd_eg;
    mCast_t->six_rd_idx = entry.six_rd_idx;
	
	if(entry.valid == 0)
	{
		return FAILED;
	}
	return SUCCESS;
}

//add for 8198c ipv6 multicast(-)
#endif

int32 rtl865x_initAsicL3(void)
{
	int32 index;
	rtl865x_tblAsicDrv_routingParam_t rth;
	
	/*clear asic table*/
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	REG32(MEMCR) = 0;
	REG32(MEMCR) = (1<<3); // ARP and Multicast

	// wait clear done
	while ( (REG32(MEMCR) & (1<<11)) != (1<<11) ) {} ;
#else
	rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);
#endif	
	rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOETBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	rtl8651_clearSpecifiedAsicTable(TYPE_DS_LITE_TABLE, RTL8198C_6RDTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_6RD_TABLE, RTL8198C_DSLITETBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_L3_V6_ROUTING_TABLE, RTL8198C_ROUTINGV6TBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_V6_TABLE, RTL8198C_MULTICASTV6TBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_ARP_V6_TABLE, RTL8198C_ARPV6TBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_V6_TABLE, RTL8198C_NEXTHOPV6TBL_SIZE);
#endif  
	rtl8651_setAsicMulticastEnable(TRUE); /* Enable multicast table */
	WRITE_MEM32(ALECR, READ_MEM32(ALECR)|(uint32)EN_PPPOE);//enable PPPoE auto encapsulation
	rtl8651_setAsicMulticastMTU(1522);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#ifdef CONFIG_RTL_HW_DSLITE_SUPPORT
	REG32(V6DSLITECTR0) = 0;
	WRITE_MEM32(V6DSLITECTR0,(CF_DSLITE_EN_MASK|CF_UKN_DSLDIP_CPU_MASK));
	REG32(V6DSLITECTR1) = 0;
	WRITE_MEM32(V6DSLITECTR1,(CF_HOP_LIMIT<<CF_DSL_HOPLIMIT_OFFSET & CF_DSL_HOPLIMIT_MASK));
#endif	
#ifdef CONFIG_RTL_HW_6RD_SUPPORT
	REG32(V6RDCTR0) = 0;	
	WRITE_MEM32(V6RDCTR0,(CF_6RD_EN_MASK|CF_6RDTOS_MASK|CF_UKN_6RDDIP_CPU_MASK));
	REG32(V6RDCTR1) = 0;		
	WRITE_MEM32(V6RDCTR1,(CF_DSLITE_EN_MASK|CF_DSLTC_EN_MASK|CF_UKN_DSLDIP_CPU_MASK|CF_DSLITE_NAT_MASK));
#endif
#endif
	//Enable TTL-1
	WRITE_MEM32(TTLCR,READ_MEM32(TTLCR)|(uint32)EN_TTL1);//Don't hide this router. enable TTL-1 when routing on this gateway.

	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++) 
	{		
		if( rtl8651_setAsicMulticastPortInternal(index, TRUE) )
			return FAILED;		
	}
	{
		memset(&rth, 0, sizeof(rtl865x_tblAsicDrv_routingParam_t));
		rth.process = 0x4; /* CPU */
		rth.ipAddr = 0;
		rth.ipMask = 0;
		rth.vidx = 0;
		rth.internal = 0;
		rtl8651_setAsicRouting(7, &rth);
	}

	return SUCCESS;	
}

