dirs := $(shell ls -d */)

all:
ifneq (,$(filter $(PROJECTARC),QZS3))
# 全志芯片走在ubuntu16上面，无法vue编译 所以应当在其他系统编译完成以后，放在 lighttpd/webroot/www/dist 里面再开始编译
else
	@if [ -d "www/src" ]; then \
		cd www;rm -rf $(shell pwd)/www/dist;rm -rf $(shell pwd)/www/package-lock.json;rm -rf $(shell pwd)/www/node_modules;npm install;npm run build; \
	fi
endif
clean:
	
install:
	 cp -R $(shell pwd)/www/dist $(ROOTDIR)/lighttpd/www
dstinstall:
	$(foreach N,$(dirs),make -C $(N) dstinstall )
