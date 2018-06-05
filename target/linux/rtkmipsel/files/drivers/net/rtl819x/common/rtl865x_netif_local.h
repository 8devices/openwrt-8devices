/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : network interface driver header file
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

#ifndef RTL865X_NETIF_LOCAL_H
#define RTL865X_NETIF_LOCAL_H

#if 0//!defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define REDUCE_MEMORY_SIZE_FOR_16M
#endif

#if defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define RTL865X_ACL_CHAIN_NUMBER	32
#else
#define RTL865X_ACL_CHAIN_NUMBER	16
#endif
#define RTL865X_ACL_SYSTEM_USED	-10000

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
typedef struct _rtl865x_acl_chain_s
{
	int32 ruleCnt;
	int32 priority; /*chain priosity: the minimum value is the highest proirity*/
	rtl865x_AclRule_t *head,*tail;
	struct _rtl865x_acl_chain_s *preChain,*nextChain;
}rtl865x_acl_chain_t;
#endif

/*the following fields are invalid when the interface is slave interface:
* inAclStart, inAclEnd, outAclStart, outAclEnd,asicIdx,chainListHead
*/
typedef struct rtl865x_netif_local_s
{
	uint16 	vid; /*netif->vid*/
	uint16 	mtu; /*netif's MTU*/	
	uint16 	macAddrNumber; /*how many continuous mac is attached*/
	uint16 	inAclStart, inAclEnd, outAclStart, outAclEnd; /*acl index*/
	uint16 	enableRoute; /*enable route*/
	uint32 	valid:1,	/*valid?*/
		if_type:5, /*interface type, IF_ETHER, IF_PPPOE*/
		refCnt:5, /*referenc count by other table entry*/			
		asicIdx:3, /*asic index, total 8 entrys in asic*/
		is_wan:1, /*this interface is wan?*/
		is_defaultWan:1, /*if there is multiple wan interface, which interface is default wan*/
		dmz:1,	/*dmz interface?*/
		is_slave:1; /*is slave interface*/
	
	ether_addr_t macAddr;
	uint8	name[MAX_IFNAMESIZE];
#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
	rtl865x_acl_chain_t  *chainListHead[2]; /*0: ingress acl chain, 1: egress acl chain*/
#endif //CONFIG_RTL_LAYERED_DRIVER_ACL
	struct rtl865x_netif_local_s *master; /*point master interface when this interface is slave interface*/

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
	uint16 	enableRouteV6; /*enable ipv6 route*/
	uint16 	mtuV6;         /*netif's ipv6 MTU*/
#endif
}rtl865x_netif_local_t;

#define	RTL_ACL_INGRESS	0
#define	RTL_ACL_EGRESS	1

#ifdef CONFIG_RTL_LAYERED_DRIVER_ACL
typedef struct rtl865x_aclBuf_s
{
	int16 totalCnt;
	int16 freeCnt;
	rtl865x_AclRule_t *freeHead;
}rtl865x_aclBuf_t;
#endif

int32 rtl865x_enableNetifRouting(rtl865x_netif_local_t *netif);
int32 rtl865x_disableNetifRouting(rtl865x_netif_local_t *netif);

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
int32 rtl865x_enableNetifRoutingV6(rtl865x_netif_local_t *netif);
int32 rtl865x_disableNetifRoutingV6(rtl865x_netif_local_t *netif);
#endif

rtl865x_netif_local_t *_rtl865x_getNetifByName(char *name);
rtl865x_netif_local_t *_rtl865x_getSWNetifByName(char * name);
rtl865x_netif_local_t *_rtl865x_getDefaultWanNetif(void);
int32 _rtl865x_setDefaultWanNetif(char *name);
int32 _rtl865x_clearDefaultWanNetif(char * name);
int32 _rtl865x_getNetifIdxByVid(uint16 vid);
int32 _rtl865x_getNetifIdxByName(uint8 *name);
int32 _rtl865x_getNetifIdxByNameExt(uint8 *name);
int32 rtl865x_get_drvNetifName_by_psName(const char *psName,char *netifName);

#endif
