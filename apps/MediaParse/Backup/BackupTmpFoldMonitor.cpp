#include "BackupTmpFoldMonitor.h"
#include "../Util/CommonUtil.h"
#include "BackupManager.h"
CBackupTmpFoldMonitor* CBackupTmpFoldMonitor::m_pInstance = NULL;
CBackupTmpFoldMonitor::CBackupTmpFoldMonitor()
{
    m_bImmediately = FALSE;
    m_iPrevClearTimeSec = 0;
    m_bExit = FALSE;
    m_hTimer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TimerProc, this, 0, NULL);
}

CBackupTmpFoldMonitor::~CBackupTmpFoldMonitor()
{
    m_bExit = TRUE;
    if(NULL != m_hTimer)
    {
        WaitForSingleObject(m_hTimer, INFINITE);
        CloseHandle(m_hTimer);
        m_hTimer = NULL;
    }
}
CBackupTmpFoldMonitor* CBackupTmpFoldMonitor::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CBackupTmpFoldMonitor();
    }
    return m_pInstance;
}
void CBackupTmpFoldMonitor::Release()
{
    if(NULL == m_pInstance)
    {
        return;
    }
    delete m_pInstance;
    m_pInstance = NULL;
}
DWORD CBackupTmpFoldMonitor::TimerProc(void* lpParameter)
{
    CBackupTmpFoldMonitor* pBackupTmpFoldMonitor = (CBackupTmpFoldMonitor*)lpParameter;
    while(FALSE == pBackupTmpFoldMonitor->m_bExit)
    {
        Sleep(500);
        if(TRUE == pBackupTmpFoldMonitor->m_bImmediately)
        {
            pBackupTmpFoldMonitor->m_bImmediately = FALSE;
            pBackupTmpFoldMonitor->RemoveTempFold();
            continue;
        }
        if(CCommonUtil::CurTimeSec() - pBackupTmpFoldMonitor->m_iPrevClearTimeSec > 60)
        {
            //60秒删除一次
            pBackupTmpFoldMonitor->RemoveTempFold();
        }
    }

    return 1;
}
void CBackupTmpFoldMonitor::SetImmediately()
{
    m_bImmediately = TRUE;
}
void CBackupTmpFoldMonitor::RemoveTempFold()
{
    string strTempRoot = CBackupManager::GetInstance()->GetBackupTempRoot("");
    if(strTempRoot.length() == 0)
    {
        return;
    }
    if(FALSE == CCommonUtil::CheckFoldExist(strTempRoot.c_str()))
    {
        CCommonUtil::CreateFold(strTempRoot.c_str());
    }
    if(FALSE == CCommonUtil::CheckFoldEmpty(strTempRoot.c_str()))
    {
        // strTempRoot.append("**");
        // printf("Remove Fold %s\n", strTempRoot.c_str());
        // CCommonUtil::RemoveFold(strTempRoot.c_str());
        // std::vector<FoldEntry> foldEntryList = CBackupManager::GetInstance()->ScanFold(strTempRoot);
        // for(size_t i = 0; i < foldEntryList.size(); ++i)
        // {
        //     string strFold = strTempRoot;
        //     strFold.append(foldEntryList[i].strName.c_str());
        //     printf("Remove fold:%s\n", strFold.c_str());
        //     CCommonUtil::RemoveFold(strFold.c_str());
        // }
        string strCommand = CCommonUtil::StringFormat("find %s -mindepth 1 -delete", strTempRoot.c_str());
        CCommonUtil::ExecuteCmdWithOutReplay(strCommand.c_str());
    }
    m_iPrevClearTimeSec = CCommonUtil::CurTimeSec();
}