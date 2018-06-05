/*
 *      Headler file of Realtek STP
 *
 *      $Id: rtk_stp.h,v 1.3 2009/06/01 07:00:27 davidhsu Exp $
 */

#ifndef _RTK_STP_H
#define _RTK_STP_H
#include "rtl_types.h"
#include <linux/netdevice.h>

#ifdef CONFIG_RTK_MESH
#define MAX_RE865X_STP_PORT	7
#define WLAN_MESH_PSEUDO_IF_INDEX	6
#define WLAN_MESH_IF_NAME		"wlan0-msh0"
#else
#define MAX_RE865X_STP_PORT	6
#endif

#define MAX_RE865X_ETH_STP_PORT	5
#define WLAN_PSEUDO_IF_INDEX	5
#define WLAN_IF_NAME			"wlan0"

#define NO_MAPPING				-1

#define PORT_NAME_PREFIX	    "port"

#define ETH_NAME_PREFIX	    "eth"

#define MAX_RTL_STP_PORT_WH	5

#define RTL8651_PORTSTA_DISABLED		0x00
#define RTL8651_PORTSTA_BLOCKING		0x01
#define RTL8651_PORTSTA_LISTENING		0x02
#define RTL8651_PORTSTA_LEARNING		0x03
#define RTL8651_PORTSTA_FORWARDING	0x04

extern int8 STP_PortDev_Mapping[MAX_RE865X_STP_PORT];

int32 rtl865x_setSpanningEnable(int8 spanningTreeEnabled);
int32 rtl865x_setMulticastSpanningTreePortState(uint32 port, uint32 portState);
int32 rtl865x_setSpanningTreePortState(uint32 port, uint32 portState);

int rtl_get_phy_port_by_dev(struct net_device *dev);

#endif // _RTK_STP_H
