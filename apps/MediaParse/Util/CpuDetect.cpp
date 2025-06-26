#include "CpuDetect.h"
#include <unistd.h>
#include "CommonUtil.h"
CCpuDetect* CCpuDetect::m_pInstance = NULL;
CCpuDetect::CCpuDetect()
{
    m_iUsage = 0;
    m_bExit = FALSE;
    memset(&m_CpuInfo, 0, sizeof(m_CpuInfo));
    m_hTimer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CpuTimerProc, this, 0, NULL);
}

CCpuDetect::~CCpuDetect()
{
    m_bExit = TRUE;
    if(NULL != m_hTimer)
    {
        WaitForSingleObject(m_hTimer, INFINITE);
        CloseHandle(m_hTimer);
        m_hTimer = NULL;
    }
}
CCpuDetect* CCpuDetect::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CCpuDetect();
    }
    return m_pInstance;
}
void CCpuDetect::Release()
{
    if(NULL != m_pInstance)
    {
       delete m_pInstance;
       m_pInstance = NULL;
    }
}

DWORD CCpuDetect::CpuTimerProc(void* lpParameter)
{
    CCpuDetect* pCpuDetect = (CCpuDetect*)lpParameter;
    int i = 0;
    while (FALSE == pCpuDetect->m_bExit)
    {
        Sleep(100);
        i++;
        if(i == 10)
        {
            i = 0;
            pCpuDetect->Calculate();
        }
    }
    
    return 1;
}
void CCpuDetect::Calculate()
{
    FILE* pFile = fopen("/proc/stat", "r");
    if(NULL == pFile)
    {
        m_iUsage = -1;
        return;
    }
    char szBuffer[1024] = {0};
    fgets(szBuffer, sizeof(szBuffer), pFile);
    fclose(pFile);

    if(memcmp(szBuffer, "cpu ", strlen("cpu ")) != 0)
    {
        m_iUsage = -1;
        return;
    }
    CpuInfo cpuInfo;
    memset(&cpuInfo, 0, sizeof(cpuInfo));
    sscanf(szBuffer, "%s %u %u %u %u %u %u %u", cpuInfo.szName, &cpuInfo.iUser, &cpuInfo.iNice, 
                &cpuInfo.iSystem, &cpuInfo.iIdle, &cpuInfo.iLowait, &cpuInfo.iIrq, &cpuInfo.iSoftirq);
    // cpu的占用率 = （当前时刻的任务占用cpu总时间-前一时刻的任务占用cpu总时间）/ （当前时刻 - 前一时刻的总时间
    m_iUsage = GetCpuUse(&m_CpuInfo, &cpuInfo);
    m_CpuInfo = cpuInfo;
}
int CCpuDetect::GetCpuUse(CpuInfo* pOld, CpuInfo* pNew)
{
    unsigned long iOld = (unsigned long)(pOld->iUser + pOld->iNice + pOld->iSystem + pOld->iIdle + pOld->iLowait + pOld->iIrq + pOld->iSoftirq);
    unsigned long iNew = (unsigned long)(pNew->iUser + pNew->iNice + pNew->iSystem + pNew->iIdle + pNew->iLowait + pNew->iIrq + pNew->iSoftirq); 
    double dSum = iNew - iOld;
    double diDle = pNew->iIdle - pOld->iIdle;
    return (int)((dSum - diDle) / dSum*100);
}
int CCpuDetect::GetUsage()
{
    return m_iUsage;
}
int CCpuDetect::GetCpuCount()
{
    string strCpuCount = CCommonUtil::ExecuteCmd("grep -c ^processor /proc/cpuinfo");
    return atoi(strCpuCount.c_str());
}
int CCpuDetect::GetMemUsage()
{
    FILE* pFile = fopen("/proc/meminfo", "r");
    if (pFile == NULL) 
    {
        return 0;
    }
    char szBuffer[1024] = {0};
    long long iTotalMemory = -1;
    long long iFreeMemory = -1;
    long long iAvailableMemory = -1;
    
    while (fgets(szBuffer, sizeof(szBuffer), pFile) != NULL)
    {
        if (strncmp(szBuffer, "MemTotal:", 9) == 0) 
        {
            sscanf(szBuffer, "MemTotal: %lld kB", &iTotalMemory);
            continue;
        }
        if (strncmp(szBuffer, "MemFree:", 8) == 0)
        {
            sscanf(szBuffer, "MemFree: %lld kB", &iFreeMemory);
            continue;
        }
        if (strncmp(szBuffer, "MemAvailable:", 13) == 0)
        {
            sscanf(szBuffer, "MemAvailable: %lld kB", &iAvailableMemory);
            continue;
        }
    }
    long long iUsageMemory = 0;
    if (iAvailableMemory != -1) 
    {
        iUsageMemory = iTotalMemory - iAvailableMemory;
    }
    else 
    {
        iUsageMemory = iTotalMemory - iFreeMemory;
    }

    int iUsage = (int)((iUsageMemory / (iTotalMemory*1.0f)) * 100.0);

    fclose(pFile);
    pFile = NULL;
    return iUsage;
}