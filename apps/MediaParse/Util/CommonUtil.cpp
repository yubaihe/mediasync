#include "CommonUtil.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/timeb.h>  
#include <sys/stat.h>
#include <sys/wait.h>
#include "Dbus/libdbus.h"
#include "JsonUtil.h"
#include "md5.h"
#include "Base64Coding.h"
CCommonUtil::CCommonUtil()
{

}

CCommonUtil::~CCommonUtil()
{

}

std::vector<std::string> CCommonUtil::StringSplit(const  std::string& s, const std::string& delim/* = ","*/)
{
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len)
    {
        int find_pos = s.find(delim, pos);
        if (find_pos < 0)
        {
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

time_t CCommonUtil::TimeToSec(std::string strTime)
{
    struct tm tm = {0};  // 初始化时间结构体
    int iParsed = sscanf(strTime.c_str(), "%d-%d-%d %d:%d:%d",
                        &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                        &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    if (iParsed != 6)
    {
        return 0;
    }  // 格式错误检查
    
    // 调整年份和月份（tm_year从1900开始，tm_mon范围0-11）
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    tm.tm_isdst = -1;
    return mktime(&tm);
}

std::string CCommonUtil::StringFormat(const char* pszFmt, ...)
{
    std::string str;
    va_list args;
    va_start(args, pszFmt);
    int nLength = vsnprintf(NULL, 0, pszFmt, args);
    nLength += 1;  //上面返回的长度是包含\0，这里加上
    va_end(args);
    va_start(args, pszFmt);
    std::vector<char> vectorChars(nLength);
    vsnprintf(vectorChars.data(), nLength, pszFmt, args);

    str.assign(vectorChars.data());
    va_end(args);
    return str;
}

uint64_t CCommonUtil::CurTimeSec()
{
    //long long iTimeLast;
    //iTimeLast = time(NULL);     //总秒数  
    time_t curMilSec;
    time(&curMilSec);
    return curMilSec;
}

std::string& CCommonUtil::ReplaceStr(std::string& str, const std::string& strFrom, const std::string& strTo)
{
    for (std::string::size_type pos(0); pos != std::string::npos; pos += strTo.length())
    {
        pos = str.find(strFrom, pos);
        if (pos != std::string::npos)
        {
            str.replace(pos, strFrom.length(), strTo);
        }
        else
        {
            break;
        }
    }
    return str;
}

std::string CCommonUtil::SecToTime(long iSec)
{
    struct tm* ptr;
    time_t lt;
    lt = iSec;
    ptr = localtime(&lt);
    return CCommonUtil::StringFormat("%04d-%02d-%02d %02d:%02d:%02d", ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday, ptr->tm_hour, ptr->tm_min, ptr->tm_sec);
}
std::string CCommonUtil::TrimStr(std::string str)
{
    if (str.empty())
    {
        return str;
    }

    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}

BOOL CCommonUtil::CheckFileExist(const char* pszFile)
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

BOOL CCommonUtil::CheckMagicFileExist(const char* pszPath)
{
    int iLen = strlen(pszPath);
    if(iLen == 0)
    {
        return FALSE;
    }
    char* psz = (char*)malloc(iLen);
    memset(psz, 0, iLen);
    int i = 0;
    int iIndex = 0;
    BOOL bExist = FALSE;
    while (pszPath[i] != '\0')
    {
        if(pszPath[i] == ':')
        {
            BOOL bRet = CCommonUtil::CheckFileExist(psz);
            if(TRUE == bRet)
            {
                bExist = TRUE;
                break;
            }
            iIndex = 0;
            memset(psz, 0, iLen);
        }
        else
        {
            psz[iIndex++] = pszPath[i];
        }
        
        i++;
    }
    if(FALSE == bExist && iIndex > 0)
    {
        bExist = CCommonUtil::CheckFileExist(psz);
    }
    free(psz);
    psz = NULL;
    return bExist;
}

std::string CCommonUtil::TransTime(const char* pszInput)
{
    char szOut[100] = {0};
    struct tm time = {0};
    
    // 解析输入时间（兼容微秒和时区标识）
    if (strptime(pszInput, "%Y-%m-%dT%H:%M:%S", &time) == NULL) 
    {
        fprintf(stderr, "时间解析失败\n");
        return "";
    }
    time.tm_isdst = -1; // 自动处理夏令时 

    // 转换为 time_t 类型（兼容时区处理）
    time_t t = mktime(&time);
    
    strftime(szOut, sizeof(szOut), "%Y-%m-%d %H:%M:%S", localtime(&t));
    std::string strOut(szOut);
    return strOut;
}

char* CCommonUtil::ExecuteCmd2(const char* pszCommand)
{
    char* pszRet = (char*)malloc(1);
    memset(pszRet, 0, 1);
    int iPipefd[2] = {0};
    pid_t pid;
    // 创建管道
    if (pipe(iPipefd) == -1)
    {
        return pszRet;
    }
    pid = fork();
    if (pid == -1)
    {
        return pszRet;
    }

    if (pid == 0)
    {
        // 子进程：关闭读端，重定向stdout到写端
        close(iPipefd[0]);
        if (dup2(iPipefd[1], STDOUT_FILENO) == -1) 
        {
            return pszRet;
        }
        close(iPipefd[1]);
        const char* pszArgs[] = {"/bin/sh", "-c", pszCommand, nullptr};
        execvp(pszArgs[0], const_cast<char**>(pszArgs));
        // 如果execvp返回，说明出错了
        return pszRet;
    }
    else
    {
        // 父进程：关闭写端，读取子进程的标准输出
        close(iPipefd[1]);
        ssize_t iNumRead = 0;
        char szBuffer[1024] = {0};
        while ((iNumRead = read(iPipefd[0], szBuffer, sizeof(szBuffer) - 1)) > 0)
        {
            int iNewLen = iNumRead + strlen(pszRet) + 1;
            pszRet = (char*)realloc(pszRet, iNewLen);
            strcat(pszRet, szBuffer);
            memset(szBuffer, 0, sizeof(szBuffer));
        }
        if (iNumRead == -1)
        {
            return pszRet;
        }
        close(iPipefd[0]);

        // 等待子进程结束
        int iStatus;
        if (waitpid(pid, &iStatus, 0) == -1)
        {
            return pszRet;
        }
        if (!WIFEXITED(iStatus)) 
        {
            return pszRet;
        }
    }
    return pszRet;
}
std::string CCommonUtil::ExecuteCmd(const char* pszFormat, ...)
{
    char* pszCmd = NULL;
    int iCmdLen = 0;
    va_list vArgList;
    va_start (vArgList, pszFormat);
    iCmdLen = vsnprintf(NULL, 0, pszFormat, vArgList) + 1;
    va_end(vArgList);

    va_start (vArgList, pszFormat); 
    pszCmd = (char*)malloc(iCmdLen);
    memset(pszCmd, 0, iCmdLen);
    vsnprintf(pszCmd, iCmdLen, pszFormat, vArgList);
    printf("%s\n", pszCmd);
    va_end(vArgList);
    char* pszRet = CCommonUtil::ExecuteCmd2(pszCmd);
    std::string strRet(pszRet);

    free(pszRet);
    pszRet = NULL;

    free(pszCmd);
    pszCmd = NULL;
    return strRet;
}

void CCommonUtil::ExecuteCmdWithOutReplay(const char* pszFormat, ...)
{
    char* pszCmd = NULL;
    int iCmdLen = 0;
    va_list vArgList;
    va_start (vArgList, pszFormat);
    iCmdLen = vsnprintf(NULL, 0, pszFormat, vArgList) + 1;
    va_end(vArgList);

    va_start (vArgList, pszFormat); 
    pszCmd = (char*)malloc(iCmdLen);
    memset(pszCmd, 0, iCmdLen);
    vsnprintf(pszCmd, iCmdLen, pszFormat, vArgList);
    va_end(vArgList);
    char* pszRet = ExecuteCmd2(pszCmd);
    if(NULL != pszRet)
    {
        free(pszRet);
        pszRet = NULL;
    }
    printf("Execute command:%s\n", pszCmd);
    free(pszCmd);
    pszCmd = NULL;
} 
int64_t CCommonUtil::GetFileSize(const char* pszFile)
{
    struct stat statBuf;
    int rc = stat(pszFile, &statBuf);
    if (rc != 0) 
    {
        return 0;
    }
    return statBuf.st_size;
}

BOOL CCommonUtil::CheckFoldExist(const char* pszFold)
{
    return CCommonUtil::CheckFileExist(pszFold);
}
BOOL CCommonUtil::CheckFoldEmpty(const char* pszFold)
{
    DIR* pDir = opendir(pszFold);
    if(NULL == pDir)
    {
        return FALSE;
    }
    
    int iFileCount = 0;
    struct dirent* pEntry = NULL;
    while ((pEntry = readdir(pDir)) != NULL) 
    {
        if (strcmp(pEntry->d_name, ".") != 0 && strcmp(pEntry->d_name, "..") != 0)
        {
            iFileCount++;
            break;
        }
    }
    closedir(pDir);
    return iFileCount == 0?TRUE:FALSE;
}
BOOL CCommonUtil::CheckCmdStatus(pid_t iStatus)
{
    if (-1 == iStatus)
    {
        printf("system error!");
        return FALSE;
    }
    else
    {
        //printf("exit status value = [0x%x]\n", status);
        if (WIFEXITED(iStatus))
        {
            if (0 == WEXITSTATUS(iStatus))
            {
                printf("run shell script successfully.\n");
                return TRUE;
            }
            else
            {
                printf("run shell script fail, script exit code: %d\n", WEXITSTATUS(iStatus));
                return FALSE;
            }
        }
        else
        {
            printf("exit status = [%d]\n", WEXITSTATUS(iStatus));
            return FALSE;
        }
    }
}
BOOL CCommonUtil::RemoveFold(const char* pszFold, BOOL bForce/* = TRUE*/)
{
    int iLen = strlen(pszFold) + 20;
    char* pszCommand = (char*)malloc(iLen);
    memset(pszCommand, 0, iLen);
    if(TRUE == bForce)
    {
        sprintf(pszCommand, "rm -rf \"%s\"", pszFold);
    }
    else
    {
        sprintf(pszCommand, "rmdir \"%s\"", pszFold);
    }
    printf("%s\n", pszCommand);
    int iStatus = system(pszCommand);
    free(pszCommand);
    pszCommand = NULL;
    return CheckCmdStatus(iStatus);
}
BOOL CCommonUtil::RemoveFile(const char* pszFile)
{
    return CCommonUtil::RemoveFold(pszFile);
}
BOOL CCommonUtil::CreateFold(const char* pszFold)
{
    if(CheckFoldExist(pszFold))
    {
        return TRUE;
    }
    int iLen = strlen(pszFold) + 20;
    char* pszCommand = (char*)malloc(iLen);
    memset(pszCommand, 0, iLen);
    sprintf(pszCommand, "mkdir -p \"%s\"", pszFold);
    int iStatus = system(pszCommand);
    free(pszCommand);
    pszCommand = NULL;
    return CheckCmdStatus(iStatus);
}
std::string CCommonUtil::GetPath(const char* pszFile)
{
    string strFile(pszFile);
    size_t iPos = strFile.find_last_of('/');  
    if (iPos == std::string::npos)
    {
        return "/";
    }
    return strFile.substr(0, iPos);
} 

std::string CCommonUtil::GetMime(const char* pszFile)
{
    if(FALSE == CheckFileExist(pszFile))
    {
        return "";
    }
    char szCmd[300] = {0};
    sprintf(szCmd, "file -b --mime-type \"%s\"", pszFile);
    std::string strMime = CCommonUtil::ExecuteCmd(szCmd);
    strMime.erase(std::remove(strMime.begin(), strMime.end(), '\n'), strMime.end());
    //std::transform(strMime.begin(), strMime.end(), strMime.begin(), ::toupper);
    return strMime;
}

BOOL CCommonUtil::IsMimeTypeImage(const char* pszMime)
{
    char szImage[] = "image";
    if(strlen(pszMime) > strlen(szImage) && 0 == memcmp(pszMime, szImage, strlen(szImage)))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CCommonUtil::IsMimeTypeVideo(const char* pszMime)
{
    char szVideo[] = "video";
    if(strlen(pszMime) > strlen(szVideo) && 0 == memcmp(pszMime, szVideo, strlen(szVideo)))
    {
        return TRUE;
    }
    return FALSE;
}
BOOL CCommonUtil::IsVideoFile(const char* pszFile)
{
    std::string strMime = CCommonUtil::GetMime(pszFile);
    return CCommonUtil::IsMimeTypeVideo(strMime.c_str());
}
string CCommonUtil::SizeToString(long long iSize)
{
    string strSize = "";
    if (iSize >= 1024 * 1024 * 1024)
    {
        strSize = CCommonUtil::StringFormat("%.2fGB", iSize / (1024 * 1024 * 1024 * 1.0));
    }
    else if (iSize >= 1024 * 1024)
    {
        strSize = CCommonUtil::StringFormat("%.1fMB", iSize / (1024 * 1024 * 1.0));
    }
    else if (iSize >= 1024)
    {
        strSize = CCommonUtil::StringFormat("%dKB", (int)(iSize / 1024));
    }
    else
    {
        strSize = CCommonUtil::StringFormat("%dBytes", (int)iSize);
    }
    return strSize;
}
string CCommonUtil::SecondToTime(long iSec)
{
    long iDays = iSec / 86400;//转换天数
    iSec = iSec % 86400;//剩余秒数
    long iHours = iSec / 3600;//转换小时数
    iSec = iSec % 3600;//剩余秒数
    long iMinutes = iSec / 60;//转换分钟
    iSec = iSec % 60;//剩余秒数
    if (iDays > 0)
    {
        //string strRet = CCommonUtil::StringFormat(CLanguage::Text("%02d天%02d小时%02d分%02d秒").c_str(), (int)iDays, (int)iHours, (int)iMinutes, (int)iSec);
        string strRet = CCommonUtil::StringFormat("%02d天%02d小时%02d分%02d秒", (int)iDays, (int)iHours, (int)iMinutes, (int)iSec);
        return strRet;
    }
    if (iHours > 0)
    {
        //string strRet = CCommonUtil::StringFormat(CLanguage::Text("%02d小时%02d分%02d秒").c_str(), (int)iHours, (int)iMinutes, (int)iSec);
        string strRet = CCommonUtil::StringFormat("%02d小时%02d分%02d秒", (int)iHours, (int)iMinutes, (int)iSec);
        return strRet;
    }
    if (iMinutes > 0)
    {
        //string strRet = CCommonUtil::StringFormat(CLanguage::Text("%02d分%02d秒").c_str(), (int)iMinutes, (int)iSec);
        string strRet = CCommonUtil::StringFormat("%02d分%02d秒", (int)iMinutes, (int)iSec);
        return strRet;
    }
    //string strRet = CCommonUtil::StringFormat(CLanguage::Text("%d秒").c_str(), (int)iSec);
    string strRet = CCommonUtil::StringFormat("%d秒", (int)iSec);
    return strRet;
}
BOOL CCommonUtil::MoveFile(string strFrom, string strTo)
{
    if(FALSE == CCommonUtil::CheckFileExist(strFrom.c_str()))
    {
        return FALSE;
    }
    CCommonUtil::ExecuteCmd("mv %s %s", strFrom.c_str(), strTo.c_str());
    return TRUE;
}
BOOL CCommonUtil::IsFoldEmpty(const char* pszFold)
{
    BOOL bEmpty = TRUE;  
    struct dirent* pEntry;
    DIR* pDir = opendir(pszFold);
    if(NULL == pDir)
    {
        return FALSE;
    }
    while ((pEntry = readdir(pDir)) != NULL)
    {  
        if (strcmp(pEntry->d_name, ".") == 0 || strcmp(pEntry->d_name, "..") == 0) 
        {  
            continue;  
        }
        bEmpty = FALSE;
        break;
    }
    closedir(pDir);  
    return bEmpty;
}
BOOL CCommonUtil::EndWith(const std::string& str, const std::string& strSuffix)
{
    return str.rfind(strSuffix) == str.size() - strSuffix.size();
}
string CCommonUtil::ListStringToString(list<string> List, string strSep)
{
    if(List.size() == 0)
    {
        return "";
    }
    string strRet = "";
    for(list<string>::iterator itor = List.begin(); itor != List.end(); ++itor)
    {
        strRet = strRet.append(*itor);
        strRet = strRet.append(strSep);
    }
    if (strRet.size() >= strSep.length() && strRet.substr(strRet.size() - strSep.length()) == strSep)
    {
        strRet.resize(strRet.size() - strSep.length());
    }
    return strRet;
}
string CCommonUtil::VectorStringToString(vector<string> vec, string strSep)
{
    if(vec.size() == 0)
    {
        return "";
    }
    string strRet = "";
    for(vector<string>::iterator itor = vec.begin(); itor != vec.end(); ++itor)
    {
        strRet = strRet.append("'");
        strRet = strRet.append(*itor);
        strRet = strRet.append("'");
        strRet = strRet.append(strSep);
    }
    if (strRet.size() >= strSep.length() && strRet.substr(strRet.size() - strSep.length()) == strSep)
    {
        strRet.resize(strRet.size() - strSep.length());
    }
    return strRet;
}
string CCommonUtil::ListIntToString(list<int> List, string strSep)
{
    if(List.size() == 0)
    {
        return "";
    }
    string strRet = "";
    for(list<int>::iterator itor = List.begin(); itor != List.end(); ++itor)
    {
        string strValue = CCommonUtil::StringFormat("%d", *itor);
        strRet = strRet.append(strValue);
        strRet = strRet.append(strSep);
    }
    if (strRet.size() >= strSep.length() && strRet.substr(strRet.size() - strSep.length()) == strSep)
    {
        strRet.resize(strRet.size() - strSep.length());
    }
    return strRet;
}
string CCommonUtil::RemoveFilePostfix(string strFile)
{
    size_t iIndex = strFile.rfind(".");
    if (iIndex != string::npos && iIndex > 0) 
    {
        return strFile.substr(0, iIndex);
    } 
    return strFile;
}
string CCommonUtil::GetFilePostfix(string strFile)
{
    size_t iIndex = strFile.rfind(".");
    if (iIndex != string::npos && iIndex > 0) 
    {
        return strFile.substr(iIndex + 1);
    }
    return "";
}
BOOL CCommonUtil::SpliteFile(string strFile, string& strFilePrefx, string& strPostfix)
{
    size_t iIndex = strFile.rfind(".");
    if (iIndex != string::npos && iIndex > 0) 
    {
        strFilePrefx = strFile.substr(0, iIndex);
        strPostfix = strFile.substr(iIndex + 1);
        return TRUE;
    }
    return FALSE;
}
time_t CCommonUtil::CurTimeMilSec()
{
    struct timeval tv;    
    gettimeofday(&tv,NULL);    
    return ((time_t)tv.tv_sec) * 1000 + tv.tv_usec / 1000; 
}
int CCommonUtil::GetRandomNum(int iMax)
{
    int iRandom = rand() % iMax;
    return iRandom;
}
vector<string> CCommonUtil::GpsToVec(string strGps)
{
    vector<string> vecRet;
    vector<string> gps = CCommonUtil::StringSplit(strGps, "&");
    if(gps.size() < 2)
    {
        vecRet.push_back("0.0");
        vecRet.push_back("0.0");
    }
    else
    {
        vecRet.push_back(gps[0]);
        vecRet.push_back(gps[1]);
    }
    return vecRet;
}

string CCommonUtil::GetMd5(const char* psz)
{
    md5_state_t md5StateT;
    md5_init(&md5StateT);
    md5_append(&md5StateT, (const md5_byte_t *)psz, (int)strlen(psz));
    md5_byte_t digest[16];
    md5_finish(&md5StateT, digest);
    char szMd5[33] = {0}, szHexBuffer[5] = {0};
    for (size_t i = 0; i != 16; ++i)
    {
        if (digest[i] < 16)
            sprintf(szHexBuffer, "0%x", digest[i]);
        else
            sprintf(szHexBuffer, "%x", digest[i]);
        strcat(szMd5, szHexBuffer);
    }
    //printf("%s\n", szMd5String);
    char szBase64[1024] = {0};
    int iBufferLen = 1024;
    base64_encode((unsigned char*)szMd5, 32, (unsigned char*)szBase64, &iBufferLen);
    
    string strRet(szBase64);
    return strRet;
}