#pragma once
#include "stdafx.h"
#include "Database/DbDriver.h"
#define MEDIADBNAME ".mediasync.db"
#define TABLE_MEDIAINFO         "tbl_mediainfo"         //媒体信息
#define TABLE_MEDIAJISHU        "tbl_mediajishu"        //媒体计数
#define TABLE_MEDIACOVER        "tbl_mediacover"        //设备封面
#define TABLE_MEDIAGROUP        "tbl_mediagroup"        //媒体分组
#define TABLE_MEDIAGROUPITEMS   "tbl_mediagroupitems"   //媒体分组项
#define TABLE_MEDIAGPS          "tbl_mediagps"          //媒体GPS缓存

#define LOCKMEDIADB CMediaDb::GetInstance()->Lock();
#define UNLOCKMEDIADB CMediaDb::GetInstance()->UnLock();
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
class CMediaDb
{
public:
    CMediaDb();
    ~CMediaDb();
    static CMediaDb* GetInstance();
    static void Release();
    void InitAllTables(string strStore);
    CDbDriver* Lock();
    void UnLock();
    private:
    CRITICAL_SECTION m_Section;
    CDbDriver* m_pDbDriver;
    static CMediaDb* m_pMediaDb;

};

