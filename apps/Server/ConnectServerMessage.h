#pragma once
#include "stdafx.h"
typedef char* (*OnActionHandler)(nlohmann::json& jsonRoot);
class CConnectServerMessage
{
public:
    CConnectServerMessage();
    ~CConnectServerMessage();
    void OnMessage(const char* pszMsg, char* pszRet);
private:
    string OnDeviceInfo(nlohmann::json& jsonRoot);
    string OnDiskInfo(nlohmann::json& jsonRoot);
    string OnLogin(nlohmann::json& jsonRoot);
    string OnGetPwdTips(nlohmann::json& jsonRoot);
    string OnResetUser(nlohmann::json& jsonRoot);
    string OnUpdateDeviceInfo(nlohmann::json& jsonRoot);
    string OnServerItems(nlohmann::json& jsonRoot);
    string OnMediaYearList(nlohmann::json& jsonRoot);
    string OnMediaYearInfo(nlohmann::json& jsonRoot);
    string OnRecentInfo(nlohmann::json& jsonRoot);
    string OnFavoriteInfo(nlohmann::json& jsonRoot);
    string OnDevNames(nlohmann::json& jsonRoot);
    string OnMediaUpdateGpsWeiZhi(nlohmann::json& jsonRoot);
    string OnMediaYearLocationGroup(nlohmann::json& jsonRoot);
    string OnMediaYearLocationGroupTongJi(nlohmann::json& jsonRoot);
    string OnAboutDevices(nlohmann::json& jsonRoot);
    string OnUpdateDeviceTime(nlohmann::json& jsonRoot);
    string OnSetCover(nlohmann::json& jsonRoot);
    string OnMonthList(nlohmann::json& jsonRoot);
    string OnDayList(nlohmann::json& jsonRoot);
    string OnGetYearMonthDayCover(nlohmann::json& jsonRoot);
    string OnClearCacheStart(nlohmann::json& jsonRoot);
    string OnClearCacheProcess(nlohmann::json& jsonRoot);
    string OnGroupInfo(nlohmann::json& jsonRoot);
    string OnGroupDetail(nlohmann::json& jsonRoot);
    string OnGroupAdd(nlohmann::json& jsonRoot);
    string OnGroupUpdate(nlohmann::json& jsonRoot);
    string OnGroupDel(nlohmann::json& jsonRoot);
    string OnMediaItemGroupSetting(nlohmann::json& jsonRoot);
    string OnGroupNamesFromItemID(nlohmann::json& jsonRoot);
    string OnMediaItemsGroupAdd(nlohmann::json& jsonRoot);
    string OnListDisk(nlohmann::json& jsonRoot);
    string OnStore(nlohmann::json& jsonRoot);
    string OnUploadFile(nlohmann::json& jsonRoot);
    string OnCheckMd5(nlohmann::json& jsonRoot);
    string OnReportInfo(nlohmann::json& jsonRoot);
    string OnItemDetail(nlohmann::json& jsonRoot);
    string OnMediaItemFavorite(nlohmann::json& jsonRoot);
    string OnDelMediaItem(nlohmann::json& jsonRoot);
    string OnBroadCastOnline(nlohmann::json& jsonRoot);
    string OnUpdatePaiSheTime(nlohmann::json& jsonRoot);
    string OnUpdateGpsAddr(nlohmann::json& jsonRoot);
    string OnGroupNameFromID(nlohmann::json& jsonRoot);
    string OnCheckFile(nlohmann::json& jsonRoot);
    string OnGpsLocation(nlohmann::json& jsonRoot);
    string OnSetBaiDuAk(nlohmann::json& jsonRoot);
    string OnSetMediaPinned(nlohmann::json& jsonRoot);
    // string OnSetGpsParse(nlohmann::json& jsonRoot);
private:
    std::map<std::string, std::function<string(nlohmann::json&)>> m_ActionHandlerMap;
};


