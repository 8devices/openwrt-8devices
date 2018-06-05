/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>	
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/net.h>
#include <linux/socket.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/raw.h>
#include <net/checksum.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>
#include <linux/inetdevice.h>
#include <linux/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>

#include <net/rtl/rtl_types.h>
#ifdef CONFIG_NETFILTER
#include <net/netfilter/nf_conntrack.h>
#include <net/rtl/fastpath/fastpath_core.h>
#endif
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>


struct sock *igmp_delete_sk = NULL;
struct test_struct
{
	char data[30];
};
// to_be_checked !!!
#if !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F) && !defined(CONFIG_OPENWRT_SDK)
static int try_mac(const char *data, char array[],
		      int array_size, char sep)
{
	uint32 i;

	memset(array, 0, sizeof(array[0])*array_size);

	/* Keep data pointing at next char. */
	for (i = 0;  i < array_size; data++) {
		if (*data >= '0' && *data <= '9') {
			array[i] = array[i]*16 + *data - '0';
		}
		else if(*data >='a' && *data <='f') {
			array[i] = array[i]*16 + *data - 'a'+10;
		}
		else if(*data >='A' && *data <='F') {
			array[i] = array[i]*16 + *data - 'A'+10;
		}
		else if (*data == sep)
			i++;
		else {
			/* Unexpected character; true if it's the
			   terminator and we're finished. */
			if (i == array_size - 1)
				return 1;

			printk("Char %u  '%c' unexpected\n",\
				 i, *data);
			return 0;
		}
	}
	return 0;
}
#endif

extern int32 rtl_delIgmpRecordByMacAddr(uint8 *macAddr);
void igmp_delete (struct sk_buff *__skb)
{
// to_be_checked !!!
#if !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F) && !defined(CONFIG_OPENWRT_SDK)
  	int pid;
	struct test_struct send_data,recv_data;
	uint8 mac[6];
	char *ptr;
	pid=rtk_nlrecvmsg(__skb,sizeof(struct test_struct),&recv_data);	
 	//printk("igmp_delete data:%s\n", recv_data.data);
	memset(mac, 0, 6);
	ptr = recv_data.data;
	if(try_mac(recv_data.data, mac, 6, ':'))
	{
		rtl_delIgmpRecordByMacAddr(mac);
		//printk("filter mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    		sprintf(send_data.data,"clear igmp cache ok\n");
	}
	else
	{
		printk("clear igmp cache of bad format mac:%s\n", recv_data.data);
    		sprintf(send_data.data,"please input like 00:23:e8:79:99:32\n");
	}
      	rtk_nlsendmsg(pid, igmp_delete_sk, sizeof(struct test_struct), &send_data);
#endif		
  	return;
}

int igmp_delete_init_netlink(void) 
{
// to_be_checked !!!
//#ifndef CONFIG_RTL_8198C // modified by lynn_pu, 2014-10-22
#if !defined(CONFIG_RTL_8198C) && !defined(CONFIG_RTL_8197F) && (!defined(CONFIG_RLX)&&(LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)))
  	igmp_delete_sk = netlink_kernel_create(&init_net, NETLINK_MULTICAST_DELETE, 0, igmp_delete, NULL, THIS_MODULE);

  	if (!igmp_delete_sk) {
    		printk(KERN_ERR "Netlink[Kernel] Cannot create netlink socket for igmp delete.\n");
    		return -EIO;
  	}	
  	printk("Netlink[Kernel] create socket for igmp ok.\n");
#endif	
  	return 0;
}

EXPORT_SYMBOL(igmp_delete_init_netlink);

