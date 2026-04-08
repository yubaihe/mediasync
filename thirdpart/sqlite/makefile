.PHONY: all clean install
name=sqlite-version-3.41.2
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;
	@cd ./$(name);sed -i 's:/usr/share/:$(shell pwd)/out/:g' Makefile;
	@cd ./$(name);make || exit $$?;make install;
	@cd -;
	@echo "---------------sqlite finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/sqlite/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/sqlite/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out

