#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/pwm.h>
#include <linux/leds_pwm.h>
#include <linux/module.h>
#include <bspchip.h>

#include "machtypes.h"


/* PWM */
#ifdef CONFIG_PWM_RTK
struct platform_device rtk_pwm_device = {
	.name = "rtk-pwm",
	.id   = -1,
};

static struct platform_device __initdata *rtl_pwm_devs[] = {
		&rtk_pwm_device,
};

static struct pwm_lookup rtk_pwm_lookup[] = {
	PWM_LOOKUP("rtk-pwm", 0, "leds_pwm.0", NULL,80000,PWM_POLARITY_NORMAL), //mark_cc
	PWM_LOOKUP("rtk-pwm", 1, "leds_pwm.1", NULL,80000,PWM_POLARITY_NORMAL),
	PWM_LOOKUP("rtk-pwm", 2, "leds_pwm.2", NULL,80000,PWM_POLARITY_NORMAL),
	PWM_LOOKUP("rtk-pwm", 3, "leds_pwm.3", NULL,80000,PWM_POLARITY_NORMAL),
};
void set_pwm_pin_mux(void)
{
	//97FN QA board pin MUX
	REG32(0xb8000808)  =   (REG32(0xb8000808)  & (~(0xF << 16))) |  (0x9 << 16); //set RXC
	REG32(0xb8000800)  =   (REG32(0xb8000800)  & (~(0xFFF << 16))) |  (0x754 << 16); //set TXD1, TXD2, TXD3
	REG32(0xb8000010)  =   (REG32(0xb8000010)  & (~0xFFFFFFFF )) |  (0x80003800);
	REG32(0xb8000014)  =   (REG32(0xb8000014)  & (~(0x1F << 4))) |  (0x13 << 4); //enable timer

}
static int __init rtl819x_pwm_init(void)
{
	set_pwm_pin_mux();
	REG32(0xb8148008)  =   (REG32(0xb8148008)  & (~(0x3)))|0x3;//enable DW timer 0
	pwm_add_table(rtk_pwm_lookup, ARRAY_SIZE(rtk_pwm_lookup));
	platform_add_devices(rtl_pwm_devs, ARRAY_SIZE(rtl_pwm_devs));

	return 0;
}
arch_initcall(rtl819x_pwm_init);
#endif

#ifdef CONFIG_LEDS_PWM
/* LED PWM */
static struct led_pwm rtk_led_pwm1 = {
	.name = "rtk-led.0",
	.pwm_period_ns = 60000*10,
};
static struct led_pwm_platform_data rtk_led_data1 = {
	.num_leds = 1,
	.leds = &rtk_led_pwm1,
};
struct platform_device rtk_led_device1 = {
	.name = "leds_pwm",
	.id   = 0,
	.dev		= {
		.platform_data = &rtk_led_data1
	},
};


static struct led_pwm rtk_led_pwm2 = {
	.name = "rtk-led.1",
	.pwm_period_ns = 60000*10,
};
static struct led_pwm_platform_data rtk_led_data2 = {
	.num_leds = 1,
	.leds = &rtk_led_pwm2,
};
struct platform_device rtk_led_device2 = {
	.name = "leds_pwm",
	.id   = 1,
	.dev		= {
		.platform_data = &rtk_led_data2
	},
};


static struct led_pwm rtk_led_pwm3 = {
	.name = "rtk-led.2",
	.pwm_period_ns = 60000*10,
};
static struct led_pwm_platform_data rtk_led_data3 = {
	.num_leds = 1,
	.leds = &rtk_led_pwm3,
};
struct platform_device rtk_led_device3 = {
	.name = "leds_pwm",
	.id   = 2,
	.dev		= {
		.platform_data = &rtk_led_data3
	},
};


static struct led_pwm rtk_led_pwm4 = {
	.name = "rtk-led.3",
	.pwm_period_ns = 60000*10,
};
static struct led_pwm_platform_data rtk_led_data4 = {
	.num_leds = 1,
	.leds = &rtk_led_pwm4,
};
struct platform_device rtk_led_device4 = {
	.name = "leds_pwm",
	.id   = 3,
	.dev		= {
		.platform_data = &rtk_led_data4
	},
};

static struct platform_device __initdata *rtl_led_devs[] = {
		&rtk_led_device1,
		&rtk_led_device2,
		&rtk_led_device3,
		&rtk_led_device4,
};

static int __init rtl819x_led_init(void)
{
	platform_add_devices(rtl_led_devs, ARRAY_SIZE(rtl_led_devs));
	return 0;
}
arch_initcall(rtl819x_led_init);
#endif

/* UART */
struct platform_device rtk_uart1_device = {
	.name = "rtk-uart1",
	.id   = -1,
};

struct platform_device rtk_uart2_device = {
	.name = "rtk-uart2",
	.id   = -1,
};

static struct platform_device __initdata *rtl_uart_devs[] = {
		&rtk_uart1_device,
		&rtk_uart2_device,
};

static int __init rtl819x_uart_init(void)
{

	platform_add_devices(rtl_uart_devs, ARRAY_SIZE(rtl_uart_devs));

	return 0;
}
arch_initcall(rtl819x_uart_init);

/* SD Card */
#if defined(CONFIG_MMC_RTK_SDMMC) || defined(CONFIG_MMC_RTK_SDMMC_MODULE)
struct platform_device rtk_sd_device = {
	.name = "rtk-sdmmc",
	.id   = -1,
};

static struct platform_device __initdata *rtl_sd_devs[] = {
		&rtk_sd_device,
};

static int __init rtl819x_sd_init(void)
{

	platform_add_devices(rtl_sd_devs, ARRAY_SIZE(rtl_sd_devs));

	return 0;
}
arch_initcall(rtl819x_sd_init);
#endif

/* EMMC */
#ifdef CONFIG_MMC_RTKEMMC
struct platform_device rtk_emmc_device = {
	.name = "rtkemmc",
	.id   = -1,
};

static struct platform_device __initdata *rtl_emmc_devs[] = {
		&rtk_emmc_device,
};

static int __init rtl819x_emmc_init(void)
{

	platform_add_devices(rtl_emmc_devs, ARRAY_SIZE(rtl_emmc_devs));

	return 0;
}
arch_initcall(rtl819x_emmc_init);
#endif


unsigned int rtl819x_bond_option(void)
{
	unsigned int type = 0, ret = 0;

	type = __raw_readl((void __iomem*)BSP_BOND_OPTION) & 0xf;

	switch(type) {
	case 0x0:	/* 97FB */
		ret = BSP_BOND_97FB;
		break;
	case 0x4:	/* 97FN */
	case 0x5:
	case 0x6:
		 ret = BSP_BOND_97FN;
		break;
	case 0xa:	/* 97FS */
	case 0xb:
	case 0xc:
		ret = BSP_BOND_97FS;
	}

	pr_debug("[%s][%d]: 97F type %d\n", __FUNCTION__, __LINE__, ret);
	return ret;
}
EXPORT_SYMBOL(rtl819x_bond_option);

static int __init rtl8197f_setup(void)
{
	mips_machine_setup();

	return 0;
}

arch_initcall(rtl8197f_setup);

__setup("board=", mips_machtype_setup);


