.PHONY: all clean install
all:
	@make clean;
	@tar -xvzf zlog-1.2.15.tar.gz
	@sed -i 's:PREFIX?=/usr/local:PREFIX?=$(shell pwd)/out:g' $(shell pwd)/zlog-1.2.15/src/makefile
	@cd zlog-1.2.15;make || exit $$?;make install;
	@cd -;
	@echo "---------------zlog finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/zlog/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/zlog/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./zlog-1.2.15 ] &  rm -rf ./zlog-1.2.15
	@[ -d ./out ] &  rm -rf ./out
