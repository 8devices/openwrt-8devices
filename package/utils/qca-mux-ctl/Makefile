include $(TOPDIR)/rules.mk

PKG_NAME:=qca-mux-ctl
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/qca-mux-ctl
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Qualcomm Atheros SoC GPIO MUX configuration utility
  DEPENDS:=+jansson
endef

define Package/qca-mux-ctl/conffiles
/etc/qca_mux_conf.json
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

define Package/qca-mux-ctl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qca_mux_ctl $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/usr/share/qca-mux-ctl/examples/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/json\ examples/* $(1)/usr/share/qca-mux-ctl/examples/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/json\ examples/empty-qca-mux.json $(1)/etc/qca_mux_conf.json
endef

$(eval $(call BuildPackage,qca-mux-ctl))
