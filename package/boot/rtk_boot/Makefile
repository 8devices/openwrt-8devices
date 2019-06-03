include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtk_boot
PKG_VERSION:=v2.4
PKG_RELEASE:=1

PKG_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/rtk_boot
  SECTION:=Boot Loaders
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_rtkmipsel_rtl8197f
  TITLE:=Realtek bootloader (binary package)
endef

define Package/rtk_boot/description
Realtek bootloader
Tested with 8devices rtl8197fs based boards: kinkan, komikan
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/rtk_boot/install
	$(INSTALL_DIR) $(BIN_DIR)/sdboot/
	$(CP) ./files/rtl8197f_sdboot_$(PKG_VERSION).bin $(BIN_DIR)/sdboot/boot.bin
	$(CP) ./files/rtl8197f_sdboot_$(PKG_VERSION).img $(BIN_DIR)/sdboot/boot.img
endef

$(eval $(call BuildPackage,rtk_boot))

