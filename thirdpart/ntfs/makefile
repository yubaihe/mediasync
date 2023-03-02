.PHONY: all clean install
all:
	@make clean;
	@tar xvzf ntfs-3g_ntfsprogs-2021.8.22.tgz
	@cd ./ntfs-3g_ntfsprogs-2021.8.22/;./configure --prefix=$(shell pwd)/out --exec-prefix=$(shell pwd)/out $(CONFIGARGS) --disable-shared --enable-static || exit $$?;
	@cd ./ntfs-3g_ntfsprogs-2021.8.22/;sed -i 's/install-binPROGRAMS install-exec-local/install-binPROGRAMS/g' ./src/Makefile
	@cd ./ntfs-3g_ntfsprogs-2021.8.22/;sed -i 's/(AM_MAKEFLAGS) install-exec-hook/(AM_MAKEFLAGS) /g' ./src/Makefile
	@cd ./ntfs-3g_ntfsprogs-2021.8.22/;make || exit $$?;make install;
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
	@[ -d ./ntfs-3g_ntfsprogs-2021.8.22 ] &  rm -rf ./ntfs-3g_ntfsprogs-2021.8.22
	@[ -d ./out ] &  rm -rf ./out
