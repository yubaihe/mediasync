#include "stdafx.h"
#include "DataBaseDriver.h"
#include "DataBaseMedia.h"
#include "DataBaseMediaJiShu.h"
#include "DataBaseGroupItems.h"
#include "DataBaseGroup.h"
void DataBaseMedia_SetParam(char** pKey, const char* pValue)
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

BOOL DataBaseMedia_CheckMd5Exist(const char* pMd5Num)
{
	DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    if (NULL == pDataBaseDriver->m_pSqlite3)
	{
		DataBaseDriver_CloseConn(pDataBaseDriver);
		return TRUE;
	}
    char szBuffer[1024] = {0};
    sprintf(szBuffer, "select id from tbl_mediainfo where md5num='%s'", pMd5Num);

    DataBaseDriver_QuerySQL(pDataBaseDriver, szBuffer);
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
	DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseMedia_FileNameFromMd5(const char* pMd5Num, char* pszOutFileName)
{
	DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    if (NULL == pDataBaseDriver->m_pSqlite3)
	{
		DataBaseDriver_CloseConn(pDataBaseDriver);
		return TRUE;
	}
    char szBuffer[1024] = {0};
    sprintf(szBuffer, "select mediaaddr from tbl_mediainfo where md5num='%s' ", pMd5Num);

    DataBaseDriver_QuerySQL(pDataBaseDriver, szBuffer);
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
	DataBaseDriver_MoveToFirst(pDataBaseDriver);
	const char* pszFileName = DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr");
	strcpy(pszOutFileName, pszFileName);

	DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseMedia_FileNameFromPaiTime(const char* pPaiTime, const char* pszDevName, const char* pszMediaType, long iLFileSize, char* pszOutFileName)
{
	DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    if (NULL == pDataBaseDriver->m_pSqlite3)
	{
		DataBaseDriver_CloseConn(pDataBaseDriver);
		return TRUE;
	}
    //meititype, meitisize
    char szBuffer[1024] = {0};
    if(iLFileSize != 0)
    {
        sprintf(szBuffer, "select mediaaddr from tbl_mediainfo where deviceidentify='%s' and paishetime='%s' and meititype='%s' and meitisize='%ld'", pszDevName, pPaiTime, pszMediaType, iLFileSize);
    }
    else
    {
        sprintf(szBuffer, "select mediaaddr from tbl_mediainfo where deviceidentify='%s' and paishetime='%s' and meititype='%s'", pszDevName, pPaiTime, pszMediaType);
    }
    printf("%s\n", szBuffer);
    DataBaseDriver_QuerySQL(pDataBaseDriver, szBuffer);
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
	DataBaseDriver_MoveToFirst(pDataBaseDriver);
	const char* pszFileName = DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr");
	strcpy(pszOutFileName, pszFileName);

	DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseMedia_AddItem(MediaItem* pItem)
{
	// if(TRUE == DataBaseMedia_CheckMd5Exist(pItem->pMd5Num))
    // {
    //     //这张图片已经存在了
    //     return FALSE;
    // }

	DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    int iYear = 0;
    int iMonth = 0;
    int iDay = 0;
    Tools_SecInfo(pItem->iPaiSheTime, &iYear, &iMonth, &iDay);
    float dLat = 0;
	float dLong = 0;
	GPSTYPE iGpsType = 0;
    BOOL bGpsCheckOk = Tools_CheckGps(pItem->pszWeiZhi, &iGpsType, &dLong, &dLat);
    if(FALSE == bGpsCheckOk)
    {
        return FALSE;
    }
   	char szSQL[2048] = { 0 };
    sprintf(szSQL, "insert into tbl_mediainfo(paishetime, year, month, day, md5num, weizhi, location, meititype, meitisize, deviceidentify,width,height,duration,mediaaddr,addtime)values \
												( '%d', '%d', '%d', '%d', '%s', '%s', '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%s', '%d')",  
    				pItem->iPaiSheTime, iYear, iMonth, iDay, pItem->pszMd5Num, pItem->pszWeiZhi, pItem->pszLocation, pItem->iMediaType, pItem->iMeiTiSize, pItem->pszDeviceIdentify, pItem->iWidth, pItem->iHeight, pItem->iDuration, pItem->pszAddr, pItem->iAddTime);
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    //计数加 1
    DataBaseMediaJiShu_Increase(pItem->iPaiSheTime, pItem->iMediaType, pItem->pszDeviceIdentify);
    return TRUE;
}
BOOL DataBaseMedia_GetRecordCount(int* iCount)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szSQL[] = "select count(id) as recordcount from tbl_mediainfo";
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    *iCount = DataBaseDriver_GetLong(pDataBaseDriver, "recordcount");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

MediaItem* DataBaseMedia_GetLatestItem()
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return NULL;
	}
    char szSQL[] = "select * from tbl_mediainfo where paishetime in(select max(paishetime) from tbl_mediainfo)";
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);

    char* pBuffer = (char*)malloc(sizeof(MediaItem));
    memset(pBuffer, 0, sizeof(MediaItem));
    MediaItem* pItem = (MediaItem*)pBuffer;

    pItem->iId = DataBaseDriver_GetLong(pDataBaseDriver, "id");
    pItem->iPaiSheTime = DataBaseDriver_GetLong(pDataBaseDriver, "paishetime");
    DataBaseMedia_SetParam(&(pItem->pszMd5Num), DataBaseDriver_GetString(pDataBaseDriver, "md5num"));
    DataBaseMedia_SetParam(&(pItem->pszWeiZhi), DataBaseDriver_GetString(pDataBaseDriver, "weizhi"));
    pItem->iMediaType = DataBaseDriver_GetInt(pDataBaseDriver, "meititype");
    pItem->iFavorite = DataBaseDriver_GetInt(pDataBaseDriver, "favorite");
    pItem->iMeiTiSize = DataBaseDriver_GetLong(pDataBaseDriver, "meitisize");
    DataBaseMedia_SetParam(&(pItem->pszDeviceIdentify), DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify"));
    pItem->iWidth = DataBaseDriver_GetInt(pDataBaseDriver, "width");
    pItem->iHeight = DataBaseDriver_GetInt(pDataBaseDriver, "height");
    pItem->iDuration = DataBaseDriver_GetInt(pDataBaseDriver, "duration");
    DataBaseMedia_SetParam(&(pItem->pszAddr), DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
    pItem->iAddTime = DataBaseDriver_GetLong(pDataBaseDriver, "addtime");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return pItem;
}

MediaItem* DataBaseMedia_GetItemByID(int iID)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return NULL;
	}
    char szSQL[1024] = { 0 };
	sprintf(szSQL, "select * from tbl_mediainfo where id='%d'", iID);
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);

    char* pBuffer = (char*)malloc(sizeof(MediaItem));
    memset(pBuffer, 0, sizeof(MediaItem));
    MediaItem* pItem = (MediaItem*)pBuffer;

    pItem->iId = DataBaseDriver_GetLong(pDataBaseDriver, "id");
    pItem->iPaiSheTime = DataBaseDriver_GetLong(pDataBaseDriver, "paishetime");
    DataBaseMedia_SetParam(&(pItem->pszMd5Num), DataBaseDriver_GetString(pDataBaseDriver, "md5num"));
    DataBaseMedia_SetParam(&(pItem->pszWeiZhi), DataBaseDriver_GetString(pDataBaseDriver, "weizhi"));
    pItem->iMediaType = DataBaseDriver_GetInt(pDataBaseDriver, "meititype");
    pItem->iFavorite = DataBaseDriver_GetInt(pDataBaseDriver, "favorite");
    pItem->iMeiTiSize = DataBaseDriver_GetLong(pDataBaseDriver, "meitisize");
    DataBaseMedia_SetParam(&(pItem->pszDeviceIdentify), DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify"));
    pItem->iWidth = DataBaseDriver_GetInt(pDataBaseDriver, "width");
    pItem->iHeight = DataBaseDriver_GetInt(pDataBaseDriver, "height");
    pItem->iDuration = DataBaseDriver_GetInt(pDataBaseDriver, "duration");
    DataBaseMedia_SetParam(&(pItem->pszAddr), DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
    pItem->iAddTime = DataBaseDriver_GetLong(pDataBaseDriver, "addtime");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return pItem;
}

int DataBaseMedia_GetRecentRecordCount(uint32_t iType, char* pDevNames)
{
    int iCurTime = Tools_CurTimeSec();
    int iLimitTime = iCurTime - 7*24*60*60;
    int iSqlLen = strlen(pDevNames) + 300;
    char* pSQL = malloc(iSqlLen);
    memset(pSQL, 0, iSqlLen);
    if (strlen(pDevNames) != 0)
    {
        sprintf(pSQL, "select count(id) as mediacount from tbl_mediainfo where addtime > '%d' and meititype='%d' and deviceidentify in('%s')", iLimitTime, iType, pDevNames);
    }
    else
    {
        sprintf(pSQL, "select count(id) as mediacount from tbl_mediainfo where addtime > '%d' and meititype='%d' ", iLimitTime, iType);
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
        free(pSQL);
        pSQL = NULL;
		return 0;
	}

    //printf("%s\n", pSQL);
	
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pSQL);
    if(FALSE == bRet)
    {
        free(pSQL);
        pSQL = NULL;
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return 0;
    }
   
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    int iCount = DataBaseDriver_GetLong(pDataBaseDriver, "mediacount");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    free(pSQL);
    pSQL = NULL;
    return iCount;
}

/***
 * 分页查询最近上传记录
 */
BOOL DataBaseMedia_GetRecentRecords(int iPage, int iLimit, char* pDevNames, char** pRetBuffer)
{
    int iCurTime = Tools_CurTimeSec();
    int iLimitTime = iCurTime - 7*24*60*60;
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    int iDevNamesLen = strlen(pDevNames);
    char* pSQL = malloc(iDevNamesLen + 500);
    memset(pSQL, 0, iDevNamesLen + 500);
    if(iDevNamesLen == 0)
    {
        sprintf(pSQL, "select id,width,height,duration,meititype,meitisize,mediaaddr from tbl_mediainfo where addtime > '%d' order by addtime desc limit %d offset %d", iLimitTime, iLimit, iLimit*iPage);
    }
    else
    {
        sprintf(pSQL, "select id,width,height,duration,meititype,meitisize,mediaaddr from tbl_mediainfo where addtime > '%d' and deviceidentify in ('%s') order by addtime desc limit %d offset %d", iLimitTime, pDevNames, iLimit, iLimit*iPage);
    }
    
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pSQL);
    free(pSQL);
    pSQL = NULL;

    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        if(*pRetBuffer[0] != '\0')
        {
            strcat(*pRetBuffer, ",");
        }
        char* pszID = NULL;
        DataBaseMedia_SetParam(&pszID, DataBaseDriver_GetString(pDataBaseDriver, "id"));
        char* pszWidth = NULL;
        DataBaseMedia_SetParam(&pszWidth, DataBaseDriver_GetString(pDataBaseDriver, "width"));
        char* pszHeight = NULL;
        DataBaseMedia_SetParam(&pszHeight, DataBaseDriver_GetString(pDataBaseDriver, "height"));
        char* pszDuration = NULL;
        DataBaseMedia_SetParam(&pszDuration, DataBaseDriver_GetString(pDataBaseDriver, "duration"));
        char* pszMeiTiType = NULL;
        DataBaseMedia_SetParam(&pszMeiTiType, DataBaseDriver_GetString(pDataBaseDriver, "meititype"));
        char* pszMeiTiSize = NULL;
        DataBaseMedia_SetParam(&pszMeiTiSize, DataBaseDriver_GetString(pDataBaseDriver, "meitisize"));
        char* pszMeiTiAddr = NULL;
        DataBaseMedia_SetParam(&pszMeiTiAddr, DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
        char szBuffer[1024] = {0};
        sprintf(szBuffer, "{\"id\":\"%s\",\"w\":\"%s\",\"h\":\"%s\",\"dur\":\"%s\",\"mtype\":\"%s\",\"msize\":\"%s\",\"maddr\":\"%s\"}",
                pszID, pszWidth, pszHeight, pszDuration, pszMeiTiType, pszMeiTiSize, pszMeiTiAddr);
        strcat(*pRetBuffer, szBuffer);
        free(pszID);
        pszID = NULL;
        free(pszWidth);
        pszWidth = NULL;
        free(pszHeight);
        pszHeight = NULL;
        free(pszDuration);
        pszDuration = NULL;
        free(pszMeiTiType);
        pszMeiTiType = NULL;
        free(pszMeiTiSize);
        pszMeiTiSize = NULL;
        free(pszMeiTiAddr);
        pszMeiTiAddr = NULL;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
    
}

/***
 * 分页查询收藏的上传记录
 */
BOOL DataBaseMedia_GetFavoriteRecords(int iPage, int iLimit, char* pDevNames, char** pRetBuffer)
{
    //int iCurTime = Tools_CurTimeSec();
   // int iLimitTime = iCurTime - 24*60*60;
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}

    int iDevNamesLen = strlen(pDevNames);
    char* pSQL = malloc(iDevNamesLen + 500);
    memset(pSQL, 0, iDevNamesLen + 500);
    if(iDevNamesLen == 0)
    {
        sprintf(pSQL, "select id,width,height,duration,meititype,meitisize,mediaaddr from tbl_mediainfo where favorite == '1' order by paishetime desc limit %d offset %d", iLimit, iLimit * iPage);     
    }
    else
    {
        sprintf(pSQL, "select id,width,height,duration,meititype,meitisize,mediaaddr from tbl_mediainfo where favorite == '1' and deviceidentify in ('%s') order by paishetime desc limit %d offset %d", pDevNames, iLimit, iLimit * iPage); 
    }
    
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pSQL);
    free(pSQL);
    pSQL = NULL;

    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        if(*pRetBuffer[0] != '\0')
        {
            strcat(*pRetBuffer, ",");
        }
        char* pszID = NULL;
        DataBaseMedia_SetParam(&pszID, DataBaseDriver_GetString(pDataBaseDriver, "id"));
        char* pszWidth = NULL;
        DataBaseMedia_SetParam(&pszWidth, DataBaseDriver_GetString(pDataBaseDriver, "width"));
        char* pszHeight = NULL;
        DataBaseMedia_SetParam(&pszHeight, DataBaseDriver_GetString(pDataBaseDriver, "height"));
        char* pszDuration = NULL;
        DataBaseMedia_SetParam(&pszDuration, DataBaseDriver_GetString(pDataBaseDriver, "duration"));
        char* pszMeiTiType = NULL;
        DataBaseMedia_SetParam(&pszMeiTiType, DataBaseDriver_GetString(pDataBaseDriver, "meititype"));
        char* pszMeiTiSize = NULL;
        DataBaseMedia_SetParam(&pszMeiTiSize, DataBaseDriver_GetString(pDataBaseDriver, "meitisize"));
        char* pszMeiTiAddr = NULL;
        DataBaseMedia_SetParam(&pszMeiTiAddr, DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
        char szBuffer[1024] = {0};
        sprintf(szBuffer, "{\"id\":\"%s\",\"w\":\"%s\",\"h\":\"%s\",\"dur\":\"%s\",\"mtype\":\"%s\",\"msize\":\"%s\",\"maddr\":\"%s\"}",
                pszID, pszWidth, pszHeight, pszDuration, pszMeiTiType, pszMeiTiSize, pszMeiTiAddr);
        strcat(*pRetBuffer, szBuffer);
        free(pszID);
        pszID = NULL;
        free(pszWidth);
        pszWidth = NULL;
        free(pszHeight);
        pszHeight = NULL;
        free(pszDuration);
        pszDuration = NULL;
        free(pszMeiTiType);
        pszMeiTiType = NULL;
        free(pszMeiTiSize);
        pszMeiTiSize = NULL;
        free(pszMeiTiAddr);
        pszMeiTiAddr = NULL;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

/***
 * 分页查询年份的上传记录
 */
BOOL DataBaseMedia_GetYearRecords(int iPage, int iLimit, int iYear, int iMonth, int iDay, char* pDevNames, char* pszLocation, char** pRetBuffer)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
   
    int iDevNamesLen = strlen(pDevNames);
    char* pszSQL = malloc(iDevNamesLen + 500);
    memset(pszSQL, 0, iDevNamesLen + 500);
    strcpy(pszSQL, "select id,width,height,duration,meititype,meitisize,mediaaddr from tbl_mediainfo where ");
    if(iYear > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "year=%d and ", iYear);
        strcat(pszSQL, szBuffer);
    }
    if(iMonth > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "month=%d and ", iMonth);
        strcat(pszSQL, szBuffer);
    }
    if(iDay > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "day=%d and ", iDay);
        strcat(pszSQL, szBuffer);
    }
    if(strlen(pszLocation) != 0)
    {
        char szBuffer[1024] = {0};
        if(0 == strcmp(pszLocation, "待定位"))
        {
            sprintf(szBuffer, "location='%s' and ", "");
        }
        else
        {
            sprintf(szBuffer, "location='%s' and ", pszLocation);
        }
        strcat(pszSQL, szBuffer);
    }
    if(iDevNamesLen != 0)
    {
        char *pszBuffer = malloc(iDevNamesLen + 100);
        memset(pszBuffer, 0, iDevNamesLen + 100);
        sprintf(pszBuffer, "deviceidentify in ('%s') and ", pDevNames);
        strcat(pszSQL, pszBuffer);
        free(pszBuffer);
        pszBuffer = NULL;
    }

    char szLastBuffer[200] = {0};
    sprintf(szLastBuffer, "1=1 order by paishetime desc limit %d offset %d", iLimit, iLimit*iPage);
    strcat(pszSQL, szLastBuffer);
    printf("%s\n", pszSQL);

    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    free(pszSQL);
    pszSQL = NULL;

    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        if(*pRetBuffer[0] != '\0')
        {
            strcat(*pRetBuffer, ",");
        }
        char* pszID = NULL;
        DataBaseMedia_SetParam(&pszID, DataBaseDriver_GetString(pDataBaseDriver, "id"));
        char* pszWidth = NULL;
        DataBaseMedia_SetParam(&pszWidth, DataBaseDriver_GetString(pDataBaseDriver, "width"));
        char* pszHeight = NULL;
        DataBaseMedia_SetParam(&pszHeight, DataBaseDriver_GetString(pDataBaseDriver, "height"));
        char* pszDuration = NULL;
        DataBaseMedia_SetParam(&pszDuration, DataBaseDriver_GetString(pDataBaseDriver, "duration"));
        char* pszMeiTiType = NULL;
        DataBaseMedia_SetParam(&pszMeiTiType, DataBaseDriver_GetString(pDataBaseDriver, "meititype"));
        char* pszMeiTiSize = NULL;
        DataBaseMedia_SetParam(&pszMeiTiSize, DataBaseDriver_GetString(pDataBaseDriver, "meitisize"));
        char* pszMeiTiAddr = NULL;
        DataBaseMedia_SetParam(&pszMeiTiAddr, DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
        char szBuffer[1024] = {0};
        sprintf(szBuffer, "{\"id\":\"%s\",\"w\":\"%s\",\"h\":\"%s\",\"dur\":\"%s\",\"mtype\":\"%s\",\"msize\":\"%s\",\"maddr\":\"%s\"}",
                pszID, pszWidth, pszHeight, pszDuration, pszMeiTiType, pszMeiTiSize, pszMeiTiAddr);
        strcat(*pRetBuffer, szBuffer);
        free(pszID);
        pszID = NULL;
        free(pszWidth);
        pszWidth = NULL;
        free(pszHeight);
        pszHeight = NULL;
        free(pszDuration);
        pszDuration = NULL;
        free(pszMeiTiType);
        pszMeiTiType = NULL;
        free(pszMeiTiSize);
        pszMeiTiSize = NULL;
        free(pszMeiTiAddr);
        pszMeiTiAddr = NULL;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

/***
 * year按位置分组
 */
char* DataBaseMedia_GetLocationGroup(int iStart, int iLimit, int iYear, int iMonth, int iDay, char* pDevNames)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return NULL;
	}

    int iDevNamesLen = strlen(pDevNames);
    char* pszSQL = (char*)malloc(iDevNamesLen + 500);
    memset(pszSQL, 0, iDevNamesLen + 500);
    strcpy(pszSQL, "Select weizhi,location,Count(id) As Count from tbl_mediainfo where ");
    if(iYear > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "year=%d and ", iYear);
        strcat(pszSQL, szBuffer);
    }
    if(iMonth > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "month=%d and ", iMonth);
        strcat(pszSQL, szBuffer);
    }
    if(iDay > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "day=%d and ", iDay);
        strcat(pszSQL, szBuffer);
    }
    if(iDevNamesLen != 0)
    {
        char *pszBuffer = malloc(iDevNamesLen + 100);
        memset(pszBuffer, 0, iDevNamesLen + 100);
        sprintf(pszBuffer, "deviceidentify in ('%s') and ", pDevNames);
        strcat(pszSQL, pszBuffer);
        free(pszBuffer);
        pszBuffer = NULL;
    }
    char szLastSql[100] = {0};
    sprintf(szLastSql, "1=1 group by location limit %d offset %d", iLimit, iStart);
    strcat(pszSQL, szLastSql);
    printf("%s\n", pszSQL);
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    free(pszSQL);
    pszSQL = NULL;

    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    char* pRetBuffer = NULL;
    int iRetBufferLen = 0;
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        char* pszWeiZhi = NULL;
        DataBaseMedia_SetParam(&pszWeiZhi, DataBaseDriver_GetString(pDataBaseDriver, "weizhi"));
        char* pszLocation = NULL;
        DataBaseMedia_SetParam(&pszLocation, DataBaseDriver_GetString(pDataBaseDriver, "location"));
        char* pszCount = NULL;
        DataBaseMedia_SetParam(&pszCount, DataBaseDriver_GetString(pDataBaseDriver, "Count"));
      
        int iLen = strlen(pszWeiZhi) + strlen(pszLocation) + strlen(pszCount);
        char* pszBuffer = malloc(iLen + 100);
        memset(pszBuffer, 0, iLen + 100);
        
        if(NULL == pRetBuffer)
        {
            sprintf(pszBuffer, "{\"weizhi\":\"%s\",\"location\":\"%s\",\"count\":\"%s\"}",
                pszWeiZhi, strlen(pszLocation) > 0?pszLocation:"待定位", pszCount);
            int iDataLen = strlen(pszBuffer) + 1;
            pRetBuffer = (char*)malloc(iDataLen);
            memset(pRetBuffer, 0, iDataLen);
            strcpy(pRetBuffer, pszBuffer);
            iRetBufferLen = iDataLen - 1;
        }
        else
        {
            sprintf(pszBuffer, ",{\"weizhi\":\"%s\",\"location\":\"%s\",\"count\":\"%s\"}",
                pszWeiZhi, strlen(pszLocation) > 0?pszLocation:"待定位", pszCount);
            int iDataLen = strlen(pszBuffer) + 1;
            pRetBuffer = realloc(pRetBuffer, iRetBufferLen + iDataLen);
            memset(pRetBuffer + iRetBufferLen, 0, iDataLen);
            memcpy(pRetBuffer + iRetBufferLen, pszBuffer, iDataLen - 1);
            iRetBufferLen += iDataLen - 1;
        }
        free(pszBuffer);
        pszBuffer = NULL;
    
    
        free(pszWeiZhi);
        pszWeiZhi = NULL;
        free(pszLocation);
        pszLocation = NULL;
        free(pszCount);
        pszCount = NULL;
        
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return pRetBuffer;
}
/***
 * 通过ID查询媒体信息
 */
BOOL DataBaseMedia_RecordsFromIds(char* pszIds, char** pRetBuffer)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
   
    int iDevNamesLen = strlen(pszIds);
    char* pszSQL = malloc(iDevNamesLen + 500);
    memset(pszSQL, 0, iDevNamesLen + 500);
    sprintf(pszSQL, "select id,width,height,duration,meititype,meitisize,mediaaddr from tbl_mediainfo where id in('%s')", pszIds);
    
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    free(pszSQL);
    pszSQL = NULL;

    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        if(*pRetBuffer[0] != '\0')
        {
            strcat(*pRetBuffer, ",");
        }
        char* pszID = NULL;
        DataBaseMedia_SetParam(&pszID, DataBaseDriver_GetString(pDataBaseDriver, "id"));
        char* pszWidth = NULL;
        DataBaseMedia_SetParam(&pszWidth, DataBaseDriver_GetString(pDataBaseDriver, "width"));
        char* pszHeight = NULL;
        DataBaseMedia_SetParam(&pszHeight, DataBaseDriver_GetString(pDataBaseDriver, "height"));
        char* pszDuration = NULL;
        DataBaseMedia_SetParam(&pszDuration, DataBaseDriver_GetString(pDataBaseDriver, "duration"));
        char* pszMeiTiType = NULL;
        DataBaseMedia_SetParam(&pszMeiTiType, DataBaseDriver_GetString(pDataBaseDriver, "meititype"));
        char* pszMeiTiSize = NULL;
        DataBaseMedia_SetParam(&pszMeiTiSize, DataBaseDriver_GetString(pDataBaseDriver, "meitisize"));
        char* pszMeiTiAddr = NULL;
        DataBaseMedia_SetParam(&pszMeiTiAddr, DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
        char szBuffer[1024] = {0};
        sprintf(szBuffer, "{\"id\":\"%s\",\"w\":\"%s\",\"h\":\"%s\",\"dur\":\"%s\",\"mtype\":\"%s\",\"msize\":\"%s\",\"maddr\":\"%s\"}",
                pszID, pszWidth, pszHeight, pszDuration, pszMeiTiType, pszMeiTiSize, pszMeiTiAddr);
        strcat(*pRetBuffer, szBuffer);
        free(pszID);
        pszID = NULL;
        free(pszWidth);
        pszWidth = NULL;
        free(pszHeight);
        pszHeight = NULL;
        free(pszDuration);
        pszDuration = NULL;
        free(pszMeiTiType);
        pszMeiTiType = NULL;
        free(pszMeiTiSize);
        pszMeiTiSize = NULL;
        free(pszMeiTiAddr);
        pszMeiTiAddr = NULL;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}
void DataBaseMedia_ReleaseItem(MediaItem* pItem)
{
    if(NULL == pItem)
    {
        return;
    }

    if(NULL != pItem->pszMd5Num)
    {
        free(pItem->pszMd5Num);
        pItem->pszMd5Num = NULL;
    }
    if(NULL != pItem->pszWeiZhi)
    {
        free(pItem->pszWeiZhi);
        pItem->pszWeiZhi = NULL;
    }
    if(NULL != pItem->pszDeviceIdentify)
    {
        free(pItem->pszDeviceIdentify);
        pItem->pszDeviceIdentify = NULL;
    }
    if(NULL != pItem->pszAddr)
    {
        free(pItem->pszAddr);
        pItem->pszAddr = NULL;
    }
     if(NULL != pItem->pszLocation)
    {
        free(pItem->pszLocation);
        pItem->pszLocation = NULL;
    }
    free((char*)pItem);
    pItem = NULL;
}
/*
BOOL UpdateItemAddr(DataBaseDriver* pDataBaseDriver, char* pszName, int iID)
{
	int iCurTimeMilSec = Tools_CurTimeMilSec();
	char szSQL[1024] = { 0 };
	sprintf(szSQL, "update tbl_mediainfo set mediaaddr='%s', addtime='%lu' where id='%lu'", 
		pszName, iCurTimeMilSec, iID);
	return ExecuteSQL(pDataBaseDriver, szSQL);
}
*/
BOOL DataBaseMedia_GetItem(int iID, char** pRetBuffer)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
	char szSQL[1024] = { 0 };
	sprintf(szSQL, "select * from tbl_mediainfo where id='%d'", iID);
	BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
	int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return TRUE;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);

    char* pszID = NULL;
    DataBaseMedia_SetParam(&pszID, DataBaseDriver_GetString(pDataBaseDriver, "id"));
    char* pszWidth = NULL;
    DataBaseMedia_SetParam(&pszWidth, DataBaseDriver_GetString(pDataBaseDriver, "width"));
    char* pszHeight = NULL;
    DataBaseMedia_SetParam(&pszHeight, DataBaseDriver_GetString(pDataBaseDriver, "height"));
    char* pszDuration = NULL;
    DataBaseMedia_SetParam(&pszDuration, DataBaseDriver_GetString(pDataBaseDriver, "duration"));
    char* pszMeiTiType = NULL;
    DataBaseMedia_SetParam(&pszMeiTiType, DataBaseDriver_GetString(pDataBaseDriver, "meititype"));
    char* pszMeiTiSize = NULL;
    DataBaseMedia_SetParam(&pszMeiTiSize, DataBaseDriver_GetString(pDataBaseDriver, "meitisize"));
    char* pszMeiTiAddr = NULL;
    DataBaseMedia_SetParam(&pszMeiTiAddr, DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
    char* pszDeviceIdentify = NULL;
    DataBaseMedia_SetParam(&pszDeviceIdentify, DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify"));
    char* pszPaiSheTime = NULL;
    DataBaseMedia_SetParam(&pszPaiSheTime, DataBaseDriver_GetString(pDataBaseDriver, "paishetime"));
    char* pszWeiZhi = NULL;
    DataBaseMedia_SetParam(&pszWeiZhi, DataBaseDriver_GetString(pDataBaseDriver, "weizhi"));
    char* pszAddTime = NULL;
    DataBaseMedia_SetParam(&pszAddTime, DataBaseDriver_GetString(pDataBaseDriver, "addtime"));
    char* pszLocation = NULL;
    DataBaseMedia_SetParam(&pszLocation, DataBaseDriver_GetString(pDataBaseDriver, "location"));
    uint32_t iFavorite = DataBaseDriver_GetInt(pDataBaseDriver, "favorite");
    char szBuffer[1024] = {0};
    sprintf(szBuffer, "{\"id\":\"%s\",\"w\":\"%s\",\"h\":\"%s\",\"dur\":\"%s\",\"mtype\":\"%s\",\"msize\":\"%s\",\"maddr\":\"%s\",\"device\":\"%s\",\"paitime\":\"%s\",\"weizhi\":\"%s\",\"location\":\"%s\",\"addtime\":\"%s\",\"favorite\":\"%d\"}",
            pszID, pszWidth, pszHeight, pszDuration, pszMeiTiType, pszMeiTiSize, pszMeiTiAddr, pszDeviceIdentify, pszPaiSheTime, pszWeiZhi, strlen(pszLocation) > 0?pszLocation:"待定位", pszAddTime, iFavorite);
    strcat(*pRetBuffer, szBuffer);
    free(pszID);
    pszID = NULL;
    free(pszWidth);
    pszWidth = NULL;
    free(pszHeight);
    pszHeight = NULL;
    free(pszDuration);
    pszDuration = NULL;
    free(pszMeiTiType);
    pszMeiTiType = NULL;
    free(pszMeiTiSize);
    pszMeiTiSize = NULL;
    free(pszMeiTiAddr);
    pszMeiTiAddr = NULL;
    free(pszDeviceIdentify);
    pszDeviceIdentify = NULL;
    free(pszPaiSheTime);
    pszPaiSheTime = NULL;
    free(pszWeiZhi);
    pszWeiZhi = NULL;
    free(pszAddTime);
    pszAddTime = NULL;
    free(pszLocation);
    pszLocation = NULL;
    DataBaseDriver_CloseConn(pDataBaseDriver);
	return TRUE;
}

BOOL DataBaseMedia_UpdateFavorite(int iID, uint32_t iFavorite)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
	char szSQL[1024] = { 0 };
	sprintf(szSQL, "update tbl_mediainfo set favorite='%d' where id='%d'", iFavorite, iID);
	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

int DataBaseMedia_GetRecordFavoriteCount(uint32_t iType, char* pDevNames)
{
    int iSqlLen = strlen(pDevNames) + 300;
    char* pSQL = malloc(iSqlLen);
    memset(pSQL, 0, iSqlLen);
    if (strlen(pDevNames) != 0)
    {
        sprintf(pSQL, "select count(id) as mediacount from tbl_mediainfo where favorite = '1' and meititype='%d' and deviceidentify in('%s')", iType, pDevNames);
    }
    else
    {
        sprintf(pSQL, "select count(id) as mediacount from tbl_mediainfo where favorite = '1' and meititype='%d' ", iType);
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
        free(pSQL);
        pSQL = NULL;
		return 0;
	}
	
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pSQL);
    if(FALSE == bRet)
    {
        free(pSQL);
        pSQL = NULL;
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return 0;
    }
   
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    int iCount = DataBaseDriver_GetLong(pDataBaseDriver, "mediacount");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    free(pSQL);
    pSQL = NULL;
    return iCount;
}

// BOOL DataBaseMedia_RemoveAll()
// {
//     DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
// 	if(NULL == pDataBaseDriver)
// 	{
// 		return FALSE;
// 	}
// 	char szSQL[1024] = { 0 };
// 	strcpy(szSQL, "delete from tbl_mediainfo");
// 	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
//     if(FALSE == bRet)
//     {
// 		DataBaseDriver_CloseConn(pDataBaseDriver);
//         return FALSE;
//     }
//     DataBaseDriver_CloseConn(pDataBaseDriver);
//     return TRUE;
// }

BOOL DataBaseMedia_RemoveItem(int iID)
{
    //计数 -1
    DataBaseMediaJiShu_DecreaseFromID(iID);
    //Groupitems表也要删除
    DataBaseGroupItems_RemoveFromItemID(iID);

    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    
	char szSQL[1024] = { 0 };
	sprintf(szSQL, "delete from tbl_mediainfo where id='%d'", iID);
	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    
    return TRUE;
}

MediaItem* DataBaseMedia_GetItemFromName(const char* pszName)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return NULL;
	}
    char szSQL[1024] = { 0 };
	sprintf(szSQL, "select * from tbl_mediainfo where mediaaddr='%s'", pszName);
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);

    char* pBuffer = (char*)malloc(sizeof(MediaItem));
    memset(pBuffer, 0, sizeof(MediaItem));
    MediaItem* pItem = (MediaItem*)pBuffer;

    pItem->iId = DataBaseDriver_GetLong(pDataBaseDriver, "id");
    pItem->iPaiSheTime = DataBaseDriver_GetLong(pDataBaseDriver, "paishetime");
    DataBaseMedia_SetParam(&(pItem->pszMd5Num), DataBaseDriver_GetString(pDataBaseDriver, "md5num"));
    DataBaseMedia_SetParam(&(pItem->pszWeiZhi), DataBaseDriver_GetString(pDataBaseDriver, "weizhi"));
    pItem->iMediaType = DataBaseDriver_GetInt(pDataBaseDriver, "meititype");
    pItem->iFavorite = DataBaseDriver_GetInt(pDataBaseDriver, "favorite");
    pItem->iMeiTiSize = DataBaseDriver_GetLong(pDataBaseDriver, "meitisize");
    DataBaseMedia_SetParam(&(pItem->pszDeviceIdentify), DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify"));
    pItem->iWidth = DataBaseDriver_GetInt(pDataBaseDriver, "width");
    pItem->iHeight = DataBaseDriver_GetInt(pDataBaseDriver, "height");
    pItem->iDuration = DataBaseDriver_GetInt(pDataBaseDriver, "duration");
    DataBaseMedia_SetParam(&(pItem->pszAddr), DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
    pItem->iAddTime = DataBaseDriver_GetLong(pDataBaseDriver, "addtime");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return pItem;
}

BOOL DataBaseMedia_RemoveItemFromName(const char* pszName)
{
    MediaItem* pMediaItem = DataBaseMedia_GetItemFromName(pszName);
    if(NULL == pMediaItem)
    {
        return TRUE;
    }
    //计数 -1
    DataBaseMediaJiShu_Decrease(pMediaItem->iPaiSheTime, pMediaItem->iMediaType, pMediaItem->pszDeviceIdentify);
    //group也需要删除
    DataBaseGroupItems_RemoveFromItemID(pMediaItem->iId);
    //设置group封面为空
    DataBaseGroup_SetCoverEmpty(pMediaItem->pszAddr);

    DataBaseMedia_ReleaseItem(pMediaItem);

    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}

	char szSQL[1024] = { 0 };
	sprintf(szSQL, "delete from tbl_mediainfo where mediaaddr='%s'", pszName);
    printf("%s\n", szSQL);
	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseMedia_GetFileName(int iID, char* pRetBuffer)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
	char szSQL[1024] = { 0 };
	sprintf(szSQL, "select mediaaddr from tbl_mediainfo where id='%d'", iID);
	BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
   int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return TRUE;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    strcpy(pRetBuffer, DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr"));
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

// int DataBaseMedia_GetMediaYearCount(int iStartTime, int iEndTime, uint32_t iType, char* pDevNames)
// {
//     int iSqlLen = NULL == pDevNames?100:strlen(pDevNames) + 300;
//     char* pSQL = malloc(iSqlLen);
//     memset(pSQL, 0, iSqlLen);
//     if (NULL == pDevNames || strlen(pDevNames) != 0)
//     {
//         sprintf(pSQL, "select count(id) as mediacount from tbl_mediainfo where paishetime >= '%d' and paishetime <= '%d' and meititype='%d' and deviceidentify in('%s')", iStartTime, iEndTime, iType, pDevNames);
//     }
//     else
//     {
//         sprintf(pSQL, "select count(id) as mediacount from tbl_mediainfo where paishetime >= '%d' and paishetime <= '%d' and meititype='%d' ", iStartTime, iEndTime, iType);
//     }
//     DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
// 	if(NULL == pDataBaseDriver)
// 	{
//         free(pSQL);
//         pSQL = NULL;
// 		return 0;
// 	}
	
//     BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pSQL);
//     if(FALSE == bRet)
//     {
//         free(pSQL);
//         pSQL = NULL;
// 		DataBaseDriver_CloseConn(pDataBaseDriver);
//         return 0;
//     }
   
//     DataBaseDriver_MoveToFirst(pDataBaseDriver);
//     int iCount = DataBaseDriver_GetLong(pDataBaseDriver, "mediacount");
//     DataBaseDriver_CloseConn(pDataBaseDriver);
//     free(pSQL);
//     pSQL = NULL;
//     return iCount;
// }

// char* DataBaseMedia_GetDevNames()
// {
//     char szSQL[] = "select distinct(deviceidentify) from tbl_mediainfo";
//     DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
// 	if(NULL == pDataBaseDriver)
// 	{
// 		char* pBuffer = malloc(1);
//         memset(pBuffer, 0, 1);
//         return pBuffer;
// 	}
	
//     BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
//     if(FALSE == bRet)
//     {
// 		DataBaseDriver_CloseConn(pDataBaseDriver);
//         char* pBuffer = malloc(1);
//         memset(pBuffer, 0, 1);
//         return pBuffer;
//     }
//     char* pRetBuffer = NULL;
//     DataBaseDriver_BeforeFirst(pDataBaseDriver);
//     while(DataBaseDriver_MoveToNext(pDataBaseDriver))
//     {
//         const char* pDeviceName = DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify");
//         if(pRetBuffer == NULL)
//         {
//             pRetBuffer = malloc(strlen(pDeviceName) + 1);
//             memset(pRetBuffer, 0, strlen(pDeviceName) + 1);
//             strcpy(pRetBuffer, pDeviceName);
//         }
//         else
//         {
//             pRetBuffer = realloc(pRetBuffer, strlen(pRetBuffer) + 2 + strlen(pDeviceName));
//             strcat(pRetBuffer, "&");
//             strcat(pRetBuffer, pDeviceName);
//         }
//     }
//     DataBaseDriver_CloseConn(pDataBaseDriver);
//     if(pRetBuffer == 0)
//     {
//         char* pBuffer = malloc(1);
//         memset(pBuffer, 0, 1);
//         return pBuffer;
//     }
//     return pRetBuffer;
// }

//获取未被获取的位置
char* DataBaseMedia_GetUnCheckWeiZhi(int iLimit)
{
    char szSQL[1024] = {0};
    sprintf(szSQL, "select distinct(weizhi) from tbl_mediainfo where location='' limit %d offset 0", iLimit);
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
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        const char* pszWeiZhi = DataBaseDriver_GetString(pDataBaseDriver, "weizhi");
        char szBuffer[1024] = {0};
        sprintf(szBuffer, "{\"weizhi\":\"%s\"}", pszWeiZhi);
        if(pszRetBuffer == NULL)
        {
            pszRetBuffer = malloc(strlen(szBuffer) + 1);
            memset(pszRetBuffer, 0, strlen(szBuffer) + 1);
            strcpy(pszRetBuffer, szBuffer);
        }
        else
        {
            pszRetBuffer = realloc(pszRetBuffer, strlen(pszRetBuffer) + 2 + strlen(szBuffer));
            strcat(pszRetBuffer, ",");
            strcat(pszRetBuffer, szBuffer);
        }
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(pszRetBuffer == NULL)
    {
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
    }
    return pszRetBuffer;
}

BOOL DataBaseMedia_UpdateGpsWeiZhi(const char* pszGps, const char* pszBaiDuGps, const char* pszLocation)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
	char szSQL[1024] = { 0 };
	sprintf(szSQL, "update tbl_mediainfo set location='%s' , weizhi='%s' where weizhi='%s'", pszLocation, pszBaiDuGps, pszGps);
    printf("%s\n", szSQL);
	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

char* DataBaseMedia_YearLocationGroupTongJi(const char* pszLocation, int iYear, int iMonth, int iDay, const char* pszDevNames)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return NULL;
	}
    
    int iDevNamesLen = strlen(pszDevNames);
    char* pszSQL = (char*)malloc(iDevNamesLen + 500);
    memset(pszSQL, 0, iDevNamesLen + 500);
    strcpy(pszSQL, "SELECT meititype, COUNT(*) as tcount from tbl_mediainfo where ");
    if(iYear > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "year=%d and ", iYear);
        strcat(pszSQL, szBuffer);
    }
    if(iMonth > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "month=%d and ", iMonth);
        strcat(pszSQL, szBuffer);
    }
    if(iDay > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "day=%d and ", iDay);
        strcat(pszSQL, szBuffer);
    }
    if(strlen(pszLocation) != 0)
    {
        char szBuffer[1024] = {0};
        if(0 == strcmp(pszLocation, "待定位"))
        {
            sprintf(szBuffer, "location='%s' and ", "");
        }
        else
        {
            sprintf(szBuffer, "location='%s' and ", pszLocation);
        }
        
        strcat(pszSQL, szBuffer);
    }
    if(iDevNamesLen != 0)
    {
        char *pszBuffer = malloc(iDevNamesLen + 100);
        memset(pszBuffer, 0, iDevNamesLen + 100);
        sprintf(pszBuffer, "deviceidentify in ('%s') and ", pszDevNames);
        strcat(pszSQL, pszBuffer);
        free(pszBuffer);
        pszBuffer = NULL;
    }

    strcat(pszSQL, " 1=1 GROUP BY meititype");
    printf("%s\n", pszSQL);

    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSQL);
    free(pszSQL);
    pszSQL = NULL;
    if(FALSE == bRet)
    {
		return NULL;
    }
    char* pszRetBuffer = NULL;
    int iRetLen = 0;
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        int iMeiTiType = DataBaseDriver_GetInt(pDataBaseDriver, "meititype");
        int iMediaCount = DataBaseDriver_GetInt(pDataBaseDriver, "tcount");
        
        if(NULL == pszRetBuffer)
        {
            char szBuffer[1024] = {0};
            sprintf(szBuffer, "{\"type\":%d,\"count\":%d}", iMeiTiType, iMediaCount);
            int iLen = strlen(szBuffer) + 1;
            iRetLen = iLen - 1;
            
            pszRetBuffer = (char*)malloc(iLen);
            memset(pszRetBuffer, 0, iLen);
            strcpy(pszRetBuffer, szBuffer);
        }
        else
        {
            char szBuffer[1024] = {0};
            sprintf(szBuffer, ",{\"type\":%d,\"count\":%d}", iMeiTiType, iMediaCount);
            int iLen = strlen(szBuffer) + 1;
            
            pszRetBuffer = (char*)realloc(pszRetBuffer, iRetLen +iLen);
            memset(pszRetBuffer + iRetLen, 0, iLen);
            memcpy(pszRetBuffer + iRetLen, szBuffer, iLen - 1);
            iRetLen += iLen - 1;

        }
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(NULL == pszRetBuffer)
    {
        pszRetBuffer = malloc(1);
        memset(pszRetBuffer, 0, 1);
    }
    return pszRetBuffer;
}

BOOL DataBaseMedia_UpdateItemPaiSheShiJian(int iID, int iTime, char* pszNewFileName)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
     int iYear = 0;
    int iMonth = 0;
    int iDay = 0;
    Tools_SecInfo(iTime, &iYear, &iMonth, &iDay);

	char szSQL[1024] = { 0 };
	sprintf(szSQL, "update tbl_mediainfo set paishetime='%d',year='%d',month='%d',day='%d',mediaaddr='%s' where id='%d'", iTime, iYear, iMonth, iDay, pszNewFileName, iID);
	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseMedia_UpdateItemGpsAddr(int iID, const char* pszGps, const char* pszAddr)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    float dLat = 0;
	float dLong = 0;
	GPSTYPE iGpsType = 0;
    BOOL bGpsCheckOk = Tools_CheckGps(pszGps, &iGpsType, &dLong, &dLat);
    if(FALSE == bGpsCheckOk)
    {
        return FALSE;
    }
    char szSQL[1024] = { 0 };
    sprintf(szSQL, "update tbl_mediainfo set weizhi='%s', location='%s' where id='%d'", pszGps, pszAddr, iID);
    //printf("%s\n", szSQL);
	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseMedia_YearMonthDayCover(int iYear, int iMonth, int iDay, const char* pszDevNames, char* pszOut)
{
    if(iYear <= 0)
    {
        return FALSE;
    }
    int iSqlLen = strlen(pszDevNames) + 500;
    char* pszSql = (char*)malloc(iSqlLen);
    memset(pszSql, 0, iSqlLen);
    strcpy(pszSql, "select mediaaddr from tbl_mediainfo where ");
    if (iMonth > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "month='%d' and ", iMonth);
        strcat(pszSql, szBuffer);
    }
    if (iDay > 0)
    {
        char szBuffer[100] = {0};
        sprintf(szBuffer, "day='%d' and ", iDay);
        strcat(pszSql, szBuffer);
    }
    if (strlen(pszDevNames) > 0)
    {
        char* pszDevNamesSql = malloc(iSqlLen);
        memset(pszDevNamesSql, 0, iSqlLen);
        sprintf(pszDevNamesSql, "deviceidentify in ('%s') and ", pszDevNames);
        strcat(pszSql, pszDevNamesSql);
        free(pszDevNamesSql);
        pszDevNamesSql = NULL;
    }
    char szLastSql[200] = {0};
    sprintf(szLastSql, "year='%d' order by paishetime desc limit 0,1", iYear);
    strcat(pszSql, szLastSql);

    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
        free(pszSql);
        pszSql = NULL;
		return FALSE;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSql);
    free(pszSql);
    pszSql = NULL;
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
		return FALSE;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
	const char* pszFileName = DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr");
    if(NULL != pszFileName)
    {
        strcpy(pszOut, pszFileName);
    }
    
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

char* DataBaseMedia_GetGroupInfoFomItemID(int iID)
{
    //all gids
    char* pszGids = DataBaseGroupItems_GidsFromMediaItemID(iID);
    printf("gids:%s\n", pszGids);
    if(strlen(pszGids) == 0)
    {
        free(pszGids);
        pszGids = NULL;

        char* pszRet = malloc(1);
        memset(pszRet, 0, 1);
        return pszRet;
    }
    char* pszNewGids = Tools_ReplaceString((char*)pszGids, "&", "','");
    free(pszGids);
    pszGids = NULL;
    //{id:1,name:"11"},{id:2,name:"22",cover:"22"}
    char* pszIdNames = DataBaseGroup_GetJsonAllGroupsFromGids(pszNewGids);
    free(pszNewGids);
    pszNewGids = NULL;
    return pszIdNames;
}