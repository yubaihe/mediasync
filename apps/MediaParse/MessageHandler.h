#include "stdafx.h"
#include <functional>
class CMessageHandler
{
public:
    CMessageHandler();
    ~CMessageHandler();
    void ServiceList(char* pszServiceList);
    void OnMessage(const char* pszMsg, char* pszRet);
private:
    string OnListDisk(nlohmann::json& jsonRoot);
    string OnDevEnter(nlohmann::json& jsonRoot);
    string OnDevSyncPrecent(nlohmann::json& jsonRoot);
    string OnParseGps(nlohmann::json& jsonRoot);
    string OnDiskMediaItemList(nlohmann::json& jsonRoot);
    string OnDiskAddIgnore(nlohmann::json& jsonRoot);
    string OnDiskRemoveIgnore(nlohmann::json& jsonRoot);
    string OnDiskMediaItemIgnoreList(nlohmann::json& jsonRoot);
    string OnDiskUploadItemList(nlohmann::json& jsonRoot);
    string OnDiskItemUpload(nlohmann::json& jsonRoot);
    string OnDiskItemUploadPrecent(nlohmann::json& jsonRoot);
    string OnDiskItemDetail(nlohmann::json& jsonRoot);
    string OnDiskDeleteItem(nlohmann::json& jsonRoot);
    string OnDiskSetIgnore(nlohmann::json& jsonRoot);
    string OnPrintItem(nlohmann::json& jsonRoot);
    string OnGenThumb(nlohmann::json& jsonRoot);
    string OnGetBackupFolds(nlohmann::json& jsonRoot);
    string OnGetBackupFoldAdd(nlohmann::json& jsonRoot);
    string OnBackupFoldDelete(nlohmann::json& jsonRoot);
    string OnBackupFoldModify(nlohmann::json& jsonRoot);
    string OnGetBackupFoldItemDetail(nlohmann::json& jsonRoot);
    string OnBackupUploadItem(nlohmann::json& jsonRoot);
    string OnBackupUploadItemPrecent(nlohmann::json& jsonRoot);
    string OnBackupUploadItemList(nlohmann::json& jsonRoot);
    string OnBackupUploadItemDel(nlohmann::json& jsonRoot);
    string OnBackupFoldSyncStart(nlohmann::json& jsonRoot);
    string OnBackupFoldSyncPrecent(nlohmann::json& jsonRoot);
    string OnBackupUploadItemAddrChange(nlohmann::json& jsonRoot);
    string OnSyncToDevice(nlohmann::json& jsonRoot);
    string OnSyncToDevicePrecent(nlohmann::json& jsonRoot);
    string OnDiskIgnoreCount(nlohmann::json& jsonRoot);
    string OnGetFileMime(nlohmann::json& jsonRoot);
    string OnTransFileStart(nlohmann::json& jsonRoot);
    string OnTransFileStop(nlohmann::json& jsonRoot);
    string OnTransFilePrecent(nlohmann::json& jsonRoot);
    string OnTransFileEnd(nlohmann::json& jsonRoot);
    
private:
    std::map<std::string, std::function<string(nlohmann::json&)>> m_ActionHandlerMap;
};


