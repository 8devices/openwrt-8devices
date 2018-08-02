 /*
 *  Realtek/RTL819X common GPIO support
 *
 *  Copyright (C) 2018 Mantas Pucka <mantas@8devices.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _RTL819X_GPIO_H
#define _RTL819X_GPIO_H

#include <linux/gpio.h>

void rtl819x_gpio_pin_enable(u32 pin);
void rtl819x_gpio_pin_set_val(u32 pin, unsigned val);

#endif /* _RTL819X_GPIO_H */
