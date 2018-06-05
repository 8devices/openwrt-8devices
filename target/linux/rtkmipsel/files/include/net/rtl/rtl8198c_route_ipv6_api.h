/*
 * * Copyright c                  Realtek Semiconductor Corporation, 2013  
 * * All rights reserved.
 * * 
 * * Program : route table driver
 * * Abstract : 
 * * Author : Jia Wenjian (wenjain_jai@realsil.com.cn)  
 * */
#ifndef RTL8198C_ROUTE_IPV6_API_H
#define RTL8198C_ROUTE_IPV6_API_H

#define IPV6_RT_ASIC_ENTRY_NUM 	8
#define IPV6_RT_DRV_ENTRY_NUM        8

int32 rtl8198c_addIpv6Route(inv6_addr_t ipAddr, int fc_dst_len, inv6_addr_t nextHop, int8 * ifName);
int32 rtl8198c_delIpv6Route(inv6_addr_t ipAddr, int fc_dst_len);

#endif

