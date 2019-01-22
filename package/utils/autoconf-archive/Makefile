include $(TOPDIR)/rules.mk

PKG_NAME:=autoconf-archive
PKG_VERSION:=2016-09-16
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=git://git.sv.gnu.org/autoconf-archive.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=d3d6bb4a4fb7df1508d1470d908dbd81b225a962
PKG_LICENSE:=GNU Free Documentation License 1.3
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/autoconf-archive
$(call Package/autoconf-archive/Default)
  SECTION:=util
  CATEGORY:=Utilities
  DEPENDS:=
  TITLE:=Host build tool, autoconf-archive library.
endef

define Package/autoconf-archive/description
	Collection of m4 scripts.
endef

define Host/Configure
endef

define Host/Compile
endef

define Host/Install
	$(CP) $(BUILD_DIR_HOST)/$(PKG_SOURCE_SUBDIR)/m4/*.m4 $(STAGING_DIR_HOST)/share/aclocal/
endef

define Build/Configure
endef

define Package/autoconf-archive/install
	#optional, make host tool installable by selecting Utilities/autoconf-archive in menuconfig
	$(CP) $(BUILD_DIR_HOST)/$(PKG_SOURCE_SUBDIR)/m4/*.m4 $(STAGING_DIR_HOST)/share/aclocal/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,autoconf-archive))
