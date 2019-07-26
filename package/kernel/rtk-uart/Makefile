include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtk-$(BUILD_VARIANT)
PKG_RELEASE:=v1.1
PKG_LICENSE:=GPL-2.0
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtk-uart-template
  SUBMENU:=Other modules
  TITLE:=Realtek $(1) driver
  VARIANT:=$(1)
  DEPENDS:=
  FILES:=$(PKG_BUILD_DIR)/rtk_$(1).ko
  AUTOLOAD:=
  KCONFIG:=
endef

define KernelPackage/rtk-uart1/description
  Kernel module for Realtek serial driver $(1)

  NOTE. Before probing need define pinmux pins
  You can put defines in target/linux/rtkmipsel/files/arch/mips/rtl8197f/mach-*
  or using rtk_mux_ctl app (examples in /usr/share/rtk-mux-ctl/examples/)
endef

KernelPackage/rtk-uart1=$(call KernelPackage/rtk-uart-template,uart1)
KernelPackage/rtk-uart2=$(call KernelPackage/rtk-uart-template,uart2)

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
		BUILD_VARIANT=$(BUILD_VARIANT) \
                modules
endef

$(eval $(call KernelPackage,rtk-uart1))
$(eval $(call KernelPackage,rtk-uart2))
