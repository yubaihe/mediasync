#pragma once
#include "../stdafx.h"
namespace Server
{
    class CCommonUtil
    {
    public:
    static std::vector<std::string> StringSplit(const  std::string& s, const std::string& delim/* = ","*/);
    static std::string StringFormat(const char* pszFmt, ...);
    static string ExecuteCmd2(string strCmd);
    static std::string ExecuteCmd(const char* pszFormat, ...);
    static BOOL ExecuteCmdWithOutReplay(const char* pszFormat, ...);
    static BOOL CheckStatus(pid_t status, const char* pszCommand);
    static string ListToString(list<string> List, string strSep);
    static list<string> AllNetNames();
    static BOOL ContainNetName(string strNetName);
    };
}



