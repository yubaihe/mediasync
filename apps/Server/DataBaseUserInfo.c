#include "DataBaseUserInfo.h"
void DataBaseUserInfo_SetParam(char** pKey, const char* pValue)
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
BOOL DataBaseUserInfo_GetRecordCount(int* iCount)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szSQL[] = "select count(id) as recordcount from tbl_userinfo";
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

BOOL DataBaseUserInfo_AddRecord(UserItem* pItem)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szSQL[2048] = "select id from tbl_userinfo";
    BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    char* pszUserPwdTip = Tools_ReplaceString(pItem->pszUserPwdTip, "'", "''");

    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        sprintf(szSQL, "insert into tbl_userinfo(userpwd, userpwdtip)values \
												('%s', '%s')",  
    				pItem->pszUserPwd, pszUserPwdTip);
    }
    else
    {
        DataBaseDriver_MoveToFirst(pDataBaseDriver);
        int iID = DataBaseDriver_GetInt(pDataBaseDriver, "id");
        sprintf(szSQL, "update tbl_userinfo set userpwd='%s', userpwdtip='%s' where id='%d'",  
    				pItem->pszUserPwd, pszUserPwdTip, iID);
    }
    free(pszUserPwdTip);
    pszUserPwdTip = NULL;
	bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseUserInfo_DeleteRecord()
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szSQL[] = "delete from tbl_userinfo";
    BOOL bRet = DataBaseDriver_ExecuteSQL(pDataBaseDriver, szSQL);
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return bRet;
}

BOOL DataBaseUserInfo_CheckPwd(const char* pszPwd)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szSQL[2048] = { 0 };
    sprintf(szSQL, "select * from tbl_userinfo where userpwd='%s'",  
    				pszPwd);
	BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}

BOOL DataBaseUserInfo_GetPwdTip(char** pszTip)
{
    DataBaseDriver* pDataBaseDriver = DataBaseDriver_GetDataBaseConn();
	if(NULL == pDataBaseDriver)
	{
		return FALSE;
	}
    char szSQL[2048] = "select * from tbl_userinfo";
	BOOL bRet = DataBaseDriver_QuerySQL(pDataBaseDriver, szSQL);
    if(FALSE == bRet)
    {
		DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    if(DataBaseDriver_GetCount(pDataBaseDriver) == 0)
    {
        DataBaseDriver_CloseConn(pDataBaseDriver);
        return FALSE;
    }
    DataBaseDriver_MoveToFirst(pDataBaseDriver);
    DataBaseUserInfo_SetParam(pszTip, DataBaseDriver_GetString(pDataBaseDriver, "userpwdtip"));
    DataBaseDriver_CloseConn(pDataBaseDriver);
    return TRUE;
}