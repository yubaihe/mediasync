#include "DataBaseDriver.h"
int DataBaseDriver_Callback(void *pNotUsed, int iArgc, char **pszArgv, char **pszColName)
{
	for (int i = 0; i<iArgc; ++i)
	{
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%s = %s\n", pszColName[i], pszArgv[i] ? pszArgv[i] : "NULL");
		printf("%s", szBuffer);
	}
	return 0;
}

BOOL DataBaseDriver_ExecuteSQL(DataBaseDriver* pDataBaseDriver, const char* pSQL)
{
	//printf("%s\n", pSQL);
	if (NULL == pDataBaseDriver->m_pSqlite3 || NULL == pSQL)
	{
		return FALSE;
	}

	char *pszErrMsg = NULL;
	int iRet = sqlite3_exec(pDataBaseDriver->m_pSqlite3, pSQL, DataBaseDriver_Callback, 0, &pszErrMsg);
	if (iRet != SQLITE_OK) 
	{
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "SQL error: %s", pszErrMsg);
		printf("%s\n", szBuffer);
		sqlite3_free(pszErrMsg);
		pszErrMsg = NULL;
		return FALSE;
	}
	if(NULL != pszErrMsg)
	{
		sqlite3_free(pszErrMsg);
		pszErrMsg = NULL;
	}
	return TRUE;
}

BOOL DataBaseDriver_QuerySQL(DataBaseDriver* pDataBaseDriver, const char* pSQL)
{
	if (NULL != pDataBaseDriver->m_pSqliteStmt)
	{
		sqlite3_finalize(pDataBaseDriver->m_pSqliteStmt);
		pDataBaseDriver->m_pSqliteStmt = NULL;
	}

	int iRet = sqlite3_prepare_v2(pDataBaseDriver->m_pSqlite3, pSQL, -1, &pDataBaseDriver->m_pSqliteStmt, 0);
	if (iRet != SQLITE_OK)
	{
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "QuerySQL error: %s\n", sqlite3_errmsg(pDataBaseDriver->m_pSqlite3));
		printf("%s\n", szBuffer);
		return FALSE;
	}

	return TRUE;
}

DataBaseDriver* DataBaseDriver_GetMediaDataBase()
{
	int iLen = sizeof(DataBaseDriver);
    char* pBuffer = malloc(iLen);
    memset(pBuffer, 0, iLen);
    DataBaseDriver* pDataBaseDriver = (DataBaseDriver*)pBuffer;
  
    char szDbFile[300] = {0}; 
    sprintf(szDbFile, "%s/.mediasync.db", TODIR);

    int rc = sqlite3_open_v2(szDbFile, &pDataBaseDriver->m_pSqlite3, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) 
    {
        printf("open database %s failed:%d\n", szDbFile, rc);
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;
        return NULL;
    } 
	BOOL bThreadSafe = sqlite3_threadsafe();
	if(bThreadSafe)
	{
		printf("sqlite3 thread safe\n");
	}
	else
	{
		printf("sqlite3 thread not safe\n");
	}
 	// sqlite3_exec(pDataBaseDriver->m_pSqlite3, "PRAGMA synchronous = OFF; ", 0, 0, 0);
    const char*  pszTable = "create table if not exists tbl_mediainfo(id integer PRIMARY KEY autoincrement, " \
                                    "paishetime text, year integer,month integer, day integer, md5num text, weizhi text, location text, meititype text, meitisize text, deviceidentify text, width text, height text, duration text, mediaaddr text, addtime text, favorite text DEFAULT (0))";
    //paishetime 拍摄时间
	//year 年份
	//month 月份
	//day 日期
	//md5num     MD5值
	//weizhi     拍摄时候的GPS位置
	//location     拍摄时候的位置
	//meititype  媒体类型(0:图片  1:视频)
	//meitisize  大小(MB GB...)
	//deviceidentify 设备类型(apple,android...)
	//width 宽度
	//height 高度
	//duration 持续时间
	//mediaaddr  媒体本地存储地址
	//addtime    添加时间
    if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszTable))
    {
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;

        return NULL;
    }

	const char*  pszJiShuTable = "create table if not exists tbl_mediajishu(year integer, " \
                                    "month integer, day integer, pic integer, video integer, deviceidentify text)";
    //year 年份
	//month    月份 
	//day     日
	//pic     图片计数
	//video  视频计数
	//deviceidentify 所属设备 
    if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszJiShuTable))
    {
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;

        return NULL;
    }

	const char*  pszCoverTable = "create table if not exists tbl_mediacover(covertype integer UNIQUE, " \
                                    " mediaaddr text)";
    //covertype 封面类型 0:最前面的封面 其他的为分组ID
	//mediaaddr    封面地址 
    if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszCoverTable))
    {
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;

        return NULL;
    }

	const char*  pszGroupTable = "create table if not exists tbl_mediagroup(id integer PRIMARY KEY autoincrement, " \
                                    " name text, coverpic text)";
    //id 分组ID
	//name    分组名称 
	//coverpic 分组封面图片
    if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszGroupTable))
    {
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;

        return NULL;
    }

	const char*  pszGroupItemsTable = "create table if not exists tbl_mediagroupitems(gid integer, mediaid integer, meititype integer,deviceidentify text)";
    //gid 分组ID
	//mediaid    媒体文件的唯一ID 
	//meititype 媒体文件的类型
	//deviceidentify 设备标示
    if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszGroupItemsTable))
    {
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;

        return NULL;
    }

    return pDataBaseDriver;
}

DataBaseDriver* DataBaseDriver_GetDataBase()
{
    int iLen = sizeof(DataBaseDriver);
    char* pBuffer = malloc(iLen);
    memset(pBuffer, 0, iLen);
    DataBaseDriver* pDataBaseDriver = (DataBaseDriver*)pBuffer;

	char* pszCurrPath = FileUtil_CurrentPath();   
    char szDbFile[300] = {0}; 
    sprintf(szDbFile, "%s/mediasync.db", pszCurrPath);
    free(pszCurrPath);
    pszCurrPath = NULL;

    int rc = sqlite3_open_v2(szDbFile, &pDataBaseDriver->m_pSqlite3, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) 
    {
        printf("open database mediasync.db failed:%d\n", rc);
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;
        return NULL;
    } 
	
    // const char*  pszTable = "create table if not exists tbl_mediainfo(id integer PRIMARY KEY autoincrement, " 
    //                                 "paishetime text, md5num text, weizhi text, location text, meititype text, meitisize text, deviceidentify text, width text, height text, duration text, mediaaddr text, addtime text, favorite text DEFAULT (0) )";
    // //paishetime 拍摄时间
	// //md5num     MD5值
	// //weizhi     拍摄时候的GPS位置
	// //location     拍摄时候的位置
	// //meititype  媒体类型(1:图片  2:视频)
	// //meitisize  大小(MB GB...)
	// //deviceidentify 设备类型(apple,android...)
	// //width 宽度
	// //height 高度
	// //duration 持续时间
	// //mediaaddr  媒体本地存储地址
	// //addtime    添加时间
    // if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszTable))
    // {
    //     if(NULL != pDataBaseDriver->m_pSqlite3)
    //     {
    //         sqlite3_close(pDataBaseDriver->m_pSqlite3);
    //         pDataBaseDriver->m_pSqlite3 = NULL;
    //     }
    //     free(pBuffer);
    //     pBuffer = NULL;

    //     return NULL;
    // }
	//设备信息
	const char* pszTable = "create table if not exists tbl_devinfo(devid text, devname text, devversion text, devblue text)";
    //devname 		 设备名称
	//devversion     设备版本号
	//blue           模糊度
    if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszTable))
    {
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;

        return NULL;
    }
    
	//用户信息
	pszTable = "create table if not exists tbl_userinfo(id integer PRIMARY KEY autoincrement, userpwd text, userpwdtip text)";
    //userpwd        用户密码(MD5值)
	//userpwdtip     用户密码提示信息(MD5值)
    if(FALSE == DataBaseDriver_ExecuteSQL(pDataBaseDriver, pszTable))
    {
        if(NULL != pDataBaseDriver->m_pSqlite3)
        {
            sqlite3_close(pDataBaseDriver->m_pSqlite3);
            pDataBaseDriver->m_pSqlite3 = NULL;
        }
        free(pBuffer);
        pBuffer = NULL;

        return NULL;
    }

    return pDataBaseDriver;
}
DataBaseDriver* g_pMediaDataBase = NULL;
DataBaseDriver* DataBaseDriver_GetMediaDataBaseConn()
{
	if(NULL == g_pMediaDataBase)
	{
		g_pMediaDataBase = DataBaseDriver_GetMediaDataBase();
	}

	int iLen = sizeof(DataBaseDriver);
    char* pBuffer = malloc(iLen);
    memset(pBuffer, 0, iLen);
    DataBaseDriver* pDataBaseDriver = (DataBaseDriver*)pBuffer;
	pDataBaseDriver->m_pSqlite3 = g_pMediaDataBase->m_pSqlite3;
	return pDataBaseDriver;
}

DataBaseDriver* g_pDataBase = NULL;
DataBaseDriver* DataBaseDriver_GetDataBaseConn()
{
	if(NULL == g_pDataBase)
	{
		g_pDataBase = DataBaseDriver_GetDataBase();
	}

	int iLen = sizeof(DataBaseDriver);
    char* pBuffer = malloc(iLen);
    memset(pBuffer, 0, iLen);
    DataBaseDriver* pDataBaseDriver = (DataBaseDriver*)pBuffer;
	pDataBaseDriver->m_pSqlite3 = g_pDataBase->m_pSqlite3;
	return pDataBaseDriver;
}

void DataBaseDriver_CloseConn(DataBaseDriver* pDataBaseDriver)
{
	if(NULL == pDataBaseDriver)
    {
        return;
    }
    
    if (NULL != pDataBaseDriver->m_pSqliteStmt)
	{
		sqlite3_finalize(pDataBaseDriver->m_pSqliteStmt);
		pDataBaseDriver->m_pSqliteStmt = NULL;
	}

	// if(NULL != pDataBaseDriver->m_pSqlite3)
    // {
    //     sqlite3_close(pDataBaseDriver->m_pSqlite3);
    //     pDataBaseDriver->m_pSqlite3 = NULL;
    // }
    if(NULL != pDataBaseDriver->pstrValue)
    {
        free(pDataBaseDriver->pstrValue);
        pDataBaseDriver->pstrValue = NULL;
    }
    if(NULL != pDataBaseDriver)
    {
        free(pDataBaseDriver);
        pDataBaseDriver = NULL;
    }
}

void DataBaseDriver_Close(DataBaseDriver* pDataBaseDriver)
{
    if(NULL == pDataBaseDriver)
    {
        return;
    }
    
    if (NULL != pDataBaseDriver->m_pSqliteStmt)
	{
		sqlite3_finalize(pDataBaseDriver->m_pSqliteStmt);
		pDataBaseDriver->m_pSqliteStmt = NULL;
	}

	if(NULL != pDataBaseDriver->m_pSqlite3)
    {
        sqlite3_close(pDataBaseDriver->m_pSqlite3);
        pDataBaseDriver->m_pSqlite3 = NULL;
    }
    if(NULL != pDataBaseDriver->pstrValue)
    {
        free(pDataBaseDriver->pstrValue);
        pDataBaseDriver->pstrValue = NULL;
    }
    if(NULL != pDataBaseDriver)
    {
        free(pDataBaseDriver);
        pDataBaseDriver = NULL;
    }
}

BOOL DataBaseDriver_Init()
{
	if(NULL == g_pDataBase)
	{
		g_pDataBase = DataBaseDriver_GetDataBase();
		if(NULL == g_pDataBase)
		{
			return FALSE;
		}
	}
	if(NULL == g_pMediaDataBase)
	{
		g_pMediaDataBase = DataBaseDriver_GetMediaDataBase();
		if(NULL == g_pMediaDataBase)
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

void DataBaseDriver_Release()
{
	DataBaseDriver_Close(g_pDataBase);
	DataBaseDriver_Close(g_pMediaDataBase);
}
int DataBaseDriver_GetCount(DataBaseDriver* pDataBaseDriver) 
{
	if (NULL == pDataBaseDriver->m_pSqliteStmt)
	{
		return 0;
	}

	int iCount = 0;
	while (sqlite3_step(pDataBaseDriver->m_pSqliteStmt) == SQLITE_ROW)
	{
		iCount++;
	}

	sqlite3_reset(pDataBaseDriver->m_pSqliteStmt);
	return iCount;
}

BOOL DataBaseDriver_MoveToNext(DataBaseDriver* pDataBaseDriver)
{
	if (sqlite3_step(pDataBaseDriver->m_pSqliteStmt) != SQLITE_ROW)
	{
		return FALSE;
	}

	return TRUE;
}

void DataBaseDriver_MoveToFirst(DataBaseDriver* pDataBaseDriver)
{
	sqlite3_reset(pDataBaseDriver->m_pSqliteStmt);
	DataBaseDriver_MoveToNext(pDataBaseDriver);
}

void DataBaseDriver_BeforeFirst(DataBaseDriver* pDataBaseDriver)
{
	sqlite3_reset(pDataBaseDriver->m_pSqliteStmt);
}

int DataBaseDriver_GetIndex(DataBaseDriver* pDataBaseDriver, const char* pKey)
{
	int iCount = sqlite3_column_count(pDataBaseDriver->m_pSqliteStmt);
	for (int i = 0; i < iCount; ++i)
	{
		const char *pName = sqlite3_column_name(pDataBaseDriver->m_pSqliteStmt, i);
		if (strcmp(pName, pKey) == 0)
		{
			return i;
		}
	}

	return -1;
}

long DataBaseDriver_GetLong(DataBaseDriver* pDataBaseDriver, const char* pKey)
{
	int iIndex = DataBaseDriver_GetIndex(pDataBaseDriver, pKey);
	int iType = sqlite3_column_type(pDataBaseDriver->m_pSqliteStmt, iIndex);
	long iRet = 0;
	switch (iType)
	{
		case SQLITE_NULL:
		{
			iRet = 0;
			break;
		}
		case SQLITE_INTEGER:
		{
			iRet = sqlite3_column_int(pDataBaseDriver->m_pSqliteStmt, iIndex);
			break;
		}
		case SQLITE_FLOAT:
		{
			iRet = (long)sqlite3_column_double(pDataBaseDriver->m_pSqliteStmt, iIndex);
			break;
		}

		case SQLITE_BLOB:
		{
			iRet = -1;
			break;
		}
		case SQLITE_TEXT:
		{
			iRet = atol((const char*)sqlite3_column_text(pDataBaseDriver->m_pSqliteStmt, iIndex));
			break;
		}
		default:
		{
			iRet = -1;
			// char szBuffer[100] = { 0 };
			// sprintf(szBuffer, "%s: ERROR [%s]\n", pKey, sqlite3_errmsg(pDataBaseDriver->m_pSqlite3));
			// OutputDebugStringA(szBuffer);
			break;
		}
	}
	return iRet;
}

int DataBaseDriver_GetInt(DataBaseDriver* pDataBaseDriver, const char* pKey)
{
	long iRet = DataBaseDriver_GetLong(pDataBaseDriver, pKey);
	if (iRet >= 0)
	{
		return iRet;
	}

	return -1;
}

const char* DataBaseDriver_GetString(DataBaseDriver* pDataBaseDriver, const char* pKey)
{
	int iIndex = DataBaseDriver_GetIndex(pDataBaseDriver, pKey);
	int iType = sqlite3_column_type(pDataBaseDriver->m_pSqliteStmt, iIndex);
    if(NULL != pDataBaseDriver->pstrValue)
    {
        free(pDataBaseDriver->pstrValue);
        pDataBaseDriver->pstrValue = NULL;
    }
	switch (iType)
	{
		case SQLITE_NULL:
		{
			break;
		}
		case SQLITE_INTEGER:
		{
			int32_t iValue = sqlite3_column_int(pDataBaseDriver->m_pSqliteStmt, iIndex);
            char* pBuffer = malloc(100);
            memset(pBuffer, 0, 100);
            pDataBaseDriver->pstrValue = pBuffer;
			sprintf(pBuffer, "%d", iValue);
			break;
		}
		case SQLITE_FLOAT:
		{
            char* pBuffer = malloc(100);
            memset(pBuffer, 0, 100);
            pDataBaseDriver->pstrValue = pBuffer;

			double dValue = sqlite3_column_double(pDataBaseDriver->m_pSqliteStmt, iIndex);
			sprintf(pBuffer, "%.1f", dValue);
			break;
		}

		case SQLITE_BLOB:
		{
			const char* pData = (const char*)sqlite3_column_blob(pDataBaseDriver->m_pSqliteStmt, iIndex);
			int nDataLen = sqlite3_column_bytes(pDataBaseDriver->m_pSqliteStmt, iIndex);

            char* pBuffer = malloc(nDataLen);
            memset(pBuffer, 0, nDataLen);
            pDataBaseDriver->pstrValue = pBuffer;
            memcpy(pBuffer, (const char*)pData, nDataLen);
			break;
		}
		case SQLITE_TEXT:
		{
			const  char* pText = (const char*)sqlite3_column_text(pDataBaseDriver->m_pSqliteStmt, iIndex);
            int iLen = strlen(pText) + 1;
            char* pBuffer = malloc(iLen);
            memset(pBuffer, 0, iLen);
            pDataBaseDriver->pstrValue = pBuffer;
            memcpy(pBuffer, (const char*)pText, iLen - 1);
			break;
		}
		default:
		{
			// char szBuffer[100] = { 0 };
			// sprintf(szBuffer, "%s: ERROR [%s]\n", pKey, sqlite3_errmsg(pDataBaseDriver->m_pSqlite3));
			// OutputDebugStringA(szBuffer);
			break;
		}
	}

	return pDataBaseDriver->pstrValue;
}

