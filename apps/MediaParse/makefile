#make arm=y memcheck=y
#make arm=fsl memcheck=y
#make arm=n memcheck=y
TARGET = MediaParse

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
CFLAGS  += -std=gnu++11 -g -Wall -MD -fPIE -fpermissive -fno-stack-protector -Wno-conversion-null \
		-DBRANCH="\$(BRANCH)\" -DBUILDDATE="\$(BUILDDATE)\" \
		-I$(ROOTDIR)/include/exiv2 \
		-I$(ROOTDIR)/include/nlohmannjson \
		-I$(ROOTDIR)/include/nlohmannjson/nlohmann \
		-I$(ROOTDIR)/include/Common \
		-I$(ROOTDIR)/include/dbus\
		-I$(ROOTDIR)/include/sqlite \
		-I$(ROOTDIR)/include/ffmpeg \
		-I./SyncToDevice \
		-I./Media \
		-I./Disk \
		-I./Backup \
		-I./Util \
		-I./DataBase \
		-I.

STATICLIB = -lpthread -L$(ROOTDIR)/libs/ -lexiv2 -lexpat -lsqlite3 -ldbus-1 -lcommon \
			-lavformat -lavfilter -lavcodec -lswresample -lavutil -lswscale 

ifeq ($(LIBDLPATH)/libdl.so, $(wildcard $(LIBDLPATH)/libdl.so))
STATICLIB += $(LIBDLPATH)/libdl.so
else
STATICLIB += -ldl
endif
.PHONY: clean install
####################################################################################################
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Media ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Disk ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Util ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, Backup ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, DataBase ) )
SOURCE +=  $(wildcard $(addsuffix /*.cpp, SyncToDevice ) )

SOURCE += stdafx.cpp MessageHandler.cpp main.cpp  
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
	
