#include "ClearCache.h"
#include "DataBase/MediaGroupItemsTable.h"
#include "DbusUtil.h"
CClearCache* CClearCache::m_pInstance = NULL;
CClearCache::CClearCache()
{
    m_eStatus = CLEARCACHE_SUCCESS;
    m_hClearHandle = NULL;
    m_iPrecent = 0;
    m_iStepCount = 0;
    m_iCurStepIndex = 0;    
}

CClearCache::~CClearCache()
{
    if(NULL != m_hClearHandle)
    {
        WaitForSingleObject(m_hClearHandle, INFINITE);
        CloseHandle(m_hClearHandle);
        m_hClearHandle = NULL;
    }
}
CClearCache* CClearCache::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CClearCache();
    }
    return m_pInstance;
}
void CClearCache::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
BOOL CClearCache::Start()
{
    if(m_eStatus == CLEARCACHE_PROCESS)
    {
        return FALSE;
    }
    if(NULL != m_hClearHandle)
    {
        WaitForSingleObject(m_hClearHandle, INFINITE);
        CloseHandle(m_hClearHandle);
        m_hClearHandle = NULL;
    }
    m_iPrecent = 0;
    m_iStepCount = 4;
    m_iCurStepIndex = 0;
    m_eStatus = CLEARCACHE_PROCESS;
    m_hClearHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClearCacheProc, this, 0, NULL);
    return TRUE;
}
void CClearCache::ScanMediaParse()
{
    BOOL bStart = CDbusUtil::BackupFoldSyncStart();
    if(FALSE == bStart)
    {
        return ;
    }
    int iFailedCount = 0;
    while (TRUE)
    {
        Sleep(500);
        BOOL bError = FALSE;
        int iPrecent = CDbusUtil::BackupFoldSyncPrecent(bError);
        if(TRUE == bError)
        {
            iFailedCount++;
        }
        else
        {
            iFailedCount = 0;
            CallBack(iPrecent, (LPVOID*)this);
        }
        if(iFailedCount > 5 || iPrecent == 100)
        {
            break;
        }
    }
}
DWORD CClearCache::ClearCacheProc(LPVOID* lpParameter)
{
    CClearCache* pClearCache = (CClearCache*)lpParameter;
    pClearCache->m_iPrecent = 0;
    pClearCache->m_iCurStepIndex = 0;
    pClearCache->m_iStepCount = 5;
    pClearCache->m_eStatus = CLEARCACHE_PROCESS;
//扫描原图文件 如果原图不在数据库 就将原图删除掉 否则检测缩略图 如果没有就产生新的缩率图
    pClearCache->m_MediaScan.ScanOriginal(CallBack, lpParameter);
    pClearCache->m_MediaScan.Stop();
    pClearCache->m_iCurStepIndex++;
//扫描媒体文件 将不存在的原图片的缩略图删除掉
    pClearCache->m_MediaScan.ScanThumb(CallBack, lpParameter);
    pClearCache->m_MediaScan.Stop();
    pClearCache->m_iCurStepIndex++;
//重新计数
    BOOL bRet =  CMediaJishuTable::ReSet(CallBack, lpParameter); 
    if(FALSE == bRet)
    {
        pClearCache->m_eStatus = CLEARCACHE_FAILED;
        return 1;
    }
    pClearCache->m_iCurStepIndex++;
//删除groupid
    bRet = CMediaGroupItemsTable::ClearCacheRecord();
    if(FALSE == bRet)
    {
        pClearCache->m_eStatus = CLEARCACHE_FAILED;
        return 1;
    }
//扫描备份文件
    pClearCache->m_iCurStepIndex++;
    pClearCache->ScanMediaParse();

    pClearCache->m_iPrecent = 100;
    pClearCache->m_eStatus = CLEARCACHE_SUCCESS;
    return 1;
}
void CClearCache::CallBack(int iPrecent, LPVOID* lpParameter)
{
    CClearCache* pClearCache = (CClearCache*)lpParameter;
    double dCurPos = pClearCache->m_iCurStepIndex*100.0f + iPrecent;
    double dTotalPos = pClearCache->m_iStepCount*100.0f;
    pClearCache->m_iPrecent = (int)(dCurPos*100/dTotalPos);
    printf("%.1f %.1f %d\n", dCurPos, dTotalPos, pClearCache->m_iPrecent);
}
 BOOL CClearCache::IsRuning()
 {
    if(m_eStatus == CLEARCACHE_PROCESS)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
 }
 void CClearCache::GetStatus(CLEARCACHESTATUS& eStatus, int& iPrecent)
 {
    eStatus = m_eStatus;
    iPrecent = m_iPrecent;
 }