#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/RAMBUTAN
        NAME:=Rambutan board from 8Devices
        PACKAGES:=kmod-usb-core kmod-usb2
endef

define Profile/RAMBUTAN/Description
        Package set optimized for the 8devices Rambutan board.
endef

$(eval $(call Profile,RAMBUTAN))
