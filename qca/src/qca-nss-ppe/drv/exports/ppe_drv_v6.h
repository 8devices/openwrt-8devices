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
 * @file ppe_drv_v6.h
 *	NSS PPE IPv6 specific definitions.
 */

#ifndef _PPE_DRV_V6_H_
#define _PPE_DRV_V6_H_

/**
 * @addtogroup ppe_drv_v6_subsystem
 * @{
 */

/*
 * v6 rule flags
 */
#define PPE_DRV_V6_RULE_FLAG_BRIDGE_FLOW	0x0001		/**< Bridge Flow */
#define PPE_DRV_V6_RULE_FLAG_ROUTED_FLOW	0x0002		/**< Rule is for a routed connection */
#define PPE_DRV_V6_RULE_FLAG_DSCP_MARKING	0x0004  	/**< Rule creation for DSCP marking */
#define PPE_DRV_V6_RULE_FLAG_VLAN_MARKING	0x0008		/**< Rule creation for VLAN marking */
#define PPE_DRV_V6_RULE_FLAG_FLOW_VALID		0x0010		/**< Rule creation for flow direction */
#define PPE_DRV_V6_RULE_FLAG_RETURN_VALID	0x0020		/**< Rule creation for return direction */

/*
 * v6 valid flags
 */
#define PPE_DRV_V6_VALID_FLAG_PPPOE_FLOW	0x0001  /**< PPPoE fields are valid for flow direction. */
#define PPE_DRV_V6_VALID_FLAG_PPPOE_RETURN	0x0002  /**< PPPoE fields are valid for return direction. */
#define PPE_DRV_V6_VALID_FLAG_VLAN		0x0004  /**< VLAN fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_QOS		0x0008  /**< QoS fields are valid. */
#define PPE_DRV_V6_VALID_FLAG_DSCP_MARKING	0x0010  /**< DSCP fields are valid. */

/**
 * ppe_drv_v6_5tuple
 *	Common 5-tuple information.
 */
struct ppe_drv_v6_5tuple {
	uint32_t flow_ip[4];		/**< Flow IP address. */
	uint32_t flow_ident;		/**< Flow identifier (e.g., TCP or UDP port). */
	uint32_t return_ip[4];		/**< Return IP address. */
	uint32_t return_ident;		/**< Return identier (e.g., TCP or UDP port). */
	uint8_t protocol;		/**< Protocol number. */
};

/**
 * ppe_drv_v6_connection_rule
 *	Information for creating a connection.
 */
struct ppe_drv_v6_connection_rule {
	uint8_t flow_mac[ETH_ALEN];		/**< Flow MAC address. */
	uint8_t return_mac[ETH_ALEN];		/**< Return MAC address. */
	uint16_t flow_mtu;			/**< MTU for the flow interface. */
	uint16_t return_mtu;			/**< MTU for the return interface. */
	ppe_drv_iface_t rx_if;			/**< From PPE interface number */
	ppe_drv_iface_t tx_if;			/**< To PPE interface number */
};

/**
 * ppe_drv_v6_rule_create
 *	PPE IPv6 rule create structure.
 */
struct ppe_drv_v6_rule_create {
	/*
	 * Request
	 */
	uint16_t valid_flags;				/**< Bit flags associated with the validity of parameters. */
	uint16_t rule_flags;				/**< Bit flags associated with the rule. */
	struct ppe_drv_v6_5tuple tuple;			/**< Holds values of the 5 tuple. */
	struct ppe_drv_v6_connection_rule conn_rule;	/**< Basic connection-specific data. */
	struct ppe_drv_pppoe_rule pppoe_rule;		/**< PPPoE-related acceleration parameters. */
	struct ppe_drv_qos_rule qos_rule;		/**< QoS-related acceleration parameters. */
	struct ppe_drv_dscp_rule dscp_rule;		/**< DSCP-related acceleration parameters. */
	struct ppe_drv_vlan_rule vlan_rule;		/**< VLAN-related acceleration parameters. */
	struct ppe_drv_top_if_rule top_rule;		/**< Parameters related to the top interface in hierarchy. */
};

/**
 * ppe_drv_v6_rule_destroy
 *	PPE IPv6 rule destroy structure.
*/
struct ppe_drv_v6_rule_destroy {
        struct ppe_drv_v6_5tuple tuple;			/**< Holds values of the 5 tuple. */
};

/*
 * ppe_drv_v6_conn_sync
 *	PPE connection sync structure for one connection.
 */
struct ppe_drv_v6_conn_sync {
        uint8_t protocol;				/**< Protocol number. */
        uint32_t flow_ip[4];				/**< Flow IP address. */
        uint32_t flow_ident;				/**< Flow ident (e.g. port). */
        uint32_t flow_rx_packet_count;			/**< Flow interface's RX packet count. */
        uint32_t flow_rx_byte_count;			/**< Flow interface's RX byte count. */
        uint32_t flow_tx_packet_count;			/**< Flow interface's TX packet count. */
        uint32_t flow_tx_byte_count;			/**< Flow interface's TX byte count. */
        uint32_t return_ip[4];				/**< Return IP address. */
        uint32_t return_ident;				/**< Return ident (e.g. port). */
        uint32_t return_rx_packet_count;		/**< Return interface's RX packet count. */
        uint32_t return_rx_byte_count;			/**< Return interface's RX byte count. */
        uint32_t return_tx_packet_count;		/**< Return interface's TX packet count. */
        uint32_t return_tx_byte_count;			/**< Return interface's TX byte count. */
        enum ppe_drv_stats_sync_reason reason;		/**< Reason for the sync. */
};

/*
 * ppe_drv_v6_conn_sync_many
 *	PPE connection sync many structure.
 */
struct ppe_drv_v6_conn_sync_many {
        uint16_t count;					/* How many conn_sync included in this sync callback */
        struct ppe_drv_v6_conn_sync conn_sync[];	/* Connection sync array */
};

/**
 * Provide a registration mechanism for syncing IPv6 connection stats.
 */
typedef void (*ppe_drv_v6_sync_callback_t)(void *app_data, struct ppe_drv_v6_conn_sync_many *conn_sync);

/**
 * ppe_drv_v6_stats_callback_register
 *	API to register IPv6 connection stats sync callback.
 *
 * @param[in] cb         Pointer to the callback function.
 * @param[in] app_data   Pointer to the app data which is passed with the callback.
 *
 * @return
 * Status of the register operation.
 */
bool ppe_drv_v6_stats_callback_register(ppe_drv_v6_sync_callback_t cb, void *app_data);

/**
 * ppe_drv_v6_destroy
 *	Destroys IPv6 connection rule in PPE.
 *
 * @datatypes
 * ppe_drv_v6_rule_destroy
 *
 * @param[in] destroy   Pointer to the NSS PPE IPv6 destroy rule message.
 *
 * @return
 * Status of the destroy operation.
 */
ppe_drv_ret_t ppe_drv_v6_destroy(struct ppe_drv_v6_rule_destroy *destroy);

/**
 * ppe_drv_v6_create
 *	Creates IPv6 connection rule in PPE.
 *
 * @datatypes
 * ppe_drv_v6_rule_create
 *
 * @param[in] create   Pointer to the NSS PPE IPv6 create rule message.
 *
 * @return
 * Status of the create operation.
 */
ppe_drv_ret_t ppe_drv_v6_create(struct ppe_drv_v6_rule_create *create);

/** @} */ /* end_addtogroup ppe_drv_v6_subsystem */

#endif /* _PPE_DRV_V6_H_ */

