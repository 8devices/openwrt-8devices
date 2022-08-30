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

#define PPE_DRV_PORT_SRC_PROFILE_MAX	4	/* Source profile for a port can be between 0-3 */
#define PPE_DRV_PHY_PORT_CHK(n) ((n) >= PPE_DRV_PHYSICAL_START && (n) < PPE_DRV_PHYSICAL_MAX)
#define PPE_DRV_VIRTUAL_PORT_CHK(n) ((n) >= PPE_DRV_VIRTUAL_START && (n) < PPE_DRV_PORTS_MAX)

/*
 * ppe_drv_port
 *	Port information
 */
struct ppe_drv_port {
	struct list_head l3_list;		/* List head of associated L3 interface */
	struct ppe_drv_l3_if *port_l3_if;	/* Port L3_IF */
	struct ppe_drv_vsi *port_vsi;		/* Pointer to Port's VSI */
	struct ppe_drv_vsi *br_vsi;		/* Pointer to Bridge's VSI TODO: To be added to the vsi_list*/
	struct ppe_l2_vp *l2_vp;		/* Pointer to L2 VP instance. */
	struct net_device *dev;			/* Associated netdev */
	struct kref ref_cnt;			/* Reference count object */
	enum ppe_drv_port_type type;		/* Port type */
	bool port_l3_if_attached;               /* Port L3_IF attached? */
	uint16_t mtu;				/* MTU value of port */
	uint16_t mru;				/* MRU value of port */
	uint8_t mac_addr[ETH_ALEN];		/* MAC address of port */
	uint8_t port;				/* Port number */
	uint8_t mac_valid;			/* 1 if MAC address is valid */
	uint8_t src_profile;			/* Source profile of the port */
	uint8_t ucast_queue;			/* Base queue ID for the port */
	uint8_t is_tunnel_vp;			/* Port is of type tunnel VP */
};

bool ppe_drv_port_ucast_queue_set(struct ppe_drv_port *pp, uint8_t queue_id);
bool ppe_drv_port_is_tunnel_vp(struct ppe_drv_port *pp);

struct net_device *ppe_drv_port_to_dev(struct ppe_drv_port *pp);
struct ppe_drv_port *ppe_drv_port_from_dev(struct net_device *dev);

void ppe_drv_port_mac_addr_set(struct ppe_drv_port *pp, uint8_t *mac_addr);
void ppe_drv_port_mac_addr_clear(struct ppe_drv_port *pp);

bool ppe_drv_port_mtu_mru_set(struct ppe_drv_port *pp, uint16_t mtu, uint16_t mru);
void ppe_drv_port_mtu_mru_clear(struct ppe_drv_port *pp);

struct ppe_drv_vsi *ppe_drv_port_find_vlan_vsi(struct ppe_drv_port *pp, uint32_t in_vlan, uint32_t out_vlan);
struct ppe_drv_vsi *ppe_drv_port_find_bridge_vsi(struct ppe_drv_port *pp);

struct ppe_drv_l3_if *ppe_drv_port_find_port_l3_if(struct ppe_drv_port *pp);
struct ppe_drv_l3_if *ppe_drv_port_find_pppoe_l3_if(struct ppe_drv_port *pp, uint16_t session_id, uint8_t *smac);

void ppe_drv_port_l3_if_detach(struct ppe_drv_port *pp, struct ppe_drv_l3_if *l3_if);
bool ppe_drv_port_l3_if_attach(struct ppe_drv_port *pp, struct ppe_drv_l3_if *l3_if);

void ppe_drv_port_vsi_detach(struct ppe_drv_port *pp, struct ppe_drv_vsi *vsi);
void ppe_drv_port_vsi_attach(struct ppe_drv_port *pp, struct ppe_drv_vsi *vsi);

bool ppe_drv_port_deref(struct ppe_drv_port *pp);
struct ppe_drv_port *ppe_drv_port_ref(struct ppe_drv_port *pp);
struct ppe_drv_port *ppe_drv_port_alloc(enum ppe_drv_port_type type, struct net_device *dev, bool is_tunnel_vp);
struct ppe_drv_port *ppe_drv_port_phy_alloc(uint8_t port_num, struct net_device *dev);

void ppe_drv_port_entries_free(struct ppe_drv_port *port);
struct ppe_drv_port *ppe_drv_port_entries_alloc(void);
