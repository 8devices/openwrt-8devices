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

#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <fal/fal_fdb.h>
#include <fal/fal_ip.h>
#include <fal/fal_misc.h>
#include <fal/fal_port_ctrl.h>
#include <fal/fal_qm.h>
#include <fal/fal_qos.h>
#include <fal/fal_vport.h>
#include <fal/fal_vsi.h>
#include <ref/ref_vsi.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_port_dump()
 *	Dumps port table configuration
 */
static void ppe_drv_port_dump(struct ppe_drv_port *pp)
{
	fal_port_cnt_cfg_t cntr = {0};
	fal_vport_state_t vp_state = {0};
	fal_qos_pri_precedence_t pre = {0};
	fal_vsi_invalidvsi_ctrl_t vsi_ctrl = {0};
	fal_macaddr_entry_t macaddr = {0};
	fal_intf_id_t intf_ctrl = {0};
	fal_mtu_ctrl_t mtu_ctrl = {0};
	fal_mru_ctrl_t mru_ctrl = {0};
	fal_mtu_cfg_t mtu_cfg = {0};
	fal_fwd_cmd_t fwd_cmd;
	a_bool_t promisc_mode;
	a_bool_t station_move;
	uint32_t vsi_index;

	/*
	 * Dump invalid vsi config.
	 */
	if (fal_vsi_invalidvsi_ctrl_get(PPE_DRV_SWITCH_ID, pp->port, &vsi_ctrl) != SW_OK) {
		ppe_drv_warn("%p: failed to get invalid vsi configuration for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: dest_en: %d", pp, vsi_ctrl.dest_en);
	ppe_drv_trace("%p: dest_info_type: %d", pp, vsi_ctrl.dest_info.dest_info_type);
	ppe_drv_trace("%p: dest_info_value: %d", pp, vsi_ctrl.dest_info.dest_info_value);

	/*
	 * Dump promiscous mode
	 */
	if (fal_port_promisc_mode_get(PPE_DRV_SWITCH_ID, pp->port, &promisc_mode) != SW_OK) {
		ppe_drv_warn("%p: failed to get promiscous mode config for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: promisc_mode: %d", pp, promisc_mode);

	/*
	 * Dump station move learning
	 */
	if (fal_fdb_port_stamove_ctrl_get(PPE_DRV_SWITCH_ID, pp->port, &station_move, &fwd_cmd) != SW_OK) {
		ppe_drv_warn("%p: failed to get station move control for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: station_move: %d", pp, station_move);
	ppe_drv_trace("%p: fwd_cmd: %d", pp, fwd_cmd);

	/*
	 * Dump VP state.
	 */
	if (fal_vport_state_check_get(PPE_DRV_SWITCH_ID, pp->port, &vp_state) != SW_OK) {
		ppe_drv_warn("%p: failed to get state check config for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: vp_type: %d", pp, vp_state.vp_type);
	ppe_drv_trace("%p: check_en: %d", pp, vp_state.check_en);
	ppe_drv_trace("%p: vp_active: %d", pp, vp_state.vp_active);
	ppe_drv_trace("%p: eg_data_valid: %d", pp, vp_state.eg_data_valid);

	/*
	 * Dump port counter config
	 */
	if (fal_port_cnt_cfg_get(PPE_DRV_SWITCH_ID, pp->port, &cntr) != SW_OK) {
		ppe_drv_warn("%p: failed to get counters for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: rx_cnt_en: %d", pp, cntr.rx_cnt_en);
	ppe_drv_trace("%p: tl_rx_cnt_en: %d", pp, cntr.tl_rx_cnt_en);
	ppe_drv_trace("%p: uc_tx_cnt_en: %d", pp, cntr.uc_tx_cnt_en);
	ppe_drv_trace("%p: mc_tx_cnt_en: %d", pp, cntr.mc_tx_cnt_en);
	ppe_drv_trace("%p: rx_cnt_mode: %d", pp, cntr.rx_cnt_mode);
	ppe_drv_trace("%p: tx_cnt_mode: %d", pp, cntr.tx_cnt_mode);

	/*
	 * Dump QOS resolution precedence.
	 */
	if (fal_qos_port_pri_precedence_get(PPE_DRV_SWITCH_ID, pp->port, &pre) != SW_OK) {
		ppe_drv_warn("%p: failed to get priority precedence for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: preheader_pri: %d", pp, pre.preheader_pri);
	ppe_drv_trace("%p: pcp_pri: %d", pp, pre.pcp_pri);
	ppe_drv_trace("%p: dscp_pri: %d", pp, pre.dscp_pri);
	ppe_drv_trace("%p: pre_acl_outer_pri: %d", pp, pre.pre_acl_outer_pri);
	ppe_drv_trace("%p: pre_acl_inner_pri: %d", pp, pre.pre_acl_inner_pri);
	ppe_drv_trace("%p: acl_pri: %d", pp, pre.acl_pri);
	ppe_drv_trace("%p: post_acl_pri: %d", pp, pre.post_acl_pri);
	ppe_drv_trace("%p: flow_pri: %d", pp, pre.flow_pri);

	/*
	 * Dump L3_IF interface attached to port.
	 */
	if (fal_ip_port_intf_get(PPE_DRV_SWITCH_ID, pp->port, &intf_ctrl) != SW_OK) {
		ppe_drv_warn("%p: failed to get l3 interface for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: l3_if_valid: %d", pp, intf_ctrl.l3_if_valid);
	ppe_drv_trace("%p: l3_if_index: %d", pp, intf_ctrl.l3_if_index);

	/*
	 * Dump VSI attached to port.
	 */
	if (ppe_port_vsi_get(PPE_DRV_SWITCH_ID, pp->port, &vsi_index) != SW_OK) {
		ppe_drv_warn("%p: failed to get vsi for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: l3_if_index: %d", pp, vsi_index);

	/*
	 * Dump MTU/MRU configuration.
	 */
	if (fal_port_mtu_cfg_get(PPE_DRV_SWITCH_ID, pp->port, &mtu_cfg) != SW_OK) {
		ppe_drv_warn("%p: failed to get MTU configuration for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: mtu_enable: %d", pp, mtu_cfg.mtu_enable);
	ppe_drv_trace("%p: mtu_type: %d", pp, mtu_cfg.mtu_type);

	if (fal_port_mtu_get(PPE_DRV_SWITCH_ID, pp->port, &mtu_ctrl) != SW_OK) {
		ppe_drv_warn("%p: failed to get MTU for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: mtu_size: %d", pp, mtu_ctrl.mtu_size);
	ppe_drv_trace("%p: mtu_action: %d", pp, mtu_ctrl.action);

	if (fal_port_mru_get(PPE_DRV_SWITCH_ID, pp->port, &mru_ctrl) != SW_OK) {
		ppe_drv_warn("%p: failed to get MRU for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: mru_size: %d", pp, mru_ctrl.mru_size);
	ppe_drv_trace("%p: mru_action: %d", pp, mru_ctrl.action);

	/*
	 * Dump port MAC configuration.
	 */
	if (fal_ip_port_macaddr_get(PPE_DRV_SWITCH_ID, pp->port, &macaddr) != SW_OK) {
		ppe_drv_warn("%p: failed to get mac address for port: %d", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: macaddr.valid: %d", pp, macaddr.valid);
	ppe_drv_trace("%p: macaddr.mac_addr.uc: %pM", pp, macaddr.mac_addr.uc);
}
#else
static void ppe_drv_port_dump(struct ppe_drv_port *pp)
{
}
#endif

/*
 * ppe_drv_port_get_free_port()
 *	Returns a free port entry of given type.
 */
static inline struct ppe_drv_port *ppe_drv_port_get_free_port(enum ppe_drv_port_type type)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint16_t i;

	/*
	 * As we add support for more port types it will get added here.
	 * LAG as an example.
	 */
	switch (type) {
	case PPE_DRV_PORT_VIRTUAL:
		/*
		 * Fetch the first available virtual port
		 */
		for (i = PPE_DRV_VIRTUAL_START; i < (PPE_DRV_VIRTUAL_START + PPE_DRV_VIRTUAL_MAX); i++) {
			if (!kref_read(&p->port[i].ref_cnt)) {
				kref_init(&p->port[i].ref_cnt);
				return &p->port[i];
			}
		}

		ppe_drv_stats_inc(&p->stats.gen_stats.fail_vp_full);
		break;

	default:
		ppe_drv_assert(false, "%p: cannot allocate port type: %d", p, type);
	}

	ppe_drv_info("%p: cannot allocate port type: %d", p, type);
	return NULL;
}

/*
 * ppe_drv_port_destroy()
 *	Destroys the port entry in PPE.
 */
static void ppe_drv_port_destroy(struct kref *kref)
{
	sw_error_t err;
	fal_port_cnt_cfg_t cntr = {0};
	fal_mtu_ctrl_t mtu_ctrl = {0};
	fal_mru_ctrl_t mru_ctrl = {0};
	fal_mtu_cfg_t mtu_cfg = {0};
	fal_vport_state_t vp_state = {0};
	fal_macaddr_entry_t macaddr = {0};
	fal_qos_pri_precedence_t pre = {0};
	fal_vsi_invalidvsi_ctrl_t vsi_ctrl = {0};
	struct ppe_drv_port *pp = container_of(kref, struct ppe_drv_port, ref_cnt);

	ppe_drv_assert(!pp->port_vsi, "%p: port is still attached to port vsi: %p", pp, pp->port_vsi);
	ppe_drv_assert(!pp->br_vsi, "%p: port is still attached to bridge vsi: %p", pp, pp->br_vsi);
	ppe_drv_assert(list_empty(&pp->l3_list), "%p: port still attached to l3_ifs: %p", pp, &pp->l3_list);

	/*
	 * Clear MTU.
	 */
	mtu_ctrl.action = FAL_MAC_RDT_TO_CPU;
	err = fal_port_mtu_set(PPE_DRV_SWITCH_ID, pp->port, &mtu_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mtu: %u", pp, pp->port);
		return;
	}

	mtu_cfg.mtu_enable = true;
	mtu_cfg.mtu_type = FAL_MTU_ETHERNET;
	err = fal_port_mtu_cfg_set(PPE_DRV_SWITCH_ID, pp->port, &mtu_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mtu config: %u", pp, pp->port);
		return;
	}

	mru_ctrl.action = FAL_MAC_RDT_TO_CPU;
	err = fal_port_mru_set(PPE_DRV_SWITCH_ID, pp->port, &mru_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mru: %u", pp, pp->port);
		return;
	}

	pp->mtu = 0;
	pp->mru = 0;

	/*
	 * Clear MAC address
	 */
	macaddr.valid = false;
	err = fal_ip_port_macaddr_set(PPE_DRV_SWITCH_ID, pp->port, &macaddr);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mac address", pp);
		return;
	}

	memset(pp->mac_addr, 0, ETH_ALEN);
	pp->mac_valid = 0;

	/*
	 * Enable invalid VSI forwarding so that we don't need to use a default VSI for standalone ports.
	 */
	vsi_ctrl.dest_en = true;
	vsi_ctrl.dest_info.dest_info_type = FAL_DEST_INFO_PORT_ID;
	vsi_ctrl.dest_info.dest_info_value = PPE_DRV_PORT_CPU;
	err = fal_vsi_invalidvsi_ctrl_set(PPE_DRV_SWITCH_ID, pp->port, &vsi_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure invalid VSI forwarding for port: %u", pp, pp->port);
		return;
	}

	/*
	 * Enable promiscous mode
	 */
	err = fal_port_promisc_mode_set(PPE_DRV_SWITCH_ID, pp->port, true);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure promiscous mode for port: %u", pp, pp->port);
		return;
	}

	/*
	 * Disable station move learning
	 */
	err = fal_fdb_port_stamove_ctrl_set(PPE_DRV_SWITCH_ID, pp->port, false, FAL_MAC_RDT_TO_CPU);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to clear station move control config for port: %u", pp, pp->port);
		return;
	}

	/*
	 * Set VP type as normal VP.
	 */
	vp_state.vp_type = FAL_VPORT_TYPE_NORMAL;
	err = fal_vport_state_check_set(PPE_DRV_SWITCH_ID, pp->port, &vp_state);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to reset state check for port: %u", pp, pp->port);
		return;
	}

	/*
	 * Disable port counters
	 */
	cntr.rx_cnt_mode = FAL_PORT_CNT_MODE_FULL_PKT;
	cntr.tx_cnt_mode = FAL_PORT_CNT_MODE_FULL_PKT;
	err = fal_port_cnt_cfg_set(PPE_DRV_SWITCH_ID, pp->port, &cntr);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to clear counter config for port: %u", pp, pp->port);
		return;
	}

	/*
	 * Set QOS resolution precedence of different classification engines of this port.
	 * Precedence values of 0, 6 and 7 are reserved for specific requirements.
	 */
	err = fal_qos_port_pri_precedence_set(PPE_DRV_SWITCH_ID, pp->port, &pre);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to clear priority precedence for port: %u", pp, pp->port);
		return;
	}

	ppe_drv_trace("%p: ppe port %u destroyed", pp, pp->port);
}

/*
 * ppe_drv_port_l3_if_attach()
 *	Attaches port to given l3_if
 */
bool ppe_drv_port_l3_if_attach(struct ppe_drv_port *pp, struct ppe_drv_l3_if *pl3)
{
	sw_error_t err;
	fal_intf_id_t intf_ctrl = {0};

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: attaching l3_if to unused port:%d", pp, pp->port);

	if ((pl3->type == PPE_DRV_L3_IF_TYPE_PORT) && pp->port_l3_if_attached) {
		ppe_drv_warn("%p: port(%d) is already attached to port type l3_if(%d): ", pp, pp->port, pl3->l3_if_index);
		return false;
	}

	/*
	 * Add l3 to the port l3_if list - only applicable for PPPoE devices.
	 */
	if (pl3->type == PPE_DRV_L3_IF_TYPE_PPPOE) {
		list_add(&pl3->list, &pp->l3_list);
	}

	/*
	 * Update L3_VP_PORT_TBL.
	 */
	if ((pl3->type == PPE_DRV_L3_IF_TYPE_PORT) || (pl3->type == PPE_DRV_L3_IF_TYPE_LAG)) {
		intf_ctrl.l3_if_valid = true;
		intf_ctrl.l3_if_index = pl3->l3_if_index;
		err = fal_ip_port_intf_set(PPE_DRV_SWITCH_ID, pp->port, &intf_ctrl);
		if (err != SW_OK) {
			ppe_drv_warn("%p port l3_if configuration failed: %p port_num: %u l3_if_num: %u",
					pp, pl3, pp->port, pl3->l3_if_index);
			return false;
		}

		pp->port_l3_if_attached = true;
	}

	/*
	 * Update shadow copy and take a reference to L3 interface.
	 * The reference for this will be let go in ppe_port_detach_l3_if.
	 */
	ppe_drv_l3_if_ref(pl3);

	ppe_drv_trace("%p: attaching l3_if %u to port %u", pp, pl3->l3_if_index, pp->port);
	ppe_drv_port_dump(pp);
	return true;
}

/*
 * ppe_drv_port_l3_if_detach()
 *	Detaches port to given l3_if
 */
void ppe_drv_port_l3_if_detach(struct ppe_drv_port *pp, struct ppe_drv_l3_if *pl3)
{
	sw_error_t err;
	struct ppe_drv_l3_if *walk = NULL;
	struct ppe_drv_l3_if *port_l3_if = NULL;
	fal_intf_id_t intf_ctrl = {0};

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: attaching l3_if to unused port:%u", pp, pp->port);

	/*
	 * Delete l3_if from port's l3 list
	 */
	if (pl3->type == PPE_DRV_L3_IF_TYPE_PPPOE) {
		list_for_each_entry(walk, &pp->l3_list, list) {
			if (walk == pl3) {
				break;
			}
		}

		/*
		 * If above walk is not successful, l3_if is already detached from port.
		 */
		if (!walk) {
			ppe_drv_info("%p: port's pppoe l3_if is already detached:%p", pp, pl3);
			return;
		}

		list_del_init(&walk->list);
		if (list_empty(&pp->l3_list)) {
			INIT_LIST_HEAD(&pp->l3_list);
		}
	}

	/*
	 * Update L3_VP_PORT_TBL.
	 */
	if (pl3->type == PPE_DRV_L3_IF_TYPE_LAG) {
		port_l3_if = ppe_drv_port_find_port_l3_if(pp);
		if (port_l3_if) {
			ppe_drv_info("%p: restoring port l3_if:%p", pp, pl3);
			intf_ctrl.l3_if_valid = true;
			intf_ctrl.l3_if_index = port_l3_if->l3_if_index;
			err = fal_ip_port_intf_set(PPE_DRV_SWITCH_ID, pp->port, &intf_ctrl);
			if (err != SW_OK) {
				ppe_drv_warn("%p port l3_if configuration failed: %p port_num: %u l3_if_num: %u",
						pp, port_l3_if, pp->port, port_l3_if->l3_if_index);
				return;
			}
		}
	} else if (pl3->type == PPE_DRV_L3_IF_TYPE_PORT) {
		intf_ctrl.l3_if_valid = false;
		intf_ctrl.l3_if_index = pl3->l3_if_index;
		err = fal_ip_port_intf_set(PPE_DRV_SWITCH_ID, pp->port, &intf_ctrl);
		if (err != SW_OK) {
			ppe_drv_warn("%p port l3_if configuration failed: %p port_num: %u l3_if_num: %u",
					pp, pl3, pp->port, pl3->l3_if_index);
			return;
		}

		pp->port_l3_if_attached = false;
	}

	ppe_drv_trace("%p: detaching l3_if %u from port %u", pp, pl3->l3_if_index, pp->port);

	/*
	 * Release reference taken during l3_if attach.
	 */
	ppe_drv_l3_if_deref(pl3);
	ppe_drv_port_dump(pp);
}

/*
 * ppe_drv_port_vsi_attach()
 *	Attaches port to given vsi
 */
void ppe_drv_port_vsi_attach(struct ppe_drv_port *pp, struct ppe_drv_vsi *vsi)
{
	sw_error_t err;
	fal_intf_id_t intf_ctrl = {0};
	struct ppe_drv_vsi *active_vsi;

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: attaching vsi to unused port:%d", pp, pp->port);

	switch (vsi->type) {
	case PPE_DRV_VSI_TYPE_PORT:

		/*
		 * If bridge VSI is already assigned skip programming L3_VP_PORT_TBL with port vsi.
		 */
		if (pp->br_vsi) {
			/*
			 * Port was added to bridge in down state and then up is called.
			 * save port_vsi for future use.
			 */
			pp->port_vsi = vsi;
			return;
		}

		pp->port_vsi = ppe_drv_vsi_ref(vsi);
		break;

	case PPE_DRV_VSI_TYPE_BRIDGE:
		/*
		 * Detach port_vsi while attaching a new bridge-vsi.
		 */
		if (!pp->port_vsi && pp->port_l3_if) {
			ppe_drv_port_l3_if_detach(pp, pp->port_l3_if);
		}

		pp->br_vsi = ppe_drv_vsi_ref(vsi);
		break;

	case PPE_DRV_VSI_TYPE_VLAN:
		/*
		 * Note: VLAN vsi are obtained from IN_VLAN_XLT_ACTION table.
		 * L3_VP_PORT_TBL need not be touched for vlan cases here.
		 */

		/*
		 * Take reference to vsi.
		 * This reference will be freed up in port_detach_vsi call.
		 */
		ppe_drv_vsi_ref(vsi);
		return;

	default:
		ppe_drv_assert(false, "%p: attaching port: %u of unknown type vsi :%u", pp, pp->port, vsi->type);
		ppe_drv_warn("%p: attaching port to unknown vsi type: %u", pp, vsi->type);
		return;
	}

	/*
	 * Attach the new VSI to port
	 * If port is attached to bridge, it takes higher precedence.
	 */
	active_vsi = pp->br_vsi ? pp->br_vsi : pp->port_vsi;

	/*
	 * If port has a valid vsi, mark port's l3_if invalid.
	 *
	 * Note: By design PPE gives higher precedence to L3_IF configured in L3_VP_PORT_TBL
	 * as compared to L3_VSI_TBL. This works fine if port’s MAC address is same as br-lan
	 * (or vlan interface) MAC address, but causes unwanted exception at ingress due to mismatch
	 * between packet's DMAC and the MAC address of the port (in IN_L3_VP_PORT_TBL).
	 *
	 * Mark port’s l3_if invalid whenever a port has a valid VSI, this will allow PPE to use L3_IF
	 * from L3_VSI_TBL instead of L3_VP_PORT_TBL for both VLAN and bridge flows.
	 */
	err = fal_ip_port_intf_set(PPE_DRV_SWITCH_ID, pp->port, &intf_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p port l3_if configuration failed for port_num: %u",
				pp, pp->port);
		goto fail;
	}

	err = ppe_port_vsi_set(PPE_DRV_SWITCH_ID, pp->port, active_vsi->index);
	if (err != SW_OK) {
		ppe_drv_warn("%p port vsi configuration failed: %p port_num: %u vsi_num: %u",
				pp, active_vsi, pp->port, active_vsi->index);
		goto fail;
	}

	ppe_drv_trace("%p: attaching vsi %u to port %u", pp, active_vsi->index, pp->port);

	ppe_drv_port_dump(pp);
	return;

fail:
	if (vsi->type == PPE_DRV_VSI_TYPE_PORT) {
		ppe_drv_vsi_deref(pp->port_vsi);
		pp->port_vsi = NULL;
	} else if (vsi->type == PPE_DRV_VSI_TYPE_BRIDGE) {
		ppe_drv_vsi_deref(pp->br_vsi);
		pp->br_vsi = NULL;
	}
}

/*
 * ppe_drv_port_vsi_detach()
 *	Detaches port from its vsi
 */
void ppe_drv_port_vsi_detach(struct ppe_drv_port *pp, struct ppe_drv_vsi *vsi)
{
	sw_error_t err;

	switch (vsi->type) {
	case PPE_DRV_VSI_TYPE_PORT:
		/*
		 * If bridge VSI is already assigned skip detaching port vsi.
		 */
		if (pp->br_vsi) {
			return;
		}

		/*
		 * port_vsi will be cleared when the final ref count is release on vsi.
		 */
		ppe_drv_vsi_deref(vsi);
		break;

	case PPE_DRV_VSI_TYPE_BRIDGE:
		pp->br_vsi = NULL;
		ppe_drv_vsi_deref(vsi);

		/*
		 * Attach port_vsi if br_vsi is going away
		 */
		if (!pp->port_vsi && pp->port_l3_if) {
			ppe_drv_port_l3_if_attach(pp, pp->port_l3_if);
		}

		break;

	case PPE_DRV_VSI_TYPE_VLAN:
		/*
		 * Note: VLAN vsi are obtained from IN_VLAN_XLT_ACTION table.
		 * L3_VP_PORT_TBL need not be touched for vlan cases here.
		 */
		ppe_drv_vsi_deref(vsi);
		return;

	default:
		ppe_drv_warn("%p: detaching port - unknown vsi type: %u", pp, vsi->type);
		return;
	}

	ppe_drv_trace("%p: detached vsi %u from port %u", pp, vsi->index, pp->port);

	err = ppe_port_vsi_set(PPE_DRV_SWITCH_ID, pp->port, FAL_VSI_INVALID);
	if (err != SW_OK) {
		ppe_drv_warn("%p port vsi configuration failed: %p port_num: %u vsi_num: %u",
				pp, vsi, pp->port, vsi->index);
	}

	ppe_drv_port_dump(pp);
}

/*
 * ppe_drv_port_find_port_l3_if()
 *	Find Port l3_if associted with port
 */
struct ppe_drv_l3_if *ppe_drv_port_find_port_l3_if(struct ppe_drv_port *pp)
{
	if (pp->port_l3_if_attached) {
		return pp->port_l3_if;
	}

	return NULL;
}

/*
 * ppe_drv_port_find_pppoe_l3_if()
 *	Find PPPOE l3_if associted with port
 */
struct ppe_drv_l3_if *ppe_drv_port_find_pppoe_l3_if(struct ppe_drv_port *pp, uint16_t session_id, uint8_t *smac)
{
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_l3_if *pppoe_l3_if;

	/*
	 * Find pppoe interface's l3_if.
	 */
	pppoe_l3_if = ppe_drv_pppoe_find_l3_if(session_id, smac);
	if (!pppoe_l3_if) {
		ppe_drv_info("%p: no pppoe l3_if found corresponding to session_id: %u", pp, session_id);
		return NULL;
	}

	/*
	 * Check if l3_if is attached to port's l3_if list.
	 */
	list_for_each_entry(l3_if, &pp->l3_list, list) {
		if ((l3_if->type == PPE_DRV_L3_IF_TYPE_PPPOE) && (l3_if == pppoe_l3_if)) {
			return l3_if;
		}
	}

	return NULL;
}

/*
 * ppe_drv_port_from_dev()
 *	Get PPE port from net-device
 */
struct ppe_drv_port *ppe_drv_port_from_dev(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp = NULL;
	uint16_t i;

	for (i = 0; i < PPE_DRV_PORTS_MAX; i++) {
		pp = &p->port[i];

		if (!kref_read(&pp->ref_cnt)) {
			continue;
		}

		if (pp->dev == dev) {
			return pp;
		}
	}

	ppe_drv_info("%p: No ppe port for dev: %p", p, dev);
	ppe_drv_stats_inc(&p->stats.gen_stats.fail_dev_port_map);
	return NULL;
}

/*
 * ppe_drv_port_from_dev_and_ref()
 *	Get PPE port from net device
 */
struct ppe_drv_port *ppe_drv_port_from_dev_and_ref(struct net_device *dev)
{
	struct ppe_drv_port *pp = NULL;

	pp = ppe_drv_port_from_dev(dev);
	if (pp) {
		ppe_drv_port_ref(pp);
	}

	return pp;
}

/*
 * ppe_drv_port_from_port_num()
 *	Get PPE port from port number
 */
struct ppe_drv_port *ppe_drv_port_from_port_num(uint16_t port_num)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp = NULL;
	uint16_t i;

	if (port_num >= PPE_DRV_PORTS_MAX) {
		ppe_drv_warn("%p: invalid port number: %u", p, port_num);
		return NULL;
	}

	pp = &p->port[i];
	if (!kref_read(&pp->ref_cnt)) {
		ppe_drv_warn("%p: port number not initialized: %u", p, port_num);
		return NULL;
	}

	return pp;
}

/* ppe_drv_port_num_get()
 *	Get PPE port number from ppe port.
 */
uint16_t ppe_drv_port_num_get(struct ppe_drv_port *pp)
{
	return pp->port;
}

/*
 * ppe_drv_port_num_from_dev()
 *	Get PPE port index from net device
 */
int32_t ppe_drv_port_num_from_dev(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp = NULL;

	spin_lock_bh(&p->lock);
	pp = ppe_drv_port_from_dev(dev);
	if (pp) {
		spin_unlock_bh(&p->lock);
		return pp->port;
	}

	spin_unlock_bh(&p->lock);
	return -1;
}
EXPORT_SYMBOL(ppe_drv_port_num_from_dev);

/*
 * ppe_drv_port_is_physical()
 *	Returns true if ppe port is physical.
 */
bool ppe_drv_port_is_physical(struct ppe_drv_port *pp)
{
	return pp->port < PPE_DRV_PHYSICAL_MAX;
}

/*
 * ppe_drv_port_is_virtual()
 *	Returns true if ppe port is virtual.
 */
bool ppe_drv_port_is_virtual(struct ppe_drv_port *pp)
{
	return pp->port >= PPE_DRV_VIRTUAL_START;
}

/*
 * ppe_drv_port_is_tunnel_vp()
 *	Return true if port is a hardware tunnel vp
 */
bool ppe_drv_port_is_tunnel_vp(struct ppe_drv_port *pp)
{
	return pp->is_tunnel_vp;
}

/*
 * ppe_drv_port_num_to_dev()
 *	Get netdev from port number
 */
struct net_device *ppe_drv_port_num_to_dev(uint8_t port_num)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct net_device *dev = NULL;
	struct ppe_drv_port *pp;

	spin_lock_bh(&p->lock);
	pp = &p->port[port_num];

	if (kref_read(&pp->ref_cnt)) {
		dev = pp->dev;
	}

	spin_unlock_bh(&p->lock);
	return dev;
}
EXPORT_SYMBOL(ppe_drv_port_num_to_dev);

/*
 * ppe_drv_port_to_dev()
 *	Get netdev mapped to PPE port.
 */
struct net_device *ppe_drv_port_to_dev(struct ppe_drv_port *pp)
{
	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: port unused, no reference associated", pp);

	return pp->dev;
}

/*
 * ppe_drv_port_ref()
 *	Takes reference on PPE port
 */
struct ppe_drv_port *ppe_drv_port_ref(struct ppe_drv_port *pp)
{
	kref_get(&pp->ref_cnt);

	ppe_drv_trace("%p: port:%u ref inc:%u", pp, pp->port, kref_read(&pp->ref_cnt));
	return pp;
}

/*
 * ppe_drv_port_deref()
 *	Lets go of reference to PPE port
 */
bool ppe_drv_port_deref(struct ppe_drv_port *pp)
{
	if (kref_put(&pp->ref_cnt, ppe_drv_port_destroy)) {
		ppe_drv_trace("reference goes down to 0 for port: %p\n", pp);
		return true;
	}

	ppe_drv_trace("%p: port:%u ref dec:%u", pp, pp->port, kref_read(&pp->ref_cnt));
	return false;
}

/*
 * ppe_drv_port_ucast_queue_set()
 *	Set queue ID of a given port in PPE.
 */
bool ppe_port_ucast_queue_set(struct ppe_drv_port *pp, uint8_t queue_id)
{
	sw_error_t err;
	fal_ucast_queue_dest_t q_dst = {0};

	/*
	 * Set unicast queue base for port
	 */
	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: setting queue ID for an unused port:%u", pp, pp->port);

	/*
	 * TODO confirm with SSDK team if using profile-ID 0 for CPU port
	 */
	q_dst.src_profile = 0;
	q_dst.dst_port = pp->port;
	err = fal_ucast_queue_base_profile_set(PPE_DRV_SWITCH_ID, &q_dst, queue_id, 0);
	if (err != SW_OK) {
		ppe_drv_warn("%p unable to change port queue base ID: %u", pp, queue_id);
		return false;
	}

	/*
	 * Update shadow copy.
	 */
	pp->ucast_queue = queue_id;
	ppe_drv_info("%p: set port ucast queue base id: %u", pp, queue_id);
	return true;
}

/*
 * ppe_drv_port_ucast_queue_get()
 *	Get queue ID of a given port in PPE.
 */
uint8_t ppe_drv_port_ucast_queue_get(struct ppe_drv_port *pp)
{
	return pp->ucast_queue;
}

/*
 * ppe_drv_port_mac_addr_set()
 *	Set MAC addr of a given port in PPE.
 */
void ppe_drv_port_mac_addr_set(struct ppe_drv_port *pp, uint8_t *mac_addr)
{
	sw_error_t err;
	fal_macaddr_entry_t macaddr = {0};

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: setting mac addr on an unused port:%u", pp, pp->port);

	macaddr.valid = true;
	memcpy(macaddr.mac_addr.uc, mac_addr, ETH_ALEN);
	err = fal_ip_port_macaddr_set(PPE_DRV_SWITCH_ID, pp->port, &macaddr);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to configure port mac address", pp);
		return;
	}

	/*
	 * Update shadow copy
	 */
	memcpy(pp->mac_addr, mac_addr, ETH_ALEN);
	pp->mac_valid = 1;

	ppe_drv_trace("%p: mac_addr set for port %u", pp, pp->port);
}

/*
 * ppe_drv_port_mac_addr_clear()
 *	Clear MAC addr of a given port in PPE.
 */
void ppe_drv_port_mac_addr_clear(struct ppe_drv_port *pp)
{
	sw_error_t err;
	fal_macaddr_entry_t macaddr = {0};

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: operating on an unused port:%u", pp, pp->port);

	macaddr.valid = false;
	err = fal_ip_port_macaddr_set(PPE_DRV_SWITCH_ID, pp->port, &macaddr);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mac address", pp);
		return;
	}

	memset(pp->mac_addr, 0, ETH_ALEN);
	pp->mac_valid = 0;

	ppe_drv_trace("%p: mac addr cleared for port %u", pp, pp->port);
}

/*
 * ppe_drv_port_mtu_mru_set()
 *	Set MTU and MRU of given port in PPE.
 */
bool ppe_drv_port_mtu_mru_set(struct ppe_drv_port *pp, uint16_t mtu, uint16_t mru)
{
	sw_error_t err;
	fal_mtu_ctrl_t mtu_ctrl = {0};
	fal_mtu_cfg_t mtu_cfg = {0};
	fal_mru_ctrl_t mru_ctrl = {0};

	/*
	 * Adjust MTU/MRU to accept MAC header addtional to mtu.
	 *
	 * Note: PPE ignore vlan headers at L2 layer and only need
	 * additional 14 byte for MAC header in PORT MTU/MRU
	 * while considering size of frames at L2
	 */
	uint16_t port_mtu = mtu + ETH_HLEN;
	uint16_t port_mru = mru + ETH_HLEN;

	if (mtu > PPE_DRV_PORT_JUMBO_MAX || mru > PPE_DRV_PORT_JUMBO_MAX) {
		ppe_drv_warn("%p: set mtu/mru fail - %u/%u larger than max %u", pp, mtu, mru, PPE_DRV_PORT_JUMBO_MAX);
		return false;
	}

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: setting mtu/mru for unused port:%u", pp, pp->port);

	/*
	 * Configure MTU.
	 * TODO: update extra header length and mtu_type as required for tunnel VP.
	 */
	mtu_ctrl.mtu_size = port_mtu;
	mtu_ctrl.action = FAL_MAC_RDT_TO_CPU;
	err = fal_port_mtu_set(PPE_DRV_SWITCH_ID, pp->port, &mtu_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to configure port mtu: %u", pp, port_mtu);
		return false;
	}

	mtu_cfg.mtu_enable = true;
	mtu_cfg.mtu_type = FAL_MTU_ETHERNET;
	err = fal_port_mtu_cfg_set(PPE_DRV_SWITCH_ID, pp->port, &mtu_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to configure port mtu config: %u", pp, port_mtu);
		return false;
	}

	mru_ctrl.mru_size = port_mru;
	mru_ctrl.action = FAL_MAC_RDT_TO_CPU;
	err = fal_port_mru_set(PPE_DRV_SWITCH_ID, pp->port, &mru_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to configure port mru: %u", pp, port_mru);
		return false;
	}

	/*
	 * Update shadow copy
	 */
	pp->mtu = port_mtu;
	pp->mru = port_mru;

	/*
	 * Set MTU/MRU of associated L3_IF
	 */
	if (pp->port_vsi && pp->port_vsi->l3_if) {
		ppe_drv_l3_if_mtu_mru_set(pp->port_vsi->l3_if, mtu, mru);
	} else if (pp->port_l3_if) {
		ppe_drv_l3_if_mtu_mru_set(pp->port_l3_if, mtu, mru);
	}

	ppe_drv_info("%p: mtu %u mru %u set for port %u", pp, mtu, mru, pp->port);
	return true;
}

/*
 * ppe_drv_port_mtu_mru_clear()
 *	Clear MTU and MRU of given port in PPE.
 */
void ppe_drv_port_mtu_mru_clear(struct ppe_drv_port *pp)
{
	sw_error_t err;
	fal_mtu_ctrl_t mtu_ctrl = {0};
	fal_mtu_cfg_t mtu_cfg = {0};
	fal_mru_ctrl_t mru_ctrl = {0};

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: operating on an unused port:%u", pp, pp->port);

	/*
	 * Configure MTU.
	 */
	mtu_ctrl.mtu_size = 0;
	mtu_ctrl.action = FAL_MAC_RDT_TO_CPU;
	err = fal_port_mtu_set(PPE_DRV_SWITCH_ID, pp->port, &mtu_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mtu: %u", pp, pp->port);
		return;
	}

	mtu_cfg.mtu_enable = true;
	mtu_cfg.mtu_type = FAL_MTU_ETHERNET;
	mtu_cfg.extra_header_len = 0;
	err = fal_port_mtu_cfg_set(PPE_DRV_SWITCH_ID, pp->port, &mtu_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mtu configuration: %u", pp, pp->port);
		return;
	}

	mru_ctrl.mru_size = 0;
	mru_ctrl.action = FAL_MAC_RDT_TO_CPU;
	err = fal_port_mru_set(PPE_DRV_SWITCH_ID, pp->port, &mru_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to clear port mru: %u", pp, pp->port);
		return;
	}

	pp->mtu = 0;
	pp->mru = 0;

	/*
	 * Clear MTU/MRU of associated L3_IF
	 */
	if (pp->port_vsi && pp->port_vsi->l3_if) {
		ppe_drv_l3_if_mtu_mru_clear(pp->port_vsi->l3_if);
	} else if (pp->port_l3_if) {
		ppe_drv_l3_if_mtu_mru_clear(pp->port_l3_if);
	}

	ppe_drv_info("%p: mtu/mru for port %u cleared", pp, pp->port);
}

/*
 * ppe_drv_port_src_profile_set()
 *	Set source profile of given port in PPE.
 */
bool ppe_drv_port_src_profile_set(struct ppe_drv_port *pp, uint8_t src_profile)
{
	sw_error_t err;

	ppe_drv_assert(kref_read(&pp->ref_cnt), "%p: setting src profile for unused port:%u", pp, pp->port);

	/*
	 * Set the source profile in register
	 */
	err = fal_qm_port_source_profile_set(PPE_DRV_SWITCH_ID, pp->port, src_profile);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to configure src profile", pp);
		return false;
	}

	pp->src_profile = src_profile;
	ppe_drv_info("%p: src profile %u set for port %u", pp, src_profile, pp->port);
	return true;
}

/*
 * ppe_drv_port_alloc()
 *	Create a new physical port in PPE.
 */
struct ppe_drv_port *ppe_drv_port_alloc(enum ppe_drv_port_type type, struct net_device *dev, bool is_tunnel_vp)
{
	uint32_t port;
	sw_error_t err;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp = NULL;
	fal_port_cnt_cfg_t cntr = {0};
	fal_vport_state_t vp_state = {0};
	fal_qos_pri_precedence_t pre = {0};
	fal_vsi_invalidvsi_ctrl_t vsi_ctrl = {0};

	/*
	 * Allocate a free port
	 */
	if (type == PPE_DRV_PORT_PHYSICAL) {
		ppe_drv_warn("%p: physical port dynamic allocation not supported dev: %p", p, dev);
		return NULL;
	} else if (type == PPE_DRV_PORT_VIRTUAL) {
		/*
		 * Get a free virtual port entry
		 */
		pp = ppe_drv_port_get_free_port(PPE_DRV_PORT_VIRTUAL);
		if (!pp) {
			ppe_drv_warn("%p: unable to find a free port", p);
			return NULL;
		}

		port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, pp->port);
	} else if (type == PPE_DRV_PORT_EIP) {

		/*
		 * Use fixed port number 7 for EIP port configuration.
		 */
		port = PPE_DRV_PORT_EIP197;
		pp = &p->port[port];

		/*
		 * Is port already configured?
		 */
		if (kref_read(&pp->ref_cnt)) {
			ppe_drv_warn("%p: inline EIP port: %u already configured!",
					p, PPE_DRV_PORT_EIP197);
			return NULL;
		}

		/*
		 * Take a reference on the port.
		 * This will be released when the user releases the final reference on port.
		 */
		kref_init(&pp->ref_cnt);
	} else {
		ppe_drv_assert(false, "%p: Invalid type of port: %u", p, type);
	}

	/*
	 * Enable invalid VSI forwarding so that we don't need to use a default VSI for standalone ports.
	 */
	vsi_ctrl.dest_en = true;
	vsi_ctrl.dest_info.dest_info_type = FAL_DEST_INFO_PORT_ID;
	vsi_ctrl.dest_info.dest_info_value = PPE_DRV_PORT_CPU;
	err = fal_vsi_invalidvsi_ctrl_set(PPE_DRV_SWITCH_ID, port, &vsi_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure invalid VSI forwarding for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Enable promiscous mode
	 */
	err = fal_port_promisc_mode_set(PPE_DRV_SWITCH_ID, port, true);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure promiscous mode for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Enable station move learning
	 */
	err = fal_fdb_port_stamove_ctrl_set(PPE_DRV_SWITCH_ID, port, true, FAL_MAC_RDT_TO_CPU);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure station move control for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Set VP type as normal VP.
	 */
	if (is_tunnel_vp) {
		vp_state.vp_type = FAL_VPORT_TYPE_TUNNEL;
		vp_state.check_en = true;
		vp_state.vp_active = false;
		vp_state.eg_data_valid = false;
	} else {
		vp_state.vp_type = FAL_VPORT_TYPE_NORMAL;
		vp_state.check_en = false;
		vp_state.vp_active = true;
		vp_state.eg_data_valid = false;
	}

	err = fal_vport_state_check_set(PPE_DRV_SWITCH_ID, port, &vp_state);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure state check for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Enable port counters
	 * TODO: change this to formal API instead of debug API.
	 */
	cntr.rx_cnt_en = true;
	cntr.tl_rx_cnt_en = true;
	cntr.uc_tx_cnt_en = true;
	cntr.mc_tx_cnt_en = true;
	cntr.rx_cnt_mode = FAL_PORT_CNT_MODE_FULL_PKT;
	cntr.tx_cnt_mode = FAL_PORT_CNT_MODE_FULL_PKT;
	err = fal_port_cnt_cfg_set(PPE_DRV_SWITCH_ID, port, &cntr);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure counter config for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Set QOS resolution precedence of different classification engines of this port.
	 * Precedence values of 0, 6 and 7 are reserved for specific requirements.
	 */
	pre.preheader_pri = PPE_DRV_PORT_QOS_RES_PREC_1;
	pre.pcp_pri = PPE_DRV_PORT_QOS_RES_PREC_2;
	pre.dscp_pri = PPE_DRV_PORT_QOS_RES_PREC_3;
	pre.pre_acl_outer_pri = PPE_DRV_PORT_QOS_RES_PREC_4;
	pre.pre_acl_inner_pri = PPE_DRV_PORT_QOS_RES_PREC_4;
	pre.acl_pri = PPE_DRV_PORT_QOS_RES_PREC_5;
	pre.post_acl_pri = PPE_DRV_PORT_QOS_RES_PREC_5;
	pre.flow_pri = PPE_DRV_PORT_QOS_RES_PREC_6;
	err = fal_qos_port_pri_precedence_set(PPE_DRV_SWITCH_ID, pp->port, &pre);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure priority precedence for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Fill shadow copy of port entry
	 */
	pp->l2_vp= NULL;
	pp->br_vsi = NULL;
	pp->port_vsi = NULL;
	pp->port_l3_if = NULL;
	pp->dev = dev;
	pp->type = type;
	pp->is_tunnel_vp = is_tunnel_vp;
	INIT_LIST_HEAD(&pp->l3_list);

	ppe_drv_info("%p: allocated ppe port:%u for dev(%s): %p", pp, port, netdev_name(dev), dev);
	return pp;
}

/*
 * ppe_drv_port_phy_alloc()
 *	Create a new port in PPE.
 */
struct ppe_drv_port *ppe_drv_port_phy_alloc(uint8_t port_num, struct net_device *dev)
{
	sw_error_t err;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp = NULL;
	fal_port_cnt_cfg_t cntr = {0};
	fal_vport_state_t vp_state = {0};
	fal_qos_pri_precedence_t pre = {0};
	fal_vsi_invalidvsi_ctrl_t vsi_ctrl = {0};

	if (port_num >= PPE_DRV_PHYSICAL_MAX) {
		ppe_drv_assert(false, "%p: physical port number out of range: %u dev: %p",
				p, port_num, dev);
		ppe_drv_warn("%p: physical port number out of range: %u, dev: %p",
				p, port_num, dev);
		return NULL;
	}

	pp = &p->port[port_num];
	if (kref_read(&pp->ref_cnt)) {
		ppe_drv_warn("%p: physical port already in use: %u, dev: %p",
				p, port_num, dev);
		return NULL;
	}

	kref_init(&pp->ref_cnt);

	/*
	 * Enable invalid VSI forwarding so that we don't need to use a default VSI for standalone ports.
	 */
	vsi_ctrl.dest_en = true;
	vsi_ctrl.dest_info.dest_info_type = FAL_DEST_INFO_PORT_ID;
	vsi_ctrl.dest_info.dest_info_value = PPE_DRV_PORT_CPU;
	err = fal_vsi_invalidvsi_ctrl_set(PPE_DRV_SWITCH_ID, pp->port, &vsi_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure invalid VSI forwarding for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Enable promiscous mode
	 */
	err = fal_port_promisc_mode_set(PPE_DRV_SWITCH_ID, pp->port, true);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure promiscous mode for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Enable station move learning
	 */
	err = fal_fdb_port_stamove_ctrl_set(PPE_DRV_SWITCH_ID, pp->port, true, FAL_MAC_RDT_TO_CPU);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure station move control for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Set VP type as normal VP.
	 */
	vp_state.vp_type = FAL_VPORT_TYPE_NORMAL;
	vp_state.vp_active = true;
	err = fal_vport_state_check_set(PPE_DRV_SWITCH_ID, pp->port, &vp_state);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure state check for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Enable port counters
	 */
	cntr.rx_cnt_en = true;
	cntr.tl_rx_cnt_en = true;
	cntr.uc_tx_cnt_en = true;
	cntr.mc_tx_cnt_en = true;
	cntr.rx_cnt_mode = FAL_PORT_CNT_MODE_FULL_PKT;
	cntr.tx_cnt_mode = FAL_PORT_CNT_MODE_FULL_PKT;
	err = fal_port_cnt_cfg_set(PPE_DRV_SWITCH_ID, pp->port, &cntr);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure counter config for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Set QOS resolution precedence of different classification engines of this port.
	 * Precedence values of 0, 6 and 7 are reserved for specific requirements.
	 */
	pre.preheader_pri = PPE_DRV_PORT_QOS_RES_PREC_1;
	pre.pcp_pri = PPE_DRV_PORT_QOS_RES_PREC_2;
	pre.dscp_pri = PPE_DRV_PORT_QOS_RES_PREC_3;
	pre.pre_acl_outer_pri = PPE_DRV_PORT_QOS_RES_PREC_4;
	pre.pre_acl_inner_pri = PPE_DRV_PORT_QOS_RES_PREC_4;
	pre.acl_pri = PPE_DRV_PORT_QOS_RES_PREC_5;
	pre.post_acl_pri = PPE_DRV_PORT_QOS_RES_PREC_5;
	pre.flow_pri = PPE_DRV_PORT_QOS_RES_PREC_6;
	err = fal_qos_port_pri_precedence_set(PPE_DRV_SWITCH_ID, pp->port, &pre);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure priority precedence for port: %u", p, pp->port);
		ppe_drv_port_deref(pp);
		return NULL;
	}

	/*
	 * Fill shadow copy of port entry
	 */
	pp->l2_vp= NULL;
	pp->br_vsi = NULL;
	pp->port_vsi = NULL;
	pp->port_l3_if = NULL;
	pp->dev = dev;
	pp->type = PPE_DRV_PORT_PHYSICAL;
	INIT_LIST_HEAD(&pp->l3_list);

	ppe_drv_info("%p: allocated physical port:%u for dev(%s): %p",
			pp, port_num, netdev_name(dev), dev);
	return pp;
}

/*
 * ppe_drv_port_entries_free()
 *	Free port table entries if it was allocated.
 */
void ppe_drv_port_entries_free(struct ppe_drv_port *port)
{
	vfree(port);
}

/*
 * ppe_drv_port_entries_alloc()
 *	Allocate and initialize port entries.
 */
struct ppe_drv_port *ppe_drv_port_entries_alloc(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;
	uint16_t i;

	port = vzalloc(sizeof(struct ppe_drv_port) * p->port_num);
	if (!port) {
		ppe_drv_warn("%p: failed to allocate port entries", p);
		return NULL;
	}

	for (i = 0; i < p->port_num; i++) {
		port[i].port = i;
	}

	return port;
}
