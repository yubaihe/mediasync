#include "DataBaseDevice.h"
void DataBaseDevice_SetParam(char** pKey, const char* pValue)
{
    if(NULL == pValue)
    {
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

DevItem* DataBaseDevice_GetDevInfo()
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return NULL;
	}
    if (NULL == pDataBaseDriver->m_pSqlite3)
	{
		DataBaseDriver_CloseConn(pDataBaseDriver);
		return NULL;
	}
    char szBuffer[] = "select * from tbl_devinfo";

    DataBaseDriver_QuerySQL(pDataBaseDriver, szBuffer);
    int iCount = DataBaseDriver_GetCount(pDataBaseDriver);
    if(iCount == 0)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return NULL;
    }
    int iBufferLen = sizeof(DevItem);
    char* pBuffer = (char*)malloc(iBufferLen);
    memset(pBuffer, 0, iBufferLen);
    DevItem* pItem = (DevItem*)pBuffer;
    
	DataBaseDriver_MoveToFirst(pDataBaseDriver);
	const char* pszID = DataBaseDriver_GetString(pDataBaseDriver, "devid");
	DataBaseDevice_SetParam(&(pItem->pszDevID), pszID);
    const char* pszDevName = DataBaseDriver_GetString(pDataBaseDriver, "devname");
	DataBaseDevice_SetParam(&(pItem->pszDevName), pszDevName);
    const char* pszDevVersion = DataBaseDriver_GetString(pDataBaseDriver, "devversion");
	DataBaseDevice_SetParam(&(pItem->pszDevVersion), pszDevVersion);
    const char* pszDevBlue = DataBaseDriver_GetString(pDataBaseDriver, "devblue");
	DataBaseDevice_SetParam(&(pItem->pszDevBlue), pszDevBlue);

	DataBaseDriver_CloseConn(pDataBaseDriver);
    return pItem;
}

BOOL  DataBaseDevice_RemoveAll()
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szSQL[] = "DELETE FROM tbl_devinfo";
	BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL  DataBaseDevice_AddOrUpdateDevItem(DevItem* pItem)
{
    DevItem* pOldItem = DataBaseDevice_GetDevInfo();
    char szSQL[255] = {0};
    if(NULL == pOldItem)
    {
        sprintf(szSQL, "insert into tbl_devinfo(devid, devname, devversion, devblue)values \
												('%s', '%s', '%s', '%s')",  
    				pItem->pszDevID, pItem->pszDevName, pItem->pszDevVersion, pItem->pszDevBlue);
    }
    else
    {
        if(NULL != pItem->pszDevName)
        {
            int iLen = strlen(pItem->pszDevName);
            free(pOldItem->pszDevName);
            pOldItem->pszDevName = NULL;
            pOldItem->pszDevName = (char*)malloc(iLen + 1);
            memset(pOldItem->pszDevName, 0, iLen + 1);
            strcpy(pOldItem->pszDevName, pItem->pszDevName);
        }

        if(NULL != pItem->pszDevBlue)
        {
            int iLen = strlen(pItem->pszDevBlue) + 1;
            free(pOldItem->pszDevBlue);
            pOldItem->pszDevBlue = NULL;
            pOldItem->pszDevBlue = (char*)malloc(iLen + 1);
            memset(pOldItem->pszDevBlue, 0, iLen + 1);
            strcpy(pOldItem->pszDevBlue, pItem->pszDevBlue);
        }
        char* pszFilterDeviceID = Tools_ReplaceString(pOldItem->pszDevName, "'", "''");
        sprintf(szSQL, "update tbl_devinfo set devname='%s', devblue='%s' where devid='%s'",  
    				pszFilterDeviceID, pOldItem->pszDevBlue, pOldItem->pszDevID);
        free(pszFilterDeviceID);
        pszFilterDeviceID = NULL;
    }
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
         DataBaseDevice_ReleaseItem(pOldItem);
		return FALSE;
	}
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        DataBaseDevice_ReleaseItem(pOldItem);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    DataBaseDevice_ReleaseItem(pOldItem);
    return TRUE;
}

void DataBaseDevice_ReleaseItem(DevItem* pItem)
{
    if(NULL == pItem)
    {
        return;
    }
    if(NULL != pItem->pszDevID)
    {
        free(pItem->pszDevID);
        pItem->pszDevID = NULL;
    }
    if(NULL != pItem->pszDevName)
    {
        free(pItem->pszDevName);
        pItem->pszDevName = NULL;
    }
    if(NULL != pItem->pszDevVersion)
    {
        free(pItem->pszDevVersion);
        pItem->pszDevVersion = NULL;
    }
    if(NULL != pItem->pszDevBlue)
    {
        free(pItem->pszDevBlue);
        pItem->pszDevBlue = NULL;
    }
    free((char*)pItem);
    pItem = NULL;
}