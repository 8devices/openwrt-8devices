/*
 * Realtek Semiconductor Corp.
 *
 * bsp/gpio.c
 *     DesignWare GPIO chip intialization and handlers
 *
 *  Copyright (C) 2015 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#ifdef CONFIG_RTL819X_GPIO
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/platform_data/rtl819x-gpio.h>
#endif
#include <bspchip.h>

#if defined(CONFIG_RTL_8367R_SUPPORT)
#define BSP_RESET_BTN_PIN		BSP_GPIO_PIN_G6
#define BSP_WPS_BTN_PIN			BSP_GPIO_PIN_H0
#define BSP_RESET_LED_PIN		BSP_GPIO_PIN_H1
#define BSP_WPS_LED_PIN			BSP_GPIO_PIN_H1
#define BSP_8367R_RST_PIN		BSP_GPIO_PIN_H2
#define BSP_8367R_I2C_PIN1		BSP_GPIO_PIN_C2
#define BSP_8367R_I2C_PIN2		BSP_GPIO_PIN_C3
#else
#define BSP_RESET_BTN_PIN		BSP_GPIO_PIN_C3
#define BSP_WPS_BTN_PIN			BSP_GPIO_PIN_C1
#define BSP_RESET_LED_PIN		BSP_GPIO_PIN_B7
#define BSP_WPS_LED_PIN			BSP_GPIO_PIN_B7
#endif

static DEFINE_SPINLOCK(rtl819x_gpio_lock);

static unsigned int rtl819x_gpio_mux(u32 pin, u32 *value, u32 *address );
 void rtl819x_gpio_pin_enable(u32 pin);

int rtl819x_gpio_request(struct gpio_chip *chip, unsigned pin)
{
	/* The pins you don't want to share */
	switch(pin) {
		case 16384:
			return -EINVAL;
			break;
	}

	rtl819x_gpio_pin_enable(pin);

	return 0;
}

static int rtl819x_gpio_get_value(struct gpio_chip *chip, unsigned pin)
{
	unsigned int data;

	data = (__raw_readl((void __iomem*)BSP_GPIO_DAT_REG(pin)) >> BSP_GPIO_BIT(pin) ) & 1;

	pr_debug("[%s][%d]: return %d\n", __FUNCTION__, __LINE__, data);
	return data;
}

static void rtl819x_gpio_set_value(struct gpio_chip *chip,
				  unsigned pin, int value)
{
	unsigned int data;

	data = __raw_readl((void __iomem*)BSP_GPIO_DAT_REG(pin));

	if (value == 0)
		data &= ~(1 << BSP_GPIO_BIT(pin));
	else
		data |= (1 << BSP_GPIO_BIT(pin));

	//printk("%s(%d)yyy 0x%08x\n",__FUNCTION__,__LINE__,data);

	__raw_writel(data, (void __iomem*)BSP_GPIO_DAT_REG(pin));

	//printk("%s(%d)xxx 0x%08x\n",__FUNCTION__,__LINE__,
	//	__raw_readl((void __iomem*)BSP_GPIO_DAT_REG(pin)));
}

static int rtl819x_gpio_direction_input(struct gpio_chip *chip,
				       unsigned pin)
{
	unsigned long flags;

	if (pin >= BSP_GPIO_PIN_MAX)
		return -EINVAL;

	/* these pins are only for output */
	switch(pin) {
	case BSP_GPIO_PIN_A0:
	case BSP_GPIO_PIN_A4:
	case BSP_GPIO_PIN_A6:
	case BSP_GPIO_PIN_A7:
	case BSP_GPIO_PIN_B0:
	case BSP_GPIO_PIN_B6:
	case BSP_GPIO_PIN_C0:
	case BSP_GPIO_PIN_C4:
	case BSP_GPIO_PIN_C7:
	case BSP_GPIO_PIN_E5:
	case BSP_GPIO_PIN_F0:
	case BSP_GPIO_PIN_F1:
	case BSP_GPIO_PIN_F2:
	case BSP_GPIO_PIN_F3:
	case BSP_GPIO_PIN_F7:
	case BSP_GPIO_PIN_G2:
	case BSP_GPIO_PIN_G4:
	case BSP_GPIO_PIN_G5:
	case BSP_GPIO_PIN_H4:
	case BSP_GPIO_PIN_H5:
		printk(KERN_ERR "rtl819x_gpio: request rejection, GPIO%d is only for output\n", pin);
		return -EINVAL;
	}

	spin_lock_irqsave(&rtl819x_gpio_lock, flags);

	/* 0 : input */
	__raw_writel(__raw_readl((void __iomem*)BSP_GPIO_DIR_REG(pin))
			& ~(1 << BSP_GPIO_BIT(pin)),
			(void __iomem*)BSP_GPIO_DIR_REG(pin));

	spin_unlock_irqrestore(&rtl819x_gpio_lock, flags);

	return 0;
}

static int rtl819x_gpio_direction_output(struct gpio_chip *chip,
					unsigned pin, int value)
{
	unsigned long flags;

	if (pin >= BSP_GPIO_PIN_MAX)
		return -1;

	spin_lock_irqsave(&rtl819x_gpio_lock, flags);

	__raw_writel(__raw_readl((void __iomem*)BSP_GPIO_DIR_REG(pin))
			| (1 << BSP_GPIO_BIT(pin)),
			(void __iomem*)BSP_GPIO_DIR_REG(pin) );

	spin_unlock_irqrestore(&rtl819x_gpio_lock, flags);

	return 0;
}

static int rtl819x_gpio_get_direction(struct gpio_chip *chip, unsigned pin)
{
	return __raw_readl((void __iomem*)BSP_GPIO_DIR_REG(pin))
			& 0x1 << BSP_GPIO_BIT(pin) ? 0 : 1;
}

static int rtl819x_gpio_to_irq(struct gpio_chip *chip, unsigned pin)
{
	pr_debug("rtl819x_gpio: GPIO%d requests IRQ%d\n", pin, BSP_GPIO_TO_IRQ(pin));
		return BSP_GPIO_TO_IRQ(pin);
}
#ifdef CONFIG_OPENWRT_SDK
int gpio_to_irq(unsigned gpio)
{
    /* FIXME */
    return -EINVAL;
}
EXPORT_SYMBOL(gpio_to_irq);

int irq_to_gpio(unsigned irq)
{
    /* FIXME */
    return -EINVAL;
}
EXPORT_SYMBOL(irq_to_gpio);
#endif

static struct gpio_chip rtl819x_gpio_peripheral = {

	.label				= "rtl819x_gpio",
	.request			= rtl819x_gpio_request,
	.get				= rtl819x_gpio_get_value,
	.set				= rtl819x_gpio_set_value,
	.direction_input	= rtl819x_gpio_direction_input,
	.direction_output	= rtl819x_gpio_direction_output,
	.get_direction		= rtl819x_gpio_get_direction,
	.to_irq				= rtl819x_gpio_to_irq,
	.base				= 0,
};

#ifdef CONFIG_RTL819X_GPIO
/* LED devices */
static struct rtl819x_gpio_platdata rtl_pdata_led1 = {
	.flags		= RTL819X_GPIO_ACTLOW,
	.name		= "reset led",
};

static struct platform_device rtl_led_dev1 = {
	.name		= "rtl819x_led",
	.id			= 0,
	.dev		= {
		.platform_data = &rtl_pdata_led1
	},
};

static struct rtl819x_gpio_platdata rtl_pdata_led2 = {
	.flags		= RTL819X_GPIO_ACTLOW,
	.name		= "wps led",
};

static struct platform_device rtl_led_dev2 = {
	.name		= "rtl819x_led",
	.id			= 1,
	.dev		= {
		.platform_data = &rtl_pdata_led2
	},
};

/* Button devices */
static struct rtl819x_gpio_platdata rtl_pdata_btn1 = {
	.flags		= RTL819X_GPIO_ACTLOW,
	.name		= "reset btn",
};

static struct platform_device rtl_btn_dev1 = {
	.name		= "rtl819x_btn",
	.id			= 0,
	.dev		= {
		.platform_data = &rtl_pdata_btn1
	},
};

static struct rtl819x_gpio_platdata rtl_pdata_btn2 = {
	.flags		= RTL819X_GPIO_ACTLOW,
	.name		= "wps btn",
};

static struct platform_device rtl_btn_dev2 = {
	.name		= "rtl819x_btn",
	.id			= 1,
	.dev		= {
		.platform_data = &rtl_pdata_btn2
	},
};

/* Other devices */
#if defined(CONFIG_RTL_8367R_SUPPORT)
static struct rtl819x_gpio_platdata rtl_pdata_gpio1 = {
	.flags		= RTL819X_GPIO_ACTLOW,
	.name		= "8367r reset pin",
};

static struct platform_device rtl_gpio_dev1 = {
	.name		= "rtl819x_8367r_reset_pin",
	.id			= 0,
	.dev		= {
		.platform_data = &rtl_pdata_gpio1
	},
};

static struct rtl819x_gpio_platdata rtl_pdata_gpio2 = {
	.flags		= RTL819X_GPIO_ACTLOW,
	.name		= "8367r i2c pin1",
};

static struct platform_device rtl_gpio_dev2 = {
	.name		= "rtl819x_8367r_i2c_pin",
	.id			= 1,
	.dev		= {
		.platform_data = &rtl_pdata_gpio2
	},
};

static struct rtl819x_gpio_platdata rtl_pdata_gpio3 = {
	.flags		= RTL819X_GPIO_ACTLOW,
	.name		= "8367r i2c pin2",
};

static struct platform_device rtl_gpio_dev3 = {
	.name		= "rtl819x_8367r_i2c_pin",
	.id			= 2,
	.dev		= {
		.platform_data = &rtl_pdata_gpio3
	},
};
#endif

/* devices we initialise */
static struct platform_device __initdata *rtl_devs[] = {
	&rtl_led_dev1,
	&rtl_led_dev2,
	&rtl_btn_dev1,
	&rtl_btn_dev2,
#if defined(CONFIG_RTL_8367R_SUPPORT)
	&rtl_gpio_dev1,
	&rtl_gpio_dev2,
	&rtl_gpio_dev3,
#endif
};
#endif
void rtl819x_gpio_pin_enable(u32 pin)
{
	unsigned long flags;
	unsigned int mask  = 0;
	unsigned int mux = 0;
	unsigned int mux_reg = 0;
	unsigned int val = 0;

    if (pin >= TOTAL_PIN_MAX)
		return;

	spin_lock_irqsave(&rtl819x_gpio_lock, flags);

	/* pin MUX1 */
	mask = rtl819x_gpio_mux(pin,&val,&mux_reg);

	//mux  = __raw_readl((void __iomem*) BSP_PINMUX_SEL_REG(pin));
	mux  = __raw_readl((void __iomem*)mux_reg);

	//if (mask != 0 && (mux & mask) == 0)
	if (mask != 0)
	    __raw_writel( ((mux&(~mask)) | (val)), (void __iomem*)mux_reg);

	/* 0 as BSP_GPIO pin */
    if(pin < BSP_GPIO_PIN_MAX)
    {
        __raw_writel(__raw_readl((void __iomem*)BSP_GPIO_CNR_REG(pin)) & ~(1<<BSP_GPIO_BIT(pin)),
					(void __iomem*)BSP_GPIO_CNR_REG(pin));
    }

	spin_unlock_irqrestore(&rtl819x_gpio_lock, flags);
}

void rtl819x_gpio_pin_disable(u32 pin)
{
	unsigned long flags;

	spin_lock_irqsave(&rtl819x_gpio_lock, flags);

	/* 1 as peripheral pin */
	__raw_writel(__raw_readl((void __iomem*)BSP_GPIO_CNR_REG(pin)) | (1<<BSP_GPIO_BIT(pin)),
					(void __iomem*)BSP_GPIO_CNR_REG(pin));

	spin_unlock_irqrestore(&rtl819x_gpio_lock, flags);
}

void rtl819x_gpio_pin_set_val(u32 pin, int val) //mark_wrt
{
        rtl819x_gpio_set_value(0,pin, val);
}

#if 0 //move to platform.c
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
#endif
static int __init rtl819x_gpio_peripheral_init(void)
{
	int err;

	printk("Realtek GPIO controller driver init\n");

	rtl819x_gpio_peripheral.ngpio = BSP_GPIO_PIN_MAX;
	err = gpiochip_add(&rtl819x_gpio_peripheral);

	if (err) {
		panic("cannot add rtl89x BSP_GPIO chip, error=%d", err);
		return err;
	}
#ifdef CONFIG_RTL819X_GPIO
	rtl_pdata_led1.gpio = BSP_RESET_LED_PIN;
	rtl_pdata_led2.gpio = BSP_WPS_LED_PIN;
	rtl_pdata_btn1.gpio = BSP_RESET_BTN_PIN;
	rtl_pdata_btn2.gpio = BSP_WPS_BTN_PIN;
#if defined(CONFIG_RTL_8367R_SUPPORT)
	rtl_pdata_gpio1.gpio = BSP_8367R_RST_PIN;
	rtl_pdata_gpio2.gpio = BSP_8367R_I2C_PIN1;
	rtl_pdata_gpio3.gpio = BSP_8367R_I2C_PIN2;
#endif

	platform_add_devices(rtl_devs, ARRAY_SIZE(rtl_devs));
#endif
	return 0;
}
arch_initcall(rtl819x_gpio_peripheral_init);


static unsigned int rtl819x_gpio_mux(u32 pin, u32 *value, u32 *address )
{
	unsigned int mask = 0;

	switch(pin) {
	case BSP_GPIO_PIN_A0:
		mask = 0xf<<28;
		*value = 0x8<<28;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A1:
		mask = 0xf<<28;
		*value = 0x8<<28;
		*address = BSP_PIN_MUX_SEL6;
		break;
	case BSP_GPIO_PIN_A2:
		mask = 0xf<<24;
		*value = 0xb<<24;
		*address = BSP_PIN_MUX_SEL6;
		break;
	case BSP_GPIO_PIN_A3:
		mask = 0xf<<16;
		*value = 0x8<<16;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A4:
		mask = 0xf<<24;
		*value = 0x7<<24;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A5:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL7;
		break;
	case BSP_GPIO_PIN_A6:
		mask = 0xf<<16;
		*value = 0x5<<16;
		*address = BSP_PIN_MUX_SEL0;
		break;
	case BSP_GPIO_PIN_A7:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL0;
		break;

	case BSP_GPIO_PIN_B0:
		mask = 0xf<<24;
		*value = 0x8<<24;
		*address = BSP_PIN_MUX_SEL0;
		break;
	case BSP_GPIO_PIN_B1:
		mask = 0xf<<16;
		*value = 0xa<<16;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_B2:
		mask = 0xf<<16;
		*value = 0x8<<16;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B3:
		mask = 0xf<<20;
		*value = 0x8<<20;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B4:
		mask = 0xf<<24;
		*value = 0x8<<24;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B5:
		mask = 0xf<<28;
		*value = 0x7<<28;
		*address = BSP_PIN_MUX_SEL1;
		break;
	case BSP_GPIO_PIN_B6:
		mask = 0xf<<28;
		*value = 0x8<<28;
		*address = BSP_PIN_MUX_SEL0;
		break;
	case BSP_GPIO_PIN_B7:
		mask = 0xf<<24;
		*value = 0x8<<24;
		*address = BSP_PIN_MUX_SEL2;
		break;

	case BSP_GPIO_PIN_C0:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C1:
		mask = 0xf<<12;
		*value = 0x7<<12;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C2:
		mask = 0xf<<8;
		*value = 0x6<<8;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C3:
		mask = 0xf<<4;
		*value = 0x6<<4;
		*address = BSP_PIN_MUX_SEL2;
		break;
	case BSP_GPIO_PIN_C4:
		mask = 0xf<<16;
		*value = 0x2<<16;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_C5:
		mask = 0xf<<12;
		*value = 0x6<<12;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_C6:
		mask = 0xf<<8;
		*value = 0x8<<8;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_C7:
		mask = 0xf<<4;
		*value = 0x5<<4;
		*address = BSP_PIN_MUX_SEL16;
		break;

	case BSP_GPIO_PIN_D0:
		mask = 0xf<<0;
		*value = 0x5<<0;
		*address = BSP_PIN_MUX_SEL16;
		break;
	case BSP_GPIO_PIN_D1:
		mask = 0xf<<28;
		*value = 0x5<<28;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D2:
		mask = 0xf<<24;
		*value = 0x5<<24;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D3:
		mask = 0xf<<20;
		*value = 0x5<<20;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D4:
		mask = 0xf<<16;
		*value = 0x5<<16;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D5:
		mask = 0xf<<12;
		*value = 0x7<<12;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D6:
		mask = 0xf<<8;
		*value = 0x6<<8;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_D7:
		mask = 0xf<<4;
		*value = 0x7<<4;
		*address = BSP_PIN_MUX_SEL15;
		break;

	case BSP_GPIO_PIN_E0:
		mask = 0xf<<0;
		*value = 0x7<<0;
		*address = BSP_PIN_MUX_SEL15;
		break;
	case BSP_GPIO_PIN_E1:
		mask = 0xf<<28;
		*value = 0x1<<28;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E2:
		mask = 0xf<<24;
		*value = 0x1<<24;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E3:
		mask = 0xf<<20;
		*value = 0x1<<20;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E4:
		mask = 0xf<<16;
		*value = 0x1<<16;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E5:
		mask = 0xf<<12;
		*value = 0x1<<12;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E6:
		mask = 0xf<<8;
		*value = 0x1<<8;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_E7:
		mask = 0xf<<4;
		*value = 0x1<<4;
		*address = BSP_PIN_MUX_SEL17;
		break;

	case BSP_GPIO_PIN_F0:
		mask = 0xf<<0;
		*value = 0x1<<0;
		*address = BSP_PIN_MUX_SEL17;
		break;
	case BSP_GPIO_PIN_F1:
		mask = 0xf<<28;
		*value = 0x2<<28;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F2:
		mask = 0xf<<24;
		*value = 0x4<<24;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F3:
		mask = 0xf<<20;
		*value = 0x4<<20;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F4:
		mask = 0xf<<16;
		*value = 0x6<<16;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F5:
		mask = 0xf<<12;
		*value = 0x6<<12;
		*address = BSP_PIN_MUX_SEL18;
		break;
	case BSP_GPIO_PIN_F6:
		mask = 0xf<<24;
		*value = 0x6<<24;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_F7:
		mask = 0xf<<28;
		*value = 0x6<<28;
		*address = BSP_PIN_MUX_SEL8;
		break;

	case BSP_GPIO_PIN_G0:
		mask = 0xf<<20;
		*value = 0x6<<20;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_G1:
		mask = 0xf<<16;
		*value = 0x7<<16;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_G2:
		mask = 0xf<<12;
		*value = 0x7<<12;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_G3:
		mask = 0xf<<28;
		*value = 0x2<<28;
		*address = BSP_PIN_MUX_SEL9;
		break;
	case BSP_GPIO_PIN_G4:
		mask = 0xf<<24;
		*value = 0x1<<24;
		*address = BSP_PIN_MUX_SEL9;
		break;
	case BSP_GPIO_PIN_G5:
		mask = 0xf<<20;
		*value = 0x0<<20;
		*address = BSP_PIN_MUX_SEL9;
		break;
	case BSP_GPIO_PIN_G6:
		mask = 0xf<<28;
		*value = 0x3<<28;
		*address = BSP_PIN_MUX_SEL13;
		break;
	case BSP_GPIO_PIN_G7:
		mask = 0xf<<24;
		*value = 0x3<<24;
		*address = BSP_PIN_MUX_SEL13;
		break;

	case BSP_GPIO_PIN_H0:
		mask = 0xf<<20;
		*value = 0x3<<20;
		*address = BSP_PIN_MUX_SEL13;
		break;
	case BSP_GPIO_PIN_H1:
		mask = 0xf<<16;
		*value = 0x2<<16;
		*address = BSP_PIN_MUX_SEL13;
		break;
	case BSP_GPIO_PIN_H2:
		mask = 0xf<<28;
		*value = 0x2<<28;
		*address = BSP_PIN_MUX_SEL14;
		break;
	case BSP_GPIO_PIN_H3:
		mask = 0xf<<24;
		*value = 0x1<<24;
		*address = BSP_PIN_MUX_SEL12;
		break;
	case BSP_GPIO_PIN_H4:
		mask = 0xf<<28;
		*value = 0x1<<28;
		*address = BSP_PIN_MUX_SEL12;
		break;
	case BSP_GPIO_PIN_H5:
		mask = 0xf<<0;
		*value = 0x1<<0;
		*address = BSP_PIN_MUX_SEL8;
		break;
	case BSP_GPIO_PIN_H6:
		break;

	case BSP_GPIO_PIN_H7:
		break;

	case BSP_UART1_PIN:
		break;
	case BSP_UART2_PIN:
		break;
	default:
		break;
	}

	return mask;
}
