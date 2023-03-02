.PHONY: all clean install
all:
	@make clean
	@tar -xvzf exiv2-0.27.5-Source.tar.gz
	@mkdir -p $(shell pwd)/exiv2-0.27.5-Source/lib; cp $(ROOTDIR)/libs/libexpat** /$(shell pwd)/exiv2-0.27.5-Source/lib/
	@cd ./exiv2-0.27.5-Source; cmake . -DCMAKE_INSTALL_PREFIX=$(shell pwd)/out $(CMAKEARGS) -DZLIB_LIBRARY=$(ROOTDIR)/libs/libz.so -DZLIB_INCLUDE_DIR=$(ROOTDIR)/include/zlib/ -DEXPAT_LIBRARY=$(ROOTDIR)/libs/libexpat.so || exit $$?;make || exit $$?;make install;cd -;
	@echo "---------------exiv2 finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/exiv2/
	@mkdir -p $(ROOTDIR)/bin/exiv2/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/exiv2/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/exiv2/
clean:
	@[ -d ./exiv2-0.27.5-Source ] &  rm -rf ./exiv2-0.27.5-Source
	@[ -d ./out ] &  rm -rf ./out
