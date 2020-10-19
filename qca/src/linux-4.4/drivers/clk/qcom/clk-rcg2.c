/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/bug.h>
#include <linux/export.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/math64.h>

#include <asm/div64.h>

#include "clk-rcg.h"
#include "common.h"

#define CMD_REG			0x0
#define CMD_UPDATE		BIT(0)
#define CMD_ROOT_EN		BIT(1)
#define CMD_DIRTY_CFG		BIT(4)
#define CMD_DIRTY_N		BIT(5)
#define CMD_DIRTY_M		BIT(6)
#define CMD_DIRTY_D		BIT(7)
#define CMD_ROOT_OFF		BIT(31)

#define CFG_REG			0x4
#define CFG_SRC_DIV_SHIFT	0
#define CFG_SRC_SEL_SHIFT	8
#define CFG_SRC_SEL_MASK	(0x7 << CFG_SRC_SEL_SHIFT)
#define CFG_MODE_SHIFT		12
#define CFG_MODE_MASK		(0x3 << CFG_MODE_SHIFT)
#define CFG_MODE_DUAL_EDGE	(0x2 << CFG_MODE_SHIFT)

#define M_REG			0x8
#define N_REG			0xc
#define D_REG			0x10
#define FEPLL_500_SRC		0x2

#define RCG2_CMD(rcg)	(rcg->cmd_rcgr + CMD_REG)
#define RCG2_CFG(rcg)	(rcg->cmd_rcgr + CFG_REG + rcg->cfg_offset)
#define RCG2_M(rcg)	(rcg->cmd_rcgr + M_REG + rcg->cfg_offset)
#define RCG2_N(rcg)	(rcg->cmd_rcgr + N_REG + rcg->cfg_offset)
#define RCG2_D(rcg)	(rcg->cmd_rcgr + D_REG + rcg->cfg_offset)

static int clk_rcg2_is_enabled(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	u32 cmd;
	int ret;

	ret = regmap_read(rcg->clkr.regmap, RCG2_CMD(rcg), &cmd);
	if (ret)
		return ret;

	return (cmd & CMD_ROOT_OFF) == 0;
}

static u8 clk_rcg2_get_parent(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	int num_parents = clk_hw_get_num_parents(hw);
	u32 cfg;
	int i, ret;

	ret = regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg), &cfg);
	if (ret)
		goto err;

	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i].cfg)
			return i;

err:
	pr_debug("%s: Clock %s has invalid parent, using default.\n",
		 __func__, clk_hw_get_name(hw));
	return 0;
}

static int update_config(struct clk_rcg2 *rcg, bool check_update_clear)
{
	int count, ret;
	u32 cmd;
	struct clk_hw *hw = &rcg->clkr.hw;
	const char *name = clk_hw_get_name(hw);

	ret = regmap_update_bits(rcg->clkr.regmap, RCG2_CMD(rcg),
				 CMD_UPDATE, CMD_UPDATE);
	if (ret)
		return ret;

	if (!check_update_clear)
		return 0;

	/* Wait for update to take effect */
	for (count = 500; count > 0; count--) {
		ret = regmap_read(rcg->clkr.regmap, RCG2_CMD(rcg), &cmd);
		if (ret)
			return ret;
		if (!(cmd & CMD_UPDATE))
			return 0;
		udelay(1);
	}

	WARN(1, "%s: rcg didn't update its configuration.", name);
	return 0;
}

static int clk_rcg2_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	int ret;
	bool check_update_clear = true;
	u32 cfg = rcg->parent_map[index].cfg << CFG_SRC_SEL_SHIFT;

	if ((rcg->flags & CLK_RCG2_HW_CONTROLLED) &&
	    !clk_hw_is_enabled(clk_hw_get_parent_by_index(hw, index)))
		check_update_clear = false;

	ret = regmap_update_bits(rcg->clkr.regmap, RCG2_CFG(rcg),
				 CFG_SRC_SEL_MASK, cfg);
	if (ret)
		return ret;

	return update_config(rcg, check_update_clear);
}

/*
 * Calculate m/n:d rate
 *
 *          parent_rate     m
 *   rate = ----------- x  ---
 *            hid_div       n
 */
static unsigned long
calc_rate(unsigned long parent_rate, u32 m, u32 n, u32 mode, u32 hid_div)
{
	u64 rate = parent_rate;

	if (hid_div) {
		rate *= 2;
		do_div(rate, hid_div + 1);
	}

	if (mode) {
		rate *= m;
		do_div(rate, n);
	}

	return rate;
}

static unsigned long
clk_rcg2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	u32 cfg, hid_div, m = 0, n = 0, mode = 0, mask;

	regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg), &cfg);

	if (rcg->mnd_width) {
		mask = BIT(rcg->mnd_width) - 1;
		regmap_read(rcg->clkr.regmap, RCG2_M(rcg), &m);
		m &= mask;
		regmap_read(rcg->clkr.regmap, RCG2_N(rcg), &n);
		n =  ~n;
		n &= mask;
		n += m;
		mode = cfg & CFG_MODE_MASK;
		mode >>= CFG_MODE_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	hid_div = cfg >> CFG_SRC_DIV_SHIFT;
	hid_div &= mask;

	return calc_rate(parent_rate, m, n, mode, hid_div);
}

static const struct freq_tbl *
clk_rcg2_find_best_freq(struct clk_hw *hw, const struct freq_tbl *f,
			unsigned long rate)
{
	unsigned long req_rate = rate, best = 0, freq;
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	int index;
	u64 tmp;
	const struct freq_tbl *best_ftable = NULL;

	f = qcom_find_freq(f, rate);

	/*
	 * Check for duplicate frequencies in frequency table if
	 * CLK_SET_RATE_PARENT flag is not set
	 */
	if (!f || (clk_hw_get_flags(hw) & CLK_SET_RATE_PARENT) ||
	    ((f->freq && (f + 1)->freq != f->freq)))
		return f;

	/*
	 * Check for all the duplicate entries in frequency table and
	 * calculate the actual rate from current parent rate with each
	 * entries pre_div, m and n values. The entry, which gives the
	 * minimum difference in requested rate and actual rate, will be
	 * selected as the best one.
	 */
	for (freq = f->freq; freq == f->freq; f++) {
		index = qcom_find_src_index(hw, rcg->parent_map, f->src);
		if (index < 0)
			continue;

		rate =  clk_hw_get_rate(clk_hw_get_parent_by_index(hw, index));
		if (rcg->hid_width && f->pre_div) {
			rate *= 2;
			rate /= f->pre_div + 1;
		}

		if (rcg->mnd_width && f->n) {
			tmp = rate;
			tmp = tmp * f->n;
			do_div(tmp, f->m);
			rate = tmp;
		}

		if (abs(req_rate - rate) < abs(best - rate)) {
			best_ftable = f;
			best = rate;

			if (req_rate == rate)
				break;
		}
	}

	return best_ftable;
}

static int _freq_tbl_determine_rate(struct clk_hw *hw,
		const struct freq_tbl *f, struct clk_rate_request *req)
{
	unsigned long clk_flags, rate = req->rate;
	struct clk_hw *p;
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	int index;

	f = clk_rcg2_find_best_freq(hw, f, rate);
	if (!f)
		return -EINVAL;

	index = qcom_find_src_index(hw, rcg->parent_map, f->src);
	if (index < 0)
		return index;

	clk_flags = clk_hw_get_flags(hw);
	p = clk_hw_get_parent_by_index(hw, index);
	if (clk_flags & CLK_SET_RATE_PARENT) {
		if (f->pre_div) {
			rate /= 2;
			rate *= f->pre_div + 1;
		}

		if (f->n) {
			u64 tmp = rate;
			tmp = tmp * f->n;
			do_div(tmp, f->m);
			rate = tmp;
		}
	} else {
		rate =  clk_hw_get_rate(p);
	}
	req->best_parent_hw = p;
	req->best_parent_rate = rate;
	req->rate = f->freq;

	return 0;
}

static int clk_rcg2_determine_rate(struct clk_hw *hw,
				   struct clk_rate_request *req)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	return _freq_tbl_determine_rate(hw, rcg->freq_tbl, req);
}

static int clk_rcg2_configure(struct clk_rcg2 *rcg, const struct freq_tbl *f)
{
	u32 cfg, mask;
	struct clk_hw *hw = &rcg->clkr.hw;
	bool check_update_clear = true;
	int ret, index = qcom_find_src_index(hw, rcg->parent_map, f->src);

	if (index < 0)
		return index;

	if (rcg->mnd_width && f->n) {
		mask = BIT(rcg->mnd_width) - 1;
		ret = regmap_update_bits(rcg->clkr.regmap,
				RCG2_M(rcg), mask, f->m);
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				RCG2_N(rcg), mask, ~(f->n - f->m));
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				RCG2_D(rcg), mask, ~f->n);
		if (ret)
			return ret;
	}

	mask = BIT(rcg->hid_width) - 1;
	mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
	cfg = f->pre_div << CFG_SRC_DIV_SHIFT;
	cfg |= rcg->parent_map[index].cfg << CFG_SRC_SEL_SHIFT;
	if (rcg->mnd_width && f->n && (f->m != f->n))
		cfg |= CFG_MODE_DUAL_EDGE;
	ret = regmap_update_bits(rcg->clkr.regmap,
			RCG2_CFG(rcg), mask, cfg);
	if (ret)
		return ret;

	if ((rcg->flags & CLK_RCG2_HW_CONTROLLED) &&
	    !clk_hw_is_enabled(clk_hw_get_parent_by_index(hw, index)))
		check_update_clear = false;

	return update_config(rcg, check_update_clear);
}

static int __clk_rcg2_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const struct freq_tbl *f;

	f = clk_rcg2_find_best_freq(hw, rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	return clk_rcg2_configure(rcg, f);
}

static int clk_rcg2_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_rcg2_set_rate(hw, rate);
}

static int clk_rcg2_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_rcg2_set_rate(hw, rate);
}

const struct clk_ops clk_rcg2_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.determine_rate = clk_rcg2_determine_rate,
	.set_rate = clk_rcg2_set_rate,
	.set_rate_and_parent = clk_rcg2_set_rate_and_parent,
};
EXPORT_SYMBOL_GPL(clk_rcg2_ops);

static int clk_rcg2_shared_force_enable(struct clk_hw *hw, unsigned long rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const char *name = clk_hw_get_name(hw);
	int ret, count;

	/* force enable RCG */
	ret = regmap_update_bits(rcg->clkr.regmap, RCG2_CMD(rcg),
				 CMD_ROOT_EN, CMD_ROOT_EN);
	if (ret)
		return ret;

	/* wait for RCG to turn ON */
	for (count = 500; count > 0; count--) {
		ret = clk_rcg2_is_enabled(hw);
		if (ret)
			break;
		udelay(1);
	}
	if (!count)
		pr_err("%s: RCG did not turn on\n", name);

	/* set clock rate */
	ret = __clk_rcg2_set_rate(hw, rate);
	if (ret)
		return ret;

	/* clear force enable RCG */
	return regmap_update_bits(rcg->clkr.regmap, RCG2_CMD(rcg),
				 CMD_ROOT_EN, 0);
}

static int clk_rcg2_shared_set_rate(struct clk_hw *hw, unsigned long rate,
				    unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	/* cache the rate */
	rcg->current_freq = rate;

	if (!__clk_is_enabled(hw->clk))
		return 0;

	return clk_rcg2_shared_force_enable(hw, rcg->current_freq);
}

static unsigned long
clk_rcg2_shared_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	return rcg->current_freq = clk_rcg2_recalc_rate(hw, parent_rate);
}

static int clk_rcg2_shared_enable(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	return clk_rcg2_shared_force_enable(hw, rcg->current_freq);
}

static void clk_rcg2_shared_disable(struct clk_hw *hw)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);

	/* switch to XO, which is the lowest entry in the freq table */
	clk_rcg2_shared_set_rate(hw, rcg->freq_tbl[0].freq, 0);
}

const struct clk_ops clk_rcg2_shared_ops = {
	.enable = clk_rcg2_shared_enable,
	.disable = clk_rcg2_shared_disable,
	.get_parent = clk_rcg2_get_parent,
	.recalc_rate = clk_rcg2_shared_recalc_rate,
	.determine_rate = clk_rcg2_determine_rate,
	.set_rate = clk_rcg2_shared_set_rate,
};
EXPORT_SYMBOL_GPL(clk_rcg2_shared_ops);

struct frac_entry {
	int num;
	int den;
};

static const struct frac_entry frac_table_675m[] = {	/* link rate of 270M */
	{ 52, 295 },	/* 119 M */
	{ 11, 57 },	/* 130.25 M */
	{ 63, 307 },	/* 138.50 M */
	{ 11, 50 },	/* 148.50 M */
	{ 47, 206 },	/* 154 M */
	{ 31, 100 },	/* 205.25 M */
	{ 107, 269 },	/* 268.50 M */
	{ },
};

static struct frac_entry frac_table_810m[] = { /* Link rate of 162M */
	{ 31, 211 },	/* 119 M */
	{ 32, 199 },	/* 130.25 M */
	{ 63, 307 },	/* 138.50 M */
	{ 11, 60 },	/* 148.50 M */
	{ 50, 263 },	/* 154 M */
	{ 31, 120 },	/* 205.25 M */
	{ 119, 359 },	/* 268.50 M */
	{ },
};

static int clk_edp_pixel_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	struct freq_tbl f = *rcg->freq_tbl;
	const struct frac_entry *frac;
	int delta = 100000;
	s64 src_rate = parent_rate;
	s64 request;
	u32 mask = BIT(rcg->hid_width) - 1;
	u32 hid_div;

	if (src_rate == 810000000)
		frac = frac_table_810m;
	else
		frac = frac_table_675m;

	for (; frac->num; frac++) {
		request = rate;
		request *= frac->den;
		request = div_s64(request, frac->num);
		if ((src_rate < (request - delta)) ||
		    (src_rate > (request + delta)))
			continue;

		regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg),
				&hid_div);
		f.pre_div = hid_div;
		f.pre_div >>= CFG_SRC_DIV_SHIFT;
		f.pre_div &= mask;
		f.m = frac->num;
		f.n = frac->den;

		return clk_rcg2_configure(rcg, &f);
	}

	return -EINVAL;
}

static int clk_edp_pixel_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	/* Parent index is set statically in frequency table */
	return clk_edp_pixel_set_rate(hw, rate, parent_rate);
}

static int clk_edp_pixel_determine_rate(struct clk_hw *hw,
					struct clk_rate_request *req)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const struct freq_tbl *f = rcg->freq_tbl;
	const struct frac_entry *frac;
	int delta = 100000;
	s64 request;
	u32 mask = BIT(rcg->hid_width) - 1;
	u32 hid_div;
	int index = qcom_find_src_index(hw, rcg->parent_map, f->src);

	/* Force the correct parent */
	req->best_parent_hw = clk_hw_get_parent_by_index(hw, index);
	req->best_parent_rate = clk_hw_get_rate(req->best_parent_hw);

	if (req->best_parent_rate == 810000000)
		frac = frac_table_810m;
	else
		frac = frac_table_675m;

	for (; frac->num; frac++) {
		request = req->rate;
		request *= frac->den;
		request = div_s64(request, frac->num);
		if ((req->best_parent_rate < (request - delta)) ||
		    (req->best_parent_rate > (request + delta)))
			continue;

		regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg),
				&hid_div);
		hid_div >>= CFG_SRC_DIV_SHIFT;
		hid_div &= mask;

		req->rate = calc_rate(req->best_parent_rate,
				      frac->num, frac->den,
				      !!frac->den, hid_div);
		return 0;
	}

	return -EINVAL;
}

const struct clk_ops clk_edp_pixel_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.set_rate = clk_edp_pixel_set_rate,
	.set_rate_and_parent = clk_edp_pixel_set_rate_and_parent,
	.determine_rate = clk_edp_pixel_determine_rate,
};
EXPORT_SYMBOL_GPL(clk_edp_pixel_ops);

static int clk_byte_determine_rate(struct clk_hw *hw,
				   struct clk_rate_request *req)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	const struct freq_tbl *f = rcg->freq_tbl;
	int index = qcom_find_src_index(hw, rcg->parent_map, f->src);
	unsigned long parent_rate, div;
	u32 mask = BIT(rcg->hid_width) - 1;
	struct clk_hw *p;

	if (req->rate == 0)
		return -EINVAL;

	req->best_parent_hw = p = clk_hw_get_parent_by_index(hw, index);
	req->best_parent_rate = parent_rate = clk_hw_round_rate(p, req->rate);

	div = DIV_ROUND_UP((2 * parent_rate), req->rate) - 1;
	div = min_t(u32, div, mask);

	req->rate = calc_rate(parent_rate, 0, 0, 0, div);

	return 0;
}

static int clk_byte_set_rate(struct clk_hw *hw, unsigned long rate,
			 unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	struct freq_tbl f = *rcg->freq_tbl;
	unsigned long div;
	u32 mask = BIT(rcg->hid_width) - 1;

	div = DIV_ROUND_UP((2 * parent_rate), rate) - 1;
	div = min_t(u32, div, mask);

	f.pre_div = div;

	return clk_rcg2_configure(rcg, &f);
}

static int clk_byte_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	/* Parent index is set statically in frequency table */
	return clk_byte_set_rate(hw, rate, parent_rate);
}

const struct clk_ops clk_byte_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.set_rate = clk_byte_set_rate,
	.set_rate_and_parent = clk_byte_set_rate_and_parent,
	.determine_rate = clk_byte_determine_rate,
};
EXPORT_SYMBOL_GPL(clk_byte_ops);

static int clk_byte2_determine_rate(struct clk_hw *hw,
				    struct clk_rate_request *req)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	unsigned long parent_rate, div;
	u32 mask = BIT(rcg->hid_width) - 1;
	struct clk_hw *p;
	unsigned long rate = req->rate;

	if (rate == 0)
		return -EINVAL;

	p = req->best_parent_hw;
	req->best_parent_rate = parent_rate = clk_hw_round_rate(p, rate);

	div = DIV_ROUND_UP((2 * parent_rate), rate) - 1;
	div = min_t(u32, div, mask);

	req->rate = calc_rate(parent_rate, 0, 0, 0, div);

	return 0;
}

static int clk_byte2_set_rate(struct clk_hw *hw, unsigned long rate,
			 unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	struct freq_tbl f = { 0 };
	unsigned long div;
	int i, num_parents = clk_hw_get_num_parents(hw);
	u32 mask = BIT(rcg->hid_width) - 1;
	u32 cfg;

	div = DIV_ROUND_UP((2 * parent_rate), rate) - 1;
	div = min_t(u32, div, mask);

	f.pre_div = div;

	regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg), &cfg);
	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++) {
		if (cfg == rcg->parent_map[i].cfg) {
			f.src = rcg->parent_map[i].src;
			return clk_rcg2_configure(rcg, &f);
		}
	}

	return -EINVAL;
}

static int clk_byte2_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	/* Read the hardware to determine parent during set_rate */
	return clk_byte2_set_rate(hw, rate, parent_rate);
}

const struct clk_ops clk_byte2_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.set_rate = clk_byte2_set_rate,
	.set_rate_and_parent = clk_byte2_set_rate_and_parent,
	.determine_rate = clk_byte2_determine_rate,
};
EXPORT_SYMBOL_GPL(clk_byte2_ops);

static const struct frac_entry frac_table_pixel[] = {
	{ 3, 8 },
	{ 2, 9 },
	{ 4, 9 },
	{ 1, 1 },
	{ }
};

static int clk_pixel_determine_rate(struct clk_hw *hw,
				    struct clk_rate_request *req)
{
	unsigned long request, src_rate;
	int delta = 100000;
	const struct frac_entry *frac = frac_table_pixel;

	for (; frac->num; frac++) {
		request = (req->rate * frac->den) / frac->num;

		src_rate = clk_hw_round_rate(req->best_parent_hw, request);
		if ((src_rate < (request - delta)) ||
			(src_rate > (request + delta)))
			continue;

		req->best_parent_rate = src_rate;
		req->rate = (src_rate * frac->num) / frac->den;
		return 0;
	}

	return -EINVAL;
}

static int clk_pixel_set_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate)
{
	struct clk_rcg2 *rcg = to_clk_rcg2(hw);
	struct freq_tbl f = { 0 };
	const struct frac_entry *frac = frac_table_pixel;
	unsigned long request;
	int delta = 100000;
	u32 mask = BIT(rcg->hid_width) - 1;
	u32 hid_div, cfg;
	int i, num_parents = clk_hw_get_num_parents(hw);

	regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg), &cfg);
	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i].cfg) {
			f.src = rcg->parent_map[i].src;
			break;
		}

	for (; frac->num; frac++) {
		request = (rate * frac->den) / frac->num;

		if ((parent_rate < (request - delta)) ||
			(parent_rate > (request + delta)))
			continue;

		regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg),
				&hid_div);
		f.pre_div = hid_div;
		f.pre_div >>= CFG_SRC_DIV_SHIFT;
		f.pre_div &= mask;
		f.m = frac->num;
		f.n = frac->den;

		return clk_rcg2_configure(rcg, &f);
	}
	return -EINVAL;
}

static int clk_pixel_set_rate_and_parent(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate, u8 index)
{
	return clk_pixel_set_rate(hw, rate, parent_rate);
}

const struct clk_ops clk_pixel_ops = {
	.is_enabled = clk_rcg2_is_enabled,
	.get_parent = clk_rcg2_get_parent,
	.set_parent = clk_rcg2_set_parent,
	.recalc_rate = clk_rcg2_recalc_rate,
	.set_rate = clk_pixel_set_rate,
	.set_rate_and_parent = clk_pixel_set_rate_and_parent,
	.determine_rate = clk_pixel_determine_rate,
};
EXPORT_SYMBOL_GPL(clk_pixel_ops);

static int clk_cdiv_rcg2_is_enabled(struct clk_hw *hw)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	u32 cmd = 0;
	int ret;

	ret = regmap_read(rcg->clkr.regmap, RCG2_CMD(rcg), &cmd);
	if (ret < 0)
		return false;

	return (cmd & CMD_ROOT_OFF) == 0;
}

static u8 clk_cdiv_rcg2_get_parent(struct clk_hw *hw)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	int num_parents = clk_hw_get_num_parents(hw);
	u32 cfg;
	int i, ret;

	ret = regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg), &cfg);
	if (ret)
		goto err;

	cfg &= CFG_SRC_SEL_MASK;
	cfg >>= CFG_SRC_SEL_SHIFT;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i].cfg)
			return i;
err:
	pr_debug("%s: Cannot find parent of %s clock, using default.\n",
			__func__, __clk_get_name(hw->clk));
	return 0;
}

static int cdiv_update_config(struct clk_cdiv_rcg2 *rcg)
{
	int count, ret;
	struct clk_hw *hw = &rcg->clkr.hw;
	const char *name = __clk_get_name(hw->clk);

	ret = regmap_update_bits(rcg->clkr.regmap, RCG2_CMD(rcg),
				 CMD_UPDATE, CMD_UPDATE);
	if (ret)
		return ret;

	/* Wait for update to take effect */
	for (count = 500; count > 0; count--) {
		u32 cmd = ~0U;

		/* ignore regmap errors - until we exhaust retry count. */
		ret = regmap_read(rcg->clkr.regmap, RCG2_CMD(rcg),
									&cmd);

		if (ret >= 0 && !(cmd & CMD_UPDATE))
			return 0;

		udelay(1);
	}

	WARN(ret, "%s: rcg didn't update its configuration.", name);
	return ret ? ret : -ETIMEDOUT;
}

static int clk_cdiv_rcg2_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	int ret;

	ret = regmap_update_bits(rcg->clkr.regmap, RCG2_CFG(rcg),
				 CFG_SRC_SEL_MASK,
				 rcg->parent_map[index].cfg <<
				 CFG_SRC_SEL_SHIFT);
	if (ret)
		return ret;

	return cdiv_update_config(rcg);
}

static unsigned long
clk_cdiv_rcg2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	u32 cfg, hid_div, m = 0, n = 0, mode = 0, mask, rate, cdiv;
	int ret;

	ret = regmap_read(rcg->clkr.regmap, RCG2_CFG(rcg), &cfg);
	if (ret)
		return 0UL;

	if (rcg->mnd_width) {
		mask = BIT(rcg->mnd_width) - 1;
		ret = regmap_read(rcg->clkr.regmap, RCG2_M(rcg), &m);
		if (ret)
			return 0UL;

		m &= mask;
		ret = regmap_read(rcg->clkr.regmap, RCG2_N(rcg), &n);
		if (ret)
			return 0UL;

		n =  ~n;
		n &= mask;

		n += m;
		mode = cfg & CFG_MODE_MASK;
		mode >>= CFG_MODE_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	hid_div = cfg >> CFG_SRC_DIV_SHIFT;
	hid_div &= mask;
	rate = calc_rate(parent_rate, m, n, mode, hid_div);

	ret = regmap_read(rcg->clkr.regmap, rcg->cdiv.offset, &cdiv);
	if (ret)
		return 0UL;

	cdiv &= (rcg->cdiv.mask << rcg->cdiv.shift);
	cdiv =  (cdiv >> rcg->cdiv.shift);
	if (cdiv)
		rate *= cdiv + 1;
	return rate;
}

static int _cdiv_rcg2_freq_tbl_determine_rate(struct clk_hw *hw,
		const struct freq_tbl *f,
		struct clk_rate_request *req)
{
	unsigned long clk_flags;
	struct clk_hw *p_hw;
	unsigned long rate = req->rate;

	f = clk_rcg2_find_best_freq(hw, f, rate);
	if (!f)
		return 0L;

	clk_flags = __clk_get_flags(hw->clk);
	p_hw = clk_hw_get_parent_by_index(hw, f->src);
	if (clk_flags & CLK_SET_RATE_PARENT) {
		if (f->pre_div)
			rate *= f->pre_div;
		if (f->n) {
			u64 tmp = rate;

			tmp = tmp * f->n;
			do_div(tmp, f->m);
			rate = tmp;
		}
	} else {
		rate =	clk_hw_get_rate(p_hw);
	}

	req->best_parent_rate = rate;
	req->best_parent_hw = p_hw;
	req->rate = f->freq;

	return 0;
}


static int clk_cdiv_rcg2_determine_rate(struct clk_hw *hw,
					struct clk_rate_request *req)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);

	return _cdiv_rcg2_freq_tbl_determine_rate(hw, rcg->freq_tbl, req);
}

static int clk_cdiv_rcg2_configure(struct clk_cdiv_rcg2 *rcg,
				const struct freq_tbl *f)
{
	u32 cfg = 0, mask;
	u32 i;
	int ret;

	if (rcg->mnd_width && f->n) {
		mask = BIT(rcg->mnd_width) - 1;
		ret = regmap_update_bits(rcg->clkr.regmap,
				RCG2_M(rcg), mask, f->m);
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				RCG2_N(rcg), mask, ~(f->n - f->m));
		if (ret)
			return ret;

		ret = regmap_update_bits(rcg->clkr.regmap,
				RCG2_D(rcg), mask, ~f->n);
		if (ret)
			return ret;
	}


	if (rcg->cdiv.mask && f->pre_div > 16) {

		/* The division is handled by two dividers. Both of which can
		 * divide by a maximum value of 16. To achieve a division of
		 * 256 = 16 * 16, we use a divider of 16 in the RCGR and the
		 * other divider of 16 in the MISC Register.
		 */
		for (i = 2; i <= 16; i++) {
			if (f->pre_div % i == 0)
				cfg = i;
		}

		if (cfg == 0) {
			pr_err("%s: invalid divider\n", __func__);
			return -EINVAL;
		}

		if (f->pre_div/cfg > 16)
			return -EINVAL;
		mask = (rcg->cdiv.mask)<<rcg->cdiv.shift;
		ret = regmap_update_bits(rcg->clkr.regmap,
					rcg->cdiv.offset, mask,
				((cfg - 1) << rcg->cdiv.shift) & mask);
		if (ret)
			return ret;
		cfg = (2 * (f->pre_div / cfg)) - 1;
	} else {
		ret = regmap_write(rcg->clkr.regmap, rcg->cdiv.offset, 0x0);
		if (ret)
			return ret;
		cfg = ((2 * f->pre_div) - 1) << CFG_SRC_DIV_SHIFT;
	}

	mask = BIT(rcg->hid_width) - 1;
	mask |= CFG_SRC_SEL_MASK | CFG_MODE_MASK;
	cfg |= rcg->parent_map[f->src].cfg << CFG_SRC_SEL_SHIFT;
	if (rcg->mnd_width && f->n)
		cfg |= CFG_MODE_DUAL_EDGE;
	ret = regmap_update_bits(rcg->clkr.regmap,
			RCG2_CFG(rcg), mask, cfg);
	if (ret)
		return ret;

	return cdiv_update_config(rcg);
}

static int __clk_cdiv_rcg2_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_cdiv_rcg2 *rcg = to_clk_cdiv_rcg2(hw);
	const struct freq_tbl *f;

	f = clk_rcg2_find_best_freq(hw, rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	return clk_cdiv_rcg2_configure(rcg, f);
}

static int clk_cdiv_rcg2_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_cdiv_rcg2_set_rate(hw, rate);
}

static int clk_cdiv_rcg2_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_cdiv_rcg2_set_rate(hw, rate);
}

const struct clk_ops clk_cdiv_rcg2_ops = {
	.is_enabled			= clk_cdiv_rcg2_is_enabled,
	.get_parent			= clk_cdiv_rcg2_get_parent,
	.set_parent			= clk_cdiv_rcg2_set_parent,
	.recalc_rate			= clk_cdiv_rcg2_recalc_rate,
	.determine_rate			= clk_cdiv_rcg2_determine_rate,
	.set_rate			= clk_cdiv_rcg2_set_rate,
	.set_rate_and_parent		= clk_cdiv_rcg2_set_rate_and_parent,
};
EXPORT_SYMBOL_GPL(clk_cdiv_rcg2_ops);

static int clk_muxr_is_enabled(struct clk_hw *hw)
{
	return 0;
}

static u8 clk_muxr_get_parent(struct clk_hw *hw)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	int num_parents = clk_hw_get_num_parents(hw);
	u32 cfg;
	int i, ret;

	ret = regmap_read(rcg->clkr.regmap, rcg->muxr.offset, &cfg);
	if (ret)
		goto err;

	cfg >>= rcg->muxr.shift;
	cfg &= rcg->muxr.mask;

	for (i = 0; i < num_parents; i++)
		if (cfg == rcg->parent_map[i].cfg)
			return i;

err:
	pr_debug("%s: Cannot find parent of %s clock, using default.\n",
			__func__, __clk_get_name(hw->clk));
	return 0;
}

static int clk_muxr_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	int ret;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->muxr.offset,
				 (rcg->muxr.mask<<rcg->muxr.shift),
				 rcg->parent_map[index].cfg << rcg->muxr.shift);
	if (ret)
		return ret;

	return 0;
}

static unsigned long
clk_muxr_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	u32 misc;
	int ret;

	ret = regmap_read(rcg->clkr.regmap, rcg->misc.offset, &misc);
	if (ret)
		return 0UL;

	misc &= rcg->misc.mask;
	misc >>= rcg->misc.shift;

	return parent_rate * (misc + 1);
}

static int clk_muxr_determine_rate(struct clk_hw *hw,
				   struct clk_rate_request *req)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	const struct freq_tbl *f;
	unsigned long clk_flags;
	unsigned long rate = req->rate;
	struct clk_hw *p_hw;

	f = clk_rcg2_find_best_freq(hw, rcg->freq_tbl, rate);
	if (!f)
		return 0L;

	clk_flags = __clk_get_flags(hw->clk);
	p_hw = clk_hw_get_parent_by_index(hw, f->src);
	if (clk_flags & CLK_SET_RATE_PARENT) {
		if (f->pre_div)
			rate *= f->pre_div;
	} else {
		rate =	clk_hw_get_rate(p_hw);
	}

	req->best_parent_rate = rate;
	req->best_parent_hw = p_hw;
	req->rate = f->freq;

	return 0;
}

static int __clk_muxr_set_rate(struct clk_hw *hw, unsigned long rate)
{
	struct clk_muxr_misc *rcg = to_clk_muxr_misc(hw);
	const struct freq_tbl *f;
	int ret;

	f = clk_rcg2_find_best_freq(hw, rcg->freq_tbl, rate);
	if (!f)
		return -EINVAL;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->muxr.offset,
				rcg->muxr.mask << rcg->muxr.shift,
				rcg->parent_map[f->src].cfg << rcg->muxr.shift);
	if (ret)
		return ret;

	ret = regmap_update_bits(rcg->clkr.regmap, rcg->misc.offset,
				rcg->misc.mask << rcg->misc.shift,
				(f->pre_div - 1) << rcg->misc.shift);
	return ret;
}

static int clk_muxr_set_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	return __clk_muxr_set_rate(hw, rate);
}

static int clk_muxr_set_rate_and_parent(struct clk_hw *hw,
		unsigned long rate, unsigned long parent_rate, u8 index)
{
	return __clk_muxr_set_rate(hw, rate);
}

const struct clk_ops clk_muxr_misc_ops = {
	.is_enabled	=	clk_muxr_is_enabled,
	.get_parent	=	clk_muxr_get_parent,
	.set_parent	=	clk_muxr_set_parent,
	.recalc_rate	=	clk_muxr_recalc_rate,
	.determine_rate	=	clk_muxr_determine_rate,
	.set_rate	=	clk_muxr_set_rate,
	.set_rate_and_parent	=	clk_muxr_set_rate_and_parent,
};
EXPORT_SYMBOL_GPL(clk_muxr_misc_ops);
