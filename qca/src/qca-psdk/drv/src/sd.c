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


#include "sw.h"
#include "ssdk_init.h"
#include "sd.h"
#include "sw_api.h"
#if ((!defined(KERNEL_MODULE)) && defined(UK_IF))
#include "sw_api_us.h"
#endif

mdio_reg_set ssdk_mdio_set    = NULL;
mdio_reg_get ssdk_mdio_get    = NULL;
i2c_reg_set ssdk_i2c_set    = NULL;
i2c_reg_get ssdk_i2c_get    = NULL;
sw_error_t
sd_reg_mdio_set(a_uint32_t dev_id, a_uint32_t phy, a_uint32_t reg,
                a_uint16_t data)
{
    sw_error_t rv = SW_OK;

    if (NULL != ssdk_mdio_set)
    {
        rv = ssdk_mdio_set(dev_id, phy, reg, data);
    }
    else
    {
#if ((!defined(KERNEL_MODULE)) && defined(UK_IF))
        {
            a_uint32_t args[SW_MAX_API_PARAM];

            args[0] = SW_API_PHY_SET;
            args[1] = (a_uint32_t) & rv;
            args[2] = dev_id;
            args[3] = phy;
            args[4] = reg;
            args[5] = data;
            if (SW_OK != sw_uk_if(args))
            {
                return SW_FAIL;
            }
        }
#else
        return SW_NOT_SUPPORTED;
#endif
    }

    return rv;
}

sw_error_t
sd_reg_mdio_get(a_uint32_t dev_id, a_uint32_t phy, a_uint32_t reg, a_uint16_t * data)
{
    sw_error_t rv = SW_OK;

    if (NULL != ssdk_mdio_get)
    {
        rv = ssdk_mdio_get(dev_id, phy, reg, data);
    }
    else
    {
#if ((!defined(KERNEL_MODULE)) && defined(UK_IF))
        {
            a_uint32_t args[SW_MAX_API_PARAM];
            a_uint32_t tmp;

            args[0] = SW_API_PHY_GET;
            args[1] = (a_uint32_t) & rv;
            args[2] = dev_id;
            args[3] = phy;
            args[4] = reg;
            args[5] = (a_uint32_t) & tmp;
            if (SW_OK != sw_uk_if(args))
            {
                return SW_FAIL;
            }
            *data = *((a_uint16_t *)&tmp);
        }
#else
        return SW_NOT_SUPPORTED;
#endif
    }

    return rv;
}

sw_error_t
sd_reg_i2c_set(a_uint32_t dev_id, a_uint32_t phy, a_uint32_t reg,
                a_uint16_t data)
{
    sw_error_t rv = SW_OK;

    if (NULL != ssdk_i2c_set)
    {
        rv = ssdk_i2c_set(dev_id, phy, reg, data);
    }
    else
    {
#if ((!defined(KERNEL_MODULE)) && defined(UK_IF))
        {
            a_uint32_t args[SW_MAX_API_PARAM];

            args[0] = SW_API_PHY_I2C_SET;
            args[1] = (a_uint32_t) & rv;
            args[2] = dev_id;
            args[3] = phy;
            args[4] = reg;
            args[5] = data;
            if (SW_OK != sw_uk_if(args))
            {
                return SW_FAIL;
            }
        }
#else
        return SW_NOT_SUPPORTED;
#endif
    }

    return rv;
}

sw_error_t
sd_reg_i2c_get(a_uint32_t dev_id, a_uint32_t phy, a_uint32_t reg, a_uint16_t * data)
{
    sw_error_t rv = SW_OK;

    if (NULL != ssdk_i2c_get)
    {
        rv = ssdk_i2c_get(dev_id, phy, reg, data);
    }
    else
    {
#if ((!defined(KERNEL_MODULE)) && defined(UK_IF))
        {
            a_uint32_t args[SW_MAX_API_PARAM];
            a_uint32_t tmp;

            args[0] = SW_API_PHY_I2C_GET;
            args[1] = (a_uint32_t) & rv;
            args[2] = dev_id;
            args[3] = phy;
            args[4] = reg;
            args[5] = (a_uint32_t) & tmp;
            if (SW_OK != sw_uk_if(args))
            {
                return SW_FAIL;
            }
            *data = *((a_uint16_t *)&tmp);
        }
#else
        return SW_NOT_SUPPORTED;
#endif
    }

    return rv;
}
sw_error_t
sd_init(a_uint32_t dev_id, ssdk_init_cfg * cfg)
{
    if (NULL != cfg->reg_func.mdio_set)
    {
        ssdk_mdio_set = cfg->reg_func.mdio_set;
    }

    if (NULL != cfg->reg_func.mdio_get)
    {
        ssdk_mdio_get = cfg->reg_func.mdio_get;
    }

    if (NULL != cfg->reg_func.i2c_set)
    {
        ssdk_i2c_set = cfg->reg_func.i2c_set;
    }

    if (NULL != cfg->reg_func.i2c_get)
    {
        ssdk_i2c_get = cfg->reg_func.i2c_get;
    }
    return SW_OK;
}
