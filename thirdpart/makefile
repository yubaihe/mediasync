.PHONY: all install clean

#dirs := json-c/ sqlite-amalgamation-3310100/ pcre/ libiconv/ jsoncpp/ exiv2/
dirs=$(shell ls -d */)
ifneq ($(THIRDPARTLIB),)
	dirs=$(THIRDPARTLIB)
endif

all:
	$(foreach N,$(dirs),make -C $(N) || exit $$?; make -C $(N) install || exit $$?;)
install:
	$(foreach N,$(dirs),make -C $(N) install || exit $$?;)
clean:
	$(foreach N,$(dirs),make -C $(N) clean || exit $$?;)

