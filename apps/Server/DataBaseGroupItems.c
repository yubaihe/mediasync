#include "DataBaseGroupItems.h"
#include "DataBaseDriver.h"
#include "DataBaseMedia.h"
#include "DataBaseGroupItems.h"
void DataBaseGroupItems_SetParam(char** pKey, const char* pValue)
{
    if(NULL == pValue)
    {
        *pKey = (char*)malloc(1);
        memset(*pKey, 0, 1);
        return;
    }
    if(NULL == *pKey)
    {
        int iLen = strlen(pValue) + 1;
        *pKey = (char*)malloc(iLen);
        memset(*pKey, 0, iLen);
    }
    
    strcpy(*pKey, pValue);
}

void DataBaseGroupItems_ReleaseItem(DataBaseGroupItem* pItem)
{
    if(NULL == pItem)
    {
        return;
    }
    if(NULL != pItem->pszDeviceIdentify)
    {
        free(pItem->pszDeviceIdentify);
        pItem->pszDeviceIdentify = NULL;
    }
    free((char*)pItem);
    pItem = NULL;
}

BOOL DataBaseGroupItems_AddItem(DataBaseGroupItem* pItem)
{
    DataBaseGroupItem* pFindItem = DataBaseGroupItems_GetItem(pItem->iGID, pItem->iItemID);
    if(NULL != pFindItem)
    {
        DataBaseGroupItems_ReleaseItem(pFindItem);
        pFindItem = NULL;
        return TRUE;
    }

    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    int iSqlLen = strlen(pItem->pszDeviceIdentify) + 500;
    char* pszSQL = (char*)malloc(iSqlLen);
    memset(pszSQL, 0, iSqlLen);
    sprintf(pszSQL, "insert into tbl_mediagroupitems(gid,mediaid,meititype,deviceidentify)values('%d','%d','%d','%s')",
        pItem->iGID, pItem->iItemID, pItem->iItemType, pItem->pszDeviceIdentify);
        printf("%s\n", pszSQL);
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    free(pszSQL);
    pszSQL = NULL;
    return bRet;
}

DataBaseGroupItem* DataBaseGroupItems_GetItem(uint32_t iGID, uint32_t iItemID)
{
    char szSQL[200] = {0};
    sprintf(szSQL, "select * from tbl_mediagroupitems where gid='%d' and mediaid='%d'", iGID, iItemID);
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
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    char* pBuffer = (char*)malloc(sizeof(DataBaseGroupItem));
    memset(pBuffer, 0, sizeof(DataBaseGroupItem));
    DataBaseGroupItem* pItem = (DataBaseGroupItem*)pBuffer;
    pItem->iGID = DataBaseDriver_GetLong(pDataBaseDriver, "gid");
    pItem->iItemID = DataBaseDriver_GetLong(pDataBaseDriver, "mediaid");
    pItem->iItemType = DataBaseDriver_GetLong(pDataBaseDriver, "meititype");
    DataBaseGroupItems_SetParam(&(pItem->pszDeviceIdentify), DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify"));
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return pItem;
}

BOOL DataBaseGroupItems_DelItem(uint32_t iGID, uint32_t iItemID)
{
    char szSQL[200] = {0};
    sprintf(szSQL, "delete from tbl_mediagroupitems where gid='%d' and mediaid='%d'", iGID, iItemID);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseGroupItems_Update(uint32_t iItemID, uint32_t iFromID, uint32_t iToID)
{
    BOOL bRet = DataBaseGroupItems_DelItem(iFromID, iItemID);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    MediaItem* pMediaItem = DataBaseMedia_GetItemByID(iItemID);
    if(NULL == pMediaItem)
    {
        return FALSE;
    }
    
    char* pBuffer = (char*)malloc(sizeof(DataBaseGroupItem));
    memset(pBuffer, 0, sizeof(DataBaseGroupItem));
    DataBaseGroupItem* pItem = (DataBaseGroupItem*)pBuffer;
    pItem->iGID = iToID;
    pItem->iItemID = iItemID;
    pItem->iItemType = pMediaItem->iMediaType;
    DataBaseGroupItems_SetParam(&(pItem->pszDeviceIdentify), pMediaItem->pszDeviceIdentify);
    DataBaseMedia_ReleaseItem(pMediaItem);
    return DataBaseGroupItems_AddItem(pItem);
}
  
BOOL DataBaseGroupItems_Detail(uint32_t iGid, char* pszDeviceName, uint32_t* piPicCount, uint32_t* piVideoCount)
{
    //select count(*) as tcount, meititype from tbl_mediagroupitems where gid in('1','2') and deviceidentify in('a','b') group by meititype
    int iSqlLen = strlen(pszDeviceName) + 200;
    char* pszSQL = malloc(iSqlLen);
    memset(pszSQL, 0, iSqlLen);
    if(strlen(pszDeviceName) == 0)
    {
        sprintf(pszSQL, "select count(*) as tcount, meititype from tbl_mediagroupitems where gid = '%d' group by meititype",
            iGid);
    }
    else
    {
        sprintf(pszSQL, "select count(*) as tcount, meititype from tbl_mediagroupitems where gid = '%d' and deviceidentify in('%s') group by meititype",
            iGid, pszDeviceName);
    }
    
    printf("%s\n", pszSQL);
    *piPicCount = 0;
    *piVideoCount = 0;
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        free(pszSQL);
        pszSQL = NULL;
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSQL);
        pszSQL = NULL;
        return FALSE;
    }
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSQL);
        pszSQL = NULL;
        return TRUE;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
     while(DataBaseDriver_MoveToNext(pDataBaseDriver))
     {
         int iMeiType = DataBaseDriver_GetInt(pDataBaseDriver, "meititype");
         switch (iMeiType)
         {
            case MEDIATYPE_IMAGE:
            {
                *piPicCount = DataBaseDriver_GetInt(pDataBaseDriver, "tcount");
                break;
            }
            case MEDIATYPE_VIDEO:
            {
                *piVideoCount = DataBaseDriver_GetInt(pDataBaseDriver, "tcount");
                break;
            }
         }
     }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    free(pszSQL);
    pszSQL = NULL;
    return TRUE;
}

char* DataBaseGroupItems_GidsFromMediaItemID(uint32_t iMediaItemID)
{
    char szSQL[200] = {0};
    sprintf(szSQL, "select gid from tbl_mediagroupitems where mediaid='%d'", iMediaItemID);
    printf("%s\n", szSQL);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    char* pszRet = NULL;
     while(DataBaseDriver_MoveToNext(pDataBaseDriver))
     {
         uint32_t iGid = DataBaseDriver_GetLong(pDataBaseDriver, "gid");
         
         if(NULL == pszRet)
         {
            char szTmp[100] = {0};
            sprintf(szTmp, "%d", iGid);
            int iTmpLen = strlen(szTmp);

             pszRet = malloc(iTmpLen + 1);
             memset(pszRet, 0, iTmpLen + 1);
             strcpy(pszRet, szTmp);
         }
         else
         {
            char szTmp[100] = {0};
            sprintf(szTmp, "&%d", iGid);
            int iTmpLen = strlen(szTmp);

             int iTotalLen = strlen(pszRet) + iTmpLen + 1;
             pszRet = realloc(pszRet, iTotalLen);
             strcat(pszRet, szTmp);
         }
     }
     DataBaseDriver_CloseConn(pDataBaseDriver);
     //printf("%s\n", pszRet);
     return pszRet;
}

BOOL DataBaseGroupItems_RemoveFromGroupID(uint32_t iGroupID)
{
    char szSQL[200] = {0};
    sprintf(szSQL, "delete from tbl_mediagroupitems where gid='%d'", iGroupID);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseGroupItems_RemoveFromItemID(uint32_t iMediaItemID)
{
    char szSQL[200] = {0};
    sprintf(szSQL, "delete from tbl_mediagroupitems where mediaid='%d'", iMediaItemID);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

int DataBaseGroupItems_MediaItemCount(uint32_t iGid, char* pDevNames)
{
    int iSqlLen = strlen(pDevNames) + 500;
    char* pszSQL = malloc(iSqlLen);
    memset(pszSQL, 0, iSqlLen);
    if(strlen(pDevNames) > 0)
    {
        sprintf(pszSQL, "select count(mediaid) as mcount from tbl_mediagroupitems where gid = '%d' and deviceidentify in('%s') ", iGid, pDevNames);
    
    }
    else
    {
        sprintf(pszSQL, "select count(mediaid) as mcount from tbl_mediagroupitems where gid = '%d' ", iGid);
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        free(pszSQL);
        pszSQL = NULL;
        return 0;
    }
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSQL);
        pszSQL = NULL;
        return 0;
    }

    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    int iCount = DataBaseDriver_GetInt(pDataBaseDriver, "mcount");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    free(pszSQL);
    pszSQL = NULL;
    return iCount;
}

char* DataBaseGroupItems_MediaIds(int iPage, int iLimit, uint32_t iGid, char* pDevNames)
{
    int iSqlLen = strlen(pDevNames) + 500;
    char* pszSQL = malloc(iSqlLen);
    memset(pszSQL, 0, iSqlLen);
    if(strlen(pDevNames) > 0)
    {
        sprintf(pszSQL, "select mediaid from tbl_mediagroupitems where gid = '%d' and deviceidentify in('%s') order by mediaid desc limit %d offset %d", iGid, pDevNames, iLimit, iPage*iLimit);
    
    }
    else
    {
        sprintf(pszSQL, "select mediaid from tbl_mediagroupitems where gid = '%d' order by mediaid desc limit %d offset %d", iGid, iLimit, iLimit*iPage);
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        free(pszSQL);
        pszSQL = NULL;
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSQL);
        pszSQL = NULL;
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }

    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSQL);
        pszSQL = NULL;
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    char* pszRet = NULL;
     while(DataBaseDriver_MoveToNext(pDataBaseDriver))
     {
         uint32_t iMediaID = DataBaseDriver_GetLong(pDataBaseDriver, "mediaid");
         
         if(NULL == pszRet)
         {
            char szTmp[100] = {0};
            sprintf(szTmp, "%d", iMediaID);
            int iTmpLen = strlen(szTmp);

             pszRet = malloc(iTmpLen + 1);
             memset(pszRet, 0, iTmpLen + 1);
             strcpy(pszRet, szTmp);
         }
         else
         {
            char szTmp[100] = {0};
            sprintf(szTmp, "&%d", iMediaID);
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

BOOL DataBaseGroupItems_ClearCacheRecord()
{
    char szSQL[200] = {0};
    strcpy(szSQL, "delete from tbl_mediagroupitems where gid not in(select id from tbl_mediagroup)");
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    memset(szSQL, 0, 200);
    strcpy(szSQL, "delete from tbl_mediagroupitems where mediaid not in(select id from tbl_mediainfo)");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

char* DataBaseGroupItems_GidsFromDevNames(char* pDevNames)
{
    if(strlen(pDevNames) > 0)
    {
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
        return pszRet;
    }

    int iSqlLen = strlen(pDevNames) + 500;
    char* pszSQL = malloc(iSqlLen);
    memset(pszSQL, 0, iSqlLen);
    
    sprintf(pszSQL, "select distinct(gid) from tbl_mediagroupitems where deviceidentify in('%s')", pDevNames);
   
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        free(pszSQL);
        pszSQL = NULL;
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSQL);
        pszSQL = NULL;
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }

    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSQL);
        pszSQL = NULL;
        char* pszRet = malloc(1);
        memset(pszRet,0, 1);
        return pszRet;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    char* pszRet = NULL;
     while(DataBaseDriver_MoveToNext(pDataBaseDriver))
     {
         uint32_t iGid = DataBaseDriver_GetLong(pDataBaseDriver, "gid");
         
         if(NULL == pszRet)
         {
            char szTmp[100] = {0};
            sprintf(szTmp, "%d", iGid);
            int iTmpLen = strlen(szTmp);

             pszRet = malloc(iTmpLen + 1);
             memset(pszRet, 0, iTmpLen + 1);
             strcpy(pszRet, szTmp);
         }
         else
         {
            char szTmp[100] = {0};
            sprintf(szTmp, "&%d", iGid);
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