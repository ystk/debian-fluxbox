# Some basic constants we'll use in the build process.

DEB_CONFIGURE_SYSCONFDIR        := /etc/X11/fluxbox
DEB_STYLE_DIR                   := /usr/share/fluxbox/styles/

VERSION	= $(shell dpkg-parsechangelog|grep ^Version|awk '{print $$2}' \
	|sed 's/-[[:digit:]]\+$$//')
