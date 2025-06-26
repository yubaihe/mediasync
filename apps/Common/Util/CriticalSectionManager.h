#pragma once
#include "stdafx.h"
#include <map>
#include <pthread.h>  
#include "Common.h"
// #ifdef EnterSection
//     #define EnterCriticalSection(section)  EnterSection((section), __FILE__, __LINE__, __func__);
//     #define LeaveCriticalSection(section)  LeaveSection((section), __FILE__, __LINE__, __func__);
// #else
//     #define EnterCriticalSection(section)  pthread_mutex_lock(section);
//     #define LeaveCriticalSection(section)  pthread_mutex_unlock(section);
// #endif
using namespace std;
typedef void* LPVOID;
typedef void* HANDLE;
#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif
#define MAX_FUNCTION_LEN 100
#define MAX_FILENAME_LEN 100
struct SectionItem
{
    char szFunction[MAX_FUNCTION_LEN];
    char szFile[MAX_FILENAME_LEN];
    int iLine;
    uint32_t iTimeSec;
};
#define EnterSection(section, file, line, function) CCriticalSectionManager::GetInstance()->Enter((section), (file), (line), (function));
#define LeaveSection(section, file, line, function) CCriticalSectionManager::GetInstance()->Leave((section), (file), (line), (function));
class CCriticalSectionManager
{
public:
    CCriticalSectionManager();
    ~CCriticalSectionManager();
    static CCriticalSectionManager* GetInstance();
    static void Release();
    void Enter(pthread_mutex_t* pSection, const char* pszFile, int iLine, const char* pszFunction);
    void Leave(pthread_mutex_t* pSection, const char* pszFile, int iLine, const char* pszFunction);
private:
    static LPVOID TimerProc(LPVOID lpParam);
    time_t GetCurRunTimeSec();
private:
    BOOL m_bExit;
    HANDLE m_hTimer;
    pthread_mutex_t m_Mutex;
    map<pthread_mutex_t*, SectionItem> m_SectionItemMap;
    static CCriticalSectionManager* m_pInstance;
};

