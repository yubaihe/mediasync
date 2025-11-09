#pragma once
#include "../stdafx.h"
#include "arch.h"
struct MediaInfoItem
{
    //uint32_t最大值4,294,967,295
    int iID;               //ID
    long iPaiSheTime;       //拍摄时间
    long iAddTime;          //添加时间
    uint16_t iYear;              //年
    uint8_t iMonth;             //月
    uint8_t iDay;             //日
    uint8_t  iMediaType;        //媒体类型
#if CHIPTYPE == QZS3
    unsigned long iMeiTiSize;         //拍摄图片的大小
#else
    int64_t iMeiTiSize;         //拍摄图片的大小
#endif
    uint32_t iWidth;             //宽度
    uint32_t iHeight;            //高度
    uint32_t iDuration;          //持续时间
    long  iFavoriteTime;            //添加喜欢时间
    string strMd5Num;              //MD5值
    string strWeiZhi;              //拍摄时候的精度纬度 //32.063305,118.946005
    string strDeviceName;      //设备的名称
    string strAddr;                //媒体地址
    uint8_t iHasExtra;                //是否有额外视频比如livephoto
    string strLocation;            //拍摄位置
    long iPinnedTime;              //置顶时间秒数
};
#define MEDIATYPE_IMAGE 1
#define MEDIATYPE_VIDEO 2
class CMediaInfoTable
{
public:
    CMediaInfoTable();
    ~CMediaInfoTable();
    static BOOL CreateTable();
    static MediaInfoItem GetItemByID(int iItemID);
    static MediaInfoItem GetItem(CDbCursor& cursor);
    static BOOL CheckMd5Exist(string strMd5Num);
    static string FileNameFromMd5(string strMd5Num);
    static string FileNameFromPaiTime(long iPaiTime, string strDevNames, int iMediaType, long iLFileSize);
    static BOOL AddItem(MediaInfoItem item);
    static int GetRecordCount();
    static MediaInfoItem GetLatestItem();//最后更新项
    static int GetItemPaiTime(int iID);
    static int GetRecentRecordCount(uint32_t iType, string strDevNames);
    static string GetRecentRecords(int iStart, int iLimit, string strDevNames);
    static string GetFavoriteRecords(int iStart, int iLimit, string strDevNames);
    static string GetYearRecords(int iStart, int iLimit, int iYear, int iMonth, int iDay, string strDevNames, string strLocation, BOOL bLocation);
    static string GetLocationGroup(int iStart, int iLimit, int iYear, int iMonth, int iDay, string strDevNames);
    static string RecordsFromIds(string strIds, string strSort);
    static string GetItemIDSql(int iID, BOOL bNext, string strOtype, string strDevNames, int iGid, int iYear, int iMonth, int iDay, string strLocation);
    
    /**
     * //都需要devname Type
        //recent  => 不需要任何参数
        //favorite  => 不需要任何参数
        //group   => gid
        //year  => year month day location
    */
    static int GetPrevItemID(int iID, string strOtype, string strDevNames, int iGid, int iYear, int iMonth, int iDay, string strLocation);
    /**
     * //都需要devname Type ()
        //recent  => 不需要任何参数
        //favorite  => 不需要任何参数
        //group   => gid
        //year  => year month day location
    */
    static int GetNextItemID(int iID, string strOtype, string strDevNames, int iGid, int iYear, int iMonth, int iDay, string strLocation);
    static string GetItem(int iID, BOOL& bGet);
    static BOOL UpdateFavorite(int iID, BOOL bFavorite);
    static int GetRecordFavoriteCount(uint32_t iType, string strDevNames);
    static BOOL RemoveItem(int iID);
    static MediaInfoItem GetItemFromName(string strName);
    static BOOL RemoveItemFromName(string strName);
    static string GetFileName(int iID);
    static list<string> GetUnCheckWeiZhi(int iLimit);
    static BOOL UpdateGpsWeiZhi(string strGps, string strBaiDuGps, string strLocation);
    static string YearLocationGroupTongJi(string strLocation, int iYear, int iMonth, int iDay, string strDevNames);
    static BOOL UpdateItemPaiSheShiJian(int iID, int iTime, string strNewFileName);
    static BOOL UpdateItemGpsAddr(int iID, string strGps, string strAddr);
    static string YearMonthDayCover(int iYear, int iMonth, int iDay, string strDevNames);
    static string GetGroupInfoFomItemID(int iID);
    static list<MediaInfoItem> AssembleItems(list<map<string, string>> List);
    static BOOL SetPinned(int iID, BOOL bPinned);
    static list<int> GetTodayYear(int iMonth, int iDay, string strDevNames);
    static MediaInfoItem GetFirstMediaInDay(int iYear, int iMonth, int iDay, string strDevNames);
    static BOOL ChangeMediaAddr(int iID, string strAddr, string strMd5);
};

