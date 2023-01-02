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

/*
 * ppe_drv_vp_deinit()
 *	De-Initialize API exposed to VP driver
 */
ppe_drv_ret_t ppe_drv_vp_deinit(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_port *port;

	spin_lock_bh(&p->lock);
	l3_if = ppe_drv_iface_l3_if_get(iface);
	if (!l3_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get l3_if from iface\n", iface);
		return PPE_DRV_RET_L3_IF_NOT_FOUND;
	}

	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get port from iface\n", iface);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Release references on l3_if and port.
	 */
	ppe_drv_port_l3_if_detach(port, l3_if);
	ppe_drv_l3_if_deref(l3_if);
	ppe_drv_port_deref(port);
	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vp_deinit);

/*
 * ppe_drv_vp_init()
 *	Initialize API exposed to VP driver
 */
ppe_drv_ret_t ppe_drv_vp_init(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_port *port;
	bool is_tunnel_vp;

	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
		is_tunnel_vp = false;
		break;
	case PPE_DRV_IFACE_TYPE_VP_TUN:
		is_tunnel_vp = true;
		break;
	default:
		ppe_drv_warn("%p: Incorrect interface type: %d", iface, iface->type);
		return PPE_DRV_RET_IFACE_INVALID;
	}

	spin_lock_bh(&p->lock);
	port = ppe_drv_port_alloc(PPE_DRV_PORT_VIRTUAL, iface->dev, is_tunnel_vp);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get a valid virtual port of iface type(%d)", iface, iface->type);
		return PPE_DRV_RET_PORT_ALLOC_FAIL;
	}

	l3_if = ppe_drv_l3_if_alloc(PPE_DRV_L3_IF_TYPE_PORT);
	if (!l3_if) {
		ppe_drv_port_deref(port);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get a valid l3_if of type(%d)\n", iface, PPE_DRV_L3_IF_TYPE_PORT);
		return PPE_DRV_RET_L3_IF_ALLOC_FAIL;
	}

	/*
	 * Attach l3_if to port
	 */
	if (!ppe_drv_port_l3_if_attach(port, l3_if)) {
		ppe_drv_port_deref(port);
		ppe_drv_l3_if_deref(l3_if);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to attach valid l3_if(%p) to port(%p)\n", iface, l3_if, port);
		return PPE_DRV_RET_L3_IF_PORT_ATTACH_FAIL;
	}

	port->port_l3_if = l3_if;

	ppe_drv_iface_port_set(iface, port);
	ppe_drv_iface_l3_if_set(iface, l3_if);

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_vp_init);
