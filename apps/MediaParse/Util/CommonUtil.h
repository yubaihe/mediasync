#pragma once
#include <string>
#include <vector>
#include <string.h>
#include <sys/stat.h>
#include "stdafx.h"
class CCommonUtil
{
public:
    CCommonUtil();
    ~CCommonUtil();
public:
    static std::vector<std::string> StringSplit(const  std::string& s, const std::string& delim/* = ","*/);
    static time_t TimeToSec(std::string strTime);
    static std::string StringFormat(const char* pszFmt, ...);
    static uint64_t CurTimeSec();
    static std::string& ReplaceStr(std::string& str, const std::string& strFrom, const std::string& strTo);
    static std::string SecToTime(long iSec);
    static char* ExecuteCmd2(const char* pszCmd);
    static std::string ExecuteCmd(const char* pszFormat, ...);
    static void ExecuteCmdWithOutReplay(const char* pszFormat, ...);
    static std::string TrimStr(std::string str);
    static BOOL CheckFileExist(const char* pszFile);
    static BOOL CheckMagicFileExist(const char* pszPath);
    //2024-08-29T05:44:43.000000Z  ==> 2024-08-29 05:44:43
    static std::string TransTime(const char* pszInput);
    static int64_t GetFileSize(const char* pszFile);
    static BOOL CheckFoldExist(const char* pszFold);
    static BOOL CheckFoldEmpty(const char* pszFold);
    static BOOL RemoveFold(const char* pszFold, BOOL bForce = TRUE);
    static BOOL RemoveFile(const char* pszFile);
    static BOOL MoveFile(string strFrom, string strTo);
    static BOOL CheckCmdStatus(pid_t iStatus);
    static BOOL CreateFold(const char* pszFold);
    static std::string GetPath(const char* pszFile);
    static std::string GetMime(const char* pszFile);
    static BOOL IsMimeTypeImage(const char* pszMime);
    static BOOL IsMimeTypeVideo(const char* pszMime);
    static BOOL IsVideoFile(const char* pszFile);
    static string SizeToString(long long iSize);
    static string SecondToTime(long iSec);
    static BOOL IsFoldEmpty(const char* pszFold);
    static BOOL EndWith(const std::string& str, const std::string& strSuffix);
    static string ListStringToString(list<string> List, string strSep);
    static string ListIntToString(list<int> List, string strSep);
    static string VectorStringToString(vector<string> vec, string strSep);
    static string RemoveFilePostfix(string strFile);
    static string GetFilePostfix(string strFile);
    static BOOL SpliteFile(string strFile, string& strFilePrefx, string& strPostfix);
    static time_t CurTimeMilSec();
    static int GetRandomNum(int iMax);
    static vector<string> GpsToVec(string strGps);
    static string GetMd5(const char* psz);
};
