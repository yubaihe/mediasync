#ifndef RELECH_NETWORKDRIVER__
#define RELECH_NETWORKDRIVER__
#include "stdafx.h"
#include "NetWorkUtil.h"
#define DETECTSTATUSINTERVALMILSEC_MIN 500
#define DETECTSTATUSINTERVALMILSEC_MAX 5000
typedef struct
{
   WPASTATUS eStatus;//无线网络当前状态
   int iConnectedNetWorkID;//当前连接的网络ID
   char szSsid[MAX_SSID_LEN]; //ssid
   char szIpAddr[MAX_IPADDR_LEN];//ip地址
   int iDetectStatusIntervalMilSec; //检测当前状态的间隔时间
   int iNeedConnectedNetWorkID;//需要连接的网络ID
   char szNeedConnectedNetWorkPwd[MAX_WIFIPASSWORDLEN];//需要连接的网络密码
   long iPrevScanTime; //上次scan的间隔时间
   HANDLE hDriverHandle;//线程句柄
   BOOL bRun;//是否需要运行
   BOOL bNeedDisConnect;
   CRITICAL_SECTION section;//互斥

}NetWorkDriver;

NetWorkDriver* NetWorkDriver_Start();
void NetWorkDriver_Stop(NetWorkDriver* pNetWorkDriver);
struct json_object* NetWorkDriver_GetWifiScanItems();
int NetWorkDriver_ConnectNetWork(int iNetWorkID, const char* pszPasswd);
void NetWorkDriver_DisConnectNetWork();
WPASTATUS NetWorkDriver_GetWlan1Status(char* pszSsid, char* pszIpAddr, int* iNetWorkID);
#endif