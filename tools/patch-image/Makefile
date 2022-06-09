# 
# Copyright (C) 2007-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=patch-image

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) -include endian.h -o $(HOST_BUILD_DIR)/patch-cmdline src/patch-cmdline.c
	$(HOSTCC) $(HOST_CFLAGS) -include endian.h -o $(HOST_BUILD_DIR)/patch-dtb src/patch-dtb.c
	$(HOSTCC) $(HOST_CFLAGS) -include endian.h -o $(HOST_BUILD_DIR)/patch-info \
		$(foreach src,patch-info sha1 cyg_crc32,src/patch-info/$(src).c)
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/patch-cmdline $(STAGING_DIR_HOST)/bin/
	$(CP) $(HOST_BUILD_DIR)/patch-dtb $(STAGING_DIR_HOST)/bin/
	$(CP) $(HOST_BUILD_DIR)/patch-info $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/patch-cmdline
	rm -f $(STAGING_DIR_HOST)/bin/patch-dtb
	rm -f $(STAGING_DIR_HOST)/bin/patch-info
endef

$(eval $(call HostBuild))
