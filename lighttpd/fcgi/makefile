dirs := $(shell ls -d */)

all:
	$(foreach N,$(dirs),make -C $(N) || exit $$? ;)
clean:
	$(foreach N,$(dirs),make -C $(N) clean;)
install:
	@mkdir -p $(ROOTDIR)/lighttpd/fcgi
	$(foreach N,$(dirs),make -C $(N) install;)
dstinstall:
	$(foreach N,$(dirs),make -C $(N) dstinstall )