#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
  PACKAGES:=-wpad-mini
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,Default))

define Profile/kinkan
  NAME:=Kinkan Profile
  PRIORITY:=1
  PACKAGES:=-wpad-mini
endef

define Profile/kinkan/Description
	Default package set compatible with 8devices Kinkan devboard
endef
$(eval $(call Profile,kinkan))
