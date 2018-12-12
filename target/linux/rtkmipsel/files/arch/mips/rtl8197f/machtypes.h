 
/*
 *  Realtek 8197F machine type definitions
 *
 *  Copyright (C) 2008 Mantas Pucka <mantas@8devices.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _RTL8197_MACHTYPE_H
#define _RTL8197_MACHTYPE_H

#include <asm/mips_machine.h>

enum ath79_mach_type {
	RTL8197_MACH_GENERIC_OF = -1,	/* Device tree board */
	RTL8197_MACH_GENERIC = 0,
	RTL8197_MACH_KINKAN,			/* 8devices Kinkan devboard*/
	RTL8197_MACH_KOMIKAN,			/* 8devices Komikan devboard*/
};

#endif /* _RTL8197_MACHTYPE_H */
