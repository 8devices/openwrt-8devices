/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table Layer4 napt driver,following features are included:
*	napt
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "asicRegs.h"
#include "rtl865x_asicBasic.h"
#include "rtl865x_asicCom.h"
#include "rtl865x_asicL4.h"
//#include "assert.h"
//#include "rtl_utils.h"


uint32 _rtl8651_NaptAgingToSec(uint32 value) {
	/* 
	 * Convert differentiated timer value to actual second value 
	 * Actual Timeout second = Theory timeout sec * 5/3 - (unit==1? 0: granularity)
	 */
	uint32 unit = 0, G = value>>3, S = 1;

	value &= 0x07;
	while (G > 0) {
		unit += (S << 3);
		S = S << 2;
		G--;
	}
	unit += (S * (value + 1));
	unit = (unit-1)*5/3 - (S>1? S: 0);
	return unit;
}

 uint32 _rtl8651_NaptAgingToUnit(uint32 sec) {
	/* 
	 * Convert actual second value to differentiated timer value
	 * Unit = naptAgingToUnit(sec*0.6) + 1
	 */
	uint32 value = 0, S = 1;

	sec++;
	sec = sec * 3 / 5;
	while(sec >= (S<<3)) {
		sec -= (S << 3);
		S = S << 2;
		value++;
	}
	return (sec%S > (S>>1))? ((value << 3) + (sec / S) + 1) : ((value << 3) + (sec / S) - 1 + 1);
}

/*
@func int8	| Is4WayEnabled	| This function returns if the 4-way hash function is enabled?
@rvalue TRUE	| Yes, 4-way hash is enabled.
@comm 
	(under construction)
@devnote
	(under construction)
 */
uint32 _Is4WayHashEnabled(void)
{
	/*
	rtlglue_printf("%s (%s %d) No More Support \n",__FUNCTION__,__FILE__,__LINE__);
	return 0;	
	*/
	uint32 regValue = READ_MEM32( SWTCR1 );
	return ( regValue & EN_TCPUDP_4WAY_HASH );
}

/*
@func int8	| Is4WayEnabled	| This function sets SWTECR register to enable or diable 4-way hash.
@parm int	| enable	| TRUE if you want to enable 4-way hash.
@rvalue SUCCESS	| Done
@comm 
This function sets SWTECR register to enable or diable 4-way hash.
@devnote
	(under construction)
 */
int32 _set4WayHash( int32 enable )
{
	/*
	rtlglue_printf("%s (%s %d) No More Support \n",__FUNCTION__,__FILE__,__LINE__);
	*/
	uint32 regValue = READ_MEM32( SWTCR1 );
	if ( enable )
	{
		regValue |= EN_TCPUDP_4WAY_HASH;
	}
	else
	{
		//rtlglue_printf("%s (%s %d) Warning!! Disable 4 wayHash would bring on something unexpected! \n",__FUNCTION__,__FILE__,__LINE__);
		regValue &= ~EN_TCPUDP_4WAY_HASH;
	}
	#if 0
	// write back
	WRITE_MEM32( SWTECR, regValue );
	#else
	WRITE_MEM32( SWTCR1, regValue );
	#endif
	
	return SUCCESS;	
}

int32 _rtl8651_enableEnhancedHash1()
{
	uint32 regValue = READ_MEM32( SWTCR1 );

	regValue |= L4EnHash1;
	WRITE_MEM32( SWTCR1, regValue );
	return SUCCESS;
}

int32 _rtl8651_disableEnhancedHash1()
{
	uint32 regValue = READ_MEM32( SWTCR1 );
	
	regValue &= (~L4EnHash1);
	WRITE_MEM32( SWTCR1, regValue );
	return SUCCESS;
}


/*
 * Get table index using the default hash algorithm of RTL865xC
 *
 *   {VERI,isTCP}  srcAddr  srcPort  destAddr  destPort
 *      0     *       	*        	*       	!=0       !=0       	HASH1 index (for tranditional and enhanced Hash1)
 *      0     *      	*        	*        	0         	0        	HASH2 index
 *      1     *       	*        	*        	*         	*        	Verification of Enhanced HASH1
 */
uint32 rtl8651_naptTcpUdpTableIndex(int8 isTCP, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort) {
	uint32 eidx;

	if(destAddr==0 && destPort==0 && ((isTCP&HASH_FOR_VERI)==0))
	{
		/* From alpha's mail (2006/05/02)
		hash 2
		hash2id = sip[9:0] ^ sip[19:10] ^ sip[29:20] ^
		          sport[9:0] ^
		          { 1'd0, tcp_pkt, sip[31:30], sport[15:10] } ;
		*/ 
		eidx = (srcAddr&0x3ff) ^ ((srcAddr>>10)&0x3ff) ^ ((srcAddr>>20)&0x3ff) ^
		       (srcPort&0x3ff) ^
		       (((isTCP&1)<<8) | ((srcAddr&0xc0000000)>>24) | ((srcPort>>10)&0x3f) );
		assert(eidx < RTL8651_TCPUDPTBL_SIZE);
		return eidx;
	} 

	/* From alpha's mail (2006/05/02) 
	  hash 1
	  idx[96:0] = { dip, dport, tcp_pkt, sip, sport } ;  
	  hash1id   = idx[9:0] ^ idx[19:10] ^ idx[29:20] ^
	              idx[39:30] ^ idx[49:40] ^ idx[59:50] ^ 
	              idx[69:60] ^ idx[79:70] ^ idx[89:80] ^
	              { 3'd0,  idx[96:90] } ; 
	*/
	eidx = (srcPort&0x3ff) ^ (((srcPort&0xfc00)>>10)|((srcAddr&0xf)<<6)) ^ ((srcAddr>>4)&0x3ff) ^
	       ((srcAddr>>14)&0x3ff) ^ (((srcAddr&0xff000000)>>24)|((isTCP&1)<<8)|((destPort&1)<<9)) ^
	       ((destPort>>1)&0x3ff) ^ (((destPort>>11)&0x1f)|((destAddr&0x1f)<<5)) ^ ((destAddr>>5)&0x3ff) ^
	       ((destAddr>>15)&0x3ff) ^ ((destAddr>>25)&0x7f);
	assert(eidx < RTL8651_TCPUDPTBL_SIZE);
	return eidx;
}

int32 rtl8651_setAsicLiberalNaptTcpUdpTable( int8 forced, uint16 index,
        ipaddr_t insideLocalIpAddr, uint16 insideLocalPort, int8 selExtIPIdx, uint16 insideGlobalPort, 
        uint32 ageSec, int8 entryType, int8 isTcp, int8 isCollision, int8 isCollision2, int8 isValid ) 
{
	rtl865xc_tblAsic_naptTcpUdpTable_t  entry;
	int8 tcpQuiet, directional, outbound;

	//use liberal hash algorithm
	assert(entryType&RTL8651_STATIC_NAPT_ENTRY);
	assert(entryType & RTL8651_LIBERAL_NAPT_ENTRY);
	assert(index<RTL8651_TCPUDPTBL_SIZE);
	
	memset(&entry,0, sizeof(entry));
	entry.collision = isCollision==TRUE? 1: 0;
	entry.collision2 = isCollision2==TRUE? 1: 0;
	entry.offset = insideGlobalPort>>RTL8651_TCPUDPTBL_BITS;

	// Edward said: col1 and col2 are always set.
	// To recover those code, cvs checkout version 1.3 (2004/01/14).
	entry.collision = entry.collision2 = 1;

	if(isValid){
		entry.valid = 1;
		entry.isStatic = entry.dedicate = 1;
		entry.intIPAddr = insideLocalIpAddr;
		entry.intPort = insideLocalPort;
		
		entry.agingTime = _rtl8651_NaptAgingToUnit(ageSec);

		/* TCPFlag is reused in RTL8651B when dedicate bit is set. 
		 * TCPFlag[2]= 1: Quiet	, 0: Trap SYN/FIN/RST to CPU
		 * TCPFlag[1]= 1: unidirectional,   0: bidirectional
		 * TCPFlag[0]= 1: outbound flow,   0: inbound flow
		 */
		tcpQuiet = (ISSET(entryType, RTL8651_NAPT_SYNFIN_QUIET))? 1:0;
		directional = (ISSET(entryType, RTL8651_NAPT_UNIDIRECTIONAL_FLOW))? 1:0;
		outbound = (directional&&ISSET(entryType, RTL8651_NAPT_OUTBOUND_FLOW))? 1:0;

		/* In fact, these two fields should be filled if entry is for outbound */
		entry.selEIdx = insideGlobalPort & (RTL8651_TCPUDPTBL_SIZE-1);
		entry.selIPIdx = selExtIPIdx;

		/* TCP flag field for HASH2 is {Quiet,Dir,Out}. */
		entry.TCPFlag = (tcpQuiet<<2)|(directional<<1) | (outbound);
		entry.isTCP= isTcp==TRUE? 1: 0;
	}
	else
	{
		entry.isStatic = entry.dedicate= entry.intIPAddr=entry.intPort =0;
		entry.agingTime =entry.selEIdx=entry.selIPIdx= entry.TCPFlag = 0;
	}

	if(forced == TRUE)
		return _rtl8651_forceAddAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);
	else if(_rtl8651_addAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry))
			return FAILED;
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpUdpTable(int8 forced, uint32 index, rtl865x_tblAsicDrv_naptTcpUdpParam_t *naptTcpUdpp) 
{
	rtl865xc_tblAsic_naptTcpUdpTable_t  entry;

	if((index >= RTL8651_TCPUDPTBL_SIZE) || (naptTcpUdpp == NULL))
		return FAILED;

	assert( naptTcpUdpp->tcpFlag < TCP_CLOSED_FLOW );

	memset(&entry, 0, sizeof(entry));
	entry.collision = 1; // always TRUE. naptTcpUdpp->isCollision==TRUE? 1: 0;
	entry.collision2 = 1; // always TRUE. naptTcpUdpp->isCollision2==TRUE? 1: 0;
	entry.valid = naptTcpUdpp->isValid==TRUE? 1: 0;
	entry.intIPAddr = naptTcpUdpp->insideLocalIpAddr;

	entry.intPort=naptTcpUdpp->insideLocalPort;
	entry.offset = naptTcpUdpp->offset;
	entry.agingTime = _rtl8651_NaptAgingToUnit(naptTcpUdpp->ageSec);
	entry.isStatic = naptTcpUdpp->isStatic==TRUE? 1: 0;
	entry.isTCP = naptTcpUdpp->isTcp==TRUE? 1: 0;
	entry.TCPFlag = naptTcpUdpp->tcpFlag;
	entry.dedicate = naptTcpUdpp->isDedicated==TRUE? 1: 0;
	entry.selIPIdx = naptTcpUdpp->selExtIPIdx;
	entry.selEIdx = naptTcpUdpp->selEIdx;

	// Edward said: col1 and col2 are always set.
	// To recover those code, cvs checkout version 1.3 (2004/01/14).
	entry.collision = entry.collision2 = 1;
	entry.priValid =  naptTcpUdpp->priValid;
	entry.priority = naptTcpUdpp->priority;
	
#if defined(CONFIG_RTL_8197F)
	entry.NHIDX      = naptTcpUdpp->NHIDX;
	entry.NHIDXValid = naptTcpUdpp->NHIDXValid;
#endif
    
	if(forced == TRUE){
		_rtl8651_forceAddAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry);
	}else if(_rtl8651_addAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, &entry))
		return FAILED;
	return SUCCESS;
}


int32 rtl8651_getAsicRawNaptTable(uint32 index, void   *entry)
{
	/*entry is actually an rtl8651_tblAsic_naptTcpUdpTable_t pointer*/
	uint32  *pEntry=(uint32 *)entry;
	int32 retval;

	// WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) | 0x2); /*ASIC patch: disable L4 Aging aftrer reading L4 table */
	retval=_rtl8651_readAsicEntry(TYPE_L4_TCP_UDP_TABLE, index, entry);
	//WRITE_MEM32(TEATCR, READ_MEM32(TEATCR) & ~0x2); /*ASIC patch: enable L4 Aging aftrer reading NAPT table */

	//ASIC patch. static entry's valid bit would be cleared if aged out. 
	if(((pEntry[1]&0x1)==0)&&((pEntry[1]&0x10000)==0x10000))  
		pEntry[1] |=0x1;            

	return retval;
}

int32 rtl8651_setAsicRawNaptTable( uint32 index, void *vEntry, int8 forced )
{
	rtl865xc_tblAsic_naptTcpUdpTable_t *entry = (rtl865xc_tblAsic_naptTcpUdpTable_t *)vEntry;

	// Edward said: col1 and col2 are always set.
	// To recover those code, cvs checkout version 1.3 (2004/01/14).
	entry->collision = entry->collision2 = 1;
	if( forced == TRUE )
		return _rtl8651_forceAddAsicEntry( TYPE_L4_TCP_UDP_TABLE, index, vEntry );
	else if( _rtl8651_addAsicEntry( TYPE_L4_TCP_UDP_TABLE, index, vEntry ) )
		return FAILED;
	return SUCCESS;
}

	
int32 rtl8651_getAsicNaptTcpUdpTable(uint32 index, rtl865x_tblAsicDrv_naptTcpUdpParam_t *naptTcpUdpp) 
{
	rtl865xc_tblAsic_naptTcpUdpTable_t  entry;
	if((index >= RTL8651_TCPUDPTBL_SIZE) || (naptTcpUdpp == NULL))
		return FAILED;

	rtl8651_getAsicRawNaptTable( index, &entry);
    
	naptTcpUdpp->insideLocalIpAddr = entry.intIPAddr;
	naptTcpUdpp->insideLocalPort = entry.intPort;
	naptTcpUdpp->offset = entry.offset;
	naptTcpUdpp->ageSec = _rtl8651_NaptAgingToSec(entry.agingTime);
	naptTcpUdpp->isStatic = entry.isStatic;
	naptTcpUdpp->isTcp = entry.isTCP;
	naptTcpUdpp->isCollision = entry.collision;
	naptTcpUdpp->isCollision2 = entry.collision2;
	naptTcpUdpp->isValid = entry.valid;
	naptTcpUdpp->isDedicated = entry.dedicate;
	naptTcpUdpp->selExtIPIdx = entry.selIPIdx;
	naptTcpUdpp->selEIdx = entry.selEIdx;
	naptTcpUdpp->tcpFlag = entry.TCPFlag;
	naptTcpUdpp->priValid= entry.priValid;
	naptTcpUdpp->priority= entry.priority;

#if defined(CONFIG_RTL_8197F)
	naptTcpUdpp->NHIDXValid = entry.NHIDXValid;
	naptTcpUdpp->NHIDX      = entry.NHIDX;
#endif
   	 return SUCCESS;
}

int32 rtl8651_delAsicNaptTcpUdpTable(uint32 start, uint32 end) {
	if((start >= RTL8651_TCPUDPTBL_SIZE) || (end >= RTL8651_TCPUDPTBL_SIZE))
		return FAILED;

	_rtl8651_delAsicEntry(TYPE_L4_TCP_UDP_TABLE, start, end);
	return SUCCESS;
}

/*=========================================
  * ASIC DRIVER API: NAPT TIMER
  * hyking
  * 2007_7_12
  * for 865xC: 0~29bits are valid, 30~31 bits are reserved.
  *=========================================*/

int32 rtl8651_setAsicNaptIcmpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = (uint32)0x3f<< 24;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<24));
	return SUCCESS;
}

int32 rtl8651_getAsicNaptIcmpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>24) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptUdpTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 18;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<18));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptUdpTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>18) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpLongTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 12;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<12));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpLongTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>12) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpMediumTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f << 6;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit<<6));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpMediumTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)>>6) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptTcpFastTimeout(uint32 timeout) {
	uint32 timeUnit = _rtl8651_NaptAgingToUnit(timeout), timeMask;

	if(timeUnit >= 0x400)
		return FAILED;
	timeMask = 0x3f;
	WRITE_MEM32(TEATCR,(READ_MEM32(TEATCR) & ~timeMask) | (timeUnit));

	return SUCCESS;
}

int32 rtl8651_getAsicNaptTcpFastTimeout(uint32 *timeout) {
	if(timeout == NULL)
		return FAILED;
	*timeout = _rtl8651_NaptAgingToSec((READ_MEM32(TEATCR)) & 0x3f);
	return SUCCESS;
}

int32 rtl8651_setAsicNaptAutoAddDelete(int8 autoAdd, int8 autoDelete) {
	if(autoAdd == TRUE)
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)|EN_NAPT_AUTO_LEARN);
	else
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~EN_NAPT_AUTO_LEARN);
	if(autoDelete == TRUE)
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)|EN_NAPT_AUTO_DELETE);
	else
		WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~EN_NAPT_AUTO_DELETE);
	return SUCCESS;
}

int32 rtl865x_initAsicL4(void)
{
	/*clear asic table*/
	rtl8651_clearSpecifiedAsicTable(TYPE_L4_TCP_UDP_TABLE, RTL8651_TCPUDPTBL_SIZE);

	WRITE_MEM32(SWTCR0,READ_MEM32(SWTCR0)&~NAPTR_NOT_FOUND_DROP);//When reverse NAPT entry not found, CPU process it.
	rtl8651_setAsicNaptAutoAddDelete(FALSE, TRUE);

	return SUCCESS;
	
}

