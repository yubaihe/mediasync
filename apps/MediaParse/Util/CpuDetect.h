#pragma once
#include "../stdafx.h"
#define MAX_CUPNAME_LEN 20
struct CpuInfo
{
    char szName[MAX_CUPNAME_LEN + 1];
    unsigned int iUser;
    unsigned int iNice;
    unsigned int iSystem;
    unsigned int iIdle;
    unsigned int iLowait;
    unsigned int iIrq;
    unsigned int iSoftirq;
};
class CCpuDetect
{
public:
    static CCpuDetect* GetInstance();
    static void Release();
    CCpuDetect();
    ~CCpuDetect();
    int GetUsage();
    int GetMemUsage();
    int GetCpuCount();
private:
    static DWORD CpuTimerProc(void* lpParameter);
    void Calculate();
    int GetCpuUse(CpuInfo* pOld, CpuInfo* pNew);
private:
    BOOL m_bExit;
    HANDLE m_hTimer;
    CpuInfo m_CpuInfo;
    int m_iUsage;
    static CCpuDetect* m_pInstance;
};


