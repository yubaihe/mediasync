#pragma once
#include "../stdafx.h"
struct MediaGroupItemOne
{
    int iID;
    int iGID;
    int iMediaID;
    int iMediaType;
    string strDeviceIdentify;
};

class CMediaGroupItemsTable
{
public:
    CMediaGroupItemsTable();
    ~CMediaGroupItemsTable();
    static BOOL CreateTable();
    static BOOL AddItem(MediaGroupItemOne item);
    static MediaGroupItemOne GetItem(int iGID, int iItemID);
    static BOOL Update(int iItemID, int iFromID, int iToID);
    static BOOL Detail(int iGID, string strDeviceName, int* piPicCount, int* piVideoCount);
    static string GidsFromMediaItemID(int iMediaItemID);//&分割
    static BOOL RemoveFromItemID(int iMediaItemID);
    static BOOL RemoveFromGroupID(int iGID);
    static string MediaIds(int iPage, int iLimit, int iGID, string strDeviceNames);
    static BOOL ClearCacheRecord();//将不存在的GID和mid相关联的记录删除
    static int MediaItemCount(uint32_t iGID, string strDeviceNames);
    static string GidsFromDevNames(string strDeviceNames);
    static string GetPrevItemIDSql(int iMediaItemID, int iGID, string strDeviceNames);
    static string GetNextItemIDSql(int iMediaItemID, int iGID, string strDeviceNames);
private:
    static BOOL DelItem(int iGID, int iItemID);

private:
    static list<MediaGroupItemOne> AssembleItems(list<map<string, string>> List);

};

