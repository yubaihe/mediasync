.PHONY: all clean install
name=ntp-dev-4.2.5p59
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------ntp finish----------------"
install:
	@mkdir -p $(ROOTDIR)/bin/ntp/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/ntp/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
