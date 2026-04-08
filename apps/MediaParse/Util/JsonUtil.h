#pragma once
#include "../stdafx.h"
namespace MediaParse
{
class CJsonUtil
{
public:
    CJsonUtil();
    ~CJsonUtil();
    static std::string ToString(nlohmann::json& json);
    static std::string ToString(nlohmann::ordered_json& json);
	static BOOL FromString(std::string strJson, nlohmann::json& json);
    static BOOL CheckKey(std::string strKey, nlohmann::json& json);
};

}
