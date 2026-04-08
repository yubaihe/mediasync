.PHONY: all clean install
name=x264-snapshot-20191217-2245
configflags=
ifneq (,$(filter $(PROJECTARC),QZS3))
	configflags=--disable-asm --host=arm-linux-gnueabihf --enable-cross-compile --cross-prefix=arm-linux-gnueabihf- --arch=ARM --target-os=linux
else
	configflags= --disable-asm
endif
all:
	@make clean
	@tar -xvjf $(name).tar.bz2
	cd ./$(name);./configure $(configflags) --enable-shared  --prefix=$(shell pwd)/out || exit $$?;make || exit $$?;make install;
	@echo "---------------x264 finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/x264/
	@mkdir -p $(ROOTDIR)/bin/x264/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/x264/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/x264/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
