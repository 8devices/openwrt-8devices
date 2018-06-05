#ifndef RTL865X_ARP_H
#define RTL865X_ARP_H

#include <net/rtl/rtl865x_arp_api.h>
/*for driver initialization*/
int32 rtl865x_arp_init(void);
int32 rtl865x_arp_reinit(void);

/*for routing module usage*/
int32 rtl865x_arp_tbl_alloc(rtl865x_route_t *route);
int32 rtl865x_arp_tbl_free(rtl865x_route_t *route);
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
int32 rtl865x_isArpExisted(ipaddr_t ip);
#endif
#if defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
int32 rtl865x_get_ps_arpMapping(ipaddr_t ip,rtl865x_arpMapping_entry_t *entry);
#endif

#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#ifdef CONFIG_RTL_HW_6RD_SUPPORT
int32 rtl8198c_6rd_Ipv6ArpMapping(ipaddr_t ip, rtl865x_arpMapping_entry_t * arp_mapping);
#endif
#endif

#endif

