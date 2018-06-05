/*
 *  linux/arch/arm/mach-integrator/lm.c
 *
 *  Copyright (C) 2003 Deep Blue Solutions Ltd, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>

//#include <asm/arch/lm.h>	//cathy
#include "lm.h"

#define to_lm_device(d)	container_of(d, struct lm_device, dev)
#define to_lm_driver(d)	container_of(d, struct lm_driver, driver)

static int lm_match(struct device *dev, struct device_driver *drv)
{
	return 1;
}

static int lm_bus_probe(struct device *dev)
{
	struct lm_device *lmdev = to_lm_device(dev);
	struct lm_driver *lmdrv = to_lm_driver(dev->driver);
	return lmdrv->probe(lmdev);
}

static int lm_bus_remove(struct device *dev)
{
	struct lm_device *lmdev = to_lm_device(dev);
	struct lm_driver *lmdrv = to_lm_driver(dev->driver);

	if (lmdrv->remove)
		lmdrv->remove(lmdev);
	return 0;
}

static struct bus_type lm_bustype = {
	.name		= "logicmodule",
	.match		= lm_match,
	.probe		= lm_bus_probe,
	.remove		= lm_bus_remove,
//	.suspend	= lm_bus_suspend,
//	.resume		= lm_bus_resume,
};

static int __init lm_init(void)
{
//	printk("=> lm_init \n");
	return bus_register(&lm_bustype);
}

postcore_initcall(lm_init);

int lm_driver_register(struct lm_driver *drv)
{
	drv->driver.bus = &lm_bustype;
	return driver_register(&drv->driver);
}

void lm_driver_unregister(struct lm_driver *drv)
{
	driver_unregister(&drv->driver);
}

void lm_device_release(struct device *dev)
{
	struct lm_device *d = to_lm_device(dev);
	//printk("dev=%x, d=%x, d->dev=%x\n", dev, d, &d->dev);
	device_unregister(&d->dev); //wei add
	//printk("lm_device_release\n");
	kfree(d);
}

int lm_device_register(struct lm_device *dev)
{
	int ret;

//	dev->dev.release = lm_device_release;
	dev->dev.bus = &lm_bustype;
//#ifdef DWC_HOST_ONLY
if(gHostMode==1)
{
	/* Ethan: internal dma */
	dev->dev.dma_mask = (void *)~0;
	dev->dev.coherent_dma_mask = ~0;
}
//#endif

//	snprintf(dev->dev.bus_id, sizeof(dev->dev.bus_id), "lm%d", dev->id);   //wei del
//	dev->resource.name = dev->dev.bus_id;     //wei del
	dev->resource.name = &lm_bustype.name;     //wei del
	dev->dev.init_name= lm_bustype.name; //wei add, very import, kernel object is need in our kernel
#if 0
	ret = request_resource(&iomem_resource, &dev->resource);
	if (ret == 0)
#endif
	{
		ret = device_register(&dev->dev);
		printk("device_register :");
		if (ret)
		{	printk("register fail");
			release_resource(&dev->resource);
		}
		else
		{
			printk("register pass\n" );
		}
	}
#if 0
	else
	{	printk("request_resource fail\n" );
	}
#endif
	return ret;
}

EXPORT_SYMBOL(lm_driver_register);
EXPORT_SYMBOL(lm_driver_unregister);


//==========================================================
#if 0
void dwdump_swap(unsigned char * pData, int count)
{
	unsigned int *sbuf = pData;
	unsigned int tmp;
	volatile unsigned char *p=&tmp;
	int length=count;  //is word unit

	//dprintf("Addr=%x, len=%d", sbuf, length);
	printk(" [Addr]    .3.2.1.0    .7.6.5.4    .B.A.9.8    .F.E.D.C [SWAP]" );

	{
		int i;
		for(i=0;i<length; i++)
		{
			if((i%4)==0)
			{	printk("\n\r");
				printk("%08X:  ", (sbuf+i) );
			}
			tmp=sbuf[i];
			//dprintf("%02X%02X%02X%02X    ", p[3], p[2], p[1], p[0] );
			printk("%08X    ",  (p[3]<<24) |  (p[2]<<16) | (p[1]<<8)  | p[0] );


		}
		printk("\n\r");
	}
}
#endif
