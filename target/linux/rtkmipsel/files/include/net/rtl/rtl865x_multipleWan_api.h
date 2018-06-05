/*
* Copyright c                  Realtek Semiconductor Corporation, 2010  
* All rights reserved.
* 
* Program : multiple wan device driver header file
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_MULTIPLEWAN_API_H
#define RTL865X_MULTIPLEWAN_API_H
#include "rtl_types.h"

#define RTL_ADVRT_MAC						0x00
#define RTL_ADVRT_DSTFILTER_IPRANGE		0x01
#define RTL_ADVRT_IP					0x02
#define RTL_ADVRT_ICMP				0x04
#define RTL_ADVRT_IGMP				0x05
#define RTL_ADVRT_TCP					0x06
#define RTL_ADVRT_UDP					0x07
#define RTL_ADVRT_SRCFILTER			0x08
#define RTL_ADVRT_DSTFILTER			0x09
#define RTL_ADVRT_IP_RANGE			0x0A
#define RTL_ADVRT_SRCFILTER_IPRANGE 	0x0B
#define RTL_ADVRT_ICMP_IPRANGE		0x0C
#define RTL_ADVRT_IGMP_IPRANGE 		0x0D
#define RTL_ADVRT_TCP_IPRANGE		0x0E
#define RTL_ADVRT_UDP_IPRANGE		0x0F


typedef struct _rtl_advRoute_entry
{
	union 
	{
		/* MAC rule */
		struct {
			ether_addr_t _dstMac, _dstMacMask;
			ether_addr_t _srcMac, _srcMacMask;
			uint16 _typeLen, _typeLenMask;
		} MAC;
		
		/* IP Group rule */
		struct
		{
			ipaddr_t _srcIpAddr, _srcIpAddrMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint8 _tos, _tosMask;
			union
			{
				/* IP rle */
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
				
				/* ICMP rule */
				struct 
				{
					uint8 _type, _typeMask, _code, _codeMask;
				} icmp; 
				
				/* IGMP rule */
				struct
				{
					uint8 _type, _typeMask;
				} igmp; 
				
				/* TCP rule */
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
				
				/* UDP rule */
				struct 
				{
					uint16 _srcPortUpperBound, _srcPortLowerBound;
					uint16 _dstPortUpperBound, _dstPortLowerBound;										
				}udp; 
			}is;			
		}L3L4; 
#if defined(CONFIG_RTL_8198C)
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

		/* Source filter rule */
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
		
		/* Destination filter rule */
		struct 
		{
			ether_addr_t _dstMac, _dstMacMask;
			uint16 _vlanIdx, _vlanIdxMask;
			ipaddr_t _dstIpAddr, _dstIpAddrMask;
			uint16 _dstPortUpperBound, _dstPortLowerBound;
			uint32 _ignoreL4:1, //L3 rule
				   _ignoreL3L4:1; //L2 rule
		} DSTFILTER;
#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT) ||defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
		struct {
			uint8	vlanTagPri;
		} VLANTAG;
#endif
	}un_ty;

	
	uint32	valid_:1,	
			ruleType_:5,			
			pktOpApp_:3;
	
	ipaddr_t	extIp; /*which ip address is used when napt*/
	ipaddr_t	nexthop; /* next hop IP address */
	char outIfName[16]; /*out interface name, it should the netif name in driver*/	
}rtl_advRoute_entry_t;

/* MAC ACL rule Definition */
#define advrt_dstMac_				un_ty.MAC._dstMac
#define advrt_dstMacMask_			un_ty.MAC._dstMacMask
#define advrt_srcMac_				un_ty.MAC._srcMac
#define advrt_srcMacMask_			un_ty.MAC._srcMacMask
#define advrt_typeLen_				un_ty.MAC._typeLen
#define advrt_typeLenMask_			un_ty.MAC._typeLenMask

/* Common IP ACL Rule Definition */
#define advrt_srcIpAddr_				un_ty.L3L4._srcIpAddr
#define advrt_srcIpAddrMask_			un_ty.L3L4._srcIpAddrMask
#define advrt_srcIpAddrUB_				un_ty.L3L4._srcIpAddr
#define advrt_srcIpAddrLB_				un_ty.L3L4._srcIpAddrMask
#define advrt_dstIpAddr_				un_ty.L3L4._dstIpAddr
#define advrt_dstIpAddrMask_			un_ty.L3L4._dstIpAddrMask
#define advrt_dstIpAddrUB_				un_ty.L3L4._dstIpAddr
#define advrt_dstIpAddrLB_				un_ty.L3L4._dstIpAddrMask
#define advrt_tos_					un_ty.L3L4._tos
#define advrt_tosMask_				un_ty.L3L4._tosMask
/* IP Rrange */
/*Hyking:Asic use Addr to srore Upper address
	and use Mask to store Lower address
*/
#define advrt_srcIpAddrStart_			un_ty.L3L4._srcIpAddrMask
#define advrt_srcIpAddrEnd_			un_ty.L3L4._srcIpAddr
#define advrt_dstIpAddrStart_			un_ty.L3L4._dstIpAddrMask
#define advrt_dstIpAddrEnd_			un_ty.L3L4._dstIpAddr

/* IP ACL Rule Definition */
#define advrt_ipProto_				un_ty.L3L4.is.ip._proto
#define advrt_ipProtoMask_			un_ty.L3L4.is.ip._protoMask
#define advrt_ipFlagMask_			un_ty.L3L4.is.ip._flagMask
#define advrt_ipFOP_      				un_ty.L3L4.is.ip._FOP
#define advrt_ipFOM_      				un_ty.L3L4.is.ip._FOM
#define advrt_ipHttpFilter_      			un_ty.L3L4.is.ip._httpFilter
#define advrt_ipHttpFilterM_			un_ty.L3L4.is.ip._httpFilterM
#define advrt_ipIdentSrcDstIp_   		un_ty.L3L4.is.ip._identSrcDstIp
#define advrt_ipIdentSrcDstIpM_		un_ty.L3L4.is.ip._identSrcDstIpM
#define advrt_ipFlag_					un_ty.L3L4.is.ip.un._flag
#define advrt_ipDF_					un_ty.L3L4.is.ip.un.s._DF
#define advrt_ipMF_					un_ty.L3L4.is.ip.un.s._MF

/* ICMP ACL Rule Definition */
#define advrt_icmpType_				un_ty.L3L4.is.icmp._type
#define advrt_icmpTypeMask_			un_ty.L3L4.is.icmp._typeMask	
#define advrt_icmpCode_				un_ty.L3L4.is.icmp._code
#define advrt_icmpCodeMask_			un_ty.L3L4.is.icmp._codeMask

/* IGMP ACL Rule Definition */
#define advrt_igmpType_				un_ty.L3L4.is.igmp._type
#define advrt_igmpTypeMask_			un_ty.L3L4.is.igmp._typeMask

/* TCP ACL Rule Definition */
#define advrt_tcpSrcPortUB_			un_ty.L3L4.is.tcp._srcPortUpperBound
#define advrt_tcpSrcPortLB_			un_ty.L3L4.is.tcp._srcPortLowerBound
#define advrt_tcpDstPortUB_			un_ty.L3L4.is.tcp._dstPortUpperBound
#define advrt_tcpDstPortLB_			un_ty.L3L4.is.tcp._dstPortLowerBound
#define advrt_tcpFlagMask_			un_ty.L3L4.is.tcp._flagMask
#define advrt_tcpFlag_				un_ty.L3L4.is.tcp.un._flag
#define advrt_tcpURG_				un_ty.L3L4.is.tcp.un.s._urg
#define advrt_tcpACK_				un_ty.L3L4.is.tcp.un.s._ack
#define advrt_tcpPSH_				un_ty.L3L4.is.tcp.un.s._psh
#define advrt_tcpRST_				un_ty.L3L4.is.tcp.un.s._rst
#define advrt_tcpSYN_				un_ty.L3L4.is.tcp.un.s._syn
#define advrt_tcpFIN_				un_ty.L3L4.is.tcp.un.s._fin

/* UDP ACL Rule Definition */
#define advrt_udpSrcPortUB_			un_ty.L3L4.is.udp._srcPortUpperBound
#define advrt_udpSrcPortLB_			un_ty.L3L4.is.udp._srcPortLowerBound
#define advrt_udpDstPortUB_			un_ty.L3L4.is.udp._dstPortUpperBound
#define advrt_udpDstPortLB_			un_ty.L3L4.is.udp._dstPortLowerBound

/* Source Filter ACL Rule Definition */
#define advrt_srcFilterMac_				un_ty.SRCFILTER._srcMac
#define advrt_srcFilterMacMask_		un_ty.SRCFILTER._srcMacMask
#define advrt_srcFilterPort_				un_ty.SRCFILTER._srcPort
#define advrt_srcFilterPortMask_		un_ty.SRCFILTER._srcPortMask
#define advrt_srcFilterVlanIdx_			un_ty.SRCFILTER._srcVlanIdx
#define advrt_srcFilterVlanId_			un_ty.SRCFILTER._srcVlanIdx
#define advrt_srcFilterVlanIdxMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define advrt_srcFilterVlanIdMask_		un_ty.SRCFILTER._srcVlanIdxMask
#define advrt_srcFilterIpAddr_			un_ty.SRCFILTER._srcIpAddr
#define advrt_srcFilterIpAddrMask_		un_ty.SRCFILTER._srcIpAddrMask
#define advrt_srcFilterIpAddrUB_		un_ty.SRCFILTER._srcIpAddr
#define advrt_srcFilterIpAddrLB_		un_ty.SRCFILTER._srcIpAddrMask
#define advrt_srcFilterPortUpperBound_	un_ty.SRCFILTER._srcPortUpperBound
#define advrt_srcFilterPortLowerBound_	un_ty.SRCFILTER._srcPortLowerBound
#define advrt_srcFilterIgnoreL3L4_		un_ty.SRCFILTER._ignoreL3L4
#define advrt_srcFilterIgnoreL4_		un_ty.SRCFILTER._ignoreL4

/* Destination Filter ACL Rule Definition */
#define advrt_dstFilterMac_				un_ty.DSTFILTER._dstMac
#define advrt_dstFilterMacMask_		un_ty.DSTFILTER._dstMacMask
#define advrt_dstFilterVlanIdx_			un_ty.DSTFILTER._vlanIdx
#define advrt_dstFilterVlanIdxMask_		un_ty.DSTFILTER._vlanIdxMask
#define advrt_dstFilterVlanId_			un_ty.DSTFILTER._vlanIdx
#define advrt_dstFilterVlanIdMask_		un_ty.DSTFILTER._vlanIdxMask
#define advrt_dstFilterIpAddr_			un_ty.DSTFILTER._dstIpAddr
#define advrt_dstFilterIpAddrMask_		un_ty.DSTFILTER._dstIpAddrMask
#define advrt_dstFilterPortUpperBound_	un_ty.DSTFILTER._dstPortUpperBound
#define advrt_dstFilterIpAddrUB_		un_ty.DSTFILTER._dstIpAddr
#define advrt_dstFilterIpAddrLB_		un_ty.DSTFILTER._dstIpAddrMask
#define advrt_dstFilterPortLowerBound_	un_ty.DSTFILTER._dstPortLowerBound
#define advrt_dstFilterIgnoreL3L4_		un_ty.DSTFILTER._ignoreL3L4
#define advrt_dstFilterIgnoreL4_		un_ty.DSTFILTER._ignoreL4
#if	defined(CONFIG_RTL_QOS_8021P_SUPPORT)
#define advrt_vlanTagPri_			un_ty.VLANTAG.vlanTagPri
#endif


#endif
