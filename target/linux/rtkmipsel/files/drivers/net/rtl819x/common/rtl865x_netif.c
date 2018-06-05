/*
* Copyright c                  Realtek Semiconductor Corporation, 2008
* All rights reserved.
*
* Program : network interface driver
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_netif.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.

        @index | RTL_LAYEREDDRV_API
*/
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "rtl_errno.h"
//#include "rtl_utils.h"
//#include "rtl_glue.h"
#include <net/rtl/rtl865x_netif.h>
#include "rtl865x_netif_local.h"
#include "rtl865x_vlan.h" /*reference vlan*/
#include "rtl865x_eventMgr.h" /*call back function....*/
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include "AsicDriver/rtl865x_asicBasic.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#else
#include "AsicDriver/rtl865xC_tblAsicDrv.h"
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
//#include "rtl865x_localPublic.h"
#endif

#ifdef CONFIG_RTL_HARDWARE_IPV6_SUPPORT
extern int rtk_hwL3v6_enable;
#endif

static rtl865x_netif_local_t *netifTbl;

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
static int Netif_Asic2Sw_Mapping[NETIF_NUMBER] = {-1};
#endif

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static RTL_DECLARE_MUTEX(netif_sem);
#endif
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
static rtl865x_netif_local_t virtualNetIf;
#endif

static int (*rtl_get_drv_netifName_by_psName)(const char *psName,char *netifName);
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
static rtl865x_aclBuf_t freeAclList;
static rtl865x_acl_chain_t *freeChainHead;

#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q)
static uint32 rtl865x_acl_enable = 1; //default enable
#endif

static int32 _rtl865x_regist_aclChain(char *netifName, int32 priority,uint32 flag);
static int32 _rtl865x_unRegister_all_aclChain(char *netifName);
#endif

static int32 _rtl865x_delNetif(char *name);

int32 rtl865x_dump_AclRule(	rtl865x_AclRule_t *rule)
{

	int8 *actionT[] = { "permit", "redirect to ether", "drop", "to cpu", "legacy drop",
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match",
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
					};

	switch(rule->ruleType_)
	{
		case RTL865X_ACL_MAC:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Ethernet", actionT[rule->actionType_]);
			printk("\tether type: %x   ether type mask: %x\n", rule->typeLen_, rule->typeLenMask_);
			printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
					rule->dstMac_.octet[0], rule->dstMac_.octet[1], rule->dstMac_.octet[2],
					rule->dstMac_.octet[3], rule->dstMac_.octet[4], rule->dstMac_.octet[5],
					rule->dstMacMask_.octet[0], rule->dstMacMask_.octet[1], rule->dstMacMask_.octet[2],
					rule->dstMacMask_.octet[3], rule->dstMacMask_.octet[4], rule->dstMacMask_.octet[5]
					);

			printk( "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
					rule->srcMac_.octet[0], rule->srcMac_.octet[1], rule->srcMac_.octet[2],
					rule->srcMac_.octet[3], rule->srcMac_.octet[4], rule->srcMac_.octet[5],
					rule->srcMacMask_.octet[0], rule->srcMacMask_.octet[1], rule->srcMacMask_.octet[2],
					rule->srcMacMask_.octet[3], rule->srcMacMask_.octet[4], rule->srcMacMask_.octet[5]
				);
			break;

		case RTL865X_ACL_IP:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP", actionT[rule->actionType_]);
			printk( "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
					rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
				);

			printk("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
					rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
					rule->ipIdentSrcDstIpM_
					);
			printk( "\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_);
				break;

		case RTL865X_ACL_IP_RANGE:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP Range", actionT[rule->actionType_]);
			printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
					rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
					);
			printk("\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
					rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
					rule->ipIdentSrcDstIpM_
					);
				printk("\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_);
				break;
		case RTL865X_ACL_ICMP:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP", actionT[rule->actionType_]);
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
					rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_,
					rule->icmpCode_, rule->icmpCodeMask_);
			break;
		case RTL865X_ACL_ICMP_IPRANGE:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP IP RANGE", actionT[rule->actionType_]);
			printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
					rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_,
					rule->icmpCode_, rule->icmpCodeMask_);
			break;
		case RTL865X_ACL_IGMP:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP", actionT[rule->actionType_]);
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
					rule->igmpType_, rule->igmpTypeMask_
					);
			break;


		case RTL865X_ACL_IGMP_IPRANGE:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP IP RANGE", actionT[rule->actionType_]);
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
					rule->igmpType_, rule->igmpTypeMask_
					);
			break;

		case RTL865X_ACL_TCP:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP", actionT[rule->actionType_]);
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
					rule->tcpDstPortLB_, rule->tcpDstPortUB_
					);
			printk("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
					rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
					rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
					);
			break;
		case RTL865X_ACL_TCP_IPRANGE:
				printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP IP RANGE", actionT[rule->actionType_]);
				printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
				printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
				printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
					rule->tcpDstPortLB_, rule->tcpDstPortUB_
					);
				printk("\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
					rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
					rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
				);
			break;

		case RTL865X_ACL_UDP:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx,"UDP", actionT[rule->actionType_]);
			printk("\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
					rule->udpDstPortLB_, rule->udpDstPortUB_
					);
			break;
		case RTL865X_ACL_UDP_IPRANGE:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "UDP IP RANGE", actionT[rule->actionType_]);
			printk("\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
					((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
					(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
					((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
					);
			printk("\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
					((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
					(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
					((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
					);
			printk("\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
					rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
					rule->udpDstPortLB_, rule->udpDstPortUB_
				);
			break;


		case RTL865X_ACL_SRCFILTER:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter", actionT[rule->actionType_]);
			printk("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
					rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2],
					rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
					rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
					rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
					);
			printk("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
					rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
					(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
					);
			printk("\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
					((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
					(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
					((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
					(rule->srcFilterIpAddrMask_&0xff)
					);
			printk("\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
			break;

		case RTL865X_ACL_SRCFILTER_IPRANGE:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter(IP RANGE)", actionT[rule->actionType_]);
			printk("\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
					rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2],
					rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
					rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
					rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
					);
			printk("\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
					rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
					(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
					);
			printk("\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
					((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
					(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
					((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
					(rule->srcFilterIpAddrMask_&0xff)
					);
			printk("\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
			break;

		case RTL865X_ACL_DSTFILTER:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter", actionT[rule->actionType_]);
			printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
					rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2],
					rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
					rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
					rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
					);
			printk("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
					rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_,
					(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)),
					rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
					);
			printk("\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
					((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
					(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
					((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
					(rule->dstFilterIpAddrMask_&0xff)
					);
			break;
		case RTL865X_ACL_DSTFILTER_IPRANGE:
			printk(" [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter(IP Range)", actionT[rule->actionType_]);
			printk("\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
					rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2],
					rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
					rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
					rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
					);
			printk("\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
					rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_,
					(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)),
					rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
					);
			printk("\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
					((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
					(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
					((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
					(rule->dstFilterIpAddrMask_&0xff)
				);
			break;

			default:
				printk("rule->ruleType_(0x%x)\n", rule->ruleType_);

	}

	switch (rule->actionType_)
	{
		case RTL865X_ACL_PERMIT:
		case RTL865X_ACL_REDIRECT_ETHER:
		case RTL865X_ACL_DROP:
		case RTL865X_ACL_TOCPU:
		case RTL865X_ACL_LEGACY_DROP:
		case RTL865X_ACL_DROPCPU_LOG:
		case RTL865X_ACL_MIRROR:
		case RTL865X_ACL_REDIRECT_PPPOE:
		case RTL865X_ACL_MIRROR_KEEP_MATCH:
			printk("\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", rule->netifIdx_, rule->pppoeIdx_, rule->L2Idx_);
			break;

		case RTL865X_ACL_PRIORITY:
			printk("\tprioirty: %d   ",  rule->priority_) ;
			break;

		case RTL865X_ACL_DEFAULT_REDIRECT:
			printk("\tnextHop:%d  ",  rule->nexthopIdx_);
			break;

		case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
			printk("\tratelimitIdx: %d  ",  rule->ratelimtIdx_);
			break;
		default:
			;

		}
		printk("pktOpApp: %d\n",  rule->pktOpApp_);
		printk("===========================\n");
		return SUCCESS;

}

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
int32 rtl865x_show_allAclChains(struct seq_file *s)
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_acl_chain_t *chain;
	rtl865x_AclRule_t *rule;
	int32 i,j;
	int8 *actionT[] = { "permit", "redirect to ether", "drop", "to cpu", "legacy drop",
					"drop for log", "mirror", "redirect to pppoe", "default redirect", "mirror keep match",
					"drop rate exceed pps", "log rate exceed pps", "drop rate exceed bps", "log rate exceed bps","priority "
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                    , "change vid"
#endif
					};
    
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
    rtl865x_AclRule_t *rule2 = NULL;
    int8 ipv6_flag = 0;    
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		if(netifTbl[i].valid == 1)
		{
			netif = &netifTbl[i];
			for(j = RTL865X_ACL_INGRESS; j<= RTL865X_ACL_EGRESS;j++)
			{
				seq_printf(s,"netif(%s),isEgress(%d):\n",netif->name,j);
				chain = netif->chainListHead[j];
				while(chain)
				{
					seq_printf(s,"\tchain:priority(%d),rulecnt(%d)\n",chain->priority,chain->ruleCnt);
					seq_printf(s,"===========================\n");
					rule = chain->head;
					while(rule)
					{
						switch(rule->ruleType_)
						{
							case RTL865X_ACL_MAC:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Ethernet", actionT[rule->actionType_]);
								seq_printf(s,"\tether type: %x   ether type mask: %x\n", rule->typeLen_, rule->typeLenMask_);
								seq_printf(s,"\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
										rule->dstMac_.octet[0], rule->dstMac_.octet[1], rule->dstMac_.octet[2],
										rule->dstMac_.octet[3], rule->dstMac_.octet[4], rule->dstMac_.octet[5],
										rule->dstMacMask_.octet[0], rule->dstMacMask_.octet[1], rule->dstMacMask_.octet[2],
										rule->dstMacMask_.octet[3], rule->dstMacMask_.octet[4], rule->dstMacMask_.octet[5]
										);

								seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
										rule->srcMac_.octet[0], rule->srcMac_.octet[1], rule->srcMac_.octet[2],
										rule->srcMac_.octet[3], rule->srcMac_.octet[4], rule->srcMac_.octet[5],
										rule->srcMacMask_.octet[0], rule->srcMacMask_.octet[1], rule->srcMacMask_.octet[2],
										rule->srcMacMask_.octet[3], rule->srcMacMask_.octet[4], rule->srcMacMask_.octet[5]
									);
								break;

							case RTL865X_ACL_IP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP", actionT[rule->actionType_]);
								seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
										rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
									);

								seq_printf(s,"\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
										rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
										rule->ipIdentSrcDstIpM_
										);
								seq_printf(s, "\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_);
									break;

							case RTL865X_ACL_IP_RANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP Range", actionT[rule->actionType_]);
								seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
										rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
										);
								seq_printf(s,"\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
										rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
										rule->ipIdentSrcDstIpM_
										);
									seq_printf(s,"\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_);
									break;
							case RTL865X_ACL_ICMP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
										rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_,
										rule->icmpCode_, rule->icmpCodeMask_);
								break;
							case RTL865X_ACL_ICMP_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP IP RANGE", actionT[rule->actionType_]);
								seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
										rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_,
										rule->icmpCode_, rule->icmpCodeMask_);
								break;
							case RTL865X_ACL_IGMP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
										rule->igmpType_, rule->igmpTypeMask_
										);
								break;


							case RTL865X_ACL_IGMP_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP IP RANGE", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
										rule->igmpType_, rule->igmpTypeMask_
										);
								break;

							case RTL865X_ACL_TCP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
										rule->tcpDstPortLB_, rule->tcpDstPortUB_
										);
								seq_printf(s,"\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
										rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
										rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
										);
								break;
							case RTL865X_ACL_TCP_IPRANGE:
									seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP IP RANGE", actionT[rule->actionType_]);
									seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
									seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
									seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
										rule->tcpDstPortLB_, rule->tcpDstPortUB_
										);
									seq_printf(s,"\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
										rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
										rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
									);
								break;

							case RTL865X_ACL_UDP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx,"UDP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
										rule->udpDstPortLB_, rule->udpDstPortUB_
										);
								break;
							case RTL865X_ACL_UDP_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "UDP IP RANGE", actionT[rule->actionType_]);
								seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
										rule->udpDstPortLB_, rule->udpDstPortUB_
									);
								break;


							case RTL865X_ACL_SRCFILTER:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter", actionT[rule->actionType_]);
								seq_printf(s,"\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
										rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2],
										rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
										rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
										rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
										rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
										(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
										((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
										(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
										((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->srcFilterIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
								break;

							case RTL865X_ACL_SRCFILTER_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter(IP RANGE)", actionT[rule->actionType_]);
								seq_printf(s,"\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
										rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2],
										rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
										rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
										rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
										rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
										(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
										((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
										(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
										((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->srcFilterIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
								break;

							case RTL865X_ACL_DSTFILTER:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter", actionT[rule->actionType_]);
								seq_printf(s,"\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
										rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2],
										rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
										rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
										rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
										rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_,
										(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)),
										rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
										);
								seq_printf(s,"\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
										((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
										(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
										((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->dstFilterIpAddrMask_&0xff)
										);
								break;
							case RTL865X_ACL_DSTFILTER_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter(IP Range)", actionT[rule->actionType_]);
								seq_printf(s,"\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
										rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2],
										rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
										rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
										rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
										rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_,
										(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)),
										rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
										);
								seq_printf(s,"\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
										((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
										(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
										((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->dstFilterIpAddrMask_&0xff)
									);
								break;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                            case RTL865X_ACL_IPV6: /* IP Rule Type: 0x0010 */
                            case RTL865X_ACL_IPV6_RANGE:
                                /* a ipv6 rule occupied  two entry, rule point to the first entry  
                                                    * and rule2 point to the second entry.
                                                    */
                                rule2 = rule->next;
                                ipv6_flag = 0;
                                
                                if (rule2 != NULL)//the second entry 
                                {
                                    if ((!rule2->ipv6EntryType_) && rule->ipv6EntryType_)
                                    {
                                        rule->dstIpV6Addr_.v6_addr32[3] = rule2->dstIpV6Addr_.v6_addr32[3];
                                        rule->dstIpV6Addr_.v6_addr32[2] = rule2->dstIpV6Addr_.v6_addr32[2];
                                        rule->dstIpV6Addr_.v6_addr32[1] = rule2->dstIpV6Addr_.v6_addr32[1];
                                        rule->dstIpV6Addr_.v6_addr32[0] = rule2->dstIpV6Addr_.v6_addr32[0];
                                        
                                        rule->dstIpV6AddrMask_.v6_addr32[3] = rule2->dstIpV6AddrMask_.v6_addr32[3];
                                        rule->dstIpV6AddrMask_.v6_addr32[2] = rule2->dstIpV6AddrMask_.v6_addr32[2];
                                        rule->dstIpV6AddrMask_.v6_addr32[1] = rule2->dstIpV6AddrMask_.v6_addr32[1];
                                        rule->dstIpV6AddrMask_.v6_addr32[0] = rule2->dstIpV6AddrMask_.v6_addr32[0];
            
                                        rule->ipv6TrafficClass_ = rule2->ipv6TrafficClass_;
                                        rule->ipv6TrafficClassM_ = rule2->ipv6TrafficClassM_; 
                                        rule->ipv6NextHeader_    = rule2->ipv6NextHeader_; 
                                        rule->ipv6NextHeaderM_   = rule2->ipv6NextHeaderM_; 
                                        rule->ipv6HttpFilter_    = rule2->ipv6HttpFilter_;
                                        rule->ipv6HttpFilterM_ = rule2->ipv6HttpFilterM_;
                                        rule->ipv6IdentSrcDstIp_ = rule2->ipv6IdentSrcDstIp_;
                                        rule->ipv6IdentSrcDstIpM_ = rule2->ipv6IdentSrcDstIpM_;
                                        /* ActionType and ActionField useless in entry0 */
                                        rule->actionType_ = rule2->actionType_;
                                        switch(rule->actionType_) {
                                    
                                        case RTL865X_ACL_PERMIT:
                                        case RTL865X_ACL_REDIRECT_ETHER:
                                        case RTL865X_ACL_DROP:
                                        case RTL865X_ACL_TOCPU:
                                        case RTL865X_ACL_LEGACY_DROP:
                                        case RTL865X_ACL_DROPCPU_LOG:
                                        case RTL865X_ACL_MIRROR:
                                        case RTL865X_ACL_REDIRECT_PPPOE:
                                        case RTL865X_ACL_MIRROR_KEEP_MATCH:
                                            rule->L2Idx_ = rule2->L2Idx_ ;
                                            rule->netifIdx_ =  rule2->netifIdx_;
                                            rule->pppoeIdx_ = rule2->pppoeIdx_;
                                             break;
                                    
                                        case RTL865X_ACL_DEFAULT_REDIRECT:
                                            rule->nexthopIdx_ = rule2->nexthopIdx_;
                                            break;
                                    
                                        case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
                                        case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
                                        case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
                                        case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
                                            rule->ratelimtIdx_ = rule2->ratelimtIdx_;
                                            break;
                                        case RTL865X_ACL_PRIORITY:
                                            rule->priority_ = rule2->priority_;
                                            break;
                                        case RTL865X_ACL_VID:
                                            rule->aclvid_ = rule2->aclvid_;
                                            break;                      
                                        }
                                        /* INV useless in entry 0 */
                                        rule->ipv6Invert_    = rule2->ipv6Invert_;
                                        //rule->ipv6Combine_  = rule2->ipv6Combine_;
                                        //rule->ipv6IPtunnel_ = rule2->ipv6IPtunnel_;
                                        seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IPv6", actionT[rule->actionType_]);
                                        if (RTL865X_ACL_IPV6 == rule->ruleType_)
                                        {
                                            seq_printf(s,"\tsip: %x:%x:%x:%x:%x:%x:%x:%x  sipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->srcIpV6Addr_.v6_addr16[0], rule->srcIpV6Addr_.v6_addr16[1], 
                                                    rule->srcIpV6Addr_.v6_addr16[2],rule->srcIpV6Addr_.v6_addr16[3],
                                                    rule->srcIpV6Addr_.v6_addr16[4],rule->srcIpV6Addr_.v6_addr16[5],
                                                    rule->srcIpV6Addr_.v6_addr16[6],rule->srcIpV6Addr_.v6_addr16[7],
                                                    rule->srcIpV6AddrMask_.v6_addr16[0], rule->srcIpV6AddrMask_.v6_addr16[1], 
                                                    rule->srcIpV6AddrMask_.v6_addr16[2],rule->srcIpV6AddrMask_.v6_addr16[3],
                                                    rule->srcIpV6AddrMask_.v6_addr16[4],rule->srcIpV6AddrMask_.v6_addr16[5],
                                                    rule->srcIpV6AddrMask_.v6_addr16[6],rule->srcIpV6AddrMask_.v6_addr16[7]
                                                    );
                                            seq_printf(s,"\tdip: %x:%x:%x:%x:%x:%x:%x:%x  dipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->dstIpV6Addr_.v6_addr16[0], rule->dstIpV6Addr_.v6_addr16[1], 
                                                    rule->dstIpV6Addr_.v6_addr16[2],rule->dstIpV6Addr_.v6_addr16[3],
                                                    rule->dstIpV6Addr_.v6_addr16[4],rule->dstIpV6Addr_.v6_addr16[5],
                                                    rule->dstIpV6Addr_.v6_addr16[6],rule->dstIpV6Addr_.v6_addr16[7],
                                                    rule->dstIpV6AddrMask_.v6_addr16[0], rule->dstIpV6AddrMask_.v6_addr16[1], 
                                                    rule->dstIpV6AddrMask_.v6_addr16[2],rule->dstIpV6AddrMask_.v6_addr16[3],
                                                    rule->dstIpV6AddrMask_.v6_addr16[4],rule->dstIpV6AddrMask_.v6_addr16[5],
                                                    rule->dstIpV6AddrMask_.v6_addr16[6],rule->dstIpV6AddrMask_.v6_addr16[7]
                                                    );
            
            
                                        }
                                        else if (RTL865X_ACL_IPV6_RANGE == rule->ruleType_)
                                        {
                                            seq_printf(s,"\tsipLB: %x:%x:%x:%x:%x:%x:%x:%x  sipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->srcIpV6AddrLB_.v6_addr16[0], rule->srcIpV6AddrLB_.v6_addr16[1], 
                                                    rule->srcIpV6AddrLB_.v6_addr16[2],rule->srcIpV6AddrLB_.v6_addr16[3],
                                                    rule->srcIpV6AddrLB_.v6_addr16[4],rule->srcIpV6AddrLB_.v6_addr16[5],
                                                    rule->srcIpV6AddrLB_.v6_addr16[6],rule->srcIpV6AddrLB_.v6_addr16[7],
                                                    rule->srcIpV6AddrUB_.v6_addr16[0], rule->srcIpV6AddrUB_.v6_addr16[1], 
                                                    rule->srcIpV6AddrUB_.v6_addr16[2],rule->srcIpV6AddrUB_.v6_addr16[3],
                                                    rule->srcIpV6AddrUB_.v6_addr16[4],rule->srcIpV6AddrUB_.v6_addr16[5],
                                                    rule->srcIpV6AddrUB_.v6_addr16[6],rule->srcIpV6AddrUB_.v6_addr16[7]
                                                    );
                                            seq_printf(s,"\tdipLB: %x:%x:%x:%x:%x:%x:%x:%x  dipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->dstIpV6AddrLB_.v6_addr16[0], rule->dstIpV6AddrLB_.v6_addr16[1], 
                                                    rule->dstIpV6AddrLB_.v6_addr16[2],rule->dstIpV6AddrLB_.v6_addr16[3],
                                                    rule->dstIpV6AddrLB_.v6_addr16[4],rule->dstIpV6AddrLB_.v6_addr16[5],
                                                    rule->dstIpV6AddrLB_.v6_addr16[6],rule->dstIpV6AddrLB_.v6_addr16[7],
                                                    rule->dstIpV6AddrUB_.v6_addr16[0], rule->dstIpV6AddrUB_.v6_addr16[1], 
                                                    rule->dstIpV6AddrUB_.v6_addr16[2],rule->dstIpV6AddrUB_.v6_addr16[3],
                                                    rule->dstIpV6AddrUB_.v6_addr16[4],rule->dstIpV6AddrUB_.v6_addr16[5],
                                                    rule->dstIpV6AddrUB_.v6_addr16[6],rule->dstIpV6AddrUB_.v6_addr16[7]
                                                    );
                                        }
                                        seq_printf(s,"\tFlowLabel: 0x%x   FlowLabelM: 0x%x\n",
                                                rule->ipv6FlowLabel_, rule->ipv6FlowLabelM_
                                                );
                                        seq_printf(s,"\tInvert: %d   Combine: %d   IPtunnel: %d\n",
                                                rule->ipv6Invert_, rule->ipv6Combine_,rule->ipv6IPtunnel_
                                                );
                                        seq_printf(s,"\tTrafficClassP: %d   TrafficClassM: %d   NextHeaderP: %d   NextHeaderM: %d\n",
                                                rule->ipv6TrafficClass_, rule->ipv6TrafficClassM_,rule->ipv6NextHeader_,rule->ipv6NextHeaderM_ 
                                                );
                                        seq_printf(s,"\tHTTPP: %d   HTTPM: %d   IdentSDIPP: %d   IdentSDIPM: %d\n",
                                                rule->ipv6HttpFilter_, rule->ipv6HttpFilterM_,rule->ipv6IdentSrcDstIp_,rule->ipv6IdentSrcDstIpM_ 
                                                );
                                        /* update  */
                                        ipv6_flag = 1;//handle the ipv6 second rule successful 
                                    }
                                    
                                }
                            break;
#endif

								default:
									seq_printf(s,"rule->ruleType_(0x%x)\n", rule->ruleType_);

						}

						switch (rule->actionType_)
						{
							case RTL865X_ACL_PERMIT:
							case RTL865X_ACL_REDIRECT_ETHER:
							case RTL865X_ACL_DROP:
							case RTL865X_ACL_TOCPU:
							case RTL865X_ACL_LEGACY_DROP:
							case RTL865X_ACL_DROPCPU_LOG:
							case RTL865X_ACL_MIRROR:
							case RTL865X_ACL_REDIRECT_PPPOE:
							case RTL865X_ACL_MIRROR_KEEP_MATCH:
								seq_printf(s,"\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", rule->netifIdx_, rule->pppoeIdx_, rule->L2Idx_);
								break;

							case RTL865X_ACL_PRIORITY:
								seq_printf(s,"\tprioirty: %d   ",  rule->priority_) ;
								break;

							case RTL865X_ACL_DEFAULT_REDIRECT:
								seq_printf(s,"\tnextHop:%d  ",  rule->nexthopIdx_);
								break;

							case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
							case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
							case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
							case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
								seq_printf(s,"\tratelimitIdx: %d  ",  rule->ratelimtIdx_);
								break;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                            case RTL865X_ACL_VID:
                                seq_printf(s,"\taclvid: %d  ", rule->aclvid_);
                                break;
#endif
							default:
								;

							}
						seq_printf(s,"pktOpApp: %d\n",  rule->pktOpApp_);

						rule = rule->next;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                        if (ipv6_flag == 1)
                        {
                            rule = rule->next;
                            ipv6_flag= 0;
                        }
#endif
					}
					seq_printf(s,"===========================\n");
					chain = chain->nextChain;
				}
				seq_printf(s,"--------------------------------\n\n");
			}

		}
	}

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	{
		if(virtualNetIf.valid == 1)
		{
			netif = &virtualNetIf;
			for(j = RTL865X_ACL_INGRESS; j<= RTL865X_ACL_EGRESS;j++)
			{
				seq_printf(s,"netif(%s),isEgress(%d):\n",netif->name,j);
				chain = netif->chainListHead[j];
				while(chain)
				{
					seq_printf(s,"\tchain:priority(%d),rulecnt(%d)\n",chain->priority,chain->ruleCnt);
					seq_printf(s,"===========================\n");
					rule = chain->head;
					while(rule)
					{
						switch(rule->ruleType_)
						{
							case RTL865X_ACL_MAC:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Ethernet", actionT[rule->actionType_]);
								seq_printf(s,"\tether type: %x   ether type mask: %x\n", rule->typeLen_, rule->typeLenMask_);
								seq_printf(s,"\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
										rule->dstMac_.octet[0], rule->dstMac_.octet[1], rule->dstMac_.octet[2],
										rule->dstMac_.octet[3], rule->dstMac_.octet[4], rule->dstMac_.octet[5],
										rule->dstMacMask_.octet[0], rule->dstMacMask_.octet[1], rule->dstMacMask_.octet[2],
										rule->dstMacMask_.octet[3], rule->dstMacMask_.octet[4], rule->dstMacMask_.octet[5]
										);

								seq_printf(s, "\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
										rule->srcMac_.octet[0], rule->srcMac_.octet[1], rule->srcMac_.octet[2],
										rule->srcMac_.octet[3], rule->srcMac_.octet[4], rule->srcMac_.octet[5],
										rule->srcMacMask_.octet[0], rule->srcMacMask_.octet[1], rule->srcMacMask_.octet[2],
										rule->srcMacMask_.octet[3], rule->srcMacMask_.octet[4], rule->srcMacMask_.octet[5]
									);
								break;

							case RTL865X_ACL_IP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP", actionT[rule->actionType_]);
								seq_printf(s, "\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
										rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
									);

								seq_printf(s,"\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
										rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
										rule->ipIdentSrcDstIpM_
										);
								seq_printf(s, "\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_);
									break;

							case RTL865X_ACL_IP_RANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IP Range", actionT[rule->actionType_]);
								seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   ipProto: %x   ipProtoM: %x   ipFlag: %x   ipFlagM: %x\n",
										rule->tos_, rule->tosMask_, rule->ipProto_, rule->ipProtoMask_, rule->ipFlag_, rule->ipFlagMask_
										);
								seq_printf(s,"\t<FOP:%x> <FOM:%x> <http:%x> <httpM:%x> <IdentSdip:%x> <IdentSdipM:%x> \n",
										rule->ipFOP_, rule->ipFOM_, rule->ipHttpFilter_, rule->ipHttpFilterM_, rule->ipIdentSrcDstIp_,
										rule->ipIdentSrcDstIpM_
										);
									seq_printf(s,"\t<DF:%x> <MF:%x>\n", rule->ipDF_, rule->ipMF_);
									break;
							case RTL865X_ACL_ICMP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
										rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_,
										rule->icmpCode_, rule->icmpCodeMask_);
								break;
							case RTL865X_ACL_ICMP_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "ICMP IP RANGE", actionT[rule->actionType_]);
								seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x   code: %x   codeM: %x\n",
										rule->tos_, rule->tosMask_, rule->icmpType_, rule->icmpTypeMask_,
										rule->icmpCode_, rule->icmpCodeMask_);
								break;
							case RTL865X_ACL_IGMP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
										rule->igmpType_, rule->igmpTypeMask_
										);
								break;


							case RTL865X_ACL_IGMP_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IGMP IP RANGE", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos: %x   TosM: %x   type: %x   typeM: %x\n", rule->tos_, rule->tosMask_,
										rule->igmpType_, rule->igmpTypeMask_
										);
								break;

							case RTL865X_ACL_TCP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
										rule->tcpDstPortLB_, rule->tcpDstPortUB_
										);
								seq_printf(s,"\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
										rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
										rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
										);
								break;
							case RTL865X_ACL_TCP_IPRANGE:
									seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "TCP IP RANGE", actionT[rule->actionType_]);
									seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
									seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
									seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->tcpSrcPortLB_, rule->tcpSrcPortUB_,
										rule->tcpDstPortLB_, rule->tcpDstPortUB_
										);
									seq_printf(s,"\tflag: %x  flagM: %x  <URG:%x> <ACK:%x> <PSH:%x> <RST:%x> <SYN:%x> <FIN:%x>\n",
										rule->tcpFlag_, rule->tcpFlagMask_, rule->tcpURG_, rule->tcpACK_,
										rule->tcpPSH_, rule->tcpRST_, rule->tcpSYN_, rule->tcpFIN_
									);
								break;

							case RTL865X_ACL_UDP:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx,"UDP", actionT[rule->actionType_]);
								seq_printf(s,"\tdip: %d.%d.%d.%d dipM: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d sipM: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
										rule->udpDstPortLB_, rule->udpDstPortUB_
										);
								break;
							case RTL865X_ACL_UDP_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "UDP IP RANGE", actionT[rule->actionType_]);
								seq_printf(s,"\tdipU: %d.%d.%d.%d dipL: %d.%d.%d.%d\n", (rule->dstIpAddr_>>24),
										((rule->dstIpAddr_&0x00ff0000)>>16), ((rule->dstIpAddr_&0x0000ff00)>>8),
										(rule->dstIpAddr_&0xff), (rule->dstIpAddrMask_>>24), ((rule->dstIpAddrMask_&0x00ff0000)>>16),
										((rule->dstIpAddrMask_&0x0000ff00)>>8), (rule->dstIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d sipL: %d.%d.%d.%d\n", (rule->srcIpAddr_>>24),
										((rule->srcIpAddr_&0x00ff0000)>>16), ((rule->srcIpAddr_&0x0000ff00)>>8),
										(rule->srcIpAddr_&0xff), (rule->srcIpAddrMask_>>24), ((rule->srcIpAddrMask_&0x00ff0000)>>16),
										((rule->srcIpAddrMask_&0x0000ff00)>>8), (rule->srcIpAddrMask_&0xff)
										);
								seq_printf(s,"\tTos:%x  TosM:%x  sportL:%d  sportU:%d  dportL:%d  dportU:%d\n",
										rule->tos_, rule->tosMask_, rule->udpSrcPortLB_, rule->udpSrcPortUB_,
										rule->udpDstPortLB_, rule->udpDstPortUB_
									);
								break;


							case RTL865X_ACL_SRCFILTER:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter", actionT[rule->actionType_]);
								seq_printf(s,"\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
										rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2],
										rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
										rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
										rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
										rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
										(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
										);
								seq_printf(s,"\tsip: %d.%d.%d.%d   sipM: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
										((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
										(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
										((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->srcFilterIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
								break;

							case RTL865X_ACL_SRCFILTER_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Source Filter(IP RANGE)", actionT[rule->actionType_]);
								seq_printf(s,"\tSMAC: %x:%x:%x:%x:%x:%x  SMACM: %x:%x:%x:%x:%x:%x\n",
										rule->srcFilterMac_.octet[0], rule->srcFilterMac_.octet[1], rule->srcFilterMac_.octet[2],
										rule->srcFilterMac_.octet[3], rule->srcFilterMac_.octet[4], rule->srcFilterMac_.octet[5],
										rule->srcFilterMacMask_.octet[0], rule->srcFilterMacMask_.octet[1], rule->srcFilterMacMask_.octet[2],
										rule->srcFilterMacMask_.octet[3], rule->srcFilterMacMask_.octet[4], rule->srcFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tsvidx: %d   svidxM: %x   sport: %d   sportM: %x   ProtoType: %x\n",
										rule->srcFilterVlanIdx_, rule->srcFilterVlanIdxMask_, rule->srcFilterPort_, rule->srcFilterPortMask_,
										(rule->srcFilterIgnoreL3L4_==TRUE? 2: (rule->srcFilterIgnoreL4_ == 1? 1: 0))
										);
								seq_printf(s,"\tsipU: %d.%d.%d.%d   sipL: %d.%d.%d.%d\n", (rule->srcFilterIpAddr_>>24),
										((rule->srcFilterIpAddr_&0x00ff0000)>>16), ((rule->srcFilterIpAddr_&0x0000ff00)>>8),
										(rule->srcFilterIpAddr_&0xff), (rule->srcFilterIpAddrMask_>>24),
										((rule->srcFilterIpAddrMask_&0x00ff0000)>>16), ((rule->srcFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->srcFilterIpAddrMask_&0xff)
										);
								seq_printf(s,"\tsportL: %d   sportU: %d\n", rule->srcFilterPortLowerBound_, rule->srcFilterPortUpperBound_);
								break;

							case RTL865X_ACL_DSTFILTER:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter", actionT[rule->actionType_]);
								seq_printf(s,"\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
										rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2],
										rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
										rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
										rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
										rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_,
										(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)),
										rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
										);
								seq_printf(s,"\tdip: %d.%d.%d.%d   dipM: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
										((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
										(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
										((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->dstFilterIpAddrMask_&0xff)
										);
								break;
							case RTL865X_ACL_DSTFILTER_IPRANGE:
								seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "Deatination Filter(IP Range)", actionT[rule->actionType_]);
								seq_printf(s,"\tDMAC: %x:%x:%x:%x:%x:%x  DMACM: %x:%x:%x:%x:%x:%x\n",
										rule->dstFilterMac_.octet[0], rule->dstFilterMac_.octet[1], rule->dstFilterMac_.octet[2],
										rule->dstFilterMac_.octet[3], rule->dstFilterMac_.octet[4], rule->dstFilterMac_.octet[5],
										rule->dstFilterMacMask_.octet[0], rule->dstFilterMacMask_.octet[1], rule->dstFilterMacMask_.octet[2],
										rule->dstFilterMacMask_.octet[3], rule->dstFilterMacMask_.octet[4], rule->dstFilterMacMask_.octet[5]
										);
								seq_printf(s,"\tdvidx: %d   dvidxM: %x  ProtoType: %x   dportL: %d   dportU: %d\n",
										rule->dstFilterVlanIdx_, rule->dstFilterVlanIdxMask_,
										(rule->dstFilterIgnoreL3L4_==TRUE? 2: (rule->dstFilterIgnoreL4_ == 1? 1: 0)),
										rule->dstFilterPortLowerBound_, rule->dstFilterPortUpperBound_
										);
								seq_printf(s,"\tdipU: %d.%d.%d.%d   dipL: %d.%d.%d.%d\n", (rule->dstFilterIpAddr_>>24),
										((rule->dstFilterIpAddr_&0x00ff0000)>>16), ((rule->dstFilterIpAddr_&0x0000ff00)>>8),
										(rule->dstFilterIpAddr_&0xff), (rule->dstFilterIpAddrMask_>>24),
										((rule->dstFilterIpAddrMask_&0x00ff0000)>>16), ((rule->dstFilterIpAddrMask_&0x0000ff00)>>8),
										(rule->dstFilterIpAddrMask_&0xff)
									);
								break;

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                            case RTL865X_ACL_IPV6: /* IP Rule Type: 0x0010 */
                            case RTL865X_ACL_IPV6_RANGE:
                                /* a ipv6 rule occupied  two entry, rule point to the first entry  
                                                    * and rule2 point to the second entry.
                                                    */
                                rule2 = rule->next;
                                ipv6_flag = 0;
                                
                                if (rule2 != NULL)//the second entry 
                                {
                                    if ((!rule2->ipv6EntryType_) && rule->ipv6EntryType_)
                                    {
                                        rule->dstIpV6Addr_.v6_addr32[3] = rule2->dstIpV6Addr_.v6_addr32[3];
                                        rule->dstIpV6Addr_.v6_addr32[2] = rule2->dstIpV6Addr_.v6_addr32[2];
                                        rule->dstIpV6Addr_.v6_addr32[1] = rule2->dstIpV6Addr_.v6_addr32[1];
                                        rule->dstIpV6Addr_.v6_addr32[0] = rule2->dstIpV6Addr_.v6_addr32[0];
                                        
                                        rule->dstIpV6AddrMask_.v6_addr32[3] = rule2->dstIpV6AddrMask_.v6_addr32[3];
                                        rule->dstIpV6AddrMask_.v6_addr32[2] = rule2->dstIpV6AddrMask_.v6_addr32[2];
                                        rule->dstIpV6AddrMask_.v6_addr32[1] = rule2->dstIpV6AddrMask_.v6_addr32[1];
                                        rule->dstIpV6AddrMask_.v6_addr32[0] = rule2->dstIpV6AddrMask_.v6_addr32[0];
            
                                        rule->ipv6TrafficClass_ = rule2->ipv6TrafficClass_;
                                        rule->ipv6TrafficClassM_ = rule2->ipv6TrafficClassM_; 
                                        rule->ipv6NextHeader_    = rule2->ipv6NextHeader_; 
                                        rule->ipv6NextHeaderM_   = rule2->ipv6NextHeaderM_; 
                                        rule->ipv6HttpFilter_    = rule2->ipv6HttpFilter_;
                                        rule->ipv6HttpFilterM_ = rule2->ipv6HttpFilterM_;
                                        rule->ipv6IdentSrcDstIp_ = rule2->ipv6IdentSrcDstIp_;
                                        rule->ipv6IdentSrcDstIpM_ = rule2->ipv6IdentSrcDstIpM_;
                                        /* ActionType and ActionField useless in entry0 */
                                        rule->actionType_ = rule2->actionType_;
                                        switch(rule->actionType_) {
                                    
                                        case RTL865X_ACL_PERMIT:
                                        case RTL865X_ACL_REDIRECT_ETHER:
                                        case RTL865X_ACL_DROP:
                                        case RTL865X_ACL_TOCPU:
                                        case RTL865X_ACL_LEGACY_DROP:
                                        case RTL865X_ACL_DROPCPU_LOG:
                                        case RTL865X_ACL_MIRROR:
                                        case RTL865X_ACL_REDIRECT_PPPOE:
                                        case RTL865X_ACL_MIRROR_KEEP_MATCH:
                                            rule->L2Idx_ = rule2->L2Idx_ ;
                                            rule->netifIdx_ =  rule2->netifIdx_;
                                            rule->pppoeIdx_ = rule2->pppoeIdx_;
                                             break;
                                    
                                        case RTL865X_ACL_DEFAULT_REDIRECT:
                                            rule->nexthopIdx_ = rule2->nexthopIdx_;
                                            break;
                                    
                                        case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
                                        case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
                                        case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
                                        case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
                                            rule->ratelimtIdx_ = rule2->ratelimtIdx_;
                                            break;
                                        case RTL865X_ACL_PRIORITY:
                                            rule->priority_ = rule2->priority_;
                                            break;
                                        case RTL865X_ACL_VID:
                                            rule->aclvid_ = rule2->aclvid_;
                                            break;                      
                                        }
                                        /* INV useless in entry 0 */
                                        rule->ipv6Invert_    = rule2->ipv6Invert_;
                                        //rule->ipv6Combine_  = rule2->ipv6Combine_;
                                        //rule->ipv6IPtunnel_ = rule2->ipv6IPtunnel_;
                                        seq_printf(s," [%d] rule type: %s   rule action: %s\n", rule->aclIdx, "IPv6", actionT[rule->actionType_]);
                                        if (RTL865X_ACL_IPV6 == rule->ruleType_)
                                        {
                                            seq_printf(s,"\tsip: %x:%x:%x:%x:%x:%x:%x:%x  sipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->srcIpV6Addr_.v6_addr16[0], rule->srcIpV6Addr_.v6_addr16[1], 
                                                    rule->srcIpV6Addr_.v6_addr16[2],rule->srcIpV6Addr_.v6_addr16[3],
                                                    rule->srcIpV6Addr_.v6_addr16[4],rule->srcIpV6Addr_.v6_addr16[5],
                                                    rule->srcIpV6Addr_.v6_addr16[6],rule->srcIpV6Addr_.v6_addr16[7],
                                                    rule->srcIpV6AddrMask_.v6_addr16[0], rule->srcIpV6AddrMask_.v6_addr16[1], 
                                                    rule->srcIpV6AddrMask_.v6_addr16[2],rule->srcIpV6AddrMask_.v6_addr16[3],
                                                    rule->srcIpV6AddrMask_.v6_addr16[4],rule->srcIpV6AddrMask_.v6_addr16[5],
                                                    rule->srcIpV6AddrMask_.v6_addr16[6],rule->srcIpV6AddrMask_.v6_addr16[7]
                                                    );
                                            seq_printf(s,"\tdip: %x:%x:%x:%x:%x:%x:%x:%x  dipM: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->dstIpV6Addr_.v6_addr16[0], rule->dstIpV6Addr_.v6_addr16[1], 
                                                    rule->dstIpV6Addr_.v6_addr16[2],rule->dstIpV6Addr_.v6_addr16[3],
                                                    rule->dstIpV6Addr_.v6_addr16[4],rule->dstIpV6Addr_.v6_addr16[5],
                                                    rule->dstIpV6Addr_.v6_addr16[6],rule->dstIpV6Addr_.v6_addr16[7],
                                                    rule->dstIpV6AddrMask_.v6_addr16[0], rule->dstIpV6AddrMask_.v6_addr16[1], 
                                                    rule->dstIpV6AddrMask_.v6_addr16[2],rule->dstIpV6AddrMask_.v6_addr16[3],
                                                    rule->dstIpV6AddrMask_.v6_addr16[4],rule->dstIpV6AddrMask_.v6_addr16[5],
                                                    rule->dstIpV6AddrMask_.v6_addr16[6],rule->dstIpV6AddrMask_.v6_addr16[7]
                                                    );
            
            
                                        }
                                        else if (RTL865X_ACL_IPV6_RANGE == rule->ruleType_)
                                        {
                                            seq_printf(s,"\tsipLB: %x:%x:%x:%x:%x:%x:%x:%x  sipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->srcIpV6AddrLB_.v6_addr16[0], rule->srcIpV6AddrLB_.v6_addr16[1], 
                                                    rule->srcIpV6AddrLB_.v6_addr16[2],rule->srcIpV6AddrLB_.v6_addr16[3],
                                                    rule->rcIpV6AddrLB_.v6_addr16[4],rule->srcIpV6AddrLB_.v6_addr16[5],
                                                    rule->srcIpV6AddrLB_.v6_addr16[6],rule->srcIpV6AddrLB_.v6_addr16[7],
                                                    rule->srcIpV6AddrUB_.v6_addr16[0], rule->srcIpV6AddrUB_.v6_addr16[1], 
                                                    rule->srcIpV6AddrUB_.v6_addr16[2],rule->srcIpV6AddrUB_.v6_addr16[3],
                                                    rule->srcIpV6AddrUB_.v6_addr16[4],rule->srcIpV6AddrUB_.v6_addr16[5],
                                                    rule->srcIpV6AddrUB_.v6_addr16[6],rule->srcIpV6AddrUB_.v6_addr16[7]
                                                    );
                                            seq_printf(s,"\tdipLB: %x:%x:%x:%x:%x:%x:%x:%x  dipUB: %x:%x:%x:%x:%x:%x:%x:%x\n",
                                                    rule->dstIpV6AddrLB_.v6_addr16[0], rule->dstIpV6AddrLB_.v6_addr16[1], 
                                                    rule->dstIpV6AddrLB_.v6_addr16[2],rule->dstIpV6AddrLB_.v6_addr16[3],
                                                    rule->dstIpV6AddrLB_.v6_addr16[4],rule->dstIpV6AddrLB_.v6_addr16[5],
                                                    rule->dstIpV6AddrLB_.v6_addr16[6],rule->dstIpV6AddrLB_.v6_addr16[7],
                                                    rule->dstIpV6AddrUB_.v6_addr16[0], rule->dstIpV6AddrUB_.v6_addr16[1], 
                                                    rule->dstIpV6AddrUB_.v6_addr16[2],rule->dstIpV6AddrUB_.v6_addr16[3],
                                                    rule->dstIpV6AddrUB_.v6_addr16[4],rule->dstIpV6AddrUB_.v6_addr16[5],
                                                    rule->dstIpV6AddrUB_.v6_addr16[6],rule->dstIpV6AddrUB_.v6_addr16[7]
                                                    );
                                        }
                                        seq_printf(s,"\tFlowLabel: 0x%x   FlowLabelM: 0x%x\n",
                                                rule->ipv6FlowLabel_, rule->ipv6FlowLabelM_
                                                );
                                        seq_printf(s,"\tInvert: %d   Combine: %d   IPtunnel: %d\n",
                                                rule->ipv6Invert_, rule->ipv6Combine_,rule->ipv6IPtunnel_
                                                );
                                        seq_printf(s,"\tTrafficClassP: %d   TrafficClassM: %d   NextHeaderP: %d   NextHeaderM: %d\n",
                                                rule->ipv6TrafficClass_, rule->ipv6TrafficClassM_,rule->ipv6NextHeader_,rule->ipv6NextHeaderM_ 
                                                );
                                        seq_printf(s,"\tHTTPP: %d   HTTPM: %d   IdentSDIPP: %d   IdentSDIPM: %d\n",
                                                rule->ipv6HttpFilter_, rule->ipv6HttpFilterM_,rule->ipv6IdentSrcDstIp_,rule->ipv6IdentSrcDstIpM_ 
                                                );
                                        /* update  */
                                        ipv6_flag = 1;//handle the ipv6 second rule successful 
                                    }
                                    
                                }
                            break;
#endif

								default:
									seq_printf(s,"rule->ruleType_(0x%x)\n", rule->ruleType_);

						}

						switch (rule->actionType_)
						{
							case RTL865X_ACL_PERMIT:
							case RTL865X_ACL_REDIRECT_ETHER:
							case RTL865X_ACL_DROP:
							case RTL865X_ACL_TOCPU:
							case RTL865X_ACL_LEGACY_DROP:
							case RTL865X_ACL_DROPCPU_LOG:
							case RTL865X_ACL_MIRROR:
							case RTL865X_ACL_REDIRECT_PPPOE:
							case RTL865X_ACL_MIRROR_KEEP_MATCH:
								seq_printf(s,"\tnetifIdx: %d   pppoeIdx: %d   l2Idx:%d  ", rule->netifIdx_, rule->pppoeIdx_, rule->L2Idx_);
								break;

							case RTL865X_ACL_PRIORITY:
								seq_printf(s,"\tprioirty: %d   ",  rule->priority_) ;
								break;

							case RTL865X_ACL_DEFAULT_REDIRECT:
								seq_printf(s,"\tnextHop:%d  ",  rule->nexthopIdx_);
								break;

							case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
							case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
							case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
							case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
								seq_printf(s,"\tratelimitIdx: %d  ",  rule->ratelimtIdx_);
								break;
                                
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                            case RTL865X_ACL_VID:
                                seq_printf(s,"\taclvid: %d  ", rule->aclvid_);
                                break;
#endif
							default:
								;

							}
						seq_printf(s,"pktOpApp: %d\n",  rule->pktOpApp_);

						rule = rule->next;
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
                        if (ipv6_flag == 1)
                        {
                            rule = rule->next;
                            ipv6_flag= 0;
                        }
#endif
					}
					seq_printf(s,"===========================\n");
					chain = chain->nextChain;
				}
				seq_printf(s,"--------------------------------\n\n");
			}

		}
	}
#endif
	return SUCCESS;

}


#if RTL_LAYERED_DRIVER_DEBUG

static int32 _rtl865x_print_allChain_allAcl(rtl865x_netif_local_t *netif)
{
	rtl865x_acl_chain_t *chain;
	rtl865x_AclRule_t *rule;
	int32 i;

	for(i = RTL865X_ACL_INGRESS; i<= RTL865X_ACL_EGRESS;i++)
	{
		printk("netif(%s),isEgress(%d):\n",netif->name,i);
		chain = netif->chainListHead[i];
		while(chain)
		{
			printk("\tchain:priority(%d),rulecnt(%d)\n",chain->priority,chain->ruleCnt);
			printk("===========================\n");
			rule = chain->head;
			while(rule)
			{
				printk("\tIdx%d:  aclIdx(%d),ruleType(%d),action(%d),direction(%d),pktOpApp(%d)\n", rule->aclIdx,rule->aclIdx,rule->ruleType_,rule->actionType_,rule->direction_,rule->pktOpApp_);
				rule = rule->next;
			}
			printk("===========================\n");
			chain = chain->nextChain;
		}
		printk("--------------------------------\n\n");
	}
	return SUCCESS;

}


static int32 _rtl865x_print_freeChainNum(void)
{
	rtl865x_acl_chain_t *entry;
	rtl865x_AclRule_t *acl;
	int32 freeCnt = 0;

	entry = freeChainHead;
	while(entry)
	{
		freeCnt++;
		entry = entry->nextChain;
	}
	printk("the free chain number is: %d\n",freeCnt);

	acl = freeAclList.freeHead;
	freeCnt = 0;
	while(acl)
	{
		freeCnt++;
		acl = acl->next;
	}
	printk("freeAclList total(%d),free(%d),in fact free(%d)\n",freeAclList.totalCnt,freeAclList.freeCnt,freeCnt);
	return SUCCESS;
}

#endif

#endif //CONFIG_RTL_LAYERED_DRIVER_ACL
static int32 _rtl865x_setAsicNetif(rtl865x_netif_local_t *entry)
{
	int32 retval = FAILED;
	rtl865x_tblAsicDrv_intfParam_t asicEntry;

	if(entry->is_slave == 1)
		return retval;
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(entry==(&virtualNetIf))
	{
		return FAILED;
	}
#endif
	memset(&asicEntry,0,sizeof(rtl865x_tblAsicDrv_intfParam_t));
	asicEntry.enableRoute = entry->enableRoute;
	asicEntry.inAclStart = entry->inAclStart;
	asicEntry.inAclEnd = entry->inAclEnd;
	asicEntry.outAclStart = entry->outAclStart;
	asicEntry.outAclEnd = entry->outAclEnd;
	//asicEntry.macAddr = entry->macAddr;
	memcpy(asicEntry.macAddr.octet,entry->macAddr.octet,ETHER_ADDR_LEN);
	asicEntry.macAddrNumber = entry->macAddrNumber;
	asicEntry.mtu = entry->mtu;
	asicEntry.vid = entry->vid;
	asicEntry.valid = entry->valid;

#if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	asicEntry.mtuV6         = entry->mtuV6;
	asicEntry.enableRouteV6 = entry->enableRouteV6;
#endif

#ifdef CONFIG_RTL_8367R_SUPPORT
	{
	/*
	  Prevent port blocked.
	  When 8367RB got attacked packet which source mac address
	  is equal to DUT LAN/WAN MAC address, the 8367RB will learn 
	  the SA to L2 table. Later, the incoming packet which DA =
	  DUT LAN/WAN MAC will be dropped until this L2 entry timeout.
	 */
	extern void set_8367r_L2(uint8 *mac, int intf_wan, int is_static);
	set_8367r_L2(&entry->macAddr.octet[0], entry->is_wan, 1);
	}
#elif defined(CONFIG_RTL_83XX_SUPPORT)
	{
	extern void set_83XX_L2(uint8 *mac, int intf_wan, int is_static);
	set_83XX_L2(&entry->macAddr.octet[0], entry->is_wan, 1);
	}
#endif

	retval = rtl8651_setAsicNetInterface( entry->asicIdx, &asicEntry);
	return retval;

}

rtl865x_netif_local_t *_rtl865x_getSWNetifByName(char *name)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;

	if(name == NULL)
		return NULL;

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		//printk("%s:%d,i(%d),valid(%d),ifname(%s),strlen of name(%d), netifTbl(0x%p),netifTblName(%s)\n",__FUNCTION__,__LINE__,i,netifTbl[i].valid,name,strlen(name),&netifTbl[i],netifTbl[i].name);
		if(netifTbl[i].valid == 1 && strlen(name) == strlen(netifTbl[i].name) && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			netif = &netifTbl[i];
			break;
		}
	}

	#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(virtualNetIf.valid == 1 && strlen(name) == strlen(virtualNetIf.name) && memcmp(virtualNetIf.name,name,strlen(name)) == 0)
	{
		netif = &virtualNetIf;
	}
	#endif

	return netif;
}

rtl865x_netif_local_t *_rtl865x_getNetifByName(char *name)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;

	if(name == NULL)
		return NULL;

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		//printk("i(%d),ifname(%s),netifTbl(0x%p),netifTblName(%s)\n",i,name,&netifTbl[i],netifTbl[i].name);
		if(netifTbl[i].valid == 1 && strlen(name) == strlen(netifTbl[i].name) && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			if(netifTbl[i].is_slave == 0)
				netif = &netifTbl[i];
			else
			{
				netif = netifTbl[i].master;
			}
			break;
		}
	}
	#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(virtualNetIf.valid == 1 && strlen(name) == strlen(virtualNetIf.name) && memcmp(virtualNetIf.name,name,strlen(name)) == 0)
	{
		netif = &virtualNetIf;

	}
	#endif
	return netif;
}


#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT) || defined (CONFIG_RTL_HW_6RD_SUPPORT)
rtl865x_netif_local_t *_rtl865x_getNetifByType(int iftype)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;

	if(iftype <IF_6RD||iftype >IF_DSLT)
		return NULL;

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		//printk("i(%d),ifname(%s),netifTbl(0x%p),netifTblName(%s)\n",i,name,&netifTbl[i],netifTbl[i].name);
		if(netifTbl[i].valid == 1 && netifTbl[i].if_type== iftype )
		{			
			netif = &netifTbl[i];			
			break;
		}
	}
	return netif;
}
int32 rtl865x_updatev6SwNetif(char *netifName,int iftype)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *netifold = NULL;
	netifold=_rtl865x_getNetifByType(iftype);
	if(netifold == NULL)
	{
		printk("retval:%d,%s,%d,[%s]:[%d].\n",retval,netifName,iftype,__FUNCTION__,__LINE__);
		return retval;
	}
	
	strcpy(netifold->name, netifName);
	netifold->if_type = iftype;
	retval = SUCCESS;
	_rtl865x_setDefaultWanNetif(netifName);
	return retval;
}
#endif
#endif


rtl865x_netif_local_t *_rtl865x_getDefaultWanNetif(void)
{
	int32 i;
	rtl865x_netif_local_t *firstWan, *defNetif;
	firstWan = defNetif = NULL;

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		//printk("i(%d),netifTbl(0x%p)\n",i,&netifTbl[i]);
		if(netifTbl[i].valid == 1 && netifTbl[i].is_wan == 1 && firstWan == NULL)
			firstWan = &netifTbl[i];

		if(netifTbl[i].valid == 1 && netifTbl[i].is_defaultWan == 1)
		{
			defNetif = &netifTbl[i];
			break;
		}
	}

	/*if not found default wan, return wan interface first found*/
	if(defNetif == NULL)
	{
		defNetif = firstWan;
	}
	return defNetif;
}

int32 _rtl865x_setDefaultWanNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getSWNetifByName(name);

	//printk("set default wan interface....(%s)\n",name);
	if(entry)
		entry->is_defaultWan = 1;
	return SUCCESS;
}

int32 _rtl865x_clearDefaultWanNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getSWNetifByName(name);

	//printk("set default wan interface....(%s)\n",name);
	if(entry)
		entry->is_defaultWan = 0;

	return SUCCESS;
}

static int32 _rtl865x_attachMasterNetif(char *slave, char *master)
{
	rtl865x_netif_local_t *slave_netif, *master_netif;

	slave_netif = _rtl865x_getSWNetifByName(slave);
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT) && defined(CONFIG_RTL_PPPOE_HWACC)
	if (NULL == slave_netif) {
		extern int32 rtl_add_ppp_netif(char *ifname);
		if (SUCCESS != rtl_add_ppp_netif(slave)) {
			//print("Leave %s @ %d\n",__FUNCTION__,__LINE__);
			return RTL_EENTRYNOTFOUND;
		}
		//get netif again, we have created a new netif through rtl_add_ppp_netif()
		slave_netif = _rtl865x_getSWNetifByName(slave);
	}
#endif
	master_netif = _rtl865x_getNetifByName(master);

	if(slave_netif == NULL || master_netif == NULL)
		return RTL_EENTRYNOTFOUND;

	//printk("===%s(%d),slave(%s),master(%s),slave_netif->master(0x%p)\n",__FUNCTION__,__LINE__,slave,master,slave_netif->master);
	if(slave_netif->master != NULL)
		return RTL_EENTRYALREADYEXIST;

	slave_netif ->master = master_netif;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	slave_netif->vid = master_netif->vid;
#endif

	return SUCCESS;
}

static int32 _rtl865x_detachMasterNetif(char *slave)
{
	rtl865x_netif_local_t *slave_netif;

	slave_netif = _rtl865x_getSWNetifByName(slave);

	if(slave_netif == NULL)
		return RTL_EENTRYNOTFOUND;

	slave_netif ->master = NULL;

	return SUCCESS;
}

int32 _rtl865x_addNetif(rtl865x_netif_t *netif)
{
	rtl865x_netif_local_t *entry;
	int32 retval = FAILED;
	int32 i;
#if defined (CONFIG_RTL_LOCAL_PUBLIC) || defined(CONFIG_RTL_MULTIPLE_WAN)
	int asicIdx;
#if defined(CONFIG_RTL_LOCAL_PUBLIC)
	rtl865xc_tblAsic_netifTable_t asicEntry;
#endif
#endif
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	int32 sw_netif_idx = 0;
#endif

	if(netif == NULL)
		return RTL_EINVALIDINPUT;

	/*duplicate entry....*/
	entry = _rtl865x_getSWNetifByName(netif->name);
	if(entry)
		return RTL_EENTRYALREADYEXIST;

	/*get netif buffer*/
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		if(netifTbl[i].valid == 0) {
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
			sw_netif_idx = i;
#endif
			break;
		}	
	}

#if defined (CONFIG_RTL_LOCAL_PUBLIC) || defined(CONFIG_RTL_MULTIPLE_WAN)
#if defined(CONFIG_RTL_LOCAL_PUBLIC)
	for (asicIdx=0;asicIdx<RTL865XC_NETIFTBL_SIZE;asicIdx++)
	{
		_rtl8651_readAsicEntry(TYPE_NETINTERFACE_TABLE, asicIdx, &asicEntry);
		if (asicEntry.valid==0)
		{
			break;
		}
	}
#endif

	if(netif->forMacBasedMCast==TRUE)
	{
		asicIdx=RTL865XC_NETIFTBL_SIZE-1;
	}
#if defined(CONFIG_RTL_MULTIPLE_WAN)
	else
	{
		asicIdx = i;
	}
#endif
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	if(i == NETIF_SW_NUMBER)
#else
	if(i == NETIF_NUMBER)
#endif
		return RTL_ENOFREEBUFFER;

	/*add new entry*/
	entry = &netifTbl[i];

	#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	memset(entry, 0, sizeof(rtl865x_netif_local_t));
	#endif

	entry->valid = 1;
	entry->mtu = netif->mtu;
	entry->if_type = netif->if_type;
	entry->macAddr = netif->macAddr;
	entry->vid = netif->vid;
	entry->is_wan = netif->is_wan;
	entry->dmz = netif->dmz;
	entry->is_slave = netif->is_slave;

    #if defined (CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT) || defined(CONFIG_RTL_HARDWARE_MULTICAST)
	#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
	entry->enableRouteV6 = rtk_hwL3v6_enable?1:0;
	#endif
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
    entry->enableRouteV6 = (rtl8651_getAsicOperationLayer()>2)?(netif->enableRouteV6):0;
	#endif
	#else
    entry->enableRouteV6 = netif->enableRouteV6;
	#endif
    entry->mtuV6         = netif->mtuV6;
    #endif

	memcpy(entry->name,netif->name,MAX_IFNAMESIZE);
//	printk("netif:%s,%d,[%s]:[%d].\n",netif->name,netif->is_slave,__FUNCTION__,__LINE__);
	/*private number...*/
#if defined (CONFIG_RTL_LOCAL_PUBLIC) ||defined(CONFIG_RTL_MULTIPLE_WAN)
	entry->asicIdx=asicIdx;
#else
	entry->asicIdx = i;
#endif
#if (defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS) || defined(CONFIG_RTL_HARDWARE_MULTICAST)) && defined(CONFIG_OPENWRT_SDK)
	entry->enableRoute = (rtl8651_getAsicOperationLayer()>2)?(netif->enableRoute):0;
#else
	entry->enableRoute = netif->enableRoute;
#endif
	entry->macAddrNumber = 1;
	entry->inAclEnd = entry->inAclStart = entry->outAclEnd = entry->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/
	entry->refCnt = 1;
	entry->master = NULL;

#ifdef 	CONFIG_RTL_LAYERED_DRIVER_ACL
	//ingress acl chains head
	entry->chainListHead[RTL865X_ACL_INGRESS] = NULL;

	//init egress acl
	entry->chainListHead[RTL865X_ACL_EGRESS] = NULL;
#endif

	/*only write master interface into ASIC*/
	if(entry->is_slave == 0)
	{
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
		{			
			int hw_netif_idx=0;

			/* find new available hw asic index */
			for (hw_netif_idx=0; hw_netif_idx<NETIF_NUMBER; hw_netif_idx++)		
			{
				if (Netif_Asic2Sw_Mapping[hw_netif_idx] == -1)
					break;		
			}
			
			if (hw_netif_idx == NETIF_NUMBER) {				
				printk("%s[%d] Warning , there is no more empty entry in Asic Netif Table\n ", __FUNCTION__, __LINE__);
				return RTL_ENOFREEBUFFER;
			}	
#if defined(CONFIG_RTL_MAC_BASED_NETIF)
			if (netif->forMacBasedMCast == TRUE)
				hw_netif_idx = RTL865XC_NETIFTBL_SIZE-1;
#endif
			Netif_Asic2Sw_Mapping[hw_netif_idx] = sw_netif_idx ;
			entry->asicIdx = hw_netif_idx;
		}
#endif
		retval = _rtl865x_setAsicNetif(entry);
		if(retval == SUCCESS)
			rtl865x_referVlan(entry->vid);

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
		/*register 2 ingress chains: system/user*/
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_SYSTEM_USED, RTL865X_ACL_INGRESS);
#if RTL_LAYERED_DRIVER_DEBUG
		printk("register system acl chain, return %d\n",retval);
		_rtl865x_print_freeChainNum();
#endif
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_USER_USED, RTL865X_ACL_INGRESS);
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_USER_USED, RTL865X_ACL_EGRESS);

#if RTL_LAYERED_DRIVER_DEBUG
		printk("register user acl chain, return %d\n",retval);
		_rtl865x_print_freeChainNum();
#endif

#if defined(CONFIG_RTL_HW_QOS_SUPPORT)
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_QOS_USED0, RTL865X_ACL_INGRESS);
		retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_QOS_USED1, RTL865X_ACL_INGRESS);
#endif
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL
	}


	return SUCCESS;
}

static int32 _rtl865x_delNetif(char *name)
{
	rtl865x_netif_local_t *entry;
	int32 retval = FAILED;

	/*FIXME:hyking, get swNetif entry.....*/
	entry = _rtl865x_getSWNetifByName(name);
	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	if(entry->refCnt > 1)
	{
		printk("name(%s),refcnt(%d)\n",name,entry->refCnt);
		return RTL_EREFERENCEDBYOTHER;
	}

	if(entry->is_slave == 0)
	{
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
		retval = _rtl865x_unRegister_all_aclChain(name);
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
		Netif_Asic2Sw_Mapping[entry->asicIdx] = -1;
#endif
		retval = rtl865x_delNetInterfaceByVid(entry->vid);
		if(retval == SUCCESS)
		{
			rtl865x_deReferVlan(entry->vid);

			/*flush acl*/
			#if 0
			do_eventAction(EV_DEL_NETIF, (void*)entry);
			#else
			rtl865x_raiseEvent(EVENT_DEL_NETIF, (void*)entry);
			#endif
		}

		/*now delete all slave interface whose master is the deleting master interface*/
		{
			int32 i ;

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
			{
				if(netifTbl[i].valid == 1 && netifTbl[i].is_slave == 1 && netifTbl[i].master == entry)
					netifTbl[i].master = NULL;
			}
		}
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
#if RTL_LAYERED_DRIVER_DEBUG
		printk("unregist all acl chain, return %d\n",retval);
		_rtl865x_print_freeChainNum();
#endif
#endif
	}

	//entry->valid = 0;
	memset(entry,0,sizeof(rtl865x_netif_local_t));
	retval = SUCCESS;

	return retval;
}

static int32 _rtl865x_referNetif(char *ifName)
{
	rtl865x_netif_local_t *entry;
	if(ifName == NULL)
		return FAILED;

	entry = _rtl865x_getSWNetifByName(ifName);
	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->refCnt++;
	return SUCCESS;
}

static int32 _rtl865x_deReferNetif(char *ifName)
{
	rtl865x_netif_local_t *entry;
	if(ifName == NULL)
		return FAILED;

	entry = _rtl865x_getSWNetifByName(ifName);
	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->refCnt--;

	return SUCCESS;
}

static int32 _rtl865x_setNetifVid(char *name, uint16 vid)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL || vid < 1 ||vid > 4095)
		return RTL_EINVALIDINPUT;

	entry = _rtl865x_getSWNetifByName(name);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	if(entry->vid > 0 && entry->vid <= 4095)
		rtl865x_deReferVlan(entry->vid);

	entry->vid = vid;

	/*update asic table*/
	if (entry->is_slave)
		return SUCCESS;
	else
		return _rtl865x_setAsicNetif(entry);
}


static int32 _rtl865x_setNetifType(char *name, uint32 ifType)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL || ifType <= IF_NONE ||ifType > IF_L2TP)
		return RTL_EINVALIDINPUT;

	entry = _rtl865x_getSWNetifByName(name);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->if_type = ifType;

	return SUCCESS;
}

int32 _rtl865x_setNetifMac(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *entry;

	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	entry = _rtl865x_getNetifByName(netif->name);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->macAddr = netif->macAddr;

	/*update asic table*/
	retval = _rtl865x_setAsicNetif(entry);

	return retval;

}

int32 _rtl865x_setNetifMtu(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getNetifByName(netif->name);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->mtu = netif->mtu;
	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	entry->mtuV6 = netif->mtuV6;
	#endif
	/*update asic table*/
	retval = _rtl865x_setAsicNetif(entry);

	return retval;

}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
int32 _rtl865x_setNetifMtuV6(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getNetifByName(netif->name);

	if(entry == NULL)
		return RTL_EENTRYNOTFOUND;

	entry->mtuV6 = netif->mtuV6;

	/*update asic table*/
	retval = _rtl865x_setAsicNetif(entry);
	return retval;
}
#endif

int32 _rtl865x_getNetifIdxByVid(uint16 vid)
{
	int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		if(netifTbl[i].valid == 1 && netifTbl[i].vid == vid)
			break;
	}

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	if(i == NETIF_SW_NUMBER)
#else
	if(i == NETIF_NUMBER)
#endif
		return -1;

	return i;
}

int32 _rtl865x_getNetifIdxByName(uint8 *name)
{
	int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		if(netifTbl[i].valid == 1 && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
			break;
	}

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	if(i == NETIF_SW_NUMBER)
#else
	if(i == NETIF_NUMBER)
#endif
		return -1;

	return i;
}

int32 _rtl865x_getNetifIdxByNameExt(uint8 *name)
{
	int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		if(netifTbl[i].valid == 1 && memcmp(netifTbl[i].name,name,strlen(name)) == 0)
		{
			if (netifTbl[i].is_slave==TRUE)
			{
				if(netifTbl[i].master)
					return _rtl865x_getNetifIdxByNameExt(netifTbl[i].master->name);
				else
					return -1;
			}
			else
				return i;
		}
	}

	return -1;
}

int32 _rtl865x_getAclFromAsic(int32 index, rtl865x_AclRule_t *rule)
{
	rtl865xc_tblAsic_aclTable_t	   entry;

	if(index >=	RTL865X_ACL_MAX_NUMBER + RTL865X_ACL_RESERVED_NUMBER ||	rule ==	NULL)
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_ACL_RULE_TABLE,	index, &entry);
	bzero(rule,	sizeof(rtl865x_AclRule_t));

#if	defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	rule->ruleType_	 = entry.ruleType  ;
	rule->ruleType_	|= entry.ruleType1<<4;
	switch(rule->ruleType_)	{
#else	 
	switch(entry.ruleType) {
#endif
	case RTL865X_ACL_MAC: /* Ethernet rule type	*/
		 rule->dstMac_.octet[0]		= entry.is.ETHERNET.dMacP47_32 >> 8;
		 rule->dstMac_.octet[1]		= entry.is.ETHERNET.dMacP47_32 & 0xff;
		 rule->dstMac_.octet[2]		= entry.is.ETHERNET.dMacP31_16 >> 8;
		 rule->dstMac_.octet[3]		= entry.is.ETHERNET.dMacP31_16 & 0xff;
		 rule->dstMac_.octet[4]		= entry.is.ETHERNET.dMacP15_0 >> 8;
		 rule->dstMac_.octet[5]		= entry.is.ETHERNET.dMacP15_0 &	0xff;
		 rule->dstMacMask_.octet[0]	= entry.is.ETHERNET.dMacM47_32 >> 8;
		 rule->dstMacMask_.octet[1]	= entry.is.ETHERNET.dMacM47_32 & 0xff;
		 rule->dstMacMask_.octet[2]	= entry.is.ETHERNET.dMacM31_16 >> 8;
		 rule->dstMacMask_.octet[3]	= entry.is.ETHERNET.dMacM31_16 & 0xff;
		 rule->dstMacMask_.octet[4]	= entry.is.ETHERNET.dMacM15_0 >> 8;
		 rule->dstMacMask_.octet[5]	= entry.is.ETHERNET.dMacM15_0 &	0xff;
		 rule->srcMac_.octet[0]		= entry.is.ETHERNET.sMacP47_32 >> 8;
		 rule->srcMac_.octet[1]		= entry.is.ETHERNET.sMacP47_32 & 0xff;
		 rule->srcMac_.octet[2]		= entry.is.ETHERNET.sMacP31_16 >> 8;
		 rule->srcMac_.octet[3]		= entry.is.ETHERNET.sMacP31_16 & 0xff;
		 rule->srcMac_.octet[4]		= entry.is.ETHERNET.sMacP15_0 >> 8;
		 rule->srcMac_.octet[5]		= entry.is.ETHERNET.sMacP15_0 &	0xff;
		 rule->srcMacMask_.octet[0]	= entry.is.ETHERNET.sMacM47_32 >> 8;
		 rule->srcMacMask_.octet[1]	= entry.is.ETHERNET.sMacM47_32 & 0xff;
		 rule->srcMacMask_.octet[2]	= entry.is.ETHERNET.sMacM31_16 >> 8;
		 rule->srcMacMask_.octet[3]	= entry.is.ETHERNET.sMacM31_16 & 0xff;
		 rule->srcMacMask_.octet[4]	= entry.is.ETHERNET.sMacM15_0 >> 8;
		 rule->srcMacMask_.octet[5]	= entry.is.ETHERNET.sMacM15_0 &	0xff;
		 rule->typeLen_				= entry.is.ETHERNET.ethTypeP;
		 rule->typeLenMask_			= entry.is.ETHERNET.ethTypeM;
		 rule->ruleType_			= entry.ruleType;
		 break;

	case RTL865X_ACL_IP: /*	IP mask	rule type */
	case RTL865X_ACL_IP_RANGE: /* IP range rule	type*/
		 rule->tos_			= entry.is.L3L4.is.IP.IPTOSP;
		 rule->tosMask_		= entry.is.L3L4.is.IP.IPTOSM;
		 rule->ipProto_		= entry.is.L3L4.is.IP.IPProtoP;
		 rule->ipProtoMask_	= entry.is.L3L4.is.IP.IPProtoM;
		 rule->ipFlag_		= entry.is.L3L4.is.IP.IPFlagP;
		 rule->ipFlagMask_	= entry.is.L3L4.is.IP.IPFlagM;
		 rule->ipFOP_ =	entry.is.L3L4.is.IP.FOP;
		 rule->ipFOM_ =	entry.is.L3L4.is.IP.FOM;
		 rule->ipHttpFilterM_ =	entry.is.L3L4.is.IP.HTTPM;
		 rule->ipHttpFilter_  =	entry.is.L3L4.is.IP.HTTPP;
		 rule->ipIdentSrcDstIp_	= entry.is.L3L4.is.IP.identSDIPM;
		 rule->ruleType_= entry.ruleType;
		 goto l3l4_shared;

	case RTL865X_ACL_ICMP: /* ICMP	(ip	is mask) rule type */
	case RTL865X_ACL_ICMP_IPRANGE: /* ICMP (ip is  range) rule type	*/
		 rule->tos_	= entry.is.L3L4.is.ICMP.IPTOSP;
		 rule->tosMask_	= entry.is.L3L4.is.ICMP.IPTOSM;
		 rule->icmpType_ = entry.is.L3L4.is.ICMP.ICMPTypeP;
		 rule->icmpTypeMask_ = entry.is.L3L4.is.ICMP.ICMPTypeM;
		 rule->icmpCode_ = entry.is.L3L4.is.ICMP.ICMPCodeP;
		 rule->icmpCodeMask_ = entry.is.L3L4.is.ICMP.ICMPCodeM;
		 rule->ruleType_ = entry.ruleType;
		 goto l3l4_shared;

	case RTL865X_ACL_IGMP: /* IGMP (ip is mask)	rule type */
	case RTL865X_ACL_IGMP_IPRANGE: /* IGMP (ip is range) rule type */
		 rule->tos_	= entry.is.L3L4.is.IGMP.IPTOSP;
		 rule->tosMask_	= entry.is.L3L4.is.IGMP.IPTOSM;
		 rule->igmpType_ = entry.is.L3L4.is.IGMP.IGMPTypeP;
		 rule->igmpTypeMask_ = entry.is.L3L4.is.IGMP.IGMPTypeM;
		 rule->ruleType_ = entry.ruleType;
		 goto l3l4_shared;

	case RTL865X_ACL_TCP: /* TCP rule type */
	case RTL865X_ACL_TCP_IPRANGE:
		 rule->tos_	= entry.is.L3L4.is.TCP.IPTOSP;
		 rule->tosMask_	= entry.is.L3L4.is.TCP.IPTOSM;
		 rule->tcpFlag_	= entry.is.L3L4.is.TCP.TCPFlagP;
		 rule->tcpFlagMask_	= entry.is.L3L4.is.TCP.TCPFlagM;
		 rule->tcpSrcPortUB_ = entry.is.L3L4.is.TCP.TCPSPUB;
		 rule->tcpSrcPortLB_ = entry.is.L3L4.is.TCP.TCPSPLB;
		 rule->tcpDstPortUB_ = entry.is.L3L4.is.TCP.TCPDPUB;
		 rule->tcpDstPortLB_ = entry.is.L3L4.is.TCP.TCPDPLB;
		 rule->ruleType_ = entry.ruleType;
		 goto l3l4_shared;

	case RTL865X_ACL_UDP: /* UDP rule type */
	case RTL865X_ACL_UDP_IPRANGE:
		 rule->tos_	= entry.is.L3L4.is.UDP.IPTOSP;
		 rule->tosMask_	= entry.is.L3L4.is.UDP.IPTOSM;
		 rule->udpSrcPortUB_ = entry.is.L3L4.is.UDP.UDPSPUB;
		 rule->udpSrcPortLB_ = entry.is.L3L4.is.UDP.UDPSPLB;
		 rule->udpDstPortUB_ = entry.is.L3L4.is.UDP.UDPDPUB;
		 rule->udpDstPortLB_ = entry.is.L3L4.is.UDP.UDPDPLB;
		 rule->ruleType_ = entry.ruleType;
l3l4_shared:
		rule->srcIpAddr_ = entry.is.L3L4.sIPP;
		rule->srcIpAddrMask_ = entry.is.L3L4.sIPM;
		rule->dstIpAddr_ = entry.is.L3L4.dIPP;
		rule->dstIpAddrMask_ = entry.is.L3L4.dIPM;
		break;

	case RTL865X_ACL_SRCFILTER:	/* Source Filter */
	case RTL865X_ACL_SRCFILTER_IPRANGE:
		 rule->srcFilterMac_.octet[0]	  =	entry.is.SRC_FILTER.sMacP47_32 >> 8;
		 rule->srcFilterMac_.octet[1]	  =	entry.is.SRC_FILTER.sMacP47_32 & 0xff;
		 rule->srcFilterMac_.octet[2]	  =	entry.is.SRC_FILTER.sMacP31_16 >> 8;
		 rule->srcFilterMac_.octet[3]	  =	entry.is.SRC_FILTER.sMacP31_16 & 0xff;
		 rule->srcFilterMac_.octet[4]	  =	entry.is.SRC_FILTER.sMacP15_0 >> 8;
		 rule->srcFilterMac_.octet[5]	  =	entry.is.SRC_FILTER.sMacP15_0 &	0xff;
		 if	( entry.is.SRC_FILTER.sMacM3_0&0x8)
		{
			 rule->srcFilterMacMask_.octet[0] =	0xff;
			 rule->srcFilterMacMask_.octet[1] =	0xff;
			 rule->srcFilterMacMask_.octet[2] =	0xff;
			 rule->srcFilterMacMask_.octet[3] =	0xff;
			 rule->srcFilterMacMask_.octet[4] =	0xff;
			 rule->srcFilterMacMask_.octet[5] =	0xF0|entry.is.SRC_FILTER.sMacM3_0;
		}
		 else
		{
			 rule->srcFilterMacMask_.octet[0] =	0x0;
			 rule->srcFilterMacMask_.octet[1] =	0x0;
			 rule->srcFilterMacMask_.octet[2] =	0x0;
			 rule->srcFilterMacMask_.octet[3] =	0x0;
			 rule->srcFilterMacMask_.octet[4] =	0x0;
			 rule->srcFilterMacMask_.octet[5] =	entry.is.SRC_FILTER.sMacM3_0;
		}

		 rule->srcFilterPort_ =	entry.is.SRC_FILTER.spaP;
		 rule->srcFilterVlanIdx_ = entry.is.SRC_FILTER.sVidP;
		 rule->srcFilterVlanIdxMask_ = entry.is.SRC_FILTER.sVidM;
		 if(entry.is.SRC_FILTER.protoType == 2)	rule->srcFilterIgnoreL4_ = 1;
		 else if(entry.is.SRC_FILTER.protoType == 1) rule->srcFilterIgnoreL3L4_	= 1;
		 rule->srcFilterIpAddr_	= entry.is.SRC_FILTER.sIPP;
		 rule->srcFilterIpAddrMask_	= entry.is.SRC_FILTER.sIPM;
		 rule->srcFilterPortUpperBound_	= entry.is.SRC_FILTER.SPORTUB;
		 rule->srcFilterPortLowerBound_	= entry.is.SRC_FILTER.SPORTLB;
		 rule->ruleType_ = entry.ruleType;
		 break;

	case RTL865X_ACL_DSTFILTER:	/* Destination Filter */
	case RTL865X_ACL_DSTFILTER_IPRANGE:	/* Destination Filter(IP range)	*/
		 rule->dstFilterMac_.octet[0]	  =	entry.is.DST_FILTER.dMacP47_32 >> 8;
		 rule->dstFilterMac_.octet[1]	  =	entry.is.DST_FILTER.dMacP47_32 & 0xff;
		 rule->dstFilterMac_.octet[2]	  =	entry.is.DST_FILTER.dMacP31_16 >> 8;
		 rule->dstFilterMac_.octet[3]	  =	entry.is.DST_FILTER.dMacP31_16 & 0xff;
		 rule->dstFilterMac_.octet[4]	  =	entry.is.DST_FILTER.dMacP15_0 >> 8;
		 rule->dstFilterMac_.octet[5]	  =	entry.is.DST_FILTER.dMacP15_0 &	0xff;
		 if	( entry.is.DST_FILTER.dMacM3_0&0x8)
		{
			 rule->dstFilterMacMask_.octet[0] =	0xff;
			 rule->dstFilterMacMask_.octet[1] =	0xff;
			 rule->dstFilterMacMask_.octet[2] =	0xff;
			 rule->dstFilterMacMask_.octet[3] =	0xff;
			 rule->dstFilterMacMask_.octet[4] =	0xff;
			 rule->dstFilterMacMask_.octet[5] =	0xF0|entry.is.DST_FILTER.dMacM3_0;
		}
		 else
		{
			 rule->dstFilterMacMask_.octet[0] =	0x0;
			 rule->dstFilterMacMask_.octet[1] =	0x0;
			 rule->dstFilterMacMask_.octet[2] =	0x0;
			 rule->dstFilterMacMask_.octet[3] =	0x0;
			 rule->dstFilterMacMask_.octet[4] =	0x0;
			 rule->dstFilterMacMask_.octet[5] =	entry.is.DST_FILTER.dMacM3_0;
		}


		 rule->dstFilterVlanIdx_ = entry.is.DST_FILTER.vidP;
		 rule->dstFilterVlanIdxMask_ = entry.is.DST_FILTER.vidM;
		 if(entry.is.DST_FILTER.protoType == 1)	rule->dstFilterIgnoreL3L4_ = 1;
		 else if(entry.is.DST_FILTER.protoType == 2) rule->dstFilterIgnoreL4_ =	1;
		 rule->dstFilterIpAddr_	= entry.is.DST_FILTER.dIPP;
		 rule->dstFilterIpAddrMask_	= entry.is.DST_FILTER.dIPM;
		 rule->dstFilterPortUpperBound_	= entry.is.DST_FILTER.DPORTUB;
		 rule->dstFilterPortLowerBound_	= entry.is.DST_FILTER.DPORTLB;
		 rule->ruleType_ = entry.ruleType;
		 break;

#if	defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_IPV6: /* IP Rule Type:	0x0010 */
	case RTL865X_ACL_IPV6_RANGE:

		if(entry.ipv6ETY0)//first entry
		{
			rule->srcIpV6Addr_.v6_addr32[3]	= entry.is.L3V6.is.entry0.sip_addr31_0;
			rule->srcIpV6Addr_.v6_addr32[2]	= entry.is.L3V6.is.entry0.sip_addr63_32;
			rule->srcIpV6Addr_.v6_addr32[1]	= entry.is.L3V6.is.entry0.sip_addr95_64;
			rule->srcIpV6Addr_.v6_addr32[0]	= entry.is.L3V6.is.entry0.sip_addr127_96;
			
			rule->srcIpV6AddrMask_.v6_addr32[3]	= entry.is.L3V6.is.entry0.sip_mask31_0;
			rule->srcIpV6AddrMask_.v6_addr32[2]	= entry.is.L3V6.is.entry0.sip_mask63_32;
			rule->srcIpV6AddrMask_.v6_addr32[1]	= entry.is.L3V6.is.entry0.sip_mask95_64;

			rule->srcIpV6AddrMask_.v6_addr32[0]	=  entry.is_d.ipv6.is.entry0.sip_mask119_96;
			rule->srcIpV6AddrMask_.v6_addr32[0]	|= entry.is_d.ipv6.is.entry0.sip_mask127_120 <<24;
			rule->ipv6FlowLabel_  =	 entry.is_d.ipv6.is.entry0.flowLabel;
			rule->ipv6FlowLabelM_ =	 entry.is_d.ipv6.is.entry0.flowLabelM3_0;
			rule->ipv6FlowLabelM_ |= entry.is_d.ipv6.is.entry0.flowLabelM19_4<<4;
		}
		else
		{
			rule->dstIpV6Addr_.v6_addr32[3]	= entry.is.L3V6.is.entry1.dip_addr31_0;
			rule->dstIpV6Addr_.v6_addr32[2]	= entry.is.L3V6.is.entry1.dip_addr63_32;
			rule->dstIpV6Addr_.v6_addr32[1]	= entry.is.L3V6.is.entry1.dip_addr95_64;
			rule->dstIpV6Addr_.v6_addr32[0]	= entry.is.L3V6.is.entry1.dip_addr127_96;
	  
			rule->dstIpV6AddrMask_.v6_addr32[3]	= entry.is.L3V6.is.entry1.dip_mask31_0;
			//entry.is.L3V6.is.entry1.dip_mask63_32	 = rule->dstIpV6AddrMask_.v6_addr32[2];
			//entry.is.L3V6.is.entry1.dip_mask95_64	 = rule->dstIpV6AddrMask_.v6_addr32[1];
			rule->dstIpV6AddrMask_.v6_addr32[2]	= entry.is.L3V6.is.entry1.dip_mask63_32;
			rule->dstIpV6AddrMask_.v6_addr32[1]	= entry.is.L3V6.is.entry1.dip_mask95_64;
			
			rule->dstIpV6AddrMask_.v6_addr32[0]	= entry.is_d.ipv6.is.entry1.dip_mask119_96;
			rule->dstIpV6AddrMask_.v6_addr32[0]	|= entry.is_d.ipv6.is.entry1.dip_mask127_120<<24;
			rule->ipv6TrafficClass_	 = entry.is_d.ipv6.is.entry1.trafficClass; 
			rule->ipv6TrafficClassM_ = entry.is_d.ipv6.is.entry1.trafficClassM;	
			rule->ipv6NextHeader_	 = entry.is_d.ipv6.is.entry1.nextHeader; 
			rule->ipv6NextHeaderM_	 = entry.is_d.ipv6.is.entry1.nextHeaderM; 
			rule->ipv6HttpFilter_	 = entry.is_d.ipv6.is.entry1.HTTPP;
			rule->ipv6HttpFilterM_	  =	entry.is_d.ipv6.is.entry1.HTTPM;
			rule->ipv6IdentSrcDstIp_ = entry.is_d.ipv6.is.entry1.identSDIPP;
			rule->ipv6IdentSrcDstIpM_ =	entry.is_d.ipv6.is.entry1.identSDIPM;
		}

		rule->ipv6Invert_	 = entry.inv;
		rule->ipv6EntryType_ = entry.ipv6ETY0;
		rule->ipv6Combine_	= entry.comb ;
		rule->ipv6IPtunnel_	= entry.ip_tunnel;
		break;
#elif defined(CONFIG_RTL_8198C)
	case RTL865X_ACL_IPV6: /* IP Rule Type:	0x0010 */
	case RTL865X_ACL_IPV6_RANGE:

		if(entry.ipv6ETY0)//first entry
		{
			rule->srcIpV6Addr_.v6_addr32[3]	= entry.is.L3V6.is.entry0.sip_addr31_0;
			rule->srcIpV6Addr_.v6_addr32[2]	= entry.is.L3V6.is.entry0.sip_addr63_32;
			rule->srcIpV6Addr_.v6_addr32[1]	= entry.is.L3V6.is.entry0.sip_addr95_64;
			rule->srcIpV6Addr_.v6_addr32[0]	= entry.is.L3V6.is.entry0.sip_addr127_96;
			
			rule->srcIpV6AddrMask_.v6_addr32[3]	= entry.is.L3V6.is.entry0.sip_mask31_0;
			rule->srcIpV6AddrMask_.v6_addr32[2]	= entry.is.L3V6.is.entry0.sip_mask63_32;
			rule->srcIpV6AddrMask_.v6_addr32[1]	= entry.is.L3V6.is.entry0.sip_mask95_64;

			rule->srcIpV6AddrMask_.v6_addr32[0]	=  entry.ipv6.is.entry0.sip_mask119_96;
			rule->srcIpV6AddrMask_.v6_addr32[0]	|= entry.ipv6.is.entry0.sip_mask127_120	<<24;
			rule->ipv6FlowLabel_  =	 entry.ipv6.is.entry0.flowLabel;
			rule->ipv6FlowLabelM_ =	 entry.ipv6.is.entry0.flowLabelM3_0;
			rule->ipv6FlowLabelM_ |= entry.ipv6.is.entry0.flowLabelM19_4<<4;
		}
		else
		{
			rule->dstIpV6Addr_.v6_addr32[3]	= entry.is.L3V6.is.entry1.dip_addr31_0;
			rule->dstIpV6Addr_.v6_addr32[2]	= entry.is.L3V6.is.entry1.dip_addr63_32;
			rule->dstIpV6Addr_.v6_addr32[1]	= entry.is.L3V6.is.entry1.dip_addr95_64;
			rule->dstIpV6Addr_.v6_addr32[0]	= entry.is.L3V6.is.entry1.dip_addr127_96;
	  
			rule->dstIpV6AddrMask_.v6_addr32[3]	= entry.is.L3V6.is.entry1.dip_mask31_0;
			//entry.is.L3V6.is.entry1.dip_mask63_32	 = rule->dstIpV6AddrMask_.v6_addr32[2];
			//entry.is.L3V6.is.entry1.dip_mask95_64	 = rule->dstIpV6AddrMask_.v6_addr32[1];
			rule->dstIpV6AddrMask_.v6_addr32[2]	= entry.is.L3V6.is.entry1.dip_mask63_32;
			rule->dstIpV6AddrMask_.v6_addr32[1]	= entry.is.L3V6.is.entry1.dip_mask95_64;
			
			rule->dstIpV6AddrMask_.v6_addr32[0]	= entry.ipv6.is.entry1.dip_mask119_96;
			rule->dstIpV6AddrMask_.v6_addr32[0]	|= entry.ipv6.is.entry1.dip_mask127_120<<24;
			rule->ipv6TrafficClass_	 = entry.ipv6.is.entry1.trafficClass; 
			rule->ipv6TrafficClassM_ = entry.ipv6.is.entry1.trafficClassM; 
			rule->ipv6NextHeader_	 = entry.ipv6.is.entry1.nextHeader;	
			rule->ipv6NextHeaderM_	 = entry.ipv6.is.entry1.nextHeaderM; 
			rule->ipv6HttpFilter_	 = entry.ipv6.is.entry1.HTTPP;
			rule->ipv6HttpFilterM_	  =	entry.ipv6.is.entry1.HTTPM;
			rule->ipv6IdentSrcDstIp_ = entry.ipv6.is.entry1.identSDIPP;
			rule->ipv6IdentSrcDstIpM_ =	entry.ipv6.is.entry1.identSDIPM;
		}

		rule->ipv6Invert_	 = entry.inv;
		rule->ipv6EntryType_ = entry.ipv6ETY0;
		rule->ipv6Combine_	= entry.comb ;
		rule->ipv6IPtunnel_	= entry.ip_tunnel;
		break;
#endif
		
#if	defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_PM: /*	acl	pattern	match */
		if(entry.ipv6ETY0)//first entry
		{
			rule->pmSPUB_ =	entry.is.PM_U.is.entry0.spub;
			rule->pmSPLB_ =	entry.is.PM_U.is.entry0.splb;
			rule->pmDPUB_ =	entry.is.PM_U.is.entry0.dpub;
			rule->pmDPLB_ =	entry.is.PM_U.is.entry0.dplb;
			rule->pmProtocol_ =	entry.is.PM_U.is.entry0.protocol;			 

			rule->pmOffset0_  =	entry.is.PM_U.is.entry0.offset0;
			rule->pmPattern0_ =	entry.is.PM_U.is.entry0.pattern0;
			rule->pmPatternMask0_ =	((entry.is.PM_U.is.entry0.pm0_15_1<<1 )
										| entry.is.PM_U.is.entry0.pm0_0);
			rule->pmOR0_ =entry.is.PM_U.is.entry0.or0 ;		  

			rule->pmOffset1_	  =	entry.is.PM_U.is.entry0.offset1;
			rule->pmPattern1_ =	((entry.is.PM_U.is.entry0.pattern1_8_0)
								|(entry.is.PM_U.is.entry0.pattern1_15_9<<9));
			rule->pmPatternMask1_ =	entry.is.PM_U.is.entry0.pm1;
			rule->pmOR1_		  =	entry.is.PM_U.is.entry0.or1;

			rule->pmOffset2_	  =	entry.is.PM_U.is.entry0.offset2;
			rule->pmPattern2_ =	(entry.is.PM_U.is.entry0.pattern2_0
							  |( entry.is.PM_U.is.entry0.pattern2_15_1<<1));
			rule->pmPatternMask2_ =	entry.is.PM_U.is.entry0.pm2;
			rule->pmOR2_		  =	entry.is.PM_U.is.entry0.or2;		

			rule->pmOffset3_  =	entry.is_d.PM_D.is.entry0.offset3;
			rule->pmPattern3_ =	entry.is_d.PM_D.is.entry0.pattern3;
			rule->pmPatternMask3_ =	(entry.is_d.PM_D.is.entry0.pm3_8_0
								   |(entry.is_d.PM_D.is.entry0.pm3_15_9<<9));
			rule->pmOR3_ = entry.is_d.PM_D.is.entry0.or3;
		}
		else
		{
			rule->pmOffset4_	  =	entry.is.PM_U.is.entry1.offset4;
			rule->pmPattern4_	  =	entry.is.PM_U.is.entry1.pattern4;
			rule->pmPatternMask4_ =	(entry.is.PM_U.is.entry1.pm4_8_0
								  |	(entry.is.PM_U.is.entry1.pm4_15_9<<9));

			rule->pmOR4_		  =	entry.is.PM_U.is.entry1.or4;
			
			rule->pmOffset5_ = entry.is.PM_U.is.entry1.offset5;
			rule->pmPattern5_ =	entry.is.PM_U.is.entry1.pattern5;
			rule->pmPatternMask5_ =	(entry.is.PM_U.is.entry1.pm5_0
									|(entry.is.PM_U.is.entry1.pm5_15_1<<1));
			rule->pmOR5_ = entry.is.PM_U.is.entry1.or5;

			rule->pmOffset6_ = entry.is.PM_U.is.entry1.offset6;
			rule->pmPattern6_ =	(entry.is.PM_U.is.entry1.pattern6_8_0
							   |(entry.is.PM_U.is.entry1.pattern6_15_9<<9));
			rule->pmPatternMask6_ =	entry.is.PM_U.is.entry1.pm6;
			rule->pmOR6_ = entry.is.PM_U.is.entry1.or6;

			rule->pmOffset7_  =	entry.is.PM_U.is.entry1.offset7;
			rule->pmPattern7_ =	(entry.is.PM_U.is.entry1.pattern7_0	
							   |(entry.is.PM_U.is.entry1.pattern7_15_1<<1));
			rule->pmPatternMask7_ =	entry.is.PM_U.is.entry1.pm7;
			rule->pmOR7_ = entry.is.PM_U.is.entry1.or7;

			rule->pmSpecialop_ = entry.is.PM_U.is.entry1.specialop;
			rule->pmSPA_ = entry.is.PM_U.is.entry1.spa;
			rule->pmSPAM_ =	entry.is.PM_U.is.entry1.spam;
			rule->pmPPPCTL_	= entry.is.PM_U.is.entry1.pppctl;
			rule->pmPPPCTLM_ = entry.is.PM_U.is.entry1.pppctlm;
			rule->pmPPPCTLOR_ =	entry.is.PM_U.is.entry1.pppctl_or;
		}
		rule->ipv6IPtunnel_	= entry.ip_tunnel;
		rule->ipfrag_apply_	= entry.is_d.PM_D.is.entry1.ipfrag_apply;
		rule->ipv6EntryType_ = entry.ipv6ETY0;
		break;
#endif
	default: return	FAILED;	/* Unknown rule	type */

	}

	#if	defined(CONFIG_RTL_8197F) || defined(CONFIG_RTL_8198C)
	rule->ipv6Combine_	= entry.comb;
	rule->ipv6Invert_	 = entry.inv;
	rule->ipv6IPtunnel_	= entry.ip_tunnel;		
	#endif

	#if	defined(CONFIG_RTL_8197F)
	rule->ipfrag_apply_ = entry.is_d.PM_D.is.entry1.ipfrag_apply;
	#endif
	
	rule->aclIdx = index;

	switch(entry.actionType) {

	case RTL865X_ACL_PERMIT:
	case RTL865X_ACL_REDIRECT_ETHER:
	case RTL865X_ACL_DROP:
	case RTL865X_ACL_TOCPU:
	case RTL865X_ACL_LEGACY_DROP:
	case RTL865X_ACL_DROPCPU_LOG:
	case RTL865X_ACL_MIRROR:
	case RTL865X_ACL_REDIRECT_PPPOE:
	case RTL865X_ACL_MIRROR_KEEP_MATCH:
		rule->L2Idx_ = entry.nextHop ;
		rule->netifIdx_	=  entry.vid;
		rule->pppoeIdx_	= entry.PPPoEIndex;
		 break;

	case RTL865X_ACL_DEFAULT_REDIRECT:
		rule->nexthopIdx_ =	entry.nextHop;
		break;

	case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
		rule->ratelimtIdx_ = entry.nextHop;
		break;
	case RTL865X_ACL_PRIORITY:
		rule->priority_	= entry.nextHop;
		break;

#if	defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_VID:
		rule->aclvid_ =	(entry.nextHop | (entry.vid	<<10)) & 0xfff;
		rule->aclfid_ =	((entry.PPPoEIndex << 1)| (entry.vid >>2)) & 0x3;
		break;

#elif defined(CONFIG_RTL_8198C)
	case RTL865X_ACL_VID:
		rule->aclvid_ =	(entry.nextHop | (entry.vid	<<10)) & 0xfff;
		break;
#endif

	}

	rule->actionType_ =	entry.actionType;
	rule->pktOpApp_	= entry.pktOpApp;

	return SUCCESS;

}

static int32 _rtl865x_setAclToAsic(int32 startIdx, rtl865x_AclRule_t *rule)
{
	rtl865xc_tblAsic_aclTable_t entry;

	if(rule->aclIdx >= RTL865X_ACL_MAX_NUMBER + RTL865X_ACL_RESERVED_NUMBER || rule == NULL)
		return FAILED;

	memset(&entry, 0, sizeof(entry));
	switch(rule->ruleType_)
	{

	case RTL865X_ACL_MAC:			/* Etnernet Rule Type : 0x0000 */
		 entry.is.ETHERNET.dMacP47_32 = rule->dstMac_.octet[0]<<8 | rule->dstMac_.octet[1];
		 entry.is.ETHERNET.dMacP31_16 = rule->dstMac_.octet[2]<<8 | rule->dstMac_.octet[3];
		 entry.is.ETHERNET.dMacP15_0 = rule->dstMac_.octet[4]<<8 | rule->dstMac_.octet[5];
	 	 entry.is.ETHERNET.dMacM47_32 = rule->dstMacMask_.octet[0]<<8 | rule->dstMacMask_.octet[1];
		 entry.is.ETHERNET.dMacM31_16 = rule->dstMacMask_.octet[2]<<8 | rule->dstMacMask_.octet[3];
		 entry.is.ETHERNET.dMacM15_0 = rule->dstMacMask_.octet[4]<<8 | rule->dstMacMask_.octet[5];
		 entry.is.ETHERNET.sMacP47_32 = rule->srcMac_.octet[0]<<8 | rule->srcMac_.octet[1];
		 entry.is.ETHERNET.sMacP31_16 = rule->srcMac_.octet[2]<<8 | rule->srcMac_.octet[3];
		 entry.is.ETHERNET.sMacP15_0 = rule->srcMac_.octet[4]<<8 | rule->srcMac_.octet[5];
		 entry.is.ETHERNET.sMacM47_32 = rule->srcMacMask_.octet[0]<<8 | rule->srcMacMask_.octet[1];
		 entry.is.ETHERNET.sMacM31_16 = rule->srcMacMask_.octet[2]<<8 | rule->srcMacMask_.octet[3];
		 entry.is.ETHERNET.sMacM15_0 = rule->srcMacMask_.octet[4]<<8 | rule->srcMacMask_.octet[5];
		 entry.is.ETHERNET.ethTypeP = rule->typeLen_;
		 entry.is.ETHERNET.ethTypeM = rule->typeLenMask_;

		 entry.ruleType = rule->ruleType_;
		 break;

	case RTL865X_ACL_IP:			/* IP Rule Type: 0x0002 */
	case RTL865X_ACL_IP_RANGE:						/* 0x0010 */
		 entry.is.L3L4.is.IP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IP.IPProtoP = rule->ipProto_;
		 entry.is.L3L4.is.IP.IPProtoM = rule->ipProtoMask_;
		 entry.is.L3L4.is.IP.IPFlagP = rule->ipFlag_;
		 entry.is.L3L4.is.IP.IPFlagM = rule->ipFlagMask_;
 		 entry.is.L3L4.is.IP.FOP = rule->ipFOP_;
		 entry.is.L3L4.is.IP.FOM = rule->ipFOM_;
		 entry.is.L3L4.is.IP.HTTPP = entry.is.L3L4.is.IP.HTTPM = rule->ipHttpFilter_;
		 entry.is.L3L4.is.IP.identSDIPP = entry.is.L3L4.is.IP.identSDIPM = rule->ipIdentSrcDstIp_;

		 goto l3l4_shared;

	case RTL865X_ACL_ICMP:			/* ICMP Rule Type: 0x0004 */
	case RTL865X_ACL_ICMP_IPRANGE:					/* 0x000C */
		 entry.is.L3L4.is.ICMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.ICMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.ICMP.ICMPTypeP = rule->icmpType_;
		 entry.is.L3L4.is.ICMP.ICMPTypeM = rule->icmpTypeMask_;
		 entry.is.L3L4.is.ICMP.ICMPCodeP = rule->icmpCode_;
		 entry.is.L3L4.is.ICMP.ICMPCodeM = rule->icmpCodeMask_;
 		 goto l3l4_shared;

	case RTL865X_ACL_IGMP:			/* IGMP Rule Type: 0x0005 */
	case RTL865X_ACL_IGMP_IPRANGE:					/* 0x000D */
		 entry.is.L3L4.is.IGMP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.IGMP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.IGMP.IGMPTypeP = rule->igmpType_;
		 entry.is.L3L4.is.IGMP.IGMPTypeM = rule->igmpTypeMask_;

 		 goto l3l4_shared;

	case RTL865X_ACL_TCP:			/* TCP Rule Type: 0x0006 */
	case RTL865X_ACL_TCP_IPRANGE:					/* 0x000E */
		 entry.is.L3L4.is.TCP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.TCP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.TCP.TCPFlagP = rule->tcpFlag_;
		 entry.is.L3L4.is.TCP.TCPFlagM = rule->tcpFlagMask_;
		 entry.is.L3L4.is.TCP.TCPSPUB = rule->tcpSrcPortUB_;
		 entry.is.L3L4.is.TCP.TCPSPLB = rule->tcpSrcPortLB_;
		 entry.is.L3L4.is.TCP.TCPDPUB = rule->tcpDstPortUB_;
		 entry.is.L3L4.is.TCP.TCPDPLB = rule->tcpDstPortLB_;

         goto l3l4_shared;

	case RTL865X_ACL_UDP:			/* TCP Rule Type: 0x0007 */
	case RTL865X_ACL_UDP_IPRANGE:					/* 0x000F */
		 entry.is.L3L4.is.UDP.IPTOSP = rule->tos_;
		 entry.is.L3L4.is.UDP.IPTOSM = rule->tosMask_;
		 entry.is.L3L4.is.UDP.UDPSPUB = rule->udpSrcPortUB_;
		 entry.is.L3L4.is.UDP.UDPSPLB = rule->udpSrcPortLB_;
		 entry.is.L3L4.is.UDP.UDPDPUB = rule->udpDstPortUB_;
		 entry.is.L3L4.is.UDP.UDPDPLB = rule->udpDstPortLB_;

l3l4_shared:
		 entry.ruleType = rule->ruleType_;
		 entry.is.L3L4.sIPP = rule->srcIpAddr_;
		 entry.is.L3L4.sIPM = rule->srcIpAddrMask_;
		 entry.is.L3L4.dIPP = rule->dstIpAddr_;
		 entry.is.L3L4.dIPM = rule->dstIpAddrMask_;
		 break;

 	case RTL865X_ACL_SRCFILTER:		/* TCP Rule Type: 0x0008 */
 	case RTL865X_ACL_SRCFILTER_IPRANGE:				/* 0x000B */
 		 rule->srcFilterMac_.octet[0] = rule->srcFilterMac_.octet[0] & rule->srcFilterMacMask_.octet[0];
 		 rule->srcFilterMac_.octet[1] = rule->srcFilterMac_.octet[1] & rule->srcFilterMacMask_.octet[1];
 		 rule->srcFilterMac_.octet[2] = rule->srcFilterMac_.octet[2] & rule->srcFilterMacMask_.octet[2];
 		 rule->srcFilterMac_.octet[3] = rule->srcFilterMac_.octet[3] & rule->srcFilterMacMask_.octet[3];
 		 rule->srcFilterMac_.octet[4] = rule->srcFilterMac_.octet[4] & rule->srcFilterMacMask_.octet[4];
 		 rule->srcFilterMac_.octet[5] = rule->srcFilterMac_.octet[5] & rule->srcFilterMacMask_.octet[5];

		 entry.is.SRC_FILTER.sMacP47_32 = rule->srcFilterMac_.octet[0]<<8 | rule->srcFilterMac_.octet[1];
		 entry.is.SRC_FILTER.sMacP31_16 = rule->srcFilterMac_.octet[2]<<8 | rule->srcFilterMac_.octet[3];
		 entry.is.SRC_FILTER.sMacP15_0 = rule->srcFilterMac_.octet[4]<<8 | rule->srcFilterMac_.octet[5];
		 entry.is.SRC_FILTER.sMacM3_0 =rule->srcFilterMacMask_.octet[5] &0xf;

		 rule->srcFilterVlanId_ = rule->srcFilterVlanId_ & rule->srcFilterVlanIdMask_;
		 entry.is.SRC_FILTER.spaP = rule->srcFilterPort_;
		 entry.is.SRC_FILTER.sVidP = rule->srcFilterVlanId_;
		 entry.is.SRC_FILTER.sVidM = rule->srcFilterVlanIdMask_;
		 if(rule->srcFilterIgnoreL3L4_)
		 	entry.is.SRC_FILTER.protoType = 1;
		 else if(rule->srcFilterIgnoreL4_)
		 	entry.is.SRC_FILTER.protoType = 2;
		 else
		 	entry.is.SRC_FILTER.protoType = 0;

		 entry.is.SRC_FILTER.sIPP = rule->srcFilterIpAddr_;
		 entry.is.SRC_FILTER.sIPM = rule->srcFilterIpAddrMask_;
		 entry.is.SRC_FILTER.SPORTUB = rule->srcFilterPortUpperBound_;
		 entry.is.SRC_FILTER.SPORTLB = rule->srcFilterPortLowerBound_;

		 entry.ruleType = rule->ruleType_;
		 break;

	case RTL865X_ACL_DSTFILTER:		/* TCP Rule Type: 0x0009 */
 	case RTL865X_ACL_DSTFILTER_IPRANGE:				/* 0x0001 */
		 entry.is.DST_FILTER.dMacP47_32 = rule->dstFilterMac_.octet[0]<<8 | rule->dstFilterMac_.octet[1];
		 entry.is.DST_FILTER.dMacP31_16 = rule->dstFilterMac_.octet[2]<<8 | rule->dstFilterMac_.octet[3];
		 entry.is.DST_FILTER.dMacP15_0 = rule->dstFilterMac_.octet[4]<<8 | rule->dstFilterMac_.octet[5];
	 	 entry.is.DST_FILTER.dMacM3_0 =  rule->dstFilterMacMask_.octet[5]&0xf;
		 entry.is.DST_FILTER.vidP = rule->dstFilterVlanIdx_;
		 entry.is.DST_FILTER.vidM = rule->dstFilterVlanIdxMask_;
		 if(rule->dstFilterIgnoreL3L4_)
		 	entry.is.DST_FILTER.protoType = 1;
		 else if(rule->dstFilterIgnoreL4_)
		 	entry.is.DST_FILTER.protoType = 2;
		 else
		 	entry.is.DST_FILTER.protoType = 0;
		 entry.is.DST_FILTER.dIPP = rule->dstFilterIpAddr_;
		 entry.is.DST_FILTER.dIPM = rule->dstFilterIpAddrMask_;
		 entry.is.DST_FILTER.DPORTUB = rule->dstFilterPortUpperBound_;
		 entry.is.DST_FILTER.DPORTLB = rule->dstFilterPortLowerBound_;

    		 entry.ruleType = rule->ruleType_;
		 break;


#if defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_IPV6:			/* IP Rule Type: 0x0012	*/
	case RTL865X_ACL_IPV6_RANGE:					/* 0x001A */
		entry.ruleType = rule->ruleType_;
		entry.ruleType1	= rule->ruleType_>>4;
		if(rule->ipv6EntryType_==1)	//first	entry
		{
			entry.is.L3V6.is.entry0.sip_addr31_0   = rule->srcIpV6Addr_.v6_addr32[3];
			entry.is.L3V6.is.entry0.sip_addr63_32  = rule->srcIpV6Addr_.v6_addr32[2];
			entry.is.L3V6.is.entry0.sip_addr95_64  = rule->srcIpV6Addr_.v6_addr32[1];
			entry.is.L3V6.is.entry0.sip_addr127_96 = rule->srcIpV6Addr_.v6_addr32[0];
			
			entry.is.L3V6.is.entry0.sip_mask31_0   = rule->srcIpV6AddrMask_.v6_addr32[3];
			entry.is.L3V6.is.entry0.sip_mask63_32  = rule->srcIpV6AddrMask_.v6_addr32[2];
			entry.is.L3V6.is.entry0.sip_mask95_64  = rule->srcIpV6AddrMask_.v6_addr32[1];

			entry.is_d.ipv6.is.entry0.sip_mask119_96	= rule->srcIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
			entry.is_d.ipv6.is.entry0.sip_mask127_120	= rule->srcIpV6AddrMask_.v6_addr32[0]>>24;
			entry.is_d.ipv6.is.entry0.flowLabel			= rule->ipv6FlowLabel_;
			entry.is_d.ipv6.is.entry0.flowLabelM3_0		= rule->ipv6FlowLabelM_&0xF;
			entry.is_d.ipv6.is.entry0.flowLabelM19_4	= rule->ipv6FlowLabelM_>>4;		 
		}
		else
		{
			entry.is.L3V6.is.entry1.dip_addr31_0   = rule->dstIpV6Addr_.v6_addr32[3];
			entry.is.L3V6.is.entry1.dip_addr63_32  = rule->dstIpV6Addr_.v6_addr32[2];
			entry.is.L3V6.is.entry1.dip_addr95_64  = rule->dstIpV6Addr_.v6_addr32[1];
			entry.is.L3V6.is.entry1.dip_addr127_96 = rule->dstIpV6Addr_.v6_addr32[0];
			
			entry.is.L3V6.is.entry1.dip_mask31_0   = rule->dstIpV6AddrMask_.v6_addr32[3];
			entry.is.L3V6.is.entry1.dip_mask63_32  = rule->dstIpV6AddrMask_.v6_addr32[2];
			entry.is.L3V6.is.entry1.dip_mask95_64  = rule->dstIpV6AddrMask_.v6_addr32[1];
			
			entry.is_d.ipv6.is.entry1.dip_mask119_96	= rule->dstIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
			entry.is_d.ipv6.is.entry1.dip_mask127_120	= rule->dstIpV6AddrMask_.v6_addr32[0]>>24;
			entry.is_d.ipv6.is.entry1.trafficClass		= rule->ipv6TrafficClass_; 
			entry.is_d.ipv6.is.entry1.trafficClassM		= rule->ipv6TrafficClassM_;	
			entry.is_d.ipv6.is.entry1.nextHeader		= rule->ipv6NextHeader_; 

			entry.is_d.ipv6.is.entry1.nextHeaderM		= rule->ipv6NextHeaderM_; 
			entry.is_d.ipv6.is.entry1.HTTPP		  =	entry.is_d.ipv6.is.entry1.HTTPM	= rule->ipv6HttpFilter_;
			entry.is_d.ipv6.is.entry1.identSDIPP  =	entry.is_d.ipv6.is.entry1.identSDIPM  =	rule->ipv6IdentSrcDstIp_; 
		}

		entry.inv		= rule->ipv6Invert_;
		entry.ipv6ETY0	= rule->ipv6EntryType_;
		entry.comb		= rule->ipv6Combine_;
		entry.ip_tunnel	= rule->ipv6IPtunnel_;
		break;
#elif defined(CONFIG_RTL_8198C)
	case RTL865X_ACL_IPV6:			/* IP Rule Type: 0x0012 */
	case RTL865X_ACL_IPV6_RANGE:					/* 0x001A */
        entry.ruleType = rule->ruleType_;
        entry.ruleType1 = rule->ruleType_>>4;
        if(rule->ipv6EntryType_==1)//first entry
        {
            entry.is.L3V6.is.entry0.sip_addr31_0   = rule->srcIpV6Addr_.v6_addr32[3];
            entry.is.L3V6.is.entry0.sip_addr63_32  = rule->srcIpV6Addr_.v6_addr32[2];
            entry.is.L3V6.is.entry0.sip_addr95_64  = rule->srcIpV6Addr_.v6_addr32[1];
            entry.is.L3V6.is.entry0.sip_addr127_96 = rule->srcIpV6Addr_.v6_addr32[0];
            
            entry.is.L3V6.is.entry0.sip_mask31_0   = rule->srcIpV6AddrMask_.v6_addr32[3];
            entry.is.L3V6.is.entry0.sip_mask63_32  = rule->srcIpV6AddrMask_.v6_addr32[2];
            entry.is.L3V6.is.entry0.sip_mask95_64  = rule->srcIpV6AddrMask_.v6_addr32[1];

            entry.ipv6.is.entry0.sip_mask119_96    = rule->srcIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
            entry.ipv6.is.entry0.sip_mask127_120   = rule->srcIpV6AddrMask_.v6_addr32[0]>>24;
            entry.ipv6.is.entry0.flowLabel         = rule->ipv6FlowLabel_;
            entry.ipv6.is.entry0.flowLabelM3_0     = rule->ipv6FlowLabelM_&0xF;
            entry.ipv6.is.entry0.flowLabelM19_4    = rule->ipv6FlowLabelM_>>4;      
        }
        else
        {
            entry.is.L3V6.is.entry1.dip_addr31_0   = rule->dstIpV6Addr_.v6_addr32[3];
            entry.is.L3V6.is.entry1.dip_addr63_32  = rule->dstIpV6Addr_.v6_addr32[2];
            entry.is.L3V6.is.entry1.dip_addr95_64  = rule->dstIpV6Addr_.v6_addr32[1];
            entry.is.L3V6.is.entry1.dip_addr127_96 = rule->dstIpV6Addr_.v6_addr32[0];
            
            entry.is.L3V6.is.entry1.dip_mask31_0   = rule->dstIpV6AddrMask_.v6_addr32[3];
            entry.is.L3V6.is.entry1.dip_mask63_32  = rule->dstIpV6AddrMask_.v6_addr32[2];
            entry.is.L3V6.is.entry1.dip_mask95_64  = rule->dstIpV6AddrMask_.v6_addr32[1];
            
            entry.ipv6.is.entry1.dip_mask119_96    = rule->dstIpV6AddrMask_.v6_addr32[0]&0xFFFFFF;
            entry.ipv6.is.entry1.dip_mask127_120   = rule->dstIpV6AddrMask_.v6_addr32[0]>>24;
            entry.ipv6.is.entry1.trafficClass      = rule->ipv6TrafficClass_; 
            entry.ipv6.is.entry1.trafficClassM     = rule->ipv6TrafficClassM_; 
            entry.ipv6.is.entry1.nextHeader        = rule->ipv6NextHeader_; 

            entry.ipv6.is.entry1.nextHeaderM       = rule->ipv6NextHeaderM_; 
            entry.ipv6.is.entry1.HTTPP       = entry.ipv6.is.entry1.HTTPM = rule->ipv6HttpFilter_;
            entry.ipv6.is.entry1.identSDIPP  = entry.ipv6.is.entry1.identSDIPM  = rule->ipv6IdentSrcDstIp_; 
        }

        entry.inv       = rule->ipv6Invert_;
        entry.ipv6ETY0  = rule->ipv6EntryType_;
        entry.comb      = rule->ipv6Combine_;
        entry.ip_tunnel = rule->ipv6IPtunnel_;
		break;
#endif
#if defined(CONFIG_RTL_8197F)
		case RTL865X_ACL_PM:			/* IP Rule Type: 0x0012 */
			entry.ruleType = rule->ruleType_;
			entry.ruleType1 = rule->ruleType_>>4;
			if(rule->ipv6EntryType_==1) //first entry
			{
				entry.is.PM_U.is.entry0.spub = rule->un_ty.PM._spub;
				entry.is.PM_U.is.entry0.splb = rule->un_ty.PM._splb;
				entry.is.PM_U.is.entry0.dpub = rule->un_ty.PM._dpub;
				entry.is.PM_U.is.entry0.dplb = rule->un_ty.PM._dplb;
				entry.is.PM_U.is.entry0.protocol = rule->un_ty.PM._protocol;
				
				entry.is.PM_U.is.entry0.offset0 = rule->un_ty.PM._offset0;
				entry.is.PM_U.is.entry0.pattern0 = rule->un_ty.PM._pattern0;
				entry.is.PM_U.is.entry0.pm0_15_1 = rule->un_ty.PM._pm0>>1;
				entry.is.PM_U.is.entry0.pm0_0 = rule->un_ty.PM._pm0 & 0x1;
				entry.is.PM_U.is.entry0.or0 = rule->un_ty.PM._or0;
					
				entry.is.PM_U.is.entry0.offset1 = rule->un_ty.PM._offset1;
				entry.is.PM_U.is.entry0.pattern1_8_0 = rule->un_ty.PM._pattern1&0x1ff;
				entry.is.PM_U.is.entry0.pattern1_15_9 = rule->un_ty.PM._pattern1>>9; 
				entry.is.PM_U.is.entry0.pm1 = rule->un_ty.PM._pm1;
				entry.is.PM_U.is.entry0.or1 = rule->un_ty.PM._or1;
				
				entry.is.PM_U.is.entry0.offset2 = rule->un_ty.PM._offset2;
				entry.is.PM_U.is.entry0.pattern2_0 = rule->un_ty.PM._pattern2&0x1;
				entry.is.PM_U.is.entry0.pattern2_15_1 = rule->un_ty.PM._pattern2>>1;
				entry.is.PM_U.is.entry0.pm2 = rule->un_ty.PM._pm2;
				entry.is.PM_U.is.entry0.or2 = rule->un_ty.PM._or2;
				
				entry.is_d.PM_D.is.entry0.offset3 = rule->un_ty.PM._offset3;
				entry.is_d.PM_D.is.entry0.pattern3 = rule->un_ty.PM._pattern3;
				entry.is_d.PM_D.is.entry0.pm3_8_0 = rule->un_ty.PM._pm3&0x1ff;
				entry.is_d.PM_D.is.entry0.pm3_15_9 = rule->un_ty.PM._pm3>>9;
				entry.is_d.PM_D.is.entry0.or3 = rule->un_ty.PM._or3;
			}
			else
			{
				entry.is.PM_U.is.entry1.offset4 = rule->un_ty.PM._offset4;
				entry.is.PM_U.is.entry1.pattern4 = rule->un_ty.PM._pattern4;
				entry.is.PM_U.is.entry1.pm4_8_0 = rule->un_ty.PM._pm4&0x1ff;
				entry.is.PM_U.is.entry1.pm4_15_9 = rule->un_ty.PM._pm4>>9;			
				entry.is.PM_U.is.entry1.or4 = rule->un_ty.PM._or4;
				
				entry.is.PM_U.is.entry1.offset5 = rule->un_ty.PM._offset5;
				entry.is.PM_U.is.entry1.pattern5 = rule->un_ty.PM._pattern5;
				entry.is.PM_U.is.entry1.pm5_0 = rule->un_ty.PM._pm5&0x1;
				entry.is.PM_U.is.entry1.pm5_15_1 = rule->un_ty.PM._pm5>>1;
				entry.is.PM_U.is.entry1.or5	= rule->un_ty.PM._or5; 
				
				entry.is.PM_U.is.entry1.offset6 = rule->un_ty.PM._offset6; 
				entry.is.PM_U.is.entry1.pattern6_8_0 = rule->un_ty.PM._pattern6&0x1ff; 
				entry.is.PM_U.is.entry1.pattern6_15_9 = rule->un_ty.PM._pattern6>>9;
				entry.is.PM_U.is.entry1.pm6 = rule->un_ty.PM._pm6; 
				entry.is.PM_U.is.entry1.or6 = rule->un_ty.PM._or6;
				
				entry.is.PM_U.is.entry1.offset7 = rule->un_ty.PM._offset7;
				entry.is.PM_U.is.entry1.pattern7_0 = rule->un_ty.PM._pattern7&0x1;
				entry.is.PM_U.is.entry1.pattern7_15_1 = rule->un_ty.PM._pattern7>>1;
				entry.is.PM_U.is.entry1.pm7 = rule->un_ty.PM._pm7;
				entry.is.PM_U.is.entry1.or7 = rule->un_ty.PM._or7;
				
				entry.is.PM_U.is.entry1.specialop = rule->un_ty.PM._specialop;
				entry.is.PM_U.is.entry1.spa = rule->un_ty.PM._spa;
				entry.is.PM_U.is.entry1.spam = rule->un_ty.PM._spam;
				entry.is.PM_U.is.entry1.pppctl = rule->un_ty.PM._pppctl;
				entry.is.PM_U.is.entry1.pppctlm = rule->un_ty.PM._pppctlm;
				entry.is.PM_U.is.entry1.pppctl_or = rule->un_ty.PM._pppctlor;
			}
	
			entry.inv = rule->ipv6Invert_;
			entry.ipv6ETY0 = rule->ipv6EntryType_;
			entry.comb = rule->ipv6Combine_;
			entry.ip_tunnel = rule->ipv6IPtunnel_;
			entry.is_d.PM_D.is.entry1.ipfrag_apply = rule->ipfrag_apply_;
			break;
#endif
	default: return FAILED; /* Unknown rule type: -1 */

	}

	#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	/*   when comb = 1, will continue to check down next ACL rule, 
		the last one of the rule set must set comb = 0, 
		all ACL rule match will do the last one's action 
	*/
	entry.comb		= rule->ipv6Combine_;
	entry.inv		= rule->ipv6Invert_;
	entry.ip_tunnel = rule->ipv6IPtunnel_;
	#endif
	#if defined(CONFIG_RTL_8197F)
	entry.is_d.PM_D.is.entry1.ipfrag_apply = rule->ipfrag_apply_;
	#endif
	
	switch(rule->actionType_)
	{
	case RTL865X_ACL_PERMIT:
	case RTL865X_ACL_REDIRECT_ETHER:
	case RTL865X_ACL_DROP:
	case RTL865X_ACL_TOCPU:
	case RTL865X_ACL_LEGACY_DROP:
	case RTL865X_ACL_DROPCPU_LOG:
	case RTL865X_ACL_MIRROR:
	case RTL865X_ACL_REDIRECT_PPPOE:
	case RTL865X_ACL_MIRROR_KEEP_MATCH:
		 entry.nextHop = rule->L2Idx_;
		 entry.vid = rule->netifIdx_;
		 entry.PPPoEIndex = rule->pppoeIdx_;
		 break;

	case RTL865X_ACL_DEFAULT_REDIRECT:
		entry.nextHop = rule->nexthopIdx_;
		break;

	case RTL865X_ACL_DROP_RATE_EXCEED_PPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_PPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_DROP_RATE_EXCEED_BPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_LOG_RATE_EXCEED_BPS:
		entry.nextHop = rule->ratelimtIdx_;
		break;
	case RTL865X_ACL_PRIORITY:
		entry.nextHop = rule->priority_;
		break;
#if defined(CONFIG_RTL_8197F)
	case RTL865X_ACL_VID:        
		entry.nextHop = (rule->aclvid_ & 0x3ff);
		entry.vid = (rule->aclvid_ & 0xc00)>>10;
		entry.vid |= (rule->aclfid_&0x1)<<2;
		entry.PPPoEIndex = (rule->aclfid_&0x2)>>1;
		break;
#elif defined(CONFIG_RTL_8198C)
	case RTL865X_ACL_VID:
		entry.nextHop = (rule->aclvid_ & 0x3ff);
        entry.vid = (rule->aclvid_ & 0xc00)>>10;
		break;
#endif
	}

	entry.actionType = rule->actionType_;
	entry.pktOpApp = rule->pktOpApp_;


	return _rtl8651_forceAddAsicEntry(TYPE_ACL_RULE_TABLE, startIdx + rule->aclIdx -1, &entry);
}

/*config the reserved acl rules: default permit/drop/toCPU*/
static int32 _rtl865x_confReservedAcl(void)
{
	rtl865x_AclRule_t defAcl;

	/*ipv6 packet trap to cpu*/
	memset(&defAcl,0,sizeof(rtl865x_AclRule_t));
	defAcl.ruleType_ = RTL865X_ACL_MAC;
	defAcl.actionType_		= RTL865X_ACL_TOCPU;
	defAcl.aclIdx = 1;
	defAcl.pktOpApp_		= RTL865X_ACL_ALL_LAYER;
	defAcl.dstMac_.octet[0]=0x33;
	defAcl.dstMac_.octet[1]=0x33;
	defAcl.dstMac_.octet[2]=0x00;
	defAcl.dstMac_.octet[3]=0x00;
	defAcl.dstMac_.octet[4]=0x00;
	defAcl.dstMac_.octet[5]=0x00;
	defAcl.dstMacMask_.octet[0]=0xFF;
	defAcl.dstMacMask_.octet[1]=0xFF;
	_rtl865x_setAclToAsic(RTL865X_ACLTBL_IPV6_TO_CPU,&defAcl);
	
	/*default permit*/
	memset(&defAcl,0,sizeof(rtl865x_AclRule_t));
	defAcl.actionType_ = RTL865X_ACL_PERMIT;
	defAcl.aclIdx = 1;
	defAcl.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(RTL865X_ACLTBL_PERMIT_ALL,&defAcl);

	/*default drop*/
	defAcl.actionType_ = RTL865X_ACL_DROP;
	defAcl.aclIdx = 1;
	defAcl.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(RTL865X_ACLTBL_DROP_ALL, &defAcl);

	/*default to cpu*/
	defAcl.actionType_ = RTL865X_ACL_TOCPU;
	defAcl.aclIdx = 1;
	defAcl.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
	_rtl865x_setAclToAsic(RTL865X_ACLTBL_ALL_TO_CPU, &defAcl);

	/*hyking:set default permit when network interface decision miss match*/
	#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
	rtl865x_setDefACLForNetDecisionMiss(RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);
	#endif

	return SUCCESS;

}

/*
@func int32 | rtl865x_reinit_acl |memory reinit.
@rvalue SUCCESS | Success.
@comm
	this API must be called when system boot.
*/
int32 rtl865x_reinit_acl(void)
{
	_rtl865x_confReservedAcl();
	return SUCCESS;
}

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
/*=====================================
*acl releated function
*======================================*/
static int32 _rtl865x_setDefACLForAllNetif(uint8 start_ingressAclIdx, uint8 end_ingressAclIdx,uint8 start_egressAclIdx,uint8 end_egressAclIdx)
{
	rtl865x_netif_local_t *netif = NULL;
	int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		netif = &netifTbl[i];
		if(netif->valid == 0 || netif->is_slave == 1)
			continue;

		netif->inAclStart = start_ingressAclIdx;
		netif->inAclEnd = end_ingressAclIdx;
		netif->outAclStart = start_egressAclIdx;
		netif->outAclEnd = end_egressAclIdx;
		_rtl865x_setAsicNetif(netif);
	}
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	rtl865x_setDefACLForNetDecisionMiss(start_ingressAclIdx,end_ingressAclIdx,start_egressAclIdx,end_egressAclIdx);
#endif

	return SUCCESS;
}

int32 rtl865x_setDefACLForAllNetif(uint8 start_ingressAclIdx, uint8 end_ingressAclIdx,uint8 start_egressAclIdx,uint8 end_egressAclIdx)
{
	_rtl865x_setDefACLForAllNetif(start_ingressAclIdx, end_ingressAclIdx,start_egressAclIdx,end_egressAclIdx);
	return SUCCESS;
}

static int8 _rtl865x_sameAclRule(rtl865x_AclRule_t *rule1, rtl865x_AclRule_t *rule2)
{

	if (rule1->actionType_ != rule2->actionType_ || rule1->ruleType_ != rule2->ruleType_)
		return FALSE;

	switch(rule1->ruleType_) {
	case RTL865X_ACL_MAC:
		 if (rule1->typeLen_ != rule2->typeLen_ || rule1->typeLenMask_ != rule2->typeLenMask_)
		 	return FALSE;
		 if (memcmp(&rule1->dstMac_, &rule2->dstMac_, sizeof(ether_addr_t)) ||
		 	 memcmp(&rule1->dstMacMask_, &rule2->dstMacMask_, sizeof(ether_addr_t)) ||
			 memcmp(&rule1->srcMac_, &rule2->srcMac_, sizeof(ether_addr_t)) ||
			 memcmp(&rule1->srcMacMask_, &rule2->srcMacMask_, sizeof(ether_addr_t)) )
			 return FALSE;
		 return TRUE;
	case RTL865X_ACL_IP:
	case RTL865X_ACL_IP_RANGE:
		 if (rule1->ipProto_ != rule2->ipProto_ || rule1->ipProtoMask_ != rule2->ipProtoMask_ ||
			rule1->ipFlag_ != rule2->ipFlag_ || rule1->ipFlagMask_ != rule2->ipFlagMask_)
			return FALSE;
		break;

	case RTL865X_ACL_ICMP:
	case RTL865X_ACL_ICMP_IPRANGE:
		 if (rule1->icmpType_ != rule2->icmpType_ || rule1->icmpTypeMask_ != rule2->icmpTypeMask_ ||
			rule1->icmpCode_ != rule2->icmpCode_ || rule1->icmpCodeMask_ != rule2->icmpCodeMask_)
			return FALSE;
		 break;

	case RTL865X_ACL_IGMP:
	case RTL865X_ACL_IGMP_IPRANGE:
		 if(rule1->igmpType_ != rule2->igmpType_ || rule1->igmpTypeMask_ != rule2->igmpTypeMask_)
		 	return FALSE;
		 break;
	case RTL865X_ACL_TCP:
	case RTL865X_ACL_TCP_IPRANGE:
		 if(rule1->tcpFlag_ != rule2->tcpFlag_ || rule1->tcpFlagMask_ != rule2->tcpFlagMask_ ||
			rule1->tcpSrcPortUB_ != rule2->tcpSrcPortUB_ || rule1->tcpSrcPortLB_ != rule2->tcpSrcPortLB_ ||
			rule1->tcpDstPortUB_ != rule2->tcpDstPortUB_ || rule1->tcpDstPortLB_ != rule2->tcpDstPortLB_)
		 	return FALSE;
		 break;
	case RTL865X_ACL_UDP:
	case RTL865X_ACL_UDP_IPRANGE:
		 if(rule1->udpSrcPortUB_ != rule2->udpSrcPortUB_ || rule1->udpSrcPortLB_ != rule2->udpSrcPortLB_ ||
			rule1->udpDstPortUB_ != rule2->udpDstPortUB_ || rule1->udpDstPortLB_ != rule2->udpDstPortLB_)
			return FALSE;
		 break;

	case RTL865X_ACL_SRCFILTER:
	case RTL865X_ACL_SRCFILTER_IPRANGE:
		if((rule1->srcFilterPort_ != rule2->srcFilterPort_)||
			memcmp(&rule1->srcFilterMac_, &rule2->srcFilterMac_, sizeof(ether_addr_t)) != 0||
			memcmp(&rule1->srcFilterMacMask_, &rule2->srcFilterMacMask_,sizeof(ether_addr_t)) != 0||
			(rule1->srcFilterVlanIdx_ != rule2->srcFilterVlanIdx_)||
			(rule1->srcFilterVlanIdxMask_ != rule2->srcFilterVlanIdxMask_)||
			(rule1->srcFilterIgnoreL3L4_ != rule2->srcFilterIgnoreL3L4_)||
			(rule1->srcFilterIgnoreL4_ != rule2->srcFilterIgnoreL4_))
		{
			return FALSE;
		}

		if(rule1->srcFilterIgnoreL4_==0 && rule1->srcFilterIgnoreL3L4_==0)
		{
			if((rule1->srcFilterPortUpperBound_ != rule2->srcFilterPortUpperBound_)||
			   (rule1->srcFilterPortLowerBound_ != rule2->srcFilterPortLowerBound_))
				return FALSE;
		}

		if(rule1->srcFilterIgnoreL3L4_==0)
		{
			if((rule1->srcFilterIpAddr_ != rule2->srcFilterIpAddr_)||
				(rule2->srcFilterIpAddrMask_ != rule2->srcFilterIpAddrMask_))
				return FALSE;
		}

		break;

	case RTL865X_ACL_DSTFILTER:
	case RTL865X_ACL_DSTFILTER_IPRANGE:
		if(	memcmp(&rule1->dstFilterMac_, &rule2->dstFilterMac_, sizeof(ether_addr_t)) != 0||
			memcmp(&rule1->dstFilterMacMask_, &rule2->dstFilterMacMask_,sizeof(ether_addr_t)) != 0||
			(rule1->dstFilterVlanIdx_ != rule2->dstFilterVlanIdx_)||
			(rule1->dstFilterVlanIdxMask_ != rule2->dstFilterVlanIdxMask_)||
			(rule1->dstFilterIgnoreL3L4_ != rule2->dstFilterIgnoreL3L4_)||
			(rule1->dstFilterIgnoreL4_ != rule2->dstFilterIgnoreL4_))
		{
			return FALSE;
		}

		if(rule1->dstFilterIgnoreL4_==0 && rule1->dstFilterIgnoreL4_==0)
		{
			if((rule1->dstFilterPortUpperBound_ != rule2->dstFilterPortUpperBound_)||
			   (rule1->dstFilterPortLowerBound_ != rule2->dstFilterPortLowerBound_))
				return FALSE;
		}

		if(rule1->dstFilterIgnoreL3L4_==0)
		{
			if((rule1->dstFilterIpAddr_ != rule2->dstFilterIpAddr_)||
				(rule2->dstFilterIpAddrMask_ != rule2->dstFilterIpAddrMask_))
				return FALSE;
		}

		break;
	default: return FALSE; /* Unknown rule type */

	}
	/* Compare common part */
	if (rule1->srcIpAddr_ != rule2->srcIpAddr_ || rule1->srcIpAddrMask_ != rule2->srcIpAddrMask_ ||
		rule1->dstIpAddr_ != rule2->dstIpAddr_ || rule1->dstIpAddrMask_ != rule2->dstIpAddrMask_ ||
		rule1->tos_ != rule2->tos_ || rule1->tosMask_ != rule2->tosMask_ )
		return FALSE;
	return TRUE;
}


static int32 _rtl865x_addAclToChain(rtl865x_AclRule_t *rule, rtl865x_AclRule_t **head, rtl865x_AclRule_t **tail)
{
	rtl865x_AclRule_t *addAcl;
	rtl865x_AclRule_t *tmpRule;

	if(head == NULL || tail == NULL)
	{
		return RTL_EINVALIDINPUT;
	}


	if((*head) != NULL)
	{
		tmpRule = *head;
		while(tmpRule)
		{
			if(_rtl865x_sameAclRule(tmpRule, rule)==TRUE)
			{
				return RTL_EENTRYALREADYEXIST;
			}
			tmpRule = tmpRule->next;
		}
	}

	addAcl = freeAclList.freeHead;
	if(addAcl == NULL)
		return RTL_ENOFREEBUFFER;

	/*remove acl buffer from freeAclList*/
	freeAclList.freeHead = freeAclList.freeHead->next;
	if(freeAclList.freeHead)
		freeAclList.freeHead->pre = NULL;
	freeAclList.freeCnt--;

	memcpy(addAcl,rule,sizeof(rtl865x_AclRule_t));

	addAcl->pre = addAcl->next = NULL;
	if((*head) == NULL)
	{
		/*head = null, tail must null*/
		addAcl->aclIdx = 1;
		*head = addAcl;
		*tail = addAcl;
	}
	else
	{
		if(addAcl->aclIdx == 0 || addAcl->aclIdx > (*tail)->aclIdx)
		{
			/*append this rule to tail*/
			addAcl->aclIdx = (*tail)->aclIdx + 1;
			(*tail)->next = addAcl;
			addAcl->pre = *tail;
			addAcl->next = NULL;
			*tail = addAcl;
		}
		else
		{
			/*user specified the index, it's means: this rule should be inserted before the rule->aclIdx*/

			tmpRule = *head;
			while(tmpRule)
			{
				if(tmpRule->aclIdx == addAcl->aclIdx)
				{
					/*found the rule...*/
					break;
				}

				tmpRule = tmpRule->next;
			}

			if(tmpRule == NULL)
			{
				/*not found the correct position, append this rule at the tail??*/


				printk("%s(%d): BUG!!!\n",__FUNCTION__,__LINE__);

				addAcl->pre = NULL;
				addAcl->next = freeAclList.freeHead;
				if(freeAclList.freeHead)
					freeAclList.freeHead->pre = addAcl;
				freeAclList.freeHead = addAcl;
				freeAclList.freeCnt++;
				return FAILED;
			}

			/*insert new rule before the found rule*/
			if(tmpRule->pre == NULL)
			{
				/*tmpRule->pre = null, means: tmprule is the head of this chain*/
				addAcl->next = tmpRule;
				tmpRule ->pre = addAcl;
				*head = addAcl;
				addAcl->aclIdx = 1;
			}
			else
			{
				tmpRule->pre->next = addAcl;
				addAcl->pre = tmpRule->pre;
				addAcl->next = tmpRule;
				tmpRule->pre = addAcl;
			}

			/*update aclIdx...*/
			while(tmpRule)
			{
				tmpRule->aclIdx++;
				tmpRule = tmpRule->next;
			}

		}
	}
	return SUCCESS;
}

static int32 _rtl865x_delAclFromChain(rtl865x_AclRule_t *rule, rtl865x_AclRule_t **head, rtl865x_AclRule_t **tail)
{
	rtl865x_AclRule_t *delRule,*nextRule;
	int8 isSame = FALSE;

	if(head == NULL || tail == NULL)
	{
		return RTL_EINVALIDINPUT;
	}

	delRule = *head;
	while(delRule)
	{
		if(rule->aclIdx != 0)
		{
			if(rule->aclIdx == delRule->aclIdx)
			{
				break;
			}
		}
		else
		{
			isSame = _rtl865x_sameAclRule(delRule,rule);
			if(isSame == TRUE)
			{
				break;
			}
		}

		delRule = delRule->next;
	}

	if(delRule == NULL)
		return RTL_EENTRYNOTFOUND;

	/*remove the acl rule from chains to free list*/
	nextRule = delRule->next;
	if(delRule->pre)
		delRule->pre->next = delRule->next;
	if(delRule->next)
		delRule->next->pre = delRule->pre;

	if(delRule == *head)
		*head = delRule->next;

	if(delRule == *tail)
		*tail = delRule->pre;

	/*inser the rule to free list*/
	delRule->pre = NULL;
	delRule->next = freeAclList.freeHead;
	if(freeAclList.freeHead)
		freeAclList.freeHead->pre = delRule;
	freeAclList.freeHead = delRule;
	freeAclList.freeCnt++;

	/*update acl index whose position is after the delRule*/
	while(nextRule)
	{
		nextRule->aclIdx--;
		nextRule = nextRule->next;
	}
	return SUCCESS;
}

static int _rtl865x_checkDefAclAvailable(rtl865x_AclRule_t *endRule)
{
	rtl865x_AclRule_t defRule;
	bzero((void*)&defRule,sizeof(rtl865x_AclRule_t));
	
	
	if(endRule==NULL)
	{
		return 0;
	}
			

	defRule.ruleType_ = RTL865X_ACL_MAC;
	defRule.actionType_ 	= RTL865X_ACL_PERMIT;
	defRule.pktOpApp_		= RTL865X_ACL_ALL_LAYER;
	
	if(_rtl865x_sameAclRule(&defRule,endRule)==TRUE)
	{
		return 1;
	}


	defRule.actionType_ 	= RTL865X_ACL_TOCPU;
	if(_rtl865x_sameAclRule(&defRule,endRule)==TRUE)
	{
		return 1;
	}


	defRule.actionType_ 	= RTL865X_ACL_DROP;
	if(_rtl865x_sameAclRule(&defRule,endRule)==TRUE)
	{
		return 1;
	}

	return 0;
}


#if defined(CONFIG_RTL_IPTABLES2ACL_PATCH)
int32 _rtl865x_synAclwithAsicTbl(void)
#else
static int32 _rtl865x_synAclwithAsicTbl(void)
#endif
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_acl_chain_t *chain;
	rtl865x_AclRule_t *rule;
	int32 i,startIdx,addCnt,totalAddCnt;

	rtl865x_AclRule_t *preRule=NULL;
	rtl865x_AclRule_t *endRule=NULL;

	/*prepare default permit acl*/
	rtl865x_AclRule_t defRule;
	bzero((void*)&defRule,sizeof(rtl865x_AclRule_t));
	defRule.aclIdx=1;
	defRule.ruleType_ = RTL865X_ACL_MAC;
	defRule.actionType_		= RTL865X_ACL_PERMIT;
	defRule.pktOpApp_ 		= RTL865X_ACL_ALL_LAYER;
	
	//hyking:when rearrange asic acl, permit all first...
	_rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);
	startIdx = 0;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		netif = &netifTbl[i];
		if(netif->valid)
		{
			/*ingress acl*/
			chain = netif->chainListHead[RTL865X_ACL_INGRESS];
			totalAddCnt = 0;
			netif->inAclStart = startIdx;
			preRule=NULL;
			while(chain)
			{
				addCnt = 0;
				
				rule = chain->head;
				
				while(rule)
				{
					
					if((preRule==NULL) || (_rtl865x_sameAclRule(preRule,rule)==FALSE))
					{	
						_rtl865x_setAclToAsic(startIdx, rule);
						addCnt++;
					}
					
					preRule=rule;
					endRule=rule;
					rule = rule->next;
				}
				/*next chain..*/
				chain = chain->nextChain;
				startIdx += addCnt;
				totalAddCnt += addCnt;
			}

			/*addCnt = 0: default permit??*/
			if(totalAddCnt > 0)
			{
				netif->inAclEnd = netif->inAclStart + totalAddCnt -1;
//				startIdx += addCnt;

				if(netif->inAclEnd<(RTL865X_ACL_MAX_NUMBER-1))
				{
					if(_rtl865x_checkDefAclAvailable(endRule)==0)
					{
						/*add default acl to permit all*/
						_rtl865x_setAclToAsic(startIdx, &defRule);
						startIdx++;
						netif->inAclEnd++;
						
					}
					
				}
		
			}
			else
				netif->inAclEnd = netif->inAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/

			/*egress acl*/
			chain = netif->chainListHead[RTL865X_ACL_EGRESS];
			totalAddCnt = 0;
			netif->outAclStart = startIdx;
			preRule=NULL;
			while(chain)
			{
				addCnt = 0;
				rule = chain->head;
				while(rule)
				{
					if((preRule==NULL) || (_rtl865x_sameAclRule(preRule,rule)==FALSE))
					{
						_rtl865x_setAclToAsic(startIdx, rule);
						addCnt++;
					}
				
					preRule=rule;
					endRule=rule;
					rule = rule->next;
				}
				chain = chain->nextChain;
				startIdx += addCnt;
				totalAddCnt += addCnt;
			}

			if(totalAddCnt > 0)
			{
				netif->outAclEnd = netif->outAclStart + totalAddCnt -1;
//				startIdx += addCnt;
				if(netif->outAclEnd<(RTL865X_ACL_MAX_NUMBER-1))
				{
					if(_rtl865x_checkDefAclAvailable(endRule)==0)
					{

						/*add default acl to permit all*/
						_rtl865x_setAclToAsic(startIdx, &defRule);
						startIdx++;
						netif->outAclEnd++;
					}
					
				}

			

			}
			else
				netif->outAclEnd = netif->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/

			#if defined(CONFIG_RTL_MULTIPLE_WAN)  ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
				netif->enableRoute = (rtl8651_getAsicOperationLayer()>2)? 1:0;
			#endif

			_rtl865x_setAsicNetif(netif);

		}
	}

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	{

		netif = &virtualNetIf;
		if(netif->valid)
		{
			chain = netif->chainListHead[RTL865X_ACL_INGRESS];
			totalAddCnt = 0;
			netif->inAclStart = startIdx;
			while(chain)
			{
				addCnt = 0;
				/*ingress acl*/
				rule = chain->head;
				while(rule)
				{
					_rtl865x_setAclToAsic(startIdx, rule);
					addCnt++;
					rule = rule->next;
				}
				/*next chain..*/
				chain = chain->nextChain;
				startIdx += addCnt;
				totalAddCnt += addCnt;
			}

			/*addCnt = 0: default permit??*/
			if(totalAddCnt > 0)
			{
				netif->inAclEnd = netif->inAclStart + totalAddCnt -1;
//				startIdx += addCnt;
			}
			else
				netif->inAclEnd = netif->inAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/


			/*egress acl*/
			chain = netif->chainListHead[RTL865X_ACL_EGRESS];
			totalAddCnt = 0;
			netif->outAclStart = startIdx;
			while(chain)
			{
				addCnt = 0;
				rule = chain->head;
				while(rule)
				{
					_rtl865x_setAclToAsic(startIdx, rule);
					addCnt++;
					rule = rule->next;
				}
				chain = chain->nextChain;
				startIdx += addCnt;
				totalAddCnt += addCnt;
			}

			if(totalAddCnt > 0)
			{
				netif->outAclEnd = netif->outAclStart + totalAddCnt -1;
//				startIdx += addCnt;
			}
			else
				netif->outAclEnd = netif->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/
			 //printk("netif->inAclStart is %d,netif->inAclEnd is %d,netif->outAclStart is %d, netif->outAclEnd is %d\n",netif->inAclStart,netif->inAclEnd,netif->outAclStart, netif->outAclEnd);
			 rtl865x_setDefACLForNetDecisionMiss(netif->inAclStart, netif->inAclEnd, netif->outAclStart, netif->outAclEnd);
		}
	}
#endif
	return SUCCESS;
}


/*flag, 0: ingress, 1: egress*/
static int32 _rtl865x_regist_aclChain(char *netifName, int32 priority,uint32 flag)
{
	rtl865x_netif_local_t *netif;
	rtl865x_acl_chain_t *addEntry,*chainEntry;
	uint32 aclDir = RTL865X_ACL_INGRESS;/*default :RTL865X_ACL_INGRESS*/

	netif = _rtl865x_getNetifByName(netifName);

	if(netif == NULL)
		return RTL_ENETIFINVALID;

	if(flag == RTL865X_ACL_EGRESS)
		aclDir = RTL865X_ACL_EGRESS;

	chainEntry = netif->chainListHead[aclDir];


	while(chainEntry)
	{
		if(chainEntry->priority < priority)
			chainEntry = chainEntry->nextChain;
		else
			/*bingo!, find the position*/
			break;
	}

	/*duplicate?*/
	if(chainEntry && chainEntry->priority == priority)
		return RTL_EENTRYALREADYEXIST;

	addEntry = freeChainHead;
	if(addEntry == NULL)
		return RTL_ENOFREEBUFFER;

	freeChainHead = addEntry->nextChain;
	if(freeChainHead)
		freeChainHead->preChain = NULL;

	memset(addEntry,0,sizeof(rtl865x_acl_chain_t));
	addEntry->ruleCnt = 0;
	addEntry->priority = priority;
	addEntry->head = NULL;
	addEntry->tail = NULL;
	addEntry->preChain = addEntry->nextChain = NULL;

	if(chainEntry)
	{
		/*insert addentry before the chainEntry*/
		addEntry->nextChain = chainEntry;
		addEntry->preChain = chainEntry->preChain;

		if(chainEntry->preChain)
			chainEntry->preChain->nextChain = addEntry;
		else
		{
			/*insert before head???*/
			netif->chainListHead[aclDir] = addEntry;
		}

		chainEntry->preChain = addEntry;
	}
	else
	{
		if(netif->chainListHead[aclDir] == NULL)
			netif->chainListHead[aclDir] = addEntry;
		else
		{
			/*append addEntry to the tail of this list...*/
			chainEntry = netif->chainListHead[aclDir];
			while(chainEntry)
			{
				if(chainEntry->nextChain == NULL)
					break;

				chainEntry = chainEntry->nextChain;
			}

			if(chainEntry == NULL)
				printk("%s(%d) BUG!!!!\n",__FUNCTION__,__LINE__);

			chainEntry->nextChain = addEntry;
			addEntry->preChain = chainEntry;
		}
	}

	return SUCCESS;

}


static int32 _rtl865x_unRegist_aclChain(char *netifName,int32 priority, uint32 flag)
{
	rtl865x_netif_local_t *netif;
	rtl865x_AclRule_t *aclRule;
	rtl865x_acl_chain_t *chainEntry;
	uint32 aclDir = RTL865X_ACL_INGRESS;/*default :RTL865X_ACL_INGRESS*/

	netif = _rtl865x_getNetifByName(netifName);

	if(netif == NULL)
		return RTL_ENETIFINVALID;

	if(flag == RTL865X_ACL_EGRESS)
		aclDir = RTL865X_ACL_EGRESS;

	chainEntry = netif->chainListHead[aclDir];


	while(chainEntry)
	{
		if(chainEntry->priority == priority)
			break;
		chainEntry = chainEntry->nextChain;
	}

	if(chainEntry == NULL)
		return RTL_EENTRYNOTFOUND;

	/*remove all aclrule*/
	aclRule = chainEntry->head;
	while(aclRule)
	{
		_rtl865x_delAclFromChain(aclRule, &chainEntry->head, &chainEntry->tail);
		aclRule = chainEntry->head;
		chainEntry->ruleCnt--;
	}

	chainEntry->ruleCnt = 0;

	if(chainEntry->nextChain)
		chainEntry->nextChain->preChain = chainEntry->preChain;

	if(chainEntry->preChain)
		chainEntry->preChain->nextChain = chainEntry->nextChain;
	else
		/*remove head???*/
		netif->chainListHead[aclDir] = chainEntry->nextChain;


	memset(chainEntry,0,sizeof(rtl865x_acl_chain_t));

	chainEntry->nextChain = freeChainHead;
	if (freeChainHead)
		freeChainHead->preChain = chainEntry;
	freeChainHead = chainEntry;

	_rtl865x_synAclwithAsicTbl();

	return SUCCESS;
}

static int32 _rtl865x_flush_allAcl_fromChain(char *netifName,int32 priority, uint32 flag)
{
	rtl865x_netif_local_t *netif;
	rtl865x_AclRule_t *aclRule;
	rtl865x_acl_chain_t *chainEntry;
	uint32 aclDir = RTL865X_ACL_INGRESS;/*default :RTL865X_ACL_INGRESS*/

	if(flag == RTL865X_ACL_EGRESS)
		aclDir = RTL865X_ACL_EGRESS;

	if(netifName && netifName[0] != '\0')
	{
		netif = _rtl865x_getNetifByName(netifName);

		if(netif == NULL)
			return RTL_ENETIFINVALID;

		chainEntry = netif->chainListHead[aclDir];
		while(chainEntry)
		{
			if(chainEntry->priority == priority)
				break;
			chainEntry = chainEntry->nextChain;
		}

		if(chainEntry == NULL)
			return RTL_EENTRYNOTFOUND;

		/*remove all aclrule*/
		aclRule = chainEntry->head;
		while(aclRule)
		{
			_rtl865x_delAclFromChain(aclRule, &chainEntry->head, &chainEntry->tail);
			aclRule = chainEntry->head;
			chainEntry->ruleCnt--;
		}

		chainEntry->ruleCnt = 0;

	}
	else
	{
		/*add this rule to every netif*/
		int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
		{
			netif = &netifTbl[i];
			if(netif->valid)
			{
				chainEntry = netif->chainListHead[aclDir];
				while(chainEntry)
				{
					if(chainEntry->priority == priority)
						break;
					chainEntry = chainEntry->nextChain;
				}

				if(chainEntry == NULL)
					continue;

				/*remove all aclrule*/
				aclRule = chainEntry->head;
				while(aclRule)
				{
					_rtl865x_delAclFromChain(aclRule, &chainEntry->head, &chainEntry->tail);
					aclRule = chainEntry->head;
					chainEntry->ruleCnt--;
				}

				chainEntry->ruleCnt = 0;
			}
		}
	}

	_rtl865x_synAclwithAsicTbl();

	return SUCCESS;
}

static rtl865x_AclRule_t* _rtl865x_matched_layer4_aclChain(char *netifName,int32 priority, uint32 flag, rtl865x_AclRule_t *match)
{
	rtl865x_netif_local_t *netif;
	rtl865x_AclRule_t *aclRule;
	rtl865x_acl_chain_t *chainEntry;
	uint32			isRange;
	uint32 aclDir = RTL865X_ACL_INGRESS;/*default :RTL865X_ACL_INGRESS*/

	if(flag == RTL865X_ACL_EGRESS)
		aclDir = RTL865X_ACL_EGRESS;

	if(netifName && netifName[0] != '\0')
	{
		netif = _rtl865x_getNetifByName(netifName);

		if(netif == NULL)
			return NULL;

		chainEntry = netif->chainListHead[aclDir];
		while(chainEntry)
		{
			if(chainEntry->priority == priority)
				break;
			chainEntry = chainEntry->nextChain;
		}

		if(chainEntry == NULL)
			return NULL;

		/*check all aclrule*/
		for(aclRule = chainEntry->head;aclRule;aclRule = aclRule->next)
		{
			isRange = FALSE;
			switch(aclRule->ruleType_)
			{
				case RTL865X_ACL_TCP_IPRANGE:
					isRange = TRUE;
				case RTL865X_ACL_TCP:
					if ( (aclRule->tcpSrcPortLB_<=match->tcpSrcPortLB_)
						&&(aclRule->tcpSrcPortUB_>=match->tcpSrcPortLB_)
						&&(aclRule->tcpDstPortLB_<=match->tcpDstPortLB_)
						&&(aclRule->tcpDstPortUB_>=match->tcpDstPortLB_)
						&& (match->ruleType_==RTL865X_ACL_TCP))
						{
							break;
						}
					else
						continue;
				case RTL865X_ACL_UDP_IPRANGE:
					isRange = TRUE;
				case RTL865X_ACL_UDP:
					if ( (aclRule->udpSrcPortLB_<=match->tcpSrcPortLB_)
						&&(aclRule->udpSrcPortUB_>=match->tcpSrcPortLB_)
						&&(aclRule->udpDstPortLB_<=match->tcpDstPortLB_)
						&&(aclRule->udpDstPortUB_>=match->tcpDstPortLB_)
						&&(match->ruleType_==RTL865X_ACL_UDP))
						{
							break;
						}
					else
						continue;
				case RTL865X_ACL_IP_RANGE:
					isRange = TRUE;
				case RTL865X_ACL_IP:
					if ( ((aclRule->tos_&aclRule->tosMask_)==(match->tos_&aclRule->tosMask_))
						&& ((aclRule->ipProto_&aclRule->ipProtoMask_)==(match->ipProto_&aclRule->ipProtoMask_))
						&& ((aclRule->ipFlag_&aclRule->ipFlagMask_)==(match->ipFlag_&aclRule->ipFlagMask_)))
					{
						break;
					}
					else
						continue;
				default:
					continue;
			}

			if (isRange)
			{
				if ( (aclRule->srcIpAddrStart_<=match->srcIpAddr_)
					&&(aclRule->srcIpAddrEnd_>=match->srcIpAddr_)
					&&(aclRule->dstIpAddrStart_<=match->dstIpAddr_)
					&&(aclRule->dstIpAddrEnd_>=match->dstIpAddr_) )
				{
					break;
				}
				else
					continue;
			}
			else
			{
				if ( ((aclRule->srcIpAddr_&aclRule->srcIpAddrMask_)
					==(match->srcIpAddr_&aclRule->srcIpAddrMask_))
					&&((aclRule->dstIpAddr_&aclRule->dstIpAddrMask_)
					==(match->dstIpAddr_&aclRule->dstIpAddrMask_)) )
				{
					break;
				}
				else
					continue;
			}
		}
	}
	else
	{
		return NULL;
	}

	return aclRule;
}

static inline int _rtl865x_cmpMacAddr(ether_addr_t *mac1, ether_addr_t *mac2, ether_addr_t *mask)
{
	int	i;

	for(i=0;i<ETHER_ADDR_LEN;i++)
	{
		if ((mac1->octet[i]&mask->octet[i])!=(mac2->octet[i]&mask->octet[i]))
			return (SUCCESS+i+1);
	}
	return SUCCESS;
}

static rtl865x_AclRule_t* _rtl865x_matched_layer2_aclChain(char *netifName,int32 priority, uint32 flag, rtl865x_AclRule_t *match)
{
	rtl865x_netif_local_t *netif;
	rtl865x_AclRule_t *aclRule;
	rtl865x_acl_chain_t *chainEntry;
	uint32 aclDir = RTL865X_ACL_INGRESS;/*default :RTL865X_ACL_INGRESS*/

	if(flag == RTL865X_ACL_EGRESS)
		aclDir = RTL865X_ACL_EGRESS;

	if(netifName && netifName[0] != '\0')
	{
		netif = _rtl865x_getNetifByName(netifName);

		if(netif == NULL)
			return NULL;

		chainEntry = netif->chainListHead[aclDir];
		while(chainEntry)
		{
			if(chainEntry->priority == priority)
				break;
			chainEntry = chainEntry->nextChain;
		}

		if(chainEntry == NULL)
			return NULL;

		/*check all aclrule*/
		for(aclRule = chainEntry->head;aclRule;aclRule = aclRule->next)
		{
			switch(aclRule->ruleType_)
			{
				case RTL865X_ACL_MAC:
					break;
				default:
					continue;
			}

			{
				if ( _rtl865x_cmpMacAddr(&aclRule->srcMac_, &match->srcMac_, &aclRule->srcMacMask_)==SUCCESS
					&&_rtl865x_cmpMacAddr(&aclRule->dstMac_, &match->dstMac_, &aclRule->dstMacMask_)==SUCCESS
					&&((aclRule->typeLen_&aclRule->typeLenMask_)==(match->typeLen_&aclRule->typeLenMask_))
					&& aclRule->ruleType_==match->ruleType_)
				{
					break;
				}
				else
					continue;
			}
		}
	}
	else
	{
		return NULL;
	}

	return aclRule;
}

static int32 _rtl865x_unRegister_all_aclChain(char *netifName)
{
	rtl865x_netif_local_t *netif;
	rtl865x_AclRule_t *aclRule;
	rtl865x_acl_chain_t *chainEntry;
	uint32 aclDir = RTL865X_ACL_INGRESS;

	netif = _rtl865x_getNetifByName(netifName);
	if(netif == NULL)
		return RTL_ENETIFINVALID;

	for(aclDir = RTL865X_ACL_INGRESS; aclDir <= RTL865X_ACL_EGRESS; aclDir++)
	{
		chainEntry = netif->chainListHead[aclDir];
		while(chainEntry)
		{
			/*remove all aclrule*/
			aclRule = chainEntry->head;
			while(aclRule)
			{
				_rtl865x_delAclFromChain(aclRule, &chainEntry->head, &chainEntry->tail);
				aclRule = chainEntry->head;
			}

			chainEntry->ruleCnt = 0;
			netif->chainListHead[aclDir] = chainEntry->nextChain;

			/*remove to freelist*/
			chainEntry->nextChain = freeChainHead;
			if(freeChainHead)
				freeChainHead->preChain = chainEntry;
			freeChainHead = chainEntry;

			chainEntry = netif->chainListHead[aclDir];
		}
	}

	netif->inAclEnd = netif->inAclStart = netif->outAclEnd = netif->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/
	_rtl865x_setAsicNetif(netif);

	return SUCCESS;
}

static rtl865x_acl_chain_t* _rtl865x_find_aclChain_byPriority(rtl865x_acl_chain_t *head, int32 priority)
{
	rtl865x_acl_chain_t *entry;
	entry = head;
	while(entry)
	{
		if(entry->priority == priority)
			break;
		entry = entry->nextChain;
	}

	return entry;
}

static int32 _rtl865x_add_acl(rtl865x_AclRule_t *rule, char *netifName,int32 chainNo)
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_acl_chain_t *chain = NULL;
	int32 isSynAsic = 0;
	int32 retval = FAILED;

	if(rule == NULL)
		return RTL_EINVALIDINPUT;

	if(netifName && netifName[0] != '\0' && (rule->inv_flag == 0))
	{
		netif = _rtl865x_getNetifByName(netifName);
		if(netif == NULL)
			return RTL_EINVALIDINPUT;

		chain = _rtl865x_find_aclChain_byPriority(netif->chainListHead[rule->direction_],chainNo);
		if(chain == NULL)
			return RTL_EENTRYNOTFOUND;

		retval = _rtl865x_addAclToChain(rule, &(chain->head), &(chain->tail));
		if(retval == SUCCESS)
		{
			isSynAsic = 1;
			chain->ruleCnt++;
		}
	}
	else
	{
		/*add this rule to every netif*/
		int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
		{
			netif = &netifTbl[i];
			if(netif->valid == 0)
				continue;

			/*inv_flag != 0, means alc should add to the other netifs else netifName*/
			/*now: the inv_flag only should be RTL865X_INVERT_IN_NETIF or RTL865X_INVERT_OUT_NETIF*/
			if(netifName && netifName[0] != '\0' && rule->inv_flag != 0)
			{
				if(memcmp(netif->name,netifName,strlen(netifName)) == 0)
					continue;
			}

			/*add rule to netif*/
			{
				chain = _rtl865x_find_aclChain_byPriority(netif->chainListHead[rule->direction_],chainNo);
				if(chain == NULL)
					continue;

				retval = _rtl865x_addAclToChain(rule, &(chain->head), &(chain->tail));
				if(retval == SUCCESS)
				{
					isSynAsic = 1;
					chain->ruleCnt++;
				}
			}
		}
	}

	if(isSynAsic == 1)
		_rtl865x_synAclwithAsicTbl();

	return retval;
}

static int32 _rtl865x_del_acl(rtl865x_AclRule_t *rule, char *netifName,int32 chainNo)
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_acl_chain_t *chain = NULL;
	int32 isSynAsic = 0;
	int32 retval = FAILED;

	if(rule == NULL)
		return RTL_EINVALIDINPUT;

	if(netifName)
	{
		netif = _rtl865x_getNetifByName(netifName);
		if(netif == NULL)
			return RTL_EINVALIDINPUT;
	}


	if(netif != NULL)
	{
		chain = _rtl865x_find_aclChain_byPriority(netif->chainListHead[rule->direction_],chainNo);
		if(chain == NULL)
			return RTL_EENTRYNOTFOUND;

		retval = _rtl865x_delAclFromChain(rule, &(chain->head), &(chain->tail));
		if(retval == SUCCESS)
		{
			isSynAsic = 1;
			chain->ruleCnt--;
		}
	}
	else
	{
		/*remvoe this rule from every netif*/
		int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
		{
			netif = &netifTbl[i];
			if(netif->valid)
			{
				chain = _rtl865x_find_aclChain_byPriority(netif->chainListHead[rule->direction_],chainNo);
				if(chain == NULL)
					continue;
				retval = _rtl865x_delAclFromChain(rule, &(chain->head), &(chain->tail));
				if(retval == SUCCESS)
				{
					isSynAsic = 1;
					chain->ruleCnt--;
				}
			}
		}
	}

	if(isSynAsic == 1)
		_rtl865x_synAclwithAsicTbl();

	return retval;

}

/*
@func int32 | rtl865x_regist_aclChain | register an acl chain for a network interface.
@parm char* | netifName | network interface Name.
@parm int32 | priority | priority of this acl chain.
@parm uint32 | flag | flags. it's value should be RTL865X_ACL_EGRESS or RTL865X_ACL_INGRESS.
@rvalue SUCCESS | Success.
@rvalue RTL_ENETIFINVALID | network interface is invalid.
@rvalue RTL_EENTRYALREADYEXIST | acl chain is already exist.
@rvalue RTL_ENOFREEBUFFER | no aclchains buffer.
@comm
In realtek 865x, there are two types acl list in each network interface: Ingress and Egress ACL list.
NOTE 1:the priority of acl chain is the primary key, which should be unique!
	  2:priority RTL865X_ACL_USER_USED = 0 is used by system.
	  3:acl chain with minimal priority value hold the highest priority.
In order to easily configure the ACL list, acl chains architecture is designed. lots of acl chains are allowed in one network work interface and lots of ACL rules can be attached in an acl chain.
according to the priority of acl chains, ACL engine sequentially scan ACL rules in each chain.
for example:
ingress acl head--->acl chain(priority0)--->acl chain(priority1)--->acl chain(priority2)
					|					|					|
					|->acl rule1			|->acl rule1			|->aclrule1
					|->acl rule2			|->acl rule2			|->aclrule2
					|->acl rule3			|->acl rule3			|->aclrule3
					|->acl rule4								|->aclrule4
					|->acl rule5								|->aclrule5


*/
int32 rtl865x_regist_aclChain(char *netifName, int32 priority, uint32 flag)
{
	return _rtl865x_regist_aclChain(netifName, priority, flag);
}

/*
@func int32 | rtl865x_unRegist_aclChain | unregister an acl chain from a network interface.
@parm char* | netifName | network interface Name.
@parm int32 | priority | priority of this acl chain.
@parm uint32 | flag | flags. it's value should be RTL865X_ACL_EGRESS or RTL865X_ACL_INGRESS.
@rvalue SUCCESS | Success.
@rvalue RTL_ENETIFINVALID | network interface is invalid.
@rvalue RTL_EENTRYNOTFOUND | Not found the acl chain .
@comm
an acl chain is unregistered, all ACL rules which attach on the acl chain are deleted at the same time.
*/
int32 rtl865x_unRegist_aclChain(char *netifName, int32 priority, uint32 flag)
{
	//printk("============%s(%d),netif(%s),priority(%d),flag(%d)\n",__FUNCTION__,__LINE__,netifName,priority,flag);
	return _rtl865x_unRegist_aclChain(netifName, priority, flag);
}

/*
@func int32 | rtl865x_flush_allAcl_fromChain | delete all Acl Rules which attach on the acl chain.
@parm char* | netifName | network interface Name.
@parm int32 | priority | priority of this acl chain.
@parm uint32 | flag | flags. it's value should be RTL865X_ACL_EGRESS or RTL865X_ACL_INGRESS.
@rvalue SUCCESS | Success.
@rvalue RTL_ENETIFINVALID | network interface is invalid.
@rvalue RTL_EENTRYNOTFOUND | Not found the acl chain .
@comm
	just delete the ACL rules, the acl chain is alive.
*/
int32 rtl865x_flush_allAcl_fromChain(char *netifName, int32 priority, uint32 flag)
{
	return _rtl865x_flush_allAcl_fromChain(netifName, priority, flag);
}

#if defined(CONFIG_RTL_IPTABLES2ACL_PATCH)

static int32 _rtl865x_flush_allAcl_sw_fromChain(char *netifName,int32 priority, uint32 flag)
{
	rtl865x_netif_local_t *netif;
	rtl865x_AclRule_t *aclRule;
	rtl865x_acl_chain_t *chainEntry;
	uint32 aclDir = RTL865X_ACL_INGRESS;/*default :RTL865X_ACL_INGRESS*/

	if(flag == RTL865X_ACL_EGRESS)
		aclDir = RTL865X_ACL_EGRESS;

	if(netifName && netifName[0] != '\0')
	{
		netif = _rtl865x_getNetifByName(netifName);

		if(netif == NULL)
			return RTL_ENETIFINVALID;

		chainEntry = netif->chainListHead[aclDir];
		while(chainEntry)
		{
			if(chainEntry->priority == priority)
				break;
			chainEntry = chainEntry->nextChain;
		}

		if(chainEntry == NULL)
			return RTL_EENTRYNOTFOUND;

		/*remove all aclrule*/
		aclRule = chainEntry->head;
		while(aclRule)
		{
			_rtl865x_delAclFromChain(aclRule, &chainEntry->head, &chainEntry->tail);
			aclRule = chainEntry->head;
			chainEntry->ruleCnt--;
		}

		chainEntry->ruleCnt = 0;

	}
	else
	{
		/*add this rule to every netif*/
		int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
		{
			netif = &netifTbl[i];
			if(netif->valid)
			{
				chainEntry = netif->chainListHead[aclDir];
				while(chainEntry)
				{
					if(chainEntry->priority == priority)
						break;
					chainEntry = chainEntry->nextChain;
				}

				if(chainEntry == NULL)
					continue;

				/*remove all aclrule*/
				aclRule = chainEntry->head;
				while(aclRule)
				{
					_rtl865x_delAclFromChain(aclRule, &chainEntry->head, &chainEntry->tail);
					aclRule = chainEntry->head;
					chainEntry->ruleCnt--;
				}

				chainEntry->ruleCnt = 0;
			}
		}
	}

	//_rtl865x_synAclwithAsicTbl();
	return SUCCESS;
}

int32 rtl865x_flush_allAcl_sw_fromChain(char *netifName, int32 priority, uint32 flag)
{
	return _rtl865x_flush_allAcl_sw_fromChain(netifName, priority, flag);
}

static int32 _rtl865x_add_sw_acl(rtl865x_AclRule_t *rule, char *netifName,int32 chainNo)
{
	rtl865x_netif_local_t *netif = NULL;
	rtl865x_acl_chain_t *chain = NULL;
	int32 isSynAsic = 0;
	int32 retval = FAILED;

	if(rule == NULL)
		return RTL_EINVALIDINPUT;

	if(netifName && netifName[0] != '\0' && (rule->inv_flag == 0))
	{
		netif = _rtl865x_getNetifByName(netifName);
		if(netif == NULL)
			return RTL_EINVALIDINPUT;

		chain = _rtl865x_find_aclChain_byPriority(netif->chainListHead[rule->direction_],chainNo);
		if(chain == NULL)
			return RTL_EENTRYNOTFOUND;

		retval = _rtl865x_addAclToChain(rule, &(chain->head), &(chain->tail));
		if(retval == SUCCESS)
		{
			isSynAsic = 1;
			chain->ruleCnt++;
		}
	}
	else
	{
		/*add this rule to every netif*/
		int32 i;
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
		{
			netif = &netifTbl[i];
			if(netif->valid == 0)
				continue;

			/*inv_flag != 0, means alc should add to the other netifs else netifName*/
			/*now: the inv_flag only should be RTL865X_INVERT_IN_NETIF or RTL865X_INVERT_OUT_NETIF*/
			if(netifName && netifName[0] != '\0' && rule->inv_flag != 0)
			{
				if(memcmp(netif->name,netifName,strlen(netifName)) == 0)
					continue;
			}

			/*add rule to netif*/
			{
				chain = _rtl865x_find_aclChain_byPriority(netif->chainListHead[rule->direction_],chainNo);
				if(chain == NULL)
					continue;

				retval = _rtl865x_addAclToChain(rule, &(chain->head), &(chain->tail));
				if(retval == SUCCESS)
				{
					isSynAsic = 1;
					chain->ruleCnt++;
				}
			}
		}
	}

	//if(isSynAsic == 1)
		//_rtl865x_synAclwithAsicTbl();

	return retval;
}


int32 rtl865x_add_sw_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//printk("********%s(%d)*********,netif(%s),priority(%d)\n",__FUNCTION__,__LINE__,netifName,priority);
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if(rtl865x_acl_enable == 0)
		return retval;
#endif
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_add_sw_acl(rule,netifName,priority);
	if(retval == RTL_ENOFREEBUFFER){	// acl entries is full.
		_rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);
	}
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH(flags);
	//printk("********%s(%d)*********retval(%d)\n",__FUNCTION__,__LINE__,retval);
	return retval;
}

#endif

rtl865x_AclRule_t* rtl865x_matched_layer4_aclChain(char *netifName,int32 priority, uint32 flag, rtl865x_AclRule_t *match)
{
	return _rtl865x_matched_layer4_aclChain(netifName, priority, flag, match);
}

rtl865x_AclRule_t* rtl865x_matched_layer2_aclChain(char *netifName,int32 priority, uint32 flag, rtl865x_AclRule_t *match)
{
	return _rtl865x_matched_layer2_aclChain(netifName, priority, flag, match);
}
rtl865x_acl_chain_t* rtl865x_find_aclChain_byPriority(rtl865x_acl_chain_t *head, int32 priority)
{
	rtl865x_acl_chain_t *entry;
	entry = head;
	while(entry)
	{
		if(entry->priority == priority)
			break;
		entry = entry->nextChain;
	}

	return entry;
}

/*
@func int32 | rtl865x_init_acl_chain |memory init for acl chains.
@rvalue SUCCESS | Success.
@comm
	this API must be called when system boot.
*/
int32 rtl865x_init_acl_chain(void)
{
	int32 i;
	rtl865x_acl_chain_t *entry;

	freeChainHead = NULL;

	TBL_MEM_ALLOC(entry, rtl865x_acl_chain_t, RTL865X_ACL_CHAIN_NUMBER);

	for(i = 0; i<RTL865X_ACL_CHAIN_NUMBER;i++)
	{
		memset(&entry[i],0,sizeof(rtl865x_acl_chain_t));
		entry[i].preChain= NULL;
		entry[i].nextChain= freeChainHead;

		entry[i].head = NULL;
		entry[i].tail = NULL;
		if(freeChainHead)
			freeChainHead->preChain = &entry[i];
		freeChainHead = &entry[i];
	}

	return SUCCESS;
}

/*
@func int32 | rtl865x_init_acl |memory init for acl rules.
@rvalue SUCCESS | Success.
@comm
	this API must be called when system boot.
*/
int32 rtl865x_init_acl(void)
{
	int32 i;
	rtl865x_AclRule_t *aclEntry;

	freeAclList.freeHead = NULL;

	TBL_MEM_ALLOC(aclEntry, rtl865x_AclRule_t, RTL865X_ACL_MAX_NUMBER);

	for(i = 0; i<RTL865X_ACL_MAX_NUMBER;i++)
	{
		memset(&aclEntry[i],0,sizeof(rtl865x_AclRule_t));
		aclEntry[i].pre = NULL;
		aclEntry[i].next = freeAclList.freeHead;
		if(freeAclList.freeHead)
			freeAclList.freeHead->pre = &aclEntry[i];
		freeAclList.freeHead = &aclEntry[i];

		freeAclList.totalCnt++;
		freeAclList.freeCnt++;
	}

	_rtl865x_confReservedAcl();
	/*init acl chains*/
	rtl865x_init_acl_chain();
	return SUCCESS;
}



/*
@func int32 | rtl865x_add_acl |add an ACL Rule to acl chain.
@parm rtl865x_AclRule_t* | rule | realtek ACL rule
@parm char* | netifName | network interface Name.
@parm int32 | priority | priority of this acl chain.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | ACL rule is NULL or netifName is NULL
@rvalue RTL_EENTRYNOTFOUND | acl chain with priority is not found
@rvalue FAILED | Failed
@comm
	ACL rule structure: please refer in header file.
*/
int32 rtl865x_add_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//printk("********%s(%d)*********,netif(%s),priority(%d)\n",__FUNCTION__,__LINE__,netifName,priority);
#if defined(CONFIG_RTK_VLAN_SUPPORT)
	if(rtl865x_acl_enable == 0)
		return retval;
#endif
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_add_acl(rule,netifName,priority);
	if(retval == RTL_ENOFREEBUFFER){	// acl entries is full.
		_rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);
	}
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH(flags);
	//printk("********%s(%d)*********retval(%d)\n",__FUNCTION__,__LINE__,retval);
	return retval;
}

/*
@func int32 | rtl865x_del_acl |del an ACL Rule from the acl chain.
@parm rtl865x_AclRule_t* | rule | realtek ACL rule
@parm char* | netifName | network interface Name.
@parm int32 | priority | priority of this acl chain.
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | ACL rule is NULL or netifName is NULL
@rvalue RTL_EENTRYNOTFOUND | acl chain with priority is not found
@rvalue FAILED | Failed
@comm
	ACL rule structure: please refer in header file.
*/
int32 rtl865x_del_acl(rtl865x_AclRule_t *rule, char *netifName,int32 priority)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_del_acl(rule,netifName,priority);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}

#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q)
int32 rtl865x_enable_acl(uint32 enable)
{
	if(enable)
	{
		rtl865x_acl_enable = 1;
		_rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);
	}
	else
	{
		rtl865x_acl_enable = 0;
		_rtl865x_setDefACLForAllNetif(RTL865X_ACLTBL_ALL_TO_CPU,RTL865X_ACLTBL_ALL_TO_CPU,RTL865X_ACLTBL_PERMIT_ALL,RTL865X_ACLTBL_PERMIT_ALL);
	}
	return SUCCESS;
}
#endif



int  rtl865x_add_pattern_acl_for_contentFilter(rtl865x_AclRule_t *rule,char *netifName)
{
	uint32 i;
	//### add by sen_liu 2011.5.4 to get wan prot
	rtl865x_netif_local_t	*netif;
	rtl865x_vlan_entry_t	*vlan;
	union
	{
		char pat[4];
		uint32 pattern;
	}u;

	if(rule == NULL)
		return FAILED;

        rtl865x_add_acl(rule, netifName, RTL865X_ACL_SYSTEM_USED);
 		netif = _rtl865x_getNetifByName(netifName);
        if(netif == NULL)
        	return FAILED;
        vlan = _rtl8651_getVlanTableEntry(netif->vid);
        if(vlan == NULL)
        	return FAILED;

        u.pat[0]='T';
        u.pat[1]='T';
        u.pat[2]='P';
        u.pat[3]='/';
        for(i=0;i<RTL8651_PORT_NUMBER;i++)
        {
			if (vlan->memberPortMask & 1<<i) {
				if(rtl8651_setAsicPortPatternMatch(i, u.pattern, 0xffffffff, 0x2)!=SUCCESS)
					return FAILED;
			}

        }

        return SUCCESS;
}

int  rtl865x_del_pattern_acl_for_contentFilter(rtl865x_AclRule_t *rule,char *netifName)
{
	union
	{
		char pat[4];
		uint32 pattern;
	}u;
	int32 i;

	rtl865x_netif_local_t	*netif;
	rtl865x_vlan_entry_t	*vlan;

	if(rule == NULL)
		return FAILED;

	rtl865x_del_acl(rule, netifName, RTL865X_ACL_SYSTEM_USED);

	netif = _rtl865x_getNetifByName(netifName);
        if(netif == NULL)
        	return FAILED;
        vlan = _rtl8651_getVlanTableEntry(netif->vid);
        if(vlan == NULL)
        	return FAILED;

	u.pat[0]='T';
	u.pat[1]='T';
	u.pat[2]='P';
	u.pat[3]='/';
	for(i=0;i<RTL8651_PORT_NUMBER;i++)
	{
		if (vlan->memberPortMask & 1<<i) {
				rtl8651_setAsicPortPatternMatch(i, 0, 0, 0x2);
			}
	}

	return SUCCESS;
}



#ifdef CONFIG_FAST_PATH_MODULE
EXPORT_SYMBOL(rtl865x_del_acl);
EXPORT_SYMBOL(rtl865x_add_acl);
#endif
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL
int32 rtl865x_attachMasterNetif(char *slave, char *master)
{
	return _rtl865x_attachMasterNetif(slave, master);
}

int32 rtl865x_detachMasterNetif(char *slave)
{
	return _rtl865x_detachMasterNetif(slave);
}

/*
@func int32 | rtl865x_addNetif |add network interface.
@parm rtl865x_netif_t* | netif | network interface
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | netif is NULL
@rvalue RTL_EENTRYALREADYEXIST | netif is already exist
@rvalue RTL_ENOFREEBUFFER | no netif to used
@rvalue FAILED | Failed
@comm
	rtl865x_netif_t: please refer in header file.
*/

int32 rtl865x_addNetif(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_addNetif(netif);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}
/*
@func int32 | rtl865x_delNetif |delete network interface.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue RTL_EREFERENCEDBYOTHER | netif is referenced by onter table entry
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_delNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delNetif(ifName);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH(flags);
	return retval;
}
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
int32 rtl865x_addVirtualNetif(rtl865x_netif_t *netif)
{

	rtl865x_netif_local_t *entry;
	int32 retval = FAILED;
	if(netif == NULL)
		return RTL_EINVALIDINPUT;

	/*duplicate entry....*/
	entry = _rtl865x_getSWNetifByName(netif->name);
	if(entry)
		return RTL_EENTRYALREADYEXIST;


	/*add new entry*/
	entry = &virtualNetIf;

	memset(entry, 0, sizeof(rtl865x_netif_local_t));

	entry->valid = 1;
	memcpy(entry->name,netif->name,MAX_IFNAMESIZE);
	entry->inAclEnd = entry->inAclStart = entry->outAclEnd = entry->outAclStart = RTL865X_ACLTBL_PERMIT_ALL; /*default permit...*/
	entry->refCnt = 1;


#ifdef 	CONFIG_RTL_LAYERED_DRIVER_ACL
	entry->chainListHead[RTL865X_ACL_INGRESS] = NULL;
	entry->chainListHead[RTL865X_ACL_EGRESS] = NULL;
#endif


#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	/*register 2 ingress chains: system/user*/
	retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_SYSTEM_USED, RTL865X_ACL_INGRESS);
	retval = rtl865x_regist_aclChain(netif->name, RTL865X_ACL_USER_USED, RTL865X_ACL_INGRESS);
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL

	return SUCCESS;
}

int32 rtl865x_delVirtualNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH(flags);
	retval = _rtl865x_delNetif(ifName);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH(flags);
	_rtl865x_confReservedAcl();
	return retval;
}
#endif
/*
@func int32 | rtl865x_referNetif |reference network interface entry.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
when other table entry refer network interface table entry, please call this API.
*/
int32 rtl865x_referNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH2(flags);
	retval = _rtl865x_referNetif(ifName);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH2(flags);
	return retval;
}

/*
@func int32 | rtl865x_deReferNetif |dereference network interface.
@parm char* | ifName | network interface name
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
this API should be called after rtl865x_referNetif.
*/
int32 rtl865x_deReferNetif(char *ifName)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH2(flags);
	retval = _rtl865x_deReferNetif(ifName);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH2(flags);
	return retval;
}

/*
@func int32 | rtl865x_setNetifVid |mapping network interface with vlan.
@parm char* | name | network interface name
@parm uint16 | vid | vlan id
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifVid(char *name, uint16 vid)
{
	int32 ret;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH(flags);
	ret = _rtl865x_setNetifVid(name,vid);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH(flags);
	return ret;
}

int32 rtl865x_setPortToNetif(char *name,uint32 port)
{
	int32 ret;
	rtl865x_netif_local_t *entry;
	entry = _rtl865x_getNetifByName(name);

	if(entry == NULL)
		return FAILED;

	ret = rtl8651_setPortToNetif(port, entry->asicIdx);
	return ret;
}

/*
@func int32 | rtl865x_setNetifType |config network interface type.
@parm char* | ifName | network interface name
@parm uint32 | ifType | interface type. IF_ETHER/IF_PPPOE/IF_PPTP/IF_L2TP allowed.
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifType(char *name, uint32 ifType)
{
	int32 ret;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH_HW(flags);
	ret = _rtl865x_setNetifType(name,ifType);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return ret;
}

EXPORT_SYMBOL(rtl865x_setNetifType);
/*
@func int32 | rtl865x_setNetifMac |config network interface Mac address.
@parm rtl865x_netif_t* | netif | netif name&MAC address
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifMac(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	SMP_LOCK_ETH_HW(flags);
	retval = _rtl865x_setNetifMac(netif);
	//rtl_up(&netif_sem);
	SMP_UNLOCK_ETH_HW(flags);
	return retval;
}

/*
@func int32 | rtl865x_setNetifMtu |config network interface MTU.
@parm rtl865x_netif_t* | netif | netif name & MTU
@rvalue SUCCESS | Success.
@rvalue RTL_EENTRYNOTFOUND | network interface is NOT found
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_setNetifMtu(rtl865x_netif_t *netif)
{
	int32 retval = FAILED;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	//SMP_LOCK_ETH(flags);
	local_irq_save(flags);
	retval = _rtl865x_setNetifMtu(netif);
	//rtl_up(&netif_sem);
	local_irq_restore(flags);
	//SMP_UNLOCK_ETH(flags);
	return retval;
}


/*
@func int32 | rtl865x_initNetifTable | initialize network interface table.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failed,system should be reboot.
*/
int32 rtl865x_initNetifTable(void)
{
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	int i;

	TBL_MEM_ALLOC(netifTbl, rtl865x_netif_local_t, NETIF_SW_NUMBER);
	memset(netifTbl,0,sizeof(rtl865x_netif_local_t)*NETIF_SW_NUMBER);
	for (i=0; i<NETIF_NUMBER; i++)
		Netif_Asic2Sw_Mapping[i] = -1;
#else
	TBL_MEM_ALLOC(netifTbl, rtl865x_netif_local_t, NETIF_NUMBER);
	memset(netifTbl,0,sizeof(rtl865x_netif_local_t)*NETIF_NUMBER);
#endif
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	/*init reserved acl in function init_acl...*/
#else
	_rtl865x_confReservedAcl();
#endif

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	memset(&virtualNetIf,0,sizeof(rtl865x_netif_local_t));
#endif
	rtl_get_drv_netifName_by_psName = NULL;

	return SUCCESS;
}

int32 rtl865x_config_callback_for_get_drv_netifName(int (*fun)(const char *psName,char *netifName))
{
	rtl_get_drv_netifName_by_psName = fun;
	return SUCCESS;
}

int32 rtl865x_get_drvNetifName_by_psName(const char *psName,char *netifName)
{
	if(strlen(psName) >= MAX_IFNAMESIZE)
		return FAILED;

	if(rtl_get_drv_netifName_by_psName)
		rtl_get_drv_netifName_by_psName(psName,netifName);
	else
	{
		memcpy(netifName,psName,MAX_IFNAMESIZE);
	}

	return SUCCESS;
}

/*
@func int32 | rtl865x_enableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_enableNetifRouting(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;

	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	if(netif ->enableRoute == 1)
		return SUCCESS;

#if (defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS) || defined(CONFIG_RTL_HARDWARE_MULTICAST)) && defined(CONFIG_OPENWRT_SDK)
	netif->enableRoute = (rtl8651_getAsicOperationLayer()>2)? 1 :0;
#else
	netif->enableRoute = 1;
#endif
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}

/*
@func int32 | rtl865x_disableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_disableNetifRouting(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;

	if(netif == NULL)
		return RTL_EINVALIDINPUT;

	if(netif ->enableRoute == 0)
		return SUCCESS;

	netif->enableRoute = 0;
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
int32 rtl865x_enableNetifRoutingV6(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;

	if(netif == NULL)
		return RTL_EINVALIDINPUT;

	if(netif ->enableRouteV6 == 1)
		return SUCCESS;

	#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT) || defined(CONFIG_RTL_HARDWARE_MULTICAST)
	#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
	netif->enableRouteV6 = rtk_hwL3v6_enable?1:0;
	#endif
	#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
    	netif->enableRouteV6 = (rtl8651_getAsicOperationLayer()>2)? 1 :0;
	#endif
	#else
	netif->enableRouteV6 = 1;
	#endif
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}

int32 rtl865x_disableNetifRoutingV6(rtl865x_netif_local_t *netif)
{
	int32 retval = FAILED;

	if(netif == NULL)
		return RTL_EINVALIDINPUT;
	
	#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
	netif->enableRouteV6 = rtk_hwL3v6_enable?1:0;
	#else
	if(netif ->enableRouteV6 == 0)
		return SUCCESS;

	netif->enableRouteV6 = 0;
	#endif
	retval = _rtl865x_setAsicNetif(netif);
	return retval;
}


#endif
/*
@func int32 | rtl865x_disableNetifRouting |config network interface operation layer.
@parm rtl865x_netif_local_t* | netif | netif & enableRoute
@rvalue SUCCESS | Success.
@rvalue RTL_EINVALIDINPUT | input is invalid
@rvalue FAILED | Failed
@comm
*/
int32 rtl865x_reinitNetifTable(void)
{
	int32 i;
	unsigned long flags=0;
	//rtl_down_interruptible(&netif_sem);
	/*Fix jwj: If here use SMP_LOCK_ETH(flags), rtl865x_reChangeOpMode will cause dead-lock.*/
	//SMP_LOCK_ETH(flags);
	local_irq_save(flags);

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		if(netifTbl[i].valid)
		{
			_rtl865x_delNetif(netifTbl[i].name);
		}
	}
	//memset(netifTbl,0,sizeof(rtl865x_netif_local_t)*NETIF_NUMBER);

#if defined (CONFIG_RTL_LOCAL_PUBLIC)
	if(virtualNetIf.valid)
	{
		_rtl865x_delNetif(virtualNetIf.name);
	}
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for (i=0; i<NETIF_NUMBER; i++)
		Netif_Asic2Sw_Mapping[i] = -1;
#endif

	//rtl_up(&netif_sem);
	//SMP_UNLOCK_ETH(flags);
	local_irq_restore(flags);
	return SUCCESS;
}


#if defined (CONFIG_RTL_HARDWARE_MULTICAST) ||defined(CONFIG_SWCONFIG_SUPPORT_HW_NAT)
uint32 rtl865x_getExternalPortMask(void)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;
	uint32 externalPortMask=0;

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		netif = &netifTbl[i];
		if((netif->valid == 1) && (netif->is_wan==1))
		{
			externalPortMask|=rtl865x_getVlanPortMask(netif->vid);
		}
	}

	return externalPortMask;
}

int32 rtl865x_getNetifVid(char *name, uint32 *vid)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL)
	{
		return FAILED;
	}

	entry = _rtl865x_getNetifByName(name);

	if(entry == NULL)
	{
		return FAILED;
	}

	*vid=(uint32)(entry->vid);
	return SUCCESS;

}

int32 rtl865x_getNetifType(char *name,uint32 *type)
{
	rtl865x_netif_local_t *entry;

	if(name == NULL)
	{
		return FAILED;
	}

	if(type==NULL)
	{
		return FAILED;
	}

	entry = _rtl865x_getNetifByName(name);

	if(entry == NULL)
	{
		return FAILED;
	}

	*type=(uint32)(entry->if_type);
	return SUCCESS;

}
#endif


#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
#if RTL_LAYERED_DRIVER_DEBUG
int32 rtl865x_acl_test(int32 testNo)
{
	int32 retval = 0;
	printk("testNo = %d\n",testNo);
	switch(testNo)
	{
		/*add a chain to br0, and delete it...*/
		case 0:
		{
			rtl865x_acl_chain_t *chain = NULL;
			rtl865x_netif_local_t *netif = NULL;
			int32 cnt = 0;
			retval = _rtl865x_regist_aclChain(RTL_DRV_LAN_NETIF_NAME, -500, RTL865X_ACL_INGRESS);
			netif = _rtl865x_getNetifByName(RTL_DRV_LAN_NETIF_NAME);
			if(netif == NULL)
			{
				printk("netif is NULL!!!!\n");
				return FAILED;
			}
			chain = netif->chainListHead[RTL865X_ACL_INGRESS];

			printk("chains of netif(%s):\n",netif->name);
			while(chain)
			{
				printk("chain %d:\n",cnt);
				printk("chain:priority(%d),ruleCnt(%d)\n",chain->priority,chain->ruleCnt);
				cnt++;
				chain = chain->nextChain;
			}
			printk("====================================\n");

			retval = _rtl865x_unRegist_aclChain(RTL_DRV_LAN_NETIF_NAME, -500, RTL865X_ACL_INGRESS);

			chain = netif->chainListHead[RTL865X_ACL_INGRESS];
			cnt = 0;
			printk("=============after unregist the chain================\n");
			printk("chains of netif(%s):\n",netif->name);
			while(chain)
			{
				printk("chain %d:\n",cnt);
				printk("chain:priority(%d),ruleCnt(%d)\n",chain->priority,chain->ruleCnt);
				cnt++;
				chain = chain->nextChain;
			}
		}
		break;

		/*add acls and delete acl....*/
		case 1:
		{
			rtl865x_acl_chain_t *chain = NULL;
			rtl865x_netif_local_t *netif = NULL;
			rtl865x_AclRule_t rule,rule1,rule2;
			retval = _rtl865x_regist_aclChain(RTL_DRV_LAN_NETIF_NAME, -500, RTL865X_ACL_INGRESS);

			netif = _rtl865x_getNetifByName(RTL_DRV_LAN_NETIF_NAME);
			chain = netif->chainListHead[RTL865X_ACL_INGRESS];

			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();


			memset(&rule,0,sizeof(rtl865x_AclRule_t));
			rule.pktOpApp_ = RTL865X_ACL_L2_AND_L3;
			rule.actionType_ = RTL865X_ACL_DROP;

			printk("============add 1st acl===========\n");
			rtl865x_add_acl(&rule, RTL_DRV_LAN_NETIF_NAME, -500);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

			printk("del the 1st acl\n");
			rtl865x_del_acl(&rule, RTL_DRV_LAN_NETIF_NAME, -500);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

			printk("add 2 rule, delete the tail...\n");
			rtl865x_add_acl(&rule,RTL_DRV_LAN_NETIF_NAME, -500);
			memset(&rule1,0,sizeof(rtl865x_AclRule_t));

			rule1.pktOpApp_ = RTL865X_ACL_L3_AND_L4;
			rule1.ruleType_ = RTL865X_ACL_IP;
			rule1.actionType_ = RTL865X_ACL_DROP;

			rtl865x_add_acl(&rule1,RTL_DRV_LAN_NETIF_NAME,-500);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

			printk("del the rule1...\n");
			rtl865x_del_acl(&rule1, RTL_DRV_LAN_NETIF_NAME, -500);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

			rtl865x_add_acl(&rule1,RTL_DRV_LAN_NETIF_NAME,-500);
			memset(&rule2,0,sizeof(rtl865x_AclRule_t));

			rule2.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule2.ruleType_ = RTL865X_ACL_ICMP;
			rule2.actionType_ = RTL865X_ACL_TOCPU;
			rtl865x_add_acl(&rule2,RTL_DRV_LAN_NETIF_NAME,-500);

			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

			printk("del the rule1...\n");
			rtl865x_del_acl(&rule1, RTL_DRV_LAN_NETIF_NAME, -500);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

			printk("unregist the chain(-500)\n");
			_rtl865x_unRegist_aclChain(RTL_DRV_LAN_NETIF_NAME,-500,RTL865X_ACL_INGRESS);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();


			printk("flush all chain of br0");
			_rtl865x_unRegister_all_aclChain(RTL_DRV_LAN_NETIF_NAME);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

		}
		break;

		/*add acls....*/
		case 2:
		{
			rtl865x_acl_chain_t *chain = NULL;
			rtl865x_netif_local_t *netif = NULL;
			rtl865x_AclRule_t rule;
			retval = _rtl865x_regist_aclChain(RTL_DRV_LAN_NETIF_NAME, -500, RTL865X_ACL_INGRESS);

			netif = _rtl865x_getNetifByName(RTL_DRV_LAN_NETIF_NAME);
			chain = netif->chainListHead[RTL865X_ACL_INGRESS];

			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();


			memset(&rule,0,sizeof(rtl865x_AclRule_t));
			rule.pktOpApp_ = RTL865X_ACL_L2_AND_L3;
			rule.actionType_ = RTL865X_ACL_DROP;

			printk("============add 1st acl===========\n");
			rtl865x_add_acl(&rule, RTL_DRV_LAN_NETIF_NAME, -500);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

		}
		break;

		/*add acls....*/
		case 3:
		{
			rtl865x_acl_chain_t *chain = NULL;
			rtl865x_netif_local_t *netif = NULL;
			rtl865x_AclRule_t rule1;

			netif = _rtl865x_getNetifByName(RTL_DRV_LAN_NETIF_NAME);
			chain = netif->chainListHead[RTL865X_ACL_INGRESS];

			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();


			memset(&rule1,0,sizeof(rtl865x_AclRule_t));
			rule1.pktOpApp_ = RTL865X_ACL_L3_AND_L4;
			rule1.actionType_ = RTL865X_ACL_DROP;
			rule1.ruleType_ = RTL865X_ACL_IP;


			rtl865x_add_acl(&rule1, RTL_DRV_LAN_NETIF_NAME, -500);
			printk("now the information of netif(%s) is:\n\n",netif->name);
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();

		}
		break;

		case 4:
		{
			rtl865x_netif_local_t *netif = NULL;
			netif = _rtl865x_getNetifByName(RTL_DRV_LAN_NETIF_NAME);
			printk("unregist the chain(-500)\n");
			_rtl865x_unRegist_aclChain(RTL_DRV_LAN_NETIF_NAME,-500,RTL865X_ACL_INGRESS);
			printk("now the information of netif(br0) is:\n\n");
			_rtl865x_print_allChain_allAcl(netif);
			_rtl865x_print_freeChainNum();
		}
		break;

		case 5:
		{
			rtl865x_AclRule_t rule;
			union
			{
				char pat[4];
				uint32 pattern;
			}u;
			int32 i;


			printk("for url filter test....");
			memset(&rule,0,sizeof(rtl865x_AclRule_t));
			rule.actionType_ = RTL865X_ACL_TOCPU;
			rule.ruleType_ = RTL865X_ACL_IP;
			rule.ipHttpFilter_=rule.ipHttpFilterM_=1;
 			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rtl865x_add_acl(&rule, RTL_DRV_LAN_NETIF_NAME, -10000);

			memset(&rule,0,sizeof(rtl865x_AclRule_t));
			rule.actionType_ = RTL865X_ACL_PERMIT;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rtl865x_add_acl(&rule, RTL_DRV_LAN_NETIF_NAME, -10000);

			u.pat[0]='T';
			u.pat[1]='T';
			u.pat[2]='P';
			u.pat[3]='/';
			for(i=0;i<RTL8651_PORT_NUMBER;i++)
			{
				if(rtl8651_setAsicPortPatternMatch(i, u.pattern, 0xffffffff, 0x2 /* fwd to CPU */)!=SUCCESS)
					return FAILED;
			}

		}
		break;

		case 6:
		{
			/*print software netif table information*/
			int32 i;
			for(i = 0; i < NETIF_NUMBER; i++)
			{
				printk("i(%d),netifTbl(0x%p),name(%s)\n",i,&netifTbl[i],netifTbl[i].name);
			}
		}
		break;


	}
	return retval;
}

#endif
#endif

#if 0
int rtl865x_show_all_netif(void)
{
	int32 i;
	rtl865x_netif_local_t *netif = NULL;

	for(i = 0; i < NETIF_NUMBER; i++)
	{
		if(netifTbl[i].valid == 1 )
		{
			netif = &netifTbl[i];
			printk("idx(%d),valid(%d),name(%s),vid(%d),is_slave(%d),type(%d)\n",i,netif->valid,netif->name,netif->vid,netif->is_slave,netif->if_type);
		}
	}

	return SUCCESS;
}
#endif

#if 0 //defined (CONFIG_RTL_LOCAL_PUBLIC)
int32 rtl865x_getNetifFid(char *name,  uint16 *fid)
{
	rtl865x_netif_local_t *entry;
	rtl865x_vlan_entry_t *vlan;

	if(name == NULL)
	{
		return FAILED;
	}

	entry = _rtl865x_getNetifByName(name);

	if(entry == NULL)
	{
		return FAILED;
	}

	vlan = _rtl8651_getVlanTableEntry(entry->vid);
	*fid = vlan->fid;
	return SUCCESS;

}
#endif
extern int rtk_vlan_support_enable;
int32 rtl865x_reConfigDefaultAcl(char *ifName)
{
	rtl865x_AclRule_t	rule;
	int ret=FAILED;

	unsigned long flags=0;
	SMP_LOCK_ETH(flags);

#if defined (CONFIG_RTK_VLAN_SUPPORT)
		if(rtk_vlan_support_enable==0)
		{		
			/*del old default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_del_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);	

			/*add new default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_add_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);
		}
		else
		{
			/*del old default to cpu acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_TOCPU;
			ret=_rtl865x_del_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);

			/*add new default to cpu acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_TOCPU;
			ret=_rtl865x_add_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);
		}
#else
		{
			/*del old default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_del_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);

			/*add new default permit acl*/
			bzero((void*)&rule,sizeof(rtl865x_AclRule_t));
			rule.ruleType_ = RTL865X_ACL_MAC;
			rule.pktOpApp_ = RTL865X_ACL_ALL_LAYER;
			rule.actionType_ = RTL865X_ACL_PERMIT;
			ret=_rtl865x_add_acl(&rule, ifName, RTL865X_ACL_SYSTEM_USED);
		}
#endif
		SMP_UNLOCK_ETH(flags);

		return SUCCESS;
}

//#if defined(CONFIG_RTL_HW_QOS_BRIDGE_FWD_SUPPORT)
int rtl865x_isWanNetifByIdx(int netifIdx)
{
	if (netifIdx <0 || netifIdx >= NETIF_NUMBER)
		return -1;
	
	if (netifTbl[netifIdx].valid) {
		if (strncmp(netifTbl[netifIdx].name, RTL_DRV_LAN_NETIF_NAME, strlen(RTL_DRV_LAN_NETIF_NAME))==0)
			return 0;
		else if (strncmp(netifTbl[netifIdx].name, RTL_DRV_WAN0_NETIF_NAME, strlen(RTL_DRV_WAN0_NETIF_NAME))==0)
			return 1;
	}

	return -1;
}
//#endif

#ifdef CONFIG_RTL_PROC_NEW
int32 sw_netif_read(struct seq_file *s, void *v)
{
	uint8 *mac;
	int32 i;

	seq_printf(s, "%s\n", "SW Network Interface Table:");
#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
#else
	for(i = 0; i < NETIF_NUMBER; i++)
#endif
	{
		
		rtl865x_netif_local_t *intf;
		
		intf = & netifTbl[i];
		
		if ( intf->valid )
		{
			mac = (uint8 *)&intf->macAddr.octet[0];
			seq_printf(s,"[%d]netif:%s	VID[%d] %x:%x:%x:%x:%x:%x",
				i, intf->name,intf->vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			seq_printf(s,"	Routing %s	\n",
				intf->enableRoute==TRUE? "enabled": "disabled" );
			seq_printf(s,"is slave %d	",intf->is_slave);
			if(intf->is_slave){
				seq_printf(s,"master netif:%s",(intf->master->name));
			}	
			seq_printf(s,"\n");
			#if 0
			seq_printf(s,"      ingress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.inAclStart )
			{
				if ( intf.inAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					seq_printf(s,"permit all,");
				if ( intf.inAclStart == RTL8651_ACLTBL_ALL_TO_CPU )
					seq_printf(s,"all to cpu,");
				if ( intf.inAclStart == RTL8651_ACLTBL_DROP_ALL )
					seq_printf(s,"drop all,");
			}
			else
				seq_printf(s,"ACL %d-%d, ", intf.inAclStart, intf.inAclEnd);

			seq_printf(s,"  egress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.outAclStart )
			{
				if ( intf.outAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					seq_printf(s,"permit all,");
				if ( intf.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU )
					seq_printf(s,"all to cpu,");
				if ( intf.outAclStart==RTL8651_ACLTBL_DROP_ALL )
					seq_printf(s,"drop all,");
			}
			else
				seq_printf(s,"ACL %d-%d, ", intf.outAclStart, intf.outAclEnd);
			#endif
			seq_printf(s, "\n   VID:%d,MTU %d Bytes\n",  intf->vid,intf->mtu);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) 
			seq_printf(s, "      enableRouteV6:%d,    mtuv6:%d\n", intf->enableRouteV6, intf->mtuV6);
#endif

			
			seq_printf(s, ")\n\n");
		}

	}
	return 0;
}
#else
int32 sw_netif_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
	int len;
	uint8 *mac;
	int32 i;

	len = sprintf(page, "%s\n", "sw Network Interface Table:");
	for ( i = 0; i < RTL865XC_NETIFTBL_SIZE; i++ )
	{
		rtl865x_netif_local_t *intf;
		
		intf = & netifTbl[i];

		if (intf->valid)
		{
			
			mac = (uint8 *)&intf->macAddr.octet[0];
			len += sprintf(page+len,"[%d]  %s	VID[%d] %x:%x:%x:%x:%x:%x",
				i, intf->name,intf->vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			len += sprintf(page+len,"  Routing %s \n",
				intf->enableRoute==TRUE? "enabled": "disabled" );
			
			len += sprintf(page+len,"is slave %d		",intf->is_slave);
			if(intf->is_slave){
				len += sprintf(page+len,"master netif:%s",intf->master->name);
			}	
			len += sprintf(page+len," \n");
			#if 0
			len += sprintf(page+len,"      ingress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.inAclStart )
			{
				if ( intf.inAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					len += sprintf(page+len,"permit all,");
				if ( intf.inAclStart == RTL8651_ACLTBL_ALL_TO_CPU )
					len += sprintf(page+len,"all to cpu,");
				if ( intf.inAclStart == RTL8651_ACLTBL_DROP_ALL )
					len += sprintf(page+len,"drop all,");
			}
			else
				len += sprintf(page+len,"ACL %d-%d, ", intf.inAclStart, intf.inAclEnd);

			len += sprintf(page+len,"  egress ");

			if ( RTL8651_ACLTBL_DROP_ALL <= intf.outAclStart )
			{
				if ( intf.outAclStart == RTL8651_ACLTBL_PERMIT_ALL )
					len += sprintf(page+len,"permit all,");
				if ( intf.outAclStart==RTL8651_ACLTBL_ALL_TO_CPU )
					len += sprintf(page+len,"all to cpu,");
				if ( intf.outAclStart==RTL8651_ACLTBL_DROP_ALL )
					len += sprintf(page+len,"drop all,");
			}
			else
				len += sprintf(page+len,"ACL %d-%d, ", intf.outAclStart, intf.outAclEnd);
			#endif
			len += sprintf(page+len, "\n      %d MAC Addresses, MTU %d Bytes\n", intf->macAddrNumber, intf->mtu);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F) 
			len += sprintf(page+len, "      enableRouteV6:%d,    mtuv6:%d\n", intf->enableRouteV6, intf->mtuV6);
#endif

			
			len += sprintf(page+len, ")\n\n");
		}

	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count)
		len = count;
	if (len<0)
	  	len = 0;

	return len;
}
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
void rtl865x_get_master_netif_by_vid(int vid, char* netif_name)
{
	int i;
	
	if(!netif_name)
		return;	
	
	strcpy(netif_name,"");
	
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	{	
		if ((netifTbl[i].valid==1) && (netifTbl[i].is_slave==0) && (netifTbl[i].vid==vid)) {
			strcpy(netif_name, netifTbl[i].name);
			return;		
		}
	}
	return;
}

int32 rtl865x_netif_exist(char *ifName)
{
	if (_rtl865x_getSWNetifByName(ifName) != NULL)
		return 1;
	else
		return 0;
}

/* Be careful,.... NAPT and pure routing are WAN  (If according to netif->is_wan, pure routing netif is LAN) */
int32 rtl865x_netif_is_wan(char *ifName)
{
	rtl865x_netif_local_t* netif;
	netif = _rtl865x_getNetifByName(ifName);
	
	//if( netif && netif->is_wan == 1)
	if (netif && strcmp(netif->name, RTL_DRV_LAN_NETIF_NAME))
		return 1;
	else
		return 0;
}

int32 rtl865x_netif_is_master(char *ifName)
{
	rtl865x_netif_local_t* netif;
	netif = _rtl865x_getNetifByName(ifName);
	
	if (netif && netif->is_slave == 0)
		return 1;
	else
		return 0;
}


#endif

#if defined(CONFIG_RTL_HARDWARE_IPV6_SUPPORT)
void rtl_set_netif_v6_route(int enable)
{
	int i = 0;
	
	#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
	for(i = 0; i < NETIF_SW_NUMBER; i++)
	#else
	for(i = 0; i < NETIF_NUMBER; i++)
	#endif
	{	
		if ((netifTbl[i].valid==1) && (netifTbl[i].is_slave==0)) {
			if (enable){
				rtl865x_enableNetifRoutingV6((rtl865x_netif_local_t *)&netifTbl[i]);
			}
			else
			{
				rtl865x_disableNetifRoutingV6((rtl865x_netif_local_t *)&netifTbl[i]);
			}
		}
	}
	
	return; 	
}
#endif

#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS) || defined(CONFIG_RTL_HARDWARE_MULTICAST)
int rtl_enable_netif_routing(void)
{
	int i;
	unsigned long flags = 0;

	SMP_LOCK_ETH(flags);
	for (i=0; i<NETIF_NUMBER; i++)
	{
		if (netifTbl[i].valid == 1) {
			rtl865x_enableNetifRouting(&netifTbl[i]);
#if defined(CONFIG_RTL_HARDWARE_MULTICAST) && (defined(CONFIG_RTL_8198C)||defined(CONFIG_RTL_8197F))
			rtl865x_enableNetifRoutingV6(&netifTbl[i]);
#endif
		}
	}
	SMP_UNLOCK_ETH(flags);

	return SUCCESS;
}
#endif

#if defined(CONFIG_SWCONFIG_SUPPORT_HW_NAT)
uint8* rtl_get_netif_gmac(char *name)
{
	rtl865x_netif_local_t *entry;

	if (name == NULL)
		return NULL;

	entry = _rtl865x_getNetifByName(name);

	if (entry == NULL)
		return NULL;

	return entry->macAddr.octet;
}
#endif

