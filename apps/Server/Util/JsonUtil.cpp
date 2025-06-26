#include "JsonUtil.h"
namespace Server
{
CJsonUtil::CJsonUtil()
{
}

CJsonUtil::~CJsonUtil()
{
}
std::string CJsonUtil::ToString(nlohmann::json& json)
{
	std::string strJson = json.dump();
	return strJson;
}
std::string CJsonUtil::ToString(nlohmann::ordered_json& json)
{
	std::string strJson = json.dump();
	return strJson;
}
BOOL CJsonUtil::FromString(std::string strJson, nlohmann::json& json)
{
	try
	{
		json = nlohmann::json::parse(strJson.c_str());
		return TRUE;
	}
	catch (...)
	{
		return FALSE;
	}
}
}
