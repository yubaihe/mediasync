.PHONY: all clean install
name=exiv2-0.27.7
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@mkdir -p $(shell pwd)/$(name)/lib; cp $(ROOTDIR)/libs/libexpat** /$(shell pwd)/$(name)/lib/
	@cd ./$(name); cmake . -DCMAKE_INSTALL_PREFIX=$(shell pwd)/out $(CMAKEARGS) -DEXIV2_ENABLE_INIH=OFF -DEXIV2_ENABLE_BROTLI=OFF -DZLIB_LIBRARY=$(ROOTDIR)/libs/libz.so -DZLIB_INCLUDE_DIR=$(ROOTDIR)/include/zlib/ -DEXPAT_LIBRARY=$(ROOTDIR)/libs/libexpat.so || exit $$?;make || exit $$?;make install;cd -;
	@echo "---------------exiv2 finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/exiv2/
	@mkdir -p $(ROOTDIR)/bin/exiv2/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/exiv2/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/exiv2/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
