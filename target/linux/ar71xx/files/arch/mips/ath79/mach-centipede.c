/*
 *  8devices Centipede board support
 *
 *  Copyright (C) 2016 Mantas Pucka <mantas@8devices.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define CENTIPEDE_MAC0_OFFSET			0x0000
#define CENTIPEDE_CALDATA_OFFSET		0x1000
#define CENTIPEDE_WMAC_MAC_OFFSET		0x1002

#define CENTIPEDE_GPIO_LED_ETH0			13

static struct gpio_led centipede_leds_gpio[] __initdata = {
	{
		.name		= "centipede:green:eth0",
		.gpio		= CENTIPEDE_GPIO_LED_ETH0,
		.active_low	= 1,
	}
};

static void __init centipede_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	ath79_register_wmac(art + CENTIPEDE_CALDATA_OFFSET,
			    art + CENTIPEDE_WMAC_MAC_OFFSET);

	ath79_setup_ar933x_phy4_switch(true, true);

	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);
	ath79_register_usb();
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth1_data.mac_addr, art + CENTIPEDE_MAC0_OFFSET, 0);

	/* LAN port */
	ath79_register_eth(1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(centipede_leds_gpio),
				 centipede_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_CENTIPEDE, "CENTIPEDE", "8devices Centipede board",
		centipede_setup);
