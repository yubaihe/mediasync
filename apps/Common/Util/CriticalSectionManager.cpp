#include "CriticalSectionManager.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <sys/sysinfo.h>
#include <unistd.h>
#define MAX_LOCKTIMESEC 60 //最多锁一分钟
CCriticalSectionManager* CCriticalSectionManager::m_pInstance = NULL;
CCriticalSectionManager::CCriticalSectionManager()
{
    // printf("CCriticalSectionManager::CCriticalSectionManager\n");
    pthread_mutex_init(&m_Mutex, NULL);
    m_bExit = FALSE;
    m_hTimer = CreateThread(NULL, 0, TimerProc, this, 0, NULL);
}
CCriticalSectionManager::~CCriticalSectionManager()
{
    m_bExit = TRUE;
    if(NULL != m_hTimer)
    {
        WaitForSingleObject(m_hTimer, INFINITE);
        CloseHandle(m_hTimer);
        m_hTimer = NULL;
    }
    pthread_mutex_destroy(&m_Mutex);
}
CCriticalSectionManager* CCriticalSectionManager::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CCriticalSectionManager();
    }
    return m_pInstance;
}
void CCriticalSectionManager::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
time_t CCriticalSectionManager::GetCurRunTimeSec()
{
    struct sysinfo info;
    sysinfo(&info);
    return info.uptime;
}
void CCriticalSectionManager::Enter(pthread_mutex_t* pSection, const char* pszFile, int iLine, const char* pszFunction)
{
    pthread_mutex_lock(pSection);
    
    pthread_mutex_lock(&m_Mutex);
    map<pthread_mutex_t*, SectionItem>::iterator itor = m_SectionItemMap.find(pSection);
    if(itor != m_SectionItemMap.end())
    {
        pthread_mutex_unlock(&m_Mutex);
        LOGE("ReEnter Old:%s===>%s==>%d\n", itor->second.szFunction, itor->second.szFile, itor->second.iLine);
        LOGE("ReEnter New:%s===>%s==>%d\n", pszFunction, pszFile, iLine);
        sleep(1);
        assert(itor == m_SectionItemMap.end());
        return;
    }
    
    SectionItem item;
    memset((char*)&item, 0, sizeof(item));
    item.iLine = iLine;
    strcpy(item.szFile, pszFile);
    strcpy(item.szFunction, pszFunction);
    item.iTimeSec = GetCurRunTimeSec();
    m_SectionItemMap.insert(pair<pthread_mutex_t*, SectionItem>(pSection, item));
    pthread_mutex_unlock(&m_Mutex);
}
void CCriticalSectionManager::Leave(pthread_mutex_t* pSection, const char* pszFile, int iLine, const char* pszFunction)
{
    pthread_mutex_lock(&m_Mutex);
    map<pthread_mutex_t*, SectionItem>::iterator itor = m_SectionItemMap.find(pSection);
    if(itor != m_SectionItemMap.end())
    {
         m_SectionItemMap.erase(itor);
    }
    // printf("Leave:%s===>%s==>%d\n", itor->second.szFunction, itor->second.szFile, itor->second.iLine);
    pthread_mutex_unlock(&m_Mutex);
    pthread_mutex_unlock(pSection);
}

LPVOID CCriticalSectionManager::TimerProc(LPVOID lpParam)
{
    CCriticalSectionManager* pCriticalSectionManager = (CCriticalSectionManager*)lpParam;
    uint32_t iPrevDetectTimeSec = 0;
    while(FALSE == pCriticalSectionManager->m_bExit)
    {
        uint32_t iCurTimeSec = pCriticalSectionManager->GetCurRunTimeSec();
        if(iCurTimeSec - iPrevDetectTimeSec < MAX_LOCKTIMESEC/3)
        {
            Sleep(500);
            continue;
        }
        iPrevDetectTimeSec = iCurTimeSec;
        
        pthread_mutex_lock(&pCriticalSectionManager->m_Mutex);
        map<pthread_mutex_t*, SectionItem>::iterator itor = pCriticalSectionManager->m_SectionItemMap.begin();
        for(; itor != pCriticalSectionManager->m_SectionItemMap.end(); ++itor)
        {
            if(iCurTimeSec - itor->second.iTimeSec >= MAX_LOCKTIMESEC)
            {
                pthread_mutex_unlock(&pCriticalSectionManager->m_Mutex);
                LOGE("CriticalSection Long:%s===>%s==>%d\n", itor->second.szFunction, itor->second.szFile, itor->second.iLine);
                Sleep(1000);
                assert(iCurTimeSec - itor->second.iTimeSec < MAX_LOCKTIMESEC);
            }
        }
        pthread_mutex_unlock(&pCriticalSectionManager->m_Mutex);
    }
    return 0;
}