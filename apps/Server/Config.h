#pragma once
#include "stdafx.h"
struct ConfigDevice
{
    string strDeviceID;
    string strDeviceName;
    string strDeviceVersion;
    int iDeviceBlue;
    string strDevPwd;
    string strDevPwdTip;
    void Print()
    {
        printf("DeviceID:%s\n", strDeviceID.c_str());
        printf("DeviceName:%s\n", strDeviceName.c_str());
        printf("DeviceVersion:%s\n", strDeviceVersion.c_str());
        printf("DeviceBlue:%d\n", iDeviceBlue);
        printf("DevPwd:%s\n", strDevPwd.c_str());
        printf("DevPwdTip:%s\n", strDevPwdTip.c_str());
    }
};
struct ConfigStore
{
    string strAddr;   // 同步到这个目录,如果这个目录为空 标识挂接在外部存储上 否则挂载到指定目录 那么外部存储是用来同步的
    void Print()
    {
        printf("Addr:%s\n", strAddr.c_str());
    }
};
struct ConfigOther
{
    BOOL bSambaEnable;
    void Print()
    {
        printf("SambaEnable:%d\n", bSambaEnable);
    }
};
struct ConfigUser
{
    //userpwd        用户密码(MD5值)
    string strPwd;
	//userpwdtip     用户密码提示信息(MD5值)
    string strPwdTip;
    void Print()
    {
        printf("Pwd:%s\n", strPwd.c_str());
        printf("PwdTip:%s\n", strPwdTip.c_str());
    }
};

class CConfig
{
public:
    CConfig();
    ~CConfig();
    static CConfig* GetInstance();
    static void Release();

    
    ConfigDevice GetDevice();
    BOOL SetDevice(ConfigDevice device);

    int GetBroadCastPort();
    BOOL SetBroadCastPort(int iBroadCastPort);

    ConfigStore GetStore();
    BOOL SetStore(ConfigStore store);

    BOOL SupportLocalStorage();
    BOOL SupportSamba();
    BOOL SupportWlan();
    BOOL SupportHotPot();
    BOOL SupportEth();
    BOOL SetSambaEnable(BOOL bEnable);
    
    string GetThumbRoot();
    string GetStoreRoot();
    string GetUploadRoot();
    string GetExtraRoot();
    string GetTempRoot();

    ConfigUser GetUser();
    BOOL SetUser(ConfigUser user);
    BOOL IsNeedLogin();
    BOOL IsSamePwd(string strPwd);
    string GetPwdTip();
private:
    BOOL Init();
    BOOL Store();
    BOOL InitNetWork();
private:
    string m_strConfigFile;
    nlohmann::json m_JsonValue;
    CRITICAL_SECTION m_Section;
    static CConfig* m_pInstance;
};

