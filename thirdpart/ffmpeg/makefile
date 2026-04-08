.PHONY: all clean install
name=FFmpeg-n7.1.1

ifneq (,$(filter $(PROJECTARC),QZS3))
        name = ffmpeg-4.0.6
	configflags = --enable-gpl --enable-libx264 --enable-version3 --extra-cflags="-fPIC -I$(ROOTDIR)/include/x264" --extra-ldflags="-L$(ROOTDIR)/libs -lx264" --cross-prefix=arm-linux-gnueabihf- --disable-asm --arch=ARM --target-os=linux 
else
	configflags = --enable-gpl --enable-libx264 --enable-version3 --extra-cflags="-I$(ROOTDIR)/include/x264" --extra-ldflags="-L$(ROOTDIR)/libs -lx264"
endif
all:
	@make clean
	@tar -xvzf $(name).tar.gz
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

