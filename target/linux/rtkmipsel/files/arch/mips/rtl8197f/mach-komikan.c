 /*
 *  8devices Komikan devboard support
 *
 *  Copyright (C) 2018 Mantas Pucka <mantas@8devices.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/leds.h>
#include <generated/autoconf.h>

#include "gpio.h"
#include "bspchip.h"
#include "machtypes.h"
#include "dev_leds_gpio.h"
#include "dev-gpio-buttons.h"


#define KOMIKAN_BUTTONS_POLL_INTERVAL		100
#define KOMIKAN_BUTTONS_DEBOUNCE_INTERVAL	(3 * KOMIKAN_BUTTONS_POLL_INTERVAL)

static struct gpio_led komikan_leds_gpio[] __initdata = {
	{
		.name		= "komikan:green:led0",
		.gpio		= BSP_GPIO_PIN_G6,
		.active_low	= 0,
	},
	{
		.name		= "komikan:green:led1",
		.gpio		= BSP_GPIO_PIN_C4,
		.active_low	= 0,
	},
	{
		.name		= "komikan:green:led2",
		.gpio		= BSP_GPIO_PIN_H2,
		.active_low	= 0,
	},
	{
		.name		= "komikan:green:led3",
		.gpio		= BSP_GPIO_PIN_E0,
		.active_low	= 0,
	},

};

static struct gpio_keys_button komikan_buttons[] __initdata = {
	{
		.desc		= "default",
		.type		= EV_KEY,
		.code		= BTN_1,
		.debounce_interval = KOMIKAN_BUTTONS_DEBOUNCE_INTERVAL,
		.gpio		= BSP_GPIO_PIN_H0,
		.active_low	= 1,
	}
};

#define SET_PINMUX(reg, field, val)\
	REG32(reg) = (REG32(reg) & (~(0xF << field))) | (val << field)

static void komikan_set_sd_pinmux(void)
{
	SET_PINMUX(BSP_PIN_MUX_SEL15, 28, 0); // MMC_D0
	SET_PINMUX(BSP_PIN_MUX_SEL15, 24, 0); // MMC_D1
	SET_PINMUX(BSP_PIN_MUX_SEL15, 20, 0); // MMC_D2
	SET_PINMUX(BSP_PIN_MUX_SEL15, 16, 0); // MMC_D3
	SET_PINMUX(BSP_PIN_MUX_SEL16,  8, 0); // MMC_CD
	SET_PINMUX(BSP_PIN_MUX_SEL16,  4, 0); // MMC_CLK
	SET_PINMUX(BSP_PIN_MUX_SEL16,  0, 0); // MMC_CMD
}

static void __init komikan_setup(void)
{
	int i;

	rtl819x_register_leds_gpio(-1, ARRAY_SIZE(komikan_leds_gpio),
		komikan_leds_gpio);

	for (i=0; i<ARRAY_SIZE(komikan_leds_gpio); i++) {
		rtl819x_gpio_pin_enable(komikan_leds_gpio[i].gpio);
	}

	for (i=0; i<ARRAY_SIZE(komikan_buttons); i++) {
		rtl819x_gpio_pin_enable(komikan_buttons[i].gpio);
	}
	rtl819x_add_device_gpio_buttons(-1, KOMIKAN_BUTTONS_POLL_INTERVAL,
				       ARRAY_SIZE(komikan_buttons),
				       komikan_buttons);

	komikan_set_sd_pinmux();
}

MIPS_MACHINE(RTL8197_MACH_KOMIKAN, "KOMIKAN", "8devices Komikan devboard",
			 komikan_setup);
