.PHONY: all clean install
name=pcre2-10.40
all:
	@make clean
	@tar -xvzf $(name).tar.gz
	@cd ./$(name);./configure --prefix=$(shell pwd)/out $(CONFIGARGS) || exit $$?;
	cd ./$(name);make || exit $$?;
	cd ./$(name);make install;
	@echo "---------------pcre finish----------------"
install:
	mkdir -p $(ROOTDIR)include/pcre2/
	mkdir -p $(ROOTDIR)libs
	cp -r $(shell pwd)/out/include/** $(ROOTDIR)include/pcre2/
	rm -rf $(ROOTDIR)libs/libpcre2** 
	cp -r $(shell pwd)/out/lib/** $(ROOTDIR)libs/
	cp -r $(shell pwd)/out/bin/pcre2-config $(ROOTDIR)libs/
clean:
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out
