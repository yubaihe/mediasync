#ifndef RELECH_DATABASEDEVICE__
#define RELECH_DATABASEDEVICE__
#include "stdafx.h"
#include "DataBaseDriver.h"
typedef struct 
{
    char* pszDevID;
    char* pszDevName;
    char* pszDevVersion;
    char* pszDevBlue;
}DevItem;

DevItem* DataBaseDevice_GetDevInfo();
BOOL  DataBaseDevice_AddOrUpdateDevItem(DevItem* pItem);
void DataBaseDevice_ReleaseItem(DevItem* pItem);
#endif