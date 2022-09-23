/*
 * Copyright (c) 2014, 2017-2019, The Linux Foundation. All rights reserved.
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


#ifndef _SSDK_INIT_H_
#define _SSDK_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
    typedef enum {
        HSL_MDIO = 1,
        HSL_HEADER,
    }
    hsl_access_mode;

    typedef enum
    {
        HSL_NO_CPU = 0,
        HSL_CPU_1,
        HSL_CPU_2,
        HSL_CPU_1_PLUS,
    } hsl_init_mode;
    typedef sw_error_t
    (*mdio_reg_set) (a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t reg,
                     a_uint16_t data);

    typedef sw_error_t
    (*mdio_reg_get) (a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t reg,
                     a_uint16_t * data);

    typedef sw_error_t
    (*i2c_reg_set) (a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t reg,
                     a_uint16_t data);

    typedef sw_error_t
    (*i2c_reg_get) (a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t reg,
                     a_uint16_t * data);
    typedef struct
    {
        mdio_reg_set    mdio_set;
        mdio_reg_get    mdio_get;
        i2c_reg_set    i2c_set;
        i2c_reg_get    i2c_get;
    } hsl_reg_func;
    typedef enum
    {
        CHIP_UNSPECIFIED = 0,
        CHIP_ATHENA,
        CHIP_GARUDA,
        CHIP_SHIVA,
        CHIP_HORUS,
        CHIP_ISIS,
        CHIP_ISISC,
        CHIP_DESS,
        CHIP_HPPE,
    } ssdk_chip_type;
typedef struct
{
	hsl_init_mode   cpu_mode;
	hsl_access_mode reg_mode;
	hsl_reg_func    reg_func;

	ssdk_chip_type  chip_type;
	a_uint32_t      chip_revision;
	/* os specific parameter */
	/* when uk_if based on netlink, it's netlink protocol type*/
	/* when uk_if based on ioctl, it's minor device number, major number
	is always 10(misc device) */
	a_uint32_t      nl_prot;
	/* chip specific parameter */
	void *          chip_spec_cfg;
	a_uint32_t      phy_id;
	a_uint32_t      mac_mode1;
	a_uint32_t      mac_mode2;
} ssdk_init_cfg;
#define def_init_cfg  {.reg_mode = HSL_MDIO, .cpu_mode = HSL_CPU_2};
#define CFG_STR_SIZE 20
    typedef struct
    {
        a_uint8_t build_ver[CFG_STR_SIZE];
        a_uint8_t build_date[CFG_STR_SIZE];

        a_uint8_t chip_type[CFG_STR_SIZE]; //GARUDA
        a_uint8_t cpu_type[CFG_STR_SIZE];  //mips
        a_uint8_t os_info[CFG_STR_SIZE];   //OS=linux OS_VER=2_6

        a_bool_t  fal_mod;
        a_bool_t  kernel_mode;
        a_bool_t  uk_if;
        ssdk_init_cfg init_cfg;
    } ssdk_cfg_t;
    sw_error_t
    ssdk_init(a_uint32_t dev_id, ssdk_init_cfg *cfg);
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _SSDK_INIT_H */
