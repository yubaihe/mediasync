#pragma once
#include "stdafx.h"
#include "Database/DbDriver.h"
#include "MediaDefine.h"
#define MEDIADBNAME ".mediaparse.db"
#define TABLE_BACKUP    "tbl_backup"         //备份信息


struct BackupItem
{
    int iID;
    string strFile;
    string strMd5;
    time_t iCreateTimeSec;
    MEDIATYPE eMediaType;
};
struct BackupItemFull:BackupItem
{
    string strAddr;                 //媒体地址 lat&long
    string strLocation;             //拍摄位置
    int64_t iMeiTiSize;                //拍摄图片的大小
    string strFoldName;             //文件夹名称
    uint32_t iWidth;                //宽度
    uint32_t iHeight;               //高度
    uint32_t iDuration;             //持续时间
    uint8_t iHasExtra;              //是否有额外视频比如livephoto
    string strCommentShort;
    string strComment;
};
#define LOCKBACKUPDB CBackupDb::GetInstance()->Lock();
#define UNLOCKBACKUPDB CBackupDb::GetInstance()->UnLock();
/**
 * 大数据量查询
 *  CDbCursor cursor;
 *  CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->QuerySQL(cursor, "select * from tbl_mediainfo");
    UNLOCKMEDIADB
    int iCount = cursor.GetCount();
    printf("iCount:%d\n", iCount);
    if(FALSE == bRet)
    {
        return 0;
    }
    while(FALSE == cursor.Next())
    {
        int iID = cursor.GetInt("id");
        string strMediaAddr = cursor.GetString("mediaaddr");
        printf("ID:%d\tAddr:%s\n", iID, strMediaAddr.c_str());
    }
 */
class CBackupDb
{
public:
    CBackupDb();
    ~CBackupDb();
    static CBackupDb* GetInstance();
    static void Release();
    void InitAllTables(string strStore);
    CDbDriver* Lock();
    void UnLock();
    private:
    CRITICAL_SECTION m_Section;
    CDbDriver* m_pDbDriver;
    static CBackupDb* m_pBackupDb;
};
class CBackupTable
{
public:
    CBackupTable();
    ~CBackupTable();
    static BOOL CreateTable();
    list<string> AllFolds();
    BOOL DeleteFold(string strFold);
    vector<BackupItem> BackupFileListShort(string strFold, int* piVideoCount, int* piPicCount);
    list<BackupItemFull> BackupFileList(string strFold, int iStart, int iLimit);
    BOOL CheckExist(string strFold, string strMd5, time_t iCreateTimeSec = 0);
    BOOL RemoveFromIDList(list<int> idList);
    BOOL AddItem(BackupItemFull item);
    BackupItemFull GetItemFromFileName(string strFileName, string strFoldName);
    BackupItemFull GetItem(int iItemID);
    list<BackupItemFull> AssembleItems(list<map<string, string>> List);
    BOOL RemoveItem(int iItemID);
    BackupItemFull GetNextItem(BackupItemFull curItem);
    BackupItemFull GetPrevItem(BackupItemFull curItem);
    BOOL UpdateFoldName(string strFrom, string strTo);
    BackupItemFull GetItemFromMd5(string strFold, string strMd5);
    list<string> RemoveFoldNotIn(vector<string> foldList);
    BOOL UpdateItemGpsAddr(int iItemID, string strGps, string strAddr);
    BOOL GetComment(int iID, string& strCommentShort, string& strComment);
    BOOL UpdateComment(int iID, string strCommentShort, string strComment);
private:
    void RemoveComment(list<string> idList);
};
