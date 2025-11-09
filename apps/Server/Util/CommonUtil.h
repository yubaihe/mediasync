#pragma once
#include "../stdafx.h"
namespace Server
{
    typedef enum
    {
        COLORTYPE_SUCCESS = 0,
        COLORTYPE_WARNING = 1,
        COLORTYPE_ERROR = 2,
    }COLORTYPE;
    class CCommonUtil
    {
    public:
    static std::vector<std::string> StringSplit(const  std::string& s, const std::string& delim = ",");
    static std::string StringFormat(const char* pszFmt, ...);
    static string ExecuteCmd2(string strCmd);
    static std::string ExecuteCmd(const char* pszFormat, ...);
    static BOOL ExecuteCmdWithOutReplay(const char* pszFormat, ...);
    static BOOL CheckStatus(pid_t status, const char* pszCommand);
    static string ListToString(list<string> List, string strSep);
    static list<string> AllNetNames();
    static BOOL ContainNetName(string strNetName);
    static void ColorPrintf(COLORTYPE eColorType, const char* pszFormat, ...);
    static uint64_t CurTimeSec();
    static BOOL CheckFoldEmpty(const char* pszFold);
    static BOOL RemoveFold(const char* pszFold, BOOL bForce = TRUE);
    static BOOL CheckCmdStatus(pid_t iStatus);
    };
}



