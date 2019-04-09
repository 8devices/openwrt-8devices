#
## Copyright (C) 2006-2009 OpenWrt.org
#
## This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtk_hciattach
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/rtk_hciattach
  SECTION:=Tool
  CATEGORY:=Utilities
  DEPENDS:=+rtk-mux-ctl +kmod-rtk-uart1 +bluez-utils
  TITLE:=Utility for connecting to realtek serial bluetooth
  KCONFIG:=CONFIG_BT=m \
	CONFIG_BT_BNEP=m \
	CONFIG_BT_COEXIST=y \
	CONFIG_BT_COEXIST_SOCKET=y \
	CONFIG_BT_HCIUART=m \
	CONFIG_BT_HCIUART_3WIRE=y \
	CONFIG_BT_RFCOMM=m \
	CONFIG_CRYPTO_ECB=y \
	CONFIG_CRYPTO_MANAGER=y \
	CONFIG_CRYPTO_MANAGER2=y \
	CONFIG_CRYPTO_SHA256=y
endef

define Package/rtk_hciattach/description
  Package for rtk_hciattach tool and bluetooth
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/rtk_hciattach/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rtk_hciattach $(1)/usr/sbin/rtk_hciattach
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/komikan-bt-switch.init $(1)/etc/init.d/komikan-bt-switch
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/bluetooth.config $(1)/etc/config/bluetooth
	$(INSTALL_DIR) $(1)/lib/firmware/rtlbt/
	$(INSTALL_BIN) ./blobs/rtl8822b_config $(1)/lib/firmware/rtlbt/rtl8822b_config
	$(INSTALL_BIN) ./blobs/rtl8822b_fw $(1)/lib/firmware/rtlbt/rtl8822b_fw
endef

$(eval $(call BuildPackage,rtk_hciattach))
