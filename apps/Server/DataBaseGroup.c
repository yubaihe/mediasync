#include "DataBaseGroup.h"
#include "DataBaseDevice.h"
#include "DataBaseGroupItems.h"
char* DataBaseGroup_GetJsonAllGroups()
{
    char szSQL[] = "select * from tbl_mediagroup";
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
    }
    char* pszRetBuffer = NULL;
    GroupItem item;
    int iItemLen = sizeof(item) + 100;
    char* pszTmpBuffer = malloc(iItemLen);

    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        strcpy(item.szName, DataBaseDriver_GetString(pDataBaseDriver, "name"));
        item.iID = DataBaseDriver_GetLong(pDataBaseDriver, "id");
        strcpy(item.szCoverPic, DataBaseDriver_GetString(pDataBaseDriver, "coverpic"));
        memset(pszTmpBuffer, 0, iItemLen);

        if(pszRetBuffer == NULL)
        {
            sprintf(pszTmpBuffer, "{\"id\":%d,\"name\":\"%s\",\"cover\":\"%s\"}", item.iID, item.szName, item.szCoverPic);

            pszRetBuffer = malloc(strlen(pszTmpBuffer) + 1);
            memset(pszRetBuffer, 0, strlen(pszTmpBuffer) + 1);
            strcpy(pszRetBuffer, pszTmpBuffer);
        }
        else
        {
            sprintf(pszTmpBuffer, ",{\"id\":%d,\"name\":\"%s\",\"cover\":\"%s\"}", item.iID, item.szName, item.szCoverPic);
            pszRetBuffer = realloc(pszRetBuffer, strlen(pszTmpBuffer) + 1 + strlen(pszRetBuffer));
            strcat(pszRetBuffer, pszTmpBuffer);
        }
    }
    free(pszTmpBuffer);
    pszTmpBuffer = NULL;
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(pszRetBuffer == NULL)
    {
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
    }
    return pszRetBuffer;
}

uint32_t DataBaseGroup_AddGroup(const char* pszName)
{
    int iInputNameLen = strlen(pszName);
    if(iInputNameLen == 0 || iInputNameLen > MAX_GROUPNAME_LEN)
    {
        return 0;
    }
    uint32_t iID = DataBaseGroup_GroupIdFromName(pszName);
    if(iID > 0)
    {
        return iID;
    }
    char szSQL[200] = {0};
    sprintf(szSQL, "insert into tbl_mediagroup(name,coverpic)values('%s','%s')", pszName, "");
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return 0;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(FALSE == bRet)
    {
        return 0;
    }
    else
    {
        return DataBaseGroup_GroupIdFromName(pszName);
    }
}

char* DataBaseGroup_GroupNameFromID(uint32_t iID)
{
    char szSQL[200] = {0};
    sprintf(szSQL, "select name from tbl_mediagroup where id='%d'", iID);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
		return pszRet;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
		return pszRet;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
		return pszRet;
    }

    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    const char* pszName = DataBaseDriver_GetString(pDataBaseDriver, "name");
    int iRetLen = strlen(pszName) + 1;
    char* pszRet = malloc(iRetLen);
    memset(pszRet, 0, iRetLen);
    strcpy(pszRet, pszName);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return pszRet;
}

char* DataBaseGroup_GetJsonAllGroupsFromGids(char* pszGids)
{
    int iSqlLen = strlen(pszGids) + 100;
    char* pszSQL = malloc(iSqlLen);
    memset(pszSQL, 0, iSqlLen);
    sprintf(pszSQL, "select id, name from tbl_mediagroup where id in('%s')", pszGids);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
        free(pszSQL);
        pszSQL = NULL;
		return pszRet;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
        free(pszSQL);
        pszSQL = NULL;
		return pszRet;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
        free(pszSQL);
        pszSQL = NULL;
		return pszRet;
    }

    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    char* pszRet = NULL;
     while(DataBaseDriver_MoveToNext(pDataBaseDriver))
     {
         const char* pszName = DataBaseDriver_GetString(pDataBaseDriver, "name");
         uint32_t iID = DataBaseDriver_GetLong(pDataBaseDriver, "id");
         
         if(NULL == pszRet)
         {
            char szTmp[500] = {0};
            sprintf(szTmp, "{\"id\":%d,\"name\":\"%s\"}", iID, pszName);
            int iTmpLen = strlen(szTmp);

             pszRet = malloc(iTmpLen + 1);
             memset(pszRet, 0, iTmpLen + 1);
             strcpy(pszRet, szTmp);
         }
         else
         {
            char szTmp[500] = {0};
            sprintf(szTmp, ",{\"id\":%d,\"name\":\"%s\"}", iID, pszName);
            int iTmpLen = strlen(szTmp);

             int iTotalLen = strlen(pszRet) + iTmpLen + 1;
             pszRet = realloc(pszRet, iTotalLen);
             strcat(pszRet, szTmp);
         }
     }
     DataBaseDriver_CloseConn(pDataBaseDriver);
     free(pszSQL);
     pszSQL = NULL;
     return pszRet;
}

uint32_t DataBaseGroup_GroupIdFromName(const char* pszName)
{
    char szSQL[300] = {0};
    sprintf(szSQL, "select id from tbl_mediagroup where name='%s'", pszName);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
		return 0;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
		return 0;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
		return 0;
    }

    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    uint32_t iID = DataBaseDriver_GetLong(pDataBaseDriver, "id");
    DataBaseDriver_CloseConn(pDataBaseDriver);
   
    return iID;
}

BOOL DataBaseGroup_GroupItemUpdate(uint32_t iID, const char* pszName)
{
    int iInputNameLen = strlen(pszName);
    if(iInputNameLen == 0 || iInputNameLen > MAX_GROUPNAME_LEN)
    {
        return FALSE;
    }

    char* pszOldName = DataBaseGroup_GroupNameFromID(iID);
    if (strlen(pszOldName) == 0)
    {
        //空的表示这个ID不存在 或者数据库操作异常了
        free(pszOldName);
        pszOldName = NULL;
        return FALSE;
    }
    
    if (0 == strcmp(pszOldName, pszName))
    {
        //新的名字和旧的名字一样
        free(pszOldName);
        pszOldName = NULL;
        return TRUE;
    }
    uint32_t iOldID = DataBaseGroup_GroupIdFromName(pszName);
    if(iOldID > 0)
    {
        //这个名字已经存在了
        free(pszOldName);
        pszOldName = NULL;
        return FALSE;
    }
    free(pszOldName);
    pszOldName = NULL;
    //都符合条件了
    char szSQL[300] = {0};
    sprintf(szSQL, "update tbl_mediagroup set name='%s' where id='%d'", pszName, iID);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseGroup_RemoveItemFromID(uint32_t iID)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
        return FALSE;
	}
    char szSQL[200] = {0};
    sprintf(szSQL, "delete from tbl_mediagroup where id='%d'", iID);
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseGroup_RemoveItemFromName(char* pszName)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
        return FALSE;
	}
    char szSQL[200] = {0};
    sprintf(szSQL, "delete from tbl_mediagroup where name='%s'", pszName);
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseGroup_SetCover(uint32_t iID, const char* pszCover)
{
    if(NULL == pszCover)
    {
        return FALSE;
    }
    char szSQL[300] = {0};
    sprintf(szSQL, "update tbl_mediagroup set coverpic='%s' where id='%d'", pszCover, iID);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseGroup_SetCoverEmpty(const char* pszCover)
{
    if(NULL == pszCover || strlen(pszCover) == 0)
    {
        return FALSE;
    }
    char szSQL[300] = {0};
    sprintf(szSQL, "update tbl_mediagroup set coverpic='' where coverpic='%s'", pszCover);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

char* DataBaseGroup_GetCover(uint32_t iID)
{
    char szSQL[300] = {0};
    sprintf(szSQL, "select coverpic from tbl_mediagroup where id='%d'", iID);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return NULL;
    }
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
		return NULL;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pszRet = malloc(1);
        memset(pszRet, 0, 1);
		return pszRet;
    }

    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    const char* pszCoverPic = DataBaseDriver_GetString(pDataBaseDriver, "coverpic");
    int iRetLen = strlen(pszCoverPic) + 1;
    char* pszRet = malloc(iRetLen);
    memset(pszRet, 0, iRetLen);
    strcpy(pszRet, pszCoverPic);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return pszRet;
}