.PHONY: all clean install
all:
	@make clean;
	@tar -xzf libiconv-1.14.tar.gz
##将下面这段代码注释掉./srclib/stdio.in.h
#//_GL_WARN_ON_USE (gets, "gets is a security hole - use fgets instead");	
	@cd ./libiconv-1.14;sed -i 's/_GL_WARN_ON_USE/\/\/_GL_WARN_ON_USE/g' $(shell pwd)/libiconv-1.14/srclib/stdio.in.h;
	@cd ./libiconv-1.14;./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------libiconv-c finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/libiconv/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/libiconv/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./libiconv-1.14 ] &  rm -rf ./libiconv-1.14
	@[ -d ./out ] &  rm -rf ./out
