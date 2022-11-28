TARGET = server.fcgi
LDFLAGS = -lm 
CFLAGS  += -MD -fPIE -fno-stack-protector \
		-I$(ROOTDIR)/include/json-c\
		-I$(ROOTDIR)/include/fcgi
			
STATICLIB =  -lpthread \
			$(ROOTDIR)/libs/libjson-c.a  \
			$(ROOTDIR)/libs/libfcgi.a 

ifeq ($(memcheck), y)
SOURCE += memwatch.c 
CFLAGS += -DMEMWATCH -DMW_STDIO 
endif
####################################################################################################
.PHONY: clean install
SOURCE +=  MessageHandler.c SocketClient.c RelechMap.c main.c  \
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
	$(CC) $(CFLAGS) $(LDFLAGS)  -c -o $@ $<

####################################################################################################
clean:
	rm -rf obj.$(TARGET)
	rm -rf $(TARGET)
install:
	@cp ./server.fcgi $(ROOTDIR)/lighttpd/fcgi/

	