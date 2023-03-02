.PHONY: all clean install
all:
	@make clean;
	@tar -xvzf lighttpd-1.4.66.tar.gz
	@cd ./lighttpd-1.4.66;./configure --prefix=$(ROOTDIR)/lighttpd $(CONFIGARGS) --without-zlib --without-bzip2 --enable-fastcgi PCRECONFIG=$(ROOTDIR)/libs/pcre2-config PCRE_LIB=$(ROOTDIR)/libs/libpcre2-8.so CPPFLAGS=-I$(ROOTDIR)/include/pcre2 || exit $$?;make || exit $$?;make install || exit $$?;
	@cd -;
	@echo "---------------lighttpd finish----------------"
install:
	@mkdir -p $(ROOTDIR)/lighttpd/log
	@cp config $(ROOTDIR)/lighttpd -R
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/lighttpd/config/lighttpd.conf
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/lighttpd/config/conf.d/fastcgi.conf
	@rm -rf $(ROOTDIR)/lighttpd/lib/**.la
clean:
	@[ -d ./lighttpd-1.4.66 ] &  rm -rf ./lighttpd-1.4.66
#	
