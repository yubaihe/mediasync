.PHONY: all clean install
name=tar-1.34
all:
	@make clean;
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------$(name) finish----------------"
install:
	@mkdir -p $(ROOTDIR)/bin/tar/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/tar/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
