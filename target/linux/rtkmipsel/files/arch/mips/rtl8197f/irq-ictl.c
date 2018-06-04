/*
 * Realtek Semiconductor Corp.
 *
 * bsp/irq-ictl.c
 *     DesignWare ICTL intialization and handlers
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
static void bsp_ictl_irq_mask(struct irq_data *d)
{
	unsigned int irq = d->irq;
#else
static void bsp_ictl_irq_mask(unsigned int irq)
{
#endif
	if (irq >= BSP_IRQ_ICTL_BASE2)
		REG32(BSP_GIMR2) &= ~(1 << (irq - BSP_IRQ_ICTL_BASE2));
	else
		REG32(BSP_GIMR) &= ~(1 << (irq - BSP_IRQ_ICTL_BASE));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
static void bsp_ictl_irq_unmask(struct irq_data *d)
{
	unsigned int irq = d->irq;
#else
static void bsp_ictl_irq_unmask(unsigned int irq)
{
#endif
	if (irq >= BSP_IRQ_ICTL_BASE2)
		REG32(BSP_GIMR2) |= (1 << (irq - BSP_IRQ_ICTL_BASE2));
	else
		REG32(BSP_GIMR) |= (1 << (irq - BSP_IRQ_ICTL_BASE));
}

static struct irq_chip bsp_ictl_irq = {
	.name = "ICTL",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
	.irq_ack = bsp_ictl_irq_mask,
	.irq_mask = bsp_ictl_irq_mask,
	.irq_unmask = bsp_ictl_irq_unmask,
#else
	.ack = bsp_ictl_irq_mask,
	.mask = bsp_ictl_irq_mask,
	.unmask = bsp_ictl_irq_unmask,
#endif
};

irqreturn_t bsp_ictl_irq_dispatch(int cpl, void *dev_id)
{
	unsigned int pending = REG32(BSP_GISR) & REG32(BSP_GIMR) & (~(BSP_GIMR_TOCPU_MASK));
	int irq;

	irq = irq_ffs(pending, 0);
	if (irq >= 0) {
		do_IRQ(BSP_IRQ_ICTL_BASE + irq);
	}
	else {
			unsigned int pending2 = REG32(BSP_GISR2) & REG32(BSP_GIMR2) & (~(BSP_GIMR_TOCPU_MASK2));
			irq = irq_ffs(pending2, 0);
			if (irq >= 0)
				do_IRQ(BSP_IRQ_ICTL_BASE2 + irq);
			else
				spurious_interrupt();
		}
	return IRQ_HANDLED;
}

static struct irqaction irq_cascade = {
	.handler = bsp_ictl_irq_dispatch,
	.name = "IRQ cascade",
};

static void __init bsp_ictl_irq_init(unsigned int irq_base)
{
	int i;

	for (i=0; i < BSP_IRQ_ICTL_NUM + BSP_IRQ_ICTL_NUM2; i++)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
		irq_set_chip_and_handler(irq_base + i, &bsp_ictl_irq, handle_level_irq);
#else
		set_irq_chip_and_handler(irq_base + i, &bsp_ictl_irq, handle_level_irq);
#endif

	setup_irq(BSP_ICTL_IRQ, &irq_cascade);
}
