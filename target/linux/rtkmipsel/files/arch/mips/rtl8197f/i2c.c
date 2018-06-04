/*Realtek Semiconductor Corp.
 *
 * bsp/i2c.c:
 *     bsp I2C initialization code
 *
 * Copyright (C) 2016 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include "bspchip.h"
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
#include <linux/platform_data/i2c-designware.h>
#endif
MODULE_LICENSE("GPL");

#define DW_IC_CON_SPEED_STD		0x2
#define DW_IC_CON_SPEED_FAST		0x4
#define DW_IC_CON_SPEED_HIGH		0x6


#define rtlRegRead(addr)        \
        (*(volatile u32 *)(addr))

#define rtlRegWrite(addr, val)  \
        ((*(volatile u32 *)(addr)) = (val))

/*
* dw_i2c_dew_info provides Linux 3.10 kernel to set
* data hold time and speed mode of Designware I2C controller.
*
* sda_hold_time_ns: data hold time. Time unit is nanosecond.
* Data hold time should match I2C device spec.
*
* speed_mode: the modes of speed are DW_IC_CON_SPEED_STD, DW_IC_CON_SPEED_FAST
* and DW_IC_CON_SPEED_HIGH.
* If DW_IC_CON_SPEED_STD is standard mode, I2C frequency is about 40KHz.
* If DW_IC_CON_SPEED_FAST is fast mode, I2C frequency is about 390KHz.
* If DW_IC_CON_SPEED_HIGH is high speed mode, I2C frequency is about 1MHz.
* Speec mode should match I2C device spec.
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
struct dw_i2c_dev_info {
        u32                     sda_hold_time_ns;
        u8                      speed_mode;
};
#endif

static struct resource rtl819x_i2c0_resource[] = {
	[0] = DEFINE_RES_MEM(BSP_DW_I2C_0_BASE, BSP_DW_I2C_0_SIZE),
	[1] = DEFINE_RES_IRQ(BSP_DW_I2C_0_IRQ),
};

static struct resource rtl819x_i2c1_resource[] = {
	[0] = DEFINE_RES_MEM(BSP_DW_I2C_1_BASE, BSP_DW_I2C_1_SIZE),
	[1] = DEFINE_RES_IRQ(BSP_DW_I2C_1_IRQ),
};

/*
* Because Designware I2C controller uses ic_clk as unit,
* Designware I2C driver will set sda_hold_time_ns after transform its unit into ic_clk.
* In 8197F setting, the frequency of Designware I2C IP is 100MHz.
* Therefore, ic_clk is 100MHz.
*
* If there are many I2C devices on a I2C bus, suggest using lowest value of maximum data hold time in all devices' spec in order to assure all I2C deviews own enough data hold time to access data.
*
* Example:
* If sda_hold_time_ns is 400 nanosecond, Designware I2C driver will set
* (400 * ic_clk + 500000000) / 1000000000 = 40 ic_clk.
*
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
static struct dw_i2c_platform_data bsp_dw_i2c_info_device0 = {
	.i2c_scl_freq= 100000 ,//100k = Standard mode
	.sda_hold_time_ns = 400
};
#else
static struct dw_i2c_dev_info bsp_dw_i2c_info_device0 = {
	.sda_hold_time_ns = 400,
	.speed_mode = DW_IC_CON_SPEED_STD
};
#endif
/*
* Because Designware I2C controller uses ic_clk as unit,
* Designware I2C driver will set sda_hold_time_ns after transform its unit into ic_clk.
* In 8197F setting, the frequency of Designware I2C IP is 100MHz.
* Therefore, ic_clk is 100MHz.
*
* If there are many I2C devices on a I2C bus, suggest using lowest value of maximum data hold time in all devices' spec in order to assure all I2C deviews own enough data hold time to access data.
*
* Example:
* If sda_hold_time_ns is 400 nanosecond, Designware I2C driver will set
* (400 * ic_clk + 500000000) / 1000000000 = 40 ic_clk.
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
static struct dw_i2c_platform_data bsp_dw_i2c_info_device1 = {
	.i2c_scl_freq= 400000 , //400k = fast mode
	.sda_hold_time_ns = 400
};
#else
static struct dw_i2c_dev_info bsp_dw_i2c_info_device1 = {
        .sda_hold_time_ns = 400,
        .speed_mode = DW_IC_CON_SPEED_FAST
};
#endif
static u64 bsp_i2c_dmamask = 0xFFFFFFFFUL;

struct platform_device bsp_dw_i2c_device0 = {
	.name = "i2c_designware",
	.id = 1,
	.num_resources = ARRAY_SIZE(rtl819x_i2c0_resource),
	.resource = rtl819x_i2c0_resource,
	.dev = {
		.dma_mask = &bsp_i2c_dmamask,
		.coherent_dma_mask = 0xffffffffUL,
		.platform_data = &bsp_dw_i2c_info_device0
	}
};

struct platform_device bsp_dw_i2c_device1 = {
        .name = "i2c_designware",
        .id = 2,
        .num_resources = ARRAY_SIZE(rtl819x_i2c1_resource),
        .resource = rtl819x_i2c1_resource,
        .dev = {
                .dma_mask = &bsp_i2c_dmamask,
                .coherent_dma_mask = 0xffffffffUL,
		.platform_data = &bsp_dw_i2c_info_device1
        }
};


static struct platform_device *bsp_i2c_devs[] __initdata = {
	&bsp_dw_i2c_device0, &bsp_dw_i2c_device1};

static inline u32 rtlRegMask(u32 addr, u32 mask, u32 value)
{
        u32 reg;

        reg = rtlRegRead(addr);
        reg &= ~mask;
        reg |= value & mask;
        rtlRegWrite(addr, reg);
        reg = rtlRegRead(addr); /* flush write to the hardware */

        return reg;
}

static int __init bsp_i2c_init(void)
{
	int ret;

	printk("INFO: initializing i2c devices ...\n");



	ret = platform_add_devices(bsp_i2c_devs, ARRAY_SIZE(bsp_i2c_devs));
	if (ret < 0) {
		printk("ERROR: unable to add devices\n");
		return ret;
	}


#ifdef CONFIG_I2C_DESIGNWARE_PLATFORM


        /*Enable I2C*/
	/* I2C1 is not tested, so its pinmux is marked.*/

	/*default setting to enable I2C function. Default I2C1 is closed, so its pinmux (0x800 and 0x808) is closed here.*/
	#ifdef CONFIG_I2C_DEFAULT_PINMUX

	/*
	* Reg: 0x010
	* Set: [9]=1, [11]=1, [31]=1
	* Description: I2C Enable
	*/
	rtlRegMask(BSP_CLK_MANAGE1, 1<<9 | 1<<11 | 1<<31, 1 << 9 | 1<<11 | 1<<31);

	/*
	* Reg:0x014
	* Set: [4]=1, [8]=1, [10]=1, [12]=1
	* Description: I2C Enable
	*/
	rtlRegMask(BSP_CLK_MANAGE2, 1<<4 | 1<<8 | 1<<10 | 1<<12, 1<<4 | 1<<8 | 1<<10 | 1<<12);

	/*
	* Reg: 0x800
	* Set: PIN_MUX_SEL0[27:24](TXD1)=3
	* Description: For I2C1 setting
	*/
	//rtlRegMask(BSP_PIN_MUX_SEL0, 0xF<<24, 3<<24);

	/*
        * Reg: 0x808
        * Set: PIN_MUX_SEL2[23:20](TXCTL)=4, PIN_MUX_SEL2[15:12](RXCTL)=5
	* Description: PIN_MUX_SEL2[23:20], PIN_MUX_SEL2[15:12] For I2C0.
        */
	rtlRegMask(BSP_PIN_MUX_SEL2, 0xF<<20 | 0xF<<12, 4<<20 | 5<<12);

	/*
        * Reg: 0x808
        * Set: PIN_MUX_SEL2[19:16](RXC)=5
        * Description: PIN_MUX_SEL2[19:16] for I2C1.
        */
        //rtlRegMask(BSP_PIN_MUX_SEL2, 0xF<<16, 5<<16);


	#else
	/*The other setting to enable I2C function, only I2C1 have another choice.*/
	 /*
        * Reg: 0x010
        * Set: [9]=1, [11]=1, [31]=1
	* Description: I2C Enable
        */
        rtlRegMask(BSP_CLK_MANAGE1, 1<<9 | 1<<11 | 1<<31, 1 << 9 | 1<<11 | 1<<31);

	/*
        * Reg:0x014
        * Set: [4]=1, [8]=1, [10]=1, [12]=1
	* Description: I2C Enable
        */
        rtlRegMask(BSP_CLK_MANAGE2, 1<<4 | 1<<8 | 1<<10 | 1<<12, 1<<4 | 1<<8 | 1<<10 | 1<<12);

	/*
        * Reg: 0x800
        * Set: PIN_MUX_SEL0[31:28](TXD0)=5
	* Description: For I2C1 setting
        */
        rtlRegMask(BSP_PIN_MUX_SEL0, 0xF<<28, 5 << 28);

	/*
        * Reg: 0x808
        * Set: PIN_MUX_SEL2[27:24](TXC)=5, PIN_MUX_SEL2[23:20](TXCTL)=4, PIN_MUX_SEL2[15:12](RXCTL)=5
	* Description: PIN_MUX_SEL2[23:20] and PIN_MUX_SEL2[15:12] For I2C0. PIN_MUX_SEL2[27:24] for I2C1.
        */
        rtlRegMask(BSP_PIN_MUX_SEL2, 0xF<<20 | 0xF<<12 | 0xF<<24, 4<<20 | 5<<12 | 5<<24);

	#endif

#endif


	return 0;
}
arch_initcall(bsp_i2c_init);
