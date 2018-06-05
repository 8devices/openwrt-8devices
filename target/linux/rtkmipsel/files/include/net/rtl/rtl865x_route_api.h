
/*
 * * Copyright c                  Realtek Semiconductor Corporation, 2008  
 * * All rights reserved.
 * * 
 * * Program : route table driver
 * * Abstract : 
 * * Author : hyking (hyking_liu@realsil.com.cn)  
 * */
#ifndef RTL865X_ROUTE_API_H
#define RTL865X_ROUTE_API_H

#define RT_ASIC_ENTRY_NUM 8
#define RT_DRV_ENTRY_NUM        8

#define RT_MAX_ARP_SIZE 256

int32 rtl865x_addRoute(ipaddr_t ipAddr, ipaddr_t ipMask, ipaddr_t nextHop, int8 * ifName,ipaddr_t srcIp);
int32 rtl865x_delRoute(ipaddr_t ipAddr, ipaddr_t ipMask);
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int rtl865x_checkL3L2Valid(ipaddr_t ipAddr );
#endif

#endif
