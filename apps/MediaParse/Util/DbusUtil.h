#pragma once
#include "../stdafx.h"
#include "Dbus/libdbus.h"
#define DBUS_MEDIAPARSE "MediaParse"
#define MAX_DBUS_RECV_LEN 65535
struct StoreInfo
{
    string strStorage; //挂载点映射的存储路径
    string strTempAddr; //上传临时目录的地址
};
struct DiskInfo
{
    size_t iTotal; //KB
    size_t iUsed; //KB
};
class CDbusUtil
{
public:
    CDbusUtil();
    ~CDbusUtil();
    static string GetUploadFileName(void);
    static StoreInfo GetStoreInfo(void);
    static DiskInfo GetDiskInfo(void);
    static BOOL CheckMd5(const char* pszMd5, char* pszErrorInfo, char* pszRemoteFile);
    static BOOL ReportInfo(const char* pszData, char* pszErrorInfo);
    static BOOL TranscodeFinish(int iItemID, string strFile, string strMd5, string strIdentify);
    static BOOL SetMediaComment(int iItemID, string strComment);
    static BOOL RemoveMediaComment(int iItemID);
    static BOOL RemoveMediaCommentFromIds(string strIds);
private:

};


