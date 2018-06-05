/*
 * Copyright c                  Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Program : ppoe table driver
 * Abstract :
 * Author : hyking (hyking_liu@realsil.com.cn)
 */
#ifndef RTL865X_PPP_LOCAL_H
#define RTL865X_PPP_LOCAL_H

typedef struct rtl865x_ppp_s
{
        rtl865x_netif_local_t *netif;   /*dest network interface*/
        ether_addr_t server_mac;        /*server mac address*/
        uint32 sessionId;
        uint16 valid:1,
                type:5; /*PPPOE, PPTP, L2TP */

        uint16 refCnt;
} rtl865x_ppp_t;

int32 rtl865x_getPppIdx(rtl865x_ppp_t *entry, int32 *idx);
int32 rtl865x_getPppBySessionId(uint32 sessionId,rtl865x_ppp_t *entry);
rtl865x_ppp_t* rtl865x_getPppByNetifName(char *name);

int32 rtl865x_initPppTable(void);
int32 rtl865x_reinitPppTable(void);
int32 rtl865x_referPpp(uint32 sessionId);
int32 rtl865x_deReferPpp(uint32 sessionId);


#endif
