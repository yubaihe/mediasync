#pragma once
#include "../stdafx.h"
class CBackupTmpFoldMonitor
{

public:
    CBackupTmpFoldMonitor();
    ~CBackupTmpFoldMonitor();
    static CBackupTmpFoldMonitor* GetInstance();
    static void Release();
    void SetImmediately();
private:
    void RemoveTempFold();
    static DWORD TimerProc(void* lpParameter);
private:
    BOOL m_bExit;
    HANDLE m_hTimer;
    BOOL m_bImmediately;
    time_t m_iPrevClearTimeSec;
    static CBackupTmpFoldMonitor* m_pInstance;
};


