#pragma once
#include <signal.h>
#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <regex>
#include <algorithm>
#include <map>
#include <cassert>
#include <atomic>
#include <stdexcept>
#include <cstring>
#include <limits>
#include "CommonSdkUtil.h"
// #include "CriticalSectionManager.h"

using namespace std;
#undef ASSERT
#define ASSERT assert
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define SD_BOTH SHUT_RDWR
#ifdef __x86_64__
typedef uint64_t SOCKET;
#else
typedef uint32_t SOCKET;
#endif


#define Sleep(MillSec)  usleep((MillSec)*1000);
typedef unsigned long       DWORD;

#define MAX_PATH 255
#define BOOL int
#define TRUE 1
#define FALSE 0

typedef void *HANDLE;

#ifndef CRITICAL_SECTION
#define CRITICAL_SECTION pthread_mutex_t
#endif
typedef void *LPVOID;
typedef void* (*PTHREAD_START_ROUTINE)(
LPVOID lpThreadParameter
);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
extern pthread_t LinuxCreateThread(pthread_attr_t* lpThreadAttributes, LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter);
#define CreateThread(lpThreadAttributes,dwStackSize,lpStartAddress,lpParameter,dwCreationFlags,lpThreadId) (HANDLE)LinuxCreateThread((lpThreadAttributes), (lpStartAddress), (lpParameter));
#define WaitForSingleObject(threadid, status) do{pthread_join((pthread_t)(threadid), NULL); (threadid) = NULL;}while(0);
#define CloseHandle(threadid) do{if(NULL != (threadid)){pthread_detach((pthread_t)(threadid));(threadid) = NULL;}}while(0);
#define InitializeCriticalSection(section) pthread_mutex_init((section), NULL);
#define DeleteCriticalSection(section) pthread_mutex_destroy(section);
#ifdef EnterSection
    #define EnterCriticalSection(section)  EnterSection((section), __FILE__, __LINE__, __func__);
    #define LeaveCriticalSection(section)  LeaveSection((section), __FILE__, __LINE__, __func__);
#else
    #define EnterCriticalSection(section)  pthread_mutex_lock(section);
    #define LeaveCriticalSection(section)  pthread_mutex_unlock(section);
#endif

#define closesocket(socketfd) close(socketfd);
#define shutdown(socketfd, type) shutdown((socketfd), (type));
#define SignalNotify(sem)                   sem_post(sem);
#define SignalCreate(sem, shared, value)    sem_init((sem), (shared), (value));
#define SignalWait(sem)                     sem_wait(sem);
#define SignalDestroy(sem)                  sem_destroy(sem);



#define LOGD(pszFormat, ...)  CCommonSdkUtil::Log(LOGLEVEL_DEBUG, __func__, __FILE__, __LINE__, pszFormat, ##__VA_ARGS__);
#define LOGI(pszFormat, ...)  CCommonSdkUtil::Log(LOGLEVEL_INFO, __func__, __FILE__, __LINE__, pszFormat, ##__VA_ARGS__);
#define LOGE(pszFormat, ...)  CCommonSdkUtil::Log(LOGLEVEL_ERROR, __func__, __FILE__, __LINE__, pszFormat, ##__VA_ARGS__);