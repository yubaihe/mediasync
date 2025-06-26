#include "MediaJishuTable.h"
#include "MediaDb.h"
#include "MediaInfoTable.h"
#include "../Util/Tools.h"
#include "../Util/CommonUtil.h"
#include "../Util/JsonUtil.h"
CMediaJishuTable::CMediaJishuTable()
{
}

CMediaJishuTable::~CMediaJishuTable()
{
}
BOOL CMediaJishuTable::CreateTable()
{
    //const char*  pszJiShuTable = "create table if not exists tbl_mediajishu(year integer, " 
    //                                "month integer, day integer, pic integer, video integer, devicename text)";
    //year 年份
	//month    月份 
	//day     日
	//pic     图片计数
	//video  视频计数
	//devicename 所属设备 
    map<string, string> param;
    param.insert(pair<string, string>("year", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("month", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("day", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("pic", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("video", "integer DEFAULT '0'"));
    param.insert(pair<string, string>("devicename", "text DEFAULT ''"));
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->CreateTable(TABLE_MEDIAJISHU, param);
    UNLOCKMEDIADB
    return bRet;
}
list<MediaJishuItem> CMediaJishuTable::AssembleItems(list<map<string, string>> List)
{
    list<MediaJishuItem> retList;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        MediaJishuItem item = {};
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("year"))
            {
                item.iYear = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("month"))
            {
                item.iMonth = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("day"))
            {
                item.iDay = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("pic"))
            {
                item.iPicCount = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("video"))
            {
                item.iVideoCount = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("devicename"))
            {
                item.strDeviceName = strValue.c_str();
            }
        }
        retList.push_back(item);
    }
    return retList;
}

BOOL CMediaJishuTable::ReSet(ResetJiShuCallBack callBack, LPVOID* lpParameter)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = pDbDriver->ExecuteSQL("delete from tbl_mediajishu");
    if(FALSE == bRet)
    {
        UNLOCKMEDIADB
        return FALSE;
    }
    CDbCursor cursor;
    bRet = pDbDriver->QuerySQL(cursor, "select meititype,paishetime,devicename from tbl_mediainfo");
    if(FALSE == bRet)
    {
        UNLOCKMEDIADB
        return FALSE;
    }
    int iCount = cursor.GetCount();
    int iIndex = 0;
    while(TRUE == cursor.Next())
    {
        MediaInfoItem item = CMediaInfoTable::GetItem(cursor);
        //printf("iPaiSheTime:%lld iMediaType:%ld strDeviceName:%s\n", item.iPaiSheTime, item.iMediaType, item.strDeviceName.c_str());
        bRet = Increase(pDbDriver, item.iPaiSheTime, item.iMediaType, item.strDeviceName);
        if(FALSE == bRet)
        {
            break;
        }
        iIndex++;
        callBack((int)(iIndex*100.0)/iCount, lpParameter);
    }
    cursor.Close();
    UNLOCKMEDIADB
    return bRet;
}

BOOL CMediaJishuTable::Increase(CDbDriver* pDbDriver, long iPaiSheTime, int iMediaType, string strDeviceName)
{
    uint16_t iYear;
    uint8_t iMonth;
    uint8_t iDay;
    Server::CTools::SecInfo(iPaiSheTime, &iYear, &iMonth, &iDay);
    BOOL bRet = TRUE;
    list<map<string,string>> List = pDbDriver->QuerySQL("select pic,video from tbl_mediajishu where year='%d' and month='%d' and day='%d' and devicename='%s'", iYear, iMonth, iDay, strDeviceName.c_str());
    if(List.size() == 0)
    {
        switch (iMediaType)
        {
            case MEDIATYPE_IMAGE:
            {
                bRet = pDbDriver->ExecuteSQL("insert into tbl_mediajishu(pic,video,year,month,day,devicename)values('%d','%d','%d','%d','%d','%s')",
                                                    1, 0, iYear, iMonth, iDay, strDeviceName.c_str());
                break;
            }
            case MEDIATYPE_VIDEO:
            {
                bRet = pDbDriver->ExecuteSQL("insert into tbl_mediajishu(pic,video,year,month,day,devicename)values('%d','%d','%d','%d','%d','%s')",
                                            0, 1, iYear, iMonth, iDay, strDeviceName.c_str());
                break;
            }
        }
        return bRet;
    }
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        int iPicCount = 0;
        int iVideoCount = 0;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("pic"))
            {
                iPicCount = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("video"))
            {
                iVideoCount = atoi(strValue.c_str());
            }
        }
        switch (iMediaType)
        {
            case MEDIATYPE_IMAGE:
            {
                bRet = pDbDriver->ExecuteSQL("update tbl_mediajishu set pic=%d where year='%d' and month='%d' and day='%d' and devicename='%s'",
                                                    iPicCount + 1, iYear, iMonth, iDay, strDeviceName.c_str());
                
                break;
            }
            case MEDIATYPE_VIDEO:
            {
                bRet = pDbDriver->ExecuteSQL("update tbl_mediajishu set video=%d where year='%d' and month='%d' and day='%d' and devicename='%s'",
                                            iVideoCount + 1, iYear, iMonth, iDay, strDeviceName.c_str());
                break;
            }
        }
        if(FALSE == bRet)
        {
            break;
        }
    }
    return bRet;
}

BOOL CMediaJishuTable::Increase(long iPaiSheTime, int iMediaType, string strDeviceName)
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    BOOL bRet = Increase(pDbDriver, iPaiSheTime, iMediaType, strDeviceName);
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaJishuTable::Decrease(long iPaiSheTime, int iMediaType, string strDeviceName)
{
    uint16_t iYear;
    uint8_t iMonth;
    uint8_t iDay;
    Server::CTools::SecInfo(iPaiSheTime, &iYear, &iMonth, &iDay);
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<map<string,string>> List = pDbDriver->QuerySQL("select pic,video from tbl_mediajishu where year='%d' and month='%d' and day='%d' and devicename='%s'", iYear, iMonth, iDay, strDeviceName.c_str());
    if(List.size() == 0)
    {
        UNLOCKMEDIADB
        return TRUE;
    }
    BOOL bRet = TRUE;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        int iPicCount = 0;
        int iVideoCount = 0;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("pic"))
            {
                iPicCount = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("video"))
            {
                iVideoCount = atoi(strValue.c_str());
            }
        }
        switch (iMediaType)
        {
            case MEDIATYPE_IMAGE:
            {
                if (iPicCount == 1 && iVideoCount == 0)
                {
                    bRet = pDbDriver->ExecuteSQL("delete from tbl_mediajishu where year='%d' and month='%d' and day='%d' and devicename='%s'", iYear, iMonth, iDay, strDeviceName.c_str());
                }
                else
                {
                    bRet = pDbDriver->ExecuteSQL("update tbl_mediajishu set pic=%d where year='%d' and month='%d' and day='%d' and devicename='%s'", iPicCount - 1, iYear, iMonth, iDay, strDeviceName.c_str());
                }
                break;
            }
            case MEDIATYPE_VIDEO:
            {
                if (iVideoCount == 1 && iPicCount == 0)
                {
                    bRet = pDbDriver->ExecuteSQL("delete from tbl_mediajishu where year='%d' and month='%d' and day='%d' and devicename='%s'", iYear, iMonth, iDay, strDeviceName.c_str());
                }
                else
                {
                    bRet = pDbDriver->ExecuteSQL("update tbl_mediajishu set video=%d where year='%d' and month='%d' and day='%d' and devicename='%s'", iVideoCount - 1, iYear, iMonth, iDay, strDeviceName.c_str());
                }
                break;
            }
        }
        if(FALSE == bRet)
        {
            break;
        }
    }
    UNLOCKMEDIADB
    return bRet;
}
BOOL CMediaJishuTable::DecreaseFromID(int iMediaID)
{
    MediaInfoItem item = CMediaInfoTable::GetItemByID(iMediaID);
    return Decrease(item.iPaiSheTime, item.iMediaType, item.strDeviceName);
}
BOOL CMediaJishuTable::IncreaseFromID(int iMediaID)
{
    MediaInfoItem item = CMediaInfoTable::GetItemByID(iMediaID);
    return Increase(item.iPaiSheTime, item.iMediaType, item.strDeviceName);
}
void CMediaJishuTable::GetYearInfo(int iYear, string strDeviceNames, int* piPicCount, int* piVideoCount)
{
    list<map<string,string>> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strDeviceNames.length() == 0)
    {
        List = pDbDriver->QuerySQL("select sum(pic) as pic,sum(video) as video from tbl_mediajishu where year='%d' group by year order by year desc", iYear);
    }
    else 
    {
        List = pDbDriver->QuerySQL("select sum(pic) as pic,sum(video) as video from tbl_mediajishu where year='%d' and devicename in('%s') group by year order by year desc", iYear, strDeviceNames.c_str());
    }
    UNLOCKMEDIADB
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("pic"))
            {
                *piPicCount = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("video"))
            {
                *piVideoCount = atoi(strValue.c_str());
            }
        }
        break;
    }
}
string CMediaJishuTable::GetYears(string strDeviceNames)
{
    list<string> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strDeviceNames.length() == 0)
    {
        List = pDbDriver->QuerySQL2("select distinct(year) as year from tbl_mediajishu order by year desc");
    }
    else
    {
        List = pDbDriver->QuerySQL2("select distinct(year) as year from tbl_mediajishu where devicename in('%s') order by year desc", strDeviceNames.c_str());
    }
    UNLOCKMEDIADB
    if(0 == List.size())
    {
        return "";
    }
    return Server::CCommonUtil::ListToString(List, "&");
}
string CMediaJishuTable::GetDevNames()
{
    CDbDriver* pDbDriver = LOCKMEDIADB
    list<string> List = pDbDriver->QuerySQL2("select distinct(devicename) from tbl_mediajishu");
    UNLOCKMEDIADB
    if(0 == List.size())
    {
        return "[]";
    }
    nlohmann::json jsonRoot = nlohmann::json::array();
    for(list<string>::iterator itor = List.begin(); itor != List.end(); ++itor)
    {
        nlohmann::json jsonItem;
        jsonItem["name"] = itor->c_str();
        jsonRoot.push_back(jsonItem);
    }
    string strJson = Server::CJsonUtil::ToString(jsonRoot);
    return strJson;
}
string CMediaJishuTable::GetMonths(int iYear, string strDeviceNames)
{
    list<map<string,string>> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strDeviceNames.length() == 0)
    {
        List = pDbDriver->QuerySQL("select month, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' group by month order by month asc", iYear);
    }
    else
    {
        List = pDbDriver->QuerySQL("select month, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' and devicename in('%s') group by month order by month asc", iYear, strDeviceNames.c_str());
    }
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "[]";
    }
    nlohmann::json jsonRoot = nlohmann::json::array();;
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        int iMonth = 0;
        int iPicCount = 0;
        int iVideoCount = 0;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("month"))
            {
                iMonth = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("PicCount"))
            {
                iPicCount = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("VideoCount"))
            {
                iVideoCount = atoi(strValue.c_str());
            }
        }
        nlohmann::json jsonItem;
        jsonItem["month"] = iMonth;
        jsonItem["piccount"] = iPicCount;
        jsonItem["videocount"] = iVideoCount;
        jsonRoot.push_back(jsonItem);
    }
    return Server::CJsonUtil::ToString(jsonRoot);
}
string CMediaJishuTable::GetDays(int iYear, int iMonth, string strDeviceNames)
{
     list<map<string,string>> List;
    CDbDriver* pDbDriver = LOCKMEDIADB
    if(strDeviceNames.length() == 0)
    {
        List = pDbDriver->QuerySQL("select day, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' and month='%d' group by day order by day asc", iYear, iMonth);
    }
    else
    {
        List = pDbDriver->QuerySQL("select day, sum(pic) as PicCount,sum(video) as VideoCount from tbl_mediajishu where year='%d' and month='%d' and devicename in('%s') group by day order by day asc", iYear, iMonth, strDeviceNames.c_str());
    }
    UNLOCKMEDIADB
    if(List.size() == 0)
    {
        return "[]";
    }
    nlohmann::json jsonRoot = nlohmann::json::array();
    list<map<string, string>>::iterator itor = List.begin();
    for(; itor != List.end(); ++itor)
    {
        map<string, string> mapItem = *itor;
        map<string,string>::iterator itorMap;
        int iDay = 0;
        int iPicCount = 0;
        int iVideoCount = 0;
        for(itorMap = mapItem.begin(); itorMap != mapItem.end(); ++itorMap)
        {
            
            string strKey = itorMap->first;
            string strValue = itorMap->second;
            if(0 == strKey.compare("day"))
            {
                iDay = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("PicCount"))
            {
                iPicCount = atoi(strValue.c_str());
            }
            else if(0 == strKey.compare("VideoCount"))
            {
                iVideoCount = atoi(strValue.c_str());
            }
        }
        nlohmann::json jsonItem;
        jsonItem["day"] = iDay;
        jsonItem["piccount"] = iPicCount;
        jsonItem["videocount"] = iVideoCount;
        jsonRoot.push_back(jsonItem);
    }
    return Server::CJsonUtil::ToString(jsonRoot);
}