PKG_NAME := zlib-1.2.11.dfsg
PKG_FILE := $(PKG_NAME).tar.gz

all:
	[ -L zlib ] || (tar -zxvf $(PKG_FILE) && ln -sf $(PKG_NAME) zlib && \
		cd zlib && ./configure --static --prefix=$(INSTALL_DIR))
	cd zlib && make -j4 install

clean:
	rm -rf zlib $(PKG_NAME)

.PHONY: all clean

