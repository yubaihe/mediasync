.PHONY: all install clean
dirs := $(shell ls -d */)
#dirs := $(subst out/, ,$(dirs) )
all:
	@make clean
	$(foreach N,$(dirs),make -C $(N) || exit $$?;)
install:
	$(foreach N,$(dirs),make -C $(N) install || exit $$?;)
clean:
	$(foreach N,$(dirs),make -C $(N) clean || exit $$?;)
	@[ -d $(ROOTDIR)/lighttpd ] &  rm -rf $(ROOTDIR)/lighttpd
