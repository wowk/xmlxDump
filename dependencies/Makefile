subdirs := zlib libxml2

all:
	for dir in $(subdirs) ; \
	do	\
		make -C $$dir ; \
	done

clean:
	for dir in $(subdirs) ; \
	do	\
		make -C $$dir clean; \
	done

.PHONY: all clean 
