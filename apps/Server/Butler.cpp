#include "Butler.h"
#include "Config.h"
#include "Util/CommonUtil.h"
CButler* CButler::m_pInstance = NULL;
CButler::CButler()
{
    m_bImmediately = FALSE;
    m_iPrevClearTimeSec = 0;
    m_bExit = FALSE;
    m_hTimer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TimerProc, this, 0, NULL);
}

CButler::~CButler()
{
     m_bExit = TRUE;
    if(NULL != m_hTimer)
    {
        WaitForSingleObject(m_hTimer, INFINITE);
        CloseHandle(m_hTimer);
        m_hTimer = NULL;
    }
}
CButler* CButler::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CButler();
    }
    return m_pInstance;
}
void CButler::Release()
{
    if(NULL == m_pInstance)
    {
        return;
    }
    delete m_pInstance;
    m_pInstance = NULL;
}
DWORD CButler::TimerProc(void* lpParameter)
{
    CButler* pButler = (CButler*)lpParameter;
    while(FALSE == pButler->m_bExit)
    {
        Sleep(500);
        if(TRUE == pButler->m_bImmediately)
        {
            pButler->m_bImmediately = FALSE;
            pButler->RemoveTempFold();
            continue;
        }
        if(Server::CCommonUtil::CurTimeSec() - pButler->m_iPrevClearTimeSec > 60)
        {
            //60秒删除一次
            pButler->RemoveTempFold();
        }
    }

    return 1;
}
void CButler::SetImmediately()
{
    m_bImmediately = TRUE;
}
void CButler::RemoveTempFold()
{
    string strTempRoot = CConfig::GetInstance()->GetTempRoot();
    if(strTempRoot.length() == 0)
    {
        return;
    }
    if(FALSE == Server::CCommonUtil::CheckFoldEmpty(strTempRoot.c_str()))
    {
        printf("Remove fold:%s\n", strTempRoot.c_str());
        string strCommand = Server::CCommonUtil::StringFormat("find %s -mindepth 1 -delete", strTempRoot.c_str());
        Server::CCommonUtil::ExecuteCmdWithOutReplay(strCommand.c_str());
    }
    m_iPrevClearTimeSec = Server::CCommonUtil::CurTimeSec();
}