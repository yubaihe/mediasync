#pragma once
enum LOGLEVEL
{
    LOGLEVEL_DEBUG = 0,
    LOGLEVEL_INFO,
    LOGLEVEL_ERROR
};
extern void CommonSdk_WriteLog(LOGLEVEL eLogLevel, const char* pszFun, const char* pszFile, int iLine, const char* pszLog);
class CCommonSdkUtil
{
public:
    CCommonSdkUtil();
    ~CCommonSdkUtil();
    static void Log(LOGLEVEL eLogLevel, const char* pszFun, const char* pszFile, int iLine, const char* pszFormat, ...);
};
