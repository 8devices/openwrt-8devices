#include <linux/clkdev.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <bspchip.h>

struct clk {
	struct clk_lookup cl;
	unsigned long int rate;
};

unsigned long int cal_cpu_clock(void)
{
	int sys_hw_reg = REG32(BSP_HW_STRAP);

	switch((sys_hw_reg >> 15) & 0xf) {
		case 0x7:
			return 800000000;
		case 0xb:
			return 1000000000;
		default:
			return 600000000;
	}
}

void rtl819x_clk_add(const char *dev, unsigned long rate)
{
	struct clk *clk = kzalloc(sizeof(struct clk), GFP_KERNEL);

	if (!clk)
		panic("failed to add clock\n");

	clk->cl.dev_id = dev;
	clk->cl.clk = clk;

	clk->rate = rate;

	clkdev_add(&clk->cl);
}

static int __init rtl819x_clk_init(void)
{
	unsigned long int cpu_clk = cal_cpu_clock();
	rtl819x_clk_add("cpu_clk", cpu_clk);
	rtl819x_clk_add("dw_spi_mmio.1", BSP_DW_SPI_FREQ);
	rtl819x_clk_add("dw_spi_mmio.2", BSP_DW_SPI_FREQ);
	rtl819x_clk_add("spi-sheipa.0", BSP_SPIC_FREQ);

	rtl819x_clk_add("i2c_designware.1", BSP_DW_I2C_FREQ);
	rtl819x_clk_add("i2c_designware.2", BSP_DW_I2C_FREQ);

	return 0;
}
arch_initcall(rtl819x_clk_init);

/*
 * Linux clock API
 */
int clk_enable(struct clk *clk)
{
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	return clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);
