#
# Copyright (C) 2012 Žilvinas Valinskas
# See LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mmio
PKG_RELEASE:=2

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/mmio
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Memory Mapped I/O
endef

define Package/mmio/description
Memory Mapped I/O utility
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	make -C $(PKG_BUILD_DIR)		\
		$(TARGET_CONFIGURE_OPTS)	\
		CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)"	\
		LIBS="$(TARGET_LDFLAGS)"
endef

define Package/mmio/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mmio $(1)/usr/sbin/mmio
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtscts $(1)/usr/sbin/rtscts
endef

$(eval $(call BuildPackage,mmio))
