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
 * @file ppe_drv_br.h
 *	NSS PPE driver bridge client definitions.
 */

#ifndef _PPE_DRV_BR_H_
#define _PPE_DRV_BR_H_

#include <fal/fal_stp.h>

#define PPE_DRV_BR_SPANNING_TREE_ID	0

/**
 * ppe_drv_br_fdb_del_bymac
 *	Delete FDB entry by MAC address.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] mac_addr    MAC address for which FDB entry should be deleted.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_br_fdb_del_bymac(struct ppe_drv_iface *br_iface, uint8_t *mac_addr);

/**
 * ppe_drv_br_fdb_lrn_ctrl
 *	Configure FDB learn control for a bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] enable      Flag to enable or disable FDB learning.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_br_fdb_lrn_ctrl(struct ppe_drv_iface *br_iface, bool enable);

/**
 * ppe_drv_br_stp_state_set
 *	Set STP state on bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 * fal_stp_state_t
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] member      Bridge member netdevice for which STP state is being configured.
 * @param[in] state       STP state of the bridge interface.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_br_stp_state_set(struct ppe_drv_iface *br_iface, struct net_device *member, fal_stp_state_t state);

/**
 * ppe_drv_br_leave
 *	Remove a member from bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] member      Pointer to the member net device.
 *
 * @return
 * Status of the leave operation.
 */
ppe_drv_ret_t ppe_drv_br_leave(struct ppe_drv_iface *br_iface, struct net_device *member);

/**
 * ppe_drv_br_join
 *	Add a member to bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 * net_device
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 * @param[in] member      Pointer to the member net device.
 *
 * @return
 * Status of the join operation.
 */
ppe_drv_ret_t ppe_drv_br_join(struct ppe_drv_iface *br_iface, struct net_device *member);

/**
 * ppe_drv_br_deinit
 *	Uninitialize bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface for bridge.
 *
 * @return
 * Status of the uninitialization.
 */
ppe_drv_ret_t ppe_drv_br_deinit(struct ppe_drv_iface *br_iface);

/**
 * ppe_drv_br_init
 *	Initialize bridge interface in PPE.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface allocated for bridge.
 *
 * @return
 * Status of the initialization.
 */
ppe_drv_ret_t ppe_drv_br_init(struct ppe_drv_iface *br_iface);
#endif /* _PPE_DRV_BR_H_ */
