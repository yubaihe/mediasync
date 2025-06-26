#include "ConnectServerMessage.h"
#include "Config.h"
#include "DataBase/MediaInfoTable.h"
#include "DataBase/MediaCoverTable.h"
#include "DataBase/MediaGroupTable.h"
#include "DataBase/MediaGroupItemsTable.h"
#include "DataBase/MediaJishuTable.h"
#include "DataBase/MediaGpsTable.h"
#include "Util/FileUtil.h"
#include "Util/Tools.h"
#include "Util/JsonUtil.h"
#include "ClearCache.h"
#include "Util/DbusUtil.h"
#include "BroadCastServer.h"
#include "GpsManager.h"
CConnectServerMessage::CConnectServerMessage(/* args */)
{
}

CConnectServerMessage::~CConnectServerMessage()
{
}
// string CConnectServerMessage::ToString(nlohmann::json json)
// {
//     Json::StreamWriterBuilder jsonBuilder;
//     jsonBuilder["commentStyle"] = "None"; //"All"：保留所有的注释 (默认)；"None"：删除所有的注释
//     jsonBuilder["indentation"] = "";      // 缩进字符串,如果为空字符串 "" 输出没有缩进和换行
//     jsonBuilder.settings_["precision"] = 5; //整数值，表示浮点数的精度。科学计数法：有效数字位数 (默认17)，小数：小数点后保留位数。
//     jsonBuilder.settings_["precisionType"] = "decimal"; //"significant"：用科学计数法表示浮点数 (默认);"decimal"：用小数表示浮点数
//     std::unique_ptr<Json::StreamWriter> writer(jsonBuilder.newStreamWriter());
//     std::ostringstream os;
//     writer->write(json, &os);
//     std::string strRet = os.str();
//     return strRet;
// }
string CConnectServerMessage::OnDeviceInfo(nlohmann::json& jsonRoot)
{
    //获取设备名称 版本号
    ConfigDevice device = CConfig::GetInstance()->GetDevice();
	int iRecordCount = CMediaInfoTable::GetRecordCount();
	//是否需要登录
	BOOL bNeedLogin = CConfig::GetInstance()->IsNeedLogin();
	//最后更新时间 这个指针有可能是空的 用的时候需要做好判断
    MediaInfoItem mediaItem = CMediaInfoTable::GetLatestItem();
	//读取封面
    string strCover = CMediaCoverTable::GetHomeCover();
	if(strCover.length() == 0)
	{
        strCover = mediaItem.strAddr;
	}
	else
	{
        string strCoverFile = CConfig::GetInstance()->GetStoreRoot();
        strCoverFile.append(strCover);
        BOOL bCoverExist = CFileUtil::CheckFileExist(strCoverFile);
		if(FALSE == bCoverExist)
		{
			//文件不存在
			CMediaCoverTable::RemoveHomeCover();
            strCover = mediaItem.strAddr;
		}
	}
    string strMac = Server::CTools::GetMacAddr();
    string strLicence = CFileUtil::GetLicence();

    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["devid"] = device.strDeviceID;
    jsonRet["devname"] = device.strDeviceName;
    jsonRet["devversion"] = device.strDeviceVersion;
    jsonRet["devblue"] = device.iDeviceBlue;
    jsonRet["mediacount"] = iRecordCount;
    jsonRet["lastupdatetime"] = mediaItem.iAddTime;
    jsonRet["login"] = bNeedLogin;
    jsonRet["pic"] = strCover;
    jsonRet["pic2"] = mediaItem.strAddr;

    jsonRet["eth"] = CConfig::GetInstance()->SupportEth();
    jsonRet["wlan"] = CConfig::GetInstance()->SupportWlan();
    jsonRet["hotpot"] = CConfig::GetInstance()->SupportHotPot();
    jsonRet["samba"] = CConfig::GetInstance()->SupportSamba();

    if(TRUE == CConfig::GetInstance()->SupportWlan())
    {
        jsonRet["netsetenable"] = TRUE;
    }
    else
    {
        jsonRet["netsetenable"] = FALSE;
    }
    jsonRet["mac"] = strMac;
    jsonRet["licence"] = strLicence;
    jsonRet["buildtime"] = BUILDDATE;//版本编译时间

    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnDiskInfo(nlohmann::json& jsonRoot)
{
    string strStoreAddr = CConfig::GetInstance()->GetStoreRoot();
    size_t iTotal = 0;
	size_t iUsed = 0;
    Server::CTools::GetDiskInfo(strStoreAddr.c_str(), &iTotal, &iUsed);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["total"] = iTotal;
    jsonRet["used"] = iUsed;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnLogin(nlohmann::json& jsonRoot)
{
    string strPwd = jsonRoot["pwd"];
    BOOL bSame = CConfig::GetInstance()->IsSamePwd(strPwd);
    nlohmann::json jsonRet;
    jsonRet["status"] = bSame;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGetPwdTips(nlohmann::json& jsonRoot)
{
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["pwdtip"] = CConfig::GetInstance()->GetPwdTip();
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnResetUser(nlohmann::json& jsonRoot)
{
    ConfigUser user = {};
    user.strPwd = jsonRoot["pwd"];
    user.strPwdTip = jsonRoot["pwdtip"];
	printf("PWD:%s\n", user.strPwd.c_str());
	printf("PwdTip:%s\n", user.strPwdTip.c_str());
    if(user.strPwd.length() > 0)
    {
        string strPwd = user.strPwd;
        user.strPwd = Server::CTools::GetMd5(strPwd.c_str());
    }
    CConfig::GetInstance()->SetUser(user);
    BOOL bRet = FALSE;
    if(TRUE == CConfig::GetInstance()->SupportSamba())
    {
        string strPwd = jsonRoot["pwd"];
        if(strPwd.length() == 0)
        {
            ConfigDevice device = CConfig::GetInstance()->GetDevice();
            strPwd = device.strDeviceID;
        }
        bRet = Server::CCommonUtil::ExecuteCmdWithOutReplay("passwd=%s && (echo $passwd;echo $passwd) | smbpasswd -a root -s", strPwd.c_str());
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnUpdateDeviceInfo(nlohmann::json& jsonRoot)
{
    string strDeviceName = jsonRoot["devname"];
    int iDeviceBlue = jsonRoot["devblue"];
    ConfigDevice device = CConfig::GetInstance()->GetDevice();
    device.strDeviceName = strDeviceName;
    device.iDeviceBlue = iDeviceBlue;
    BOOL bRet = CConfig::GetInstance()->SetDevice(device);
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    if(TRUE == bRet)
    {
        jsonRet["devname"] = strDeviceName.c_str();
        jsonRet["devblue"] = iDeviceBlue;
        Server::CCommonUtil::ExecuteCmd("echo '%s' > /etc/hostname", strDeviceName.c_str());
        Server::CCommonUtil::ExecuteCmd("nmbd restart");
    }
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnServerItems(nlohmann::json& jsonRoot)
{
    string strAction = jsonRoot["action"];
    
    int iPage = jsonRoot["page"];
    int iLimited = jsonRoot["limit"];
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    string strRetItems = "[]";
    int iTotalCount = 0;
    if(0 == strAction.compare("recentupload"))
    {
        // 0最近
        strRetItems = CMediaInfoTable::GetRecentRecords(iPage, iLimited, strDevice);
    }
    else if(0 == strAction.compare("favorite"))
    {
        //2收藏
        strRetItems = CMediaInfoTable::GetFavoriteRecords(iPage, iLimited, strDevice);
    }
    else if(0 == strAction.compare("yearitemlist"))
    {
        //年份
        int iYear = jsonRoot["year"];
        int iMonth = jsonRoot["month"];
        int iDay = jsonRoot["day"];
        string strLocation = jsonRoot["location"];
        strRetItems = CMediaInfoTable::GetYearRecords(iPage, iLimited, iYear, iMonth, iDay, strDevice, strLocation);
    }
    else if(0 == strAction.compare("groupitemlist"))
    {
        //分组查询
        int iGid = jsonRoot["gid"];
        string strMediaIDs = CMediaGroupItemsTable::MediaIds(iPage, iLimited, iGid, strDevice);
        string strMediaIds = Server::CTools::ReplaceString(strMediaIDs.c_str(), "&", "','");
        string strMediaIdsSort = Server::CTools::ReplaceString(strMediaIDs.c_str(), "&", ",");
        strRetItems = CMediaInfoTable::RecordsFromIds(strMediaIds, strMediaIdsSort);
        if (iPage == 0)
        {
            iTotalCount =  CMediaGroupItemsTable::MediaItemCount(iGid, strDevice);
        }
    }
    
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    nlohmann::json itemsJson = nlohmann::json::array();
    Server::CJsonUtil::FromString(strRetItems, itemsJson);
    jsonRet["items"] = itemsJson;
    if(iTotalCount != 0)
    {
        jsonRet["tcount"] = iTotalCount;
    }
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaYearList(nlohmann::json& jsonRoot)
{
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    string strYear = CMediaJishuTable::GetYears(strDevice);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["years"] = strYear;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaYearInfo(nlohmann::json& jsonRoot)
{
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iYear = jsonRoot["year"];
	int iPicCount = 0;
	int iVideoCount = 0;
	CMediaJishuTable::GetYearInfo(iYear, strDevice.c_str(), &iPicCount, &iVideoCount);

    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["year"] = iYear;
    jsonRet["piccount"] = iPicCount;
    jsonRet["videocount"] = iVideoCount;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnRecentInfo(nlohmann::json& jsonRoot)
{
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iPicCount = CMediaInfoTable::GetRecentRecordCount(MEDIATYPE_IMAGE, strDevice);
    int iVideoCount = CMediaInfoTable::GetRecentRecordCount(MEDIATYPE_VIDEO, strDevice);

    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["piccount"] = iPicCount;
    jsonRet["videocount"] = iVideoCount;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnFavoriteInfo(nlohmann::json& jsonRoot)
{
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iPicCount = CMediaInfoTable::GetRecordFavoriteCount(MEDIATYPE_IMAGE, strDevice);
    int iVideoCount = CMediaInfoTable::GetRecordFavoriteCount(MEDIATYPE_VIDEO, strDevice);

    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["piccount"] = iPicCount;
    jsonRet["videocount"] = iVideoCount;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnDevNames(nlohmann::json& jsonRoot)
{
    string strDevNames = CMediaJishuTable::GetDevNames();
    nlohmann::json jsonItems;
    Server::CJsonUtil::FromString(strDevNames, jsonItems);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["devnames"] = jsonItems;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaUpdateGpsWeiZhi(nlohmann::json& jsonRoot)
{
    string strWeiZhi = jsonRoot["weizhi"];
    string strBaiDuWeiZhi = jsonRoot["bdweizhi"];
    string strLocation = jsonRoot["location"];
    CMediaGpsTable gpsTable;
    gpsTable.SetRecord(strWeiZhi, strBaiDuWeiZhi, strLocation);
    BOOL bRet = CMediaInfoTable::UpdateGpsWeiZhi(strWeiZhi, strBaiDuWeiZhi, strLocation);
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaYearLocationGroup(nlohmann::json& jsonRoot)
{
    int iStart = jsonRoot["start"];
    int iLimited = jsonRoot["limit"];
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iYear = jsonRoot["year"];
    int iMonth = jsonRoot["month"];
    int iDay = jsonRoot["day"];
    string strItems = CMediaInfoTable::GetLocationGroup(iStart, iLimited, iYear, iMonth, iDay, strDevice);
    nlohmann::json jsonItems;
    Server::CJsonUtil::FromString(strItems, jsonItems);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["items"] = jsonItems;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaYearLocationGroupTongJi(nlohmann::json& jsonRoot)
{
    string strLocation = jsonRoot["location"];
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iYear = jsonRoot["year"];
    int iMonth = jsonRoot["month"];
    int iDay = jsonRoot["day"];
    string strItems = CMediaInfoTable::YearLocationGroupTongJi(strLocation, iYear, iMonth, iDay, strDevice);
    nlohmann::json jsonItems;
    Server::CJsonUtil::FromString(strItems, jsonItems);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["items"] = jsonItems;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnAboutDevices(nlohmann::json& jsonRoot)
{
    size_t iTotal = 0;
    size_t iUsed = 0;
    Server::CTools::GetDiskInfo(CConfig::GetInstance()->GetStoreRoot().c_str(), &iTotal, &iUsed);
    //获取设备名称 版本号
    ConfigDevice device = CConfig::GetInstance()->GetDevice();
    string strMac = Server::CTools::GetMacAddr();
    nlohmann::ordered_json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["space"] = iTotal;
    jsonRet["devversion"] = device.strDeviceVersion;
    jsonRet["mac"] = strMac;
    jsonRet["eth"] = CConfig::GetInstance()->SupportEth();
    jsonRet["wifi"] = CConfig::GetInstance()->SupportWlan();
    jsonRet["hotpot"] = CConfig::GetInstance()->SupportHotPot();
    jsonRet["samba"] = CConfig::GetInstance()->SupportSamba();
    if(TRUE == CConfig::GetInstance()->SupportSamba())
    {
        jsonRet["samba"] = Server::CTools::GetSambaVersion();
    }
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnUpdateDeviceTime(nlohmann::json& jsonRoot)
{
    string strTime = jsonRoot["time"];
    time_t iTime = atol(strTime.c_str());
    BOOL bRet = TRUE;
    if(Server::CTools::CurTimeSec() < iTime)
    {
        bRet = Server::CTools::UpdateTimeSec(iTime);
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnSetCover(nlohmann::json& jsonRoot)
{
    string strCover = jsonRoot["coveraddr"];
    //2021/IMG_20210412_072616.jpg
    BOOL bRet = CMediaCoverTable::SetHomeCover(strCover);
	nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMonthList(nlohmann::json& jsonRoot)
{
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iYear = jsonRoot["year"];
    string strItems = CMediaJishuTable::GetMonths(iYear, strDevice);
    nlohmann::json jsonItems;
    Server::CJsonUtil::FromString(strItems, jsonItems);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["months"] = jsonItems;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnDayList(nlohmann::json& jsonRoot)
{
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iYear = jsonRoot["year"];
    int iMonth = jsonRoot["month"];
    string strItems = CMediaJishuTable::GetDays(iYear, iMonth, strDevice);
    nlohmann::json jsonItems;
    Server::CJsonUtil::FromString(strItems, jsonItems);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["days"] = jsonItems;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGetYearMonthDayCover(nlohmann::json& jsonRoot)
{
    int iYear = jsonRoot["year"];
	int iMonth = 0;
    if(TRUE == jsonRoot.contains("month"))
    {
         iMonth = jsonRoot["month"];
    }
    int iDay = 0;
    if(TRUE == jsonRoot.contains("day"))
    {
        iDay = jsonRoot["day"];
    }
	string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");

    string strCover = CMediaInfoTable::YearMonthDayCover(iYear, iMonth, iDay, strDevice);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["cover"] = strCover;
    jsonRet["year"] = iYear;
    jsonRet["month"] = iMonth;
    jsonRet["day"] = iDay;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnClearCacheStart(nlohmann::json& jsonRoot)
{
    BOOL bRunning = CClearCache::GetInstance()->IsRuning();
    if(TRUE == bRunning)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        return Server::CJsonUtil::ToString(jsonRet);
    }
    bRunning = CClearCache::GetInstance()->Start();
    nlohmann::json jsonRet;
    jsonRet["status"] = bRunning;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnClearCacheProcess(nlohmann::json& jsonRoot)
{
    int iPrecent = 100;
	CLEARCACHESTATUS eStatus = CLEARCACHE_SUCCESS;
    CClearCache::GetInstance()->GetStatus(eStatus, iPrecent);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["pstatus"] = eStatus;
    jsonRet["precent"] = iPrecent;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGroupInfo(nlohmann::json& jsonRoot)
{
    string strDeviceName = "";
    if(TRUE == jsonRoot.contains("devnames"))
    {
        strDeviceName = jsonRoot["devnames"];
    }
    int iEmptyFilter = 0;
    if(TRUE == jsonRoot.contains("emptyfilter"))
    {
        iEmptyFilter = jsonRoot["emptyfilter"];
    }
    string strItems = "";
    if(strDeviceName.length() == 0)
    {
        strItems = CMediaGroupTable::GetJsonAllGroups();
    }
    else
    {
        string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
		string strGids = CMediaGroupItemsTable::GidsFromDevNames(strDevice);
        string strGid = Server::CTools::ReplaceString(strGids, "&", "','");
        strItems = CMediaGroupTable::GetJsonAllGroupsFromGids(strGid);
    }
	if(TRUE == iEmptyFilter)
	{
        string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
        nlohmann::json jsonRetItems = nlohmann::json::array();
        nlohmann::json jsonItems;
        Server::CJsonUtil::FromString(strItems, jsonItems);
        for(uint32_t i = 0; i < jsonItems.size(); ++i)
        {
            int iGid = jsonItems[i]["id"];
            int iPicCount = 0;
			int iVideoCount = 0;
			CMediaGroupItemsTable::Detail(iGid, strDevice, &iPicCount, &iVideoCount);
			if(iPicCount != 0 || iVideoCount != 0)
            {
                jsonRetItems.push_back(jsonItems[i]);
            }
        }
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["items"] = jsonRetItems;
        return Server::CJsonUtil::ToString(jsonRet);
    }
    else
    {
        nlohmann::json jsonItems;
        Server::CJsonUtil::FromString(strItems, jsonItems);
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["items"] = jsonItems;
        return Server::CJsonUtil::ToString(jsonRet);
    }
}
string CConnectServerMessage::OnGroupDetail(nlohmann::json& jsonRoot)
{
    int iGid = jsonRoot["id"];
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    int iPicCount = 0;
    int iVideoCount = 0;
    CMediaGroupItemsTable::Detail(iGid, strDevice, &iPicCount, &iVideoCount);
    //取cover
    string strCoverPic = CMediaGroupTable::GetCoverPic(iGid);
    //取name
    string strGname = CMediaGroupTable::GroupNameFromID(iGid);
    if(strCoverPic.length() > 0)
    {
        string strCoverFile(CConfig::GetInstance()->GetStoreRoot());
        strCoverFile.append(strCoverPic);
        if(FALSE == CFileUtil::CheckFileExist(strCoverFile))
        {
            CMediaGroupTable::SetCoverEmpty(strCoverPic);
            strCoverPic = "";
        }
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["gid"] = iGid;
    jsonRet["name"] = strGname;
    jsonRet["piccount"] = iPicCount;
    jsonRet["videocount"] = iVideoCount;
    jsonRet["cover"] = strCoverPic;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGroupAdd(nlohmann::json& jsonRoot)
{
    string strGroupName = jsonRoot["name"];
    int iGroupID = CMediaGroupTable::AddGroup(strGroupName);
    nlohmann::json jsonRet;
    jsonRet["status"] = iGroupID > 0?1:0;
    jsonRet["id"] = iGroupID;
    jsonRet["name"] = strGroupName;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGroupUpdate(nlohmann::json& jsonRoot)
{
    string strGroupName = jsonRoot["name"];
    int iGid = jsonRoot["id"];
    BOOL bRet = CMediaGroupTable::GroupItemUpdate(iGid, strGroupName);
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    jsonRet["id"] = iGid;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGroupDel(nlohmann::json& jsonRoot)
{
    int iGid = jsonRoot["id"];
    string strDeviceName = jsonRoot["devnames"];
    BOOL bRet = CMediaGroupItemsTable::RemoveFromGroupID(iGid);
    if(TRUE == bRet)
    {
        string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
        int iMediaCount =  CMediaGroupItemsTable::MediaItemCount(iGid, strDevice);
        if(iMediaCount == 0)
        {
            bRet = CMediaGroupTable::RemoveItemFromID(iGid);
        }
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    jsonRet["id"] = iGid;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaItemGroupSetting(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["itemid"];
    string strDeviceName = jsonRoot["devnames"];
    int iMediaType = jsonRoot["mediatype"];
    string strMediaAddr = jsonRoot["mediaaddr"];
    CMediaGroupItemsTable::RemoveFromItemID(iItemID);
    
	MediaGroupItemOne item = {};
    item.iMediaID = iItemID;
    item.iMediaType = iMediaType;
    item.strDeviceIdentify = strDeviceName;
    nlohmann::json groupsJson = jsonRoot["groups"];
    for(uint32_t i = 0; i < groupsJson.size(); ++i)
    {
        item.iGID = groupsJson[i]["gid"];
        int iCover = groupsJson[i]["cover"];
        if(1 == iCover)
        {
            CMediaGroupTable::SetCover(item.iGID, strMediaAddr);
        }
        CMediaGroupItemsTable::AddItem(item);
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGroupNamesFromItemID(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["id"];
    string strGroups = CMediaInfoTable::GetGroupInfoFomItemID(iItemID);
    nlohmann::json jsonItems;
    Server::CJsonUtil::FromString(strGroups, jsonItems);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["groups"] = jsonItems;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaItemsGroupAdd(nlohmann::json& jsonRoot)
{
    string strItemIds = jsonRoot["itemids"];
    string strGroupIds = jsonRoot["groupids"];
    vector<string> itemIdsVec = Server::CCommonUtil::StringSplit(strItemIds, "&");
    vector<string> groupIdsVec = Server::CCommonUtil::StringSplit(strGroupIds, "&");
    for(size_t i = 0; i < itemIdsVec.size(); ++i)
    {
        int iItemID = atoi(itemIdsVec[i].c_str());
        MediaInfoItem mediaItem = CMediaInfoTable::GetItemByID(iItemID);
        for (size_t j = 0; j < groupIdsVec.size(); ++j)
        {
            int iGroupId = atoi(groupIdsVec[j].c_str());
            MediaGroupItemOne groupItem = {};
            groupItem.iMediaID = iItemID;
            groupItem.iMediaType = mediaItem.iMediaType;
            groupItem.strDeviceIdentify = mediaItem.strDeviceName;
            groupItem.iGID = iGroupId;
            CMediaGroupItemsTable::AddItem(groupItem);
        }
    }
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnListDisk(nlohmann::json& jsonRoot)
{
    return CDbusUtil::AllDisks();
}
string CConnectServerMessage::OnStore(nlohmann::json& jsonRoot)
{
    string strStoreAddr = CConfig::GetInstance()->GetStoreRoot();
    string strThumb = CConfig::GetInstance()->GetUploadRoot();

    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["addr"] = strStoreAddr.c_str();
    jsonRet["tempaddr"] = strThumb.c_str();
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnUploadFile(nlohmann::json& jsonRoot)
{
    //取一个文件名称 名称为当前毫秒数
    string strFileName = CFileUtil::GetNewFileName(CConfig::GetInstance()->GetUploadRoot());
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["filename"] = strFileName;
     //端口号默认用web的
    jsonRet["conport"] = 0;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnCheckMd5(nlohmann::json& jsonRoot)
{
    string strMd5 = jsonRoot["md5"];
    string strFileName = CMediaInfoTable::FileNameFromMd5(strMd5);
    if(strFileName.length() > 0)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["filename"] = strFileName;
        jsonRet["info"] = "media exist";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    else
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        return Server::CJsonUtil::ToString(jsonRet);
    }
}
string CConnectServerMessage::OnReportInfo(nlohmann::json& jsonRoot)
{
    //fname sfile slen lfile llen md5 ptime weizhi meititype meitisize deviceidentify mediaaddr addtime
	
	//小文件信息
	string strSFileName = jsonRoot["sfile"];
    long iSFileSize = jsonRoot["slen"];
	//大文件信息
	string strLFileName = jsonRoot["lfile"];
    long iLFileSize = jsonRoot["llen"];
	//LivePhoto
    string strExFileName = jsonRoot["exfile"];
    long iExFileSize = jsonRoot["exlen"];

	//先判断文件在不再
    string strPathFile(CConfig::GetInstance()->GetUploadRoot());
    strPathFile.append(strSFileName);
	printf("~~~%s\n", strPathFile.c_str());
	if(CFileUtil::GetFileSize(strPathFile) != iSFileSize)
	{
		printf("%s device:%ld server:%ld\n", strSFileName.c_str(), iSFileSize, CFileUtil::GetFileSize(strPathFile));
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "small file size not match";
        return Server::CJsonUtil::ToString(jsonRet);
	}
	strPathFile = CConfig::GetInstance()->GetUploadRoot();
    strPathFile.append(strLFileName);
	if(CFileUtil::GetFileSize(strPathFile) != iLFileSize)
	{
		printf("%s device:%ld server:%ld\n", strPathFile.c_str(), iLFileSize, CFileUtil::GetFileSize(strPathFile));
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "large file size not match";
        return Server::CJsonUtil::ToString(jsonRet);
	}
	strPathFile = CConfig::GetInstance()->GetUploadRoot();
    strPathFile.append(strExFileName);
	if(iExFileSize > 0 && CFileUtil::GetFileSize(strPathFile) != iExFileSize)
	{
		printf("%s device:%ld server:%ld\n", strPathFile.c_str(), iExFileSize, CFileUtil::GetFileSize(strPathFile));
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "exfile file size not match";
        return Server::CJsonUtil::ToString(jsonRet);
	}
	//检查当前时间是否正确
	if(Server::CTools::GetYear(Server::CTools::CurTimeSec()) == 1970)
	{
		long iUpdateTime = jsonRoot["utimesec"];
		Server::CTools::UpdateTimeSec(iUpdateTime);
	}
	//文件都是对的 下面开始存储信息了
	//判断年份文件夹是否存在
	long iPaiSheTime = jsonRoot["paitime"];
	if(iPaiSheTime == 0)
	{
		//没有拍摄时间用当前时间
		iPaiSheTime = Server::CTools::CurTimeSec();
	}
	int iYear = Server::CTools::GetYear(iPaiSheTime);
	printf("iPaiSheTime:%ld year:%d\n", iPaiSheTime, iYear);
    string strDestFold = Server::CCommonUtil::StringFormat("%s%d/", CConfig::GetInstance()->GetStoreRoot().c_str(), iYear);
    string strDestThumbFold = Server::CCommonUtil::StringFormat("%s%d/", CConfig::GetInstance()->GetThumbRoot().c_str(), iYear);
    string strDestExFold = Server::CCommonUtil::StringFormat("%s%d/", CConfig::GetInstance()->GetExtraRoot().c_str(), iYear);
	
	if(FALSE == CFileUtil::CheckFoldExist(strDestFold))
	{
		//文件夹不存在 创建
		if(FALSE == CFileUtil::CreateFold(strDestFold))
		{
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            jsonRet["info"] = "fold no permission";
            return Server::CJsonUtil::ToString(jsonRet);
		}
	}
	if(FALSE == CFileUtil::CheckFoldExist(strDestThumbFold))
	{
		//文件夹不存在 创建
		if(FALSE == CFileUtil::CreateFold(strDestThumbFold))
		{
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            jsonRet["info"] = "fold no permission";
            return Server::CJsonUtil::ToString(jsonRet);
		}
	}
	if(FALSE == CFileUtil::CheckFoldExist(strDestExFold))
	{
		//文件夹不存在 创建
		if(FALSE == CFileUtil::CreateFold(strDestExFold))
		{
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            jsonRet["info"] = "fold no permission";
            return Server::CJsonUtil::ToString(jsonRet);
		}
	}
    
	// char szFileName[MAX_PATH] = {0};
	// char szSmallDestFile[MAX_PATH] = {0};
	// char szLargeDestFile[MAX_PATH] = {0};
	// char szExDestFile[MAX_PATH] = {0};
	//使用图片原来的名字
	//const char* pszFileName = json_object_get_string(json_object_object_get(pJsonRoot, "fname"));
	//char szName[MAX_PATH] = {0};
	//char szPostFix[MAX_PATH] = {0};
	//FileUtil_SepFile(pszFileName, szName, szPostFix);
	// FileUtil_GetNewFileName2(iYear, szName, szPostFix, szFileName);
	//使用毫秒数的名字
	int iMediaType = jsonRoot["mediatype"];
	char szPostFix[10] = {0};
	if(MEDIATYPE_IMAGE == iMediaType)
	{
		strcpy(szPostFix, "jpg");
	}
	else
	{
		strcpy(szPostFix, "mp4");
	}
	string strFileName = CFileUtil::GetNewFileName2(iYear, strLFileName, szPostFix);
    string strLargeDestFile = Server::CCommonUtil::StringFormat("%s%s.%s", strDestFold.c_str(), strFileName.c_str(), szPostFix);
    string strSmallDestFile = Server::CCommonUtil::StringFormat("%s%s_%s.%s", strDestThumbFold.c_str(), strFileName.c_str(), szPostFix, "jpg");
    string strExDestFile = Server::CCommonUtil::StringFormat("%s%s_%s.%s", strDestExFold.c_str(), strFileName.c_str(), szPostFix, "mp4");
	
    string strSmallSrcFile = Server::CCommonUtil::StringFormat("%s/%s", CConfig::GetInstance()->GetUploadRoot().c_str(), strSFileName.c_str());
    string strLargeSrcFile = Server::CCommonUtil::StringFormat("%s/%s", CConfig::GetInstance()->GetUploadRoot().c_str(), strLFileName.c_str());
    string strExSrcFile = Server::CCommonUtil::StringFormat("%s/%s", CConfig::GetInstance()->GetUploadRoot().c_str(), strExFileName.c_str());
    if(FALSE == CFileUtil::MoveFile(strLargeSrcFile.c_str(), strLargeDestFile.c_str()))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "move lager file error";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    if(FALSE == CFileUtil::MoveFile(strSmallSrcFile.c_str(), strSmallDestFile.c_str()))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "move small file error";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    if(iExFileSize > 0 && FALSE == CFileUtil::MoveFile(strExSrcFile.c_str(), strExDestFile.c_str()))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "move ex file error";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    string strMd5Num = jsonRoot["md5"];
    string strWeiZhi = jsonRoot["weizhi"];
    string strDeviceName = jsonRoot["devicename"];
    int iWidth = jsonRoot["width"];
    int iHeight = jsonRoot["height"];
    int iDuration = jsonRoot["duration"];
    string strLocation = jsonRoot["location"];

	//校验位置信息如果没有在获取一次
	float dLat = 0;
	float dLong = 0;
	GPSTYPE iGpsType = GPSTYPE_UNKNOW;
	string strWeiZhi2 = "0.000000&0.000000";
	if(FALSE == Server::CTools::CheckGps(strWeiZhi, &iGpsType, &dLong, &dLat))
	{
		strWeiZhi2 = CDbusUtil::MediaParseGps(strLargeDestFile);
	}
    else
    {
        strWeiZhi2 = strWeiZhi;
    }
	string strFileAddr = Server::CCommonUtil::StringFormat("%d/%s.%s", iYear, strFileName.c_str(), szPostFix);
	MediaInfoItem item = {};
	
	item.iPaiSheTime = iPaiSheTime; 							//拍摄时间
	item.iAddTime = Server::CTools::CurTimeSec();               //添加时间
	item.iMediaType = iMediaType;                               //媒体类型
	item.strMd5Num = strMd5Num;  								//MD5值
	item.strWeiZhi = strWeiZhi2; 								//拍摄时候的精度纬度 
	item.iMeiTiSize = iLFileSize; 							//拍摄图片的大小
	item.iWidth = iWidth;							//宽度
	item.iHeight = iHeight;							//高度
	item.iDuration = iDuration;						//持续时间
	item.strDeviceName = strDeviceName; 					//设备名称
	item.strAddr = strFileAddr;									//媒体地址
	item.strLocation = strLocation.length()==0?"":strLocation;  //拍摄时候地理位置
	item.iHasExtra = iExFileSize > 0?TRUE:FALSE;
    BOOL bRet = CMediaInfoTable::AddItem(item);
	if(FALSE == bRet)
	{
		//入库失败了
        CFileUtil::RemoveFile(strSmallDestFile);
        CFileUtil::RemoveFile(strLargeDestFile);
        CFileUtil::RemoveFile(strExDestFile);
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "add record error";
        return Server::CJsonUtil::ToString(jsonRet);
	}
	else
	{
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["file"] = strFileAddr;
        return Server::CJsonUtil::ToString(jsonRet);
	}
}
// string CConnectServerMessage::OnItemDetail(nlohmann::json& jsonRoot)
// {
//     int iItemID = jsonRoot["itemid"];
//     string strDeviceName = jsonRoot["devnames"];
//     string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
//     string strType = jsonRoot["type"];
//     //都需要devname Type
// 	//recent  => 不需要任何参数
// 	//favorite  => 不需要任何参数
// 	//group   => gid
//     int iGid = jsonRoot["gid"];
//     //year
//     int iYear = jsonRoot["year"];
//     int iMonth = jsonRoot["month"];
//     int iDay = jsonRoot["day"];
//     string strLocation = jsonRoot["location"];
// 	BOOL bRet = TRUE;
// 	if(0 == strType.length())
// 	{
// 		//只取当前的Item
//         string strItem = CMediaInfoTable::GetItem(iItemID, bRet);
//         nlohmann::json jsonItem;
//         Server::CJsonUtil::FromString(strItem, jsonItem);
//         if(FALSE == bRet)
//         {
//             nlohmann::json jsonRet;
//             jsonRet["status"] = 0;
//             return Server::CJsonUtil::ToString(jsonRet);
//         }
//         nlohmann::json jsonRet;
//         jsonRet["status"] = 1;
//         jsonRet["cur"] = jsonItem;
//         return Server::CJsonUtil::ToString(jsonRet);
// 	}
// 	//取当前itemid
//     string strItem = CMediaInfoTable::GetItem(iItemID, bRet);
//     nlohmann::json jsonItem;
//     Server::CJsonUtil::FromString(strItem, jsonItem);
//     if(FALSE == bRet)
//     {
//         int iNextItemID = CMediaInfoTable::GetNextItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
//         if(iNextItemID < 0)
//         {
//             int iPrevItemID = CMediaInfoTable::GetPrevItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
//             if(iPrevItemID > 0)
//             {
//                 iItemID = iPrevItemID;
//                 bRet = TRUE;
//             }
//         }
//         else
//         {
//             iItemID = iNextItemID;
//             bRet = TRUE;
//         }
//     }

// 	//没有取到当前itemid
// 	if(FALSE == bRet)
// 	{
//         nlohmann::json jsonRet;
//         jsonRet["status"] = 1;
//         return Server::CJsonUtil::ToString(jsonRet);
// 	}
//     int iNextItemID = CMediaInfoTable::GetNextItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
//     int iPrevItemID = CMediaInfoTable::GetPrevItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
//     nlohmann::json jsonRet;
//     jsonRet["status"] = 1;
//     printf("PrevItemID:%d\t CurItemID:%d NextItemID:%d\n", iPrevItemID, iItemID, iNextItemID);
//     if(iItemID >= 0)
//     {
//         string strItem = CMediaInfoTable::GetItem(iItemID, bRet);
//         if(TRUE == bRet)
//         {
//             nlohmann::json jsonCur;
//             Server::CJsonUtil::FromString(strItem, jsonCur);
//             jsonRet["cur"] = jsonCur;
//         }
//     }
//     if(iNextItemID >= 0)
//     {
//         string strItem = CMediaInfoTable::GetItem(iNextItemID, bRet);
//         if(TRUE == bRet)
//         {
//             nlohmann::json jsonNext;
//             Server::CJsonUtil::FromString(strItem, jsonNext);
//             jsonRet["next"] = jsonNext;
//         }
//     }
//     if(iPrevItemID >= 0)
//     {
//         string strItem = CMediaInfoTable::GetItem(iPrevItemID, bRet);
//         if(TRUE == bRet)
//         {
//             nlohmann::json jsonPrev;
//             Server::CJsonUtil::FromString(strItem, jsonPrev);
//             jsonRet["prev"] = jsonPrev;
//         }
//     }
//     return Server::CJsonUtil::ToString(jsonRet);
// }
string CConnectServerMessage::OnItemDetail(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["itemid"];
    string strDeviceName = jsonRoot["devnames"];
    string strDevice = Server::CTools::ReplaceString(strDeviceName, "&", "','");
    string strType = jsonRoot["type"];
    //都需要devname Type
	//recent  => 不需要任何参数
	//favorite  => 不需要任何参数
	//group   => gid
    int iGid = jsonRoot["gid"];
    //year
    int iYear = jsonRoot["year"];
    int iMonth = jsonRoot["month"];
    int iDay = jsonRoot["day"];
    string strLocation = jsonRoot["location"];
	BOOL bRet = TRUE;
	if(0 == strType.length())
	{
		//只取当前的Item
        string strItem = CMediaInfoTable::GetItem(iItemID, bRet);
        nlohmann::json jsonItem;
        Server::CJsonUtil::FromString(strItem, jsonItem);
        if(FALSE == bRet)
        {
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            return Server::CJsonUtil::ToString(jsonRet);
        }
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["cur"] = jsonItem;
        return Server::CJsonUtil::ToString(jsonRet);
	}
	//取当前itemid
    printf("Get curitemid\n");
    string strItem = CMediaInfoTable::GetItem(iItemID, bRet);
    printf("==>%s\n", strItem.c_str());
    nlohmann::json jsonItem;
    Server::CJsonUtil::FromString(strItem, jsonItem);
    if(FALSE == bRet)
    {
        printf("Failed get cur item id info re get itemid:%d\n", iItemID);
        int iNextItemID = CMediaInfoTable::GetNextItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
        if(iNextItemID < 0)
        {
            printf("Failed get next item id info:%d\n", iItemID);
            int iPrevItemID = CMediaInfoTable::GetPrevItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
            if(iPrevItemID > 0)
            {
                iItemID = iPrevItemID;
                bRet = TRUE;
                printf("success get prev item id info:%d\n", iItemID);
            }
            else
            {
                printf("Failed get prev item id info:%d\n", iItemID);
            }
        }
        else
        {
            iItemID = iNextItemID;
            bRet = TRUE;
            printf("success get next item id info:%d\n", iItemID);
        }
    }

	//没有取到当前itemid
	if(FALSE == bRet)
	{
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        return Server::CJsonUtil::ToString(jsonRet);
	}
    printf("begin get next item id\n");
    int iNextItemID = CMediaInfoTable::GetNextItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
    printf("next item id:%d\n", iNextItemID);
    printf("end get next item id\n");
    printf("begin get prev item id\n");
    int iPrevItemID = CMediaInfoTable::GetPrevItemID(iItemID, strType, strDevice, iGid, iYear, iMonth, iDay, strLocation);
    printf("prev item id:%d\n", iPrevItemID);
    printf("end get prev item id\n");
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    printf("PrevItemID:%d\t CurItemID:%d NextItemID:%d\n", iPrevItemID, iItemID, iNextItemID);
    if(iItemID >= 0)
    {
        string strItem = CMediaInfoTable::GetItem(iItemID, bRet);
        if(TRUE == bRet)
        {
            nlohmann::json jsonCur;
            Server::CJsonUtil::FromString(strItem, jsonCur);
            jsonRet["cur"] = jsonCur;
        }
    }
    if(iNextItemID >= 0)
    {
        string strItem = CMediaInfoTable::GetItem(iNextItemID, bRet);
        if(TRUE == bRet)
        {
            nlohmann::json jsonNext;
            Server::CJsonUtil::FromString(strItem, jsonNext);
            jsonRet["next"] = jsonNext;
        }
    }
    if(iPrevItemID >= 0)
    {
        string strItem = CMediaInfoTable::GetItem(iPrevItemID, bRet);
        if(TRUE == bRet)
        {
            nlohmann::json jsonPrev;
            Server::CJsonUtil::FromString(strItem, jsonPrev);
            jsonRet["prev"] = jsonPrev;
        }
    }
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnMediaItemFavorite(nlohmann::json& jsonRoot)
{
    int iID = jsonRoot["id"];
    int iFavorite = jsonRoot["favorite"];
    BOOL bRet = CMediaInfoTable::UpdateFavorite(iID, iFavorite);
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    if(TRUE == bRet)
    {
        jsonRet["id"] = iID;
    }
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnDelMediaItem(nlohmann::json& jsonRoot)
{
    int iID = jsonRoot["id"];
    string strFileName = CMediaInfoTable::GetFileName(iID);
    if(strFileName.length() == 0)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        return Server::CJsonUtil::ToString(jsonRet);
    }
    BOOL bRet = CMediaGroupItemsTable::RemoveFromItemID(iID);
	if(FALSE == bRet)
	{
		nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        return Server::CJsonUtil::ToString(jsonRet);
	}
    //删除group cover
    bRet = CMediaGroupTable::SetCoverEmpty(strFileName);
	if(FALSE == bRet)
	{
		nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        return Server::CJsonUtil::ToString(jsonRet);
	}
    bRet = CMediaInfoTable::RemoveItem(iID);
	if(FALSE == bRet)
	{
		nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        return Server::CJsonUtil::ToString(jsonRet);
	}
	//删除文件thumb
    string strThumbFile(CConfig::GetInstance()->GetThumbRoot());
    strThumbFile.append(CFileUtil::ThumbNameFromFileName(strFileName));
    CFileUtil::RemoveFile(strThumbFile);
    printf("remove file:%s\n", strThumbFile.c_str());
    //删除文件ex
    string strExFile(CConfig::GetInstance()->GetExtraRoot());
    strExFile.append(CFileUtil::FileExNameFromFileName(strFileName));
    CFileUtil::RemoveFile(strExFile);
    printf("remove file:%s\n", strExFile.c_str());

    string strFile(CConfig::GetInstance()->GetStoreRoot());
    strFile.append(strFileName);
    CFileUtil::RemoveFile(strFile);
    printf("remove file:%s\n", strFile.c_str());
	//删除目录
	int iYear = CFileUtil::GetYear(strFile);
	CFileUtil::RemoveYearEmptyFold(iYear);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["id"] = iID;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnBroadCastOnline(nlohmann::json& jsonRoot)
{
    extern CBroadCastServer g_BroadCastServer;
    int iBroadCastPort = CConfig::GetInstance()->GetBroadCastPort();
    g_BroadCastServer.BroadCastNotifyOnLine(iBroadCastPort);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnUpdatePaiSheTime(nlohmann::json& jsonRoot)
{
    printf("OnUpdatePaiSheTime\n");
    int iItemID = jsonRoot["id"];
    int iTimeSec = jsonRoot["timesec"];
    MediaInfoItem item = CMediaInfoTable::GetItemByID(iItemID);
    if(item.iID < 0)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "item not find";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    
    int iYear = Server::CTools::GetYear(iTimeSec);
    printf("Time:%d Year:%d\n", iTimeSec, iYear);
    string strDestFold = Server::CCommonUtil::StringFormat("%s%d/", CConfig::GetInstance()->GetStoreRoot().c_str(), iYear);
    string strDestThumbFold = Server::CCommonUtil::StringFormat("%s%d/", CConfig::GetInstance()->GetThumbRoot().c_str(), iYear);
	string strDestExFold = Server::CCommonUtil::StringFormat("%s%d/", CConfig::GetInstance()->GetExtraRoot().c_str(), iYear);
    
	if(FALSE == CFileUtil::CheckFoldExist(strDestFold))
    {
        //文件夹不存在 创建
        if(FALSE == CFileUtil::CreateFold(strDestFold))
        {
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            jsonRet["info"] = "fold no permission";
            return Server::CJsonUtil::ToString(jsonRet);
        }
    }
    if(FALSE == CFileUtil::CheckFoldExist(strDestThumbFold))
    {
        //文件夹不存在 创建
        if(FALSE == CFileUtil::CreateFold(strDestThumbFold))
        {
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            jsonRet["info"] = "fold no permission";
            return Server::CJsonUtil::ToString(jsonRet);
        }
    }
    if(FALSE == CFileUtil::CheckFoldExist(strDestExFold))
    {
        //文件夹不存在 创建
        if(FALSE == CFileUtil::CreateFold(strDestExFold))
        {
            nlohmann::json jsonRet;
            jsonRet["status"] = 0;
            jsonRet["info"] = "fold no permission";
            return Server::CJsonUtil::ToString(jsonRet);
        }
    }
    //2020/IMG_2937_1.jpg   => IMG_2937_1.jpg
    string strFileName = CFileUtil::GetFileOnlyName(item.strAddr);
	//IMG_2937_1.jpg => szName(IMG_2937_1) szPostFix(jpg)
    char szName[MAX_PATH] = {0};
    char szPostFix[MAX_PATH] = {0};
    CFileUtil::SepFile(strFileName, szName, szPostFix);
    //get new filename.
    string strNewFileName = CFileUtil::GetNewFileName2(iYear, szName, szPostFix);

    string strSmallDestFile = Server::CCommonUtil::StringFormat("%s%s_%s.%s", strDestThumbFold.c_str(), strNewFileName.c_str(), szPostFix, "jpg");
    string strLargeDestFile = Server::CCommonUtil::StringFormat("%s%s.%s", strDestFold.c_str(), strNewFileName.c_str(), szPostFix);
    string strExDestFile = Server::CCommonUtil::StringFormat("%s%s_%s.%s", strDestExFold.c_str(), strNewFileName.c_str(), szPostFix, "mp4");

    memset(szName, 0, MAX_PATH);
    memset(szPostFix, 0, MAX_PATH);
    CFileUtil::SepFile(item.strAddr, szName, szPostFix);
    string strSmallSrcFile = Server::CCommonUtil::StringFormat("%s%s_%s.jpg", CConfig::GetInstance()->GetThumbRoot().c_str(), szName, szPostFix);
    string strLargeSrcFile = Server::CCommonUtil::StringFormat("%s%s", CConfig::GetInstance()->GetStoreRoot().c_str(), item.strAddr.c_str());
    string strExSrcFile = Server::CCommonUtil::StringFormat("%s%s_%s.mp4", CConfig::GetInstance()->GetExtraRoot().c_str(), szName, szPostFix);

    printf("LargeSrcFile:%s\n", strLargeSrcFile.c_str());
    printf("LargeDestFile:%s\n", strLargeDestFile.c_str());

    if(FALSE == CFileUtil::MoveFile(strLargeSrcFile, strLargeDestFile))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "move lager file error";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    if(FALSE == CFileUtil::MoveFile(strSmallSrcFile, strSmallDestFile))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "move small file error";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    if(item.iHasExtra == TRUE && FALSE == CFileUtil::MoveFile(strExSrcFile, strExDestFile))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["info"] = "move ex file error";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    int iOldYear = CFileUtil::GetYear(item.strAddr);
	CFileUtil::RemoveYearEmptyFold(iOldYear);
    
    string strNewFile = Server::CCommonUtil::StringFormat("%d/%s.%s", iYear, strNewFileName.c_str(), szPostFix);
    BOOL bRet = CMediaInfoTable::UpdateItemPaiSheShiJian(iItemID, iTimeSec, strNewFile.c_str());
    if(FALSE == bRet)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["id"] = iItemID;
        return Server::CJsonUtil::ToString(jsonRet);
    }
    //计数 -1
    CMediaJishuTable::Decrease(item.iPaiSheTime, item.iMediaType, item.strDeviceName);
    //计数 +1
    CMediaJishuTable::Increase(iTimeSec, item.iMediaType, item.strDeviceName);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["id"] = iItemID;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnUpdateGpsAddr(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["id"];
    string strGps = jsonRoot["gps"];
    string strAddr = jsonRoot["addr"];
    int iTag = jsonRoot["tag"];
    BOOL bRet = FALSE;
    if(iTag == 0)
    {
        //sync
        bRet = CMediaInfoTable::UpdateItemGpsAddr(iItemID, strGps, strAddr);
    }
    else if(iTag == 1)
    {
        bRet = CDbusUtil::BackupUpdateGps(iItemID, strGps, strAddr);
    }
    
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    if(FALSE == bRet)
    {
        jsonRet["info"] = "update failed";
    }
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGroupNameFromID(nlohmann::json& jsonRoot)
{
    int iGid = jsonRoot["id"];
    string strGroupName = CMediaGroupTable::GroupNameFromID(iGid);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["name"] = strGroupName.c_str();
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnCheckFile(nlohmann::json& jsonRoot)
{
    long iPaiSheTime = jsonRoot["paitime"];
    string strDevName = jsonRoot["devname"];
    int iMediaType = jsonRoot["mediatype"];
	long iLFileSize = jsonRoot["llen"];
    string strFileName = CMediaInfoTable::FileNameFromPaiTime(iPaiSheTime, strDevName, iMediaType, iLFileSize);
    BOOL bExist = strFileName.length() == 0?FALSE:TRUE;
    if(TRUE == bExist)
    {
        //存在了报错
        nlohmann::json jsonRet;
        jsonRet["status"] = 0;
        jsonRet["filename"] = strFileName;
        jsonRet["info"] = "media exist";
        return Server::CJsonUtil::ToString(jsonRet);
    }
    
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnGpsLocation(nlohmann::json& jsonRoot)
{
    string strGps = jsonRoot["gps"];
    BOOL bNeedParse = jsonRoot["parse"];
    BOOL bSupport  = CGpsManager::GetInstance()->IsSupport();
    if(FALSE == bSupport)
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["location"] = "";
        jsonRet["gps"] = "";
        jsonRet["gps2"] = "";
        jsonRet["support"] = bSupport;
        return Server::CJsonUtil::ToString(jsonRet);
    }
    MediaGpsItem gpsItem = {};
    CMediaGpsTable gpsTable;
    if(TRUE == gpsTable.QueryItem(strGps, gpsItem))
    {
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["location"] = gpsItem.strLocation;
        jsonRet["gps"] = gpsItem.strGps;
        jsonRet["gps2"] = gpsItem.strGps2;
        jsonRet["support"] = bSupport;
        return Server::CJsonUtil::ToString(jsonRet);
    }
    if(FALSE == bNeedParse)
    {
        //不用解析
        nlohmann::json jsonRet;
        jsonRet["status"] = 1;
        jsonRet["location"] = gpsItem.strLocation;
        jsonRet["gps"] = gpsItem.strGps;
        jsonRet["gps2"] = gpsItem.strGps2;
        jsonRet["support"] = bSupport;
        return Server::CJsonUtil::ToString(jsonRet);
    }
    //需要解析 客户端轮询
    CGpsManager::GetInstance()->AddDetectItem(strGps);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    jsonRet["location"] = "";
    jsonRet["gps"] = "";
    jsonRet["gps2"] = "";
    jsonRet["support"] = bSupport;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnSetBaiDuAk(nlohmann::json& jsonRoot)
{
    string strBaiDuAk = jsonRoot["baiduak"];
    CGpsManager::GetInstance()->SetBaiDuKey(strBaiDuAk);
    nlohmann::json jsonRet;
    jsonRet["status"] = 1;
    return Server::CJsonUtil::ToString(jsonRet);
}
string CConnectServerMessage::OnSetMediaPinned(nlohmann::json& jsonRoot)
{
    int iItemID = jsonRoot["id"];
    BOOL bPinned = jsonRoot["pinned"];
    CMediaInfoTable table;
    BOOL bRet = table.SetPinned(iItemID, bPinned);
    nlohmann::json jsonRet;
    jsonRet["status"] = bRet;
    return Server::CJsonUtil::ToString(jsonRet);
}
// string CConnectServerMessage::OnSetGpsParse(nlohmann::json& jsonRoot)
// {
//     string strGps = jsonRoot["gps"];
//     MediaGpsItem gpsItem = {};
//     CMediaGpsTable gpsTable;
//     if(TRUE == gpsTable.QueryItem(strGps, gpsItem))
//     {
//         nlohmann::json jsonRet;
//         jsonRet["status"] = 1;
//         return Server::CJsonUtil::ToString(jsonRet);
//     }
//     BOOL bRet = CGpsManager::GetInstance()->AddDetectItem(strGps);
//     nlohmann::json jsonRet;
//     jsonRet["status"] = bRet;
//     return Server::CJsonUtil::ToString(jsonRet);
// }
void CConnectServerMessage::OnMessage(const char* pszMsg, char* pszRet)
{
	nlohmann::json jsonValue;
    BOOL bRet = Server::CJsonUtil::FromString(pszMsg, jsonValue);
    if(FALSE == bRet)
    {
        strcpy(pszRet, "{\"status\":0,\"info\":\"input error\"}");
        return;
    }
    if(m_ActionHandlerMap.size() == 0)
    {
        m_ActionHandlerMap["deviceinfo"] = std::bind(&CConnectServerMessage::OnDeviceInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["diskinfo"] = std::bind(&CConnectServerMessage::OnDiskInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["login"] = std::bind(&CConnectServerMessage::OnLogin, this, std::placeholders::_1);
        m_ActionHandlerMap["pwdtip"] = std::bind(&CConnectServerMessage::OnGetPwdTips, this, std::placeholders::_1);
        m_ActionHandlerMap["resetuser"] = std::bind(&CConnectServerMessage::OnResetUser, this, std::placeholders::_1);
        m_ActionHandlerMap["updatedeviceinfo"] = std::bind(&CConnectServerMessage::OnUpdateDeviceInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["recentupload"] = std::bind(&CConnectServerMessage::OnServerItems, this, std::placeholders::_1);
        m_ActionHandlerMap["favorite"] = std::bind(&CConnectServerMessage::OnServerItems, this, std::placeholders::_1);
        m_ActionHandlerMap["yearitemlist"] = std::bind(&CConnectServerMessage::OnServerItems, this, std::placeholders::_1);
        m_ActionHandlerMap["groupitemlist"] = std::bind(&CConnectServerMessage::OnServerItems, this, std::placeholders::_1);
        m_ActionHandlerMap["yearlist"] = std::bind(&CConnectServerMessage::OnMediaYearList, this, std::placeholders::_1);
        m_ActionHandlerMap["yearinfo"] = std::bind(&CConnectServerMessage::OnMediaYearInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["recentinfo"] = std::bind(&CConnectServerMessage::OnRecentInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["favoriteinfo"] = std::bind(&CConnectServerMessage::OnFavoriteInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["devnames"] = std::bind(&CConnectServerMessage::OnDevNames, this, std::placeholders::_1);
        m_ActionHandlerMap["updategpslocation"] = std::bind(&CConnectServerMessage::OnMediaUpdateGpsWeiZhi, this, std::placeholders::_1);
        m_ActionHandlerMap["yearlocationgroup"] = std::bind(&CConnectServerMessage::OnMediaYearLocationGroup, this, std::placeholders::_1);
        m_ActionHandlerMap["yearlocationgrouptongji"] = std::bind(&CConnectServerMessage::OnMediaYearLocationGroupTongJi, this, std::placeholders::_1);
        m_ActionHandlerMap["aboutdevice"] = std::bind(&CConnectServerMessage::OnAboutDevices, this, std::placeholders::_1);
        m_ActionHandlerMap["updatetime"] = std::bind(&CConnectServerMessage::OnUpdateDeviceTime, this, std::placeholders::_1);
        m_ActionHandlerMap["setcover"] = std::bind(&CConnectServerMessage::OnSetCover, this, std::placeholders::_1);
        m_ActionHandlerMap["monthlist"] = std::bind(&CConnectServerMessage::OnMonthList, this, std::placeholders::_1);
        m_ActionHandlerMap["daylist"] = std::bind(&CConnectServerMessage::OnDayList, this, std::placeholders::_1);
        m_ActionHandlerMap["yearmonthdaycover"] = std::bind(&CConnectServerMessage::OnGetYearMonthDayCover, this, std::placeholders::_1);
        m_ActionHandlerMap["clearcachestart"] = std::bind(&CConnectServerMessage::OnClearCacheStart, this, std::placeholders::_1);
        m_ActionHandlerMap["clearcacheprocess"] = std::bind(&CConnectServerMessage::OnClearCacheProcess, this, std::placeholders::_1);
        m_ActionHandlerMap["groupinfo"] = std::bind(&CConnectServerMessage::OnGroupInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["groupdetail"] = std::bind(&CConnectServerMessage::OnGroupDetail, this, std::placeholders::_1);
        m_ActionHandlerMap["groupadd"] = std::bind(&CConnectServerMessage::OnGroupAdd, this, std::placeholders::_1);
        m_ActionHandlerMap["groupupdate"] = std::bind(&CConnectServerMessage::OnGroupUpdate, this, std::placeholders::_1);
        m_ActionHandlerMap["groupdel"] = std::bind(&CConnectServerMessage::OnGroupDel, this, std::placeholders::_1);
        m_ActionHandlerMap["mediaitemgroupsetting"] = std::bind(&CConnectServerMessage::OnMediaItemGroupSetting, this, std::placeholders::_1);
        m_ActionHandlerMap["groupnamesfromitemid"] = std::bind(&CConnectServerMessage::OnGroupNamesFromItemID, this, std::placeholders::_1);
        m_ActionHandlerMap["mediaitemsgroupadd"] = std::bind(&CConnectServerMessage::OnMediaItemsGroupAdd, this, std::placeholders::_1);
        m_ActionHandlerMap["listdisk"] = std::bind(&CConnectServerMessage::OnListDisk, this, std::placeholders::_1);
        m_ActionHandlerMap["store"] = std::bind(&CConnectServerMessage::OnStore, this, std::placeholders::_1);
        m_ActionHandlerMap["upfilename"] = std::bind(&CConnectServerMessage::OnUploadFile, this, std::placeholders::_1);
        m_ActionHandlerMap["checkmd5"] = std::bind(&CConnectServerMessage::OnCheckMd5, this, std::placeholders::_1);
        m_ActionHandlerMap["reportinfo"] = std::bind(&CConnectServerMessage::OnReportInfo, this, std::placeholders::_1);
        m_ActionHandlerMap["itemdetail"] = std::bind(&CConnectServerMessage::OnItemDetail, this, std::placeholders::_1);
        m_ActionHandlerMap["mediafavorite"] = std::bind(&CConnectServerMessage::OnMediaItemFavorite, this, std::placeholders::_1);
        m_ActionHandlerMap["delmediaitem"] = std::bind(&CConnectServerMessage::OnDelMediaItem, this, std::placeholders::_1);
        m_ActionHandlerMap["broadcastonline"] = std::bind(&CConnectServerMessage::OnBroadCastOnline, this, std::placeholders::_1);
        m_ActionHandlerMap["updatepaishetime"] = std::bind(&CConnectServerMessage::OnUpdatePaiSheTime, this, std::placeholders::_1);
        m_ActionHandlerMap["updategpsaddr"] = std::bind(&CConnectServerMessage::OnUpdateGpsAddr, this, std::placeholders::_1);
        m_ActionHandlerMap["groupnamefromid"] = std::bind(&CConnectServerMessage::OnGroupNameFromID, this, std::placeholders::_1);
        m_ActionHandlerMap["checkfile"] = std::bind(&CConnectServerMessage::OnCheckFile, this, std::placeholders::_1);
        m_ActionHandlerMap["gpslocation"] = std::bind(&CConnectServerMessage::OnGpsLocation, this, std::placeholders::_1);
        m_ActionHandlerMap["setbaiduak"] = std::bind(&CConnectServerMessage::OnSetBaiDuAk, this, std::placeholders::_1);
        m_ActionHandlerMap["setmediapinned"] = std::bind(&CConnectServerMessage::OnSetMediaPinned, this, std::placeholders::_1);
    }
    string strAction = jsonValue["action"];
    std::map<std::string, std::function<string(nlohmann::json&)>>::iterator itor = m_ActionHandlerMap.find(strAction);
    if(itor == m_ActionHandlerMap.end())
    {
        printf("================>Unknow action:%s\n", strAction.c_str());
        string strRet = Server::CCommonUtil::StringFormat("{\"status\":0,\"info\":\"unknow action %s\"}", strAction.c_str());
        strcpy(pszRet, strRet.c_str());
        return;
    }
    string strRet = itor->second(jsonValue);
    strcpy(pszRet, strRet.c_str());
}