/*
 * Realtek Semiconductor Corp.
 *
 * bsp/bspinit.h:
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#ifndef __BSPINIT_H_
#define __BSPINIT_H_

#include <asm/mipsregs.h>

#define C0_ErrCtl      $26

.macro  kernel_entry_setup
#if 0
	/* disable ErrCtl bit 28 */
	mfc0   t0, C0_ErrCtl
	li     t1, 0xefffffff      # set bit 28 = 0
	and    t0, t1
	mtc0   t0, C0_ErrCtl
#endif
.endm

.macro  smp_slave_setup
.endm

#endif
