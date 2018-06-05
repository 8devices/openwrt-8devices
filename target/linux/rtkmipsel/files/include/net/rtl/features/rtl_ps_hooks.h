#ifndef	RTL_PS_HOOKS_H
#define	RTL_PS_HOOKS_H		1

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nf_conntrack_common.h>

#include <linux/netfilter/nf_conntrack_tcp.h>

#include <net/rtl/rtl_types.h>
#include <net/ip_fib.h>
#include <net/ip_vs.h>

#if defined(CONFIG_RTL_AP_PACKAGE) && !defined(CONFIG_NETFILTER)
#else
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_helper.h>
#endif

#include <net/rtl/features/rtl_features.h>
#if defined(CONFIG_RTL_FASTBRIDGE)
#include <net/rtl/features/fast_bridge.h>
#endif
#include "../../../net/bridge/br_private.h"
#if defined (CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_OPENWRT_SDK)
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl865x_outputQueue.h>
#endif

#define	RTL_PS_HOOKS_CONTINUE	0		/*	keep the process flow	*/
#define	RTL_PS_HOOKS_BREAK		1		/*	should break from caller's loops	*/
#define	RTL_PS_HOOKS_RETURN		2		/*	should immediately return from the caller function	*/
#define	RTL_PS_HOOKS_DROP		3
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
extern int extPortEnabled;
#ifndef PKTHDR_EXTPORT_P3
#define 	PKTHDR_EXTPORT_P3 8
#endif
#endif

typedef struct {
	struct net				*net;
	struct nf_conn				*ct;
	struct sk_buff 			*skb;
	struct nf_conntrack_l3proto *l3proto;
	struct nf_conntrack_l4proto *l4proto;
	u_int8_t 					protonum;
	u_int8_t 					pf;
	unsigned int 				hooknum;
	enum ip_conntrack_info		ctinfo;
	enum tcp_conntrack 		new_state;
	enum tcp_conntrack 		old_state;
}	rtl_nf_conntrack_inso_s;

int32 rtl_nf_conntrack_in_hooks(rtl_nf_conntrack_inso_s *info);
int32 rtl_nf_conntrack_death_by_timeout_hooks(rtl_nf_conntrack_inso_s *info);
int32 rtl_nf_conntrack_destroy_hooks(rtl_nf_conntrack_inso_s *info);
int32 rtl_nf_conntrack_confirm_hooks(rtl_nf_conntrack_inso_s *info);
int32 rtl_nf_init_conntrack_hooks(rtl_nf_conntrack_inso_s *info);
int32 rtl_nf_conntrack_init_hooks(void);
int32 rtl_tcp_packet_hooks(rtl_nf_conntrack_inso_s *info);
int32 rtl_nf_nat_packet_hooks(rtl_nf_conntrack_inso_s *info);
int32 rtl_nat_init_hooks(void);
int32 rtl_nat_cleanup_hooks(void);

int32 rtl_fn_hash_insert_hooks(struct fib_table *tb, struct fib_config *cfg, struct fib_info *fi);
int32 rtl_fn_hash_delete_hooks(struct fib_table *tb, struct fib_config *cfg);
int32 rtl_fn_flush_list_hooks(int	 fz_order, int idx, u32 tb_id, u32 fn_key);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
int32 rtl_fib_flush_list_hooks(u32 tb_id, u32 fn_key, u32 ip_mask);
#endif
int32 rtl_fn_hash_replace_hooks(struct fib_table *tb, struct fib_config *cfg, struct fib_info *fi);

#if defined(CONFIG_OPENWRT_SDK)
#if defined(CONFIG_IPV6) && defined(CONFIG_RTL_8198C)
int32 rtl8198c_fib6_add_hooks(struct rt6_info *rt);
int32 rtl8198c_fib6_del_hooks(struct rt6_info *rt);
#endif
#else
int32 rtl8198c_fib6_add_hooks(void *rt);
int32 rtl8198c_fib6_del_hooks(void *rt);
#endif

int32 rtl_dev_queue_xmit_hooks(struct sk_buff *skb, struct net_device *dev);
int32 rtl_dev_hard_start_xmit_hooks(struct sk_buff *skb, struct net_device *dev, struct netdev_queue *txq);
int32 rtl_netif_receive_skb_hooks(struct sk_buff **pskb);

int32 rtl_br_dev_queue_push_xmit_before_xmit_hooks(struct sk_buff *skb);

#ifdef CONFIG_NET_SCHED

extern int gQosEnabled; 
#endif

int32 rtl_neigh_forced_gc_hooks(struct neigh_table *tbl, struct neighbour *n);
int32 rtl_neigh_flush_dev_hooks(struct neigh_table *tbl, struct net_device *dev, struct neighbour *n);
int32 rtl_neigh_destroy_hooks(struct neighbour *n);
int32 rtl_neigh_connect_hooks(struct neighbour *neigh);
int32 rtl_neigh_timer_handler_pre_update_hooks(struct neighbour *neigh, unsigned state);
int32 rtl_neigh_timer_handler_during_update_hooks(struct neighbour *neigh, unsigned state);
int32 rtl_neigh_timer_handler_post_update_hooks(struct neighbour *neigh, unsigned state);
int32 rtl___neigh_event_send_pre_hooks(struct neighbour *neigh, struct sk_buff *skb);
int32 rtl___neigh_event_send_post_hooks(struct neighbour *neigh, struct sk_buff *skb);
int32 rtl_neigh_update_hooks(struct neighbour *n, const u8 *lladdr, uint8 old);
int32 rtl_neigh_update_post_hooks(struct neighbour *n, const u8 *lladdr, uint8 old);
int32  rtl_neigh_periodic_timer_hooks(struct neighbour *n,  unsigned int  refresh);
int32 rtl_neigh_init_hooks(void);
#if defined(CONFIG_BRIDGE)
int32 rtl___br_fdb_get_timeout_hooks(struct net_bridge *br, struct net_bridge_fdb_entry *fdb, const unsigned char *addr);
#endif
int32 rtl_translate_table_hooks(const char *name,

		unsigned int valid_hooks,
		struct xt_table_info *newinfo,
		void *entry0,
		unsigned int size,
		unsigned int number,
		const unsigned int *hook_entries,
		const unsigned int *underflows);

int32 rtl_ip_tables_init_hooks(void);

int32 rtl_masq_device_event_hooks(struct notifier_block *this, struct net_device *dev,  unsigned long event);
int32 rtl_masq_inet_event_hooks(struct notifier_block *this, unsigned long event, void *ptr);

#if defined(CONFIG_IP_VS)
int32 rtl_ip_vs_conn_expire_hooks1(struct ip_vs_conn *cp);
int32 rtl_ip_vs_conn_expire_hooks2(struct ip_vs_conn *cp);
#endif

#if defined(CONFIG_IP_VS_PROTO_TCP)
int32 rtl_tcp_state_transition_hooks(struct ip_vs_conn *cp, int direction, const struct sk_buff *skb, struct ip_vs_protocol *pp);
#endif

#if defined(CONFIG_IP_VS_PROTO_UDP)
int32 rtl_udp_state_transition_hooks(struct ip_vs_conn *cp, int direction, const struct sk_buff *skb, struct ip_vs_protocol *pp);
#endif

#ifdef CONFIG_PROC_FS
int rtl_ct_seq_show_hooks(struct seq_file *s, struct nf_conn *ct);
#endif
#if 0
int rtl_get_unique_tuple_hooks(struct nf_conntrack_tuple *tuple,
		 const struct nf_conntrack_tuple *orig_tuple,
		 const struct nf_nat_range *range,
		 struct nf_conn *ct,
		 enum nf_nat_manip_type maniptype,
		 struct nf_nat_protocol *proto);

#endif

int32 rtl_dst_alloc_gc_pre_check_hooks(struct dst_ops * ops);
int32 rtl_dst_alloc_gc_post_check1_hooks(struct dst_ops * ops);
int32 rtl_dst_alloc_gc_post_check2_hooks(struct dst_ops * ops, struct dst_entry * dst);

#if defined(CONFIG_RTL_NF_CONNTRACK_GARBAGE_NEW)
int32 clean_from_lists_hooks(struct nf_conn *ct, struct net *net);
int32 __nf_ct_refresh_acct_proto_hooks(struct nf_conn *ct, 
					enum ip_conntrack_info ctinfo,
					const struct sk_buff *skb, 
					int do_acct,
					int *event);
int32 __drop_one_conntrack_process_hooks1(struct nf_conn* ct, int dropPrioIdx, int factor, int checkFlags, int tcpUdpState);
int32 __drop_one_conntrack_process_hooks2(struct nf_conn* ct, int dropPrioIdx, int factor, int checkFlags, int tcpUdpState);
int32 rtl_nf_conn_GC_init_hooks(void);
#endif


#if defined(CONFIG_BRIDGE)
int32 rtl_fdb_create_hooks(struct net_bridge_fdb_entry *fdb,const unsigned char *addr);
int32 rtl865x_addAuthFDBEntry_hooks(const unsigned char *addr);
int32 rtl_fdb_delete_hooks(struct net_bridge_fdb_entry *f);
int32 rtl_br_fdb_cleanup_hooks(struct net_bridge *br, struct net_bridge_fdb_entry *f, unsigned long delay);

#endif
#if defined (CONFIG_RTL_HW_QOS_SUPPORT) && defined(CONFIG_OPENWRT_SDK)
int rtl_hwQosSyncTcHook(struct net_device *dev, struct Qdisc  *q,unsigned int parent);

#endif
#if defined (CONFIG_RTL_QOS_SYNC_SUPPORT)


 int rtl_QosSyncTcHook(struct net_device *dev, struct Qdisc  *q,unsigned int parent, int process);
 
#define RTL_QOS_QUEUE_HOOK 	0x1
#define RTL_QOS_CLASS_HOOK 	0x2
#define RTL_QOS_FILTER_HOOK 	0x4
#define RTL_QOS_ADD_PROCESS	0x10
#define RTL_QOS_DEL_PROCESS	0x20

#endif
#endif

//#define RTL_UPDATE_PS_NEIGH_CONFIRM 1


