# Realtek Semiconductor Corp.
#
# bsp/Makefile
#     bsp Makefile
#
# Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
#


# mips-ori
#core-y   += arch/mips/bsp/
#cflags-y += -Iinclude/asm-mips -Iarch/mips/bsp/
#load-y   += 0x80000000

obj-y := prom.o setup.o irq.o timer.o serial.o clk.o platform.o
obj-$(CONFIG_PCI) += pci-fixup.o pci-ops.o
#obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_RTL_819X) += pci.o
obj-$(CONFIG_USB) += dev_usb.o
obj-$(CONFIG_SPI_SHEIPA) += mtd.o
#obj-$(CONFIG_RTL819X_GPIO) += gpio.o
obj-$(CONFIG_GPIOLIB) += gpio.o dev-gpio-buttons.o
obj-$(CONFIG_LEDS_GPIO) += dev_leds_gpio.o
obj-$(CONFIG_RTL819X_DW_SPI)  += spi.o
obj-$(CONFIG_I2C_DESIGNWARE_PLATFORM)  += i2c.o

#
# Machines
#
obj-$(CONFIG_RTL8197_MACH_GENERIC)	+= mach_rtl819x.o
obj-$(CONFIG_RTL8197_MACH_KINKAN)	+= mach-kinkan.o
obj-$(CONFIG_RTL8197_MACH_KOMIKAN)	+= mach-komikan.o
