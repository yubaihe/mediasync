#include "MediaInfoTable.h"
#include <map>
#include <string>
#include "MediaDb.h"
#include "../Util/Tools.h"
#include "MediaJishuTable.h"
#include "../Util/CommonUtil.h"
#include "../Util/JsonUtil.h"
#include "MediaGroupItemsTable.h"
#include "MediaGroupTable.h"
#include "../GpsManager.h"
#include "CommentTable.h"
CMediaInfoTable::CMediaInfoTable()
{
}

CMediaInfoTable::~CMediaInfoTable()
{
}
BOOL CMediaInfoTable::CreateTable()
{
    //const char*  pszTable = "create table if not exists tbl_mediainfo(id integer PRIMARY KEY autoincrement, " 
    //                                "paishetime text, year integer,month integer, day integer, md5num text, weizhi text, location text, meititype text, meitisize text, devicename text, width text, height text, duration text, mediaaddr text, addtime text, favoritetime text DEFAULT (0), hasextra text DEFAULT (0))";
    //paishetime 拍摄时间
	//year 年份
	//month 月份
	//day 日期
	//md5num     MD5值
	//weizhi     拍摄时候的GPS位置
	//location     拍摄时候的位置
	//meititype  媒体类型(0:图片  1:视频)
	//meitisize  大小(MB GB...)
	//deviceidentify 设备类型(apple,android...)
	//width 宽度
	//height 高度
	//duration 持续时间
	//mediaaddr  媒体本地存储地址
	//addtime    添加时间
    //hasextra    是否包含livephoto 0:否 1:是
    //pinnedtime 置顶时间
    //commentshort 备注前20个字符
    //comment 备注
    map<string, string> param;
    param.insert(pair<string, string>("id", "integer PRIMARY KEY autoincrement"));
    param.insert(pair<string, string>("paishetime", "text DEFAULT '0'")); //虽然是毫秒级别的，但是精确度是分钟
    param.insert(pair<string, string>("year", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("month", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("day", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("md5num", "text DEFAULT ''"));
    param.insert(pair<string, string>("weizhi", "text DEFAULT ''"));
    param.insert(pair<string, string>("location", "text DEFAULT ''"));
    param.insert(pair<string, string>("meititype", "text DEFAULT ''"));
    param.insert(pair<string, string>("meitisize", "text DEFAULT ''"));
    param.insert(pair<string, string>("devicename", "text DEFAULT ''"));
    param.insert(pair<string, string>("width", "text DEFAULT ''"));
    param.insert(pair<string, string>("height", "text DEFAULT ''"));
    param.insert(pair<string, string>("duration", "text DEFAULT ''"));
    param.insert(pair<string, string>("mediaaddr", "text DEFAULT ''"));
    param.insert(pair<string, string>("addtime", "text DEFAULT ''"));
    param.insert(pair<string, string>("favoritetime", "text DEFAULT (0)"));
    param.insert(pair<string, string>("hasextra", "text DEFAULT (0)"));
    param.insert(pair<string, string>("pinnedtime", "text DEFAULT (0)"));
    param.insert(pair<string, string>("commentshort", "text DEFAULT ''"));
    param.insert(pair<string, string>("comment", "text DEFAULT ''"));
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->CreateTable(TABLE_MEDIAINFO, param);
    UNLOCKMEDIADB
    return bRet;
}
list<MediaInfoItem> CMediaInfoTable::AssembleItems(list<map<string, string>> List)
{
    list<MediaInfoItem> retList;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        MediaInfoItem item = {};
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("id"))
            {
                item.iID = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("paishetime"))
            {
                item.iPaiSheTime = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("year"))
            {
                item.iYear = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("month"))
            {
                item.iMonth = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("day"))
            {
                item.iDay = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("md5num"))
            {
                item.strMd5Num = strValue;
            }
            else if(0 == strKey.compare("weizhi"))
            {
                item.strWeiZhi = strValue;
            }
            else if(0 == strKey.compare("location"))
            {
                item.strLocation = strValue.c_str();
            }
            else if(0 == strKey.compare("meititype"))
            {
                item.iMediaType = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("meitisize"))
            {
                item.iMeiTiSize = atoll(strValue.c_str());
            }
            else if(0 == strKey.compare("devicename"))
            {
                item.strDeviceName = strValue;
            }
            else if(0 == strKey.compare("width"))
            {
                item.iWidth = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("height"))
            {
                item.iHeight = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("duration"))
            {
                item.iDuration = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("mediaaddr"))
            {
                item.strAddr = strValue;
            }
            else if(0 == strKey.compare("addtime"))
            {
                item.iAddTime = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("favoritetime"))
            {
                item.iFavoriteTime = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("hasextra"))
            {
                item.iHasExtra = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("pinnedtime"))
            {
                item.iPinnedTime = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("commentshort"))
            {
                item.strCommentShort = strValue;
            }
            else if(0 == strKey.compare("comment"))
            {
                item.strComment = strValue;
            }
        }
        retList.push_back(item);
    }
    return retList;
}
MediaInfoItem CMediaInfoTable::GetItemByID(int iItemID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL("select * from tbl_mediainfo where id='%d'", iItemID);
    UNLOCKMEDIADB
    if(0 == List.size())
    {
        MediaInfoItem item = {};
        item.iID = -1;
        return item;
    }
    list<MediaInfoItem> retList = AssembleItems(List);
    return retList.front();
}
MediaInfoItem CMediaInfoTable::GetItem(CDbCursor& cursor)
{
    MediaInfoItem item = {};
    item.iID = cursor.GetInt("id");
    item.iPaiSheTime = cursor.GetLong("paishetime");
    item.iYear = cursor.GetInt("year");
    item.iMonth = cursor.GetInt("month");
    item.iDay = cursor.GetInt("day");
    item.strMd5Num = cursor.GetString("md5num");
    item.strWeiZhi = cursor.GetString("weizhi");
    item.strLocation = cursor.GetString("location");
    item.iMediaType = cursor.GetInt("meititype");
    item.iMeiTiSize = cursor.GetInt64("meitisize");
    item.strDeviceName = cursor.GetString("devicename");
    item.iWidth = cursor.GetInt("width");
    item.iHeight = cursor.GetInt("height");
    item.iDuration = cursor.GetInt("duration");
    item.strAddr = cursor.GetString("mediaaddr");
    item.iAddTime = cursor.GetLong("addtime");
    item.iFavoriteTime = cursor.GetLong("favoritetime");
    item.iHasExtra = cursor.GetInt("hasextra");
    item.iPinnedTime = cursor.GetLong("pinnedtime");
    item.strCommentShort = cursor.GetString("commentshort");
    item.strComment = cursor.GetString("comment");
    return item;
}
BOOL CMediaInfoTable::CheckMd5Exist(string strMd5Num)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select id from tbl_mediainfo where md5num='%s'", strMd5Num.c_str());
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return FALSE;
    }
    return TRUE;
}
string CMediaInfoTable::FileNameFromMd5(string strMd5Num)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select mediaaddr from tbl_mediainfo where md5num='%s' ", strMd5Num.c_str());
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    return List.front();
}
string CMediaInfoTable::FileNameFromPaiTime(long iPaiTime, string strDevNames, int iMediaType, long iLFileSize)
{
    list<string> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(0 == iLFileSize)
    {
        List = pDbDriver->QuerySQL2("select mediaaddr from tbl_mediainfo where devicename='%s' and paishetime='%ld' and meititype='%d'", strDevNames.c_str(), iPaiTime, iMediaType);
    }
    else
    {
        List = pDbDriver->QuerySQL2("select mediaaddr from tbl_mediainfo where devicename='%s' and paishetime='%ld' and meititype='%d' and meitisize='%ld'", strDevNames.c_str(), iPaiTime, iMediaType, iLFileSize);
    }
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    return List.front();
}
BOOL CMediaInfoTable::AddItem(MediaInfoItem& item)
{
    Server::CTools::SecInfo(item.iPaiSheTime, &(item.iYear), &(item.iMonth), &(item.iDay));
    float dLat = 0;
	float dLong = 0;
	GPSTYPE iGpsType = GPSTYPE_UNKNOW;
    BOOL bGpsCheckOk = Server::CTools::CheckGps(item.strWeiZhi, &iGpsType, &dLong, &dLat);
    if(FALSE == bGpsCheckOk)
    {
        return FALSE;
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("insert into tbl_mediainfo(paishetime, year, month, day, md5num, weizhi, location, meititype, meitisize, devicename,width,height,duration,mediaaddr,addtime,hasextra,pinnedtime,commentshort,comment)values \
												( '%d', '%d', '%d', '%d', '%s', '%s', '%s', '%d', '%ld', '%s', '%d', '%d', '%d', '%s', '%d', '%d', '0', '%s', '%s')",  
    				item.iPaiSheTime, item.iYear, item.iMonth, item.iDay, item.strMd5Num.c_str(), item.strWeiZhi.c_str(), item.strLocation.c_str(),
                    item.iMediaType, item.iMeiTiSize, item.strDeviceName.c_str(), item.iWidth, 
                    item.iHeight, item.iDuration, item.strAddr.c_str(), item.iAddTime, item.iHasExtra, item.strCommentShort.c_str(), item.strComment.c_str());
    item.iID = pDbDriver->GetLastInsertID();
     UNLOCKMEDIADB
     //计数加 1
     CMediaJishuTable mediaJiShu;
     mediaJiShu.Increase(item.iPaiSheTime, item.iMediaType, item.strDeviceName.c_str());
     if(item.strLocation.length() == 0)
     {
        CGpsManager::GetInstance()->NeedCheck();
     }
     //备注也要加上去
     if(item.strCommentShort.length() > 0)
    {
        CCommentTable::AddItem(item.iID, item.strComment);
    }
     return bRet;
}
int CMediaInfoTable::GetRecordCount()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select count(id) as recordcount from tbl_mediainfo");
    UNLOCKMEDIADB
    if(0 == List.size())
    {
        return 0;
    }
    return atoi(List.front().c_str());
}
MediaInfoItem CMediaInfoTable::GetLatestItem()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL("select * from tbl_mediainfo where paishetime in(select max(paishetime) from tbl_mediainfo)");
    UNLOCKMEDIADB
    list<MediaInfoItem> retList = AssembleItems(List);
    if(retList.size() == 0)
    {
        MediaInfoItem item = {};
        item.iID = -1;
        return item;
    }
    return retList.front();
}
int CMediaInfoTable::GetItemPaiTime(int iID)
{
    MediaInfoItem mediaInfoItem = GetItemByID(iID);
    return mediaInfoItem.iPaiSheTime;
}
int CMediaInfoTable::GetRecentRecordCount(uint32_t iType, string strDevNames)
{
    int iCurTime = Server::CTools::CurTimeSec();
    int iLimitTime = iCurTime - 7*24*60*60;
    list<string> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(0 == strDevNames.length())
    {
        List = pDbDriver->QuerySQL2("select count(id) as mediacount from tbl_mediainfo where addtime > '%d' and meititype='%d' ", iLimitTime, iType);
    }
    else
    {
        List = pDbDriver->QuerySQL2("select count(id) as mediacount from tbl_mediainfo where addtime > '%d' and meititype='%d' and devicename in('%s')", iLimitTime, iType, strDevNames.c_str());
    }
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return 0;
    }
    return atoi(List.front().c_str());
}
string CMediaInfoTable::GetRecentRecords(int iStart, int iLimit, string strDevNames)
{
    int iCurTime = Server::CTools::CurTimeSec();
    int iLimitTime = iCurTime - 7*24*60*60;
    list<map<string,string>> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(0 == strDevNames.length())
    {
        List = pDbDriver->QuerySQL("select id,width,height,duration,meititype,meitisize,mediaaddr,hasextra from tbl_mediainfo where addtime > '%d' order by addtime desc limit %d offset %d", iLimitTime, iLimit, iStart);
    }
    else
    {
        List = pDbDriver->QuerySQL("select id,width,height,duration,meititype,meitisize,mediaaddr,hasextra from tbl_mediainfo where addtime > '%d' and devicename in ('%s') order by addtime desc limit %d offset %d", iLimitTime, strDevNames.c_str(), iLimit, iStart);
    }
    UNLOCKMEDIADB
    list<MediaInfoItem> retList = AssembleItems(List);
    nlohmann::json jsonRoot = nlohmann::json::array();
    for(list<MediaInfoItem>::iterator itor = retList.begin(); itor != retList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        jsonItem["id"] = itor->iID;
        jsonItem["w"] = itor->iWidth;
        jsonItem["h"] = itor->iHeight;
        jsonItem["dur"] = itor->iDuration;
        jsonItem["mtype"] = itor->iMediaType;
        jsonItem["msize"] = itor->iMeiTiSize;
        jsonItem["maddr"] = itor->strAddr;
        jsonItem["hasextra"] = itor->iHasExtra;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}
string CMediaInfoTable::GetFavoriteRecords(int iStart, int iLimit, string strDevNames)
{
    list<map<string,string>> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(0 == strDevNames.length())
    {
        List = pDbDriver->QuerySQL("select id,width,height,duration,meititype,meitisize,mediaaddr,hasextra from tbl_mediainfo where favoritetime > '0' order by favoritetime desc limit %d offset %d", iLimit, iStart);
    }
    else
    {
        List = pDbDriver->QuerySQL("select id,width,height,duration,meititype,meitisize,mediaaddr,hasextra from tbl_mediainfo where favoritetime > '0' and devicename in ('%s') order by favoritetime desc limit %d offset %d", strDevNames.c_str(), iLimit, iStart);
    }
    UNLOCKMEDIADB
    list<MediaInfoItem> retList = AssembleItems(List);
    nlohmann::json jsonRoot = nlohmann::json::array();
    for(list<MediaInfoItem>::iterator itor = retList.begin(); itor != retList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        jsonItem["id"] = itor->iID;
        jsonItem["w"] = itor->iWidth;
        jsonItem["h"] = itor->iHeight;
        jsonItem["dur"] = itor->iDuration;
        jsonItem["mtype"] = itor->iMediaType;
        jsonItem["msize"] = itor->iMeiTiSize;
        jsonItem["maddr"] = itor->strAddr;
        jsonItem["hasextra"] = itor->iHasExtra;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}

string CMediaInfoTable::GetYearRecords(int iStart, int iLimit, int iYear, int iMonth, int iDay, string strDevNames, string strLocation, BOOL bLocation)
{
    string strSQL = "select * from tbl_mediainfo where ";
    if(iYear > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("year=%d and ", iYear));
    }
    if(iMonth > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("month=%d and ", iMonth));
    }
    if(iDay > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("day=%d and ", iDay));
    }
    if(true == bLocation)
    {
        //如果是位置，即使strLocation是空的也得加上 因为待定位的location是空的
        strSQL.append(Server::CCommonUtil::StringFormat("location='%s' and ", strLocation.c_str()));
    }
    else if(strLocation.length() != 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("location='%s' and ", strLocation.c_str()));
    }
    if(strDevNames.length() != 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("devicename in ('%s') and ", strDevNames.c_str()));
    }
    strSQL.append(Server::CCommonUtil::StringFormat("1=1 order by pinnedtime desc, paishetime desc limit %d offset %d", iLimit, iStart));
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string,string>> List = pDbDriver->QuerySQL(strSQL.c_str());
    UNLOCKMEDIADB
    list<MediaInfoItem> retList = AssembleItems(List);
    nlohmann::json jsonRoot = nlohmann::json::array();
    for(list<MediaInfoItem>::iterator itor = retList.begin(); itor != retList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        jsonItem["id"] = itor->iID;
        jsonItem["w"] = itor->iWidth;
        jsonItem["h"] = itor->iHeight;
        jsonItem["dur"] = itor->iDuration;
        jsonItem["mtype"] = itor->iMediaType;
        jsonItem["msize"] = itor->iMeiTiSize;
        jsonItem["maddr"] = itor->strAddr;
        jsonItem["hasextra"] = itor->iHasExtra;
        jsonItem["pinnedtime"] = itor->iPinnedTime;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}
string CMediaInfoTable::GetLocationGroup(int iStart, int iLimit, int iYear, int iMonth, int iDay, string strDevNames)
{
    string strSQL = "Select weizhi,location,Count(id) As Count from tbl_mediainfo where ";
    if(iYear > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("year=%d and ", iYear));
    }
    if(iMonth > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("month=%d and ", iMonth));
    }
    if(iDay > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("day=%d and ", iDay));
    }
    if(strDevNames.length() != 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("devicename in ('%s') and ", strDevNames.c_str()));
    }
    strSQL.append(Server::CCommonUtil::StringFormat("1=1 group by location limit %d offset %d", iLimit, iStart));
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string,string>> List = pDbDriver->QuerySQL(strSQL.c_str());
    UNLOCKMEDIADB
    
    nlohmann::json jsonRoot = nlohmann::json::array();
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        string strWeiZhi = "";
        string strLocation = "";
        int iTotalCount = 0;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("weizhi"))
            {
                strWeiZhi = strValue;
            }
            else if(0 == strKey.compare("location"))
            {
                strLocation = strValue;
            }
            else if(0 == strKey.compare("Count"))
            {
                iTotalCount = atoi(strValue.c_str());
            }
        }
        nlohmann::json jsonItem;
        jsonItem["weizhi"] = strWeiZhi;
        jsonItem["location"] = strLocation;
        jsonItem["count"] = iTotalCount;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}
/***
 * 通过ID查询媒体信息
 */
string CMediaInfoTable::RecordsFromIds(string strIds, string strSort)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string,string>> List = pDbDriver->QuerySQL("select id,width,height,duration,meititype,meitisize,mediaaddr,hasextra from tbl_mediainfo where id in('%s') ORDER BY INSTR('%s',id)", strIds.c_str(), strSort.c_str());
    UNLOCKMEDIADB
    list<MediaInfoItem> retList = AssembleItems(List);
    nlohmann::json jsonRoot = nlohmann::json::array();
    for(list<MediaInfoItem>::iterator itor = retList.begin(); itor != retList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        jsonItem["id"] = itor->iID;
        jsonItem["w"] = itor->iWidth;
        jsonItem["h"] = itor->iHeight;
        jsonItem["dur"] = itor->iDuration;
        jsonItem["mtype"] = itor->iMediaType;
        jsonItem["msize"] = itor->iMeiTiSize;
        jsonItem["maddr"] = itor->strAddr;
        jsonItem["hasextra"] = itor->iHasExtra;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}
string CMediaInfoTable::GetItemIDSql(int iID, BOOL bNext, string strOtype, string strDevNames, int iGid, int iYear, int iMonth, int iDay, string strLocation)
{
    if(TRUE == bNext)
    {
        printf("Next\n");
    }
    else
    {
        printf("Prev\n");
    }
    //recent  => 不需要任何参数
	//favorite  => 不需要任何参数
	//group   => gid
    //year  => year month day location
    int iDevNamesLen = strDevNames.length();
    MediaInfoItem mediaInfoItem = GetItemByID(iID);
    
    
    string strSQL = "";
    if(0 == strOtype.compare("recent"))
    {
        long iAddTimeSec = mediaInfoItem.iAddTime;
        int iCurTime = Server::CTools::CurTimeSec();
        int iLimitTime = iCurTime - 7*24*60*60;
        //最近
        if(TRUE == bNext)
        {
            if(iDevNamesLen == 0)
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where addtime <= %d and addtime > '%d' order by addtime desc", iAddTimeSec, iLimitTime);
            }
            else
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where addtime <= %d and devicename in ('%s') and addtime > '%d' order by addtime desc", iAddTimeSec, strDevNames.c_str(), iLimitTime);
            }
        }
        else
        {
            
            if(iDevNamesLen == 0)
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where addtime >= %d and addtime > '%d' order by addtime desc", iAddTimeSec, iLimitTime);
            }
            else
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where addtime >= %d and devicename in ('%s') and addtime > '%d' order by addtime desc", iAddTimeSec, strDevNames.c_str(), iLimitTime);
            }
        }
    }
    else if(0 == strOtype.compare("favorite"))
    {
        long iFaviroteTimeSec = mediaInfoItem.iFavoriteTime;
        //收藏
        if(TRUE == bNext)
        {
            if(iDevNamesLen == 0)
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where favoritetime <= '%ld' and favoritetime > '0' order by favoritetime desc ", iFaviroteTimeSec);     
            }
            else
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where favoritetime <= '%ld' and favoritetime > '0' and devicename in ('%s') order by favoritetime desc", iFaviroteTimeSec, strDevNames.c_str()); 
            }
        }
        else
        {
            if(iDevNamesLen == 0)
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where favoritetime >= '%ld' order by favoritetime desc ", iFaviroteTimeSec);     
            }
            else
            {
                strSQL = Server::CCommonUtil::StringFormat("select id as mediaitemid from tbl_mediainfo where favoritetime >= '%ld' and devicename in ('%s') order by favoritetime desc ", iFaviroteTimeSec, strDevNames.c_str()); 
            }
        }
    }
    else if(0 == strOtype.compare("group"))
    {
        //分组
        if(TRUE == bNext)
        {
            strSQL = CMediaGroupItemsTable::GetNextItemIDSql(iID, iGid, strDevNames);
        }
        else
        {
            strSQL = CMediaGroupItemsTable::GetPrevItemIDSql(iID, iGid, strDevNames);
        }
    }
    else
    {
        // long iPaiTimeSec = mediaInfoItem.iPaiSheTime;
        //年月日
        strSQL = "select id as mediaitemid from tbl_mediainfo where ";
        if(iYear > 0)
        {
            strSQL.append(Server::CCommonUtil::StringFormat("year=%d and ", iYear));
        }
        if(iMonth > 0)
        {
            strSQL.append(Server::CCommonUtil::StringFormat("month=%d and ", iMonth));
        }
        if(iDay > 0)
        {
            strSQL.append(Server::CCommonUtil::StringFormat("day=%d and ", iDay));
        }
        if(strOtype == "location")
        {
            //足迹的话 一定会带location的 如果location为空表示GPS未定位 所以对于location类型的查询必带Location
            strSQL.append(Server::CCommonUtil::StringFormat("location='%s' and ", strLocation.c_str()));
        }
        else if(strLocation.length() != 0)
        {
            strSQL.append(Server::CCommonUtil::StringFormat("location='%s' and ", strLocation.c_str()));
        }
        if(iDevNamesLen != 0)
        {
            strSQL.append(Server::CCommonUtil::StringFormat("devicename in ('%s') and ", strDevNames.c_str()));
        }
        if(TRUE == bNext)
        {
            strSQL.append("1=1 order by pinnedtime desc, paishetime desc");
        }
        else
        {
            strSQL.append("1=1 order by pinnedtime desc, paishetime desc");
        }
    }
    return strSQL;
}
int CMediaInfoTable::GetPrevItemID(int iID, string strOtype, string strDevNames, int iGid, int iYear, int iMonth, int iDay, string strLocation)
{
    string strSQL = CMediaInfoTable::GetItemIDSql(iID, FALSE, strOtype, strDevNames, iGid, iYear, iMonth, iDay, strLocation);
    CDbDriver* pDbDriver = LOCKMEDIADB
    CDbCursor cursor;
    pDbDriver->QuerySQL(cursor, strSQL.c_str());
    UNLOCKMEDIADB
    int iCount = cursor.GetCount();
    printf("GetPrevItemID count:%d\n", iCount);
    if(iCount == 0)
    {
        return -1;
    }
    if(iCount == 1)
    {
        cursor.Next();
        int iCurID = cursor.GetInt("mediaitemid");
        return iCurID;
    }
    int iRetID = -1;
    while(TRUE == cursor.Next())
    {
        int iCurID = cursor.GetInt("mediaitemid");
        if(iCurID == iID)
        {
            break;
        }
        else
        {
            iRetID = iCurID;
        }
    }
    return iRetID;
}
int CMediaInfoTable::GetNextItemID(int iID, string strOtype, string strDevNames, int iGid, int iYear, int iMonth, int iDay, string strLocation)
{
    string strSQL = CMediaInfoTable::GetItemIDSql(iID, TRUE, strOtype, strDevNames, iGid, iYear, iMonth, iDay, strLocation);
    CDbDriver* pDbDriver = LOCKMEDIADB
    CDbCursor cursor;
    pDbDriver->QuerySQL(cursor, strSQL.c_str());
    UNLOCKMEDIADB
    int iCount = cursor.GetCount();
    printf("GetNextItemID count:%d\n", iCount);
    if(iCount == 0)
    {
        return -1;
    }
    if(iCount == 1)
    {
        cursor.Next();
        int iCurID = cursor.GetInt("mediaitemid");
        return iCurID;
    }
    int iRetID = -1;
    BOOL bGetNext = FALSE;
    while(TRUE == cursor.Next())
    {
        int iCurID = cursor.GetInt("mediaitemid");
        if(iCurID == iID)
        {
            bGetNext = TRUE;
        }
        else
        {
            if(TRUE == bGetNext)
            {
                iRetID = iCurID;
                break;
            }
        }
    }
    return iRetID;
}
string CMediaInfoTable::GetItem(int iID, BOOL& bGet)
{
    bGet = TRUE;
    MediaInfoItem item = GetItemByID(iID);
    if(item.iID < 0)
    {
        bGet = FALSE;
    }
    nlohmann::json jsonItem;
    jsonItem["id"] = item.iID;
    jsonItem["w"] = item.iWidth;
    jsonItem["h"] = item.iHeight;
    jsonItem["dur"] = item.iDuration;
    jsonItem["mtype"] = item.iMediaType;
    jsonItem["msize"] = item.iMeiTiSize;
    jsonItem["maddr"] = item.strAddr;
    jsonItem["device"] = item.strDeviceName;
    jsonItem["paitime"] = item.iPaiSheTime;
    jsonItem["weizhi"] = item.strWeiZhi;
    jsonItem["location"] = item.strLocation;
    jsonItem["addtime"] = item.iAddTime;
    jsonItem["favoritetime"] = item.iFavoriteTime;
    jsonItem["hasextra"] = item.iHasExtra;
    jsonItem["pinnedtime"] = item.iPinnedTime;
    jsonItem["commentshort"] = item.strCommentShort;
    string strJson = Server::CJsonUtil::ToString(jsonItem);
    return strJson;
}
BOOL CMediaInfoTable::UpdateFavorite(int iID, BOOL bFavorite)
{
    time_t iTimeSec = bFavorite == TRUE?Server::CTools::CurTimeSec():0;
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediainfo set favoritetime='%d' where id='%d'", iTimeSec, iID);
    UNLOCKMEDIADB
    return bRet;
}
int CMediaInfoTable::GetRecordFavoriteCount(uint32_t iType, string strDevNames)
{
    list<string> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strDevNames.length() == 0)
    {
        List = pDbDriver->QuerySQL2("select count(id) as mediacount from tbl_mediainfo where favoritetime > '0' and meititype='%d' ", iType);
    }
    else
    {
        List = pDbDriver->QuerySQL2("select count(id) as mediacount from tbl_mediainfo where favoritetime > '0' and meititype='%d' and devicename in('%s')", iType, strDevNames.c_str());
    }
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return -1;
    }
    return atoi(List.front().c_str());
}
BOOL CMediaInfoTable::RemoveItem(int iID)
{
     //计数 -1
    CMediaJishuTable::DecreaseFromID(iID);
    //Groupitems表也要删除
    CMediaGroupItemsTable::RemoveFromItemID(iID);
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediainfo where id='%d'", iID);
    UNLOCKMEDIADB
    //备注也要删除一下
    CCommentTable::DeleteItem(iID);
    return bRet;
}
MediaInfoItem CMediaInfoTable::GetItemFromName(string strName)
{
    MediaInfoItem retItem = {};
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string,string>> List = pDbDriver->QuerySQL("select * from tbl_mediainfo where mediaaddr='%s'", strName.c_str());
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        retItem.iID = -1;
        return retItem;
    }
    list<MediaInfoItem> retList = AssembleItems(List);
    return retList.front();
}
BOOL CMediaInfoTable::RemoveItemFromName(string strName)
{
    printf("CMediaInfoTable::RemoveItemFromName:%s\n", strName.c_str());
    MediaInfoItem item = GetItemFromName(strName);
    if(item.iID < 0)
    {
        return FALSE;
    }
    //计数 -1
    CMediaJishuTable::Decrease(item.iPaiSheTime, item.iMediaType, item.strDeviceName);
    //group也需要删除
    CMediaGroupItemsTable::RemoveFromItemID(item.iID);
    //设置group封面为空
    CMediaGroupTable::SetCoverEmpty(item.strAddr);
    //把自己删除掉
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediainfo where mediaaddr='%s'", strName.c_str());
    UNLOCKMEDIADB
    //备注也要删除一下
    CCommentTable::DeleteItem(item.iID);
    return bRet;
}
string CMediaInfoTable::GetFileName(int iID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select mediaaddr from tbl_mediainfo where id='%d'", iID);
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    return List.front();
}
list<string> CMediaInfoTable::GetUnCheckWeiZhi(int iLimit)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select distinct(weizhi) from tbl_mediainfo where location='' limit %d offset 0", iLimit);
    UNLOCKMEDIADB
    return List;
}
BOOL CMediaInfoTable::UpdateGpsWeiZhi(string strGps, string strBaiDuGps, string strLocation)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediainfo set location='%s', weizhi='%s' where weizhi='%s'", strLocation.c_str(), strBaiDuGps.c_str(), strGps.c_str());
    UNLOCKMEDIADB
    return bRet;
}
string CMediaInfoTable::YearLocationGroupTongJi(string strLocation, int iYear, int iMonth, int iDay, string strDevNames)
{
    string strSQL = "SELECT meititype, COUNT(*) as tcount from tbl_mediainfo where ";
    if(iYear > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("year=%d and ", iYear));
    }
    if(iMonth > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("month=%d and ", iMonth));
    }
    if(iDay > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("day=%d and ", iDay));
    }
    if(strLocation.length() != 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("location='%s' and ", strLocation.c_str()));
    }
    if(strDevNames.length() != 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("devicename in ('%s') and ", strDevNames.c_str()));
    }
    strSQL.append(" 1=1 GROUP BY meititype");
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL(strSQL.c_str());
    UNLOCKMEDIADB
    nlohmann::json jsonRoot = nlohmann::json::array();
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        int iMeidaType = 0;
        int iMediaCount = 0;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("meititype"))
            {
                iMeidaType = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("tcount"))
            {
                iMediaCount = atoi(strValue.c_str());
            }
        }
        nlohmann::json jsonItem;
        jsonItem["type"] = iMeidaType;
        jsonItem["count"] = iMediaCount;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}
BOOL CMediaInfoTable::UpdateItemPaiSheShiJian(int iID, int iTime, string strNewFileName)
{
    uint16_t iYear = 0;
    uint8_t iMonth = 0;
    uint8_t iDay = 0;
    Server::CTools::SecInfo(iTime, &iYear, &iMonth, &iDay);
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediainfo set paishetime='%d',year='%d',month='%d',day='%d',mediaaddr='%s' where id='%d'", iTime, iYear, iMonth, iDay, strNewFileName.c_str(), iID);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaInfoTable::UpdateItemGpsAddr(int iID, string strGps, string strAddr)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediainfo set weizhi='%s', location='%s' where id='%d'", strGps.c_str(), strAddr.c_str(), iID);
    UNLOCKMEDIADB
    return bRet;
}
string CMediaInfoTable::YearMonthDayCover(int iYear, int iMonth, int iDay, string strDevNames)
{
    if(iYear <= 0)
    {
        return FALSE;
    }
    string strSQL = "select mediaaddr from tbl_mediainfo where ";
    if(iYear > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("year=%d and ", iYear));
    }
    if(iMonth > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("month=%d and ", iMonth));
    }
    if(iDay > 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("day=%d and ", iDay));
    }
    if(strDevNames.length() != 0)
    {
        strSQL.append(Server::CCommonUtil::StringFormat("devicename in ('%s') and ", strDevNames.c_str()));
    }
    strSQL.append(" 1=1 order by pinnedtime desc, paishetime desc limit 0,1");
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2(strSQL.c_str());
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    return List.front();
}
string CMediaInfoTable::GetGroupInfoFomItemID(int iID)
{
    string strGids = CMediaGroupItemsTable::GidsFromMediaItemID(iID);
    string strNewGids = Server::CTools::ReplaceString(strGids, "&", "','");
    //{id:1,name:"11"},{id:2,name:"22"}
    return CMediaGroupTable::GetJsonAllGroupsFromGids(strNewGids);
}

BOOL CMediaInfoTable::SetPinned(int iID, BOOL bPinned)
{
    time_t iTimeSec = 0;
    if(TRUE == bPinned)
    {
        iTimeSec = Server::CTools::CurTimeSec();
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediainfo set pinnedtime='%ld' where id='%d'", iTimeSec, iID);
    UNLOCKMEDIADB
    return bRet;
}
list<int> CMediaInfoTable::GetTodayYear(int iMonth, int iDay, string strDevNames)
{
    list<string> yearList;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strDevNames.length() > 0)
    {
        yearList = pDbDriver->QuerySQL2("select DISTINCT(year) from tbl_mediainfo where month='%d' and day='%d' and devicename in ('%s') ORDER BY paishetime DESC", iMonth, iDay, strDevNames.c_str());
    }
    else
    {
        yearList = pDbDriver->QuerySQL2("select DISTINCT(year) from tbl_mediainfo where month='%d' and day='%d' ORDER BY paishetime DESC", iMonth, iDay);
    }
    UNLOCKMEDIADB
    list<int> retList;
    list<string>::iterator itor = yearList.begin();
    for(; itor != yearList.end(); ++itor)
    {
        retList.push_back(atoi((*itor).c_str()));
    }
    return retList;
}
MediaInfoItem CMediaInfoTable::GetFirstMediaInDay(int iYear, int iMonth, int iDay, string strDevNames)
{
    list<map<string, string>> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strDevNames.length() > 0)
    {
        List = pDbDriver->QuerySQL("select * from tbl_mediainfo where year='%d' and month='%d' and day='%d' and devicename in ('%s') ORDER BY paishetime DESC LIMIT 1", iYear, iMonth, iDay, strDevNames.c_str());
    }
    else
    {
        List = pDbDriver->QuerySQL("select * from tbl_mediainfo where year='%d' and month='%d' and day='%d' ORDER BY paishetime DESC LIMIT 1", iYear, iMonth, iDay);
    }
    UNLOCKMEDIADB
    if(0 == List.size())
    {
        MediaInfoItem item = {};
        item.iID = -1;
        return item;
    }
    list<MediaInfoItem> retList = AssembleItems(List);
    return retList.front();
}
BOOL CMediaInfoTable::ChangeMediaAddr(int iID, string strAddr, string strMd5)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediainfo set mediaaddr='%s', md5num='%s' where id='%d'", strAddr.c_str(), strMd5.c_str(), iID);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaInfoTable::GetComment(int iID, string& strCommentShort, string& strComment)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL("select * from tbl_mediainfo where id='%d'", iID);
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return FALSE;
    }
    list<MediaInfoItem> itemList = AssembleItems(List);
    if(itemList.size() > 0)
    {
        MediaInfoItem item = itemList.front();
        strCommentShort = item.strCommentShort;
        strComment = item.strComment;
        return TRUE;
    }
    return FALSE;
}
BOOL CMediaInfoTable::UpdateComment(int iID, string strCommentShort, string strComment)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediainfo set commentshort='%s', comment='%s' where id='%d'", strCommentShort.c_str(), strComment.c_str(), iID);
    UNLOCKMEDIADB
    if(strCommentShort.length() > 0)
    {
        CCommentTable::AddItem(iID, strComment);
    }
    else
    {
        CCommentTable::DeleteItem(iID);
    }
    return bRet;
}