#ifndef __RTL_NF_H
#define __RTL_NF_H

#if defined(CONFIG_RTL_USB_IP_HOST_SPEEDUP)
struct rnf_ops {
	int (*local_in)(uint8_t pf, unsigned int hook, struct sk_buff *skb,
				struct net_device *in, struct net_device *out,
				int (*okfn)(struct sk_buff *));
	int (*local_out)(u_int8_t pf, unsigned int hook, struct sk_buff *skb,
				  struct net_device *in, struct net_device *out,
				  int (*okfn)(struct sk_buff *));
	int (*pre_routing)(uint8_t pf, unsigned int hook, struct sk_buff *skb,
				struct net_device *in, struct net_device *out,
				int (*okfn)(struct sk_buff *));
	int (*post_routing)(uint8_t pf, unsigned int hook, struct sk_buff *skb,
		     struct net_device *in, struct net_device *out,
		     int (*okfn)(struct sk_buff *), bool cond);
	int (*to_local)(struct sk_buff *skb);
};

extern struct rnf_ops *rnf_hook;
#endif /* #if defined(CONFIG_RTL_USB_IP_HOST_SPEEDUP) */

#endif /*__RTL_NF_H*/
