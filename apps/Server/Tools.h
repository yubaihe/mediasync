#ifndef RELECH_TOOLS_H_
#define RELECH_TOOLS_H_
#include "stdafx.h"
//char * strtok_r2 (char *s, const char *delim, char **save_ptr);
long long Tools_CurTimeMilSec();
int Tools_GetYear(long iTimeSec);
long Tools_CurTimeSec();
char* Tools_GetMacAddr();
BOOL Tools_IsIpFormatRight(char* pIpAddress);
char* Tools_CurrDir();
BOOL Tools_IsFile(const char* pszPath);
long Tools_GetFileLen(char* pPathFile);
int Tools_Rfind(const char* psz, char sz);
int Tools_RandomValue(int iMaxValue);
int Tools_RandomRangeValue(int iMinValue, int iMaxValue);
int Tools_YearStartSec(int iYear);
int Tools_YearEndSec(int iYear);
char* Tools_ReplaceString(const char* pSrc, char* pSep, char* pNewSep);//返回值需要手动释放
//char* Tools_SortYear(char* pYears);//返回值需要手动释放
char* Tools_ExecuteCmd(const char* pszCmd);//返回值需要手动释放
void Tools_ExecuteCmd2(const char* pszCmd);
char* Tools_ExecuteCmdWithFormat(const char* pszFormat, ...);//返回值需要手动释放
void Tools_ExecuteCmdWithFormatNoReplay(const char* pszFormat, ...);
BOOL Tools_GetDiskInfo(long* iTotal, long* iUsed);
char* Tools_GetMd5(const char* psz);//返回值需要手动释放
char* Tools_SecToTime(long iSec);//返回值需要手动释放
void Tools_SecInfo(long iSec, int* piYear, int* pMonth, int* pDay);
BOOL Tools_UpdateTimeSec(long iSec);
BOOL Tools_GetMountInfo(const char* pszDevName, char* pszMountAddr, char* pszOutDevName, char* pszOutMountPoint, char* pszFsType, int* piReadOnly);//
BOOL Tools_CheckGps(const char* pszGps, GPSTYPE* piGpsType, float* pdLong, float* pdLat);
BOOL Tools_GetMountPointFromDevName(const char* pszDevName, char* pszOutMountPoint);
BOOL Tools_HasRootPermission();
#endif