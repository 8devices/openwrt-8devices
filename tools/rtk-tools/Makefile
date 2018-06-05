#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtk-tools
PKG_VERSION:=0.1

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/${PKG_NAME}-$(PKG_VERSION)
HOST_CFLAGS += -DCOMPACT_FILENAME_BUFFER -DHOME_GATEWAY

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_8196C src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl8196c
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_8196C src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl8196c
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_8196E src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl8196e
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_8196E src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl8196e
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl819xd
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl819xd
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl819xdt
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl819xdt
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl89xxd
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl8198c
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl8954e
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl8197f
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl89xxd
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl8198c
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl8954e
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_819XD src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl8197f
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_8881A src/cvimg.c -o $(HOST_BUILD_DIR)/cvimg-rtl8881a
	$(HOSTCC) $(HOST_CFLAGS) -DCONFIG_RTL_8881A src/mgbin.c -o $(HOST_BUILD_DIR)/mgbin-rtl8881a
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/cvimg* $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mgbin* $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/cvimg*
	rm -f $(STAGING_DIR_HOST)/bin/mgbin*
endef

$(eval $(call HostBuild))
