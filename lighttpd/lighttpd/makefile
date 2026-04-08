.PHONY: all clean install
name=lighttpd-1.4.66
all:
ifeq ($(wildcard $(name)),)
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(ROOTDIR)/lighttpd $(CONFIGARGS) --without-zlib --without-bzip2 --enable-fastcgi PCRECONFIG=$(ROOTDIR)/libs/pcre2-config PCRE_LIB=$(ROOTDIR)/libs/libpcre2-8.so CPPFLAGS=-I$(ROOTDIR)/include/pcre2 || exit $$?;make || exit $$?;make install || exit $$?;
	@echo "---------------lighttpd finish----------------"
endif
install:
	@mkdir -p $(ROOTDIR)/lighttpd/log
	@cp config $(ROOTDIR)/lighttpd -R
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/lighttpd/config/lighttpd.conf
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/lighttpd/config/lighttpd.conf
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/lighttpd/config/conf.d/fastcgi.conf
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/lighttpd/config/alias.conf
	@rm -rf $(ROOTDIR)/lighttpd/lib/**.la
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
#	
