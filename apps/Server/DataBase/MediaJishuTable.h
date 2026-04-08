#pragma once
#include "../stdafx.h"

struct MediaJishuItem
{
    int iYear;
    int iMonth;
    int iDay;
    int iPicCount;//图片数量
    int iVideoCount;//视频数量
    string strDeviceName;//设备名称
};

typedef void(*ResetJiShuCallBack)(int iPrecent, LPVOID* lpParameter);

class CMediaJishuTable
{
public:
    CMediaJishuTable();
    ~CMediaJishuTable();
    static BOOL CreateTable();
    static BOOL Increase(long iPaiSheTime, int iMediaType, string strDeviceName);
    static BOOL DecreaseFromID(int iMediaID);
    static BOOL IncreaseFromID(int iMediaID);
    static BOOL Decrease(long iPaiSheTime, int iMediaType, string strDeviceName);
    static BOOL ReSet(ResetJiShuCallBack callBack, LPVOID* lpParameter);
    static void GetYearInfo(int iYear, string strDeviceNames, int* piPicCount, int* piVideoCount);
    static string GetYears(string strDeviceNames);
    static string GetDevNames();
    static string GetMonths(int iYear, string strDeviceNames);
    static string GetDays(int iYear, int iMonth, string strDeviceNames);
private:
    static BOOL Increase(CDbDriver* pDbDriver, long iPaiSheTime, int iMediaType, string strDeviceName);
    static list<MediaJishuItem> AssembleItems(list<map<string, string>> List);
};

