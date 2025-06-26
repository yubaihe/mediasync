TARGET = server.fcgi
LDFLAGS = -lm 
CFLAGS  += -MD -fPIE -fno-stack-protector -std=gnu++11\
		-I$(ROOTDIR)/include/dbus\
		-I$(ROOTDIR)/include/Common \
		-I$(ROOTDIR)/include/fcgi \
		-I$(ROOTDIR)/include/sqlite \
		-I.
			
STATICLIB =  -lpthread -L$(ROOTDIR)/libs/ -lfcgi -ldbus-1 -lcommon

ifeq ($(memcheck), y)
SOURCE += memwatch.c 
CFLAGS += -DMEMWATCH -DMW_STDIO 
endif
####################################################################################################
.PHONY: clean install
SOURCE += MessageHandler.cpp main.cpp  \
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
	@cp ./server.fcgi $(ROOTDIR)/lighttpd/fcgi/

	