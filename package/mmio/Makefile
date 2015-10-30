#
# Copyright (C) 2012 Žilvinas Valinskas
# See LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mmio
PKG_VERSION:=2012-10-14
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/8devices/mmio.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=30618dacfd07fd1050496e8a896c84bb201a1c14
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/mmio
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Memory Mapped I/O
  MAKE_PATH:=src
endef

define Package/mmio/description
Memory Mapped I/O utility
endef

define Package/mmio/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(MAKE_PATH)/mmio $(1)/usr/sbin/mmio
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(MAKE_PATH)/rtscts $(1)/usr/sbin/rtscts
endef

$(eval $(call BuildPackage,mmio))
