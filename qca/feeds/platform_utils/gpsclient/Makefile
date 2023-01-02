include $(TOPDIR)/rules.mk

PKG_NAME:=gpsclient
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/gpsclient
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=GPS CLIENT APPLICATION
  DEPENDS:=@TARGET_ipq95xx +libgps
 endef

define Package/gpsclient/description
 This package contains a sample client application for parsing gps data
endef

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib -lgps

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
			CC="$(TARGET_CC)" \
			CFLAGS="$(TARGET_CFLAGS) -Wall" \
			LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/gpsclient/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/gpsclient $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,gpsclient))
