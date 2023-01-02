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
 * @file ppe_drv_iface.h
 *	NSS PPE interface definitions.
 */

#ifndef _PPE_DRV_IFACE_H_
#define _PPE_DRV_IFACE_H_

/**
 * @addtogroup ppe_drv_iface_subsystem
 * @{
 */

/**
 * ppe_drv_iface
 *	Forward declaration for PPE interface object.
 */
struct ppe_drv_iface;
enum ppe_drv_ret;
typedef enum ppe_drv_ret ppe_drv_ret_t;

/**
 * PPE Interface Number.
 */
typedef int32_t ppe_drv_iface_t;

/**
 * ppe_drv_iface_type
 *	PPE interface type.
 */
enum ppe_drv_iface_type {
	PPE_DRV_IFACE_TYPE_INVALID,		/**< Interface type invalid. */
	PPE_DRV_IFACE_TYPE_BRIDGE,		/**< Interface type bridge. */
	PPE_DRV_IFACE_TYPE_LAG,			/**< Interface type LAG. */
	PPE_DRV_IFACE_TYPE_PPPOE,		/**< Interface type PPPoE. */
	PPE_DRV_IFACE_TYPE_VLAN,		/**< Interface type VLAN. */
	PPE_DRV_IFACE_TYPE_PHYSICAL,		/**< Interface type physical port. */
	PPE_DRV_IFACE_TYPE_VIRTUAL,		/**< Interface type virtual port. */
	PPE_DRV_IFACE_TYPE_VP_TUN,		/**< Interface type VP for hardware tunnel. */
	PPE_DRV_IFACE_TYPE_MAX,			/**< Interface type max. */
};

/*
 * ppe_drv_iface_mtu_set()
 *	Set MTU for a given PPE interface.
 *
 * @datatypes
 * ppe_drv_iface
 * uint16_t
 *
 * @param[IN] iface   PPE interface.
 * @param[IN] MTU   New MTU to be used.
 *
 * @return
 * ppe_drv_iface_error
 */
ppe_drv_ret_t ppe_drv_iface_mtu_set(struct ppe_drv_iface *iface, uint16_t mtu);

/*
 * ppe_drv_iface_mac_addr_clear()
 *	Clear Mac address for a given PPE interface.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[IN] iface   PPE interface.
 *
 * @return
 * ppe_drv_iface_error
 */
ppe_drv_ret_t ppe_drv_iface_mac_addr_clear(struct ppe_drv_iface *iface);

/*
 * ppe_drv_iface_mac_addr_set()
 *	Set Mac address for a given PPE interface.
 *
 * @datatypes
 * ppe_drv_iface
 * uint8_t *
 *
 * @param[IN] iface   PPE interface.
 * @param[IN] mac_addr   New MAC address to be used.
 *
 * @return
 * ppe_drv_iface_error
 */
ppe_drv_ret_t ppe_drv_iface_mac_addr_set(struct ppe_drv_iface *iface, uint8_t *mac_addr);

/**
 * ppe_drv_iface_alloc
 *	Allocate PPE interface of a specific type.
 *
 * @datatypes
 * ppe_drv_iface_type
 *
 * @param[in] type   Type of PPE interface need to be allocated.
 * @param[in] dev    Associated net device.
 *
 * @return
 * ppe_drv_iface
 */
struct ppe_drv_iface *ppe_drv_iface_alloc(enum ppe_drv_iface_type type, struct net_device *dev);

/**
 * ppe_drv_iface_deref
 *	Release reference on PPE interface.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] ppe_iface   Pointer to the PPE interface.
 *
 * @return
 * True if there are more reference or false if this was the last reference on PPE interface.
 */
bool ppe_drv_iface_deref(struct ppe_drv_iface *ppe_iface);

/**
 * ppe_drv_iface_get_by_dev
 *	Get PPE interface from net device.
 *
 * @param[in] dev    Associated net device.
 *
 * @return
 * PPE interface number if lookup is successful else -1 for failure.
 */
struct ppe_drv_iface *ppe_drv_iface_get_by_dev(struct net_device *dev);

/**
 * ppe_drv_iface_port_idx_get
 *	Get port index from PPE interface.
 *
 * @param[in] iface    Associated PPE interface.
 *
 * @return
 * int32_t
 */
int32_t ppe_drv_iface_port_idx_get(struct ppe_drv_iface *iface);

/**
 * ppe_drv_iface_idx_get_by_dev
 *	Get PPE interface index from net device.
 *
 * @param[in] dev    Associated net device.
 *
 * @return
 * ppe_drv_iface_t
 */
ppe_drv_iface_t ppe_drv_iface_idx_get_by_dev(struct net_device *dev);

/** @} */ /* end_addtogroup ppe_drv_iface_subsystem */

#endif /* _PPE_DRV_IFACE_H_ */
