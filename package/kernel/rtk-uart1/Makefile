include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtk-uart1
PKG_RELEASE:=v1.0
PKG_LICENSE:=GPL-2.0
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtk-uart1
  SUBMENU:=Other modules
  TITLE:=Realtek uart1 driver
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/rtk_uart1.ko
  AUTOLOAD:=
  KCONFIG:=
endef

define KernelPackage/rtk-uart1/description
  Kernel module for Realtek serial driver uart1
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef



MAKE_OPTS:= \
        $(KERNEL_MAKE_FLAGS) \
        SUBDIRS="$(PKG_BUILD_DIR)" \
        EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
        $(EXTRA_KCONFIG)

define Build/Compile
        $(MAKE) -C "$(LINUX_DIR)" \
                $(MAKE_OPTS) \
                modules
endef

$(eval $(call KernelPackage,rtk-uart1))
