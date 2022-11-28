.PHONY: all clean install
all:
	@make clean;
	@$(CC) -o sqlite3.o -c sqlite3.c -lpthread -ldl;$(AR) -rcv libsqlite3.a sqlite3.o
	@mkdir -p $(shell pwd)/out/include -p $(shell pwd)/out/lib 
	@cp libsqlite3.a $(shell pwd)/out/lib; cp *.h $(shell pwd)/out/include
	@cd -;
	@echo "---------------sqlite3 finish----------------"
install:
	@mkdir -p $(ROOTDIR)/include/sqlite3/
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/out/include/** $(ROOTDIR)/include/sqlite3/
	@cp -r $(shell pwd)/out/lib/** $(ROOTDIR)/libs/
clean:
	@[ -d ./**.o ] &  rm -rf ./**.o
	@[ -d ./**.o ] &  rm -rf ./**.a
	@[ -d ./out ] &  rm -rf ./out
