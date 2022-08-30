include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=nss-udp-st-drv
LOCAL_SRC:=$(TOPDIR)/qca/src/nss-speedtest/nss-udp-st-drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_VERSION:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
endif

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@TARGET_ipq50xx||TARGET_ipq95xx
  TITLE:=Kernel driver for NSS UDP Speedtest
  FILES:=$(PKG_BUILD_DIR)/nss-udp-st.ko
endef

define KernelPackage/$(PKG_NAME)/Description
Kernel driver for host data path NSS UDP speedtest
endef

define KernelPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/nss-udp-st-drv
	$(CP) $(PKG_BUILD_DIR)/exports/* $(STAGING_DIR)/usr/include/nss-udp-st-drv/
endef

EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/nss-udp-st-drv/

define Build/Compile
	$(MAKE) $(EXTRA_CFLAGS) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
