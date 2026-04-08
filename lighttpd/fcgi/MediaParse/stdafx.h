#pragma once
#include<errno.h>
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h> 
#include <net/if.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <stdarg.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sqlite3.h>
#include <limits.h>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <string>
#include <cctype>
#include <list>
#include <map>
#include <algorithm>
#define MAX_MESSAGE_LEN 65535
using namespace std;
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0x2000
#endif

typedef void *LPVOID;
typedef void* (*PTHREAD_START_ROUTINE)(
LPVOID lpThreadParameter
);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;



