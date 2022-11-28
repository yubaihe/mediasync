#sudo apt-get install cmake
#sudo apt-get install yasm ffmpeg linux需要
#sudo apt install autoconf file linux需要
#sudo apt-get install libtool file linux需要
.PHONY: all lighttpd lighttpdclean apps appsclean thirdpart thirdpartclean package clean
all:
	@make thirdpart;
	@make lighttpd;
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
clean:
ifneq ($(PROJECTARC),)
	@make thirdpartclean
	@make lighttpdclean
	@make appsclean
	@rm -rf $(ROOTDIR)/include
	@rm -rf $(ROOTDIR)/libs
	@rm -rf $(ROOTDIR)/bin
	@rm -rf $(ROOTDIR)/apps
	@rm -rf $(ROOTDIR)/lighttpd
	@rm -rf ./package/install
endif
package:
ifneq ($(PROJECTARC),)
	@./package/install_$(shell echo $(PROJECTARC) | tr A-Z a-z ).sh
else
	@./package/install.sh
endif
	
	
