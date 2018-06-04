/*
 *  Realtek/RTL819X common common GPIO LEDs support
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _RTL819X_DEV_LEDS_GPIO_H
#define _RTL819X_DEV_LEDS_GPIO_H

#include <linux/leds.h>

void rtl819x_register_leds_gpio(int id,
			      unsigned num_leds,
			      struct gpio_led *leds);

#endif /* _RTL819X_DEV_LEDS_GPIO_H */
