/*
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk-provider.h>
#include <linux/spinlock.h>

#include <asm/krait-l2-accessors.h>

#include "clk-krait.h"

/* Secondary and primary muxes share the same cp15 register */
static DEFINE_SPINLOCK(krait_clock_reg_lock);

#define LPL_SHIFT	8
static void __krait_mux_set_sel(struct krait_mux_clk *mux, int sel)
{
	unsigned long flags;
	u32 regval;

	spin_lock_irqsave(&krait_clock_reg_lock, flags);
	regval = krait_get_l2_indirect_reg(mux->offset);
	regval &= ~(mux->mask << mux->shift);
	regval |= (sel & mux->mask) << mux->shift;
	if (mux->lpl) {
		regval &= ~(mux->mask << (mux->shift + LPL_SHIFT));
		regval |= (sel & mux->mask) << (mux->shift + LPL_SHIFT);
	}
	krait_set_l2_indirect_reg(mux->offset, regval);
	spin_unlock_irqrestore(&krait_clock_reg_lock, flags);

	/* Wait for switch to complete. */
	mb();
	udelay(1);
}

static int krait_mux_set_parent(struct clk_hw *hw, u8 index)
{
	struct krait_mux_clk *mux = to_krait_mux_clk(hw);
	u32 sel;

	sel = clk_mux_reindex(index, mux->parent_map, 0);
	mux->en_mask = sel;
	/* Don't touch mux if CPU is off as it won't work */
	if (__clk_is_enabled(hw->clk))
		__krait_mux_set_sel(mux, sel);
	return 0;
}

static u8 krait_mux_get_parent(struct clk_hw *hw)
{
	struct krait_mux_clk *mux = to_krait_mux_clk(hw);
	u32 sel;

	sel = krait_get_l2_indirect_reg(mux->offset);
	sel >>= mux->shift;
	sel &= mux->mask;
	mux->en_mask = sel;

	return clk_mux_get_parent(hw, sel, mux->parent_map, 0);
}

static struct clk_hw *krait_mux_get_safe_parent(struct clk_hw *hw)
{
	int i;
	struct krait_mux_clk *mux = to_krait_mux_clk(hw);
	struct clk_hw *p;
	int num_parents = clk_hw_get_num_parents(hw);

	i = mux->safe_sel;
	for (i = 0; i < num_parents; i++)
		if (mux->safe_sel == mux->parent_map[i])
			break;
	p = clk_hw_get_parent_by_index(hw, i);

	return __clk_get_hw(p->clk);
}

static int krait_mux_enable(struct clk_hw *hw)
{
	struct krait_mux_clk *mux = to_krait_mux_clk(hw);

	__krait_mux_set_sel(mux, mux->en_mask);

	return 0;
}

static void krait_mux_disable(struct clk_hw *hw)
{
	struct krait_mux_clk *mux = to_krait_mux_clk(hw);

	__krait_mux_set_sel(mux, mux->safe_sel);
}

const struct clk_ops krait_mux_clk_ops = {
	.enable = krait_mux_enable,
	.disable = krait_mux_disable,
	.set_parent = krait_mux_set_parent,
	.get_parent = krait_mux_get_parent,
	.determine_rate = __clk_mux_determine_rate_closest,
	.get_safe_parent = krait_mux_get_safe_parent,
};
EXPORT_SYMBOL_GPL(krait_mux_clk_ops);

/* The divider can divide by 2, 4, 6 and 8. But we only really need div-2. */
static long krait_div2_round_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long *parent_rate)
{
	struct clk_hw *hw_parent = clk_hw_get_parent(hw);

	if (hw_parent) {
		*parent_rate = clk_hw_round_rate(hw_parent, rate * 2);
		return DIV_ROUND_UP(*parent_rate, 2);
	} else
		return -1;

}

static int krait_div2_set_rate(struct clk_hw *hw, unsigned long rate,
			unsigned long parent_rate)
{
	struct krait_div2_clk *d = to_krait_div2_clk(hw);
	unsigned long flags;
	u32 val;
	u32 mask = BIT(d->width) - 1;

	if (d->lpl)
		mask = mask << (d->shift + LPL_SHIFT) | mask << d->shift;

	spin_lock_irqsave(&krait_clock_reg_lock, flags);
	val = krait_get_l2_indirect_reg(d->offset);
	val &= ~mask;
	krait_set_l2_indirect_reg(d->offset, val);
	spin_unlock_irqrestore(&krait_clock_reg_lock, flags);

	return 0;
}

static unsigned long
krait_div2_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct krait_div2_clk *d = to_krait_div2_clk(hw);
	u32 mask = BIT(d->width) - 1;
	u32 div;

	div = krait_get_l2_indirect_reg(d->offset);
	div >>= d->shift;
	div &= mask;
	div = (div + 1) * 2;

	return DIV_ROUND_UP(parent_rate, div);
}

const struct clk_ops krait_div2_clk_ops = {
	.round_rate = krait_div2_round_rate,
	.set_rate = krait_div2_set_rate,
	.recalc_rate = krait_div2_recalc_rate,
};
EXPORT_SYMBOL_GPL(krait_div2_clk_ops);
