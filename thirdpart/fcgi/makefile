.PHONY: all clean install
all:
	@make clean;
	@tar -xvzf fcgi-2.4.1.tar.gz
	@cd ./fcgi-2.4.1-SNAP-0910052249;./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;
	@sed -i 's/#include <iostream>/#include <iostream>\n#include <stdio.h>/g' $(shell pwd)/fcgi-2.4.1-SNAP-0910052249/include/fcgio.h
	@sed -i 's/-lnsl/-lnsl -lm/g' $(shell pwd)/fcgi-2.4.1-SNAP-0910052249/libfcgi/Makefile
	@cd ./fcgi-2.4.1-SNAP-0910052249;make || exit $$?;make install;
	@cd -;
	@echo "---------------fcgi finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/fcgi/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/fcgi/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./fcgi-2.4.1-SNAP-0910052249 ] &  rm -rf ./fcgi-2.4.1-SNAP-0910052249
	@[ -d ./out ] &  rm -rf ./out
