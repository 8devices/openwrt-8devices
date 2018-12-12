#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/komikan
  NAME:=Komikan Profile
  PRIORITY:=1
  PACKAGES:=-wpad-mini
endef

define Profile/komikan/Description
	Default package set compatible with 8devices Komikan devboard
endef
$(eval $(call Profile,komikan))
