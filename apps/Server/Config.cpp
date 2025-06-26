#include "stdafx.h"
#include "Config.h"
#include <fstream>
#include <iomanip>
#include "Util/DbusUtil.h"
#include "CommonUtil.h"
CConfig* CConfig::m_pInstance = NULL;
CConfig::CConfig()
{
    InitializeCriticalSection(&m_Section);
    Init();
}
CConfig::~CConfig()
{
    DeleteCriticalSection(&m_Section);
}
CConfig* CConfig::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CConfig();
    }
    return m_pInstance;
}
void CConfig::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
BOOL CConfig::Init()
{
    //LOGD("=================CConfig::Init=================\n");
    char szCfgFile[MAX_PATH] = {0};
    readlink("/proc/self/exe", szCfgFile, MAX_PATH);
    char *psz = NULL;
    if (NULL != (psz = strrchr(szCfgFile, '/')))
    {
        *psz = '\0';
    }
    m_strConfigFile = szCfgFile;
    m_strConfigFile = m_strConfigFile.append("/Config.json");
    //printf("Config file:%s\n", m_strConfigFile.c_str());

    std::ifstream ifs(m_strConfigFile.c_str());
    if (!ifs.is_open())
    {
        printf("Open config file failed\n");
        return FALSE;  
    }
    std::stringstream buffer;  
    buffer << ifs.rdbuf();  
    ifs.close(); 

    m_JsonValue = nlohmann::json::parse(buffer.str());
	
	return TRUE;
}
BOOL CConfig::Store()
{
    std::ofstream ofs(m_strConfigFile.c_str());  
    if (!ofs.is_open()) 
    {  
        printf("写入Config文件失败了\n");
        return FALSE;  
    }
    ofs << std::setw(0) << m_JsonValue;  // 缩进4字符
    ofs.close();
    return TRUE;
}

ConfigDevice CConfig::GetDevice()
{
    ConfigDevice device = {};
    EnterCriticalSection(&m_Section);
    device.strDeviceID = m_JsonValue["dev"]["devid"];
    device.strDeviceName = m_JsonValue["dev"]["devname"];
    device.strDeviceVersion = m_JsonValue["dev"]["devversion"];
    device.iDeviceBlue = m_JsonValue["dev"]["devblue"];
    device.strDevPwd = m_JsonValue["dev"]["devpwd"];
    device.strDevPwdTip = m_JsonValue["dev"]["devpwdtip"];
    LeaveCriticalSection(&m_Section);
    return device;
}
BOOL CConfig::SetDevice(ConfigDevice device)
{
    EnterCriticalSection(&m_Section);
    m_JsonValue["dev"]["devid"] = device.strDeviceID;
    m_JsonValue["dev"]["devname"] = device.strDeviceName;
    m_JsonValue["dev"]["devversion"] = device.strDeviceVersion;
    m_JsonValue["dev"]["devpwd"] = device.strDevPwd;
    m_JsonValue["dev"]["devpwdtip"] = device.strDevPwdTip;
    m_JsonValue["dev"]["devblue"] = device.iDeviceBlue;
    BOOL bSuccess = Store();
    LeaveCriticalSection(&m_Section);
    return bSuccess;
}

 int CConfig::GetBroadCastPort()
 {
    EnterCriticalSection(&m_Section);
    int iBroadCastPort = m_JsonValue["port"]["broadcast"];
    LeaveCriticalSection(&m_Section);
    return iBroadCastPort;
 }
BOOL CConfig::SetBroadCastPort(int iBroadCastPort)
{
    EnterCriticalSection(&m_Section);
    m_JsonValue["port"]["broadcast"] = iBroadCastPort;
    BOOL bSuccess = Store();
    LeaveCriticalSection(&m_Section);
    return bSuccess;
}
ConfigStore CConfig::GetStore()
{
    ConfigStore store = {};
    EnterCriticalSection(&m_Section);
    store.strAddr = m_JsonValue["store"]["addr"];
    LeaveCriticalSection(&m_Section);
    return store;
}
BOOL CConfig::SetStore(ConfigStore store)
{
    EnterCriticalSection(&m_Section);
    m_JsonValue["store"]["addr"] = store.strAddr;
    BOOL bSuccess = Store();
    LeaveCriticalSection(&m_Section);
    return bSuccess;
}
BOOL CConfig::SupportLocalStorage()
{
    ConfigStore store = GetStore();
    if(store.strAddr.length() > 0)
    {
        return TRUE;
    }
    return FALSE;
}
BOOL CConfig::SupportSamba()
{
    EnterCriticalSection(&m_Section);
    BOOL bSupport = m_JsonValue["others"]["samba"];
    LeaveCriticalSection(&m_Section);
    return bSupport;
}
BOOL CConfig::SetSambaEnable(BOOL bEnable)
{
    EnterCriticalSection(&m_Section);
    m_JsonValue["others"]["samba"] = bEnable;
    BOOL bSuccess = Store();
    LeaveCriticalSection(&m_Section);
    return bSuccess;
}
BOOL CConfig::InitNetWork()
{
    if (m_JsonValue["others"].find("eth") != m_JsonValue["others"].end())
    {
        return TRUE;
    }
    string strEth = "";
    string strWlan = "";
    string strHotpot = "";
    BOOL bRet = CDbusUtil::GetNetworkConfig(strEth, strWlan, strHotpot);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    EnterCriticalSection(&m_Section);
    m_JsonValue["others"]["eth"] = strEth;
    m_JsonValue["others"]["wlan"] = strWlan;
    m_JsonValue["others"]["hotpot"] = strHotpot;
    BOOL bSuccess = Store();
    LeaveCriticalSection(&m_Section);
    return bSuccess;
}
BOOL CConfig::SupportWlan()
{
    if(FALSE == InitNetWork())
    {
        return FALSE;
    }
    string strWlan = m_JsonValue["others"]["wlan"];
    if(strWlan.length() == 0)
    {
        return FALSE;
    }
    return TRUE;
}
BOOL CConfig::SupportHotPot()
{
    if(FALSE == InitNetWork())
    {
        return FALSE;
    }
    string strHotpot = m_JsonValue["others"]["hotpot"];
    if(strHotpot.length() == 0)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL CConfig::SupportEth()
{
    if(FALSE == InitNetWork())
    {
        //没有network模块 说明只有有线
        return TRUE;
    }
    string strEth = m_JsonValue["others"]["eth"];
    string strWlan = m_JsonValue["others"]["wlan"];
    string strHotpot = m_JsonValue["others"]["hotpot"];
    CDbusUtil::GetNetworkConfig(strEth, strWlan, strHotpot);
    if(strEth.length() == 0 && strWlan.length() == 0 && strHotpot.length() == 0)
    {
        return TRUE;
    }
    if(strEth.length() == 0)
    {
        return FALSE;
    }
    return TRUE;
}
string CConfig::GetThumbRoot()
{
    ConfigStore store = GetStore();
    string strStore = store.strAddr;
    if(strStore[strStore.length() - 1] != '/')
    {
        strStore.append("/");
    }
    string strThumb = Server::CCommonUtil::StringFormat("%s.media/", strStore.c_str());
    return strThumb;
}
string CConfig::GetStoreRoot()
{
    ConfigStore store = GetStore();
    string strStore = store.strAddr;
    if(strStore[strStore.length() - 1] != '/')
    {
        strStore.append("/");
    }
    string strStoreRoot = strStore;
    return strStoreRoot;
}
string CConfig::GetUploadRoot()
{
    ConfigStore store = GetStore();
    string strStore = store.strAddr;
    if(strStore[strStore.length() - 1] != '/')
    {
        strStore.append("/");
    }
    string strUploadRoot = Server::CCommonUtil::StringFormat("%s.media_tmp/", strStore.c_str());
    return strUploadRoot;
}
string CConfig::GetExtraRoot()
{
    ConfigStore store = GetStore();
    string strStore = store.strAddr;
    if(strStore[strStore.length() - 1] != '/')
    {
        strStore.append("/");
    }
    string strUploadRoot = Server::CCommonUtil::StringFormat("%s.media_ex/", strStore.c_str());
    return strUploadRoot;
}
ConfigUser CConfig::GetUser()
{
    ConfigUser user = {};
    EnterCriticalSection(&m_Section);
    user.strPwd = m_JsonValue["user"]["pwd"];
    user.strPwdTip = m_JsonValue["user"]["pwdtip"];
    LeaveCriticalSection(&m_Section);
    return user;
}
BOOL CConfig::SetUser(ConfigUser user)
{
    EnterCriticalSection(&m_Section);
    m_JsonValue["user"]["pwd"] = user.strPwd;
    m_JsonValue["user"]["pwdtip"] = user.strPwdTip;
    BOOL bSuccess = Store();
    LeaveCriticalSection(&m_Section);
    return bSuccess;
}
BOOL CConfig::IsNeedLogin()
{
    ConfigUser user = GetUser();
    if(user.strPwd.length() == 0)
    {
        return FALSE;
    }
    return TRUE;
}
BOOL CConfig::IsSamePwd(string strPwd)
{
    ConfigUser user = GetUser();
    if(0 == user.strPwd.compare(strPwd))
    {
        return TRUE;
    }
    return FALSE;
}
string CConfig::GetPwdTip()
{
    ConfigUser user = GetUser();
    return user.strPwdTip;
}