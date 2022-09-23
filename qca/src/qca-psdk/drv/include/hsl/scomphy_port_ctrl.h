/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


#ifndef _SCOMPHY_PORT_CTRL_H_
#define _SCOMPHY_PORT_CTRL_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#include "fal/fal_port_ctrl.h"

sw_error_t scomphy_port_ctrl_init (a_uint32_t dev_id);

sw_error_t
scomphy_port_reset (a_uint32_t dev_id, fal_port_t port_id);

#define SCOMPHY_PORT_CTRL_INIT(rv, dev_id) \
    { \
        rv = scomphy_port_ctrl_init(dev_id); \
        SW_RTN_ON_ERROR(rv); \
    }
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _SCOMPHY_PORT_CTRL_H_ */
