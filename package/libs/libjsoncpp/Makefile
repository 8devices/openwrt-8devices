
include $(TOPDIR)/rules.mk

PKG_SOURCE_VERSION:=21a4185

PKG_NAME:=libjsoncpp
PKG_VERSION:=1.8.4-2019-01-16-$(PKG_SOURCE_VERSION)
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/open-source-parsers/jsoncpp.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_SOURCE_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+=-DCMAKE_BUILD_TYPE=MinSizeRel
CMAKE_OPTIONS+=-DJSONCPP_WITH_TESTS=OFF
CMAKE_OPTIONS+=-DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
CMAKE_OPTIONS+=-DBUILD_SHARED_LIBS=ON

define Package/libjsoncpp
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=implementation of a JSON reader and writer in C++
  DEPENDS:=+libstdcpp
  URL:=https://github.com/open-source-parsers/jsoncpp
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/jsoncpp/json
	$(CP) $(PKG_INSTALL_DIR)/usr/include/json/{autolink,config,features,forwards,json,reader,value,version,writer}.h $(1)/usr/include/jsoncpp/json/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libjsoncpp.so* $(1)/usr/lib
endef


define Package/libjsoncpp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libjsoncpp.so* $(1)/usr/lib
endef


$(eval $(call BuildPackage,libjsoncpp))
