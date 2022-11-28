#include "stdafx.h"
#include <json/json.h>
#include <string>

class CJsonUtil
{
public:
	CJsonUtil();
	~CJsonUtil();
	
	BOOL Parse(const char* pMsg);
	int GetInt(const char* pKey);
	const char* GetStrng(const char* pKey);
	long GetLong(const char* pKey);
	int GetInt(const char* pSubKey, int iIndex);
	const char* GetStrng(const char* pSubKey, int iIndex);
	long GetLong(const char* pSubKey, int iIndex);
	int GetInt(const char* pArrayKey, const char* pSubKey, int iIndex);
	const char* GetStrng(const char* pArrayKey, const char* pSubKey, int iIndex);
	long GetLong(const char* pArrayKey, const char* pSubKey, int iIndex);
	int GetItemCount();
	int GetArrayCount(const char* pArrayKey);
	void Delete(const char* pKey);
	void Set(const char* pKey, int iValue);
	void Set(const char* pKey, const char* pszValue);
	void Set(const char* pKey, long iValue);
	void Set(const char* pKey, CJsonUtil util);
	int GetObjectInt(const char * pObjectKey, const char* pItemKey);
	const char* GetObjectString(const char * pObjectKey, const char* pItemKey);
	const char* GetObjectString(const char* pObjectKey, int iIndex);
	const char* GetObjectString(const char* pObjectKey);
	long GetObjectLong(const char * pObjectKey, const char* pItemKey);
	Json::Value ToObject();
	const char* ToString();
	const char* ToString(Json::Value Value);
	
private:
	Json::Reader m_JsonReader;
	Json::Value m_JsonValue;
	Json::Value m_JsonArray;
	std::string m_strArrayKey;

	Json::Value m_JsonArrayItem;
	int m_iArrayItemIndex;

	std::string m_strRet;
};


