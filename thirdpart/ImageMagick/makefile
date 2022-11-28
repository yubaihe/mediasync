.PHONY: all clean install
all:
	@make clean;
	@tar -xvzf ImageMagick.tar.gz
	@cd ./ImageMagick-7.1.0-5;./configure --prefix=$(shell pwd)/out $(CONFIGARGS)|| exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------ImageMagick finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/imagemagic/
	@mkdir -p $(ROOTDIR)/bin/imagemagic/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/imagemagic/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/imagemagic/
clean:
	@[ -d ./ImageMagick-7.1.0-5 ] &  rm -rf ./ImageMagick-7.1.0-5
	@[ -d ./out ] &  rm -rf ./out
