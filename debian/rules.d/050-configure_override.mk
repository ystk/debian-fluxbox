# Intercept the configure stage to throw it some flags.

FLUXBOX_VENDOR := $(shell \
	if dpkg-vendor --derives-from Ubuntu; then \
		echo Ubuntu; \
	else \
		echo Debian; \
	fi \
)

THEME_DEFAULT := $(shell \
	cat ./debian/additional-themes/defaults | \
		grep ^$(FLUXBOX_VENDOR) | \
		awk '{print $$2}' \
)

DEB_CONFIGURE_EXTRA_FLAGS += -with-style=$(DEB_STYLE_DIR)/$(THEME_DEFAULT)

override_dh_auto_configure:
	./configure $(DEB_CONFIGURE_EXTRA_FLAGS)
