/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table basic operation driver
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_ASICCOM_H
#define RTL865X_ASICCOM_H
#include <linux/seq_file.h>

#if defined(CONFIG_RTL_8197F)
#define CONFIG_FINETUNE_RUNOUT_IRQ 1
#endif 

// note: CONFIG_RTL_CPU_TAG should NOT be defined when CONFIG_RTL_8370_SUPPORT is chose.
#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT)
#define CONFIG_RTL_CPU_TAG		1
#endif

#if defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
#define RTL865XC_VLANTBL_SIZE				16
#else
#define RTL865XC_VLANTBL_SIZE				4096
#endif

#define RTL865XC_NETIFTBL_SIZE			8
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL8651_ACLTBL_SIZE			252
#define RTL8651_ACLHWTBL_SIZE			256
#define RTL8651_ACLTBL_RESERV_SIZE	4
#else
#define RTL8651_ACLTBL_SIZE			125
#define RTL8651_ACLHWTBL_SIZE			128
#define RTL8651_ACLTBL_RESERV_SIZE	3
#endif

#if defined(CONFIG_RTL_8198C)
#define CONFIG_RTL_GIGA_LITE_REFINE		1
#endif

#define RTL8651_MAC_NUMBER				6
#define RTL8651_PORT_NUMBER				RTL8651_MAC_NUMBER
#define RTL8651_ALLPORTMASK				((1<<RTL8651_AGGREGATOR_NUMBER)-1)
#define RTL8651_PHYSICALPORTMASK			((1<<RTL8651_MAC_NUMBER)-1)

extern int32 rtl8651_totalExtPortNum; //this replaces all rtl8651_totalExtPortNum defines
extern int32		rtl8651_allExtPortMask;
#define RTL8651_AGGREGATOR_NUMBER		(RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
//#define RTL8651_PSOFFLOAD_RESV_PORT		(RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum+1)	/* port reserved for protocol stack offloading */

/*	external PHY Property :
	Because there really has many different external PHY, so we just simply define for each MODEL. */
#define RTL8651_TBLASIC_EXTPHYPROPERTY_PORT1234_RTL8212		(1 << 0)
#define RTL8651_TBLASIC_EXTPHYPROPERTY_PORT5_RTL8211B			(1 << 1)

/*===============================================
 * ASIC DRIVER DEFINITION: Protocol-based VLAN
 *==============================================*/
#define RTL8651_PBV_RULE_IPX				1	/* Protocol-based VLAN rule 1: IPX */
#define RTL8651_PBV_RULE_NETBIOS			2	/* Protocol-based VLAN rule 2: NetBIOS */
#define RTL8651_PBV_RULE_PPPOE_CONTROL		3	/* Protocol-based VLAN rule 3: PPPoE Control */
#define RTL8651_PBV_RULE_PPPOE_SESSION		4	/* Protocol-based VLAN rule 4: PPPoE Session */
#define RTL8651_PBV_RULE_USR1				5	/* Protocol-based VLAN rule 5: user-defined 1 */
#define RTL8651_PBV_RULE_USR2				6	/* Protocol-based VLAN rule 6: user-defined 2 */
#define RTL8651_PBV_RULE_MAX				7


#define ASICDRV_ASSERT(expr) do {\
	if(!(expr)){\
		rtlglue_printf("Error >>> initialize failed at function %s line %d!!!\n", __FUNCTION__, __LINE__);\
			return FAILED;\
	}\
}while(0)

#define ASICDRV_INIT_CHECK(expr) do {\
	if((expr)!=SUCCESS){\
		rtlglue_printf("Error >>> initialize failed at function %s line %d!!!\n", __FUNCTION__, __LINE__);\
			return FAILED;\
	}\
}while(0)

#if 1
enum ENUM_NETDEC_POLICY
{
	NETIF_VLAN_BASED = 0x0,   /* Net interface Multilayer-Decision-Based Control by VLAN Based. */
	NETIF_PORT_BASED = 0x1,   /* Net interface Multilayer-Decision-Based Control by PORT Based. */
	NETIF_MAC_BASED = 0x2,    /* Net interface Multilayer-Decision-Based Control by MAC Based. */
};
enum ENUM_DEFACL_FOR_NETDECMISS
{
	PERMIT_FOR_NETDECMISS = 0x0, /*default permit when netif decision miss match*/
	DROP_FOR_NETDECMISS = 0x1,
	TOCPU_FOR_NETDECMISS = 0x2,
};
#endif

/* ======================================================
	ASIC Driver initiation parameters
    ====================================================== */
typedef struct rtl8651_tblAsic_InitPara_s {

/*			Add Parameters for ASIC initiation                  */
/* ===================================== */
	uint32	externalPHYProperty;
	uint32	externalPHYId[RTL8651_MAC_NUMBER];
/* ===================================== */

} rtl8651_tblAsic_InitPara_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    /* word 0 */
    uint32          mac18_0:19;
    uint32          vid		 : 12;
    uint32          valid       : 1;	
    /* word 1 */
#if defined(CONFIG_RTL_8198C)
    uint32          inACLStartL:1;
    uint32          enHWRouteV6    : 1;	
#else    
    uint32         inACLStartL:2;	
#endif
    uint32         enHWRoute : 1;	
    uint32         mac47_19:29;

    /* word 2 */
#if defined(CONFIG_RTL_8198C)
    uint32         macMaskL    :1;
    uint32         outACLEnd   :8;
    uint32         outACLStart :8;
    uint32         inACLEnd    :8;	
    uint32         inACLStartH :7;	
#else
    uint32         mtuL       : 3;
    uint32         macMask :3;	
    uint32         outACLEnd : 7;	
    uint32         outACLStart : 7;	
    uint32         inACLEnd : 7;	
    uint32         inACLStartH: 5;	
#endif

    /* word 3 */
#if defined(CONFIG_RTL_8198C)
    uint32         mtuV6       :15;
    uint32         mtu         :15;
    uint32         macMaskH    :2;
#else
    uint32          reserv10   : 20;
    uint32          mtuH       : 12;
#endif

#else /*_LITTLE_ENDIAN*/
    /* word 0 */
    uint32          valid       : 1;	
    uint32          vid		 : 12;
    uint32          mac18_0:19;

    /* word 1 */
    uint32         mac47_19:29;
    uint32          enHWRoute      : 1;	
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32          enHWRouteV6    : 1;	
    uint32          inACLStartL:1;
#else
    uint32          inACLStartL:2;	
#endif


    /* word 2 */
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32         inACLStartH :7;	
    uint32         inACLEnd    :8;	
    uint32         outACLStart :8;
    uint32         outACLEnd   :8;
    uint32         macMaskL    :1;
#else
    uint32         inACLStartH : 5;	
    uint32         inACLEnd : 7;	
    uint32         outACLStart : 7;
    uint32         outACLEnd : 7;	
    uint32         macMask :3;
    uint32         mtuL       : 3;
#endif

    /* word 3 */
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32         macMaskH    :2;
    uint32         mtu         :15;
    uint32         mtuV6       :15;
#else
    uint32          mtuH       : 12;
    uint32          reserv10   : 20;
#endif

#endif /*_LITTLE_ENDIAN*/
    /* word 4 */
    uint32          reservw4;
    /* word 5 */
    uint32          reservw5;
    /* word 6 */
    uint32          reservw6;
    /* word 7 */
    uint32          reservw7;
} rtl865xc_tblAsic_netifTable_t;



typedef struct {
#ifndef _LITTLE_ENDIAN
	 /* word 0 */
#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && !defined(CONFIG_RTL_8198C)
	uint32	vid:12;
#else
	uint32	reserved1:12;
#endif
	uint32	fid:2;
	uint32     extEgressUntag  : 3;
	uint32     egressUntag : 6;
	uint32     extMemberPort   : 3;
	uint32     memberPort  : 6;
#else /*_LITTLE_ENDIAN*/
	/* word 0 */
	
	uint32     memberPort  : 6;
	uint32     extMemberPort   : 3;
	uint32     egressUntag : 6;
	uint32     extEgressUntag  : 3;
	uint32	fid:2;
#if defined(CONFIG_RTL_8197F)
	uint32	hp:3;
	uint32	reserved2:9;
#else
	uint32	reserved1:12;
#endif

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
} rtl865xc_tblAsic_vlanTable_t;

typedef struct {
#ifndef _LITTLE_ENDIAN
    union {
        struct {
            /* word 0 */
            uint16          dMacP31_16;
            uint16          dMacP15_0;
            /* word 1 */
            uint16          dMacM15_0;
            uint16          dMacP47_32;
            /* word 2 */
            uint16          dMacM47_32;
            uint16          dMacM31_16;
            /* word 3 */
            uint16          sMacP31_16;
            uint16          sMacP15_0;
            /* word 4 */
            uint16          sMacM15_0;
            uint16          sMacP47_32;
            /* word 5 */
            uint16          sMacM47_32;
            uint16          sMacM31_16;
            /* word 6 */
            uint16          ethTypeM;
            uint16          ethTypeP;
        } ETHERNET;
        struct {
            /* word 0 */
            uint32          reserv1     : 24;
            uint32          gidxSel     : 8;
            /* word 1~6 */
            uint32          reserv2[6];
        } IFSEL;
        struct {
            /* word 0 */
            ipaddr_t        sIPP;
            /* word 1 */
            ipaddr_t        sIPM;
            /* word 2 */
            ipaddr_t        dIPP;
            /* word 3 */
            ipaddr_t        dIPM;
            union {
                struct {
                    /* word 4 */
                    uint8           IPProtoM;
                    uint8           IPProtoP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint32          reserv0     : 20;
                    uint32          identSDIPM  : 1;
                    uint32          identSDIPP  : 1;
                    uint32          HTTPM       : 1;
                    uint32          HTTPP       : 1;
                    uint32          FOM         : 1;
                    uint32          FOP         : 1;
                    uint32          IPFlagM     : 3;
                    uint32          IPFlagP     : 3;
                    /* word 6 */
                    uint32          reserv1;
                } IP;
                struct {
                    /* word 4 */
                    uint8           ICMPTypeM;
                    uint8           ICMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          reserv0;
                    uint8           ICMPCodeM;
                    uint8           ICMPCodeP;
                    /* word 6 */
                    uint32          reserv1;
                } ICMP;
                struct {
                    /* word 4 */
                    uint8           IGMPTypeM;
                    uint8           IGMPTypeP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5,6 */
                    uint32          reserv0[2];
                } IGMP;
                struct {
                    /* word 4 */
                    uint8           TCPFlagM;
                    uint8           TCPFlagP;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          TCPSPLB;
                    uint16          TCPSPUB;
                    /* word 6 */
                    uint16          TCPDPLB;
                    uint16          TCPDPUB;
                } TCP;
                struct {
                    /* word 4 */
                    uint16          reserv0;
                    uint8           IPTOSM;
                    uint8           IPTOSP;
                    /* word 5 */
                    uint16          UDPSPLB;
                    uint16          UDPSPUB;
                    /* word 6 */
                    uint16          UDPDPLB;
                    uint16          UDPDPUB;
                } UDP;
            } is;
        } L3L4;

#if defined(CONFIG_RTL_8198C)
        struct{
            union{
                struct{
                    /* word 0 */
                    uint32        sip_addr31_0;
                    /* word 1 */
                    uint32        sip_addr63_32;
                    /* word 2 */
                    uint32        sip_addr95_64;
                    /* word 3 */
                    uint32        sip_addr127_96;
                    /* word 4 */
                    uint32        sip_mask31_0;
                    /* word 5 */
                    uint32        sip_mask63_32;
                    /* word 6 */
                    uint32        sip_mask95_64;             
                }entry0;
                struct{
                    /* word 0 */
                    uint32        dip_addr31_0;
                    /* word 1 */
                    uint32        dip_addr63_32;
                    /* word 2 */
                    uint32        dip_addr95_64;
                    /* word 3 */
                    uint32        dip_addr127_96;
                    /* word 4 */
                    uint32        dip_mask31_0;
                    /* word 5 */
                    uint32        dip_mask63_32;
                    /* word 6 */
                    uint32        dip_mask95_64;     
                }entry1;
            }is;
        }L3V6;
#endif        
        struct {
            /* word 0 */
            uint16          sMacP31_16;
            uint16          sMacP15_0;
            /* word 1 */
            uint16          reserv1:3;
            uint16          spaP:9;
            uint16          sMacM3_0:4;
            uint16          sMacP47_32;
			/* word 2 */
		    uint32	        reserv3:2;
            uint32          sVidM:12;
		    uint32          sVidP:12;
		    uint32		    reserv2:6;
            /* word 3 */
            uint32          reserv5     : 6;
            uint32          protoType   : 2;
     	    uint32          reserv4        : 24;
			/* word 4 */
            ipaddr_t        sIPP;
            /* word 5 */
            ipaddr_t        sIPM;
            /* word 6 */
            uint16          SPORTLB;
            uint16          SPORTUB;
        } SRC_FILTER;
        struct {
            /* word 0 */
            uint16          dMacP31_16;
            uint16          dMacP15_0;
            /* word 1 */
		    uint16 	        vidP:12;	
            uint16          dMacM3_0:4;
            uint16          dMacP47_32;			
            /* word 2 */
		    uint32          reserv2:20;
		    uint32          vidM:12;			
            /* word 3 */
            uint32          reserv4     : 24;
            uint32          protoType   : 2;
		     uint32         reserv3:6;
            /* word 4 */
            ipaddr_t        dIPP;
            /* word 5 */
            ipaddr_t        dIPM;
            /* word 6 */
            uint16          DPORTLB;
            uint16          DPORTUB;
        } DST_FILTER;

    } is;
    /* word 7 */
#if defined(CONFIG_RTL_8198C)
    uint32          ip_tunnel   : 1;
    uint32          comb      : 1;
    uint32          ruleType1 : 1;
    uint32          ipv6ETY0  : 1;
    uint32          inv       : 1;
#else
    uint32          reserv0     : 5;
#endif
    uint32          pktOpApp    : 3;
    uint32          PPPoEIndex  : 3;
    uint32          vid         : 3;
    uint32          nextHop     : 10; //index of l2, next hop, or rate limit tables
    uint32          actionType  : 4;
    uint32          ruleType    : 4;

#if defined(CONFIG_RTL_8198C)
    struct{
        union{
            struct{
                /*word 8*/
                uint32 sip_mask119_96  :24;
                uint32 reserv1         :8;
                /*word 9*/
                uint32 flowLabelM3_0   :4;
                uint32 flowLabel       :20;
                uint32 sip_mask127_120 :8;
                /*word 10*/
                uint32 reserv2         :16;
                uint32 flowLabelM19_4  :16;
            }entry0;
            struct{
                /*word 8*/
                uint32 dip_mask119_96  :24;
                uint32 reserv1         :8;
                /*word 9*/
                uint32 nextHeader      :8;
                uint32 trafficClassM   :8;                
                uint32 trafficClass    :8;
                uint32 dip_mask127_120 :8;
                /*word 10*/             
                uint32 reserv2         :20;
                uint32 identSDIPM     :1;
                uint32 identSDIPP      :1;                 
                uint32 HTTPM          :1;
                uint32 HTTPP           :1;              
                uint32 nextHeaderM     :8;
            }entry1;
        }is;
    }ipv6;
#endif    
#else /* littlen endian*/
    union {
        struct {
            /* word 0 */
            uint16          dMacP15_0;
            uint16          dMacP31_16;
            /* word 1 */
            uint16          dMacP47_32;
            uint16          dMacM15_0;
            /* word 2 */
            uint16          dMacM31_16;
            uint16          dMacM47_32;
            /* word 3 */
            uint16          sMacP15_0;
            uint16          sMacP31_16;
            /* word 4 */
            uint16          sMacP47_32;
            uint16          sMacM15_0;
            /* word 5 */
            uint16          sMacM31_16;
            uint16          sMacM47_32;
            /* word 6 */
            uint16          ethTypeP;
            uint16          ethTypeM;
        } ETHERNET;
        struct {
            /* word 0 */
            uint32          gidxSel     : 8;
            uint32          reserv1     : 24;
            /* word 1~6 */
            uint32          reserv2[6];
        } IFSEL;
        struct {
            /* word 0 */
            ipaddr_t        sIPP;
            /* word 1 */
            ipaddr_t        sIPM;
            /* word 2 */
            ipaddr_t        dIPP;
            /* word 3 */
            ipaddr_t        dIPM;
            union {
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           IPProtoP;
                    uint8           IPProtoM;
                    /* word 5 */
                    uint32          IPFlagP     : 3;
                    uint32          IPFlagM     : 3;
                    uint32          FOP         : 1;
                    uint32          FOM         : 1;
                    uint32          HTTPP       : 1;
                    uint32          HTTPM       : 1;
                    uint32          identSDIPP  : 1;
                    uint32          identSDIPM  : 1;
                    uint32          reserv0     : 20;

                    /* word 6 */
                    uint32          reserv1;
                } IP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           ICMPTypeP;
                    uint8           ICMPTypeM;
                    /* word 5 */
                    uint8           ICMPCodeP;
                    uint8           ICMPCodeM;
                    uint16          reserv0;
                    /* word 6 */
                    uint32          reserv1;
                } ICMP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           IGMPTypeP;
                    uint8           IGMPTypeM;
                    /* word 5,6 */
                    uint32          reserv0[2];
                } IGMP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint8           TCPFlagP;
                    uint8           TCPFlagM;
                    /* word 5 */
                    uint16          TCPSPUB;
                    uint16          TCPSPLB;
                    /* word 6 */
                    uint16          TCPDPUB;
                    uint16          TCPDPLB;
                } TCP;
                struct {
                    /* word 4 */
                    uint8           IPTOSP;
                    uint8           IPTOSM;
                    uint16          reserv0;
                    /* word 5 */
                    uint16          UDPSPUB;
                    uint16          UDPSPLB;
                    /* word 6 */
                    uint16          UDPDPUB;
                    uint16          UDPDPLB;
                } UDP;
            } is;
        } L3L4;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
        struct{
            union{
                struct{
                    /* word 0 */
                    uint32        sip_addr31_0;
                    /* word 1 */
                    uint32        sip_addr63_32;
                    /* word 2 */
                    uint32        sip_addr95_64;
                    /* word 3 */
                    uint32        sip_addr127_96;
                    /* word 4 */
                    uint32        sip_mask31_0;
                    /* word 5 */
                    uint32        sip_mask63_32;
                    /* word 6 */
                    uint32        sip_mask95_64;             
                }entry0;
                struct{
                    /* word 0 */
                    uint32        dip_addr31_0;
                    /* word 1 */
                    uint32        dip_addr63_32;
                    /* word 2 */
                    uint32        dip_addr95_64;
                    /* word 3 */
                    uint32        dip_addr127_96;
                    /* word 4 */
                    uint32        dip_mask31_0;
                    /* word 5 */
                    uint32        dip_mask63_32;
                    /* word 6 */
                    uint32        dip_mask95_64;     
                }entry1;
            }is;
        }L3V6;
#endif

        struct {
            /* word 0 */
            uint16          sMacP15_0;
            uint16          sMacP31_16;
            /* word 1 */
            uint16          sMacP47_32;
            uint16          sMacM3_0:4;
	     uint16		   spaP:9;
	     uint16           reserv1:3;
            /* word 2 */
	     uint32		   reserv2:6;
	     uint32          sVidP:12;
            uint32          sVidM:12;
	     uint32	          reserv3:2;
	     
            /* word 3 */
            uint32          reserv4        : 24;
            uint32          protoType   : 2;
            uint32          reserv5     : 6;
            /* word 4 */
            ipaddr_t        sIPP;
            /* word 5 */
            ipaddr_t        sIPM;
            /* word 6 */
            uint16          SPORTUB;
            uint16          SPORTLB;
        } SRC_FILTER;
        struct {
            /* word 0 */
            uint16          dMacP15_0;
            uint16          dMacP31_16;

            /* word 1 */
            uint16          dMacP47_32;			
            uint16          dMacM3_0:4;
	     uint16 	   vidP:12;	
            /* word 2 */
	     uint32          vidM:12;			
	     uint32          reserv2:20;
            /* word 3 */
	     uint32          reserv3:6;
            uint32          protoType   : 2;
            uint32          reserv4     : 24;
            /* word 4 */
            ipaddr_t        dIPP;
            /* word 5 */
            ipaddr_t        dIPM;
            /* word 6 */
            uint16          DPORTUB;
            uint16          DPORTLB;
        } DST_FILTER;
#if defined(CONFIG_RTL_8197F)
		struct {
			union{
				struct{
					/* word 0 */            
					uint32  spub:16; ////sport upper bound
					uint32  splb:16; ////sport lower bound
					/* word 1 */
					uint32  dpub:16; ////dport upper bound
					uint32  dplb:16; ////dport lower bound
                    
					/* word 2 */
					uint32  protocol :2; 
					uint32  rsv      :6;
					uint32  offset0  :7;
					uint32  pattern0 :16;
					uint32  pm0_0    :1;

					/* word 3 */
					uint32  pm0_15_1     :15;
					uint32  or0      :1;
					uint32  offset1  :7;
					uint32  pattern1_8_0 :9; 
            
					/* word 4 */
					uint32  pattern1_15_9 :7; 
					uint32  pm1           :16;        
					uint32  or1          :1;
					uint32  offset2      :7;            
					uint32  pattern2_0    :1;

					/* word 5 */
					uint32  pattern2_15_1 :15;
					uint32  pm2          :16;
					uint32  or2          :1;

					/* word 6 */
					uint32  rsv1:32;             
				} entry0;
				struct{
					/* word 0 */
					uint32  offset4   :7;
					uint32  pattern4  :16;
					uint32  pm4_8_0   :9;

					/* word 1 */
					uint32  pm4_15_9      :7;
					uint32  or4           :1;            
					uint32  offset5  :7; 
					uint32  pattern5 :16; 
					uint32  pm5_0      :1;  

					/* word 2 */
					uint32  pm5_15_1     :15;
					uint32  or5          :1;
					uint32  offset6      :7; 
					uint32  pattern6_8_0 :9; 

					/* word 3 */
					uint32  pattern6_15_9 :7; 
					uint32  pm6           :16;        
					uint32  or6           :1;
					uint32  offset7       :7; 
					uint32  pattern7_0    :1;

					/* word 4 */
					uint32  pattern7_15_1 :15;
					uint32  pm7           :16;  
					uint32  or7           :1;

					/* word 5 */
					uint32  specialop     :1;
					uint32  spa           :9;
					uint32  spam          :9;
					uint32  pppctl        :1;
					uint32  pppctlm       :1;
					uint32  pppctl_or     :1;
					uint32  rsv2          :10;
					/* word 6 */
					uint32        rsv3;     
				}entry1;
			}is;
		}PM_U;//pattern match rule up        
#endif       
    } is;
    /* word 7 */

    uint32          ruleType    : 4;
    uint32          actionType  : 4;
    uint32          nextHop     : 10; //index of l2, next hop, or rate limit tables
    uint32          vid         : 3;
    uint32          PPPoEIndex  : 3;
    uint32          pktOpApp    : 3;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32          inv       : 1;
    uint32          ipv6ETY0  : 1;
    uint32          ruleType1 : 1;
    uint32          comb      : 1;
    uint32          ip_tunnel : 1;
#else
    uint32          reserv0     : 5;
#endif

#if defined(CONFIG_RTL_8198C)
    struct{
        union{
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 sip_mask119_96  :24;
                /*word 9*/
                uint32 sip_mask127_120 :8;
                uint32 flowLabel       :20;
                uint32 flowLabelM3_0   :4;
                /*word 10*/
                uint32 flowLabelM19_4  :16;
                uint32 reserv2         :16;
            }entry0;
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 dip_mask119_96  :24;
                /*word 9*/
                uint32 dip_mask127_120 :8;
                uint32 trafficClass    :8;
                uint32 trafficClassM   :8;                
                uint32 nextHeader      :8;
                /*word 10*/             
                uint32 nextHeaderM     :8;
                uint32 HTTPP           :1;
                uint32 HTTPM          :1;
                uint32 identSDIPP      :1;                
                uint32 identSDIPM     :1;
                uint32 reserv2         :20;
            }entry1;
        }is;
    }ipv6;
#endif

#if defined(CONFIG_RTL_8197F)
    union {
    struct{
        union{
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 sip_mask119_96  :24;
                /*word 9*/
                uint32 sip_mask127_120 :8;
                uint32 flowLabel       :20;
                uint32 flowLabelM3_0   :4;
                /*word 10*/
                uint32 flowLabelM19_4  :16;
                uint32 reserv2         :16;
            }entry0;
            struct{
                /*word 8*/
                uint32 reserv1         :8;
                uint32 dip_mask119_96  :24;
                /*word 9*/
                uint32 dip_mask127_120 :8;
                uint32 trafficClass    :8;
                uint32 trafficClassM   :8;                
                uint32 nextHeader      :8;
                /*word 10*/             
                uint32 nextHeaderM     :8;
                uint32 HTTPP           :1;
                uint32 HTTPM          :1;
                uint32 identSDIPP      :1;                
                uint32 identSDIPM     :1;
                uint32 reserv2         :20;
            }entry1;
            }is;
        }ipv6;
    struct {
        union{
            struct{
                /* word 8 */            
                uint32 offset3  :7;
                uint32 pattern3 :16;
                uint32 pm3_8_0  :9;

                /* word 9 */
                uint32 pm3_15_9 :7;
                uint32 or3      :1;
                uint32 rsv      :24;                    

                /* word 10 */
                uint32 rsv1  ;
            }entry0;
            struct{
                /*word 8*/
                uint32 nexthopaddr2 :1;
                uint32 ipfrag_apply :1; ////todo maybe better coding style here.
                uint32 rsv          :30;
                /*word 9*/
                uint32 rsv1;                    

                /*word 10*/
                uint32 rsv2;        
            }entry1;
        }is;
    }PM_D;//pattern match rule down part
    } is_d;
#endif

#endif /*_LITTLE_ENDIAN*/
} rtl865xc_tblAsic_aclTable_t;

typedef struct rtl865x_tblAsicDrv_vlanParam_s {
	uint32 	memberPortMask; /*extension ports [rtl8651_totalExtPortNum-1:0] are located at bits [RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum-1:RTL8651_PORT_NUMBER]*/
	uint32 	untagPortMask; /*extension ports [rtl8651_totalExtPortNum-1:0] are located at bits [RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum-1:RTL8651_PORT_NUMBER]*/
	uint32  fid:2;

#if defined(CONFIG_RTL_8197F)
	uint32  hp:3;
#elif (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)) && !defined(CONFIG_RTL_8198C)
	uint32  vid:12;
#endif
} rtl865x_tblAsicDrv_vlanParam_t;

typedef struct rtl865x_tblAsicDrv_intfParam_s {
	ether_addr_t macAddr;
	uint16 	macAddrNumber;
	uint16 	vid;
	uint32 	inAclStart, inAclEnd, outAclStart, outAclEnd;
	uint32 	mtu;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    uint32  mtuV6;
#endif        
	uint32 	enableRoute:1;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	uint32  enableRouteV6:1;
#endif    
	uint32  valid:1;

} rtl865x_tblAsicDrv_intfParam_t;


#define ASIC_IN_COUNTERS 					0x01
#define ASIC_OUT_COUNTERS 					0x02
#define ASIC_WHOLE_SYSTEM_COUNTERS 		0x04

typedef struct rtl865x_tblAsicDrv_basicCounterParam_s {
	uint32	mbr;
	uint32	txPackets;
	uint32	txBytes;
	uint32	rxPackets;
	uint32	rxBytes;
	uint32	rxErrors;
	uint32	drops;
	uint32	cpus;
} rtl865x_tblAsicDrv_basicCounterParam_t;

typedef struct rtl865x_tblAsicDrv_simpleCounterParam_s 
{
	uint64	rxBytes;
	uint32	rxPkts;
	uint32	rxPausePkts;
	
	uint32	drops;
	
	uint64	txBytes;
	uint32	txPkts;
	uint32	txPausePkts;
	
} rtl865x_tblAsicDrv_simpleCounterParam_t;

typedef struct rtl865x_tblAsicDrv_advancedCounterParam_s 
{
	/*here is in counters  definition*/
	uint64 ifInOctets;
	uint32 ifInUcastPkts;
	uint64 etherStatsOctets;
	uint32 etherStatsUndersizePkts;
	uint32 etherStatsFraments;
	uint32 etherStatsPkts64Octets;
	uint32 etherStatsPkts65to127Octets;
	uint32 etherStatsPkts128to255Octets;
	uint32 etherStatsPkts256to511Octets;
	uint32 etherStatsPkts512to1023Octets;
	uint32 etherStatsPkts1024to1518Octets;
	uint32 etherStatsOversizePkts;
	uint32 etherStatsJabbers;
	uint32 etherStatsMulticastPkts;
	uint32 etherStatsBroadcastPkts;
	uint32 dot1dTpPortInDiscards;
	uint32 etherStatusDropEvents;
	uint32 dot3FCSErrors;
	uint32 dot3StatsSymbolErrors;
	uint32 dot3ControlInUnknownOpcodes;
	uint32 dot3InPauseFrames;

	/*here is out counters  definition*/
	uint64 ifOutOctets;
	uint32 ifOutUcastPkts;
	uint32 ifOutMulticastPkts;
	uint32 ifOutBroadcastPkts;
	uint32 ifOutDiscards;
	uint32 dot3StatsSingleCollisionFrames;
	uint32 dot3StatsMultipleCollisionFrames;
	uint32 dot3StatsDefferedTransmissions;
	uint32 dot3StatsLateCollisions;
	uint32 dot3StatsExcessiveCollisions;
	uint32 dot3OutPauseFrames;
	uint32 dot1dBasePortDelayExceededDiscards;
	uint32 etherStatsCollisions;

	/*here is whole system couters definition*/
	uint32 dot1dTpLearnedEntryDiscards;
	uint32 etherStatsCpuEventPkts;
	
}rtl865x_tblAsicDrv_advancedCounterParam_t;

extern rtl8651_tblAsic_InitPara_t rtl8651_tblAsicDrvPara;

int32 rtl8651_clearAsicCommTable(void);
/*vlan*/
#if (defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E))  && !defined(CONFIG_RTL_8198C)
int rtl8651_findAsicVlanIndexByVid(uint16 *vid);
#endif
int32 rtl8651_setAsicVlan(uint16 vid, rtl865x_tblAsicDrv_vlanParam_t *vlanp);
int32 rtl8651_getAsicVlan(uint16 vid, rtl865x_tblAsicDrv_vlanParam_t *vlanp);
int32 rtl8651_delAsicVlan(uint16 vid);
int32 rtl8651_setAsicPvid(uint32 port, uint32 pvid);
int32 rtl8651_getAsicPVlanId(uint32 port, uint32 *pvid);
int32 rtl8651_setPortToNetif(uint32 port, uint32 netifidx);

int32 rtl8651_defineProtocolBasedVLAN( uint32 ruleNo, uint8 ProtocolType, uint16 ProtocolValue );
int32 rtl8651_setProtocolBasedVLAN( uint32 ruleNo, uint32 port, uint8 valid, uint16 vlanId);
int32 rtl8651_getProtocolBasedVLAN( uint32 ruleNo, uint32 port, uint8* valid, uint32* vlanIdx );

/*netif*/
int32 rtl8651_getAsicNetInterface(uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp);
int32 rtl8651_setAsicNetInterface(uint32 idx, rtl865x_tblAsicDrv_intfParam_t *intfp);
int32 rtl865x_delNetInterfaceByVid(uint16 vid);
int32 rtl865xC_setNetDecisionPolicy( enum ENUM_NETDEC_POLICY policy );
int32 rtl865x_setDefACLForNetDecisionMiss(uint8 start_ingressAclIdx, uint8 end_ingressAclIdx,uint8 start_egressAclIdx,uint8 end_egressAclIdx);
#if 1 //defined (CONFIG_RTL_LOCAL_PUBLIC)
int32 rtl865xC_getNetDecisionPolicy( enum ENUM_NETDEC_POLICY *policy );
int32 rtl865x_getDefACLForNetDecisionMiss(uint8 *start_ingressAclIdx, uint8 *end_ingressAclIdx,uint8 *start_egressAclIdx,uint8 *end_egressAclIdx);
#endif
/*operation layer*/
int32 rtl8651_getAsicOperationLayer(void);
int32 rtl8651_setAsicOperationLayer(uint32 layer);

/*switch core*/
int32 rtl865xC_lockSWCore(void);
int32 rtl865xC_unLockSWCore(void);
int32 rtl8651_getChipVersion(int8 *name,uint32 size, int32 *rev);
int32 rtl8651_getChipNameID(int32 *id);

void rtl8651_clearRegister(void);
void rtl8651_clearSpecifiedAsicTable(uint32 type, uint32 count);


int32 rtl8651_setAsicAgingFunction(int8 l2Enable, int8 l4Enable);
int32 rtl8651_setAsicAgingFunction(int8 l2Enable, int8 l4Enable);

unsigned int rtl865x_probeSdramSize(void);
void rtl865x_start(void);
void rtl865x_down(void);

/*counter*/
uint32 rtl8651_returnAsicCounter(uint32 offset);
uint64 rtl865xC_returnAsicCounter64(uint32 offset);
int32 rtl8651_clearAsicCounter(void);
#ifdef CONFIG_RTL_PROC_NEW
int32 rtl865xC_dumpAsicDiagCounter(struct seq_file *s);
#else
int32 rtl865xC_dumpAsicDiagCounter(char *page, int *len);
#endif

int32 rtl865xC_dumpAsicCounter(void);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
extern int rtl819x_setSwEthPvid(uint16 port, uint16 pvid);
extern int rtl819x_getSwEthPvid(uint16 port, uint16* pvid);
#endif

//#define EN_STROM_CTRL_BEFORE_SYS_SETTLED  1
//#define EN_RTL_INTERRUPT_MIGRATION		1
#define LOW_FIFO_MARK_VLAUE			(0xA0)
#define HIGH_FIFO_MARK_VLAUE		(0xA0)

#endif
