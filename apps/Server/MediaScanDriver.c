#include "MediaScanDriver.h"
#include <ctype.h>
#include "DataBaseMedia.h"
/*
 *首先读取缩略图里面的所有文件，判断原图文件是否存在不存在删除原图文件，然后从数据库中删除该记录
 */
BOOL MediaScanDriver_IsNeedEnterDir(MediaScanDriver* pDriver, const char* pDir)
{
    char* pTemp = NULL;
    char* pLastFlodName = strdup(pDir + pDriver->iPrefxLen);
    char* pTmpFold = pLastFlodName;
    if(pTmpFold[0] == '/')
    {
        pTmpFold = pTmpFold + 1;
    }
    
    char* b = pTmpFold;
    char *pRet = (char*)strtok_r(b, "/", &pTemp);
    if(NULL == pRet)
    {
        printf("pRet:%s\n", "NULL");
    }
    
    int iCount = 0;
    BOOL bRet = FALSE;
	while(NULL != pRet)
	{
        if(iCount == 0)
        {
            if(strlen(pRet) != 4)
            {
                bRet = FALSE;
                break;
            }
            if(isdigit(pRet[0]) && 
                isdigit(pRet[1]) && 
                isdigit(pRet[2]) && 
                isdigit(pRet[3]))
            {
                bRet = TRUE;
            }

        }
        iCount++;
        if(iCount > 1)
        {
            bRet = FALSE;
            break;
        }
		pRet = strtok_r(NULL, "&", &pTemp);
	}
    free(pLastFlodName);
    pLastFlodName = NULL;
    return bRet;
}

void MediaScanDriver_ScanDir(MediaScanDriver* pDriver, const char* pszDir)
{
    //printf("open dir: %s\n", pszDir);
    DIR* pRootDir = NULL;
    struct dirent* pEntry = NULL;
    if((pRootDir = opendir(pszDir)) == NULL)
    {
        //printf("can't open dir.\n");
        return;
    }
    chdir(pszDir);
    while(NULL != (pEntry = readdir(pRootDir))) 
    { 
        // 获取下一级目录信息
        struct stat statbuf;
        lstat(pEntry->d_name, &statbuf);   // 获取下一级成员属性
        
        if(S_ISDIR(statbuf.st_mode)) 
        {      
            // 判断下一级成员是否是目录  
            if (strcmp(".", pEntry->d_name) == 0 || strcmp("..", pEntry->d_name) == 0)
            {
                continue;
            }
            char szDir[520] = {0};
            sprintf(szDir, "%s/%s", pszDir, pEntry->d_name);
            //printf("check dir:%s\n", szDir);
            BOOL bRet = MediaScanDriver_IsNeedEnterDir(pDriver, szDir);
            if(bRet)
            {
                MediaScanDriver_ScanDir(pDriver, szDir); 
                char szBuffer[520] = {0};
                sprintf(szBuffer, "%s/%s", pszDir, pEntry->d_name);

                int iFileLen = strlen(szBuffer) - pDriver->iPrefxLen;
                char* pszFile = malloc(iFileLen);
                memset(pszFile, 0, iFileLen);
                strcpy(pszFile, szBuffer + pDriver->iPrefxLen + 1);
                //printf("fold:%s\n", pszFile);
                RelechQueue_PushBack(pDriver->pFileQueue, pszFile);
            }
            else
            {
                printf("Failed:%s\n", szDir);
            }
            
        }
        else
        {
            char szBuffer[520] = {0};
            sprintf(szBuffer, "%s/%s", pszDir, pEntry->d_name);

            int iFileLen = strlen(szBuffer) - pDriver->iPrefxLen;
            char* pszFile = malloc(iFileLen);
            memset(pszFile, 0, iFileLen);
            strcpy(pszFile, szBuffer + pDriver->iPrefxLen + 1);
            //printf("file:%s\n", pszFile);
            RelechQueue_PushBack(pDriver->pFileQueue, pszFile);
        }
    }
        
    chdir(".."); // 回到上级目录  
    closedir(pRootDir);
}

void MediaScanDriver_Run(MediaScanDriver* pDriver)
{
    MediaScanDriver_ScanDir(pDriver, FOLDTHUMB_PREFIX);
    int iTotalLen = pDriver->pFileQueue->iItemLen;
    while(pDriver->pFileQueue->iItemLen > 0)
    {
        // sleep(1);
        if(NULL != pDriver->callBack)
        {
            int iDealLen = iTotalLen - pDriver->pFileQueue->iItemLen;
            int iPrecent = (iDealLen*100.0)/iTotalLen;
            pDriver->callBack(iPrecent, pDriver->lpParameter);
        }

        char* psz = RelechQueue_Front(pDriver->pFileQueue);
        RelechQueue_PopFront(pDriver->pFileQueue);

        char szThumbFile[MAX_PATH] = {0};
        sprintf(szThumbFile, "%s/%s", FOLDTHUMB_PREFIX, psz);

        // if(FileUtil_IsFile(szThumbFile) && FALSE == FileUtil_IsSoftLink(szThumbFile))
        if(FileUtil_IsFile(szThumbFile))
        {
            char szFileName[MAX_PATH] = {0};       
            BOOL bRet = FileUtil_FileNameFromThumbName(psz, szFileName);
            if(FALSE == bRet)
            {
                FileUtil_RemoveFile(szThumbFile);
                free(psz);
                psz = NULL;
                continue;
            }
            char szFilePrefix[MAX_PATH] = {0};
            sprintf(szFilePrefix, "%s/%s", FOLD_PREFIX, szFileName);
            //文件处理
            if(FALSE == FileUtil_CheckFileExist(szFilePrefix))
            {
                bRet = FileUtil_RemoveFile(szThumbFile);
                if(bRet)
                {
                    bRet = DataBaseMedia_RemoveItemFromName(szFileName);
                    if(FALSE == bRet)
                    {
                        FileUtil_CreateEmptyFile(szThumbFile);
                    }
                }
            }
        }
        else if(FileUtil_IsFold(szThumbFile))
        {
            char szFilePrefix[MAX_PATH] = {0};
            sprintf(szFilePrefix, "%s/%s", FOLD_PREFIX, psz);
            if(FileUtil_IsEmptyDir(szThumbFile))
            {
                FileUtil_RemoveFold(szThumbFile);
                FileUtil_RemoveFold(szFilePrefix);
            }
            // if(FileUtil_IsEmptyDir(szFilePrefix))
            // {
            //    FileUtil_RemoveFold(szFilePrefix);
            // }
        }
        free(psz);
        psz = NULL;
    }
    RelechQueue_Clear(pDriver->pFileQueue);
    pDriver->pFileQueue = NULL;
}

DWORD MediaScanDriver_ScanProc(LPVOID* lpParameter)
{
    //pthread_detach(pthread_self());
    MediaScanDriver* pDriver = (MediaScanDriver*)lpParameter;
    MediaScanDriver_Run(pDriver);
    pDriver->callBack(100, pDriver->lpParameter);
    return 1;
}

MediaScanDriver* MediaScanDriver_Start(MediaScanCallBack callBack, LPVOID* lpParameter)
{
    int iBufferLen = sizeof(MediaScanDriver);
    char* pBuffer = malloc(iBufferLen);
    memset(pBuffer, 0, iBufferLen);
    MediaScanDriver* pDriver = (MediaScanDriver*)pBuffer;
    pDriver->pFileQueue = RelechQueue_Init();
    pDriver->iPrefxLen = strlen(FOLDTHUMB_PREFIX);
    pDriver->callBack = callBack;
    pDriver->lpParameter = lpParameter;

    pDriver->hScanHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MediaScanDriver_ScanProc, pDriver, 0, NULL);
    
    return pDriver;
}

void MediaScanDriver_Stop(MediaScanDriver* pDriver)
{
    if(NULL == pDriver)
    {
        return;
    }
    if(NULL != pDriver->hScanHandle)
    {
        WaitForSingleObject(pDriver->hScanHandle, INFINITE);
        CloseHandle(pDriver->hScanHandle);
        pDriver->hScanHandle = NULL;
    }
    if(NULL != pDriver->pFileQueue)
    {
        while(pDriver->pFileQueue->iItemLen > 0)
        {
            char* psz = RelechQueue_Front(pDriver->pFileQueue);
            RelechQueue_PopFront(pDriver->pFileQueue);
            free((char*)psz);
            psz = NULL;
        }
        RelechQueue_Clear(pDriver->pFileQueue);
    }
    
    free((char*)pDriver);
    pDriver = NULL;
}