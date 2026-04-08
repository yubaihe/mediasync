.PHONY: all clean install
name=libexpat-R_2_6_4
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@cd ./$(name)/expat;./buildconf.sh;./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------libexpat finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/libexpat/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/libexpat/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
