include $(TOPDIR)/rules.mk

PKG:=nss-udp-st
PKG_NAME:=$(PKG)
PKG_RELEASE:=1

LOCAL_SRC:=$(TOPDIR)/qca/src/nss-speedtest/nss-udp-st/
include $(INCLUDE_DIR)/local-development.mk

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=QTI
  CATEGORY:=$(QTI_SOFTWARE_CATEGORY)
  URL:=http://www.qualcomm.com
  MAINTAINER:=Qualcomm Technologies, Inc.
  TITLE:= NSS UDP SpeedTest
  DEPENDS:= @TARGET_ipq50xx||TARGET_ipq95xx +kmod-nss-udp-st-drv
endef

define Package/$(PKG_NAME)/description/Default
	A userspace utility for host data path nss udp speedtest
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/nss-udp-st-drv/

define Build/Compile
$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nss-udp-st $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG)))
