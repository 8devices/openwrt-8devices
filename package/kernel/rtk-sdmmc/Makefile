include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtk-sdmmc
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtk-sdmmc
  SUBMENU:=Other modules
  TITLE:=Realtek SD card driver
  DEPENDS:=@!KERNEL_MMC +kmod-mmc
  FILES:=$(LINUX_DIR)/drivers/mmc/host/rtk-sdmmc.ko
  AUTOLOAD:=$(call AutoLoad,30,rtk-sdmmc,1)
  KCONFIG:=CONFIG_MMC_RTK_SDMMC=m
endef

define KernelPackage/rtk-sdmmc/description
  Kernel module for Realtek sd/mmc driver
endef

define Build/Compile
endef

$(eval $(call KernelPackage,rtk-sdmmc))

