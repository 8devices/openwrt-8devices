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
#include "ppe_drv_stats.h"

/*
 * ppe_drv_iface_free()
 *	FREE the interface entry in PPE.
 */
static void ppe_drv_iface_free(struct kref *kref)
{
	struct ppe_drv_iface *iface = container_of(kref, struct ppe_drv_iface, ref);

	ppe_drv_assert(!iface->port, "%p: Interface still associated with the port", iface);
	ppe_drv_assert(!iface->vsi, "%p: Interface still associated with the vsi", iface);
	ppe_drv_assert(!iface->l3, "%p: Interface still associated with the l3_if", iface);

	iface->flags &= ~PPE_DRV_IFACE_FLAG_VALID;

	iface->port = NULL;
	iface->vsi = NULL;
	iface->l3 = NULL;
	iface->type = PPE_DRV_IFACE_TYPE_INVALID;
	iface->dev = NULL;
	iface->parent = NULL;
}

/*
 * ppe_drv_iface_ref()
 *	Reference PPE interface
 */
struct ppe_drv_iface *ppe_drv_iface_ref(struct ppe_drv_iface *iface)
{
	kref_get(&iface->ref);
	return iface;
}

/*
 * ppe_drv_iface_deref_internal()
 *	Let go of reference on iface.
 */
bool ppe_drv_iface_deref_internal(struct ppe_drv_iface *iface)
{
	if (kref_put(&iface->ref, ppe_drv_iface_free)) {
		ppe_drv_trace("reference goes down to 0 for iface: %p\n", iface);
		return true;
	}

	return false;
}

/*
 * ppe_drv_iface_deref()
 *	Let go of reference on iface.
 */
bool ppe_drv_iface_deref(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	bool ret;

	spin_lock_bh(&p->lock);
	ret = ppe_drv_iface_deref_internal(iface);
	spin_unlock_bh(&p->lock);

	return ret;
}
EXPORT_SYMBOL(ppe_drv_iface_deref);

/*
 * ppe_drv_iface_get_by_idx()
 *	Get PPE interface by netdev
 */
struct ppe_drv_iface *ppe_drv_iface_get_by_idx(ppe_drv_iface_t idx)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;

	if (idx < 0 || idx >= p->iface_num) {
		return NULL;
	}

	iface = &p->iface[idx];
	if (kref_read(&iface->ref) && ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID)) {
		return iface;
	}

	return NULL;
}

/*
 * ppe_drv_iface_idx_get_by_dev()
 *	Get PPE interface by netdev
 */
ppe_drv_iface_t ppe_drv_iface_idx_get_by_dev(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	int i, index = -1;

	spin_lock_bh(&p->lock);
	for (i = 0; i < p->iface_num; i++) {
		iface = &p->iface[i];
		if (!kref_read(&iface->ref) || ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID)) {
			continue;
		}

		if (iface->dev == dev) {
			index = iface->index;
			break;
		}
	}

	spin_unlock_bh(&p->lock);
	return index;
}
EXPORT_SYMBOL(ppe_drv_iface_idx_get_by_dev);

/*
 * ppe_drv_iface_get_by_dev()
 *	Get PPE interface by device
 */
struct ppe_drv_iface *ppe_drv_iface_get_by_dev(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	int i, index = -1;

	spin_lock_bh(&p->lock);
	for (i = 0; i < p->iface_num; i++) {
		iface = &p->iface[i];
		if (!kref_read(&iface->ref) || ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID)) {
			continue;
		}

		if (iface->dev == dev) {
			index = iface->index;
			break;
		}
	}

	if (index < 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: No valid PPE interface found for the given dev", dev);
		return NULL;
	}

	iface = ppe_drv_iface_get_by_idx(index);
	spin_unlock_bh(&p->lock);
	return iface;
}
EXPORT_SYMBOL(ppe_drv_iface_get_by_dev);

/*
 * ppe_drv_iface_get_by_dev_internal()
 *	Get PPE interface by device
 */
struct ppe_drv_iface *ppe_drv_iface_get_by_dev_internal(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	int i, index = -1;

	for (i = 0; i < p->iface_num; i++) {
		iface = &p->iface[i];
		if (!kref_read(&iface->ref) || ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID)) {
			continue;
		}

		if (iface->dev == dev) {
			index = iface->index;
			break;
		}
	}

	if (index < 0) {
		return NULL;
	}

	iface = ppe_drv_iface_get_by_idx(index);
	return iface;
}

/*
 * ppe_drv_iface_base_clear()
 *	Get base_if of a given PPE interface
 */
void ppe_drv_iface_base_clear(struct ppe_drv_iface *iface)
{
	if (iface->base_if) {
		ppe_drv_iface_deref_internal(iface->base_if);
		iface->base_if = NULL;
	}
}

/*
 * ppe_drv_iface_base_get()
 *	Get base_if of a given PPE interface
 */
struct ppe_drv_iface *ppe_drv_iface_base_get(struct ppe_drv_iface *iface)
{
	return iface->base_if;
}

/*
 * ppe_drv_iface_base_set()
 *	Set base_if of a given PPE interface
 */
void ppe_drv_iface_base_set(struct ppe_drv_iface *iface, struct ppe_drv_iface *base_if)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (iface->base_if) {
		ppe_drv_warn("%p: base_if already set for iface(%p)\n", p, iface);
		return;
	}

	if ((iface->type == PPE_DRV_IFACE_TYPE_BRIDGE) || (iface->type == PPE_DRV_IFACE_TYPE_LAG)) {
		ppe_drv_warn("%p: Base_if cannot be set for iface(%p)\n", p, iface);
		return;
	}

	iface->base_if = ppe_drv_iface_ref(base_if);
}

/*
 * ppe_drv_iface_parent_clear()
 *	Clear parent of a given PPE interface
 */
void ppe_drv_iface_parent_clear(struct ppe_drv_iface *iface)
{
	if (iface->parent) {
		ppe_drv_iface_deref_internal(iface->parent);
		iface->parent = NULL;
	}
}

/*
 * ppe_drv_iface_parent_get()
 *	Get parent of a given PPE interface
 */
struct ppe_drv_iface *ppe_drv_iface_parent_get(struct ppe_drv_iface *iface)
{
	return iface->parent;
}

/*
 * ppe_drv_iface_parent_set()
 *	Set parent of a given PPE interface
 */
bool ppe_drv_iface_parent_set(struct ppe_drv_iface *iface, struct ppe_drv_iface *parent)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (iface->parent) {
		ppe_drv_warn("%p: parent already set for iface(%p)\n", p, iface);
		return false;
	}

	if (parent->type != PPE_DRV_IFACE_TYPE_BRIDGE) {
		ppe_drv_warn("%p: Parent cannot be set for iface(%p)\n", p, iface);
		return false;
	}

	iface->parent = ppe_drv_iface_ref(parent);
	return true;
}

/*
 * ppe_drv_iface_port_idx_get()
 *	Get port of a given PPE interface
 */
int32_t ppe_drv_iface_port_idx_get(struct ppe_drv_iface *iface)
{
	struct ppe_drv_port *port;
	if ((iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID) != PPE_DRV_IFACE_FLAG_PORT_VALID) {
		return -1;
	}

	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		return -1;
	}

	return port->port;
}
EXPORT_SYMBOL(ppe_drv_iface_port_idx_get);

/*
 * ppe_drv_iface_port_clear()
 *	Clear port of a given PPE interface
 */
void ppe_drv_iface_port_clear(struct ppe_drv_iface *iface)
{
	if (iface->port) {
		iface->port = NULL;
		iface->flags &= ~PPE_DRV_IFACE_FLAG_PORT_VALID;
	}
}

/*
 * ppe_drv_iface_port_get()
 *	Get port of a given PPE interface
 */
struct ppe_drv_port *ppe_drv_iface_port_get(struct ppe_drv_iface *iface)
{
	if ((iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID) != PPE_DRV_IFACE_FLAG_PORT_VALID) {
		return NULL;
	}

	return iface->port;
}

/*
 * ppe_drv_iface_port_set()
 *	Set port of a given PPE interface
 */
bool ppe_drv_iface_port_set(struct ppe_drv_iface *iface, struct ppe_drv_port *port)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (iface->port) {
		ppe_drv_warn("%p: port already set for iface(%p)\n", p, iface);
		return false;
	}

	iface->port = port;
	iface->flags |= PPE_DRV_IFACE_FLAG_PORT_VALID;
	return true;
}

/*
 * ppe_drv_iface_vsi_clear()
 *	Clear VSI of a given PPE interface
 */
void ppe_drv_iface_vsi_clear(struct ppe_drv_iface *iface)
{
	if (iface->vsi) {
		iface->vsi = NULL;
		iface->flags &= ~PPE_DRV_IFACE_FLAG_VSI_VALID;
	}
}

/*
 * ppe_drv_iface_vsi_get()
 *	Get VSI of a given PPE interface
 */
struct ppe_drv_vsi *ppe_drv_iface_vsi_get(struct ppe_drv_iface *iface)
{
	if ((iface->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) != PPE_DRV_IFACE_FLAG_VSI_VALID) {
		return NULL;
	}

	return iface->vsi;
}

/*
 * ppe_drv_iface_vsi_set()
 *	Set VSI of a given PPE interface
 */
bool ppe_drv_iface_vsi_set(struct ppe_drv_iface *iface, struct ppe_drv_vsi *vsi)
{
	if (iface->vsi) {
		ppe_drv_warn("%p: vsi already set for iface\n", iface);
		return false;
	}

	iface->vsi = vsi;
	iface->flags |= PPE_DRV_IFACE_FLAG_VSI_VALID;
	return true;
}

/*
 * ppe_drv_iface_l3_if_clear()
 *	Clear L3_IF of a given PPE interface
 */
void ppe_drv_iface_l3_if_clear(struct ppe_drv_iface *iface)
{
	if (iface->l3) {
		iface->l3 = NULL;
		iface->flags &= ~PPE_DRV_IFACE_FLAG_L3_IF_VALID;
	}
}

/*
 * ppe_drv_iface_l3_if_get()
 *	Get L3_IF of a given PPE interface
 */
struct ppe_drv_l3_if *ppe_drv_iface_l3_if_get(struct ppe_drv_iface *iface)
{
	if ((iface->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) != PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
		return NULL;
	}

	return iface->l3;
}

/*
 * ppe_drv_iface_l3_if_set()
 *	Set L3_IF of a given PPE interface
 */
bool ppe_drv_iface_l3_if_set(struct ppe_drv_iface *iface, struct ppe_drv_l3_if *l3_if)
{
	if (iface->l3) {
		ppe_drv_warn("%p: l3_if already set for iface\n", iface);
		return false;
	}

	iface->l3 = l3_if;
	iface->flags |= PPE_DRV_IFACE_FLAG_L3_IF_VALID;
	return true;
}

/*
 * ppe_drv_iface_mtu_set()
 *	Set mac address for the given interface
 */
ppe_drv_ret_t ppe_drv_iface_mtu_set(struct ppe_drv_iface *iface, uint16_t mtu)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_ret_t status = PPE_DRV_RET_SUCCESS;

	spin_lock_bh(&p->lock);
	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_BRIDGE:
	case PPE_DRV_IFACE_TYPE_VLAN:
	{
		struct ppe_drv_vsi *vsi = ppe_drv_iface_vsi_get(iface);
		struct ppe_drv_l3_if *l3_if;
		if (!vsi) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		l3_if = vsi->l3_if;
		if (!l3_if) {
			ppe_drv_warn("%p: No L3_IF associated with vsi(%p)\n", iface, vsi);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_l3_if_mtu_mru_set(l3_if, mtu, mtu)) {
			ppe_drv_warn("%p: L3_IF MTU MRU failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_LAG:
	case PPE_DRV_IFACE_TYPE_PPPOE:
	{
		struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_l3_if_mtu_mru_set(l3_if, mtu, mtu)) {
			ppe_drv_warn("%p: L3_IF MTU MRU failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_PHYSICAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
	case PPE_DRV_IFACE_TYPE_VP_TUN:
	{
		struct ppe_drv_port *port = ppe_drv_iface_port_get(iface);
		if (!port) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_port_mtu_mru_set(port, mtu, mtu)) {
			ppe_drv_warn("%p: PORT MTU MRU failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	default:
		ppe_drv_warn("%p: invalid inface type(%d)\n", iface, iface->type);
		status = PPE_DRV_RET_IFACE_INVALID;
	}

	spin_unlock_bh(&p->lock);
	return status;
}
EXPORT_SYMBOL(ppe_drv_iface_mtu_set);

/*
 * ppe_drv_iface_mac_addr_clear()
 *	Clear mac address for the given interface
 */
ppe_drv_ret_t ppe_drv_iface_mac_addr_clear(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_ret_t status = PPE_DRV_RET_SUCCESS;

	spin_lock_bh(&p->lock);
	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_BRIDGE:
	case PPE_DRV_IFACE_TYPE_VLAN:
	{
		struct ppe_drv_vsi *vsi = ppe_drv_iface_vsi_get(iface);
		struct ppe_drv_l3_if *l3_if;
		if (!vsi) {
			ppe_drv_warn("%p: No VSI associated with iface\n", iface);
			status = PPE_DRV_RET_VSI_NOT_FOUND;
			break;
		}

		l3_if = vsi->l3_if;
		if (!l3_if) {
			ppe_drv_warn("%p: No L3_IF associated with vsi(%p)\n", iface, vsi);
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_mac_addr_clear(l3_if)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, vsi);
			status =  PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_LAG:
	case PPE_DRV_IFACE_TYPE_PPPOE:
	{
		struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_mac_addr_clear(l3_if)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_PHYSICAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
	case PPE_DRV_IFACE_TYPE_VP_TUN:
	{
		struct ppe_drv_l3_if *l3_if;
		struct ppe_drv_port *port = ppe_drv_iface_port_get(iface);
		if (!port) {
			status = PPE_DRV_RET_PORT_NOT_FOUND;
			break;
		}

		l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_eg_mac_addr_clear(l3_if)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL;
			break;
		}

		ppe_drv_port_mac_addr_clear(port);

		break;
	}

	default:
		ppe_drv_warn("%p: invalid inface type(%d)\n", iface, iface->type);
		status = PPE_DRV_RET_IFACE_INVALID;
	}

	spin_unlock_bh(&p->lock);
	return status;
}
EXPORT_SYMBOL(ppe_drv_iface_mac_addr_clear);

/*
 * ppe_drv_iface_mac_addr_set()
 *	Set mac address for the given interface
 */
ppe_drv_ret_t ppe_drv_iface_mac_addr_set(struct ppe_drv_iface *iface, uint8_t *mac_addr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_ret_t status = PPE_DRV_RET_SUCCESS;

	spin_lock_bh(&p->lock);
	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_BRIDGE:
	case PPE_DRV_IFACE_TYPE_VLAN:
	{
		struct ppe_drv_vsi *vsi = ppe_drv_iface_vsi_get(iface);
		struct ppe_drv_l3_if *l3_if;
		if (!vsi) {
			ppe_drv_warn("%p: No VSI associated with iface\n", iface);
			status = PPE_DRV_RET_VSI_NOT_FOUND;
			break;
		}

		l3_if = vsi->l3_if;
		if (!l3_if) {
			ppe_drv_warn("%p: No L3_IF associated with vsi(%p)\n", iface, vsi);
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_mac_addr_set(l3_if, mac_addr)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, vsi);
			status =  PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_LAG:
	case PPE_DRV_IFACE_TYPE_PPPOE:
	{
		struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_mac_addr_set(l3_if, mac_addr)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_PHYSICAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
	case PPE_DRV_IFACE_TYPE_VP_TUN:
	{
		struct ppe_drv_l3_if *l3_if;
		struct ppe_drv_port *port = ppe_drv_iface_port_get(iface);
		if (!port) {
			status = PPE_DRV_RET_PORT_NOT_FOUND;
			break;
		}

		l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_eg_mac_addr_set(l3_if, mac_addr)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL;
			break;
		}

		ppe_drv_port_mac_addr_set(port, mac_addr);

		break;
	}

	default:
		ppe_drv_warn("%p: invalid inface type(%d)\n", iface, iface->type);
		status = PPE_DRV_RET_IFACE_INVALID;
	}

	spin_unlock_bh(&p->lock);
	return status;
}
EXPORT_SYMBOL(ppe_drv_iface_mac_addr_set);

/*
 * ppe_drv_iface_alloc()
 *	Allocates a free interface and takes a reference.
 */
struct ppe_drv_iface *ppe_drv_iface_alloc(enum ppe_drv_iface_type type, struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface = NULL;
	uint16_t i;

	if (type > PPE_DRV_IFACE_TYPE_MAX) {
		ppe_drv_warn("%p: Unsupported iface type(%d)\n", dev, type);
		return NULL;
	}

	/*
	 * Get a free L3 interface entry from pool
	 */
	spin_lock_bh(&p->lock);
	for (i = 0; i < p->iface_num; i++) {
		if (!kref_read(&p->iface[i].ref)) {
			iface = &p->iface[i];
			break;
		}
	}

	if (!iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: cannot alloc iface, table full", p);
		return NULL;
	}

	kref_init(&iface->ref);
	iface->type = type;
	iface->dev = dev;
	iface->parent = NULL;
	iface->flags = PPE_DRV_IFACE_FLAG_VALID;
	iface->base_if = NULL;
	iface->port = NULL;
	iface->vsi = NULL;
	iface->l3 = NULL;

	spin_unlock_bh(&p->lock);

	return iface;
}
EXPORT_SYMBOL(ppe_drv_iface_alloc);

/*
 * ppe_drv_iface_get_index
 *	Return PPE interface index
 */
ppe_drv_iface_t ppe_drv_iface_get_index(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_iface_t index;

	spin_lock_bh(&p->lock);
	index = iface->index;
	spin_unlock_bh(&p->lock);

	return index;
}

/*
 * ppe_drv_iface_entries_free()
 *	Free iface entries if it was allocated.
 */
void ppe_drv_iface_entries_free(struct ppe_drv_iface *iface)
{
	vfree(iface);
}

/*
 * ppe_drv_iface_entries_alloc()
 *	Allocated and initialize interface entries.
 */
struct ppe_drv_iface *ppe_drv_iface_entries_alloc()
{
	struct ppe_drv_iface *iface;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint16_t i;

	iface = vzalloc(sizeof(struct ppe_drv_iface) * p->iface_num);
	if (!iface) {
		ppe_drv_warn("%p: failed to allocate interface entries", p);
		return NULL;
	}

	/*
	 * Initialize interface values
	 */
	for (i = 0; i < p->iface_num; i++) {
		iface[i].index = i;
	}

	return iface;
}
