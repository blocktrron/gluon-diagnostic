include $(TOPDIR)/rules.mk

PKG_NAME:=gluon-diagnostic
PKG_VERSION:=1
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-defaults.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(CURDIR)/../../src $(PKG_BUILD_DIR)/
endef

MAKE_PATH:=src
MAKE_VARS += \
        NL_LIB="libnl-tiny" \
        NL_GENL_LIB="libnl-tiny"

define Package/gluon-diagnostic
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libubox +libubus +libblobmsg-json +libnl-tiny +libbatadv +libgluonutil
  TITLE:=Daemon for Gluon diagnostic using 802.11 beacon frames
endef

define Package/gluon-diagnostic/conffiles
/etc/config/gluon-diagnostic
endef

define Package/gluon-diagnostic/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/etc/config
	$(CP) $(PKG_BUILD_DIR)/src/gluon-diagnostic $(1)/usr/bin
	$(INSTALL_BIN) ./files/gluon-diagnostic.init $(1)/etc/init.d/gluon-diagnostic
	$(CP) ./files/gluon-diagnostic.uci $(1)/etc/config/gluon-diagnostic
endef

$(eval $(call BuildPackage,gluon-diagnostic))
