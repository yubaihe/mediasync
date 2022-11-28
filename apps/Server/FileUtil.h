#ifndef FILEUTIL_H__
#define FILEUTIL_H__
#include "stdafx.h"
#include <fcntl.h> 
#include<unistd.h>
BOOL FileUtil_CheckFileExist(const char* pszFile);

BOOL FileUtil_GetFileName(const char* pszInFileName,  char* pszOutFileName);
BOOL FileUtil_GetFileOnlyName(const char* pszFile, char* pszFileName);
int FileUtil_GetYear(const char* pszFile);
void FileUtil_GetNewFileName2(int iYear, const char* pszName, char* pPostFix, char* pRetName);
BOOL FileUtil_CheckFoldExist(const char* pszFold);
BOOL FileUtil_CreateFold(const char* pszFold);
BOOL FileUtil_RemoveFile(const char* pszFile);
BOOL FileUtil_RemoveFold(const char* pszFile);
BOOL FileUtil_MoveFile(const char* pFromFile, const char* pToFile);
BOOL FileUtil_CreateEmptyFile(const char* pFile);

void FileUtil_GetNewFileName(char* pFileName, char* pszPath);
long FileUtil_GetFileSize(const char* pszPathFile);
void FileUtil_SepFile(const char* pFileName, char* pName, char* pPostFix);
void FileUtil_ThumbNameFromFileName(char* pFileName, char* pszOut);
char* FileUtil_GetRelativePath(char* pszFile);
int FileUtil_GetFoldFileCount(char* pFold);
char* FileUtil_GetFolds(char* pFold);//返回值需要手动释放
char* FileUtil_CurrentPath();//返回值需要手动释放
BOOL FileUtil_IsEmptyDir(char* pszDir);
void FileUtil_RemoveYearEmptyFold(int iYear);
BOOL FileUtil_CheckStatus(pid_t status);
BOOL FileUtil_IsFile(const char* pszFile);
BOOL FileUtil_IsFold(const char* pszFold);
BOOL FileUtil_FileNameFromThumbName(char* pszThumbFileName, char* pszFileName);
BOOL FileUtil_ReadLink(char* pFileName, char* pFullFileName, size_t  iBufferLen);
BOOL FileUtil_IsSoftLink(char* pszFileName);
#endif