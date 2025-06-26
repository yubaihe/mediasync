#pragma once
#include "stdafx.h"
// typedef enum
// {
//     GPSTYPE_WGS84ll = 0,
//     GPSTYPE_BD09ll
// }GPSTYPE;
typedef enum
{
    GPSTAG_FORCEPRASE = -2, //放到gps解析列表的最前面
    GPSTAG_UNCHECK = -1,//已同步到mediainfo数据库 但是没有被解析的GPS
    GPSTAG_SERVER,
    GPSTAG_MEDIAPARSE
}GPSTAG;
typedef struct
{
    int iItemID;
    GPSTAG eGpsTag;
    GPSTYPE eGpsType;
    string strLongitude;
    string strLatitude;
    int iFailedCount;
}GpsItem;


 // CGpsManager::GetInstance()->AddDetectItem(GPSTYPE_NORMAL, "118.745003", "32.054980", 0, 0);
 // 118.745003:经度
 // 32.054980：纬度
class CGpsManager
{
public:
    CGpsManager();
    ~CGpsManager();
    static CGpsManager* GetInstance();
    static void Release();
    void AddDetectItem(GPSTYPE eGpsType, string strLongitude, string strLatitude,  int iItemID, GPSTAG iTag);
    void SetBaiDuKey(string strBaiDuKey);
    void NeedCheck();
    BOOL AddDetectItem(string strGps);
    BOOL IsSupport();
    BOOL ParseGps(string strGps, GPSTYPE* piGpsType, string* pstrLongitude = NULL, string* pstrLatitude = NULL);
private:
    void FilterUncheckGps();
    void ReqeustGpsDetail(GpsItem item);
    void ParseGpsDetail(GpsItem item, string strGpsDetail);
    static DWORD GpsDetectProc(void* lpParameter);
    static size_t RecvCallBack(char* pszData, size_t iDataSize, size_t iNmemb, std::string* pstrBuffer);
private:
    string m_strBaiDuKey;
    BOOL m_bExit;
    HANDLE m_hDetect;
    time_t m_iNeedCheckTimeSec;
    CRITICAL_SECTION m_Section;
    std::list<GpsItem> m_pGpsList;
    static CGpsManager* m_pInstance;
};

