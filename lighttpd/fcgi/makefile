dirs := $(shell ls -d */)

all:
	$(foreach N,$(dirs),make -C $(N) || exit $$? ;)
clean:
	$(foreach N,$(dirs),make -C $(N) clean;)
install:
	@mkdir -p $(ROOTDIR)/lighttpd/fcgi
	$(foreach N,$(dirs),make -C $(N) install;)
	@cp ./Config.json $(ROOTDIR)/lighttpd/fcgi/Config.json
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/lighttpd/fcgi/Config.json
dstinstall:
	$(foreach N,$(dirs),make -C $(N) dstinstall )