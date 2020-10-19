/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * ovsmgr_dp.c
 */

#include <asm/atomic.h>
#include <linux/etherdevice.h>

/*
 * OpenVSwitch header files
 */
#include <datapath.h>
#include "ovsmgr.h"
#include "ovsmgr_priv.h"

/*
 * ovsmgr_dp_find()
 *	Find datapath instance.
 *
 * This function should be called under read_lock.
 */
static struct ovsmgr_dp *ovsmgr_dp_find(void *dp)
{
	struct ovsmgr_dp *nod;

	list_for_each_entry(nod, &ovsmgr_ctx.dp_list, node) {
		if (nod->dp == dp) {
			return nod;
		}
	}

	return NULL;
}

/*
 * ovsmgr_dp_port_find_by_dev()
 *	Find datapath instance using dev.
 *
 * This function should be called under read_lock
 */
static struct ovsmgr_dp_port *ovsmgr_dp_port_find_by_dev(struct ovsmgr_dp *nod, struct net_device *dev)
{
	struct ovsmgr_dp_port *nodp;

	list_for_each_entry(nodp, &nod->port_list, node) {
		if (nodp->dev == dev) {
			return nodp;
		}
	}

	return NULL;
}

/*
 * ovsmgr_dp_port_find_by_num()
 *	Find datapath port instance using port number and datapath instance.
 *
 * This function should be called under read_lock
 */
static struct ovsmgr_dp_port *ovsmgr_dp_port_find_by_num(struct ovsmgr_dp *nod, int vport_num)
{
	struct ovsmgr_dp_port *nodp;

	list_for_each_entry(nodp, &nod->port_list, node) {
		if (nodp->vport_num == vport_num) {
			return nodp;
		}
	}

	return NULL;
}

/*
 * ovsmgr_dp_find_by_port()
 *	Find datapath instance using port number.
 *
 * This function should be called under read_lock
 */
static struct ovsmgr_dp *ovsmgr_dp_find_by_port(int vport_num)
{
	struct ovsmgr_dp_port *nodp;
	struct ovsmgr_dp *nod;

	list_for_each_entry(nod, &ovsmgr_ctx.dp_list, node) {
		nodp = ovsmgr_dp_port_find_by_num(nod, vport_num);
		if (nodp) {
			return nod;
		}
	}

	return NULL;
}

/*
 * ovsmgr_dp_find_by_dev()
 *	Find datapath instance using dev.
 *
 * This function should be called under read_lock
 */
static struct ovsmgr_dp *ovsmgr_dp_find_by_dev(struct net_device *dev)
{
	struct ovsmgr_dp_port *nodp;
	struct ovsmgr_dp *nod;

	list_for_each_entry(nod, &ovsmgr_ctx.dp_list, node) {
		nodp = ovsmgr_dp_port_find_by_dev(nod, dev);
		if (nodp) {
			return nod;
		}
	}

	return NULL;
}

/*
 * ovsmgr_dp_send_ports_add_notifier
 *	Notify port add event
 *
 * Send port add notifiers Return the first output interface in the actions.
 */
static void ovsmgr_dp_send_ports_add_notifier(struct ovsmgr_dp *nod, struct net_device *master_dev)
{
	struct ovsmgr_dp_port *nodp;
	struct ovsmgr_dp_port_info port;
	struct ovsmgr_notifiers_info info;

	list_for_each_entry(nodp, &nod->port_list, node) {
		if (!strncmp(nodp->master_name, master_dev->name, IFNAMSIZ) &&
			    !nodp->add_notified) {
			nodp->add_notified = true;
			nodp->master_dev = master_dev;
			port.master = master_dev;
			port.dev = nodp->dev;
			info.port = &port;
			ovsmgr_notifiers_call(&info, OVSMGR_DP_PORT_ADD);
		}
	}
}

/*
 * ovsmgr_dp_flow_key_dump()
 *	Dump flow key
 */
static void ovsmgr_dp_flow_key_dump(struct sw_flow_key *key)
{
	int i;

	ovsmgr_trace("Phy:\n");
	ovsmgr_trace("\tpriority = %u\n", key->phy.priority);
	ovsmgr_trace("\tskb_mark = %u\n", key->phy.skb_mark);
	ovsmgr_trace("\tin_port = %u\n", key->phy.in_port);

	ovsmgr_trace("Flow Hash = %u\n", key->ovs_flow_hash);
	ovsmgr_trace("Recirc_id = %u\n", key->recirc_id);
	ovsmgr_trace("Eth:\n");
	ovsmgr_trace("\tsmac = %pM\n", &key->eth.src);
	ovsmgr_trace("\tdmac = %pM\n", &key->eth.dst);
	ovsmgr_trace("\ttype = %x\n", key->eth.type);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
	ovsmgr_trace("\tcvlan tci = %x\n", OVSMGR_KEY_CVLAN_TCI(key));
#endif
	ovsmgr_trace("\tvlan tci = %x\n", OVSMGR_KEY_VLAN_TCI(key));

	ovsmgr_trace("IP:\n");
	ovsmgr_trace("\tproto = %d\n", key->ip.proto);
	ovsmgr_trace("\ttos = %d\n", key->ip.tos);
	ovsmgr_trace("\tttl = %d\n", key->ip.ttl);

	ovsmgr_trace("\tfrag = %d\n", key->ip.frag);

	ovsmgr_trace("TCP/UDP:\n");
	ovsmgr_trace("\tsrc = %x\n", key->tp.src);
	ovsmgr_trace("\tdst = %x\n", key->tp.dst);
	ovsmgr_trace("\tflags = %x\n", key->tp.flags);

	if (key->eth.type == htons(ETH_P_IP)) {
		ovsmgr_trace("IPv4:\n");
		ovsmgr_trace("\tsrc ip = %pI4\n", &key->ipv4.addr.src);
		ovsmgr_trace("\tdst ip = %pI4\n", &key->ipv4.addr.dst);
	} else if (key->eth.type == htons(ETH_P_IPV6)) {
		ovsmgr_trace("IPv6:\n");
		ovsmgr_trace("\tsrc ip = %pI6\n", &key->ipv6.addr.src);
		ovsmgr_trace("\tdst ip = %pI6\n", &key->ipv6.addr.dst);
	} else {
		ovsmgr_trace("Eth type = %x\n", key->eth.type);
	}

	ovsmgr_trace("CT:\n");
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
	ovsmgr_trace("\tzone = %x\n", key->ct.zone);
	ovsmgr_trace("\tstate = %x\n", key->ct.state);
#else
	ovsmgr_trace("\tzone = %x\n", key->ct_zone);
	ovsmgr_trace("\tstate = %x\n", key->ct_state);
#endif

	ovsmgr_trace("\tmark = %x\n", key->ct.mark);
	ovsmgr_trace("\tlabel: ");
	for (i = 0 ; i < OVS_CT_LABELS_LEN ; i++) {
		ovsmgr_trace("%x ", key->ct.labels.ct_labels[i]);
	}
}

/*
 * ovsmgr_dp_flow_mask_dump()
 *	Dump flow mask
 */
static void ovsmgr_dp_flow_mask_dump(struct sw_flow_mask *mask)
{
	ovsmgr_trace("ref_count = %d\n", mask->ref_count);
	ovsmgr_trace("Range:\n");
	ovsmgr_trace("\tstart = %d\n", mask->range.start);
	ovsmgr_trace("\tend = %d\n", mask->range.end);
}

/*
 * ovsmgr_dp_flow_actions_dump()
 *	Dump flow actions
 */
static void ovsmgr_dp_flow_actions_dump(struct sw_flow_actions *acts)
{
	const struct nlattr *a;
	int rem;

	for (a = acts->actions, rem = acts->actions_len; rem > 0;
	     a = nla_next(a, &rem)) {

		switch (nla_type(a)) {
		case OVS_ACTION_ATTR_OUTPUT:
			ovsmgr_trace("OVS_ACTION_ATTR_OUTPUT: Port = %d\n", nla_get_u32(a));
			break;

		case OVS_ACTION_ATTR_USERSPACE:
			ovsmgr_trace("OVS_ACTION_ATTR_USERSPACE\n");
			break;

		case OVS_ACTION_ATTR_HASH:
			ovsmgr_trace("OVS_ACTION_ATTR_HASH\n");
			break;

		case OVS_ACTION_ATTR_PUSH_MPLS:
			ovsmgr_trace("OVS_ACTION_ATTR_PUSH_MPLS\n");
			break;

		case OVS_ACTION_ATTR_POP_MPLS:
			ovsmgr_trace("OVS_ACTION_ATTR_POP_MPLS\n");
			break;

		case OVS_ACTION_ATTR_PUSH_VLAN: {
			const struct ovs_action_push_vlan *vlan;

			ovsmgr_trace("OVS_ACTION_ATTR_PUSH_VLAN\n");
			vlan = nla_data(a);
			ovsmgr_trace("Vlan details:\n");
			ovsmgr_trace("\tvlan_tpid = %x\n", vlan->vlan_tpid);
			ovsmgr_trace("\tvlan_tci = %x\n", vlan->vlan_tci);
			break;
		}

		case OVS_ACTION_ATTR_POP_VLAN:
			ovsmgr_trace("OVS_ACTION_ATTR_POP_VLAN\n");
			break;

		case OVS_ACTION_ATTR_RECIRC:
			ovsmgr_trace("OVS_ACTION_ATTR_RECIRC\n");
			break;

		case OVS_ACTION_ATTR_SET:
			ovsmgr_trace("OVS_ACTION_ATTR_SET\n");
			break;

		case OVS_ACTION_ATTR_SET_MASKED:
			ovsmgr_trace("OVS_ACTION_ATTR_SET_MASKED\n");
			break;

		case OVS_ACTION_ATTR_SET_TO_MASKED:
			ovsmgr_trace("OVS_ACTION_ATTR_SET_TO_MASKED\n");
			break;

		case OVS_ACTION_ATTR_SAMPLE:
			ovsmgr_trace("OVS_ACTION_ATTR_SAMPLE\n");
			break;

		case OVS_ACTION_ATTR_CT:
			ovsmgr_trace("OVS_ACTION_ATTR_CT\n");
			break;
		default:
			ovsmgr_trace("Unknown action: %d\n", nla_type(a));
		}
	}
}

/*
 * ovsmgr_dp_flow_dump()
 *	Dump flow details
 */
static void ovsmgr_dp_flow_dump(struct sw_flow *flow)
{
	struct sw_flow_actions *acts;

	ovsmgr_trace("------- Flow Key --------\n");
	ovsmgr_dp_flow_key_dump(&flow->key);

	ovsmgr_trace("------- Flow Mask --------\n");
	ovsmgr_dp_flow_mask_dump(flow->mask);

	acts = rcu_dereference(flow->sf_acts);
	ovsmgr_trace("------- Flow Actions ------\n");
	ovsmgr_dp_flow_actions_dump(acts);
}

/*
 * ovsmgr_dp_mcast_port_dev_find()
 *	Find egress datapath port, given skb and datapath interface (dev)
 */
static struct net_device *ovsmgr_dp_mcast_port_dev_find(struct ovsmgr_dp *nod,
							    struct ovsmgr_dp_port *master,
							    struct ovsmgr_dp_flow *flow, struct sk_buff *skb)
{
	struct ovsmgr_dp_port *p;
	struct sw_flow_key key;
	struct net_device *dev;

	/*
	 * For multicast flow there is only one flow rule in datapath.  In case
	 * of multicast routing flow there is no flow from OVS bridge interface
	 * to bridge port.  The only way is to search for all the ports that are
	 * part of bridge.
	 *
	 * Create a originate flow key
	 */
	memset(&key, 0, sizeof(key));
	ether_addr_copy(key.eth.src, flow->smac);
	ether_addr_copy(key.eth.dst, flow->dmac);

	if (flow->tuple.ip_version == 4) {
		key.ipv4.addr.src = flow->tuple.ipv4.src;
		key.ipv4.addr.dst = flow->tuple.ipv4.dst;
		key.eth.type = htons(ETH_P_IP);
	} else {
		memcpy(&key.ipv6.addr.src, &flow->tuple.ipv6.src, sizeof(key.ipv6.addr.src));
		memcpy(&key.ipv6.addr.dst, &flow->tuple.ipv6.dst, sizeof(key.ipv6.addr.dst));
		key.eth.type = htons(ETH_P_IPV6);
	}

	key.tp.src = flow->tuple.src_port;
	key.tp.dst = flow->tuple.dst_port;
	key.ip.proto = flow->tuple.protocol;

	read_lock(&ovsmgr_ctx.lock);
	list_for_each_entry(p, &nod->port_list, node) {
		/*
		 * Skip search if the port is not part of same bridge
		 */
		if (p->master_dev != master->dev) {
			continue;
		}

		key.phy.in_port = p->vport_num;
		dev = ovs_accel_egress_dev_find(nod->dp, &key, skb);
		if (dev) {
			read_unlock(&ovsmgr_ctx.lock);
			ovsmgr_info("%px: Found the egress interface for this flow, returning ingress port: %s\n",
					nod, p->dev->name);
			return p->dev;
		}
	}

	read_unlock(&ovsmgr_ctx.lock);
	return NULL;
}

/*
 * ovsmgr_dp_flow_can_accel()
 *	Validate flow and actions and see if flow can be accelerated
 */
static bool ovsmgr_dp_flow_can_accel(struct sw_flow *sf, struct sw_flow_key *key,
					 struct sw_flow_actions *sfa, struct sk_buff *skb)
{
	struct ovsmgr_dp_port *nodp;
	struct ovsmgr_dp *nod;
	const struct nlattr *a;
	int port_num, rem;
	int egress_cnt = 0;

	/*
	 * Following are the checks done:
	 *	1. Only TCP and UDP packets are accelerated
	 *	2. Ingress port shouldn't be internal port (bridge interface)
	 *	3. Parse supported actions
	 */

	/*
	 * 1. Allow specific IP protocols
	 */
	switch (key->ip.proto) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
		break;
	default:
		ovsmgr_warn("%px: IP protocol is not supported, proto=%d\n", sf, key->ip.proto);
		return false;
	}

	/*
	 * 2. Do not allow incoming routed packets, ingress
	 *    interface shouldn't be bridge interface
	 */
	port_num = key->phy.in_port;
	read_lock(&ovsmgr_ctx.lock);

	nod = ovsmgr_dp_find_by_port(port_num);
	if (!nod) {
		read_unlock(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: datapath instance is not found, ingress port_num = %d\n", sf, port_num);
		return false;
	}

	nodp = ovsmgr_dp_port_find_by_num(nod, port_num);
	if (!nodp) {
		read_unlock(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: datapath port instance is not found, ingress port_num = %d\n", sf, port_num);
		return false;
	}

	/*
	 * Do not accelerate packets received from OVS bridge interface.
	 * These are routed flows.
	 */
	if (nodp->vport_type == OVS_VPORT_TYPE_INTERNAL) {
		read_unlock(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: ingress port(%d:%s): incoming routed flows not forwarded through hook.\n",
				sf, port_num, nodp->dev->name);
		return false;
	}
	read_unlock(&ovsmgr_ctx.lock);

	for (a = sfa->actions, rem = sfa->actions_len; rem > 0;
			a = nla_next(a, &rem)) {
		/*
		 * 3. Allow specific actions
		 *	- Allow multiple output actions (multiple egress ports) only for multicast
		 *	  flow.
		 *	- Allow specific actions, if any other action is set for
		 *	  this flow then do not accelerate.
		 */
		switch (nla_type(a)) {
		case OVS_ACTION_ATTR_OUTPUT: {

			port_num = nla_get_u32(a);

			read_lock(&ovsmgr_ctx.lock);
			nodp = ovsmgr_dp_port_find_by_num(nod, port_num);
			if (!nodp) {
				read_unlock(&ovsmgr_ctx.lock);
				ovsmgr_warn("%px: datapath port instance is not found, port_num = %d\n", sf, port_num);
				return false;
			}

			if (nodp->vport_type == OVS_VPORT_TYPE_INTERNAL) {
				read_unlock(&ovsmgr_ctx.lock);
				ovsmgr_warn("%px: output port(%d:%s): egress routed flows not forwarded through hook.\n",
						sf, port_num, nodp->dev->name);
				return false;
			}

			read_unlock(&ovsmgr_ctx.lock);
			egress_cnt++;
		}
		case OVS_ACTION_ATTR_PUSH_VLAN:
		case OVS_ACTION_ATTR_POP_VLAN:
		case OVS_ACTION_ATTR_CT:
		     break;
		default:
		     ovsmgr_warn("%px: Action is not supported, action=%x\n", sf, nla_type(a));
		     return false;
		}
	}

	/*
	 * Do not accelerate the flow if egress_cnt is more than 1 and
	 * destination IP is unicast.  This could be a broadcast flow rule
	 * to find the egress for destination port.
	 */
	if ((egress_cnt > 1) && (skb->pkt_type != PACKET_MULTICAST)) {
		ovsmgr_warn("%px: Not multicast packet :egress > 1, type = %d, egress_cnt = %d\n",
				sf, skb->pkt_type, egress_cnt);
		return false;
	}

	return true;
}

/*
 * ovsmgr_dp_flow_first_egress_find()
 *	Find the first egress port from flow rule
 */
static int ovsmgr_dp_flow_first_egress_find(struct sw_flow *sf)
{
	struct sw_flow_actions *sfa;
	const struct nlattr *a;
	int rem;

	if (!sf) {
		ovsmgr_warn("flow is null\n");
		return -EINVAL;
	}

	sfa = rcu_dereference(sf->sf_acts);
	if (!sfa) {
		ovsmgr_warn("%px: flow actions is null\n", sf);
		return -EINVAL;
	}

	for (a = sfa->actions, rem = sfa->actions_len; rem > 0;
	     a = nla_next(a, &rem)) {

		if (nla_type(a) == OVS_ACTION_ATTR_OUTPUT) {
			return nla_get_u32(a);
		}
	}

	return -EINVAL;
}

/*
 * ovsmgr_dp_flow_egress_find()
 *	Return the first output interface in the actions
 */
static struct net_device *ovsmgr_dp_flow_egress_find(void *dp, struct sw_flow *sf)
{
	int port_num;

	port_num = ovsmgr_dp_flow_first_egress_find(sf);
	if (unlikely(port_num < 0)) {
		return NULL;
	}

	return ovs_accel_dev_find(dp, port_num);
}

/*
 * ovsmgr_dp_flow_get()
 *	Return the flow tuple from datapath flow rule
 */
static int ovsmgr_dp_flow_get(void *dp, struct sw_flow *sf, struct ovsmgr_dp_flow *flow)
{
	struct sw_flow_key *key = &sf->key;

	memset(flow, 0, sizeof(*flow));

	flow->indev = ovs_accel_dev_find(dp, key->phy.in_port);
	if (!flow->indev) {
		ovsmgr_warn("%px: Input device (indev) is NULL\n", sf);
		return -EINVAL;
	}

	flow->outdev = ovsmgr_dp_flow_egress_find(dp, sf);
	if (!flow->outdev) {
		ovsmgr_warn("%px: Output device (outdev) is NULL\n", sf);
		return -EINVAL;
	}

	ether_addr_copy(flow->smac, key->eth.src);
	ether_addr_copy(flow->dmac, key->eth.dst);

	if (key->eth.type == htons(ETH_P_IP)) {
		memcpy(&flow->tuple.ipv4, &key->ipv4.addr, sizeof(flow->tuple.ipv4));
		flow->tuple.ip_version = 4;
	} else if (key->eth.type == htons(ETH_P_IPV6)) {
		memcpy(&flow->tuple.ipv6, &key->ipv6.addr, sizeof(flow->tuple.ipv6));
		flow->tuple.ip_version = 6;
	} else {
		ovsmgr_warn("%px: Eth type is not accelerated: %x\n", sf, htons(key->eth.type));
		return -EINVAL;
	}

	/*
	 * copy ingress VLAN
	 */
	if (OVSMGR_KEY_VLAN_TCI(key)) {
		flow->ingress_vlan.h_vlan_TCI = ntohs(OVSMGR_KEY_VLAN_TCI(key)) & ~VLAN_TAG_PRESENT;
		ovsmgr_info("%px: ingress VLAN tag is %X\n", sf, flow->ingress_vlan.h_vlan_TCI);
	}

	flow->tuple.src_port = key->tp.src;
	flow->tuple.dst_port = key->tp.dst;
	flow->tuple.protocol = key->ip.proto;

	return 0;
}

/*
 * ovsmgr_dp_pkt_process()
 *	Process datapath packet.
 */
static void ovsmgr_dp_pkt_process(void *dp, struct sk_buff *skb, struct sw_flow_key *key,
		struct sw_flow *sf, struct sw_flow_actions *sfa)
{
	struct ovs_skb_cb ovs_cb;
	struct ovsmgr_dp_hook_ops *ops;
	struct net_device *out;
	bool hook_called = false;

	/*
	 * skb->cb is used internally by OVS datapath.
	 * Save skb->cb before using, restore it back before returning.
	 */
	ovs_cb = *OVS_CB(skb);
	atomic64_inc(&ovsmgr_ctx.stats.pkts_from_ovs_dp);

	/*
	 * Call pre-flow hook if flow is NULL
	 */
	if (!sf) {
		/*
		 * Forward only IGMP/ICMPv6 packets.
		 */
		if (likely(key->ip.proto != IPPROTO_IGMP) && likely(key->ip.proto != IPPROTO_ICMPV6))
			return;

		/*
		 * Pull Ethernet header from skb before forwarding the packet to hook
		 */
		skb_pull(skb, ETH_HLEN);
		list_for_each_entry(ops, &ovsmgr_ctx.dp_hooks, list) {
			if (ops->hook_num == OVSMGR_DP_HOOK_PRE_FLOW_PROC) {
				ops->hook(skb, NULL);
				hook_called = true;
			}
		}

		/*
		 * Restore Ethernet header
		 */
		skb_push(skb, ETH_HLEN);

		if (!hook_called) {
			ovsmgr_warn("%px: Pre flow hook is not registered.\n", dp);
		} else {
			atomic64_inc(&ovsmgr_ctx.stats.pkts_fwd_pre_flow);
		}

		return;
	}

	if (!ovsmgr_dp_flow_can_accel(sf, key, sfa, skb)) {
		ovsmgr_warn("%px: flow cannot be accelerated\n", skb);
		return;
	}

	out = ovsmgr_dp_flow_egress_find(dp, sf);
	if (unlikely(!out)) {
		ovsmgr_warn("%px: egress interface is not found in flow rule - %px\n", skb, sf);
		return;
	}

	/*
	 * Pull Ethernet header from skb before forwarding the packet to hook
	 */
	hook_called = false;
	OVSMGR_OVS_CB(skb)->flow = sf;
	OVSMGR_OVS_CB(skb)->ovs_sig = OVS_SIGNATURE;

	skb_pull(skb, ETH_HLEN);
	list_for_each_entry(ops, &ovsmgr_ctx.dp_hooks, list) {
		if (ops->hook_num == OVSMGR_DP_HOOK_POST_FLOW_PROC) {
			ops->hook(skb, out);
			hook_called = true;
		}
	}

	/*
	 * Restore Ethernet header
	 */
	skb_push(skb, ETH_HLEN);

	if (!hook_called) {
		ovsmgr_warn("%px: Post flow hook is not registered.\n", dp);
	} else {
		atomic64_inc(&ovsmgr_ctx.stats.pkts_fwd_post_flow);
	}

	/*
	 * Restore skb->cb
	 */
	*OVS_CB(skb) = ovs_cb;
}

/*
 * ovsmgr_dp_port_vlan_notify()
 *	Send VLAN add/del notification
 */
static void ovsmgr_dp_port_vlan_notify(void *dp, struct ovsmgr_dp_flow *flow, int event)
{
	struct ovsmgr_dp_port_vlan_info vlan;
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp_port *nodp;
	struct ovsmgr_dp *nod;
	int i;

	read_lock_bh(&ovsmgr_ctx.lock);

	nod = ovsmgr_dp_find(dp);
	if (!nod) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: datapath instance is not found\n", dp);
		return;
	}

	nodp = ovsmgr_dp_port_find_by_dev(nod, flow->indev);
	if (!nodp) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Port is not found, dev = %s\n", dp, flow->indev->name);
		return;
	}

	/*
	 * Check if VLAN is set for the port.
	 */
	for (i = 0; i < OVSMGR_PORT_VLAN_MAX_CNT; i++) {
		if (nodp->vlan[i].h_vlan_TCI == flow->ingress_vlan.h_vlan_TCI) {
			if (event == OVSMGR_DP_VLAN_ADD) {
				/*
				 * VLAN ID is already notified
				 */
				read_unlock_bh(&ovsmgr_ctx.lock);
				return;
			}

			if (event == OVSMGR_DP_VLAN_DEL) {
				vlan.dev = flow->indev;
				vlan.master = nodp->master_dev;
				vlan.vh.h_vlan_TCI = flow->ingress_vlan.h_vlan_TCI;
				info.vlan = &vlan;
				ovsmgr_notifiers_call(&info, OVSMGR_DP_VLAN_DEL);
				nodp->vlan[i].h_vlan_TCI = 0;
				read_unlock_bh(&ovsmgr_ctx.lock);
				return;
			}
		}
	}

	for (i = 0; i < OVSMGR_PORT_VLAN_MAX_CNT; i++) {
		if (!nodp->vlan[i].h_vlan_TCI) {
			nodp->vlan[i].h_vlan_TCI = flow->ingress_vlan.h_vlan_TCI;
			vlan.dev = flow->indev;
			vlan.master = nodp->master_dev;
			vlan.vh.h_vlan_TCI = flow->ingress_vlan.h_vlan_TCI;
			info.vlan = &vlan;
			ovsmgr_notifiers_call(&info, OVSMGR_DP_VLAN_ADD);
			break;
		}
	}

	read_unlock_bh(&ovsmgr_ctx.lock);
}

/*
 * ovsmgr_dp_flow_tbl_flush()
 *	Flow table is flushed in datapath
 */
static void ovsmgr_dp_flow_tbl_flush(void *dp)
{
	ovsmgr_notifiers_call(NULL, OVSMGR_DP_FLOW_TBL_FLUSH);
}

/*
 * ovsmgr_dp_flow_del()
 *	Flow rule is deleted from datapath
 */
static void ovsmgr_dp_flow_del(void *dp, struct sw_flow *sf)
{
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp_flow flow;

	ovsmgr_info("%px: flow del - %px\n", dp, sf);
	ovsmgr_dp_flow_dump(sf);

	if (!ovsmgr_dp_flow_get(dp, sf, &flow)) {
		if (flow.ingress_vlan.h_vlan_TCI) {
			ovsmgr_dp_port_vlan_notify(dp, &flow, OVSMGR_DP_VLAN_DEL);
		}

		info.flow = &flow;
		ovsmgr_notifiers_call(&info, OVSMGR_DP_FLOW_DEL);
	}
}

/*
 * ovsmgr_dp_flow_set()
 *	Flow rule is modified
 */
static void ovsmgr_dp_flow_set(void *dp, struct sw_flow *sf, struct sw_flow_actions *sfa)
{
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp_flow flow;

	ovsmgr_info("%px: flow set - %px:%px\n", dp, sf, sfa);
	ovsmgr_dp_flow_dump(sf);

	if (ovsmgr_dp_flow_get(dp, sf, &flow)) {
		return;
	}

	info.flow = &flow;
	if (is_multicast_ether_addr(flow.dmac)) {
		/*
		 * Multicast flow is updated, send change notifier.
		 */
		ovsmgr_notifiers_call(&info, OVSMGR_DP_FLOW_CHANGE);
	} else {
		/*
		 * Unicast flow is updated, send delete notifier.
		 */
		ovsmgr_notifiers_call(&info, OVSMGR_DP_FLOW_DEL);
	}
}

/*
 * ovsmgr_dp_flow_add()
 *	New flow rule is added in the datapath
 */
static void ovsmgr_dp_flow_add(void *dp, struct sw_flow *sf)
{
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp_flow flow;

	ovsmgr_info("%px: flow add - %px\n", dp, sf);
	ovsmgr_dp_flow_dump(sf);

	if (!ovsmgr_dp_flow_get(dp, sf, &flow)) {
		if (flow.ingress_vlan.h_vlan_TCI) {
			ovsmgr_dp_port_vlan_notify(dp, &flow, OVSMGR_DP_VLAN_ADD);
		}

		info.flow = &flow;
		ovsmgr_notifiers_call(&info, OVSMGR_DP_FLOW_ADD);
	}
}

/*
 * ovsmgr_dp_port_del()
 *	OVS datapath port is deleted
 */
static void ovsmgr_dp_port_del(void *dp, void *vp, struct net_device *dev)
{
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp_port_info port;
	struct ovsmgr_dp_port *nodp;
	struct ovsmgr_dp *nod;
	struct net_device *master_dev;
	bool is_vport_internal = false;

	ovsmgr_info("%px:%px port deleting - dev = %s\n", dp, vp, dev->name);

	write_lock_bh(&ovsmgr_ctx.lock);
	nod = ovsmgr_dp_find(dp);
	if (!nod) {
		write_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: datapath instance is not found\n", dp);
		return;
	}

	nodp = ovsmgr_dp_port_find_by_dev(nod, dev);
	if (!nodp) {
		write_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Port is not found, dev = %s\n", dp, dev->name);
		return;
	}

	master_dev = nodp->master_dev;
	if (nodp->vport_type == OVS_VPORT_TYPE_INTERNAL) {
		is_vport_internal = true;
	}

	/*
	 * Remove port instance
	 */
	list_del(&nodp->node);
	write_unlock_bh(&ovsmgr_ctx.lock);

	kfree(nodp);

	/*
	 * Check if master_dev and dev are same and they are internal port.
	 * These conditions will mean that bridge device is deleted,
	 * send OVSMGR_DP_BR_DEL notifier.
	 */
	if ((master_dev == dev) && is_vport_internal) {
		info.dev = master_dev;
		ovsmgr_notifiers_call(&info, OVSMGR_DP_BR_DEL);
		return;
	}

	port.master = master_dev;
	port.dev = dev;
	info.port = &port;

	ovsmgr_notifiers_call(&info, OVSMGR_DP_PORT_DEL);
}

/*
 * ovsmgr_dp_port_add()
 *	New OVS datapath port is added
 */
static void ovsmgr_dp_port_add(void *dp, void *vp, int vp_num, enum ovs_vport_type vp_type,
				   const char *master, struct net_device *dev)
{
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp_port_info port;
	struct ovsmgr_dp_port *nodp;
	struct ovsmgr_dp *nod;
	struct net_device *master_dev = NULL;

	if (!master) {
		ovsmgr_info("%px: There is no master\n", dp);
	} else {
		ovsmgr_info("%px: Master device name set: %s\n", dp, master);
		master_dev = __dev_get_by_name(&init_net, master);
		if (!master_dev) {
			ovsmgr_warn("%px: Master netdev not found\n", dp);
		}
	}

	ovsmgr_info("%px:%px new port - dev = %s\n", dp, vp, dev->name);

	nodp = kmalloc(sizeof(*nodp), GFP_KERNEL);
	if (!nodp) {
		ovsmgr_warn("%px: Memory allocation failed\n", dp);
		return;
	}

	INIT_LIST_HEAD(&nodp->node);
	nodp->dev = dev;
	nodp->master_dev = master_dev;
	nodp->vport_type = vp_type;
	nodp->vport = vp;
	nodp->vport_num = vp_num;
	nodp->add_notified = false;
	if (master) {
		strlcpy(nodp->master_name, master, IFNAMSIZ);
		ovsmgr_info("%px: Master device name set: %s\n", dp, master);
		master_dev = __dev_get_by_name(&init_net, master);
		if (!master_dev) {
			ovsmgr_warn("%px: Master netdev not found\n", dp);
		}
	}

	write_lock_bh(&ovsmgr_ctx.lock);

	nod = ovsmgr_dp_find(dp);
	if (!nod) {
		write_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: datapath instance is not found\n", dp);
		kfree(nodp);
		return;
	}

	list_add(&nodp->node, &nod->port_list);
	write_unlock_bh(&ovsmgr_ctx.lock);

	/*
	 * OVS bridge interface is also a port in datapath.  It
	 * can be created before or after bridge ports.  So it is
	 * possible that master is set but master_dev is NULL.  Generate
	 * OVSMGR_DP_PORT_ADD notifier if master_dev is set, otherwise
	 * generate when ovs bridge is created.
	 */
	if (master_dev && (vp_type != OVS_VPORT_TYPE_INTERNAL)) {
		port.master = master_dev;
		port.dev = dev;
		info.port = &port;
		ovsmgr_notifiers_call(&info, OVSMGR_DP_PORT_ADD);
		nodp->add_notified = true;
		return;
	}

	/*
	 * If master_dev is same as dev, it means that OVS bridge interface is
	 * created.  Send OVSMGR_DP_BR_ADD notifier and later send
	 * OVSMGR_DP_PORT_ADD notifier for all the ports which are created
	 * before bridge interface.
	 */
	if ((vp_type == OVS_VPORT_TYPE_INTERNAL) && (master_dev == dev)) {
		info.dev = master_dev;
		ovsmgr_notifiers_call(&info, OVSMGR_DP_BR_ADD);
		nodp->add_notified = true;
		ovsmgr_dp_send_ports_add_notifier(nod, master_dev);
	}
}

/*
 * ovsmgr_dp_del()
 *	OVS datapath is deleted
 */
static void ovsmgr_dp_del(void *dp, struct net_device *dev)
{
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp *nod;

	ovsmgr_info("%px: datapath deleting - dev = %s\n", dp, dev->name);

	write_lock_bh(&ovsmgr_ctx.lock);
	nod = ovsmgr_dp_find(dp);
	if (!nod) {
		write_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: datapath instance is not found\n", dp);
		return;
	}

	list_del(&nod->node);
	write_unlock_bh(&ovsmgr_ctx.lock);

	kfree(nod);
	info.dev = dev;
	ovsmgr_notifiers_call(&info, OVSMGR_DP_DEL);
}

/*
 * ovsmgr_dp_add()
 *	New OVS data path is created
 */
static void ovsmgr_dp_add(void *dp, struct net_device *dev)
{
	struct ovsmgr_notifiers_info info;
	struct ovsmgr_dp *new_dp;

	ovsmgr_info("%px: new datapath - dev = %s\n", dp, dev->name);

	new_dp = kmalloc(sizeof(*new_dp), GFP_KERNEL);
	if (!new_dp) {
		ovsmgr_warn("%px: failed to allocation memory\n", dp);
		return;
	}

	INIT_LIST_HEAD(&new_dp->node);
	INIT_LIST_HEAD(&new_dp->port_list);
	new_dp->dev = dev;
	new_dp->dp = dp;

	write_lock_bh(&ovsmgr_ctx.lock);
	list_add(&new_dp->node, &ovsmgr_ctx.dp_list);
	write_unlock_bh(&ovsmgr_ctx.lock);

	info.dev = dev;
	ovsmgr_notifiers_call(&info, OVSMGR_DP_ADD);
}

/*
 * OVS datapath acceleration callbacks
 */
static struct ovs_accel_callback ovs_cb = {
	.ovs_accel_dp_add = ovsmgr_dp_add,
	.ovs_accel_dp_del = ovsmgr_dp_del,
	.ovs_accel_dp_port_add = ovsmgr_dp_port_add,
	.ovs_accel_dp_port_del = ovsmgr_dp_port_del,
	.ovs_accel_dp_flow_add = ovsmgr_dp_flow_add,
	.ovs_accel_dp_flow_del = ovsmgr_dp_flow_del,
	.ovs_accel_dp_flow_set = ovsmgr_dp_flow_set,
	.ovs_accel_dp_flow_tbl_flush = ovsmgr_dp_flow_tbl_flush,
	.ovs_accel_dp_pkt_process = ovsmgr_dp_pkt_process
};

/*
 * ovsmgr_dp_flow_key_fill()
 *	Fill key from flow and in_port
 */
static void ovsmgr_dp_flow_key_fill(struct ovsmgr_dp_flow *flow, int in_port, struct sw_flow_key *key)
{
	memset(key, 0, sizeof(*key));

	key->phy.in_port = in_port;
	ether_addr_copy(key->eth.src, flow->smac);
	ether_addr_copy(key->eth.dst, flow->dmac);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
	key->mac_proto = MAC_PROTO_ETHERNET;
#endif

	if (flow->ingress_vlan.h_vlan_TCI) {
		OVSMGR_KEY_VLAN_TCI(key) = htons(flow->ingress_vlan.h_vlan_TCI | VLAN_TAG_PRESENT);
	}

	if (flow->tuple.ip_version == 4) {
		key->ipv4.addr.src = flow->tuple.ipv4.src;
		key->ipv4.addr.dst = flow->tuple.ipv4.dst;
		key->eth.type = htons(ETH_P_IP);
	} else {
		memcpy(&key->ipv6.addr.src, &flow->tuple.ipv6.src, sizeof(key->ipv6.addr.src));
		memcpy(&key->ipv6.addr.dst, &flow->tuple.ipv6.dst, sizeof(key->ipv6.addr.dst));
		key->eth.type = htons(ETH_P_IPV6);
	}

	key->tp.src = flow->tuple.src_port;
	key->tp.dst = flow->tuple.dst_port;
	key->ip.proto = flow->tuple.protocol;

	ovsmgr_dp_flow_key_dump(key);
}


/*
 * ovsmgr_dp_flow_info_get_by_mac()
 *	Find OVS flow rule from MAC address and update VLAN information
 */
enum ovsmgr_flow_status ovsmgr_dp_flow_info_get_by_mac(void *dp, struct net_device *br_dev,
							struct sw_flow_key *key, struct ovsmgr_vlan_info *ovi)
{
	struct sw_flow_key *sfk;
	struct sw_flow *sf;

	/*
	 * Find datapath rule from bridge port to bridge interface (br_dev)
	 * using MAC addresses. SMAC and DMAC in key represent packet direction from
	 * br_dev to port, search flow in other direction.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
	sf = ovs_accel_flow_find_by_mac(dp, br_dev, key->eth.dst, key->eth.src, key->eth.type);
	if (!sf) {
		ovsmgr_warn("%px: Couldn't find flow rule using dev = %s, SMAC:%pM, DMAC:%pM\n",
				dp, br_dev->name, key->eth.dst, key->eth.src);
		return OVSMGR_FLOW_STATUS_DENY_ACCEL;
	}
#else
	return OVSMGR_FLOW_STATUS_DENY_ACCEL;
#endif

	/*
	 * Flow is found and flow key has ingress packet details.
	 * If the flow rule is expecting VLAN header then it is expected
	 * that from br_dev packet should be transmitted with VLAN header.
	 * Copy ingress VLAN header to egress VLAN in VLAN info.
	 */
	sfk = &sf->key;
	if (OVSMGR_KEY_VLAN_TCI(sfk)) {
		ovi->egress[0].h_vlan_TCI = ntohs(OVSMGR_KEY_VLAN_TCI(sfk)) & ~VLAN_TAG_PRESENT;
		ovi->egress[0].h_vlan_encapsulated_proto = ETH_P_8021Q;
		ovsmgr_info("Egress VLAN : id = %x:%x\n",
				ovi->egress[0].h_vlan_encapsulated_proto,
				ovi->egress[0].h_vlan_TCI);
		return OVSMGR_FLOW_STATUS_ALLOW_VLAN_ACCEL;
	}

	return OVSMGR_FLOW_STATUS_ALLOW_ACCEL;
}

/*
 * ovsmgr_dp_flow_info_get()
 *	Fill key from flow and in_port
 */
enum ovsmgr_flow_status ovsmgr_dp_flow_info_get(struct ovsmgr_dp_flow *flow,
						     struct sk_buff *skb, struct ovsmgr_vlan_info *ovi)
{
	struct ovsmgr_dp_port *in_port, *out_port;
	struct sw_flow_actions *acts;
	struct sw_flow_key key;
	struct ovsmgr_dp *nod;
	const struct nlattr *a;
	struct sw_flow *sf;
	enum ovsmgr_flow_status status = OVSMGR_FLOW_STATUS_DENY_ACCEL;
	bool found_egress = false;
	int in_port_num, out_port_num;
	int vlan_index = 0;
	int rem;

	read_lock_bh(&ovsmgr_ctx.lock);

	nod = ovsmgr_dp_find_by_dev(flow->indev);
	if (unlikely(!nod)) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Couldn't find datapath instance for dev = %s\n", flow, flow->indev->name);
		return OVSMGR_FLOW_STATUS_DENY_ACCEL;
	}

	in_port = ovsmgr_dp_port_find_by_dev(nod, flow->indev);
	if (!in_port) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: input datapath port instance is not found for dev = %s\n",
				nod, flow->indev->name);
		return OVSMGR_FLOW_STATUS_DENY_ACCEL;
	}

	out_port = ovsmgr_dp_port_find_by_dev(nod, flow->outdev);
	if (!out_port) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: output datapath port instance is not found for dev = %s\n",
				nod, flow->outdev->name);
		return OVSMGR_FLOW_STATUS_DENY_ACCEL;
	}

	in_port_num = in_port->vport_num;
	out_port_num = out_port->vport_num;
	read_unlock_bh(&ovsmgr_ctx.lock);

	ovsmgr_dp_flow_key_fill(flow, in_port_num, &key);

	/*
	 * If ingress packet is VLAN tagged then copy VLAN ID
	 */
	if (flow->ingress_vlan.h_vlan_TCI) {
		/*
		 * If ingress vlan is set then there is no need to update
		 * ovi->ingress, caller is already aware that there is
		 * ingress VLAN header.
		 * This case is valid for multicast flow when there is an
		 * egress list update.
		 * Fill TCI in key, OVS datapath expects VLAN_TAG_PRESENT
		 * to be set in TCI.
		 */
		OVSMGR_KEY_VLAN_TCI(&key) = htons(flow->ingress_vlan.h_vlan_TCI) | VLAN_TAG_PRESENT;
	} else if (skb && skb_vlan_tag_present(skb)) {
		/*
		 * Read VLAN tag from skb
		 * VLAN_TAG_PRESENT is set by kernel in skb, fill TCI in key.
		 */
		OVSMGR_KEY_VLAN_TCI(&key) = htons(skb->vlan_tci);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 0))
		key.eth.vlan.tpid = htons(ETH_P_8021Q);
		OVSMGR_KEY_VLAN_TCI(&key) |= htons(VLAN_TAG_PRESENT);
#endif

		ovi->ingress[0].h_vlan_TCI = skb_vlan_tag_get(skb);
		ovi->ingress[0].h_vlan_encapsulated_proto = ntohs(skb->vlan_proto);
		ovsmgr_info("Ingress VLAN : id = %x:%x\n",
				ovi->ingress[0].h_vlan_encapsulated_proto,
				ovi->ingress[0].h_vlan_TCI);

		status = OVSMGR_FLOW_STATUS_ALLOW_VLAN_ACCEL;
	}

	/*
	 * Find datapath rule with key.
	 * If rule is not found and it is a routed flow, then find rule using
	 * MAC addresses if the flow is not multicast.
	 * For routed flows flow->indev is always OVS bridge interface.
	 */
	sf = ovs_accel_flow_find(nod->dp, &key);
	if (!sf) {
		if (is_multicast_ether_addr(flow->smac) ||
			is_multicast_ether_addr(flow->dmac) || !flow->is_routed) {
			ovsmgr_warn("%px:Couldn't find flow rule \n", flow);
			return OVSMGR_FLOW_STATUS_DENY_ACCEL;
		}

		return ovsmgr_dp_flow_info_get_by_mac(nod->dp, flow->indev, &key, ovi);
	}

	rcu_read_lock();

	/*
	 * Flow is found, find if VLAN operations are needed and check
	 * if egress port is allowed.
	 */
	acts = rcu_dereference(sf->sf_acts);
	rcu_read_unlock();

	for (a = acts->actions, rem = acts->actions_len; rem > 0;
	     a = nla_next(a, &rem)) {
		switch (nla_type(a)) {
		case OVS_ACTION_ATTR_OUTPUT:
			if (out_port_num == nla_get_u32(a)) {
				ovsmgr_info("%px: Found egress port in flow rule: %d\n", flow, out_port_num);
				found_egress = true;
				if (status == OVSMGR_FLOW_STATUS_DENY_ACCEL)
					status = OVSMGR_FLOW_STATUS_ALLOW_ACCEL;
				break;
			}

			/*
			 * output port do not match, reset vlan parameters if they are set.
			 */
			if (!vlan_index) {
				break;
			}

			vlan_index--;
			ovi->egress[vlan_index].h_vlan_TCI = 0;
			ovi->egress[vlan_index].h_vlan_encapsulated_proto = 0;
			break;

		case OVS_ACTION_ATTR_PUSH_VLAN: {
			const struct ovs_action_push_vlan *vlan;

			vlan = nla_data(a);
			ovsmgr_info("Vlan details:\n");
			ovsmgr_info("\tvlan_tpid = %x\n", vlan->vlan_tpid);
			ovsmgr_info("\tvlan_tci = %x\n", vlan->vlan_tci & ~VLAN_TAG_PRESENT);

			/*
			 * Allow only two VLAN headers
			 */
			if (vlan_index == 2) {
				ovsmgr_info("%px: More than 2 VLAN headers, don't accelerate\n", flow);
				return OVSMGR_FLOW_STATUS_DENY_ACCEL;
			}

			ovi->egress[vlan_index].h_vlan_TCI = ntohs(vlan->vlan_tci) & ~VLAN_TAG_PRESENT;
			ovi->egress[vlan_index].h_vlan_encapsulated_proto = ntohs(vlan->vlan_tpid);
			vlan_index++;
			break;
		}

		case OVS_ACTION_ATTR_POP_VLAN:
			break;
		}

		/*
		 * If the egress port is found then the job is done.
		 */
		if (found_egress) {
			break;
		}
	}

	switch (vlan_index) {
	case 1: /* return single VLAN operation. */
		return OVSMGR_FLOW_STATUS_ALLOW_VLAN_ACCEL;
	case 2: /* return QinQ VLAN operation. */
		return OVSMGR_FLOW_STATUS_ALLOW_VLAN_QINQ_ACCEL;
	default:
		/*
		 * Do not accelerate if flow->outdev is not part of output list
		 */
		if (!found_egress) {
			return OVSMGR_FLOW_STATUS_DENY_ACCEL;
		}
	}

	return status;
}

/*
 * ovsmgr_dp_dev_get_master()
 *	Find datapath bridge interface for given bridge port
 */
struct net_device *ovsmgr_dp_dev_get_master(struct net_device *dev)
{
	struct ovsmgr_dp *nod;
	struct ovsmgr_dp_port *nodp;
	struct net_device *master_dev;

	read_lock_bh(&ovsmgr_ctx.lock);
	nod = ovsmgr_dp_find_by_dev(dev);
	if (!nod) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("Couldn't find datapath instance, dev = %s\n", dev->name);
		return NULL;
	}

	nodp = ovsmgr_dp_port_find_by_dev(nod, dev);
	if (!nodp) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: input datapath port instance is not found for dev = %s\n",
				nod, dev->name);
		return NULL;
	}

	master_dev = nodp->master_dev;
	read_unlock_bh(&ovsmgr_ctx.lock);
	return master_dev;
}

/*
 * ovsmgr_dp_dev_is_master()
 *	Return true if dev is OVS bridge interface
 */
bool ovsmgr_dp_dev_is_master(struct net_device *dev)
{
	struct ovsmgr_dp *nod;
	struct ovsmgr_dp_port *nodp;
	bool is_master = false;

	read_lock_bh(&ovsmgr_ctx.lock);
	nod = ovsmgr_dp_find_by_dev(dev);
	if (!nod) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("Couldn't find datapath instance, dev = %s\n", dev->name);
		return false;
	}

	nodp = ovsmgr_dp_port_find_by_dev(nod, dev);
	if (!nodp) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: input datapath port instance is not found for dev = %s\n",
				nod, dev->name);
		return false;
	}

	if ((nodp->vport_type == OVS_VPORT_TYPE_INTERNAL) && (nodp->master_dev == dev)) {
		is_master = true;
	}

	read_unlock_bh(&ovsmgr_ctx.lock);
	return is_master;
}

/*
 * ovsmgr_dp_bridge_interface_stats_update()
 *	Update OVS datapath bridge interface statistics.
 */
void ovsmgr_dp_bridge_interface_stats_update(struct net_device *dev,
					  uint32_t rx_packets, uint32_t rx_bytes,
					  uint32_t tx_packets, uint32_t tx_bytes)
{
	struct pcpu_sw_netstats *ovs_stats;

	if (!ovsmgr_dp_dev_is_master(dev)) {
		ovsmgr_warn("%px: %s is not an OVS bridge device\n", dev, dev->name);
		return;
	}

	ovs_stats = this_cpu_ptr(dev->tstats);
	u64_stats_update_begin(&ovs_stats->syncp);
	ovs_stats->rx_packets += rx_packets;
	ovs_stats->rx_bytes += rx_bytes;
	ovs_stats->tx_packets += tx_packets;
	ovs_stats->tx_bytes += tx_bytes;
	u64_stats_update_end(&ovs_stats->syncp);
}

/*
 * ovsmgr_dp_flow_stats_update()
 *	Update datapath flow statistics
 */
void ovsmgr_dp_flow_stats_update(struct ovsmgr_dp_flow *flow, struct ovsmgr_dp_flow_stats *stats)
{
	struct sw_flow_key key;
	struct ovsmgr_dp_port *in_port, *out_port;
	struct ovsmgr_dp *nod;
	void *out_vport;
	int in_port_num;

	/*
	 * for bridge flows:
	 *	indev and outdev are OVS bridge ports.
	 * for route flows:
	 *	indev or outdev is OVS bridge port.
	 * TODO: Do we need to add route/bridge flag in flow?  This would
	 * help us to check if we need to find dp with outdev if indev fails
	 */
	read_lock_bh(&ovsmgr_ctx.lock);
	nod = ovsmgr_dp_find_by_dev(flow->indev);
	if (unlikely(!nod)) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Couldn't find datapath instance for dev = %s\n", flow, flow->indev->name);
		return;
	}

	in_port = ovsmgr_dp_port_find_by_dev(nod, flow->indev);
	if (!in_port) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: input datapath port instance is not found for dev = %s\n",
				nod, flow->indev->name);
		return;
	}

	out_port = ovsmgr_dp_port_find_by_dev(nod, flow->outdev);
	if (!out_port) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: output datapath port instance is not found for dev = %s\n",
				nod, flow->outdev->name);
		return;
	}

	in_port_num = in_port->vport_num;
	out_vport = out_port->vport;
	read_unlock_bh(&ovsmgr_ctx.lock);

	ovsmgr_dp_flow_key_fill(flow, in_port_num, &key);
	if (ovs_accel_flow_stats_update(nod->dp, out_vport, &key, stats->pkts, stats->bytes)) {
		ovsmgr_warn("%px: Couldnt update statistics\n", nod->dp);
	}
}

/*
 * ovsmgr_dp_port_dev_find_by_mac()
 *	Find egress datapath port, given skb, datapath interface (dev) and flow
 */
struct net_device *ovsmgr_dp_port_dev_find_by_mac(struct sk_buff *skb, struct net_device *dev,
							struct ovsmgr_dp_flow *flow)
{
	struct ovsmgr_dp *nod;
	struct ovsmgr_dp_port *nodp;
	struct sw_flow *sf;
	struct net_device *ovs_port;
	void *dp;
	uint16_t type;

	read_lock_bh(&ovsmgr_ctx.lock);
	nod = ovsmgr_dp_find_by_dev(dev);
	if (!nod) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Couldn't find datapath instance, dev = %s\n", skb, dev->name);
		return NULL;
	}

	nodp = ovsmgr_dp_port_find_by_dev(nod, dev);
	if (!nodp) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Couldn't find datapath port instance, dev = %s\n", skb, dev->name);
		return NULL;
	}

	if (nodp->vport_type != OVS_VPORT_TYPE_INTERNAL) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Interface type is not internal, dev = %s\n", skb, dev->name);
		return NULL;
	}

	dp = nod->dp;
	read_unlock_bh(&ovsmgr_ctx.lock);

	if (flow->tuple.ip_version == 4) {
		type = htons(ETH_P_IP);
	} else {
		type = htons(ETH_P_IPV6);
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0))
	sf = ovs_accel_flow_find_by_mac(dp, dev, flow->smac, flow->dmac, type);
	if (!sf) {
		ovsmgr_warn("%px: datapath flow is not found with SMAC: %pM, DMAC: %pM, dev = %s\n",
				skb, flow->smac, flow->dmac, dev->name);
		return NULL;
	}
#else
	return NULL;
#endif

	/*
	 * Find the dev by ingress port number
	 */
	read_lock_bh(&ovsmgr_ctx.lock);
	nodp = ovsmgr_dp_port_find_by_num(nod, sf->key.phy.in_port);
	if (!nodp) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: DP port is not found, port number: %d, dev = %s\n", skb, sf->key.phy.in_port, dev->name);
		return NULL;
	}

	ovs_port = nodp->dev;
	read_unlock_bh(&ovsmgr_ctx.lock);

	return ovs_port;
}

/*
 * ovsmgr_dp_port_dev_find()
 *	Find egress datapath port, given skb and datapath interface (dev)
 */
struct net_device *ovsmgr_dp_port_dev_find(struct sk_buff *skb,
					       struct net_device *dev, struct ovsmgr_dp_flow *flow)
{
	struct ovsmgr_dp *nod;
	struct ovsmgr_dp_port *nodp;
	struct sw_flow_key key;
	void *dp;
	int vport_num;

	read_lock_bh(&ovsmgr_ctx.lock);
	nod = ovsmgr_dp_find_by_dev(dev);
	if (!nod) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Couldn't find datapath instance, dev = %s\n", skb, dev->name);
		return NULL;
	}

	nodp = ovsmgr_dp_port_find_by_dev(nod, dev);
	if (!nodp) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Couldn't find datapath port instance, dev = %s\n", skb, dev->name);
		return NULL;
	}

	if (nodp->vport_type != OVS_VPORT_TYPE_INTERNAL) {
		read_unlock_bh(&ovsmgr_ctx.lock);
		ovsmgr_warn("%px: Interface type is not internal, dev = %s\n", skb, dev->name);
		return NULL;
	}

	dp = nod->dp;
	vport_num = nodp->vport_num;
	read_unlock_bh(&ovsmgr_ctx.lock);

	/*
	 * Multicast address requires finding the ingress interface through all
	 * bridge ports
	 */
	if (is_multicast_ether_addr(flow->dmac)) {
		return ovsmgr_dp_mcast_port_dev_find(nod, nodp, flow, skb);
	}

	ether_addr_copy(flow->smac, dev->dev_addr);
	ovsmgr_dp_flow_key_fill(flow, vport_num, &key);

	return ovs_accel_egress_dev_find(dp, &key, skb);
}

/*
 * ovsmgr_exit()
 *	Cleanup datapath context
 */
void ovsmgr_dp_exit(void)
{
	struct ovsmgr_dp *nod, *temp_dp;

	ovs_unregister_accelerator(&ovs_cb);
	/*
	 * Cleanup the database.
	 */
	write_lock_bh(&ovsmgr_ctx.lock);
	list_for_each_entry_safe(nod, temp_dp, &ovsmgr_ctx.dp_list, node) {
		struct ovsmgr_dp_port *nodp, *temp_port;

		list_for_each_entry_safe(nodp, temp_port, &nod->port_list, node) {
			list_del(&nodp->node);
			kfree(nodp);
		}

		list_del(&nod->node);
		kfree(nod);
	}

	write_unlock_bh(&ovsmgr_ctx.lock);
}

/*
 * ovsmgr_init()
 *	Initialize datapath context
 */
int ovsmgr_dp_init(void)
{
	return ovs_register_accelerator(&ovs_cb);
}
