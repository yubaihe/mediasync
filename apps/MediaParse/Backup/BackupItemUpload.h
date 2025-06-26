#pragma once
#include "../stdafx.h"
#include "MediaDefine.h"
class CBackupItemUpload
{
public:
    CBackupItemUpload();
    ~CBackupItemUpload();
    BOOL UploadItem(string strSrcFile, string strSrcThumbFile, string strDestFold);
    int GetPrecent(char* pszErrorInfo);
    BOOL CheckExit();
private:
    string GetNewFileName(string strFold, MEDIATYPE eMediaType, string& strOutThumbFile);
    void Reset();
    static DWORD BackupItemProc(void* lpParameter);
    BOOL Backup();
    BOOL CopyFile(const char* pszSrc, const char* pszDest);
    BOOL Save();
    
private:
    size_t m_iThumbSize;
    size_t m_iOrigianlSize;
    size_t m_iTransSize;
    size_t m_iTotalSize;
    HANDLE m_hUpload;
    string m_strErrorInfo;
    string m_strOriginalFile;
    string m_strThumbFile;
    string m_strBackupOriginalFile;
    string m_strBackupThumbFile;
    string m_strTmpOriginalFile;
    string m_strTmpThumbFile;
    MEDIATYPE m_eMediaType;
    string m_strDestFileName;
    string m_strFoldName;
    time_t m_iOldTimeSec;
};


