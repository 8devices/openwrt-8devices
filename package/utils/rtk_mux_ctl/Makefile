include $(TOPDIR)/rules.mk

PKG_NAME:=rtk-mux-ctl
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/rtk-mux-ctl
SECTION:=utils
CATEGORY:=Utilities
TITLE:=Realtek SoC GPIO MUX configuration utility
DEPENDS:=+jansson
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/rtk-mux-ctl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtk_mux_ctl $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/share/rtk-mux-ctl/examples/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/examples/empty-rtk-mux.json $(1)/usr/share/rtk-mux-ctl/examples/
	$(INSTALL_DIR) $(1)/usr/share/rtk-mux-ctl/examples/komikan/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/examples/komikan/* $(1)/usr/share/rtk-mux-ctl/examples/komikan/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/examples/empty-rtk-mux.json $(1)/etc/rtk_mux_conf.json
endef

$(eval $(call BuildPackage,rtk-mux-ctl))
