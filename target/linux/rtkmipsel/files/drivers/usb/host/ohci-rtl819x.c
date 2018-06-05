/*
#if 1
 *  OHCI HCD (Host Controller Driver) for USB.
 *
 *  Bus Glue for Realtek rtl819x built-in OHCI controller.
 *
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Realtek' 2.6.30 BSP
 *	Copyright (C) 2013 Realtek Semiconductor, Corp.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>

extern int usb_disabled(void);

static int usb_hcd_rtl819x_probe(const struct hc_driver *driver,
				struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	struct ohci_hcd	*ohci;
	int irq;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no IRQ specified for %s\n",
			dev_name(&pdev->dev));
		return -ENODEV;
	}
	irq = res->start;

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no base address specified for %s\n",
			dev_name(&pdev->dev));
		ret = -ENODEV;
		goto err_put_hcd;
	}
	hcd->rsrc_start	= res->start;
	hcd->rsrc_len	= res->end - res->start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		ret = -EBUSY;
		goto err_put_hcd;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		ret = -EFAULT;
		goto err_release_region;
	}

	ohci = hcd_to_ohci(hcd);
	ohci->flags |= OHCI_QUIRK_BE_MMIO | OHCI_QUIRK_BE_DESC;

	ohci_hcd_init(ohci);

	ret = usb_add_hcd(hcd, irq, IRQF_SHARED);

	if (ret)
		goto err_stop_hcd;

	return 0;

 err_stop_hcd:
	iounmap(hcd->regs);
 err_release_region:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
 err_put_hcd:
	usb_put_hcd(hcd);
	return ret;
}

void usb_hcd_rtl819x_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	//rtl8652_stop_hc(&pdev->dev);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

static int /*__devinit*/ ohci_rtl819x_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	int ret;
    unsigned int fminterval;

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;

#if 1
    fminterval = 0x2edf;
    ohci_writel (ohci,(fminterval * 9) / 10, &ohci->regs->periodicstart);
    fminterval |= ((((fminterval - 210) * 6) / 7) << 16);
    ohci_writel (ohci, fminterval, &ohci->regs->fminterval);

    ohci_writel (ohci, 0x0628, &ohci->regs->lsthresh); 		/* default value from datasheet */
    ohci_writel (ohci, 0x3e67, &ohci->regs->periodicstart); /* default value from datasheet */
#endif

	ret = ohci_run(ohci);

	if (ret < 0)
		goto err;

	return 0;

 err:
	ohci_stop(hcd);
	return ret;
}

static const struct hc_driver ohci_rtl819x_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Realtek rtl819x built-in OHCI controller",
	.hcd_priv_size		= sizeof(struct ohci_hcd),

	.irq				= ohci_irq,
	.flags				= HCD_USB11 | HCD_MEMORY,

	.start				= ohci_rtl819x_start,
	.stop				= ohci_stop,
	.shutdown			= ohci_shutdown,

	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number	= ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,

#ifdef CONFIG_PM
    .bus_suspend        = ohci_bus_suspend,
    .bus_resume     	= ohci_bus_resume,
#endif

	.start_port_reset	= ohci_start_port_reset,
};

static int ohci_hcd_rtl819x_drv_probe(struct platform_device *pdev)
{
	if (usb_disabled())
		return -ENODEV;

	return usb_hcd_rtl819x_probe(&ohci_rtl819x_hc_driver, pdev);
}

static int ohci_hcd_rtl819x_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_rtl819x_remove(hcd, pdev);
	return 0;
}

#ifdef CONFIG_PM
static int ohci_hcd_rtl8652_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct usb_hcd *hcd = platform_get_drvdata(pdev);
    struct ohci_hcd *ohci = hcd_to_ohci(hcd);

    if (time_before(jiffies, ohci->next_statechange))
        msleep(5);
    ohci->next_statechange = jiffies;

    //rtl8652_stop_hc(&pdev->dev);
    hcd->state = HC_STATE_SUSPENDED;
    pdev->dev.power.power_state = PMSG_SUSPEND;

    return 0;
}

static int ohci_hcd_rtl8652_drv_resume(struct platform_device *pdev)
{
    struct usb_hcd *hcd = platform_get_drvdata(pdev);
    struct ohci_hcd *ohci = hcd_to_ohci(hcd);
    int status;

    if (time_before(jiffies, ohci->next_statechange))
        msleep(5);
    ohci->next_statechange = jiffies;

    //rtl8652_start_hc(&pdev->dev);
    pdev->dev.power.power_state = PMSG_ON;
    usb_hcd_resume_root_hub(hcd);

    return 0;
}
#endif

MODULE_ALIAS("platform:rtl819x-ohci");

static struct platform_driver ohci_hcd_rtl819x_driver = {
	.probe		= ohci_hcd_rtl819x_drv_probe,
	.remove		= ohci_hcd_rtl819x_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
#ifdef CONFIG_PM
    .suspend    = ohci_hcd_rtl8652_drv_suspend,
    .resume     = ohci_hcd_rtl8652_drv_resume,
#endif
	.driver		= {
		.name	= "rtl819x-ohci",
		.owner	= THIS_MODULE,
	},
};
