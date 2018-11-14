#include <asm/mach-rtl8197f/bspchip.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/module.h>

struct clk {
	struct clk_lookup cl;
	unsigned long int rate;
};

unsigned long cal_cpu_clock(void)
{
	int sys_hw_reg = REG32(BSP_HW_STRAP);
	int sel;
	int div;


	sel = (sys_hw_reg >> BSP_CPUFREQ_SEL_OFFSET) & 0xf;
	div = (sys_hw_reg >> BSP_CPUFREQ_DIV_OFFSET) & 0x3;

	return (450000000 + sel * 50000000) >> div;
}
static struct clk_freq {
	unsigned long freq; //rate in KHz
	unsigned char sel;
	unsigned char div;
};

static struct clk_freq clk_freqs[] = {
	{ 1000000000, 0xb, 0 },
	{  950000000, 0xa, 0 },
	{  900000000, 0x9, 0 },
	{  850000000, 0x8, 0 },
	{  800000000, 0x7, 0 },
	{  750000000, 0x6, 0 },
	{  700000000, 0x5, 0 },
	{  650000000, 0x4, 0 },
	{  600000000, 0x3, 0 },
	{  550000000, 0x2, 0 },
	{  500000000, 0x1, 0 },
	{  450000000, 0x0, 0 },
	{  400000000, 0x7, 1 },
	{  350000000, 0x5, 1 },
	{  300000000, 0x3, 1 },
	{  250000000, 0x1, 1 },
	{  200000000, 0x7, 2 },
};

static int get_sel_div(unsigned long freq, unsigned char * sel, unsigned char * div)
{
	int i;


	for( i = 0; i < ARRAY_SIZE(clk_freqs); i++){
		if (freq == clk_freqs[i].freq){
			*sel = clk_freqs[i].sel;
			*div = clk_freqs[i].div;

			return 0;
		}
	}
	printk("FREQ not found\n");

	return -1;
};

extern void plat_time_init(void);
int SettingCPUClk(unsigned char clk_sel, unsigned char clk_div)
{
	unsigned int old_clk_sel;
	unsigned int mask;
	unsigned int sysreg;
	unsigned int old_sysreg;


	REG32(BSP_INIT_STATUS) = (1<<1); // clear cpu wakeup

	mask = REG32(BSP_GIMR); //backup
	REG32(BSP_GIMR) = 0;
	REG32(BSP_IRR3) = 0x2000;

	sysreg = REG32(BSP_HW_STRAP);
	old_sysreg = sysreg;
	old_clk_sel = (sysreg >> BSP_CPUFREQ_SEL_OFFSET) & 0x0f;

	sysreg &= ~(0x03 << BSP_CPUFREQ_DIV_OFFSET);
	sysreg &= ~(0x0f << BSP_CPUFREQ_SEL_OFFSET);

	sysreg |= (clk_div & 0x03) << BSP_CPUFREQ_DIV_OFFSET;
	sysreg |= (clk_sel & 0x0f) << BSP_CPUFREQ_SEL_OFFSET;

	REG32(BSP_HW_STRAP) = sysreg;

	if(old_clk_sel != clk_sel)
	{
		REG32(BSP_GISR)=0xffffffff;

		__asm__ volatile("nop");
		__asm__ volatile("nop");
		__asm__ volatile("nop");
		__asm__ volatile("nop");
		__asm__ volatile("wait");
		__asm__ volatile("nop");

		__asm__ volatile("nop");
		__asm__ volatile("nop");
		__asm__ volatile("nop");
		__asm__ volatile("nop");
		__asm__ volatile("nop");
	}

	REG32(BSP_GIMR) = mask; // restore
	plat_time_init();

	return 0;
}

void rtl819x_clk_add(const char *dev, unsigned long rate)
{
	struct clk *clk = kzalloc(sizeof(struct clk), GFP_KERNEL);


	if (!clk)
		panic("failed to add clock\n");

	clk->cl.dev_id = dev;
	clk->cl.clk = clk;

	clk->rate = rate;

	printk("Adding clock %s@%luMHz\n", dev, rate / 1000000);
	clkdev_add(&clk->cl);
}

static int __init rtl819x_clk_init(void)
{
	unsigned long int cpu_clk = cal_cpu_clock(); //kernel using cpu speed in KHz


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
	return cal_cpu_clock();
}
EXPORT_SYMBOL(clk_get_rate);

int clk_set_rate(struct clk *clk, unsigned long freq)
{
	unsigned char sel;
	unsigned char div;


	if(!get_sel_div(freq, &sel, &div)){
		SettingCPUClk(sel, div);

		return 0;
	}
	printk("Unsupported freq: %ld \n", freq);

	return -1;
}
EXPORT_SYMBOL(clk_set_rate);
