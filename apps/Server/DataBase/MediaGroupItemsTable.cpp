
#include "MediaGroupItemsTable.h"
#include "MediaDb.h"
#include "MediaInfoTable.h"
#include "../Util/CommonUtil.h"
CMediaGroupItemsTable::CMediaGroupItemsTable()
{
}

CMediaGroupItemsTable::~CMediaGroupItemsTable()
{
}
BOOL CMediaGroupItemsTable::CreateTable()
{
    //const char*  pszGroupItemsTable = "create table if not exists tbl_mediagroupitems(id integer PRIMARY KEY autoincrement, gid integer, mediaid integer, meititype integer,deviceidentify text)";
    //gid 分组ID
	//mediaid    媒体文件的唯一ID 
	//meititype 媒体文件的类型
	//deviceidentify 设备标示
    map<string, string> param;
    param.insert(pair<string, string>("id", "integer PRIMARY KEY autoincrement"));
    param.insert(pair<string, string>("gid", "integer DEFAULT'0'"));
    param.insert(pair<string, string>("mediaid", "integer DEFAULT'0'"));
    param.insert(pair<string, string>("meititype", "integer DEFAULT'0'"));
    param.insert(pair<string, string>("deviceidentify", "text DEFAULT''"));
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->CreateTable(TABLE_MEDIAGROUPITEMS, param);
    UNLOCKMEDIADB
    return bRet;
}
list<MediaGroupItemOne> CMediaGroupItemsTable::AssembleItems(list<map<string, string>> List)
{
    list<MediaGroupItemOne> retList;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        MediaGroupItemOne item = {};
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("id"))
            {
                item.iID = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("gid"))
            {
                item.iGID = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("mediaid"))
            {
                item.iMediaID = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("meititype"))
            {
                item.iMediaType = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("deviceidentify"))
            {
                item.strDeviceIdentify = strValue;
            }
        }
        retList.push_back(item);
    }
    return retList;
}
BOOL CMediaGroupItemsTable::AddItem(MediaGroupItemOne item)
{
    MediaGroupItemOne findItem = GetItem(item.iGID, item.iMediaID);
    if(findItem.iID > 0)
    {
        return TRUE;
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("insert into tbl_mediagroupitems(gid,mediaid,meititype,deviceidentify)values('%d','%d','%d','%s')",
        item.iGID, item.iMediaID, item.iMediaType, item.strDeviceIdentify.c_str());
    UNLOCKMEDIADB
    return bRet;
}
MediaGroupItemOne CMediaGroupItemsTable::GetItem(int iGID, int iItemID)
{
    MediaGroupItemOne retItem = {};
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL("select * from tbl_mediagroupitems where gid='%d' and mediaid='%d'", iGID, iItemID);
    UNLOCKMEDIADB
    list<MediaGroupItemOne> retList = AssembleItems(List);
    if(0 == List.size())
    {
        retItem.iID = -1;
        return retItem;
    }
    retItem = retList.front();
    return retItem;
}
BOOL CMediaGroupItemsTable::DelItem(int iGID, int iItemID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediagroupitems where gid='%d' and mediaid='%d'", iGID, iItemID);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaGroupItemsTable::Update(int iItemID, int iFromID, int iToID)
{
    BOOL bRet = DelItem(iFromID, iItemID);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    CMediaInfoTable mediaInfoTable;
    MediaInfoItem mediaItem = mediaInfoTable.GetItemByID(iItemID);
    if(mediaItem.iID <= 0)
    {
        return FALSE;
    }
    MediaGroupItemOne item = {0};
    item.iGID = iToID;
    item.iMediaID = iItemID;
    item.iMediaType = mediaItem.iMediaType;
    return AddItem(item);
}
BOOL CMediaGroupItemsTable::Detail(int iGID, string strDeviceName, int* piPicCount, int* piVideoCount)
{
    list<map<string, string>> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(0 == strDeviceName.length())
    {
        List = pDbDriver->QuerySQL("select count(*) as tcount, meititype from tbl_mediagroupitems where gid = '%d' group by meititype", iGID);
    }
    else
    {
        List = pDbDriver->QuerySQL("select count(*) as tcount, meititype from tbl_mediagroupitems where gid = '%d' and deviceidentify in('%s') group by meititype", iGID, strDeviceName.c_str());
    }
    UNLOCKMEDIADB
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        MediaGroupItemOne item = {};
        int iMediaType = MEDIATYPE_IMAGE;
        int iMediaCount = 0;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("meititype"))
            {
                iMediaType = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("tcount"))
            {
                iMediaCount = atoi(strValue.c_str());
            }
        }
        if(MEDIATYPE_IMAGE == iMediaType)
        {
            *piPicCount = iMediaCount;
        }
        else
        {
            *piVideoCount = iMediaCount;
        }
    }
    return TRUE;
}
string CMediaGroupItemsTable::GidsFromMediaItemID(int iMediaItemID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select gid from tbl_mediagroupitems where mediaid='%d'", iMediaItemID);
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    return Server::CCommonUtil::ListToString(List, "&");
}
BOOL CMediaGroupItemsTable::RemoveFromItemID(int iMediaItemID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediagroupitems where mediaid='%d'", iMediaItemID);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaGroupItemsTable::RemoveFromGroupID(int iGID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediagroupitems where gid='%d'", iGID);
    UNLOCKMEDIADB
    return bRet;
}
string CMediaGroupItemsTable::MediaIds(int iStart, int iLimit, int iGID, string strDeviceNames)
{
    list<string> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(0 == strDeviceNames.length())
    {
        List = pDbDriver->QuerySQL2("select mediaid from tbl_mediagroupitems where gid = '%d' order by id desc limit %d offset %d", iGID, iLimit, iStart);
    }
    else
    {
        List = pDbDriver->QuerySQL2("select mediaid from tbl_mediagroupitems where gid = '%d' and deviceidentify in('%s') order by id desc limit %d offset %d", iGID, strDeviceNames.c_str(), iLimit,iStart);
    }
    UNLOCKMEDIADB
    return Server::CCommonUtil::ListToString(List, "&");
}
BOOL CMediaGroupItemsTable::ClearCacheRecord()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediagroupitems where gid not in(select id from tbl_mediagroup)");
    if(FALSE == bRet)
    {
        UNLOCKMEDIADB
        return FALSE;
    }
    bRet = pDbDriver->ExecuteSQL("delete from tbl_mediagroupitems where mediaid not in(select id from tbl_mediainfo)");
    if(FALSE == bRet)
    {
        UNLOCKMEDIADB
        return FALSE;
    }
    UNLOCKMEDIADB
    return bRet;
}
int CMediaGroupItemsTable::MediaItemCount(uint32_t iGID, string strDeviceNames)
{
    list<string> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(0 == strDeviceNames.length())
    {
        List = pDbDriver->QuerySQL2("select count(mediaid) as mcount from tbl_mediagroupitems where gid = '%d' ", iGID);
    }
    else
    {
        List = pDbDriver->QuerySQL2("select count(mediaid) as mcount from tbl_mediagroupitems where gid = '%d' and deviceidentify in('%s') ", iGID, strDeviceNames.c_str());
    }
    UNLOCKMEDIADB
    return atoi(List.front().c_str());
}
string CMediaGroupItemsTable::GidsFromDevNames(string strDeviceNames)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select distinct(gid) from tbl_mediagroupitems where deviceidentify in('%s')", strDeviceNames.c_str());
    UNLOCKMEDIADB
    return Server::CCommonUtil::ListToString(List, "&");
}
string CMediaGroupItemsTable::GetPrevItemIDSql(int iMediaItemID, int iGID, string strDeviceNames)
{
    string strSQL = "";
    MediaGroupItemOne findItem = GetItem(iGID, iMediaItemID);
    if(strDeviceNames.length() == 0)
    {
        strSQL = Server::CCommonUtil::StringFormat("select mediaid as mediaitemid from tbl_mediagroupitems where gid=%d and id>%d order by id asc LIMIT 1", iGID, findItem.iID);
    }
    else
    {
        strSQL = Server::CCommonUtil::StringFormat("select mediaid as mediaitemid from tbl_mediagroupitems where gid=%d and id>%d and deviceidentify in('%s') order by id asc LIMIT 1", iGID, findItem.iID, strDeviceNames.c_str());
    }
    return strSQL;
}
string CMediaGroupItemsTable::GetNextItemIDSql(int iMediaItemID, int iGID, string strDeviceNames)
{
    string strSQL = "";
    MediaGroupItemOne findItem = GetItem(iGID, iMediaItemID);
    if(strDeviceNames.length() == 0)
    {
        strSQL = Server::CCommonUtil::StringFormat("select mediaid as mediaitemid from tbl_mediagroupitems where gid=%d and id<%d order by id desc limit 1", iGID, findItem.iID);
    }
    else
    {
        strSQL = Server::CCommonUtil::StringFormat("select mediaid as mediaitemid from tbl_mediagroupitems where gid=%d and id<%d and deviceidentify in('%s') order by id desc LIMIT 1", iGID, findItem.iID, strDeviceNames.c_str());
    }
    return strSQL;
}