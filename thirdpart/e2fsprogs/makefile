.PHONY: all clean install
name=e2fsprogs-1.47.2

all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out --with-udev-rules-dir=$(shell pwd)/out/udev --with-crond-dir=$(shell pwd)/out/crond --with-systemd-unit-dir=$(shell pwd)/out/systemd $(CONFIGARGS) ;
	@cd ./$(name);make || exit $$?;
	@cd ./$(name);make install;
#要加一个755的权限不然第二次install的时候 会出现类似错误:cannot create regular file '/home/relech/mediasync/libs/libblkid.a': Permission denied
	@chmod 755 $(shell pwd)/out/lib/** -R
	@echo "---------------$(name) finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/e2fsprogs/
	@mkdir -p $(ROOTDIR)/bin/e2fsprogs/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)include/e2fsprogs/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)bin/e2fsprogs/
	
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
