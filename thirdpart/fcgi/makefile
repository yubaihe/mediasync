.PHONY: all clean install
name=fcgi-2.4.1-SNAP-0910052249
all:
	@make clean
	@tar -xvzf fcgi-2.4.1.tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;
	@sed -i 's/#include <iostream>/#include <iostream>\n#include <stdio.h>/g' $(shell pwd)/$(name)/include/fcgio.h
	@sed -i 's/-lnsl/-lnsl -lm/g' $(shell pwd)/$(name)/libfcgi/Makefile
	@cd ./$(name);make || exit $$?;make install;
	@cd -;
	@echo "---------------fcgi finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/fcgi/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/fcgi/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
