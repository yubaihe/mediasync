#pragma once
#include "stdafx.h"
#include "SyncToDeviceItem.h"
class CSyncToDeviceManager
{
public:
    CSyncToDeviceManager();
    ~CSyncToDeviceManager();
    static CSyncToDeviceManager* GetInstance();
    static void Release();
    BOOL AddToSync(int iItemID);
    void ClearCache();
    void GetSyncInfo(int iItemID, int* piPrecent, char* pszError);
private:
    CRITICAL_SECTION m_Section;
    map<int, CSyncToDeviceItem*> m_SyncToDeviceItemMap;
    static CSyncToDeviceManager* m_pInstance;
};

