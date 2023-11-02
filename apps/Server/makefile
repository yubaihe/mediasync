#make arm=y memcheck=y
#make arm=fsl memcheck=y
#make arm=n memcheck=y
TARGET = Server

BRANCHNAME="$(shell git rev-parse --abbrev-ref HEAD)" # git分支号
COMMIT="$(shell git log --pretty=format:"%h" -1)" # git提交号
BRANCH="$(strip $(BRANCHNAME))-$(COMMIT)"
BUILDDATE = "$(shell date '+%F %T')"


LDFLAGS = -lm

CFLAGS  += -g -Wall  -MD -fPIE -fno-stack-protector -Wno-format-overflow\
		-DBRANCH="\$(BRANCH)\" -DBUILDDATE="\$(BUILDDATE)\" \
		-I$(ROOTDIR)/include/sqlite3 \
		-I$(ROOTDIR)/include/json-c/ \

STATICLIB = -lpthread $(ROOTDIR)/libs/libsqlite3.a \
			$(ROOTDIR)/libs/libjson-c.a 
			
ifeq ($(LIBDLPATH)/libdl.so, $(wildcard $(LIBDLPATH)/libdl.so))
STATICLIB += $(LIBDLPATH)/libdl.so
else
STATICLIB += -ldl
endif

ifeq ($(memcheck), y)
SOURCE += memwatch.c 
CFLAGS += -DMEMWATCH -DMW_STDIO 
endif

####################################################################################################
.PHONY: clean install
#SOURCE +=  $(shell find . -name "*.c")
SOURCE +=  md5.c Base64Coding.c stdafx.c RelechQueue.c RelechList.c RelechMap.c Tools.c FileUtil.c \
			MediaScanDriver.c DataBaseDriver.c DataBaseMedia.c DataBaseDevice.c DataBaseMediaJiShu.c DataBaseUserInfo.c ConnectBroadCast.c \
			ConnectBroadCastMessage.c ConnectServer.c ConnectServerMessage.c HttpFileServer.c NetWorkUtil.c NetWorkDriver.c DataBaseCover.c \
			 DataBaseGroup.c DataBaseGroupItems.c ClearCache.c main.c  \
#SOURCE = test2.c\
####################################################################################################
OBJS = $(SOURCE:%.c=obj.$(TARGET)/%.o)
DEPS = $(SOURCE:%.c=obj.$(TARGET)/%.d)

default:
	make $(TARGET)
	
$(TARGET): $(OBJS)
	$(CC) obj.$(TARGET)/*.o $(LDFLAGS) $(STATICLIB) -o $@

obj.$(TARGET)/%.o: %.c
	@mkdir -p obj.$(TARGET)
	$(CC) $(CFLAGS) $(APPFLAGS) $(LDFLAGS) -c -o $@ $<

####################################################################################################
clean:
	rm -rf obj.$(TARGET)
	rm -rf $(TARGET)
	rm -rf $(ROOTDIR)/apps/Server
#	rm -rf /home/relech/share/media/**
#	rm -rf /tmp/mediasync
#	rm -rf mediasync.db 
install:
	@mkdir $(ROOTDIR)/apps/Server -p
	@cp Server $(ROOTDIR)/apps/Server/
	@cp Config.json $(ROOTDIR)/apps/Server/
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/apps/Server/Config.json
	@cp $(ROOTDIR)/apps/Server/Config.json $(ROOTDIR)/apps/Server/Config_default.json
	@sed -i 's:/dev/sda1:/dev/devnodename:g' $(ROOTDIR)/apps/Server/Config_default.json
	

	
