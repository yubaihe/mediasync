#ifndef RELECH_DATABASEMEDIA__
#define RELECH_DATABASEMEDIA__
#include "stdafx.h"

typedef struct 
{
    //uint32_t最大值4,294,967,295
    uint32_t iId;               //ID
    int iPaiSheTime;       //拍摄时间
    int iAddTime;          //添加时间
    uint8_t  iMediaType;        //媒体类型
    int iMeiTiSize;         //拍摄图片的大小
    uint32_t iWidth;             //宽度
    uint32_t iHeight;            //高度
    uint32_t iDuration;          //持续时间
    uint8_t  iFavorite;            //表记喜欢 0:默认 1:喜欢
    char* pszMd5Num;              //MD5值
    char* pszWeiZhi;              //拍摄时候的精度纬度 //32.063305,118.946005
    char* pszDeviceIdentify;      //设备的名称
    char* pszAddr;                //媒体地址
    uint8_t iHasExtra;                //是否有额外视频比如livephoto
    char* pszLocation;            //拍摄位置
}MediaItem;

BOOL DataBaseMedia_CheckMd5Exist(const char* pMd5Num);
BOOL DataBaseMedia_FileNameFromMd5(const char* pMd5Num, char* pszOutFileName);
BOOL DataBaseMedia_FileNameFromPaiTime(const char* pPaiTime, const char* pszDevName, const char* pszMediaType, long iLFileSize, char* pszOutFileName);
BOOL DataBaseMedia_AddItem(MediaItem* pItem);
BOOL DataBaseMedia_GetRecordCount(int* iCount);
MediaItem* DataBaseMedia_GetLatestItem();//最后更新项
MediaItem* DataBaseMedia_GetItemByID(int iID);//制定项目
void DataBaseMedia_ReleaseItem(MediaItem* pItem);
int DataBaseMedia_GetRecentRecordCount(uint32_t iType, char* pDevNames);
BOOL DataBaseMedia_GetRecentRecords(int iPage, int iLimit, char* pDevNames, char** pRetBuffer);
BOOL DataBaseMedia_GetFavoriteRecords(int iPage, int iLimit, char* pDevNames, char** pRetBuffer);
BOOL DataBaseMedia_GetYearRecords(int iPage, int iLimit, int iYear, int iMonth, int iDay, char* pDevNames, char* pszLocation, char** pRetBuffer);
char* DataBaseMedia_GetLocationGroup(int iID, int iLimit, int iYear, int iMonth, int iDay, char* pDevNames);
BOOL DataBaseMedia_RecordsFromIds(char* pszIds, char** pRetBuffer);
//int GetItemID(DataBaseDriver* pDataBaseDriver, MediaItem* pItem);
//int GetIDFromMd5(DataBaseDriver* pDataBaseDriver, char* pMd5Num);
BOOL DataBaseMedia_GetItem(int iID, char** pRetBuffer);
BOOL DataBaseMedia_UpdateFavorite(int iID, uint32_t iFavorite);
int DataBaseMedia_GetRecordFavoriteCount(uint32_t iType, char* pDevNames);
//BOOL DataBaseMedia_RemoveAll();
BOOL DataBaseMedia_RemoveItem(int iID);
BOOL DataBaseMedia_RemoveItemFromName(const char* pszName);
BOOL DataBaseMedia_GetFileName(int iID, char* pRetBuffer);
//int DataBaseMedia_GetMediaYearCount(int iStartTime, int iEndTime, uint32_t iType, char* pDevNames);
//char* DataBaseMedia_GetDevNames();
char* DataBaseMedia_GetUnCheckWeiZhi(int iLimit);
BOOL DataBaseMedia_UpdateGpsWeiZhi(const char* pszGps, const char* pszBaiDuGps, const char* pszLocation);
char* DataBaseMedia_YearLocationGroupTongJi(const char* pszLocation, int iYear, int iMonth, int iDay, const char* pszDevNames);
BOOL DataBaseMedia_UpdateItemPaiSheShiJian(int iID, int iTime, char* pszNewFileName);
BOOL DataBaseMedia_UpdateItemGpsAddr(int iID, const char* pszGps, const char* pszAddr);
BOOL DataBaseMedia_YearMonthDayCover(int iYear, int iMonth, int iDay, const char* pszDevNames, char* pszOut);
char* DataBaseMedia_GetGroupInfoFomItemID(int iID);
#endif