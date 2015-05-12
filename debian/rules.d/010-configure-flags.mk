# configure flags

DEB_CONFIGURE_EXTRA_FLAGS := \
	--enable-nls \
	--enable-xft \
	--prefix=/usr \
	--enable-shape \
	--enable-fribidi \
	--enable-xinerama \
	--with-locale=/usr/share/fluxbox/nls \
	--with-apps=$(DEB_CONFIGURE_SYSCONFDIR)/apps \
	--with-keys=$(DEB_CONFIGURE_SYSCONFDIR)/keys \
	--with-init=$(DEB_CONFIGURE_SYSCONFDIR)/init \
	--with-overlay=$(DEB_CONFIGURE_SYSCONFDIR)/overlay \
	--with-menu=$(DEB_CONFIGURE_SYSCONFDIR)/fluxbox.menu-user \
	--with-windowmenu=$(DEB_CONFIGURE_SYSCONFDIR)/window.menu

CFLAGS := $(shell dpkg-buildflags --get CFLAGS)
CPPFLAGS := $(shell dpkg-buildflags --get CPPFLAGS)
CXXFLAGS := $(shell dpkg-buildflags --get CXXFLAGS)
FFLAGS := $(shell dpkg-buildflags --get FFLAGS)
LDFLAGS := $(shell dpkg-buildflags --get LDFLAGS)
export CFLAGS CPPFLAGS CXXFLAGS FFLAGS LDFLAGS
