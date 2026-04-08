#include "SyncToDeviceManager.h"
#include "DataBase/BackupDb.h"
CSyncToDeviceManager* CSyncToDeviceManager::m_pInstance = NULL;
CSyncToDeviceManager::CSyncToDeviceManager()
{
    InitializeCriticalSection(&m_Section);
}

CSyncToDeviceManager::~CSyncToDeviceManager()
{
    EnterCriticalSection(&m_Section);
    map<int, CSyncToDeviceItem*>::iterator itor = m_SyncToDeviceItemMap.begin();
    while (itor != m_SyncToDeviceItemMap.end()) 
    {
        delete itor->second;
        itor->second = NULL;
        itor = m_SyncToDeviceItemMap.erase(itor);
    }
    LeaveCriticalSection(&m_Section);
    DeleteCriticalSection(&m_Section);
}
CSyncToDeviceManager* CSyncToDeviceManager::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CSyncToDeviceManager();
    }
    return m_pInstance;
}
void CSyncToDeviceManager::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

BOOL CSyncToDeviceManager::AddToSync(int iItemID)
{
    CBackupTable table;
    BackupItemFull item = table.GetItem(iItemID);
    if(item.iID < 0)
    {
        return FALSE;
    }
    EnterCriticalSection(&m_Section);
    map<int, CSyncToDeviceItem*>::iterator itor = m_SyncToDeviceItemMap.find(iItemID);
    if(itor == m_SyncToDeviceItemMap.end())
    {
        printf("Create sync item\n");
        CSyncToDeviceItem* pSyncToDeviceItem = new CSyncToDeviceItem();
        pSyncToDeviceItem->Start(item);
        m_SyncToDeviceItemMap.insert(pair<int, CSyncToDeviceItem*>(iItemID, pSyncToDeviceItem));
    }
    LeaveCriticalSection(&m_Section);
    return TRUE;
}
void CSyncToDeviceManager::GetSyncInfo(int iItemID, int* piPrecent, char* pszError)
{
    CSyncToDeviceItem* pSyncToDeviceItem = NULL;
    EnterCriticalSection(&m_Section);
    map<int, CSyncToDeviceItem*>::iterator itor = m_SyncToDeviceItemMap.find(iItemID);
    if(itor != m_SyncToDeviceItemMap.end())
    {
        pSyncToDeviceItem = itor->second;
    }
    if(NULL == pSyncToDeviceItem)
    {
        *piPrecent = 0;
        strcpy(pszError, "not find");
    }
    else
    {
        *piPrecent = pSyncToDeviceItem->GetPrecent(pszError);
    }
    
    LeaveCriticalSection(&m_Section);
}
void CSyncToDeviceManager::ClearCache()
{
    EnterCriticalSection(&m_Section);
    map<int, CSyncToDeviceItem*>::iterator itor = m_SyncToDeviceItemMap.begin();
    while (itor != m_SyncToDeviceItemMap.end()) 
    {
        if (TRUE == itor->second->CheckExit()) 
        {
            delete itor->second;
             itor->second = NULL;
            itor = m_SyncToDeviceItemMap.erase(itor);
        }
        else
        {
            ++itor;
        }
    }
    LeaveCriticalSection(&m_Section);
}