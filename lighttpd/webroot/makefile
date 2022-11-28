dirs := $(shell ls -d */)

all:
	
clean:
	
install:
	cp -r $(shell pwd)/www $(ROOTDIR)/lighttpd/
dstinstall:
	$(foreach N,$(dirs),make -C $(N) dstinstall )