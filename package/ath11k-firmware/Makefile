include $(TOPDIR)/rules.mk

PKG_NAME:=ath11k-firmware
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/kvalo/ath11k-firmware.git
PKG_SOURCE_VERSION:=0497fb0c84eeb90182e729aedf6f741c26b878ce
PKG_SOURCE_DATE:=2022-05-06
PKG_CONFIG_DEPENDS := CONFIG_FORCE_6G_PINEAPPLE_BOARDDATA
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

define Package/ath11k-firmware-qcn90xx
$(Package/ath11k-firmware-default)
  TITLE:=ath11k firmware for QCN90xx devices
endef

define Package/ath11k-firmware-ipq60xx/description
Standard ath11k firmware for IPQ60xx from QCA
endef

define Package/ath11k-firmware-ipq807x/description
Standard ath11k firmware for IPQ807x from QCA
endef

define Package/ath11k-firmware-qcn90xx/description
Standard ath11k firmware for QCN90xx from QCA
endef

define Package/ath11k-firmware-qcn90xx/config
	config FORCE_6G_PINEAPPLE_BOARDDATA
		bool "Force 6GHz board data for 8devices Pineapple"
		depends on PACKAGE_ath11k-firmware-qcn90xx
		default n
endef

define Build/Compile

endef

define Package/ath11k-firmware-ipq60xx/install
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ6018
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/IPQ6018/hw1.0/2.4.0.1/WLAN.HK.2.4.0.1-01746-QCAHKSWPL_SILICONZ-1/* \
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

define Package/ath11k-firmware-qcn90xx/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/QCN9074/hw1.0
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/QCN9074/hw1.0/2.5.0.1/WLAN.HK.2.5.0.1-01208-QCAHKSWPL_SILICONZ-1/* \
		$(1)/lib/firmware/ath11k/QCN9074/hw1.0
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/QCN9074/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath11k/QCN9074/hw1.0/

	[ "x$(CONFIG_FORCE_6G_PINEAPPLE_BOARDDATA)" == "xy" ] && $(INSTALL_DATA) ./files/board-2-pineapple-6g.bin \
		$(1)/lib/firmware/ath11k/QCN9074/hw1.0/board-2.bin || true
endef

$(eval $(call BuildPackage,ath11k-firmware-ipq60xx))
$(eval $(call BuildPackage,ath11k-firmware-ipq807x))
$(eval $(call BuildPackage,ath11k-firmware-qcn90xx))
