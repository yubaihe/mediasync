.PHONY: all clean install
all:
	@make clean;
	@tar -xzf pcre2-10.40.tar.gz
	@cd ./pcre2-10.40;./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------pcre finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/pcre2/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/pcre2/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/pcre2-config $(ROOTDIR)/libs/
clean:
	@[ -d ./pcre2-10.40 ] &  rm -rf ./pcre2-10.40
	@[ -d ./out ] &  rm -rf ./out
