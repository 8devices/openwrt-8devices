/*
 * Realtek Semiconductor Corp.
 *
 * bsp/timer.c
 *     bsp timer initialization code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/timex.h>
#include <linux/delay.h>

#include <asm/time.h>

#include "bspchip.h"

const unsigned int cpu_clksel_table[]={ 450, 500, 550, 600, 650,700,
										750, 800, 850, 900, 950, 1000,
										1050, 1100, 1150, 1200 };

const unsigned int cpu_clkdiv_table[]={1, 2, 4, 8};


#ifdef CONFIG_CEVT_EXT
void inline bsp_timer_ack(void)
{
	unsigned volatile int eoi;
	eoi = REG32(BSP_TIMER0_EOI);
}

void __init bsp_timer_init(void)
{
	change_c0_cause(CAUSEF_DC, 0);

	/* disable timer */
	REG32(BSP_TIMER0_TCR) = 0x00000000;

	/* initialize timer registers */
	REG32(BSP_TIMER0_TLCR) = BSP_TIMER0_FREQ / HZ;

	/* hook up timer interrupt handler */
	ext_clockevent_init(BSP_TIMER0_IRQ);

	/* enable timer */
	REG32(BSP_TIMER0_TCR) = 0x00000003;       /* 0000-0000-0000-0011 */
}
#endif /* CONFIG_CEVT_EXT */

#ifdef CONFIG_RTL_WTDOG
int bBspWatchdog = 0;

void bsp_enable_watchdog( void )
{
	bBspWatchdog = 1;
	*(volatile unsigned long *)(0xb800311C)=0x00240000; // 2^24
}

void bsp_disable_watchdog( void )
{
	*(volatile unsigned long *)(0xb800311C)=0xA5240000;
	bBspWatchdog = 0;
}
#endif

#ifdef CONFIG_CEVT_R4K
unsigned int __cpuinit get_c0_compare_int(void)
{
	return BSP_COMPARE_IRQ;
}

//void __init bsp_timer_init(void)
void plat_time_init(void) // mips-ori
{
	unsigned int freq, div, hwstrap;

	/* set cp0_compare_irq and cp0_perfcount_irq */
	cp0_compare_irq = BSP_COMPARE_IRQ;
	cp0_perfcount_irq = BSP_PERFCOUNT_IRQ;

	if (cp0_perfcount_irq == cp0_compare_irq)
		cp0_perfcount_irq = -1;

	//mips_hpt_frequency = BSP_CPU0_FREQ / 2;
	hwstrap = REG32(BSP_HW_STRAP);
	if ((hwstrap >> 14) & 0x1) {
		freq = 200 * 1000000;
	}
	else {
		div = (hwstrap >> 19) & 0x3;
		freq = (hwstrap >> 15) & 0xF;
		freq = cpu_clksel_table[freq] * 1000000 / cpu_clkdiv_table[div];
	}
	mips_hpt_frequency = freq / 2;

	//write_c0_count(0);
	//mips_clockevent_init(cp0_compare_irq); // mips-ori

	if (-1 != cp0_perfcount_irq)
		printk("TBD if not shared with timer\n");

#ifdef CONFIG_RTL_WTDOG
	REG32(BSP_CDBR)=(BSP_DIVISOR) << BSP_DIVF_OFFSET;
	bsp_enable_watchdog();
#endif
}
#endif /* CONFIG_CEVT_R4K */
