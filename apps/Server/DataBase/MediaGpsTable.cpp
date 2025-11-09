#include "MediaGpsTable.h"
#include "MediaDb.h"
#include "../Util/CommonUtil.h"
#include "../GpsManager.h"
CMediaGpsTable::CMediaGpsTable()
{
}

CMediaGpsTable::~CMediaGpsTable()
{
}
BOOL CMediaGpsTable::CreateTable()
{
    map<string, string> param;
    param.insert(pair<string, string>("gps", "text DEFAULT''"));
    param.insert(pair<string, string>("gps2", "text DEFAULT''"));
    param.insert(pair<string, string>("gpslocation", "text DEFAULT''"));
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->CreateTable(TABLE_MEDIAGPS, param);
    UNLOCKMEDIADB
    return bRet;
}
list<MediaGpsItem> CMediaGpsTable::AssembleItems(list<map<string, string>> List)
{
    list<MediaGpsItem> retList;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        MediaGpsItem item = {};
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("gps"))
            {
                item.strGps = strValue;
            }
            else if(0 == strKey.compare("gps2"))
            {
                item.strGps2 = strValue;
            }
            else if(0 == strKey.compare("gpslocation"))
            {
                item.strLocation = strValue;
            }
        }
        retList.push_back(item);
    }
    return retList;
}
BOOL CMediaGpsTable::QueryItem(string strGps, MediaGpsItem& item)
{
    GPSTYPE eRetGpsType = GPSTYPE_NORMAL;
    CGpsManager gpsManager;
    gpsManager.ParseGps(strGps, &eRetGpsType);
    switch (eRetGpsType)
    {
        case GPSTYPE_NORMAL:
        {
            return GetRecordFromGps(strGps, item);
            break;
        }
        case GPSTYPE_BAIDU:
        {
            return GetRecordFromGps2(strGps, item);
            break;
        }
        default:
        {
            return FALSE;
            break;
        }
    }
}
BOOL CMediaGpsTable::GetRecordFromGps(string strGps, MediaGpsItem& item)
{
    item = {};
    if(strGps.length() == 0)
    {
        return TRUE;
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> itemList = pDbDriver->QuerySQL("select * from %s where gps='%s'", TABLE_MEDIAGPS, strGps.c_str());
    UNLOCKMEDIADB
    list<MediaGpsItem> ItemsList = AssembleItems(itemList);
    if(ItemsList.size() == 0)
    {
        return FALSE;
    }
    
    item = ItemsList.front();
    return TRUE;
}
BOOL CMediaGpsTable::GetRecordFromGps2(string strGps2, MediaGpsItem& item)
{
    item = {};
    if(strGps2.length() == 0)
    {
        return TRUE;
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> itemList = pDbDriver->QuerySQL("select * from %s where gps2='%s'", TABLE_MEDIAGPS, strGps2.c_str());
    UNLOCKMEDIADB
    if(itemList.size() == 0)
    {
        return FALSE;
    }
    item = AssembleItems(itemList).front();
    return TRUE;
}
BOOL CMediaGpsTable::SetRecord(string strGps, string strGps2, string strLocation)
{
    if(strGps.length() > 0)
    {
        MediaGpsItem item = {};
        BOOL bHasRecord = GetRecordFromGps(strGps, item);
        if(strGps2.length() == 0)
        {
            strGps2 = item.strGps2;
        }
        if(strLocation.length() == 0)
        {
            strLocation = item.strLocation;
        }
        string strSQL = Server::CCommonUtil::StringFormat("insert into %s (gps,gps2,gpslocation)values('%s','%s','%s')", TABLE_MEDIAGPS, strGps.c_str(), strGps2.c_str(), strLocation.c_str());
        if(TRUE == bHasRecord)
        {
            strSQL = Server::CCommonUtil::StringFormat("update %s  set gpslocation='%s' , gps2='%s' where gps='%s'", TABLE_MEDIAGPS, strLocation.c_str(), strGps2.c_str(), strGps.c_str());
        }
        CDbDriver* pDbDriver = LOCKMEDIADB
        BOOL bRet = pDbDriver->ExecuteSQL(strSQL.c_str());
        UNLOCKMEDIADB
        return bRet;
    }
    if(strGps2.length() > 0)
    {
        MediaGpsItem item = {};
        BOOL bHasRecord = GetRecordFromGps2(strGps, item);
        if(strGps2.length() == 0)
        {
            strGps2 = item.strGps2;
        }
        if(strLocation.length() == 0)
        {
            strLocation = item.strLocation;
        }
        string strSQL = Server::CCommonUtil::StringFormat("insert into %s (gps,gps2,gpslocation)values('%s','%s','%s')", TABLE_MEDIAGPS, "", strGps2.c_str(), strLocation.c_str());
        
        if(TRUE == bHasRecord)
        {
            strSQL = Server::CCommonUtil::StringFormat("update %s set gpslocation='%s' where gps2='%s'", TABLE_MEDIAGPS, strLocation.c_str(), strGps2.c_str());
        }
        CDbDriver* pDbDriver = LOCKMEDIADB
        BOOL bRet = pDbDriver->ExecuteSQL(strSQL.c_str());
        UNLOCKMEDIADB
        return bRet;
    }
    return FALSE;
}
BOOL CMediaGpsTable::RemoveAll()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from %s", TABLE_MEDIAGPS);
    UNLOCKMEDIADB
    return bRet;
}