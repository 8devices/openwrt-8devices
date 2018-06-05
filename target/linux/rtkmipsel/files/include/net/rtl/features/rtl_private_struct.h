#ifndef	RTL_PRIVATE_STRUCT_H
#define RTL_PRIVATE_STRUCT_H

#if defined(CONFIG_RTL_HARDWARE_NAT)||defined(CONFIG_RTL_IPTABLES_FAST_PATH)
#define CONFIG_RTL_HW_NAT_BYPASS_PKT 1
#define RTL_HW_NAT_BYPASS_PKT_NUM	50
#endif
/**
 * inside structure in kernel struct nf_conn, 
 * add any data member here you need,not add 
 * in kernel struct nf_conn,same rule with 
 * sk_buff, rtl_sk_buff will create with same 
 * purpose
 **/
struct rtl_nf_conn{
	#if defined(CONFIG_RTL_HW_NAT_BYPASS_PKT)
	unsigned long count;
	#endif
};
#endif
