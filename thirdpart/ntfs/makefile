.PHONY: all clean install
name=ntfs-3g_ntfsprogs-2021.8.22
all:
	@make clean
	@tar xvzf $(name).tgz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out --exec-prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;
	@cd ./$(name);sed -i 's/install-binPROGRAMS install-exec-local/install-binPROGRAMS/g' ./src/Makefile
	@cd ./$(name);sed -i 's/(AM_MAKEFLAGS) install-exec-hook/(AM_MAKEFLAGS) /g' ./src/Makefile
	@cd ./$(name);make || exit $$?;make install;
	@cd -;
	@echo "---------------ntfs finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/ntfs-3g/
	@mkdir -p $(ROOTDIR)/bin/ntfs-3g/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/ntfs-3g/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/ntfs-3g/
	@cp -r $(shell pwd)/out/sbin/** $(ROOTDIR)/bin/ntfs-3g/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
