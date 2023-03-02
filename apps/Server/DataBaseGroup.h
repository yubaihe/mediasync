#ifndef RELECH_DATABASEGROUP_H__
#define RELECH_DATABASEGROUP_H__
#include "stdafx.h"
#define MAX_GROUPNAME_LEN 30
typedef struct 
{
    uint32_t iID;
    char szName[MAX_GROUPNAME_LEN];
    char szCoverPic[MAX_PATH];
}GroupItem;

char* DataBaseGroup_GetJsonAllGroups();
uint32_t DataBaseGroup_AddGroup(const char* pszName);
BOOL DataBaseGroup_SetCoverEmpty(const char* pszCover);
BOOL DataBaseGroup_SetCover(uint32_t iID, const char* pszCover);
BOOL DataBaseGroup_RemoveItemFromID(uint32_t iID);
BOOL DataBaseGroup_RemoveItemFromName(char* pszName);
char* DataBaseGroup_GroupNameFromID(uint32_t iID);
char* DataBaseGroup_GetJsonAllGroupsFromGids(char* pszGids);
uint32_t DataBaseGroup_GroupIdFromName(const char* pszName);
BOOL DataBaseGroup_GroupItemUpdate(uint32_t iID, const char* pszName);
char* DataBaseGroup_GetCover(uint32_t iID);
#endif