#ifndef _RTL_MULTI_WAN_H_
#define _RTL_MULTI_WAN_H_

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/version.h>

/* found in socket.c */
extern void smux_ioctl_set(int (*hook)(void __user *));
extern int rtl_get_vid_of_smux_dev(char *name);
extern int rtl_smux_upstream_port_mapping_check(struct sk_buff *skb);
extern int rtl_smux_downstream_port_mapping_check(struct sk_buff *skb);
extern struct net_device * rtl_get_smux_dev(struct sk_buff *skb);
extern unsigned int rtl_get_member_of_smux_dev_by_vid(unsigned int vid);


#ifndef VLAN_VID_MASK
#define VLAN_VID_MASK	0xfff
#endif
#define VLAN_1P_MASK 	0xe000


/* smux device info in net_device. */
struct smux_dev_info {
	struct smux_group *smux_grp;
	struct net_device *vdev;
	struct net_device_stats stats; 
	int    proto;
	int    vid;		/* -1 means vlan disable */
	int    napt;
	int    brpppoe;
	unsigned int    m_1p;
	unsigned int    member;	/* for port mapping */
	struct list_head  list;

#ifdef CONFIG_PORT_MIRROR
	int port_mirror;
	struct net_device* mirror_dev;
	
#define OUT_MIR_MASK			0x1
#define IN_MIR_MASK			0x2
#define OUT_NEED_MIR(val) ((val)  & (OUT_MIR_MASK)) 
#define IN_NEED_MIR(val)  ((val)  & (IN_MIR_MASK)) 

#endif
#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
	int igmpproxy_enable;
	int on_phy;
#endif
};

/* represents a group of smux devices */
struct smux_group {
	struct net_device	*real_dev;
	struct list_head	smux_grp_devs;	
	struct list_head	virtual_devs;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
#define SMUX_DEV_INFO(x) ((struct smux_dev_info *)(netdev_priv(x)))
#else
#define SMUX_DEV_INFO(x) ((struct smux_dev_info *)(x->priv))
#endif

/* inline functions */

static inline struct net_device_stats *smux_dev_get_stats(struct net_device *dev)
{
  return &(SMUX_DEV_INFO(dev)->stats);
}

/* SMUX IOCTLs are found in sockios.h */

/* Passed in smux_ioctl_args structure to determine behaviour. Should be same as busybox/networking/smuxctl.c  */
enum smux_ioctl_cmds {
	ADD_SMUX_CMD,
	REM_SMUX_CMD,
};

enum smux_proto_types {
	SMUX_PROTO_PPPOE,
	SMUX_PROTO_IPOE,
	SMUX_PROTO_BRIDGE
};

/* 
 * for vlan device, smux dev name is nas0.VID, others' name is nas0_No
 */
struct smux_ioctl_args {
	int cmd; /* Should be one of the smux_ioctl_cmds enum above. */
	int proto;
	int vid; /* vid==-1 means vlan disabled on this dev. */
	int napt;
	int brpppoe;
	char ifname[IFNAMSIZ];
	union {
		char ifname[IFNAMSIZ]; /* smux device info */
	} u;
#ifdef CONFIG_RTL_IGMP_PROXY_MULTIWAN
	int igmpproxy_enable;
	int on_phy;
#endif
};
#define rsmux_ifname	ifname
#define osmux_ifname	u.ifname

static inline void TOKEN_NUM(char *inputStr, int *outInt)
{
    *outInt = inputStr[strlen(inputStr)-1]-'0';
   // printk("port :%d\n",*outInt);
}

#endif /* _RTL_MULTI_WAN_H_ */

