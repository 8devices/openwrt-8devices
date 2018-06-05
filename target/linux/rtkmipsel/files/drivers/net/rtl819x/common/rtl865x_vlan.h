/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : Vlan driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#ifndef	RTL865X_VLAN_H
#define	RTL865X_VLAN_H

//#include "rtl_types.h"

#if 0//!defined(REDUCE_MEMORY_SIZE_FOR_16M)
#define REDUCE_MEMORY_SIZE_FOR_16M
#endif


#define VLAN_NUMBER			4096


#if defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_819XD)
#define VLANTBL_SIZE				16
#else
#define VLANTBL_SIZE				4096
#endif

#define RTL865X_FDB_NUMBER	4

#define RTL865X_PPTP_HWACC_PORTMASK		0x80
#define RTL865X_PPTP_HWACC_VLANID		10


typedef struct rtl865x_vlan_entry_s {
	uint32	memberPortMask; /*extension ports [rtl8651_totalExtPortNum-1:0] are located at bits [RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum-1:RTL8651_PORT_NUMBER]*/
	uint32	untagPortMask; /*extension ports [rtl8651_totalExtPortNum-1:0] are located at bits [RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum-1:RTL8651_PORT_NUMBER]*/
	uint32	valid:1,
			fid:2,	/*there are 4 fdbs in 865x*/
			vid:12,
			refCnt:5; /*reference count: this vlan entry is referenced by networkInterface...*/
#if defined(CONFIG_RTL_8197F)
	uint32	hp:3;
#endif

}rtl865x_vlan_entry_t;

int32 rtl865x_initVlanTable(void);
int32 rtl865x_reinitVlantable(void);
int32 rtl865x_addVlan(uint16 vid);
int32 rtl865x_delVlan(uint16 vid);
int32 rtl865x_addVlanPortMember(uint16 vid, uint32 portMask);
int32 rtl865x_delVlanPortMember(uint16 vid,uint32 portMask);

#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
uint32 rtl865x_getVlanPortMask(uint32 vid); 
#endif

int32 rtl865x_setVlanPortTag(uint16 vid,uint32 portMask,uint8 tag);
int32 rtl865x_setVlanFilterDatabase(uint16 vid, uint32 fid);
int32 rtl865x_getVlanFilterDatabaseId(uint16 vid, uint32 *fid);
rtl865x_vlan_entry_t *_rtl8651_getVlanTableEntry(uint16 vid);
int32 rtl865x_referVlan(uint16 vid);
int32 rtl865x_deReferVlan(uint16 vid);
#if defined(CONFIG_RTL_8197F)
int32 rtl865x_setVlanHp(uint16 vid, uint8 hp);
int32 rtl865x_getVlanHp(uint16 vid, uint8 *hp);
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
int32 rtl865x_modVlanPortMember(uint16 vid, uint32 portMask, uint32 untagset);
#endif

#endif
	
