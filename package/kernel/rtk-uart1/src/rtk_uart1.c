#include <linux/delay.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/slab.h>

#include <asm/serial.h>

#include "bspchip.h"

unsigned int last_lcr;

unsigned int dwapb_serial_in(struct uart_port *p, int offset)
{
	offset <<= p->regshift;
	return readl(p->membase + offset);
}

void dwapb_serial_out(struct uart_port *p, int offset, int value)
{
	/* Save the LCR value so it can be re-written when a
	 * Busy Detect interrupt occurs. */
	if (offset == UART_LCR)
		last_lcr = value;

	writel(value, p->membase + (offset << p->regshift));
}

#define UART_USR	0x1f

static int dwapb_serial_irq(struct uart_port *p)
{
	unsigned int iir = readl(p->membase + (UART_IIR << p->regshift));

	if (serial8250_handle_irq(p, iir)) {
		return IRQ_HANDLED;
	} else if ((iir & UART_IIR_BUSY) == UART_IIR_BUSY) {
		/*
		 * The DesignWare APB UART has an Busy Detect (0x07) interrupt
		 * meaning an LCR write attempt occurred while the UART was
		 * busy. The interrupt must be cleared by reading the UART
		 * status register (USR) and the LCR re-written.
		 */
		(void)readl(p->membase + (UART_USR << p->regshift));
		writel(last_lcr, p->membase + (UART_LCR << p->regshift));
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static int rtk_uart1_probe(struct platform_device *pdev)
{
	struct uart_8250_port uart = {};
	struct uart_port *up = &uart.port;


	printk("probing rtk uart1\n");
	spin_lock_init(&up->lock);

	//disable UART1 irq
	REG32(BSP_IRR4) &= ~(0xf << 24);

	//active UART1
	REG32(BSP_CLK_MANAGE2) |= (1 << 7);

	up->line = 1;
	up->type = PORT_16550A;
	up->uartclk = BSP_UART1_FREQ;
	up->fifosize = 16;
	up->irq = BSP_UART1_IRQ;
	up->flags = UPF_SKIP_TEST;
	up->mapbase = BSP_UART1_MAP_BASE;
	up->membase = ioremap_nocache(up->mapbase, BSP_UART1_MAPSIZE);
	up->regshift = 2;

	up->iotype = UPIO_MEM32;
	up->serial_out = dwapb_serial_out;
	up->serial_in = dwapb_serial_in;
	up->handle_irq = dwapb_serial_irq;

	if (serial8250_register_8250_port(&uart) < 0) {
		printk("bsp_serial_init UART %d failed!", up->line);
		REG32(BSP_CLK_MANAGE2) &= ~(1 << 7);

		return -1;
	}

	//enable UART1 irq
	REG32(BSP_IRR4) |= 0x2 << 24;

	return 0;
}

static int rtk_uart1_remove(struct platform_device *pdev)
{
	REG32(BSP_IRR4) &= ~(0xf << 24);
	serial8250_unregister_port(1);
	REG32(BSP_CLK_MANAGE2) &= ~(1 << 7);

	return 0;
}

static struct platform_driver rtk_uart1_platform_driver = {
	.driver = {
		.name           = "rtk-uart1",
		.owner          = THIS_MODULE,
	},
	.probe                  = rtk_uart1_probe,
	.remove                 = rtk_uart1_remove,
};

module_platform_driver(rtk_uart1_platform_driver);

MODULE_AUTHOR("Rytis Zigmantavicius <rytis.z@8devices.com>");
MODULE_DESCRIPTION("rtk uart1 driver");
MODULE_LICENSE("GPL");
