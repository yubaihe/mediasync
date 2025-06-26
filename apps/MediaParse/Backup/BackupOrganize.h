#pragma once
#include "../stdafx.h"
#include "../DataBase/BackupDb.h"
typedef enum
{
    ORGANIZESTATE_NONE = 0,
    ORGANIZESTATE_RUNNING,
    ORGANIZESTATE_FINISH
}ORGANIZESTATE;
class CBackupOrganize
{
public:
    CBackupOrganize();
    ~CBackupOrganize();
    static CBackupOrganize* GetInstance();
    static void Release();
    BOOL Start();
    int GetProcess();
private:
    map<int, BackupItem> BackupFileMap(string strFoldName);
    void DealOneFold(string strFoldName);
    static DWORD BackupOrganizeProc(void* lpParameter);
    static void OnInitPhotoCallBack(int iPrecent, LPVOID* lpParameter);
    void SetProcess(int iPrecent);
    void RemoveFile(string strFile, string strReason);
private:
    int m_iBackupFoldCount;
    int m_iBackupFoldCurrent;
    int m_iPrecent;
    BOOL m_bExit;
    ORGANIZESTATE m_eOrganizeState;
    HANDLE m_hOrganize;
    static CBackupOrganize* m_pInstance;
};


