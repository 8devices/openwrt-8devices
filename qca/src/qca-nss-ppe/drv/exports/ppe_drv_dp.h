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
 * @file ppe_drv_dp.h
 *	NSS PPE DP definitions.
 */

#ifndef _PPE_DRV_DP_H_
#define _PPE_DRV_DP_H_

/**
 * @addtogroup ppe_drv_dp_subsystem
 * @{
 */

/**
 * ppe_drv_dp_deinit
 *	Deinitialize DP.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface   Pointer to the PPE interface for bridge.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_deinit(struct ppe_drv_iface *iface);

/**
 * ppe_drv_dp_init
 *	Initialize DP.
 *
 * @datatypes
 * ppe_drv_iface
 *
 * @param[in] iface   Pointer to the PPE interface for bridge.
 * @param[in] macid   MAC ID of the port.
 *
 * @return
 * Status of the operation.
 */
ppe_drv_ret_t ppe_drv_dp_init(struct ppe_drv_iface *iface, uint32_t macid);

/** @} */ /* end_addtogroup ppe_drv_dp_subsystem */

#endif /* _PPE_DRV_DP_H_ */
