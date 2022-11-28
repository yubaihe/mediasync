#ifndef RELECH_DATABASEMEDIAJISHU_H__
#define RELECH_DATABASEMEDIAJISHU_H__
#include "stdafx.h"
#include "DataBaseDriver.h"
typedef struct 
{
    //uint32_t最大值4,29000000004,967,295
    uint32_t iYear; //年份
    uint32_t iMonth;//月份
    uint32_t iDay;//日
    uint32_t iPicCount;//图片数量
    uint32_t iVideoCount;//视频数量
    char* pszDeviceIdentify;//设备名称
}MediaJiShuItem;

typedef void(*ResetJiShuCallBack)(int iPrecent, LPVOID* lpParameter);

BOOL DataBaseMediaJiShu_ReSet(ResetJiShuCallBack callBack, LPVOID* lpParameter);
BOOL DataBaseMediaJiShu_Increase(long iPaiSheTime, int iMediaType, const char* pszIdentify);
BOOL DataBaseMediaJiShu_Decrease(long iPaiSheTime, int iMediaType, const char* pszIdentify);
BOOL DataBaseMediaJiShu_GetYearInfo(int iYear, char* pszDeviceName, int* piPicCount, int* piVideoCount);
BOOL DataBaseMediaJiShu_IncreaseFromID(long iMediaID);
BOOL DataBaseMediaJiShu_DecreaseFromID(long iMediaID);
char* DataBaseMediaJiShu_GetYears(char* pszDeviceName);
char* DataBaseMediaJiShu_GetDevNames();
char* DataBaseMediaJiShu_GetMonths(int iYear, char* pszDeviceName);
char* DataBaseMediaJiShu_GetDays(int iYear, int iMonth, char* pszDeviceName);
#endif