#ifndef RTL865X_ARP_API_H
#define RTL865X_ARP_API_H

#define RTL865X_ARPTBL_SIZE 512

typedef struct rtl865x_arpMapping_entry_s
{
        ipaddr_t ip;
        ether_addr_t mac;
#if defined(CONFIG_RTL_HW_NAPT_REFINE_KERNEL_HOOKS)
	 uint16	exist_in_ps;
	 uint16	valid;
#endif
}rtl865x_arpMapping_entry_t;


struct rtl865x_arp_table {
        uint8                                           allocBitmap[64];
        rtl865x_arpMapping_entry_t      mappings[512];
};

/*for linux protocol stack sync*/
int32 rtl865x_addArp(ipaddr_t ip, ether_addr_t * mac);
int32 rtl865x_delArp(ipaddr_t ip);
uint32 rtl865x_arpSync( ipaddr_t ip, uint32 refresh );
int32 rtl865x_getArpMapping(ipaddr_t ip, rtl865x_arpMapping_entry_t * arp_mapping);
#if defined(CONFIG_RTL_MULTIPLE_WAN) ||defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
int32 rtl_set_callback_for_ps_arp(int (*call_back_fn)(u32 ip,rtl865x_arpMapping_entry_t *entry));
#endif

#if 1//#if defined(CONFIG_RTL_AVOID_ADDING_WLAN_PKT_TO_HW_NAT)
int32 rtl865x_isEthArp(ipaddr_t ip);
#endif
#endif
