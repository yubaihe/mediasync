.PHONY: all clean install
all:
	@make clean;
	@tar -xvzf expat-2.4.6.tar.gz
	@cd ./expat-2.4.6;./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------libexpat finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/libexpat/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/libexpat/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./expat-2.4.6 ] &  rm -rf ./expat-2.4.6
	@[ -d ./out ] &  rm -rf ./out
