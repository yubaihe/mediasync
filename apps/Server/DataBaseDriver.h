#ifndef RELECH_DATABASEDRIVER_H__
#define RELECH_DATABASEDRIVER_H__
#include "stdafx.h"
#include "sqlite3.h"
#include "Tools.h"
typedef struct
{
   sqlite3* m_pSqlite3;
   sqlite3_stmt* m_pSqliteStmt;
   char* pstrValue;
}DataBaseDriver;

BOOL DataBaseDriver_ExecuteSQL(DataBaseDriver* pDataBaseDriver, const char* pSQL);
//DataBaseDriver* DataBaseDriver_GetDataBaseConn(DataBaseDriver* pDataBaseDriver);
//DataBaseDriver* DataBaseDriver_GetMediaDataBaseConn(DataBaseDriver* pDataBaseDriver);
DataBaseDriver* DataBaseDriver_GetMediaDataBaseConn();
DataBaseDriver* DataBaseDriver_GetDataBaseConn();
void DataBaseDriver_CloseConn(DataBaseDriver* pDataBaseDriver);
//void DataBaseDriver_CloseConn(DataBaseDriver* pDataBaseDriver);
BOOL DataBaseDriver_Init();
void DataBaseDriver_Release();
BOOL DataBaseDriver_QuerySQL(DataBaseDriver* pDataBaseDriver, const char* pSQL);
int DataBaseDriver_GetCount(DataBaseDriver* pDataBaseDriver);
BOOL DataBaseDriver_MoveToNext(DataBaseDriver* pDataBaseDriver);
void DataBaseDriver_MoveToFirst(DataBaseDriver* pDataBaseDriver);
void DataBaseDriver_BeforeFirst(DataBaseDriver* pDataBaseDriver);
long DataBaseDriver_GetLong(DataBaseDriver* pDataBaseDriver, const char* pKey);
int DataBaseDriver_GetInt(DataBaseDriver* pDataBaseDriver, const char* pKey);
const char* DataBaseDriver_GetString(DataBaseDriver* pDataBaseDriver, const char* pKey);
#endif