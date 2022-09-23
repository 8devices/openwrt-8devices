/*
 * Copyright (c) 2012,2018, The Linux Foundation. All rights reserved.
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


#ifndef _HSL_H
#define _HSL_H

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "ssdk_init.h"

    typedef sw_error_t
    (*hsl_acl_rule_copy) (a_uint32_t dev_id, a_uint32_t src_addr,
                          a_uint32_t dest_addr, a_uint32_t size);

    typedef sw_error_t
    (*hsl_acl_rule_invalid) (a_uint32_t dev_id, a_uint32_t addr,
                             a_uint32_t size);

    typedef sw_error_t
    (*hsl_acl_addr_update) (a_uint32_t dev_id, a_uint32_t old_addr,
                            a_uint32_t new_addr, a_uint32_t info);

    typedef struct
    {
        hsl_acl_rule_copy     acl_rule_copy;
        hsl_acl_rule_invalid  acl_rule_invalid;
        hsl_acl_addr_update   acl_addr_update;
    } hsl_acl_func_t;
extern sw_error_t reduce_hsl_phy_get(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t* value);
#define HSL_PHY_GET(rv, dev, phy_addr, reg, value) \
		rv = reduce_hsl_phy_get(dev,phy_addr,reg,value);


extern sw_error_t reduce_hsl_phy_set(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t value);
#define HSL_PHY_SET(rv, dev, phy_addr, reg, value) \
		rv = reduce_hsl_phy_set(dev,phy_addr,reg,value);

extern sw_error_t hsl_phy_i2c_get(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t* value);
#define HSL_PHY_I2C_GET(rv, dev, phy_addr, reg, value) \
		rv = hsl_phy_i2c_get(dev,phy_addr,reg,value);


extern sw_error_t hsl_phy_i2c_set(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t value);
#define HSL_PHY_I2C_SET(rv, dev, phy_addr, reg, value) \
		rv = hsl_phy_i2c_set(dev,phy_addr,reg,value);
#if (defined(API_LOCK) \
&& (defined(HSL_STANDALONG) || (defined(KERNEL_MODULE) && defined(USER_MODE))))
    extern  aos_lock_t sw_hsl_api_lock;
#define HSL_API_LOCK    aos_lock(&sw_hsl_api_lock)
#define HSL_API_UNLOCK  aos_unlock(&sw_hsl_api_lock)
#else
#define HSL_API_LOCK
#define HSL_API_UNLOCK
#endif

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _HSL_H */
