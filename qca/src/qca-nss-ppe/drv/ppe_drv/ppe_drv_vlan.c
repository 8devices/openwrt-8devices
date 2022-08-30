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

#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <fal/fal_rss_hash.h>
#include <fal/fal_ip.h>
#include <fal/fal_init.h>
#include <fal/fal_pppoe.h>
#include <fal/fal_tunnel.h>
#include <fal/fal_api.h>
#include <fal/fal_vsi.h>
#include <ref/ref_vsi.h>
#include <fal/fal_portvlan.h>
#include "ppe_drv.h"
#include "ppe_drv_stats.h"

/*
 * ppe_drv_vlan_tpid_set()
 *	Set PPE vlan TPID
 */
ppe_drv_ret_t ppe_drv_vlan_tpid_set(uint16_t ctpid, uint16_t stpid, uint32_t mask)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	fal_tpid_t tpid;

	tpid.mask = mask;
	tpid.ctpid = ctpid;
	tpid.stpid = stpid;
	tpid.tunnel_ctpid = ctpid;
	tpid.tunnel_stpid = stpid;

	spin_lock_bh(&p->lock);
	if ((fal_ingress_tpid_set(PPE_DRV_SWITCH_ID, &tpid) != SW_OK) || (fal_egress_tpid_set(PPE_DRV_SWITCH_ID, &tpid) != SW_OK)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("failed to set ctpid %d stpid %d\n", tpid.ctpid, tpid.stpid);
		return PPE_DRV_RET_VLAN_TPID_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;

}
EXPORT_SYMBOL(ppe_drv_vlan_tpid_set);

/*
 * ppe_drv_vlan_port_role_set()
 *	Set VLAN port role
 */
ppe_drv_ret_t ppe_drv_vlan_port_role_set(struct ppe_drv_iface *iface, uint32_t port_id, fal_port_qinq_role_t *mode)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_port_t fal_port;

	spin_lock_bh(&p->lock);
	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, port_id);

	if (fal_port_qinq_mode_set(PPE_DRV_SWITCH_ID, fal_port, mode) != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("failed to set %d as edge port\n", fal_port);
		return PPE_DRV_RET_PORT_ROLE_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_port_role_set);

/*
 * ppe_drv_vlan_del_xlate_rule()
 *	Delete Ingress and Egress VLAN translation rules
 */
ppe_drv_ret_t ppe_drv_vlan_del_xlate_rule(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_vlan_trans_adv_rule_t xlt_rule;	/* VLAN Translation Rule */
	fal_vlan_trans_adv_action_t xlt_action;	/* VLAN Translation Action */
	struct ppe_drv_vsi *vsi;
	fal_port_t fal_port;
	int vsi_idx, rc;

	/*
	 * Check with vlan device created under bridge
	 */
	spin_lock_bh(&p->lock);
	if (info->br) {
		vsi = ppe_drv_iface_vsi_get(info->br);
	} else {
		vsi = ppe_drv_iface_vsi_get(iface);
	}

	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	vsi_idx = vsi->index;

	/*
	 * Delete old ingress vlan translation rule
	 */
	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(info->port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, info->port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, info->port_id);


	rc = ppe_port_vlan_vsi_set(PPE_DRV_SWITCH_ID, fal_port, info->svid, info->cvid, PPE_VSI_INVALID);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to delete old ingress vlan translation rule of port %d, error: %d\n", fal_port, rc);
		return PPE_DRV_RET_VLAN_INGRESS_DEL_FAIL;
	}

	/*
	 * Add egress vlan translation rule
	 */
	memset(&xlt_rule, 0, sizeof(xlt_rule));
	memset(&xlt_action, 0, sizeof(xlt_action));

	/*
	 * Fields for match
	 */
	xlt_rule.vsi_valid = true;				/* Use vsi as search key */
	xlt_rule.vsi_enable = true;				/* Use vsi as search key */
	xlt_rule.vsi = vsi_idx;					/* Use vsi as search key */
	xlt_rule.s_tagged = 0x7;				/* Accept tagged/untagged/priority tagged svlan */
	xlt_rule.c_tagged = 0x7;				/* Accept tagged/untagged/priority tagged cvlan */

	/*
	 * Fields for action
	 */
	xlt_action.cvid_xlt_cmd = (info->cvid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.cvid_xlt = (info->cvid == 0xFFFF) ? 0 : info->cvid;
	xlt_action.svid_xlt_cmd = (info->svid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.svid_xlt = (info->svid == 0xFFFF) ? 0 : info->svid;;

	/*
	 * Delete old egress vlan translation rule
	 */
	rc = fal_port_vlan_trans_adv_del(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_EGRESS,
			&xlt_rule, &xlt_action);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to delete old egress vlan translation of port %d, error: %d\n", fal_port, rc);
		return PPE_DRV_RET_VLAN_EGRESS_DEL_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_del_xlate_rule);

/*
 * ppe_drv_vlan_add_xlate_rule()
 *	Add Ingress and Egress VLAN translation rules
 */
ppe_drv_ret_t ppe_drv_vlan_add_xlate_rule(struct ppe_drv_iface *iface, struct ppe_drv_vlan_xlate_info *info)
{
	fal_vlan_trans_adv_rule_t xlt_rule;
	fal_vlan_trans_adv_action_t xlt_action;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi;
	int vsi_idx, ret, rc;
	fal_port_t fal_port;

	/*
	 * Check with vlan device created under bridge
	 */
	spin_lock_bh(&p->lock);
	if (info->br) {
		vsi = ppe_drv_iface_vsi_get(info->br);
	} else {
		vsi = ppe_drv_iface_vsi_get(iface);
	}

	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid VSI for given iface\n", iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	vsi_idx = vsi->index;

	fal_port = PPE_DRV_VIRTUAL_PORT_CHK(info->port_id) ? FAL_PORT_ID(FAL_PORT_TYPE_VPORT, info->port_id)
			: FAL_PORT_ID(FAL_PORT_TYPE_PPORT, info->port_id);

	/*
	 * Add new ingress vlan translation rule
	 */
	rc = ppe_port_vlan_vsi_set(PPE_DRV_SWITCH_ID, fal_port, info->svid, info->cvid, vsi_idx);
	if (rc != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_ingress_vlan_add);
		ppe_drv_warn("Failed to update ingress vlan translation of port %d, error: %d\n", fal_port, rc);
		return PPE_DRV_RET_INGRESS_VLAN_FAIL;
	}

	/*
	 * Add egress vlan translation rule
	 */
	memset(&xlt_rule, 0, sizeof(xlt_rule));
	memset(&xlt_action, 0, sizeof(xlt_action));

	/*
	 * Fields for match
	 */
	xlt_rule.vsi_valid = true;				/* Use vsi as search key */
	xlt_rule.vsi_enable = true;				/* Use vsi as search key */
	xlt_rule.vsi = vsi_idx;					/* Use vsi as search key */
	xlt_rule.s_tagged = 0x7;				/* Accept tagged/untagged/priority tagged svlan */
	xlt_rule.c_tagged = 0x7;				/* Accept tagged/untagged/priority tagged cvlan */

	/*
	 * Fields for action
	 */
	xlt_action.cvid_xlt_cmd = (info->cvid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.cvid_xlt = (info->cvid == 0xFFFF) ? 0 : info->cvid;
	xlt_action.svid_xlt_cmd = (info->svid == 0xFFFF) ? 0 : FAL_VID_XLT_CMD_ADDORREPLACE;
	xlt_action.svid_xlt = (info->svid == 0xFFFF) ? 0 : info->svid;;

	ret = fal_port_vlan_trans_adv_add(PPE_DRV_SWITCH_ID, fal_port, FAL_PORT_VLAN_EGRESS, &xlt_rule,
				&xlt_action);
	if (ret != SW_OK) {
		ppe_drv_warn("%px: Failed to update egress translation rule for port: %d, error: %d\n",
				iface, fal_port, ret);

		/*
		 * Delete ingress vlan translation rule
		 */
		if (ret != SW_ALREADY_EXIST) {
			ppe_port_vlan_vsi_set(PPE_DRV_SWITCH_ID, fal_port, FAL_VLAN_INVALID, info->cvid, PPE_VSI_INVALID);
		}

		spin_unlock_bh(&p->lock);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_egress_vlan_add);

		return PPE_DRV_RET_EGRESS_VLAN_FAIL;
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_add_xlate_rule);

/*
 * ppe_drv_vlan_deinit()
 *	De-Initialize VLAN interfaces
 */
void ppe_drv_vlan_deinit(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(iface);
	ppe_drv_iface_base_clear(iface);
	ppe_drv_iface_vsi_clear(iface);
	ppe_drv_iface_l3_if_clear(iface);

	if (vsi) {
		ppe_drv_l3_if_deref(vsi->l3_if);
		ppe_drv_vsi_deref(vsi);
	}

	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_vlan_deinit);

/*
 * ppe_drv_vlan_init()
 *	VLAN init
 */
ppe_drv_ret_t ppe_drv_vlan_init(struct ppe_drv_iface *ppe_iface, struct net_device *base_dev, uint32_t vlan_id)
{
	struct ppe_drv_iface *base_if;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_l3_if *l3_if;

	spin_lock_bh(&p->lock);
	base_if = ppe_drv_iface_get_by_dev_internal(base_dev);
	if (!base_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: NULL base interface for iface\n", ppe_iface);
		return PPE_DRV_RET_BASE_IFACE_NOT_FOUND;
	}

	ppe_drv_iface_base_set(ppe_iface, base_if);

	vsi = ppe_drv_vsi_alloc(PPE_DRV_VSI_TYPE_VLAN);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: NULL VSI for iface\n", ppe_iface);
		return PPE_DRV_RET_VSI_ALLOC_FAIL;
	}

	ppe_drv_iface_vsi_set(ppe_iface, vsi);

	/*
	 * Set inner and outer vlan for a given VSI
	 */
	if (!ppe_drv_vsi_set_vlan(vsi, vlan_id, base_if)) {
		ppe_drv_vsi_deref(vsi);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Set vlan failed for iface\n", ppe_iface);
		return PPE_DRV_RET_VSI_ALLOC_FAIL;
	}

	l3_if = ppe_drv_vsi_l3_if_get_and_ref(vsi);
	if (!l3_if) {
		ppe_drv_vsi_deref(vsi);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: L3_IF not found for vsi(%p)\n", ppe_iface, vsi);
		return PPE_DRV_RET_FAILURE_NO_RESOURCE;
	}

	ppe_drv_iface_l3_if_set(ppe_iface, l3_if);
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vlan_init);
