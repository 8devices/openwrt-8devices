/*
 * Realtek Semiconductor Corp.
 *
 * bsp/serial.c
 *     BSP serial port initialization
 *
 * Copyright 2006-2012 Tony Wu (tonywu@realtek.com)
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <linux/tty.h>
#include <linux/irq.h>
#include <linux/delay.h>

#include <asm/serial.h>

#include "bspchip.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
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
#endif

int __init bsp_serial_init(void)
{
	struct uart_port up;

	/* clear memory */
	memset(&up, 0, sizeof(up));

	/*
	 * UART0
	 */
	up.line = 0;
	up.type = PORT_16550A;
	up.uartclk = BSP_UART0_FREQ;
	up.fifosize = 16;
	up.irq = BSP_UART0_IRQ;
	up.flags = UPF_SKIP_TEST;
	up.mapbase = BSP_UART0_MAP_BASE;
	up.membase = ioremap_nocache(up.mapbase, BSP_UART0_MAPSIZE);
	up.regshift = 2;
	up.iotype = UPIO_MEM32;
	up.serial_out = dwapb_serial_out;
	up.serial_in = dwapb_serial_in;
	up.handle_irq = dwapb_serial_irq;

	if (early_serial_setup(&up) != 0) {
		panic("bsp_serial_init failed!");
	}
	return 0;
}
device_initcall(bsp_serial_init);
