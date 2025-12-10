dirs := $(shell ls -d */)

all:
	@if [ -d "www/src" ]; then \
		cd www;rm -rf $(shell pwd)/www/dist;rm -rf $(shell pwd)/www/package-lock.json;rm -rf $(shell pwd)/www/node_modules;npm install;npm run build; \
	fi
clean:
	@if [ -d "www/src" ]; then \
		rm -rf $(shell pwd)/www/dist; \
	fi
	rm -rf $(shell pwd)/www/package-lock.json;rm -rf $(shell pwd)/www/node_modules;
install:
	rm -rf $(ROOTDIR)/lighttpd/www;mkdir -p $(ROOTDIR)/lighttpd/www;
	cp -R $(shell pwd)/www/dist/** $(ROOTDIR)/lighttpd/www
dstinstall:
	$(foreach N,$(dirs),make -C $(N) dstinstall )
