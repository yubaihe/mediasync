#pragma once
#include "../stdafx.h"
#include "../Disk/PhotoManager.h"
#include "../DataBase/BackupDb.h"
extern string g_strBackupPoint;
extern string g_strBackThumbPath;
struct FileEntry
{
    std::string strName;
    time_t iCreateTimeSec;
};

class CBackupFlod
{
public:
    CBackupFlod();
    CBackupFlod(string strFolderName, BOOL bScanFile = TRUE);
    ~CBackupFlod();
    string GetThumbFile(MEDIATYPE eMediaType, string strFile);
    std::vector<FileEntry> AllFilesVec(string strPath);
    std::map<string, time_t> AllFilesMap(string strPath);
    BOOL RemoveItem(int iItemID);
    string GetBackupRoot();
    string GetBackupThumbRoot();
private:
    static bool CompareFileWithCreatTime(const FileEntry& a, const FileEntry& b);
private:
   string m_strFoldName;
   vector<FileEntry> m_ScanFileVec;
};


