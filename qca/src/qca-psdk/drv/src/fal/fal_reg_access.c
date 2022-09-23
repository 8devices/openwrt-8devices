/*
 * Copyright (c) 2012, 2017-2018, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup fal_reg_access FAL_REG_ACCESS
 * @{
 */
#include "sw.h"
#include "fal_reg_access.h"
#include "hsl_api.h"
#include "hsl_phy.h"

static sw_error_t
_fal_phy_get(a_uint32_t dev_id, a_uint32_t phy_addr,
             a_uint32_t reg, a_uint16_t * value)
{
    sw_error_t rv;
    hsl_api_t *p_api;
    a_uint8_t phy_addr_type;
    hsl_phy_get phy_get_func;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    /* the MSB first byte of phy_addr marks the type of
     * phy address, such as the i2c address, the value of
     * MSB first byte should be 1 */
    phy_addr_type = (phy_addr & 0xff000000) >> 24;
    phy_addr = phy_addr & 0xff;
    switch (phy_addr_type) {
	    case PHY_I2C_ACCESS:
		    phy_get_func = p_api->phy_i2c_get;
		    break;
	    default:
		    phy_get_func = p_api->phy_get;
		    break;
    }

    if (NULL == phy_get_func) {
	    return SW_NOT_SUPPORTED;
    }

    rv = phy_get_func(dev_id, phy_addr, reg, value);
    return rv;
}

static sw_error_t
_fal_phy_set(a_uint32_t dev_id, a_uint32_t phy_addr,
             a_uint32_t reg, a_uint16_t value)
{
    sw_error_t rv;
    hsl_api_t *p_api;
    a_uint8_t phy_addr_type;
    hsl_phy_set phy_set_func;

    SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));

    /* the MSB first byte of phy_addr marks the type of
     * phy address, such as the i2c address, the value of
     * MSB first byte should be 1 */
    phy_addr_type = (phy_addr & 0xff000000) >> 24;
    phy_addr = phy_addr & 0xff;
    switch (phy_addr_type) {
	    case PHY_I2C_ACCESS:
		    phy_set_func = p_api->phy_i2c_set;
		    break;
	    default:
		    phy_set_func = p_api->phy_set;
		    break;
    }

    if (NULL == phy_set_func) {
	    return SW_NOT_SUPPORTED;
    }

    rv = phy_set_func(dev_id, phy_addr, reg, value);
    return rv;
}
/**
  * fal_phy_get - get value of specific phy device
  * @phy_addr: id of the phy device
  * @reg: register id of phy device
  * @value: pointer to the memory storing the value.
  * @return SW_OK or error code
  */
sw_error_t
fal_phy_get(a_uint32_t dev_id, a_uint32_t phy_addr,
            a_uint32_t reg, a_uint16_t * value)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_phy_get(dev_id, phy_addr, reg, value);
    FAL_API_UNLOCK;
    return rv;
}

/**
  * fal_phy_set - set value of specific phy device
  * @phy_addr: id of the phy device
  * @reg: register id of phy device
  * @value: register value.
  * @return SW_OK or error code
  */
sw_error_t
fal_phy_set(a_uint32_t dev_id, a_uint32_t phy_addr,
            a_uint32_t reg, a_uint16_t value)
{
    sw_error_t rv;

    FAL_API_LOCK;
    rv = _fal_phy_set(dev_id, phy_addr, reg, value);
    FAL_API_UNLOCK;
    return rv;
}
