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

#include "ppe_drv.h"

static inline void ppe_drv_v4_flow_vlan_set(struct ppe_drv_v4_conn_flow *pcf,
		uint32_t primary_ingress_vlan_tag, uint32_t primary_egress_vlan_tag,
		uint32_t secondary_ingress_vlan_tag, uint32_t secondary_egress_vlan_tag)
{
	if ((primary_ingress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->ingress_vlan[0].tpid = primary_ingress_vlan_tag >> 16;
		pcf->ingress_vlan[0].tci = (uint16_t) primary_ingress_vlan_tag;
		pcf->ingress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->ingress_vlan[0].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v4_conn_flow_flags_set(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}

	if ((secondary_ingress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->ingress_vlan[1].tpid = secondary_ingress_vlan_tag >> 16;
		pcf->ingress_vlan[1].tci = (uint16_t) secondary_ingress_vlan_tag;
		pcf->ingress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->ingress_vlan[1].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v4_conn_flow_flags_set(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}

	if ((primary_egress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->egress_vlan[0].tpid = primary_egress_vlan_tag >> 16;
		pcf->egress_vlan[0].tci = (uint16_t) primary_egress_vlan_tag;
		pcf->egress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->egress_vlan[0].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v4_conn_flow_flags_set(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}

	if ((secondary_egress_vlan_tag & PPE_DRV_VLAN_ID_MASK) != PPE_DRV_VLAN_NOT_CONFIGURED) {
		pcf->egress_vlan[1].tpid = ((secondary_egress_vlan_tag & PPE_DRV_VLAN_TPID_MASK) >> 16);
		pcf->egress_vlan[1].tci = (secondary_egress_vlan_tag & PPE_DRV_VLAN_TCI_MASK);
		pcf->egress_vlan_cnt++;

		/*
		 * Check if flow needs 802.1p marking.
		 */
		if (pcf->egress_vlan[1].tci & PPE_DRV_VLAN_PRIORITY_MASK) {
			ppe_drv_v4_conn_flow_flags_set(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_VLAN_PRI_MARKING);
		}
	}
}

/*
 * ppe_drv_v4_conn_fill()
 *	Populate each direction flow object.
 */
static ppe_drv_ret_t ppe_drv_v4_conn_fill(struct ppe_drv_v4_rule_create *create, struct ppe_drv_v4_conn *cn)
{
	struct ppe_drv_v4_connection_rule *conn = &create->conn_rule;
	struct ppe_drv_v4_5tuple *tuple = &create->tuple;
	struct ppe_drv_pppoe_session *flow_pppoe_rule = &create->pppoe_rule.flow_session;
	struct ppe_drv_pppoe_session *return_pppoe_rule = &create->pppoe_rule.return_session;
	struct ppe_drv_vlan_info *vlan_primary_rule = &create->vlan_rule.primary_vlan;
	struct ppe_drv_vlan_info *vlan_secondary_rule = &create->vlan_rule.secondary_vlan;
	struct ppe_drv_iface *if_rx, *if_tx, *top_if_rx, *top_if_tx;
	struct ppe_drv_top_if_rule *top_rule = &create->top_rule;
	struct ppe_drv_v4_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_v4_conn_flow *pcr = &cn->pcr;
	uint16_t valid_flags = create->valid_flags;
	uint16_t rule_flags = create->rule_flags;
	struct ppe_drv_port *pp_rx, *pp_tx;
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * Make sure both Rx and Tx inteface are mapped to PPE ports properly.
	 */
	if_rx = ppe_drv_iface_get_by_idx(conn->rx_if);
	if (!if_rx) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_invalid_rx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to rx_if: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_rx = ppe_drv_iface_port_get(if_rx);
	if (!pp_rx) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_invalid_rx_port);
		ppe_drv_warn("%p: Invalid Rx IF: %d", create, conn->rx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	if_tx = ppe_drv_iface_get_by_idx(conn->tx_if);
	if (!if_tx) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_invalid_tx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	pp_tx = ppe_drv_iface_port_get(if_tx);
	if (!pp_tx) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_invalid_tx_port);
		ppe_drv_warn("%p: Invalid Tx IF: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_IFACE_PORT_MAP;
	}

	top_if_rx = ppe_drv_iface_get_by_idx(top_rule->rx_if);
	if (!if_rx) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_invalid_rx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to rx_if: %d", create, top_rule->rx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	top_if_tx = ppe_drv_iface_get_by_idx(top_rule->tx_if);
	if (!if_tx) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_invalid_tx_if);
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", create, top_rule->tx_if);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * Bridge flow
	 */
	if (rule_flags & PPE_DRV_V4_RULE_FLAG_BRIDGE_FLOW) {
		/*
		 * Bridge flow with NAT?
		 */
		if ((tuple->flow_ip != conn->flow_ip_xlate) || (tuple->return_ip != conn->return_ip_xlate)) {
			ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_bridge_nat);
			ppe_drv_warn("%p: NAT not support with bridge flows! rule_flags: 0x%x "
					"flow_ip: %pI4 flow_ip_xlate: %pI4 return_ip: %pI4 return_ip_xlate: %pI4",
					create, rule_flags, &tuple->flow_ip, &conn->flow_ip_xlate,
					&tuple->return_ip, &conn->return_ip_xlate);
			return PPE_DRV_RET_FAILURE_BRIDGE_NAT;
		}

		ppe_drv_v4_conn_flow_flags_set(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW);
		ppe_drv_v4_conn_flow_flags_set(pcr, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW);
	}


	/*
	 * Note: PPE can't support both SNAT and DNAT simultaneously.
	 */
	if ((tuple->flow_ip != conn->flow_ip_xlate) && (tuple->return_ip != conn->return_ip_xlate)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_snat_dnat);
		ppe_drv_warn("%p: Invalid Tx IF: %d", create, conn->tx_if);
		return PPE_DRV_RET_FAILURE_SNAT_DNAT_SIMUL;
	}

	/*
	 * Prepare flow direction rule
	 */
	if (rule_flags & PPE_DRV_V4_RULE_FLAG_FLOW_VALID) {
		ppe_drv_v4_conn_flow_conn_set(pcf, cn);

		/*
		 * Set Rx and Tx port.
		 */
		ppe_drv_v4_conn_flow_rx_port_set(pcf, pp_rx);
		ppe_drv_v4_conn_flow_tx_port_set(pcf, pp_tx);

		/*
		 * Set 5-tuple along with SNAT/DNAT requirement.
		 */
		ppe_drv_v4_conn_flow_match_protocol_set(pcf, tuple->protocol);
		ppe_drv_v4_conn_flow_match_src_ip_set(pcf, tuple->flow_ip);
		ppe_drv_v4_conn_flow_match_src_ident_set(pcf, tuple->flow_ident);
		ppe_drv_v4_conn_flow_match_dest_ip_set(pcf, tuple->return_ip);
		ppe_drv_v4_conn_flow_match_dest_ident_set(pcf, tuple->return_ident);
		ppe_drv_v4_conn_flow_xlate_src_ip_set(pcf, conn->flow_ip_xlate);
		ppe_drv_v4_conn_flow_xlate_src_ident_set(pcf, conn->flow_ident_xlate);
		ppe_drv_v4_conn_flow_xlate_dest_ip_set(pcf, conn->return_ip_xlate);
		ppe_drv_v4_conn_flow_xlate_dest_ident_set(pcf, conn->return_ident_xlate);

		/*
		 * Flow MTU and transmit MAC address.
		 */
		ppe_drv_v4_conn_flow_xmit_interface_mtu_set(pcf, conn->return_mtu);
		ppe_drv_v4_conn_flow_xmit_dest_mac_addr_set(pcf, conn->return_mac);

		if (valid_flags & PPE_DRV_V4_VALID_FLAG_VLAN) {
			pcf->ingress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcf->ingress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcf->egress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcf->egress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			ppe_drv_v4_flow_vlan_set(pcf, vlan_primary_rule->ingress_vlan_tag,
					vlan_primary_rule->egress_vlan_tag,
					vlan_secondary_rule->ingress_vlan_tag,
					vlan_secondary_rule->egress_vlan_tag);
		}

		if (valid_flags & PPE_DRV_V4_VALID_FLAG_RETURN_PPPOE) {
			ppe_drv_v4_conn_flow_pppoe_session_id_set(pcf, return_pppoe_rule->session_id);
			ppe_drv_v4_conn_flow_pppoe_server_mac_set(pcf, return_pppoe_rule->server_mac);
			ppe_drv_v4_conn_flow_flags_set(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_PPPOE_FLOW);
		}

		/*
		 * Bridge + VLAN? Make sure both top interfaces are attached to same parent.
		 */
		if ((rule_flags & PPE_DRV_V4_RULE_FLAG_BRIDGE_FLOW)
		       && (ppe_drv_v4_conn_flow_ingress_vlan_cnt_get(pcf)
			|| ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf))) {

			if (ppe_drv_iface_parent_get(top_if_rx) != ppe_drv_iface_parent_get(top_if_tx)) {
				ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_vlan_filter);
				ppe_drv_warn("%p: IF not part of same bridge rx_if: %d tx_if: %d",
						create, top_rule->rx_if, top_rule->tx_if);
				return PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES;
			}
		}
	}

	/*
	 * Prepare return direction rule
	 */
	if (rule_flags & PPE_DRV_V4_RULE_FLAG_RETURN_VALID) {
		ppe_drv_v4_conn_flow_conn_set(pcr, cn);

		/*
		 * Set Rx and Tx port.
		 */
		ppe_drv_v4_conn_flow_rx_port_set(pcr, pp_tx);
		ppe_drv_v4_conn_flow_tx_port_set(pcr, pp_rx);

		/*
		 * Set 5-tuple along with SNAT/DNAT requirement.
		 */
		ppe_drv_v4_conn_flow_match_protocol_set(pcr, tuple->protocol);
		ppe_drv_v4_conn_flow_match_src_ip_set(pcr, conn->return_ip_xlate);
		ppe_drv_v4_conn_flow_match_src_ident_set(pcr, conn->return_ident_xlate);
		ppe_drv_v4_conn_flow_match_dest_ip_set(pcr, conn->flow_ip_xlate);
		ppe_drv_v4_conn_flow_match_dest_ident_set(pcr, conn->flow_ident_xlate);
		ppe_drv_v4_conn_flow_xlate_src_ip_set(pcr, tuple->return_ip);
		ppe_drv_v4_conn_flow_xlate_src_ident_set(pcr, tuple->return_ident);
		ppe_drv_v4_conn_flow_xlate_dest_ip_set(pcr, tuple->flow_ip);
		ppe_drv_v4_conn_flow_xlate_dest_ident_set(pcr, tuple->flow_ident);

		/*
		 * Flow MTU and transmit MAC address.
		 */
		ppe_drv_v4_conn_flow_xmit_interface_mtu_set(pcr, conn->flow_mtu);
		ppe_drv_v4_conn_flow_xmit_dest_mac_addr_set(pcr, conn->flow_mac);

		if (valid_flags & PPE_DRV_V4_VALID_FLAG_VLAN) {
			pcr->ingress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcr->ingress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcr->egress_vlan[0].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			pcr->egress_vlan[1].tci = PPE_DRV_VLAN_NOT_CONFIGURED;
			ppe_drv_v4_flow_vlan_set(pcr, vlan_primary_rule->egress_vlan_tag,
					vlan_primary_rule->ingress_vlan_tag,
					vlan_secondary_rule->egress_vlan_tag,
					vlan_secondary_rule->ingress_vlan_tag);
		}

		if (valid_flags & PPE_DRV_V4_VALID_FLAG_FLOW_PPPOE) {
			ppe_drv_v4_conn_flow_pppoe_session_id_set(pcr, flow_pppoe_rule->session_id);
			ppe_drv_v4_conn_flow_pppoe_server_mac_set(pcr, flow_pppoe_rule->server_mac);
			ppe_drv_v4_conn_flow_flags_set(pcr, PPE_DRV_V4_CONN_FLOW_FLAG_PPPOE_FLOW);
		}

		/*
		 * Bridge + VLAN? Make sure both top interfaces are attached to same parent.
		 */
		if ((rule_flags & PPE_DRV_V4_RULE_FLAG_BRIDGE_FLOW)
		       && (ppe_drv_v4_conn_flow_ingress_vlan_cnt_get(pcr)
			|| ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcr))) {

			if (ppe_drv_iface_parent_get(top_if_rx) != ppe_drv_iface_parent_get(top_if_tx)) {
				ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_vlan_filter);
				ppe_drv_warn("%p: IF not part of same bridge rx_if: %d tx_if: %d",
						create, top_rule->rx_if, top_rule->tx_if);
				return PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES;
			}
		}
	};;

	return PPE_DRV_RET_SUCCESS;
}

/*
 * ppe_drv_v4_if_walk_release()
 *	Release references taken during interface hierarchy walk.
 */
static void ppe_drv_v4_if_walk_release(struct ppe_drv_v4_conn_flow *pcf)
{
	if (pcf->eg_vsi_if) {
		ppe_drv_iface_deref_internal(pcf->eg_vsi_if);
		pcf->eg_vsi_if = NULL;
	}

	if (pcf->eg_l3_if) {
		ppe_drv_iface_deref_internal(pcf->eg_l3_if);
		pcf->eg_l3_if = NULL;
	}

	if (pcf->eg_port_if) {
		ppe_drv_iface_deref_internal(pcf->eg_port_if);
		pcf->eg_port_if = NULL;
	}
}

/*
 * ppe_drv_v4_if_walk()
 *	Walk iface heirarchy to obtain egress L3_IF and VSI
 */
static bool ppe_drv_v4_if_walk(struct ppe_drv_v4_conn_flow *pcf, struct ppe_drv_top_if_rule *top_if, ppe_drv_iface_t tx_if)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *eg_vsi_if = NULL;
	struct ppe_drv_iface *eg_l3_if = NULL;
	struct ppe_drv_iface *iface, *top_iface = NULL;
	struct ppe_drv_iface *tx_port_if = NULL;
	uint32_t egress_vlan_inner = PPE_DRV_VLAN_NOT_CONFIGURED, egress_vlan_outer = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf);

	switch (vlan_cnt) {
	case 2:
		egress_vlan_inner = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		egress_vlan_outer = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 1:
		egress_vlan_inner = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 0:
		break;
	default:
		return false;
	}

	/*
	 * Should have a valid top interface.
	 */
	tx_port_if = ppe_drv_iface_get_by_idx(tx_if);
	if (!tx_port_if) {
		ppe_drv_warn("%p: No PPE interface corresponding to tx_if: %d", p, tx_if);
		return false;
	}

	/*
	 * if it's a bridge flow, hierarchy walk not needed.
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		ppe_drv_v4_conn_flow_eg_port_if_set(pcf, ppe_drv_iface_ref(tx_port_if));
		ppe_drv_info("%p: No PPE interface corresponding\n", p);
		return true;
	}

	/*
	 * Should have a valid top interface.
	 */
	iface = top_iface = ppe_drv_iface_get_by_idx(top_if->tx_if);
	if (!iface) {
		ppe_drv_warn("%p: No PPE interface corresponding\n", p);
		return false;
	}

	/*
	 * Walk through if hierarchy.
	 */
	while (iface) {
		/*
		 * Routing to bridge device?
		 * Bridge's VSI and L3_if is the final egress VSI and egress L3_IF.
		 */
		if (iface->type == PPE_DRV_IFACE_TYPE_BRIDGE) {
			eg_vsi_if = iface;
			break;
		}

		if ((iface->type == PPE_DRV_IFACE_TYPE_VLAN)) {
			if (ppe_drv_vsi_match_vlan(ppe_drv_iface_vsi_get(iface), egress_vlan_inner, egress_vlan_outer)) {
				eg_vsi_if = iface;

				/*
				 * If desired eg_l3_if is also available, no need to continue the walk.
				 */
				if (eg_l3_if) {
					break;
				}
			}
		}

		if (iface->type == PPE_DRV_IFACE_TYPE_PPPOE) {
			if (ppe_drv_l3_if_pppoe_match(ppe_drv_iface_l3_if_get(iface), pcf->pppoe_session_id, pcf->pppoe_server_mac)) {
				eg_l3_if = iface;

				/*
				 * If desired eg_vsi_if is also available, no need to continue the walk.
				 */
				if (eg_vsi_if) {
					break;
				}
			}
		}

		iface = ppe_drv_iface_base_get(iface);
	}

	/*
	 * For create request with egress-VLAN, there must be a corresponding egress-VSI IF.
	 */
	if (ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf) && !eg_vsi_if) {
		ppe_drv_warn("%p: not able to find a matching vlan-if", pcf);
		return false;
	}

	/*
	 * For create request with egress-pppoe, there must be a corresponding egress-L3 IF.
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_PPPOE_FLOW) && !eg_l3_if) {
		ppe_drv_warn("%p: not able to find a matching pppoe-if", pcf);
		return false;
	}

	/*
	 * Take the reference on egress interfaces used for this flow.
	 * Dereference: connection destroy.
	 */
	pcf->eg_vsi_if = eg_vsi_if ? ppe_drv_iface_ref(eg_vsi_if) : NULL;
	pcf->eg_l3_if = eg_l3_if ? ppe_drv_iface_ref(eg_l3_if) : NULL;
	pcf->eg_port_if = ppe_drv_iface_ref(tx_port_if);
	return true;
}

/*
 * ppe_drv_v4_flow_check()
 *	Search an entry into the flow table and returns the flow object.
 */
static bool ppe_drv_v4_flow_check(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv_v4_5tuple tuple;
	struct ppe_drv_flow *flow = NULL;

	tuple.flow_ip = ppe_drv_v4_conn_flow_match_src_ip_get(pcf);
	tuple.flow_ident = ppe_drv_v4_conn_flow_match_src_ident_get(pcf);
	tuple.return_ip = ppe_drv_v4_conn_flow_match_dest_ip_get(pcf);
	tuple.return_ident = ppe_drv_v4_conn_flow_match_dest_ident_get(pcf);
	tuple.protocol = ppe_drv_v4_conn_flow_match_protocol_get(pcf);

	/*
	 * Get flow table entry.
	 */
	flow = ppe_drv_flow_v4_get(&tuple);
	if (!flow) {
		ppe_drv_info("%p: flow entry not found", pcf);
		return false;
	}

	ppe_drv_info("%p: flow found: index=%d host_idx=%d", pcf, flow->index, flow->host->index);

	return true;
}

/*
 * ppe_drv_v4_flow_del()
 *	Delete an entry from the flow table.
 */
static bool ppe_drv_v4_flow_del(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = pcf->pf;

	ppe_drv_trace("%p, flow deletion request for flow-idx: %d host-idx: %u", pcf, flow->index, flow->host->index);

	/*
	 * As opposed to 'add', flow 'delete' needs host and flow entry to be handled separately.
	 * Note: a host entry or nexthop entry could be referenced by multiple flow entries.
	 */
	if (!ppe_drv_flow_del(flow)) {
		ppe_drv_warn("%p: flow entry deletion failed for flow_index: %d", flow, flow->index);
		return false;
	}

	/*
	 * Release host entry.
	 */
	ppe_drv_host_deref(flow->host);
	flow->host = NULL;

	/*
	 * Release nexthop entry.
	 */
	if (flow->nh) {
		ppe_drv_nexthop_deref(flow->nh);
		flow->nh = NULL;
	}

	/*
	 * Clear the QOS map information.
	 */
	ppe_drv_flow_v4_qos_clear(flow);

	/*
	 * Read stats and clear counters for deleted flow.
	 * Note: it is assured that no new flow can take the same index since all of this
	 * is lock protected. Unless this operation is complete, a new flow cannot be offloaded.
	 *
	 * TODO: update this with stats patch.
	 */
	/* ppe_drv_v4_conn_stats_sync(p, pf->pcf); */

	ppe_drv_flow_stats_clear(flow);

	/*
	 * Update stats
	 */
	if (flow->flags & PPE_DRV_FLOW_BRIDGED) {
		ppe_drv_stats_dec(&p->stats.gen_stats.v4_l2_flows);
	} else {
		ppe_drv_stats_dec(&p->stats.gen_stats.v4_l3_flows);
	}

	/*
	 * Clear the flow entry in SW so that it can be reused.
	 */
	flow->pkts = 0;
	flow->bytes = 0;
	flow->flags = 0;
	flow->service_code = 0;
	flow->type = 0;

	return true;
}

/*
 * ppe_drv_v4_flow_add()
 *	Adds an entry into the flow table and returns the flow index.
 */
static struct ppe_drv_flow *ppe_drv_v4_flow_add(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_nexthop *nh = NULL;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_host *host = NULL;

	/*
	 * Fetch a new nexthop entry.
	 * Note: NEXTHOP entry is not required for bridged flows.
	 */
	if (!ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		nh = ppe_drv_nexthop_v4_get_and_ref(pcf);
		if (!nh) {
			ppe_drv_warn("%p: unable to allocate nexthop", pcf);
			return NULL;
		}
	}

	/*
	 * Add host table entry.
	 */
	host = ppe_drv_host_v4_add(pcf);
	if (!host) {
		ppe_drv_warn("%p: host entry add failed for conn flow", pcf);
		goto flow_add_fail;
	}

	/*
	 * Add flow table entry.
	 */
	flow = ppe_drv_flow_v4_add(pcf, nh, host, false);
	if (!flow) {
		ppe_drv_warn("%p: flow entry add failed for conn flow", pcf);
		goto flow_add_fail;
	}

	ppe_drv_info("%p: flow accelerated: index=%d host_idx=%d", pcf, flow->index, host->index);

	/*
	 * qos mapping can updated only after flow entry is added. It is added at the same index
	 * as that of flow entry.
	 */
	if (!ppe_drv_flow_v4_qos_set(pcf, flow)) {
		ppe_drv_warn("%p: qos mapping failed for flow: %p", pcf, flow);
		goto flow_add_fail;
	}

	/*
	 * Now the QOS mapping is set, mark the flow entry as valid.
	 */
	if (!ppe_drv_flow_valid_set(flow, true)) {
		ppe_drv_warn("%p: flow entry valid set failed for flow: %p", pcf, flow);
		goto flow_add_fail;
	}

	/*
	 * Update stats
	 */
	if (ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_BRIDGE_FLOW)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_l2_flows);
	} else {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_l3_flows);
	}

	ppe_drv_host_dump(flow->host);
	ppe_drv_flow_dump(flow);
	if (flow->nh) {
		ppe_drv_nexthop_dump(flow->nh);
	}

	return flow;

flow_add_fail:
	if (flow) {
		ppe_drv_flow_del(flow);
	}

	if (host) {
		ppe_drv_host_deref(host);
	}

	if (nh) {
		ppe_drv_nexthop_deref(nh);
	}

	return NULL;
}

/*
 * ppe_drv_v4_mc_destroy()
 *	Destroy a multicast connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v4_mc_destroy(struct ppe_drv_v4_rule_destroy *destroy)
{
	return PPE_DRV_RET_FAILURE_NOT_SUPPORTED;
}

/*
 * ppe_drv_v4_mc_update()
 *	Update member list of ports in l2 multicast group.
 */
ppe_drv_ret_t ppe_drv_v4_mc_update(struct ppe_drv_v4_rule_create *update)
{
	return PPE_DRV_RET_FAILURE_NOT_SUPPORTED;
}

/*
 * ppe_drv_v4_mc_create()
 *	Adds a l2 multicast flow and host entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v4_mc_create(struct ppe_drv_v4_rule_create *create)
{
	return PPE_DRV_RET_FAILURE_NOT_SUPPORTED;
}

/*
 * ppe_drv_v4_destroy()
 *	Destroy a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v4_destroy(struct ppe_drv_v4_rule_destroy *destroy)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_flow *flow = NULL;
	struct ppe_drv_v4_conn_flow *pcf;
	struct ppe_drv_v4_conn_flow *pcr;
	struct ppe_drv_v4_conn *cn;

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&p->stats.gen_stats.v4_destroy_req);

	/*
	 * Get flow table entry.
	 */
	spin_lock_bh(&p->lock);
	flow = ppe_drv_flow_v4_get(&destroy->tuple);
	if (!flow) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_destroy_conn_not_found);
		ppe_drv_warn("%p: flow entry not found", pcf);
		return PPE_DRV_RET_FAILURE_DESTROY_NO_CONN;
	}

	pcf = flow->pcf.v4;
	if (!ppe_drv_v4_flow_del(pcf)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_destroy_fail);
		ppe_drv_warn("%p: deletion of flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
	}

	/*
	 * Release references on interfaces.
	 */
	ppe_drv_v4_if_walk_release(pcf);

	/*
	 * Find the other flow associated with this connection.
	 */
	cn = ppe_drv_v4_conn_flow_conn_get(pcf);
	pcr = (pcf == &cn->pcf) ? &cn->pcr : &cn->pcf;
	if (!ppe_drv_v4_flow_del(pcr)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_destroy_fail);
		ppe_drv_warn("%p: deletion of return flow failed: %p", p, pcf);
		return PPE_DRV_RET_FAILURE_DESTROY_FAIL;
	}

	/*
	 * Release references on interfaces.
	 */
	ppe_drv_v4_if_walk_release(pcr);

	/*
	 * Delete connection entry from the active connection list.
	 */
	list_del(&cn->list);

	/*
	 * We maintain reference per connection on main ppe context.
	 * Dereference: connection destroy.
	 *
	 * TODO: check if this is needed
	 */
	/* ppe_drv_deref(p); */
	spin_unlock_bh(&p->lock);

	/*
	 * Free the connection entry memory.
	 */
	kfree(cn);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_v4_destroy);

/*
 * ppe_drv_v4_create()
 *	Adds a connection entry in PPE.
 */
ppe_drv_ret_t ppe_drv_v4_create(struct ppe_drv_v4_rule_create *create)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v4_conn_flow *pcf = NULL;
	struct ppe_drv_v4_conn_flow *pcr = NULL;
	struct ppe_drv_top_if_rule top_if;
	struct ppe_drv_v4_conn *cn = NULL;
	ppe_drv_ret_t ret;

	/*
	 * Update stats
	 */
	ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_req);

	/*
	 * Allocate a new connection entry
	 *
	 * TODO: kzalloc with GFP_ATOMIC is used while considering sync method, in
	 * that case this API would be called from softirq.
	 *
	 * Revisit if we later handle this in a workqueue in async model.
	 */
	cn = kzalloc(sizeof(struct ppe_drv_v4_conn), GFP_ATOMIC);
	if (!cn) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_mem);
		ppe_drv_warn("%p: failed to allocate connection memory: %p", p, create);
		return PPE_DRV_RET_FAILURE_CREATE_OOM;
	}

	/*
	 * Fill the connection entry.
	 */
	spin_lock_bh(&p->lock);
	ret = ppe_drv_v4_conn_fill(create, cn);
	if (ret != PPE_DRV_RET_SUCCESS) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_conn);
		ppe_drv_warn("%p: failed to fill connection object: %p", p, create);
		goto fail;
	}

	/*
	 * Ensure either direction flow is not already offloaded by us.
	 */
	if (ppe_drv_v4_flow_check(pcf) || ppe_drv_v4_flow_check(pcr)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_collision);
		ppe_drv_warn("%p: create collision detected: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_CREATE_COLLISSION;
		goto fail;
	}

	/*
	 * Perform interface hierarchy walk and obtain egress L3_If and egress VSI
	 * for each direction.
	 */
	top_if.rx_if = create->top_rule.rx_if;
	top_if.tx_if = create->top_rule.tx_if;
	if (!ppe_drv_v4_if_walk(&cn->pcf, &top_if, create->conn_rule.tx_if)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_if_hierarchy);
		ppe_drv_warn("%p: create failed invalid interface hierarchy: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_INVALID_HIERARCHY;
		goto fail;
	}

	pcf = &cn->pcf;

	/*
	 * Reverse the top interfaces for return direction.
	 */
	top_if.rx_if = create->top_rule.tx_if;
	top_if.tx_if = create->top_rule.rx_if;
	if (!ppe_drv_v4_if_walk(&cn->pcr, &top_if, create->conn_rule.rx_if)) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail_if_hierarchy);
		ppe_drv_warn("%p: create failed invalid interface hierarchy: %p", p, create);
		ret = PPE_DRV_RET_FAILURE_INVALID_HIERARCHY;
		goto fail;
	}

	pcr = &cn->pcr;

	/*
	 * Add flow direction flow entry
	 */
	pcf->pf = ppe_drv_v4_flow_add(pcf);
	if (!pcf->pf) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail);
		ppe_drv_warn("%p: acceleration of flow failed: %p", p, pcf);
		ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
		goto fail;
	}

	pcr->pf = ppe_drv_v4_flow_add(pcr);
	if (!pcr->pf) {
		/*
		 * Destroy the offloaded flow entry
		 */
		ppe_drv_v4_flow_del(pcf);

		ppe_drv_stats_inc(&p->stats.gen_stats.v4_create_fail);
		ppe_drv_warn("%p: acceleration of return direction failed: %p", p, pcr);
		ret = PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL;
		goto fail;
	}

	/*
	 * Add connection entry to the active connection list.
	 */
	pcf->conn = cn;
	pcr->conn = cn;
	list_add(&cn->list, &p->conn_v4);

	/*
	 * We maintain reference per connection on main ppe context.
	 * Dereference: connection destroy.
	 *
	 * TODO: check if this needed
	 */
	/* ppe_drv_ref(p); */
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;

fail:
	/*
	 * Free flow direction references.
	 */
	if (pcf) {
		ppe_drv_v4_if_walk_release(pcf);
	}

	/*
	 * Free return direction references.
	 */
	if (pcr) {
		ppe_drv_v4_if_walk_release(pcr);
	}

	spin_unlock_bh(&p->lock);
	kfree(cn);
	return ret;
}
EXPORT_SYMBOL(ppe_drv_v4_create);
