/*
 * Copyright (c) 2012, 2017-2019, The Linux Foundation. All rights reserved.
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
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_lock.h"
#include "sd.h"
#include "scomphy_init.h"
#include "sw_api.h"
#include "sw_api_ks.h"
static hsl_dev_t dev_table[SW_MAX_NR_DEV];
static ssdk_init_cfg *dev_ssdk_cfg[SW_MAX_NR_DEV] = { 0 };
ssdk_chip_type SSDK_CURRENT_CHIP_TYPE = CHIP_UNSPECIFIED;

static sw_error_t hsl_set_current_chip_type(ssdk_chip_type chip_type)
{
    sw_error_t rv = SW_OK;

    SSDK_CURRENT_CHIP_TYPE = chip_type;

    if (SSDK_CURRENT_CHIP_TYPE == CHIP_UNSPECIFIED)
    {
        SSDK_CURRENT_CHIP_TYPE = CHIP_SCOMPHY;
    }
    return rv;
}

hsl_dev_t *
hsl_dev_ptr_get(a_uint32_t dev_id)
{
    if (dev_id >= SW_MAX_NR_DEV)
        return NULL;

    return &dev_table[dev_id];
}
sw_error_t
hsl_dev_init(a_uint32_t dev_id, ssdk_init_cfg *cfg)
{
    sw_error_t rv = SW_OK;
    static int dev_init = 0;

    if (SW_MAX_NR_DEV <= dev_id)
    {
        return SW_BAD_PARAM;
    }

    aos_mem_set(&dev_table[dev_id], 0, sizeof (hsl_dev_t));

    if (!dev_init) {
        SW_RTN_ON_ERROR(sd_init(dev_id,cfg));

#ifdef UK_IF
        SW_RTN_ON_ERROR(sw_uk_init(cfg->nl_prot));
#endif

#if defined API_LOCK
        SW_RTN_ON_ERROR(hsl_api_lock_init());
#endif
        dev_init = 1;
    }
    rv = hsl_set_current_chip_type(cfg->chip_type);
    SW_RTN_ON_ERROR(rv);

    if (NULL == dev_ssdk_cfg[dev_id])
    {
        dev_ssdk_cfg[dev_id] = aos_mem_alloc(sizeof (ssdk_init_cfg));
    }

    if (NULL == dev_ssdk_cfg[dev_id])
    {
        return SW_OUT_OF_MEM;
    }

    aos_mem_copy(dev_ssdk_cfg[dev_id], cfg, sizeof (ssdk_init_cfg));
#if defined UK_MINOR_DEV
    dev_ssdk_cfg[dev_id]->nl_prot = UK_MINOR_DEV;
#endif

    rv = SW_INIT_ERROR;
    switch (cfg->chip_type)
    {
	case CHIP_SCOMPHY:
	    rv = scomphy_init(dev_id, cfg);
	    break;
        default:
            return SW_BAD_PARAM;
    }

    return rv;
}
sw_error_t
hsl_dev_cleanup(void)
{
    sw_error_t rv = SW_OK;
    a_uint32_t dev_id;

    for (dev_id = 0; dev_id < SW_MAX_NR_DEV; dev_id++)
    {
        if (dev_ssdk_cfg[dev_id])
        {
            hsl_api_t *p_api;

            SW_RTN_ON_NULL(p_api = hsl_api_ptr_get(dev_id));
            if (p_api->dev_clean)
            {
                rv = p_api->dev_clean(dev_id);
                SW_RTN_ON_ERROR(rv);
            }

            aos_mem_free(dev_ssdk_cfg[dev_id]);
            dev_ssdk_cfg[dev_id] = NULL;
        }
    }

#ifdef UK_IF
    SW_RTN_ON_ERROR(sw_uk_cleanup());
#endif

    return SW_OK;
}
sw_error_t reduce_hsl_phy_set(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t value)
{
	sw_error_t rv;

	hsl_api_t *p_api = hsl_api_ptr_get(dev); 
	if (p_api) { 
	    rv = p_api->phy_set(dev, phy_addr, reg, value); 
	} else { 
	    rv = SW_NOT_INITIALIZED; 
	} 

	return rv;
}

sw_error_t reduce_hsl_phy_get(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t* value)
{
	sw_error_t rv;

	hsl_api_t *p_api = hsl_api_ptr_get(dev);
	if (p_api) {
	    rv = p_api->phy_get(dev, phy_addr, reg, value);
	} else {
	    rv = SW_NOT_INITIALIZED;
	}

	return rv;
}

sw_error_t hsl_phy_i2c_set(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t value)
{
	sw_error_t rv;

	hsl_api_t *p_api = hsl_api_ptr_get(dev);
	if (p_api) {
	    rv = p_api->phy_i2c_set(dev, phy_addr, reg, value);
	} else {
	    rv = SW_NOT_INITIALIZED;
	}

	return rv;
}

sw_error_t hsl_phy_i2c_get(a_uint32_t dev,a_uint32_t phy_addr,a_uint32_t reg,a_uint16_t* value)
{
	sw_error_t rv;

	hsl_api_t *p_api = hsl_api_ptr_get(dev);
	if (p_api) {
	    rv = p_api->phy_i2c_get(dev, phy_addr, reg, value);
	} else {
	    rv = SW_NOT_INITIALIZED;
	}

	return rv;
}

#if 0
void reduce_sw_set_reg_by_field_u32(unsigned int reg_value,unsigned int field_value,
													unsigned int reg_offset,unsigned int reg_len)
{
    do {
        (reg_value) = (((reg_value) & SW_FIELD_MASK_NOT_U32((reg_offset),(reg_offset)))
              | (((field_value) & SW_BIT_MASK_U32(reg_len)) << (reg_offset)));
    } while (0);

}


void reduce_sw_field_get_by_reg_u32(unsigned int reg_value,unsigned int field_value,
													unsigned int reg_offset,unsigned int reg_len)
{
    do {
        (field_value) = (((reg_value) >> (reg_offset)) & SW_BIT_MASK_U32(reg_len));
    } while (0);

}
#endif
