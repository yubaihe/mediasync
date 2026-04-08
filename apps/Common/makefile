
TARGET = libcommon.so
include ./config.mk
ifneq (,$(filter $(BUILDTYPE),DEBUG))
	CFLAGS = -g
else
	CFLAGS = 
endif
####################################################################################################
LDFLAGS +=
CFLAGS  += -MD -DMW_STDIO  -fPIC -shared -std=c++11 \
		-IMedia\
		-IDbus\
		-IUtil\
		-I$(ROOTDIR)/include/dbus\
		-I$(ROOTDIR)/include/sqlite\
		-I.
STATICLIB = -lpthread -L$(ROOTDIR)/libs/ -ldbus-1 -lsqlite3

# ifeq (1,${USENETWORK})
# 	SOURCE +=  NetWork/Base/RelechBase.cpp NetWork/Base/BaseServer.cpp NetWork/Base/BaseClient.cpp
# 	CFLAGS += -DUSENETWORK 
# endif

####################################################################################################
SOURCE +=  Common.cpp stdafx.cpp
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Media ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Dbus ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Database ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Util ) )
####################################################################################################
.PHONY: clean install
####################################################################################################
OBJS = $(SOURCE:%.cpp=obj.$(TARGET)/%.o)
DEPS = $(SOURCE:%.cpp=obj.$(TARGET)/%.d)
default: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX)  $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@ $(STATICLIB)

obj.$(TARGET)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

####################################################################################################
clean:
	rm -rf $(TARGET) $(LN_TARGET) $(OBJS) $(DEPS) obj obj.$(TARGET)
	rm -rf $(ROOTDIR)/include/Common/
	rm -rf $(ROOTDIR)/libs/$(TARGET)
install:
	@mkdir -p $(ROOTDIR)/include/Common/
	@mkdir -p $(ROOTDIR)/include/Common/Dbus
	@mkdir -p $(ROOTDIR)/include/Common/Database
	@mkdir -p $(ROOTDIR)/include/Common/Util
	@mkdir -p $(ROOTDIR)/libs
	@cp -r $(shell pwd)/Dbus/libdbus.h $(ROOTDIR)/include/Common/Dbus/
	@cp -r $(shell pwd)/Database/DbDriver.h $(ROOTDIR)/include/Common/Database/
	@cp -r $(shell pwd)/Util/CommonSdkUtil.h $(ROOTDIR)/include/Common/Util/
	@cp -r $(shell pwd)/Util/CriticalSectionManager.h $(ROOTDIR)/include/Common/Util/
	@cp -r $(shell pwd)/Common.h $(ROOTDIR)/include/Common/
	@cp -r $(shell pwd)/$(TARGET) $(ROOTDIR)/libs/


