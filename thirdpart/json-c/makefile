.PHONY: all clean install
all:
	@make clean;
	@unzip json-c.zip
	@cd ./json-c;chmod +x ./configure;./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------json-c finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/json-c/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./json-c ] &  rm -rf ./json-c
	@[ -d ./out ] &  rm -rf ./out
