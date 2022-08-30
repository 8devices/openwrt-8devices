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

/**
 * @file ppe_drv.h
 *	NSS PPE driver definitions.
 */

#ifndef _PPE_DRV_H_
#define _PPE_DRV_H_

/**
 * @addtogroup ppe_drv_subsystem
 * @{
 */
#include <linux/if_ether.h>
#include "ppe_drv_iface.h"

/*
 * ppe_drv_ip_type
 *	Types of IP addresses handled
 */
enum ppe_drv_ip_type {
	PPE_DRV_IP_TYPE_V4,	/**< IPv4 unicast IP-type. */
	PPE_DRV_IP_TYPE_V6,	/**< IPv6 unicast IP-type. */
	PPE_DRV_IP_TYPE_MC_V4,	/**< IPv4 multicast IP-type. */
	PPE_DRV_IP_TYPE_MC_V6	/**< IPv6 multicast IP-type. */
};

/**
 * ppe_drv_pppoe_session
 *	Information for PPPoE session.
 */
struct ppe_drv_pppoe_session {
	uint16_t session_id;				/**< Session id */
	uint8_t server_mac[ETH_ALEN];			/**< Server MAC address */
};

/**
 * ppe_drv_pppoe_rule
 *	Information for PPPoE connection rules.
 */
struct ppe_drv_pppoe_rule {
	struct ppe_drv_pppoe_session flow_session;		/**< Flow PPPoE session */
	struct ppe_drv_pppoe_session return_session;		/**< Return PPPoE session */
};

/**
 * ppe_drv_dscp_rule
 *	Information for DSCP connection rules.
 */
struct ppe_drv_dscp_rule {
	uint8_t flow_dscp;		/**< Egress DSCP value for the flow direction. */
	uint8_t return_dscp;		/**< Egress DSCP value for the return direction. */
};

/**
 * ppe_drv_vlan_info
 *	Information for ingress and egress VLANs.
 */
struct ppe_drv_vlan_info {
	uint32_t ingress_vlan_tag;	/**< VLAN tag for the ingress packets. */
	uint32_t egress_vlan_tag;	/**< VLAN tag for egress packets. */
};

/**
 * ppe_drv_vlan_rule
 *	Information for VLAN connection rules.
 */
struct ppe_drv_vlan_rule {
	struct ppe_drv_vlan_info primary_vlan;		/* Primary VLAN info */
	struct ppe_drv_vlan_info secondary_vlan;	/* Secondary VLAN info */
};

/**
 * ppe_drv_qos_rule
 *	Information for QoS connection rules.
 */
struct ppe_drv_qos_rule {
	uint32_t flow_qos_tag;		/**< QoS tag associated with this rule for the flow direction. */
	uint32_t return_qos_tag;	/**< QoS tag associated with this rule for the return direction. */
};

/**
 * ppe_drv_top_if_rule
 *	Information for top interface in hierarchy.
 */
struct ppe_drv_top_if_rule {
	ppe_drv_iface_t rx_if;		/**< Top PPE interface for from direction */
	ppe_drv_iface_t tx_if;		/**< Top PPE interface for return direction */
};

/*
 * ppe_drv_stats_sync_reason
 *	Stats sync reasons.
 */
enum ppe_drv_stats_sync_reason {
	PPE_DRV_STATS_SYNC_REASON_STATS,	/* Sync is to synchronize stats */
	PPE_DRV_STATS_SYNC_REASON_FLUSH,	/* Sync is to flush a connection entry */
	PPE_DRV_STATS_SYNC_REASON_EVICT,	/* Sync is to evict a connection entry */
	PPE_DRV_STATS_SYNC_REASON_DESTROY,	/* Sync is to destroy a connection entry */
};

/**
 * enum ppe_drv_ret
 *	PPE return status
 */
typedef enum ppe_drv_ret {
	PPE_DRV_RET_SUCCESS = 0,			/**< Success */
	PPE_DRV_RET_IFACE_INVALID,			/**< Failure due to Invalid PPE interface */
	PPE_DRV_RET_FAILURE_NOT_SUPPORTED,		/**< Failure due to unsupported feature */
	PPE_DRV_RET_FAILURE_NO_RESOURCE,		/**< Failure due to out of resource */
	PPE_DRV_RET_FAILURE_INVALID_PARAM,		/**< Failure due to invalid parameter */
	PPE_DRV_RET_PORT_NOT_FOUND,			/**< Port not found */
	PPE_DRV_RET_VSI_NOT_FOUND,			/**< VSI not found */
	PPE_DRV_RET_L3_IF_NOT_FOUND,			/**< L3_IF not found */
	PPE_DRV_RET_PORT_ALLOC_FAIL,			/**< Port allocation fails */
	PPE_DRV_RET_L3_IF_ALLOC_FAIL,			/**< L3_IF allocation fails */
	PPE_DRV_RET_L3_IF_PORT_ATTACH_FAIL,		/**< L3_IF PORT attach fails */
	PPE_DRV_RET_VSI_ALLOC_FAIL,			/**< VSI allocation fails */
	PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL,		/**< Mac address clear configuration fails */
	PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL,		/**< Mac address set configuration fails */
	PPE_DRV_RET_MTU_CFG_FAIL,			/**< MTU configuration fails */
	PPE_DRV_RET_DEL_MAC_FDB_FAIL,			/**< Failed to delete FDB entry by MAC */
	PPE_DRV_RET_STA_MOVE_FAIL,			/**< Failed to configure station movement */
	PPE_DRV_RET_NEW_ADDR_LRN_FAIL,			/**< Failed to configure new address learning */
	PPE_DRV_RET_FDB_FLUSH_VSI_FAIL,			/**< Failed to flush FDB entries by VSI */
	PPE_DRV_RET_MEM_IF_INVALID_PORT,		/**< Failed to find port for member interface */
	PPE_DRV_RET_STP_STATE_FAIL,			/**< Failed to set STP state on the bridge port */
	PPE_DRV_RET_IFACE_L3_IF_FAIL,			/**< Failed to find L3_IF for the interface */
	PPE_DRV_RET_PPPOE_ALLOC_FAIL,			/**< PPPOE session allocation failure */
	PPE_DRV_RET_L3_IF_PPPOE_SET_FAIL,		/**< Failed to set PPPOE session information in L3_IF */
	PPE_DRV_RET_BASE_IFACE_NOT_FOUND,		/**< Base interface not found */
	PPE_DRV_RET_VLAN_TPID_FAIL,			/**< VLAN TPID not found */
	PPE_DRV_RET_PORT_ROLE_FAIL,			/**< Port role configuration failed */
	PPE_DRV_RET_INGRESS_VLAN_FAIL,			/**< Ingress vlan configuration failed */
	PPE_DRV_RET_EGRESS_VLAN_FAIL,			/**< Egress vlan configuration failed */
	PPE_DRV_RET_VLAN_INGRESS_DEL_FAIL,		/**< Ingress vlan deletion configuration failed */
	PPE_DRV_RET_VLAN_EGRESS_DEL_FAIL,		/**< Egress vlan deletion configuration failed */
	PPE_DRV_RET_FAILURE_INVALID_HIERARCHY,		/**< Failure due to invalid hierarchy */
	PPE_DRV_RET_FAILURE_SNAT_DNAT_SIMUL,		/**< Failure due to both snat and dnat requested */
	PPE_DRV_RET_FAILURE_NOT_BRIDGE_SLAVES,		/**< Failure due to from and to interfaces not in same bridge */
	PPE_DRV_RET_FAILURE_IFACE_PORT_MAP,
	PPE_DRV_RET_FAILURE_CREATE_COLLISSION,		/**< Failure due to create collision */
	PPE_DRV_RET_FAILURE_CREATE_OOM,			/**< Failure due to memory allocation failed */
	PPE_DRV_RET_FAILURE_FLOW_ADD_FAIL,		/**< Failure due to flow addition failed in hardware */
	PPE_DRV_RET_FAILURE_DESTROY_NO_CONN,		/**< Failure due to connection not found in hardware */
	PPE_DRV_RET_FAILURE_DESTROY_FAIL,		/**< Failure due to connection not found in hardware */
	PPE_DRV_RET_FAILURE_BRIDGE_NAT,			/**< Failure due to Bridge + NAT flows */
} ppe_drv_ret_t;

/** @} */ /* end_addtogroup ppe_drv_subsystem */

#endif /* _PPE_DRV_H_ */
