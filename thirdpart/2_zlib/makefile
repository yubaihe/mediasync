.PHONY: all clean install
all:
	@make clean;
	@tar -xvzf zlib-1.2.11.tar.gz
	@cd ./zlib-1.2.11;./configure --prefix=$(shell pwd)/out  || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------zlib finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/zlib/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/zlib/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./zlib-1.2.11 ] &  rm -rf ./zlib-1.2.11
	@[ -d ./out ] &  rm -rf ./out
