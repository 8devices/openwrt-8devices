#
# Makefile extension for gpsd
#

define gpsd_installdev_append
	$(CP) $(PKG_BUILD_DIR)/gpsdclient.h $(1)/usr/include/
endef

Build/InstallDev += $(newline)$(gpsd_installdev_append)
