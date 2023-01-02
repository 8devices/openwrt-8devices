/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/qcom_scm.h>

#define MEM_NOC_XM_APP0_QOSGEN_MAINCTL_LOW 0x6088
#define MEM_NOC_XM_APP0_QOSGEN_REGUL0CTL_LOW 0x60C0
#define MEM_NOC_XM_APP0_QOSGEN_REGUL0BW_LOW 0x60C8
#define MAINCTL_LOW_VAL 0x70
#define REGUL0CTL_LOW_VAL 0x7703
#define REGUL0BW_LOW_VAL 0x3FF0FFF

static int reg_update_probe(struct platform_device *pdev)
{
	struct resource *res;
	void __iomem *base;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "memnoc");
	if(res) {
		base = devm_ioremap_resource(&pdev->dev, res);
		if(IS_ERR(base))
			return PTR_ERR(base);
		writel(MAINCTL_LOW_VAL, base + MEM_NOC_XM_APP0_QOSGEN_MAINCTL_LOW);
		writel(REGUL0CTL_LOW_VAL, base + MEM_NOC_XM_APP0_QOSGEN_REGUL0CTL_LOW);
		writel(REGUL0BW_LOW_VAL, base + MEM_NOC_XM_APP0_QOSGEN_REGUL0BW_LOW);
	}

	return 0;
}

static const struct of_device_id reg_update_dt_match[] = {
	{ .compatible = "ipq,reg_update", },
	{ },
};

MODULE_DEVICE_TABLE(of, reg_update_dt_match);

static struct platform_driver reg_update_driver = {
	.driver = {
		.name	= "reg_update",
		.owner	= THIS_MODULE,
		.of_match_table	= reg_update_dt_match,
	},
	.probe = reg_update_probe,
};

module_platform_driver(reg_update_driver);
