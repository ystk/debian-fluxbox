#!/usr/bin/make -f

DEB_STYLE_DIR   := /usr/share/fluxbox/styles/
DEB_IMAGE_DIR   := /usr/share/images/fluxbox/

FLUXBOX_VENDOR := $(shell \
	if dpkg-vendor --derives-from Ubuntu; then \
		echo Ubuntu; \
	else \
		echo Debian; \
	fi \
)

all:
	@echo "OK. Doing the theme building."
	@echo "Looks like we're doing this package for $(FLUXBOX_VENDOR)"
	# Let's blast the old directory
	rm -rf ./stage ./stage-images
	mkdir  ./stage ./stage-images
	# OK. Let's stage up some files.
	cp ./Common/*            ./stage -rv
	cp ./$(FLUXBOX_VENDOR)/* ./stage -rv
	cp ./Images/Common/*            ./stage-images -rv
	cp ./Images/$(FLUXBOX_VENDOR)/* ./stage-images -rv

install:
	mkdir -p ../fluxbox/$(DEB_STYLE_DIR)
	mkdir -p ../fluxbox/$(DEB_IMAGE_DIR)
	cp ./stage/*        ../fluxbox/$(DEB_STYLE_DIR) -rv
	cp ./stage-images/* ../fluxbox/$(DEB_IMAGE_DIR) -rv

clean:
	rm -rf ./stage ./stage-images
