#ifndef RTL865X_ARP_IPV6_API_H
#define RTL865X_ARP_IPV6_API_H

#define RTL8198C_IPV6_ARPTBL_SIZE 256

typedef struct rtl8198c_ipv6_arpMapping_entry_s
{
	 uint32 valid;
        inv6_addr_t ip;
        ether_addr_t mac;
	 uint32 subnetIdx;
}rtl8198c_ipv6_arpMapping_entry_t;

struct rtl8198c_ipv6_arp_table {
        rtl8198c_ipv6_arpMapping_entry_t      mappings[RTL8198C_IPV6_ARPTBL_SIZE];
};

/*for linux protocol stack sync*/
int32 rtl8198c_addIpv6Arp(inv6_addr_t ip, ether_addr_t * mac);
int32 rtl8198c_delIpv6Arp(inv6_addr_t ip);
int32 rtl8198c_delIpv6ArpBySubnetIdx(uint32 subnetIdx);
uint32 rtl8198c_ipv6ArpSync(inv6_addr_t ip, uint32 refresh);
int32 rtl8198c_getIpv6ArpMapping(inv6_addr_t ip, rtl8198c_ipv6_arpMapping_entry_t * arp_mapping);
int32 rtl8198c_changeIpv6ArpSubnetIdx(uint32 old_subnetIdx, uint32 new_subnetIdx);
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_HW_DSLITE_SUPPORT)
int32 rtl8198c_dslite_Ipv6ArpMapping(inv6_addr_t ip, rtl8198c_ipv6_arpMapping_entry_t * arp_mapping);
#endif
#endif
#endif

