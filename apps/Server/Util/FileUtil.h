#pragma once
#include "stdafx.h"
#include <fcntl.h> 
#include <unistd.h>
#include <string>
#include <vector>
#include "./CommonUtil.h"
using namespace std;
class CFileUtil
{
public:
    static BOOL CheckFileExist(string strFile);
    static BOOL CheckFoldExist(string strFile);
    static string GetFileOnlyName(string strFile);
    static int GetYear(string strFile);
    static string GetNewFileName(int iYear, string strFileName, string strPostFix);
    static string GetNewFileName2(int iYear, string strFileName, string strPostFix);
    static string GetNewFileName(string strPath);
    static BOOL CreateFold(string strFold);
    static BOOL RemoveFile(string strFile);
    static BOOL RemoveFold(string strFold);
    static BOOL MoveFile(string strFromFile, string strToFile);
    static BOOL CreateEmptyFile(string strFile);
    static string GetFileContent(const char* pszFile);
    static string GetLicence();
    static long GetFileSize(string strFile);
    static void SepFile(string strFile, char* pszName, char* pszPostFix);
    static string ThumbNameFromFileName(string strFileName);
    static string GetRelativePath(string strFile);
    static int GetFoldFileCount(char* pszFold);
    //FileUtil_GetFolds 
    static list<string> GetSubFolds(char* pszFold);
    static string CurrentPath();
    static BOOL IsEmptyDir(const char* pszDir);
    static void RemoveYearEmptyFold(int iYear);
    static BOOL CheckStatus(pid_t status);
    static BOOL IsFile(string strFile);
    static BOOL IsFold(string strFold);
    static string FileNameFromThumbName(string strThumbFileName);
    static string FileExNameFromFileName(string strThumbFileName);
    static string ReadLink(string strFileName);
    static BOOL IsSoftLink(string strFileName);
};


