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
#ifdef CONFIG_SERIAL_RTL_UART1
#if defined(CONFIG_RTL_8197F)
	REG32(0xb8000014) |= (1<<7);//REG_CLK_MANAGE2 	active UART1


	#ifdef CONFIG_SERIAL_RTL_UART1_PINMUX1
	//0x800h [31:28]-> TXD0 (U1_RTS): 4
	REG32(0xb8000800) = (REG32(0xb8000800) & (0x0FFFFFFF))|(0x4<<28);
	//0x808h [27:24]-> TXC (U1_RX): 3
	//0x808h [23:20]-> TXCTL (U1_TX): 3
	//0x808h [15:12]-> RXCTL (U1_CTS): 4
	REG32(0xb8000808) = (REG32(0xb8000808) & (0xF00F0FFF))|(0x3<<24)|(0x3<<20)|(0x4<<12);

	#elif defined(CONFIG_SERIAL_RTL_UART1_PINMUX2)
        // UART1 pinmux set2
        // REG_PINMUX_15 (83Ch):
        // [31:28] MMC_D0 =2
        // [27:24] MMC_D1 =2
        // [23:20] MMC_D2 =2
        // [19:16] MMC_D3 =2
	REG32(0xb800083C) = (REG32(0xb800083C) & ~(0xFFFF0000))|(0x22220000);//MMC pin mux to UART1

	#endif


#if defined(CONFIG_BT_HCIUART_RTKH5) || defined(CONFIG_BT_HCIUART_3WIRE)
	REG32(0xb800083C) &= ~(0xF<<8);
	REG32(0xb800083C) |= (6<<8);//MMC pin mux D6 for GPIO
	REG32(0xb8003508) |= (1<<30);
	REG32(0xb800350C) &= ~(1<<30);//pull to low reset BT
	msleep(100);
	REG32(0xb800350C) |= (1<<30);//pull high D6 to enable BT
#endif
#endif

	up.line = 1;
	up.type = PORT_16550A;
	up.uartclk = BSP_UART1_FREQ;
	up.fifosize = 16;
	up.irq = BSP_UART1_IRQ;
	up.flags = UPF_SKIP_TEST;
	up.mapbase = BSP_UART1_MAP_BASE;
	up.membase = ioremap_nocache(up.mapbase, BSP_UART1_MAPSIZE);
	up.regshift = 2;
	up.iotype = UPIO_MEM32;
	up.serial_out = dwapb_serial_out;
	up.serial_in = dwapb_serial_in;
	up.handle_irq = dwapb_serial_irq;

	if (early_serial_setup(&up) != 0) {
		panic("bsp_serial_init UART 1 failed!");
	}

#endif


#ifdef CONFIG_SERIAL_RTL_UART2
#if defined(CONFIG_RTL_8197F)
	  REG32(0xb8000014) |= (1<<6); // active UART2


	#ifdef CONFIG_SERIAL_RTL_UART2_PINMUX1
	// UART2 pinmux set1
	// REG_PINMUX_01 (804h):
	// [31:28] PIN_P0_RXD0 = 3
	// [27:24] PIN_P0_RXD1 = 4
	// [23:20] PIN_P0_RXD2 = 4
	// [19:16] PIN_P0_RXD3 = 4
	REG32(0xb8000804) = (REG32(0xb8000804) & (0x0000FFFF)) | (0x34440000);

	#elif defined(CONFIG_SERIAL_RTL_UART2_PINMUX2)
	// UART2 pinmux set2
	// REG_PINMUX_15 (83Ch):
	// [31:28] MMC_D0 =3
	// [27:24] MMC_D1 =3
	// [23:20] MMC_D2 =3
	// [19:16] MMC_D3 =3
        REG32(0xb800083C) = (REG32(0xb800083C) & ~(0xFFFF0000))|(0x33330000);

	#endif
#endif
        up.line = 2;
        up.type = PORT_16550A;
        up.uartclk = BSP_UART2_FREQ;
        up.fifosize = 16;
        up.irq = BSP_UART2_IRQ;
        up.flags = UPF_SKIP_TEST;
        up.mapbase = BSP_UART2_MAP_BASE;
        up.membase = ioremap_nocache(up.mapbase, BSP_UART2_MAPSIZE);
        up.regshift = 2;
        up.iotype = UPIO_MEM32;
        up.serial_out = dwapb_serial_out;
        up.serial_in = dwapb_serial_in;
        up.handle_irq = dwapb_serial_irq;

        if (early_serial_setup(&up) != 0) {
                panic("bsp_serial_init UART 2 failed!");
        }

#endif



	return 0;
}
device_initcall(bsp_serial_init);
