/*
 * Copyright (c) 2012, 2014-2020, The Linux Foundation. All rights reserved.
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
#include "fal_init.h"
#include "fal.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "hsl_phy.h"
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/phy.h>
#include <linux/mdio.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/reset.h>
#include "ssdk_plat.h"
#if defined(IN_PHY_I2C_MODE)
#include "ssdk_phy_i2c.h"
#endif
struct qca_phy_priv **qca_phy_priv_global;

struct qca_phy_priv* ssdk_phy_priv_data_get(a_uint32_t dev_id)
{
	if (dev_id >= SW_MAX_NR_DEV || !qca_phy_priv_global)
		return NULL;

	return qca_phy_priv_global[dev_id];
}
sw_error_t
ssdk_init(a_uint32_t dev_id, ssdk_init_cfg * cfg)
{
	sw_error_t rv;

	rv = fal_init(dev_id, cfg);
	if (rv != SW_OK)
		SSDK_ERROR("ssdk fal init failed: %d. \r\n", rv);

	rv = ssdk_phy_driver_init(dev_id, cfg);
	if (rv != SW_OK)
		SSDK_ERROR("ssdk phy init failed: %d. \r\n", rv);

	return rv;
}

sw_error_t
ssdk_cleanup(void)
{
	sw_error_t rv;

	rv = fal_cleanup();
	rv = ssdk_phy_driver_cleanup();

	return rv;
}
static int chip_is_scomphy(a_uint32_t dev_id, ssdk_init_cfg* cfg)
{
	int rv = -ENODEV;
	a_uint32_t phy_id = 0, port_id = 0;
	a_uint32_t port_bmp = qca_ssdk_port_bmp_get(dev_id);
	while (port_bmp) {
		if (port_bmp & 0x1) {
			phy_id = hsl_phyid_get(dev_id, port_id, cfg);
			switch (phy_id) {
				case QCA8081_PHY_V1_1:
						cfg->chip_type = CHIP_SCOMPHY;
						/*MP GEPHY is always the first port*/
						if(cfg->phy_id == 0)
						{
							cfg->phy_id = phy_id;
						}
						rv = SW_OK;
					break;
				default:
					break;
			}
		}
		port_bmp >>= 1;
		port_id++;
	}

	return rv;
}

static int chip_ver_get(a_uint32_t dev_id, ssdk_init_cfg* cfg)
{
	int rv = SW_OK;
	a_uint8_t chip_ver = 0;
	a_uint8_t chip_revision = 0;
	if(chip_ver == QCA_VER_AR8227)
		cfg->chip_type = CHIP_SHIVA;
	else if(chip_ver == QCA_VER_AR8337)
		cfg->chip_type = CHIP_ISISC;
	else if(chip_ver == QCA_VER_AR8327)
		cfg->chip_type = CHIP_ISIS;
	else if(chip_ver == QCA_VER_DESS)
		cfg->chip_type = CHIP_DESS;
	else if(chip_ver == QCA_VER_HPPE) {
		cfg->chip_type = CHIP_HPPE;
		cfg->chip_revision = chip_revision;
	}
	else {
		/* try single phy without switch connected */
		rv = chip_is_scomphy(dev_id, cfg);
	}

	return rv;
}
static void ssdk_cfg_default_init(ssdk_init_cfg *cfg)
{
	memset(cfg, 0, sizeof(ssdk_init_cfg));
	cfg->cpu_mode = HSL_CPU_1;
	cfg->nl_prot = 30;
	cfg->reg_func.mdio_set = qca_ar8327_phy_write;
	cfg->reg_func.mdio_get = qca_ar8327_phy_read;
#if defined(IN_PHY_I2C_MODE)
	cfg->reg_func.i2c_set = qca_phy_i2c_write;
	cfg->reg_func.i2c_get = qca_phy_i2c_read;
#endif
}
static void ssdk_free_priv(void)
{
	a_uint32_t dev_id, dev_num = 1;

	if(!qca_phy_priv_global) {
		return;
	}
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (qca_phy_priv_global[dev_id]) {
			kfree(qca_phy_priv_global[dev_id]);
		}

		qca_phy_priv_global[dev_id] = NULL;
	}

	kfree(qca_phy_priv_global);

	qca_phy_priv_global = NULL;
}

static int ssdk_alloc_priv(a_uint32_t dev_num)
{
	int rev = SW_OK;
	a_uint32_t dev_id = 0;

	qca_phy_priv_global = kzalloc(dev_num * sizeof(struct qca_phy_priv *), GFP_KERNEL);
	if (qca_phy_priv_global == NULL) {
		return -ENOMEM;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		qca_phy_priv_global[dev_id] = kzalloc(sizeof(struct qca_phy_priv), GFP_KERNEL);
		if (qca_phy_priv_global[dev_id] == NULL) {
			return -ENOMEM;
		}
		qca_ssdk_port_bmp_init(dev_id);
		qca_ssdk_phy_info_init(dev_id);
	}

	return rev;
}

#ifndef SSDK_STR
#define SSDK_STR "ssdk"
#endif
#if defined (ISISC) || defined (ISIS)
static void qca_ar8327_gpio_reset(struct qca_phy_priv *priv)
{
	struct device_node *np = NULL;
	const __be32 *reset_gpio;
	a_int32_t len;
	int gpio_num = 0, ret = 0;

	if (priv->ess_switch_flag == A_TRUE)
		np = priv->of_node;
	else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
		np = priv->phy->mdio.dev.of_node;
#else
		np = priv->phy->dev.of_node;
#endif
	if(!np)
		return;

	reset_gpio = of_get_property(np, "reset_gpio", &len);
	if (!reset_gpio )
	{
		SSDK_INFO("reset_gpio node does not exist\n");
		return;
	}

	gpio_num = be32_to_cpup(reset_gpio);
	if(gpio_num <= 0)
	{
		SSDK_INFO("reset gpio doesn't exist\n ");
		return;
	}
	ret = gpio_request(gpio_num, "reset_gpio");
	if(ret)
	{
		SSDK_ERROR("gpio%d request failed, ret:%d\n", gpio_num, ret);
		return;
	}
	gpio_direction_output(gpio_num, SSDK_GPIO_RESET);
	msleep(200);
	gpio_set_value(gpio_num, SSDK_GPIO_RELEASE);
	SSDK_INFO("GPIO%d reset switch done\n", gpio_num);

	gpio_free(gpio_num);

	return;
}
#endif
static int __init regi_init(void)
{
	a_uint32_t num = 0, dev_id = 0, dev_num = 1;
	ssdk_init_cfg cfg;
	int rv = 0;
	rv = ssdk_alloc_priv(dev_num);
	if (rv)
		goto out;

	for (num = 0; num < dev_num; num++) {
		ssdk_cfg_default_init(&cfg);
		rv = ssdk_plat_init(&cfg, dev_id);
		SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
		rv = chip_ver_get(dev_id, &cfg);
		SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);
		rv = ssdk_init(dev_id, &cfg);
		SW_CNTU_ON_ERROR_AND_COND1_OR_GOTO_OUT(rv, -ENODEV);

	}

out:
	if (rv == 0)
		SSDK_INFO("qca-%s module init succeeded!\n", SSDK_STR);
	else {
		if (rv == -ENODEV) {
			rv = 0;
			SSDK_INFO("qca-%s module init, no device found!\n", SSDK_STR);
		} else {
			SSDK_INFO("qca-%s module init failed! (code: %d)\n", SSDK_STR, rv);
			ssdk_free_priv();
		}
	}

	return rv;
}

static void __exit
regi_exit(void)
{
	sw_error_t rv;
	rv = ssdk_cleanup();

	if (rv == 0)
		SSDK_INFO("qca-%s module exit  done!\n", SSDK_STR);
	else
		SSDK_ERROR("qca-%s module exit failed! (code: %d)\n", SSDK_STR, rv);
	ssdk_free_priv();
}

module_init(regi_init);
module_exit(regi_exit);

MODULE_DESCRIPTION("QCA SSDK Driver");
MODULE_LICENSE("Dual BSD/GPL");
