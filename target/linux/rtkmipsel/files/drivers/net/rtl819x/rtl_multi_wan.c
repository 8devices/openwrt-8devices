/***************************************************************************
 * File Name    : rtl_multi_wan.c
 * Description  : smux mean server mux.
 ***************************************************************************/
#include <asm/uaccess.h>
#include <linux/capability.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/init.h>
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
//#include <net/rtl/rtl_multi_wan.h>
#include <net/rtl/rtl_types.h>
#ifdef CONFIG_IP_MROUTE
#include <linux/inetdevice.h>
#endif
#include <linux/if_vlan.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <net/rtl/rtl_multi_wan.h>
#include <net/rtl/rtl_nic.h>
//#include <net/rtl/rtl867x_hwnat_api.h>
#include "../../net/bridge/br_private.h"

static char vlan_passthru = 0;

extern int rtl_register_multi_wan_dev(char *ifname, int proto, int vid, int napt);
extern int rtl_update_port_mapping_multi_wan_dev(char *ifname,unsigned int member);
extern int rtl_unregister_multi_wan_dev(char *ifname);
extern int rtl_set_multi_wan_netif_mac_addr(char *ifname, char *addr);

int smux_dev_open(struct net_device *vdev);
int smux_dev_stop(struct net_device *dev);
int smux_dev_ioctl(struct net_device *vdev, struct ifreq *ifr, int cmd);
int smux_dev_change_mtu(struct net_device *vdev, int new_mtu);
int smux_dev_set_mac_address(struct net_device *dev, void *addr_struct_p);
__IRAM int smux_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);

#ifdef CONFIG_PORT_MIRROR
extern void nic_tx_mirror (struct sk_buff *skb);
static inline void smux_mirror_pkt(struct sk_buff *skb, 
			const struct smux_dev_info *dev_info, const int flag);

#define IN  0x1
#define OUT 0x2
#endif

#ifdef DEBUG
#define DPRINTK(format, args...) printk(KERN_DEBUG "SMUX: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif

//extern unsigned int pvid_per_port[RTL8651_PORT_NUMBER+3];

//#define UNIQUE_MAC_PER_DEV
#undef UNIQUE_MAC_PER_DEV

/***************************************************************************
                         Global variables 
 ***************************************************************************/
#define SET_MODULE_OWNER(dev) do { } while (0)

static DEFINE_RWLOCK(smux_lock);

static LIST_HEAD(smux_grp_devs);

static int smux_device_event(struct notifier_block *, unsigned long, void *);

static struct notifier_block smux_notifier_block = {
	.notifier_call = smux_device_event,
};

#if defined(CONFIG_COMPAT_NET_DEV_OPS)
#else
static const struct net_device_ops smux_netdev_ops = {
	.ndo_open			= smux_dev_open,
	.ndo_stop			= smux_dev_stop,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address 	= smux_dev_set_mac_address,
	.ndo_do_ioctl			= smux_dev_ioctl,
	.ndo_start_xmit		= smux_dev_hard_start_xmit,
	.ndo_change_mtu		= smux_dev_change_mtu,
	.ndo_get_stats 		= smux_dev_get_stats,
};
#endif

#ifdef UNIQUE_MAC_PER_DEV
unsigned char wan_dev_def_vid[9]={[0 ... 8]=0};//index 0 is reserved

/*
 * return value: -1 : FAIL
 */
int allocSmuxDevVid(void)
{
	int i;
	
	for (i=1; i<9; i++)
	{
		if (!wan_dev_def_vid[i])
			break;
	}
	if (i<9)
		return i;

	return -1;
}

int freeSmuxDevVid(int vid)
{
	if ((vid >= 9) || (vid <= 0))
		return -1;
	
	wan_dev_def_vid[vid] = 0;

	return 0;
}
#endif

/***************************************************************************
                         Function Definisions
 ***************************************************************************/

static int smux_ioctl_handler(void __user *);
static inline struct smux_group *list_entry_smuxgrp(const struct list_head *le)
{
	return list_entry(le, struct smux_group, smux_grp_devs);
}

/***************************************************************************
 * Function Name: __find_smux_group
 * Description  : returns the smux group of interfaces/devices from list
 * Returns      : struct smux_group.
 ***************************************************************************/
static struct smux_group *__find_smux_group(const char *ifname)
{
	struct list_head *lh;
	struct smux_group *smux_grp;
	struct smux_group *ret_smux = NULL;

	read_lock(&smux_lock);
	list_for_each(lh, &smux_grp_devs) {
		smux_grp = (struct smux_group *)list_entry_smuxgrp(lh);
		if (!strncmp(smux_grp->real_dev->name, ifname, IFNAMSIZ)) {
			ret_smux = smux_grp;
			break;
		}
	}
	read_unlock(&smux_lock);

	return ret_smux;
} /* __find_smux_group */

static inline struct smux_dev_info *list_entry_smuxdev(const struct list_head *le)
{
  return list_entry(le, struct smux_dev_info, list);
}

void open_smux_device(char *ifname)
{
	struct list_head *lg, *lh;
	struct smux_group *smux_grp;
	struct smux_dev_info * sdev = NULL;

	read_lock(&smux_lock);
	list_for_each(lh, &smux_grp_devs){
		smux_grp = (struct smux_group *)list_entry_smuxgrp(lh);
		if (!strncmp(smux_grp->real_dev->name, ifname, IFNAMSIZ)){
			list_for_each(lg, &smux_grp->virtual_devs){
				sdev = list_entry_smuxdev(lg);
				netif_wake_queue(sdev->vdev);
			}
			break;
		}
	}
	read_unlock(&smux_lock);

} /* open_smux_device */


void close_smux_device(char *ifname)
{
	struct list_head *lg, *lh;
	struct smux_group *smux_grp;
	struct smux_dev_info * sdev = NULL;

	read_lock(&smux_lock);
	list_for_each(lh, &smux_grp_devs){
		smux_grp = (struct smux_group *)list_entry_smuxgrp(lh);
		if (!strncmp(smux_grp->real_dev->name, ifname, IFNAMSIZ)){
			list_for_each(lg, &smux_grp->virtual_devs){
				sdev = list_entry_smuxdev(lg);
				netif_stop_queue(sdev->vdev);
			}
			break;
		}
	}
	read_unlock(&smux_lock);

} /* close_smux_device */

/***************************************************************************
 * Function Name: __find_smux_in_smux_group
 * Description  : returns the smux device from smux group of devices 
 * Returns      : struct net_device
 ***************************************************************************/
static struct net_device *__find_smux_in_smux_group(
                                     struct smux_group *smux_grp, 
                                     const char *ifname)
{
	struct list_head *lh;
	struct smux_dev_info * sdev = NULL;
	struct net_device    * ret_dev = NULL;

	read_lock(&smux_lock);
	list_for_each(lh, &smux_grp->virtual_devs) {
		sdev = list_entry_smuxdev(lh);
		if(!strncmp(sdev->vdev->name, ifname, IFNAMSIZ)) {
			ret_dev = sdev->vdev;
			break;
		}
	}
	read_unlock(&smux_lock);

	return ret_dev;
} /* __find_smux_in_smux_group */

static void smux_eat_vlan(struct sk_buff *skb, struct smux_dev_info *info)
{
//	struct vlan_hdr *vhdr=NULL;
	struct net_bridge_port *p;		

	if ((skb->dev->priv_flags & IFF_VSMUX) && (skb->protocol ==  __constant_htons(ETH_P_8021Q))) {
		if (info->proto == SMUX_PROTO_BRIDGE) { // smux tagged bridge interface: do not passthrough vlan.
			skb_push(skb, ETH_HLEN);
			memmove(skb->data + VLAN_HLEN, skb->data, VLAN_ETH_ALEN<<1);
			skb_pull(skb, VLAN_HLEN);
			skb->protocol = eth_type_trans(skb, skb->dev);
			//printk("brg protocol 0x%x\n", skb->protocol);
		} else { // pull out vlan tag
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
			p = br_port_get_rcu(skb->dev);
			#else
			p = skb->dev->br_port;	
			#endif
			if ((p!=NULL) && (p->br!=NULL)) {
				skb_push(skb, ETH_HLEN);
				memmove(skb->data + VLAN_HLEN, skb->data, VLAN_ETH_ALEN<<1);
				skb_pull(skb, VLAN_HLEN);
				skb->protocol = eth_type_trans(skb, skb->dev);
			} else {
				#if 0
				/*Here still keep the vlan hdr*/
				vhdr = (struct vlan_hdr *)(skb->data);
				skb->protocol = vhdr->h_vlan_encapsulated_proto;
				skb_pull(skb, VLAN_HLEN);
				#else
				skb_push(skb, ETH_HLEN);
				memmove(skb->data + VLAN_HLEN, skb->data, VLAN_ETH_ALEN<<1);
				skb_pull(skb, VLAN_HLEN);
				skb->protocol = eth_type_trans(skb, skb->dev);
				#endif
			}
			//printk("rt protocol 0x%x\n", skb->protocol);
		}
	}
}

#if defined(CONFIG_RTL_MAC_BASED_NETIF)
/*
 *	Return Value:
 *	0: smux device not accept this packet
 *	1: smux device accept this packet
 */
static inline int smux_check_mac(struct smux_dev_info *dev_info, struct sk_buff *skb)
{
	struct net_device *vdev;
	
	vdev = dev_info->vdev;

	if (!memcmp(eth_hdr(skb)->h_dest, vdev->dev_addr, ETH_ALEN))
		return 1;
	
	return 0;
}
#else
/*
 *	Return Value:
 *	0: smux device not accept this packet
 *	1: smux device accept this packet
 */
static inline int smux_check_vlan(struct smux_dev_info *dev_info, struct sk_buff *skb)
{
	struct net_device *vdev;
	unsigned short tpid_check = skb->protocol;	
	unsigned short vid = 0;
	struct vlan_hdr *vhdr=NULL;	
	
	if (tpid_check == __constant_htons(ETH_P_8021Q)){
		vhdr = (struct vlan_hdr *)(skb->data);
		vid = (vhdr->h_vlan_TCI)&VLAN_VID_MASK;
	}
	
	vdev = dev_info->vdev;
	
	if (vdev->priv_flags & IFF_VSMUX) { // tagged wan
		if (vid == 0) // untagged packet
			return 0;
		if (vid != dev_info->vid)
			return 0;
	} else { // untagged wan
		if (vid) { // tagged packet
			if (vlan_passthru){ // vlan passthrough on untagged bridged wan
				if (dev_info->proto != SMUX_PROTO_BRIDGE)
					return 0;
			} else {
				return 0;
			}
		}
	}
	
	return 1;
}
#endif

/***************************************************************************
 * Function Name: rtl_smux_pkt_recv
 * Description  : packet recv routine for all smux devices from real dev.
 * Returns      : 0 on Success
 ***************************************************************************/
int rtl_smux_pkt_recv(struct sk_buff *skb, struct net_device *dev)
{
	struct smux_group *grp;
	unsigned char *dstAddr;
	struct sk_buff *skb2;
	struct smux_dev_info *dev_info;
	struct smux_dev_info *dev_info_first;
	struct list_head *lh;
	struct net_device *vdev = NULL;
	struct vlan_hdr *vhdr=NULL;
	unsigned short protocol=skb->protocol;
	int isTxDone = 0;

	if (dev == NULL) {
		dev_kfree_skb(skb);
		return 1;
	}

	grp = __find_smux_group(dev->name);
	if (grp == NULL) {
		dev_kfree_skb(skb);
		return 1;
	}

	if (protocol ==  __constant_htons(ETH_P_8021Q)) {
		vhdr = (struct vlan_hdr *)(skb->data);
		protocol = vhdr->h_vlan_encapsulated_proto;
	}
	
	dstAddr = eth_hdr(skb)->h_dest;
	//printk("%s %d enter=================>\n", __FUNCTION__, __LINE__);
	read_lock(&smux_lock);
	/* Multicast Traffic will go on all intf.*/
	if (dstAddr[0] & 1)
	{
		dev_info_first = NULL;
		/* multicast or broadcast frames */
		list_for_each(lh, &grp->virtual_devs)
		{
			dev_info = list_entry_smuxdev(lh);
			vdev = dev_info->vdev;
		
#if defined(CONFIG_RTL_MAC_BASED_NETIF)
#else
			if (!smux_check_vlan(dev_info, skb))
				continue;
#endif

#if 0
#if 0
			if (((dev_info->proto == SMUX_PROTO_PPPOE) && (protocol != htons(ETH_P_PPP_DISC)) && (protocol != htons(ETH_P_PPP_SES))) ||
				((dev_info->proto == SMUX_PROTO_IPOE) && ((protocol == htons(ETH_P_PPP_DISC)) || (protocol == htons(ETH_P_PPP_SES)))))
			{
				DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
				continue;
			}
#else
			if ((dev_info->proto == SMUX_PROTO_IPOE))
			{
#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
				if((dstAddr[0]==0x01) && (dstAddr[1]==0x00)&& (dstAddr[2]==0x5e))
				{
					if(!dev_info->igmpproxy_enable)
					{
						//drop all multicast packets
						DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
						continue;
					}
					else
					{
						if((protocol == htons(ETH_P_PPP_DISC)) || (protocol == htons(ETH_P_PPP_SES)))
						{
							DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
							continue;
						}
					}
				}
				else
#endif
				{
					if((protocol == htons(ETH_P_PPP_DISC)) || (protocol == htons(ETH_P_PPP_SES)))
					{
						DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
						continue;
					}
				}
			}


			if (dev_info->proto == SMUX_PROTO_PPPOE)
			{
#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
				if((dstAddr[0]==0x01) && (dstAddr[1]==0x00)&& (dstAddr[2]==0x5e))
				{
					if(!dev_info->igmpproxy_enable)
					{
						//drop all multicast packets
						DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
						continue;
					}
					else
					{
						if((dev_info->on_phy) && ((protocol == htons(ETH_P_PPP_DISC)) || (protocol == htons(ETH_P_PPP_SES))))
						{
							DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
							continue;
						}
						else if((!dev_info->on_phy) && ((protocol != htons(ETH_P_PPP_DISC)) && (protocol != htons(ETH_P_PPP_SES))))
						{
							DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
							continue;
						}
					}
				}
				else
#endif
				{
					if((protocol != htons(ETH_P_PPP_DISC)) && (protocol != htons(ETH_P_PPP_SES)))
					{
						DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
						continue;
					}
				}
			}

#endif
#endif

			if (dev_info_first == NULL) {
				dev_info_first = dev_info;
				continue;
			}
	
#ifdef	CONFIG_PORT_MIRROR
			if(IN_NEED_MIR(dev_info->port_mirror))
			{
				smux_mirror_pkt(skb, dev_info, IN);
			}
#endif
			skb2 = skb_clone(skb, GFP_ATOMIC);
			dev_info->stats.rx_packets++;
			//printk("%s %d vdev is %s, rdev is %s\n", __FUNCTION__, __LINE__, vdev->name, skb2->dev->name);
			dev_info->stats.rx_bytes += skb2->len;
			skb2->dev = vdev;
			skb2->from_dev = vdev;
			//skb2->pkt_type = PACKET_HOST;
			
			smux_eat_vlan(skb2, dev_info);
			netif_rx(skb2);
		}

		if (dev_info_first == NULL) {
			dev_kfree_skb(skb);
			read_unlock(&smux_lock);
			return 1;
		} else {
			//printk("%s %d vdev is %s, rdev is %s\n", __FUNCTION__, __LINE__, dev_info_first->vdev->name, skb->dev->name);

			dev_info_first->stats.rx_packets++;
			dev_info_first->stats.rx_bytes += skb->len; 
			skb->dev = dev_info_first->vdev;
			skb->from_dev = dev_info_first->vdev;
			//skb->pkt_type = PACKET_HOST;
#ifdef	CONFIG_PORT_MIRROR
			if(IN_NEED_MIR(dev_info->port_mirror))
			{
				smux_mirror_pkt(skb, dev_info, IN);
			}
#endif
			smux_eat_vlan(skb, dev_info_first);
			netif_rx(skb);
		}	
		isTxDone = 1;		
	}
	else /* route Traffic.*/
	{
		#ifndef UNIQUE_MAC_PER_DEV
		dev_info_first = NULL;
		#endif
		/* Routing Interface Traffic : check dst mac */
		list_for_each(lh, &grp->virtual_devs)
		{
			dev_info = list_entry_smuxdev(lh);
			if (dev_info->proto == SMUX_PROTO_BRIDGE)
				continue;
			
			vdev = dev_info->vdev;
#if defined(CONFIG_RTL_MAC_BASED_NETIF)
			if (!smux_check_mac(dev_info, skb))
				continue;
#else	
			if (!smux_check_vlan(dev_info, skb))
				continue;
#endif

#if 0
			if (((dev_info->proto == SMUX_PROTO_PPPOE) && (protocol != htons(ETH_P_PPP_DISC)) && (protocol != htons(ETH_P_PPP_SES))) ||
				((dev_info->proto == SMUX_PROTO_IPOE) && ((protocol == htons(ETH_P_PPP_DISC)) || (protocol == htons(ETH_P_PPP_SES)))))
			{
				DPRINTK("TRACE %d: packet dropped on RX dev %s\n", __LINE__, vdev->name);
				continue;
			}
#endif

			#ifndef UNIQUE_MAC_PER_DEV
			if (!memcmp(dstAddr, vdev->dev_addr, ETH_ALEN))
			{
				if(dev_info_first == NULL) {
					dev_info_first = dev_info;
					continue;
				}
				
				skb2 = skb_copy(skb, GFP_ATOMIC);
			//printk("%s %d vdev is %s, rdev is %s\n", __FUNCTION__, __LINE__, vdev->name, skb2->dev->name);

				skb2->dev = vdev;
				skb2->from_dev = vdev;
				dev_info->stats.rx_packets++;
				dev_info->stats.rx_bytes += skb2->len;
				skb2->pkt_type = PACKET_HOST;
				//printk("(route) receive from %s\n", vdev->name);
#ifdef	CONFIG_PORT_MIRROR
				if(IN_NEED_MIR(dev_info->port_mirror))
				{
					smux_mirror_pkt(skb, dev_info, IN);
				}
#endif			
				smux_eat_vlan(skb2, dev_info);
				netif_rx(skb2);

				isTxDone = 1;
			}
			#else
			if (!memcmp(dstAddr, vdev->dev_addr, ETH_ALEN))
			{
				skb->dev = vdev;
				skb->from_dev = vdev;
				dev_info->stats.rx_packets++;
				dev_info->stats.rx_bytes += skb->len;
				skb->pkt_type = PACKET_HOST;
				//printk("(route) receive from %s\n", vdev->name);

#ifdef	CONFIG_PORT_MIRROR
				if(IN_NEED_MIR(dev_info->port_mirror))
				{
					smux_mirror_pkt(skb, dev_info, IN);
				}
#endif
				smux_eat_vlan(skb, dev_info);
				netif_rx(skb);

				isTxDone = 1;
				break;
			}
			#endif
		}

		#ifndef UNIQUE_MAC_PER_DEV
		if (dev_info_first) {
			dev_info_first->stats.rx_packets++;
			dev_info_first->stats.rx_bytes += skb->len; 
			//printk("%s %d vdev is %s, rdev is %s\n", __FUNCTION__, __LINE__, vdev->name, skb->dev->name);

			skb->dev = dev_info_first->vdev;
			skb->from_dev = dev_info_first->vdev;
			skb->pkt_type = PACKET_HOST;
			//printk("(route) receive from %s\n", dev_info_first->vdev->name);
#ifdef	CONFIG_PORT_MIRROR
			if(IN_NEED_MIR(dev_info->port_mirror))
			{
				smux_mirror_pkt(skb, dev_info, IN);
			}
#endif	
			smux_eat_vlan(skb, dev_info_first);
			netif_rx(skb);
			
			isTxDone = 1;
		}
		#endif
	}

	if (isTxDone != 1) 
	{
		/* Bridging Interface Traffic */
		list_for_each(lh, &grp->virtual_devs)
		{
			dev_info = list_entry_smuxdev(lh);
			if (dev_info->proto != SMUX_PROTO_BRIDGE && !dev_info->brpppoe)
				continue;
			
			vdev = dev_info->vdev;

#if defined(CONFIG_RTL_MAC_BASED_NETIF)
#else
			if (!smux_check_vlan(dev_info, skb))
				continue;
#endif
			
			if (vdev->promiscuity)
			{
				skb->dev = vdev;
				skb->from_dev = vdev;
				dev_info->stats.rx_packets++;
				dev_info->stats.rx_bytes += skb->len; 
				skb->pkt_type = PACKET_OTHERHOST;
				//printk("(bridge) receive from %s\n", vdev->name);
#ifdef	CONFIG_PORT_MIRROR
				if(IN_NEED_MIR(dev_info->port_mirror))
				{
					smux_mirror_pkt(skb, dev_info, IN);
				}
#endif
				smux_eat_vlan(skb, dev_info);
				netif_rx(skb);
				isTxDone = 1;
				break;
			}
		}
	}
	read_unlock(&smux_lock);

	if (isTxDone == 0) {
		DPRINTK("dropping packet that has wrong dest. on RX dev %s\n", dev->name);
		dev_kfree_skb(skb);
		return 1;
	}

	return 0;
} /* rtl_smux_pkt_recv */
/*Start:add by caoxiafei cKF24361 20100426 for fastpath*/
/***************************************************************************
 * Function Name: rtl_get_smux_dev
 * Description  : fetch smux devices according to real dev.
 * Returns      : 0 on fail
 ***************************************************************************/
 /*jwj: It not used now, later....*/
struct net_device * rtl_get_smux_dev(struct sk_buff *skb)
{
	struct smux_group *grp;
	unsigned char *dstAddr;
	struct smux_dev_info *dev_info;
	struct list_head *lh;
	struct net_device *vdev;

	grp = __find_smux_group(skb->dev->name);
	if (!grp)
		return NULL;
	dstAddr = eth_hdr(skb)->h_dest;
	if (dstAddr[0]&0x01)
		return NULL;
	read_lock(&smux_lock);
	list_for_each(lh, &grp->virtual_devs)
	{
		dev_info = list_entry_smuxdev(lh);
		vdev = dev_info->vdev;
		if (!compare_ether_addr(dstAddr, vdev->dev_addr)) {
			skb->dev = vdev;
			read_unlock(&smux_lock);
			return vdev;
		}
	}

	read_unlock(&smux_lock);
	return NULL;
}
/*End:add by caoxiafei cKF24361 20100426 for fastpath*/

/***************************************************************************
 * Function Name: smux_dev_hard_start_xmit
 * Description  : xmit routine for all smux devices on real dev.
 * Returns      : 0 on Success
 ***************************************************************************/
__IRAM
int smux_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = smux_dev_get_stats(dev);
	struct smux_dev_info *dev_info;

	stats->tx_packets++; 
	stats->tx_bytes += skb->len;

	dev_info = SMUX_DEV_INFO(dev);
	skb->dev = dev_info->smux_grp->real_dev;	//replace as the real dev
	if (dev_info->vid == -1) {
		#if defined(CONFIG_RTL_8676HWNAT)
		if (dev_info->proto == SMUX_PROTO_BRIDGE)
			skb->vlan_tci = RTL_BRIDGE_WANVLANID;
		else
			skb->vlan_tci = RTL_WANVLANID;
		#else
		skb->vlan_tci = 0;
		#endif
	} else {
		if (dev_info->m_1p == 0)
			skb->vlan_tci = (dev_info->vid&VLAN_VID_MASK);
		else
			skb->vlan_tci = (dev_info->vid&VLAN_VID_MASK) |((dev_info->m_1p-1)<<13);
	}
		
	skb->vlan_member = dev_info->member;
#ifdef CONFIG_PORT_MIRROR
	if ((OUT_NEED_MIR(dev_info->port_mirror))) {
		smux_mirror_pkt(skb, dev_info, OUT);
	}
#endif
	//printk("%s,%d::dev_info->member: %x\n",__FUNCTION__,__LINE__,skb->vlan_member);

	//skb->dev->hard_start_xmit(skb, skb->dev);
	skb->dev->netdev_ops->ndo_start_xmit(skb,skb->dev);

	//dev_queue_xmit(skb);

	return 0;
} /* smux_dev_hard_start_xmit */

/***************************************************************************
 * Function Name: smux_dev_open
 * Description  : 
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_open(struct net_device *vdev)
{
	if (!(SMUX_DEV_INFO(vdev)->smux_grp->real_dev->flags & IFF_UP))
		return -ENETDOWN;

	return 0;
} /* smux_dev_open */

/***************************************************************************
 * Function Name: smux_dev_stop
 * Description  : 
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_stop(struct net_device *dev)
{
	return 0;
} /* smux_dev_stop */

/***************************************************************************
 * Function Name: smux_dev_set_mac_address
 * Description  : sets the mac for devs
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_set_mac_address(struct net_device *dev, void *addr_struct_p)
{
	struct sockaddr *addr = (struct sockaddr *)(addr_struct_p);
	//int i, flgs;

	if (netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	memset(dev->broadcast, 0xff, ETH_ALEN);

//#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL_FLOW_BASE_HWNAT)
#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_set_multi_wan_netif_mac_addr(dev->name, addr->sa_data);
#endif

	return 0;
} /* smux_dev_set_mac_address */


/***************************************************************************
 * Function Name: smux_dev_ioctl
 * Description  : handles device related ioctls
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_ioctl(struct net_device *vdev, struct ifreq *ifr, int cmd)
{
	struct net_device *real_dev = SMUX_DEV_INFO(vdev)->smux_grp->real_dev;
	struct ifreq ifrr;
	int err = -EOPNOTSUPP;
#ifdef CONFIG_RTL_8676HWNAT		
	extern int set_port_mapping(struct net_device *dev, struct ifreq *rq, int cmd);
#endif

	strncpy(ifrr.ifr_name, real_dev->name, IFNAMSIZ);
	ifrr.ifr_ifru = ifr->ifr_ifru;

	//printk("%s %d cmd 0x%x (dev:%s)\n", __FUNCTION__, __LINE__, cmd, vdev->name);
	switch(cmd) {
		case SIOCGMIIPHY:
		case SIOCGMIIREG:
		case SIOCSMIIREG:
			//if (real_dev->do_ioctl && netif_device_present(real_dev))
			//err = real_dev->do_ioctl(real_dev, &ifrr, cmd);
			if (real_dev->netdev_ops->ndo_do_ioctl && netif_device_present(real_dev))
				err = real_dev->netdev_ops->ndo_do_ioctl(real_dev, &ifrr, cmd);
			break;

		case SIOCETHTOOL:
			err = dev_ethtool(&init_net, &ifrr);

			if (!err)
				ifr->ifr_ifru = ifrr.ifr_ifru;
			break;

#ifdef CONFIG_PORT_MIRROR
		case SIOCPORTMIRROR:
		{
			struct portmir *pmr;
			struct smux_dev_info *dev_info = SMUX_DEV_INFO(vdev);
			struct net_device *dev = NULL;

			pmr = (struct portmir *)ifr->ifr_data;
			//AUG_DBG("the pmr->mir_dev_name is %s\n", pmr->mir_dev_name);

			dev = dev_get_by_name(&init_net, pmr->mir_dev_name);
			if(!dev)
			{
				printk("error lan device!\n");
				break;
			}	
			if((dev->priv_flags & IFF_DOMAIN_ELAN) == 0)
			{
				printk("error lan device!\n");
				break;
			}
			dev_info->port_mirror = pmr->port_mirror;
			dev_info->mirror_dev  = dev;

			err = 0;
			break;
		}
#endif
			
		case SIOCSITFGROUP:
			{
				struct ifvlan *ifvl;
				struct smux_dev_info *dev_info = SMUX_DEV_INFO(vdev);

				ifvl = (struct ifvlan *)ifr->ifr_data;
				if (ifvl->enable)
					dev_info->member = ifvl->member;
				else
					dev_info->member = 0xFFFFFFFF;
				printk("%s %d set portmapping  (dev:%s  member:0x%x) \n", __FUNCTION__, __LINE__,vdev->name,dev_info->member);
#ifdef CONFIG_RTL_8676HWNAT					
				err = set_port_mapping(vdev, ifr, cmd);
#else
				err = 0;
#endif
				#if defined(CONFIG_RTL_HARDWARE_NAT)
				rtl_update_port_mapping_multi_wan_dev(vdev->name,dev_info->member);
				#endif
			}
			//rtl865x_updateNetifForPortmapping();
          
			return err;
	}

	return err;
} /* smux_dev_ioctl */

/***************************************************************************
 * Function Name: smux_dev_change_mtu
 * Description  : changes mtu for dev
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_change_mtu(struct net_device *vdev, int new_mtu)
{
	//MTU should be larger than real device.
	if (SMUX_DEV_INFO(vdev)->smux_grp->real_dev->mtu < new_mtu)
		return -ERANGE;

	//vdev->mtu = new_mtu;
	//SMUX_DEV_INFO(vdev)->smux_grp->real_dev->change_mtu(vdev, new_mtu);
	SMUX_DEV_INFO(vdev)->smux_grp->real_dev->netdev_ops->ndo_change_mtu(vdev, new_mtu);

	return 0;
}

/***************************************************************************
 * Function Name: smux_setup
 * Description  : inits device api
 * Returns      : None
 ***************************************************************************/
static void smux_setup(struct net_device *new_dev)
{
	SET_MODULE_OWNER(new_dev);
	//new_dev->get_stats = smux_dev_get_stats;
	//new_dev->netdev_ops->ndo_get_stats = smux_dev_get_stats;
	/* Make this thing known as a SMUX device */
	new_dev->priv_flags |= IFF_OSMUX;
	new_dev->tx_queue_len = 0;

#ifdef CONFIG_COMPAT_NET_DEV_OPS
	/* set up method calls */
	new_dev->change_mtu = smux_dev_change_mtu;
	new_dev->open = smux_dev_open;
	new_dev->stop = smux_dev_stop;
	new_dev->set_mac_address = smux_dev_set_mac_address;
	/*new_dev->set_multicast_list = smux_dev_set_multicast_list; TODO: */
	new_dev->destructor = free_netdev;
	new_dev->do_ioctl = smux_dev_ioctl;
#endif
} /* smux_setup */

/***************************************************************************
 * Function Name: smux_transfer_operstate
 * Description  : updates the operstate of overlay device 
 * Returns      : None.
 ***************************************************************************/
static void smux_transfer_operstate(const struct net_device *rdev, 
                                    struct net_device *vdev)
{

	if (rdev->operstate == IF_OPER_DORMANT)
		netif_dormant_on(vdev);
	else
		netif_dormant_off(vdev);

	if (netif_carrier_ok(rdev)) {
		if (!netif_carrier_ok(vdev))
			netif_carrier_on(vdev);
	} else {
		if (netif_carrier_ok(vdev))
			netif_carrier_off(vdev);
	}
} /* smux_transfer_operstate */

static const struct ethtool_ops smux_ethtool_ops = {
	.get_link = ethtool_op_get_link,
};

/***************************************************************************
 * Function Name: smux_register_device
 * Description  : regists new overlay device on real device & registers for 
                  packet handlers depending on the protocol types
 * Returns      : 0 on Success
 ***************************************************************************/
#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
static struct net_device *smux_register_device(const char *rifname,
					       const char *nifname, int smux_proto, int vid, int napt, int brpppoe, int enable, int on_phy)
#else
static struct net_device *smux_register_device(const char *rifname,
					       const char *nifname, int smux_proto, int vid, int napt, int brpppoe)
#endif
{
	struct net_device *new_dev = NULL;
	struct net_device *real_dev = NULL; 
	struct smux_group *grp = NULL;
	struct smux_dev_info *vdev_info = NULL;
	#ifdef UNIQUE_MAC_PER_DEV
	int    mac_reused = 0;
	unsigned char LSB=0;
	struct list_head *lh;
	#endif

	//printk("%s %d rifname is %s, nifname is %s\n", __FUNCTION__, __LINE__, rifname, nifname);
	real_dev = dev_get_by_name(&init_net, rifname);
	if (real_dev == NULL)
		goto real_dev_invalid;

	if (!(real_dev->flags & IFF_UP))
		goto real_dev_invalid;

	new_dev = alloc_netdev(sizeof(struct smux_dev_info), nifname, smux_setup);
	if (new_dev == NULL) {
		printk("smux netdev alloc failure\n");
		goto new_dev_invalid;
	}

	ether_setup(new_dev);
	if (vid != -1)
		new_dev->priv_flags |= IFF_VSMUX;

	new_dev->flags &= ~IFF_UP;
	new_dev->flags &= ~IFF_MULTICAST;
	new_dev->priv_flags |= IFF_DOMAIN_WAN;
	real_dev->priv_flags |= IFF_RSMUX;

	new_dev->state = (real_dev->state & 
                    ((1<<__LINK_STATE_NOCARRIER) |
                     (1<<__LINK_STATE_DORMANT))) |
                     (1<<__LINK_STATE_PRESENT);

	new_dev->mtu = real_dev->mtu;
	new_dev->type = real_dev->type;
	new_dev->hard_header_len = real_dev->hard_header_len;
#ifdef CONFIG_COMPAT_NET_DEV_OPS
	new_dev->hard_start_xmit = smux_dev_hard_start_xmit;
	new_dev->set_mac_address = smux_dev_set_mac_address;
#else
	new_dev->netdev_ops = &smux_netdev_ops;
#endif
	SET_ETHTOOL_OPS(new_dev, &smux_ethtool_ops);

	/* find smux group name. if not found create all new smux group */
	/*One real wan dev has one grp, and one grp can have some virtual wan dev.*/
	grp = __find_smux_group(rifname);
	if (grp == NULL) {
		grp = kzalloc(sizeof(struct smux_group), GFP_KERNEL);

		if (grp) {
			INIT_LIST_HEAD(&grp->virtual_devs);
			INIT_LIST_HEAD(&grp->smux_grp_devs);

			grp->real_dev = real_dev;

			write_lock_irq(&smux_lock);
			list_add_tail(&grp->smux_grp_devs, &smux_grp_devs);
			write_unlock_irq(&smux_lock);
		} else {
			free_netdev(new_dev);
			new_dev = NULL;
		}
	}

	if (grp && new_dev) {
		/* Assign default mac to bridge so that we can add it to linux bridge */
		if (smux_proto == SMUX_PROTO_BRIDGE) {
			memcpy( new_dev->dev_addr, "\xFE\xFF\xFF\xFF\xFF\xFF", ETH_ALEN );
		} else {
			#ifdef UNIQUE_MAC_PER_DEV
			if (list_empty(&grp->virtual_devs)) {
				memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
			} else {
				list_for_each(lh, &grp->virtual_devs)
				{
					vdev_info = list_entry_smuxdev(lh);
					if (!memcmp(real_dev->dev_addr, vdev_info->vdev->dev_addr, ETH_ALEN)) {
		            			mac_reused = 1;
					}
					if (LSB < vdev_info->vdev->dev_addr[5])
						LSB = vdev_info->vdev->dev_addr[5];
				}

				memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
				if (mac_reused) {
					//generate new mac address, real_addr mac addr increased by 1.
					new_dev->dev_addr[5] = LSB+1;
				}
			}
			#else
			memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
			#endif
		}
	}
  
	if (grp && new_dev) {
		struct net_device *ret_dev;
		/*find new smux in smux group if it does not exit create one*/
		if ((ret_dev=__find_smux_in_smux_group(grp, nifname)) == NULL) {
			vdev_info = SMUX_DEV_INFO(new_dev);
			memset(vdev_info, 0, sizeof(struct smux_dev_info));
			//m_1p : 0~8, 0 is meaning disable
			if (vid>=0)
				vdev_info->m_1p = vid>>13;
			else
				vdev_info->m_1p=0;			
			vdev_info->smux_grp = grp;
			vdev_info->vdev = new_dev;
			vdev_info->proto = smux_proto;	//pppoe or DHCP or bridge
		#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
			vdev_info->igmpproxy_enable = enable;
			vdev_info->on_phy = on_phy;
			//printk("igmpproxy enable: %d, on Phy: %d, [%s:%d]\n", vdev_info->igmpproxy_enable, vdev_info->on_phy, __FUNCTION__, __LINE__);
		#endif
			#ifdef UNIQUE_MAC_PER_DEV
			if ((vid == -1) && (smux_proto != SMUX_PROTO_BRIDGE)) {			
				if ((vid = allocSmuxDevVid()) == -1)
					printk("fatal error, too many wan interface created.\n");
			}
			#endif
			if (vid != -1)
				vdev_info->vid = (vid&VLAN_VID_MASK);
			else
				vdev_info->vid = vid;
			vdev_info->napt = napt;
			vdev_info->brpppoe = brpppoe;
			vdev_info->member = 0xFFFFFFFF;	//init membership to include all interface.
#ifdef CONFIG_PORT_MIRROR
			vdev_info->port_mirror = 0;
			vdev_info->mirror_dev = NULL;
#endif			
			if (smux_proto == SMUX_PROTO_BRIDGE) {
				new_dev->promiscuity = 1;
			} else if(smux_proto == SMUX_PROTO_IPOE) {
				new_dev->flags |= IFF_MULTICAST;
			}

			if (register_netdev(new_dev)) {
				printk("register_netdev failed\n");
				//list_del(&vdev_info->list);
				free_netdev(new_dev);
				new_dev = NULL;
			} else {
				INIT_LIST_HEAD(&vdev_info->list);
				write_lock_irq(&smux_lock);
				list_add_tail(&vdev_info->list, &grp->virtual_devs);
				write_unlock_irq(&smux_lock);
				smux_transfer_operstate(real_dev, new_dev);
			}
			#ifdef CONFIG_RTL_HARDWARE_NAT
			if (new_dev) {
				if (smux_proto != SMUX_PROTO_BRIDGE && brpppoe) {
					//char ifname_extend[IFNAMSIZ];
					//strcpy(ifname_extend,vdev_info->vdev->name);
					//strcat(ifname_extend,"_B");
					//#if defined(CONFIG_RTL_FLOW_BASE_HWNAT)
					rtl_register_multi_wan_dev(vdev_info->vdev->name, smux_proto, vid, napt);
					//rtl_register_multi_wan_dev(ifname_extend, SMUX_PROTO_BRIDGE, vid, napt);	
					//#endif
				} else {
//#if defined(CONFIG_RTL_FLOW_BASE_HWNAT)				
					rtl_register_multi_wan_dev(vdev_info->vdev->name, smux_proto, vid, napt);
//#endif
				}
			}
			#endif
		} else {
			printk("device %s already exist.\n", nifname);
			free_netdev(new_dev);
			new_dev = ret_dev;
		}
	}

	return new_dev;

real_dev_invalid:
new_dev_invalid:

	return NULL;
} /* smux_register_device */

/***************************************************************************
 * Function Name: smux_unregister_device
 * Description  : unregisters the smux devices along with releasing mem.
 * Returns      : 0 on Success
 ***************************************************************************/
static int smux_unregister_device(const char* vifname)
{
	struct net_device *vdev = NULL;
	struct net_device *real_dev = NULL;
	int ret;
	struct smux_dev_info *dev_info;
	ret = -EINVAL;

	vdev = dev_get_by_name(&init_net, vifname);
	if (vdev) {
		printk("%s[%d] remove smux dev %s\n", __FUNCTION__, __LINE__, vifname);
		/* remove related acl rule */
		#if 0
		#ifdef CONFIG_RTL8676_Dynamic_ACL
		rtl865x_acl_control_delete_all_by_netif(vdev->name);
		#endif
		rtl865x_delNetif(vdev->name);
		#endif

		dev_info = SMUX_DEV_INFO(vdev);
		#if defined(CONFIG_RTL_HARDWARE_NAT)
		#if defined(CONFIG_RTL_REDIRECT_ACL_SUPPORT_FOR_ISP_MULTI_WAN)
		rtl_disable_advRt_by_netifName(vdev->name);
		#endif
		rtl_unregister_multi_wan_dev(vdev->name);
		{
			char ifname_extend[IFNAMSIZ];
			strcpy(ifname_extend,vdev->name);
			strcat(ifname_extend,"_B");
			if(rtl865x_netif_exist(ifname_extend))
				rtl865x_unregisterDev(ifname_extend);
		}
		#endif
		#ifdef UNIQUE_MAC_PER_DEV
		freeSmuxDevVid(dev_info->vid);
		#endif
		real_dev = dev_info->smux_grp->real_dev;

		write_lock_irq(&smux_lock);
		list_del(&dev_info->list);
		write_unlock_irq(&smux_lock);

		if (list_empty(&dev_info->smux_grp->virtual_devs)) {
			write_lock_irq(&smux_lock);
			list_del(&dev_info->smux_grp->smux_grp_devs);
			write_unlock_irq(&smux_lock);

			kfree(dev_info->smux_grp);
		}

		dev_put(vdev);
		unregister_netdev(vdev);

		synchronize_net();
		dev_put(real_dev); 

		ret = 0;
	}

	return ret;
} /* smux_unregister_device */

/***************************************************************************
 * Function Name: smux_device_event
 * Description  : handles real device events to update overlay devs. status
 * Returns      : 0 on Success
 ***************************************************************************/
static int smux_device_event(struct notifier_block *unused, unsigned long event, void *ptr)
{
	struct net_device *rdev = ptr;
	struct smux_group *grp = __find_smux_group(rdev->name);
	int flgs;
	struct list_head *lh;
	struct list_head *lhp;
	struct smux_dev_info *dev_info;

	if (!grp)
		goto out;

	switch (event) {
		case NETDEV_CHANGE:

			/* Propagate real device state to overlay devices */
			read_lock(&smux_lock);
			list_for_each(lh, &grp->virtual_devs) {
				dev_info = list_entry_smuxdev(lh);
				if(dev_info) {
					smux_transfer_operstate(rdev, dev_info->vdev);
				}
			}
			read_unlock(&smux_lock);
			break;

		case NETDEV_DOWN:

			/* Put all Overlay devices for this dev in the down state too.*/
			read_lock(&smux_lock);
			list_for_each(lh, &grp->virtual_devs) {
				dev_info = list_entry_smuxdev(lh);
				if(dev_info) {
					flgs = dev_info->vdev->flags;

					if (!(flgs & IFF_UP))
						continue;

					dev_change_flags(dev_info->vdev, flgs & ~IFF_UP);
				}
			}
			read_unlock(&smux_lock);
			break;

		case NETDEV_UP:

			/* Put all Overlay devices for this dev in the up state too.  */
			read_lock(&smux_lock);
			list_for_each(lh, &grp->virtual_devs) {
				dev_info = list_entry_smuxdev(lh);
				if(dev_info) {
					flgs = dev_info->vdev->flags;

					if (flgs & IFF_UP)
						continue;

					dev_change_flags(dev_info->vdev, flgs & IFF_UP);
				}
			}
			read_unlock(&smux_lock);
			break;

		case NETDEV_UNREGISTER:
			
			/* Delete all Overlay devices for this dev. */
			write_lock_irq(&smux_lock);
			list_for_each_safe(lh, lhp, &grp->virtual_devs) {
				dev_info = list_entry_smuxdev(lh);
				if(dev_info) {
					/* delete by l67530 for cpu0 when reboot system. HG551c.2010/12/07 */
					//list_del(&dev_info->list);
					smux_unregister_device(dev_info->vdev->name);
				}
			}
			write_unlock_irq(&smux_lock);
			break;
		}

out:
  return NOTIFY_DONE;
} /* smux_device_event */

/***************************************************************************
 * Function Name: smux_ioctl_handler
 * Description  : ioctl handler for user apps
 * Returns      : 0 on Success
 ***************************************************************************/
static int smux_ioctl_handler(void __user *arg)
{
	int err = 0;
	struct smux_ioctl_args args;

	if (copy_from_user(&args, arg, sizeof(struct smux_ioctl_args)))
		return -EFAULT;

	args.rsmux_ifname[IFNAMSIZ-1] = 0;
	args.osmux_ifname[IFNAMSIZ-1] = 0;

	switch (args.cmd) {
		case ADD_SMUX_CMD:
			if (!capable(CAP_NET_ADMIN))
				return -EPERM;
		#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
			if(smux_register_device(args.rsmux_ifname, args.osmux_ifname, args.proto, args.vid, args.napt, args.brpppoe, args.igmpproxy_enable, args.on_phy)) 
		#else
			if(smux_register_device(args.rsmux_ifname, args.osmux_ifname, args.proto, args.vid, args.napt, args.brpppoe)) 
		#endif
			{
				err = 0;
			} else {
				err = -EINVAL;
			}
			break;

		case REM_SMUX_CMD:
			if (!capable(CAP_NET_ADMIN))
				return -EPERM;
			err = smux_unregister_device(args.u.ifname);
			break;

		default:
			printk("%s: Unknown SMUX CMD: %x \n", __FUNCTION__, args.cmd);
			return -EINVAL;
	}

	return err;
} /* smux_ioctl_handler */

static int vlan_passthru_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{	  
    unsigned char chartmp; 
	  
    if (count > 1){
      	if (buffer && !copy_from_user(&chartmp, buffer, 1)){
	    	vlan_passthru = chartmp - '0';				
			printk("vlan passthru  : %s\n", vlan_passthru ? "Enable" : "Disable");	
      	}
	} else {
		printk("write fail\n");
		return -EFAULT;
	}

	return count;
}

#ifdef CONFIG_RTL_PROC_NEW
extern struct proc_dir_entry proc_root;
static int32 vlan_passthru_read(struct seq_file *s, void *v)
{
	seq_printf(s, "vlan passthru  : %s\n",  vlan_passthru ? "Enable" : "Disable");
	
        return 0;
}
/*rtk_vlan_support*/
int vlan_passthru_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, vlan_passthru_read,NULL));
}

static ssize_t vlan_passthru_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return vlan_passthru_write(file, userbuf,count, off);
}

struct file_operations vlan_passthru_proc_fops = {
        .open          	= vlan_passthru_single_open,
        .write		= vlan_passthru_single_write,
        .read           	= seq_read,
        .llseek         	= seq_lseek,
        .release        	= single_release,
};
#else/*CONFIG_RTL_PROC_NEW*/
static struct proc_dir_entry *vlan_passthru_proc=NULL;
static int vlan_passthru_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
      int len;
      len = sprintf(page, "vlan passthru  : %s\n",  vlan_passthru ? "Enable" : "Disable");

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}
#endif

static int initVlanPassThruProc(void)
{
#ifdef CONFIG_RTL_PROC_NEW
    	proc_create_data("vlan_passthru",0, &proc_root, &vlan_passthru_proc_fops, NULL);
#else
	vlan_passthru_proc = create_proc_entry("vlan_passthru", 0, NULL);	
	if(vlan_passthru_proc)
	{
		vlan_passthru_proc->read_proc = vlan_passthru_read;
		vlan_passthru_proc->write_proc = vlan_passthru_write;
	}
#endif
	return 0;
}

/***************************************************************************
 * Function Name: smux_drv_init
 * Description  : Initialization of smux driver
 * Returns      : struct net_device
 ***************************************************************************/
static int __init smux_drv_init(void)
{
	register_netdevice_notifier(&smux_notifier_block);
	smux_ioctl_set(smux_ioctl_handler);
	initVlanPassThruProc();

	return 0;
} /* smux_drv_init */

/***************************************************************************
 * Function Name: smux_cleanup_devices
 * Description  : cleans up all the smux devices and releases memory on exit
 * Returns      : None
 ***************************************************************************/
static void __exit smux_cleanup_devices(void)
{
	struct net_device *dev;
	struct list_head *lh;
	struct list_head *lhp;
	struct smux_dev_info *dev_info;
	struct smux_group *grp;

	/* clean up all the smux devices */
	rtnl_lock();
	for_each_netdev(&init_net, dev)
	{
		if (dev->priv_flags & IFF_OSMUX) {
			dev_info = SMUX_DEV_INFO(dev);
			write_lock_irq(&smux_lock);
			list_del(&dev_info->list);
			write_unlock_irq(&smux_lock);
			unregister_netdevice(dev);
		}
	}
	rtnl_unlock();

	/* cleanup all smux groups  */
	write_lock_irq(&smux_lock);
	list_for_each_safe(lh, lhp, &smux_grp_devs) {
		grp = list_entry_smuxgrp(lh);
		if(grp) {
			list_del(&grp->virtual_devs);
		}
	}
	write_unlock_irq(&smux_lock);
} /* smux_cleanup_devices */

/***************************************************************************
 * Function Name: smux_drv_exit
 * Description  : smux module clean routine
 * Returns      : None
 ***************************************************************************/
static void __exit smux_drv_exit(void)
{
	smux_ioctl_set(NULL);

	/* Un-register us from receiving netdevice events */
	unregister_netdevice_notifier(&smux_notifier_block);
	smux_cleanup_devices();
	synchronize_net();
} /* smux_drv_exit */

/*jwj: It is not used now.*/
int rtl_get_vid_of_smux_dev(char *name)
{
	struct smux_dev_info *dev_info;
	struct net_device *vdev;

	vdev = dev_get_by_name(&init_net, name);
	if (!vdev)
		return -1;

	dev_put(vdev);
	dev_info = SMUX_DEV_INFO(vdev);
	
	if (dev_info->vid != -1)
		return dev_info->vid;
	else {
		if (dev_info->proto == SMUX_PROTO_BRIDGE)
			return RTL_BRIDGE_WANVLANID;
		else
			return RTL_WANVLANID;
	}
}

/*jwj: This API is used for hw multicast member port set. ?????*/
unsigned int rtl_get_member_of_smux_dev_by_vid(unsigned int vid)
{
	struct smux_dev_info *dev_info;
	struct net_device *vdev;
	unsigned char name[MAX_IFNAMESIZE];

	rtl865x_get_master_netif_by_vid(vid, name);

	vdev = dev_get_by_name(&init_net, name);
	if (!vdev)
		return -1;

	dev_put(vdev);
	
	dev_info = SMUX_DEV_INFO(vdev);

	return dev_info->member;
}

int rtl_smux_upstream_port_mapping_check(struct sk_buff *skb)
{
	struct net_device *from, *to;
	struct smux_dev_info *dev_info;
	unsigned int member, port;

	from = skb->from_dev;
	to = skb->dev;

	dev_info = SMUX_DEV_INFO(to);
	member = dev_info->member;
	
    	if (from->priv_flags & IFF_DOMAIN_ELAN) {
		TOKEN_NUM(from->name, &port);
		port -= 1;
	} else if (from->priv_flags & IFF_DOMAIN_WLAN) {
	    	TOKEN_NUM(from->name,&port);
	} else {
		//printk("%s not from lan dev.\n", __function__);
		return 0;
	}

	if (member & (1<<port)) {
		return 1;
	} 

	return 0;
}

int rtl_smux_downstream_port_mapping_check(struct sk_buff *skb)
{
	struct net_device *from, *to;
	struct smux_dev_info *dev_info;
	unsigned int member, port_mask;

	from = skb->from_dev;
	to = skb->dev;
    
	dev_info = SMUX_DEV_INFO(from);
	member = dev_info->member;

 #if 0
   	if (to->priv_flags & IFF_DOMAIN_ELAN) {
		TOKEN_NUM(to->name, &port);
		if (port > 1)
			port -= 1;
	} else if (to->priv_flags & IFF_DOMAIN_WLAN) {
		/*jwj: wlan dev name index???*/
	    	TOKEN_NUM(to->name, &port);
	} else {
		//	printk("%s not to lan dev.\n", __function__);
		return 0;
	}

	if (member & (1<<port)) {
		printk("%s %d from %s to %s member0x%x  member & (1<<port)=true \n", __FUNCTION__, __LINE__, from->name, to->name, member);
		return 1;
	}
#else
	port_mask = rtl_get_bind_port_mask_by_dev_name(to->name);
	if (member & port_mask) {
		//printk("%s %d from %s to %s member0x%x  member & port_mask=true \n", __FUNCTION__, __LINE__, from->name, to->name, member);
		return 1;
	}
#endif

	return 0;
}

#ifdef CONFIG_PORT_MIRROR
static inline void smux_mirror_pkt(struct sk_buff *skb, 
					const struct smux_dev_info *dev_info, const int flag)
{
	struct sk_buff *skb2;
	//AUG_DBG("the dev_info->port_mirror is %d\n", dev_info->port_mirror);
	if ((skb2 = skb_clone(skb, GFP_ATOMIC)) != NULL) 
	{		
		skb2->dev = dev_info->mirror_dev;

		if(IN == flag)
			skb_push(skb2, ETH_HLEN);
			
		//AUG_DBG("the dev_info->mirror_dev is %s\n", dev_info->mirror_dev->name);
		nic_tx_mirror(skb2);
	}
}
#endif
module_init(smux_drv_init);
module_exit(smux_drv_exit);

EXPORT_SYMBOL(rtl_smux_pkt_recv);


