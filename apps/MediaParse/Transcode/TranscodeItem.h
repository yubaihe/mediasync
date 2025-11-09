#pragma once
#include "../stdafx.h"
class CTranscodeItem
{
public:
    CTranscodeItem(string strFrom, string strTo, time_t iDurMilSec, int iItemID, string strIdentify);
    ~CTranscodeItem();
    void Start();
    void Stop();
    BOOL CheckExit();
    int GetPrecent();
    void GetTransInfo(DWORD& iDurationSec, DWORD& iCurSec, int& iPrecent);
    int GetItemID();
    string GetIdentify();
    void TransEnd();
private:
    DWORD GetTranscodePid(string strCommand);
    static DWORD TranscodeProc(void* lpParameter);
private:
    int m_iItemID;
    string m_strFrom;
    string m_strTo;
    time_t m_iDurSec;
    HANDLE m_hTranscode;
    BOOL m_bExit;
    FILE* m_pProcess;
    time_t m_iTimeSec;
    string m_strCommand;
    time_t m_iCurTimeSec;
    BOOL m_bFinish;
    string m_strIdentify;
    CRITICAL_SECTION m_Section;
};

