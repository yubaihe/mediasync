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
#include <sys/stat.h> 
#include <sys/types.h>
#include <stdarg.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sqlite3.h>
#include <limits.h>
#include "Common.h"
#include <vector>
#include <string.h>
#include <json.hpp>
#include <curl.h>
#include <thread> 

using namespace std;

#define MAX_MESSAGE_LEN 65535
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
// #define EnterCriticalSection(section)  pthread_mutex_lock(section);
// #define LeaveCriticalSection(section)  pthread_mutex_unlock(section);
#define EnterCriticalSection(section)  EnterSection((section), __FILE__, __LINE__, __func__);
#define LeaveCriticalSection(section)  LeaveSection((section), __FILE__, __LINE__, __func__);
#define TryEnterCriticalSection(section) 0 == pthread_mutex_trylock(section)?TRUE:FALSE;
#define closesocket(socketfd) close(socketfd);
#define shutdown(socketfd, type) shutdown(socketfd, type);
#define Sleep(MillSec)  usleep(MillSec*1000);
#define LONGSLEEPPRE(SEC, LIJI)  int iCount = 0; if(LIJI){iCount = SEC;}
#define LONGSLEEP(SEC) if(iCount >= SEC){iCount = 0;}else{iCount++;Sleep(1000);continue;}
#define PRINTERROR(format, ...) Server::CCommonUtil::ColorPrintf(Server::COLORTYPE_ERROR, format, ##__VA_ARGS__);
#define PRINTSUCCESS(format, ...) Server::CCommonUtil::ColorPrintf(Server::COLORTYPE_SUCCESS, format, ##__VA_ARGS__);
#define PRINTWARNING(format, ...) Server::CCommonUtil::ColorPrintf(Server::COLORTYPE_WARNING, format, ##__VA_ARGS__);
#define CASTTAG "PhotoVideo"
typedef enum
{
    GPSTYPE_UNKNOW,
    GPSTYPE_NORMAL,
    GPSTYPE_BAIDU
}GPSTYPE;