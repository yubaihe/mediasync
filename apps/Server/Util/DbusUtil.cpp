#include "DbusUtil.h"
#include "JsonUtil.h"
#include "CommonUtil.h"
CDbusUtil::CDbusUtil()
{

}
CDbusUtil::~CDbusUtil()
{

}
BOOL CDbusUtil::ContainModule(string strModuleName)
{
    char szService[5000] = {0};
    LibDbus_ServiceList(szService);
    std::vector<std::string> vec = Server::CCommonUtil::StringSplit(szService, ",");
    for(size_t i = 0; i < vec.size(); ++i)
    {
        if(0 == strModuleName.compare(vec[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

string CDbusUtil::AllDisks()
{
    if(FALSE == CDbusUtil::ContainModule(DBUS_MEDIAPARSE))
    {
        return "{\"status\":0}";
    }
    char szMessage[MAX_DBUSMESSAGE_LEN] = {0};
    sprintf(szMessage, "{\"action\":\"listdisk\"}");
    char szRet[MAX_DBUSMESSAGE_LEN] = {0};
    int iRetLen = MAX_DBUSMESSAGE_LEN;
    BOOL bSend = LibDbus_SendSync(DBUS_MEDIAPARSE, 0x5007, (const char*)szMessage, strlen(szMessage) + 1, szRet, &iRetLen);
    if(FALSE == bSend)
    {
        return "{\"status\":0}";
    }
    return szRet;
}

BOOL CDbusUtil::GenThumb(int iMediaType, string strFile, string strThumbFile)
{
    if(FALSE == CDbusUtil::ContainModule(DBUS_MEDIAPARSE))
    {
        return FALSE;
    }
    char szMessage[MAX_DBUSMESSAGE_LEN] = {0};
    sprintf(szMessage, "{\"action\":\"genthumb\",\"mediatype\":%d,\"file\":\"%s\",\"thumb\":\"%s\"}", iMediaType, strFile.c_str(), strThumbFile.c_str());
    char szRet[MAX_DBUSMESSAGE_LEN] = {0};
    int iRetLen = MAX_DBUSMESSAGE_LEN;
    BOOL bSend = LibDbus_SendSync(DBUS_MEDIAPARSE, 0x5007, (const char*)szMessage, strlen(szMessage) + 1, szRet, &iRetLen);
    if(FALSE == bSend)
    {
        return FALSE;
    }
    nlohmann::json json;
    BOOL bRet = Server::CJsonUtil::FromString(szRet, json);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    if(0 == json["status"])
    {
        return FALSE;
    }
    return TRUE;
}
string CDbusUtil::MediaParseGps(string strFile)
{
    if(FALSE == CDbusUtil::ContainModule(DBUS_MEDIAPARSE))
    {
        return "{\"status\":0}";
    }
    char szMessage[MAX_DBUSMESSAGE_LEN] = {0};
    sprintf(szMessage, "{\"action\":\"parsegps\",\"file\":\"%s\"}", strFile.c_str());
    char szRet[MAX_DBUSMESSAGE_LEN] = {0};
    int iRetLen = MAX_DBUSMESSAGE_LEN;
    BOOL bSend = LibDbus_SendSync(DBUS_MEDIAPARSE, 0x5007, (const char*)szMessage, strlen(szMessage) + 1, szRet, &iRetLen);
    if(FALSE == bSend)
    {
        return "0.000000&0.000000";
    }
    nlohmann::json json;
    BOOL bRet = Server::CJsonUtil::FromString(szRet, json);
    if(FALSE == bRet)
    {
        return "0.000000&0.000000";
    }
    if(0 == json["status"])
    {
        return "0.000000&0.000000";
    }
    string strGps = json["gps"];
    if(strGps.length() > 3)
    {
        return strGps;
    }
    return "0.000000&0.000000";
}
BOOL CDbusUtil::GetNetworkConfig(string& strEth, string& strWlan, string& strHotpot)
{
    if(FALSE == CDbusUtil::ContainModule(DBUS_NETWORK))
    {
        return FALSE;
    }
    char szMessage[MAX_DBUSMESSAGE_LEN] = {0};
    sprintf(szMessage, "{\"action\":\"getnetworkconfig\"}");
    char szRet[MAX_DBUSMESSAGE_LEN] = {0};
    int iRetLen = MAX_DBUSMESSAGE_LEN;
    BOOL bSend = LibDbus_SendSync(DBUS_NETWORK, 0x5007, (const char*)szMessage, strlen(szMessage) + 1, szRet, &iRetLen);
    if(FALSE == bSend)
    {
        return FALSE;
    }
    nlohmann::json json;
    BOOL bRet = Server::CJsonUtil::FromString(szRet, json);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    strEth = json["eth"];
    strWlan = json["wlan"];
    strHotpot = json["hotpot"];
    return TRUE;
}
BOOL CDbusUtil::BackupFoldSyncStart()
{
    if(FALSE == CDbusUtil::ContainModule(DBUS_MEDIAPARSE))
    {
        return TRUE;
    }
    nlohmann::json json;
    json["action"] = "backupfoldsyncstart";
    string strRequest = Server::CJsonUtil::ToString(json);
    char szRet[MAX_DBUSMESSAGE_LEN] = {0};
    int iRetLen = MAX_DBUSMESSAGE_LEN;
    BOOL bSend = LibDbus_SendSync(DBUS_MEDIAPARSE, 0x5007, (const char*)strRequest.c_str(), strRequest.length() + 1, szRet, &iRetLen);
    if(FALSE == bSend)
    {
        return FALSE;
    }
    nlohmann::json jsonRet;
    BOOL bRet = Server::CJsonUtil::FromString(szRet, jsonRet);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    if(0 == jsonRet["status"])
    {
        return FALSE;
    }
    return TRUE;
}
 int CDbusUtil::BackupFoldSyncPrecent(BOOL& bError)
 {
    bError = FALSE;
    if(FALSE == CDbusUtil::ContainModule(DBUS_MEDIAPARSE))
    {
        return 100;
    }
    nlohmann::json json;
    json["action"] = "backupfoldsyncprecent";
    string strRequest = Server::CJsonUtil::ToString(json);
    char szRet[MAX_DBUSMESSAGE_LEN] = {0};
    int iRetLen = MAX_DBUSMESSAGE_LEN;
    BOOL bSend = LibDbus_SendSync(DBUS_MEDIAPARSE, 0x5007, (const char*)strRequest.c_str(), strRequest.length() + 1, szRet, &iRetLen);
    if(FALSE == bSend)
    {
        bError = TRUE;
        return FALSE;
    }
    nlohmann::json jsonRet;
    BOOL bRet = Server::CJsonUtil::FromString(szRet, jsonRet);
    if(FALSE == bRet)
    {
        bError = TRUE;
        return FALSE;
    }
    if(0 == jsonRet["status"])
    {
        bError = TRUE;
        return FALSE;
    }
    int iPrecent = jsonRet["precent"];
    return iPrecent;
 }
 BOOL CDbusUtil::BackupUpdateGps(int iItemID, string strGps, string strAddr)
 {
    if(FALSE == CDbusUtil::ContainModule(DBUS_MEDIAPARSE))
    {
        return FALSE;
    }
    nlohmann::json json;
    json["action"] = "backupuploaditemaddrchange";
    json["itemid"] = iItemID;
    json["gps"] = strGps;
    json["addr"] = strAddr;

    string strRequest = Server::CJsonUtil::ToString(json);
    char szRet[MAX_DBUSMESSAGE_LEN] = {0};
    int iRetLen = MAX_DBUSMESSAGE_LEN;
    BOOL bSend = LibDbus_SendSync(DBUS_MEDIAPARSE, 0x5007, (const char*)strRequest.c_str(), strRequest.length() + 1, szRet, &iRetLen);
    if(FALSE == bSend)
    {
        return FALSE;
    }
    nlohmann::json jsonRet;
    BOOL bRet = Server::CJsonUtil::FromString(szRet, jsonRet);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    if(0 == jsonRet["status"])
    {
        return FALSE;
    }
    return TRUE;
 }