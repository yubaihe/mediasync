#include "DataBaseCover.h"
#include "DataBaseDriver.h"

BOOL DataBaseCover_SetHomeCover(const char* pszMediaAddr)
{
    if(strlen(pszMediaAddr) == 0)
    {
        return DataBaseCover_RemoveHomeCover();
    }
    char szSQL[300] = {0};
    char* pHomeCover = DataBaseCover_GetHomeCover();
    if(strlen(pHomeCover) > 0)
    {
        sprintf(szSQL, "update tbl_mediacover set mediaaddr='%s' where covertype='%d'", pszMediaAddr, DATABASECOVER_HOME);
    }
    else
    {
        sprintf(szSQL, "insert into tbl_mediacover(covertype, mediaaddr)values('%d','%s')", DATABASECOVER_HOME, pszMediaAddr);
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
    {
        free(pHomeCover);
        pHomeCover = NULL;
        return FALSE;
    }
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    free(pHomeCover);
    pHomeCover = NULL;
    return bRet;
}

char* DataBaseCover_GetHomeCover()
{
    char szSQL[200] = {0};
    sprintf(szSQL, "select * from tbl_mediacover where covertype='%d'", DATABASECOVER_HOME);
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
    const char* pszMediaAddr = DataBaseDriver_GetString(pDataBaseDriver, "mediaaddr");
    int iRetLen = strlen(pszMediaAddr) + 1;
    char* pszRet = malloc(iRetLen);
    memset(pszRet, 0, iRetLen);
    strcpy(pszRet, pszMediaAddr);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    if(strlen(pszRet) == 0)
    {
        DataBaseCover_RemoveHomeCover();
    }
    return pszRet;
}

BOOL DataBaseCover_RemoveHomeCover()
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetMediaDataBaseConn();
    if(NULL == pDataBaseDriver)
	{
        return FALSE;
	}
    char szSQL[200] = {0};
    sprintf(szSQL, "delete from tbl_mediacover where covertype='%d'", DATABASECOVER_HOME);
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    printf("%s\n", szSQL);
    return bRet;
}