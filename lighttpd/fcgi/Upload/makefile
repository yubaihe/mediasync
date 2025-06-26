TARGET = upload.fcgi
LDFLAGS = -lm 
CFLAGS  += -std=gnu++11 -MD -fPIE -fno-stack-protector \
		-I$(ROOTDIR)/include/dbus\
		-I$(ROOTDIR)/include/Common \
		-I$(ROOTDIR)/include/fcgi \
		-I$(ROOTDIR)/include/sqlite \
		-I$(ROOTDIR)/include/nlohmannjson/ \
		-I$(ROOTDIR)/include/nlohmannjson/nlohmann/ \
		-I.
			
STATICLIB =  -lpthread -L$(ROOTDIR)/libs/ -lfcgi -ldbus-1 -lcommon \

####################################################################################################
.PHONY: clean install
SOURCE +=  main.cpp  \
#SOURCE = test2.c\
####################################################################################################
OBJS = $(SOURCE:%.cpp=obj.$(TARGET)/%.o)
DEPS = $(SOURCE:%.cpp=obj.$(TARGET)/%.d)

default:
	make $(TARGET)
	
$(TARGET): $(OBJS)
	$(CXX) obj.$(TARGET)/*.o $(LDFLAGS) $(STATICLIB) -o $@

obj.$(TARGET)/%.o: %.cpp
	@mkdir -p obj.$(TARGET)
	$(CXX) $(CFLAGS) $(LDFLAGS)  -c -o $@ $<

####################################################################################################
clean:
	rm -rf obj.$(TARGET)
	rm -rf $(TARGET)
install:
	@cp ./upload.fcgi $(ROOTDIR)/lighttpd/fcgi/
	