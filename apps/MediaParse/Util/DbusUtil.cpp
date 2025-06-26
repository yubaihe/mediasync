#include "DbusUtil.h"
#include "JsonUtil.h"
CDbusUtil::CDbusUtil()
{
}

CDbusUtil::~CDbusUtil()
{
}
string CDbusUtil::GetUploadFileName(void)
{
    char szRetBuffer[MAX_DBUS_RECV_LEN] = {0};
    int iDbusLen = MAX_DBUS_RECV_LEN;
    char szRequest[] = "{\"action\":\"upfilename\"}";
    BOOL bRet = LibDbus_SendSync("Server", 0x5007, szRequest, strlen(szRequest) + 1, szRetBuffer, &iDbusLen);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return "";
    }
    //sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"filename\":\"%s\",\"conport\":\"%d\"}", MESSAGETYPECMD_UPLOADFILE_ACK, szFileName, 0);

    nlohmann::json jsonValue;
    bRet = MediaParse::CJsonUtil::FromString(szRetBuffer, jsonValue);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return "";
    }
    return jsonValue["filename"];
}
StoreInfo CDbusUtil::GetStoreInfo(void)
{
    StoreInfo storeInfo;
    char szRetBuffer[MAX_DBUS_RECV_LEN] = {0};
    int iDbusLen = MAX_DBUS_RECV_LEN;
    BOOL bRet = LibDbus_SendSync("Server", 0x5007, (const char*)"{\"action\":\"store\"}", strlen("{\"action\":\"store\"}") + 1, szRetBuffer, &iDbusLen);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return storeInfo;
    }
    nlohmann::json jsonValue;
    bRet = MediaParse::CJsonUtil::FromString(szRetBuffer, jsonValue);
    if(FALSE == bRet)
    {
        return storeInfo;
    }
    storeInfo.strStorage = jsonValue["addr"];
    storeInfo.strTempAddr = jsonValue["tempaddr"];
    return storeInfo;
}
DiskInfo CDbusUtil::GetDiskInfo(void)
{
	DiskInfo diskInfo;
    memset(&diskInfo, 0, sizeof(diskInfo));
    char szRequest[] = "{\"action\":\"diskinfo\"}";
    char szRetBuffer[MAX_DBUS_RECV_LEN] = {0};
    int iDbusLen = MAX_DBUS_RECV_LEN;
    BOOL bRet = LibDbus_SendSync("Server", 0x5007, szRequest, strlen(szRequest) + 1, szRetBuffer, &iDbusLen);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return diskInfo;
    }
    nlohmann::json jsonValue;
    bRet = MediaParse::CJsonUtil::FromString(szRetBuffer, jsonValue);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return diskInfo;
    }
    diskInfo.iTotal = jsonValue["total"];
    diskInfo.iUsed = jsonValue["used"];
    return diskInfo;
}
BOOL CDbusUtil::CheckMd5(const char* pszMd5, char* pszErrorInfo, char* pszRemoteFile)
{
    DiskInfo diskInfo;
    memset(&diskInfo, 0, sizeof(diskInfo));
    char szRequest[100] = {0};
    sprintf(szRequest, "{\"action\":\"checkmd5\",\"md5\":\"%s\"}", pszMd5);
    char szRetBuffer[MAX_DBUS_RECV_LEN] = {0};
    int iDbusLen = MAX_DBUS_RECV_LEN;
    BOOL bRet = LibDbus_SendSync("Server", 0x5007, szRequest, strlen(szRequest) + 1, szRetBuffer, &iDbusLen);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return FALSE;
    }
    nlohmann::json jsonValue;
    bRet = MediaParse::CJsonUtil::FromString(szRetBuffer, jsonValue);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return FALSE;
    }
    BOOL iStatus = jsonValue["status"];
    if(FALSE == iStatus)
    {
        strcpy(pszErrorInfo, jsonValue["info"].get<std::string>().c_str());
        strcpy(pszRemoteFile, jsonValue["filename"].get<std::string>().c_str());
    }
    return iStatus;
}
BOOL CDbusUtil::ReportInfo(const char* pszData, char* pszErrorInfo)
{
    
    char szRetBuffer[MAX_DBUS_RECV_LEN] = {0};
    int iDbusLen = MAX_DBUS_RECV_LEN;
    BOOL bRet = LibDbus_SendSync("Server", 0x5007, pszData, strlen(pszData) + 1, szRetBuffer, &iDbusLen);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return FALSE;
    }
    nlohmann::json jsonValue;
    bRet = MediaParse::CJsonUtil::FromString(szRetBuffer, jsonValue);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return FALSE;
    }
    BOOL iStatus = jsonValue["status"];
    if(FALSE == iStatus)
    {
        printf("Report info error:%s\n", jsonValue["info"].get<std::string>().c_str());
        strcpy(pszErrorInfo, jsonValue["info"].get<std::string>().c_str());
    }
    return iStatus;
}