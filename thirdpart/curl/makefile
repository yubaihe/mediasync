.PHONY: all clean install
name = curl-7.71.1
all:
	@make clean;
	@tar -xzf $(name).tar.gz
#	@sed -i 's/-Werror-implicit-function-declaration/ /g' ./$(name)/configure
#	cd $(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) LDFLAGS='$(LDFLAGS) -lssl -lcrypto' -with-ssl || exit $$?;
	cd $(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) LDFLAGS='$(LDFLAGS)' --without-ssl || exit $$?;
	@cd $(name);make || exit $$?;make install;
	@cd -;
	@echo "---------------curl finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/curl/
	@mkdir -p $(ROOTDIR)/libs
	@mkdir -p $(ROOTDIR)/bin/
	@cp -r $(shell pwd)/out/include/curl/** $(ROOTDIR)/include/curl/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/bin/curl $(ROOTDIR)/bin/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out

