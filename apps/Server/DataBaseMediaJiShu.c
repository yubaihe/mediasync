#include "DataBaseMediaJiShu.h"
#include "DataBaseMedia.h"
#include "DataBaseDriver.h"


BOOL DataBaseMediaJiShu_ReSet(ResetJiShuCallBack callBack, LPVOID* lpParameter)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char* pszSql = "delete from tbl_mediajishu";
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszSql);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    char* pszSql2 = "select meititype,paishetime,deviceidentify from tbl_mediainfo";
    bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSql2);
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    int iCursorIndex = 0;
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        iCursorIndex++;
        //pItem->iId = DataBaseDriver_GetLong(pDataBaseDriver, "id");
        int iMediaType = DataBaseDriver_GetInt(pDataBaseDriver, "meititype");
        long iPaiSheTime = DataBaseDriver_GetLong(pDataBaseDriver, "paishetime");
        const char* pszDeviceIdentify = DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify");
        BOOL bRet = DataBaseMediaJiShu_Increase(iPaiSheTime, iMediaType, pszDeviceIdentify);
        if(FALSE == bRet)
        {
            break;
        }
        callBack((int)(iCursorIndex*100.0)/iCount, lpParameter);
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseMediaJiShu_Increase(long iPaiSheTime, int iMediaType, const char* pszIdentify)
{
    int iIdentifyLen = strlen(pszIdentify);
    int iSqlLen = iIdentifyLen + 500;
    char* pszSql = malloc(iSqlLen);
    memset(pszSql, 0, iSqlLen);

    int iYear;
    int iMonth;
    int iDay;
    Tools_SecInfo(iPaiSheTime, &iYear, &iMonth, &iDay);
    sprintf(pszSql, "select pic,video from tbl_mediajishu where year='%d' and month='%d' and day='%d' and deviceidentify='%s'", iYear, iMonth, iDay, pszIdentify);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
        free(pszSql);
        pszSql = NULL;
		return FALSE;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSql);
    if(FALSE == bRet)
    {
        free(pszSql);
        pszSql = NULL;
        return FALSE;
    }
    int iPicCount = 0;
    int iVideoCount = 0;
    if(DataBaseDriver_GetCount(pDataBaseDriver) != 0)
    {
        DataBaseDriver_MoveToFirst(pDataBaseDriver);
        iPicCount = DataBaseDriver_GetInt(pDataBaseDriver, "pic");
        iVideoCount = DataBaseDriver_GetInt(pDataBaseDriver, "video");
    }

    switch (iMediaType)
    {
        case MEDIATYPE_IMAGE:
        {
            if (iPicCount == 0 && iVideoCount == 0)
            {
                sprintf(pszSql, "insert into tbl_mediajishu(pic,video,year,month,day,deviceidentify)values('%d','%d','%d','%d','%d','%s')",
                    1, 0, iYear, iMonth, iDay, pszIdentify);
            }
            else
            {
                sprintf(pszSql, "update tbl_mediajishu set pic=%d where year='%d' and month='%d' and day='%d' and deviceidentify='%s'",
                    iPicCount + 1, iYear, iMonth, iDay, pszIdentify);
            }
            
            break;
        }
        case MEDIATYPE_VIDEO:
        {
             if (iPicCount == 0 && iVideoCount == 0)
            {
                sprintf(pszSql, "insert into tbl_mediajishu(pic,video,year,month,day,deviceidentify)values('%d','%d','%d','%d','%d','%s')",
                    0, 1, iYear, iMonth, iDay, pszIdentify);
            }
            else
            {
                sprintf(pszSql, "update tbl_mediajishu set video=%d where year='%d' and month='%d' and day='%d' and deviceidentify='%s'",
                    iVideoCount + 1, iYear, iMonth, iDay, pszIdentify);
            }
            break;
        }
    }
     printf("%s\n", pszSql);
    bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszSql);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    free(pszSql);
    pszSql = NULL;
    return bRet;
}

BOOL DataBaseMediaJiShu_Decrease(long iPaiSheTime, int iMediaType, const char* pszIdentify)
{
    int iIdentifyLen = strlen(pszIdentify);
    int iSqlLen = iIdentifyLen + 500;
    char* pszSql = malloc(iSqlLen);
    memset(pszSql, 0, iSqlLen);

    int iYear;
    int iMonth;
    int iDay;
    Tools_SecInfo(iPaiSheTime, &iYear, &iMonth, &iDay);
    sprintf(pszSql, "select pic,video from tbl_mediajishu where year='%d' and month='%d' and day='%d' and deviceidentify='%s'", iYear, iMonth, iDay, pszIdentify);
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
        free(pszSql);
        pszSql = NULL;
		return FALSE;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSql);
    if(FALSE == bRet)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        free(pszSql);
        pszSql = NULL;
        return FALSE;
    }
    int iPicCount = 0;
    int iVideoCount = 0;
    if(DataBaseDriver_GetCount(pDataBaseDriver) != 0)
    {
        DataBaseDriver_MoveToFirst(pDataBaseDriver);
        iVideoCount = DataBaseDriver_GetInt(pDataBaseDriver, "video");
        iPicCount = DataBaseDriver_GetInt(pDataBaseDriver, "pic");
    }
    printf("piccount:%d iVideoCount:%d", iPicCount, iVideoCount);
    switch (iMediaType)
    {
        case MEDIATYPE_IMAGE:
        {
            if (iPicCount == 0 && iVideoCount == 0)
            {
                //都为0表示不存在了
                DataBaseDriver_CloseConn(pDataBaseDriver);
            }
            else if (iPicCount == 1 && iVideoCount == 0)
            {
               sprintf(pszSql, "delete from tbl_mediajishu where year='%d' and month='%d' and day='%d' and deviceidentify='%s'",
                    iYear, iMonth, iDay, pszIdentify);
                    printf("%s\n", pszSql);
                bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszSql);
                DataBaseDriver_CloseConn(pDataBaseDriver);
            }
            else
            {
                 sprintf(pszSql, "update tbl_mediajishu set pic=%d where year='%d' and month='%d' and day='%d' and deviceidentify='%s'",
                    iPicCount - 1, iYear, iMonth, iDay, pszIdentify);
                    printf("%s\n", pszSql);
                bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszSql);
                DataBaseDriver_CloseConn(pDataBaseDriver);
            }
            
            break;
        }
        case MEDIATYPE_VIDEO:
        {
            if (iPicCount == 0 && iVideoCount == 0)
            {
                //都为0表示不存在了
                DataBaseDriver_CloseConn(pDataBaseDriver);
            }
            else if (iVideoCount == 1 && iPicCount == 0)
            {
               sprintf(pszSql, "delete from tbl_mediajishu where year='%d' and month='%d' and day='%d' and deviceidentify='%s'",
                    iYear, iMonth, iDay, pszIdentify);
                bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszSql);
                DataBaseDriver_CloseConn(pDataBaseDriver);
            }
            else
            {
                 sprintf(pszSql, "update tbl_mediajishu set video=%d where year='%d' and month='%d' and day='%d' and deviceidentify='%s'",
                    iVideoCount - 1, iYear, iMonth, iDay, pszIdentify);
                bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszSql);
                DataBaseDriver_CloseConn(pDataBaseDriver);
            }
            break;
        }
    }
    free(pszSql);
    pszSql = NULL;
    return bRet;
}


BOOL DataBaseMediaJiShu_IncreaseFromID(long iMediaID)
{
    MediaItem* pMediaItem = DataBaseMedia_GetItemByID(iMediaID);
    BOOL bRet = DataBaseMediaJiShu_Increase(pMediaItem->iPaiSheTime, pMediaItem->iMediaType, pMediaItem->pszDeviceIdentify);
    DataBaseMedia_ReleaseItem(pMediaItem);
    return bRet;
}

BOOL DataBaseMediaJiShu_DecreaseFromID(long iMediaID)
{
    MediaItem* pMediaItem = DataBaseMedia_GetItemByID(iMediaID);
    BOOL bRet = DataBaseMediaJiShu_Decrease(pMediaItem->iPaiSheTime, pMediaItem->iMediaType, pMediaItem->pszDeviceIdentify);
    DataBaseMedia_ReleaseItem(pMediaItem);
    return bRet;
}


BOOL DataBaseMediaJiShu_GetYearInfo(int iYear, char* pszDeviceName, int* piPicCount, int* piVideoCount)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szBuffer[2048] = {0};
    if (strlen(pszDeviceName) > 0)
    {
        sprintf(szBuffer, "select year, sum(pic) as pic,sum(video) as video,deviceidentify from tbl_mediajishu where year='%d' and deviceidentify in('%s') group by year order by year desc",
        iYear, pszDeviceName);
    }
    else
    {
        sprintf(szBuffer, "select year, sum(pic) as pic,sum(video) as video,deviceidentify from tbl_mediajishu where year='%d' group by year order by year desc",
        iYear);
    }
    
    //printf("%s\n", szBuffer);
    DataBaseDriver_QuerySQL(pDataBaseDriver, szBuffer);
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    *piPicCount = DataBaseDriver_GetInt(pDataBaseDriver, "pic");
    *piVideoCount = DataBaseDriver_GetInt(pDataBaseDriver, "video");
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

char* DataBaseMediaJiShu_GetYears(char* pszDeviceName)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
	}
    char szBuffer[2048] = {0};
    if (NULL == pszDeviceName || strlen(pszDeviceName) == 0)
    {
        sprintf(szBuffer, "select distinct(year) as year from tbl_mediajishu order by year desc");
    }
    else
    {
        sprintf(szBuffer, "select distinct(year) as year from tbl_mediajishu where deviceidentify in('%s') order by year desc",
            pszDeviceName);
    }
    //printf("%s\n", szBuffer);
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szBuffer);
   if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
    }
    char* pRetBuffer = NULL;
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        const char* pszYear = DataBaseDriver_GetString(pDataBaseDriver, "year");
        if(pRetBuffer == NULL)
        {
            pRetBuffer = malloc(strlen(pszYear) + 1);
            memset(pRetBuffer, 0, strlen(pszYear) + 1);
            strcpy(pRetBuffer, pszYear);
        }
        else
        {
            pRetBuffer = realloc(pRetBuffer, strlen(pRetBuffer) + 2 + strlen(pszYear));
            strcat(pRetBuffer, "&");
            strcat(pRetBuffer, pszYear);
        }
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(pRetBuffer == NULL)
    {
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
    }
    return pRetBuffer;
}

char* DataBaseMediaJiShu_GetDevNames()
{
    char szSQL[] = "select distinct(deviceidentify) from tbl_mediajishu";
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
    char* pRetBuffer = NULL;
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        const char* pDeviceName = DataBaseDriver_GetString(pDataBaseDriver, "deviceidentify");
        if(pRetBuffer == NULL)
        {
            pRetBuffer = malloc(strlen(pDeviceName) + 100);
            memset(pRetBuffer, 0, strlen(pDeviceName) + 100);
            sprintf(pRetBuffer, "{\"name\":\"%s\"}", pDeviceName);
        }
        else
        {
            char szBuffer[500] = {0};
            sprintf(szBuffer, ",{\"name\":\"%s\"}", pDeviceName);

            pRetBuffer = realloc(pRetBuffer, strlen(pRetBuffer) + 2 + strlen(szBuffer));
            strcat(pRetBuffer, szBuffer);
        }
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(pRetBuffer == 0)
    {
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        return pBuffer;
    }
    return pRetBuffer;
}


char* DataBaseMediaJiShu_GetMonths(int iYear, char* pszDeviceName)
{
    int iSqlLen = strlen(pszDeviceName) + 1024;
    char* pszSql = malloc(iSqlLen);
    memset(pszSql, 0, iSqlLen);
    if(strlen(pszDeviceName) == 0)
    {
        sprintf(pszSql, "select month, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' group by month order by month asc", iYear);
    }
    else
    {
        sprintf(pszSql, "select month, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' and deviceidentify in('%s') group by month order by month asc", iYear, pszDeviceName);
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        free(pszSql);
        pszSql = NULL;
        return pBuffer;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSql);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        free(pszSql);
        pszSql = NULL;
        return pBuffer;
    }
    char* pRetBuffer = NULL;
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        int iMonth = DataBaseDriver_GetInt(pDataBaseDriver, "month");
        int iPicCount = DataBaseDriver_GetInt(pDataBaseDriver, "PicCount");
        int iVideoCount = DataBaseDriver_GetInt(pDataBaseDriver, "VideoCount");
        char szBuffer[1024] = {0};
        sprintf(szBuffer, "{\"month\":%d,\"piccount\":%d,\"videocount\":%d}", iMonth, iPicCount, iVideoCount);
        if(pRetBuffer == NULL)
        {
            pRetBuffer = malloc(strlen(szBuffer) + 1);
            memset(pRetBuffer, 0, strlen(szBuffer) + 1);
            strcpy(pRetBuffer, szBuffer);
        }
        else
        {
            pRetBuffer = realloc(pRetBuffer, strlen(pRetBuffer) + 2 + strlen(szBuffer));
            strcat(pRetBuffer, ",");
            strcat(pRetBuffer, szBuffer);
        }
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(pRetBuffer == 0)
    {
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        free(pszSql);
        pszSql = NULL;
        return pBuffer;
    }
    free(pszSql);
    pszSql = NULL;
    return pRetBuffer;
}

char* DataBaseMediaJiShu_GetDays(int iYear, int iMonth, char* pszDeviceName)
{
    int iSqlLen = strlen(pszDeviceName) + 1024;
    char* pszSql = malloc(iSqlLen);
    memset(pszSql, 0, iSqlLen);
    if(strlen(pszDeviceName) == 0)
    {
        sprintf(pszSql, "select day, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' and month='%d' group by day order by day asc", iYear, iMonth);
    }
    else
    {
        sprintf(pszSql, "select day, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' and month='%d' and deviceidentify in('%s') group by day order by day asc", iYear, iMonth, pszDeviceName);
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        free(pszSql);
        pszSql = NULL;
        return pBuffer;
	}
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, pszSql);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        free(pszSql);
        pszSql = NULL;
        return pBuffer;
    }
    char* pRetBuffer = NULL;
    DataBaseDriver_BeforeFirst(pDataBaseDriver);
    while(DataBaseDriver_MoveToNext(pDataBaseDriver))
    {
        int iDay = DataBaseDriver_GetInt(pDataBaseDriver, "day");
        int iPicCount = DataBaseDriver_GetInt(pDataBaseDriver, "PicCount");
        int iVideoCount = DataBaseDriver_GetInt(pDataBaseDriver, "VideoCount");
        char szBuffer[1024] = {0};
        sprintf(szBuffer, "{\"day\":%d,\"piccount\":%d,\"videocount\":%d}", iDay, iPicCount, iVideoCount);
        if(pRetBuffer == NULL)
        {
            pRetBuffer = malloc(strlen(szBuffer) + 1);
            memset(pRetBuffer, 0, strlen(szBuffer) + 1);
            strcpy(pRetBuffer, szBuffer);
        }
        else
        {
            pRetBuffer = realloc(pRetBuffer, strlen(pRetBuffer) + 2 + strlen(szBuffer));
            strcat(pRetBuffer, ",");
            strcat(pRetBuffer, szBuffer);
        }
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(pRetBuffer == 0)
    {
        char* pBuffer = malloc(1);
        memset(pBuffer, 0, 1);
        free(pszSql);
        pszSql = NULL;
        return pBuffer;
    }
    free(pszSql);
    pszSql = NULL;
    return pRetBuffer;
}