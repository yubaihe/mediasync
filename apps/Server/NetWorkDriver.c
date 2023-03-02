#include "stdafx.h"
#include "NetWorkDriver.h"
#include "Tools.h"


extern NetWorkDriver* g_pNetWorkDriver;
int NetWorkDriver_WifiSortBySignal(const void *j1, const void *j2)
{
	json_object * const * pJson1 = (json_object* const*)j1;
	json_object * const * pJson2 = (json_object* const*)j2;
	int iSignal1 = json_object_get_int(json_object_object_get(*pJson1, "signal"));
    int iSignal2 = json_object_get_int(json_object_object_get(*pJson2, "signal"));
	return iSignal2 - iSignal1;
}
//#define PRINTTIME(a,b) do{char* pTtime = Tools_SecToTime(b);printf("%s~~%s\n", a, pTtime);free(pTtime);pTtime=NULL;}while(0);
DWORD NetWorkDriver_Proc(LPVOID* lpParameter)
{
    NetWorkDriver* pDriver = (NetWorkDriver*)lpParameter;
    long long iWifiStatusTimeMilSec = 0;//无线状态检测时间
    int iFastCount = 0;
    int iMaxFastCount = 30;
    while (pDriver->bRun)
    {
        long long iCurTime = Tools_CurTimeMilSec(); 
        //如果有需要断开wifi的
        if(pDriver->bNeedDisConnect)
        {
            pDriver->bNeedDisConnect = FALSE;
            NetWorkUtil_Disconnect();
        }
        //printf("DETECTSTATUSINTERVALMILSEC_MAX:%d~~~~%d\n", iFastCount, pDriver->iDetectStatusIntervalMilSec);
        if( DETECTSTATUSINTERVALMILSEC_MIN == pDriver->iDetectStatusIntervalMilSec && iFastCount++ > iMaxFastCount)
        {
            //9秒钟不行
            pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MAX;
            iFastCount = 0;
        }
        if(pDriver->iNeedConnectedNetWorkID >= 0)
        {
            pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MIN;
            iFastCount = 0;
        }
      
        //查询网络状态
        if(iCurTime - iWifiStatusTimeMilSec > pDriver->iDetectStatusIntervalMilSec)
        {
            iWifiStatusTimeMilSec = iCurTime;
            EnterCriticalSection(&pDriver->section);
            //PRINTTIME("iDetectStatusIntervalMilSec", iCurTime/1000);
            pDriver->eStatus = NetWorkUtil_GetWlan1Status(pDriver->szSsid, pDriver->szIpAddr, &pDriver->iConnectedNetWorkID);
            LeaveCriticalSection(&pDriver->section);
            switch (pDriver->eStatus)
            {
               case WPASTATUS_SCANNING:
               {
                   //正在连接 要一直fast取状态
                   if(iFastCount > iMaxFastCount)
                   {
                        //printf("NetWorkUtil_Disconnect\n");
                        //NetWorkUtil_Disconnect();
                        //printf("~~~%d~~%d~~%d\n", iFastCount, iMaxFastCount, pDriver->iDetectStatusIntervalMilSec);
                        pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MAX;
                        iFastCount = 0;
                   }
                   break;
               }
               case WPASTATUS_ASSOCIATED:
               case WPASTATUS_ASSOCIATING:
               {
                   //正在关联 要延时一下
                   pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MIN;
                   iFastCount = 0;
                   break;
               }
               case WPASTATUS_COMPLETED:
               {
                   if(strlen(pDriver->szIpAddr) > 0)
                   {
                        if(iFastCount > 0)
                        {
                            pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MAX;
                            iFastCount = 0;
                        }
                   }
                   else
                   {
                       //正在获取IP地址 要一直fast取状态
                        pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MIN;
                        iFastCount = 0;
                   }
                   break;
               }
               default:
               {
                   break;
               }
            }

            if(pDriver->iNeedConnectedNetWorkID < 0 && pDriver->eStatus == WPASTATUS_COMPLETED && strlen(pDriver->szIpAddr) == 0)
            {
                //连接上了 也不需要更换连接 但是没有获取到IP地址 启动udhcpc读取IP地址
                NetWorkUtil_RestartWlan1Udhcpc();
            }
            
            if(pDriver->iNeedConnectedNetWorkID >= 0)
            {
                EnterCriticalSection(&pDriver->section);
                pDriver->iConnectedNetWorkID = NetWorkUtil_ConnectWifiItem(pDriver->iNeedConnectedNetWorkID, pDriver->szNeedConnectedNetWorkPwd);
                pDriver->iNeedConnectedNetWorkID = -1;
                memset(pDriver->szNeedConnectedNetWorkPwd, 0, MAX_WIFIPASSWORDLEN);
                LeaveCriticalSection(&pDriver->section);
                pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MIN;
                iFastCount = 0;
            }
        }

        usleep(300 * 1000);
    }
    return 1;
}

void NetWorkDriver_Init(NetWorkDriver* pDriver)
{
    pDriver->bRun = TRUE;
    pDriver->bNeedDisConnect = FALSE;
    pDriver->eStatus = WPASTATUS_INACTIVE;
    pDriver->iDetectStatusIntervalMilSec = DETECTSTATUSINTERVALMILSEC_MIN;
    pDriver->iNeedConnectedNetWorkID = -1;
    pDriver->iConnectedNetWorkID = -1;
    InitializeCriticalSection(&pDriver->section);
}

NetWorkDriver* NetWorkDriver_Start()
{
    int iLen = sizeof(NetWorkDriver);
    char* pBuffer = (char*)malloc(iLen);
    memset(pBuffer, 0, iLen);
    NetWorkDriver* pDriver = (NetWorkDriver*)pBuffer;
    NetWorkDriver_Init(pDriver);
    
    pDriver->hDriverHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NetWorkDriver_Proc, pDriver, 0, NULL);
  
    return pDriver;
}

void NetWorkDriver_Stop(NetWorkDriver* pNetWorkDriver)
{
    pNetWorkDriver->bRun = FALSE;
    WaitForSingleObject(pNetWorkDriver->hDriverHandle, INFINITE);
    CloseHandle(pNetWorkDriver->hDriverHandle);
    pNetWorkDriver->hDriverHandle = NULL;
    DeleteCriticalSection(&pNetWorkDriver->section);
    free((char*)pNetWorkDriver);
    pNetWorkDriver = NULL;
}

struct json_object* NetWorkDriver_GetWifiScanItems()
{
    long long iCurTime = Tools_CurTimeMilSec();  
    if(iCurTime/1000 - g_pNetWorkDriver->iPrevScanTime > 20)
    {
        //10S钟左右扫描一次附近wifi设备
        g_pNetWorkDriver->iPrevScanTime = iCurTime/1000;
        NetWorkUtil_WifiScan();
    }
    struct json_object* pScanItems = NetWorkUtil_GetWifiScanItems();
    int iItemLen = json_object_array_length(pScanItems);
    if(iItemLen > 0)
    {
        json_object_array_sort(pScanItems, NetWorkDriver_WifiSortBySignal);
        
    } 
    return pScanItems;
}

int NetWorkDriver_ConnectNetWork(int iNetWorkID, const char* pszPasswd)
{
    EnterCriticalSection(&g_pNetWorkDriver->section);
    g_pNetWorkDriver->iNeedConnectedNetWorkID = iNetWorkID;
    if(strlen(pszPasswd) > 0)
    {
        strcpy(g_pNetWorkDriver->szNeedConnectedNetWorkPwd, pszPasswd);
    }
    
    LeaveCriticalSection(&g_pNetWorkDriver->section);
    return iNetWorkID;
}

void NetWorkDriver_DisConnectNetWork()
{
    g_pNetWorkDriver->bNeedDisConnect = TRUE;
}

WPASTATUS NetWorkDriver_GetWlan1Status(char* pszSsid, char* pszIpAddr, int* iNetWorkID)
{
    EnterCriticalSection(&g_pNetWorkDriver->section);
    strcpy(pszSsid, g_pNetWorkDriver->szSsid);
    strcpy(pszIpAddr, g_pNetWorkDriver->szIpAddr);
    *iNetWorkID = g_pNetWorkDriver->iConnectedNetWorkID;
    WPASTATUS eStatus = g_pNetWorkDriver->eStatus;
    LeaveCriticalSection(&g_pNetWorkDriver->section);
    return eStatus;
}
