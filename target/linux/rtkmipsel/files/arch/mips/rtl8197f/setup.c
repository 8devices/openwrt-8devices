/*
 * Realtek Semiconductor Corp.
 *
 * bsp/setup.c
 *     bsp setup code
 *
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 */
#include <linux/console.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

#include <asm/addrspace.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/time.h>
#include <asm/reboot.h>

#include "bspchip.h"

static void plat_machine_restart(char *command)
{
	REG32(BSP_GIMR)=0;
	REG32(BSP_GIMR2) = 0;
	local_irq_disable();
	REG32(BSP_WDTCNR) = 0; //enable watch dog
	while(1);
}

void bsp_reboot(void)
{
        REG32(BSP_GIMR)=0;
	REG32(BSP_GIMR2) = 0;
        local_irq_disable();
        REG32(BSP_WDTCNR) = 0; //enable watch dog
        printk("System halted.\n");
        while(1);
}


static void plat_machine_halt(void)
{
	printk("System halted.\n");
	while(1);
}

/* callback function */
//void __init bsp_setup(void)
void __init plat_mem_setup(void) // mips-ori
{
	/* define io/mem region */
	ioport_resource.start = 0x18000000;
	ioport_resource.end = 0x1fffffff;

	iomem_resource.start = 0x10000000;
	iomem_resource.end = 0x1fffffff;

	/* set reset vectors */
	_machine_restart = plat_machine_restart;
	_machine_halt = plat_machine_halt;
	pm_power_off = plat_machine_halt;
}
#ifdef CONFIG_RTL819X_WDT
#include <linux/platform_device.h>
#define PADDR(addr)  ((addr) & 0x1FFFFFFF)
static int __init  rtl819x_register_wdt(void)
{
        struct resource res;

        memset(&res, 0, sizeof(res));

        res.flags = IORESOURCE_MEM;
        res.start = PADDR(BSP_CDBR); // wdt clock control
        res.end = res.start + 0x8 - 1;
         platform_device_register_simple("rtl819x-wdt", -1, &res, 1);
         return 0;

}
arch_initcall(rtl819x_register_wdt);
#endif
