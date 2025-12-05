#make arm=y memcheck=y
#make arm=fsl memcheck=y
#make arm=n memcheck=y
TARGET = Server

BRANCHNAME="$(shell git rev-parse --abbrev-ref HEAD)" # git分支号
COMMIT="$(shell git log --pretty=format:"%h" -1)" # git提交号
BRANCH="$(strip $(BRANCHNAME))-$(COMMIT)"
BUILDDATE = "$(shell date '+%F %T')"

LDFLAGS = -lm
ifneq (,$(filter $(BUILDTYPE),DEBUG))
	CFLAGS = -g
else
	CFLAGS = 
endif
CFLAGS  += -std=gnu++11 -Wall -MD -fPIE -fpermissive -fno-stack-protector -Wno-conversion-null \
		-DBRANCH="\$(BRANCH)\" -DBUILDDATE="\$(BUILDDATE)\" \
		-I$(ROOTDIR)/include/nlohmannjson \
		-I$(ROOTDIR)/include/nlohmannjson/nlohmann \
		-I$(ROOTDIR)/include/Common \
		-I$(ROOTDIR)/include/dbus\
		-I$(ROOTDIR)/include/sqlite \
		-I$(ROOTDIR)/include/curl \
		-I./Util \
		-I./BroadCast \
		-I./DataBase \
		-I$(ROOTDIR) \
		-I.

STATICLIB = -lpthread -L$(ROOTDIR)/libs/ -lsqlite3 -lcurl -ldbus-1 -lcommon 
			

# ifeq ($(LIBDLPATH)/libdl.so, $(wildcard $(LIBDLPATH)/libdl.so))
# STATICLIB += $(LIBDLPATH)/libdl.so
# else
# STATICLIB += -ldl
# endif
.PHONY: clean install
####################################################################################################
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Util ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, DataBase ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, BroadCast ) )
SOURCE += stdafx.cpp Config.cpp BroadCastServer.cpp ConnectServer.cpp ConnectServerMessage.cpp ClearCache.cpp MediaScan.cpp GpsManager.cpp main.cpp Butler.cpp 
####################################################################################################

OBJS = $(SOURCE:%.cpp=obj.$(TARGET)/%.o)
DEPS = $(SOURCE:%.cpp=obj.$(TARGET)/%.d)
default:
	rm -rf memwatch.log
	make $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(STATICLIB) -o $@

obj.$(TARGET)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(LDFLAGS)  -c -o $@ $<

####################################################################################################
clean:
	rm -rf obj.$(TARGET)
	rm -rf $(TARGET)
	rm -rf $(ROOTDIR)/apps/$(TARGET)
install:
	@mkdir $(ROOTDIR)/apps/$(TARGET) -p
	@cp $(TARGET) $(ROOTDIR)/apps/$(TARGET)/
	@cp Config.json $(ROOTDIR)/apps/$(TARGET)/
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/apps/$(TARGET)/Config.json
	
