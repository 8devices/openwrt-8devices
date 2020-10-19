/*
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>

#include <linux/reset-controller.h>
#include <dt-bindings/clock/qca,apss-ipq6018.h>

#include "common.h"
#include "clk-regmap.h"
#include "clk-pll.h"
#include "clk-rcg.h"
#include "clk-branch.h"
#include "clk-alpha-pll.h"
#include "clk-regmap-divider.h"
#include "clk-regmap-mux.h"
#include "reset.h"

#define F(f, s, h, m, n) { (f), (s), (2 * (h) - 1), (m), (n) }

enum {
	P_XO,
	P_GPLL0,
	P_GPLL2,
	P_GPLL4,
	P_APSS_PLL_EARLY,
	P_APSS_PLL
};

static const u32 apss_pll_regs_offsets[] = {
	[ALPHA_PLL_MODE] = 0x00,
	[ALPHA_PLL_L_VAL] = 0x08,
	[ALPHA_PLL_ALPHA_VAL] = 0x10,
	[ALPHA_PLL_ALPHA_VAL_U] = 0xff,
	[ALPHA_PLL_USER_CTL] = 0x18,
	[ALPHA_PLL_USER_CTL_U] = 0xff,
	[ALPHA_PLL_CONFIG_CTL] = 0x20,
	[ALPHA_PLL_CONFIG_CTL_U] = 0x24,
	[ALPHA_PLL_TEST_CTL] = 0x30,
	[ALPHA_PLL_TEST_CTL_U] = 0x34,
	[ALPHA_PLL_STATUS] = 0x28,
};

static struct clk_alpha_pll apss_pll_early = {
	.offset = 0x5000,
	.regs_offsets = apss_pll_regs_offsets,
	.flags = SUPPORTS_16BIT_ALPHA | SUPPORTS_64BIT_CTL,
	.clkr = {
		.enable_reg = 0x5000,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "apss_pll_early",
			.parent_names = (const char *[]){
				"xo"
			},
			.num_parents = 1,
			.ops = &clk_alpha_pll_huayra_ops,
		},
	},
};

static struct clk_alpha_pll_postdiv apss_pll = {
	.offset = 0x5000,
	.regs_offsets = apss_pll_regs_offsets,
	.width = 2,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "apss_pll",
		.parent_names = (const char *[]){ "apss_pll_early" },
		.num_parents = 1,
		.ops = &clk_alpha_pll_postdiv_ro_ops,
	},
};

static const char * const parents_apcs_alias0_clk_src[] = {
	"xo",
	"gpll0",
	"gpll2",
	"gpll4",
	"apss_pll",
	"apss_pll_early",
};

static const struct parent_map parents_apcs_alias0_clk_src_map[] = {
	{ P_XO, 0 },
	{ P_GPLL0, 4 },
	{ P_GPLL2, 2 },
	{ P_GPLL4, 1 },
	{ P_APSS_PLL, 3 },
	{ P_APSS_PLL_EARLY, 5 },
};

static const struct freq_tbl ftbl_apcs_alias0_clk_src[] = {
	F(24000000, P_XO, 1, 0, 0),
	F(864000000, P_APSS_PLL_EARLY, 1, 0, 0),
	F(1056000000, P_APSS_PLL_EARLY, 1, 0, 0),
	F(1200000000, P_APSS_PLL_EARLY, 1, 0, 0),
	F(1320000000, P_APSS_PLL_EARLY, 1, 0, 0),
	F(1440000000, P_APSS_PLL_EARLY, 1, 0, 0),
	F(1488000000, P_APSS_PLL_EARLY, 1, 0, 0),
	F(1608000000, P_APSS_PLL_EARLY, 1, 0, 0),
	F(1800000000, P_APSS_PLL_EARLY, 1, 0, 0),
	{ }
};

static struct clk_rcg2 apcs_alias0_clk_src = {
	.cmd_rcgr = 0x0050,
	.freq_tbl = ftbl_apcs_alias0_clk_src,
	.hid_width = 5,
	.parent_map = parents_apcs_alias0_clk_src_map,
	.clkr.hw.init = &(struct clk_init_data){
		.name = "apcs_alias0_clk_src",
		.parent_names = parents_apcs_alias0_clk_src,
		.num_parents = 6,
		.ops = &clk_rcg2_ops,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_branch apcs_alias0_core_clk = {
	.halt_reg = 0x0058,
	.clkr = {
		.enable_reg = 0x0058,
		.enable_mask = BIT(0),
		.hw.init = &(struct clk_init_data){
			.name = "apcs_alias0_core_clk",
			.parent_names = (const char *[]){
				"apcs_alias0_clk_src"
			},
			.num_parents = 1,
			.flags = CLK_SET_RATE_PARENT |
				CLK_IS_CRITICAL,
			.ops = &clk_branch2_ops,
		},
	},
};

static struct clk_regmap *apss_ipq6018_clks[] = {
	[APSS_PLL_EARLY] = &apss_pll_early.clkr,
	[APSS_PLL] = &apss_pll.clkr,
	[APCS_ALIAS0_CLK_SRC] = &apcs_alias0_clk_src.clkr,
	[APCS_ALIAS0_CORE_CLK] = &apcs_alias0_core_clk.clkr,
};

static const struct alpha_pll_config apss_pll_config = {
	.l = 0x37,
	.config_ctl_val = 0x240D4828,
	.config_ctl_hi_val = 0x6,
	.early_output_mask = BIT(3),
	.aux2_output_mask = BIT(2),
	.aux_output_mask = BIT(1),
	.main_output_mask = BIT(0),
	.test_ctl_val = 0x1C0000C0,
	.test_ctl_hi_val = 0x4000,
};

static const struct of_device_id apss_ipq6018_match_table[] = {
	{ .compatible = "qcom,apss-ipq6018" },
	{ }
};
MODULE_DEVICE_TABLE(of, apss_ipq6018_match_table);

static const struct regmap_config apss_ipq6018_regmap_config = {
	.reg_bits       = 32,
	.reg_stride     = 4,
	.val_bits       = 32,
	.max_register   = 0x5ffc,
	.fast_io        = true,
};

static const struct qcom_cc_desc apss_ipq6018_desc = {
	.config = &apss_ipq6018_regmap_config,
	.clks = apss_ipq6018_clks,
	.num_clks = ARRAY_SIZE(apss_ipq6018_clks),
};

static int apss_ipq6018_probe(struct platform_device *pdev)
{
	int ret;
	struct regmap *regmap;

	regmap = qcom_cc_map(pdev, &apss_ipq6018_desc);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	clk_alpha_pll_configure(&apss_pll_early, regmap, &apss_pll_config);

	ret = qcom_cc_really_probe(pdev, &apss_ipq6018_desc, regmap);

	dev_dbg(&pdev->dev, "Registered ipq6018 apss clock provider\n");

	return ret;
}

static int apss_ipq6018_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver apss_ipq6018_driver = {
	.probe = apss_ipq6018_probe,
	.remove = apss_ipq6018_remove,
	.driver = {
		.name   = "qcom,apss-ipq6018",
		.owner  = THIS_MODULE,
		.of_match_table = apss_ipq6018_match_table,
	},
};

static int __init apss_ipq6018_init(void)
{
	return platform_driver_register(&apss_ipq6018_driver);
}
core_initcall(apss_ipq6018_init);

static void __exit apss_ipq6018_exit(void)
{
	platform_driver_unregister(&apss_ipq6018_driver);
}
module_exit(apss_ipq6018_exit);

MODULE_DESCRIPTION("QCA APSS IPQ6018 Driver");
MODULE_LICENSE("Dual BSD/GPLv2");
MODULE_ALIAS("platform:apss-ipq6018");
