include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qca-nss-nsm
LOCAL_SRC:=$(TOPDIR)/qca/src/qca-nss-nsm
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
  DEPENDS:=@TARGET_ipq95xx||TARGET_ipq95xx_32 +PACKAGE_kmod-qca-nss-sfe:kmod-qca-nss-sfe
  TITLE:=Network Status Module
  FILES:=$(PKG_BUILD_DIR)/qca-nss-nsm.ko
endef

define KernelPackage/$(PKG_NAME)/Description
Network status module
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/qca-nss-nsm
	$(CP) $(PKG_BUILD_DIR)/exports/* $(STAGING_DIR)/usr/include/qca-nss-nsm/
endef

EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/qca-nss-nsm
EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/qca-nss-sfe

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		modules
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
