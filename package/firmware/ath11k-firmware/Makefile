include $(TOPDIR)/rules.mk

PKG_NAME:=ath11k-firmware
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/kvalo/ath11k-firmware.git
PKG_SOURCE_VERSION:=9805f61372fcf9fbc678964e7f5546105e8240ad
PKG_SOURCE_DATE:=2021-02-11

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

include $(INCLUDE_DIR)/package.mk

define Package/ath11k-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=
endef

define Package/ath11k-firmware-ipq60xx
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for IPQ60xx devices
endef

define Package/ath11k-firmware-ipq807x
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for IPQ807x devices
  DEPENDS:=@TARGET_ipq807x
endef

define Package/ath11k-firmware-ipq60xx/description
Standard ath11k firmware for IPQ60xx from QCA
endef

define Package/ath11k-firmware-ipq807x/description
Standard ath11k firmware for IPQ807x from QCA
endef

define Build/Compile

endef

define Package/ath11k-firmware-ipq60xx/install
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ6018
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/IPQ6018/hw1.0/testing/2.4.0.1.r2/WLAN.HK.2.4.0.1.r2-00012-QCAHKSWPL_SILICONZ-1/* \
		$(1)/lib/firmware/IPQ6018
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/IPQ6018/hw1.0
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/IPQ6018/hw1.0/caldata.bin \
#		$(1)/lib/firmware/ath11k/IPQ6018/hw1.0
	$(INSTALL_DATA) ./files/board-2-mango.bin \
		$(1)/lib/firmware/ath11k/IPQ6018/hw1.0/board-2.bin
	$(INSTALL_DATA) ./files/qdss_trace_config.bin \
		$(1)/lib/firmware/ath11k/IPQ6018/hw1.0/qdss_trace_config.bin
endef

define Package/ath11k-firmware-ipq807x/install
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ8074
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/IPQ8074/hw2.0/2.1.0.1/WLAN.HK.2.1.0.1-01238-QCAHKSWPL_SILICONZ-2/* \
		$(1)/lib/firmware/IPQ8074
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/IPQ8074/hw2.0/board-2.bin \
#		$(1)/lib/firmware/IPQ8074
endef

$(eval $(call BuildPackage,ath11k-firmware-ipq60xx))
$(eval $(call BuildPackage,ath11k-firmware-ipq807x))
