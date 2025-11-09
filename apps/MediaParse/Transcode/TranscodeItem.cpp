#include "TranscodeItem.h"
#include "../Util/CommonUtil.h"
#include "../Util/FileUtil.h"
#include "../Util/DbusUtil.h"
CTranscodeItem::CTranscodeItem(string strFrom, string strTo, time_t iDurSec, int iItemID, string strIdentify)
{
    m_iItemID = iItemID;
    m_strFrom = strFrom;
    m_strTo = strTo;
    m_iDurSec = iDurSec;
    m_hTranscode = NULL;
    m_bExit = TRUE;
    m_strIdentify = strIdentify;
    m_iTimeSec = CCommonUtil::CurTimeSec();
    InitializeCriticalSection(&m_Section);
}

CTranscodeItem::~CTranscodeItem()
{
    Stop();
    DeleteCriticalSection(&m_Section);
    printf("CTranscodeItem::~CTranscodeItem:%s\n", m_strCommand.c_str());
}
void CTranscodeItem::Stop()
{
    if(TRUE == m_bExit)
    {
        return;
    }
    m_bExit = TRUE;
    if(NULL != m_pProcess)
    {
        DWORD iPid = GetTranscodePid(m_strCommand);
        if(iPid > 0)
        {
            CCommonUtil::ExecuteCmdWithOutReplay("kill -9 %ld", iPid);
        }
        pclose(m_pProcess);
        m_pProcess = NULL;
    }
    if(NULL != m_hTranscode)
    {
        WaitForSingleObject(m_hTranscode, INFINITE);
        CloseHandle(m_hTranscode);
        m_hTranscode = NULL;
    }
    if(TRUE == CCommonUtil::CheckFileExist(m_strTo.c_str()))
    {
        CCommonUtil::RemoveFile(m_strTo.c_str());
    }
    m_iTimeSec = 0;
}
void CTranscodeItem::Start()
{
    Stop();
    m_bExit = FALSE;
    m_iCurTimeSec = 0;
    m_bFinish = FALSE;
    m_iTimeSec = CCommonUtil::CurTimeSec();
    m_hTranscode = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TranscodeProc, this, 0, NULL);
}

DWORD CTranscodeItem::GetTranscodePid(string strCommand)
{
    string strRet = CCommonUtil::ExecuteCmd("ps -eo pid,cmd | grep '%s' | grep -v grep", strCommand.c_str());
    printf("==============>\n%s\n===========>\n", strRet.c_str());
    std::vector<std::string> vec = CCommonUtil::StringSplit(strRet, "\n");
    for(size_t i = 0; i < vec.size(); ++i)
    {
        string strProcessInfo = CCommonUtil::TrimStr(vec[i]);
        std::vector<std::string> vec2 = CCommonUtil::StringSplit(strProcessInfo, " ");
        DWORD iPid = atol(vec2[0].c_str());
        string strCmd = strProcessInfo.substr(vec2[0].length() + 1);
        if(strCmd == strCommand)
        {
            return iPid;
        }
    }
    return -1;
}

DWORD CTranscodeItem::TranscodeProc(void* lpParameter)
{
    CTranscodeItem* pTranscodeItem = (CTranscodeItem*)lpParameter;
    pTranscodeItem->m_strCommand = CCommonUtil::StringFormat("ffmpeg -i %s %s -f mp4 -progress -", pTranscodeItem->m_strFrom.c_str(), pTranscodeItem->m_strTo.c_str());
    int iBufferLen = 1024;
    char szBuffer[iBufferLen] = {0};
    printf("===>%s\n", pTranscodeItem->m_strCommand.c_str());
    if((pTranscodeItem->m_pProcess = popen(pTranscodeItem->m_strCommand.c_str(), "r")) == NULL)
    {
        printf("===>%s\n", "Command failed");
        return -1;
    }
    size_t iTagLen = strlen("out_time_ms=");
    while(fgets(szBuffer, iBufferLen, pTranscodeItem->m_pProcess) != NULL)
    {
        pTranscodeItem->m_iTimeSec = CCommonUtil::CurTimeSec();
        if(strlen(szBuffer) > iTagLen && 0 == memcmp(szBuffer, "out_time_ms=", iTagLen))
        {
            szBuffer[strlen(szBuffer) - 1] = '\0';
            char szTimeMilSec[50] = {0};
            memcpy(szTimeMilSec, szBuffer + iTagLen, strlen(szBuffer) - iTagLen);
            if(strcmp("N/A", szTimeMilSec) != 0)
            {
                pTranscodeItem->m_iCurTimeSec = atol(szTimeMilSec)/1000;
                printf("<===%ld====%ld======%d====>\n", pTranscodeItem->m_iCurTimeSec, pTranscodeItem->m_iDurSec, (int)(pTranscodeItem->m_iCurTimeSec/(pTranscodeItem->m_iDurSec*1.0f)*99));
            }
        }
    //    printf("\n===%s==\n", szBuffer);
        memset(szBuffer, 0, iBufferLen);
    }
    printf("CurSec:%ld===Total:%ld==\n", pTranscodeItem->m_iCurTimeSec, pTranscodeItem->m_iDurSec);
    if(FALSE == pTranscodeItem->m_bExit)
    {
        string strMd5 = CCommonUtil::GetMd5(pTranscodeItem->m_strTo.c_str());
        CDbusUtil::TranscodeFinish(pTranscodeItem->m_iItemID, pTranscodeItem->m_strTo, strMd5, pTranscodeItem->m_strIdentify);
    }
    printf("CTranscodeItem::TranscodeProc end\n");
    return 1;
}
BOOL CTranscodeItem::CheckExit()
{
    if(CCommonUtil::CurTimeSec() - m_iTimeSec > 60*5)
    {
        return TRUE;
    }
    return FALSE;
}
int CTranscodeItem::GetPrecent()
{
    if(m_bFinish == TRUE)
    {
        m_iTimeSec = 0;
        return 100;
    }
    return (int)(m_iCurTimeSec/(m_iDurSec*1.0f)*99);
}
void CTranscodeItem::GetTransInfo(DWORD& iDurationSec, DWORD& iCurSec, int& iPrecent)
{
    iDurationSec = m_iDurSec;
    iCurSec = m_iCurTimeSec;
    iPrecent = GetPrecent();
}
int CTranscodeItem::GetItemID()
{
    return m_iItemID;
}
string CTranscodeItem::GetIdentify()
{
    return m_strIdentify;
}
void CTranscodeItem::TransEnd()
{
    m_bFinish = TRUE;
    m_iTimeSec = CCommonUtil::CurTimeSec();
    //End以后不能立即结束 不然进度获取不到
}