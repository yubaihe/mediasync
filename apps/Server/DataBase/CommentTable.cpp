#include "CommentTable.h"
#include "MediaDb.h"
#include "MediaInfoTable.h"
#include "DbusUtil.h"
#include "Tools.h"
#include "JsonUtil.h"
CCommentTable::CCommentTable()
{

}
CCommentTable::~CCommentTable()
{

}
BOOL CCommentTable::CreateTable()
{
    map<string, string> param;
    param.insert(pair<string, string>("id", "integer PRIMARY KEY autoincrement"));
    param.insert(pair<string, string>("mediaid", "integer"));
    param.insert(pair<string, string>("mediamodule", "text DEFAULT''")); //同步:sync 备份的话就是备份文件夹的名字
    param.insert(pair<string, string>("comment", "text DEFAULT''")); //同步:sync 备份的话就是备份文件夹的名字
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->CreateTable(TABLE_COMMENT, param);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CCommentTable::InsertItem(CommentItem item)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("insert into %s(mediaid, mediamodule, comment) values ( '%d', '%s', '%s')", TABLE_COMMENT, item.iMediaID, item.strModule.c_str(), item.strComment.c_str());
    UNLOCKMEDIADB
    return bRet;
}
BOOL CCommentTable::AddItem(int iMediaID, string strComment, string strModule/* = ""*/)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> itemList = pDbDriver->QuerySQL2("select id from %s where mediaid ='%d' and mediamodule='%s' limit 1", TABLE_COMMENT, iMediaID, strModule.c_str());
    UNLOCKMEDIADB
    if(itemList.size() != 0)
    {
        return UpdateItem(iMediaID, strComment, strModule);
    }
    else 
    {
        CommentItem item = {};
        item.iMediaID = iMediaID;
        item.strComment = strComment;
        item.strModule = strModule;
        return InsertItem(item);
    }
}

BOOL CCommentTable::UpdateItem(int iMediaID, string strComment, string strModule /*= ""*/)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("update %s set comment='%s' where mediaid='%d' and mediamodule='%s'", TABLE_COMMENT, strComment.c_str(), iMediaID, strModule.c_str());
    UNLOCKMEDIADB
    return bRet;
}
BOOL CCommentTable::DeleteItem(int iMediaID, string strModule /*= ""*/)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from %s where mediaid='%d' and mediamodule='%s'", TABLE_COMMENT, iMediaID, strModule.c_str());
    UNLOCKMEDIADB
    return bRet;
}
BOOL CCommentTable::DeleteItems(string strMediaIDs, string strModule /*= ""*/)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from %s where mediaid in ('%s') and mediamodule='%s'", TABLE_COMMENT, strMediaIDs.c_str(), strModule.c_str());
    UNLOCKMEDIADB
    return bRet;
}
nlohmann::ordered_json CCommentTable::GetItems(int iID, int iLimited, string strQuery /*= ""*/)
{
    vector<int> NeedDelItemIDVec;
    nlohmann::ordered_json jsonRet = nlohmann::ordered_json::array();
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(iID <= 0)
    {
        list<string> idList = pDbDriver->QuerySQL2("select id from %s order by id desc limit 1", TABLE_COMMENT);
        if(idList.size() == 0)
        {
            UNLOCKMEDIADB
            return jsonRet;
        }
        iID = atoi(idList.front().c_str());
        iID += 1;
    }
    CDbCursor cursor;
    if(strQuery.length() == 0)
    {
        pDbDriver->QuerySQL(cursor, "select id,mediaid,mediamodule from %s where id <%d order by id desc", TABLE_COMMENT, iID);
    }
    else
    {
        strQuery = Server::CTools::Trim(strQuery);
        strQuery = Server::CTools::ReplaceString(strQuery, " ", "%");
        pDbDriver->QuerySQL(cursor, "select id,mediaid,mediamodule from %s where id<%d and comment LIKE '%%%s%%' COLLATE NOCASE order by id desc", TABLE_COMMENT, iID, strQuery.c_str());
    }
    UNLOCKMEDIADB
    while(TRUE == cursor.Next())
    {
        int iID = cursor.GetInt("id");
        int iMediaID = cursor.GetInt("mediaid");
        string strMediiaModule = cursor.GetString("mediamodule");
        if(strMediiaModule.length() == 0)
        {
            //这个是同步
            MediaInfoItem item = CMediaInfoTable::GetItemByID(iMediaID);
            if(item.iID < 0)
            {
                NeedDelItemIDVec.push_back(iID);
                continue;
            }
            nlohmann::ordered_json jsonItem;
            jsonItem["id"] = iID;
            jsonItem["itemid"] = iMediaID;
            jsonItem["paitime"] = item.iPaiSheTime;
            jsonItem["maddr"] = item.strAddr;
            jsonItem["module"] = "Sync";
            jsonItem["submodule"] = item.strDeviceName;
            jsonItem["commentshort"] = item.strCommentShort;
            jsonItem["mtype"] = item.iMediaType;
            jsonRet.push_back(jsonItem);
        }
        else
        {
            //这个是备份
            string strItemJson = CDbusUtil::GetBackupItem(iMediaID);
            if(strItemJson.length() == 0)
            {
                //发送dbus出错了
                continue;
            }
            nlohmann::json jsonBackupItem;
            Server::CJsonUtil::FromString(strItemJson, jsonBackupItem);
            if(jsonBackupItem["status"] == 0)
            {
                NeedDelItemIDVec.push_back(iID);
                continue;
            }
            nlohmann::ordered_json jsonItem;
            jsonItem["id"] = iID;
            jsonItem["itemid"] = iMediaID;
            jsonItem["paitime"] = jsonBackupItem["createtime"];
            jsonItem["maddr"] = jsonBackupItem["file"];
            jsonItem["module"] = strMediiaModule;
            jsonItem["submodule"] = jsonBackupItem["fold"];
            jsonItem["commentshort"] = jsonBackupItem["commentshort"];
            jsonItem["mtype"] = jsonBackupItem["mtype"];
            jsonRet.push_back(jsonItem);
        }
        if(jsonRet.size() == (size_t)iLimited)
        {
            break;
        }
    }
    if(NeedDelItemIDVec.size() > 0)
    {
        string strIds = Server::CTools::VecToString(NeedDelItemIDVec, ",");
        CDbDriver* pDbDriver = LOCKMEDIADB
        pDbDriver->ExecuteSQL("delete from %s where id in('%s')", TABLE_COMMENT, strIds.c_str());
        UNLOCKMEDIADB
    }
    return jsonRet;
}
BOOL CCommentTable::IsEmpty()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> idList = pDbDriver->QuerySQL2("select id from %s limit 1", TABLE_COMMENT);
    UNLOCKMEDIADB
    if(idList.size() == 0)
    {
        return TRUE;
    }
    return FALSE;
}