#include "stdafx.h"
#include "Config.h"
#include "BroadCastServer.h"
#include "ConnectServer.h"
#include "Util/CommonUtil.h"
#include "MediaDb.h"
#include "ClearCache.h"
#include "Util/JsonUtil.h"
#include "FileUtil.h"
#include "GpsManager.h"
#include "Butler.h"
//ntp自动更新 
//timedatectl status    查询状态
//timedatectl set-ntp no   　　     关闭
//timedatectl set-ntp yes           开启
// ll /proc/19820/fd | wc -l 
//由于添加了修改时间指令 需要以sodu身份运行
//sudo valgrind --tool=memcheck  --leak-check=full --show-leak-kinds=all ./Server

//sudo wpa_cli ifname
BOOL g_exit;
CBroadCastServer g_BroadCastServer;
CConnectServer g_ConnectServer;
void CommonSdk_WriteLog(LOGLEVEL eLogLevel, const char* pszFun, const char* pszFile, int iLine, const char* pszLog)
{
    printf("%s\n", pszLog);
}

void OnSystemSig(int iSigNo) 
{
	printf("OnSystemSig\n");
	g_exit = TRUE;
}
void OnSignalHandler(int nSigno)
{
	printf("OnSignalHandler\n");
}
int main(int iArgc, char* pszArgv[])
{
    curl_global_init(CURL_GLOBAL_ALL);
    for(int i = 0; i < iArgc; ++i)
    {
        printf("%d==>%s==\n", i, pszArgv[i]);
    }
    if(iArgc >= 2)
    {
        ConfigStore store = {};
        store.strAddr = pszArgv[1];
        CConfig::GetInstance()->SetStore(store);
    }
    printf("hello wod\n");
    signal(SIGINT, OnSystemSig); 
	signal(SIGPIPE,  OnSignalHandler);
    //初始化数据库
    BOOL bSupportSamba = Server::CCommonUtil::ExecuteCmdWithOutReplay("smbd --version");
    CConfig::GetInstance()->SetSambaEnable(bSupportSamba);

    string strStore = CConfig::GetInstance()->GetStoreRoot();
    CFileUtil::CreateFold(strStore.c_str());
    strStore = strStore.append(".mediasync.db");
    LOGD("Db File:%s\n", strStore.c_str());
    CMediaDb::GetInstance()->InitAllTables(strStore.c_str());
    //启动广播服务
    int iBroadCastPort = CConfig::GetInstance()->GetBroadCastPort();
    BOOL bRet = g_BroadCastServer.Start(iBroadCastPort);
    if(FALSE == bRet)
    {
        printf("Start BroadCast:%d failed\n", iBroadCastPort);
        return 0;
    }
    printf("Start BroadCast:%d Success\n", iBroadCastPort);
    //启动服务
    bRet = g_ConnectServer.Start("Server");
    if(FALSE == bRet)
    {
        printf("Start Server failed\n");
        return 0;
    }
    printf("Start Server Success\n");
    g_exit = FALSE;
    //int iIndex = 0;
    //CMediaGpsTable gpsTable;
    //gpsTable.RemoveAll();
    CButler::GetInstance();
    while (FALSE == g_exit)
    {
        sleep(3);
        //if(0 == iIndex)
        //{
        //     CGpsManager::GetInstance()->AddDetectItem(GPSTYPE_NORMAL, "118.745003", "32.054980", 0, 0);
        //     //iIndex = 100;
        //}
    }
    CGpsManager::Release();
    CConfig::Release();
    CClearCache::Release();
    CMediaDb::Release();
    CButler::Release();
    curl_global_cleanup();
    return 1;
}
