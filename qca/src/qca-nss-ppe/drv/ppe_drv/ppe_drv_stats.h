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

#ifndef __PPE_DRV_STATS_H
#define __PPE_DRV_STATS_H

/*
 * Max service code number.
 * TODO: update it with enum definition
 */
#define PPE_DRV_SC_MAX 256

/*
 * ppe_drv_gen_stats
 *	Message structure for ppe general stats
 */
struct ppe_drv_gen_stats {
        atomic64_t v4_l3_flows;			/* No of v4 routed flows */
        atomic64_t v4_l2_flows;			/* No of v4 bridge flows */
        atomic64_t v4_create_req;		/* No of v4 create requests */
        atomic64_t v4_create_fail;		/* No of v4 create failure */
        atomic64_t v4_destroy_req;		/* No of v4 delete requests */
        atomic64_t v4_destroy_fail;		/* No of v4 delete failure */
	atomic64_t v4_destroy_conn_not_found;	/* No of v4 delete failure due to connection not found */
	atomic64_t v4_host_add_fail;			/* v4 host table add failed */
	atomic64_t v4_create_fail_mem;			/* No of v4 create failure due to OOM */
	atomic64_t v4_create_fail_conn;			/* No of v4 create failure due to invalid parameters */
	atomic64_t v4_create_fail_collision;		/* No of v4 create failure due to connection already exist */
	atomic64_t v4_unknown_interface;		/* No of v4 create failure due to invalid IF */
	atomic64_t v4_create_fail_invalid_rx_if;	/* No of v4 create failure due to invalid Rx IF */
	atomic64_t v4_create_fail_invalid_tx_if;	/* No of v4 create failure due to invalid Tx IF */
	atomic64_t v4_create_fail_invalid_rx_port;	/* No of v4 create failure due to invalid Rx Port */
	atomic64_t v4_create_fail_invalid_tx_port;	/* No of v4 create failure due to invalid Tx Port */
	atomic64_t v4_create_fail_bridge_nat;		/* No of v4 create failure due to NAT with bridge flow */
	atomic64_t v4_create_fail_snat_dnat;		/* No of v4 create failure due to both SNAT and DNAT is requested */
	atomic64_t v4_create_fail_if_hierarchy;		/* No of v4 create failure due to interface hierarchy walk fail */
	atomic64_t v4_create_fail_vlan_filter;		/* No of v4 create failure due to interface not in bridge */

        atomic64_t v6_l3_flows;			/* No of v6 routed flows */
        atomic64_t v6_l2_flows;			/* No of v6 bridge flows */
        atomic64_t v6_create_req;		/* No of v6 create requests */
        atomic64_t v6_create_fail;		/* No of v6 create failure */
        atomic64_t v6_destroy_req;		/* No of v6 delete requests */
        atomic64_t v6_destroy_fail;		/* No of v6 delete failure */
        atomic64_t v6_unknown_interface;	/* No of v6 create failure due to invalid IF */
	atomic64_t v6_host_add_fail;		/* v6 host table add failed */
	atomic64_t v6_destroy_conn_not_found;	/* No of v6 delete failure due to connection not found */
	atomic64_t v6_create_fail_mem;			/* No of v6 create failure due to OOM */
	atomic64_t v6_create_fail_conn;			/* No of v6 create failure due to invalid parameters */
	atomic64_t v6_create_fail_collision;		/* No of v6 create failure due to connection already exist */
	atomic64_t v6_create_fail_invalid_rx_if;	/* No of v6 create failure due to invalid Rx IF */
	atomic64_t v6_create_fail_invalid_tx_if;	/* No of v6 create failure due to invalid Tx IF */
	atomic64_t v6_create_fail_invalid_rx_port;	/* No of v6 create failure due to invalid Rx Port */
	atomic64_t v6_create_fail_invalid_tx_port;	/* No of v6 create failure due to invalid Tx Port */
	atomic64_t v6_create_fail_bridge_nat;		/* No of v6 create failure due to NAT with bridge flow */
	atomic64_t v6_create_fail_if_hierarchy;		/* No of v6 create failure due to interface hierarchy walk fail */
	atomic64_t v6_create_fail_vlan_filter;		/* No of v6 create failure due to interface not in bridge */

	atomic64_t fail_vp_full;		/* Create req fail due to VP table full */
        atomic64_t fail_pp_full;		/* Create req fail due to physical port table full */
        atomic64_t fail_nh_full;		/* Create req fail due to nexthop table full */
        atomic64_t fail_flow_full;		/* Create req fail due to flow table full */
        atomic64_t fail_host_full;		/* Create req fail due to host table full */
        atomic64_t fail_pubip_full;		/* Create req fail due to pub-ip table full */
        atomic64_t fail_dev_port_map;		/* Create req fail due to PPE port not mapped to net-device */
        atomic64_t fail_l3_if_full;		/* Create req fail due to L3_IF table full */
        atomic64_t fail_vsi_full;		/* Create req fail due to VSI table full */
	atomic64_t fail_pppoe_full;		/* Create req fail due to PPPoE table full */
	atomic64_t fail_rw_fifo_full;		/* Create req fail due to rw fifo full */
	atomic64_t fail_flow_command;		/* Create req fail due to PPE flow command failure */
	atomic64_t fail_unknown_proto;		/* Create req fail due to unknown protocol */
	atomic64_t fail_query_unknown_proto;	/* Query fail due to unknown protocol */
	atomic64_t fail_ppe_unresponsive;	/* Create req fail due to PPE not responding */
	atomic64_t ce_opaque_invalid;		/* Request fail due to invalid opaque in CE */
	atomic64_t fail_fqg_full;		/* Create req fail due to flow qos group full */
	atomic64_t fail_ingress_vlan_add;	/* Ingress vlan translation addition failed */
	atomic64_t fail_egress_vlan_add;	/* Egress vlan translation addition failed */
};

/*
 * ppe_drv_stats_sc
 *	Message structure for per service code stats.
 */
struct ppe_drv_stats_sc {
	atomic64_t sc_cb_unregister;	/* Per service-code counter for callback not registered */
	atomic64_t sc_cb_success;	/* Per service-code coutner for successful callback */
	atomic64_t sc_cb_failure;	/* Per service-code counter for failure callback */
};

/*
 * ppe_drv_stats
 *	Message structure for ppe stats
 */
struct ppe_drv_stats {
	struct ppe_drv_gen_stats gen_stats;			/* General connection stats */
	struct ppe_drv_stats_sc sc_stats[PPE_DRV_SC_MAX];	/* Per service-code stats */
};

/*
 * ppe_drv_stats_dec()
 *	Decrement stats counter.
 */
static inline void ppe_drv_stats_dec(atomic64_t *stat)
{
	atomic64_dec(stat);
}

/*
 * ppe_drv_stats_inc()
 *	Increment stats counter.
 */
static inline void ppe_drv_stats_inc(atomic64_t *stat)
{
	atomic64_inc(stat);
}
#endif
