#ifndef DATABASEGROUPITEM_H__
#define DATABASEGROUPITEM_H__
#include "stdafx.h"
typedef struct 
{
    uint32_t iGID;             //分组ID
    uint32_t iItemID;          //媒体ID
    uint32_t iItemType;         //媒体类型
    char* pszDeviceIdentify;    //设备标示
}DataBaseGroupItem;

BOOL DataBaseGroupItems_AddItem(DataBaseGroupItem* pItem);
DataBaseGroupItem* DataBaseGroupItems_GetItem(uint32_t iGID, uint32_t iItemID);
BOOL DataBaseGroupItems_Update(uint32_t iItemID, uint32_t iFromID, uint32_t iToID);
BOOL DataBaseGroupItems_Detail(uint32_t iGid, char* pszDeviceName, uint32_t* piPicCount, uint32_t* piVideoCount);
char* DataBaseGroupItems_GidsFromMediaItemID(uint32_t iMediaItemID);//&分割
BOOL DataBaseGroupItems_RemoveFromItemID(uint32_t iMediaItemID);
BOOL DataBaseGroupItems_RemoveFromGroupID(uint32_t iGroupID);
char* DataBaseGroupItems_MediaIds(int iPage, int iLimit, uint32_t iGid, char* pDevNames);
BOOL DataBaseGroupItems_ClearCacheRecord();//将不存在的GID和mid相关联的记录删除
int DataBaseGroupItems_MediaItemCount(uint32_t iGid, char* pDevNames);
char* DataBaseGroupItems_GidsFromDevNames(char* pDevNames);
#endif