.PHONY: all clean install
name=file-5.25
all:
	@make clean
	@tar -xzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS);make || exit $$?;make install;
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
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
