.PHONY: all clean install
name=zlog-1.2.15
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@sed -i 's:PREFIX?=/usr/local:PREFIX?=$(shell pwd)/out:g' $(shell pwd)/$(name)/src/makefile
	@cd $(name);make || exit $$?;make install;
	@cd -;
	@echo "---------------zlog finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/zlog/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/zlog/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
