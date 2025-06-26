#include "GpsManager.h"
#include "DataBase/MediaGpsTable.h"
#include "Util/CommonUtil.h"
#include "Util/JsonUtil.h"
#include "DataBase/MediaInfoTable.h"
#include "Util/DbusUtil.h"
#include "Util/Tools.h"
CGpsManager* CGpsManager::m_pInstance = NULL;
CGpsManager::CGpsManager()
{
    m_bExit = FALSE;
    m_strBaiDuKey = "";
    m_iNeedCheckTimeSec = 0;
    InitializeCriticalSection(&m_Section);
    m_hDetect = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GpsDetectProc, this, 0, NULL);
}

CGpsManager::~CGpsManager()
{
    if(TRUE == m_bExit)
    {
        return;
    }
    m_bExit = TRUE;
    if(NULL != m_hDetect)
    {
        WaitForSingleObject(m_hDetect, INFINITE);
        CloseHandle(m_hDetect);
        m_hDetect = NULL;
    }
    DeleteCriticalSection(&m_Section);
}
CGpsManager* CGpsManager::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CGpsManager();
    }
    return m_pInstance;
}
void CGpsManager::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
void CGpsManager::SetBaiDuKey(string strBaiDuKey)
{
    if(strBaiDuKey.length() == 0)
    {
        return;
    }
    
    m_strBaiDuKey = strBaiDuKey;
    NeedCheck();
}


BOOL CGpsManager::ParseGps(string strGps, GPSTYPE* piGpsType, string* pstrLongitude/* = NULL*/, string* pstrLatitude /*= NULL*/)
{
    GPSTYPE eRetGpsType = GPSTYPE_NORMAL;
    if (strGps.size() >= 2 && strGps[0] == 'B' && strGps[1] == '-')
    {
        strGps.erase(0, 2);
        eRetGpsType = GPSTYPE_BAIDU;
    }
    if(NULL != pstrLongitude && NULL != pstrLatitude)
    {
        vector<string> vec = Server::CCommonUtil::StringSplit(strGps, "&");
        if(vec.size() != 2)
        {
            *pstrLongitude = "0.000000";
            *pstrLatitude = "0.000000";
        }
        else
        {
            *pstrLongitude = vec[0];
            *pstrLatitude = vec[1];
        }
    }
    
    *piGpsType = eRetGpsType;
    return eRetGpsType;
}

void CGpsManager::FilterUncheckGps()
{
    CMediaInfoTable table;
    list<string> gpsList = table.GetUnCheckWeiZhi(1000);
    if(gpsList.size() == 0)
    {
        EnterCriticalSection(&m_Section);
        m_iNeedCheckTimeSec = 0;
        LeaveCriticalSection(&m_Section);
        return;
    }
    for(list<string>::iterator itor = gpsList.begin(); itor != gpsList.end(); ++itor)
    {
        string strGps = *itor;
        GPSTYPE eGpsType = GPSTYPE_NORMAL;
        string strLongitude;
        string strLatitude;
        ParseGps(strGps, &eGpsType, &strLongitude, &strLatitude);
        AddDetectItem(eGpsType, strLongitude, strLatitude, -1, GPSTAG_UNCHECK);
    }
}
DWORD CGpsManager::GpsDetectProc(void* lpParameter)
{
    CGpsManager* pGpsManager = (CGpsManager*)lpParameter;
    while (FALSE == pGpsManager->m_bExit)
    {
        Sleep(1000);
        if(pGpsManager->m_strBaiDuKey.length() == 0)
        {
            continue;
        }
        if(pGpsManager->m_pGpsList.size() == 0)
        {
            if(pGpsManager->m_iNeedCheckTimeSec > 0)
            {
                pGpsManager->FilterUncheckGps();
            }
            continue;
        }
        EnterCriticalSection(&pGpsManager->m_Section);
        GpsItem item = pGpsManager->m_pGpsList.front();
        pGpsManager->m_pGpsList.pop_front();
        LeaveCriticalSection(&pGpsManager->m_Section);
        CMediaGpsTable gpsTable;
        MediaGpsItem gpsItem = {};
        switch (item.eGpsType)
        {
            case GPSTYPE_NORMAL:
            {
                string strGps = Server::CCommonUtil::StringFormat("%s&%s", item.strLongitude.c_str(), item.strLatitude.c_str());
                gpsTable.GetRecordFromGps(strGps, gpsItem);
                break;
            }
            case GPSTYPE_BAIDU:
            {
                string strGps = Server::CCommonUtil::StringFormat("B-%s&%s", item.strLongitude.c_str(), item.strLatitude.c_str());
                gpsTable.GetRecordFromGps2(strGps, gpsItem);
                break;
            }
            default:
            {
                break;
            }
        } 
        if(gpsItem.strLocation.length() == 0)
        {
            pGpsManager->ReqeustGpsDetail(item);
        }
    }
    return 1;
}
size_t CGpsManager::RecvCallBack(char* pszData, size_t iDataSize, size_t iNmemb, std::string* pstrBuffer)
{
    try 
    {
        pstrBuffer->append(pszData, iDataSize * iNmemb);
    }
    catch (const std::exception& e)
    {
        printf("Write error: %s\n", e.what());
        return 0;
    }
    return iDataSize * iNmemb;
}
void CGpsManager::ReqeustGpsDetail(GpsItem item)
{
    //http://api.map.baidu.com/reverse_geocoding/v3/?ak=您的AK&output=json&coordtype=wgs84ll&location=纬度,经度
    string strParam = "";
    switch (item.eGpsType)
    {
        case GPSTYPE_NORMAL:
        {
            strParam = Server::CCommonUtil::StringFormat("ak=%s&output=json&coordtype=wgs84ll&location=%f,%f",
                m_strBaiDuKey.c_str(), std::stof(item.strLatitude), std::stof(item.strLongitude));
            break;
        }
        case GPSTYPE_BAIDU:
        {
            strParam = Server::CCommonUtil::StringFormat("ak=%s&output=json&coordtype=bd09ll&location=%f,%f",
                m_strBaiDuKey.c_str(), std::stof(item.strLatitude), std::stof(item.strLongitude));
            break;
        }
        default:
        {
            break;
        }
    }
    if(strParam.length() == 0)
    {
        return ;
    }
    printf("%s\n", strParam.c_str());
    string strUrl = Server::CCommonUtil::StringFormat("http://api.map.baidu.com/reverse_geocoding/v3/?%s", strParam.c_str());
    CURL* pCurl = curl_easy_init();
    if(NULL == pCurl)
    {
        return;
    }
    curl_easy_setopt(pCurl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10L);       // 总操作超时
    curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 5L);  // 连接阶段超时
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, RecvCallBack);
    std::string strResponse;
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &strResponse);
 
    CURLcode res = curl_easy_perform(pCurl);
    if (res != CURLE_OK) 
    {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        EnterCriticalSection(&m_Section);
        m_pGpsList.push_back(item);
        LeaveCriticalSection(&m_Section);
        curl_easy_cleanup(pCurl);
        return;
    }
    //printf("%s\n", strResponse.c_str());
    curl_easy_cleanup(pCurl);
    ParseGpsDetail(item, strResponse);
}
void CGpsManager::ParseGpsDetail(GpsItem item, string strGpsDetail)
{
    // {"status":0,"result":{"location":{"lng":118.75682754165996,"lat":32.05864509513405},"formatted_address":"江苏省南京市鼓楼区龙园东路","edz":{"name":""},"business":"虎踞路,凤凰西街,龙江","business_info":[{"name":"虎踞路","location":{"lng":118.7665861585196,"lat":32.05539093040219},"adcode":320106,"distance":173,"direction":"西"},{"name":"凤凰西街","location":{"lng":118.75436489171746,"lat":32.05337478850434},"adcode":320106,"distance":183,"direction":"北"},{"name":"龙江","location":{"lng":118.74718015441812,"lat":32.06012446430517},"adcode":320106,"distance":204,"direction":"东"}],"addressComponent":{"country":"中国","country_code":0,"country_code_iso":"CHN","country_code_iso2":"CN","province":"江苏省","city":"南京市","city_level":2,"district":"鼓楼区","town":"华侨路街道","town_code":"320106002","distance":"","direction":"","adcode":"320106","street":"龙园东路","street_number":""},"pois":[],"roads":[],"poiRegions":[],"sematic_description":"","formatted_address_poi":"","cityCode":315}}
    nlohmann::json jsonRoot;
    BOOL bRet = Server::CJsonUtil::FromString(strGpsDetail.c_str(), jsonRoot);
    if(FALSE == bRet)
    {
        return;
    }
    int iStatus = jsonRoot["status"];
    if(iStatus == 401)
    {
        printf("===Error===%s==\n", strGpsDetail.c_str());
        //{"status":401,"message":"当前并发量已经超过约定并发配额，限制访问"}
        EnterCriticalSection(&m_Section);
        m_pGpsList.push_back(item);
        LeaveCriticalSection(&m_Section);
        return;
    }
    if(0 != iStatus)
    {
        printf("===Error===%s==\n", strGpsDetail.c_str());
        return;
    }
    float dLongitude = jsonRoot["result"]["location"]["lng"];
    float dLatitude = jsonRoot["result"]["location"]["lat"];
    string strBdGps = "B-";
    strBdGps += Server::CCommonUtil::StringFormat("%06f", dLongitude);
    strBdGps += "&";
    strBdGps += Server::CCommonUtil::StringFormat("%06f", dLatitude);
    string strAddr = jsonRoot["result"]["formatted_address"];
    if(strAddr.length() == 0)
    {
        strAddr = "未知";
    }
    string strGps = "";
    if(item.eGpsType == GPSTYPE_NORMAL)
    {
        strGps = Server::CCommonUtil::StringFormat("%s&%s", item.strLongitude.c_str(), item.strLatitude.c_str());
    }
    printf("\n========\nGps:%s\nBdGps:%s\nStreet:%s\n======\n", strGps.c_str(), strBdGps.c_str(), strAddr.c_str());
    CMediaGpsTable gpsTable;
    gpsTable.SetRecord(strGps, strBdGps, strAddr);
    switch(item.eGpsTag)
    {
        case GPSTAG_UNCHECK:
        {
            if(item.eGpsType == GPSTYPE_NORMAL)
            {
                CMediaInfoTable::UpdateGpsWeiZhi(strGps, strBdGps, strAddr);
            }
            else if (item.eGpsType == GPSTYPE_BAIDU)
            {
                CMediaInfoTable::UpdateGpsWeiZhi(strBdGps, strBdGps, strAddr);
            }
            break;
        }
        case GPSTAG_SERVER:
        {
            //sync
            bRet = CMediaInfoTable::UpdateItemGpsAddr(item.iItemID, strBdGps, strAddr);
            break;
        }
        case GPSTAG_MEDIAPARSE:
        {
            bRet = CDbusUtil::BackupUpdateGps(item.iItemID, strBdGps, strAddr);
            break;
        }
        case GPSTAG_FORCEPRASE:
        {
            break;
        }
    }
}
void CGpsManager::AddDetectItem( GPSTYPE eGpsType, string strLongitude, string strLatitude,  int iItemID, GPSTAG eGpsTag)
{
    if(m_strBaiDuKey.length() == 0)
    {
        return;
    }
    GpsItem item = {};
    item.eGpsType = eGpsType;
    item.strLongitude = strLongitude;
    item.strLatitude = strLatitude;
    item.iItemID = iItemID;
    item.eGpsTag = eGpsTag;
    item.iFailedCount = 0;
    EnterCriticalSection(&m_Section);
    m_pGpsList.push_back(item);
    LeaveCriticalSection(&m_Section);
    
    MediaGpsItem gpsItem = {};
    switch (eGpsType)
    {
        case GPSTYPE_NORMAL:
        {
            gpsItem.strGps = Server::CCommonUtil::StringFormat("%s&%s", strLongitude.c_str(), strLatitude.c_str());
            break;
        }
        case GPSTYPE_BAIDU:
        {
            gpsItem.strGps2 = Server::CCommonUtil::StringFormat("B-%s&%s", strLongitude.c_str(), strLatitude.c_str());
            break;
        }
        default:
        {
            break;
        }
    }
    CMediaGpsTable gpsTable;
    gpsTable.SetRecord(gpsItem.strGps, gpsItem.strGps2, gpsItem.strLocation);
}
void CGpsManager::NeedCheck()
{
    EnterCriticalSection(&m_Section);
    m_iNeedCheckTimeSec = Server::CTools::CurTimeSec();
    LeaveCriticalSection(&m_Section);
}
BOOL CGpsManager::AddDetectItem(string strGps)
{
    if(m_strBaiDuKey.length() == 0)
    {
        return FALSE;
    }
    GPSTYPE eGpsType;
    string strLongitude;
    string strLatitude;
    ParseGps(strGps, &eGpsType, &strLongitude, &strLatitude);
    GpsItem item = {};
    item.eGpsType = eGpsType;
    item.strLongitude = strLongitude;
    item.strLatitude = strLatitude;
    item.iItemID = 0;
    item.eGpsTag = GPSTAG_FORCEPRASE;
    item.iFailedCount = 0;
    EnterCriticalSection(&m_Section);
    m_pGpsList.push_front(item);
    LeaveCriticalSection(&m_Section);
    CMediaGpsTable gpsTable;
    if(eGpsType == GPSTYPE_NORMAL)
    {
        gpsTable.SetRecord(strGps, "", "");
    }
    else if(eGpsType == GPSTYPE_BAIDU)
    {
        gpsTable.SetRecord("", strGps, "");
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}
BOOL CGpsManager::IsSupport()
{
    EnterCriticalSection(&m_Section);
    int iBaiDuAkLen = m_strBaiDuKey.length();
    LeaveCriticalSection(&m_Section);
    if(iBaiDuAkLen > 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}