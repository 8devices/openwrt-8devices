/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table Layer3 route driver,following features are included:
*	Route/Multicast
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_ASICL3_H
#define RTL865X_ASICL3_H

#define RTL8651_IPTABLE_SIZE			16
#define RTL8651_PPPOETBL_SIZE			8
#define RTL8651_NEXTHOPTBL_SIZE		32
#define RTL8651_ROUTINGTBL_SIZE		8
#define RTL8651_ARPTBL_SIZE			512


#if defined(CONFIG_RTL_8196E)  || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
//#define CONFIG_RTL_HARDWARE_MULTICAST_CAM
#endif

#if defined(CONFIG_RTL_HARDWARE_MULTICAST_CAM)
#define RTL8651_IPMCAST_CAM_SIZE			32
#else
#define RTL8651_IPMCAST_CAM_SIZE			0
#endif
#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A) || defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)

//#define RTL8651_MULTICASTTBL_SIZE			256
#define RTL8651_IPMULTICASTTBL_SIZE		256
//#define RTL8651_IPMCAST_CAM_SIZE			32
#define RTL8651_MULTICASTTBL_SIZE			(RTL8651_IPMULTICASTTBL_SIZE+RTL8651_IPMCAST_CAM_SIZE)

#elif defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) || defined(CONFIG_RTL_819XD)
#define RTL8651_IPMULTICASTTBL_SIZE		128
#define RTL8651_MULTICASTTBL_SIZE		(RTL8651_IPMULTICASTTBL_SIZE+RTL8651_IPMCAST_CAM_SIZE)
//#define RTL8651_IPMCAST_CAM_SIZE			32

#else
#define RTL8651_IPMULTICASTTBL_SIZE		64
#define RTL8651_MULTICASTTBL_SIZE		(RTL8651_IPMULTICASTTBL_SIZE+RTL8651_IPMCAST_CAM_SIZE)
#endif

#define HASH_METHOD_SIP_DIP0        0
#define HASH_METHOD_SIP_DIP1        1
#define HASH_METHOD_SIP             2
#define HASH_METHOD_DIP             3


typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t        internalIP;
    /* word 1 */
    ipaddr_t        externalIP;
    /* word 2 */
    uint32          reserv0     : 24;
    uint32          nextHop     : 5;
    uint32          isLocalPublic   : 1;
    uint32          isOne2One       : 1;
    uint32          valid       : 1;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    ipaddr_t        internalIP;
    /* word 1 */
    ipaddr_t        externalIP;
    /* word 2 */
    uint32          valid       : 1;
    uint32          isOne2One       : 1;
    uint32          isLocalPublic   : 1;
    uint32          nextHop     : 5;
    uint32          reserv0     : 24;
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
} rtl8651_tblAsic_extIpTable_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint16          reserv0     : 13;
    uint16          ageTime     : 3;
    uint16          sessionID;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint16          sessionID;
    uint16          ageTime     : 3;
    uint16          reserv0     : 13;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
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
} rtl8651_tblAsic_pppoeTable_t;



typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          reserv0     : 11;
    uint32          nextHop     : 10;
    uint32          PPPoEIndex  : 3;
    uint32          dstVid      : 3;
    uint32          IPIndex     : 4;
    uint32          type        : 1;
#else
    /* word 0 */
    uint32          type        : 1;
    uint32          IPIndex     : 4;
    uint32          dstVid      : 3;
    uint32          PPPoEIndex  : 3;
    uint32          nextHop     : 10;
    uint32          reserv0     : 11;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
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
} rtl8651_tblAsic_nextHopTable_t;



typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t        IPAddr;

    /* word 1 */
    union {
        struct {
#if defined(CONFIG_RTL_8198C)
			uint32          DSL_IDX1_0 : 2;
			uint32          DSLEG      : 1;
#else
            uint32          reserv0     : 3;
#endif
            uint32		  ARPIpIdx	: 3;
            uint32          ARPEnd      : 6;			
            uint32          ARPStart    : 6;
            uint32          netif         : 3;
            uint32          isDMZ      : 1;			
            uint32          internal   : 1;
            uint32          process     : 3;
            uint32          valid       : 1;			
            uint32          IPMask      : 5;
        } ARPEntry;
        struct {
#if defined(CONFIG_RTL_8198C)
			uint32          DSL_IDX1_0 : 2;
			uint32          DSLEG      : 1;
			uint32          reserv0     : 5;
#else
            uint32          reserv0     : 8;
#endif
            uint32          nextHop     : 10;
            uint32          netif         : 3;
            uint32          isDMZ      : 1;
            uint32          internal   : 1;
            uint32          process     : 3;			
            uint32          valid       : 1;
            uint32          IPMask      : 5;
        } L2Entry;
        struct {
#if defined(CONFIG_RTL_8198C)
			uint32          DSL_IDX1_0 : 2;
			uint32          DSLEG      : 1;
			uint32          reserv0     : 2;
#else
            uint32          reserv0     : 5;
#endif
            uint32          PPPoEIndex  : 3;
            uint32          nextHop     : 10;
	     uint32          netif         : 3;						
            uint32          isDMZ      : 1;
            uint32          internal   : 1;
            uint32          process     : 3;
            uint32          valid       : 1;
            uint32          IPMask      : 5;
        } PPPoEEntry;
        struct {
#if defined(CONFIG_RTL_8198C)
			uint32          DSL_IDX1_0 : 2;
			uint32          DSLEG      : 1;
			uint32          reserv0     : 1;
#else
            uint32          reserv0     : 4;
#endif	
            uint32          IPDomain    : 3;
            uint32          nhAlgo      : 2;
            uint32          nhNxt       : 5;
            uint32          nhStart     : 4;
            uint32          nhNum       : 3;
	     uint32          isDMZ      : 1;
	     uint32          internal   : 1;
	     uint32          process     : 3;
	     uint32          valid       : 1;
            uint32          IPMask      : 5;			
        } NxtHopEntry;

    } linkTo;

#if defined(CONFIG_RTL_8198C)    
    /* word 2 */
    uint32          reservw2:31;
    uint32          DSL_IDX2:1;
#else
    uint32          reservw2;
#endif	


#else /*_LITTLE_ENDIAN*/

    /* word 0 */
    ipaddr_t        IPAddr;
    /* word 1 */
    union {
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;
			
            uint32          netif         : 3;
            uint32          ARPStart    : 6;
            uint32          ARPEnd      : 6;
            uint32		  ARPIpIdx	: 3;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			uint32          DSLEG      : 1;
			uint32          DSL_IDX1_0 : 2;
#else
            uint32          reserv0     : 3;
#endif		
        } ARPEntry;
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;

            uint32          netif         : 3;			
            uint32          nextHop     : 10;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			uint32          reserv0     : 5;
			uint32          DSLEG      : 1;
			uint32          DSL_IDX1_0 : 2;
#else
            uint32          reserv0     : 8;
#endif	
        } L2Entry;
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;
			
	     uint32          netif         : 3;			
            uint32          nextHop     : 10;
            uint32          PPPoEIndex  : 3;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			uint32          reserv0     : 2;
			uint32          DSLEG       : 1;
			uint32          DSL_IDX1_0  : 2;
#else
            uint32          reserv0     : 5;
#endif	
        } PPPoEEntry;
        struct {
            uint32          IPMask      : 5;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          internal   : 1;
            uint32          isDMZ      : 1;

		
            uint32          nhNum       : 3;
            uint32          nhStart     : 4;
            uint32          nhNxt       : 5;
            uint32          nhAlgo      : 2;
            uint32          IPDomain    : 3;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			uint32          reserv0     : 1;	
			uint32          DSLEG       : 1;
			uint32          DSL_IDX1_0  : 2;		
#else
            uint32          reserv0     : 4;
#endif
        } NxtHopEntry;
    } linkTo;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    /* word 2 */
    uint32          DSL_IDX2:1;
    uint32          reservw2:31;
#else
    uint32          reservw2;
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
} rtl865xc_tblAsic_l3RouteTable_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          reserv0     : 16;
    uint32          aging:5;
    uint32          nextHop     : 10;
    uint32          valid       : 1;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint32          valid       : 1;
    uint32          nextHop     : 10;
    uint32		  aging:5;
    uint32          reserv0     : 21;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
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
} rtl865xc_tblAsic_arpTable_t;

typedef struct {

#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t        srcIPAddr;
    /* word 1 */
    uint32          srcPort      : 4;
    uint32          destIPAddrLsbs : 28;

    /* word 2*/
	#if defined (CONFIG_RTL_8197F)
	uint32          reserv0     : 11;
	uint32          destInterface : 3;
	#else
    uint32          reserv0     : 14;
	#endif
    uint32          ageTime     : 3;
    uint32          toCPU       : 1;
    uint32          valid       : 1;
    uint32          extIPIndex  : 4;
    uint32          portList    : 9;
#else
    /* word 0 */
    ipaddr_t        srcIPAddr;
    /* word 1 */
    uint32          destIPAddrLsbs : 28;
    uint32          srcPort      :4 ;

    /* word 2*/
    uint32          portList    : 9;
    uint32          extIPIndex  : 4;
    uint32          valid       : 1;
    uint32          toCPU       : 1;
    uint32          ageTime     : 3;
#if defined (CONFIG_RTL_8197F)    
	uint32   		destInterface : 3;
    uint32          reserv0     : 11;
#else
    uint32          reserv0     : 14;	
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

} rtl865xc_tblAsic_ipMulticastTable_t;

typedef struct rtl865x_tblAsicDrv_extIntIpParam_s {
	    ipaddr_t 	extIpAddr;
	    ipaddr_t 	intIpAddr;
	    uint32 		nhIndex; //index of next hop table
	    uint32 		localPublic:1,
	           		nat:1;
} rtl865x_tblAsicDrv_extIntIpParam_t;

typedef struct rtl865x_tblAsicDrv_pppoeParam_s {
	uint16 sessionId;
	uint16 age;
} rtl865x_tblAsicDrv_pppoeParam_t;


typedef struct rtl865x_tblAsicDrv_nextHopParam_s {
	uint32 nextHopRow;
	uint32 nextHopColumn;
	uint32 pppoeIdx;
	uint32 dvid;	//note: dvid means DVID index here! hyking
	uint32 extIntIpIdx;
	uint32 isPppoe:1;
} rtl865x_tblAsicDrv_nextHopParam_t;

typedef struct rtl865x_tblAsicDrv_routingParam_s {
	    ipaddr_t ipAddr;
	    ipaddr_t ipMask;
	    uint32 process; //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 5:napt nexthop
	    uint32 vidx;
	    uint32 arpStart;
	    uint32 arpEnd;
	    uint32 arpIpIdx; /* for RTL8650B C Version Only */
	    uint32 nextHopRow;
	    uint32 nextHopColumn;
	    uint32 pppoeIdx;
	    uint32 nhStart; //exact index
	    uint32 nhNum; //exact number
	    uint32 nhNxt;
	    uint32 nhAlgo;
	    uint32 ipDomain;
	    uint16 	internal:1,
		DMZFlag:1;
		
 	    uint32 netif;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        uint32          DSLEG;
        uint32          DSL_IDX;
#endif	
        
} rtl865x_tblAsicDrv_routingParam_t;

typedef struct rtl865x_tblAsicDrv_arpParam_s {
	uint32 nextHopRow;
	uint32 nextHopColumn;
	uint32 aging;	
} rtl865x_tblAsicDrv_arpParam_t;

typedef struct rtl865x_tblAsicDrv_multiCastParam_s {
	ipaddr_t	sip;
	ipaddr_t	dip;
	uint16	svid;
	uint16	port;
	uint32	mbr;
	uint16	age;
	uint16	cpu;
	uint16	extIdx;
#if defined (CONFIG_RTL_HARDWARE_MULTICAST_CAM)
	uint16  idx;
#endif
} rtl865x_tblAsicDrv_multiCastParam_t;


/*arp*/
int32 rtl8651_setAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp);
int32 rtl8651_delAsicArp(uint32 index);
int32 rtl8651_getAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp);

/*ip*/
int32 rtl8651_setAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp);
int32 rtl8651_delAsicExtIntIpTable(uint32 index);
int32 rtl8651_getAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp);

/*pppoe*/
int32 rtl8651_setAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep);
int32 rtl8651_getAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep);

/*nexthop*/
int32 rtl8651_setAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp);
int32 rtl8651_getAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp);

/*L3 routing*/
int32 rtl8651_setAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp);
int32 rtl8651_delAsicRouting(uint32 index);
int32 rtl8651_getAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp);

/*multicast*/
uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr);
int32 rtl8651_setAsicIpMulticastTable(rtl865x_tblAsicDrv_multiCastParam_t *mCast_t);
int32 rtl8651_delAsicIpMulticastTable(uint32 index);
int32 rtl8651_getAsicIpMulticastTable(uint32 index, rtl865x_tblAsicDrv_multiCastParam_t *mCast_t);
int32 rtl8651_setAsicMulticastPortInternal(uint32 port, int8 isInternal);
int32 rtl8651_getAsicMulticastPortInternal(uint32 port, int8 *isInternal);
int32 rtl8651_setAsicMulticastMTU(uint32 mcastMTU);
int32 rtl8651_getAsicMulticastMTU(uint32 *mcastMTU);
int32 rtl8651_setAsicMulticastEnable(uint32 enable);
int32 rtl8651_getAsicMulticastEnable(uint32 *enable);
int32 rtl865x_setAsicMulticastAging(uint32 enable);
int32 rtl865x_getAsicMCastHashMethod(unsigned int *hashMethod);
int32 rtl865x_setAsicMCastHashMethod(unsigned int hashMethod);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
int32 rtl8198C_setAsicMulticastv6Enable(uint32 enable);
int32 rtl8198C_getAsicMulticastv6Enable(uint32 *enable);
int32 rtl8198C_setAsicMulticastv6MTU(uint32 mcastMTU);
int32 rtl8198C_getAsicMulticastv6MTU(uint32 *mcastMTU);
int32 rtl8198C_getAsicMCastv6HashMethod(unsigned int *hashMethod);
int32 rtl8198C_setAsicMCastv6HashMethod(unsigned int hashMethod);
int32 rtl8198C_setAsicMulticastv6Aging(uint32 enable);
#endif




#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL8198C_6RDTBL_SIZE		 8
#define RTL8198C_DSLITETBL_SIZE		 8
#define RTL8198C_ROUTINGV6TBL_SIZE	 8
#define RTL8198C_MULTICASTV6TBL_SIZE 256
#define RTL8198C_ARPV6TBL_SIZE		 256 //4-way
#define RTL8198C_NEXTHOPV6TBL_SIZE	 32


typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    ipaddr_t      ce_ip_addr;

    /* word 1 */
    uint32        six_rd_pfx25_0 :26;
    uint32        ce_ip_mask     :6;
    /* word 2 */
	uint32        six_rd_pfx57_26;
    /* word 3 */
    uint32        br_ip_addr19_0 :20;		
    uint32        pfx_mask       :6;
    uint32        six_rd_pfx63_58:6;

    /* word 4 */
    uint32        six_rd_mtu     :15;
    uint32        br_ip_mask     :5;
    uint32        br_ip_addr31_20:12;
    /* word 5 */
    uint32        reservw5       :31;
    uint32        valid          :1;
	
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    ipaddr_t      ce_ip_addr;
    /* word 1 */
    uint32        ce_ip_mask     :6;
    uint32        six_rd_pfx25_0 :26;	
    /* word 2 */
	uint32        six_rd_pfx57_26;

    /* word 3 */
    uint32        six_rd_pfx63_58:6;
    uint32        pfx_mask       :6;
    uint32        br_ip_addr19_0 :20;	
	
    /* word 4 */
    uint32        br_ip_addr31_20:12;
    uint32        br_ip_mask     :5;
    uint32        six_rd_mtu     :15;

    /* word 5 */  
    uint32        valid          :1;	
    uint32        reservw5       :31;
#endif /*_LITTLE_ENDIAN*/
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl8198C_tblAsic_6rdTable_t;


typedef struct {
#ifndef _LITTLE_ENDIAN

    /* word 0 */
    uint32          host_id_17_0: 18;
    uint32          subnet_idx  : 3;
    uint32          nextHop     : 10;
    uint32          valid       : 1;
    /* word 1 */    
    uint32          host_id_49_18;
    /* word 2 */
    uint32          reserv0     : 13;
    uint32		    aging       : 5;
    uint32          host_id_63_50:14;

#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint32          valid       : 1;
    uint32          nextHop     : 10;
    uint32          subnet_idx  : 3;
    uint32          host_id_17_0: 18;
    /* word 1 */    
    uint32          host_id_49_18;
    /* word 2 */
    uint32          host_id_63_50:14;  
    uint32		    aging       : 5;
    uint32          reserv0     : 13;
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
} rtl8198C_tblAsic_arpV6Table_t;


typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          reserv0     : 15;
    uint32          nextHop     : 10;
    uint32          PPPoEIndex  : 3;
    uint32          dstVid      : 3;
    uint32          type        : 1;
#else
    /* word 0 */
    uint32          type        : 1;
    uint32          dstVid      : 3;
    uint32          PPPoEIndex  : 3;
    uint32          nextHop     : 10;
    uint32          reserv0     : 15;
#endif /*_LITTLE_ENDIAN*/
    /* word 1 */
    uint32          reservw1;
    /* word 2 */
    uint32          reservw2;
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
} rtl8198C_tblAsic_nextHopV6Table_t;


typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32        host_ipv6_addr31_0;
    /* word 1 */
    uint32        host_ipv6_addr63_32;
    /* word 2 */
    uint32        host_ipv6_addr95_64;
    /* word 3 */
    uint32        host_ipv6_addr127_96;
    /* word 4 */
    uint32        aftr_ipv6_addr24_0:25;
	uint32        host_ipv6_mask    :7;
	/* word 5 */
    uint32        aftr_ipv6_addr56_25;
    /* word 6 */
    uint32        aftr_ipv6_addr88_57;
    /* word 7 */
    uint32        aftr_ipv6_addr120_89;
    /* word 8 */
	uint32        reservw8             :2;	
    uint32        valid                :1;		
    uint32        dslite_mtu           :15;
    uint32        aftr_ipv6_mask       :7;
    uint32        aftr_ipv6_addr127_121:7;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint32        host_ipv6_addr31_0;
    /* word 1 */
    uint32        host_ipv6_addr63_32;
    /* word 2 */
    uint32        host_ipv6_addr95_64;
    /* word 3 */
    uint32        host_ipv6_addr127_96;	
    /* word 4 */
    uint32        host_ipv6_mask    :7;
    uint32        aftr_ipv6_addr24_0:25;
    /* word 5 */
    uint32        aftr_ipv6_addr56_25;
    /* word 6 */
    uint32        aftr_ipv6_addr88_57;
    /* word 7 */
    uint32        aftr_ipv6_addr120_89;
    /* word 8 */
    uint32        aftr_ipv6_addr127_121 :7;
    uint32        aftr_ipv6_mask        :7;
    uint32        dslite_mtu            :15;
    uint32        valid                 :1;
	uint32        reservw8              :2;
#endif /*_LITTLE_ENDIAN*/

} rtl8198C_tblAsic_dsliteTable_t;


typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32        ipv6_addr31_0;

    /* word 1 */
    uint32        ipv6_addr63_32;

    /* word 2 */
    uint32        ipv6_addr95_64;

    /* word 3 */
    uint32        ipv6_addr127_96;

    /* word 4 */
    union {
        struct {
			uint32          reserv0     : 15;
            uint32          subnet_idx  : 3;
            uint32          netif       : 3;
            uint32          process     : 3;
            uint32          valid       : 1;
            uint32          IPMask      : 7;
        } ARPEntry;
        struct {
			uint32          reserv0     : 8;
            uint32          nextHop     : 10;
            uint32          netif       : 3;
            uint32          process     : 3;			
            uint32          valid       : 1;
            uint32          IPMask      : 7;
        } L2Entry;
        struct {
			uint32          reserv0     : 5;
            uint32          PPPoEIndex  : 3;
            uint32          nextHop     : 10;
	        uint32          netif       : 3;						
            uint32          process     : 3;
            uint32          valid       : 1;
            uint32          IPMask      : 7;
        } PPPoEEntry;
        struct {
			uint32          reserv0     : 7;
            uint32          nhAlgo      : 2;
            uint32          nhNxt       : 5;
            uint32          nhStart     : 4;
            uint32          nhNum       : 3;
	        uint32          process     : 3;
	        uint32          valid       : 1;
            uint32          IPMask      : 7;			
        } NxtHopEntry;

    } linkTo;

    /* word 5 */
    uint32          reservw5   :28;
    uint32          six_rd_idx :3;
    uint32          six_rd_eg  :1;
#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint32        ipv6_addr31_0;
    /* word 1 */
    uint32        ipv6_addr63_32;
    /* word 2 */
    uint32        ipv6_addr95_64;
    /* word 3 */
    uint32        ipv6_addr127_96;
    /* word 4 */
    union {
        struct {
            uint32          IPMask      : 7;
            uint32          valid       : 1;
            uint32          process     : 3;		
            uint32          netif       : 3;
            uint32          subnet_idx   : 3;
			uint32          reserv0     : 15;
        } ARPEntry;
        struct {
            uint32          IPMask      : 7;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          netif       : 3;			
            uint32          nextHop     : 10;			
			uint32          reserv0     : 8;
        } L2Entry;
        struct {
            uint32          IPMask      : 7;
            uint32          valid       : 1;
            uint32          process     : 3;			
	        uint32          netif       : 3;			
            uint32          nextHop     : 10;
            uint32          PPPoEIndex  : 3;
			uint32          reserv0     : 5;	
        } PPPoEEntry;
        struct {
            uint32          IPMask      : 7;
            uint32          valid       : 1;
            uint32          process     : 3;
            uint32          nhNum       : 3;
            uint32          nhStart     : 4;
            uint32          nhNxt       : 5;
            uint32          nhAlgo      : 2;
			uint32          reserv0     : 7;

        } NxtHopEntry;
    } linkTo;

    /* word 5 */
    uint32          six_rd_eg  :1;
    uint32          six_rd_idx :3;
    uint32          reservw5   :28;   
#endif /*_LITTLE_ENDIAN*/
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;

} rtl8198C_tblAsic_l3v6RouteTable_t;


typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32        sip_addr31_0;
    /* word 1 */
    uint32        sip_addr63_32;
    /* word 2 */
    uint32        sip_addr95_64;
    /* word 3 */
    uint32        sip_addr127_96;
    /* word 4 */
    uint32        dip_addr31_0;
    /* word 5 */
    uint32        dip_addr63_32;
    /* word 6 */
    uint32        dip_addr95_64;
    /* word 7 */
    uint32        srcPort        : 4;
    uint32        dip_addr123_96 :28;
    /* word 8*/
#if defined (CONFIG_RTL_8197F)
    uint32        reserv0     : 11;
	uint32		  destInterface : 3;
#else
    uint32        reserv0     : 14;
#endif
    uint32        ageTime     : 3;
    uint32        toCPU       : 1;
    uint32        valid       : 1;   
    uint32        six_rd_idx  : 3;
    uint32        six_rd_eg   : 1;
    uint32        extmbr      : 3;
    uint32        mbr         : 6;
#else
    /* word 0 */
    uint32        sip_addr31_0;
    /* word 1 */
    uint32        sip_addr63_32;
    /* word 2 */
    uint32        sip_addr95_64;
    /* word 3 */
    uint32        sip_addr127_96;
    /* word 4 */
    uint32        dip_addr31_0;
    /* word 5 */
    uint32        dip_addr63_32;
    /* word 6 */
    uint32        dip_addr95_64;
    /* word 7 */
    uint32        dip_addr123_96 :28;
    uint32        srcPort        : 4;
    /* word 8*/
    uint32        mbr         : 6;
    uint32        extmbr      : 3;
    uint32        six_rd_eg   : 1;
    uint32        six_rd_idx  : 3;
    uint32        valid       : 1;   
    uint32        toCPU       : 1;
    uint32        ageTime     : 3;
#if defined (CONFIG_RTL_8197F)
	uint32        destInterface : 3;
    uint32        reserv0     : 11;
#else
    uint32        reserv0     : 14;	
#endif
#endif
} rtl8198C_tblAsic_ipMulticastv6Table_t;

typedef struct rtl865x_tblAsicDrv_arpV6Param_s {
	uint32 nextHopRow;
	uint32 nextHopColumn;
	uint32 aging;	
	uint32 subnet_id;
    inv6_addr_t hostid;
	uint32 valid;	
} rtl8198C_tblAsicDrv_arpV6Param_t;


typedef struct rtl865x_tblAsicDrv_nextHopV6Param_s {
	uint32 nextHopRow;
	uint32 nextHopColumn;
	uint32 pppoeIdx;
	uint32 dvid;	
	uint32 isPppoe;
} rtl8198C_tblAsicDrv_nextHopV6Param_t;


typedef struct rtl8198C_tblAsicDrv_6rdParam_s {
	ipaddr_t    ce_ip_addr;
	uint8       ce_ip_mask_len;/*LSB*/
	inv6_addr_t six_rd_prefix;
    uint8       six_rd_prefix_len;
	ipaddr_t    br_ip_addr;
	uint8       br_ip_mask_len;/*MSB*/
	uint16      mtu;
	uint8       valid;
} rtl8198C_tblAsicDrv_6rdParam_t;


typedef struct rtl8198C_tblAsicDrv_dsliteParam_s {
	inv6_addr_t host_ipv6_addr;
	uint32      host_ipv6_mask;
	inv6_addr_t aftr_ipv6_addr;
    uint32      aftr_ipv6_mask;
	uint32      mtu;
	uint32      valid;
} rtl8198C_tblAsicDrv_dsliteParam_t;

typedef struct rtl865x_tblAsicDrv_routingv6Param_s {
	    inv6_addr_t ipAddr;
	    uint32 ipMask;
	    uint32 process; //0: pppoe, 1:direct, 2:indirect, 4:Strong CPU, 5:napt nexthop
	    uint32 vidx;
		
	    uint32 arpStart;
	    uint32 arpEnd;
	    uint32 subnet_idx;
        
	    uint32 nextHopRow;
	    uint32 nextHopColumn;
	    uint32 pppoeIdx;
		
	    uint32 nhStart; //exact index
	    uint32 nhNum;   //exact number
	    uint32 nhNxt;
	    uint32 nhAlgo;
		
 	    uint32 netif;
        
 	    uint32 six_rd_eg;
 	    uint32 six_rd_idx;      
} rtl8198C_tblAsicDrv_routingv6Param_t;

typedef struct rtl8198C_tblAsicDrv_multiCastv6Param_s {
	inv6_addr_t	sip;
	inv6_addr_t	dip;
	uint16	port;
	uint32	mbr;
	uint16	age;
	uint16	cpu;
	uint16	six_rd_eg;
	uint16	six_rd_idx;
} rtl8198C_tblAsicDrv_multiCastv6Param_t;


/*6rd*/
int32 rtl8198C_setAsic6rdTable(uint32 index, rtl8198C_tblAsicDrv_6rdParam_t *six_rd_entry);
int32 rtl8198C_delAsic6rdTable(uint32 index);
int32 rtl8198C_getAsic6rdTable(uint32 index, rtl8198C_tblAsicDrv_6rdParam_t *six_rd_entry);

/*dslite*/
int32 rtl8198C_setAsicDsliteTable(uint32 index, rtl8198C_tblAsicDrv_dsliteParam_t *dslite_entry);
int32 rtl8198C_delAsicDsliteTable(uint32 index);
int32 rtl8198C_getAsicDsliteTable(uint32 index, rtl8198C_tblAsicDrv_dsliteParam_t *dslite_entry);

/*L3v6 routing*/
int32 rtl8198C_setAsicRoutingv6(uint32 index, rtl8198C_tblAsicDrv_routingv6Param_t *routingp);
int32 rtl8198C_delAsicRoutingv6(uint32 index);
int32 rtl8198C_getAsicRoutingv6(uint32 index, rtl8198C_tblAsicDrv_routingv6Param_t *routingp);

/*arpv6 */
int32 rtl8198C_Arpv6TableIndex(rtl8198C_tblAsicDrv_arpV6Param_t *arpp);
int32 rtl8198C_addAsicArpV6(uint32 index, rtl8198C_tblAsicDrv_arpV6Param_t *arpp);
int32 rtl8198C_setAsicArpV6(uint32 index, rtl8198C_tblAsicDrv_arpV6Param_t *arpp);
int32 rtl8198C_delAsicArpV6(uint32 index);
int32 rtl8198C_getAsicArpV6(uint32 index, rtl8198C_tblAsicDrv_arpV6Param_t *arpp);

/*nexthopv6 */
int32 rtl8198C_setAsicNextHopTableV6(uint32 index, rtl8198C_tblAsicDrv_nextHopV6Param_t *nextHopp);
int32 rtl8198C_delAsicNextHopTableV6(uint32 index) ;
int32 rtl8198C_getAsicNextHopTableV6(uint32 index, rtl8198C_tblAsicDrv_nextHopV6Param_t *nextHopp);

/*multicastv6*/
uint32 rtl8198C_ipMulticastv6TableIndex(uint32 hash_type,inv6_addr_t srcAddr, inv6_addr_t dstAddr);
int32 rtl8198C_setAsicIpMulticastv6Table(uint32 hash_type,rtl8198C_tblAsicDrv_multiCastv6Param_t *mCast_t);
int32 rtl8198C_delAsicIpMulticastv6Table(uint32 index);
int32 rtl8198C_getAsicIpMulticastv6Table(uint32 index, rtl8198C_tblAsicDrv_multiCastv6Param_t *mCast_t);

#endif





#endif
