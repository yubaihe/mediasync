#pragma once
#include "stdafx.h"
#include "MediaJishuTable.h"
#include "MediaScan.h"
typedef enum
{
    CLEARCACHE_FAILED = -1,
    CLEARCACHE_PROCESS,
    CLEARCACHE_SUCCESS
}CLEARCACHESTATUS;
class CClearCache
{
public:
    CClearCache();
    ~CClearCache();
    static CClearCache* GetInstance();
    static void Release();
    BOOL Start();
    BOOL IsRuning();
    void GetStatus(CLEARCACHESTATUS& eStatus, int& iPrecent);
private:
    static DWORD ClearCacheProc(LPVOID* lpParameter);
    static void CallBack(int iPrecent, LPVOID* lpParameter);
    void ScanMediaParse();
private:
    CLEARCACHESTATUS m_eStatus;
    int m_iStepCount;
    int m_iCurStepIndex;
    int m_iPrecent;
    HANDLE m_hClearHandle;
    CMediaScan m_MediaScan;
    static CClearCache* m_pInstance;
};

