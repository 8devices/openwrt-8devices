/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table Layer4 napt driver,following features are included:
*	napt
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_ASICL4_H
#define RTL865X_ASICL4_H

#define RTL8651_DYNAMIC_NAPT_ENTRY		(0x0<<0)
#define RTL8651_STATIC_NAPT_ENTRY			(0x1<<0)
#define 	RTL8651_LIBERAL_NAPT_ENTRY			(0x2<<0)

#define RTL8651_TCPUDPTBL_SIZE			1024
#define RTL8651_SERVERPORTTBL_SIZE	16
#define RTL8651_ALGTBL_SIZE			48
#define RTL8651_ICMPTBL_SIZE			32
#define RTL8651_TCPUDPTBL_BITS			10

#define RTL8651_NAPT_OUTBOUND_FLOW				(1<<2) 	//exact value in ASIC
#define RTL8651_NAPT_INBOUND_FLOW				(0<<2)	//exact value in ASIC
#define RTL8651_NAPT_UNIDIRECTIONAL_FLOW		(2<<2)	//exact value in ASIC
#define RTL8651_NAPT_SYNFIN_QUIET				(4<<2)	//exact value in ASIC 
#define RTL8651_NAPT_CHKAUTOLEARN				(1<<5)
//In RTL8651, TCPFlag field records current state of entry
#define RTL8651_TCPNAPT_WAIT4FIN			(0x4 <<2) //exact value in ASIC
#define RTL8651_TCPNAPT_WAITINBOUND			(0x2 <<2) //exact value in ASIC
#define RTL8651_TCPNAPT_WAITOUTBOUND		(0x1 <<2) //exact value in ASIC

#define HASH_UDP      0 /* flag for hash UDP */
#define HASH_TCP      1 /* flag for hash TCP */
#define HASH_FOR_TRAN 0 /* flag for hash hash1 and hash2 index */
#define HASH_FOR_VERI 2 /* flag for hash verification value of enhanced hash1 */

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t        intIPAddr;
    /* word 1 */
    uint32          reserv0     : 1;
    uint32          selEIdx     : 10;
    uint32          selIPIdx    : 4;
    uint32          isStatic    : 1;
    uint32          dedicate    : 1;
    uint32          collision2  : 1;
    uint32          offset      : 6;
    uint32          agingTime   : 6;
    uint32          collision   : 1;
    uint32          valid       : 1;

    /* word 2 */
#if defined(CONFIG_RTL_8197F)
    uint32          reserv2     : 2;
    uint32          NHIDX       : 5;
    uint32          NHIDXValid  : 1;
#else
    uint32          reserv2     : 8;
#endif
    uint32		  priority     :3;
    uint32          priValid	   :1;
    uint32          isTCP       : 1;
    uint32          TCPFlag     : 3;
    uint32          intPort     : 16;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    ipaddr_t        intIPAddr;
    /* word 1 */
    uint32          valid       : 1;
    uint32          collision   : 1;
    uint32          agingTime   : 6;
    uint32          offset      : 6;
    uint32          collision2  : 1;
    uint32          dedicate    : 1;
    uint32          isStatic    : 1;
    uint32          selIPIdx    : 4;
    uint32          selEIdx     : 10;
    uint32          reserv0     : 1;

    /* word 2 */
    uint32          intPort     : 16;
    uint32          TCPFlag     : 3;
    uint32          isTCP       : 1;
    uint32          priValid	   :1;
    uint32		  priority	   :3;
#if defined(CONFIG_RTL_8197F)
    uint32          NHIDXValid  : 1; 
    uint32          NHIDX       : 5;
    uint32          reserv2     : 2;
#else
    uint32          reserv2     : 8;
#endif
#endif /*_LITTLE_ENDIAN*/
    /* word 3 */
    uint32          reservw3;
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl865xc_tblAsic_naptTcpUdpTable_t;

typedef struct rtl865x_tblAsicDrv_naptTcpUdpParam_s {
	ipaddr_t 	insideLocalIpAddr;
	uint16 	insideLocalPort;
	uint32 	ageSec;
	uint8 	tcpFlag;
	uint8 	offset;
	uint8 	selExtIPIdx;
	uint16 	selEIdx;
	uint32 	isTcp:1,
			isCollision:1,
			isStatic:1,
			isCollision2:1,
			isDedicated:1,
			isValid:1,
			priValid:1,
#if defined(CONFIG_RTL_8197F)
			priority:3,
			NHIDXValid:1,
			NHIDX:5;
#else
			priority:3;
#endif
			
} rtl865x_tblAsicDrv_naptTcpUdpParam_t;


//uint32 _Is4WayHashEnabled( void );
int32 _set4WayHash( int32 enable );
int32 _rtl8651_enableEnhancedHash1(void);
int32 _rtl8651_disableEnhancedHash1(void);

uint32 rtl8651_naptTcpUdpTableIndex(int8 isTCP, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort);

int32 rtl8651_setAsicNaptTcpUdpTable(int8 forced, uint32 index, rtl865x_tblAsicDrv_naptTcpUdpParam_t *naptTcpUdpp);
int32 rtl8651_getAsicNaptTcpUdpTable(uint32 index, rtl865x_tblAsicDrv_naptTcpUdpParam_t *naptTcpUdpp);
int32 rtl8651_delAsicNaptTcpUdpTable(uint32 start, uint32 end);

int32 rtl8651_setAsicNaptIcmpTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptIcmpTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptUdpTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptUdpTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptTcpLongTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptTcpLongTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptTcpMediumTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptTcpMediumTimeout(uint32 *timeout);
int32 rtl8651_setAsicNaptTcpFastTimeout(uint32 timeout);
int32 rtl8651_getAsicNaptTcpFastTimeout(uint32 *timeout);

#endif
