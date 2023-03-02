#include "stdafx.h"
#include <syslog.h>
#include "MessageHandler.h"
#include "SocketClient.h"

char* OnGetDeviceInfo(json_object* pJsonRoot)
{
    char szBuffer[255] = {0};
    sprintf(szBuffer, "{\"otype\":%d}", MESSAGETYPECMD_DEVICEINFO);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, szBuffer);
}

char* OnGetDiskInfo(json_object* pJsonRoot)
{
    char szBuffer[255] = {0};
    sprintf(szBuffer, "{\"otype\":%d}", MESSAGETYPECMD_DISKINFO);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, szBuffer);
}

char* OnLogin(json_object* pJsonRoot)
{
    char szBuffer[255] = {0};
    const char* pPwd = json_object_get_string(json_object_object_get(pJsonRoot,"pwd"));
    sprintf(szBuffer, "{\"otype\":%d,\"pwd\":\"%s\"}", MESSAGETYPECMD_LOGIN, pPwd);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, szBuffer);
}

char* DealResponse(char* pBuffer)
{
    json_object* pJsonRoot = json_tokener_parse(pBuffer);
    json_object_object_del(pJsonRoot, "otype");
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    int iJsonBufferLen = strlen(pJsonTmpBuffer);
    char* pJsonBuffer = (char*)malloc(iJsonBufferLen + 1);
    memset(pJsonBuffer, 0, iJsonBufferLen + 1);
    strcpy(pJsonBuffer, pJsonTmpBuffer);
    json_object_put(pJsonRoot);
    free(pBuffer);
    pBuffer = NULL;
    return pJsonBuffer;
}

char* OnGetPwdTips(json_object* pJsonRoot)
{
    char szBuffer[255] = {0};
    sprintf(szBuffer, "{\"otype\":%d}", MESSAGETYPECMD_PWDTIP);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, szBuffer);
}

char* OnResetUser(json_object* pJsonRoot)
{
    const char* pszPwd = json_object_get_string(json_object_object_get(pJsonRoot, "pwd"));
	const char* pszPwdTip = json_object_get_string(json_object_object_get(pJsonRoot, "pwdtip"));
    char szBuffer[1024] = {0};
    sprintf(szBuffer, "{\"otype\":%d,\"pwd\":\"%s\",\"pwdtip\":\"%s\"}", MESSAGETYPECMD_RESETUSER, pszPwd, pszPwdTip);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, szBuffer);
}

char* OnUpdateDeviceInfo(json_object* pJsonRoot)
{
    const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devname"));//设备名称
	const char* pszDeviceBlue = json_object_get_string(json_object_object_get(pJsonRoot, "devblue"));//模糊度
    char szBuffer[1024] = {0};
    sprintf(szBuffer, "{\"otype\":%d,\"devname\":\"%s\",\"devblue\":\"%s\"}", MESSAGETYPECMD_UPDATEDEVICEINFO, pszDeviceName, pszDeviceBlue);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, szBuffer);
}

char* OnRecentUpload(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_SERVERITEMS));
    json_object_object_add(pJsonRoot, "type", json_object_new_int(0));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnFavoriteList(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_SERVERITEMS));
    json_object_object_add(pJsonRoot, "type", json_object_new_int(1));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char*  OnMediaGroupItemList(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_SERVERITEMS));
    json_object_object_add(pJsonRoot, "type", json_object_new_int(3));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnYearItemList(json_object* pJsonRoot)
{
    //type : 0最近  1收藏 2年份 3:分组信息
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_SERVERITEMS));
    json_object_object_add(pJsonRoot, "type", json_object_new_int(2));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnYearList(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAYEARLIST));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnDayList(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIADAYSLIST));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnYearMonthDayCover(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAYEARMONTHDAYCOVER));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnMonthList(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAMONTHLIST));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnYearInfo(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAYEARINFO));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnRecentInfo(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIARECENTINFO));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnFavoriteInfo(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAFAVORITEINFO));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnDevNames(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIADEVNAMES));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnUnCheckGps(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAUNCHECKGPS));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnUpdateGpsLocation(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIANUPDATEGPSWEIZHI));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnYearLocationGroup(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_YEARLOCATIONGROUP));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnYearLocationGroupTongJi(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_YEARLOCATIONGROUPTONGJI));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnAboutDevice(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_ABOUTDEVICE));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnUpdateTime(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_UPDATEDEVICETIME));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnGetBrType(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GETBRTYPE));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnSetBrType(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_SETBRTYPE));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnGetWifiItems(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GETWIFIITEMS));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnGetStoreWifiItems(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GETSTOREWIFIITEMS));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnGetHotPot(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GETHOTPOT));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnSetHotPot(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_SETHOTPOT));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnConnectWifiItem(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_CONNECTWIFIITEM));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnGetWifiStatus(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GETWIFISTATUS));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnUnconnectWifiItem(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_UNCONNECTWIFIITEM));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}
char* OnDelStoreWifiItem(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_DELSTOREWIFIITEM));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnSetCover(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_SETCOVER));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnClearCacheStart(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIACLEARCACHESTART));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnClearCacheProcess(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIACLEARCACHEPROCESS));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnGetGroupInfo(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GROUPINFO));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnGetGroupDetail(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GROUPITEMDETAIL));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}


char* OnGetGroupAdd(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GROUPADD));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnGetGroupDel(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GROUPDEL));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnGetGroupUpdate(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GROUPUPDATE));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char*  OnMediaGroupItemSetting(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAITEMGROUPSETTING));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char*  OnDelMediaItem(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_MEDIAITEMDEL));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

char* OnGroupNameFromItemID(json_object* pJsonRoot)
{
    json_object_object_del(pJsonRoot, "action");
    json_object_object_add(pJsonRoot, "otype", json_object_new_int(MESSAGETYPECMD_GROUPNAMESFROMITEMID));
    const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
    return SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
}

void OnMessage(RelechMap* pRelechMap, char* pMsg, char** pRet)
{
    json_object* pJsonRoot = json_tokener_parse(pMsg);
	const char* pstrAction = json_object_get_string(json_object_object_get(pJsonRoot,"action"));
    if(NULL == pstrAction)
    {
        const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
        *pRet = SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
    }
    else
    {
        OnActionHandler* pHandler = (OnActionHandler*)RelechMap_LookUp(pRelechMap, pstrAction);
        if(NULL != pHandler)
        {
            char* pRetBuffer = (*pHandler)(pJsonRoot);
            *pRet = DealResponse(pRetBuffer);
        }
        else
        {
            const char* pJsonTmpBuffer = json_object_to_json_string(pJsonRoot);
            *pRet = SocketClient_SendMessage(SERVERIP, SERVERPORT, pJsonTmpBuffer);
        }
    }
    json_object_put(pJsonRoot);
}
void MessageHandler_AddController(RelechMap* pRelechMap, char* pszAction, OnActionHandler handler)
{
    OnActionHandler* pHandler = RelechMap_LookUp(pRelechMap, pszAction);
    if(NULL != pHandler)
    {
        RelechMap_Earse(pRelechMap, pszAction);
    }
    RelechMap_SetAt(pRelechMap, pszAction, &handler, sizeof(OnActionHandler*));
}

MessageHandler* MessageHandler_Init()
{
    int iLen = sizeof(MessageHandler);
    char* pszBuffer = malloc(iLen);
    memset(pszBuffer, 0, iLen);
    MessageHandler* pMessageHandler = (MessageHandler*)pszBuffer;
    pMessageHandler->pHanlerMap = RelechMap_Init(100);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "deviceinfo", OnGetDeviceInfo);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "diskinfo", OnGetDiskInfo);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "login", OnLogin);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "pwdtip", OnGetPwdTips);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "resetuser", OnResetUser);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "updatedeviceinfo", OnUpdateDeviceInfo);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "recentupload", OnRecentUpload);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "favorite", OnFavoriteList);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "yearitemlist", OnYearItemList);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "yearlist", OnYearList);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "yearinfo", OnYearInfo);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "recentinfo", OnRecentInfo);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "favoriteinfo", OnFavoriteInfo);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "devnames", OnDevNames);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "uncheckgps", OnUnCheckGps);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "updategpslocation", OnUpdateGpsLocation);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "yearlocationgroup", OnYearLocationGroup);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "yearlocationgrouptongji", OnYearLocationGroupTongJi);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "aboutdevice", OnAboutDevice);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "updatetime", OnUpdateTime);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "getbrtype", OnGetBrType);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "setbrtype", OnSetBrType);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "getwifiitems", OnGetWifiItems);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "getstorewifiitems", OnGetStoreWifiItems);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "gethotpot", OnGetHotPot);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "sethotpot", OnSetHotPot);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "connectwifiitem", OnConnectWifiItem);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "getwifistatus", OnGetWifiStatus);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "unconnectwifiitem", OnUnconnectWifiItem);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "delstorewifiitem", OnDelStoreWifiItem);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "setcover", OnSetCover);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "monthlist", OnMonthList);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "daylist", OnDayList);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "yearmonthdaycover", OnYearMonthDayCover);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "clearcachestart", OnClearCacheStart);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "clearcacheprocess", OnClearCacheProcess);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "groupinfo", OnGetGroupInfo);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "groupdetail", OnGetGroupDetail);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "groupadd", OnGetGroupAdd);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "groupupdate", OnGetGroupUpdate);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "groupdel", OnGetGroupDel);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "mediaitemgroupsetting", OnMediaGroupItemSetting);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "groupitemlist", OnMediaGroupItemList);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "delmediaitem", OnDelMediaItem);
    MessageHandler_AddController(pMessageHandler->pHanlerMap, "groupnamesfromitemid", OnGroupNameFromItemID);

    return pMessageHandler;
}

void MessageHandler_UnInit(MessageHandler* pMessageHandler)
{
    RelechMap_ClearMap(pMessageHandler->pHanlerMap);
    free((char*)pMessageHandler);
    pMessageHandler = NULL;
}