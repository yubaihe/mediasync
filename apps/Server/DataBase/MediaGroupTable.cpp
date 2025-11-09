#include "MediaGroupTable.h"
#include "MediaDb.h"
#include "../Util/JsonUtil.h"
CMediaGroupTable::CMediaGroupTable()
{
}

CMediaGroupTable::~CMediaGroupTable()
{
}
BOOL CMediaGroupTable::CreateTable()
{
    //const char*  pszGroupTable = "create table if not exists tbl_mediagroup(id integer PRIMARY KEY autoincrement, " 
    //                                " name text, coverpic text)";
    //id 分组ID
	//name    分组名称 
	//coverpic 分组封面图片
    map<string, string> param;
    param.insert(pair<string, string>("id", "integer PRIMARY KEY autoincrement"));
    param.insert(pair<string, string>("name", "text DEFAULT''"));
    param.insert(pair<string, string>("coverpic", "text DEFAULT''"));
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->CreateTable(TABLE_MEDIAGROUP, param);
    UNLOCKMEDIADB
    return bRet;
}
list<MediaGroupItem> CMediaGroupTable::AssembleItems(list<map<string, string>> List)
{
    list<MediaGroupItem> retList;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        MediaGroupItem item = {};
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("id"))
            {
                item.iID = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("name"))
            {
                item.strName = strValue;
            }
            else if(0 == strKey.compare("coverpic"))
            {
                item.strCoverPic = strValue;
            }
        }
        retList.push_back(item);
    }
    return retList;
}
string CMediaGroupTable::GetJsonAllGroups()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL("select * from tbl_mediagroup");
    UNLOCKMEDIADB
    list<MediaGroupItem> retList = AssembleItems(List);
    nlohmann::json jsonRoot = nlohmann::json::array();
    for(list<MediaGroupItem>::iterator itor = retList.begin(); itor != retList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        jsonItem["id"] = itor->iID;
        jsonItem["name"] = itor->strName;
        jsonItem["cover"] = itor->strCoverPic;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}
int CMediaGroupTable::AddGroup(string strGroupName)
{
    int iID = GroupIdFromName(strGroupName);
    if(iID > 0)
    {
        return iID;
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("insert into tbl_mediagroup(name,coverpic)values('%s','%s')", strGroupName.c_str(), "");
    UNLOCKMEDIADB
    if(FALSE == bRet)
    {
        return -1;
    }
    else
    {
        return GroupIdFromName(strGroupName);
    }
}
int CMediaGroupTable::GroupIdFromName(string strGroupName)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select id from tbl_mediagroup where name='%s'", strGroupName.c_str());
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return -1;
    }
    return atoi(List.front().c_str());
}
BOOL CMediaGroupTable::SetCoverEmpty(string strCoverPic)
{
    if(strCoverPic.length() == 0)
    {
        return FALSE;
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediagroup set coverpic='' where coverpic='%s'", strCoverPic.c_str());
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaGroupTable::SetCover(int iID, string strCoverPic)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediagroup set coverpic='%s' where id='%d'", strCoverPic.c_str(), iID);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaGroupTable::RemoveItemFromID(int iID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediagroup where id='%d'", iID);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaGroupTable::RemoveItemFromName(string strName)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediagroup where name='%s'", strName.c_str());
    UNLOCKMEDIADB
    return bRet;
}
string CMediaGroupTable::GroupNameFromID(int iID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select name from tbl_mediagroup where id='%d'", iID);
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    return List.front();
}
string CMediaGroupTable::GetJsonAllGroupsFromGids(string strGids)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string, string>> List = pDbDriver->QuerySQL("select id, name from tbl_mediagroup where id in('%s')", strGids.c_str());
    UNLOCKMEDIADB
    list<MediaGroupItem> retList = AssembleItems(List);
    nlohmann::json jsonRoot = nlohmann::json::array();
    for(list<MediaGroupItem>::iterator itor = retList.begin(); itor != retList.end(); ++itor)
    {
        nlohmann::json jsonItem;
        jsonItem["id"] = itor->iID;
        jsonItem["name"] = itor->strName;
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    
    return strJson;
}

BOOL CMediaGroupTable::GroupItemUpdate(int iID, string strGroupName)
{
    string strOldGroupName = GroupNameFromID(iID);
    if(strOldGroupName.length() == 0)
    {
        //空的表示这个ID不存在 或者数据库操作异常了
        return FALSE;
    }
    if(0 == strOldGroupName.compare(strGroupName))
    {
        //新的名字和旧的名字一样
        return TRUE;
    }
    int iOldID = GroupIdFromName(strGroupName);
    if(iOldID > 0)
    {
        //这个名字已经存在了
        return FALSE;
    }
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediagroup set name='%s' where id='%d'", strGroupName.c_str(), iID);
    UNLOCKMEDIADB
    return bRet;
}
string CMediaGroupTable::GetCoverPic(int iID)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select coverpic from tbl_mediagroup where id='%d'", iID);
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "";
    }
    return List.front();
}
BOOL CMediaGroupTable::UpdateCoverPic(string strFrom, string strTo)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update tbl_mediagroup set coverpic='%s' where coverpic='%s'", strTo.c_str(), strFrom.c_str());
    UNLOCKMEDIADB
    return bRet;
}