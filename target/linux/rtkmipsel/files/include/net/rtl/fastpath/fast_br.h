#ifndef FAST_BR_H
#define FAST_BR_H

#include <linux/netdevice.h>

#define RTL_FAST_BR_HEAD_SIZE 1
#define RTL_FAST_BR_HASH_SIZE 4
#define RTL_FAST_BR_ENTRY_NUM 16
#define RTL_FAST_BR_SALT 3
#define RTL_FAST_BR_ENTRY_TIME_OUT 180*HZ

#define RTL_FAST_BR_SUCCESS 0
#define RTL_FAST_BR_FAILED -1
#define RTL_FAST_BR_ENTRY_EXPIRED 1
#define RTL_FAST_BR_ENTRY_NOT_EXPIRED 0

#define ERR_RTL_FAST_BR_ENTRY_EXIST -1000
#define ERR_RTL_FAST_BR_ENTRY_NOT_EXIST -1001
#define ERR_RTL_FAST_BR_NO_BUFFER -1002



typedef struct _fast_br_head
{
	struct hlist_head fast_br_hash[RTL_FAST_BR_HASH_SIZE];
	
}fast_br_head;

typedef struct _fast_br_cache_entry
{
	struct hlist_node	hlist;
	struct net_device *to_dev;
	unsigned long		ageing_timer;
	unsigned char		mac_addr[6];
	unsigned int 		valid;
}fast_br_cache_entry;

int rtl_add_fast_br_entry(fast_br_cache_entry *entry);
int rtl_fast_br_forwarding(struct sk_buff *skb);
#endif

