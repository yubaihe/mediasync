#make arm=y memcheck=y
#make arm=fsl memcheck=y
#make arm=n memcheck=y
TARGET = MetaParse

LDFLAGS = -lm

CFLAGS  += -std=gnu++11 -g -Wall  -MD -fPIE -fno-stack-protector \
		-I$(ROOTDIR)/include/exiv2 \
		-I$(ROOTDIR)/include/jsoncpp

STATICLIB = -lpthread $(ROOTDIR)/lib/libexiv2.so \
	$(ROOTDIR)/lib/libjsoncpp.a
			

ifeq ($(memcheck), y)
SOURCE += memwatch.c 
CFLAGS += -DMEMWATCH -DMW_STDIO 
endif

####################################################################################################
.PHONY: clean install
SOURCE +=    JsonUtil.cpp ImageParse.cpp VideoMetaParse.cpp CommonUtil.cpp main.cpp  \
####################################################################################################
OBJS = $(SOURCE:%.cpp=obj.$(TARGET)/%.o)
DEPS = $(SOURCE:%.cpp=obj.$(TARGET)/%.d)
CFLAGS  += -g  -std=c++11 -MD -fPIE -fno-stack-protector \
		-I$(ROOTDIR)/include/exiv2/include/ \
		-I$(ROOTDIR)/include/jsoncpp/include

STATICLIB = $(ROOTDIR)/libs/libexpat.so \
			$(ROOTDIR)/libs/libexiv2.so \
			$(ROOTDIR)/libs/libjsoncpp.a 
			
ifeq ($(LIBDLPATH)/libdl.so, $(wildcard $(LIBDLPATH)/libdl.so))
STATICLIB += $(LIBDLPATH)/libdl.so
else
STATICLIB += -ldl
endif
default:
	make $(TARGET)
	
$(TARGET): $(OBJS)
	$(CXX) obj.$(TARGET)/*.o $(LDFLAGS) $(STATICLIB) -o $@

obj.$(TARGET)/%.o: %.cpp
	@mkdir -p obj.$(TARGET)
	$(CXX) $(CFLAGS) $(APPFLAGS) $(LDFLAGS) -c -o $@ $<

####################################################################################################
clean:
	rm -rf obj.$(TARGET)
	rm -rf $(TARGET)
	rm -rf $(ROOTDIR)/apps/MetaParse
#	rm -rf /home/relech/share/media/**
#	rm -rf /tmp/mediasync
#	rm -rf mediasync.db 
install:
	@mkdir $(ROOTDIR)/apps/MetaParse -p
	@cp MetaParse $(ROOTDIR)/apps/MetaParse/
	
