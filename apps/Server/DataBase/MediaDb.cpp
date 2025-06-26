#include "MediaDb.h"
#include "Config.h"
#include "MediaInfoTable.h"
#include "MediaJishuTable.h"
#include "MediaCoverTable.h"
#include "MediaGroupTable.h"
#include "MediaGroupItemsTable.h"
#include "MediaGpsTable.h"
CMediaDb* CMediaDb::m_pMediaDb = NULL;
CMediaDb::CMediaDb()
{
    m_pDbDriver = NULL;
    InitializeCriticalSection(&m_Section);
    //InitAllTables();
}
CMediaDb::~CMediaDb()
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
CMediaDb* CMediaDb::GetInstance()
{
    if(NULL == m_pMediaDb)
    {
        m_pMediaDb = new CMediaDb();
    }
    return m_pMediaDb;
}
void CMediaDb::Release()
{
    if(NULL != m_pMediaDb)
    {
        delete m_pMediaDb;
        m_pMediaDb = NULL;
    }
}
CDbDriver* CMediaDb::Lock()
{
    EnterCriticalSection(&m_Section);
    return m_pDbDriver;
}
void CMediaDb::UnLock()
{
    LeaveCriticalSection(&m_Section);
}

void CMediaDb::InitAllTables(string strStore)
{
    if(NULL == m_pDbDriver)
    {
        m_pDbDriver = new CDbDriver();
    }
    m_pDbDriver->OpenDB(strStore.c_str());

    CMediaCoverTable mediaCoverTable;
    mediaCoverTable.CreateTable();

    CMediaGroupItemsTable mediaGroupItems;
    mediaGroupItems.CreateTable();

    CMediaGroupTable mediaGroupTable;
    mediaGroupTable.CreateTable();

    CMediaInfoTable mediaInfoTable;
    mediaInfoTable.CreateTable();
    
    CMediaJishuTable mediaJiShuTable;
    mediaJiShuTable.CreateTable();

    CMediaGpsTable mediaGpsTable;
    mediaGpsTable.CreateTable();
}