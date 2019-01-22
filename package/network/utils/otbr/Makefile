#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_SOURCE_VERSION:=9451b53

PKG_NAME:=otbr
PKG_VERSION:=2018-11-13-$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/openthread/borderrouter.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_FIXUP:=gettext-version
PKG_FIXUP:=patch-libtool
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/otbr
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Open Thread Border Router
  URL:=http://openthread.io
  DEPENDS:= +libc +libdbus +libcoap +libjsoncpp +boost-filesystem +libmbedtls +libavahi-client +tayga +wpantund
endef

TARGET_CFLAGS += -D_GNU_SOURCE
CONFIGURE_ARGS += --disable-tests --enable-shared=no --enable-static=no --disable-web-service --with-sysroot

define Build/Configure
  $(call Build/Configure/Default)
endef

define Build/Install
  $(call Build/Install/Default)
endef

define Package/otbr/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/etc/dbus-1	
	$(INSTALL_DIR) $(1)/etc/default
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) -r $(PKG_INSTALL_DIR)/usr/* $(1)/usr
	$(CP) -r $(PKG_INSTALL_DIR)/etc/dbus-1/* $(1)/etc/dbus-1
	$(CP) -r $(PKG_INSTALL_DIR)/etc/default/* $(1)/etc/default
	$(CP) -r ./files/init.d/* $(1)/etc/init.d
endef

$(eval $(call BuildPackage,otbr))
