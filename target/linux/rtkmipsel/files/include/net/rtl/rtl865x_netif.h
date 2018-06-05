/*
* Copyright c                  Realtek Semiconductor Corporation, 2008
* All rights reserved.
*
* Program : network interface driver header file
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)
*/

#ifndef RTL865X_NETIF_H
#define RTL865X_NETIF_H
#include "rtl_types.h"
#include <linux/seq_file.h>


#define 	IF_NONE 0
#define	IF_ETHER 1
#define	IF_PPPOE 2
#define	IF_PPTP 3
#define	IF_L2TP 4
#define IF_6RD	5
#define IF_DSLT	6

#define RTL865X_ACL_IGMP_USED		-30003
#if defined(CONFIG_RTL_INBAND_CTL_ACL)
#define RTL865X_ACL_TEST_USED		-30002
#endif
#if defined(CONFIG_RTL_DNS_TRAP)
#define RTL865X_ACL_DNSTRAP_USED	-30001
#endif
#define RTL865X_ACL_IPV6_USED		-30000
#define RTL865X_ACL_QOS_USED2		-20002		/* dummy queue for iptable 2 acl translate */
#define RTL865X_ACL_QOS_USED1		-20000		/* for default queue */
#define RTL865X_ACL_QOS_USED0		-20001		/* for user add queue */
#if defined(CONFIG_RTL_HTTP_REDIRECT_LOCAL)
#define RTL865X_ACL_HTTP_REDIRECT_USED	-14000	/*for http redirect local*/
#endif
#define RTL865X_ACL_MULTIWAN_USED	-15000	/*for multiwan*/
#define RTL865X_ACL_USER_USED		0

#if 1 //def CONFIG_RTL_LAYERED_DRIVER_ACL
typedef struct _rtl865x_AclRule_s
{
	union
	{
		/* MAC ACL rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _typeLen, _typeLenMask;
		} MAC;

		/* IP Group ACL rule */
		struct
		{
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint8 _tos, _tosMask;
			union
			{
				/* IP ACL rle */
				struct
				{
					uint8 _proto, _protoMask, _flagMask;// flag & flagMask only last 3-bit is meaning ful
					uint32 _FOP:1, _FOM:1, _httpFilter:1, _httpFilterM:1, _identSrcDstIp:1, _identSrcDstIpM:1;
					union
					{
						uint8 _flag;
						struct
						{
							uint8 pend1:5,
								 pend2:1,
								 _DF:1,	//don't fragment flag
								 _MF:1;	//more fragments flag
						} s;
					} un;
				} ip;

				/* ICMP ACL rule */
				struct
				{
					uint8 _type, _typeMask, _code, _codeMask;
				} icmp;

				/* IGMP ACL rule */
				struct
				{
					uint8 _type, _typeMask;
				} igmp;

				/* TCP ACL rule */
				struct
				{
					uint8 _flagMask;
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;
					union
					{
						uint8 _flag;
						struct
						{
							uint8 _pend:2,
								  _urg:1, //urgent bit
								  _ack:1, //ack bit
								  _psh:1, //push bit
								  _rst:1, //reset bit
								  _syn:1, //sync bit
								  _fin:1; //fin bit
						}s;
					}un;
				}tcp;

				/* UDP ACL rule */
				struct
				{
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;
				}udp;
			}is;
		}L3L4;


#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
		struct
		{        
			inv6_addr_t _srcIpV6Addr, _srcIpV6AddrMask;
			inv6_addr_t _dstIpV6Addr, _dstIpV6AddrMask;
			uint32 _INV:1,_ETY:1, _comb:1,_ip_tunnel:1 ,_httpFilter:1, _httpFilterM:1, _identSrcDstIp:1, _identSrcDstIpM:1;
			uint32 _flowLabel, _flowLabelMask;
			uint8 _trafficClass, _trafficClassMask;
			uint8 _nextheader, _nextheaderMask;
		}L3V6;
 #endif       
		/* Source filter ACL rule */
		struct
		{
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _srcPort, _srcPortMask;
			uint16 _srcVlanIdx, _srcVlanIdxMask;
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			uint16 _srcPortUpperBound, _srcPortLowerBound;
			uint32 _ignoreL4:1, //L2 rule
				  	 _ignoreL3L4:1; //L3 rule
		} SRCFILTER;

		/* Destination filter ACL rule */
		struct
		{
			ether_addr_t _dstMac, _dstMacMask;
			uint16 _vlanIdx, _vlanIdxMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint16 _dstPortUpperBound, _dstPortLowerBound;
			uint32 _ignoreL4:1, //L3 rule
				   _ignoreL3L4:1; //L2 rule
		} DSTFILTER;
#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT) ||defined(CONFIG_RTL_MULTIPLE_WAN)  ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
		struct {
			uint8	vlanTagPri;
		} VLANTAG;
#endif

#if defined(CONFIG_RTL_8197F)
		/* Pattern Match ACL rule */
		struct {
			uint16 _spub, _splb ,_dpub, _dplb ;
			uint16 _protocol, _specialop ,_spa, _spam ;
			uint16  _pppctl ,_pppctlm, _pppctlor ;
			uint16 _offset0, _pattern0 ,_pm0, _or0 ;
			uint16 _offset1, _pattern1 ,_pm1, _or1 ;
			uint16 _offset2, _pattern2 ,_pm2, _or2 ;
			uint16 _offset3, _pattern3 ,_pm3, _or3 ;
			uint16 _offset4, _pattern4 ,_pm4, _or4 ;
			uint16 _offset5, _pattern5 ,_pm5, _or5 ;
			uint16 _offset6, _pattern6 ,_pm6, _or6 ;
			uint16 _offset7, _pattern7 ,_pm7, _or7 ;
		} PM;
#endif

	}un_ty;

	uint32	ruleType_:5,
			actionType_:4,
			pktOpApp_:3,
			priority_:3,
			direction_:2,
#if defined(CONFIG_RTL_HW_QOS_SUPPORT) ||defined(CONFIG_RTL_MULTIPLE_WAN)  ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
			upDown_:1,//0: uplink acl rule for hw qos; 1: downlink acl rule for hw qos
#endif
#if defined(CONFIG_RTL_8197F)
			_INV:1,_ETY:1, _comb:1,_ip_tunnel:1,
#endif
			nexthopIdx_:5, /* Index of nexthop table (NOT L2 table) */	/* used as network interface index for 865xC qos system */
			ratelimtIdx_:4; /* Index of rate limit table */	/* used as outputQueue index for 865xC qos system */

	uint32	netifIdx_:3, /*for redirect*/
			pppoeIdx_:3, /*for redirect*/
			L2Idx_:10, /* Index of L2 table */
			inv_flag:8, /*mainly for iptables-->acl rule, when iptables rule has invert netif flag, this acl rule is added to other netifs*/
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
			aclIdx:8;	/* aisc entry idx */
#else
			aclIdx:7;	/* aisc entry idx */
#endif

#if defined(CONFIG_RTL_8197F)
	uint32 aclvid_:12, /* vid if ActionType =1111 */
			aclfid_:2, /*  fid if ActionType =1111 */
			ipfrag_apply_:1;
#elif defined(CONFIG_RTL_8198C)
	uint32 aclvid_:12; /* vid if ActionType =1111 */
#endif
	struct _rtl865x_AclRule_s *pre,*next;

}rtl865x_AclRule_t;


/* MAC ACL rule Definition */
#define dstMac_				un_ty.MAC._dstMac
#define dstMacMask_			un_ty.MAC._dstMacMask
#define srcMac_				un_ty.MAC._srcMac
#define srcMacMask_			un_ty.MAC._srcMacMask
#define typeLen_				un_ty.MAC._typeLen
#define typeLenMask_			un_ty.MAC._typeLenMask

/* Common IP ACL Rule Definition */
#define srcIpAddr_				un_ty.L3L4._srcIpAddr
#define srcIpAddrMask_			un_ty.L3L4._srcIpAddrMask
#define srcIpAddrUB_				un_ty.L3L4._srcIpAddr
#define srcIpAddrLB_				un_ty.L3L4._srcIpAddrMask
#define dstIpAddr_				un_ty.L3L4._dstIpAddr
#define dstIpAddrMask_			un_ty.L3L4._dstIpAddrMask
#define dstIpAddrUB_				un_ty.L3L4._dstIpAddr
#define dstIpAddrLB_				un_ty.L3L4._dstIpAddrMask
#define tos_					un_ty.L3L4._tos
#define tosMask_				un_ty.L3L4._tosMask
/* IP Rrange */
/*Hyking:Asic use Addr to srore Upper address
	and use Mask to store Lower address
*/
#define srcIpAddrStart_			un_ty.L3L4._srcIpAddrMask
#define srcIpAddrEnd_			un_ty.L3L4._srcIpAddr
#define dstIpAddrStart_			un_ty.L3L4._dstIpAddrMask
#define dstIpAddrEnd_			un_ty.L3L4._dstIpAddr

/* IP ACL Rule Definition */
#define ipProto_				un_ty.L3L4.is.ip._proto
#define ipProtoMask_			un_ty.L3L4.is.ip._protoMask
#define ipFlagMask_			un_ty.L3L4.is.ip._flagMask
#define ipFOP_      				un_ty.L3L4.is.ip._FOP
#define ipFOM_      				un_ty.L3L4.is.ip._FOM
#define ipHttpFilter_      			un_ty.L3L4.is.ip._httpFilter
#define ipHttpFilterM_			un_ty.L3L4.is.ip._httpFilterM
#define ipIdentSrcDstIp_   		un_ty.L3L4.is.ip._identSrcDstIp
#define ipIdentSrcDstIpM_		un_ty.L3L4.is.ip._identSrcDstIpM
#define ipFlag_					un_ty.L3L4.is.ip.un._flag
#define ipDF_					un_ty.L3L4.is.ip.un.s._DF
#define ipMF_					un_ty.L3L4.is.ip.un.s._MF

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
/* IPv6 ACL Rule Definition */
#define srcIpV6Addr_			un_ty.L3V6._srcIpV6Addr
#define srcIpV6AddrMask_		un_ty.L3V6._srcIpV6AddrMask
#define srcIpV6AddrUB_			un_ty.L3V6._srcIpV6Addr
#define srcIpV6AddrLB_			un_ty.L3V6._srcIpV6AddrMask
#define dstIpV6Addr_			un_ty.L3V6._dstIpV6Addr
#define dstIpV6AddrMask_		un_ty.L3V6._dstIpV6AddrMask
#define dstIpV6AddrUB_			un_ty.L3V6._dstIpV6Addr
#define dstIpV6AddrLB_			un_ty.L3V6._dstIpV6AddrMask
#define ipv6HttpFilter_      	un_ty.L3V6._httpFilter
#define ipv6HttpFilterM_	    un_ty.L3V6._httpFilterM
#define ipv6IdentSrcDstIp_   	un_ty.L3V6._identSrcDstIp
#define ipv6IdentSrcDstIpM_		un_ty.L3V6._identSrcDstIpM
#define ipv6FlowLabel_      	un_ty.L3V6._flowLabel
#define ipv6FlowLabelM_	        un_ty.L3V6._flowLabelMask
#define ipv6TrafficClass_      	un_ty.L3V6._trafficClass
#define ipv6TrafficClassM_	    un_ty.L3V6._trafficClassMask
#define ipv6NextHeader_      	un_ty.L3V6._nextheader
#define ipv6NextHeaderM_	    un_ty.L3V6._nextheaderMask
#endif

#if defined(CONFIG_RTL_8198C)
#define ipv6Invert_      	    un_ty.L3V6._INV
#define ipv6EntryType_     	    un_ty.L3V6._ETY
#define ipv6Combine_	        un_ty.L3V6._comb
#define ipv6IPtunnel_	        un_ty.L3V6._ip_tunnel
#elif defined(CONFIG_RTL_8197F)
#define ipv6Invert_      	    _INV
#define ipv6EntryType_     	    _ETY
#define ipv6Combine_	        _comb
#define ipv6IPtunnel_	        _ip_tunnel
#endif

/* ICMP ACL Rule Definition */
#define icmpType_				un_ty.L3L4.is.icmp._type
#define icmpTypeMask_			un_ty.L3L4.is.icmp._typeMask
#define icmpCode_				un_ty.L3L4.is.icmp._code
#define icmpCodeMask_			un_ty.L3L4.is.icmp._codeMask

/* IGMP ACL Rule Definition */
#define igmpType_				un_ty.L3L4.is.igmp._type
#define igmpTypeMask_			un_ty.L3L4.is.igmp._typeMask

/* TCP ACL Rule Definition */
#define tcpSrcPortUB_			un_ty.L3L4.is.tcp._srcPortUpperBound
#define tcpSrcPortLB_			un_ty.L3L4.is.tcp._srcPortLowerBound
#define tcpDstPortUB_			un_ty.L3L4.is.tcp._dstPortUpperBound
#define tcpDstPortLB_			un_ty.L3L4.is.tcp._dstPortLowerBound
#define tcpFlagMask_			un_ty.L3L4.is.tcp._flagMask
#define tcpFlag_				un_ty.L3L4.is.tcp.un._flag
#define tcpURG_				un_ty.L3L4.is.tcp.un.s._urg
#define tcpACK_				un_ty.L3L4.is.tcp.un.s._ack
#define tcpPSH_				un_ty.L3L4.is.tcp.un.s._psh
#define tcpRST_				un_ty.L3L4.is.tcp.un.s._rst
#define tcpSYN_				un_ty.L3L4.is.tcp.un.s._syn
#define tcpFIN_				un_ty.L3L4.is.tcp.un.s._fin

/* UDP ACL Rule Definition */
#define udpSrcPortUB_			un_ty.L3L4.is.udp._srcPortUpperBound
#define udpSrcPortLB_			un_ty.L3L4.is.udp._srcPortLowerBound
#define udpDstPortUB_			un_ty.L3L4.is.udp._dstPortUpperBound
#define udpDstPortLB_			un_ty.L3L4.is.udp._dstPortLowerBound

/* Source Filter ACL Rule Definition */
#define srcFilterMac_				un_ty.SRCFILTER._srcMac
#define srcFilterMacMask_		un_ty.SRCFILTER._srcMacMask
#define srcFilterPort_				un_ty.SRCFILTER._srcPort
#define srcFilterPortMask_		un_ty.SRCFILTER._srcPortMask
#define srcFilterVlanIdx_			un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanId_			un_ty.SRCFILTER._srcVlanIdx
#define srcFilterVlanIdxMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterVlanIdMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define srcFilterIpAddr_			un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrMask_		un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterIpAddrUB_		un_ty.SRCFILTER._srcIpAddr
#define srcFilterIpAddrLB_		un_ty.SRCFILTER._srcIpAddrMask
#define srcFilterPortUpperBound_	un_ty.SRCFILTER._srcPortUpperBound
#define srcFilterPortLowerBound_	un_ty.SRCFILTER._srcPortLowerBound
#define srcFilterIgnoreL3L4_		un_ty.SRCFILTER._ignoreL3L4
#define srcFilterIgnoreL4_		un_ty.SRCFILTER._ignoreL4

/* Destination Filter ACL Rule Definition */
#define dstFilterMac_				un_ty.DSTFILTER._dstMac
#define dstFilterMacMask_		un_ty.DSTFILTER._dstMacMask
#define dstFilterVlanIdx_			un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdxMask_		un_ty.DSTFILTER._vlanIdxMask
#define dstFilterVlanId_			un_ty.DSTFILTER._vlanIdx
#define dstFilterVlanIdMask_		un_ty.DSTFILTER._vlanIdxMask
#define dstFilterIpAddr_			un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrMask_		un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortUpperBound_	un_ty.DSTFILTER._dstPortUpperBound
#define dstFilterIpAddrUB_		un_ty.DSTFILTER._dstIpAddr
#define dstFilterIpAddrLB_		un_ty.DSTFILTER._dstIpAddrMask
#define dstFilterPortLowerBound_	un_ty.DSTFILTER._dstPortLowerBound
#define dstFilterIgnoreL3L4_		un_ty.DSTFILTER._ignoreL3L4
#define dstFilterIgnoreL4_		un_ty.DSTFILTER._ignoreL4
#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
#define vlanTagPri_			un_ty.VLANTAG.vlanTagPri
#endif

#if defined(CONFIG_RTL_8197F)
#define pmSPUB_				    un_ty.PM._spub
#define pmSPLB_				    un_ty.PM._splb
#define pmDPUB_		            un_ty.PM._dpub
#define pmDPLB_				    un_ty.PM._dplb
#define pmProtocol_				un_ty.PM._protocol
#define pmSpecialop_		    un_ty.PM._specialop
#define pmSPA_		            un_ty.PM._spa
#define pmSPAM_				    un_ty.PM._spam
#define pmPPPCTL_    		    un_ty.PM._pppctl
#define pmPPPCTLM_	            un_ty.PM._pppctlm
#define pmPPPCTLOR_				un_ty.PM._pppctlor
#define pmOffset0_				un_ty.PM._offset0
#define pmPattern0_				un_ty.PM._pattern0
#define pmPatternMask0_		    un_ty.PM._pm0
#define pmOR0_				    un_ty.PM._or0
#define pmOffset1_				un_ty.PM._offset1
#define pmPattern1_				un_ty.PM._pattern1
#define pmPatternMask1_		    un_ty.PM._pm1
#define pmOR1_				    un_ty.PM._or1
#define pmOffset2_				un_ty.PM._offset2
#define pmPattern2_				un_ty.PM._pattern2
#define pmPatternMask2_		    un_ty.PM._pm2
#define pmOR2_				    un_ty.PM._or2
#define pmOffset3_				un_ty.PM._offset3
#define pmPattern3_				un_ty.PM._pattern3
#define pmPatternMask3_		    un_ty.PM._pm3
#define pmOR3_				    un_ty.PM._or3
#define pmOffset4_				un_ty.PM._offset4
#define pmPattern4_				un_ty.PM._pattern4
#define pmPatternMask4_		    un_ty.PM._pm4
#define pmOR4_				    un_ty.PM._or4
#define pmOffset5_				un_ty.PM._offset5
#define pmPattern5_				un_ty.PM._pattern5
#define pmPatternMask5_		    un_ty.PM._pm5
#define pmOR5_				    un_ty.PM._or5
#define pmOffset6_				un_ty.PM._offset6
#define pmPattern6_				un_ty.PM._pattern6
#define pmPatternMask6_		    un_ty.PM._pm6
#define pmOR6_				    un_ty.PM._or6
#define pmOffset7_				un_ty.PM._offset7
#define pmPattern7_				un_ty.PM._pattern7
#define pmPatternMask7_		    un_ty.PM._pm7
#define pmOR7_				    un_ty.PM._or7
#endif

#endif //CONFIG_RTL_LAYERED_DRIVER_ACL

/* ACL Rule Action type Definition */
#define RTL865X_ACL_PERMIT				0x00
#define RTL865X_ACL_REDIRECT_ETHER	0x01
#define RTL865X_ACL_DROP				0x02
#define RTL865X_ACL_TOCPU				0x03
#define RTL865X_ACL_LEGACY_DROP		0x04
#define RTL865X_ACL_DROPCPU_LOG		0x05
#define RTL865X_ACL_MIRROR				0x06
#define RTL865X_ACL_REDIRECT_PPPOE	0x07
#define RTL865X_ACL_DEFAULT_REDIRECT			0x08
#define RTL865X_ACL_MIRROR_KEEP_MATCH		0x09
#define RTL865X_ACL_DROP_RATE_EXCEED_PPS		0x0a
#define RTL865X_ACL_LOG_RATE_EXCEED_PPS		0x0b
#define RTL865X_ACL_DROP_RATE_EXCEED_BPS		0x0c
#define RTL865X_ACL_LOG_RATE_EXCEED_BPS		0x0d
#define RTL865X_ACL_PRIORITY					0x0e
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL865X_ACL_VID                     0x0f
#endif

/* ACL Rule type Definition */
#define RTL865X_ACL_MAC				0x00
#define RTL865X_ACL_DSTFILTER_IPRANGE 0x01
#define RTL865X_ACL_IP					0x02
#define RTL865X_ACL_ICMP				0x04
#define RTL865X_ACL_IGMP				0x05
#define RTL865X_ACL_TCP					0x06
#define RTL865X_ACL_UDP				0x07
#define RTL865X_ACL_SRCFILTER			0x08
#define RTL865X_ACL_DSTFILTER			0x09
#define RTL865X_ACL_IP_RANGE			0x0A
#define RTL865X_ACL_SRCFILTER_IPRANGE 0x0B
#define RTL865X_ACL_ICMP_IPRANGE		0x0C
#define RTL865X_ACL_IGMP_IPRANGE 		0x0D
#define RTL865X_ACL_TCP_IPRANGE		0x0E
#define RTL865X_ACL_UDP_IPRANGE		0x0F

#if defined(CONFIG_RTL_8197F)
#define RTL865X_ACL_PM				0x10 /* pattern match*/
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL865X_ACL_IPV6			0x12 /* ipv6 mask */
#define RTL865X_ACL_IPV6_RANGE		0x1A
#endif

#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
/*	dummy acl type for qos	*/
#define RTL865X_ACL_802D1P				0x1f
#endif

/* For PktOpApp */
#define RTL865X_ACL_ONLY_L2				1 /* Only for L2 switch */
#define RTL865X_ACL_ONLY_L3				2 /* Only for L3 routing (including IP multicast) */
#define RTL865X_ACL_L2_AND_L3			3 /* Only for L2 switch and L3 routing (including IP multicast) */
#define RTL865X_ACL_ONLY_L4				4 /* Only for L4 translation packets */
#define RTL865X_ACL_L3_AND_L4			6 /* Only for L3 routing and L4 translation packets (including IP multicast) */
#define RTL865X_ACL_ALL_LAYER			7 /* No operation. Don't apply this rule. */

#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) //8198C has 256 acl rules
#define RTL865X_ACL_MAX_NUMBER		252
#else
#define RTL865X_ACL_MAX_NUMBER		124
#endif

//#define RTL865X_ACL_MAX_NUMBER		64
#define RTL865X_ACL_RESERVED_NUMBER	4

#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define RTL865X_ACLTBL_ALL_TO_CPU		255  // This rule is always "To CPU"
#define RTL865X_ACLTBL_DROP_ALL		254 //This rule is always "Drop"
#define RTL865X_ACLTBL_PERMIT_ALL		253     // This rule is always "Permit"
#define RTL865X_ACLTBL_IPV6_TO_CPU		252
#else
#define RTL865X_ACLTBL_ALL_TO_CPU		127  // This rule is always "To CPU"
#define RTL865X_ACLTBL_DROP_ALL		126 //This rule is always "Drop"
#define RTL865X_ACLTBL_PERMIT_ALL		125     // This rule is always "Permit"
#define RTL865X_ACLTBL_IPV6_TO_CPU		124
#endif

#define MAX_IFNAMESIZE 16
#define NETIF_NUMBER 8

/*invert flag*/
#define RTL865X_INVERT_IN_NETIF	0x01
#define RTL865X_INVERT_OUT_NETIF	0x02

/*ingress or egress flag*/
#define RTL865X_ACL_INGRESS	0 /*ingress acl*/
#define RTL865X_ACL_EGRESS		1 /*egress acl*/

#define RTL_DEV_NAME_NUM(name,num)	name#num
#if defined (CONFIG_OPENWRT_SDK)
#define RTL_BR_NAME "br-lan"
#else
#define RTL_BR_NAME "br0"
#endif
#define RTL_WLAN_NAME "wlan"
//flowing name in driver DO NOT duplicate
#if defined(CONFIG_BRIDGE)
#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
#define RTL_DRV_LAN_NETIF_NAME "br-lan"
#else
#define RTL_DRV_LAN_NETIF_NAME "br0"
#endif
#else
#define RTL_DRV_LAN_NETIF_NAME "eth0"
#endif

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
#define RTL_BR1_NAME "br1"
#define RTL_PS_BR1_DEV_NAME     RTL_BR1_NAME
#define RTL_PS_ETH_NAME_ETH2	"eth2"
#endif

#define RTL_DRV_WAN0_NETIF_NAME "eth1"
#if defined(CONFIG_RTL_MULTIPLE_WAN)
#define RTL_DRV_WAN1_NETIF_NAME "eth6"
#endif
#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
#define RTL_DRV_PPP_NETIF_NAME "pppoe-ppp0"
#else
#define RTL_DRV_PPP_NETIF_NAME "ppp0"
#endif
#define RTL_DRV_DSLT_NETIF_NAME "dslt"
#define RTL_DRV_6RD_NETIF_NAME "6rd"

#define RTL_DRV_LAN_P0_NETIF_NAME RTL_DRV_LAN_NETIF_NAME
#define RTL_DRV_LAN_P1_NETIF_NAME "eth2"
#define RTL_DRV_LAN_P2_NETIF_NAME "eth3"
#define RTL_DRV_LAN_P3_NETIF_NAME "eth4"
#define RTL_DRV_LAN_P4_NETIF_NAME RTL_DRV_WAN0_NETIF_NAME
#define RTL_DRV_LAN_P5_NETIF_NAME "eth5"

#if defined(CONFIG_RTK_BRIDGE_VLAN_SUPPORT)
#define RTL_DRV_LAN_P7_NETIF_NAME "eth7"
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
#define NETIF_SW_NUMBER		16
#define RTL_BRIDGE_WANVLANID	7 /* WAN vid (bridged, default no vlan tag)*/
#define LAN_DEV_NUM_MAX	14	//lan(4) + wlan0(5) + wlan1(5)

#define ALIASNAME_PPP  						"ppp"
#define ALIASNAME_ETH_LAN  					"eth0."
#define ALIASNAME_ETH_WAN 					"eth1."
#define ALIASNAME_WLAN0_VA  				"wlan0-va"
#define ALIASNAME_WLAN1_VA 				"wlan1-va"
#define RTL_DRV_ETHLAN_P0_NETIF_NAME 	"eth0"
#define RTL_DRV_WLAN0_DEV_NAME 			"wlan0"
#define RTL_DRV_WLAN1_DEV_NAME 			"wlan1"

#define RTL_ETH_LAN0_BIND_MASK		(0x1<<0)
#define RTL_ETH_LAN1_BIND_MASK		(0x1<<1)
#define RTL_ETH_LAN2_BIND_MASK		(0x1<<2)
#define RTL_ETH_LAN3_BIND_MASK		(0x1<<3)
#define RTL_WLAN0_BIND_MASK			(0x1<<4)
#define RTL_WLAN0_VA0_BIND_MASK		(0x1<<5)
#define RTL_WLAN0_VA1_BIND_MASK		(0x1<<6)
#define RTL_WLAN0_VA2_BIND_MASK		(0x1<<7)
#define RTL_WLAN0_VA3_BIND_MASK		(0x1<<8)
#define RTL_WLAN1_BIND_MASK			(0x1<<9)
#define RTL_WLAN1_VA0_BIND_MASK		(0x1<<10)
#define RTL_WLAN1_VA1_BIND_MASK		(0x1<<11)
#define RTL_WLAN1_VA2_BIND_MASK		(0x1<<12)
#define RTL_WLAN1_VA3_BIND_MASK		(0x1<<13)

#define RTL_DRV_WLAN0_VA0_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VA, 0)
#define RTL_DRV_WLAN0_VA1_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VA, 1)
#define RTL_DRV_WLAN0_VA2_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VA, 2)
#define RTL_DRV_WLAN0_VA3_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VA, 3)
#define RTL_DRV_WLAN1_VA0_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN1_VA, 0)
#define RTL_DRV_WLAN1_VA1_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN1_VA, 1)
#define RTL_DRV_WLAN1_VA2_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN1_VA, 2)
#define RTL_DRV_WLAN1_VA3_DEV_NAME 	RTL_DEV_NAME_NUM(ALIASNAME_WLAN1_VA, 3)

#define RTL_DRV_ETHLAN1_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_LAN,1)//"eth0.1"
#define RTL_DRV_ETHLAN2_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_LAN,2)//"eth0.2"
#define RTL_DRV_ETHLAN3_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_LAN,3)//"eth0.3"
#define RTL_DRV_ETHLAN4_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_LAN,4)//"eth0.4"

#define RTL_DRV_ETHWAN1_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,1)//"eth1.1"
#define RTL_DRV_ETHWAN2_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,2)//"eth1.2"
#define RTL_DRV_ETHWAN3_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,3)//"eth1.3"
#define RTL_DRV_ETHWAN4_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,4)//"eth1.4"
#define RTL_DRV_ETHWAN5_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,5)//"eth1.5"
#define RTL_DRV_ETHWAN6_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,6)//"eth1.6"
#define RTL_DRV_ETHWAN7_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,7)//"eth1.7"
#define RTL_DRV_ETHWAN8_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_ETH_WAN,8)//"eth1.8"

#define RTL_DRV_ETHWAN1_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN1_NETIF_NAME,B)//"eth1.1B"
#define RTL_DRV_ETHWAN2_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN2_NETIF_NAME,B)//"eth1.2B"
#define RTL_DRV_ETHWAN3_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN3_NETIF_NAME,B)//"eth1.3B"
#define RTL_DRV_ETHWAN4_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN4_NETIF_NAME,B)//"eth1.4B"
#define RTL_DRV_ETHWAN5_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN5_NETIF_NAME,B)//"eth1.5B"
#define RTL_DRV_ETHWAN6_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN6_NETIF_NAME,B)//"eth1.6B"
#define RTL_DRV_ETHWAN7_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN7_NETIF_NAME,B)//"eth1.7B"
#define RTL_DRV_ETHWAN8_NETIF_NAME_EXTEND RTL_DEV_NAME_NUM(RTL_DRV_ETHWAN8_NETIF_NAME,B)//"eth1.8B"

#define RTL_DRV_PPP0_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,0)//"ppp0"
#define RTL_DRV_PPP1_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,1)//"ppp1"
#define RTL_DRV_PPP2_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,2)//"ppp2"
#define RTL_DRV_PPP3_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,3)//"ppp3"
#define RTL_DRV_PPP4_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,4)//"ppp4"
#define RTL_DRV_PPP5_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,5)//"ppp5"
#define RTL_DRV_PPP6_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,6)//"ppp6"
#define RTL_DRV_PPP7_NETIF_NAME RTL_DEV_NAME_NUM(ALIASNAME_PPP,7)//"ppp7"

//#define CONFIG_RTL_MAC_BASED_NETIF	1

#ifndef CONFIG_RTL_MAC_BASED_NETIF
#define CONFIG_RTL_VLAN_BASED_NETIF 1
#endif

#if defined(CONFIG_RTL_MAC_BASED_NETIF)
#define IF_FEATURE_IS_BRIDGE	0x00000001
#endif

#define RTL_CPU_PORT		8

int rtl_get_bind_port_mask_by_dev_name(char *name);
void rtl865x_get_master_netif_by_vid(int vid, char* netif_name);
int32 rtl865x_netif_exist(char *ifName);
int32 rtl865x_netif_is_wan(char *ifName);
int32 rtl865x_netif_is_master(char *ifName);
#endif


/************************************
*	const variable defination
*************************************/
#define	RTL_WANVLANID			8
#define	RTL_LANVLANID			9
#if defined(CONFIG_RTL_MULTIPLE_WAN)
#define	RTL_WAN_1_VLANID		369
#endif

#if defined(CONFIG_RTL8196_RTL8366)
	#define	RTL_WANPORT_MASK		0x1C1
	#define	RTL_LANPORT_MASK		0x1C1
	#define	RTL8366RB_GMIIPORT		0x20
	#define	RTL8366RB_LANPORT		0xCf
	#define	RTL8366RB_WANPORT		0x10
#elif defined(CONFIG_RTL_819X) && (defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV) || defined(CONFIG_RTL_EXCHANGE_PORTMASK) || defined(CONFIG_AUTO_DHCP_CHECK) || defined(CONFIG_RTK_WLAN_EVENT_INDICATE) || defined(CONFIG_RTL_LINK_AGGREGATION))
	#if defined(CONFIG_POCKET_ROUTER_SUPPORT)
		#define	RTL_WANPORT_MASK		0x10
		#define	RTL_LANPORT_MASK		0x10
	#elif defined(CONFIG_RTL_PUBLIC_SSID)
		#define RTL_WANPORT_MASK		0x110     //port 4/port 8
		#define	RTL_LANPORT_MASK		0x10f
	#elif defined(CONFIG_8198_PORT5_RGMII)
		#define	RTL_WANPORT_MASK		0x10
		#define	RTL_LANPORT_MASK		0x12f
	#else
		#if defined(CONFIG_RTL_8196C_iNIC)
			#define	RTL_WANPORT_MASK		0x01
			#define	RTL_LANPORT_MASK		0x110 //mark_inic , only port4 connect to MII
		#elif defined (CONFIG_RTK_INBAND_HOST_HACK)
			#if defined(CONFIG_8198_PORT5_GMII)
			#define RTL_WANPORT_MASK		0x120  //port5 ,hack port,eth1
			#define RTL_LANPORT_MASK		0x11f // 0~4 port eth0
			#else
			#define RTL_WANPORT_MASK		0x110  //port4(port0 in some board) is eth1
			#define RTL_LANPORT_MASK		0x12f // 0 1 2 3 5  port are eth0
			#endif
		#elif defined(CONFIG_8198_PORT5_GMII)
			#define RTL_WANPORT_MASK		0x10  //port0
			#define RTL_LANPORT_MASK		0x12f //all port eth0
		#elif defined (CONFIG_RTL_89xxD)
			#define RTL_WANPORT_MASK		0x01  //port0
			#define RTL_LANPORT_MASK		0x11e //all port eth0
		#elif defined(CONFIG_RTL_AP_PACKAGE)
			#define	RTL_WANPORT_MASK		0x00
			#define	RTL_LANPORT_MASK		0x11f
		#elif defined(CONFIG_RTL_EXCHANGE_PORTMASK)
			//change wan port to port0
			#if defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
			#define	RTL_LANPORT_MASK			0x8
			#define RTL_WANPORT_MASK			0x2
			#else
			#define RTL_WANPORT_MASK		0x01 //port0
			#define RTL_LANPORT_MASK		0x11e
			#endif
		#else
			#if defined(CONFIG_RTL_LINK_AGGREGATION)
			#define RTL_WANPORT_MASK		0x18 //port3 and port4 for wan, testcode
			#define RTL_LANPORT_MASK			0x107 //port0/port1/port2 for lan, testcode
			#else
			#define	RTL_WANPORT_MASK		0x10
			#ifdef CONFIG_8198C_8211FS
                       	    #define	RTL_LANPORT_MASK			0x12f  
                        #else
    	                    #define	RTL_LANPORT_MASK			0x10f
                        #endif			
			#endif
		#endif
	#endif
	
	#if defined(CONFIG_RTL_89xxD)
		#define 	RTL_LANPORT_MASK_1		0x2		//port 1
		#define		RTL_LANPORT_MASK_2		0x4		//port 2
		#define 	RTL_LANPORT_MASK_3		0x8		//port 3
		#define 	RTL_LANPORT_MASK_4		0x10	//port 4
	#else
	    #if defined(CONFIG_RTL_AP_PACKAGE)
	    #define     RTL_LANPORT_MASK_0      0x10     //port 4
	    #endif
	    
		#if defined(CONFIG_RTL_EXCHANGE_PORTMASK)
		#define 	RTL_LANPORT_MASK_1		0x10 	//port 4
		#define 	RTL_LANPORT_MASK_2		0x8 	//port 3
		#define 	RTL_LANPORT_MASK_3		0x4 	//port 2
		#define 	RTL_LANPORT_MASK_4		0x2 	//port 1
		#else
		#if defined(CONFIG_RTL_LINK_AGGREGATION)
		#define 	RTL_LANPORT_MASK_1		0x0 	//none
		#define 	RTL_LANPORT_MASK_2		0x4 	//port 2
		#define 	RTL_LANPORT_MASK_3		0x2 	//port 1
		#define 	RTL_LANPORT_MASK_4		0x1 	//port 0
		#else
		#define 	RTL_LANPORT_MASK_1		0x8		//port 0
		#define		RTL_LANPORT_MASK_2		0x4		//port 1
		#define 	RTL_LANPORT_MASK_3		0x2		//port 2
		    #ifdef CONFIG_8198C_8211FS
                        #define 	RTL_LANPORT_MASK_4		0x21		//port 0 / port5
                    #else
	                #define 	RTL_LANPORT_MASK_4		0x1		//port 3
                    #endif
					#endif
		#endif
	#endif
	#ifdef CONFIG_8198_PORT5_GMII
	#define 	RTL_LANPORT_MASK_5		0x20	//port 5
	#endif
#elif defined(CONFIG_RTL_8198_NFBI_BOARD)
	#define	RTL_WANPORT_MASK		0x1e0  //port 5, port 6,port 7,port 8
	#define	RTL_LANPORT_MASK		0x1df //port 0~4 , port 6~8  , need port4 ??
#elif defined(CONFIG_8198_PORT5_GMII)
	#define	RTL_WANPORT_MASK		0x110
	#define	RTL_LANPORT_MASK		0x1ef
#elif defined (CONFIG_POCKET_ROUTER_SUPPORT)
	#define	RTL_WANPORT_MASK		0x10
	#define	RTL_LANPORT_MASK		0x10
#elif defined(CONFIG_RTL_PUBLIC_SSID)
	#define RTL_WANPORT_MASK		0x110     //port 4/port 8
	
#elif defined(CONFIG_RTL_819X)
	#ifdef CONFIG_RTL_8196C_iNIC
	#define RTL_WANPORT_MASK		0x01
	#define RTL_LANPORT_MASK		0x110 //mark_inic , only port4 connect to MII
	#elif defined(CONFIG_RTL_AP_PACKAGE)
	#define	RTL_WANPORT_MASK		0x00
	#define	RTL_LANPORT_MASK		0x11f
	#elif defined(CONFIG_RTL_8198C_8367RB) || defined(CONFIG_RTL_8198C_8211F)
	#define	RTL_WANPORT_MASK		0x01		/* port 0 as wan port */
	#define	RTL_LANPORT_MASK		0x13e		/* port 1/2/3/4/5/cpu port(port 8) */
	#else
	#define	RTL_WANPORT_MASK		0x10
		#ifdef CONFIG_8198C_8211FS
		#define	RTL_LANPORT_MASK			0x12f  
		#elif defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT)
		#define	RTL_LANPORT_MASK			0x107
		#undef RTL_WANPORT_MASK
		#define RTL_WANPORT_MASK			0x8
		#elif defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
		#define	RTL_LANPORT_MASK			0x2
		#undef RTL_WANPORT_MASK
		#define RTL_WANPORT_MASK			0x8
		#else
		#define	RTL_LANPORT_MASK			0x10f
		#endif
	#endif
#else // should not happen
	#define	RTL_WANPORT_MASK		0x01
	#define	RTL_LANPORT_MASK		0x11e		/* port1/2/3/4/cpu port(port 8) */
	#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
	#define 	RTL_LANPORT_MASK_1		0x2		//port 1
	#define		RTL_LANPORT_MASK_2		0x4		//port 2
	#define 	RTL_LANPORT_MASK_3		0x8		//port 3
	#define 	RTL_LANPORT_MASK_4		0x10	//port 4
	#endif
#endif

#if defined(CONFIG_RTL_8881A) && defined(CONFIG_RTL_8211F_SUPPORT)
#undef RTL_WANPORT_MASK
#undef RTL_LANPORT_MASK
#define	RTL_WANPORT_MASK		0x1
#define	RTL_LANPORT_MASK		0x110
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined (CONFIG_RTL_MULTI_LAN_DEV)
#if defined(CONFIG_8198_PORT5_GMII)
#if defined(CONFIG_RTK_BRIDGE_VLAN_SUPPORT)
	#define ETH_INTF_NUM	7
#else
	#define ETH_INTF_NUM	6
#endif
#else
#if defined(CONFIG_RTK_BRIDGE_VLAN_SUPPORT)
	#define ETH_INTF_NUM	6
#else
	#define ETH_INTF_NUM	5
#endif
#endif
#else
#define ETH_INTF_NUM	2
#endif

typedef struct rtl865x_netif_s
{
	uint16 	vid; /*netif->vid*/
	uint16 	mtu; /*netif's MTU*/
	uint32 	if_type:5; /*interface type, 0:ether,1:pppoe....*/
	ether_addr_t macAddr;
	uint32	is_wan:1, /*wan interface?*/
			dmz:1,	/*DMZ/routing lan*/
			is_slave:1; /*is slave interface?*/
	uint8	name[MAX_IFNAMESIZE];
	uint16	enableRoute;
#if defined (CONFIG_RTL_LOCAL_PUBLIC) ||defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_MAC_BASED_NETIF)
	uint16	forMacBasedMCast;
#endif
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	uint16	enableRouteV6;
	uint16 	mtuV6; /*netif's ipv6 MTU*/
#endif
}rtl865x_netif_t;
/*internal...*/
int32 _rtl865x_getAclFromAsic(int32 index, rtl865x_AclRule_t *rule);

int32 rtl865x_init_acl(void);
int32 rtl865x_reinit_acl(void);
int32 rtl865x_add_acl(rtl865x_AclRule_t *rule, char *netifName, int32 chainNo);
int32 rtl865x_del_acl(rtl865x_AclRule_t *rule, char *netifName, int32 chainNo);
int32 rtl865x_regist_aclChain(char *netifName, int32 priority, uint32 flag);
int32 rtl865x_unRegist_aclChain(char *netifName, int32 priority, uint32 flag);
int32 rtl865x_flush_allAcl_fromChain(char *netifName, int32 priority, uint32 flag);
int32 rtl865x_show_allAclChains(struct seq_file *s);
rtl865x_AclRule_t* rtl865x_matched_layer4_aclChain(char *netifName,int32 priority, uint32 flag, rtl865x_AclRule_t *match);
rtl865x_AclRule_t* rtl865x_matched_layer2_aclChain(char *netifName,int32 priority, uint32 flag, rtl865x_AclRule_t *match);
//int32 rtl865x_add_def_permit_acl(void);
//int32 rtl865x_del_def_permit_acl(void);

#if defined(CONFIG_RTK_VLAN_SUPPORT)
int32 rtl865x_enable_acl(uint32 enable);
#endif
int  rtl865x_add_pattern_acl_for_contentFilter(rtl865x_AclRule_t *rule,char *netifName);
int  rtl865x_del_pattern_acl_for_contentFilter(rtl865x_AclRule_t *rule,char *netifName);


#ifdef RTL_LAYERED_DRIVER_DEBUG
int32 rtl865x_acl_test(int32 testNo);
#endif

#if defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
#define CONFIG_RTL_AVOID_ADDING_WAN_SUBNET_PKT_TO_HW_NAT	1
#endif

//#define CONFIG_RTL_IPTABLES2ACL_PATCH 1
#if defined(CONFIG_RTL_IPTABLES2ACL_PATCH)
int32 rtl865x_add_sw_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority);
int32 _rtl865x_synAclwithAsicTbl(void);
int32 rtl865x_flush_allAcl_sw_fromChain(char *netifName, int32 priority, uint32 flag);
#endif


int32 rtl865x_deReferNetif(char *ifName);
int32 rtl865x_referNetif(char *ifName);
int32 rtl865x_setNetifMtu(rtl865x_netif_t *netif);
int32 rtl865x_setNetifMac(rtl865x_netif_t *netif);
int32 rtl865x_setNetifVid(char *name, uint16 vid);
int32 rtl865x_setNetifType(char *name, uint32 ifType);
int32 rtl865x_addNetif(rtl865x_netif_t *netif);
int32 rtl865x_delNetif(char *ifName);
int32 rtl865x_initNetifTable(void);
int32 rtl865x_reinitNetifTable(void);
int32 rtl865x_attachMasterNetif(char *slave, char *master);
int32 rtl865x_detachMasterNetif(char *slave);

int32 rtl865x_setPortToNetif(char *name,uint32 port);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
int32 rtl865x_setNetifMtuV6(rtl865x_netif_t *netif);
#endif

#if defined(CONFIG_SWCONFIG) && defined(CONFIG_RTL_HARDWARE_NAT)&&defined(CONFIG_OPENWRT_SDK)
#define CONFIG_SWCONFIG_SUPPORT_HW_NAT 1
#if defined(CONFIG_SWCONFIG_SUPPORT_HW_NAT)
#define RTL_SWCONFIG_DRV_PPP_NETIF_NAME "pppoe-wan"
extern int swconfig_vlan_enable;
extern char hw_nat_wan_dev_name[MAX_IFNAMESIZE];
extern uint8* rtl_get_netif_gmac(char *name);
extern int rtl_check_default_router_valid(void);
#endif
#endif

#if defined (CONFIG_RTL_HARDWARE_MULTICAST) ||defined(CONFIG_SWCONFIG_SUPPORT_HW_NAT)
int32 rtl865x_getNetifVid(char *name, uint32 *vid);
int32 rtl865x_getNetifType(char *name,uint32 *type);
uint32 rtl865x_getExternalPortMask(void);
#endif

#if  defined (CONFIG_RTL_LOCAL_PUBLIC)
//int32 rtl865x_getNetifFid(char *name,  uint16 *fid);
int32 rtl865x_addVirtualNetif(rtl865x_netif_t *netif);
int32 rtl865x_delVirtualNetif(char *ifName);
#endif

int32 rtl865x_setDefACLForAllNetif(uint8 start_ingressAclIdx, uint8 end_ingressAclIdx,uint8 start_egressAclIdx,uint8 end_egressAclIdx);
int32 rtl865x_reConfigDefaultAcl(char *ifName);
int32 rtl865x_config_callback_for_get_drv_netifName(int (*fun)(const char *psName,char *netifName));
#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
void rtl_set_netif_v6_route(int enable);
#endif
#endif

