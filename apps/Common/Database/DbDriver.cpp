#include "DbDriver.h"
#include "../stdafx.h"
CDbCursor::CDbCursor()
{
    m_pSqliteStmt = NULL;
}
CDbCursor::~CDbCursor()
{
    Close();
}
void CDbCursor::Close()
{
    if(NULL != m_pSqliteStmt)
    {
        sqlite3_finalize(m_pSqliteStmt);
		m_pSqliteStmt = NULL;
    }
}
int CDbCursor::GetCount()
{
    if (NULL == m_pSqliteStmt)
	{
		return 0;
	}

	int iCount = 0;
	while (sqlite3_step(m_pSqliteStmt) == SQLITE_ROW)
	{
		iCount++;
	}

	sqlite3_reset(m_pSqliteStmt);
	return iCount;
}
BOOL CDbCursor::Next()
{
    if (sqlite3_step(m_pSqliteStmt) != SQLITE_ROW)
	{
		return FALSE;
	}
	return TRUE;
}
void CDbCursor::First()
{
    sqlite3_reset(m_pSqliteStmt);
	Next();
}
string CDbCursor::GetString(string strKey, string strDefault/* = ""*/)
{
    int iIndex = GetIndex(strKey);
	int iType = sqlite3_column_type(m_pSqliteStmt, iIndex);
    char* pszBuffer = NULL;
	switch (iType)
	{
		case SQLITE_NULL:
		{
			break;
		}
		case SQLITE_INTEGER:
		{
			int32_t iValue = sqlite3_column_int(m_pSqliteStmt, iIndex);
            pszBuffer = (char*)malloc(100);
            memset(pszBuffer, 0, 100);
			sprintf(pszBuffer, "%d", iValue);
			break;
		}
		case SQLITE_FLOAT:
		{
            pszBuffer = (char*)malloc(100);
            memset(pszBuffer, 0, 100);
			double dValue = sqlite3_column_double(m_pSqliteStmt, iIndex);
            sprintf(pszBuffer, "%f", dValue);
			break;
		}

		case SQLITE_BLOB:
		{
			const char* pszData = (const char*)sqlite3_column_blob(m_pSqliteStmt, iIndex);
			int iDataLen = sqlite3_column_bytes(m_pSqliteStmt, iIndex);
            pszBuffer = (char*)malloc(iDataLen);
            memset(pszBuffer, 0, iDataLen);
            memcpy(pszBuffer, (const char*)pszData, iDataLen);
			break;
		}
		case SQLITE_TEXT:
		{
			const  char* pszData = (const char*)sqlite3_column_text(m_pSqliteStmt, iIndex);
            int iDataLen = sqlite3_column_bytes(m_pSqliteStmt, iIndex);
            pszBuffer = (char*)malloc(iDataLen + 1);
            memset(pszBuffer, 0, iDataLen + 1);
            memcpy(pszBuffer, (const char*)pszData, iDataLen);
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
    if(NULL == pszBuffer)
    {
        return strDefault;
    }
    string strRet(pszBuffer);
    free(pszBuffer);
    pszBuffer = NULL;
    return  strRet;
}
int CDbCursor::GetInt(string strKey, int iDefault/* =  0*/)
{
    string strValue = GetString(strKey);
    if(strValue.length() == 0)
    {
        return iDefault;
    }
    return atoi(strValue.c_str());
}
long CDbCursor::GetLong(string strKey, long iDefault/* = 0*/)
{
    string strValue = GetString(strKey);
    if(strValue.length() == 0)
    {
        return iDefault;
    }
    return atol(strValue.c_str());
}
int CDbCursor::GetIndex(string strKey)
{
    int iCount = sqlite3_column_count(m_pSqliteStmt);
	for (int i = 0; i < iCount; ++i)
	{
		const char* pszColumnName = sqlite3_column_name(m_pSqliteStmt, i);
		if (strcmp(pszColumnName, strKey.c_str()) == 0)
		{
			return i;
		}
	}
	return -1;
}
//==============================================DbDriver==========================================================
CDbDriver::CDbDriver()
{
    m_pSqlite3 = NULL;
}

CDbDriver::~CDbDriver()
{
    CloseDB();
}

void CDbDriver::CloseDB()
{
    if(NULL != m_pSqlite3)
    {
        sqlite3_close(m_pSqlite3);
        m_pSqlite3 = NULL;
    }
}

BOOL CDbDriver::OpenDB(const char* pszDatabase)
{
    CloseDB();
     int iRet = sqlite3_open_v2(pszDatabase, &m_pSqlite3, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX, NULL);
    if (SQLITE_OK != iRet) 
    {
        LOGE("open database mediasync.db failed:%d", iRet);
        CloseDB();
        return FALSE;
    }
    return TRUE;
}

BOOL CDbDriver::CreateTable(const char* pszTableName, map<string, string> param)
{
    string strValues;
    map<string, string>::iterator itor;
    for(itor = param.begin(); itor != param.end(); ++itor)
    {
        string strKey = itor->first;
        string strType = itor->second;
        strKey.append(" ");
        strKey.append(strType);
        strKey.append(",");
        strValues.append(strKey);
    }
    if(strValues.length() == 0)
    {
        return FALSE;
    }
    strValues = strValues.substr(0, strValues.length() - 1);
    return ExecuteSQL("create table if not exists %s(%s)", pszTableName, strValues.c_str());
}

int CDbDriver::ExecuteSqlCallback(void *pNotUsed, int iArgc, char **pszArgv, char **pszColName)
{
	for (int i = 0; i<iArgc; ++i)
	{
		LOGD("%s = %s", pszColName[i], pszArgv[i] ? pszArgv[i] : "NULL");
	}
	return 0;
}

BOOL CDbDriver::ExecuteSQL(const char* pszFormat, ...)
{
    va_list vArgList;
	va_start(vArgList, pszFormat);
    BOOL bRet = ExecuteSQLWithArgList(pszFormat, vArgList);
	va_end(vArgList);
    return bRet;
}

BOOL CDbDriver::ExecuteSQLWithArgList(const char* pszFormat, va_list vArgList)
{
    va_list thisList;
    va_copy(thisList, vArgList);
	string strSQL = StringFormat(pszFormat, vArgList);
	va_end(thisList);
    LOGD("ExecuteSQL: %s", strSQL.c_str());
	int iRet = sqlite3_exec(m_pSqlite3, strSQL.c_str(), ExecuteSqlCallback, 0, 0);
	if (iRet != SQLITE_OK) 
	{
        LOGE("SQL error: %s", sqlite3_errmsg(m_pSqlite3));
		return FALSE;
	}
    return TRUE;

}

list<map<string, string>> CDbDriver::QuerySQL(const char* pszFormat, ...)
{
    va_list vArgList;
	va_start(vArgList, pszFormat);
    list<map<string, string>> retList = QuerySQLWithArgList(pszFormat, vArgList);
    va_end(vArgList);
    return retList;
}

list<map<string, string>> CDbDriver::QuerySQLWithArgList(const char* pszFormat, va_list vArgList)
{
    list<map<string, string>>  retList;
    sqlite3_stmt* pSqliteStmt = NULL;
    va_list thisList;
    va_copy(thisList, vArgList);
    string strSQL = StringFormat(pszFormat, thisList);
	va_end(thisList);
    LOGD("QuerySQL:%s", strSQL.c_str());
    char** pszResult = NULL;
    int iRow = 0;
    int iCol = 0;
    int iRet = sqlite3_get_table(m_pSqlite3, strSQL.c_str(), &pszResult, &iRow, &iCol, 0);
    if (SQLITE_OK != iRet)
	{
		LOGE("SQL error: %s\n", sqlite3_errmsg(m_pSqlite3));
        if(NULL != pSqliteStmt)
        {
            sqlite3_finalize(pSqliteStmt);
            pSqliteStmt = NULL;
        }
        if(NULL != pszResult)
        {
            sqlite3_free_table(pszResult);
            pszResult = NULL;
        }
        return retList;
    }
    if(NULL != pSqliteStmt)
    {
        sqlite3_finalize(pSqliteStmt);
        pSqliteStmt = NULL;
    }
    for(int i = 0; i < iRow; ++i)
    {
        map<string, string> itemMap;
        for(int t = 0; t < iCol; ++t)
        {
            int iIndex = (i+1)*iCol + t;
            string strKey = pszResult[t];
            string strValue = pszResult[iIndex];
            itemMap.insert(pair<string, string>(strKey, strValue));
        }
        retList.push_back(itemMap);
    }

    if(NULL != pszResult)
    {
        sqlite3_free_table(pszResult);
        pszResult = NULL;
    }
    return retList;
}

list<string> CDbDriver::QuerySQL2(const char* pszFormat, ...)
{
    list<string>  retList;
    va_list vArgList;
	va_start(vArgList, pszFormat);
	retList = QuerySQL2WithArgList(pszFormat, vArgList);
	va_end(vArgList);
    return retList;
}

list<string> CDbDriver::QuerySQL2WithArgList(const char* pszFormat, va_list vArgList)
{
    list<string>  retList;
    sqlite3_stmt* pSqliteStmt = NULL;
    va_list thisList;
    va_copy(thisList, vArgList);
    string strSQL = StringFormat(pszFormat, thisList);
	va_end(thisList);
    LOGD("QuerySQL:%s", strSQL.c_str());
    char** pszResult = NULL;
    int iRow = 0;
    int iCol = 0;
    int iRet = sqlite3_get_table(m_pSqlite3, strSQL.c_str(), &pszResult, &iRow, &iCol, 0);
    if (SQLITE_OK != iRet)
	{
		LOGE("SQL error: %s\n", sqlite3_errmsg(m_pSqlite3));
        if(NULL != pSqliteStmt)
        {
            sqlite3_finalize(pSqliteStmt);
            pSqliteStmt = NULL;
        }
        if(NULL != pszResult)
        {
            sqlite3_free_table(pszResult);
            pszResult = NULL;
        }
        return retList;
    }
    if(NULL != pSqliteStmt)
    {
        sqlite3_finalize(pSqliteStmt);
        pSqliteStmt = NULL;
    }

    for(int i = 0; i < iRow; ++i)
    {
        int iIndex = (i+1)*iCol;
        string strValue(pszResult[iIndex]);
        retList.push_back(strValue);
    }

    if(NULL != pszResult)
    {
        sqlite3_free_table(pszResult);
        pszResult = NULL;
    }
    return retList;
}
BOOL CDbDriver::QuerySQL(CDbCursor& dbCursor, const char* pszFormat, ...)
{
    list<string>  retList;
    va_list vArgList;
	va_start(vArgList, pszFormat);
	string strSQL = StringFormat(pszFormat, vArgList);
	va_end(vArgList);
    LOGD("QuerySQL:%s", strSQL.c_str());
    int iRet = sqlite3_prepare_v2(m_pSqlite3, strSQL.c_str(), -1, &dbCursor.m_pSqliteStmt, 0);
    if (iRet != SQLITE_OK)
	{
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "QuerySQL error: %s\n", sqlite3_errmsg(m_pSqlite3));
		printf("%s\n", szBuffer);
		return FALSE;
	}
    sqlite3_reset(dbCursor.m_pSqliteStmt);
    return TRUE;
}

std::string CDbDriver::StringFormat(const char* pszFormat, ...)
{
	char* psz = NULL;
	int iLen = 0;
	va_list vArgList;
	va_start (vArgList, pszFormat);
	iLen = vsnprintf(NULL, 0, pszFormat, vArgList) + 1;
	va_end(vArgList);

	va_start (vArgList, pszFormat); 
	psz = (char*)malloc(iLen);
	memset(psz, 0, iLen);
	vsnprintf(psz, iLen, pszFormat, vArgList);
	va_end(vArgList);

	string str(psz);
	free(psz);
    psz = NULL;
	return str;
}

std::string CDbDriver::StringFormat(const char* pszFormat, va_list vArgList)
{
    va_list thisList;
    va_copy(thisList, vArgList);
	char* psz = NULL;
	int iLen = 0;
	iLen = vsnprintf(NULL, 0, pszFormat, thisList) + 1;
    va_end(thisList);

    va_list secondList;
    va_copy(secondList, vArgList);
	psz = (char*)malloc(iLen);
	memset(psz, 0, iLen);
	vsnprintf(psz, iLen, pszFormat, secondList);
    va_end(thisList);

	string str(psz);
	free(psz);
    psz = NULL;
	return str;
}
