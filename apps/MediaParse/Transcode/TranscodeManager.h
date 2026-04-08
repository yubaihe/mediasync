#pragma once
#include "../stdafx.h"
#include "TranscodeItem.h"
//ffprobe -v quiet -print_format json -show_format -select_streams v -show_streams test.mkv
//40.833000
//ffmpeg -i test.mkv test.mp4 -f mp4 -progress -
//out_time_ms=40666667 也可能是 out_time_ms=N/A

class CTranscodeManager
{
public:
    CTranscodeManager();
    ~CTranscodeManager();
    static CTranscodeManager* GetInstance();
    static void Release();
    static time_t GetVideoDurationSec(string strVideo);
    string TransFile(string strFrom, string strTo, time_t iDurMilSec, int iItemID);
    void TerminalTrans(string strIdentify);
    void ClearCache();
    void GetTransInfo(string strIdentify, DWORD& iDurationSec, DWORD& iCurSec, int& iPrecent);
    void TransEnd(string strIdentify);
private:
    CRITICAL_SECTION m_Section;
    std::map<std::string, CTranscodeItem*> m_pTransCodeItemMap;
    static CTranscodeManager* m_pInstance;
};


