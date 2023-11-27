#include "ClearCache.h"
#include "DataBaseMediaJiShu.h"
#include "DataBaseGroupItems.h"
#include "MediaScanDriver.h"
void ClearCache_CallBack(int iPrecent, LPVOID* lpParameter)
{
    struct ClearCache* pClearCache = (struct ClearCache*)lpParameter;
    double dCurPos = pClearCache->iCurStepIndex*100.0f + iPrecent;
    double dTotalPos = pClearCache->iStepCount*100.0f;
    pClearCache->iPrecent = (int)(dCurPos*100/dTotalPos);
}

DWORD ClearCache_Proc(LPVOID* lpParameter)
{
    struct ClearCache* pClearCache = (struct ClearCache*)lpParameter;
    pClearCache->iStepCount = 3;
//扫描媒体文件    
    MediaScanDriver* pMediaScanDriver = MediaScanDriver_Start(ClearCache_CallBack, lpParameter);
    WaitForSingleObject(pMediaScanDriver->hScanHandle, INFINITE);
    CloseHandle(pMediaScanDriver->hScanHandle);
    MediaScanDriver_Stop(pMediaScanDriver);
    pClearCache->iCurStepIndex++;
//重新计数
    BOOL bRet = DataBaseMediaJiShu_ReSet(ClearCache_CallBack, lpParameter); 
    if(FALSE == bRet)
    {
        pClearCache->eStatus = CLEARCACHE_FAILED;
        return 1;
    }
    pClearCache->iCurStepIndex++;
//删除groupid
    bRet = DataBaseGroupItems_ClearCacheRecord();
    if(FALSE == bRet)
    {
        pClearCache->eStatus = CLEARCACHE_FAILED;
        return 1;
    }
    // pClearCache->iCurStepIndex++;
    // MediaScanDriver* pMediaScanDriver = MediaScanDriver_Start(ClearCache_CallBack, lpParameter);
    // MediaScanDriver_Stop(pMediaScanDriver);
    pClearCache->iPrecent = 100;
    pClearCache->eStatus = CLEARCACHE_SUCCESS;
    return 1;
}
//ClearCache_Start(&g_pClearCache);  
BOOL ClearCache_Start(struct ClearCache** pClearCache)
{
    if(NULL == *pClearCache)
    {
        int iDataLen = sizeof(struct ClearCache);
        char* pszBuffer = (char*)malloc(iDataLen);
        memset(pszBuffer, 0, iDataLen);
        *pClearCache = (struct ClearCache*)pszBuffer;
        (*pClearCache)->eStatus = CLEARCACHE_SUCCESS;
        (*pClearCache)->hClearHandle = NULL;
        (*pClearCache)->iPrecent = 0;
    }
    if((*pClearCache)->eStatus == CLEARCACHE_PROCESS)
    {
        return FALSE;
    }
    if(NULL != (*pClearCache)->hClearHandle)
    {
        WaitForSingleObject((*pClearCache)->hClearHandle, INFINITE);
        (*pClearCache)->hClearHandle = NULL;
    }
    (*pClearCache)->iPrecent = 0;
    (*pClearCache)->iStepCount = 3;
    (*pClearCache)->iCurStepIndex = 0;
    (*pClearCache)->eStatus = CLEARCACHE_PROCESS;
    (*pClearCache)->JiShuCallBack = ClearCache_CallBack;
    (*pClearCache)->hClearHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ClearCache_Proc, *pClearCache, 0, NULL);
    return TRUE;
}

void ClearCache_Release(struct ClearCache* pClearCache)
{
    if(NULL == pClearCache)
    {
        printf("ClearCache_Release\n");
        return;
    }
    if(NULL != pClearCache->hClearHandle)
    {
        WaitForSingleObject(pClearCache->hClearHandle, INFINITE);
        CloseHandle(pClearCache->hClearHandle);
        pClearCache->hClearHandle = NULL;
    }
    free((char*)pClearCache);
    pClearCache = NULL;
}