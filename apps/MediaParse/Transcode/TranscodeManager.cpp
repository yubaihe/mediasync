#include "TranscodeManager.h"
#include "../Util/CommonUtil.h"
#include "../Util/JsonUtil.h"
CTranscodeManager* CTranscodeManager::m_pInstance = NULL;
CTranscodeManager::CTranscodeManager()
{
    InitializeCriticalSection(&m_Section);
}

CTranscodeManager::~CTranscodeManager()
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CTranscodeItem*>::iterator itor = m_pTransCodeItemMap.begin();
    for(; itor != m_pTransCodeItemMap.end();)
    {
        CTranscodeItem* pItem = itor->second;
        delete pItem;
        m_pTransCodeItemMap.erase(itor++);
    }
    LeaveCriticalSection(&m_Section);
    DeleteCriticalSection(&m_Section);
}
CTranscodeManager* CTranscodeManager::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CTranscodeManager();
    }
    return m_pInstance;
}
void CTranscodeManager::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
time_t CTranscodeManager::GetVideoDurationSec(string strVideo)
{
    string strRet = CCommonUtil::ExecuteCmd("ffprobe -v quiet -print_format json -show_format -select_streams v -show_streams %s", strVideo.c_str());
    //printf("====\n%s\n==\n", strRet.c_str());
    nlohmann::json jsonRoot;
    MediaParse::CJsonUtil::FromString(strRet, jsonRoot);
    if(MediaParse::CJsonUtil::CheckKey("format", jsonRoot) && 
        MediaParse::CJsonUtil::CheckKey("duration", jsonRoot["format"]))
    {
        string strDur = jsonRoot["format"]["duration"];
        return (int)(atof(strDur.c_str())*1000);
    }
    else
    {
        return 0;
    }
}
string CTranscodeManager::TransFile(string strFrom, string strTo, time_t iDurSec, int iItemID)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CTranscodeItem*>::iterator itor = m_pTransCodeItemMap.begin();
    for(; itor != m_pTransCodeItemMap.end(); ++itor)
    {
        if(itor->second->GetItemID() == iItemID)
        {
            string strIdentify = itor->second->GetIdentify();
            LeaveCriticalSection(&m_Section);
            return strIdentify;
        }
    }
    LeaveCriticalSection(&m_Section);
    string strBuffer = strFrom;
    strBuffer.append(strTo);
    string strMd5 = CCommonUtil::GetMd5(strBuffer.c_str());
    // printf("Buffer:%s\nMd5:%s\n", strBuffer.c_str(), strMd5.c_str());
    EnterCriticalSection(&m_Section);
    itor = m_pTransCodeItemMap.find(strMd5);
    if(itor == m_pTransCodeItemMap.end())
    {
        CTranscodeItem* pItem = new CTranscodeItem(strFrom, strTo, iDurSec, iItemID, strMd5);
        m_pTransCodeItemMap.insert(std::pair<std::string, CTranscodeItem*>(strMd5, pItem));
        pItem->Start();
    }
    LeaveCriticalSection(&m_Section);
    return strMd5;
}
void CTranscodeManager::ClearCache()
{
    EnterCriticalSection(&m_Section);
    map<std::string, CTranscodeItem*>::iterator itor = m_pTransCodeItemMap.begin();
    while (itor != m_pTransCodeItemMap.end()) 
    {
        if (TRUE == itor->second->CheckExit()) 
        {
            delete itor->second;
             itor->second = NULL;
            itor = m_pTransCodeItemMap.erase(itor);
        }
        else
        {
            ++itor;
        }
    }
    LeaveCriticalSection(&m_Section);
}
void CTranscodeManager::TerminalTrans(string strIdentify)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CTranscodeItem*>::iterator itor = m_pTransCodeItemMap.find(strIdentify);
    if(itor == m_pTransCodeItemMap.end())
    {
        LeaveCriticalSection(&m_Section);
        return;
    }
    LeaveCriticalSection(&m_Section);
    itor->second->Stop();
}
void CTranscodeManager::GetTransInfo(string strIdentify, DWORD& iDurationSec, DWORD& iCurSec, int& iPrecent)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CTranscodeItem*>::iterator itor = m_pTransCodeItemMap.find(strIdentify);
    if(itor != m_pTransCodeItemMap.end())
    {
        itor->second->GetTransInfo(iDurationSec, iCurSec, iPrecent);
    }
    LeaveCriticalSection(&m_Section);
}
void CTranscodeManager::TransEnd(string strIdentify)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CTranscodeItem*>::iterator itor = m_pTransCodeItemMap.find(strIdentify);
    if(itor != m_pTransCodeItemMap.end())
    {
        itor->second->TransEnd();
    }
    LeaveCriticalSection(&m_Section);
}