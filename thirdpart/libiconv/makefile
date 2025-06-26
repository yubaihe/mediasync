.PHONY: all clean install
name=libiconv-1.14
all:
	@make clean
	@tar -xzf $(name).tar.gz
##将下面这段代码注释掉./srclib/stdio.in.h
#//_GL_WARN_ON_USE (gets, "gets is a security hole - use fgets instead");	
	@cd ./$(name);sed -i 's/_GL_WARN_ON_USE/\/\/_GL_WARN_ON_USE/g' $(shell pwd)/$(name)/srclib/stdio.in.h;
	@cd ./$(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------libiconv-c finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/libiconv/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/libiconv/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
