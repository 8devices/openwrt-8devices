/*
 * Realtek Semiconductor Corp.
 *
 * bsp/irq-gpio.c
 *     DesignWare GPIO interrupt intialization and handlers
 *
 *  Copyright (C) 2015 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
char gpio_pin_int_masks[BSP_GPIO_PIN_MAX] = {0};

static void bsp_gpio_irq_demux(unsigned int irq)
{
	unsigned int start, end, status, mask1, mask2, mask3 = 0x0;
	unsigned int bit, i;

	start = (irq == BSP_GPIO_PABCD_IRQ) ? BSP_GPIO_PIN_A0 : BSP_GPIO_2ND_REG;
	end = (irq == BSP_GPIO_PABCD_IRQ) ? BSP_GPIO_2ND_REG: BSP_GPIO_PIN_MAX;
	status = __raw_readl((void __iomem*)((irq == BSP_GPIO_PABCD_IRQ) ? BSP_PABCD_ISR : BSP_PEFGH_ISR));
	mask1 = __raw_readl((void __iomem*)((irq == BSP_GPIO_PABCD_IRQ) ? BSP_PAB_IMR : BSP_PEF_IMR));
	mask2 = __raw_readl((void __iomem*)((irq == BSP_GPIO_PABCD_IRQ) ? BSP_PCD_IMR : BSP_PGH_IMR));

	/* get the mask for interrupt status register by ourself */
	for(i = start; i < end; i++) {
		unsigned int m = (BSP_GPIO_BIT(i) < 16) ? mask1 : mask2;
		mask3 |= ((0x3 << BSP_GPIO_2BITS(i)) & m ? 1 : 0) << BSP_GPIO_BIT(i);
	}

	/* mask the pins which don't have interrupt */
	status &= mask3;
	pr_debug("[%s][%d]: GPIO interrupt demux, mask1=0x%8x mask2=0x%x mask3=0x%x status=0x%x\n"
			, __FUNCTION__, __LINE__, mask1, mask2, mask3, status);
	while(status) {
		bit = fls(status) - 1;
		generic_handle_irq(BSP_IRQ_GPIO_BASE + start + bit);
		status &= ~(1 << bit);
	}
}

irqreturn_t bsp_gpio_pabcd_isr(int cpl, void *dev_id)
{
	pr_debug("[%s][%d]: GPIO PABCD ISR\n", __FUNCTION__, __LINE__);
	bsp_gpio_irq_demux(BSP_GPIO_PABCD_IRQ);

	return IRQ_HANDLED;
}

irqreturn_t bsp_gpio_pefgh_isr(int cpl, void *dev_id)
{
	pr_debug("[%s][%d]: GPIO PEFGH ISR\n", __FUNCTION__, __LINE__);
	bsp_gpio_irq_demux(BSP_GPIO_PEFGH_IRQ);

	return IRQ_HANDLED;
}

static void bsp_gpio_irq_mask(struct irq_data *data)
{
	unsigned int irq = data->irq;
	unsigned int mask = __raw_readl((void __iomem*)(
			BSP_GPIO_IMR_REG(
			BSP_IRQ_TO_GPIO(irq))));

	gpio_pin_int_masks[BSP_IRQ_TO_GPIO(irq)] = (mask >> BSP_GPIO_2BITS(BSP_IRQ_TO_GPIO(irq))) & 0x3;
	pr_debug("[%s][%d]: mask=0x%x data=%d\n"
			, __FUNCTION__, __LINE__, mask
			, gpio_pin_int_masks[BSP_IRQ_TO_GPIO(irq)]);

	mask &= ~(0x3 << BSP_GPIO_2BITS(BSP_IRQ_TO_GPIO(irq)));
	__raw_writel(mask,
			(void __iomem*)BSP_GPIO_IMR_REG(BSP_IRQ_TO_GPIO(irq)));
}

static void bsp_gpio_irq_unmask(struct irq_data *data)
{
	unsigned int irq = data->irq;
	unsigned int mask = __raw_readl((void __iomem*)(
			BSP_GPIO_IMR_REG(
			BSP_IRQ_TO_GPIO(irq))));

	pr_debug("[%s][%d]: mask=0x%x data=%d\n"
			, __FUNCTION__, __LINE__
			, mask, gpio_pin_int_masks[BSP_IRQ_TO_GPIO(irq)]);

	mask |= (gpio_pin_int_masks[BSP_IRQ_TO_GPIO(irq)] << BSP_IRQ_TO_GPIO(irq));
	__raw_writel(mask,
			(void __iomem*)BSP_GPIO_IMR_REG(BSP_IRQ_TO_GPIO(irq)));
}

static void bsp_gpio_irq_ack(struct irq_data *data)
{
	unsigned int irq = data->irq;

	pr_debug("[%s][%d]: ack 0x%x to 0x%lx bit %d\n",
			__FUNCTION__, __LINE__,
			0x1 << BSP_GPIO_BIT(BSP_IRQ_TO_GPIO(irq)),
			BSP_GPIO_ISR_REG(BSP_IRQ_TO_GPIO(irq)),
			BSP_GPIO_BIT(BSP_IRQ_TO_GPIO(irq)));
	__raw_writel(0x1 << BSP_GPIO_BIT(BSP_IRQ_TO_GPIO(irq))
			, (void __iomem*)BSP_GPIO_ISR_REG(BSP_IRQ_TO_GPIO(irq)));
}

static void bsp_gpio_irq_maskack(struct irq_data *data)
{
	bsp_gpio_irq_mask(data);
	bsp_gpio_irq_ack(data);
}

static int bsp_gpio_irq_set_type(struct irq_data *data, unsigned int type)
{
	int pin = BSP_IRQ_TO_GPIO(data->irq);
	unsigned int newvalue = 0;
	unsigned int mask = __raw_readl((void __iomem*)(
			BSP_GPIO_IMR_REG(
			BSP_IRQ_TO_GPIO(data->irq))));

	switch(type) {
	case IRQ_TYPE_EDGE_RISING:
		newvalue = 0x1;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		newvalue = 0x2;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		newvalue = 0x3;
		break;
	default:
		printk(KERN_ERR "No such irq type %d", type);
		return -EINVAL;
	}

	gpio_pin_int_masks[pin] = newvalue;
	mask |= (newvalue << BSP_GPIO_2BITS(pin));
	__raw_writel(mask,
			(void __iomem*)BSP_GPIO_IMR_REG(pin));
	return 0;
}

static struct irq_chip bsp_gpio_irq = {
	.name		= "GPIO",
	.irq_mask	= bsp_gpio_irq_mask,
	.irq_unmask	= bsp_gpio_irq_unmask,
	.irq_mask_ack	= bsp_gpio_irq_maskack,
	.irq_ack	= bsp_gpio_irq_ack,
	.irq_set_type	= bsp_gpio_irq_set_type,
	.flags		= IRQCHIP_SET_TYPE_MASKED,
};

static struct irqaction irq_gpio_cascade1 = {
	.handler	= bsp_gpio_pabcd_isr,
	.name 		= "gpio cascade1",
};

static struct irqaction irq_gpio_cascade2 = {
	.handler	= bsp_gpio_pefgh_isr,
	.name 		= "gpio cascade2",
};

void bsp_gpio_irq_init(int irq_base)
{
	int irq;

	printk("Realtek GPIO IRQ init\n");

	for(irq = irq_base;
			irq < irq_base + BSP_GPIO_PIN_MAX; irq++)
		irq_set_chip_and_handler(irq, &bsp_gpio_irq, handle_edge_irq);

	setup_irq(BSP_GPIO_PABCD_IRQ, &irq_gpio_cascade1);
	setup_irq(BSP_GPIO_PEFGH_IRQ, &irq_gpio_cascade2);
}
