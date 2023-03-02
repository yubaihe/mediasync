.PHONY: all clean install
all:
	@make clean;
	@tar -xvjf ffmpeg-snapshot.tar.bz2
	@cd ./ffmpeg;./configure --prefix=$(shell pwd)/out || exit $$?;make || exit $$?;make install;
	@cd -;
	@echo "---------------ffmpeg finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/ffmpeg/
	@mkdir -p $(ROOTDIR)/bin/ffmpeg/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/ffmpeg/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/ffmpeg/
clean:
	@[ -d ./ffmpeg ] &  rm -rf ./ffmpeg
	@[ -d ./out ] &  rm -rf ./out
