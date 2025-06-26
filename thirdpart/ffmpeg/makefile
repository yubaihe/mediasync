.PHONY: all clean install
name=FFmpeg-n7.1.1
configflags=
ifneq (,$(filter $(PROJECTARC),QZS3))
	configflags=--disable-asm --arch=ARM --target-os=linux --extra-cflags="-fPIC" --cross-prefix=arm-linux-gnueabihf- --enable-small --disable-runtime-cpudetect
#	configflags+=--enable-small --disable-runtime-cpudetect
else
	configflags=
endif
all:
	@make clean
	@tar -xvzf $(name).tar.gz
#	@cd ./$(name);./configure --disable-asm --arch=ARM --target-os=linux --extra-cflags="-fPIC" --cross-prefix=arm-linux-gnueabihf- --enable-shared --prefix=$(shell pwd)/out || exit $$?;make || exit $$?;make install;
	cd ./$(name);./configure $(configflags) --enable-shared  --prefix=$(shell pwd)/out || exit $$?;make || exit $$?;make install;
	@echo "---------------ffmpeg finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/ffmpeg/
	@mkdir -p $(ROOTDIR)/bin/ffmpeg/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/ffmpeg/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/ffmpeg/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
