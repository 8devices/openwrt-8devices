/*
 * Realtek Semiconductor Corp.
 *
 * bsp/irq.c
 *     bsp interrupt initialization and handler code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>

#include <asm/irq.h>
#include <asm/irq_cpu.h>
//#include <asm/gcmpregs.h>  //mark_cc , no need in kernel 3.18
#include <asm/mipsregs.h>

#include "bspchip.h"

static inline int clz(unsigned long x)
{
	__asm__ __volatile__(
	"	clz     %0, %1                                  \n"
	: "=r" (x)
	: "r" (x));

	return x;
}

/*
 * FFS
 *
 * Given pending, use ffs to find first leading non-zero. Then,
 * Use offset to shift bit range. For example, use CAUSEB_IP as offset
 * to look for bit starting at 12 in status register, so that ffs is
 * rounded between 0~7
 */
static inline int irq_ffs(unsigned int pending, unsigned int offset)
{
	return -clz(pending) + 31 - offset;
}

#ifdef CONFIG_IRQ_ICTL
#include "irq-ictl.c"
#endif

#ifdef CONFIG_IRQ_GPIO
#include "irq-gpio.c"
#endif

/*
 * IRQs on the Sheipa2 look basically (barring software IRQs which we
 * don't use at all and all external interrupt sources are combined together
 * on hardware interrupt 0 (MIPS IRQ 2)) like:
 *
 *      MIPS IRQ        GIC
 *      --------        ------           ------
 *             0        UNUSED           SWINT0
 *             1        UNUSED           SWINT1
 *             2        irq_n            ICTL
 *             3        ipi_resched      GIC IPI
 *             4        ipi_call         GIC IPI
 *             5        UNUSED           UNUSED
 *             6        r4k_perfcnt      GIC VPE local
 *             7        r4k_timer        GIC VPE local
 *
 * We handle the IRQ according to _our_ priority which is:
 *
 * Highest ----     r4k timer
 * Lowest  ----     ictl
 *
 * then we just return, if multiple IRQs are pending then we will just take
 * another exception, big deal.
 */
//asmlinkage void bsp_irq_dispatch(void)
asmlinkage void plat_irq_dispatch(void) //mips-ori
{
	unsigned int pending = read_c0_cause() & read_c0_status() & ST0_IM;
	int irq;

	irq = irq_ffs(pending, CAUSEB_IP);
	if (irq >= 0)
	{
		do_IRQ(irq);
	}
	else
		spurious_interrupt();
}

//void __init bsp_irq_init(void)
void __init arch_init_irq(void) //mips-ori
{
    //wei add, 96F
	REG32(BSP_GIMR) = 0;
	REG32(BSP_GIMR2) = 0;

	/* Set IRR */
	REG32(BSP_IRR0) = BSP_IRR0_SETTING;
	REG32(BSP_IRR1) = BSP_IRR1_SETTING;
	REG32(BSP_IRR2) = BSP_IRR2_SETTING;
	REG32(BSP_IRR3) = BSP_IRR3_SETTING;

	REG32(BSP_IRR4) = BSP_IRR4_SETTING;
	REG32(BSP_IRR5) = BSP_IRR5_SETTING;
	REG32(BSP_IRR6) = BSP_IRR6_SETTING;
	REG32(BSP_IRR7) = BSP_IRR7_SETTING;

	/* initialize IRQ action handlers */
	//mips_cpu_irq_init(BSP_IRQ_CPU_BASE);
	mips_cpu_irq_init(); // mips-ori

#ifdef CONFIG_IRQ_ICTL
	bsp_ictl_irq_init(BSP_IRQ_ICTL_BASE);
#endif

#ifdef CONFIG_IRQ_GPIO
	bsp_gpio_irq_init(BSP_IRQ_GPIO_BASE);
#endif

	REG32(BSP_GIMR) |= BSP_GIMR_TOCPU_MASK;
	REG32(BSP_GIMR2) |= BSP_GIMR_TOCPU_MASK2;
}
