.PHONY:  install clean
#dirs := Server/
dirs := $(shell ls -d */)
ifneq ($(APPS),)
	dirs=$(APPS)
endif
all:
	$(foreach N,$(dirs),make -C $(N) || exit $$?;)
install:
	$(foreach N,$(dirs),make -C $(N) install || exit $$?;)
clean:
	$(foreach N,$(dirs),make -C $(N) clean || exit $$?;)
