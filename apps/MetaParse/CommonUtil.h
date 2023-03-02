#ifndef COMMONUTIL_H__
#define COMMONUTIL_H__
#include <string>
#include <vector>
#include <string.h>
#include "stdafx.h"
class CCommonUtil
{
private:
public:
    CCommonUtil();
    ~CCommonUtil();
public:
    static std::vector<std::string> StringSplit(const  std::string& s, const std::string& delim/* = ","*/);
    static std::string TimeToSec(std::string strTime);
    static std::string StringFormat(const char* pszFmt, ...);
    static uint64_t CurTimeSec();
    static std::string& ReplaceStr(std::string& str, const std::string& strFrom, const std::string& strTo);
    static std::string SecToTime(long iSec);
    static std::string ExecuteCmd(const char* pszCmd);
    static std::string TrimStr(std::string str);
    static std::string GetMime(const char* pszFile);
    static BOOL IsMimeTypeImage(const char* pszMime);
    static BOOL IsMimeTypeVideo(const char* pszMime);
    static BOOL CheckFileExist(const char* pszFile);
    static BOOL CheckMagicFileExist(const char* pszPath);
    
};


#endif