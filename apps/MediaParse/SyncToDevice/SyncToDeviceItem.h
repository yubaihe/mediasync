#pragma once
#include "../stdafx.h"
#include "DataBase/BackupDb.h"
class CSyncToDeviceItem
{
public:
    CSyncToDeviceItem();
    ~CSyncToDeviceItem();
    BOOL Start(BackupItemFull item);
    BOOL CheckExit();
    int GetPrecent(char* pszError);
private:
    void Sync();
    void Reset();
    BOOL CopyFile(const char* pszSrc, const char* pszDest);
    static DWORD SyncProc(void* lpParameter);
private:
    size_t m_iThumbSize;
    size_t m_iOrigianlSize;
    size_t m_iTransSize;
    size_t m_iTotalSize;
    string m_strErrorInfo;
    time_t m_iOldTimeSec;
    
    BOOL m_bExit;
    string m_strThumbFile;
    string m_strOriginalFile;

    BackupItemFull m_Item;
    HANDLE m_hSync;
};


