#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/XIAOMI-MIWIFI-MINI
 NAME:=Xiaomi MiWiFi Mini
 PACKAGES:=\
	kmod-usb-core kmod-usb-dwc2 kmod-usb2 kmod-usb-ohci \
	kmod-mt76
endef

define Profile/XIAOMI-MIWIFI-MINI/Description
 Support for Xiaomi MiWiFi Mini routers
endef
$(eval $(call Profile,XIAOMI-MIWIFI-MINI))
