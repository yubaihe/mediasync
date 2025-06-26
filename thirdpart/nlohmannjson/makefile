.PHONY: all clean install
name = json-3.12.0
all:
	@make clean
	@tar -xzf $(name).tar.gz
	@mkdir build;cd build;cmake -DCMAKE_INSTALL_PREFIX=$(shell pwd)/out ../$(name) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_CXX_COMPILER=$(CXX) || exit $$?;
	@cd build;make || exit $$?;
	@cd build;make install || exit $$?;
	@rm -rf build
	@echo "---------------nlohmannjson finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/nlohmannjson/
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/nlohmannjson/
clean:
	@rm -rf build
	@[ -d ./$(name) ] &  rm -rf ./$(name)
	@[ -d ./out ] &  rm -rf ./out

