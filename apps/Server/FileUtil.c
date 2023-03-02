#include "stdafx.h"
#include "Tools.h"
#include "FileUtil.h"
#include <string.h>
#include <sys/stat.h>

BOOL FileUtil_CheckFileExist(const char* pszFile)
{
    if((access(pszFile, F_OK)) != -1)   
    {   
        return TRUE;  
    }   
    else  
    {   
        return FALSE; 
    }   
}

BOOL FileUtil_CheckFoldExist(const char* pszFold)
{
    return FileUtil_CheckFileExist(pszFold);
}

BOOL FileUtil_CreateFold(const char* pszFold)
{
    if(FileUtil_CheckFoldExist(pszFold))
    {
        return TRUE;
    }
    umask(0);
    return mkdir(pszFold, 0777) == 0?TRUE:FALSE;
}

BOOL FileUtil_RemoveFile(const char* pszFile)
{
    if(0 == remove(pszFile))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL FileUtil_RemoveFold(const char* pszFile)
{
    int iLen = strlen(pszFile) + 20;

    char* pszCommand = malloc(iLen);
    memset(pszCommand, 0, iLen);
    sprintf(pszCommand, "rm -rf %s", pszFile);
    int iStatus = system(pszCommand);
    free(pszCommand);
    pszCommand = NULL;
    return FileUtil_CheckStatus(iStatus);
    // if(0 == remove(pszFile))
    // {
    //     printf("remove suuccess~~~~~~~~~~~\n");
    //     return TRUE;
    // }
    // printf("remove failed~~~~~~~~~~~%d\n", errno);
    // return FALSE;
}

BOOL FileUtil_GetFileName(const char* pszInFileName, char* pszOutFileName)
{
    if(FALSE == FileUtil_CheckFileExist(pszInFileName))
    {
        strcpy(pszOutFileName, pszInFileName);
        return TRUE;
    }

    int iLen = strlen(pszInFileName) + 1;
    char* pBuffer = (char*) malloc(iLen);
    memset(pBuffer, 0, iLen);
    strcpy(pBuffer, pszInFileName);
    char *pszPostFix = NULL;
    //char *pszTmpIn = NULL;
    char *pszFileName = strtok_r( pBuffer, ".", &pszPostFix) ;
    BOOL bBreak = FALSE;
    int iIndex = 0;
    while(!bBreak)
    {
        char szTmp[255] = {0};
        sprintf(szTmp, "%s_%d.%s", pszFileName, iIndex++, pszPostFix);
        if(FALSE == FileUtil_CheckFileExist(szTmp))
        {
            strcpy(pszOutFileName, szTmp);
            bBreak = TRUE;
        }
    }
    free(pBuffer);
    pBuffer = NULL;
    return TRUE;
}

BOOL FileUtil_GetFileOnlyName(const char* pszFile, char* pszFileName)
{
    int iPosition = Tools_Rfind(pszFile, '/');
    if(iPosition < 0)
    {
        strcpy(pszFileName, pszFile);
    }
    else
    {
        strcpy(pszFileName, pszFile + iPosition + 1);
    }
    
    return TRUE;
}

int FileUtil_GetYear(const char* pszFile)
{
    int iPosition = Tools_Rfind(pszFile, '/');
    if(iPosition <= 0)
    {
        return 0;
    }
    char sz[MAX_PATH] = {0};
    memcpy(sz, pszFile, iPosition);
    return atoi(sz);
}

void FileUtil_GetNewFileName(char* pFileName, char* pszPath)
{
    //取一个文件名称
	char szFileName[MAX_PATH] = {0};
	while(1)
	{
		sprintf(szFileName, "%lld", Tools_CurTimeMilSec());
		char szPathFile[MAX_PATH] = {0};
		sprintf(szPathFile, "%s/%s", pszPath, szFileName);
		if(FileUtil_CheckFileExist(szPathFile))
		{
			//存在了清除在申请
			memset(szFileName, 0, MAX_PATH);
		}
		else
		{
			//不存在就用这个文件名
            strcpy(pFileName, szFileName);
			break;
		}
    }
}

void FileUtil_GetNewFileName2(int iYear, const char* pszName, char* pPostFix, char* pRetName)
{
    int iIndex = 0;
    do
    {
        char szSmallFile[MAX_PATH] = {0};
        char szLargeFile[MAX_PATH] = {0};
        char szFileName[MAX_PATH] = {0};
        if(iIndex == 0)
        {
            strcpy(szFileName, pszName);
            
        }
        else
        {
            sprintf(szFileName, "%s_%d", pszName, iIndex);
        }
        sprintf(szSmallFile, "%s/%d/%s.%s", FOLDTHUMB_PREFIX, iYear, szFileName, "jpg");
        sprintf(szLargeFile, "%s/%d/%s.%s", FOLD_PREFIX, iYear, szFileName, pPostFix);
        if(FALSE == FileUtil_CheckFileExist(szSmallFile) && FALSE == FileUtil_CheckFileExist(szLargeFile))
        {
            strcpy(pRetName, szFileName);
            break;
        }
        iIndex++;
    } while (1);
}

BOOL FileUtil_CheckStatus(pid_t status)
{
    if (-1 == status)
    {
        printf("system error!");
        return FALSE;
    }
    else
    {
        //printf("exit status value = [0x%x]\n", status);
        if (WIFEXITED(status))
        {
            if (0 == WEXITSTATUS(status))
            {
                printf("run shell script successfully.\n");
                return TRUE;
            }
            else
            {
                printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
                return FALSE;
            }
        }
        else
        {
            printf("exit status = [%d]\n", WEXITSTATUS(status));
            return FALSE;
        }
    }
}

BOOL FileUtil_MoveFile(const char* pFromFile, const char* pToFile)
{
    printf("From File:%s\n", pFromFile);
    printf("To File:%s\n", pToFile);
    char sz[512] = {0};
    sprintf(sz, "mv \"%s\" \"%s\"", pFromFile, pToFile);
    pid_t status;
    status = system(sz);
    return FileUtil_CheckStatus(status);
}

BOOL FileUtil_CreateEmptyFile(const char* pFile)
{
    char sz[512] = {0};
    sprintf(sz, "touch %s", pFile);
    pid_t status;
    status = system(sz);
    return FileUtil_CheckStatus(status);
}

long FileUtil_GetFileSize(const char* pszPathFile)
{
    struct stat buf={};
    stat(pszPathFile, &buf);
    return buf.st_size;
}

void FileUtil_SepFile(const char* pFileName, char* pName, char* pPostFix)
{
    int iPosition = Tools_Rfind(pFileName, '.');
    if(iPosition > 0)
	{
		memcpy(pName, pFileName, iPosition);
		memcpy(pPostFix, pFileName + iPosition + 1, strlen(pFileName) - iPosition);
	}
    else if(iPosition == 0)
    {
        memcpy(pPostFix, pFileName + 1, strlen(pFileName) - 1);
    }
    else
    {
        memcpy(pName, pFileName, strlen(pFileName));
    }
}

void FileUtil_ThumbNameFromFileName(char* pFileName, char* pszOut)
{
    int iPosition = Tools_Rfind(pFileName, '.');
    memcpy(pszOut, pFileName, iPosition);
    memcpy(pszOut + iPosition, "_", 1);
    strcat(pszOut, pFileName + iPosition + 1 );
    strcat(pszOut, ".jpg" );
}

int FileUtil_GetFoldFileCount(char* pFold)
{
    char szPath[255] = {0};
    struct dirent* pDirent;
    sprintf(szPath, "%s%s", FOLD_PREFIX, pFold);
    printf("FileUtil_GetFoldFileCount:%s\n", szPath);
    DIR *pDir;
    if ((pDir = opendir(szPath)) == NULL)
    {
        return 0;
    }
    int iCount = 0;
    while ((pDirent = readdir(pDir)) != NULL)  
    {
        if (strcmp(pDirent->d_name, ".") == 0 ||
            strcmp(pDirent->d_name, "..") == 0)
        {
            continue;
        }
        iCount++;         
    }
    closedir(pDir);
    return iCount;
}

char* FileUtil_GetFolds(char* pFold)
{
    struct dirent* pDirent;
    DIR *pDir;
    if ((pDir = opendir(pFold)) == NULL)
    {
        printf("opendir error\n");
        return NULL;
    }
    char* pRet = NULL;
    while ((pDirent = readdir(pDir)) != NULL)  
    {
        if (strcmp(pDirent->d_name, ".") == 0 ||
            strcmp(pDirent->d_name, "..") == 0)
        {
            continue;
        }
        if(pDirent->d_type & DT_DIR)        
        {
            if(NULL == pRet)
            {
                pRet = malloc(strlen(pDirent->d_name) + 1);
                memset(pRet, 0, strlen(pDirent->d_name) + 1);
                strcpy(pRet, pDirent->d_name);
            }
            else
            {
                pRet = realloc(pRet, strlen(pRet) + strlen(pDirent->d_name) + 2);
                strcat(pRet, "&");
                strcat(pRet, pDirent->d_name);
            }
        }
    }
    closedir(pDir);
    return pRet;
}

char* FileUtil_CurrentPath()
{
    char szPath[255] = {0};
    int iCount = readlink("/proc/self/exe", szPath, 255);
    if (iCount < 0 || iCount >= 255)
    {
        char* pRet = malloc(1);
        memset(pRet, 0, 1);
        return pRet;
    }
    //获取当前目录绝对路径，即去掉程序名
    int i;
    for (i = iCount; i >= 0; --i)
    {
        if (szPath[i] == '/')
        {
            szPath[i+1] = '\0';
            break;
        }
    }
    
    char* pRet = malloc(strlen(szPath) + 1);
    memset(pRet, 0, strlen(szPath) + 1);
    strcpy(pRet, szPath);
    return pRet;
}

BOOL FileUtil_IsEmptyDir(char* pszDir)
{
    /* 打开要进行匹配的文件目录 */
    DIR* pDir = opendir(pszDir);
    struct dirent* pDirent;
    if (pDir == NULL)
    {  
        return FALSE;
    }  
    while (1)
    {  
        pDirent = readdir(pDir);
        if (pDirent <= 0)
        {  
            break;
        }  
        if ((strcmp(".", pDirent->d_name)==0) || (strcmp("..", pDirent->d_name)==0))
        {  
            continue;
        }  
        /*判断是否有目录和文件*/
        if ((pDirent->d_type == DT_DIR) || (pDirent->d_type == DT_REG))
        {  
            closedir(pDir);
            return FALSE;
        }  
    }  
    closedir(pDir);
    return TRUE;
}

void FileUtil_RemoveYearEmptyFold(int iYear)
{
    char szFold[MAX_PATH] = {0};
	sprintf(szFold, "%s/%d/", FOLD_PREFIX, iYear);
	char szThumbFold[MAX_PATH] = {0};
	sprintf(szThumbFold, "%s/%d/", FOLDTHUMB_PREFIX, iYear);

	if(TRUE == FileUtil_IsEmptyDir(szFold))
    {
        FileUtil_RemoveFold(szFold);
		FileUtil_RemoveFold(szThumbFold);
    }
}

BOOL FileUtil_IsFile(const char* pszFile)
{
    struct stat s;
    if (stat(pszFile, &s)==0)
    {
        if(s.st_mode & S_IFREG)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL FileUtil_IsFold(const char* pszFold)
{
    struct stat s;
    if (stat(pszFold, &s)==0)
    {
        if(s.st_mode & S_IFDIR)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL FileUtil_FileNameFromThumbName(char* pszThumbFileName, char* pszFileName)
{
    int iPosition = Tools_Rfind(pszThumbFileName, '.');
    if(iPosition <= 0)
    {
        return FALSE;
    }
    char szTmpFileName[MAX_PATH] = {0};
    memcpy(szTmpFileName, pszThumbFileName, iPosition);
    iPosition = Tools_Rfind(szTmpFileName, '_');
    if(iPosition <= 0)
    {
        return FALSE;
    }
    memcpy(pszFileName, szTmpFileName, iPosition);
    memcpy(pszFileName + iPosition, ".", 1);
    strcpy(pszFileName + iPosition + 1, szTmpFileName + iPosition + 1);
    return TRUE;
}

////media/.media/2021/IMG_20210412_072616_jpg.jpg ==> 2021/IMG_20210412_072616.jpg
char* FileUtil_GetRelativePath(char* pszFile)
{
    char* pszRet = NULL;
    char szFile[MAX_PATH] = {0};
    strcpy(szFile, pszFile);
    int i = 0;
    int iFileLen = strlen(szFile);
    for(int t = iFileLen; t > 0; --t)
    {
        if(szFile[t] == '/')
        {
            i++;
            if(i == 2)
            {
                int iStartPos = iFileLen - (iFileLen - t - 1);
                pszRet = (char*)malloc(iFileLen - t);
                memset(pszRet, 0, iFileLen - t);
                strcpy(pszRet, szFile + iStartPos);
            }
        }
        if(NULL != pszRet)
        {
            return pszRet;
        }
    }
    if(NULL == pszRet)
    {
        pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
    }
    return pszRet;
}

BOOL FileUtil_ReadLink(char* pszFileName, char* pszFullFileName, size_t  iBufferLen)
{
    struct stat stat = {0};
    if (lstat(pszFileName, &stat) == -1)
    {
        return FALSE;
    }
    BOOL bLink =  S_ISLNK(stat.st_mode) ? TRUE : FALSE;
    if(FALSE == bLink)
    {
        return FALSE;
    }
    int iFileLen = readlink(pszFileName, pszFullFileName, iBufferLen);
    if(iFileLen == -1)
    {
        return FALSE;
    }
    if(FALSE == FileUtil_CheckFileExist(pszFullFileName))
    {
        return FALSE;
    }
    return TRUE;
}

BOOL FileUtil_IsSoftLink(char* pszFileName)
{
    struct stat stat = {0};
    if (lstat(pszFileName, &stat) == -1)
    {
        return FALSE;
    }
    BOOL bLink =  S_ISLNK(stat.st_mode) ? TRUE : FALSE;
    if(FALSE == bLink)
    {
        return FALSE;
    }
    return TRUE;
}