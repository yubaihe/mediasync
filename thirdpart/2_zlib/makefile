.PHONY: all clean install
name=zlib-1.2.11
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out  || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------zlib finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/zlib/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/zlib/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
