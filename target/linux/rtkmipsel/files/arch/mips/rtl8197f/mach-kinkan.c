 /*
 *  8devices Kinkan devboard support
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


#define KINKAN_BUTTONS_POLL_INTERVAL		100
#define KINKAN_BUTTONS_DEBOUNCE_INTERVAL	(3 * KINKAN_BUTTONS_POLL_INTERVAL)

static struct gpio_led kinkan_leds_gpio[] __initdata = {
	{
		.name		= "kinkan:green:status",
		.gpio		= BSP_GPIO_PIN_G6,
		.active_low	= 1,
	},

};

static struct gpio_keys_button kinkan_buttons[] __initdata = {
	{
		.desc		= "default",
		.type		= EV_KEY,
		.code		= BTN_1,
		.debounce_interval = KINKAN_BUTTONS_DEBOUNCE_INTERVAL,
		.gpio		= BSP_GPIO_PIN_H0,
		.active_low	= 1,
	}
};

#define SET_PINMUX(reg, field, val)\
	REG32(reg) = (REG32(reg) & (~(0xF << field))) | (val << field)

static void kinkan_set_sd_pinmux(void)
{
	SET_PINMUX(BSP_PIN_MUX_SEL15, 28, 0); // MMC_D0
	SET_PINMUX(BSP_PIN_MUX_SEL15, 24, 0); // MMC_D1
	SET_PINMUX(BSP_PIN_MUX_SEL15, 20, 0); // MMC_D2
	SET_PINMUX(BSP_PIN_MUX_SEL15, 16, 0); // MMC_D3
	SET_PINMUX(BSP_PIN_MUX_SEL16,  8, 0); // MMC_CD
	SET_PINMUX(BSP_PIN_MUX_SEL16,  4, 0); // MMC_CLK
	SET_PINMUX(BSP_PIN_MUX_SEL16,  0, 0); // MMC_CMD
}

static void __init kinkan_setup(void)
{
	int i;

	rtl819x_register_leds_gpio(-1, ARRAY_SIZE(kinkan_leds_gpio),
		kinkan_leds_gpio);

	for (i=0; i<ARRAY_SIZE(kinkan_leds_gpio); i++) {
		rtl819x_gpio_pin_enable(kinkan_leds_gpio[i].gpio);
	}

	for (i=0; i<ARRAY_SIZE(kinkan_buttons); i++) {
		rtl819x_gpio_pin_enable(kinkan_buttons[i].gpio);
	}
	rtl819x_add_device_gpio_buttons(-1, KINKAN_BUTTONS_POLL_INTERVAL,
				       ARRAY_SIZE(kinkan_buttons),
				       kinkan_buttons);

	kinkan_set_sd_pinmux();
}

MIPS_MACHINE(RTL8197_MACH_KINKAN, "KINKAN", "8devices Kinkan devboard",
			 kinkan_setup);
