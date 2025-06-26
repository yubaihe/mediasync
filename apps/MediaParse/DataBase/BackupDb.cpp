#include "BackupDb.h"
#include "../Disk/PhotoManager.h"
#include "../Util/CommonUtil.h"
CBackupDb* CBackupDb::m_pBackupDb = NULL;
CBackupDb::CBackupDb()
{
    m_pDbDriver = NULL;
    InitializeCriticalSection(&m_Section);
    //InitAllTables();
}
CBackupDb::~CBackupDb()
{
    EnterCriticalSection(&m_Section);
    if(NULL != m_pDbDriver)
    {
        m_pDbDriver->CloseDB();
        delete m_pDbDriver;
        m_pDbDriver = NULL;
    }
    LeaveCriticalSection(&m_Section);
    DeleteCriticalSection(&m_Section);
}
CBackupDb* CBackupDb::GetInstance()
{
    if(NULL == m_pBackupDb)
    {
        m_pBackupDb = new CBackupDb();
    }
    return m_pBackupDb;
}
void CBackupDb::Release()
{
    if(NULL != m_pBackupDb)
    {
        delete m_pBackupDb;
        m_pBackupDb = NULL;
    }
}
CDbDriver* CBackupDb::Lock()
{
    EnterCriticalSection(&m_Section);
    return m_pDbDriver;
}
void CBackupDb::UnLock()
{
    LeaveCriticalSection(&m_Section);
}

void CBackupDb::InitAllTables(string strStore)
{
    if(NULL == m_pDbDriver)
    {
        m_pDbDriver = new CDbDriver();
    }
    m_pDbDriver->OpenDB(strStore.c_str());

    CBackupTable backupTable;
    backupTable.CreateTable();

}

CBackupTable::CBackupTable()
{
}

CBackupTable::~CBackupTable()
{
}

BOOL CBackupTable::CreateTable()
{
    //const char*  pszTable = "create table if not exists tbl_mediainfo(id integer PRIMARY KEY autoincrement, " 
    //                                "paishetime text, year integer,month integer, day integer, md5num text, weizhi text, location text, meititype text, meitisize text, devicename text, width text, height text, duration text, mediaaddr text, hasextra text DEFAULT (0))";
    //paishetime 拍摄时间
	//md5num     MD5值
	//weizhi     拍摄时候的GPS位置
	//location     拍摄时候的位置
	//meititype  媒体类型(1:图片  2:视频) MEDIATYPE_IMAGE
	//meitisize  大小(MB GB...)
	//devicename 设备类型(apple,android...)
	//width 宽度
	//height 高度
	//duration 持续时间
	//mediaaddr  媒体本地存储地址
	//addtime    添加时间
    //hasextra    是否包含livephoto 0:否 1:是
    map<string, string> param;
    param.insert(pair<string, string>("id", "integer PRIMARY KEY autoincrement"));
    param.insert(pair<string, string>("paishetime", "text DEFAULT '0'")); //虽然是毫秒级别的，但是精确度是分钟
    param.insert(pair<string, string>("md5num", "text DEFAULT ''"));
    param.insert(pair<string, string>("weizhi", "text DEFAULT ''"));
    param.insert(pair<string, string>("location", "text DEFAULT ''"));
    param.insert(pair<string, string>("meititype", "text DEFAULT ''"));
    param.insert(pair<string, string>("meitisize", "text DEFAULT ''"));
    param.insert(pair<string, string>("foldname", "text DEFAULT ''"));
    param.insert(pair<string, string>("width", "text DEFAULT ''"));
    param.insert(pair<string, string>("height", "text DEFAULT ''"));
    param.insert(pair<string, string>("duration", "text DEFAULT ''"));
    param.insert(pair<string, string>("mediaaddr", "text DEFAULT ''"));
    param.insert(pair<string, string>("hasextra", "text DEFAULT (0)"));
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->CreateTable(TABLE_BACKUP, param);
    UNLOCKBACKUPDB
    return bRet;
}
list<BackupItemFull> CBackupTable::AssembleItems(list<map<string, string>> List)
{
    list<BackupItemFull> retList;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        BackupItemFull item = {};
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
                item.iCreateTimeSec = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("md5num"))
            {
                item.strMd5 = strValue;
            }
            else if(0 == strKey.compare("weizhi"))
            {
                item.strAddr = strValue;
            }
            else if(0 == strKey.compare("location"))
            {
                item.strLocation = strValue;
            }
            else if(0 == strKey.compare("meititype"))
            {
                item.eMediaType = (MEDIATYPE)atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("meitisize"))
            {
                item.iMeiTiSize = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("foldname"))
            {
                item.strFoldName = strValue;
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
                item.iDuration = atol(strValue.c_str());
            }
            else if(0 == strKey.compare("mediaaddr"))
            {
                item.strFile = strValue;
            }
            else if(0 == strKey.compare("hasextra"))
            {
                item.iHasExtra = atoi(strValue.c_str());
            }
        }
        retList.push_back(item);
    }
    return retList;
}
list<string> CBackupTable::AllFolds()
{
    CDbDriver* pDbDriver = LOCKBACKUPDB
    list<string> List = pDbDriver->QuerySQL2("select distinct(foldname) from %s", TABLE_BACKUP);
    UNLOCKBACKUPDB
    return List;
}
BOOL CBackupTable::DeleteFold(string strFold)
{
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from %s where foldname='%s'", TABLE_BACKUP, strFold.c_str());
    UNLOCKBACKUPDB
    return bRet;
}
vector<BackupItem> CBackupTable::BackupFileListShort(string strFold, int* piVideoCount, int* piPicCount)
{
    vector<BackupItem> retVec;
    if(NULL != piVideoCount)
    {
        *piVideoCount = 0;
    }
    if(NULL != piPicCount)
    {
        *piPicCount = 0;
    }
    
    CDbCursor cursor;
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->QuerySQL(cursor, "select id,meititype,mediaaddr,paishetime,md5num from %s where foldname='%s'", TABLE_BACKUP, strFold.c_str());
    UNLOCKBACKUPDB
    //int iCount = cursor.GetCount();
    if(FALSE == bRet)
    {
        return retVec;
    }
    while(TRUE == cursor.Next())
    {
        MEDIATYPE eMediaType = (MEDIATYPE)cursor.GetInt("meititype");
        if(MEDIATYPE_IMAGE == eMediaType)
        {
            if(NULL != piPicCount)
            {
                *piPicCount += 1;
            }
        }
        else
        {
            if(NULL != piVideoCount)
            {
                *piVideoCount += 1;
            }
        }
        BackupItem item = {};
        item.iID = cursor.GetInt("id");
        item.strFile = cursor.GetString("mediaaddr");
        item.strMd5 = cursor.GetString("md5num");
        item.iCreateTimeSec = cursor.GetLong("paishetime");
        item.eMediaType = eMediaType;
        //string strAddr = CCommonUtil::StringFormat("%s/%s", strFoldName.c_str(), strMediaAddr.c_str());
        retVec.push_back(item);
    }
    return retVec;
}
BOOL CBackupTable::CheckExist(string strFold, string strMd5, time_t iCreateTimeSec /*= 0*/)
{
    list<string> IdList;
    if(0 == iCreateTimeSec)
    {
        CDbDriver* pDbDriver = LOCKBACKUPDB
        IdList = pDbDriver->QuerySQL2("select id from %s where foldname='%s' and md5num='%s' limit 1 offset 0", TABLE_BACKUP, strFold.c_str(), strMd5.c_str());
        UNLOCKBACKUPDB
    }
    else
    {
        CDbDriver* pDbDriver = LOCKBACKUPDB
        IdList = pDbDriver->QuerySQL2("select id from %s where foldname='%s' and md5num='%s' and paishetime='%d' limit 1 offset 0", TABLE_BACKUP, strFold.c_str(), strMd5.c_str(), iCreateTimeSec);
        UNLOCKBACKUPDB
    }
    
    if(IdList.size() > 0)
    {
        return TRUE;
    }
    return FALSE;
}
BOOL CBackupTable::RemoveFromIDList(list<int> idList)
{
    if(idList.size() == 0)
    {
        return TRUE;
    }
    string strParam = CCommonUtil::ListIntToString(idList, ",");
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from %s where id in(%s)", TABLE_BACKUP, strParam.c_str());
    UNLOCKBACKUPDB
    return bRet;
}
BOOL CBackupTable::AddItem(BackupItemFull item)
{
    string strSQL = CCommonUtil::StringFormat("insert into %s (paishetime,md5num,weizhi,location,meititype,meitisize,foldname,width,height,duration,mediaaddr,hasextra)values('%d','%s','%s','%s','%d','%d','%s','%d','%d','%d','%s','%d')",
        TABLE_BACKUP, item.iCreateTimeSec, item.strMd5.c_str(), item.strAddr.c_str(), item.strLocation.c_str(), item.eMediaType, item.iMeiTiSize, item.strFoldName.c_str(), item.iWidth, item.iHeight, item.iDuration, item.strFile.c_str(), 0);
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->ExecuteSQL(strSQL.c_str());
    UNLOCKBACKUPDB
    return bRet;
}
BackupItemFull CBackupTable::GetItemFromFileName(string strFileName, string strFoldName)
{
    BackupItemFull item = {};
    string strSQL = CCommonUtil::StringFormat("select * from %s where foldname='%s' and mediaaddr='%s' limit 1 offset 0", TABLE_BACKUP, strFoldName.c_str(), strFileName.c_str());
    CDbDriver* pDbDriver = LOCKBACKUPDB
    list<map<string, string>> itemList = pDbDriver->QuerySQL(strSQL.c_str());
    UNLOCKBACKUPDB
    if(itemList.size() == 0)
    {
        return item;
    }
    list<BackupItemFull> retList = AssembleItems(itemList);
    return retList.front();
}
BackupItemFull CBackupTable::GetItem(int iItemID)
{
    BackupItemFull item = {};
    item.iID = -1;
    string strSQL = CCommonUtil::StringFormat("select * from %s where id='%d' limit 1 offset 0", TABLE_BACKUP, iItemID);
    CDbDriver* pDbDriver = LOCKBACKUPDB
    list<map<string, string>> itemList = pDbDriver->QuerySQL(strSQL.c_str());
    UNLOCKBACKUPDB
    if(itemList.size() == 0)
    {
        return item;
    }
    list<BackupItemFull> retList = AssembleItems(itemList);
    return retList.front();
}
BOOL CBackupTable::RemoveItem(int iItemID)
{
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from %s where id=%d", TABLE_BACKUP, iItemID);
    UNLOCKBACKUPDB
    return bRet;
}
list<BackupItemFull> CBackupTable::BackupFileList(string strFold, int iStart, int iLimit)
{
    BackupItemFull item = {};
    string strSQL = CCommonUtil::StringFormat("select * from %s where foldname='%s' order by paishetime desc limit %d offset %d ", TABLE_BACKUP, strFold.c_str(), iLimit, iStart);
    CDbDriver* pDbDriver = LOCKBACKUPDB
    list<map<string, string>> itemList = pDbDriver->QuerySQL(strSQL.c_str());
    UNLOCKBACKUPDB
    list<BackupItemFull> retList = AssembleItems(itemList);
    return retList;
}
BackupItemFull CBackupTable::GetNextItem(BackupItemFull curItem)
{
    BackupItemFull item = {};
    CDbCursor cursor;
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->QuerySQL(cursor, "select * from %s where paishetime<='%d' order by paishetime desc", TABLE_BACKUP, curItem.iCreateTimeSec);
    UNLOCKBACKUPDB
    if(FALSE == bRet)
    {
        return item;
    }
    BOOL bGetNext = FALSE;
    int iNextID = -1;
    while(TRUE == cursor.Next())
    {
        int iID = cursor.GetInt("id");
         if(iID == curItem.iID)
        {
            bGetNext = TRUE;
            continue;
        }
        if(TRUE == bGetNext)
        {
            iNextID = iID;
            break;
        }
    }
    if(iNextID >= 0)
    {
        return GetItem(iNextID);
    }
    return item;
}
BackupItemFull CBackupTable::GetPrevItem(BackupItemFull curItem)
{
    BackupItemFull item = {};
    CDbCursor cursor;
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->QuerySQL(cursor, "select * from %s where paishetime>='%d' order by paishetime desc", TABLE_BACKUP, curItem.iCreateTimeSec);
    UNLOCKBACKUPDB
    if(FALSE == bRet)
    {
        return item;
    }
    int iPrevID = -1;
    while(TRUE == cursor.Next())
    {
        int iID = cursor.GetInt("id");
        if(iID == curItem.iID)
        {
            break;
        }
        else
        {
            iPrevID = iID;
        }
    }
    if(iPrevID >= 0)
    {
        return GetItem(iPrevID);
    }
    return item;
}
BOOL CBackupTable::UpdateFoldName(string strFrom, string strTo)
{
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->ExecuteSQL("update %s set foldname='%s' where foldname='%s'", TABLE_BACKUP, strTo.c_str(), strFrom.c_str());
    UNLOCKBACKUPDB
    return bRet;
}
BackupItemFull CBackupTable::GetItemFromMd5(string strFold, string strMd5)
{
    BackupItemFull retItem = {};
    retItem.iID = -1;
    CDbDriver* pDbDriver = LOCKBACKUPDB
    list<map<string, string>> itemList = pDbDriver->QuerySQL("select * from %s where foldname='%s' and md5num='%s' limit 1 offset 0 ", TABLE_BACKUP, strFold.c_str(), strMd5.c_str());
    UNLOCKBACKUPDB
    if(0 == itemList.size())
    {
        return retItem;
    }
    list<BackupItemFull> item = AssembleItems(itemList);
    BackupItemFull backupItem = item.front();
    return backupItem;
}
list<string> CBackupTable::RemoveFoldNotIn(vector<string> foldVec)
{
    list<string> retList;
    if(foldVec.size() == 0)
    {
        return retList;
    }
    string strParam = CCommonUtil::VectorStringToString(foldVec, ",");
    CDbDriver* pDbDriver = LOCKBACKUPDB
    retList = pDbDriver->QuerySQL2("select foldname from %s where foldname not in(%s)", TABLE_BACKUP, strParam.c_str());
    pDbDriver->ExecuteSQL("delete from %s where foldname not in(%s)", TABLE_BACKUP, strParam.c_str());
    UNLOCKBACKUPDB
    return retList;
}
BOOL CBackupTable::UpdateItemGpsAddr(int iItemID, string strGps, string strAddr)
{
    CDbDriver* pDbDriver = LOCKBACKUPDB
    BOOL bRet = pDbDriver->ExecuteSQL("update %s set weizhi='%s', location='%s' where id='%d'", TABLE_BACKUP, strGps.c_str(), strAddr.c_str(), iItemID);
    UNLOCKBACKUPDB
    return bRet;
}