#sudo apt-get install cmake
#sudo apt-get install yasm ffmpeg linux需要
#sudo apt install autoconf file linux需要
#sudo apt-get install libtool file linux需要

#make BUILDTYPE=RELEASE/DEBUG
.PHONY: all lighttpd lighttpdclean samba sambaclean apps appsclean thirdpart thirdpartclean driver driverclean package clean
ifneq (,$(filter $(BUILDTYPE),DEBUG))
	export BUILDTYPE=DEBUG;
	@echo "===============Debug Mode================="
else
	export BUILDTYPE=RELEASE;
	@echo "===============RELEASE Mode================="
endif
ifneq (,$(filter $(PROJECTARC),QZS3))
	export BUILDTYPE=RELEASE;
endif
all:
	@make thirdpart;
	@make -C apps/Common;make -C apps/Common install;
	@make lighttpd;
	@make samba;
ifneq (,$(filter $(PROJECTARC),QZS3))
	@make driver;
endif
	@make apps;
	@make package;
apps:
	@cd apps; make; make install;
appsclean:
	@cd apps; make clean;
thirdpart:
	@cd thirdpart; make || exit $$?;
thirdpartclean:
	@cd thirdpart; make clean|| exit $$?;
lighttpd:
	@cd lighttpd; make || exit $$?; make install;
lighttpdclean:
	@make -C ./lighttpd clean
samba:
	@cd samba; make; make install;
sambaclean:
	@make -C ./samba clean
driver:
	@cd driver;make; make install;
driverclean:
	@cd driver; make clean|| exit $$?;
checkenv:
ifeq ($(ROOTDIR), )
	@echo "please execute:"
	@echo "	initenv.sh"
	@exit 1
endif
	@echo "env ok"

clean:
	make checkenv
ifneq ($(PROJECTARC),)
	@make thirdpartclean
	@make lighttpdclean
	@make sambaclean
	@make appsclean
#ifneq (,$(filter $(PROJECTARC),QZS3 QZS4))
ifneq (,$(filter $(PROJECTARC),QZS3))
	@make driverclean
endif
	@rm -rf $(ROOTDIR)/include
	@rm -rf $(ROOTDIR)/libs
	@rm -rf $(ROOTDIR)/bin
	@rm -rf $(ROOTDIR)/apps
	@rm -rf $(ROOTDIR)/driver
	@rm -rf $(ROOTDIR)/lighttpd
	@rm -rf $(ROOTDIR)/samba
	@rm -rf $(ROOTDIR)/share
	@rm -rf ./package/install
endif
package:
ifneq ($(PROJECTARC),)
	@./package/install_$(shell echo $(PROJECTARC) | tr A-Z a-z ).sh
else
	@./package/install.sh
endif
	
	
