
#include "JsonUtil.h"

CJsonUtil::CJsonUtil()
{
	
}

CJsonUtil::~CJsonUtil()
{

}

BOOL CJsonUtil::Parse(const char * pMsg)
{
	if(m_JsonReader.parse(pMsg, m_JsonValue))
	{
		return TRUE;
	}
	
	return FALSE;
}

int CJsonUtil::GetInt(const char* pKey)
{
	Json::Value value = m_JsonValue[pKey];
	if(value.isNull())
	{
		m_JsonValue.removeMember(pKey);
		return -1;
	}

	if(value.isInt())
	{
		return value.asInt();
	}
	if(value.isString())
	{
		std::string str =value.asString();
		return atoi(str.c_str());
	}
	
	
	return -1;
}

const char* CJsonUtil::GetStrng(const char * pKey)
{
	m_strRet = "";

	Json::Value value = m_JsonValue[pKey];
	if(value.isNull())
	{
		m_JsonValue.removeMember(pKey);
		m_strRet = "";
		return m_strRet.c_str();
	}
	
	if(value.isInt())
	{
		
		int iValue = value.asInt();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%d", iValue);
		
		m_strRet = szBuffer;
		return m_strRet.c_str();
		
	}
	
	if(value.isInt64())
	{
		long iValue = (long)value.asInt64();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%ld", iValue);
		
		m_strRet = szBuffer;
		return m_strRet.c_str();
	}
	if(value.isString())
	{
		m_strRet = value.asString();
		return m_strRet.c_str();
	}
	if (value.isObject())
	{
		Json::FastWriter writer;
		m_strRet = writer.write(value);
		if (m_strRet[m_strRet.size() - 1] == '\n')
		{
			m_strRet[m_strRet.size() - 1] = '\0';
		}
		return m_strRet.c_str();
	}

	m_strRet = "";
	return m_strRet.c_str();
}

long CJsonUtil::GetLong(const char * pKey)
{
	std::string str = GetStrng(pKey);
	if(str.size() == 0)
	{
		return -1;
	}
	
	return atol(str.c_str());
}


int CJsonUtil::GetInt(const char* pArrayKey, const char* pSubKey, int iIndex)
{
	BOOL bSameArray = TRUE;
	if(strcmp(m_strArrayKey.c_str(), pArrayKey) != 0)
	{
		m_JsonArray = m_JsonValue[pArrayKey];
		m_strArrayKey = pArrayKey;
		bSameArray = FALSE;
	}

	if(m_JsonArray.isNull())
	{
		m_JsonValue.removeMember(pArrayKey);
		return -1;
	}

	if(FALSE == bSameArray || m_iArrayItemIndex != iIndex)
	{
		m_JsonArrayItem = m_JsonArray[iIndex];
		m_iArrayItemIndex = iIndex;
	}
	
	Json::Value itemValue = m_JsonArrayItem[pSubKey];
	if (itemValue.isNull())
	{
		m_JsonArrayItem.removeMember(pSubKey);
		return -1;
	}
	
	if(itemValue.isInt())
	{
		return itemValue.asInt();
	}
	if(itemValue.isString())
	{
		std::string str = itemValue.asString();
		return atoi(str.c_str());
	}
	return -1;
}

const char* CJsonUtil::GetStrng(const char* pArrayKey, const char* pSubKey, int iIndex)
{
	m_strRet = "";
	BOOL bSameArray = TRUE;
	if(strcmp(m_strArrayKey.c_str(), pArrayKey) != 0)
	{
		m_JsonArray = m_JsonValue[pArrayKey];
		m_strArrayKey = pArrayKey;
		bSameArray = FALSE;
	}

	if(m_JsonArray.isNull())
	{
		m_JsonValue.removeMember(pArrayKey);
		m_strRet = "";
		return m_strRet.c_str();
	}

	if(FALSE == bSameArray || m_iArrayItemIndex != iIndex)
	{
		m_JsonArrayItem = m_JsonArray[iIndex];
		m_iArrayItemIndex = iIndex;
	}
	
	Json::Value itemValue = m_JsonArrayItem[pSubKey];
	if (itemValue.isNull())
	{
		m_strRet = "";
		return m_strRet.c_str();
	}

	if(itemValue.isInt())
	{
		int iValue = itemValue.asInt();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%d", iValue);
		m_strRet = szBuffer;
		return m_strRet.c_str();
	}
	if(itemValue.isInt64())
	{
		long iValue = (long)itemValue.asInt64();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%ld", iValue);
		m_strRet = szBuffer;
		return m_strRet.c_str();
	}
	if(itemValue.isString())
	{
		m_strRet = itemValue.asString();
		return m_strRet.c_str();
	}
	
	m_strRet = "";
	return m_strRet.c_str();
}

long CJsonUtil::GetLong(const char* pArrayKey, const char* pSubKey, int iIndex)
{
	std::string str = GetStrng(pArrayKey, pSubKey, iIndex);
	if(str.size() == 0)
	{
		return -1;
	}
	
	return atol(str.c_str());
}

int CJsonUtil::GetInt(const char* pSubKey, int iIndex)
{
	Json::Value item = m_JsonValue[iIndex];
	
	Json::Value itemValue = item[pSubKey];
	if (itemValue.isNull())
	{
		item.removeMember(pSubKey);
		return -1;
	}

	if (itemValue.isInt())
	{
		return itemValue.asInt();
	}
	if (itemValue.isString())
	{
		std::string str = itemValue.asString();
		return atoi(str.c_str());
	}
	return -1;
}

const char* CJsonUtil::GetStrng(const char* pSubKey, int iIndex)
{
	m_strRet = "";
	Json::Value item = m_JsonValue[iIndex];

	Json::Value itemValue = item[pSubKey];
	if (itemValue.isNull())
	{
		m_strRet = "";
		return m_strRet.c_str();
	}

	if (itemValue.isInt())
	{
		int iValue = itemValue.asInt();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%d", iValue);
		m_strRet = szBuffer;
		return m_strRet.c_str();
	}
	if (itemValue.isInt64())
	{
		long iValue = (long)itemValue.asInt64();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%ld", iValue);
		m_strRet = szBuffer;
		return m_strRet.c_str();
	}
	if (itemValue.isString())
	{
		m_strRet = itemValue.asString();
		return m_strRet.c_str();
	}

	m_strRet = "";
	return m_strRet.c_str();
}

long CJsonUtil::GetLong(const char* pSubKey, int iIndex)
{
	std::string str = GetStrng(pSubKey, iIndex);
	if (str.size() == 0)
	{
		return -1;
	}

	return atol(str.c_str());
}

void CJsonUtil::Delete(const char * pKey)
{
	m_JsonValue.removeMember(pKey);
}

void CJsonUtil::Set(const char * pKey,int iValue)
{
	m_JsonValue[pKey] = Json::Value(iValue);
}

void CJsonUtil::Set(const char* pKey, const char* pszValue)
{
	m_JsonValue[pKey] = Json::Value(pszValue);
}

void CJsonUtil::Set(const char* pKey, long iValue)
{
	char sz[30] = {0};
	sprintf(sz, "%ld", iValue);
	m_JsonValue[pKey] = Json::Value(sz);
}

void CJsonUtil::Set(const char* pKey, CJsonUtil util)
{
	m_JsonValue[pKey] = util.ToObject();
}

int CJsonUtil::GetItemCount()
{
	return m_JsonValue.size();
}

int CJsonUtil::GetArrayCount(const char * pArrayKey)
{
	if(strcmp(m_strArrayKey.c_str(), pArrayKey) != 0)
	{
		m_JsonArray = m_JsonValue[pArrayKey]; 
		m_strArrayKey = pArrayKey;
		m_iArrayItemIndex = 0;
		m_JsonArrayItem = m_JsonArray[m_iArrayItemIndex];
	}
	
	if(m_JsonArray.isNull())
	{
		m_JsonValue.removeMember(pArrayKey);
		return 0;
	}
	
	
	int iSize = m_JsonArray.size(); 
	return iSize;
}


int CJsonUtil::GetObjectInt(const char * pObjectKey, const char* pItemKey)
{
	Json::Value objectValue = m_JsonValue[pObjectKey];
	if (objectValue.isNull())
	{
		m_JsonValue.removeMember(pObjectKey);
		m_strRet = "";
		return -1;
	}

	Json::Value itemValue = objectValue[pItemKey];
	if (itemValue.isNull())
	{
		objectValue.removeMember(pItemKey);
		m_strRet = "";
		return -1;
	}

	if (itemValue.isInt())
	{
		return itemValue.asInt();
	}
	if (itemValue.isString())
	{
		std::string str = itemValue.asString();
		return atoi(str.c_str());
	}

	m_strRet = "";
	return -1;
}

const char* CJsonUtil::GetObjectString(const char * pObjectKey, const char* pItemKey)
{
	Json::Value objectValue = m_JsonValue[pObjectKey];
	if (objectValue.isNull())
	{
		m_JsonValue.removeMember(pObjectKey);
		m_strRet = "";
		return m_strRet.c_str();
	}

	Json::Value value = objectValue[pItemKey];
	if (value.isNull())
	{
		objectValue.removeMember(pItemKey);
		m_strRet = "";
		return m_strRet.c_str();
	}

	if (value.isInt())
	{

		int iValue = value.asInt();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%d", iValue);

		m_strRet = szBuffer;
		return m_strRet.c_str();
	}

	if (value.isInt64())
	{
		long iValue = (long)value.asInt64();
		char szBuffer[100] = { 0 };
		sprintf(szBuffer, "%ld", iValue);

		m_strRet = szBuffer;
		return m_strRet.c_str();
	}
	if (value.isString())
	{
		m_strRet = value.asString();
		return m_strRet.c_str();
	}

	m_strRet = "";
	return m_strRet.c_str();
}
const char* CJsonUtil::GetObjectString(const char* pObjectKey, int iIndex)
{
	Json::Value objectValue = m_JsonValue[pObjectKey];
	if (objectValue.isNull())
	{
		m_JsonValue.removeMember(pObjectKey);
		m_strRet = "";
		return m_strRet.c_str();
	}

	Json::Value value = objectValue[iIndex];
	if (value.isNull())
	{
		m_strRet = "";
		return m_strRet.c_str();
	}
	m_strRet = ToString(value);
	return m_strRet.c_str();
}

const char* CJsonUtil::GetObjectString(const char* pObjectKey)
{
	Json::Value objectValue = m_JsonValue[pObjectKey];
	if (objectValue.isNull())
	{
		m_JsonValue.removeMember(pObjectKey);
		m_strRet = "";
		return m_strRet.c_str();
	}

	m_strRet = ToString(objectValue);
	return m_strRet.c_str();
}

long CJsonUtil::GetObjectLong(const char * pObjectKey, const char* pItemKey)
{
	std::string str = GetObjectString(pObjectKey, pItemKey);
	if (str.size() == 0)
	{
		return -1;
	}

	return atol(str.c_str());
}
Json::Value CJsonUtil::ToObject()
{
	return m_JsonValue;
}
const char* CJsonUtil::ToString()
{
	//这里输出格式化的json数据
	//return m_JsonValue.toStyledString();
	//这里输出无格式化的Json数据
	Json::FastWriter writer;  
	m_strRet = writer.write(m_JsonValue);
	if(m_strRet[m_strRet.size() - 1] == '\n')
	{
		m_strRet[m_strRet.size() - 1] = '\0';
	}
	return m_strRet.c_str();
}

const char* CJsonUtil::ToString(Json::Value Value)
{
	//这里输出格式化的json数据
	//return m_JsonValue.toStyledString();
	//这里输出无格式化的Json数据
	Json::FastWriter writer;
	m_strRet = writer.write(Value);
	if (m_strRet[m_strRet.size() - 1] == '\n')
	{
		m_strRet[m_strRet.size() - 1] = '\0';
	}
	return m_strRet.c_str();
}
