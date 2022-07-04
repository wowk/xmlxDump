
CROSS_COMPILER:=/opt/toolchains/crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/bin/arm-linux-

export CC := $(CROSS_COMPILER)gcc
export STRIP := $(CROSS_COMPILER)strip
export INSTALL_DIR=$(shell pwd)/build/

CFLAGS := -Os -I$(INSTALL_DIR)/include/ -I$(INSTALL_DIR)/include/libxml2/
LIBFLAGS := -lm -L$(INSTALL_DIR)/lib/ -l:libz.a -l:libxml2.a

XMLX_DUMP_TARGET := xmlx_dump
XMLX_DUMP_OBJS	 := xmlx_dump.o

all: dependencies $(XMLX_DUMP_TARGET)

dependencies:
	@[ -d ./build/ ] || make -C dependencies

$(XMLX_DUMP_TARGET) : $(XMLX_DUMP_OBJS)
	$(CC) -o $@ $^ $(LIBFLAGS)
	$(STRIP) -s $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

distclean: clean
	make -C dependencies clean

clean:
	rm -rf build
	rm -rf $(XMLX_DUMP_OBJS) $(XMLX_DUMP_TARGET)

.PHONY: all dependencies clean

