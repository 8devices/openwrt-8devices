#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <bspchip.h>

static struct resource rtl819x_spi0_resource[] = {
	[0] = DEFINE_RES_MEM(BSP_DW_SSI_0_BASE, BSP_DW_SSI_0_SIZE),
	[1] = DEFINE_RES_IRQ(BSP_DW_SSI_0_IRQ),
};

static struct resource rtl819x_spi1_resource[] = {
	[0] = DEFINE_RES_MEM(BSP_DW_SSI_1_BASE, BSP_DW_SSI_1_SIZE),
	[1] = DEFINE_RES_IRQ(BSP_DW_SSI_1_IRQ),
};

#ifdef CONFIG_RTL819X_DW_SPI0
struct platform_device rtl819x_device_spi0 = {
	.name			= "dw_spi_mmio",
	.id				= 1,
	.num_resources	= ARRAY_SIZE(rtl819x_spi0_resource),
	.resource		= rtl819x_spi0_resource,
};
#endif

#ifdef CONFIG_RTL819X_DW_SPI1
struct platform_device rtl819x_device_spi1 = {
	.name			= "dw_spi_mmio",
	.id				= 2,
	.num_resources	= ARRAY_SIZE(rtl819x_spi1_resource),
	.resource		= rtl819x_spi1_resource,
};
#endif

static struct platform_device __initdata *rtl_spi_devs[] = {
#ifdef CONFIG_RTL819X_DW_SPI0
	&rtl819x_device_spi0,
#endif
#ifdef CONFIG_RTL819X_DW_SPI1
	&rtl819x_device_spi1
#endif
};

static int __init rtl819x_spi_peripheral_init(void)
{
	int err;

	printk("Realtek DW SPI controller init\n");
	err = platform_add_devices(rtl_spi_devs, ARRAY_SIZE(rtl_spi_devs));

	if(err) {
		printk(KERN_ERR "rtl819x_spi: cannot add rtl819x SPI platform device, error=%d\n", err);
		return err;
	}

#ifdef CONFIG_RTL819X_DW_SPI0
	/*Enable SPIC0*/
	__raw_writel(__raw_readl((void __iomem*) BSP_ENABLE_IP) | 0x00000140
			, (void __iomem*)BSP_ENABLE_IP);
	__raw_writel(__raw_readl((void __iomem*) BSP_CLK_MANAGE2) | 0x00010000
			, (void __iomem*)BSP_CLK_MANAGE2);
#endif

#ifdef CONFIG_RTL819X_DW_SPI1
	/*Enable SPIC1*/
	__raw_writel(__raw_readl((void __iomem*) BSP_ENABLE_IP) | 0x000000a0
			, (void __iomem*)BSP_ENABLE_IP);
	__raw_writel(__raw_readl((void __iomem*) BSP_CLK_MANAGE2) | 0x00004000
			, (void __iomem*)BSP_CLK_MANAGE2);
#endif

	return 0;
}
arch_initcall(rtl819x_spi_peripheral_init);
