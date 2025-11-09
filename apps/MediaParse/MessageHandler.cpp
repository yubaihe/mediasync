#include "MessageHandler.h"
#include "Dbus/libdbus.h"
#include "DiskManager.h"
#include "./Util/CommonUtil.h"
#include "./Util/JsonUtil.h"
#include "./Backup/BackupManager.h"
#include "./Backup/BackupTmpFoldMonitor.h"
#include "PhotoManager.h"
#include "Backup/BackupOrganize.h"
#include "SyncToDeviceManager.h"
#include "TranscodeManager.h"
extern string g_strMountPoint;

//进入设备
//curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"deventer\",\"dev\":\"sda4\"}" http://192.168.110.12:8080/mediaparse.fcgi
//查询进入设备进度
//curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"devsyncprecent\",\"dev\":\"sda4\"}" http://192.168.110.12:8080/mediaparse.fcgi
//上传文件
//curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"diskitemupload\",\"dev\":\"sda4\",\"itemid\":0}" http://192.168.110.12:8080/mediaparse.fcgi 
//上传文件状态
//curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"diskitemuploadprecent\",\"dev\":\"sda4\"}" http://192.168.110.12:8080/mediaparse.fcgi 



//curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"backupfoldsyncstart\"}" http://192.168.110.4:8080/mediaparse.fcgi 
CMessageHandler::CMessageHandler()
{
}

CMessageHandler::~CMessageHandler()
{
}
string CMessageHandler::OnListDisk(nlohmann::json& jsonRoot)
{
    if(0 == g_strMountPoint.length())
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["items"] = nlohmann::json::array();
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    std::string strDevList = CDiskManager::GetInstance()->ListDev();
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    if(strDevList.size() == 0)
    {
        jsonRet["items"] = nlohmann::json::array();
    }
    else
    {
        nlohmann::json jsonItems;
        MediaParse::CJsonUtil::FromString(strDevList, jsonItems);
        jsonRet["items"] = jsonItems;
    }
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDevEnter(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    BOOL bEnter = CDiskManager::GetInstance()->EnterDev(strDevName.c_str());
    int iPrecent = 0;
    if(TRUE == bEnter)
    {
        iPrecent = CDiskManager::GetInstance()->GetPrecentDev(strDevName.c_str());
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = bEnter;
    jsonRet["precent"] = iPrecent;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDevSyncPrecent(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iPrecent = CDiskManager::GetInstance()->GetPrecentDev(strDevName.c_str());
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["precent"] = iPrecent;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnParseGps(nlohmann::json& jsonRoot)
{
    std::string strFile = jsonRoot["file"];
    if(FALSE == CCommonUtil::CheckFileExist(strFile.c_str()))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    CImageParse imageParse;
    imageParse.Parse(strFile.c_str());
    if(imageParse.GetLatitude() != 0 && imageParse.GetLongtitude() != 0)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["gps"] = CCommonUtil::StringFormat("%f&%f", imageParse.GetLatitude(), imageParse.GetLongtitude());
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    CVideoParse videoParse;
    videoParse.Parse(strFile.c_str());
    if(videoParse.GetLatitude() != 0 && videoParse.GetLongtitude() != 0)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["gps"] = CCommonUtil::StringFormat("%f&%f", videoParse.GetLatitude(), videoParse.GetLongtitude());
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = 0;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskMediaItemList(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iStart = jsonRoot["start"];
    int iLimit = jsonRoot["limit"];

    list<string> itemList = CDiskManager::GetInstance()->MediaItems(strDevName.c_str(), iStart, iLimit);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["items"] = nlohmann::json::array();
    list<string>::iterator itor = itemList.begin();
    for(; itor != itemList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        MediaParse::CJsonUtil::FromString(*itor, jsonItem);
        jsonRet["items"].push_back(jsonItem);
    }
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskIgnoreCount(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iIgnoreCount = CDiskManager::GetInstance()->GetIgnoreCount(strDevName.c_str());
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["ignorecount"] = iIgnoreCount;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskAddIgnore(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    std::string strItemIds = jsonRoot["items"];
    CDiskManager::GetInstance()->SetItemsIgnore(strDevName.c_str(), strItemIds.c_str());
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskRemoveIgnore(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    std::string strItemIds = jsonRoot["items"];
    CDiskManager::GetInstance()->RemoveIgnoreItem(strDevName.c_str(), strItemIds.c_str());
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskMediaItemIgnoreList(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iStart = jsonRoot["start"];
    int iLimit = jsonRoot["limit"];
    list<string> itemList = CDiskManager::GetInstance()->MediaIgnoreItems(strDevName.c_str(), iStart, iLimit);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["items"] = nlohmann::json::array();
    list<string>::iterator itor = itemList.begin();
    for(; itor != itemList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        MediaParse::CJsonUtil::FromString(*itor, jsonItem);
        jsonRet["items"].push_back(jsonItem);
    }
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskUploadItemList(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iStart = jsonRoot["start"];
    int iLimit = jsonRoot["limit"];
    list<string> itemList = CDiskManager::GetInstance()->MediaUploadItems(strDevName.c_str(), iStart, iLimit);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["items"] = nlohmann::json::array();
    list<string>::iterator itor = itemList.begin();
    for(; itor != itemList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        MediaParse::CJsonUtil::FromString(*itor, jsonItem);
        jsonRet["items"].push_back(jsonItem);
    }
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskItemUpload(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iItemID = jsonRoot["itemid"];
    BOOL bSuccess = CDiskManager::GetInstance()->UploadItem(strDevName.c_str(), iItemID);
    nlohmann::json jsonRet;
    jsonRet["status"] = bSuccess;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskItemUploadPrecent(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iPrecent = 0;
    char szErrorInfo[2000] = {0};
    int iItemID = 0;
    CDiskManager::GetInstance()->GetUploadInfo(strDevName.c_str(), &iItemID, &iPrecent, szErrorInfo);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["itemid"] = iItemID;
    jsonRet["precent"] = iPrecent;
    jsonRet["error"] = szErrorInfo;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskItemDetail(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iItemID = jsonRoot["itemid"];
    BOOL ignore = jsonRoot["ignore"];
    string strDetail = CDiskManager::GetInstance()->GetDiskItemDetail(strDevName.c_str(), iItemID, ignore);

    nlohmann::json jsonRet;
    MediaParse::CJsonUtil::FromString(strDetail, jsonRet);
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskDeleteItem(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iItemID = jsonRoot["itemid"];
    printf("dev:%s itemid:%d\n", strDevName.c_str(), iItemID);
    CDiskManager::GetInstance()->DeleteItem(strDevName.c_str(), iItemID);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnDiskSetIgnore(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iItemID = jsonRoot["itemid"];
    int iIgnore = jsonRoot["ignore"];
    printf("dev:%s itemid:%d ignore:%d\n", strDevName.c_str(), iItemID, iIgnore);

    if(TRUE == iIgnore)
    {
        CDiskManager::GetInstance()->AddIgnoreItem(strDevName.c_str(), iItemID);
    }
    else
    {
        CDiskManager::GetInstance()->RemoveIgnoreItem(strDevName.c_str(), iItemID);
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnPrintItem(nlohmann::json& jsonRoot)
{
    std::string strDevName = jsonRoot["dev"];
    int iItemID = jsonRoot["itemid"];
    CDiskManager::GetInstance()->PrintItem(strDevName.c_str(), iItemID);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnGenThumb(nlohmann::json& jsonRoot)
{
    int iMediaType = jsonRoot["mediatype"];
    string strFile = jsonRoot["file"];
    string strThumb = jsonRoot["thumb"];
    BOOL bSuccess = FALSE;
    if(MEDIATYPE_IMAGE == iMediaType)
	{
		CImageParse imageParse;
        bSuccess = imageParse.GetThumbnail(strFile.c_str(), strThumb.c_str(), PIC_WIDTH, PIC_HEIGHT);
	}
	else
	{
		CVideoParse videoParse;
        bSuccess = videoParse.GetThumbnail(strFile.c_str(), strThumb.c_str(), PIC_WIDTH, PIC_HEIGHT);
	}
    nlohmann::json jsonRet;
    jsonRet["status"] = bSuccess;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnGetBackupFolds(nlohmann::json& jsonRoot)
{
    int iStart = jsonRoot["start"];
    int iLimit = jsonRoot["limit"];
    int iScanFoldCount = 0;
    std::vector<string> vecRet = CBackupManager::GetInstance()->BackupFoldList(iStart, iLimit, &iScanFoldCount); //扫描出来的目录
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    if(0 == iStart)
    {
        jsonRet["scantotal"] = iScanFoldCount;
        jsonRet["storetotal"] = CBackupManager::GetInstance()->GetStoreFoldList().size();
    }
    else
    {
        jsonRet["total"] = 0;
    }
    jsonRet["items"] = nlohmann::json::array();
    vector<string>::iterator itor = vecRet.begin();
    for(; itor != vecRet.end(); ++itor)
    {
        int iVideoCount = 0;
        int iPicCount = 0;
        std::vector<BackupItem> vec = CBackupManager::GetInstance()->BackupFileList(*itor, &iPicCount, &iVideoCount);
        if(vec.size() == 0)
        {
            string strRoot = CBackupManager::GetInstance()->GetBackupRoot(*itor);
            CBackupFlod fold;
            std::vector<FileEntry> foldVec = fold.AllFilesVec(strRoot);
            for(size_t i = 0; i < foldVec.size(); ++i)
            {
                BackupItem item = {};
                item.strFile = foldVec[i].strName;
                item.iCreateTimeSec = foldVec[i].iCreateTimeSec;
                vec.push_back(item);
            }
        }
        nlohmann::json jsonItem;
        jsonItem["name"] = *itor;
        jsonItem["fcount"] = vec.size();
        jsonItem["piccount"] = iPicCount;
        jsonItem["videocount"] = iVideoCount;
        jsonItem["cover"] = "";
        if(vec.size() > 0)
        {
            string strThumb = CBackupManager::GetInstance()->GetBackupThumbRoot(*itor);
            for(size_t i = 0; i < vec.size(); ++i)
            {
                string strThumbFile = strThumb;
                CBackupFlod fold;
                strThumbFile.append(fold.GetThumbFile(vec[i].eMediaType, vec[i].strFile));
                if(TRUE == CCommonUtil::CheckFileExist(strThumbFile.c_str()))
                {
                    jsonItem["cover"] = vec[i].strFile;
                    break;
                }
            }
        }
        jsonRet["items"].push_back(jsonItem);
    }
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnGetBackupFoldAdd(nlohmann::json& jsonRoot)
{
    //判断目录是否存在
    string strName = jsonRoot["name"];
    string strFullName = g_strBackupPoint;
    strFullName.append(strName);
    BOOL bExist = CCommonUtil::CheckFoldExist(strFullName.c_str());
    if(FALSE == bExist)
    {
        CCommonUtil::CreateFold(strFullName.c_str());
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["exist"] = bExist;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupFoldDelete(nlohmann::json& jsonRoot)
{
    //这里需要优化 relech
    //全部移动到 /home/relech/mediasync/backup/.media_tmp/ 以秒为文件名
    //启动定时任务删除里面的目录
    string strName = jsonRoot["name"];
    string strBackupRoot = CBackupManager::GetInstance()->GetBackupRoot(strName);
    string strBackupThumbRoot = CBackupManager::GetInstance()->GetBackupThumbRoot(strName);

    string strTmpRoot = CBackupManager::GetInstance()->GetBackupTempRoot(CCommonUtil::StringFormat("%ld", CCommonUtil::CurTimeMilSec()));
    CCommonUtil::CreateFold(strTmpRoot.c_str());
    printf("Create backup temp fold:%s\n", strTmpRoot.c_str());
    string strDestFold = CCommonUtil::StringFormat("%s%s", strTmpRoot.c_str(), strName.c_str());
    string strDestThumbFold = CCommonUtil::StringFormat("%s.%s", strTmpRoot.c_str(), strName.c_str());
    CCommonUtil::MoveFile(strBackupRoot, strDestFold);
    CCommonUtil::MoveFile(strBackupThumbRoot, strDestThumbFold);

    CBackupTmpFoldMonitor::GetInstance()->SetImmediately();

    CBackupTable backupTable;
    backupTable.DeleteFold(strName);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupFoldModify(nlohmann::json& jsonRoot)
{
    string strFromName = jsonRoot["fname"];
    string strToName = jsonRoot["tname"];
    if(0 == strFromName.compare(strToName))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["code"] = 0;
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    string strFromBackRoot = CBackupManager::GetInstance()->GetBackupRoot(strFromName);
    string strToBackRoot = CBackupManager::GetInstance()->GetBackupRoot(strToName);
    printf("From:%s\n", strFromBackRoot.c_str());
    printf("To:%s\n", strToBackRoot.c_str());
    if(FALSE == CCommonUtil::CheckFoldExist(strFromBackRoot.c_str()))
    {
        //源文件夹不存在
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["code"] = 1;
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    if(TRUE == CCommonUtil::CheckFoldExist(strToBackRoot.c_str()))
    {
        //目的文件夹已经存在
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["code"] = 2;
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    BOOL bRet = CCommonUtil::MoveFile(strFromBackRoot, strToBackRoot);
    if(FALSE == bRet)
    {
        //移动源文件错误
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["code"] = 3;
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    string strFromThumbBackRoot = CBackupManager::GetInstance()->GetBackupThumbRoot(strFromName);
    string strToThumbBackRoot = CBackupManager::GetInstance()->GetBackupThumbRoot(strToName);
    printf("From:%s\n", strFromThumbBackRoot.c_str());
    printf("To:%s\n", strToThumbBackRoot.c_str());
    if(TRUE == CCommonUtil::CheckFoldExist(strFromThumbBackRoot.c_str()))
    {
        bRet = CCommonUtil::MoveFile(strFromThumbBackRoot, strToThumbBackRoot);
        if(FALSE == bRet)
        {
            //移动源文件错误
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            jsonRet["code"] = 4;
            return MediaParse::CJsonUtil::ToString(jsonRet);
        }
    }
    CBackupTable table;
    bRet = table.UpdateFoldName(strFromName, strToName);
    if(FALSE == bRet)
    {
        //入库失败了
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["code"] = 5;
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["code"] = 0;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnGetBackupFoldItemDetail(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["itemid"];
    string strDetail = CBackupManager::GetInstance()->GetBackupFoldItemDetail(iItemID);

    nlohmann::json jsonRet;
    if(strDetail.length() > 0)
    {
        MediaParse::CJsonUtil::FromString(strDetail, jsonRet);
    }
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupUploadItemAddrChange(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["itemid"];
    string strGps = jsonRoot["gps"];
    string strAddr = jsonRoot["addr"];
    CBackupTable table;
    BOOL bRet = table.UpdateItemGpsAddr(iItemID, strGps, strAddr);
     nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupUploadItem(nlohmann::json& jsonRoot)
{
    string strDevName = jsonRoot["dev"];
    int iItemID = jsonRoot["itemid"];
    string strDestFold = jsonRoot["destfold"];
    string strSrcFile = CDiskManager::GetInstance()->GetOriginalFile(strDevName.c_str(), iItemID);
    string strSrcThumbFile = CDiskManager::GetInstance()->GetThumbFile(strDevName.c_str(), iItemID);
    printf("Srcfile:%s\n", strSrcFile.c_str());
    printf("SrcThumbFile:%s\n", strSrcThumbFile.c_str());
    if(FALSE == CCommonUtil::CheckFileExist(strSrcFile.c_str()))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "src file not exist";
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    if(FALSE == CCommonUtil::CheckFileExist(strSrcThumbFile.c_str()))
    {
       CPhotoManager::GenThumbPic(strSrcFile, strSrcThumbFile);
    }
    if(FALSE == CCommonUtil::CheckFileExist(strSrcThumbFile.c_str()))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "gen thumb file failed";
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    string strToken = "";
    CBackupManager::GetInstance()->UploadItem(strSrcFile, strSrcThumbFile, strDestFold, strToken);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["token"] = strToken;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupUploadItemPrecent(nlohmann::json& jsonRoot)
{
    string strtoken = jsonRoot["token"];
    char szErrorInfo[2000] = {0};
    int iPrecent = CBackupManager::GetInstance()->GetUploadPrecent(strtoken, szErrorInfo);

    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["token"] = strtoken;
    jsonRet["precent"] = iPrecent;
    jsonRet["error"] = szErrorInfo;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupUploadItemList(nlohmann::json& jsonRoot)
{
    std::string strName = jsonRoot["name"];
    int iStart = jsonRoot["start"];
    int iLimited = jsonRoot["limit"];
    CBackupTable table;
    std::list<BackupItemFull> itemList = table.BackupFileList(strName, iStart, iLimited);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["items"] = nlohmann::json::array();
    list<BackupItemFull>::iterator itor = itemList.begin();
    for(; itor != itemList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        BackupItemFull item = *itor;
        if(item.strMd5.length() == 0)
        {
            continue;
        }
        jsonItem["dur"] = item.iDuration;
        jsonItem["id"] = item.iID;
        jsonItem["h"] = item.iHeight;
        jsonItem["w"] = item.iWidth;
        jsonItem["maddr"] = item.strFile;
        jsonItem["msize"] = item.iMeiTiSize;
        jsonItem["mtype"] = item.eMediaType;
        jsonItem["hasextra"] = 0;
        jsonRet["items"].push_back(jsonItem);
    }
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupUploadItemDel(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["itemid"];
    CBackupFlod fold;
    BOOL bRet = fold.RemoveItem(iItemID);
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupFoldSyncStart(nlohmann::json& jsonRoot)
{
    BOOL bRet = CBackupOrganize::GetInstance()->Start();
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnBackupFoldSyncPrecent(nlohmann::json& jsonRoot)
{
    int iPrecent = CBackupOrganize::GetInstance()->GetProcess();
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["precent"] = iPrecent;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}

string CMessageHandler::OnSyncToDevice(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["itemid"];
    BOOL bRet = CSyncToDeviceManager::GetInstance()->AddToSync(iItemID);
    if(FALSE == bRet)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["itemid"] = iItemID;
        jsonRet["precent"] = 0;
        jsonRet["error"] = "No find from database";
        return MediaParse::CJsonUtil::ToString(jsonRet);
    }
    char szErrorInfo[2000] = {0};
    int iPrecent = 0;
    CSyncToDeviceManager::GetInstance()->GetSyncInfo(iItemID, &iPrecent, szErrorInfo);

    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["itemid"] = iItemID;
    jsonRet["precent"] = iPrecent;
    jsonRet["error"] = szErrorInfo;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnSyncToDevicePrecent(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["itemid"];
    char szErrorInfo[2000] = {0};
    int iPrecent = 0;
    CSyncToDeviceManager::GetInstance()->GetSyncInfo(iItemID, &iPrecent, szErrorInfo);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["itemid"] = iItemID;
    jsonRet["precent"] = iPrecent;
    jsonRet["error"] = szErrorInfo;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
// /home/relech/mediasync/bin/curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"transfilestart\",\"file\":\"/home/relech/Linux/test.mkv\",\"dest\":\"/home/relech/Linux/test1.mp4\"}" http://127.0.0.1:8080/mediaparse.fcgi
// /home/relech/mediasync/bin/curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"transfilestop\",\"identify\":\"ZWI4MDY1M2I4YTgxNDg4ZDU5MTRkOTdkMTM1ZGYzODE=\"}" http://127.0.0.1:8080/mediaparse.fcgi
// /home/relech/mediasync/bin/curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"transfileprecent\",\"identify\":\"ZWI4MDY1M2I4YTgxNDg4ZDU5MTRkOTdkMTM1ZGYzODE=\"}" http://127.0.0.1:8080/mediaparse.fcgi
// /home/relech/mediasync/bin/curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"getfilemime\",\"file\":\"/home/relech/Linux/test.mkv\"}" http://127.0.0.1:8080/mediaparse.fcgi
string CMessageHandler::OnGetFileMime(nlohmann::json& jsonRoot)
{
    std::string strFile = jsonRoot["file"];
    string strMime = CCommonUtil::GetMime(strFile.c_str());
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["mime"] = strMime;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnTransFileStart(nlohmann::json& jsonRoot)
{
    std::string strFile = jsonRoot["file"];
    std::string strDest = jsonRoot["dest"];
    int iItemID = jsonRoot["itemid"];
    time_t iDurSec = CTranscodeManager::GetVideoDurationSec(strFile);
    string strMd5 = CTranscodeManager::GetInstance()->TransFile(strFile, strDest, iDurSec, iItemID);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["identify"] = strMd5;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnTransFileStop(nlohmann::json& jsonRoot)
{
    std::string strIdentify = jsonRoot["identify"];
    CTranscodeManager::GetInstance()->TerminalTrans(strIdentify);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnTransFilePrecent(nlohmann::json& jsonRoot)
{
    std::string strIdentify = jsonRoot["identify"];
    DWORD iDurSec = 0;
    DWORD iCurSec = 0;
    int iPrecent = 0;
    CTranscodeManager::GetInstance()->GetTransInfo(strIdentify, iDurSec, iCurSec, iPrecent);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["dur"] = iDurSec;
    jsonRet["cur"] = iCurSec;
    jsonRet["precent"] = iPrecent;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
string CMessageHandler::OnTransFileEnd(nlohmann::json& jsonRoot)
{
    std::string strIdentify = jsonRoot["identify"];
    CTranscodeManager::GetInstance()->TransEnd(strIdentify);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return MediaParse::CJsonUtil::ToString(jsonRet);
}
void CMessageHandler::OnMessage(const char* pszMsg, char* pszRet)
{
    // try 
    // {

        // printf("%s\n", pszMsg);
        nlohmann::json jsonValue;
        BOOL bRet = MediaParse::CJsonUtil::FromString(pszMsg, jsonValue);
        if(FALSE == bRet)
        {
            strcpy(pszRet, "{\"status\":0,\"info\":\"input error\"}");
            return;
        }
        string strAction = jsonValue["action"];
        if(m_ActionHandlerMap.size() == 0)
        {
            m_ActionHandlerMap["listdisk"] = std::bind(&CMessageHandler::OnListDisk, this, std::placeholders::_1);
            m_ActionHandlerMap["deventer"] = std::bind(&CMessageHandler::OnDevEnter, this, std::placeholders::_1);
            m_ActionHandlerMap["devsyncprecent"] = std::bind(&CMessageHandler::OnDevSyncPrecent, this, std::placeholders::_1);
            m_ActionHandlerMap["parsegps"] = std::bind(&CMessageHandler::OnParseGps, this, std::placeholders::_1);
            m_ActionHandlerMap["diskmediaitemlist"] = std::bind(&CMessageHandler::OnDiskMediaItemList, this, std::placeholders::_1);
            m_ActionHandlerMap["diskaddignore"] = std::bind(&CMessageHandler::OnDiskAddIgnore, this, std::placeholders::_1);
            m_ActionHandlerMap["diskremoveignore"] = std::bind(&CMessageHandler::OnDiskRemoveIgnore, this, std::placeholders::_1);
            m_ActionHandlerMap["diskmediaitemignorelist"] = std::bind(&CMessageHandler::OnDiskMediaItemIgnoreList, this, std::placeholders::_1);
            m_ActionHandlerMap["diskuploaditemlist"] = std::bind(&CMessageHandler::OnDiskUploadItemList, this, std::placeholders::_1);
            m_ActionHandlerMap["diskitemupload"] = std::bind(&CMessageHandler::OnDiskItemUpload, this, std::placeholders::_1);
            m_ActionHandlerMap["diskitemuploadprecent"] = std::bind(&CMessageHandler::OnDiskItemUploadPrecent, this, std::placeholders::_1);
            m_ActionHandlerMap["diskitemdetail"] = std::bind(&CMessageHandler::OnDiskItemDetail, this, std::placeholders::_1);
            m_ActionHandlerMap["diskdeleteitem"] = std::bind(&CMessageHandler::OnDiskDeleteItem, this, std::placeholders::_1);
            m_ActionHandlerMap["disksetignore"] = std::bind(&CMessageHandler::OnDiskSetIgnore, this, std::placeholders::_1);
            m_ActionHandlerMap["diskignorecount"] = std::bind(&CMessageHandler::OnDiskIgnoreCount, this, std::placeholders::_1);
            m_ActionHandlerMap["printitem"] = std::bind(&CMessageHandler::OnPrintItem, this, std::placeholders::_1);
            m_ActionHandlerMap["genthumb"] = std::bind(&CMessageHandler::OnGenThumb, this, std::placeholders::_1);
            //备份
            m_ActionHandlerMap["backupfolds"] = std::bind(&CMessageHandler::OnGetBackupFolds, this, std::placeholders::_1);
            m_ActionHandlerMap["backupfoldadd"] = std::bind(&CMessageHandler::OnGetBackupFoldAdd, this, std::placeholders::_1);
            m_ActionHandlerMap["backupfolddel"] = std::bind(&CMessageHandler::OnBackupFoldDelete, this, std::placeholders::_1);
            m_ActionHandlerMap["backupfoldmodify"] = std::bind(&CMessageHandler::OnBackupFoldModify, this, std::placeholders::_1);
            //提交
            m_ActionHandlerMap["backupuploaditem"] = std::bind(&CMessageHandler::OnBackupUploadItem, this, std::placeholders::_1);
            m_ActionHandlerMap["backupuploaditemaddrchange"] = std::bind(&CMessageHandler::OnBackupUploadItemAddrChange, this, std::placeholders::_1);
            m_ActionHandlerMap["backupuploaditemprecent"] = std::bind(&CMessageHandler::OnBackupUploadItemPrecent, this, std::placeholders::_1);
            m_ActionHandlerMap["backupuploaditemlist"] = std::bind(&CMessageHandler::OnBackupUploadItemList, this, std::placeholders::_1);
            m_ActionHandlerMap["backupuploaditemdel"] = std::bind(&CMessageHandler::OnBackupUploadItemDel, this, std::placeholders::_1);
            //文件
            m_ActionHandlerMap["backupfolditemdetail"] = std::bind(&CMessageHandler::OnGetBackupFoldItemDetail, this, std::placeholders::_1);
            //同步
            m_ActionHandlerMap["backupfoldsyncstart"] = std::bind(&CMessageHandler::OnBackupFoldSyncStart, this, std::placeholders::_1);
            m_ActionHandlerMap["backupfoldsyncprecent"] = std::bind(&CMessageHandler::OnBackupFoldSyncPrecent, this, std::placeholders::_1);
            //同步到媒体列表
            m_ActionHandlerMap["synctodevice"] = std::bind(&CMessageHandler::OnSyncToDevice, this, std::placeholders::_1);
            m_ActionHandlerMap["synctodeviceprecent"] = std::bind(&CMessageHandler::OnSyncToDevicePrecent, this, std::placeholders::_1);
            //文件转换
            m_ActionHandlerMap["getfilemime"] = std::bind(&CMessageHandler::OnGetFileMime, this, std::placeholders::_1);    //video/mp4
            m_ActionHandlerMap["transfilestart"] = std::bind(&CMessageHandler::OnTransFileStart, this, std::placeholders::_1);
            m_ActionHandlerMap["transfilestop"] = std::bind(&CMessageHandler::OnTransFileStop, this, std::placeholders::_1);
            m_ActionHandlerMap["transfileprecent"] = std::bind(&CMessageHandler::OnTransFilePrecent, this, std::placeholders::_1);
            m_ActionHandlerMap["transfileend"] = std::bind(&CMessageHandler::OnTransFileEnd, this, std::placeholders::_1);
        }
    
        std::map<std::string, std::function<string(nlohmann::json&)>>::iterator itor = m_ActionHandlerMap.find(strAction);
        if(itor == m_ActionHandlerMap.end())
        {
            printf("================>Unknow action:%s\n", strAction.c_str());
            string strRet = CCommonUtil::StringFormat("{\"status\":0,\"info\":\"unknow action %s\"}", strAction.c_str());
            strcpy(pszRet, strRet.c_str());
            return;
        }
        string strRet = itor->second(jsonValue);
        strcpy(pszRet, strRet.c_str());
    //}
    // catch (const std::exception& e) {  
    //     nlohmann::json jsonRet;
    //     jsonRet["status"] = 0;
    //     jsonRet["info"] = e.what();
    //     string strRet = MediaParse::CJsonUtil::ToString(jsonRet);
    //     strcpy(pszRet, strRet.c_str());
    // }  
    
}