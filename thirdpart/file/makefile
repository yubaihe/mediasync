.PHONY: all clean install
all:
	@make clean;
	@mkdir ./file-FILE5_04
	@tar -xzf file-5.25.tar.gz
	@cd ./file-5.25;./configure --prefix=$(shell pwd)/out $(CONFIGARGS);make || exit $$?;make install;
	@cd -;
	@echo "---------------file finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/file/
	@mkdir -p $(ROOTDIR)/bin/file/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/file/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/file/
	@cp -r $(shell pwd)/out/share/misc/** $(ROOTDIR)/bin/file/
clean:
	@[ -d ./file-FILE5_04 ] &  rm -rf ./file-FILE5_04
	@[ -d ./file-5.25 ] &  rm -rf ./file-5.25
	@[ -d ./out ] &  rm -rf ./out
