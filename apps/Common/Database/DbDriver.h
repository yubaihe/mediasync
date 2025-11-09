#ifndef DBDRIVER_H__
#define DBDRIVER_H__
#include <list>
#include <map>
#include <string>
using namespace std;
#include "stdafx.h"
#include "sqlite3.h"
/***
CDbDriver driver;
driver.OpenDB("./teset.db");
map<string, string> param;
param.insert(pair<string, string>("id", "integer PRIMARY KEY autoincrement"));
param.insert(pair<string, string>("name", "text"));
param.insert(pair<string, string>("pwd", "text"));
driver.CreateTable("user", param);

增加
driver.ExecuteSQL("insert into user(name,pwd)values('%s','%s')","relech0","pwd0");

删除
driver.ExecuteSQL("delete from user where name='%s'","relech4");

查询
list<map<string, string>> userList = driver.QuerySQL("select * from user");
list<map<string, string>>::iterator itor = userList.begin();
for(; itor != userList.end(); ++itor)
{
    dzlog_debug("=====================================");
    map<string, string> mapItem = *itor;
    map<string,string>::iterator itorMap;
    for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
    {
        dzlog_debug("%s:%s", itorMap->first.c_str(), itorMap->second.c_str());
    }
}
使用游标查询
CDbCursor cursor;
BOOL bRet = driver.QuerySQL(cursor, "select * from tbl_mediainfo where id=%d", 100);
UNLOCKMEDIADB
if(FALSE == bRet)
{
    return 0;
}
int iCount = cursor.GetCount();
printf("iCount:%d\n", iCount);
while(TRUE == cursor.Next())
{
    int iID = cursor.GetInt("id");
    string strName = cursor.GetString("name");
    printf("ID:%d\tAddr:%s\n", iID, strName.c_str());
}
cursor.Close();
*/
class CDbCursor
{
public:
    CDbCursor();
    ~CDbCursor();
    void Close();
    int GetCount();
    BOOL Next();
    void First();
    int GetInt(string strKey, int iDefault = 0);
    string GetString(string strKey, string strDefault = "");
    long GetLong(string strKey, long iDefault = 0);
    int64_t GetInt64(string strKey, long iDefault = 0);
private:
    int GetIndex(string strKey);
private:
    friend class CDbDriver; 
    sqlite3_stmt* m_pSqliteStmt;
};
class CDbDriver
{
public:
    CDbDriver();
    ~CDbDriver();
    BOOL OpenDB(const char* pszDatabase);
    void CloseDB();
    BOOL CreateTable(const char* pszTableName, map<string, string> param);
    BOOL ExecuteSQL(const char* pszFormat, ...);
    list<map<string, string>> QuerySQL(const char* pszFormat, ...);
    list<string> QuerySQL2(const char* pszFormat, ...);
    BOOL QuerySQL(CDbCursor& DbCursor, const char* pszFormat, ...);

private:
    BOOL ExecuteSQLWithArgList(const char* pszFormat, va_list vArgList);
    list<map<string, string>> QuerySQLWithArgList(const char* pszFormat, va_list vArgList);
    list<string> QuerySQL2WithArgList(const char* pszFormat, va_list vArgList);
    std::string StringFormat(const char* pszFormat, ...);
    std::string StringFormat(const char* pszFormat, va_list vArgList);
    static int ExecuteSqlCallback(void *pNotUsed, int iArgc, char **pszArgv, char **pszColName);
private:
    sqlite3* m_pSqlite3;
};




#endif