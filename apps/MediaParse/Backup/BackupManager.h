#pragma once
#include "../stdafx.h"
#include "BackupFlod.h"
#include "BackupItemUpload.h"
#include "../DataBase/BackupDb.h"
extern string g_strBackupPoint;
extern string g_strBackThumbPath;
struct FoldEntry
{
    std::string strName;
    time_t iCreateTimeSec;
};
class CBackupManager
{
public:
    CBackupManager();
    ~CBackupManager();
    static CBackupManager* GetInstance();
    static void Release();
    BOOL Init(string strBackupRoot);
    
    std::vector<string> BackupFoldList(int iStart, int iLimited, int* piFoldCount = NULL);
    std::vector<BackupItem> BackupFileList(string strName, int* piPicCount = NULL, int* piVideoCount = NULL);
    string GetBackupRoot(string strName);
    string GetBackupThumbRoot(string strName);
    string GetBackupFoldItemDetail(int iItemID);
    BOOL UploadItem(string strDev, string strFile, string strDestFold, string& strToken);
    int GetUploadPrecent(string strToken, char* pszErrorInfo);
    std::vector<string> BackupFoldList();
    std::map<string, time_t> BackupFoldMap();
    std::map<string, time_t> BackupThumbFoldMap();
    list<string> GetStoreFoldList();
    void ClearCache();
    BOOL IsSupportBackup();
private:
    
    std::vector<FoldEntry> ScanFold(const std::string strPath);
    static bool CompareFoldWithCreatTime(const FoldEntry& a, const FoldEntry& b);
    static bool CompareFileWithCreatTime(const FileEntry& a, const FileEntry& b);
private:
    BOOL m_bSupportBackup;
    CRITICAL_SECTION m_Section;
    map<string, CBackupItemUpload*> m_pBackupItemUploadMap;
    static CBackupManager* m_pInstance;
};

