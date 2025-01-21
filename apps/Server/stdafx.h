#ifndef __STDAFX_H
#define __STDAFX_H

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
#include <json.h>
#include <sys/types.h>
#include <dirent.h>
#include <sqlite3.h>
#include <limits.h>
#include "md5.h"
#include "Base64Coding.h"
#include "MessageDefine.h"
#include "memwatch.h"

#define MAX_PATH 255
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0x2000
#endif
typedef void *LPVOID;
typedef void* (*PTHREAD_START_ROUTINE)(
LPVOID lpThreadParameter
);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef unsigned long       DWORD;

typedef enum
{
    GPSTYPE_UNKNOW,
    GPSTYPE_NORMAL,
    GPSTYPE_BAIDU
}GPSTYPE;

//typedef int                 BOOL;
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#define BOOL int
#define TRUE 1
#define FALSE 0


#ifndef NULL
#define NULL ((void *)0)
#endif

#define SD_BOTH SHUT_RDWR
#ifdef __x86_64__
typedef uint64_t SOCKET;
#else
typedef uint32_t SOCKET;
#endif

typedef void* HANDLE;
#define SOCKET_ERROR            (-1)


#ifndef CRITICAL_SECTION
#define CRITICAL_SECTION pthread_mutex_t
#endif

#define INFINITE 0
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

extern pthread_t LinuxCreateThread(pthread_attr_t* lpThreadAttributes, LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter);
#define CreateThread(lpThreadAttributes,dwStackSize,lpStartAddress,lpParameter,dwCreationFlags,lpThreadId) (void*)(pthread_t)LinuxCreateThread(lpThreadAttributes, lpStartAddress, lpParameter);
#define WaitForSingleObject(threadid, status) do{pthread_join((pthread_t)threadid, NULL); threadid = NULL;}while(0);
#define CloseHandle(threadid) do{threadid = NULL;}while(0);
#define InitializeCriticalSection(section) pthread_mutex_init(section, NULL);
#define DeleteCriticalSection(section) pthread_mutex_destroy(section);
#define EnterCriticalSection(section)  pthread_mutex_lock(section);
#define LeaveCriticalSection(section)  pthread_mutex_unlock(section);
#define TryEnterCriticalSection(section) 0 == pthread_mutex_trylock(section)?TRUE:FALSE;
#define closesocket(socketfd) close(socketfd);
#define shutdown(socketfd, type) shutdown(socketfd, type);
#define Sleep(MillSec)  usleep(MillSec*1000);
#define LONGSLEEPPRE(SEC, LIJI)  int iCount = 0; if(LIJI){iCount = SEC;}
#define LONGSLEEP(SEC) if(iCount >= SEC){iCount = 0;}else{iCount++;Sleep(1000);continue;}

extern int SERVERPORT ;
extern int CASTPORT ;
extern BOOL g_bSamba;
extern BOOL g_bEth;
extern BOOL g_bWlan;


#define CASTTAG "PhotoVideo"

extern char FOLD_PREFIX[255];
extern char FOLDTHUMB_PREFIX[255];
extern char TMPFILEPATH[255];
extern char FOLDEX_PREFIX[255];
extern char TODIR[255];
extern BOOL g_bSupportGpsGet;
#define MEDIATYPE_IMAGE 1
#define MEDIATYPE_VIDEO 2
//#define __IOS_


#include  "FileUtil.h"
#endif
