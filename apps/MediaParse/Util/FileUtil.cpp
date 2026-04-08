#include "FileUtil.h"
#include "Base64Coding.h"
CFileUtil::CFileUtil()
{
    m_pszFileContent = NULL;
}

CFileUtil::~CFileUtil()
{
    if(NULL != m_pszFileContent)
    {
        free(m_pszFileContent);
        m_pszFileContent = NULL;
    }
}

char* CFileUtil::GetFileContent(const char* pszFile)
{
    if(NULL != m_pszFileContent)
    {
        free(m_pszFileContent);
        m_pszFileContent = NULL;
    }
    
    FILE* pFile = fopen(pszFile, "r");
    if(NULL == pFile)
    {
        m_pszFileContent = (char*)malloc(1);
        memset(m_pszFileContent, 0, 1);
        return m_pszFileContent;
    }
    fseek(pFile, 0L,SEEK_END);
    int iContentLen = ftell(pFile);
    m_pszFileContent = (char*)malloc(iContentLen + 1);
    memset(m_pszFileContent, 0, iContentLen + 1);
    fseek(pFile, 0L,SEEK_SET);
    fread(m_pszFileContent, 1, iContentLen, pFile);
    fclose(pFile);
    pFile = NULL;
    return m_pszFileContent;
}

char* CFileUtil::GetBinFileContent(const char* pszFile, int& iContentLen)
{
    if(NULL != m_pszFileContent)
    {
        free(m_pszFileContent);
        m_pszFileContent = NULL;
    }
    
    FILE* pFile = fopen(pszFile, "r");
    if(NULL == pFile)
    {
        iContentLen = 0;
        m_pszFileContent = (char*)malloc(1);
        memset(m_pszFileContent, 0, 1);
        return m_pszFileContent;
    }
    fseek(pFile, 0L,SEEK_END);
    iContentLen = ftell(pFile);
    m_pszFileContent = (char*)malloc(iContentLen);
    memset(m_pszFileContent, 0, iContentLen);
    fseek(pFile, 0L,SEEK_SET);
    fread(m_pszFileContent, 1, iContentLen, pFile);
    fclose(pFile);
    pFile = NULL;
    return m_pszFileContent;
}

BOOL CFileUtil::SetFileContent(const char* pszFile, const char* pszContent)
{
    FILE* pFile = fopen(pszFile, "w");
    if(NULL == pFile)
    {
        return FALSE;
    }
    fwrite(pszContent, 1, strlen(pszContent),  pFile);
    fflush(pFile);
    fclose(pFile);
    return TRUE;
}
string CFileUtil::GetFileMd51(const char* pszFile, int iTag)
{
    string strRet = GetFileMd5(pszFile);
    if(strRet.length() == 0)
    {
        printf("%d\n", iTag);
    }
    return strRet;
}
string CFileUtil::GetFileMd5(const char* pszFile)
{
    FILE* pFile = fopen(pszFile, "rb");
    if (pFile)
    {
        //printf("正在计算MD5值...\n");
        //time_t startTime = time(NULL);
        md5_state_t md5StateT;
        md5_init(&md5StateT);
        char szBuffer[1024] = {0};
        int iBufferLen = sizeof(szBuffer);
        while (!feof(pFile))
        {
            memset(szBuffer, 0, iBufferLen);
            size_t iReadLen = fread(szBuffer, sizeof(char), iBufferLen, pFile);
            md5_append(&md5StateT, (const md5_byte_t *)szBuffer, (int)iReadLen);
        }
        md5_byte_t digest[16];
        md5_finish(&md5StateT, digest);
        char szMd5[33] = { '\0' }, szHexBuffer[5]={0};
        for (size_t i = 0; i != 16; ++i)
        {
            if (digest[i] < 16)
            {
                sprintf(szHexBuffer, "0%x", digest[i]);
            } 
            else
            {
                sprintf(szHexBuffer, "%x", digest[i]);
            }
                
            strcat(szMd5, szHexBuffer);
        }
        //time_t endTime = time(NULL);
        fclose(pFile);
        
        char szBase64[100] = {0};
        int iBase64Len = 100;
        base64_encode((unsigned char*)szMd5, 32, (unsigned char*)szBase64, &iBase64Len);
        
        //printf("计算完毕：%s，耗时%ld秒\n", szMd5, endTime - startTime);
        return szBase64;
    }
    else
    {
        printf("无法打开文件：%s\n", pszFile);
        return "";
    }
}
list<string> CFileUtil::GetSubFolds(const char* pszFold)
{
    list<string> retList;
    DIR *pDir;
    if ((pDir = opendir(pszFold)) == NULL)
    {
        printf("opendir error\n");
        return retList;
    }
    struct dirent* pDirent;
    while ((pDirent = readdir(pDir)) != NULL)  
    {
        if (strcmp(pDirent->d_name, ".") == 0 ||
            strcmp(pDirent->d_name, "..") == 0)
        {
            continue;
        }
        if(pDirent->d_type & DT_DIR)        
        {
            retList.push_back(pDirent->d_name);
        }
    }
    closedir(pDir);
    return retList;
}
BOOL CFileUtil::CheckFileExist(string strFile)
{
    int iRet = access(strFile.c_str(), F_OK);
    if(iRet != -1)   
    {   
        return TRUE;  
    }   
    else  
    {   
        return FALSE; 
    }   
}
BOOL CFileUtil::CheckFoldExist(string strFold)
{
    return CheckFileExist(strFold);
}
 BOOL CFileUtil::CreateFold(string strFold)
 {
    if(CFileUtil::CheckFoldExist(strFold))
    {
        return TRUE;
    }
    umask(0);
    return mkdir(strFold.c_str(), 0777) == 0?TRUE:FALSE;
 }
 BOOL CFileUtil::RemoveFile(string strFile)
{
    printf("Remove file:%s\n", strFile.c_str());
    if(0 == remove(strFile.c_str()))
    {
        return TRUE;
    }
    return FALSE;
}