#include "FileUtil.h"
#include "Config.h"
#include "Tools.h"
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
string CFileUtil::GetFileOnlyName(string strFile)
{
    vector<string> vec = Server::CCommonUtil::StringSplit(strFile, "/");
    if(vec.size() == 0)
    {
        return strFile;
    }
    else
    {
        return vec[vec.size() - 1];
    }
}
int CFileUtil::GetYear(string strFile)
{
    vector<string> vec = Server::CCommonUtil::StringSplit(strFile, "/");
    if(vec.size() == 0)
    {
        return 0;
    }
    else
    {
        return atoi(vec[0].c_str());
    }
}
string CFileUtil::GetNewFileName(int iYear, string strFileName, string strPostFix)
{
    int iIndex = 0;
    string strThumbRoot = CConfig::GetInstance()->GetThumbRoot();
    string strStoreRoot = CConfig::GetInstance()->GetStoreRoot();
    do
    {
        string strSmallFile = "";
        string strLargeFile = "";
        string strNewFileName = "";

        if(iIndex == 0)
        {
           strNewFileName = strFileName;
            
        }
        else
        {
            strNewFileName = Server::CCommonUtil::StringFormat("%s_%d", strFileName.c_str(), iIndex);
        }
        strSmallFile = Server::CCommonUtil::StringFormat("%s%d/%s.%s", strThumbRoot.c_str(), iYear, strNewFileName.c_str(), "jpg");
        strLargeFile = Server::CCommonUtil::StringFormat("%s%d/%s.%s", strStoreRoot.c_str(), iYear, strNewFileName.c_str(), "jpg");

        if(FALSE == CFileUtil::CheckFileExist(strSmallFile) && FALSE == CFileUtil::CheckFileExist(strLargeFile))
        {
            return strNewFileName;
        }
        iIndex++;
    } while (1);
}
string CFileUtil::GetNewFileName(string strPath)
{
    while(1)
    {
        string strFile = Server::CCommonUtil::StringFormat("%lld", Server::CTools::CurTimeMilSec());
        string strPathFile = Server::CCommonUtil::StringFormat("%s%lld", strPath.c_str(), strFile.c_str());
        if(TRUE == CFileUtil::CheckFileExist(strPathFile))
        {
            Sleep(10);
            continue;
        }
        return strFile;
    }
}
string CFileUtil::GetNewFileName2(int iYear, string strFileName, string strPostFix)
{
    printf("CFileUtil::GetNewFileName2\n");
    int iIndex = 0;
    do
    {
        string strFile("");
        if(iIndex == 0)
        {
            strFile = strFileName;
        }
        else
        {
            strFile = Server::CCommonUtil::StringFormat("%s_%d", strFileName.c_str(), iIndex);
        }
        string strSmallFile = Server::CCommonUtil::StringFormat("%s%d/%s.%s", CConfig::GetInstance()->GetThumbRoot().c_str(), iYear, strFile.c_str(), "jpg");
        string strLargeFile = Server::CCommonUtil::StringFormat("%s%d/%s.%s", CConfig::GetInstance()->GetStoreRoot().c_str(), iYear, strFile.c_str(), strPostFix.c_str());
        if(FALSE == CFileUtil::CheckFileExist(strSmallFile) && FALSE == CFileUtil::CheckFileExist(strLargeFile))
        {
            return strFile;
        }
        iIndex++;
    } while (1);
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
BOOL CFileUtil::RemoveFold(string strFold)
{
    return Server::CCommonUtil::ExecuteCmdWithOutReplay("rm -rf %s", strFold.c_str());
}
BOOL CFileUtil::MoveFile(string strFromFile, string strToFile)
{
    return Server::CCommonUtil::ExecuteCmdWithOutReplay("mv \"%s\" \"%s\"", strFromFile.c_str(), strToFile.c_str());
}
BOOL CFileUtil::CreateEmptyFile(string strFile)
{
    return Server::CCommonUtil::ExecuteCmdWithOutReplay("touch %s", strFile.c_str());
}
string CFileUtil::GetFileContent(const char* pszFile)
{
    FILE* pFile = fopen(pszFile, "r");
    if(NULL == pFile)
    {
       return "";
    }
    fseek(pFile, 0L,SEEK_END);
    int iContentLen = ftell(pFile);
    char* pszFileContent = (char*)malloc(iContentLen + 1);
    memset(pszFileContent, 0, iContentLen + 1);
    fseek(pFile, 0L,SEEK_SET);
    fread(pszFileContent, iContentLen, 1, pFile);
    fclose(pFile);
    pFile = NULL;
    string strFileContent(pszFileContent);
    free(pszFileContent);
    pszFileContent = NULL;
    return strFileContent;
}
string CFileUtil::GetLicence()
{
    return CFileUtil::GetFileContent("/etc/licence");
}
long CFileUtil::GetFileSize(string strFile)
{
    struct stat buf={};
    stat(strFile.c_str(), &buf);
    return buf.st_size;
}
void CFileUtil::SepFile(string strFile, char* pszName, char* pszPostFix)
{
    const char* pszFileName = strFile.c_str();
    int iPosition = Server::CTools::Rfind(pszFileName, '.');
    if(iPosition > 0)
	{
		memcpy(pszName, pszFileName, iPosition);
		memcpy(pszPostFix, pszFileName + iPosition + 1, strFile.length() - iPosition);
	}
    else if(iPosition == 0)
    {
        memcpy(pszPostFix, pszFileName + 1, strlen(pszFileName) - 1);
    }
    else
    {
        memcpy(pszName, pszFileName, strlen(pszFileName));
    }
}
string CFileUtil::ThumbNameFromFileName(string strFileName)
{
    const char* pszFileName = strFileName.c_str();
    int iPosition = Server::CTools::Rfind(pszFileName, '.');
    if(iPosition < 0)
    {
        return "";
    }
    char* pszOut = (char*)malloc(strFileName.length()*2);
    memset(pszOut, 0, strFileName.length()*2);
    memcpy(pszOut, pszFileName, iPosition);
    memcpy(pszOut + iPosition, "_", 1);
    strcat(pszOut, pszFileName + iPosition + 1 );
    strcat(pszOut, ".jpg" );
    string strRet(pszOut);
    free(pszOut);
    pszOut = NULL;
    return strRet;
}
//////media/.media/2021/IMG_20210412_072616_jpg.jpg ==> 2021/IMG_20210412_072616.jpg
string CFileUtil::GetRelativePath(string strFile)
{
    const char* pszFile = strFile.c_str();
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
            string strRet(pszRet);
            free(pszRet);
            pszRet = NULL;
            return strRet;
        }
    }
    if(NULL == pszRet)
    {
        pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
    }
    string strRet(pszRet);
    free(pszRet);
    pszRet = NULL;
    return strRet;
}
int CFileUtil::GetFoldFileCount(char* pszFold)
{
    string strFold = CConfig::GetInstance()->GetStoreRoot();
    strFold.append(pszFold);
    struct dirent* pDirent;
    printf("FileUtil_GetFoldFileCount:%s\n", strFold.c_str());
    DIR *pDir;
    if ((pDir = opendir(strFold.c_str())) == NULL)
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
list<string> CFileUtil::GetSubFolds(char* pszFold)
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
string CFileUtil::CurrentPath()
{
    char szPath[MAX_PATH] = {0};
    int iCount = readlink("/proc/self/exe", szPath, MAX_PATH);
    if (iCount < 0 || iCount >= MAX_PATH)
    {
        return "";
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
    
    string strPath(szPath);
    return strPath;
}
BOOL CFileUtil::IsEmptyDir(const char* pszDir)
{
    /* 打开要进行匹配的文件目录 */
    DIR* pDir = opendir(pszDir);
    if (pDir == NULL)
    {  
        return FALSE;
    }  
    while (1)
    {  
        struct dirent* pDirent = readdir(pDir);
        if (pDirent == NULL)
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
void CFileUtil::RemoveYearEmptyFold(int iYear)
{
    char szFold[MAX_PATH] = {0};
	sprintf(szFold, "%s%d/", CConfig::GetInstance()->GetStoreRoot().c_str(), iYear);
	char szThumbFold[MAX_PATH] = {0};
	sprintf(szThumbFold, "%s%d/", CConfig::GetInstance()->GetThumbRoot().c_str(), iYear);
    char szExFold[MAX_PATH] = {0};
	sprintf(szExFold, "%s%d/", CConfig::GetInstance()->GetExtraRoot().c_str(), iYear);

	if(TRUE == CFileUtil::IsEmptyDir(szFold))
    {
        CFileUtil::RemoveFold(szFold);
		CFileUtil::RemoveFold(szThumbFold);
        CFileUtil::RemoveFold(szExFold);
    }
}
BOOL CFileUtil::CheckStatus(pid_t status)
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
BOOL CFileUtil::IsFile(string strFile)
{
    struct stat s;
    if (stat(strFile.c_str(), &s) == 0)
    {
        if(s.st_mode & S_IFREG)
        {
            return TRUE;
        }
    }
    return FALSE;
}
BOOL CFileUtil::IsFold(string strFold)
{
    struct stat s;
    if (stat(strFold.c_str(), &s) == 0)
    {
        if(s.st_mode & S_IFDIR)
        {
            return TRUE;
        }
    }
    return FALSE;
}

string CFileUtil::FileNameFromThumbName(string strThumbFileName)
{
    int iPosition = Server::CTools::Rfind(strThumbFileName.c_str(), '.');
    if(iPosition <= 0)
    {
        return "";
    }
    char szTmpFileName[MAX_PATH] = {0};
    memcpy(szTmpFileName, strThumbFileName.c_str(), iPosition);
    iPosition = Server::CTools::Rfind(szTmpFileName, '_');
    if(iPosition <= 0)
    {
        return "";
    }
    char* pszFileName = (char*)malloc(strThumbFileName.length()*2);
    memset(pszFileName, 0, strThumbFileName.length()*2);
    memcpy(pszFileName, szTmpFileName, iPosition);
    memcpy(pszFileName + iPosition, ".", 1);
    strcpy(pszFileName + iPosition + 1, szTmpFileName + iPosition + 1);
    string strFileName(pszFileName);
    free(pszFileName);
    pszFileName = NULL;
    return strFileName;
}
string CFileUtil::FileExNameFromFileName(string strThumbFileName)
{
    const char* pszThumbFileName = strThumbFileName.c_str();
    int iPosition = Server::CTools::Rfind(pszThumbFileName, '.');
    if(iPosition <= 0)
    {
        return "";
    }
    char* pszFileExName = (char*)malloc(strThumbFileName.length()*2);
    memset(pszFileExName, 0, strThumbFileName.length()*2);
    memcpy(pszFileExName, pszThumbFileName, iPosition);
    memcpy(pszFileExName + iPosition, "_", 1);
    strcat(pszFileExName, pszThumbFileName + iPosition + 1);
    strcat(pszFileExName, ".mp4");
    string strFileExName(pszFileExName);
    free(pszFileExName);
    pszFileExName = NULL;
    return strFileExName;
}
string CFileUtil::ReadLink(string strFileName)
{
    struct stat stat = {0};
    if (lstat(strFileName.c_str(), &stat) == -1)
    {
        return "";
    }
    BOOL bLink =  S_ISLNK(stat.st_mode) ? TRUE : FALSE;
    if(FALSE == bLink)
    {
        return "";
    }
    char szFullFileName[MAX_PATH] = {0};
    int iFileLen = readlink(strFileName.c_str(), szFullFileName, MAX_PATH);
    if(iFileLen == -1)
    {
        return "";
    }
    if(FALSE == CheckFileExist(szFullFileName))
    {
        return "";
    }
    string strFullFileName(szFullFileName);
    return strFullFileName;
}
BOOL CFileUtil::IsSoftLink(string strFileName)
{
    struct stat stat = {0};
    if (lstat(strFileName.c_str(), &stat) == -1)
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