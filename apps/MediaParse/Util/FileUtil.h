#ifndef FILEUTIL_H_
#define FILEUTIL_H_
#include "../stdafx.h"
#include "md5.h"
class CFileUtil
{
public:
    CFileUtil();
    ~CFileUtil();
    char* GetFileContent(const char* pszFile);
    char* GetBinFileContent(const char* pszFile, int& iContentLen);
    BOOL SetFileContent(const char* pszFile, const char* pszContent);
    static string GetFileMd5(const char* pszFile);
    static list<string> GetSubFolds(const char* pszFold);
    static BOOL CheckFoldExist(string strFold);
    static BOOL CheckFileExist(string strFile);
    static BOOL CreateFold(string strFold);
    static BOOL RemoveFile(string strFile);
    static string GetFileMd51(const char* pszFile, int iTag);
private:
    char* m_pszFileContent;
    list<string> m_FoldFileList;
};


#endif