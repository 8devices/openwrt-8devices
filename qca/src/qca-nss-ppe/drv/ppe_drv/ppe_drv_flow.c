/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/in.h>
#include <net/ipv6.h>
#include <linux/netdevice.h>
#include <fal/fal_flow.h>
#include <fal/fal_qos.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_flow_dump()
 *	Read and dump the flow table entry.
 */
void ppe_drv_flow_dump(struct ppe_drv_flow *pf)
{
	fal_flow_entry_t flow_cfg = {0};
	sw_error_t err;

	flow_cfg.entry_id = pf->index;
	err = fal_flow_entry_get(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_INDEX, &flow_cfg);
	if (err != SW_OK){
		ppe_drv_warn("%p: failed to get flow entry", pf);
		return;
	}

	ppe_drv_trace("%p: index: %d", pf, flow_cfg.entry_id);
	ppe_drv_trace("%p: entry_type: %d", pf, flow_cfg.entry_type);
	ppe_drv_trace("%p: protocol: %d", pf, flow_cfg.protocol);;
	ppe_drv_trace("%p: age: %d", pf, flow_cfg.age);
	ppe_drv_trace("%p: src_intf_valid: %d", pf, flow_cfg.src_intf_valid);
	ppe_drv_trace("%p: src_intf_index: %d", pf, flow_cfg.src_intf_index);
	ppe_drv_trace("%p: fwd_type: %d", pf, flow_cfg.fwd_type);
	ppe_drv_trace("%p: snat_nexthop: %d", pf, flow_cfg.snat_nexthop);
	ppe_drv_trace("%p: snat_srcport: %d", pf, flow_cfg.snat_srcport);
	ppe_drv_trace("%p: dnat_nexthop: %d", pf, flow_cfg.dnat_nexthop);
	ppe_drv_trace("%p: dnat_dstport: %d", pf, flow_cfg.dnat_dstport);
	ppe_drv_trace("%p: route_nexthop: %d", pf, flow_cfg.route_nexthop);
	ppe_drv_trace("%p: port_valid: %d", pf, flow_cfg.port_valid);
	ppe_drv_trace("%p: route_port: %d", pf, flow_cfg.route_port);
	ppe_drv_trace("%p: bridge_port: %d", pf, flow_cfg.bridge_port);
	ppe_drv_trace("%p: deacclr_en: %d", pf, flow_cfg.deacclr_en);
	ppe_drv_trace("%p: copy_tocpu_en: %d", pf, flow_cfg.copy_tocpu_en);
	ppe_drv_trace("%p: syn_toggle: %d", pf, flow_cfg.syn_toggle);
	ppe_drv_trace("%p: pri_profile: %d", pf, flow_cfg.pri_profile);
	ppe_drv_trace("%p: sevice_code: %d", pf, flow_cfg.sevice_code);
	ppe_drv_trace("%p: src_port: %d", pf, flow_cfg.src_port);
	ppe_drv_trace("%p: dst_port: %d", pf, flow_cfg.dst_port);
	ppe_drv_trace("%p: tree_id: %d", pf, flow_cfg.tree_id);
	ppe_drv_trace("%p: pkt_counter: %d", pf, flow_cfg.pkt_counter);
	ppe_drv_trace("%p: byte_counter: %llu", pf, flow_cfg.byte_counter);
	ppe_drv_trace("%p: pmtu_check_l3: %d", pf, flow_cfg.pmtu_check_l3);
	ppe_drv_trace("%p: pmtu: %d", pf, flow_cfg.pmtu);
	ppe_drv_trace("%p: vpn_id: %d", pf, flow_cfg.vpn_id);
	ppe_drv_trace("%p: vlan_fmt_valid: %d", pf, flow_cfg.vlan_fmt_valid);
	ppe_drv_trace("%p: svlan_fmt: %d", pf, flow_cfg.svlan_fmt);
	ppe_drv_trace("%p: cvlan_fmt: %d", pf, flow_cfg.cvlan_fmt);
	ppe_drv_trace("%p: wifi_qos_en: %d", pf, flow_cfg.wifi_qos_en);
	ppe_drv_trace("%p: wifi_qos: %d", pf, flow_cfg.wifi_qos);
	ppe_drv_trace("%p: invalid: %d", pf, flow_cfg.invalid);

	if (pf->type == PPE_DRV_IP_TYPE_V4) {
		ppe_drv_trace("%p: ipv4 address: %pI4", pf, &flow_cfg.flow_ip.ipv4);
	} else {
		ppe_drv_trace("%p: ipv6 address: %pI6", pf, &flow_cfg.flow_ip.ipv6.ul[0]);
	}
}
#else
/*
 * ppe_drv_flow_dump()
 *	Read and dump the flow table entry.
 */
void ppe_drv_flow_dump(struct ppe_drv_flow *pf)
{
}
#endif

/*
 * ppe_drv_flow_valid_set()
 *	Enable ppe flow entry.
 */
bool ppe_drv_flow_valid_set(struct ppe_drv_flow *pf, bool enable)
{
	sw_error_t err;

	err = fal_flow_entry_en_set(PPE_DRV_SWITCH_ID, pf->index, enable);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to set valid bit: %d for flow index: %d", pf, enable, pf->index);
		return false;
	}

	pf->flags |= PPE_DRV_FLOW_VALID;
	ppe_drv_trace("%p: set valid bit: %d for flow index: %d", pf, enable, pf->index);
	return true;
}

/*
 * ppe_drv_flow_stats_clear()
 *	Clears the flow statistics for the given flow index.
 */
void ppe_drv_flow_stats_clear(struct ppe_drv_flow *pf)
{
	sw_error_t err;

	err = fal_flow_counter_cleanup(PPE_DRV_SWITCH_ID, pf->index);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to clear stats for flow at index: %u err: %d",
				pf, pf->index, err);
	}
}

/*
 * ppe_drv_flow_stats_update()
 *	Updates flow instance's stats counter from PPE flow hit counter.
 */
void ppe_drv_flow_stats_update(struct ppe_drv_flow *pf)
{
	sw_error_t err;
	uint32_t delta_pkts;
	uint32_t delta_bytes;
	fal_entry_counter_t flow_cntrs = {0};

	ppe_drv_trace("%p: updating flow stats", pf);

	err = fal_flow_counter_get(PPE_DRV_SWITCH_ID, pf->index, &flow_cntrs);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get stats for flow at index: %u", pf, pf->index);
		return;
	}

	/*
	 * Update ppe_conn_flow packet and byte counters
	 */
	delta_pkts = flow_cntrs.matched_pkts - pf->pkts;
	delta_bytes = flow_cntrs.matched_bytes - pf->bytes;

	/*
	 * TODO: to be completed with stats.
	 */
	pf->pkts = flow_cntrs.matched_pkts;
	pf->bytes = flow_cntrs.matched_bytes;

	ppe_drv_trace("%p: updating stats for flow [index:%u] - curr pkt:%u byte:%llu", pf, pf->index, pf->pkts, pf->bytes);
}

/*
 * ppe_drv_flow_v6_qos_set()
 *	Set QOS mapping for this flow.
 */
bool ppe_drv_flow_v6_qos_set(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_flow *flow)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	/*
	 * TODO: to be filled later.
	 */
	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, flow->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos mapping configuration failed for flow: %p", pcf, flow);
		return false;
	}

	ppe_drv_trace("%p qos mapping configuration done for flow: %p", pcf, flow);
	return true;
}

/*
 * ppe_drv_flow_v6_qos_clear()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v6_qos_clear(struct ppe_drv_flow *pf)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, pf->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos mapping configuration failed for flow", pf);
		return false;
	}

	ppe_drv_trace("%p qos mapping configuration done for flow", pf);
	return true;
}

/*
 * ppe_drv_flow_v6_tree_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v6_tree_id_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *tree_id)
{
	/*
	 * TODO: to be added later.
	 */
	*tree_id = 0;
	return true;
}

/*
 * ppe_drv_flow_v6_vpn_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v6_vpn_id_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *vpn_id)
{
	/*
	 * TODO: to be added later.
	 */
	*vpn_id = 0;
	return true;
}

/*
 * ppe_drv_flow_v6_wifi_qos_get()
 *	Find the WIFI QOS associated with a flow
 */
static bool ppe_drv_flow_v6_wifi_qos_get(struct ppe_drv_v6_conn_flow *pcf, uint32_t *wifi_qos, bool *wifi_qos_en)
{
	/*
	 * TODO: to be added later.
	 */
	*wifi_qos = 0;
	*wifi_qos_en = false;
	return true;
}

/*
 * ppe_drv_flow_v6_service_code_get()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v6_service_code_get(struct ppe_drv_v6_conn_flow *pcf, uint8_t *scp)
{
	ppe_drv_sc_t service_code = *scp;

	/*
	 * Service code to avoid PPE drop while processing bridge flows between two different VSIs.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_VLAN_FILTER_BYPASS)) {
			ppe_drv_warn("%p: flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, PPE_DRV_SC_VLAN_FILTER_BYPASS);
			return false;
		}
	}

	/*
	 * SC required to accelerate inline EIP flows.
	 */
	if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_INLINE_IPSEC)) {
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_IPSEC_PPE2EIP)) {
			ppe_drv_warn("%p: flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, PPE_DRV_SC_IPSEC_PPE2EIP);
			return false;
		}
	}

	*scp = service_code;
	return true;
}

/*
 * ppe_drv_flow_v6_get()
 *	Find flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v6_get(struct ppe_drv_v6_5tuple *tuple)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_flow_host_entry_t flow_host = {0};
	fal_flow_entry_t *flow_cfg = &flow_host.flow_entry;
	fal_host_entry_t *host_cfg = &flow_host.host_entry;
	uint8_t protocol = tuple->protocol;
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	sw_error_t err;

	/*
	 * Fill host entry.
	 */
	memcpy(host_cfg->ip6_addr.ul, tuple->flow_ip, sizeof(tuple->flow_ip));
	host_cfg->flags = FAL_IP_IP6_ADDR;

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	if (protocol == IPPROTO_TCP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", tuple, FAL_FLOW_PROTOCOL_TCP);
	} else if (protocol == IPPROTO_UDP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", tuple, FAL_FLOW_PROTOCOL_UDP);
	} else if (protocol == IPPROTO_UDPLITE) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", tuple, FAL_FLOW_PROTOCOL_UDPLITE);
	} else if (protocol == IPPROTO_ESP) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else if (protocol == IPPROTO_IPV6) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_query_unknown_proto);
		ppe_drv_warn("%p: protocol: %u incorrect for PPE", tuple, protocol);
		return NULL;
	}

	memcpy(flow_cfg->flow_ip.ipv6.ul, tuple->return_ip, sizeof(tuple->return_ip));

	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI6", tuple, &tuple->return_ip);
	if (tuple_3) {
		flow_cfg->entry_type = FAL_FLOW_IP6_3TUPLE_ADDR;
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg->ip_type = protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", tuple, protocol);
	} else {
		flow_cfg->entry_type = FAL_FLOW_IP6_5TUPLE_ADDR;

		flow_cfg->src_port = tuple->flow_ident;
		flow_cfg->dst_port = tuple->return_ident;

		ppe_drv_trace("%p: flow_tbl[sport]: %u", tuple, flow_cfg->src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", tuple, flow_cfg->dst_port);

	}

	/*
	 * query hardware
	 */
	err = fal_flow_host_get(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_host);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow get failed", tuple);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg->entry_id];
	ppe_drv_assert((flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already decelerated from PPE at index: %d",
			tuple, flow_cfg->entry_id);

	ppe_drv_assert((host_cfg->entry_id == flow->host->index),
			"%p flow entry and host entry mismatch flow-index: %d hw-host_index: %d sw-host_index: %d",
			tuple, flow->index, host_cfg->entry_id, flow->host->index);

	ppe_drv_trace("%p: flow_tbl entry found at index: %u", tuple, flow_cfg->entry_id);
	return flow;
}

/*
 * ppe_drv_flow_v6_add()
 *	Add flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v6_add(struct ppe_drv_v6_conn_flow *pcf, struct ppe_drv_nexthop *nh,
					struct ppe_drv_host *host, bool entry_valid)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_flow_entry_t flow_cfg = {0};
	uint32_t match_dest_ip[4];
	uint32_t match_protocol = ppe_drv_v6_conn_flow_match_protocol_get(pcf);
	uint8_t vlan_hdr_cnt = ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf);
	struct ppe_drv_iface *port_if = ppe_drv_v6_conn_flow_eg_port_if_get(pcf);
	struct ppe_drv_port *pp = ppe_drv_iface_port_get(port_if);
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	bool wifi_qos_en;
	uint16_t xmit_mtu;
	sw_error_t err;

	ppe_drv_trace("%p: flow_tbl[host_idx]: %u", pcf, host->index);
	flow_cfg.host_addr_type = PPE_DRV_HOST_LAN;
        flow_cfg.host_addr_index = host->index;
        flow_cfg.deacclr_en = false;
        flow_cfg.invalid = !entry_valid;

	flow_cfg.sevice_code = PPE_DRV_SC_NONE;
	if (!ppe_drv_flow_v6_service_code_get(pcf, &flow_cfg.sevice_code)) {
		ppe_drv_warn("%p: failed to obtain a valid service code", pcf);
		return NULL;
	}

	/*
	 * Get the tree ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v6_tree_id_get(pcf, &flow_cfg.tree_id)) {
		ppe_drv_warn("%p: failed to obtain a valid tree ID", pcf);
		return NULL;
	}

	/*
	 * Get the VPN ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v6_vpn_id_get(pcf, &flow_cfg.vpn_id)) {
		ppe_drv_warn("%p: failed to obtain a valid vpn_id", pcf);
		return NULL;
	}

	/*
	 * Get the WIFI QOS corresponding to flow.
	 */
	if (!ppe_drv_flow_v6_wifi_qos_get(pcf, &flow_cfg.wifi_qos, &wifi_qos_en)) {
		ppe_drv_warn("%p: failed to obtain wifi qos", pcf);
		return NULL;
	}

	flow_cfg.wifi_qos_en = wifi_qos_en;

	ppe_drv_v6_conn_flow_match_dest_ip_get(pcf, &match_dest_ip[0]);

	/*
	 * Set forwarding type
	 */
	if (ipv6_addr_is_multicast((struct in6_addr *)match_dest_ip)) {
		/*
		 * Multicast flow
		 */
		flow_cfg.fwd_type = FAL_FLOW_FORWARD;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2-Multicast: %u", pcf, FAL_FLOW_FORWARD);
	} else if (ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		/*
		 * Case bridging
		 */
		flow_cfg.fwd_type = FAL_FLOW_BRIDGE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2: %u", pcf, FAL_FLOW_BRIDGE);

		flow_cfg.port_valid = true;
		flow_cfg.bridge_port = pp->port;
		ppe_drv_trace("%p: xmit interface port: %d", pcf, pp->port);

		if (ppe_drv_port_is_tunnel_vp(pp)) {
			switch (vlan_hdr_cnt) {
				case 2:
					flow_cfg.svlan_fmt = true;
					flow_cfg.cvlan_fmt = true;
					flow_cfg.vlan_fmt_valid = 1;
					break;
				case 1:
					flow_cfg.cvlan_fmt = true;
					flow_cfg.vlan_fmt_valid = 1;
					break;
				case 0:
					break;
				default:
					ppe_drv_warn("%p: cannot acclerate bridge VLAN flows for more than 2 vlans: %u",
							pcf, vlan_hdr_cnt);
					return NULL;
			}
		}
	} else {
		/*
		 * Case simple routing (Non-NAT)
		 */
		flow_cfg.route_nexthop = nh->index;
		flow_cfg.fwd_type = FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L3: %u", pcf, FAL_FLOW_ROUTE);
	}

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	switch (match_protocol) {
	case IPPROTO_TCP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", pcf, FAL_FLOW_PROTOCOL_TCP);
		break;

	case IPPROTO_UDP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", pcf, FAL_FLOW_PROTOCOL_UDP);
		break;

	case IPPROTO_UDPLITE:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", pcf, FAL_FLOW_PROTOCOL_UDPLITE);
		break;

	case IPPROTO_ESP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	case IPPROTO_IPIP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	default:
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_unknown_proto);
		ppe_drv_warn("%p: protocol: %u cannot be offloaded to PPE", pcf, match_protocol);
		return NULL;
	}

	memcpy(flow_cfg.flow_ip.ipv6.ul, &match_dest_ip, sizeof(match_dest_ip));
	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI6", pcf, &match_dest_ip);
	if (tuple_3) {
		flow_cfg.entry_type = FAL_FLOW_IP6_3TUPLE_ADDR;
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg.ip_type = match_protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", pcf, match_protocol);
	} else {
		flow_cfg.entry_type = FAL_FLOW_IP6_5TUPLE_ADDR;

		flow_cfg.src_port = ppe_drv_v6_conn_flow_match_src_ident_get(pcf);
		flow_cfg.dst_port = ppe_drv_v6_conn_flow_match_dest_ident_get(pcf);

		ppe_drv_trace("%p: flow_tbl[sport]: %u", pcf, flow_cfg.src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", pcf, flow_cfg.dst_port);

	}

	/*
	 * Fill Path-MTU.
	 *
	 * Note: For multicast flows, it should be ok to program the minimum MTU value
	 * of all the interfaces, since PPE also check MTU for each destination interface
	 * and exception the packet (without cloning) if MTU check fail for any interface.
	 */
	xmit_mtu = ipv6_addr_is_multicast((struct in6_addr *)match_dest_ip) ? ppe_drv_v6_conn_flow_mc_min_mtu_get(pcf)
		: ppe_drv_v6_conn_flow_xmit_interface_mtu_get(pcf);
	if (xmit_mtu > PPE_DRV_PORT_JUMBO_MAX) {
		ppe_drv_trace("%p: xmit_mtu: %d is larger, restricting to max: %d", pcf, xmit_mtu, PPE_DRV_PORT_JUMBO_MAX);
		xmit_mtu = PPE_DRV_PORT_JUMBO_MAX;
	}

	flow_cfg.pmtu_check_l3 = PPE_DRV_FLOW_PMTU_TYPE_L3;
	flow_cfg.pmtu = xmit_mtu;

	ppe_drv_trace("%p: flow_tbl[PMTU]: %u", pcf, xmit_mtu);

	/*
	 * Add the flow
	 */
	err = fal_flow_entry_add(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_cfg);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow entry add failed", pcf);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg.entry_id];
	ppe_drv_assert(!(flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already accelerated to PPE at index: %d",
			pcf, flow_cfg.entry_id);

	/*
	 * Update the shadow copy
	 */
	flow->host = host;
	flow->nh = nh;
	flow->flags |= PPE_DRV_FLOW_V6;
	flow->type = PPE_DRV_IP_TYPE_V6;
	flow->entry_type = flow_cfg.entry_type;
	flow->pcf.v6 = pcf;
	ppe_drv_trace("%p: flow_tbl entry added at index: %u", pcf, flow_cfg.entry_id);
	return flow;
}

/*
 * ppe_drv_flow_v4_qos_set()
 *	Set QOS mapping for this flow.
 */
bool ppe_drv_flow_v4_qos_set(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_flow *pf)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	/*
	 * TODO: to be filled later.
	 */
	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, pf->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos mapping configuration failed for flow: %p", pcf, pf);
		return false;
	}

	ppe_drv_trace("%p qos mapping configuration done for flow: %p", pcf, pf);
	return true;
}

/*
 * ppe_drv_flow_v4_qos_clear()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v4_qos_clear(struct ppe_drv_flow *pf)
{
	sw_error_t err;
	fal_qos_cosmap_t qos_cfg = {0};

	err = fal_qos_cosmap_flow_set(PPE_DRV_SWITCH_ID, 0, pf->index, &qos_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p qos clear configuration failed for flow", pf);
		return false;
	}

	ppe_drv_trace("%p qos clear configuration done for flow", pf);
	return true;
}

/*
 * ppe_drv_flow_v4_tree_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v4_tree_id_get(struct ppe_drv_v4_conn_flow *pcf, uint32_t *tree_id)
{
	/*
	 * TODO: to be added later.
	 */
	*tree_id = 0;
	return true;
}

/*
 * ppe_drv_flow_v4_vpn_id_get()
 *	Find the tree ID associated with a flow
 */
static bool ppe_drv_flow_v4_vpn_id_get(struct ppe_drv_v4_conn_flow *pcf, uint32_t *vpn_id)
{
	/*
	 * TODO: to be added later.
	 */
	*vpn_id = 0;
	return true;
}

/*
 * ppe_drv_flow_v4_wifi_qos_get()
 *	Find the WIFI QOS associated with a flow
 */
static bool ppe_drv_flow_v4_wifi_qos_get(struct ppe_drv_v4_conn_flow *pcf, uint32_t *wifi_qos, bool *wifi_qos_en)
{
	/*
	 * TODO: to be added later.
	 */
	*wifi_qos = 0;
	*wifi_qos_en = false;
	return true;
}

/*
 * ppe_drv_flow_v4_service_code_get()
 *	Return service code required for this flow.
 */
bool ppe_drv_flow_v4_service_code_get(struct ppe_drv_v4_conn_flow *pcf, uint8_t *scp)
{
	ppe_drv_sc_t service_code = *scp;

	/*
	 * Service code to avoid PPE drop while processing bridge flows between two different VSIs.
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_VLAN_FILTER_BYPASS)) {
			ppe_drv_warn("%p: flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, PPE_DRV_SC_VLAN_FILTER_BYPASS);
			return false;
		}
	}

	/*
	 * SC required to accelerate inline EIP flows.
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_INLINE_IPSEC)) {
		if (!ppe_drv_sc_check_and_set(&service_code, PPE_DRV_SC_IPSEC_PPE2EIP)) {
			ppe_drv_warn("%p: flow requires multiple service codes existing:%u new:%u",
					pcf, service_code, PPE_DRV_SC_IPSEC_PPE2EIP);
			return false;
		}
	}

	*scp = service_code;
	return true;
}

/*
 * ppe_drv_flow_del()
 *	Delete a flow entry.
 */
bool ppe_drv_flow_del(struct ppe_drv_flow *pf)
{
	fal_flow_entry_t flow_cfg = {0};
	sw_error_t err;

	/*
	 * Delete the flow
	 */
	flow_cfg.entry_id = pf->index;
	flow_cfg.entry_type = pf->entry_type;
	err = fal_flow_entry_del(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_INDEX, &flow_cfg);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow entry deletion failed", pf);
		return false;
	}

	/*
	 * Update the shadow copy
	 */
	pf->flags = 0;
	pf->type = 0;
	pf->entry_type = 0;
	ppe_drv_trace("%p: flow_tbl entry deleted at index: %u", pf, pf->index);
	return true;
}

/*
 * ppe_drv_flow_v4_get()
 *	Find flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v4_get(struct ppe_drv_v4_5tuple *tuple)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_flow_host_entry_t flow_host = {0};
	fal_flow_entry_t *flow_cfg = &flow_host.flow_entry;
	fal_host_entry_t *host_cfg = &flow_host.host_entry;
	uint8_t protocol = tuple->protocol;
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	sw_error_t err;

	/*
	 * Fill host entry.
	 */
	host_cfg->ip4_addr = tuple->flow_ip;
	host_cfg->flags = FAL_IP_IP4_ADDR;

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	if (protocol == IPPROTO_TCP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", tuple, FAL_FLOW_PROTOCOL_TCP);
	} else if (protocol == IPPROTO_UDP) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", tuple, FAL_FLOW_PROTOCOL_UDP);
	} else if (protocol == IPPROTO_UDPLITE) {
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", tuple, FAL_FLOW_PROTOCOL_UDPLITE);
	} else if (protocol == IPPROTO_ESP) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else if (protocol == IPPROTO_IPV6) {
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", tuple, FAL_FLOW_PROTOCOL_OTHER);
	} else {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_query_unknown_proto);
		ppe_drv_warn("%p: protocol: %u incorrect for PPE", tuple, protocol);
		return NULL;
	}

	flow_cfg->flow_ip.ipv4 = tuple->return_ip;
	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI4", tuple, &tuple->return_ip);
	if (tuple_3) {
		flow_cfg->entry_type = FAL_FLOW_IP4_3TUPLE_ADDR;
		flow_cfg->protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg->ip_type = protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", tuple, protocol);
	} else {
		flow_cfg->entry_type = FAL_FLOW_IP4_5TUPLE_ADDR;

		flow_cfg->src_port = tuple->flow_ident;
		flow_cfg->dst_port = tuple->return_ident;

		ppe_drv_trace("%p: flow_tbl[sport]: %u", tuple, flow_cfg->src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", tuple, flow_cfg->dst_port);

	}

	/*
	 * query hardware
	 */
	err = fal_flow_host_get(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_host);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow get failed", tuple);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg->entry_id];
	ppe_drv_assert((flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already decelerated from PPE at index: %d",
			tuple, flow_cfg->entry_id);

	ppe_drv_assert((host_cfg->entry_id == flow->host->index),
			"%p flow entry and host entry mismatch flow-index: %d hw-host_index: %d sw-host_index: %d",
			tuple, flow->index, host_cfg->entry_id, flow->host->index);

	ppe_drv_trace("%p: flow_tbl entry found at index: %u", tuple, flow_cfg->entry_id);
	return flow;
}

/*
 * ppe_drv_flow_v4_add()
 *	Add flow table entry.
 */
struct ppe_drv_flow *ppe_drv_flow_v4_add(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_nexthop *nh,
					struct ppe_drv_host *host, bool entry_valid)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_flow_entry_t flow_cfg = {0};
	uint32_t match_src_ip = ppe_drv_v4_conn_flow_match_src_ip_get(pcf);
	uint32_t match_dest_ip = ppe_drv_v4_conn_flow_match_dest_ip_get(pcf);
	uint32_t xlate_src_ip = ppe_drv_v4_conn_flow_xlate_src_ip_get(pcf);
	uint32_t xlate_dest_ip = ppe_drv_v4_conn_flow_xlate_dest_ip_get(pcf);
	uint32_t match_protocol = ppe_drv_v4_conn_flow_match_protocol_get(pcf);
	uint8_t vlan_hdr_cnt = ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf);
	struct ppe_drv_iface *port_if = ppe_drv_v4_conn_flow_eg_port_if_get(pcf);
	struct ppe_drv_port *pp = ppe_drv_iface_port_get(port_if);
	struct ppe_drv_flow *flow;
	bool tuple_3 = false;
	bool wifi_qos_en;
	uint16_t xmit_mtu;
	sw_error_t err;

	/*
	 * Ensure it falls under SNAT, DNAT or simple L3 routing
	 */
	if ((match_src_ip != xlate_src_ip && match_dest_ip != xlate_dest_ip)
		&& (!ipv4_is_multicast(match_dest_ip))) {
		ppe_drv_warn("%p: both source and dest IPs are getting translated", pcf);
		return NULL;
	}

	ppe_drv_trace("%p: flow_tbl[host_idx]: %u", pcf, host->index);
	flow_cfg.host_addr_type = PPE_DRV_HOST_LAN;
        flow_cfg.host_addr_index = host->index;
        flow_cfg.deacclr_en = false;
        flow_cfg.invalid = !entry_valid;

	flow_cfg.sevice_code = PPE_DRV_SC_NONE;
	if (!ppe_drv_flow_v4_service_code_get(pcf, &flow_cfg.sevice_code)) {
		ppe_drv_warn("%p: failed to obtain a valid service code", pcf);
		return NULL;
	}

	/*
	 * Get the tree ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v4_tree_id_get(pcf, &flow_cfg.tree_id)) {
		ppe_drv_warn("%p: failed to obtain a valid tree ID", pcf);
		return NULL;
	}

	/*
	 * Get the VPN ID corresponding to flow.
	 */
	if (!ppe_drv_flow_v4_vpn_id_get(pcf, &flow_cfg.vpn_id)) {
		ppe_drv_warn("%p: failed to obtain a valid vpn_id", pcf);
		return NULL;
	}

	/*
	 * Get the WIFI QOS corresponding to flow.
	 */
	if (!ppe_drv_flow_v4_wifi_qos_get(pcf, &flow_cfg.wifi_qos, &wifi_qos_en)) {
		ppe_drv_warn("%p: failed to obtain wifi qos", pcf);
		return NULL;
	}

	flow_cfg.wifi_qos_en = wifi_qos_en;

	/*
	 * Set forwarding type
	 */
	if (ipv4_is_multicast(match_dest_ip)) {
		/*
		 * Multicast flow
		 */
		flow_cfg.fwd_type = FAL_FLOW_FORWARD;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2-Multicast: %u", pcf, FAL_FLOW_FORWARD);
	} else if (match_src_ip != xlate_src_ip) {
		/*
		 * Case SNAT
		 */
		flow_cfg.fwd_type = FAL_FLOW_SNAT;
		flow_cfg.snat_nexthop = nh->index;
		flow_cfg.snat_srcport = ppe_drv_v4_conn_flow_xlate_src_ident_get(pcf);
		ppe_drv_trace("%p: flow_tbl[nat_type]: SNAT: %u snat_port: %u", pcf, FAL_FLOW_SNAT, flow_cfg.snat_srcport);
	} else if (match_dest_ip != xlate_dest_ip) {
		/*
		 * Case DNAT
		 */
		flow_cfg.fwd_type = FAL_FLOW_DNAT;
		flow_cfg.dnat_nexthop = nh->index;
		flow_cfg.dnat_dstport = ppe_drv_v4_conn_flow_xlate_dest_ident_get(pcf);
		ppe_drv_trace("%p: flow_tbl[nat_type]: DNAT: %u dnat_port: %u", pcf, FAL_FLOW_DNAT, flow_cfg.dnat_dstport);
	} else if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		/*
		 * Case bridging
		 */
		flow_cfg.fwd_type = FAL_FLOW_BRIDGE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L2: %u", pcf, FAL_FLOW_BRIDGE);

		flow_cfg.port_valid = true;
		flow_cfg.bridge_port = pp->port;
		ppe_drv_trace("%p: xmit interface port: %d", pcf, pp->port);

		if (ppe_drv_port_is_tunnel_vp(pp)) {
			switch (vlan_hdr_cnt) {
			case 2:
				flow_cfg.svlan_fmt = true;
				flow_cfg.cvlan_fmt = true;
				flow_cfg.vlan_fmt_valid = 1;
				break;
			case 1:
				flow_cfg.cvlan_fmt = true;
				flow_cfg.vlan_fmt_valid = 1;
				break;
			case 0:
				break;
			default:
				ppe_drv_warn("%p: cannot accelerate bridge VLAN flows for more than 2 vlans: %u",
						pcf, vlan_hdr_cnt);
				return NULL;
			}
		}
	} else {
		/*
		 * Case simple routing (Non-NAT)
		 */
		flow_cfg.route_nexthop = nh->index;
		flow_cfg.fwd_type = FAL_FLOW_ROUTE;
		ppe_drv_trace("%p: flow_tbl[fwd_type]: L3: %u", pcf, FAL_FLOW_ROUTE);
	}

	/*
	 * Fill relevant details for 3-tuple and 5-tuple flows.
	 */
	switch (match_protocol) {
	case IPPROTO_TCP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_TCP;
		ppe_drv_trace("%p: flow_tbl[protocol]: TCP-%u", pcf, FAL_FLOW_PROTOCOL_TCP);
		break;

	case IPPROTO_UDP:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDP;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-%u", pcf, FAL_FLOW_PROTOCOL_UDP);
		break;

	case IPPROTO_UDPLITE:
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_UDPLITE;
		ppe_drv_trace("%p: flow_tbl[protocol]: UDP-Lite-%u", pcf, FAL_FLOW_PROTOCOL_UDPLITE);
		break;

	case IPPROTO_ESP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	case IPPROTO_IPIP:
		tuple_3 = true;
		ppe_drv_trace("%p: flow_tbl[protocol]: Other-%u", pcf, FAL_FLOW_PROTOCOL_OTHER);
		break;

	default:
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_unknown_proto);
		ppe_drv_warn("%p: protocol: %u cannot be offloaded to PPE", pcf, match_protocol);
		return NULL;
	}

	flow_cfg.flow_ip.ipv4 = match_dest_ip;
	ppe_drv_trace("%p: flow_tbl[dest_ip]: %pI4", pcf, &match_dest_ip);
	if (tuple_3) {
		flow_cfg.entry_type = FAL_FLOW_IP4_3TUPLE_ADDR;
		flow_cfg.protocol = FAL_FLOW_PROTOCOL_OTHER;
		flow_cfg.ip_type = match_protocol;
		ppe_drv_trace("%p: flow_tbl[ip_proto]: 0x%x", pcf, match_protocol);
	} else {
		flow_cfg.entry_type = FAL_FLOW_IP4_5TUPLE_ADDR;

		flow_cfg.src_port = ppe_drv_v4_conn_flow_match_src_ident_get(pcf);
		flow_cfg.dst_port = ppe_drv_v4_conn_flow_match_dest_ident_get(pcf);

		ppe_drv_trace("%p: flow_tbl[sport]: %u", pcf, flow_cfg.src_port);
		ppe_drv_trace("%p: flow_tbl[dport]: %u", pcf, flow_cfg.dst_port);

	}

	/*
	 * Fill Path-MTU.
	 *
	 * Note: For multicast flows, it should be ok to program the minimum MTU value
	 * of all the interfaces, since PPE also check MTU for each destination interface
	 * and exception the packet (without cloning) if MTU check fail for any interface.
	 */
	xmit_mtu = ipv4_is_multicast(match_dest_ip) ? ppe_drv_v4_conn_flow_mc_min_mtu_get(pcf)
		: ppe_drv_v4_conn_flow_xmit_interface_mtu_get(pcf);
	if (xmit_mtu > PPE_DRV_PORT_JUMBO_MAX) {
		ppe_drv_trace("%p: xmit_mtu: %d is larger, restricting to max: %d", pcf, xmit_mtu, PPE_DRV_PORT_JUMBO_MAX);
		xmit_mtu = PPE_DRV_PORT_JUMBO_MAX;
	}

	flow_cfg.pmtu_check_l3 = PPE_DRV_FLOW_PMTU_TYPE_L3;
	flow_cfg.pmtu = xmit_mtu;

	ppe_drv_trace("%p: flow_tbl[PMTU]: %u", pcf, xmit_mtu);

	/*
	 * Add the flow
	 */
	err = fal_flow_entry_add(PPE_DRV_SWITCH_ID, FAL_FLOW_OP_MODE_KEY, &flow_cfg);
	if (err != SW_OK) {
		ppe_drv_trace("%p: flow entry add failed", pcf);
		return NULL;
	}

	/*
	 * Get the sw instance of flow entry.
	 */
	flow = &p->flow[flow_cfg.entry_id];
	ppe_drv_assert(!(flow->flags & PPE_DRV_FLOW_VALID), "%p: flow entry is already accelerated to PPE at index: %d",
			pcf, flow_cfg.entry_id);

	/*
	 * Update the shadow copy
	 */
	flow->host = host;
	flow->nh = nh;
	flow->flags |= PPE_DRV_FLOW_V4;
	flow->type = PPE_DRV_IP_TYPE_V4;
	flow->entry_type = flow_cfg.entry_type;
	flow->pcf.v4 = pcf;
	ppe_drv_trace("%p: flow_tbl entry added at index: %u", pcf, flow_cfg.entry_id);
	return flow;
}

/*
 * ppe_drv_flow_entries_free()
 *	Free flow table entries if it was allocated.
 */
void ppe_drv_flow_entries_free(struct ppe_drv_flow *flow)
{
	vfree(flow);
}

/*
 * ppe_drv_flow_entries_alloc()
 *	Allocated and initialize requested number of flow table entries.
 */
struct ppe_drv_flow *ppe_drv_flow_entries_alloc()
{
	uint16_t i;
	struct ppe_drv_flow *flow;
	struct ppe_drv *p = &ppe_drv_gbl;

	flow = vzalloc(sizeof(struct ppe_drv_flow) * p->flow_num);
	if (!flow) {
		ppe_drv_warn("%p: failed to allocate flow entries", p);
		return NULL;
	}

	/*
	 * Assign flow index values to the flow entries
	 */
	for (i = 0; i < p->flow_num; i++) {
		flow[i].index = i;
	}

	return flow;
}
