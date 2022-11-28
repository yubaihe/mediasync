.PHONY: all clean install
all:
	@make clean;
	@tar -xvzf ntp-dev-4.2.5p59.tar.gz
	@cd ./ntp-dev-4.2.5p59;./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------ntp finish----------------"
install:
	@mkdir -p $(ROOTDIR)/bin/ntp/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/ntp/
clean:
	@[ -d ./ntp-dev-4.2.5p59 ] &  rm -rf ./ntp-dev-4.2.5p59
	@[ -d ./out ] &  rm -rf ./out
