.PHONY: all clean install
all:
	@make clean;
	@tar -xzf jsoncpp-1.9.5.tar.gz
	@cd ./jsoncpp-1.9.5/src/lib_json/;$(CXX) -std=c++11 -c *.cpp -I../../include -fPIC;$(AR)  cr libjsoncpp.a *.o;cd -;
	@mkdir -p $(shell pwd)/out/include;mkdir -p $(shell pwd)/out/lib;
	@cp -r ./jsoncpp-1.9.5/include/json $(shell pwd)/out/include/
	@cp ./jsoncpp-1.9.5/src/lib_json/*.a $(shell pwd)/out/lib/
	@echo "---------------jsoncpp finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/jsoncpp/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/jsoncpp/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./jsoncpp-1.9.5 ] &  rm -rf ./jsoncpp-1.9.5
	@[ -d ./out ] &  rm -rf ./out
