.PHONY: all clean install
name=dbus-1.10.8
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure  --prefix=$(shell pwd)/out --with-systemdsystemunitdir=$(shell pwd)/out/systemd --with-systemduserunitdir=$(shell pwd)/out/systemd/ CFLAGS=-I$(ROOTDIR)/include/libexpat LDFLAGS=-L$(ROOTDIR)/libs $(CONFIGARGS) || exit $$?;
	@cd ./$(name);make || exit $$?;make install||exit $$?;
	@echo "---------------dbus finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/dbus/
	@mkdir -p $(ROOTDIR)/bin/dbus/
	@mkdir -p $(ROOTDIR)/bin/dbus/dbus-1/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/dbus-1.0/** $(ROOTDIR)/include/dbus/
	@cp -r $(shell pwd)/$(name)/dbus/dbus-arch-deps.h $(ROOTDIR)/include/dbus/dbus
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
	@cp -r $(shell pwd)/out/bin/** $(ROOTDIR)/bin/dbus/
	@cp -r $(shell pwd)/out/share/dbus-1/** $(ROOTDIR)/bin/dbus/dbus-1/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
