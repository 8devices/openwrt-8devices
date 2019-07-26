#ifndef _RTL8197_SPI
#define _RTL8197_SPI

#ifdef CONFIG_RTL819X_DW_SPI0
extern struct platform_device rtl819x_device_spi0;
#endif

#ifdef CONFIG_RTL819X_DW_SPI1
extern struct platform_device rtl819x_device_spi1;
#endif

#endif
