#ifndef	RTL_IGMP_PS_HOOKS_H
#define	RTL_IGMP_PS_HOOKS_H		1

#include <net/rtl/rtl_types.h>
#include <linux/mroute.h>

/*macro*/
#define MULTICAST_MAC(mac) 	   ((mac[0]==0x01)&&(mac[1]==0x00)&&(mac[2]==0x5e))
#define DEBUG_PRINT(fmt, args...)
//#define DBG_ICMPv6
#define MCAST_TO_UNICAST
#define IGMP_EXPIRE_TIME (260*HZ)
#define M2U_DELAY_DELETE_TIME (10*HZ)
#if defined (CONFIG_RTL_MLD_SNOOPING)
#define IPV6_MULTICAST_MAC(mac) ((mac[0]==0x33)&&(mac[1]==0x33) && mac[2]!=0xff)
#if defined (MCAST_TO_UNICAST)
#define IPV6_MCAST_TO_UNICAST
#endif
#define IN6_IS_ADDR_MULTICAST(a) (((__u8 *) (a))[0] == 0xff)
#endif /*CONFIG_RTL_MLD_SNOOPING*/

#define MCAST_QUERY_INTERVAL 30

#if defined (CONFIG_RTL_QUERIER_SELECTION)
#define MAX_QUERIER_RECORD 8
#define QUERIER_EXPIRED_TIME 255
#define HOP_BY_HOP_OPTIONS_HEADER 0
#define ROUTING_HEADER 43
#define FRAGMENT_HEADER 44
#define DESTINATION_OPTION_HEADER 60
#define NO_NEXT_HEADER 59
#define ICMP_PROTOCOL 58
#define IPV4_ROUTER_ALTER_OPTION 0x94040000
#define IPV6_ROUTER_ALTER_OPTION 0x05020000
#define IPV6_HEADER_LENGTH 40
#define MLD_QUERY 130
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143
#define S_FLAG_MASK 0x08
#endif

#define MAX_UNKNOWN_MULTICAST_NUM 16
//#define MAX_UNKNOWN_MULTICAST_PPS 1500
#define BLOCK_UNKNOWN_MULTICAST 1

/*struct*/
#if defined (CONFIG_RTL_QUERIER_SELECTION)
struct querierInfo
{
	unsigned int version;
	unsigned char devName[IFNAMSIZ];
	unsigned int querierIp[4];
	unsigned int lastJiffies;
};
#endif


struct rtl865x_unKnownMCastRecord
{
	unsigned int groupAddr;
	unsigned long lastJiffies;
	unsigned long pktCnt;
	unsigned int valid;
};

#if defined(CONFIG_RTL_IGMP_PROXY)
struct mr_table {
	struct list_head	list;
#ifdef CONFIG_NET_NS
	struct net		*net;
#endif
	u32			id;
	struct sock __rcu	*mroute_sk;
	struct timer_list	ipmr_expire_timer;
	struct list_head	mfc_unres_queue;
	struct list_head	mfc_cache_array[MFC_LINES];
	struct vif_device	vif_table[MAXVIFS];
	int			maxvif;
	atomic_t		cache_resolve_queue_len;
	bool			mroute_do_assert;
	bool			mroute_do_pim;
#if defined(CONFIG_IP_PIMSM_V1) || defined(CONFIG_IP_PIMSM_V2)
	int			mroute_reg_vif_num;
#endif
};
#endif

/*prototype*/
void rtl_br_forward(const struct net_bridge_port *to, struct sk_buff *skb);
int rtl_should_deliver(const struct net_bridge_port *p, const struct sk_buff *skb);
void __rtl_br_deliver(const struct net_bridge_port *to, struct sk_buff *skb);
void __rtl_br_forward(const struct net_bridge_port *to, struct sk_buff *skb);
int32 rtl_configMulticastSnoopingFastLeave(int enableFastLeave, int ageTime);
int rtl_M2UDeletecheck(unsigned char *dMac, unsigned char *sMac);
void rtl_igmp_fdb_delete(struct net_bridge *br, struct net_bridge_fdb_entry *f);
#if defined (CONFIG_RTL_MLD_SNOOPING)
int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h);
#endif
int32 rtl_getGroupNum(uint32 ipVersion);
#if defined (CONFIG_NETFILTER)
extern unsigned int (*IgmpRxFilter_Hook)(struct sk_buff *skb,
	     unsigned int hook,
	     const struct net_device *in,
	     const struct net_device *out,
	     struct xt_table *table);
#endif
int rtl_check_ReservedMCastAddr(uint32 groupAddr);
#if defined(CONFIG_RTL_IGMP_PROXY)
struct mr_table *rtl_ipmr_get_table(struct net *net, u32 id);
struct mr_table *rtl_ipmr_rt_fib_lookup(struct net *net, struct sk_buff *skb);
struct mfc_cache *rtl_ipmr_cache_find(struct mr_table *mrt, __be32 origin, __be32 mcastgrp);
int rtl_ipmr_find_vif(struct mr_table *mrt, struct net_device *dev);
#endif


void rtl_IgmpSnooping_ProcCreate_hook(void);
void rtl_IgmpSnooping_ProcDestroy_hook(void);
int rtl_IgmpSnooping_BrXmit_Hook(struct net_bridge * br, const unsigned char * dest, struct sk_buff * skb);
void rtl_igmp_fdb_delete_hook(struct net_bridge *br, struct net_bridge_fdb_entry *f);
void rtl_igmp_fdb_create_hook(struct net_bridge_fdb_entry *fdb);
void rtl_IgmpSnooping_BrInit_Hook(const char *name,struct net_device *dev);
void rtl_IgmpSnooping_BrDeinit_Hook(const char *name, int ret);
int rtl_IgmpSnooping_Input_Hook(struct net_bridge *br,struct sk_buff *skb,struct sk_buff *skb2);
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
int rtl865x_HwMcast_Setting_HOOK(struct net_bridge *br, struct net_device *dev); 
int rtl_processAclForIgmpSnooping(int aclEnabled);

#endif
struct net_bridge_port * rtl_maybe_deliver(struct net_bridge_port *prev, struct net_bridge_port *p,struct sk_buff *skb,void (*__packet_hook)(const struct net_bridge_port *p,struct sk_buff *skb));
int rtl_deliver_clone(const struct net_bridge_port *prev,struct sk_buff *skb, void (*__packet_hook)(const struct net_bridge_port *p, struct sk_buff *skb));


/*inline*/
static inline bool is_allF_ether_addr(const u8 *addr)
{
    return ((addr[0]==0xff) && (addr[1]==0xff) && (addr[2]==0xff) && (addr[3]==0xff) && (addr[4]==0xff) && (addr[5]==0xff));
}
static inline bool is_allZero_ether_addr(const u8 *addr)
{
	return ((addr[0]==0xff) && (addr[1]==0xff) && (addr[2]==0xff) && (addr[3]==0xff) && (addr[4]==0xff) && (addr[5]==0xff));
}
static inline bool is_igmp_valid_eth_addr(const u8 *addr)
{
	return !is_allF_ether_addr(addr) && !is_allZero_ether_addr(addr);
}


/*for test*/
//#define IGMP_DEBUG
//#define TEST_PACKETS(mac) (mac[0]==0x01 && mac[1]==0x00 && mac[2]==0x5e && mac[3]==0x01 && mac[4]==0x02 && mac[5]==0x03)
//#define TEST_V6PACKETS(mac)(mac[0]==0x33 && mac[1]==0x33 && mac[2]==0x00 && mac[3]==0x00 && mac[4]==0x00 && mac[5]==0x01)

#endif
