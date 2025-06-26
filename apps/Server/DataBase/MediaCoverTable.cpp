#include "MediaCoverTable.h"
#include "MediaDb.h"
CMediaCoverTable::CMediaCoverTable()
{
}

CMediaCoverTable::~CMediaCoverTable()
{
}
BOOL CMediaCoverTable::CreateTable()
{
    //const char*  pszCoverTable = "create table if not exists tbl_mediacover(covertype integer UNIQUE, " 
    //                                " mediaaddr text)";
    //covertype 封面类型 0:最前面的封面 其他的为分组ID
	//mediaaddr    封面地址 
    map<string, string> param;
    param.insert(pair<string, string>("covertype", "integer UNIQUE"));
    param.insert(pair<string, string>("mediaaddr", "text DEFAULT''"));
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->CreateTable(TABLE_MEDIACOVER, param);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaCoverTable::SetHomeCover(string strMediaAddr)
{
    if(strMediaAddr.length() == 0)
    {
        return RemoveHomeCover();
    }
    string strHomeCover = GetHomeCover();
    BOOL bRet = FALSE;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strHomeCover.length() > 0)
    {
        bRet = pDbDriver->ExecuteSQL("update tbl_mediacover set mediaaddr='%s' where covertype='%d'", strMediaAddr.c_str(), DATABASECOVER_HOME);
    }
    else
    {
        bRet = pDbDriver->ExecuteSQL("insert into tbl_mediacover(covertype, mediaaddr)values('%d','%s')", DATABASECOVER_HOME, strMediaAddr.c_str());
    }
    UNLOCKMEDIADB
    return bRet;
}
list<MediaCoverItem> CMediaCoverTable::AssembleItems(list<map<string, string>> List)
{
    list<MediaCoverItem> retList;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        MediaCoverItem item = {};
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("covertype"))
            {
                item.iCoverType = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("mediaaddr"))
            {
                item.strMediaAddr = strValue;
            }
        }
        retList.push_back(item);
    }
    return retList;
}
BOOL CMediaCoverTable::RemoveHomeCover()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediacover where covertype='%d'", DATABASECOVER_HOME);
    UNLOCKMEDIADB
    return bRet;
}
string CMediaCoverTable::GetHomeCover()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL("select * from tbl_mediacover where covertype='%d'", DATABASECOVER_HOME);
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    list<MediaCoverItem> retList = AssembleItems(List);
    return retList.front().strMediaAddr;
}
