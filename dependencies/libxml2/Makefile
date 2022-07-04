PKG_NAME := libxml2-2.9.10
PKG_FILE := $(PKG_NAME).tar.xz

all:
	[ -L libxml2 ] || (tar -Jxvf $(PKG_FILE) && ln -sf $(PKG_NAME) libxml2 && \
		cd libxml2 && \
		./configure --without-lzma \
					--without-icu \
					--without-iconv \
					--without-zlib \
					--without-readline \
					--without-python \
					--host=arm-linux \
					--disable-shared \
					--prefix=$(INSTALL_DIR) \
					--without-modules \
					--without-xpath \
					--without-xinclude \
					--without-writer \
					--without-thread-alloc \
					--without-threads \
					--without-regexps \
					--without-http \
					--without-html \
					--without-debug \
					--without-c14n \
					--without-ftp \
					CC=$(CC))
	cd libxml2 && make -j4 install

clean:
	rm -rf libxml2 $(PKG_NAME)

.PHONY: all clean
