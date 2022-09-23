/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
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
#include "ssdk_init.h"
#include <linux/version.h>
#if defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#elif defined(CONFIG_OF) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <drivers/leds/leds-ipq40xx.h>
#include <linux/of_platform.h>
#include <linux/reset.h>
#else
#include <linux/ar8216_platform.h>
#include <drivers/net/phy/ar8216.h>
#include <drivers/net/ethernet/atheros/ag71xx/ag71xx.h>
#endif
#include "ssdk_plat.h"

extern struct qca_phy_priv **qca_phy_priv_global;
extern ssdk_chip_type SSDK_CURRENT_CHIP_TYPE;
#define MDIO_BUS_0					0
#define MDIO_BUS_1					1
a_uint32_t ssdk_log_level = SSDK_LOG_LEVEL_DEFAULT;
a_bool_t
phy_addr_validation_check(a_uint32_t phy_addr)
{

	if ((phy_addr > SSDK_PHY_BCAST_ID) || (phy_addr < SSDK_PHY_MIN_ID))
		return A_FALSE;
	else
		return A_TRUE;
}

static struct mii_bus *
ssdk_phy_miibus_get(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	struct mii_bus *bus = NULL;
	if (!bus)
		bus = qca_phy_priv_global[dev_id]->miibus;

	return bus;
}

sw_error_t
qca_ar8327_phy_read(a_uint32_t dev_id, a_uint32_t phy_addr,
                           a_uint32_t reg, a_uint16_t* data)
{
	struct mii_bus *bus = NULL;

	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return SW_BAD_PARAM;
	}

	bus = ssdk_phy_miibus_get(dev_id, phy_addr);
	if (!bus)
		return SW_NOT_SUPPORTED;

	*data = mdiobus_read(bus, phy_addr, reg);
	return 0;
}

sw_error_t
qca_ar8327_phy_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                            a_uint32_t reg, a_uint16_t data)
{
	struct mii_bus *bus = NULL;

	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return SW_BAD_PARAM;
	}

	bus = ssdk_phy_miibus_get(dev_id, phy_addr);
	if (!bus)
		return SW_NOT_SUPPORTED;

	mdiobus_write(bus, phy_addr, reg, data);
	return 0;
}

void
qca_ar8327_phy_dbg_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                                a_uint16_t dbg_addr, a_uint16_t dbg_data)
{
	struct mii_bus *bus = NULL;

	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return;
	}

	bus = ssdk_phy_miibus_get(dev_id, phy_addr);
	if (!bus)
		return;

	mdiobus_write(bus, phy_addr, QCA_MII_DBG_ADDR, dbg_addr);
	mdiobus_write(bus, phy_addr, QCA_MII_DBG_DATA, dbg_data);
}

void
qca_ar8327_phy_dbg_read(a_uint32_t dev_id, a_uint32_t phy_addr,
		                a_uint16_t dbg_addr, a_uint16_t *dbg_data)
{
	struct mii_bus *bus = NULL;

	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return;
	}

	bus = ssdk_phy_miibus_get(dev_id, phy_addr);
	if (!bus)
		return;

	mdiobus_write(bus, phy_addr, QCA_MII_DBG_ADDR, dbg_addr);
	*dbg_data = mdiobus_read(bus, phy_addr, QCA_MII_DBG_DATA);
}


void
qca_ar8327_mmd_write(a_uint32_t dev_id, a_uint32_t phy_addr,
                          a_uint16_t addr, a_uint16_t data)
{
	struct mii_bus *bus = NULL;

	if (A_TRUE != phy_addr_validation_check (phy_addr))
	{
		return;
	}

	bus = ssdk_phy_miibus_get(dev_id, phy_addr);
	if (!bus)
		return;

	mdiobus_write(bus, phy_addr, QCA_MII_MMD_ADDR, addr);
	mdiobus_write(bus, phy_addr, QCA_MII_MMD_DATA, data);
}

void qca_phy_mmd_write(u32 dev_id, u32 phy_id,
                     u16 mmd_num, u16 reg_id, u16 reg_val)
{
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR, mmd_num);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_DATA, reg_id);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR,
			0x4000 | mmd_num);
	qca_ar8327_phy_write(dev_id, phy_id,
		QCA_MII_MMD_DATA, reg_val);
}

u16 qca_phy_mmd_read(u32 dev_id, u32 phy_id,
		u16 mmd_num, u16 reg_id)
{
	u16 value = 0;
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR, mmd_num);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_DATA, reg_id);
	qca_ar8327_phy_write(dev_id, phy_id,
			QCA_MII_MMD_ADDR,
			0x4000 | mmd_num);
	qca_ar8327_phy_read(dev_id, phy_id,
			QCA_MII_MMD_DATA, &value);
	return value;
}
static int miibus_get(a_uint32_t dev_id)
{
	struct device_node *mdio_node = NULL;
	struct device_node *switch_node = NULL;
	struct platform_device *mdio_plat = NULL;
	struct ipq40xx_mdio_data *mdio_data = NULL;
	struct qca_phy_priv *priv;
	hsl_reg_mode reg_mode = HSL_REG_LOCAL_BUS;
	priv = qca_phy_priv_global[dev_id];
	switch_node = qca_phy_priv_global[dev_id]->of_node;
	if (switch_node) {
		mdio_node = of_parse_phandle(switch_node, "mdio-bus", 0);
		if (mdio_node) {
			priv->miibus = of_mdio_find_bus(mdio_node);
			return 0;
		}
	}
	if(reg_mode == HSL_REG_LOCAL_BUS)
		mdio_node = of_find_compatible_node(NULL, NULL, "qcom,ipq40xx-mdio");
	else
		mdio_node = of_find_compatible_node(NULL, NULL, "virtual,mdio-gpio");

	if (!mdio_node) {
		SSDK_ERROR("No MDIO node found in DTS!\n");
		return 1;
	}

	mdio_plat = of_find_device_by_node(mdio_node);
	if (!mdio_plat) {
		SSDK_ERROR("cannot find platform device from mdio node\n");
		return 1;
	}

	if(reg_mode == HSL_REG_LOCAL_BUS)
	{
		mdio_data = dev_get_drvdata(&mdio_plat->dev);
		if (!mdio_data) {
			SSDK_ERROR("cannot get mdio_data reference from device data\n");
			return 1;
		}
		priv->miibus = mdio_data->mii_bus;
	}
	else
		priv->miibus = dev_get_drvdata(&mdio_plat->dev);

	if (!priv->miibus) {
		SSDK_ERROR("cannot get mii bus reference from device data\n");
		return 1;
	}
	return 0;
}
int
ssdk_plat_init(ssdk_init_cfg *cfg, a_uint32_t dev_id)
{
	printk("ssdk_plat_init start\n");
		if(miibus_get(dev_id))
			return -ENODEV;

	return 0;
}

void
ssdk_plat_exit(a_uint32_t dev_id)
{
	printk("ssdk_plat_exit\n");
}
