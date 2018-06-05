#ifndef _RTL_DNS_TRAP_H
#define _RTL_DNS_TRAP_H

//#define DBG_DNS_TRAP
//#define SUPPORT_TRAP_ALL

#if defined(DBG_DNS_TRAP)
	#define DBGP_DNS_TRAP(format, arg...) 	  \
		do {printk(format , ## arg);}while(0)
#else
	#define DBGP_DNS_TRAP(format, arg...)
#endif

typedef struct _header {
	unsigned short int	id;
	unsigned short		u;

	short int	qdcount;
	short int	ancount;
	short int	nscount;
	short int	arcount;
} dnsheader_t;

extern int br_dns_filter_enter(struct sk_buff *skb);
extern int is_dns_packet(struct sk_buff *skb);
extern int is_recaped_dns_packet(struct sk_buff *skb);
extern int dns_filter_enable;

extern int br_dns_filter_init(void);
extern void br_dns_filter_exit(void);


#endif	/* _RTL_DNS_TRAP_H */
